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
    int retval;
    char buffer[ BUFSZ ];

    if ( !this->tcs.isconnected() ) {
      logwrite( function, "ERROR: no connection open to the TCS" );
      return( ERROR );
    }

    cmd.append( "\n" );

    if ( this->tcs.Write( cmd ) == -1 ) {
      logwrite( function, "ERROR writing to TCS" );
      return( ERROR );
    }

    // receive the reply
    //
    reply.clear();
    do {
      if ( ( retval = this->tcs.Poll() ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "Poll timeout waiting for response from TCS"; error = TIMEOUT; }
        if ( retval <0 ) { message.str(""); message << "Poll error waiting for response from TCS"; error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }
      memset( buffer, '\0', BUFSZ );
      retval = this->tcs.Read( buffer, BUFSZ );
      if ( retval <= 0 ) {
        logwrite( function, "ERROR reading from TCS" );
        break;
      }
      reply.append( buffer );
    } while ( retval > 0 && reply.find( "\n" ) == std::string::npos );

    return( error );
  }
  /**************** TCS::Interface::send_command ******************************/

}
