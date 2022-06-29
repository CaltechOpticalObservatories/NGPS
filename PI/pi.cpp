/**
 * @file    pi.cpp
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "pi.h"
#include "logentry.h"

namespace Physik_Instrumente {

  /**************** Physik_Instrumente::ServoInterface::ServoInterface ********/
  /**
   * @fn     ServoInterface
   * @brief  default ServoInterface class constructor
   * @param  none
   * @return none
   *
   */
  ServoInterface::ServoInterface() {
    this->name = "";
    this->host = "";
    this->port=-1;
    this->initialized = false;
  }
  ServoInterface::ServoInterface( std::string host, int port ) {
    this->name = "";
    this->host = host;
    this->port = port;
    this->initialized = true;
  }
  ServoInterface::ServoInterface( std::string name, std::string host, int port ) {
    this->name = name;
    this->port = port;
    this->host = host;
    this->initialized = true;
  }
  /**************** Physik_Instrumente::ServoInterface::ServoInterface ********/


  /**************** Physik_Instrumente::ServoInterface::~ServoInterface *******/
  /**
   * @fn     ~ServoInterface
   * @brief  ServoInterface class deconstructor
   * @param  none
   * @return none
   *
   */
  ServoInterface::~ServoInterface() {
  };
  /**************** Physik_Instrumente::ServoInterface::~ServoInterface *******/


  /**************** Physik_Instrumente::ServoInterface::open ******************/
  /**
   * @fn     open
   * @brief  open a connection to the servo controller
   * @param  
   * @return ERROR or NO_ERROR
   *
   */
  long ServoInterface::open() {
    std::string function = "Physik_Instrumente::ServoInterface::open";
    std::stringstream message;

    if ( this->controller.isconnected() ) {
      logwrite( function, "connection already open" );
      return( NO_ERROR );
    }

    Network::TcpSocket s( this->host, this->port );
    this->controller = s;

    // Initialize connection to the servo controller
    //
    message.str(""); message << "opening connection to " << this->name << " controller";
    logwrite( function, message.str() );

    if ( this->controller.Connect() != 0 ) {
      message.str(""); message << "ERROR connecting to " << this->name << " controller";
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str(""); message << "socket connection to " 
                             << this->controller.gethost() << ":" << this->controller.getport()
                             << " established on fd " << this->controller.getfd();
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /**************** Physik_Instrumente::ServoInterface::open ******************/


  /**************** Physik_Instrumente::ServoInterface::close *****************/
  /**
   * @fn     close
   * @brief  close the connection to the servo controller
   * @param  
   * @return ERROR or NO_ERROR
   *
   */
  long ServoInterface::close() {
    std::string function = "Physik_Instrumente::ServoInterface::close";
    std::stringstream message;

    if ( !this->controller.isconnected() ) {
      logwrite( function, "connection already closed" );
      return( NO_ERROR );
    }

    long error = this->controller.Close();

    if ( error == NO_ERROR ) {
      logwrite( function, "connection to servo controller closed" );
    }
    else {
      logwrite( function, "error closing servo connection" );
    }

    return( error );
  }
  /**************** Physik_Instrumente::ServoInterface::close *****************/


  /**************** Physik_Instrumente::ServoInterface::move_abs **************/
  /**
   * @fn     move_abs
   * @brief  send move command in absolute coordinates
   * @param  int addr, address of controller
   * @param  int axis, axis to move
   * @param  float pos, absolute position to move to
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   *
   */
  long ServoInterface::move_abs( int addr, float pos ) {
    return( this->move_abs( addr, -1, pos ) );
  }
  long ServoInterface::move_abs( int addr, int axis, float pos ) {
    std::stringstream cmd;
    if ( addr > 0 ) cmd << addr << " ";
    cmd << "MOV";
    if ( axis > 0 ) cmd << " " << axis;
    cmd << " " << pos;
    this->send_command( cmd.str() );
    return NO_ERROR;
  }
  /**************** Physik_Instrumente::ServoInterface::move_abs **************/


  /**************** Physik_Instrumente::ServoInterface::move_rel **************/
  /**
   * @fn     move_rel
   * @brief  move in relative coordinates
   * @param  int addr, address of controller
   * @param  int axis, axis to move
   * @param  float pos, absolute position to move to
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   *
   */
  long ServoInterface::move_rel( int addr, float pos ) {
    return( this->move_rel( addr, -1, pos ) );
  }
  long ServoInterface::move_rel( int addr, int axis, float pos ) {
    std::stringstream cmd;
    if ( addr > 0 ) cmd << addr << " ";
    cmd << "MVR";
    if ( axis > 0 ) cmd << " " << axis;
    cmd << " " << pos;
    this->send_command( cmd.str() );
    return NO_ERROR;
  }
  /**************** Physik_Instrumente::ServoInterface::move_rel **************/


  /**************** Physik_Instrumente::ServoInterface::home_axis *************/
  /**
   * @fn     home_axis
   * @brief  home an axis by moving to reference switch
   * @param  int addr, address of controller in daisy-chain
   * @param  int axis, axis to move
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  long ServoInterface::home_axis( int addr ) {
    return( this->home_axis( addr, -1 ) );       //!< all axes at this addr
  }
  long ServoInterface::home_axis( int addr, int axis ) {
    std::string function = "Physik_Instrumente::ServoInterface::home_axis";
    std::stringstream message;
    std::stringstream cmd;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "FRF";
    if ( axis > 0 ) cmd << " " << axis;

    this->send_command( cmd.str() );

    return NO_ERROR;
  }
  /**************** Physik_Instrumente::ServoInterface::home_axis *************/


  /**************** Physik_Instrumente::ServoInterface::is_home ***************/
  /**
   * @fn     is_home
   * @brief  queries whether referencing has been done
   * @param  int addr, address of controller in daisy-chain
   * @param  int axis, axis to move
   * @param  string &retsrting, contains return value (1=homed, 0=not homed)
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  long ServoInterface::is_home( int addr, std::string &retstring ) {
    return( this->is_home( addr, -1, retstring ) );       //!< all axes at this addr
  }
  long ServoInterface::is_home( int addr, int axis, std::string &retstring ) {
    std::string function = "Physik_Instrumente::ServoInterface::is_home";
    std::stringstream message;
    std::stringstream cmd;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "FRF?";
    if ( axis > 0 ) cmd << " " << axis;

    this->send_command( cmd.str(), retstring );

    return NO_ERROR;
  }
  /**************** Physik_Instrumente::ServoInterface::is_home ***************/


  /**************** Physik_Instrumente::ServoInterface::on_target *************/
  /**
   * @fn     on_target
   * @brief  query the on target state for given addr and axis
   * @param  int addr, address of controller in daisy-chain
   * @param  int axis, axis to move
   * @param  string &retsrting, contains return value (1=on_target, 0=not on_target)
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  long ServoInterface::on_target( int addr, std::string &retstring ) {
    return( this->on_target( addr, -1, retstring ) );       //!< all axes at this addr
  }
  long ServoInterface::on_target( int addr, int axis, std::string &retstring ) {
    std::string function = "Physik_Instrumente::ServoInterface::on_target";
    std::stringstream message;
    std::stringstream cmd;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "ONT?";
    if ( axis > 0 ) cmd << " " << axis;

    this->send_command( cmd.str(), retstring );

    return NO_ERROR;
  }
  /**************** Physik_Instrumente::ServoInterface::on_target *************/


  /**************** Physik_Instrumente::ServoInterface::get_pos ***************/
  /**
   * @fn     get_pos
   * @brief  get the current position of a motor
   * @param  int addr, address of controller in daisy-chain
   * @param  int axis, axis to read
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  long ServoInterface::get_pos( int addr, std::string &retstring ) {
    return( this->get_pos( addr, -1, retstring ) );       //!< all axes at this addr
  }
  long ServoInterface::get_pos( int addr, int axis, std::string &retstring ) {
    std::string function = "Physik_Instrumente::ServoInterface::get_pos";
    std::stringstream message;
    std::stringstream cmd;
    long error, retval;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "POS?";
    if ( axis > 0 ) cmd << " " << axis;

    this->send_command( cmd.str(), retstring );

    return NO_ERROR;
  }
  /**************** Physik_Instrumente::ServoInterface::get_pos ***************/


  /**************** Physik_Instrumente::ServoInterface::stop_motion ***********/
  /**
   * @fn     stop_motion
   * @brief  stop all movement on all axes
   * @param  addr
   * @return ERROR or NO_ERROR
   *
   */
  long ServoInterface::stop_motion( int addr ) {
    std::stringstream cmd;
    if ( addr > 0 ) cmd << addr << " ";
    cmd << "STP";
    this->send_command( cmd.str() );
    return NO_ERROR;
  }
  /**************** Physik_Instrumente::ServoInterface::stop_motion ***********/


  /**************** Physik_Instrumente::ServoInterface::send_command **********/
  /**
   * @fn     send_command
   * @brief  send a command string to the controller
   * @param  string command
   * @param  string &reply (if needed)
   * @return ERROR or NO_ERROR
   *
   * The needed linefeed \n is added here
   *
   * This function is overloaded with a version that accepts a return string.
   * If called with a return string then it is assumed that a reply to the
   * command is needed, in which case after writing the command the reply is
   * read and placed into the return string.
   *
   */
  long ServoInterface::send_command( std::string cmd ) {
    std::string function = "Physik_Instrumente::ServoInterface::send_command";
    std::stringstream message;

    logwrite( function, cmd );

    cmd.append( "\n" );                            // add the newline character

    int written = this->controller.Write( cmd );   // write the command

    if ( written <= 0 ) return( ERROR );           // return error if error writing to socket

    return( NO_ERROR );
  }
  long ServoInterface::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Physik_Instrumente::ServoInterface::send_command";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // send the command
    //
    long retval = this->send_command( cmd );

    if ( retval == ERROR ) {
      message.str(""); message << "ERROR sending command: " << cmd;
      logwrite( function, message.str() );
    }

    // read the reply
    //
    do {

      if ( ( retval=this->controller.Poll() ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "Poll timeout waiting for response"; error = TIMEOUT; }
        if ( retval <0 ) { message.str(""); message << "Poll error waiting for response"; error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = this->controller.Read( reply, '\n' ) ) < 0 ) {
        message.str(""); message << "error reading from motor controller: " << strerror( errno );
        logwrite( function, message.str() );
        break;
      }

      // remove any newline characters and get out
      //
      reply.erase(std::remove(reply.begin(), reply.end(), '\r' ), reply.end());
      reply.erase(std::remove(reply.begin(), reply.end(), '\n' ), reply.end());
      break;

    } while ( retval>=0 );

    retstring = reply;

    return( error );
  }
  /**************** Physik_Instrumente::ServoInterface::send_command **********/


}
