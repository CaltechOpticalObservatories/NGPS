/**
 * @file    lks.cpp
 * @brief   this file contains the code for interfacing with LKS hardware
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "lks.h"
#include "logentry.h"

namespace LKS {

  /***** LKS::Interface::Interface ********************************************/
  /**
   * @brief      default Interface class constructor
   *
   */
  Interface::Interface( ) {
    this->model = "";
    this->name = "";
    this->port = -1;
    this->host = "";
    this->initialized = false;
  }
  /***** LKS::Interface::Interface ********************************************/


  /***** LKS::Interface::Interface ********************************************/
  /**
   * @brief      Interface class constructor
   * @param[in]  model  model name/number of device
   * @param[in]  name   name of the device for info purposes only
   * @param[in]  host   hostname of the device
   * @param[in]  port   port number of the device
   *
   */
  Interface::Interface( std::string model, std::string name, std::string host, int port ) {
    this->model = model;
    this->name = name;
    this->port = port;
    this->host = host;
    this->initialized = true;
  }
  /***** LKS::Interface::Interface ********************************************/


  /***** LKS::Interface::~Interface *******************************************/
  /**
   * @brief      Interface class deconstructor
   *
   */
  Interface::~Interface() {
  };
  /***** LKS::Interface::~Interface *******************************************/


  /***** LKS::Interface::open *************************************************/
  /**
   * @brief      open a connection to the LKS hardware
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    std::string function = "LKS::Interface::open";
    std::stringstream message;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( this->sock.isconnected() ) {
      message.str(""); message << "socket connection already open to "
                               << this->sock.gethost() << ":" << this->sock.getport()
                               << " on fd " << this->sock.getfd() << " for LKS " << this->model
                               << " (" << this->name << ")";
      logwrite( function, message.str() );
      return( NO_ERROR );
    }

    Network::TcpSocket s( this->host, this->port );
    this->sock = s;

    // Initialize connection to the LKS
    //
    message.str(""); message << "opening socket connection for LKS " << this->model << " (" << this->name << ")";
    logwrite( function, message.str() );

    if ( this->sock.Connect() != 0 ) {
      message.str(""); message << "ERROR connecting socket for LKS " << this->model << " (" << this->name << ")";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Need to flush the buffer because the LKS will write stuff upon opening the socket
    //
    this->sock.Flush();

    message.str(""); message << "socket connection to " 
                             << this->sock.gethost() << ":" << this->sock.getport()
                             << " established on fd " << this->sock.getfd() << " for LKS " << this->model
                             << " (" << this->name << ")";
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** LKS::Interface::open *************************************************/


  /***** LKS::Interface::close ************************************************/
  /**
   * @brief      close the connection to the LKS socket
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    std::string function = "LKS::Interface::close";
    std::stringstream message;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( ! this->sock.isconnected() ) {
      message.str(""); message << "socket connection already closed for LKS " << this->model
                               << " (" << this->name << ")";
      logwrite( function, message.str() );
      return( NO_ERROR );
    }

    long error = this->sock.Close();

    if ( error == NO_ERROR ) {
      message.str(""); message << "socket connection for LKS " << this->model << " (" << this->name << ") closed";
      logwrite( function, message.str() );
    }
    else {
      message.str(""); message << "ERROR closing socket connection for LKS " << this->model
                               << " ( " << this->name << ")";
      logwrite( function, message.str() );
    }

    return( error );
  }
  /***** LKS::Interface::close ************************************************/


  /***** LKS::Interface::send_command *****************************************/
  /**
   * @brief      send specified command to LKS socket interface
   * @details    This function is overloaded. This version discards the reply.
   * @param[in]  cmd        command to send
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd ) {
    std::string dontcare;
    return this->send_command( cmd, dontcare );
  }
  /***** LKS::Interface::send_command *****************************************/


  /***** LKS::Interface::send_command *****************************************/
  /**
   * @brief      send specified command to LKS socket interface and return reply
   * @details    This function is overloaded. This version returns the reply.
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply read back from device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "LKS::Interface::send_command";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

#ifdef LOGLEVEL_DEBUG
//  message << "[DEBUG] send to LKS " << this->model << " (" << this->name << ") on socket " 
//          << this->sock.gethost() << "/" << this->sock.getport() << ": " << cmd;
//  logwrite( function, message.str() );
#endif

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( ! this->sock.isconnected() ) {
      message.str(""); message << "ERROR not connected to LKS " << this->model << " (" << this->name << ")";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // send the command
    //
    cmd.append( "\n" );                            // add the NEWLINE character
    int written = this->sock.Write( cmd );         // write the command
    if ( written <= 0 ) {                          // return error if error writing to socket
      cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n' ), cmd.end());  // remove the newline for better logging
      message.str(""); message << "ERROR sending \"" << cmd << "\" to LKS " << this->model << " (" << this->name << ")";
      logwrite( function, message.str() );
      return( ERROR );
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
      reply.erase(std::remove(reply.begin(), reply.end(), '\r' ), reply.end());
      reply.erase(std::remove(reply.begin(), reply.end(), '\n' ), reply.end());
      break;

    }

    retstring = reply;

#ifdef LOGLEVEL_DEBUG
//  message << " reply=" << reply; logwrite( function, message.str() );
#endif

    return( error );
  }
  /***** LKS::Interface::send_command *****************************************/

}
