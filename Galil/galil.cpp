/**
 * @file    galil.cpp
 * @brief   this file contains the code for interfacing with Galil hardware
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "galil.h"
#include "logentry.h"

namespace Galil {

  /***** Galil::Interface::send_command ***************************************/
  /**
   * @brief      send specified command to Galil socket interface and return reply
   * @details    This function will expect and return a reply if the retstring
   *             parameter is provided (defaults to nullptr).
   * @param[in]  motorname  name of motor
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply read back from device if provided
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename Type>
  long Interface<Type>::send_command(const std::string &motorname, std::string cmd, std::string* retstring) {
    std::string function = "Galil::Interface::send_command";
    std::ostringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    Network::TcpSocket socket;

    std::unique_lock<std::mutex> lock( *this->controller_mutex );

    try {
      socket = this->get_socket( motorname );   // includes check of motorname
    }
    catch ( const std::runtime_error &e ) {
      logwrite(function, "ERROR: "+std::string(e.what()));
      return ERROR;
    }

    if ( !socket.isconnected() ) {
      logwrite(function, "ERROR no socket connection to motor "+motorname);
      return ERROR;
    }

    cmd.append( "\n" );                       // add the newline character

    int written = socket.Write( cmd );        // write the command

    if ( written <= 0 ) return ERROR;         // return error if error writing to socket

    if (retstring==nullptr) return NO_ERROR;  // no reply needed

    // read the reply
    //
    while ( error == NO_ERROR && retval >= 0 ) {

      if ( ( retval=socket.Poll() ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "TIMEOUT on fd " << socket.getfd() << ": " << strerror(errno);
                           error = TIMEOUT; }
        if ( retval <0 ) { message.str(""); message << "ERROR on fd " << socket.getfd() << ": " << strerror(errno);
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = socket.Read( reply, '\n' ) ) < 0 ) {
        message.str(""); message << "ERROR reading from motor controller: " << strerror( errno );
        logwrite( function, message.str() );
        break;
      }

      // remove any newline characters and get out
      //
      strip_newline(reply);
      break;
    }

    *retstring = reply;

    return error;
  }
  /***** Galil::Interface::send_command ***************************************/

}
