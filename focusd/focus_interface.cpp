/**
 * @file    focus_interface.cpp
 * @brief   this contains the focus interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Focus namespace,
 * and is how the focus daemon interfaces to the focus hardware.
 *
 */

#include "focus_interface.h"

namespace Focus {

  /***** Focus::Interface::initialize_class ***********************************/
  /**
   * @brief      initializes the class from configure_focusd()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Focus::Server::configure_focusd() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Focus::Interface::initialize_class";
    std::stringstream message;

    if ( this->port < 0 || this->host.empty() ) {
      message.str(""); message << "ERROR: host (" << this->host << ") or port (" << this->port << ") invalid";
      logwrite( function, message.str() );
      return( ERROR );
    }

    Physik_Instrumente::Interface s( this->name, this->host, this->port );
    this->pi = s;

    this->numdev = this->motormap.size();

    // I don't want to prevent the system from working with a subset of controllers,
    // but the user should be warned, in case it wasn't intentional.
    //
    if ( this->numdev != 3 ) {
      message.str(""); message << "WARNING: " << this->numdev << " PI motor controller"
                               << ( this->numdev == 1 ? "" : "s" ) << " defined!";
      logwrite( function, message.str() );
    }

    return( NO_ERROR );
  }
  /***** Focus::Interface::initialize_class ***********************************/


  /***** Focus::Interface::open ***********************************************/
  /**
   * @brief      opens the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    std::string function = "Focus::Interface::open";
    std::stringstream message;

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Focus::Server::configure_focusd).
    //
    if ( !this->pi.is_initialized() ) {
      logwrite( function, "ERROR: pi interface not initialized" );
      return( ERROR );
    }

    // open a connection
    //
    long error = this->pi.open();

    if ( error != NO_ERROR ) return( error );

    // clear any error codes on startup, and
    // enable the servo for each address in controller_info
    //
    for ( auto const &con : this->motormap ) {
      int axis=1;
      int errcode;
      error |= this->pi.get_error( con.second.addr, errcode );     // read error to clear, don't care the value
      error |= this->pi.set_servo( con.second.addr, axis, true );  // turn the servos on
    }

    return( error );
  }
  /***** Focus::Interface::open ***********************************************/


  /***** Focus::Interface::close **********************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    std::string function = "Focus::Interface::close";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "not connected" );
      return( NO_ERROR );
    }

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Focus::Server::configure_focusd).
    //
    if ( !this->pi.is_initialized() ) {
      logwrite( function, "ERROR: pi interface not initialized" );
      return( ERROR );
    }

    return( this->pi.close() );
  }
  /***** Focus::Interface::close **********************************************/


  /***** Focus::Interface::native *********************************************/
  /**
   * @brief      send native command to controller identified by channel name
   * @param[in]  args       contains channel name, command and arg(s)
   * @param[out] retstring  return string
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::native( std::string args, std::string &retstring ) {
    std::string function = "Focus::Interface::native";
    std::stringstream message;
    std::string chan, cmd;

    // Help
    //
    if ( args == "?" ) {
      retstring = FOCUSD_NATIVE;
      retstring.append( " <chan> <cmd>\n" );
      retstring.append( "  Send native command <cmd> to controller indicated by channel name,\n" );
      retstring.append( "  where <chan> is one of { " );
      for ( auto const &mot : this->motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "} and <cmd> is any PI-native command and args. This command blocks;\n" );
      retstring.append( "native commands are not run in a separate thread.\n" );
      return( NO_ERROR );
    }

    // Anything else requires an open connection
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      retstring="not_connected";
      return( ERROR );
    }

    // Need something, anything
    //
    if ( args.empty() ) {
      logwrite( function, "ERROR expected <chan> <cmd>" );
      retstring="invalid_argument";
      return( ERROR );
    }

    std::transform( args.begin(), args.end(), args.begin(), ::toupper );

    std::size_t cmd_sep = args.find_first_of( " " );        // find first space, which separates <chan> from <cmd>

    if ( cmd_sep == std::string::npos ) {                   // no <cmd>
      logwrite( function, "ERROR expected <chan> <cmd>" );
      retstring="invalid_argument";
      return( ERROR );
    }
    else {
      chan = args.substr( 0, cmd_sep );                     // <chan> is first part before space
      cmd  = args.substr( cmd_sep+1  );                     // <cmad> is everything after the space
    }

    if ( motormap.find( chan ) == motormap.end() ) {
      message.str(""); message << "ERROR motor \"" << chan << "\" not found";
      logwrite( function, message.str() );
      retstring="unknown_motor";
      return( ERROR );
    }

    message.str("");
    message << motormap.at(chan).addr << " " << cmd;

    logwrite( function, message.str() );

    return send_command( message.str(), retstring );
  }
  /***** Focus::Interface::native *********************************************/


