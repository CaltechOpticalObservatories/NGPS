/**
 * @file    flexure_interface.cpp
 * @brief   this contains the flexure interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Flexure namespace,
 * and is how the flexure daemon interfaces to the flexure hardware.
 *
 */

#include "flexure_interface.h"

namespace Flexure {

  /***** Flexure::Interface::initialize_class *********************************/
  /**
   * @brief      initializes the class from configure_flexured()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Flexure::Server::configure_flexured() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Flexure::Interface::initialize_class";
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
  /***** Flexure::Interface::initialize_class *********************************/


  /***** Flexure::Interface::open *********************************************/
  /**
   * @brief      opens the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    std::string function = "Flexure::Interface::open";
    std::stringstream message;

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Flexure::Server::configure_flexured).
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
  /***** Flexure::Interface::open *********************************************/


  /***** Flexure::Interface::close ********************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    std::string function = "Flexure::Interface::close";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "not connected" );
      return( NO_ERROR );
    }

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Flexure::Server::configure_flexured).
    //
    if ( !this->pi.is_initialized() ) {
      logwrite( function, "ERROR: pi interface not initialized" );
      return( ERROR );
    }

    return( this->pi.close() );
  }
  /***** Flexure::Interface::close ********************************************/


  /***** Flexure::Interface::home *********************************************/
  /**
   * @brief      home all daisy-chained motors
   * @details    Both motors are homed simultaneously by spawning a thread for
   *             each. This will also apply any zeropos, after homing.
   * @param[in]  arg   optional arg for help only
   * @param[out] help  return string containing help
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::home( std::string arg, std::string &help ) {
    std::string function = "Flexure::Interface::home";
    std::stringstream message;
    int axis=1;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      help = FLEXURED_HOME;
      help.append( "  home both flexure motors simultaneously\n" );
      return( NO_ERROR );
    }

    // Anything else requires an open connection
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // Loop through map of motors, spawn a thread to home each one
    //

    std::unique_lock<std::mutex> wait_lock( this->wait_mtx );  // create a mutex object for waiting for threads
    this->thr_error.store( NO_ERROR );                         // clear the thread error state (threads can set this)

    for ( auto mot : this->motormap ) {
      std::thread( dothread_home, std::ref( *this ), mot.first ).detach();
      this->motors_running++;
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] spawning thread to home " << name;
      logwrite( function, message.str() );
#endif
    }

    // wait for the threads to finish
    //
    while ( this->motors_running != 0 ) {
      message.str(""); message << "waiting for " << this->motors_running << " motor" << ( this->motors_running > 1 ? "s":"" );
      logwrite( function, message.str() );
      this->cv.wait( wait_lock );
    }

    logwrite( function, "home complete" );

    // get any errors from the threads
    //
    error = this->thr_error.load();

    return( error );
  }
  /***** Flexure::Interface::home *********************************************/


