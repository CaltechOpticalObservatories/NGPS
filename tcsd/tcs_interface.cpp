#include "tcs_interface.h"

namespace TCS {

  /**************** TCS::Interface::Interface *********************************/
  /**
   * @fn     Interface
   * @brief  class constructor
   * @param  none
   * @return none
   *
   */
  Interface::Interface() {
  }
  /**************** TCS::Interface::Interface *********************************/


  /**************** TCS::Interface::~Interface ********************************/
  /**
   * @fn     ~Interface
   * @brief  class deconstructor
   * @param  none
   * @return none
   *
   */
  Interface::~Interface() {
  }
  /**************** TCS::Interface::~Interface ********************************/


  /**************** TCS::Interface::initialize_class **************************/
  /**
   * @fn     initialize
   * @brief  
   * @param  
   * @return 
   *
   */
  long Interface::initialize_class() {
    std::string function = "TCS::Interface::initialize_class";
    std::stringstream message;

    if ( this->port < 0 || host.empty() ) {
      message.str(""); message << "ERROR: host \"" << this->host << "\" and/or port \"" << this->port << "\" invalid";
      logwrite( function, message.str() );
      return( ERROR );
    }

    Network::TcpSocket s( this->host, this->port );
    this->tcs = s;

    logwrite( function, "interface initialized ok" );
    return( NO_ERROR );
  }
  /**************** TCS::Interface::initialize_class **************************/


  /**************** TCS::Interface::open **************************************/
  /**
   * @fn     open
   * @brief  opens the PI socket connection
   * @param  none
   * @return ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    std::string function = "TCS::Interface::open";
    std::stringstream message;

    if ( this->tcs.isconnected() ) {
      logwrite( function, "connection already open" );
      return( NO_ERROR );
    }

    // initialize connection to the TCS
    //
    logwrite( function, "opening connection to TCS" );

    if ( this->tcs.Connect() != 0 ) {
      message.str(""); message << "ERROR connecting to TCS on " << this->host << ":" << this->port << " ...";
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str(""); message << "socket connection to "
                             << this->tcs.gethost() << ":" << this->tcs.getport()
                             << " established on fd " << this->tcs.getfd();
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /**************** TCS::Interface::open **************************************/


  /**************** TCS::Interface::close *************************************/
  /**
   * @fn     close
   * @brief  closes the PI socket connection
   * @param  none
   * @return ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    std::string function = "TCS::Interface::close";
    std::stringstream message;

    if ( !this->tcs.isconnected() ) {
      logwrite( function, "connection already closed" );
      return( NO_ERROR );
    }

    long error = this->tcs.Close();

    if ( error == NO_ERROR ) {
      logwrite( function, "connection to TCS closed" );
    }
    else {
      logwrite( function, "ERROR closing connection to TCS" );
    }
    return( error );
  }
  /**************** TCS::Interface::close *************************************/


  /**************** TCS::Interface::send_command ******************************/
  /**
   * @fn     send_command
   * @brief  writes the raw command, as received, to the TCS
   * @param  string cmd
   * @return ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &reply ) {
    std::string function = "TCS::Interface::send_command";
    std::stringstream message;
    long error=NO_ERROR;
    int ret;
    std::string sbuf;
    char delim       = '\r';   /// replies from the TCS are terminated with this
    std::string term = "\r";   /// all commands to TCS are terminated with this

    if ( !this->tcs.isconnected() ) {
      logwrite( function, "ERROR: no connection open to the TCS" );
      return( ERROR );
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] sending to TCS on fd " << this->tcs.getfd() << ": " << cmd;
    logwrite( function, message.str() );
#endif

    cmd.append( term );        // add terminator before sending command to the TCS

    if ( this->tcs.Write( cmd ) == -1 ) {
      message.str(""); message << "ERROR writing to TCS on fd " << this->tcs.getfd();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // receive the reply
    //
    do {
      // wait for incoming data
      //
      if ( ( ret = this->tcs.Poll() ) <= 0 ) {
        if ( ret==0 ) { message.str(""); message << "Poll timeout on fd " << tcs.getfd() << " waiting for response from TCS"; error = TIMEOUT; }
        if ( ret <0 ) { message.str(""); message << "Poll error on fd "   << tcs.getfd() << " waiting for response from TCS"; error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      // data available, read from socket
      //
      if ( ( ret = this->tcs.Read( sbuf, delim ) ) <= 0 ) {
        if ( ret < 0 ) {
          message.str(""); message << "ERROR reading from TCS on fd " << tcs.getfd() << ": " << strerror(errno);
          logwrite( function, message.str() );
        }
        if ( ret == 0 ) {
          message.str(""); message << "TIMEOUT reading from TCS on fd " << tcs.getfd() << ": " << strerror(errno);
          logwrite( function, message.str() );
        }
        break;
      }
    } while ( ret > 0 && sbuf.find( delim ) == std::string::npos );

    // remove any trailing linefeed and carriage return
    //
    sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\r' ), sbuf.end());
    sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\n' ), sbuf.end());

    reply = sbuf;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] received from TCS on fd " << this->tcs.getfd() << ": " << reply;
    logwrite( function, message.str() );
#endif

    return( error );
  }
  /**************** TCS::Interface::send_command ******************************/

}
