/**
 * @file    galil.cpp
 * @brief   this file contains the code for interfacing with Galil hardware
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "galil.h"
#include "logentry.h"

namespace Galil {

  /***** Galil::Interface::Interface ******************************************/
  /**
   * @brief      default Interface class constructor
   *
   */
  Interface::Interface()
    : port(-1),
      is_initialized(false)
  {
  }
  /***** Galil::Interface::Interface ******************************************/


  /***** Galil::Interface::Interface ******************************************/
  /**
   * @brief      Interface class constructor
   * @param[in]  name   name of the device for info purposes only
   * @param[in]  host   hostname of the device
   * @param[in]  port   port number of the device
   *
   */
  Interface::Interface(const std::string &name, const std::string &host, const int port)
    : name(name),
      host(host),
      port(port),
      is_initialized(true)
  {
  }
  /***** Galil::Interface::Interface ******************************************/


  /***** Galil::Interface::~Interface *****************************************/
  /**
   * @brief      Interface class deconstructor
   *
   */
  Interface::~Interface() {
  };
  /***** Galil::Interface::~Interface *****************************************/


  /***** Galil::Interface::open ***********************************************/
  /**
   * @brief      open a connection to the Galil hardware
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    std::string function = "Galil::Interface::open";
    std::ostringstream message;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( this->sock.isconnected() ) {
      message.str(""); message << "socket connection already open to "
                               << this->sock.gethost() << ":" << this->sock.getport()
                               << " on fd " << this->sock.getfd() << " for Galil (" << this->name << ")";
      logwrite( function, message.str() );
      return NO_ERROR;
    }

    Network::TcpSocket s( this->host, this->port );
    this->sock = s;

    // Initialize connection to the Galil
    //
    message.str(""); message << "opening socket connection for Galil (" << this->name << ")";
    logwrite( function, message.str() );

    if ( this->sock.Connect() != 0 ) {
      message.str(""); message << "ERROR connecting socket for Galil (" << this->name << ")";
      logwrite( function, message.str() );
      return ERROR;
    }
    this->sock.Flush();

    message.str(""); message << "socket connection to " 
                             << this->sock.gethost() << ":" << this->sock.getport()
                             << " established on fd " << this->sock.getfd() << " for Galil (" << this->name << ")";
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Galil::Interface::open ***********************************************/


  /***** Galil::Interface::close **********************************************/
  /**
   * @brief      close the connection to the Galil socket
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    std::string function = "Galil::Interface::close";
    std::ostringstream message;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( ! this->sock.isconnected() ) {
      logwrite(function, "socket connection not open to "+this->sock.gethost());
      return NO_ERROR;
    }

    long error = this->sock.Close();

    if (error == NO_ERROR) {
      logwrite(function, "socket connection to "+this->sock.gethost()+" closed");
    }
    else {
      logwrite(function, "ERROR closing socket connection to "+this->sock.gethost());
    }

    return error;
  }
  /***** Galil::Interface::close **********************************************/


  /***** Galil::Interface::send_command ***************************************/
  /**
   * @brief      send specified command to Galil socket interface
   * @details    This function is overloaded. This version discards the reply.
   * @param[in]  cmd        command to send
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd ) {
    std::string dontcare;
    return this->send_command( cmd, dontcare );
  }
  /***** Galil::Interface::send_command ***************************************/


  /***** Galil::Interface::send_command ***************************************/
  /**
   * @brief      send specified command to Galil socket interface and return reply
   * @details    This function is overloaded. This version returns the reply.
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply read back from device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Galil::Interface::send_command";
    std::ostringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( ! this->sock.isconnected() ) {
      message.str(""); message << "ERROR not connected to Galil (" << this->name << ")";
      logwrite( function, message.str() );
      return ERROR;
    }

    // send the command
    //
    cmd.append( "\n" );                            // add the NEWLINE character
    int written = this->sock.Write( cmd );         // write the command
    if ( written <= 0 ) {                          // return error if error writing to socket
      cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n' ), cmd.end());  // remove the newline for better logging
      message.str(""); message << "ERROR sending \"" << cmd << "\" to Galil (" << this->name << ")";
      logwrite( function, message.str() );
      return ERROR;
    }

    // if the cmd has a question mark then read the reply
    //
    bool needs_reply = ( cmd.find("?")!=std::string::npos );

    while ( error == NO_ERROR && retval >= 0 && needs_reply ) {

      if ( ( retval=this->sock.Poll() ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "TIMEOUT on fd " << this->sock.getfd() << ": " << strerror(errno);
                           error = TIMEOUT; }
        if ( retval <0 ) { message.str(""); message << "ERROR on fd " << this->sock.getfd() << ": " << strerror(errno);
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = this->sock.Read( reply, '\n' ) ) < 0 ) {
        message.str(""); message << "ERROR: " << strerror( errno ) 
                                 << ": reading from " << this->sock.gethost() << "/" << this->sock.getport();
        logwrite( function, message.str() );
        break;
      }

      // remove any newline characters and get out
      //
      reply = strip_newline(reply);
      break;
    }

    retstring = reply;

#ifdef LOGLEVEL_DEBUG
//  message << " reply=" << reply; logwrite( function, message.str() );
#endif

    return error;
  }
  /***** Galil::Interface::send_command ***************************************/

}
