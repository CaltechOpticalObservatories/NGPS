/**
 * @file    motion_interface.cpp
 * @brief   this contains the motion interface code used by acam
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the MotionInterface class in the Acam namespace,
 * and is how the acam daemon interfaces to the motion hardware.
 *
 */

#include "motion_interface.h"

namespace Acam {

  /***** Acam::MotionInterface::MotionInterface *******************************/
  /**
   * @brief      class constructor
   *
   */
  MotionInterface::MotionInterface() {
    this->numdev=-1;
  }
  /***** Acam::MotionInterface::MotionInterface *******************************/


  /***** Acam::MotionInterface::~MotionInterface ******************************/
  /**
   * @brief      class deconstructor
   *
   */
  MotionInterface::~MotionInterface() {
  }
  /***** Acam::MotionInterface::~MotionInterface ******************************/


  /***** Acam::MotionInterface::initialize_class ******************************/
  /**
   * @brief      initializes the MotionInterface class from configure_acam()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Acam::Server::configure_acam() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long MotionInterface::initialize_class() {
    std::string function = "Acam::MotionInterface::initialize_class";
    std::stringstream message;
    long error = ERROR;

    if ( this->port < 0 || this->host.empty() ) {
      logwrite( function, "ERROR: host or port invalid" );
      return( ERROR );
    }

    Physik_Instrumente::ServoInterface s( this->name, this->host, this->port );
    this->pi = s;

    this->numdev = this->motion_info.size();

    if ( this->numdev == 2 ) {
      logwrite( function, "interface initialized ok" );
      error = NO_ERROR;
    }
    else if ( this->numdev > 2 ) {
      message.str(""); message << "ERROR: too many motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }
    else {
      message.str(""); message << "ERROR: insufficient motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    for ( auto const& con : this->motion_info ) {
      message.str(""); message << "[DEBUG] controller " << con.first
                               << " addr=" << con.second.addr
                               << " reftype=" << con.second.reftype;
      logwrite( function, message.str() );
    }
#endif

    return( error );
  }
  /***** Acam::MotionInterface::initialize_class ******************************/


  /***** Acam::Interface::open ************************************************/
  /**
   * @brief      opens the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::open( ) {
    std::string function = "Acam::MotionInterface::open";
    std::stringstream message;

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Acam::Server::configure_acam).
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
    // enable the servo for each address in motion_info
    //
    for ( auto const& mot : this->motion_info ) {
      int axis=1;
      int errcode;
      error |= this->pi.get_error( mot.second.addr, errcode );     // read error to clear, don't care the value
      error |= this->pi.set_servo( mot.second.addr, axis, true );  // turn the servos on
    }

    return( error );
  }
  /***** Acam::Interface::open ************************************************/


  /***** Acam::MotionInterface::close *****************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::close( ) {
    std::string function = "Acam::MotionInterface::close";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "not connected" );
      return( NO_ERROR );
    }

    // Should be impossible --
    // The initialization should have been called automatically at start up
    // (called in Acam::Server::configure_acam).
    //
    if ( !this->pi.is_initialized() ) {
      logwrite( function, "ERROR: pi interface not initialized" );
      return( ERROR );
    }

    return( this->pi.close() );
  }
  /***** Acam::MotionInterface::close *****************************************/


