/**
 * @file    calib_interface.cpp
 * @brief   defines the Calib namespace Interface class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "calib_interface.h"

namespace Calib {


  /***** Calib::Interface::Interface ******************************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /***** Calib::Interface::Interface ******************************************/


  /***** Calib::Interface::~Interface *****************************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /***** Calib::Interface::~Interface *****************************************/


  /***** Calib::Interface::initialize_class ***********************************/
  /**
   * @brief      initializes the class from configure_calibd()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Calib::Server::configure_calibd() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Calib::Interface::initialize_class";
    std::stringstream message;
    long error = ERROR;

    if ( this->port < 0 || this->host.empty() ) {
      message.str(""); message << "ERROR: host (" << this->host << ") or port (" << this->port << ") invalid";
      logwrite( function, message.str() );
      return( ERROR );
    }

    Physik_Instrumente::ServoInterface s( this->name, this->host, this->port );
    this->pi = s;

    this->numdev = this->controller_info.size();

    if ( this->numdev == 2 ) {
      logwrite( function, "interface initialized ok" );
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
    for ( auto const& con : this->controller_info ) {
      message.str(""); message << "[DEBUG] controller " << con.first
                               << " addr=" << con.second.addr
                               << " openpos=" << con.second.openpos
                               << " closepos=" << con.second.closepos;
      logwrite( function, message.str() );
    }
#endif

    return( error );
  }
  /***** Calib::Interface::initialize_class ***********************************/


  /***** Calib::Interface::open ***********************************************/
  /**
   * @fn         open
   * @brief      opens the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    std::string function = "Calib::Interface::open";
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
    // enable the servo for each address in controller_info
    //
    for ( auto const& con : this->controller_info ) {
      int axis=1;
      int errcode;
      error |= this->pi.get_error( con.second.addr, errcode );     // read error to clear, don't care the value
      error |= this->pi.set_servo( con.second.addr, axis, true );  // turn the servos on
    }

    return( error );
  }
  /***** Calib::Interface::open ***********************************************/


  /***** Calib::Interface::close **********************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    std::string function = "Calib::Interface::close";
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
  /***** Calib::Interface::close **********************************************/


  /***** Calib::Interface::home ***********************************************/
  /**
   * @brief      home all calib actuators
   * @param[in]  arg   input arg, currently only needed to request help
   * @param[out] help  return string contains help message if requested
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::home( std::string arg, std::string &help ) {
    std::string function = "Calib::Interface::home";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      help = CALIBD_HOME;
      help.append( "\n" );
      help.append( "  homes all actuators simultaneously using the indicated references:\n" );
      help.append( "  " );
      for ( auto const& con : this->controller_info ) {
        help.append( con.first );
        help.append( ":" );
        help.append( con.second.reftype );
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
    for ( auto mot : this->controller_info ) {

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
  /***** Calib::Interface::home ***********************************************/


  /***** Calib::Interface::dothread_home **************************************/
  /**
   * @brief      threaded function to home and apply zeropos
   * @param[in]  iface  reference to interface object
   * @param[in]  name   name of motor to home
   *
   * This is the work function to call home() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by functions
   * called in here are set in the thr_error class variable.
   *
   */
  void Interface::dothread_home( Calib::Interface &iface, std::string name ) {
    std::string function = "Calib::Interface::dothread_home";
    std::stringstream message;
    int axis=1;
    int addr = iface.controller_info[name].addr;
    std::string reftype = iface.controller_info[name].reftype;
    long error=NO_ERROR;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread sending home_axis( "
                             << addr << ", " << axis << ", " << reftype << " ) for " << name;
    logwrite( function, message.str() );
#endif

    // send the home command by calling home_axis()
    //
    iface.pi_mutex.lock();
    iface.pi.home_axis( addr, axis, reftype );
    iface.pi_mutex.unlock();
    iface.controller_info[name].ishome   = false;
    iface.controller_info[name].ontarget = false;

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
      iface.controller_info[name].ishome = state;
      iface.controller_info[name].ontarget = state;
      is_home = iface.controller_info[name].ishome;

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

    iface.thr_error.fetch_or( error );           // preserve any error returned

    --iface.motors_running;                      // atomically decrement the number of motors waiting

    iface.cv.notify_all();                       // notify parent that I'm done

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread completed  homing " << name << " addr " << addr
                             << " with error=" << error;
    logwrite( function, message.str() );
#endif

    return;
  }
  /***** Calib::Interface::dothread_home **************************************/


  /***** Calib::Interface::is_home ********************************************/
  /**
   * @brief      are all calib actuators homed?
   * @param[out] retstring  contains "true" | "false"
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::is_home( std::string &retstring ) {
    std::string function = "Calib::Interface::is_home";
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
    for ( auto mot : this->controller_info ) {

      error |= this->pi.is_home( mot.second.addr, 1, mot.second.ishome );
      homestream << mot.first << ":" << ( mot.second.ishome ? "true" : "false" );
      if ( mot.second.ishome ) num_home++;
    }

    // Set the retstring true or false, true only if all requested controllers are the same
    //
    if ( num_home == this->controller_info.size() ) retstring = "true";
    else
    if ( num_home == 0 )                            retstring = "false";
    else

    // If not all are the same state then log that but report false
    //
    if ( num_home > 0 && num_home < this->controller_info.size() ) {
      logwrite( function, homestream.str() );
      retstring = "false";
    }

    return( error );
  }
  /***** Calib::Interface::is_home ********************************************/


  /***** Calib::Interface::set ************************************************/
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
  long Interface::set( std::string input, std::string &retstring ) {
    std::string function = "Calib::Interface::set";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( input == "?" ) {
      retstring = CALIBD_SET;
      retstring.append( " <actuator>=<action> [ <actuator>=<action> ]\n" );
      retstring.append( "  where <actuator> is { " );
      for ( auto const& con : this->controller_info ) { retstring.append( con.first ); retstring.append( " " ); }
      retstring.append( "}\n" );
      retstring.append( "  and <action> is { open | close }.\n" );
      retstring.append( "  One or both actuators may be set simultaneously.\n" );
      retstring.append( "  There are no space between <actuator>=<action>, e.g. door=open\n" );
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

      std::string name, action;

      // Tokenize each item in above vector on "=" to get
      // the actuator name and the desired action.
      //
      std::vector<std::string> actuator_tokens;      // vector of individual actuator, action for each actuator entry
      Tokenize( actuator, actuator_tokens, "=" );
      if ( actuator_tokens.size() != 2 ) {
        message.str(""); message << "ERROR: bad input \"" << actuator << "\". expected actuator=state";
        logwrite( function, message.str() );
        error = ERROR;
      }
      else {
        name   = actuator_tokens[0];
        action = actuator_tokens[1];
      }

      // requested named actuator must have been defined
      //
      auto actuator_found = this->controller_info.find( name );

      float openpos, closepos, reqpos;

      if ( error==NO_ERROR && actuator_found == this->controller_info.end() ) {
        message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        error = ERROR;
      }
      else {
        openpos  = this->controller_info[ name ].openpos;
        closepos = this->controller_info[ name ].closepos;
        valid_names << name << " ";
      }

      // if an action is requested ( open | close ) then perform that action
      //
      if ( error==NO_ERROR && ! action.empty() ) {
        try {
          std::transform( action.begin(), action.end(), action.begin(), ::tolower );  // convert to lowercase
        }
        catch (...) {
          logwrite( function, "ERROR converting action to lowercase" );
          error = ERROR;
        }

        if ( error==NO_ERROR && action == "open" ) reqpos = openpos;
        else
        if ( error==NO_ERROR && action == "close" ) reqpos = closepos;
        else {
          message.str(""); message << "ERROR: undefined action \"" << action << "\". Expected { open | close }.";
          logwrite( function, message.str() );
          error = ERROR;
        }

        // Spawn a thread to perform the actual move
        //
        if ( error==NO_ERROR ) {
          std::thread( dothread_move_abs, std::ref( *this ), name, reqpos).detach();
          this->motors_running++;

#ifdef LOGLEVEL_DEBUG
          message.str(""); message << "[DEBUG] spawning thread to " << action << " " << name;
          logwrite( function, message.str() );
#endif
        }
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
  /***** Calib::Interface::set ************************************************/


  /***** Calib::Interface::dothread_move_abs **********************************/
  /**
   * @brief      threaded move_abs function
   * @param[in]  iface   reference to interface object
   * @param[in]  name    name of controller
   * @param[in]  pos     motor position
   *
   * This is the work function to call move_abs() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by the move_abs()
   * function are set in the thr_error class variable.
   *
   */
  void Interface::dothread_move_abs( Calib::Interface &iface, std::string name, float pos ) {
    std::string function = "Calib::Interface::dothread_move_abs";
    std::stringstream message;
    long error;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread sending mov_abs( " << name << ", " << pos << " )";
    logwrite( function, message.str() );
#endif

    error = iface.move_abs( name, pos ); // send the move_abs command here

    iface.thr_error.fetch_or( error );   // preserve any error returned

    --iface.motors_running;              // atomically decrement the number of motors waiting

    iface.cv.notify_all();               // notify parent that I'm done

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread completed mov_abs( " << name << ", " << pos << " ) "
                             << " *** motors_running = "<< iface.motors_running;
    logwrite( function, message.str() );
#endif

    return;
  }
  /***** Calib::Interface::dothread_move_abs **********************************/


  /***** Calib::Interface::move_abs *******************************************/
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
  long Interface::move_abs( std::string name, float pos ) {
    std::string function = "Calib::Interface::move_abs";
    std::stringstream message;
    long error=NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( this->controller_info.find( name ) == this->controller_info.end() ) {
      message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
      logwrite( function, message.str() );
      return( ERROR );
    }

    int addr = this->controller_info[ name ].addr;
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
      this->controller_info[ name ].ontarget = state;

      if ( this->controller_info[ name ].ontarget ) break;
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
  /***** Calib::Interface::move_abs *******************************************/


  /***** Calib::Interface::get ************************************************/
  /**
   * @brief      get the state of the named actuator(s)
   * @param[in]  name_in    name of actuator(s), can be space-delimited list
   * @param[out] retstring  current state of actuator(s) { open | closed }
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get( std::string name_in, std::string &retstring ) {
    std::string function = "Slit::Interface::get";
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
      for ( auto const& con : this->controller_info ) { retstring.append( con.first ); retstring.append( " " ); }
      retstring.append( "}\n" );
      retstring.append( "  If no arg is supplied then the state of both is returned.\n" );
      retstring.append( "  Supplying an actuator name returns the state of only the specified actuator.\n" );
      return( NO_ERROR );
    }

    if ( ! this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // If no name(s) supplied then create a vector of all defined actuator names
    //
    if ( name_in.empty() ) {
      for ( auto const& con : this->controller_info ) {
        name_list.push_back( con.first );
      }
    }
    else {
      Tokenize( name_in, name_list, " " );  // otherwise create a vector of the supplied name(s)
    }

    for ( auto name : name_list ) {
      std::string state;

      if ( this->controller_info.find( name ) == this->controller_info.end() ) {
        message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        state = "error";
        error = ERROR;
      }
      else {
        int addr = this->controller_info[ name ].addr;
        int axis = 1;
        float openpos = this->controller_info[ name ].openpos;
        float closepos = this->controller_info[ name ].closepos;
        float pos=-1;

        // and then get the current position of this actuator.
        //
        error = this->pi.get_pos( addr, axis, pos );

        if ( std::abs( pos - openpos ) < 0.001 ) state = "open";
        else
        if ( std::abs( pos - closepos ) < 0.001 ) state = "closed";
        else {
          message.str(""); message << "ERROR: actuator \"" << name << "\" not in known position: " << pos;
          logwrite( function, message.str() );
          state = "error";
          error = ERROR;
        }
      }
      retstream << name << "=" << state << " ";
    }

    retstring = retstream.str();

    return error;
  }
  /***** Calib::Interface::get ************************************************/


  /***** Calib::Interface::send_command ***************************************/
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
    std::string function = "Calib::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( this->pi.send_command( cmd ) );
  }
  /***** Calib::Interface::send_command ***************************************/


  /***** Calib::Interface::send_command ***************************************/
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
    std::string function = "Calib::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( cmd.find( "?" ) != std::string::npos ) return( this->pi.send_command( cmd, retstring ) );
    else return( this->pi.send_command( cmd ) );
  }
  /***** Calib::Interface::send_command ***************************************/

}
