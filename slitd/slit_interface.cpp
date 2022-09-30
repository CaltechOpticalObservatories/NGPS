/**
 * @file    slit_interface.cpp
 * @brief   this contains the slit interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Slit namespace,
 * and is how the slit daemon interfaces to the slit hardware.
 *
 */

#include "slit_interface.h"

namespace Slit {

  /**************** Slit::Interface::Interface ********************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
    this->leftcon=-1;
    this->rightcon=-1;
    this->numdev=-1;
  }
  /**************** Slit::Interface::Interface ********************************/


  /**************** Slit::Interface::~Interface *******************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Slit::Interface::~Interface *******************************/


  /**************** Slit::Interface::initialize_class *************************/
  /**
   * @fn         initialize_class
   * @brief      initializes the class from configure_slit()
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   * This is called by Slit::Server::configure_slit() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Slit::Interface::initialize_class";
    std::stringstream message;
    long error = ERROR;

    if ( this->port < 0 || host.empty() ) {
      logwrite( function, "ERROR: host or port invalid" );
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
//    logwrite( function, "WARNING: limited slit range with only one motor controller" );  // consider allowing this?
//    error = NO_ERROR;
    }
    else if ( this->numdev < 1 ) {
      message.str(""); message << "ERROR: no motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }

    this->minwidth=0;
    this->maxwidth=0;

    for ( size_t con = 0; con < this->numdev; con++ ) {
      this->maxwidth += ( this->controller_info.at(con).max - this->controller_info.at(con).min );
      this->minwidth += this->controller_info.at(con).min;
      if ( this->controller_info.at(con).name == "left" ) this->leftcon   = con;
      else
      if ( this->controller_info.at(con).name == "right" ) this->rightcon = con;
      else {
        message.str(""); message << "ERROR: unrecognized name \"" << this->controller_info.at(con).name << "\". expected left or right";
        logwrite( function, message.str() );
        error = ERROR;
      }
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] controller #" << con
                               << " max=" << this->controller_info.at(con).max << " min=" << this->controller_info.at(con).min;
      logwrite( function, message.str() );
#endif
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] numdev=" << this->numdev 
                             << " minwidth=" << this->minwidth << " maxwidth=" << this->maxwidth;
    logwrite( function, message.str() );
#endif

    return( error );
  }
  /**************** Slit::Interface::initialize_class *************************/


  /**************** Slit::Interface::open *************************************/
  /**
   * @fn         open
   * @brief      opens the PI socket connection
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    std::string function = "Slit::Interface::open";
    std::stringstream message;

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Slit::Server::configure_slit).
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
    for ( size_t con=0; con < this->numdev; con++ ) {
      try {
        int axis=1;
        int errcode;
        error |= this->pi.get_error( this->controller_info.at(con).addr, errcode );     // read error to clear, don't care the value
        error |= this->pi.set_servo( this->controller_info.at(con).addr, axis, true );  // turn the servos on
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: controller element " << con << " out of range";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    return( error );
  }
  /**************** Slit::Interface::open *************************************/


  /**************** Slit::Interface::close ************************************/
  /**
   * @fn         close
   * @brief      closes the PI socket connection
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    std::string function = "Slit::Interface::close";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "not connected" );
      return( NO_ERROR );
    }

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Slit::Server::configure_slit).
    //
    if ( !this->pi.is_initialized() ) {
      logwrite( function, "ERROR: pi interface not initialized" );
      return( ERROR );
    }

    return( this->pi.close() );
  }
  /**************** Slit::Interface::close ************************************/


