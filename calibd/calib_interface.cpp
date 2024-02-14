/**
 * @file    calib_interface.cpp
 * @brief   defines the Calib namespace Interface class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "calib_interface.h"

namespace Calib {


  /***** Calib::Motion::Motion ************************************************/
  /**
   * @brief      Motion class constructor
   *
   */
  Motion::Motion() {
    this->port=-1;
    this->numdev=0;
  }
  /***** Calib::Motion::Motion ************************************************/


  /***** Calib::Motion::configure_class ***************************************/
  /**
   * @brief      configures the class from configure_calibd()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Calib::Server::configure_calibd() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Motion::configure_class() {
    std::string function = "Calib::Motion::configure_class";
    std::stringstream message;
    long error = ERROR;

    if ( this->port < 0 || this->host.empty() ) {
      message.str(""); message << "ERROR: host (" << this->host << ") or port (" << this->port << ") invalid";
      logwrite( function, message.str() );
      return( ERROR );
    }

    Physik_Instrumente::Interface s( this->name, this->host, this->port );
    this->pi = s;

    this->numdev = this->motormap.size();

    if ( this->numdev == 2 ) {
      logwrite( function, "motion interface configured ok" );
      error = NO_ERROR;
    }
    else if ( this->numdev > 2 ) {
      message.str(""); message << "ERROR: too many motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }
    else if ( this->numdev == 1 ) {
      logwrite( function, "ERROR: only one motor controller was defined" );
      error = ERROR;
    }
    else if ( this->numdev < 1 ) {
      message.str(""); message << "ERROR: no motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    for ( auto const &mot : this->motormap ) {
      message.str(""); message << "[DEBUG] motion controller " << mot.first
                               << " addr=" << mot.second.addr;
      for ( auto const &pos : mot.second.posmap ) {
        message << " " << pos.second.posname << "=" << pos.second.position;
      }
      logwrite( function, message.str() );
    }
#endif

    return( error );
  }
  /***** Calib::Motion::configure_class ***************************************/


  /***** Calib::Motion::open **************************************************/
  /**
   * @fn         open
   * @brief      opens the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Motion::open() {
    std::string function = "Calib::Motion::open";
    std::stringstream message;
    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Calib::Server::configure_calibd).
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
    // enable the servo for each address in motormap
    //
    for ( auto const &mot : this->motormap ) {
      int axis=1;
      int errcode;
      error |= this->pi.get_error( mot.second.addr, errcode );     // read error to clear, don't care the value
      error |= this->pi.set_servo( mot.second.addr, axis, true );  // turn the servos on
    }

    return( error );
  }
  /***** Calib::Motion::open **************************************************/


  /***** Calib::Motion::close *************************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Motion::close( ) {
    std::string function = "Calib::Motion::close";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "not connected" );
      return( NO_ERROR );
    }

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Calib::Server::configure_calibd).
    //
    if ( !this->pi.is_initialized() ) {
      logwrite( function, "ERROR: pi interface not initialized" );
      return( ERROR );
    }

    return( this->pi.close() );
  }
  /***** Calib::Motion::close *************************************************/


  /***** Calib::Motion::home **************************************************/
  /**
   * @brief      home all calib actuators
   * @param[in]  arg   input arg, currently only needed to request help
   * @param[out] help  return string contains help message if requested
   * @return     ERROR or NO_ERROR
   *
   */
  long Motion::home( std::string arg, std::string &help ) {
    std::string function = "Calib::Motion::home";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      help = CALIBD_HOME;
      help.append( "\n" );
      help.append( "  homes all actuators simultaneously using the indicated references:\n" );
      help.append( "  " );
      for ( auto const &mot : this->motormap ) {
        help.append( mot.first );
        help.append( ":" );
        help.append( mot.second.reftype );
        help.append( " " );
      }
      help.append( "\n" );
      return( NO_ERROR );
    }

    // connection must be open
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    this->thr_error.store( NO_ERROR );                         // clear thread error state (threads can set this)

    std::unique_lock<std::mutex> wait_lock( this->wait_mtx );  // create a mutex object for waiting for threads

    // loop through every motor in the class
    //
    for ( auto mot : this->motormap ) {

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] spawning thread to home " << mot.first;
      logwrite( function, message.str() );
