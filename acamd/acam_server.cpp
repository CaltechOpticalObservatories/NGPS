/**
 * @file    acam_server.cpp
 * @brief   these are the main functions used by the Acam::Server
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file defines the functions used by the Server class in the Acam namespace.
 *
 */

#include "acam_server.h"

namespace Acam {

  /***** Acam::Server::exit_cleanly *******************************************/
  /**
   * @brief      shutdown nicely
   *
   */
  void Server::exit_cleanly(void) {
    std::string function = "Acam::Server::exit_cleanly";
    logwrite( function, "exiting" );
    _exit(EXIT_SUCCESS);
  }
  /***** Acam::Server::exit_cleanly *******************************************/


  /***** Acam::Server::configure_acam *****************************************/
  /**
   * @brief      read and apply the configuration file
   * @return     ERROR or NO_ERROR
   *
   */
  long Server::configure_acamd() {
    std::string function = "Acam::Server::configure_acamd";
    std::stringstream message;
    int applied=0;
    long error;

    // Clear the motormap map before loading new information from the config file
    //
    this->interface.motion.motormap.clear();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      // PI_NAME -- this is the name of the PI motor controller subsystem
      //
      if ( config.param[entry].find( "PI_NAME" ) == 0 ) {
        this->interface.motion.name = config.arg[entry];
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // PI_HOST -- hostname for the master PI motor controller
      //
      if ( config.param[entry].find( "PI_HOST" ) == 0 ) {
        this->interface.motion.host = config.arg[entry];
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // PI_PORT -- port number on PI_HOST for the master PI motor controller
      //
      if ( config.param[entry].find( "PI_PORT" ) == 0 ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad PI_PORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "PI_PORT number out of integer range");
          return(ERROR);
        }
        this->interface.motion.port = port;
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // CAMERASERVER_HOST -- hostname for the external camera server
      //
      if ( config.param[entry].find( "CAMERASERVER_HOST" ) == 0 ) {
        this->interface.cameraserver_host = config.arg[entry];
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // CAMERASERVER_PORT -- port number on CAMSERVER_HOST for the external camera server
      //
      if ( config.param[entry].find( "CAMERASERVER_PORT" ) == 0 ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad CAMSERVER_PORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "CAMSERVER_PORT number out of integer range");
          return(ERROR);
        }
        this->interface.cameraserver_port = port;
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // MOTOR_CONTROLLER -- address and name of each PI motor controller in daisy-chain
      //
      if ( config.param[entry].find( "MOTOR_CONTROLLER" ) == 0 ) {
        // Create temporary object for parsing the config line. If no error
        // then this object gets copied into the class map of objects.
        //
        Physik_Instrumente::ControllerInfo<Physik_Instrumente::StepperInfo> MOT;
        if ( MOT.load_controller_info( config.arg[entry] ) == NO_ERROR ) {
          this->interface.motion.motormap[ MOT.name ] = MOT;
          message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
          logwrite( function, message.str() );
          this->interface.async.enqueue( message.str() );
          applied++;
        }
      }

      // MOTOR_POS -- detailed position info for each named motor controller
      //
      if ( config.param[entry].find( "MOTOR_POS" ) == 0 ) {
        // Create temporary object for parsing the config line. If no error
        // and a matching name is already in the motormap, then relevant parts
        // of this object gets copied into the class map of objects.
        //
        Physik_Instrumente::ControllerInfo<Physik_Instrumente::StepperInfo> MOT;
        if ( MOT.load_pos_info( config.arg[entry] ) == NO_ERROR ) {

          // Make sure the MOTOR_POS's name has a matching name in controller_info
          // which came from the MOTOR_CONTROLLER configuration.
          //
          auto loc = this->interface.motion.motormap.find( MOT.name );

          // If found, then assign the motor map from the local object to the class object.
          //
          if ( loc != this->interface.motion.motormap.end() ) {
            std::string posname = MOT.posmap.begin()->first;  // mot is a local copy so there is only one entry
            loc->second.posmap[ posname ].id       = MOT.posmap[posname].id;
            loc->second.posmap[ posname ].position = MOT.posmap[posname].position;
            loc->second.posmap[ posname ].posname  = MOT.posmap[posname].posname;
          }
          else {
            message.str(""); message << "ERROR: MOTOR_POS name \"" << MOT.name << "\" "
                                     << "has no matching name defined by MOTOR_CONTROLLER";
            logwrite( function, message.str() );
            error = ERROR;
            break;
          }

          message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
          this->interface.async.enqueue_and_log( function, message.str() );
          applied++;
        }
      }

      // NBPORT -- nonblocking listening port for the acam daemon
      //
      if ( config.param[entry].find( "NBPORT" ) == 0 ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad NBPORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "NBPORT number out of integer range");
          return(ERROR);
        }
        this->nbport = port;
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // BLKPORT -- blocking listening port for the acam daemon
      //
      if ( config.param[entry].find( "BLKPORT" ) == 0 ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad BLKPORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "BLKPORT number out of integer range");
          return(ERROR);
        }
        this->blkport = port;
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // MESSAGEPORT -- asynchronous broadcast message port for the acam daemon
      //
      if ( config.param[entry].find( "MESSAGEPORT" ) == 0 ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch (std::invalid_argument &) {
          logwrite(function, "ERROR: bad MESSAGEPORT: unable to convert to integer");
          return(ERROR);
        }
        catch (std::out_of_range &) {
          logwrite(function, "MESSAGEPORT number out of integer range");
          return(ERROR);
        }
        this->asyncport = port;
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

      // MESSAGEGROUP -- asynchronous broadcast group for the acam daemon
      //
      if ( config.param[entry].find( "MESSAGEGROUP" ) == 0 ) {
        this->asyncgroup = config.arg[entry];
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->interface.async.enqueue( message.str() );
        applied++;
      }

    } // end loop through the entries in the configuration file