  /**************** Slit::Interface::home *************************************/
  /**
   * @fn         home
   * @brief      home all daisy-chained motors using the neg limit switch
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::home( ) {
    std::string function = "Slit::Interface::home";
    std::stringstream message;
    int axis=1;
    long error = NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // send the home_axis command for each address in controller_info
    //
    for ( size_t con=0; con < this->numdev; con++ ) {
      try {
        this->pi.home_axis( this->controller_info.at(con).addr, axis, "neg" );
        this->controller_info.at(con).ishome   = false;
        this->controller_info.at(con).ontarget = false;
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: controller element " << con << " out of range";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    // Loop sending the is_home command for each address in controller_info
    // until homed or timeout.
    //

    // get the time now for timeout purposes
    //
    std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

    do {
      size_t num_home=0;
      for ( size_t con=0; con < this->numdev; con++ ) {
        try {
          bool state;
          this->pi.is_home( this->controller_info.at(con).addr, axis, state );
          this->controller_info.at(con).ishome = state;
          this->controller_info.at(con).ontarget = state;
          if ( this->controller_info.at(con).ishome ) num_home++;
        }
        catch( std::out_of_range &e ) {
          message.str(""); message << "ERROR: controller element " << con << " out of range";
          logwrite( function, message.str() );
          return( ERROR );
        }
      }
      if ( num_home == this->numdev ) break;
      else {
#ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] waiting for homing..." );
#endif
        usleep( 1000000 );
      }

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > MOVE_TIMEOUT ) {
        logwrite( function, "TIMEOUT waiting for homing" );
        error = TIMEOUT;
        break;
      }

    } while ( 1 );

    return( error );
  }
  /**************** Slit::Interface::home *************************************/


