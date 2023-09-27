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

  /***** Slit::Interface::Interface *******************************************/
  /**
   * @brief      class constructor
   *
   */
  Interface::Interface() {
    this->leftcon=-1;
    this->rightcon=-1;
    this->numdev=-1;
  }
  /***** Slit::Interface::Interface *******************************************/


  /***** Slit::Interface::~Interface ******************************************/
  /**
   * @brief      class deconstructor
   *
   */
  Interface::~Interface() {
  }
  /***** Slit::Interface::~Interface ******************************************/


  /***** Slit::Interface::initialize_class ************************************/
  /**
   * @brief      initializes the class from configure_slitd()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Slit::Server::configure_slitd() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Slit::Interface::initialize_class";
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
  /***** Slit::Interface::initialize_class ************************************/


  /***** Slit::Interface::open ************************************************/
  /**
   * @brief      opens the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    std::string function = "Slit::Interface::open";
    std::stringstream message;

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Slit::Server::configure_slitd).
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
  /***** Slit::Interface::open ************************************************/


  /***** Slit::Interface::close ***********************************************/
  /**
   * @brief      closes the PI socket connection
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
    // (called in Slit::Server::configure_slitd).
    //
    if ( !this->pi.is_initialized() ) {
      logwrite( function, "ERROR: pi interface not initialized" );
      return( ERROR );
    }

    return( this->pi.close() );
  }
  /***** Slit::Interface::close ***********************************************/


  /***** Slit::Interface::home ************************************************/
  /**
   * @brief      home all daisy-chained motors using the neg limit switch
   * @details    Both motors are homed simultaneously by spawning a thread for
   *             each. This will also apply any zeropos, after homing.
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

    // home the left and right motors now,
    // simultaneously, each in its own thread.
    //

    std::unique_lock<std::mutex> wait_lock( this->wait_mtx );  // create a mutex object for waiting for threads

    this->motors_running = 2;                                  // set both motors running (number of threads to wait for)

    this->thr_error.store( NO_ERROR );                         // clear the thread error state (threads can set this)

    for ( size_t con=0; con < this->numdev; con++ ) {
      try {
        std::thread( dothread_home, std::ref( *this ), con ).detach();
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: controller element " << con << " out of range";
        logwrite( function, message.str() );
        return( ERROR );
      }
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
  /***** Slit::Interface::home ************************************************/


  /***** Slit::Interface::is_home *********************************************/
  /**
   * @brief       return the home state of the motors
   * @param[out]  retstring  contains the home state "true" | "false"
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
  /***** Slit::Interface::is_home *********************************************/


  /***** Slit::Interface::set *************************************************/
  /**
   * @brief      set the slit width and offset
   * @param[in]  iface      reference to main Slit::Interface object
   * @param[in]  args       string containing width, or width and offset
   * @param[out] retstring  string contains the width and offset after move
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

    float leftpos, rightpos;

    if ( setoffset >= 0 ) {
      rightpos = setoffset + setwidth/this->numdev;
      leftpos  = std::abs( setwidth - rightpos );
    }
    else {
      leftpos  = setwidth/this->numdev - setoffset;
      rightpos = std::abs( setwidth - leftpos );
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] leftpos=" << leftpos << " rightpos=" << rightpos << " width=" << setwidth << " offset=" << setoffset
                             << " leftcon=" << this->leftcon << " rightcon=" << this->rightcon;
    logwrite( function, message.str() );
#endif

    // move the left and right motors now,
    // simultaneously, each in its own thread.
    //

    std::unique_lock<std::mutex> wait_lock( iface.wait_mtx );  // create a mutex object for waiting for threads

    iface.motors_running = 2;                                  // set both motors running (number of threads to wait for)

    iface.thr_error.store( NO_ERROR );                         // clear the thread error state (threads can set this)

    // spawn the left motor thread
    //
    if ( this->leftcon >= 0 && this->leftcon < this->numdev ) {
      std::thread( dothread_move_abs,
                   std::ref( iface ),
                   this->controller_info.at( this->leftcon ).addr,
                   leftpos 
                 ).detach();
    }

    // spawn the right motor thread
    //
    if ( this->rightcon >= 0 && this->rightcon < this->numdev ) {
      std::thread( dothread_move_abs,
                   std::ref( iface ),
                   this->controller_info.at( this->rightcon ).addr,
                   rightpos
                 ).detach();
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
  /***** Slit::Interface::set *************************************************/


  /***** Slit::Interface::get *************************************************/
  /**
   * @brief      get the current width and offset
   * @param[out] retstring  string contains the current width and offset
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get( std::string &retstring ) {
    std::string function = "Slit::Interface::get";
    std::stringstream message;
    long error  = NO_ERROR;
    float leftpos  = 0.0;
    float rightpos = 0.0;
    float width = 0.0;
    float offs  = 0.0;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // get the position for each address in controller_info
    //
    std::string posstring;
    try {
      int axis=1;
      if ( this->leftcon >= 0 && this->leftcon < this->numdev ) {
        error = this->pi.get_pos( this->controller_info.at( this->leftcon ).addr, axis, leftpos );
      }
      if ( this->rightcon >= 0 && this->rightcon < this->numdev ) {
        error = this->pi.get_pos( this->controller_info.at( this->rightcon ).addr, axis, rightpos );
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
    width = leftpos + rightpos;
    offs = ( rightpos - leftpos ) / this->numdev;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] leftpos=" << leftpos << " rightpos=" << rightpos << " numdev=" << this->numdev
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
  /***** Slit::Interface::get *************************************************/


  /***** Slit::Interface::dothread_home ***************************************/
  /**
   * @brief      threaded function to home and apply zeropos
   * @param[in]  iface   reference to interface object
   *
   * This is the work function to call home() in a thread, intended
   * to be spawned in a detached thread. Any errors returned by functions
   * called in here are set in the thr_error class variable.
   *
   */
  void Interface::dothread_home( Slit::Interface &iface, int con ) {
    std::string function = "Slit::Interface::dothread_home";
    std::stringstream message;
    int axis=1;
    int addr = iface.controller_info.at(con).addr;
    float zeropos = iface.controller_info.at(con).zeropos;
    long error=NO_ERROR;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] thread sending home_axis( " << addr << ", " << axis << ", neg )";
    logwrite( function, message.str() );