  /***** Focus::Interface::home ***********************************************/
  /**
   * @brief      home all or indicated daisy-chained motors
   * @param[in]  arg        optional arg for help only
   * @param[out] retstring  return string
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::home( std::string arg, std::string &retstring ) {
    std::string function = "Focus::Interface::home";
    std::stringstream message;
    int axis=1;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = FOCUSD_HOME;
      retstring.append( " [ <axis> ]\n" );
      retstring.append( "  Home all focus motors or single motor indicated by optional <axis>.\n" );
      retstring.append( "  If no argument is supplied then all axes are homed simultaneously,\n" );
      retstring.append( "  or a single axis may be supplied from { " );
      for ( auto const &mot : this->motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "}.\n" );
      return( NO_ERROR );
    }

    // Anything else requires an open connection
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      retstring="not_connected";
      return( ERROR );
    }

    // Spawn a thread to home motor(s)
    //

    std::unique_lock<std::mutex> wait_lock( this->wait_mtx );  // create a mutex object for waiting for threads
    this->thr_error.store( NO_ERROR );                         // clear the thread error state (threads can set this)

    // If arg supplied then try to home just that home
    //
    if ( !arg.empty() ) {
      std::transform( arg.begin(), arg.end(), arg.begin(), ::toupper );
      if ( motormap.find( arg ) == motormap.end() ) {
        message.str(""); message << "ERROR motor \"" << arg << "\" not found";
        logwrite( function, message.str() );
        retstring="unknown_motor";
        return( ERROR );
      }
      std::thread( dothread_home, std::ref( *this ), arg ).detach();
      motors_running++;
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] spawning thread to home " << arg;
        logwrite( function, message.str() );
#endif
    }

    // otherwise home all motors in the motormap
    //
    else {
      for ( auto const &mot : this->motormap ) {
        std::thread( dothread_home, std::ref( *this ), mot.first ).detach();
        this->motors_running++;
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] spawning thread to home " << name;
        logwrite( function, message.str() );
#endif
      }
    }

    // wait for the threads to finish
    //
    while ( motors_running != 0 ) {
      message.str(""); message << "waiting for " << motors_running << " motor" << ( motors_running > 1 ? "s":"" );
      logwrite( function, message.str() );
      cv.wait( wait_lock );
    }

    logwrite( function, "home complete" );

    // get any errors from the threads
    //
    error = this->thr_error.load();

    return( error );
  }
  /***** Focus::Interface::home ***********************************************/


  /***** Focus::Interface::is_home ********************************************/
  /**
   * @brief       return the home state of the motors
   * @param[in]   arg        used only for help
   * @param[out]  retstring  contains the home state "true" | "false"
   * @return      ERROR or NO_ERROR
   *
   * All motors must be homed for this to return "true".
   *
   */
  long Interface::is_home( std::string arg, std::string &retstring ) {
    std::string function = "Focus::Interface::is_home";
    std::stringstream message;
    std::stringstream homestream;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = FOCUSD_ISHOME;
      retstring.append( " \n" );
      retstring.append( "  Reads the referencing state from each of the controllers.\n" );
      retstring.append( "  Returns true if all are homed, false if any one is not homed.\n" );
      return( NO_ERROR );
    }