  /***** Acam::MotionInterface::home ******************************************/
  /**
   * @brief      home all daisy-chained motors using the neg limit switch
   * @param[in]  name_in  optional list of motors to home
   * @param[out] help     return string containing help on request
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::home( std::string name_in, std::string &help ) {
    std::string function = "Acam::MotionInterface::home";
    std::stringstream message;
    long error = NO_ERROR;
    std::vector<std::string> name_list;                        // vector of input names
    std::unique_lock<std::mutex> wait_lock( this->wait_mtx );  // create a mutex object for waiting for threads

    // Help
    //
    if ( name_in == "?" ) {
      help = ACAMD_HOME;
      help.append( " [ " );
      for ( auto const& mot : this->motion_info ) {
        help.append( mot.first );
        help.append( " " );
      }
      help.append( "]\n" );
      help.append( "  no arg will home all motors simultaneously\n" );
      help.append( "  or provide name of motor to home only that motor\n" );
      return( NO_ERROR );
    }

    // Anything else requires an open connection
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // If name_in is empty then build up a vector of each motor name
    //
    if ( name_in.empty() ) {
      for ( auto mot : this->motion_info ) { name_list.push_back( mot.first ); }
    }
    else {
      std::transform( name_in.begin(), name_in.end(), name_in.begin(), ::tolower );
      Tokenize( name_in, name_list, " " );
      if ( name_list.size() > this->numdev ) {
        message.str(""); message << "ERROR: too many names specified: " << name_in.size() << " "
                                 << "(max " << this->numdev << ")";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    this->thr_error.store( NO_ERROR );    // clear thread error state (threads can set this)

    // Now loop through the built up list of motor names
    //
    for ( auto name : name_list ) {

      auto name_found = this->motion_info.find( name );

      if ( name_found == this->motion_info.end() ) {
        message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        return( ERROR );
      }

      // Spawn a thread to performm the home move.
      // If there is more than one then they can be done in parallel.
      //
      std::thread( dothread_home, std::ref( *this ), name ).detach();
      this->motors_running++;

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] spawning thread to home " << name;
      logwrite( function, message.str() );
#endif
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
  /***** Acam::MotionInterface::home ******************************************/