    message.str("");
    if (applied==0) {
      message << "ERROR: ";
      error = ERROR;
    }
    else {
      error = NO_ERROR;
    }
    message << "applied " << applied << " configuration lines to acamd";
    logwrite(function, message.str());

    // Initialize the class using the config parameters just read
    //
    if ( error == NO_ERROR ) error = this->interface.initialize_class();
    if ( error == NO_ERROR ) error = this->interface.motion.initialize_class();

    return error;
  }
  /***** Acam::Server::configure_acam *****************************************/


  /***** new_log_day **********************************************************/
  /**
   * @brief      creates a new logbook each day
   * @param[in]  logpath  path for the log file, read from config file
   *
   * This thread is started by main and never terminates.
   * It sleeps for the number of seconds that logentry determines
   * are remaining in the day, then closes and re-inits a new log file.
   *
   * The number of seconds until the next day "nextday" is a global which
   * is set by init_log.
   *
   */
  void Server::new_log_day( std::string logpath ) {
    while (1) {
      std::this_thread::sleep_for( std::chrono::seconds( nextday ) );
      close_log();
      init_log( logpath, DAEMON_NAME );
    }
  }
  /***** new_log_day **********************************************************/


  /***** block_main ***********************************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  acam  reference to Acam::Server object
   * @param[in]  sock  Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( Acam::Server &acam, Network::TcpSocket sock ) {
    while(1) {
      sock.Accept();
      acam.doit(sock);              // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** block_main ***********************************************************/


  /***** thread_main **********************************************************/
  /**
   * @brief      main function for all non-blocked threads
   * @param[in]  acam  reference to Acam::Server object
   * @param[in]  sock  Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * There are N_THREADS-1 of these, one for each non-blocking connection.
   * These threads never terminate.
   *
   * This function differs from block_main only in that the call to Accept
   * is mutex-protected.
   *
   */
  void Server::thread_main( Acam::Server &acam, Network::TcpSocket sock ) {
    while (1) {
      acam.conn_mutex.lock();
      sock.Accept();
      acam.conn_mutex.unlock();
      acam.doit(sock);           // call function to do the work
      sock.Close();
    }
    return;
  }
  /***** thread_main **********************************************************/


  /***** async_main ***********************************************************/
  /**
   * @brief      asynchronous message sending thread
   * @param[in]  acam  reference to Acam::Server object
   * @param[in]  sock  Network::UdpSocket socket object
   *
   * Loops forever, when a message arrives in the status message queue it is
   * sent out via multi-cast UDP datagram.
   *
   */
  void Server::async_main( Acam::Server &acam, Network::UdpSocket sock ) {
    std::string function = "Acam::Server::async_main";
    int retval;

    retval = sock.Create();                                   // create the UDP socket
    if (retval < 0) {
      logwrite(function, "error creating UDP multicast socket for asynchronous messages");
      acam.exit_cleanly();                                    // do not continue on error
    }
    if (retval==1) {                                          // exit this thread but continue with daemon
      logwrite(function, "asyncrhonous message port disabled by request");
    }

    while (1) {
      std::string message = acam.interface.async.dequeue();   // get the latest message from the queue (blocks)
      retval = sock.Send(message);                            // transmit the message
      if (retval < 0) {
        std::stringstream errstm;
        errstm << "error sending UDP message: " << message;
        logwrite(function, errstm.str());
      }
      if (message=="exit") {                                  // terminate this thread
        sock.Close();
        return;
      }
    }

    return;
  }
  /***** async_main ***********************************************************/


  /***** doit *****************************************************************/
  /**
   * @brief      the workhorse of each thread connetion
   * @param[in]  sock  Network::UdpSocket socket object
   *
   * stays open until closed by client
   *
   * commands come in the form: 
   * "<device> [all|<app>] [_BLOCK_] <command> [<arg>]"
   *
   * Valid commands are listed in acamd_commands.h
   *
   */
  void Server::doit( Network::TcpSocket sock ) {
    std::string function = "Acam::Server::doit";
    long  ret;
    std::stringstream message;
    std::string cmd, args;        // arg string is everything after command
    std::vector<std::string> tokens;

    bool connection_open=true;

    PySCOPE();

    while (connection_open) {

      // Wait (poll) connected socket for incoming data...
      //
      int pollret;
      if ( ( pollret=sock.Poll() ) <= 0 ) {
        if (pollret==0) {
          message.str(""); message << "Poll timeout on fd " << sock.getfd() << " thread " << sock.id;
          logwrite(function, message.str());
        }
        if (pollret <0) {
          message.str(""); message << "Poll error on fd " << sock.getfd() << " thread " << sock.id << ": " << strerror(errno);
          logwrite(function, message.str());
        }
        break;                      // this will close the connection
      }

      // Data available, now read from connected socket...
      //
      std::string buf;
      char delim='\n';
      if ( ( ret=sock.Read( buf, delim ) ) <= 0 ) {
        if (ret<0) {                // could be an actual read error
          message.str(""); message << "Read error on fd " << sock.getfd() << ": " << strerror(errno); logwrite(function, message.str());
        }
        if (ret==-2) {              // or a timeout
          message.str(""); message << "timeout reading from fd " << sock.getfd();
          logwrite( function, message.str() );
        }
        break;                      // Breaking out of the while loop will close the connection.
                                    // This probably means that the client has terminated abruptly, 
                                    // having sent FIN but not stuck around long enough
                                    // to accept CLOSE and give the LAST_ACK.
      }

      // convert the input buffer into a string and remove any trailing linefeed
      // and carriage return
      //
      buf.erase(std::remove(buf.begin(), buf.end(), '\r' ), buf.end());
      buf.erase(std::remove(buf.begin(), buf.end(), '\n' ), buf.end());

      if (buf.empty()) {sock.Write("\n"); continue;}   // acknowledge empty command so client doesn't time out

      try {
        std::size_t cmd_sep = buf.find_first_of(" ");  // find the first space, which separates command from argument list

        cmd = buf.substr(0, cmd_sep);                  // cmd is everything up until that space

        if (cmd.empty()) {sock.Write("\n"); continue;} // acknowledge empty command so client doesn't time out

        if (cmd_sep == std::string::npos) {            // If no space was found,
          args="";                                     // then the arg list is empty,
        }
        else {
          args= buf.substr(cmd_sep+1);                 // otherwise args is everything after that space.
        }

        sock.id = ++this->cmd_num;
        if ( this->cmd_num == INT_MAX ) this->cmd_num = 0;

        message.str(""); message << "received command on fd " << sock.getfd() << " (" << sock.id << "): " << cmd << " " << args;
        logwrite(function, message.str());
      }
      catch ( std::runtime_error &e ) {
        std::stringstream errstream; errstream << e.what();
        message.str(""); message << "error parsing arguments: " << errstream.str();
        logwrite(function, message.str());
        ret = -1;
      }
      catch ( ... ) {
        message.str(""); message << "unknown error parsing arguments: " << args;
        logwrite(function, message.str());
        ret = -1;
      }

      // process commands here
      //
      ret = NOTHING;
      std::string retstring="";
      bool suppress_term_state=false;

      if ( cmd == "help" || cmd == "?" ) {
                      for ( auto s : ACAMD_SYNTAX ) { sock.Write( s ); sock.Write( "\n" ); }
      }
      else
      if ( cmd == ACAMD_EXIT ) {
                      this->exit_cleanly();                      // shutdown the daemon
      }
      else

      if ( cmd == ACAMD_TCSINIT ) {
                      ret = this->interface.tcsd.init( args, retstring );
      }
      else
      if ( cmd == ACAMD_TCSISOPEN ) {
                      ret = this->interface.tcsd.client.is_open();
                      retstring = ( ret ? "true" : "false" );
                      ret = 0;
      }
      else
      if ( cmd == ACAMD_TCSISCONNECTED ) {
                      ret = this->interface.tcsd.client.is_connected(retstring);
      }
      else
      if ( cmd == ACAMD_TCSGET ) {
                      double foo;
                      this->interface.tcsd.get_cass( foo );
                      double ra_h, dec_d;
                      this->interface.tcsd.get_coords( ra_h, dec_d );
                      retstring = std::to_string(foo) + " " + std::to_string(ra_h) + " " + std::to_string(dec_d);
                      ret = 0;
      }
      else
      // commands for the Andor camera direct
      //
      if ( cmd == "cameraacquire" ) {
                      ret = this->interface.camera.start_acquisition( );
      }
      else
      if ( cmd == "camerastat" ) {
                      ret = this->interface.camera.get_status( );
      }
      else
      if ( cmd == "camerashutter" ) {
                      ret = this->interface.camera.andor.shutter( );
      }
      else
      if ( cmd == "camerafits" ) {
                      ret = this->interface.test_image( );
      }
      else
      if ( cmd == "cameragetframe" ) {
                      ret = this->interface.camera.get_frame( );
      }
      else
      if ( cmd == "cameratest" ) {
                      ret = this->interface.camera.andor.test( );
      }
      else
      if ( cmd == "testacquire" ) {
                      ret = this->interface.camera.andor.acquire_one();
      }
      else
      if ( cmd == "testsave" ) {
                      ret = this->interface.camera.andor.save_acquired( args, retstring );

      }
      else

#ifdef ACAM_ANDOR_SOURCE_SERVER
      // commands for the external camera server
      //
      if ( cmd == ACAMD_CAMERASERVER_COORDS ) {
                      ret = this->interface.camera_server.coords( args );
      }
      else
#endif

      // open connections to all devices, camera and motion
      //
      if ( cmd == ACAMD_OPEN ) {
                      ret = this->interface.open( args, retstring );
      }
      else

      // isopen
      //
      if ( cmd == ACAMD_ISOPEN ) {
                      bool isopen;
                      ret = this->interface.isopen( args, isopen, retstring );
                      if ( retstring.empty() ) { if ( isopen ) retstring = "true"; else retstring = "false"; }
      }
      else

      // close
      //
      if ( cmd == ACAMD_CLOSE ) {
                      ret = this->interface.close( args, retstring );
      }
      else
      if ( cmd == ACAMD_BIN ) {
                      ret = this->interface.camera.bin( args, retstring );
      }
      else
      if ( cmd == ACAMD_IMFLIP ) {
                      ret = this->interface.camera.imflip( args, retstring );
      }
      else
      if ( cmd == ACAMD_IMROT ) {
                      ret = this->interface.camera.imrot( args, retstring );
      }
      else
      if ( cmd == ACAMD_EXPTIME ) {
                      ret = this->interface.camera.exptime( args, retstring );
      }
      else
      if ( cmd == ACAMD_GAIN ) {
                      ret = this->interface.camera.gain( args, retstring );
      }
      else
      if ( cmd == ACAMD_GUIDESET ) {
                      ret = this->interface.guider_settings_control( args, retstring );
                      suppress_term_state = true;  // suppress the terminating state message
      }
      else
      if ( cmd == ACAMD_SPEED ) {
                      ret = this->interface.camera.speed( args, retstring );
      }
      else
      if ( cmd == ACAMD_TEMP ) {
                      ret = this->interface.camera.temperature( args, retstring );
      }
      else
      if ( cmd == ACAMD_SIMANDOR ) {
                      ret = this->interface.camera.simandor( args, retstring );
      }
      else

      // config
      //
      if ( cmd == ACAMD_CONFIG ) {
                      if ( args == "?" ) sock.Write( ACAMD_CONFIG
                                                     +"\n  re-read, parse, and re-apply config file: "
                                                     +this->config.filename+"\n" );
                      else ret = this->interface.configure_interface( config );
      }
      else

      // commands for the Python functions
      //
      if ( cmd == ACAMD_SOLVE ) {
                      ret = this->interface.solve( args, retstring );
      }
      else

      if ( cmd == ACAMD_QUALITY ) {
                      ret = this->interface.image_quality( args, retstring );
      }
      else

      if ( cmd == ACAMD_ECHO ) {
                      if ( args == "?" ) sock.Write( ACAMD_ECHO
                                                     +" <string>\n  Daemon receives and writes back <string> to the client.\n"
                                                     +"  Used to test if the daemon is responsive.\n" );
                      else {
                        sock.Write( args );
                        sock.Write( "\n" );
                      }
      }
      else

      // ACQUIRE
      //
      if ( cmd == ACAMD_ACQUIRE ) {
                      ret = this->interface.acquire( args, retstring );
      }
      else
      // ACQUIREFIX
      //
      if ( cmd == ACAMD_ACQUIREFIX ) {
                      ret = this->interface.acquire_fix( args, retstring );
      }
      else
      if ( cmd == ACAMD_INIT ) {
                      this->interface.acquire_init( );
                      ret = NO_ERROR;  // acquire_init() returns void, never fails
      }
      else

      // motion control commands
      //
      if ( cmd == ACAMD_MOTION ) {
                      ret = this->interface.motion.motion( args, retstring );
      }
      else

      if ( cmd == ACAMD_HOME ) {
                      ret = this->interface.motion.home( args, retstring );
      }
      else

      // is_home
      //
      if ( cmd == ACAMD_ISHOME ) {
                      ret = this->interface.motion.is_home( args, retstring );
      }
      else

      // set/get filter
      //
      if ( cmd == ACAMD_FILTER ) {
                      ret = this->interface.motion.filter( args, retstring );
      }
      else

      // set/get dust cover
      //
      if ( cmd == ACAMD_COVER ) {
                      ret = this->interface.motion.cover( args, retstring );
      }
      else

      // test commands
      //
      if ( cmd == ACAMD_TEST ) {
                      ret = this->interface.test( args, retstring );
      }

      // unknown commands generate an error
      //
      else {
        message.str(""); message << "ERROR: unknown command: " << cmd;
        logwrite( function, message.str() );
        ret = ERROR;
      }

      if (ret != NOTHING) {
        if ( !retstring.empty() ) retstring.append( " " );
        std::string term=(ret==0?"DONE\n":"ERROR\n");
        retstring.append( suppress_term_state ? "\n" : term );
        if ( sock.Write( retstring ) < 0 ) connection_open=false;
      }

      if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                           // Keep blocking connection open for interactive session.
    }

    sock.Close();
    return;
  }
  /***** doit *****************************************************************/

}
