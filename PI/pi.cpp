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


  /**************** Physik_Instrumente::ServoInterface::get_error *************/
  /**
   * @fn         get_error
   * @brief      set the servo on|off
   * @param[in]  int addr, address of controller
   * @param[in]  bool state (on=true, off=false)
   * @return     ERROR or NO_ERROR
   *
   */
  long ServoInterface::get_error( int addr, int &errcode ) {
    std::string function = "Physik_Instrumente::ServoInterface::get_error";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "ERR?";

    long error = this->send_command( cmd.str(), reply );                       // send the command

    if ( error != NO_ERROR ) return error;

    // When an addr is specified (>0) then the response will be 0 A R
    // where A is addr and R is the response. Grab the last token string here...
    //
    if ( addr > 0 ) {
      std::vector<std::string> tokens;
      Tokenize( reply, tokens, " " );
      if ( tokens.size() != 3 ) {
        errcode = -1;
        message.str(""); message << "ERROR bad reply: received " << tokens.size() << " but expected 3";
        logwrite( function, message.str() );
        return( ERROR );
      }
      try {
        reply = tokens.at(2);                    // set the reply string to the last token
      }
      catch ( std::out_of_range & ) {
        errcode = -1;
        logwrite( function, "ERROR reply token out of range" );
        return( ERROR );
      }
    }

    // Now, reply has either been over-written above (for addr>0)
    // or in the case where addr was not specified (-1) then then response will be simply, R
    // which is stored in reply. In either case, convert reply to <int> and that is the errcode.
    //
    try {
      errcode = std::stoi( reply );
    }
    catch ( std::invalid_argument & ) {
      errcode = -1;
      logwrite( function, "ERROR bad reply: unable to convert to integer" );
      return( ERROR );
    }
    catch ( std::out_of_range & ) {
      errcode = -1;
      logwrite( function, "ERROR reply out of integer range" );
      return( ERROR );
    }

    return error;
  }
  /**************** Physik_Instrumente::ServoInterface::get_error *************/


  /**************** Physik_Instrumente::ServoInterface::set_servo *************/
  /**
   * @fn         set_servo
   * @brief      set the servo on|off
   * @param[in]  int addr, address of controller
   * @param[in]  bool state (on=true, off=false)
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   *
   */
  long ServoInterface::set_servo( int addr, bool state ) {
    return( this->set_servo( addr, -1, state ) );
  }
  long ServoInterface::set_servo( int addr, int axis, bool state ) {
    std::stringstream cmd;
    if ( addr > 0 ) cmd << addr << " ";
    cmd << "SVO";
    if ( axis > 0 ) cmd << " " << axis;
    cmd << " " << ( state ? 1 : 0 );
    this->send_command( cmd.str() );
    return NO_ERROR;
  }
  /**************** Physik_Instrumente::ServoInterface::set_servo *************/


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
   * @param[in]  int addr, address of controller in daisy-chain
   * @param[in]  string ref, what to use for the homing
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   * Homing can be performed using a reference switch (if equipped), or the 
   * positive or negative limit switches, as indicated by the "ref" argument.
   *
   */
  long ServoInterface::home_axis( int addr, std::string ref ) {
    return( this->home_axis( addr, -1, ref ) );       //!< all axes at this addr
  }


  /**************** Physik_Instrumente::ServoInterface::home_axis *************/
  /**
   * @fn     home_axis
   * @brief  home an axis by moving to reference switch
   * @param[in]  int addr, address of controller in daisy-chain
   * @param[in]  int axis, axis to move
   * @param[in]  string ref, what to use for the homing
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   *
   * Homing can be performed using a reference switch (if equipped), or the 
   * positive or negative limit switches, as indicated by the "ref" argument.
   *
   */
  long ServoInterface::home_axis( int addr, int axis, std::string ref ) {
    std::string function = "Physik_Instrumente::ServoInterface::home_axis";
    std::stringstream message;
    std::stringstream cmd;

    // start with the address, if specified
    //
    if ( addr > 0 ) cmd << addr << " ";

    // add the appropriate homing command based on the ref argument
    //
    if ( ref == "ref" ) cmd << "FRF";  // reference switch
    else
    if ( ref == "neg" ) cmd << "FNL";  // negative limit switch
    else
    if ( ref == "pos" ) cmd << "FPL";  // positive limit switch
    else {
      message.str(""); message << "ERROR: unknown homing reference " << ref << ". expected { ref | pos | neg }";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // then add the axis, if specified
    //
    if ( axis > 0 ) cmd << " " << axis;

    return( this->send_command( cmd.str() ) );
  }
  /**************** Physik_Instrumente::ServoInterface::home_axis *************/


  /**************** Physik_Instrumente::ServoInterface::is_home ***************/
  /**
   * @fn          is_home
   * @brief       queries whether referencing has been done
   * @param[in]   int addr, address of controller in daisy-chain
   * @param[in]   int axis, axis to query
   * @param[out]  bool state of home (true|false)
   * @return      ERROR or NO_ERROR
   *
   */
  long ServoInterface::is_home( int addr, int axis, bool &state ) {
    std::string function = "Physik_Instrumente::ServoInterface::is_home";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "FRF? " << axis;

    long error = this->send_command( cmd.str(), reply );                       // send the command

    if ( error == NO_ERROR ) error = this->parse_reply( axis, reply, state );  // parse the response

    return error;
  }
  /**************** Physik_Instrumente::ServoInterface::is_home ***************/


  /**************** Physik_Instrumente::ServoInterface::on_target *************/
  /**
   * @fn     on_target
   * @brief  query the on target state for given addr and axis
   * @param  int addr, address of controller in daisy-chain
   * @param  bool state (true|false) if on target or not
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  long ServoInterface::on_target( int addr, bool &state ) {
    return( this->on_target( addr, -1, state ) );       //!< all axes at this addr
  }
  /**************** Physik_Instrumente::ServoInterface::on_target *************/


  /**************** Physik_Instrumente::ServoInterface::on_target *************/
  /**
   * @fn     on_target
   * @brief  query the on target state for given addr and axis
   * @param  int addr, address of controller in daisy-chain
   * @param  int axis, axis to move
   * @param  bool state (true|false) if on target or not
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  long ServoInterface::on_target( int addr, int axis, bool &state ) {
    std::string function = "Physik_Instrumente::ServoInterface::on_target";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "ONT?";
    if ( axis > 0 ) cmd << " " << axis;

    long error = this->send_command( cmd.str(), reply );                       // send the command

    if ( error == NO_ERROR ) error = this->parse_reply( axis, reply, state );  // parse the response

    return error;
  }
  /**************** Physik_Instrumente::ServoInterface::on_target *************/


  /**************** Physik_Instrumente::ServoInterface::get_pos ***************/
  /**
   * @fn         get_pos
   * @brief      get the current position of a motor
   * @param[in]  int addr, address of controller in daisy-chain
   * @param[out] float position read
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  long ServoInterface::get_pos( int addr, float &pos ) {
    return( this->get_pos( addr, -1, pos ) );       //!< all axes at this addr
  }
  /**************** Physik_Instrumente::ServoInterface::get_pos ***************/


  /**************** Physik_Instrumente::ServoInterface::get_pos ***************/
  /**
   * @fn         get_pos
   * @brief      get the current position of a motor
   * @param[in]  int addr, address of controller in daisy-chain
   * @param[in]  int axis, axis to read
   * @param[out] float position read
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded with a version where the axis can be specified;
   * the default is all axes when not specified.
   * 
   */
  long ServoInterface::get_pos( int addr, int axis, float &pos ) {
    std::string function = "Physik_Instrumente::ServoInterface::get_pos";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    if ( addr > 0 ) cmd << addr << " ";
    cmd << "POS?";
    if ( axis > 0 ) cmd << " " << axis;

    long error = this->send_command( cmd.str(), reply );                       // send the command

    if ( error == NO_ERROR ) error = this->parse_reply( axis, reply, pos );    // parse the response

    return error;
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
   * @fn         send_command
   * @brief      send a command string to the controller
   * @param[in]  string command
   * @return     ERROR or NO_ERROR
   *
   * The needed linefeed \n is added here
   *
   * This function is overloaded with a version that accepts a return string.
   * This version sends a command only and does not read back any reply.
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
  /**************** Physik_Instrumente::ServoInterface::send_command **********/


  /**************** Physik_Instrumente::ServoInterface::send_command **********/
  /**
   * @fn         send_command
   * @brief      send a command string to the controller
   * @param[in]  string command
   * @param[in]  string &reply (if needed)
   * @return     ERROR or NO_ERROR
   *
   * The needed linefeed \n is added here
   *
   * This function is overloaded.
   *
   * This version is called with a reference to return string, in which case 
   * after writing the command the reply is read and placed into the return
   * string.
   *
   */
  long ServoInterface::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Physik_Instrumente::ServoInterface::send_command";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    // send the command
    //
    error = this->send_command( cmd );

    if ( error == ERROR ) {
      message.str(""); message << "ERROR sending command: " << cmd;
      logwrite( function, message.str() );
    }

    // read the reply
    //
    while ( error == NO_ERROR && retval >= 0 ) {

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

    }

    retstring = reply;

    return( error );
  }
  /**************** Physik_Instrumente::ServoInterface::send_command **********/


  /**************** Physik_Instrumente::ServoInterface::parse_reply ***********/
  /**
   * @fn         parse_reply
   * @brief      parse the response from sending a ? command to the controller
   * @param[in]  axis number
   * @param[in]  reference to complete reply string
   * @return     code from controller, or -1 for internal error
   *
   * When sending a query command to the controller (one that ends in "?") then
   * a reply is read back; this function parses that reply.
   *
   * The reply from the controller is of the form:
   * 0 1 A=R
   * where A is the axis number and R is the response.
   *
   */
  template <typename T>
  long ServoInterface::parse_reply( int axis, std::string &reply, T &retval ) {
    std::string function = "Physik_Instrumente::ServoInterface::parse_reply";
    std::stringstream message;
    std::vector<std::string> tokens;
    std::stringstream sep;

    if ( reply.empty() ) {
      logwrite( function, "ERROR: empty message" );
      return( ERROR );
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] reply was \"" << reply << "\""; logwrite( function, message.str() );
#endif

    Tokenize( reply, tokens, "=" );

    // There must be two tokens. If not then the "=" is missing
    // and this is an error.
    //
    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR bad reply \"" << reply << "\" contains " << tokens.size() << " tokens but expected 2";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // The second token, tokens[1], contains the return value, R
    // which comes in as a string. Convert to the appropriate type
    // depending on the template type.
    //
    try {
      if constexpr( std::is_same_v<T, int> ) {    // convert to <int>
        retval = std::stoi( tokens.at(1) );
      }
      else
      if constexpr( std::is_same_v<T, float> ) {  // convert to <float>
        retval = std::stof( tokens.at(1) );
      }
      else
      if constexpr( std::is_same_v<T, bool> ) {   // convert to <bool>
        int tf = std::stoi( tokens.at(1) );
        if ( tf == 1 ) retval = true;
        else
        if ( tf == 0 ) retval = false;
        else {
          retval = false;
          message.str(""); message << "ERROR bad boolean " << tokens.at(1) << ": expected 1 or 0";
          logwrite( function, message.str() );
          return( ERROR );
        }
      }
      else {
        logwrite( function, "ERROR unrecognized type: expected <int> or <float>" );
        return( ERROR );
      }
    }
    catch ( std::invalid_argument & ) {
      logwrite( function, "ERROR bad reply: unable to convert to integer" );
      return( ERROR );
    }
    catch ( std::out_of_range & ) {
      logwrite( function, "ERROR reply or token out of range" );
      return( ERROR );
    }

    return NO_ERROR;

  }
  /**************** Physik_Instrumente::ServoInterface::parse_reply ***********/

}