#endif

    // send the home command by calling home_axis()
    //
    try {
      iface.pi_mutex.lock();
      iface.pi.home_axis( addr, axis, "neg" );
      iface.pi_mutex.unlock();
      iface.controller_info.at(con).ishome   = false;
      iface.controller_info.at(con).ontarget = false;
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR: controller element " << con << " out of range";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Loop sending the is_home command for each address in controller_info
    // until homed or timeout.
    //

    // get the time now for timeout purposes
    //
    std::chrono::steady_clock::time_point tstart = std::chrono::steady_clock::now();

    bool is_home=false;

    do {
      try {
        bool state;
        iface.pi_mutex.lock();
        iface.pi.is_home( addr, axis, state );
        iface.pi_mutex.unlock();
        iface.controller_info.at(con).ishome = state;
        iface.controller_info.at(con).ontarget = state;
        is_home = iface.controller_info.at(con).ishome;
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: controller element " << con << " out of range";
        logwrite( function, message.str() );
        error = ERROR;
      }

      if ( is_home ) break;
      else {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] waiting for homing " << addr << " ..." ;
        logwrite( function, message.str() );
#endif
        usleep( 1000000 );
      }

      // get time now and check for timeout
      //
      std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();

      auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(tnow - tstart).count();

      if ( elapsed > MOVE_TIMEOUT ) {
        message.str(""); message << "TIMEOUT waiting for homing " << addr;
        logwrite( function, message.str() );
        error = TIMEOUT;
        break;
      }

    } while ( 1 );

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
  /***** Slit::Interface::dothread_home ***************************************/


  /***** Slit::Interface::dothread_move_abs ***********************************/
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
  void Interface::dothread_move_abs( Slit::Interface &iface, int addr, float pos ) {
    std::string function = "Slit::Interface::dothread_move_abs";
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
  /***** Slit::Interface::dothread_move_abs ***********************************/


  /***** Slit::Interface::move_abs ********************************************/
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
    std::string function = "Slit::Interface::move_abs";
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
  /***** Slit::Interface::move_abs ********************************************/


  /***** Slit::Interface::move_rel ********************************************/
  /**
   * @brief      send move-relative command to specified controllers
   * @param[in]  args  string containing addr and offset
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
  /***** Slit::Interface::move_rel ********************************************/


  /***** Slit::Interface::stop ************************************************/
  /**
   * @brief      send the stop-all-motion command to all controllers
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
  /***** Slit::Interface::stop ************************************************/


  /***** Slit::Interface::send_command ****************************************/
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
    std::string function = "Slit::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( this->pi.send_command( cmd ) );
  }
  /***** Slit::Interface::send_command ****************************************/


  /***** Slit::Interface::send_command ****************************************/
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
    std::string function = "Slit::Interface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( cmd.find( "?" ) != std::string::npos ) return( this->pi.send_command( cmd, retstring ) );
    else return( this->pi.send_command( cmd ) );
  }
  /***** Slit::Interface::send_command ****************************************/

}