    // Anything else requires an open connection
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      retstring="not_connected";
      return( ERROR );
    }

    // Loop through all motor controllers, asking each if homed,
    // setting each controller's .ishome flag, and keeping count 
    // of the number that are homed.
    //
    size_t num_home=0;
    for ( auto &con : this->motormap ) {
      int axis=1;
      error |= this->pi.is_home( con.second.addr, axis, con.second.ishome );  // error is OR'd so any error is preserved
      homestream << con.second.addr << ":" << ( con.second.ishome ? "true" : "false" ) << " ";
      if ( con.second.ishome ) num_home++;
    }

    // Set the retstring true or false, true only if all controllers are homed.
    //
    if ( num_home == this->numdev ) retstring = "true"; else retstring = "false";

    // If not all are the same state then log that
    //
    if ( num_home > 0 && num_home < this->numdev ) {
      message.str(""); message << "NOTICE: " << homestream.str();
      logwrite( function, message.str() );
    }

    return( error );
  }
  /***** Focus::Interface::is_home ********************************************/


  /***** Focus::Interface::set ************************************************/
  /**
   * @brief      set the focus position of the selected channel
   * @param[in]  args       contains channel name and focus position
   * @param[out] retstring  return string
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set( std::string args, std::string &retstring ) {
    std::string function = "Focus::Interface::set";
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
      retstring = FOCUSD_SET;
      retstring.append( " <chan> { <pos> | nominal }\n" );
      retstring.append( "  Set focus position of indicated channel to <pos> or to the nominal best focus.\n" );
      retstring.append( "  where <chan> <min> <nominal> <max> are as follows:\n" );
      for ( auto const &mot : motormap ) {
        retstring.append( "     " );
        retstring.append( mot.first ); retstring.append( " " );
        message.str(""); message << std::fixed << std::setprecision(3) << mot.second.min << " ";
        retstring.append( message.str() );
        message.str(""); message << std::fixed << std::setprecision(3) << mot.second.posmap.at("nominal").position << " ";
        retstring.append( message.str() );
        message.str(""); message << std::fixed << std::setprecision(3) << mot.second.max << " ";
        retstring.append( message.str() );
        retstring.append( "\n" );
      }
      return( NO_ERROR );
    }

    // Anything else requires an open connection
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      retstring="not_connected";
      return( ERROR );
    }

    // Tokenize the arg string. Need two tokens: <chan> and either <pos> or "nominal"
    //
    std::transform( args.begin(), args.end(), args.begin(), ::toupper );
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      logwrite( function, "ERROR expected <chan> { <pos> | nominal }" );
      retstring="invalid_argument";
      return( ERROR );
    }

    float pos=NAN;
    int addr=-1;
    std::string chan, posstr;

    // Extract the address and position from the arg string tokens
    //
    try {
      chan   = tokens.at(0);
      posstr = tokens.at(1);

      if ( motormap.find( chan ) == motormap.end() ) {
        message.str(""); message << "ERROR motor \"" << chan << "\" not found";
        logwrite( function, message.str() );
        retstring="unknown_motor";
        return( ERROR );
      }

      addr = motormap.at( chan ).addr;

      if ( posstr == "NOMINAL" ) {  // args string was capitalized
        pos = motormap.at(chan).posmap.at("nominal").position;
      }
      else {
        pos = std::stof( posstr );
      }
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing " << args << ": " << e.what();
      logwrite( function, message.str() );
      retstring="argument_exception";
      return( ERROR );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR parsing " << args << ": " << e.what();
      logwrite( function, message.str() );
      retstring="argument_exception";
      return( ERROR );
    }

    // Perform the move
    //
    long error = move_abs( addr, pos );

    message.str("");
    message << ( error==NO_ERROR ? "success" : "failed" )
            << " moving " << chan << " focus to " << pos;
    logwrite( function, message.str() );

    return error;
  }
  /***** Focus::Interface::set ************************************************/


  /***** Focus::Interface::get ************************************************/
  /**
   * @brief      get the current position of channel indicated by arg
   * @param[in]  arg        string contains the channel name
   * @param[out] retstring  reference to string contains focus position
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get( std::string arg, std::string &retstring ) {
    std::string function = "Focus::Interface::get";
    std::stringstream message;
    long error  = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = FOCUSD_GET;
      retstring.append( " <chan>\n" );
      retstring.append( "  Get the focus position of the indicated channel\n" );
      retstring.append( "  where <chan> is one of { " );
      for ( auto const &mot : this->motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "}.\n" );
      return( NO_ERROR );
    }

    // Anything else requires an open connection
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      retstring="not_connected";
      return( ERROR );
    }

    // Need something, anything
    //
    if ( arg.empty() ) {
      logwrite( function, "ERROR expected <chan>" );
      retstring="invalid_argument";
      return( ERROR );
    }

    std::transform( arg.begin(), arg.end(), arg.begin(), ::toupper );

    if ( motormap.find( arg ) == motormap.end() ) {
      message.str(""); message << "ERROR motor \"" << arg << "\" not found";
      logwrite( function, message.str() );
      retstring="unknown_motor";
      return( ERROR );
    }

    // get the position for the requested channel
    //
    std::string posstring;
    int axis=1;
    float pos=NAN;
    error = pi.get_pos( motormap.at(arg).addr, axis, pos );

    // form the return value
    //
    std::stringstream s;
    message.str(""); message << arg << " " << std::fixed << std::setprecision(3) << pos;
    retstring = message.str();
    logwrite( function, message.str() );

    message.str(""); message << "NOTICE:" << Focus::DAEMON_NAME << " " << retstring;
    this->async.enqueue( message.str() );

    return( error );
  }
  /***** Focus::Interface::get ************************************************/


  /***** Focus::Interface::dothread_home **************************************/
  /**
   * @brief      threaded function to home a motor
   * @param[in]  iface   reference to interface object
   * @param[in]  name    reference to name of motor to home
   *
   * This is the work function to call home() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by functions
   * called in here are set in the thr_error class variable.
   *
   */
  void Interface::dothread_home( Focus::Interface &iface, std::string name ) {
    std::string function = "Focus::Interface::dothread_home";
    std::stringstream message;
    int axis=1;
    long error=NO_ERROR;
    int addr=-1;
    std::string reftype;

    try {
      addr    = iface.motormap.at(name).addr;
      reftype = iface.motormap.at(name).reftype;
    }
    catch ( const std::out_of_range &e ) {
      message.str(""); message << "ERROR: name \"" << name << "\" not in motormap: " << e.what();
      logwrite( function, message.str() );
      iface.thr_error.fetch_or( ERROR );         // preserve this error
      --iface.motors_running;                    // atomically decrement the number of motors waiting
      iface.cv.notify_all();                     // notify parent that I'm done
      return;
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread sending home_axis( " << addr << ", " << axis << ", neg )";
    logwrite( function, message.str() );
#endif

    // send the home command by calling home_axis()
    //
    iface.pi_mutex.lock();
    iface.pi.home_axis( addr, axis, reftype );
    iface.pi_mutex.unlock();
    iface.motormap[name].ishome   = false;
    iface.motormap[name].ontarget = false;

    // Loop sending the is_home command until homed or timeout.
    //

    // get the time now for timeout purposes
    //
    std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

    bool is_home=false;

    do {
      bool state;
      iface.pi_mutex.lock();
      iface.pi.is_home( addr, axis, state );
      iface.pi_mutex.unlock();
      iface.motormap[name].ishome = state;
      iface.motormap[name].ontarget = state;
      is_home = iface.motormap[name].ishome;

      if ( is_home ) break;
      else {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] waiting for homing " << name << " addr " << addr << " ..." ;
        logwrite( function, message.str() );
#endif
        std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
      }

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > HOME_TIMEOUT ) {
        message.str(""); message << "TIMEOUT waiting for homing " << name << " addr " << addr;
        logwrite( function, message.str() );
        error = TIMEOUT;
        break;
      }

    } while ( 1 );

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread completed  homing " << name << " addr " << addr
                             << " with error=" << error;
    logwrite( function, message.str() );
