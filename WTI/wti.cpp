/**
 * @file    wti.cpp
 * @brief   this file contains the code for interfacing with WTI hardware
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "wti.h"
#include "logentry.h"

namespace WTI {

  /***** WTI::Interface::open *************************************************/
  /**
   * @brief      open a connection to the WTI hardware
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    std::string function = "WTI::Interface::open";
    std::stringstream message;

    if ( this->sock.isconnected() ) {
      message.str(""); message << "socket connection already open to "
                               << this->sock.gethost() << ":" << this->sock.getport()
                               << " on fd " << this->sock.getfd() << " for " << this->name;
      logwrite( function, message.str() );
      return( NO_ERROR );
    }

    this->sock = Network::TcpSocket( this->host, this->port );

    // Initialize connection to the WTI
    //
    message.str(""); message << "opening socket connection for " << this->name;
    logwrite( function, message.str() );

    if ( this->sock.Connect() != 0 ) {
      message.str(""); message << "ERROR connecting socket for " << this->name;
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Need to flush the buffer because the WTI will write stuff upon opening the socket
    //
    this->sock.Flush();

    message.str(""); message << "socket connection to " 
                             << this->sock.gethost() << ":" << this->sock.getport()
                             << " established on fd " << this->sock.getfd() << " for " << this->name;
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** WTI::Interface::open *************************************************/


  /***** WTI::Interface::close ************************************************/
  /**
   * @brief      close the connection to the WTI socket
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    std::string function = "WTI::Interface::close";
    std::stringstream message;

    if ( ! this->sock.isconnected() ) {
      message.str(""); message << "socket connection already closed for " << this->name;
      logwrite( function, message.str() );
      return( NO_ERROR );
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
  /***** WTI::Interface::close ************************************************/


  /***** WTI::Interface::send_command *****************************************/
  /**
   * @brief      send specified command to WTI socket interface, don't return reply
   * @details    This function doesn't care about the response.
   * @param[in]  cmd  command to send
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd ) {
    std::string retstring;
    return do_send_command( cmd, retstring );
  }
  /***** WTI::Interface::send_command *****************************************/


  /***** WTI::Interface::send_command *****************************************/
  /**
   * @brief      send specified command to WTI socket interface and return response
   * @details    This function returns only the response portion of the device's raw-reply,
   *             which otherwise contains also the original command.
   * @param[in]  cmd    command to send
   * @param[out] reply  reply read back from device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &reply ) {
    std::string function = "WTI::Interface::send_command";
    std::stringstream message;
    std::string retstring;  // raw return string from device

    long error = do_send_command( cmd, retstring );

    if ( error != NO_ERROR ) return error;

    // Tokenize retstring on CR and LF
    // Reply should contain 3 tokens: <cmd> <response> "NPS>"
    //
    std::vector<std::string> tokens;
    Tokenize( retstring, tokens, "\r\n" );
    if ( tokens.size() == 3 ) {
      reply = tokens[1];  // the reply we want is this token from retstring
    }
    else {
      message.str(""); message << "ERROR malformed response: " << retstring << ": expected <cmd> <response> \"NPS>\"";
      logwrite( function, message.str() );
      error = ERROR;
    }

    return error;
  }
  /***** WTI::Interface::send_command *****************************************/


  /***** WTI::Interface::do_send_command **************************************/
  /**
   * @brief      send specified command to WTI socket interface and return raw reply
   * @details    This function returns the complete raw response from the device,
   *             which contains also the command.
   * @param[in]  cmd        command to send
   * @param[out] retstring  raw string read back from device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::do_send_command( std::string cmd, std::string &retstring ) {
    std::string function = "WTI::Interface::do_send_command";
    std::stringstream message;
    long error=NO_ERROR;
    long retval=0;

    // send the command
    //
    cmd.append( "\r" );                            // add the CR character
    int written = this->sock.Write( cmd );         // write the command
    if ( written <= 0 ) {                          // return error if error writing to socket
      message.str(""); message << "ERROR sending " << strip_newline(cmd) << " to " << this->name;
      logwrite( function, message.str() );
      return( ERROR );
    }

    // read the reply
    //
    while ( error == NO_ERROR && retval >= 0 ) {

      if ( ( retval=this->sock.Poll() ) <= 0 ) {
        message.str("");
        if ( retval==0 ) { message << "TIMEOUT on " << this->name << " polling socket " << this->sock.gethost()
                                   << "/" << this->sock.getport() << " on fd " << this->sock.getfd() << ": " << strerror(errno);
                           error = TIMEOUT; }
        if ( retval <0 ) { message << "ERROR on " << this->name << " polling socket " << this->sock.gethost()
                                   << "/" << this->sock.getport() << " on fd " << this->sock.getfd() << ": " << strerror(errno);
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = this->sock.Read( retstring, "NPS>" ) ) < 0 ) {
        message.str(""); message << "error reading from socket " << this->sock.gethost() << "/" << this->sock.getport()
                                 << " for " << this->name << ": " << strerror( errno );
        logwrite( function, message.str() );
        break;
      }

      // Check that the reply contains the command, which is acknowledgement of receipt
      //
      if ( retstring.find( cmd ) == std::string::npos ) {
        message.str(""); message << "ERROR did not receive command ack: " << retstring;
        logwrite( function, message.str() );
        return( ERROR );
      }
      else {
#ifdef LOGLEVEL_DEBUG
//      std::string debug_reply = retstring;
//      debug_reply= std::regex_replace( debug_reply, std::regex("\\r"), "\\r" );
//      debug_reply= std::regex_replace( debug_reply, std::regex("\\n"), "\\n" );
//      message.str(""); message << "[DEBUG] " << this->name << " reply: " << debug_reply;
//      logwrite( function, message.str() );
#endif
        break;
      }
    }

    return( error );
  }
  /***** WTI::Interface::do_send_command **************************************/


  /***** WTI::NPS::set_switch *************************************************/
  /**
   * @brief      turn the selected plug on|off
   * @param[in]  plugnum  plug number
   * @param[in]  action   action to take, 0=OFF, 1=ON, 2=REBOOT
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
      case  2: cmdstr << "/BOOT " << plugnum << ",Y";
               break;
      default: message.str(""); message << "ERROR unrecognized action " << action;  // should be impossible
               logwrite( function, message.str() );
               return( ERROR );
               break;
    }

    error = this->interface.send_command( cmdstr.str() );

    return( error );
  }
  /***** WTI::NPS::set_switch *************************************************/


  /***** WTI::NPS::get_switch *************************************************/
  /**
   * @brief      get the selected plug's on/off state
   * @param[in]  plugnum  plug number
   * @param[in]  state    on/off state as read back from the NPS, 0=OFF, 1=ON
   * @return     ERROR or NO_ERROR
   *
   */
  long NPS::get_switch( int plugnum, std::string &state ) {
    std::stringstream cmdstr;
    cmdstr << "/S " << plugnum;
    return this->interface.send_command( cmdstr.str(), state );
  }
  /***** WTI::NPS::get_switch *************************************************/


  /***** WTI::NPS::get_all ****************************************************/
  /**
   * @brief      get the on/off state of all plugs
   * @details    returned state is CSV format: s,s,s...,s
   * @param[in]  maxplugs  max number of plugs in this unit
   * @param[in]  state     on/off state as read back from the NPS, 0=OFF, 1=ON
   * @return     ERROR or NO_ERROR
   *
   */
  long NPS::get_all( int maxplugs, std::string &state ) {
    std::stringstream cmdstr;
    cmdstr << "/S 1:" << maxplugs;
    return this->interface.send_command( cmdstr.str(), state );
  }
  /***** WTI::NPS::get_all ****************************************************/

}
