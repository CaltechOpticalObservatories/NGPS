/**
 * @file    brainbox.cpp
 * @brief   this file contains the code for interfacing with BrainBox hardware
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "brainbox.h"
#include "logentry.h"

namespace BrainBox {

  /***** BrainBox::Interface::Interface ***************************************/
  /**
   * @brief      default Interface class constructor
   *
   */
  Interface::Interface()
    : port(-1),
      address(-1),
      is_initialized(false)
  {
  }
  /***** BrainBox::Interface::Interface ***************************************/


  /***** BrainBox::Interface::Interface ***************************************/
  /**
   * @brief      Interface class constructor
   * @param[in]  host     hostname of the device
   * @param[in]  port     port number of the device
   * @param[in]  address  address of the device
   *
   */
  Interface::Interface(const std::string &host, const int port, const int address)
    : host(host),
      port(port),
      address(address),
      is_initialized(true)
  {
  }
  /***** BrainBox::Interface::Interface ***************************************/


  /***** BrainBox::Interface::~Interface **************************************/
  /**
   * @brief      Interface class deconstructor
   *
   */
  Interface::~Interface() {
  };
  /***** BrainBox::Interface::~Interface **************************************/


  /***** BrainBox::Interface::open *************************************************/
  /**
   * @brief      open a connection to the BrainBox hardware
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    const std::string function("BrainBox::Interface::open");
    std::ostringstream message;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( this->sock.isconnected() ) {
      message << "socket connection already open to "
              << this->sock.gethost() << ":" << this->sock.getport()
              << " on fd " << this->sock.getfd() << " for BrainBox " << this->model
              << " (" << this->name << ")";
      logwrite(function, message.str());
      return NO_ERROR;
    }

    Network::TcpSocket s( this->host, this->port );
    this->sock = s;

    // Initialize connection to the BrainBox
    //
    if ( this->sock.Connect() != 0 ) {
      logwrite(function, "ERROR opening connection to "+this->sock.gethost());
      return ERROR;
    }

    message << "socket connection to " 
            << this->sock.gethost() << ":" << this->sock.getport()
            << " established on fd " << this->sock.getfd();
    logwrite(function, message.str());

    return NO_ERROR;
  }
  /***** BrainBox::Interface::open ********************************************/


  /***** BrainBox::Interface::close *******************************************/
  /**
   * @brief      close the connection to the BrainBox socket
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    const std::string function("BrainBox::Interface::close");
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
  /***** BrainBox::Interface::close *******************************************/


  /***** BrainBox::Interface::send_command ************************************/
  /**
   * @brief      send specified command to BrainBox socket interface
   * @details    This function is overloaded. This version discards the reply.
   * @param[in]  cmd        command to send
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd ) {
    std::string dontcare;
    return this->send_command( cmd, dontcare );
  }
  /***** BrainBox::Interface::send_command ************************************/


  /***** BrainBox::Interface::send_command ************************************/
  /**
   * @brief      send specified command to BrainBox socket interface and return reply
   * @details    This function is overloaded. This version returns the reply.
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply read back from device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &retstring ) {
    const std::string function("BrainBox::Interface::send_command");
    std::ostringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    std::lock_guard<std::mutex> lock( this->mtx );

    if ( ! this->sock.isconnected() ) {
      logwrite(function, "ERROR not connected to BrainBox");
      return ERROR;
    }

    // send the command
    //
    cmd.append( "\r" );                            // add the CR character
    int written = this->sock.Write( cmd );         // write the command
    if ( written <= 0 ) {                          // return error if error writing to socket
      logwrite(function, "ERROR sending \"" + strip_newline(cmd) + "\" to BrainBox");
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

      if ( ( retval = this->sock.Read( reply, '\r' ) ) < 0 ) {
        message.str(""); message << "ERROR: " << strerror( errno ) 
                                 << ": reading from " << this->sock.gethost() << "/" << this->sock.getport();
        logwrite(function, message.str());
        break;
      }

      // remove any newline characters and get out
      //
      reply = strip_newline(reply);
      break;

    }

    retstring = reply;

    return error;
  }
  /***** BrainBox::Interface::send_command ************************************/


  /***** BrainBox::Interface::read_digio **************************************/
  /**
   * @brief      read digital IO lines
   */
  long Interface::read_digio() {
  }
  /***** BrainBox::Interface::read_digio **************************************/


  /***** BrainBox::Interface::write_digio *************************************/
  /**
   * @brief      write digital IO lines
   */
  long Interface::write_digio() {
  }
  /***** BrainBox::Interface::write_digio *************************************/

}
