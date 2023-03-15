/**
 * @file    motion_interface.cpp
 * @brief   this contains the acam interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Acam namespace,
 * and is how the acam daemon interfaces to the acam hardware.
 *
 */

#include "motion_interface.h"

namespace Acam {

  /**************** Acam::MotionInterface::MotionInterface ********************/
  /**
   * @brief      class constructor
   *
   */
  MotionInterface::MotionInterface() {
    this->numdev=-1;
  }
  /**************** Acam::MotionInterface::MotionInterface ********************/


  /**************** Acam::MotionInterface::~MotionInterface *******************/
  /**
   * @brief      class deconstructor
   *
   */
  MotionInterface::~MotionInterface() {
  }
  /**************** Acam::MotionInterface::~MotionInterface *******************/


  /**************** Acam::MotionInterface::initialize_class *******************/
  /**
   * @brief      initializes the class from configure_acam()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Acam::Server::configure_acam() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long MotionInterface::initialize_class() {
    std::string function = "Acam::MotionInterface::initialize_class";
    std::stringstream message;
    long error = ERROR;

    if ( this->port < 0 || this->host.empty() ) {
      logwrite( function, "ERROR: host or port invalid" );
      return( ERROR );
    }

    Physik_Instrumente::ServoInterface s( this->name, this->host, this->port );
    this->pi = s;

    this->numdev = this->motion_info.size();

    if ( this->numdev == 2 ) {
      logwrite( function, "interface initialized ok" );
      error = NO_ERROR;
    }
    else if ( this->numdev > 2 ) {
      message.str(""); message << "ERROR: too many motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }
    else {
      message.str(""); message << "ERROR: insufficient motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }

    return( error );
  }
  /**************** Acam::MotionInterface::initialize_class *******************/


  /**************** Acam::Interface::open *************************************/
  /**
   * @brief      opens the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::open( ) {
    std::string function = "Acam::MotionInterface::open";
    std::stringstream message;

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Acam::Server::configure_acam).
    //
    if ( !this->pi.is_initialized() ) {
      logwrite( function, "ERROR: pi interface not initialized" );
      return( ERROR );
    }

    // open a connection
    //
    long error = this->pi.open();

    if ( error != NO_ERROR ) return( error );

    // clear any error codes on startup, and
    // enable the servo for each address in motion_info
    //
    for ( size_t con=0; con < this->numdev; con++ ) {
      try {
        int axis=1;
        int errcode;
        error |= this->pi.get_error( this->motion_info.at(con).addr, errcode );     // read error to clear, don't care the value
        error |= this->pi.set_servo( this->motion_info.at(con).addr, axis, true );  // turn the servos on
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: controller element " << con << " out of range";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    return( error );
  }
  /**************** Acam::MotionInterface::open *******************************/


  /**************** Acam::MotionInterface::close ******************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::close( ) {
    std::string function = "Acam::MotionInterface::close";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "not connected" );
      return( NO_ERROR );
    }

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Acam::Server::configure_acam).
    //
    if ( !this->pi.is_initialized() ) {
      logwrite( function, "ERROR: pi interface not initialized" );
      return( ERROR );
    }

    return( this->pi.close() );
  }
  /**************** Acam::MotionInterface::close ******************************/


  /**************** Acam::MotionInterface::home *******************************/
  /**
   * @brief      home all daisy-chained motors using the neg limit switch
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::home( ) {
    std::string function = "Acam::MotionInterface::home";
    std::stringstream message;
    int axis=1;
    long error = NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // send the home_axis command for each address in motion_info
    //
    for ( size_t con=0; con < this->numdev; con++ ) {
      try {
        this->pi.home_axis( this->motion_info.at(con).addr, axis, "neg" );
        this->motion_info.at(con).ishome   = false;
        this->motion_info.at(con).ontarget = false;
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: controller element " << con << " out of range";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    // Loop sending the is_home command for each address in motion_info
    // until homed or timeout.
    //

    // get the time now for timeout purposes
    //
    std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

    do {
      size_t num_home=0;
      for ( size_t con=0; con < this->numdev; con++ ) {
        try {
          bool state;
          this->pi.is_home( this->motion_info.at(con).addr, axis, state );
          this->motion_info.at(con).ishome = state;
          this->motion_info.at(con).ontarget = state;
          if ( this->motion_info.at(con).ishome ) num_home++;
        }
        catch( std::out_of_range &e ) {
          message.str(""); message << "ERROR: controller element " << con << " out of range";
          logwrite( function, message.str() );
          return( ERROR );
        }
      }
      if ( num_home == this->numdev ) break;
      else {
#ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] waiting for homing..." );
#endif
        usleep( 1000000 );
      }

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > MOVE_TIMEOUT ) {
        logwrite( function, "TIMEOUT waiting for homing" );
        error = TIMEOUT;
        break;
      }

    } while ( 1 );

    return( error );
  }
  /**************** Acam::MotionInterface::home *******************************/


