#include "sequencer_interface.h"
#define BUFSIZE 1024

namespace Sequencer {

  /** Sequencer::TargetInfo::TargetInfo ***************************************/
  /**
   * @fn     TargetInfo
   * @brief  class constructor
   * @param  
   * @return 
   *
   */
  TargetInfo::TargetInfo() {
  }
  /** Sequencer::TargetInfo::TargetInfo ***************************************/


  /** Sequencer::TargetInfo::~TargetInfo **************************************/
  /**
   * @fn     ~TargetInfo
   * @brief  class deconstructor
   * @param  
   * @return 
   *
   */
  TargetInfo::~TargetInfo() {
  }
  /** Sequencer::TargetInfo::~TargetInfo **************************************/


  /** Sequencer::TargetInfo::get_next *****************************************/
  /**
   * @fn         get_next
   * @brief      
   * @param[in]  none
   * @return     TARGET_FOUND or TARGET_NOT_FOUND
   *
   */
  long TargetInfo::get_next() {
    std::string function = "Sequencer::TargetInfo::get_next";
    logwrite( function, "reading database" );

    // hard-code some numbers for now
    //
    this->name       = "alp_ori";
    this->ra         = 5.9195278;
    this->dec        = 7.4070556;
    this->epoch      = "J2000";
    this->slitwidth  = 4.0;
    this->slitoffset = 0.0;
    this->exptime    = 10;
    this->repeat     = 1;
    this->binspect   = 1;
    this->binspat    = 1;

    return TARGET_FOUND;
  }
  /** Sequencer::TargetInfo::get_next *****************************************/


  /** Sequencer::TcpDevice::TcpDevice *****************************************/
  /**
   * @fn     TcpDevice
   * @brief  class constructor
   * @param  
   * @return 
   *
   */
  TcpDevice::TcpDevice() {
    this->socket.sethost( "localhost" );
  }
  /** Sequencer::TcpDevice::TcpDevice *****************************************/


  /** Sequencer::TcpDevice::~TcpDevice ****************************************/
  /**
   * @fn     ~TcpDevice
   * @brief  class deconstructor
   * @param  
   * @return 
   *
   */
  TcpDevice::~TcpDevice() {
  }
  /** Sequencer::TcpDevice::~TcpDevice ****************************************/


  /** Sequencer::TcpDevice::send **********************************************/
  /**
   * @fn     send
   * @brief  
   * @param  
   * @return 
   *
   */
  long TcpDevice::send( std::string command, std::string &reply ) {
    std::string function = "Sequencer::TcpDevice::send";
    std::stringstream message;
    char buf[BUFSIZE+1];
    long ret;

    memset( buf, '\0', BUFSIZE+1 );

    // send the command
    //
    this->socket.Write( command );

    // Wait (poll) connected socket for incoming data...
    //
    int pollret;
    if ( ( pollret = this->socket.Poll() ) <= 0 ) {
      if ( pollret == 0 ) {
        logwrite( function, "Poll timeout" );
      }
      if ( pollret <0 ) {
        message.str(""); message << "Poll error: " << strerror(errno);
        logwrite( function, message.str() );
      }
      return ERROR;
    }

    // read the response
    //
    if ( ( ret = this->socket.Read(buf, (size_t)BUFSIZE) ) <= 0 ) {
      if ( ret < 0) {             // could be an actual read error
        message.str(""); message << "Read error: " << strerror(errno); logwrite(function, message.str());
      }
    }

    // assign the response to the reply string, passed in by reference
    //
    reply.assign( buf, BUFSIZE );
    reply.erase( std::remove(reply.begin(), reply.end(), '\r' ), reply.end() );
    reply.erase( std::remove(reply.begin(), reply.end(), '\n' ), reply.end() );

    return NO_ERROR;
  }
  /** Sequencer::TcpDevice::send **********************************************/


  /** Sequencer::Daemon::Daemon ***********************************************/
  /**
   * @fn     Daemon
   * @brief  class constructor
   * @param  
   * @return 
   *
   */
  Daemon::Daemon() {
    this->socket.sethost( "localhost" );
    this->port = -1;
  }
  /** Sequencer::Daemon::Daemon ***********************************************/


  /** Sequencer::Daemon::~Daemon **********************************************/
  /**
   * @fn     ~Daemon
   * @brief  class deconstructor
   * @param  
   * @return 
   *
   */
  Daemon::~Daemon() {
  }
  /** Sequencer::Daemon::~Daemon **********************************************/


  void Daemon::configure( std::string name, int port ) {
    std::string function = "Sequences::Daemon::configure";
    std::stringstream message;
    this->name = name;
    this->port = port;
    message << this->name << " configured with port " << this->port;
    logwrite( function, message.str() );
    return;
  }