#endif

      // spawn a thread to perform the home
      //
      std::thread( dothread_home, std::ref( *this ), mot.first ).detach();
      this->motors_running++;
    }

    // Wait for the threads to finish
    //
    bool motors_homed = false;
    while ( this->motors_running != 0 ) {
      motors_homed = true;
      message.str(""); message << "waiting for " << this->motors_running << " motor"
                               << ( this->motors_running > 1 ? "s" : "" );
      logwrite( function, message.str() );
      this->cv.wait( wait_lock );
    }

    if ( motors_homed ) logwrite( function, "home complete" );

    // get any errors from the threads
    //
    error |= this->thr_error.load();

    return( error );
  }
  /***** Calib::Motion::home **************************************************/


  /***** Calib::Motion::dothread_home *****************************************/
  /**
   * @brief      threaded function to home and apply zeropos
   * @param[in]  motion  reference to interface object
   * @param[in]  name     name of motor to home
   *
   * This is the work function to call home() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by functions
   * called in here are set in the thr_error class variable.
   *
   */
  void Motion::dothread_home( Calib::Motion &motion, std::string name ) {
    std::string function = "Calib::Motion::dothread_home";
    std::stringstream message;
    int axis=1;
    long error=NO_ERROR;
    int addr = -1;
    std::string reftype;

    try {
      addr    = motion.motormap.at(name).addr;
      reftype = motion.motormap.at(name).reftype;
    }
    catch ( const std::out_of_range &e ) {
      message.str(""); message << "ERROR: name \"" << name << "\" not in motormap: " << e.what();
      logwrite( function, message.str() );
      motion.thr_error.fetch_or( ERROR );         // preserve this error
      --motion.motors_running;                    // atomically decrement the number of motors waiting
      motion.cv.notify_all();                     // notify parent that I'm done
      return;
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread sending home_axis( "
                             << addr << ", " << axis << ", " << reftype << " ) for " << name;
    logwrite( function, message.str() );
#endif

    // send the home command by calling home_axis()
    //
    motion.pi_mutex.lock();
    motion.pi.home_axis( addr, axis, reftype );
    motion.pi_mutex.unlock();
    motion.motormap[name].ishome   = false;
    motion.motormap[name].ontarget = false;

    // Loop sending the is_home command until homed or timeout.
    //

    // get the time now for timeout purposes
    //
    std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

    bool is_home=false;

    do {
      bool state;
      motion.pi_mutex.lock();
      motion.pi.is_home( addr, axis, state );
      motion.pi_mutex.unlock();
      motion.motormap[name].ishome = state;
      motion.motormap[name].ontarget = state;
      is_home = motion.motormap[name].ishome;

      if ( is_home ) break;
      else {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] waiting for homing " << name << " addr " << addr << " ..." ;
        logwrite( function, message.str() );
#endif
        usleep( 1000000 );
      }

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > MOVE_TIMEOUT ) {
        message.str(""); message << "TIMEOUT waiting for homing " << name << " addr " << addr;
        logwrite( function, message.str() );
        error = TIMEOUT;
        break;
      }

    } while ( 1 );

    motion.thr_error.fetch_or( error );           // preserve any error returned

    --motion.motors_running;                      // atomically decrement the number of motors waiting

    motion.cv.notify_all();                       // notify parent that I'm done

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread completed  homing " << name << " addr " << addr
                             << " with error=" << error;
    logwrite( function, message.str() );
#endif

    return;
  }
  /***** Calib::Motion::dothread_home *****************************************/


  /***** Calib::Motion::is_home ***********************************************/
  /**
   * @brief      are all calib actuators homed?
   * @param[out] retstring  contains "true" | "false"
   * @return     ERROR or NO_ERROR
   *
   */
  long Motion::is_home( std::string &retstring ) {
    std::string function = "Calib::Motion::is_home";
    std::stringstream message, homestream;
    long error = NO_ERROR;
    size_t num_home = 0;

    // Requires an open connection
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // loop through every motor in the class
    // OK to do them serially (instead of threads) because it's quick to query
    //
    for ( auto mot : this->motormap ) {

      error |= this->pi.is_home( mot.second.addr, 1, mot.second.ishome );
      homestream << mot.first << ":" << ( mot.second.ishome ? "true" : "false" );
      if ( mot.second.ishome ) num_home++;
    }

    // Set the retstring true or false, true only if all requested controllers are the same
    //
    if ( num_home == this->motormap.size() ) retstring = "true";
    else
    if ( num_home == 0 )                            retstring = "false";
    else

    // If not all are the same state then log that but report false
    //
    if ( num_home > 0 && num_home < this->motormap.size() ) {
      logwrite( function, homestream.str() );
      retstring = "false";
    }

    return( error );
  }
  /***** Calib::Motion::is_home ***********************************************/


  /***** Calib::Motion::set ***************************************************/
  /**
   * @brief      move and/or return status of an actuator
   * @details    Input list can be comma or space delimited, but there must be
   *             no space on either side of the equal sign. This function
   *             allows for all listed actuators to be set simultaneously,
   *             each in its own thread.
   * @param[in]  input      list of actuators with optional "=state" i.e. cover=open, door=close
   * @param[out] retstring  actuator state { OPEN | CLOSE }
   * @return     ERROR or NO_ERROR
   *
   */
  long Motion::set( std::string input, std::string &retstring ) {
    std::string function = "Calib::Motion::set";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( input == "?" ) {
      retstring = CALIBD_SET;
      retstring.append( " <actuator>=<posname> [ <actuator>=<posname> ]\n" );
      retstring.append( "  where <posname> is\n" );
      for ( auto const &mot : this->motormap ) {
        retstring.append( "                     { " );
        for ( auto const &pos : mot.second.posmap ) {
          retstring.append( pos.second.posname ); retstring.append( " " );
        }
        retstring.append( "} for <actuator> = " );
        retstring.append( mot.first );
        retstring.append( "\n" );
      }
      retstring.append( "  One or both actuators may be set simultaneously.\n" );
      retstring.append( "  There are no space between <actuator>=<posname>, e.g. set door=open\n" );
      return( NO_ERROR );
    }

    if ( ! this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    std::stringstream valid_names;  // list of validated actuator names used for getting position after move

    if ( ! this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    std::unique_lock<std::mutex> wait_lock( this->wait_mtx );  // create a mutex object for waiting for threads

    // Tokenize on comma to see if more than one actuator
    //
    std::vector<std::string> input_list;             // vector of each "actuator=state"
    Tokenize( input, input_list, " ," );

    // Iterate through each requested actuator
    //
    for ( auto actuator : input_list ) {

      std::string name, posname;

      // Tokenize each item in above vector on "=" to get
      // the actuator name and the desired posname.
      //
      std::vector<std::string> actuator_tokens;      // vector of individual actuator, posname for each actuator entry
      Tokenize( actuator, actuator_tokens, "=" );
      if ( actuator_tokens.size() != 2 ) {
        message.str(""); message << "ERROR: bad input \"" << actuator << "\". expected actuator=state";
        logwrite( function, message.str() );
        error = ERROR;
      }
      else {
        name    = actuator_tokens[0];
        posname = actuator_tokens[1];
      }

      // requested named actuator must have been defined
      //
      bool actuator_found = ( ( this->motormap.find( name ) != this->motormap.end() ) ? true : false );

      if ( !actuator_found ) {
        message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        error = ERROR;
      }

      // If the actuator was found then the
      // requested posname must be defined for that actuator.
      //
      bool posname_found = ( ( actuator_found &&
                               (this->motormap[name].posmap.find( posname ) != this->motormap[name].posmap.end()) )
                             ? true : false );

      if ( !posname_found ) {
        message.str(""); message << "ERROR: position \"" << posname << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        error = ERROR;
      }

      // If both are found then get the position from the posmap
      // and spawn a thread to move to that position.
      //
      if ( actuator_found && posname_found ) {
        valid_names << name << " ";
        float reqpos = this->motormap[name].posmap[posname].position;
        std::thread( dothread_move_abs, std::ref( *this ), name, reqpos).detach();
        this->motors_running++;
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] spawning thread to move " << name << " to " << posname;
        logwrite( function, message.str() );
#endif
      }
    }

    // wait for the threads to finish
    //
    bool motors_moved = false;
    while ( this->motors_running != 0 ) {
      motors_moved = true;
      message.str(""); message << "waiting for " << this->motors_running << " motor" << ( this->motors_running > 1 ? "s":"" );
      logwrite( function, message.str() );
      this->cv.wait( wait_lock );
    }

    if ( motors_moved ) logwrite( function, "calib motor moves complete" );

    // get any errors from the threads
    //
    error |= this->thr_error.load();

    // after all the moves, read and return the position(s)
    //
    error |= this->get( valid_names.str(), retstring );

    return( error );
  }
  /***** Calib::Motion::set ***************************************************/


  /***** Calib::Motion::dothread_move_abs *************************************/
  /**
   * @brief      threaded move_abs function
   * @param[in]  motion  reference to interface object
   * @param[in]  name    name of controller
   * @param[in]  pos     motor position
   *
   * This is the work function to call move_abs() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by the move_abs()
   * function are set in the thr_error class variable.
   *
   */
  void Motion::dothread_move_abs( Calib::Motion &motion, std::string name, float pos ) {
    std::string function = "Calib::Motion::dothread_move_abs";
    std::stringstream message;
    long error;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread sending mov_abs( " << name << ", " << pos << " )";
    logwrite( function, message.str() );
#endif

    error = motion.move_abs( name, pos ); // send the move_abs command here

    motion.thr_error.fetch_or( error );   // preserve any error returned

    --motion.motors_running;              // atomically decrement the number of motors waiting

    motion.cv.notify_all();               // notify parent that I'm done

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread completed mov_abs( " << name << ", " << pos << " ) "
                             << " *** motors_running = "<< motion.motors_running;
    logwrite( function, message.str() );
#endif

    return;
  }
  /***** Calib::Motion::dothread_move_abs *************************************/


  /***** Calib::Motion::move_abs **********************************************/
  /**
   * @brief      send move-absolute command to specified controllers
   * @param[in]  name  controller name
   * @param[in]  pos   motor position
   * @return     ERROR or NO_ERROR
   *
   * This could be called by a thread, so hardware interactions with the PI
   * controller are protected by a mutex.
   *
   */
  long Motion::move_abs( std::string name, float pos ) {
    std::string function = "Calib::Motion::move_abs";
    std::stringstream message;
    long error=NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( this->motormap.find( name ) == this->motormap.end() ) {
      message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
      logwrite( function, message.str() );
      return( ERROR );
    }

    int addr = this->motormap[ name ].addr;
    int axis = 1;

    // send the move command
    //
    this->pi_mutex.lock();
    error = this->pi.move_abs( addr, axis, pos );
    this->pi_mutex.unlock();

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
      this->motormap[ name ].ontarget = state;

      if ( this->motormap[ name ].ontarget ) break;
      else {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] waiting for cal " << name;
        logwrite( function, message.str() );
#endif
        usleep( 1000000 );
      }

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > MOVE_TIMEOUT ) {
        message.str(""); message << "TIMEOUT waiting for cal " << name;
        logwrite( function, message.str() );
        error = TIMEOUT;
        break;
      }
    } while ( 1 );

    return( error );
  }
  /***** Calib::Motion::move_abs **********************************************/


  /***** Calib::Motion::get ***************************************************/
  /**
   * @brief      get the position of the named actuator(s)
   * @param[in]  name_in    name of actuator(s), can be space-delimited list
   * @param[out] retstring  current position of actuator(s) { open | closed }
   * @return     ERROR or NO_ERROR
   *
   */
  long Motion::get( std::string name_in, std::string &retstring ) {
    std::string function = "Slit::Motion::get";
    std::stringstream message;
    std::stringstream retstream;
    long error = NO_ERROR;
    std::vector<std::string> name_list;

    // Help
    //
    if ( name_in == "?" ) {
      retstring = CALIBD_GET;
      retstring.append( " [ <actuator> ]\n" );
      retstring.append( "  where <actuator> is { " );
      for ( auto const &mot : this->motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "}\n" );
      retstring.append( "  If no arg is supplied then the position of both is returned.\n" );
      retstring.append( "  Supplying an actuator name returns the position of only the specified actuator.\n" );
      return( NO_ERROR );
    }

    if ( ! this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // If no name(s) supplied then create a vector of all defined actuator names
    //
    if ( name_in.empty() ) {
      for ( auto const &mot : this->motormap ) {
        name_list.push_back( mot.first );
      }
    }
    else {
      Tokenize( name_in, name_list, " " );  // otherwise create a vector of the supplied name(s)
    }

    for ( auto name : name_list ) {
      std::string thisposname;

      if ( this->motormap.find( name ) == this->motormap.end() ) {
        message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        thisposname = "error";
        error = ERROR;
      }
      else {
        // and then get the current position of this actuator.
        //
        int addr = this->motormap[ name ].addr;
        int axis = 1;
        float thispos = -1;
        float tolerance = 0.001;

        error = this->pi.get_pos( addr, axis, thispos );

        if ( error == ERROR ) {
          message.str(""); message << "ERROR reading position of actuator \"" << name << "\"";
          logwrite( function, message.str() );
          thisposname = "error";
        }
        // Check thispos against the positions in the posmap and if one
        // is found within tolerance then that is the current position.
        //
        else for ( auto const &pos : this->motormap[ name ].posmap ) {
          if ( std::abs( pos.second.position - thispos ) < tolerance ) {
            thisposname = pos.second.posname;
          }
        }
      }
      retstream << name << "=" << thisposname << " ";
    }

    retstring = retstream.str();

    return error;
  }
  /***** Calib::Motion::get ***************************************************/


  /***** Calib::Motion::send_command ******************************************/
  /**
   * @brief      writes the raw command as received to the master controller
   * @param[in]  cmd  command to send
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command that expects no reply.
   *
   */
  long Motion::send_command( std::string cmd ) {
    std::string function = "Calib::Motion::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( this->pi.send_command( cmd ) );
  }
  /***** Calib::Motion::send_command ******************************************/


  /***** Calib::Motion::send_command ******************************************/
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
  long Motion::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Calib::Motion::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( cmd.find( "?" ) != std::string::npos ) return( this->pi.send_command( cmd, retstring ) );
    else return( this->pi.send_command( cmd ) );
  }
  /***** Calib::Motion::send_command ******************************************/


  /***** Calib::Modulator::configure_host *************************************/
  /**
   * @brief      parse LAMPMOD_HOST key to configure the lamp modulator Arduino host
   * @param[in]  input  string should contain "<host> <port>"
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::configure_host( std::string input ) {
    std::string function = "Calib::Modulator::configure_host";
    std::stringstream message;
    std::vector<std::string> tokens;
    std::string tryhost;
    int tryport=-1;

    // In case the Arduino has already been configured then
    // make sure the connection is closed.
    //
    if ( this->arduino != nullptr && this->arduino->sock.isconnected() ) {
      message.str(""); message << "closing existing connection to " << this->arduino->get_name() << " "
                               << this->arduino->get_host() << ":" << this->arduino->get_port();
      logwrite( function, message.str() );
      this->arduino->close();
    }

    // Extract the host and port from the input string
    //
    Tokenize( input, tokens, " " );

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR bad number of parameters in \"" << input
                               << "\": expected 2 but received " << tokens.size();
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      tryhost = tokens.at(0);
      tryport = std::stoi( tokens.at(1) );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Check that (potentially) valid values have been extracted
    //
    if ( tryport < 1 ) {
      message.str(""); message << "ERROR port " << tryport << " must be greater than 0";
      logwrite( function, message.str() );
      return( ERROR );
    }
    if ( tryhost.empty() ) {
      logwrite( function, "ERROR host cannot be empty" );
      return( ERROR );
    }

    // Configure the modulator arduino pointer
    //
    this->arduino = std::make_unique<Network::Interface>("modulator", tryhost, tryport, '\n', '\n');

    return( NO_ERROR );

  }
  /***** Calib::Modulator::configure_host *************************************/


  /***** Calib::Modulator::configure_mod **************************************/
  /**
   * @brief      parse LAMPMOD_MOD key to configure the lamp modulators
   * @param[in]  input  string should contain 4 args, "<num> <name> <D> <T>"
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::configure_mod( std::string input ) {
    std::string function = "Calib::Modulator::configure_mod";
    std::stringstream message;
    std::vector<std::string> tokens;
    std::string tryname;
    int trynum=-1;
    double trydc=-1;
    double tryperiod=-1;

    // Extract the host and port from the input string
    //
    Tokenize( input, tokens, " " );

    if ( tokens.size() != 4 ) {
      message.str(""); message << "ERROR bad number of parameters in \"" << input
                               << "\": expected 4 but received " << tokens.size();
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      trynum    = std::stoi( tokens.at(0) );
      tryname   = tokens.at(1);
      trydc     = std::stod( tokens.at(2) );
      tryperiod = std::stod( tokens.at(3) );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Check that (potentially) valid values have been extracted
    //
    if ( trynum < 1 || trynum > MOD_MAX ) {
      message.str(""); message << "ERROR num " << trynum << " outside range {1:" << MOD_MAX << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }
    if ( tryname.empty() ) {
      logwrite( function, "ERROR name cannot be empty" );
      return( ERROR );
    }
    if ( trydc < 0.0 || trydc > 1.0 ) {
      message.str(""); message << "ERROR duty cycle " << trydc << " outside range {0:1}";
      logwrite( function, message.str() );
      return( ERROR );
    }
    if ( tryperiod != 0 && ( tryperiod < 50.0 || tryperiod > 3600000.0 ) ) {
      message.str(""); message << "ERROR period " << tryperiod << " outside range {0,50:3600000}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // By this point, all values are valid so save them in the class

    this->mod_nums.push_back( trynum );                        // used to validate modulator number <n>

    this->mod_map[ tryname ] = { trynum, trydc, tryperiod };   // used to set defaults

    return( NO_ERROR );

  }
  /***** Calib::Modulator::configure_mod **************************************/


  /***** Calib::Modulator::control ********************************************/
  /**
   * @brief      lamp modulator control main parser
   * @param[in]  args       input arg string
   * @param[out] retstring  return string
   * @return     ERROR or NO_ERROR
   * @details    Provides lamp modulator control to set and/or get settings.
   *             Input args are:
   *             ? | open | close | reconnect | default | <n> [ [ on|off ] | [ <D> <T> ] ]
   *
   * In other words,
   *             control              : invalid (returns error)
   *             control ?            : help
   *             control open         : open connection to Arduino
   *             control close        : close connection to Arduino
   *             control reconnect    : close, open connection to Arduino
   *             control default      : set all modulators as defined in config file
   *             control <n>          : get status of modulator n
   *             control <n> on|off   : set power state only for modulator <n>
   *             control <n> <D> <T>  : set <D> and <T> for modulator <n>
   *
   * where <n> = {0:MOD_MAX} and n=0 means all modulators {1:MOD_MAX}
   *       <D> = fractional duty cycle {0:1}
   *       <T> = period in msec {0,50:3600000} where 0=off
   *
   */
  long Modulator::control( std::string args, std::string &retstring ) {
    std::string function = "Calib::Modulator::control";
    std::stringstream message;
    long error=NO_ERROR;

    if ( args.empty() ) {
      logwrite( function, "ERROR missing args" );
      retstring="invalid_argument";
      return( ERROR );
    }

    if ( args == "?" ) {
      retstring = CALIBD_LAMPMOD;
      retstring.append( " ? | open | close | reconnect | <n> [ [ on|off ] | [ <D> <T> ] ]\n" );
      retstring.append( "  Lamp modulator control\n" );
      retstring.append( "  open          :  open connection to controller\n" );
      retstring.append( "  close         :  close connection to controller\n" );
      retstring.append( "  reconnect     :  close, open connection to controller\n" );
      retstring.append( "  default       :  set all modulators as defined in config file\n" );
      retstring.append( "  <n>           :  get status of modulator <n>\n" );
      retstring.append( "  <n> on | off  :  set power-only for modulator <n>\n" );
      retstring.append( "  <n> <D> <T>   :  set <D> and <T> for modulator <n>\n" );
      retstring.append( "\n" );
      message.str(""); message << "  <n> = {0:" << MOD_MAX << "} and n=0 means all modulators {1:" << MOD_MAX << "}\n";
      retstring.append( message.str() );
      retstring.append( "  <D> = fractional duty cycle {0:1}\n" );
      retstring.append( "  <T> = period in msec {0,50:3600000} where 0=off\n" );
      retstring.append( "\n" );
      retstring.append( "  modulators open every <T> msec and close every <T> + ( <D> * <T> ) msec\n" );
      return( NO_ERROR );
    }

    // Anything after this requires at least arduino to be configured
    //
    if ( ( this->arduino == nullptr ) ||
       ( ( this->arduino != nullptr ) && ( !this->arduino->is_initialized() ) ) ) {
      logwrite( function, "ERROR Arduino not initialized (check config)" );
      retstring="invalid_configuration";
      return( ERROR );
    }

    // If args is one of these simple strings then perform one of
    // the following actions and immediately return. Since these
    // have their own entry points (apart from this function) they
    // each perform their own checks of the arduino state.
    //
    if ( args == "open" ) return( this->open_arduino() );         // open connection to controller

    if ( args == "close" ) return( this->close_arduino() );       // close connection to controller

    if ( args == "reconnect" ) return( this->reopen_arduino() );  // close, open connection to controller

    if ( args == "default" ) return( this->set_defaults() );      // set defaults from config file

    // Cannot proceed unless a connection is open to the Arduino.
    // (The above calls each have their own internal checks.)
    //
    if ( !this->arduino->isopen() ) {
      logwrite( function, "ERROR no connection open to Arduino" );
      retstring="not_connected";
      return( ERROR );
    }

    // Everything else requires tokenizing the args.
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    // Checking tokens size here means not having to catch out_of_range exception
    //
    if ( tokens.size() < 1 || tokens.size() > 3 ) {
      message.str(""); message << "ERROR: expected {1,2,3} arguments but received " << tokens.size();
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return( ERROR );
    }

    // Since token size was already checked, we are guaranteed to have
    // at least one token here. The first one, tokens[0], is always <n>.
    // The range of <n> is checked first, here, against those that were
    // configured, so that no other function needs to do this.
    //
    int n=-1;

    try {
      n = std::stoi( tokens[0] );

      // check the requested mod number <n> against those configured (and 0)
      //
      auto foundn = std::find( this->mod_nums.begin(), this->mod_nums.end(), n );
      if ( n != 0 && foundn==this->mod_nums.end() ) {
       message.str(""); message << "ERROR invalid mod number " << n << ". expected { 0 ";
       for ( auto num : this->mod_nums ) message << num << " ";
       message << "} (check config)";
       retstring="invalid_argument";
       return( ERROR );
      }
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR invalid argument \"" << tokens[0] << "\" : " << e.what()
                               << ": expected integer {0:" << MOD_MAX << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Only one of the following versions of control() will be called,
    // for 1, 2, or 3 args. Then return the status of that call.

    // One arg gets status of modulator <n>,
    //
    if ( tokens.size() == 1 ) {  // <n>
      error = control( n, retstring );
    }
    else

    // or two args sets power for modulator <n>,
    //
    if ( tokens.size() == 2 ) {  // <n> on|off
      int power=-1;
      if ( tokens[1] == "on" )  { power=1; }
      else
      if ( tokens[1] == "off" ) { power=0; }
      else {
        message.str(""); message << "ERROR invalid power state \"" << tokens[1] << "\". expected {on|off}";
        logwrite( function, message.str() );
        return( ERROR );
      }
      error = control( n, power );
    }
    else

    // or three args sets <D> and <T> for modulator <n>.
    //
    if ( tokens.size() == 3 ) {  // <n> <D> <T>
      int D=-1, T=-1;
      try {
        D = std::stod( tokens[1] );
        T = std::stod( tokens[2] );
      }
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR invalid argument in \"" << tokens[1] << " " << tokens[2] << "\" : " << e.what()
                                 << ": expected floats";
        logwrite( function, message.str() );
        return( ERROR );
      }
      error = control( n, D, T );
    }

    retstring = ( error==NO_ERROR ? args : "arduino_error" );

    return( error );
  }
  /***** Calib::Modulator::control ********************************************/


  /***** Calib::Modulator::control ********************************************/
  /**
   * @brief      lamp modulator control return status
   * @param[in]  num     modulator number
   * @param[out] status  return string
   * @return     ERROR or NO_ERROR
   * @details    This function is overloaded.
   *             This version returns the status of modulator <n>
   *
   */
  long Modulator::control( int num, std::string &status ) {
    std::string function = "Calib::Modulator::control";
    std::stringstream message, retstream;
    long error=NO_ERROR;
    int pow;
    double dut;
    double per;

    if ( num==0 ) {  // read all configured modulators
      for ( auto modnum : this->mod_nums ) {
        error |= this->status( modnum, dut, per, pow );
        retstream << modnum << "," << dut << "," << per << " [" << ( pow ? "on" : "off" ) << "]\n";
      }
    }
    else {           // read the specific modulator
      error = this->status( num, dut, per, pow );
      retstream << num << "," << dut << "," << per << " [" << ( pow ? "on" : "off" ) << "]\n";
    }

    status = retstream.str();

    return( error );
  }
  /***** Calib::Modulator::control ********************************************/


  /***** Calib::Modulator::control ********************************************/
  /**
   * @brief      lamp modulator control set power
   * @param[in]  num  modulator number
   * @param[in]  pow  requested power state, 1=on, 0=off
   * @return     ERROR or NO_ERROR
   * @details    This function is overloaded.
   *             This version sets the power for modulator <n>.
   *
   */
  long Modulator::control( int num, int pow ) {
    std::string function = "Calib::Modulator::control";
    std::stringstream message;
    long error=NO_ERROR;

    if ( num==0 ) {  // set all configured modulators
      for ( auto modnum : this->mod_nums ) error |= this->power( modnum, pow );
    }
    else {           // set the specific modulator
      error = this->power( num, pow );
    }

    return( error );
  }
  /***** Calib::Modulator::control ********************************************/


  /***** Calib::Modulator::control ********************************************/
  /**
   * @brief      lamp modulator control set D and T
   * @param[in]  num     modulator number
   * @param[in]  dut     duty cycle (D)
   * @param[in]  per     period (T)
   * @return     ERROR or NO_ERROR
   * @details    This function is overloaded.
   *             This version sets the duty cycle and period for modulator <n>.
   *
   */
  long Modulator::control( int num, double dut, double per ) {
    std::string function = "Calib::Modulator::control";
    std::stringstream message;
    long error=NO_ERROR;

    if ( num==0 ) {  // set all configured modulators
      for ( auto modnum : this->mod_nums ) error |= this->mod( modnum, dut, per );
    }
    else {           // set the specific modulator
      error = this->mod( num, dut, per );
    }

    return( error );
  }
  /***** Calib::Modulator::control ********************************************/


  /***** Calib::Modulator::set_defaults ***************************************/
  /**
   * @brief      set all modulators as defined in the config file
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::set_defaults() {
    std::string function = "Calib::Modulator::set_defaults";
    std::stringstream message;
    long error=NO_ERROR;

    // Cannot proceed unless a connection is open to the Arduino.
    //
    if ( !this->arduino->isopen() ) {
      logwrite( function, "ERROR no connection open to Arduino" );
      return( ERROR );
    }

    // Set all configured modulators
    //
    for ( auto const &modit : this->mod_map ) {
      if ( this->mod( modit.second.num, modit.second.dut, modit.second.per ) != NO_ERROR ) {
        message.str(""); message << "ERROR setting modulator " << modit.second.num;
        logwrite( function, message.str() );
        error = ERROR;
      }
    }

    return( error );
  }
  /***** Calib::Modulator::set_defaults ***************************************/


  /***** Calib::Modulator::open_arduino ***************************************/
  /**
   * @brief      open connection to Arduino
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::open_arduino() {
    std::string function = "Calib::Modulator::open_arduino";
    std::stringstream message;

    // Try to open arduino. Network interface handles case if already open.
    //
    if ( this->arduino->open() == ERROR ) {
      message.str(""); message << "ERROR opening " << this->arduino->get_name();
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** Calib::Modulator::open_arduino ***************************************/


  /***** Calib::Modulator::close_arduino **************************************/
  /**
   * @brief      close connection to Arduino
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::close_arduino() {
    std::string function = "Calib::Modulator::close_arduino";
    std::stringstream message;

    // Try to close arduino. Network interface handles case if already closed.
    //
    if ( this->arduino->close() == ERROR ) {
      message.str(""); message << "ERROR closing " << this->arduino->get_name();
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** Calib::Modulator::close_arduino **************************************/


  /***** Calib::Modulator::reopen_arduino *************************************/
  /**
   * @brief      close, then open connection to Arduino
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::reopen_arduino() {
    long error=NO_ERROR;
    if ( error==NO_ERROR ) error = this->close_arduino();
    if ( error==NO_ERROR ) error = this->open_arduino();
    return( error );
  }
  /***** Calib::Modulator::reopen_arduino *************************************/


  /***** Calib::Modulator::mod ************************************************/
  /**
   * @brief      send command to change duty cycle and period of requested modulator
   * @param[in]  num  modulator number
   * @param[in]  dut  duty cycle
   * @param[in]  per  period
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::mod( int num, double dut, double per ) {
    std::string function = "Calib::Modulator::mod";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    // First set the duty cycle and period

    // Form and send command to Arduino: "mod,<num>,<dut>,<per>"
    // Expect to get back "<dut>,<per>"
    //
    cmd.str(""); cmd << "mod," << num << "," << dut << "," << per;

    if ( this->arduino->send_command( cmd.str(), reply ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Tokenize the reply. Should get back what was written.
    //
    std::vector<std::string> tokens;
    Tokenize( reply, tokens, "," );

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR: malformed reply \"" << reply
                               << "\" for modulator " << num << ". expected <dut>,<per>";
      logwrite( function, message.str() );
      return( ERROR );
    }

    double dut_ret=-1, per_ret=-1;

    try {
      dut_ret = std::stod( tokens[0] );
      per_ret = std::stod( tokens[1] );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR bad return value in \"" << reply << "\" for modulator " << num
                               << ": " << e.what() << ": expected floats for <dut>,<per>";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Make sure the returned values match the values that were requested
    //
    if ( dut_ret != dut || per_ret != per ) {
      message.str(""); message << "ERROR: modulator " << num << " returned " << dut_ret << "," << per_ret
                               << " doesn't match requested " << dut << "," << per;
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Finally, set the power state based on the duty cycle and period:
    // if either duty cycle or period are zero, then the power is set off.

    int pow;

    if ( dut==0 || per==0 ) pow=0; else pow=1;

    // Form and send command to Arduino: "power,<num>,<pow>"
    //
    cmd.str(""); cmd << "power," << num << "," << pow;

    if ( this->arduino->send_command( cmd.str() ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str(""); message << "modulator " << num << " D,T set to " << dut << "," << per << " ["
                             << ( pow ? "on" : "off" ) << "]";
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /***** Calib::Modulator::mod ************************************************/


  /***** Calib::Modulator::power **********************************************/
  /**
   * @brief      send command to change power state of requested modulator
   * @param[in]  num  modulator number
   * @param[in]  pow  power state
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::power( int num, int pow ) {
    std::string function = "Calib::Modulator::power";
    std::stringstream message;
    std::stringstream cmd;

    // Form and send command to Arduino: "power,<num>,<pow>".
    // There is no reply.
    //
    cmd.str(""); cmd << "power," << num << "," << pow;

    if ( this->arduino->send_command( cmd.str() ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str(""); message << "modulator " << num << " set to [" << ( pow ? "on" : "off" ) << "]";
    logwrite( function, message.str() );
    return( NO_ERROR );
  }
  /***** Calib::Modulator::power **********************************************/


  /***** Calib::Modulator::status *********************************************/
  /**
   * @brief      send command to read status of requested modulator
   * @param[in]  num  modulator number
   * @param[out] dut  reference to duty cycle
   * @param[out] per  reference to period
   * @param[out] pow  reference to power state
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::status( int num, double &dut, double &per, int &pow ) {
    std::string function = "Calib::Modulator::status";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    long error = NO_ERROR;

    // First ask the power state

    // Form and send command to Arduino: "power,<num>"
    // Expect to get back "<pow>"
    //
    cmd.str(""); cmd << "power," << num;

    if ( this->arduino->send_command( cmd.str(), reply ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      pow = std::stoi( reply );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR bad return value in \"" << reply << "\" for modulator " << num
                               << ": " << e.what() << ": expected integer for <pow>";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Next ask the dut,per

    // Form and send command to Arduino: "mod,<num>"
    // Expect to get back "<dut>,<per>"
    //
    cmd.str(""); cmd << "mod," << num;

    if ( this->arduino->send_command( cmd.str(), reply ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    std::vector<std::string> tokens;
    Tokenize( reply, tokens, "," );

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR: malformed reply \"" << reply
                               << "\" for modulator " << num << ". expected <dut>,<per>";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      dut = std::stod( tokens[0] );
      per = std::stod( tokens[1] );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR bad return value in \"" << reply << "\" for modulator " << num
                               << ": " << e.what() << ": expected floats for <dut>,<per>";
      logwrite( function, message.str() );
      error = ERROR;
    }

    return( error );
  }
  /***** Calib::Modulator::status *********************************************/

}