  /**************** Acam::MotionInterface::is_home ****************************/
  /**
   * @brief       return the home state of the motors
   * @param[out]  retstring  contains the home state ("true" | "false")
   * @return      ERROR or NO_ERROR
   *
   * All motors must be homed for this to return "true".
   *
   */
  long MotionInterface::is_home( std::string &retstring ) {
    std::string function = "Acam::MotionInterface::is_home";
    std::stringstream message;
    std::stringstream homestream;
    long error = NO_ERROR;

    // Loop through all motor controllers, asking each if homed,
    // setting each controller's .ishome flag, and keeping count 
    // of the number that are homed.
    //
    size_t num_home=0;
    for ( size_t con=0; con < this->numdev; con++ ) {
      try {
        int axis=1;
        error |= this->pi.is_home( this->motion_info.at(con).addr,
                                   axis,
                                   this->motion_info.at(con).ishome );  // error is OR'd so any error is preserved
        homestream << this->motion_info.at(con).addr << ":"
                   << ( this->motion_info.at(con).ishome ? "true" : "false" ) << " ";
        if ( this->motion_info.at(con).ishome ) num_home++;
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: controller element " << con << " out of range";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    // Set the retstring true or false, true only if all controllers are homed.
    //
    if ( num_home == this->numdev ) retstring = "true"; else retstring = "false";

    // If not all are the same state then log that
    //
    if ( num_home > 0 && num_home < this->numdev ) {
      message.str(""); message << "NOTICE: " << homestream.str();
      logwrite( function, message.str() );
    }

    return( error );
  }
  /**************** Acam::MotionInterface::is_home ****************************/


  /**************** Acam::MotionInterface::send_command ***********************/
  /**
   * @brief      writes the raw command as received to the master controller
   * @param[in]  cmd
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command that expects no reply.
   *
   */
  long MotionInterface::send_command( std::string cmd ) {
    std::string function = "Acam::MotionInterface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( this->pi.send_command( cmd ) );
  }
  /**************** Acam::MotionInterface::send_command ***********************/


  /**************** Acam::MotionInterface::send_command ***********************/
  /**
   * @brief      writes the raw command to the master controller, reads back reply
   * @param[in]  cmd
   * @param[in]  retstring
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command and then reads a reply if that command contains
   * a question mark, "?".
   *
   */
  long MotionInterface::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( cmd.find( "?" ) != std::string::npos ) return( this->pi.send_command( cmd, retstring ) );
    else return( this->pi.send_command( cmd ) );
  }
  /**************** Acam::MotionInterface::send_command ***********************/


  /**************** Acam::MotionInterface::filter *****************************/
  /**
   * @brief      set or get the filter
   * @param[in]  iface      reference to main Acam::MotionInterface object
   * @param[in]  args       string containing something
   * @param[out] retstring  string contains something
   * @return     ERROR or NO_ERROR
   *
   * This function requires a reference to the acam interface object because it's
   * going to spawn threads for each motor and the threads, being static, would
   * not otherwise have access to this-> object.
   *
   */
  long MotionInterface::filter( Acam::MotionInterface &iface, std::string args, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::filter";
    std::stringstream message;
    long error = NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( error );
  }
  /**************** Acam::MotionInterface::filter *****************************/


  /**************** Acam::MotionInterface::cover ******************************/
  /**
   * @brief      set or get the cover
   * @param[in]  iface      reference to main Acam::MotionInterface object
   * @param[in]  args       string containing something
   * @param[out] retstring  string contains something
   * @return     ERROR or NO_ERROR
   *
   * This function requires a reference to the acam interface object because it's
   * going to spawn threads for each motor and the threads, being static, would
   * not otherwise have access to this-> object.
   *
   */
  long MotionInterface::cover( Acam::MotionInterface &iface, std::string args, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::cover";
    std::stringstream message;
    long error = NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( error );
  }
  /**************** Acam::MotionInterface::cover ******************************/

}