  /**************** Slit::Interface::is_home **********************************/
  /**
   * @fn          is_home
   * @brief       return the home state of the motors
   * @param[in]   none
   * @param[out]  retstring contains the home state ("true" | "false")
   * @return      ERROR or NO_ERROR
   *
   * All motors must be homed for this to return "true".
   *
   */
  long Interface::is_home( std::string &retstring ) {
    std::string function = "Slit::Interface::is_home";
    std::stringstream message;
    std::stringstream homestream;
    long error = NO_ERROR;

    // Loop through all motor controllers, asking each if homed,
    // setting each controller's .ishome flag, and keeping count 
    // of the number that are homed.
    //
    size_t num_home=0;
    for ( size_t con=0; con < this->numdev; con++ ) {
      try {
        int axis=1;
        error |= this->pi.is_home( this->controller_info.at(con).addr,
                                   axis,
                                   this->controller_info.at(con).ishome );  // error is OR'd so any error is preserved
        homestream << this->controller_info.at(con).addr << ":"
                   << ( this->controller_info.at(con).ishome ? "true" : "false" ) << " ";
        if ( this->controller_info.at(con).ishome ) num_home++;
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: controller element " << con << " out of range";
        logwrite( function, message.str() );
        return( ERROR );
      }
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
  /**************** Slit::Interface::is_home **********************************/


  /**************** Slit::Interface::set **************************************/
  /**
   * @fn         set
   * @brief      set the slit width and offset
   * @param[in]  iface, reference to main Slit::Interface object
   * @param[in]  args, string containing width, or width and offset
   * @param[out] retstring, string contains the width and offset after move
   * @return     ERROR or NO_ERROR
   *
   * This function moves the "left" and "right" motors to achieve the requested
   * width (and offset, if specified, default 0). Each motor is commanded in its
   * own thread so that they can be moved in parallel.
   *
   * Note that "left" and "right" are just my words here, and don't necessarily
   * correspond with any real notion of left and right.
   *
   * This function requires a reference to the slit interface object because it's
   * going to spawn threads for each motor and the threads, being static, would
   * not otherwise have access to this-> object.
   *
   */
  long Interface::set( Slit::Interface &iface, std::string args, std::string &retstring ) {
    std::string function = "Slit::Interface::set";
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

    // Now range-check values
    //
    if ( setwidth > this->maxwidth || setwidth < this->minwidth ) {
      message.str(""); message << "ERROR: width " << setwidth << " out of range. expected {" 
                               << minwidth <<":" << maxwidth << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // offset can be limited by either of these so pick the smaller of the two:
    //
    std::vector<float> checkoffsets;
    checkoffsets.push_back( ( this->maxwidth - setwidth ) / 2 );
    checkoffsets.push_back( setwidth / 2 );

    float max_offset = *min_element( checkoffsets.begin(), checkoffsets.end() );

    // range check requested width and offset
    //
    if ( std::abs( setoffset ) > max_offset ) {
      message.str(""); message << "ERROR: requested offset " << std::abs(setoffset) 
                               << " exceeds maximum " << max_offset << " allowed for this width";
      logwrite( function, message.str() );
      return( ERROR );
    }

    if ( setoffset < 0 && this->numdev < 2 ) {
      message.str(""); message << "ERROR: negative offset " << setoffset << " not allowed with only one motor";
      logwrite( function, message.str() );
      return( ERROR );
    }

    float left, right;

    if ( setoffset >= 0 ) {
      right = setoffset + setwidth/this->numdev;
      left  = std::abs( setwidth - right );
    }
    else {
      left  = setwidth/this->numdev - setoffset;
      right = std::abs( setwidth - left );
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] left=" << left << " right=" << right << " width=" << setwidth << " offset=" << setoffset
                             << " leftcon=" << this->leftcon << " rightcon=" << this->rightcon;
    logwrite( function, message.str() );
#endif

    // move the left and right motors now,
    // simultaneously, each in its own thread.
    //

    std::unique_lock<std::mutex> wait_lock( iface.wait_mtx );  // create a mutex object for waiting for threads

    iface.motors_running = 2;                                  // set both motors running (number of threads to wait for)

    iface.thr_error.store( NO_ERROR );                         // clear the thread error state (threads can set this)

    std::stringstream movstr;                                  // string to contain the move_abs() arguments

    // spawn the left motor thread
    //
    if ( this->leftcon >= 0 && this->leftcon < this->numdev ) {
      movstr.str(""); movstr << this->controller_info.at( this->leftcon ).addr << " " << left;
      std::thread( dothread_move_abs, std::ref( iface ), movstr.str() ).detach();
    }

    // spawn the right motor thread
    //
    if ( this->rightcon >= 0 && this->rightcon < this->numdev ) {
      movstr.str(""); movstr << this->controller_info.at( this->rightcon ).addr << " " << right;
      std::thread( dothread_move_abs, std::ref( iface ), movstr.str() ).detach();
    }

    // wait for the threads to finish
    //
    while ( iface.motors_running != 0 ) {
      message.str(""); message << "waiting for " << iface.motors_running << " motor" << ( iface.motors_running > 1 ? "s":"" );
      logwrite( function, message.str() );
      iface.cv.wait( wait_lock );
    }

    logwrite( function, "slit motor moves complete" );

    // get any errors from the threads
    //
    error = iface.thr_error.load();

    // after all the moves, read and return the position
    //
    if ( error == NO_ERROR ) error = this->get( retstring );

    return( error );
  }
  /**************** Slit::Interface::set **************************************/


  /**************** Slit::Interface::get **************************************/
  /**
   * @fn         get
   * @brief      get the current width and offset
   * @param[in]  none
   * @param[out] retstring, string contains the current width and offset
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get( std::string &retstring ) {
    std::string function = "Slit::Interface::get";
    std::stringstream message;
    long error  = NO_ERROR;
    float left  = 0.0;
    float right = 0.0;
    float width = 0.0;
    float offs  = 0.0;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // get the position for each address in controller_info
    //
    std::stringstream movstr;
    std::string posstring;
    try {
      int axis=1;
      if ( this->leftcon >= 0 && this->leftcon < this->numdev ) {
        error = this->pi.get_pos( this->controller_info.at( this->leftcon ).addr, axis, left );
      }
      if ( this->rightcon >= 0 && this->rightcon < this->numdev ) {
        error = this->pi.get_pos( this->controller_info.at( this->rightcon ).addr, axis, right );
      }
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "unable to convert position " << posstring << " : " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "out of range converting position " << posstring << " : " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // calculate width and offset
    //
    width = left + right;
    offs = ( right - left ) / this->numdev;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] left=" << left << " right=" << right << " numdev=" << this->numdev
                             << " width=" << width << " offset=" << offs
                             << " leftcon=" << this->leftcon << " rightcon=" << this->rightcon;
    logwrite( function, message.str() );
#endif

    // form the return value
    //
    std::stringstream s;
    s << width << " " << offs;
    retstring = s.str();

    message.str(""); message << "NOTICE:" << Slit::DAEMON_NAME << " " << retstring;
    this->async.enqueue( message.str() );

    return( error );
  }
  /**************** Slit::Interface::get **************************************/


  /**************** Slit::Interface::dothread_move_abs ************************/
  /**
   * @fn         dothread_move_abs
   * @brief      threaded move_abs function
   * @param[in]  reference to interface object
   * @param[in]  string to pass to move_abs
   * @return     none
   *
   * This is the work function to call move_abs() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by the move_abs()
   * function are set in the thr_error class variable.
   *
   */
  void Interface::dothread_move_abs( Slit::Interface &iface, std::string movstr ) {
    std::string function = "Slit::Interface::dothread_move_abs";
    std::stringstream message;
    long error;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread sending mov_abs( " << movstr << " )";
    logwrite( function, message.str() );
#endif

    error = iface.move_abs( movstr );    // send the move_abs command here

    iface.thr_error.fetch_or( error );   // preserve any error returned

    --iface.motors_running;              // atomically decrement the number of motors waiting

    iface.cv.notify_all();               // notify parent that I'm done

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread completed mov_abs( " << movstr << " ) *** motors_running = " << iface.motors_running;
    logwrite( function, message.str() );
#endif

    return;
  }
  /**************** Slit::Interface::dothread_move_abs ************************/


  /**************** Slit::Interface::move_abs *********************************/
  /**
   * @fn         move_abs
   * @brief      send move-absolute command to specified controllers
   * @param[in]  args, string containing addr and pos
   * @return     ERROR or NO_ERROR
   *
   * The single string parameter must contain two space-delimited tokens,
   * for the address and the position to move that address.
   *
   * This could be called by a thread, so hardware interactions with the PI
   * controller are protected by a mutex.
   *
   */
  long Interface::move_abs( std::string args ) {
    std::string function = "Slit::Interface::move_abs";
    std::stringstream message;
    int addr;
    float trypos;
    long error=NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // The input args string must contain two tokens, addr pos
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR: bad number of tokens: " << tokens.size()
                               << ". expected 2 (addr pos)";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      int axis=1;
      addr   = std::stoi( tokens.at(0) );
      trypos = std::stof( tokens.at(1) );

      // send the move command
      //
      this->pi_mutex.lock();
      error = this->pi.move_abs( addr, axis, trypos );
      this->pi_mutex.unlock();

      // which controller has this addr?
      //
      int mycon=-1;
      for ( size_t con=0; con < this->numdev; con++ ) {
        if ( this->controller_info.at(con).addr == addr ) {
          mycon = con;
          break;
        }
      }

      if ( mycon == -1 ) {  // should be impossible because numdev was checked in initialize_class()
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
        this->controller_info.at( mycon ).ontarget = state;
        
        if ( this->controller_info.at( mycon ).ontarget ) break;
        else {
#ifdef LOGLEVEL_DEBUG
          message.str(""); message << "[DEBUG] waiting for " << this->controller_info.at( mycon ).name;
          logwrite( function, message.str() );
#endif
          usleep( 1000000 );
        }

        // get time now and check for timeout
        //
        std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

        if ( elapsed > MOVE_TIMEOUT ) {
          message.str(""); message << "TIMEOUT waiting for " << this->controller_info.at( mycon ).name;
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

    return( error );
  }
  /**************** Slit::Interface::move_abs *********************************/


  /**************** Slit::Interface::move_rel *********************************/
  /**
   * @fn         move_rel
   * @brief      send move-relative command to specified controllers
   * @param[in]  args, string containing addr and offset
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::move_rel( std::string args ) {
    std::string function = "Slit::Interface::move_rel";
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
  /**************** Slit::Interface::move_rel *********************************/


  /**************** Slit::Interface::stop *************************************/
  /**
   * @fn         stop
   * @brief      send the stop-all-motion command to all controllers
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::stop( ) {
    std::string function = "Slit::Interface::stop";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // send the stop_motion command for each address in controller_info
    //
    for ( size_t con=0; con < this->numdev; con++ ) {
      try {
        this->pi.stop_motion( this->controller_info.at(con).addr );
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: controller element " << con << " out of range";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    return( NO_ERROR );
  }
  /**************** Slit::Interface::stop *************************************/


  /**************** Slit::Interface::send_command *****************************/
  /**
   * @fn         send_command
   * @brief      writes the raw command as received to the master controller
   * @param[in]  string cmd
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command that expects no reply.
   *
   */
  long Interface::send_command( std::string cmd ) {
    std::string function = "Slit::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( this->pi.send_command( cmd ) );
  }
  /**************** Slit::Interface::send_command *****************************/


  /**************** Slit::Interface::send_command *****************************/
  /**
   * @fn         send_command
   * @brief      writes the raw command to the master controller, reads back reply
   * @param[in]  string cmd
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command and then reads a reply if that command contains
   * a question mark, "?".
   *
   */
  long Interface::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Slit::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( cmd.find( "?" ) != std::string::npos ) return( this->pi.send_command( cmd, retstring ) );
    else return( this->pi.send_command( cmd ) );
  }
  /**************** Slit::Interface::send_command *****************************/

}