  /***** Acam::MotionInterface::dothread_home *********************************/
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
  void MotionInterface::dothread_home( Acam::MotionInterface &iface, std::string name ) {
    std::string function = "Acam::MotionInterface::dothread_home";
    std::stringstream message;
    int axis=1;
    int addr = iface.motion_info[name].addr;
    std::string reftype = iface.motion_info[name].reftype;
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
    iface.motion_info[name].ishome   = false;
    iface.motion_info[name].ontarget = false;

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
      iface.motion_info[name].ishome = state;
      iface.motion_info[name].ontarget = state;
      is_home = iface.motion_info[name].ishome;

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
  /***** Acam::MotionInterface::dothread_home *********************************/


  /***** Acam::MotionInterface::is_home ***************************************/
  /**
   * @brief       return the home state of the motors
   * @param[in]   name_in  optionally contains one or more motors to check
   * @param[out]  retstring  contains the home state ("true" | "false")
   * @return      ERROR or NO_ERROR
   *
   * All named motors must be homed for this to return "true".
   *
   */
  long MotionInterface::is_home( std::string name_in, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::is_home";
    std::stringstream message;
    std::stringstream homestream;
    long error = NO_ERROR;
    std::vector<std::string> name_list;
    size_t num_home=0;

    // Help
    //
    if ( name_in == "?" ) {
      retstring = ACAMD_ISHOME;
      retstring.append( " [ <name> ]\n" );
      retstring.append( "  where <name> = { " );
      for ( auto mot : this->motion_info ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "}\n" );
      retstring.append( "  if no arg provided then both must be true to return true\n" );
      return( NO_ERROR );
    }

    // Requires an open connection
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // If name_in is empty then build up a vector of each motor name
    //
    if ( name_in.empty() ) {
      for ( auto mot : this->motion_info ) { name_list.push_back( mot.first ); }
    }
    else {
      std::transform( name_in.begin(), name_in.end(), name_in.begin(), ::tolower );
      Tokenize( name_in, name_list, " " );
      if ( name_list.size() > this->numdev ) {
        message.str(""); message << "ERROR: too many names specified: " << name_in.size() << " "
                                 << "(max " << this->numdev << ")";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    for ( auto name : name_list ) {

      auto name_found = this->motion_info.find( name );

      if ( name_found == this->motion_info.end() ) {
        message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        return( ERROR );
      }

      int axis = 1;

      error |= this->pi.is_home( this->motion_info[name].addr,
                                 axis,
                                 this->motion_info[name].ishome );  // error is OR'd so any error is preserved
      homestream << name << ":"
                 << ( this->motion_info[name].ishome ? "true" : "false" ) << " ";
      if ( this->motion_info[name].ishome ) num_home++;
    }

    // Set the retstring true or false, true only if all requested controllers are the same
    //
    if ( num_home == name_list.size() ) retstring = "true";
    else
    if ( num_home == 0 )                retstring = "false";
    else

    // If not all are the same state then log that but report false
    //
    if ( num_home > 0 && num_home < name_list.size() ) {
      logwrite( function, homestream.str() );
      retstring = "false";
    }

    return( error );
  }
  /***** Acam::MotionInterface::is_home ***************************************/


  /***** Acam::MotionInterface::move_abs **************************************/
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
  long MotionInterface::move_abs( std::string name, float pos ) {
    std::string function = "Acam::MotionInterface::move_abs";
    std::stringstream message;
    long error=NO_ERROR;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( this->motion_info.find( name ) == this->motion_info.end() ) {
      message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
      logwrite( function, message.str() );
      return( ERROR );
    }

    int addr = this->motion_info[ name ].addr;
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
      this->motion_info[ name ].ontarget = state;

      if ( this->motion_info[ name ].ontarget ) break;
      else {
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
  /***** Acam::MotionInterface::move_abs **************************************/


  /***** Acam::MotionInterface::motion ****************************************/
  /**
   * @brief      basic motion test commands
   * @param[in]  args       string containing optional args (see details below)
   * @param[out] retstring  return string contains response
   * @return     ERROR or NO_ERROR
   *
   * expected args: [ [ <name> [ native <cmd> | <posname> ] ] ]
   *
   * Only the native command requires an open connection to hardware;
   * everything else is just information held within the class.
   *
   */
  long MotionInterface::motion( std::string args, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::motion";
    std::stringstream message, retstream;
    long error = NO_ERROR;

    // help
    //
    if ( args == "?" ) {
      retstring = ACAMD_MOTION;
      retstring.append( " [ [ <name> [ native <cmd> | <posname> ] ] ]\n" );
      retstring.append( "  no arg gives list of motor names\n" );
      retstring.append( "  <name> only gives list of posnames\n" );
      retstring.append( "  <name> <posname> gives info for that posname\n" );
      retstring.append( "  <name> native <cmd> sends native commands to that motor\n" );
      return( NO_ERROR );
    }

    // No args, print all of the indices of the motion_info STL map,
    // which will be a list of the STEPPER_CONTROLLER names.
    //
    if ( args.empty() ) {
      retstream << "{ ";
      for ( auto mot : this->motion_info ) { retstream << mot.first << " "; }
      retstream << "}";
      retstring = retstream.str();
      logwrite( function, retstring );
      return( NO_ERROR );
    }

    std::vector<std::string> arglist;

    Tokenize( args, arglist, " " );

    if ( arglist.size() <= 0 ) {  // should be impossible since args ! empty
      logwrite( function, "ERROR: bad argument list" );
      return( ERROR );
    }

    // One arg, taken to be the name of a STEPPER_CONTROLLER,
    // print all of the indices of the stepper STL map for the named controller,
    // which will be a list of the STEPPER_POS position names.
    //
    std::string name;
    if ( arglist.size() >= 1 ) {
      name = arglist[0];

      // Check that name is found in the motion_info map
      //
      if ( this->motion_info.find( name ) == this->motion_info.end() ) {
        message.str(""); message << "ERROR: motor name \"" << name << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        return( ERROR );
      }

      if ( arglist.size() == 1 ) {
        retstream << "{ ";
        for ( auto const &pos : this->motion_info[ name ].stepper ) { retstream << pos.first << " "; }
        retstream << "}";
        retstring = retstream.str();
        logwrite( function, retstring );
      }
    }

    // Two args is taken to be requesting for info of a posname for the given controller name
    //
    if ( arglist.size() == 2 ) {
      std::string posname = arglist[1];

      // Check that posname is found in the motion_info.stepper map
      //
      if ( this->motion_info[name].stepper.find( posname ) == this->motion_info[name].stepper.end() ) {
        message.str(""); message << "ERROR: position name \"" << posname << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        retstring="";
        return( ERROR );
      }

      retstream << this->motion_info[ name ].stepper[ posname ].posname << ": "
                << "id="       << this->motion_info[ name ].stepper[ posname ].id << " "
                << "position=" << this->motion_info[ name ].stepper[ posname ].position;
      retstring = retstream.str();
      logwrite( function, retstring );
    }

    // More than 2 args is taken to be the "native" command. This allows
    // sending native controller commands directly to the PI controller
    // specified by <name>, e.g. 
    //    motion filter native FRF?
    // or 
    //    motion dustcover native MOV 1 0
    //
    // Only commands with a "?" will provide a reply in retstring.
    //
    if ( arglist.size() > 2 ) {
      if ( arglist[1] != "native" ) {
        logwrite( function, "ERROR: expected motion [ [ <name> [ native <cmd> | <posname> ] ] ]" );
        return( ERROR );
      }
      else {

        // This requires an open connection (nothing else so far in this function does)
        //
        if ( !this->isopen() ) {
          logwrite( function, "ERROR: not connected to motor controller" );
          return( ERROR );
        }

        int addr = this->motion_info[ name ].addr;  // Get the address from the class,
        std::stringstream cmdstream;                // then build the command from the arglist.
        cmdstream << addr;
        for ( int i=2; i<(int)arglist.size(); i++ ) cmdstream << " " << arglist[i];
        error = this->send_command( cmdstream.str(), retstring );
      }
    }

    return( error );
  }
  /***** Acam::MotionInterface::motion ****************************************/


  /***** Acam::MotionInterface::send_command **********************************/
  /**
   * @brief      writes the raw command as received to the master controller
   * @param[in]  cmd
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command that expects no reply.
   *
   */
  long MotionInterface::send_command( std::string cmd ) {
    std::string function = "Acam::MotionInterface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    return( this->pi.send_command( cmd ) );
  }
  /***** Acam::MotionInterface::send_command **********************************/


  /***** Acam::MotionInterface::send_command **********************************/
  /**
   * @brief      writes the raw command to the master controller, reads back reply
   * @param[in]  cmd
   * @param[in]  retstring
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command and then reads a reply if that command contains
   * a question mark, "?".
   *
   */
  long MotionInterface::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::send_command";
    std::stringstream message;

    if ( !this->pi.controller.isconnected() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    if ( cmd.find( "?" ) != std::string::npos ) return( this->pi.send_command( cmd, retstring ) );
    else return( this->pi.send_command( cmd ) );
  }
  /***** Acam::MotionInterface::send_command **********************************/


  /***** Acam::MotionInterface::filter ****************************************/
  /**
   * @brief      set or get the filter
   * @param[in]  destname   string containing destination filter position name
   * @param[out] retstring  return string contains the current position name
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::filter( std::string destname, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::filter";
    std::string filter = "filter";
    std::stringstream message;
    std::string currname;
    int axis = 1;
    int currid;
    float currpos;
    long error = NO_ERROR;

    // Help
    //
    if ( destname == "?" ) {
      retstring = ACAMD_FILTER;
      retstring.append( " [ <filtername> ]\n" );
      retstring.append( "  where <filtername> = { " );
      for ( auto const &pos : this->motion_info[ filter ].stepper ) {
        retstring.append( pos.first );
        retstring.append( " " );
      }
      retstring.append( "}\n" );
      retstring.append( "  Move filter wheel to filter <filtername>. returns <filtername>\n" );
      retstring.append( "  If no arg provided, return only the current <filtername>\n" );
      return( NO_ERROR );
    }

    // Connection must be open to motion component
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // Filter motor must be homed
    //
    std::string ishome;
    error = this->is_home( filter, ishome );
    if ( error != NO_ERROR ) return( error );
    else
    if ( error==NO_ERROR && ishome != "true" ) {
      logwrite( function, "ERROR: filter motor is not homed" );
      return( ERROR );
    }

    // If destname is supplied then move wheel to that position.
    //
    if ( !destname.empty() ) {

      // Check that destname is found in the motion_info.stepper map
      //
      if ( this->motion_info[filter].stepper.find( destname ) == this->motion_info[filter].stepper.end() ) {
        message.str(""); message << "ERROR: position \"" << destname << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        retstring="";
        return( ERROR );
      }

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dest=" << this->motion_info[filter].stepper[destname].posname << " "
                               << "id="       << this->motion_info[filter].stepper[destname].id << " "
                               << "position=" << this->motion_info[filter].stepper[destname].position;
      logwrite( function, message.str() );
#endif

      // Where are we now?
      //
      error = this->current_filter( currname, currid, currpos );

      // Where do we want to go?
      //
      int destid = this->motion_info[ filter ].stepper[ destname ].id;
      float destpos = this->motion_info[ filter ].stepper[ destname ].position;
      int newid = mod( ( destid + ( 2 - currid ) ) , 6 );

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] currid=" << currid << " destid=" << destid << " newid=" << newid;
      logwrite( function, message.str() );
#endif

      // Find the position for id=2 so that we can temporarily remap
      // current position as position #2, the "midpoint" of the wheel.
      // This allows for the shortest movement since we can move in
      // either direction +/- from the middle, without having to go
      // possibly the long way around.
      //
      std::stringstream cmd;
      float remap;
      for ( auto fit = this->motion_info[filter].stepper.begin(); 
                 fit != this->motion_info[filter].stepper.end(); ++fit) {
        if ( fit->second.id == 2 ) remap = fit->second.position;          // remap = (position for id=2)
      }
      cmd.str(""); cmd << "RON " << axis << " 0";          this->send_command( cmd.str() );
      cmd.str(""); cmd << "POS " << axis << " " << remap;  this->send_command( cmd.str() );
      cmd.str(""); cmd << "RON " << axis << " 1";          this->send_command( cmd.str() );

      // Now move to the position with the calculated "newid"
      //
      float newpos=NAN;
      for ( auto fit = this->motion_info[filter].stepper.begin(); 
                 fit != this->motion_info[filter].stepper.end(); ++fit) {
        if ( fit->second.id == newid ) newpos = fit->second.position;     // newpos = (position for id=newid)
      }
      if ( ! std::isnan( newpos ) ) error = this->move_abs( filter, newpos );
      else {
        message.str(""); message << "ERROR: no position found for filter ID " << newid;
        logwrite( function, message.str() );
      }

      // Then map back to the correct order for the destination filter,
      // or to the current position if no position was found for newid,
      // which means we never moved.
      //
      remap = ( !std::isnan(newpos) ? destpos : currpos );

      cmd.str(""); cmd << "RON " << axis << " 0";          this->send_command( cmd.str() );
      cmd.str(""); cmd << "POS " << axis << " " << remap;  this->send_command( cmd.str() );
      cmd.str(""); cmd << "RON " << axis << " 1";          this->send_command( cmd.str() );
    }

    // Whether or not a filter was supplied, read the current position now.
    //
//  int addr = this->motion_info[ filter ].addr;
//  float pos;
//  error = this->pi.get_pos( addr, axis, pos );

    error = this->current_filter( retstring );

    return( error );
  }
  /***** Acam::MotionInterface::filter ****************************************/


  /***** Acam::MotionInterface::current_filter ********************************/
  /**
   * @brief      get the current filter position name
   * @param[out] currname  return string contains the current position name
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long MotionInterface::current_filter( std::string &currname ) {
    std::string function = "Acam::MotionInterface::currernt_filter";
    int currid;
    float currpos;
    long error = this->current_filter( currname, currid, currpos );
    return( error );
  }
  /***** Acam::MotionInterface::current_filter ********************************/


  /***** Acam::MotionInterface::current_filter ********************************/
  /**
   * @brief      get the current filter position name
   * @param[out] currname  contains the current position name
   * @param[out] currid    contains the current position id
   * @param[out] currpos   contains the current position position
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long MotionInterface::current_filter( std::string &currname, int &currid, float &currpos ) {
    std::string function = "Acam::MotionInterface::currernt_filter";
    std::string filter = "filter";
    std::stringstream message;
    long error = NO_ERROR;
    float tolerance = 2;

    // Connection must be open to motion component
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // Read the current position now.
    //
    int addr = this->motion_info[ filter ].addr;
    int axis = 1;
    float pos;

    error = this->pi.get_pos( addr, axis, pos );

    currname="";
    currid=-1;
    currpos=-1;

    for ( auto fit = this->motion_info[filter].stepper.begin(); 
               fit != this->motion_info[filter].stepper.end(); ++fit) {
      if ( std::abs( fit->second.position - pos < tolerance ) ) {
        currname = fit->second.posname;
        currid   = fit->second.id;
        currpos  = fit->second.position;
      }
    }

    if ( currname.empty() ) {
      message.str(""); message << "ERROR: no matching filter found for position=" << pos;
      logwrite( function, message.str() );
      error = ERROR;
    }

    return( error );
  }
  /***** Acam::MotionInterface::current_filter ********************************/


  /***** Acam::MotionInterface::cover *****************************************/
  /**
   * @brief      set or get the cover position
   * @param[in]  posname    string containing { "open" | "close" }
   * @param[out] retstring  return string contains the current position name
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::cover( std::string posname, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::cover";
    std::string cover = "cover";
    std::stringstream message;
    float tolerance = 2;
    long error = NO_ERROR;

    // Help
    //
    if ( posname == "?" ) {
      retstring = ACAMD_COVER;
      retstring.append( " [ open | close ]\n" );
      retstring.append( "  Move cover to position open | close. returns open|closed\n" );
      retstring.append( "  If no arg provided, return only the current position { open | closed }\n" );
      return( NO_ERROR );
    }

    // Connection must be open to motion component
    //
    if ( !this->isopen() ) {
      logwrite( function, "ERROR: not connected to motor controller" );
      return( ERROR );
    }

    // Cover motor must be homed
    //
    std::string ishome;
    error = this->is_home( cover, ishome );
    if ( error != NO_ERROR ) return( error );
    else
    if ( error==NO_ERROR && ishome != "true" ) {
      logwrite( function, "ERROR: cover motor is not homed" );
      return( ERROR );
    }

    // If posname is supplied then move cover to that position.
    //
    if ( !posname.empty() ) {

      // Check that posname is found in the motion_info.stepper map
      //
      if ( this->motion_info[cover].stepper.find( posname ) == this->motion_info[cover].stepper.end() ) {
        message.str(""); message << "ERROR: position \"" << posname << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        retstring="";
        return( ERROR );
      }

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dest=" << this->motion_info[cover].stepper[posname].posname << " "
                               << "id="       << this->motion_info[cover].stepper[posname].id << " "
                               << "position=" << this->motion_info[cover].stepper[posname].position;
      logwrite( function, message.str() );
#endif

      this->move_abs( cover, this->motion_info[cover].stepper[posname].position );

    }

    // Whether or not a filter was supplied, read the current position now.
    //
    float pos;
    error = this->pi.get_pos( this->motion_info[cover].addr, 1, pos );

    // Loop through all cover positions to see if <pos> matches a known position
    //
    for ( auto cit = this->motion_info[ cover ].stepper.begin();
               cit != this->motion_info[ cover ].stepper.end(); ++cit ) {
      if ( std::abs( cit->second.position - pos ) < tolerance ) {
        retstring = cit->second.posname;
      }
    }

    return( error );
  }
  /***** Acam::MotionInterface::cover *****************************************/

}
