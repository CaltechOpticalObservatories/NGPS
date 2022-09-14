/**
 * @file    wti.cpp
 * @brief   this file contains the code for interfacing with WTI hardware
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "wti.h"
#include "logentry.h"

namespace WTI {

  /**************** WTI::Interface::Interface *********************************/
  /**
   * @fn         Interface
   * @brief      default Interface class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface( ) {
    this->name = "";
    this->port = -1;
    this->host = "";
    this->initialized = false;
  }
  /**************** WTI::Interface::Interface *********************************/


  /**************** WTI::Interface::Interface *********************************/
  /**
   * @fn         Interface
   * @brief      Interface class constructor
   * @param[in]  name
   * @param[in]  host
   * @param[in]  port
   * @return     none
   *
   */
  Interface::Interface( std::string name, std::string host, int port ) {
    this->name = name;
    this->port = port;
    this->host = host;
    this->initialized = true;
  }
  /**************** WTI::Interface::Interface *********************************/


  /**************** WTI::Interface::~Interface ********************************/
  /**
   * @fn         ~Interface
   * @brief      Interface class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  };
  /**************** WTI::Interface::~Interface ********************************/


  /**************** WTI::Interface::open **************************************/
  /**
   * @fn         open
   * @brief      open a connection to the WTI hardware
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    std::string function = "WTI::Interface::open";
    std::stringstream message;

    if ( this->sock.isconnected() ) {
      message.str(""); message << "ERROR socket connection already open to "
                               << this->sock.gethost() << ":" << this->sock.getport()
                               << " on fd " << this->sock.getfd() << " for " << this->name;
      logwrite( function, message.str() );
      return( ERROR );
    }

    Network::TcpSocket s( this->host, this->port );
    this->sock = s;

    // Initialize connection to the WTI
    //
    message.str(""); message << "opening socket connection for " << this->name;
    logwrite( function, message.str() );

    if ( this->sock.Connect() != 0 ) {
      message.str(""); message << "ERROR connecting socket for " << this->name;
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str(""); message << "socket connection to " 
                             << this->sock.gethost() << ":" << this->sock.getport()
                             << " established on fd " << this->sock.getfd() << " for " << this->name;
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /**************** WTI::Interface::open **************************************/


  /**************** WTI::Interface::close *************************************/
  /**
   * @fn         close
   * @brief      close the connection to the WTI socket
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    std::string function = "WTI::Interface::close";
    std::stringstream message;

    if ( not this->sock.isconnected() ) {
      message.str(""); message << "ERROR socket connection already closed for " << this->name;
      logwrite( function, message.str() );
      return( ERROR );
    }

    long error = this->sock.Close();

    if ( error == NO_ERROR ) {
      message.str(""); message << "socket connection for " << this->name << " closed";
      logwrite( function, message.str() );
    }
    else {
      message.str(""); message << "ERROR closing socket connection for " << this->name;
      logwrite( function, message.str() );
    }

    return( error );
  }
  /**************** WTI::Interface::close *************************************/


  /**************** WTI::Interface::send_command ******************************/
  /**
   * @fn         send_command
   * @brief      
   * @param[in]  cmd, string
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd ) {
    std::string function = "WTI::Interface::send_command";
    std::stringstream message;

#ifdef LOGLEVEL_DEBUG
    message << "[DEBUG] to " << this->name << ": " << cmd;
    logwrite( function, message.str() );
#endif

    cmd.append( "\n" );                            // add the newline character

    int written = this->sock.Write( cmd );         // write the command

    if ( written <= 0 ) return( ERROR );           // return error if error writing to socket

    return( NO_ERROR );
  }
  /**************** WTI::Interface::send_command ******************************/


  /**************** WTI::Interface::send_command ******************************/
  /**
   * @fn         send_command
   * @brief      
   * @param[in]  cmd, string
   * @param[out] retstring, reference to string
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "WTI::Interface::send_command";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    // send the command
    //
    error = this->send_command( cmd );

    if ( error == ERROR ) {
      message.str(""); message << "ERROR sending " << cmd << " to " << this->name;
      logwrite( function, message.str() );
    }

    // read the reply
    //
    while ( error == NO_ERROR && retval >= 0 ) {

      if ( ( retval=this->sock.Poll() ) <= 0 ) {
        message.str("");
        if ( retval==0 ) { message << "TIMEOUT on " << this->name << " fd " << this->sock.getfd() << ": " << strerror(errno);
                           error = TIMEOUT; }
        if ( retval <0 ) { message << "ERROR on " << this->name << " fd " << this->sock.getfd() << ": " << strerror(errno);
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = this->sock.Read( reply, '\n' ) ) < 0 ) {
        message.str(""); message << "error reading from socket for " << this->name << ": " << strerror( errno );
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
  /**************** WTI::Interface::send_command ******************************/


  /**************** WTI::NPS::NPS *********************************************/
  /**
   * @fn         NPS
   * @brief      default NPS class constructor
   * @param[in]  none
   * @return     none
   *
   */
  NPS::NPS() {
  }
  /**************** WTI::NPS::NPS *********************************************/


  /**************** WTI::NPS::~NPS ********************************************/
  /**
   * @fn         ~NPS
   * @brief      default NPS class de-constructor
   * @param[in]  none
   * @return     none
   *
   */
  NPS::~NPS() {
  }
  /**************** WTI::NPS::~NPS ********************************************/


  /**************** WTI::NPS::set_switch **************************************/
  /**
   * @fn         set_switch
   * @brief      turn the selected plug on|off
   * @param[in]  plugnum
   * @param[in]  action
   * @return     ERROR or NO_ERROR
   *
   */
  long NPS::set_switch( int plugnum, int action ) {
    std::string function = "WTI::NPS::set_switch";
    std::stringstream message;
    std::stringstream cmdstr;
    long error = NO_ERROR;

    switch( action ) {
      case  0: cmdstr << "/OFF " << plugnum << ",Y";
               break;
      case  1: cmdstr << "/ON " << plugnum << ",Y";
               break;
      default: message.str(""); message << "ERROR unrecognized action " << action;  // should be impossible
               logwrite( function, message.str() );
               return( ERROR );
               break;
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] sending " << cmdstr.str() << " to " << this->interface.get_name();
    logwrite( function, message.str() );
#endif

    error = this->interface.send_command( cmdstr.str() );

    return( error );
  }
  /**************** WTI::NPS::set_switch **************************************/


  /**************** WTI::NPS::get_switch **************************************/
  /**
   * @fn         get_switch
   * @brief      get the selected plug's on/off state
   * @param[in]  plugnum
   * @param[in]  state
   * @return     ERROR or NO_ERROR
   *
   */
  long NPS::get_switch( int plugnum, std::string &state ) {
    std::string function = "WTI::NPS::get_switch";
    std::stringstream message;
    std::stringstream cmdstr;
    long error = NO_ERROR;

    logwrite( function, "not yet implemented" );

    state="N/A";

    return( error );
  }
  /**************** WTI::NPS::get_switch **************************************/

}