  /** Sequencer::Daemon::send *************************************************/
  /**
   * @fn     send
   * @brief  
   * @param  
   * @return 
   *
   */
  long Daemon::send( std::string command, std::string &reply ) {
    std::string function = "Sequencer::Daemon::send";
    std::stringstream message;
    long ret;


    // send the command
    //
    command.append( "\n" );
    this->socket.Write( command );

    // Wait (poll) connected socket for incoming data...
    //
    int pollret;
    if ( ( pollret = this->socket.Poll() ) <= 0 ) {
      if ( pollret == 0 ) {
        logwrite( function, "Poll timeout" );
      }
      if ( pollret <0 ) {
        message.str(""); message << "Poll error: " << strerror(errno);
        logwrite( function, message.str() );
      }
      return ERROR;
    }

    // read the response
    //
    char delim = '\n';
    if ( ( ret = this->socket.Read( reply, delim ) ) <= 0 ) {
      if ( ret < 0 && errno != EAGAIN ) {             // could be an actual read error
        message.str(""); message << "Read error: " << strerror(errno); logwrite(function, message.str());
      }
      if ( ret==0 ) logwrite( function, "timeout reading from socket" );
    }

    // assign the response to the reply string, passed in by reference
    //
    reply.erase( std::remove(reply.begin(), reply.end(), '\r' ), reply.end() );
    reply.erase( std::remove(reply.begin(), reply.end(), '\n' ), reply.end() );

    return NO_ERROR;
  }
  /** Sequencer::Daemon::send *************************************************/


  /** Sequencer::Daemon::command **********************************************/
  /**
   * @fn     command
   * @brief  
   * @param[in]  args, string to send
   * @return 
   *
   */
  long Daemon::command( std::string args ) {
    std::string dontcare;
    return( this->command( args, dontcare ) );
  }
  long Daemon::command( std::string args, std::string &retstring ) {
    std::string function = "Sequencer::Daemon::command";
    std::stringstream message;
    long error = NO_ERROR;

    // this is probably a programming error if we're in here and port is undefined
    //
    if ( this->port < 0 ) {
      message.str(""); message << "daemon \"" << this->name << "\" not configured";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // open
    //
    if ( args == "open" ) {
      // initialize socket connection
      //
      if ( ( error = this->connect() ) != NO_ERROR ) return( error );

      // send the open command
      //
      error = this->send( "open", retstring );

      if ( error == NO_ERROR ) {
        message.str(""); message << "opened " << this->name << "...";
      }
      else {
        message.str(""); message << "error opening " << this->name << "...";
      }
      logwrite( function, message.str() );
    }
    else

    // close
    //
    if ( args == "close" ) {
      // send the close command
      //
      message.str(""); message << "closing " << this->name << "...";
      logwrite( function, message.str() );
      error = this->send( "close", retstring );

      // then close the connection
      //
      this->socket.Close();
    }

    // all other commands go straight on through, as-is
    //
    else {
      // but only if the connection is open of course
      //
      if ( !this->socket.isconnected() ) {
        message.str(""); message << "ERROR: connection not open to " << this->name;
        logwrite( function, message.str() );
        error = ERROR;
      }
      else {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] sending to " << this->name << ": " << args;
        logwrite( function, message.str() );
#endif
        error = this->send( args, retstring );
      }
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] reply from " << this->name << ": " << retstring;
    logwrite( function, message.str() );
#endif

    return( error );
  }
  /** Sequencer::Daemon::command **********************************************/


  /** Sequencer::Daemon::connect **********************************************/
  /**
   * @fn         connect
   * @brief      initialize socket connection to the daemon
   * @param[in]  none  
   * @return     ERROR or NO_ERROR
   *
   * This function establishes a socket connection to the daemon
   * using the Network::TcpSocket class.
   *
   */
  long Daemon::connect( ) {
    std::string function = "Sequencer::Daemon::connect";
    std::stringstream message;
    long error = NO_ERROR;

    // probably a programming error if this Daemon object is not configured
    //
    if ( this->port == -1 ) {
      message.str(""); message << "ERROR: port not configured for " << this->name;
      logwrite( function, message.str() );
      error = ERROR;
    }
    else {

      // If already connected then close the connection
      //
      if ( this->socket.isconnected() ) {
        message.str(""); message << "closing existing connection to " << this->name;
        logwrite( function, message.str() );
        this->socket.Close();
      }

      this->socket.setport( this->port );

      // Create and connect to the socket
      //
      if ( this->socket.Connect() < 0 ) {
        message.str(""); message << "ERROR: connecting to " << this->name << " on port " << this->port;
        logwrite( function, message.str() );
        error = ERROR;
      } else {
        message.str(""); message << "connected to " << this->name << " on port " << this->port;
        logwrite( function, message.str() );
      }
    }

    return( error );
  }
  /** Sequencer::Daemon::connect **********************************************/

}