#endif

    iface.thr_error.fetch_or( error );           // preserve any error returned

    --iface.motors_running;                      // atomically decrement the number of motors waiting

    iface.cv.notify_all();                       // notify parent that I'm done

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread completed home_axis( " << addr << ", " << axis << ", neg )"
                             << " with error=" << error;
    logwrite( function, message.str() );
#endif

    return;
  }
  /***** Focus::Interface::dothread_home **************************************/


  /***** Focus::Interface::dothread_move_abs **********************************/
  /**
   * @brief      threaded move_abs function
   * @param[in]  iface   reference to interface object
   * @param[in]  addr    controller address
   * @param[in]  pos     motor position
   *
   * This is the work function to call move_abs() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by the move_abs()
   * function are set in the thr_error class variable.
   *
   */
  void Interface::dothread_move_abs( Focus::Interface &iface, int addr, float pos ) {
    std::string function = "Focus::Interface::dothread_move_abs";
    std::stringstream message;
    long error;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread sending mov_abs( " << addr << ", " << pos << " )";
    logwrite( function, message.str() );
#endif

    error = iface.move_abs( addr, pos ); // send the move_abs command here

    iface.thr_error.fetch_or( error );   // preserve any error returned

    --iface.motors_running;              // atomically decrement the number of motors waiting

    iface.cv.notify_all();               // notify parent that I'm done

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread completed mov_abs( " << addr << ", " << pos << " ) "
                             << " *** motors_running = "<< iface.motors_running;
    logwrite( function, message.str() );
#endif

    return;
  }
  /***** Focus::Interface::dothread_move_abs **********************************/


  /***** Focus::Interface::move_abs *******************************************/
  /**
   * @brief      send move-absolute command to specified controller
   * @param[in]  addr  controller address
   * @param[in]  pos   motor position
   * @return     ERROR or NO_ERROR
   *
   * This could be called by a thread, so hardware interactions with the PI
   * controller are protected by a mutex.
   *
   */
  long Interface::move_abs( int addr, float pos ) {
    std::string function = "Focus::Interface::move_abs";
    std::stringstream message;
    long error=NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    try {
      int axis=1;

      // send the move command
      //
      this->pi_mutex.lock();
      error = this->pi.move_abs( addr, axis, pos );
      this->pi_mutex.unlock();

      // which controller has this addr?
      //
      std::string myname;
      for ( auto &con : this->motormap ) {
        if ( con.second.addr = addr ) {
          myname = con.second.name;
          break;
        }
      }

      if ( myname.empty() ) {
        logwrite( function, "ERROR: no motor controllers defined" );
        return( ERROR );
      }

      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR moving " << myname << " to " << pos;
        logwrite( function, message.str() );
        return error;
      }

      // Loop sending the on_target command for this address
      // until on target or timeout.
      //

      // first get the time now for timeout purposes
      //
      std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

      do {
        bool state;
        this->pi_mutex.lock();
        error = this->pi.on_target( addr, axis, state );
        this->pi_mutex.unlock();
        this->motormap.at(myname).ontarget = state;
        
        if ( this->motormap.at(myname).ontarget ) break;
        else {
#ifdef LOGLEVEL_DEBUG
          message.str(""); message << "[DEBUG] waiting for " << this->motormap.at(myname).name;
          logwrite( function, message.str() );
#endif
          std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        }

        // get time now and check for timeout
        //
        std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

        if ( elapsed > MOVE_TIMEOUT ) {
          message.str(""); message << "TIMEOUT waiting for " << this->motormap.at(myname).name;
          logwrite( function, message.str() );
          error = TIMEOUT;
          break;
        }
      } while ( 1 );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "unable to convert one or more values: " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "one or more values out of range: " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch( std::exception &e ) {
      message.str(""); message << "ERROR: other exception: " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( error );
  }
  /***** Focus::Interface::move_abs *******************************************/


  /***** Focus::Interface::move_rel *******************************************/
  /**
   * @brief      send move-relative command to specified controllers
   * @param[in]  args  string containing addr and offset
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::move_rel( std::string args ) {
    std::string function = "Focus::Interface::move_rel";
    std::stringstream message;
    int axis=1;
    int addr;
    float pos;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR: bad number of tokens: " << tokens.size()
                               << ". expected 2 (addr pos)";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      addr = std::stoi( tokens.at(0) );
      pos  = std::stof( tokens.at(1) );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "unable to convert one or more values: " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "one or more values out of range: " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( this->pi.move_rel( addr, axis, pos ) );
  }
  /***** Focus::Interface::move_rel *******************************************/


  /***** Focus::Interface::stop ***********************************************/
  /**
   * @brief      send the stop-all-motion command to all controllers
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::stop( ) {
    std::string function = "Focus::Interface::stop";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // send the stop_motion command for each address in controller_info
    //
    for ( auto const &mot : this->motormap ) {
      this->pi.stop_motion( mot.second.addr );
    }

    return( NO_ERROR );
  }
  /***** Focus::Interface::stop ***********************************************/


  /***** Focus::Interface::send_command ***************************************/
  /**
   * @brief      writes the raw command as received to the master controller
   * @param[in]  cmd  command to send
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command that expects no reply.
   *
   */
  long Interface::send_command( std::string cmd ) {
    std::string function = "Focus::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( this->pi.send_command( cmd ) );
  }
  /***** Focus::Interface::send_command ***************************************/


  /***** Focus::Interface::send_command ***************************************/
  /**
   * @brief      writes the raw command to the master controller, reads back reply
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply received
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command and then reads a reply if that command contains
   * a question mark, "?".
   *
   */
  long Interface::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Focus::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( cmd.find( "?" ) != std::string::npos ) return( this->pi.send_command( cmd, retstring ) );
    else return( this->pi.send_command( cmd ) );
  }
  /***** Focus::Interface::send_command ***************************************/

}