  /***** Flexure::Interface::is_home ******************************************/
  /**
   * @brief       return the home state of the motors
   * @param[out]  retstring  contains the home state "true" | "false"
   * @return      ERROR or NO_ERROR
   *
   * All motors must be homed for this to return "true".
   *
   */
  long Interface::is_home( std::string &retstring ) {
    std::string function = "Flexure::Interface::is_home";
    std::stringstream message;
    std::stringstream homestream;
    long error = NO_ERROR;

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
  /***** Flexure::Interface::is_home ******************************************/


  /***** Flexure::Interface::set **********************************************/
  /**
   * @brief      set the slit width and offset
   * @param[in]  iface      reference to main Flexure::Interface object
   * @param[in]  args       string containing width, or width and offset
   * @param[out] retstring  string contains the width and offset after move
   * @return     ERROR or NO_ERROR
   *
   * This function moves the "A" and "B" motors to achieve the requested
   * width (and offset, if specified, default 0). Each motor is commanded in its
   * own thread so that they can be moved in parallel.
   *
   * This function requires a reference to the slit interface object because it's
   * going to spawn threads for each motor and the threads, being static, would
   * not otherwise have access to this-> object.
   *
   */
  long Interface::set( Flexure::Interface &iface, std::string args, std::string &retstring ) {
    std::string function = "Flexure::Interface::set";
    std::stringstream message;
    long error = NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // Tokenize the input arg list.
    // Expecting either 1 token <width> for default zero offset
    // or 2 tokens <width> <offset>
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() > 2 || tokens.size() < 1 ) {
      message.str(""); message << "bad number of arguments: " << tokens.size() 
                               << ". expected <width> or <width> <offset>";
      logwrite( function, message.str() );
      return( ERROR );
    }

    float setwidth  = 0.0;  // slit width
    float setoffset = 0.0;  // default offset unless otherwise set below

    // tokens.size() is guaranteed to be either 1 OR 2 at this point
    //
    try {
      switch ( tokens.size() ) {

        case 2:    // the 2nd arg is the setoffset (and if not set here then use default above)
          setoffset = std::stof( tokens.at(1) );

          // do not break!
          // let this case drop through because if there is a 2nd arg then there's a 1st

        case 1:    // the 1st arg is the setwidth
          setwidth = std::stof( tokens.at(0) );
          break;

        default:   // impossible! because I already checked that tokens.size was 1 or 2
          message.str(""); message << "ERROR: impossible! num args=" << tokens.size() << ": " << args;
          logwrite( function, message.str() );
          return( ERROR );
      }
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "unable to convert offset from args " << args << " : " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "one or more values out of range " << args << " : " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // move the A and B motors now,
    // simultaneously, each in its own thread.
    //
    try {
      std::unique_lock<std::mutex> wait_lock( iface.wait_mtx );  // create a mutex object for waiting for threads

      iface.motors_running = 2;                                  // set both motors running (number of threads to wait for)

      iface.thr_error.store( NO_ERROR );                         // clear the thread error state (threads can set this)

      // spawn threads to move each motor, A and B
      //
//    std::thread( dothread_move_abs, std::ref( iface ), this->motormap.at("A").addr, pos_A).detach();
//    std::thread( dothread_move_abs, std::ref( iface ), this->motormap.at("B").addr, pos_B).detach();

      // wait for the threads to finish
      //
      while ( iface.motors_running != 0 ) {
        message.str(""); message << "waiting for " << iface.motors_running << " motor" << ( iface.motors_running > 1 ? "s":"" );
        logwrite( function, message.str() );
        iface.cv.wait( wait_lock );
      }
      logwrite( function, "slit motor moves complete" );

      error = iface.thr_error.load();                            // get any errors from the threads
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR: unknown motor: " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }
    catch ( std::exception &e ) {
      message.str(""); message << "ERROR: other exception: " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }

    // after all the moves, read and return the position
    //
    if ( error == NO_ERROR ) error = this->get( retstring );

    return( error );
  }
  /***** Flexure::Interface::set **********************************************/


  /***** Flexure::Interface::get **********************************************/
  /**
   * @brief      get the current width and offset
   * @param[out] retstring  string contains the current width and offset
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get( std::string &retstring ) {
    std::string function = "Flexure::Interface::get";
    std::stringstream message;
    long error  = NO_ERROR;
/***
    float pos_A = 0.0;
    float pos_B = 0.0;
    float width = 0.0;
    float offs  = 0.0;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // get the position for each address in controller_info
    //
    std::string posstring;
    int axis=1;
    try {
      error = this->pi.get_pos( this->motormap.at("A").addr, axis, pos_A );
      error = this->pi.get_pos( this->motormap.at("B").addr, axis, pos_B );
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR: unknown motor: " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }

    // calculate width and offset
    //
    width = pos_A + pos_B;
    offs = ( pos_B - pos_A ) / this->numdev;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] pos_A=" << pos_A << " pos_B=" << pos_B << " numdev=" << this->numdev
                             << " width=" << width << " offset=" << offs
                             << " con_A=" << this->con_A << " con_B=" << this->con_B;
    logwrite( function, message.str() );
#endif

    // form the return value
    //
    std::stringstream s;
    s << width << " " << offs;
    retstring = s.str();

    message.str(""); message << "NOTICE:" << Flexure::DAEMON_NAME << " " << retstring;
    this->async.enqueue( message.str() );
****/

    return( error );
  }
  /***** Flexure::Interface::get **********************************************/


  /***** Flexure::Interface::dothread_home ************************************/
  /**
   * @brief      threaded function to home and apply zeropos
   * @param[in]  iface   reference to interface object
   * @param[in]  name    reference to name of motor to home
   *
   * This is the work function to call home() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by functions
   * called in here are set in the thr_error class variable.
   *
   */
  void Interface::dothread_home( Flexure::Interface &iface, std::string name ) {
    std::string function = "Flexure::Interface::dothread_home";
    std::stringstream message;
    int axis=1;
    long error=NO_ERROR;
    int addr=-1;
    float zeropos=NAN;
    std::string reftype;

    try {
      addr    = iface.motormap.at(name).addr;
      zeropos = iface.motormap.at(name).zeropos;
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

    // If homed OK then apply the zeropos
    //
    if ( error == NO_ERROR ) {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] setting zeropos " << zeropos << " for " << addr << " ..." ;
        logwrite( function, message.str() );
#endif
      std::stringstream cmd;
      error  = iface.move_abs( addr, zeropos );  // move to zeropos position
      cmd << addr << " DFH " << axis;
      error |= iface.send_command( cmd.str() );  // define this as the home position
    }

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
  /***** Flexure::Interface::dothread_home ************************************/


  /***** Flexure::Interface::dothread_move_abs ********************************/
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
  void Interface::dothread_move_abs( Flexure::Interface &iface, int addr, float pos ) {
    std::string function = "Flexure::Interface::dothread_move_abs";
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
  /***** Flexure::Interface::dothread_move_abs ********************************/


  /***** Flexure::Interface::move_abs *****************************************/
  /**
   * @brief      send move-absolute command to specified controllers
   * @param[in]  addr  controller address
   * @param[in]  pos   motor position
   * @return     ERROR or NO_ERROR
   *
   * The single string parameter must contain two space-delimited tokens,
   * for the address and the position to move that address.
   *
   * This could be called by a thread, so hardware interactions with the PI
   * controller are protected by a mutex.
   *
   */
  long Interface::move_abs( int addr, float pos ) {
    std::string function = "Flexure::Interface::move_abs";
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
  /***** Flexure::Interface::move_abs *****************************************/


  /***** Flexure::Interface::move_rel *****************************************/
  /**
   * @brief      send move-relative command to specified controllers
   * @param[in]  args  string containing addr and offset
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::move_rel( std::string args ) {
    std::string function = "Flexure::Interface::move_rel";
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
  /***** Flexure::Interface::move_rel *****************************************/


  /***** Flexure::Interface::stop *********************************************/
  /**
   * @brief      send the stop-all-motion command to all controllers
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::stop( ) {
    std::string function = "Flexure::Interface::stop";
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
  /***** Flexure::Interface::stop *********************************************/


  /***** Flexure::Interface::send_command *************************************/
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
    std::string function = "Flexure::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( this->pi.send_command( cmd ) );
  }
  /***** Flexure::Interface::send_command *************************************/


  /***** Flexure::Interface::send_command *************************************/
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
    std::string function = "Flexure::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( cmd.find( "?" ) != std::string::npos ) return( this->pi.send_command( cmd, retstring ) );
    else return( this->pi.send_command( cmd ) );
  }
  /***** Flexure::Interface::send_command *************************************/

}
