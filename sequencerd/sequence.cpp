#include "sequence.h"

namespace Sequencer {

  /**************** Sequencer::Sequence ***************************************/
  /**
   * @fn         Sequence
   * @brief      Sequence class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Sequence::Sequence() {
    this->seqstate.store( Sequencer::SEQ_STOPPED );
    this->ready_to_start = false;                /// the sequencer is not ready by default (needs nightly startup)
    this->runstate.store(Sequencer::STOPPED);    /// the sequencer run state is STOPPED by default
    this->tcs_timeout = 0;                       /// telescope move timeout (set by config file)
  }
  /**************** Sequencer::Sequence ***************************************/


  /**************** Sequencer::Sequence::set_seqstate_bit *********************/
  /**
   * @fn         set_seqstate_bit
   * @brief      atomically sets the requested bit in the seqstate word
   * @param[in]  state
   * @return     none
   *
   */
  void Sequence::set_seqstate_bit( uint32_t state ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::set_seqstate_bit";
    std::stringstream message;
    std::uint32_t oldstate = this->seqstate.load();
    std::uint32_t newstate =  oldstate | state;
    message.str(""); message << "[DEBUG] state changed from " << oldstate << " to " << newstate;
    logwrite( function, message.str() );
#endif
    this->seqstate.fetch_or( state );
  }
  /**************** Sequencer::Sequence::set_seqstate_bit *********************/


  /**************** Sequencer::Sequence::clr_seqstate_bit *********************/
  /**
   * @fn         clr_seqstate_bit
   * @brief      atomically clears the requested bit in the seqstate word
   * @param[in]  state
   * @return     none
   *
   */
  void Sequence::clr_seqstate_bit( uint32_t state ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::clr_seqstate_bit";
    std::stringstream message;
    std::uint32_t oldstate = this->seqstate.load();
    std::uint32_t newstate = oldstate & ~state;
    message.str(""); message << "[DEBUG] state changed from " << oldstate << " to " << newstate;
    logwrite( function, message.str() );
#endif
    this->seqstate.fetch_and( ~state );
  }
  /**************** Sequencer::Sequence::clr_seqstate_bit *********************/


  /**************** Sequencer::Sequence::get_seqstate *************************/
  /**
   * @fn         get_seqstate
   * @brief      atomically returns the seqstate word
   * @param[in]  none
   * @return     seqstate
   *
   */
  uint32_t Sequence::get_seqstate( ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::get_seqstate";
    std::stringstream message;
    message.str(""); message << "[DEBUG] seqstate is " << this->seqstate.load();
    logwrite( function, message.str() );
#endif
    return( this->seqstate.load() );
  }
  /**************** Sequencer::Sequence::get_seqstate *************************/


  /**************** Sequencer::Sequence::dothread_sequence_start **************/
  /**
   * @fn         thr_sequence_start
   * @brief      main sequence start thread
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   * This thread is spawned in response to the sequencer receiving the "start"
   * command. This thread will in turn spawn additional needed threads.
   *
   * This thread will run as long as the runstate is RUNNING
   *
   */
  void Sequence::dothread_sequence_start( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_sequence_start";
    std::stringstream message;
    std::string reply;

    // The daemon shouldn't try to start more than one instance of this but
    // the class has a mutex to guarantee singular access, so lock it now.
    // It will remain locked until it goes out of scope (when the thread ends).
    //
    std::lock_guard<std::mutex> start_lock (seq.start_mtx);

    logwrite( function, "starting" );

    // This is the main loop, runs forever until stopped, or an error occurs.
    //
    while ( seq.runstate.load() == Sequencer::RUNNING ) {

      message.str(""); message << "runstate=" << seq.runstate.load();
      logwrite( function, message.str() );

      // Get the next target from the database
      //
      TargetInfo::TargetState targetstate = seq.target.get_next();

      if ( targetstate == TargetInfo::TARGET_FOUND ) {                    // target found, get the threads going
#ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] target found, starting threads" );
#endif
      }
      else
      if ( targetstate == TargetInfo::TARGET_NOT_FOUND ) {                // no target found. wait 1 sec and try again
        usleep( 1000000 );
        continue;
      }
      else
      if ( targetstate == TargetInfo::TARGET_ERROR ) {                    // request stop on error
      }

      // get the threads going --
      //
      // These things can all be done in parallel, just have to sync up at the end.
      // Set the state bit before starting each thread, then
      // the thread will clear their bit when they complete
      //
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );    std::thread( dothread_slit_set, std::ref(seq) ).detach();
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_TCS );     std::thread( dothread_acquire_target, std::ref(seq) ).detach();
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );   std::thread( dothread_focus_set, std::ref(seq) ).detach();
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE ); std::thread( dothread_flexure_set, std::ref(seq) ).detach();
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );   std::thread( dothread_calibrator_set, std::ref(seq) ).detach();

      // set the SEQ_READY bit
      //
      seq.set_seqstate_bit( Sequencer::SEQ_READY );

      // Now that the threads are running, wait until they are all finished.
      // When the SEQ_READY bit is the only bit set then we are ready.
      //
      std::thread( dothread_wait_for_state, std::ref(seq), Sequencer::SEQ_READY ).detach();

      logwrite( function, "waiting on notification" );

      std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );

      while ( seq.thr_error.load() == NO_ERROR && seq.seqstate.load() != Sequencer::SEQ_READY ) {
        seq.cv.wait( wait_lock );
      }
/*
      long error;
      while ( seq.cv.wait_for( wait_lock, std::chrono::seconds(1))==std::cv_status::timeout ) {
        logwrite( function, "waiting" );
      }
*/
      if ( seq.thr_error.load() != NO_ERROR ) {
        logwrite( function, "one or more threads returned an error -- start aborted!" );
        seq.clr_seqstate_bit( Sequencer::SEQ_READY );
//TODO I probably have to change the run state here as well
        return;
      }

      logwrite( function, "ready!!! " );

      logwrite( function, " starting exposure ... " );
      usleep( 3000000 );


      if ( seq.runstate.load() == Sequencer::STOPPED ) {
        logwrite( function, "runstate is stopped. bye!" );
        break;
      }
      usleep( 1000000 );
    }

    logwrite( function, "terminated" );
    return;
  }
  /**************** Sequencer::Sequence::dothread_sequence_start **************/


  /**************** Sequencer::Sequence::dothread_slit_set ********************/
  /**
   * @fn         dothread_slit_set
   * @brief      sets the slit according to the parameters in the target entry
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_slit_set( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_slit_set";
    std::string reply;
    long error;
    logwrite( function, "" );
    std::stringstream slitcmd;

    // send the SET command to slitd
    //
    slitcmd << SLITD_SET << " " << seq.target.slitwidth << " " << seq.target.slitoffset;

    error = seq.slitd.send( slitcmd.str(), reply );

    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to set slit" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );

    return;
  }
  /**************** Sequencer::Sequence::dothread_slit_set ********************/


  /**************** Sequencer::Sequence::dothread_power_init ******************/
  /**
   * @fn         dothread_power_init
   * @brief      initializes the power system for control from the Sequencer
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_power_init( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_power_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if not connected to the power daemon then connect
    //
    if ( !seq.powerd.socket.isconnected() ) {
      logwrite( function, "connecting to power daemon" );
      error = seq.powerd.connect();                  // connect to the daemon
    }

    // Ask powerd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.powerd.send( POWERD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to network power switch" );
      error = seq.powerd.send( POWERD_OPEN, reply );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to initialize power control" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_POWER );

    return;
  }
  /**************** Sequencer::Sequence::dothread_power_init ******************/


  /**************** Sequencer::Sequence::dothread_slit_init *******************/
  /**
   * @fn         dothread_slit_init
   * @brief      initializes the slit for control from the Sequencer
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_slit_init( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_slit_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false, ishomed=false;

    // if slit not powered on then turn it on
    //
    // TODO

    // if not connected to the slit daemon then connect
    //
    if ( !seq.slitd.socket.isconnected() ) {
      logwrite( function, "connecting to slit daemon" );
      error = seq.slitd.connect();                  // connect to the daemon
    }

    // Ask slitd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.slitd.send( SLITD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to slit hardware" );
      error = seq.slitd.send( SLITD_OPEN, reply );
    }

    // Ask slitd if the slit motors are homed,
    //
    if ( error == NO_ERROR ) {
      error  = seq.slitd.send( SLITD_ISHOME, reply );
      error |= seq.parse_state( function, reply, ishomed );
    }

    // and send the HOME command to slitd if needed.
    //
    if ( error==NO_ERROR && !ishomed ) {
      logwrite( function, "sending home command" );
      error = seq.slitd.send( SLITD_HOME, reply );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to initialize slit control" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );

    return;
  }
  /**************** Sequencer::Sequence::dothread_slit_init *******************/


  /**************** Sequencer::Sequence::dothread_calib_init ******************/
  /**
   * @fn         dothread_calib_init
   * @brief      initializes the calibrator system for control from the Sequencer
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_calib_init( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_calib_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if required systems not powered on then turn them on
    //
    // TODO

    // if not connected to the calib daemon then connect
    //
    if ( !seq.calibd.socket.isconnected() ) {
      logwrite( function, "connecting to calib daemon" );
      error = seq.calibd.connect();                  // connect to the daemon
    }

    // Ask calibd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.calibd.send( CALIBD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to calib hardware" );
      error = seq.calibd.send( CALIBD_OPEN, reply );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to initialize calibrator" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );

    return;
  }
  /**************** Sequencer::Sequence::dothread_calib_init ******************/


  /**************** Sequencer::Sequence::dothread_tcs_init ********************/
  /**
   * @fn         dothread_tcs_init
   * @brief      initializes the tcs system for control from the Sequencer
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_tcs_init( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_tcs_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if required systems not powered on then turn them on
    //
    // TODO

    // if not connected to the tcs daemon then connect
    //
    if ( !seq.tcsd.socket.isconnected() ) {
      logwrite( function, "connecting to tcs daemon" );
      error = seq.tcsd.connect();                  // connect to the daemon
    }

    // Ask tcsd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.tcsd.send( TCSD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to tcs hardware" );
      error = seq.tcsd.send( TCSD_OPEN, reply );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to initialize TCS control" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCS );

    return;
  }
  /**************** Sequencer::Sequence::dothread_tcs_init ********************/


  /**************** Sequencer::Sequence::dothread_flexure_init ****************/
  /**
   * @fn         dothread_flexure_init
   * @brief      initializes the flexure system for control from the Sequencer
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_flexure_init( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_flexure_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if required systems not powered on then turn them on
    //
    // TODO

    // if not connected to the flexure daemon then connect
    //
    if ( !seq.flexured.socket.isconnected() ) {
      logwrite( function, "connecting to flexure daemon" );
      error = seq.flexured.connect();                  // connect to the daemon
    }

    // Ask flexured if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.flexured.send( FLEXURED_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to flexure hardware" );
      error = seq.flexured.send( FLEXURED_OPEN, reply );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to initialize flexure control" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE );

    return;
  }
  /**************** Sequencer::Sequence::dothread_flexure_init ****************/


  /**************** Sequencer::Sequence::dothread_focus_init ******************/
  /**
   * @fn         dothread_focus_init
   * @brief      initializes the focus system for control from the Sequencer
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_focus_init( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_focus_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if required systems not powered on then turn them on
    //
    // TODO

    // if not connected to the focus daemon then connect
    //
    if ( !seq.focusd.socket.isconnected() ) {
      logwrite( function, "connecting to focus daemon" );
      error = seq.focusd.connect();                  // connect to the daemon
    }

    // Ask focusd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.focusd.send( FOCUSD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to focus hardware" );
      error = seq.focusd.send( FOCUSD_OPEN, reply );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to initialize focus control" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );

    return;
  }
  /**************** Sequencer::Sequence::dothread_focus_init ******************/


  /**************** Sequencer::Sequence::dothread_camera_init *****************/
  /**
   * @fn         dothread_camera_init
   * @brief      initializes the camera system for control from the Sequencer
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_camera_init( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_camera_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if required systems not powered on then turn them on
    //
    // TODO

    // if not connected to the camera daemon then connect
    //
    if ( !seq.camerad.socket.isconnected() ) {
      logwrite( function, "connecting to camera daemon" );
      error = seq.camerad.connect();                  // connect to the daemon
    }

    // Ask camerad if hardware connection is open,
    //
    if (error==NO_ERROR) error = seq.camerad.send( CAMERAD_ISOPEN, reply );

    // and open it if necessary.
    //
    if (error==NO_ERROR) error = seq.parse_state( function, reply, isopen );
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to camera hardware" );
      error = seq.camerad.send( CAMERAD_OPEN, reply );
    }

    // send a bunch of stuff, will get cleaned up later
    // TODO
    //
    if (error==NO_ERROR) { logwrite( function, "loading firmware" );  error = seq.camerad.send( "load /home/developer/ss/DSP/SWIFT/sg2_48khz.lod", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting buffer");     error = seq.camerad.send( "buffer 1024 1024", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting geometry" );  error = seq.camerad.send( "geometry 1024 1024", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting exptime" );   error = seq.camerad.send( "exptime 5000", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting readmode" );  error = seq.camerad.send( "readout U1", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting useframes" ); error = seq.camerad.send( "useframes false", reply ); }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to initialize camera" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );

    return;
  }
  /**************** Sequencer::Sequence::dothread_camera_init *****************/


  /**************** Sequencer::Sequence::dothread_acquire_target **************/
  /**
   * @fn         dothread_acquire_target
   * @brief  
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_acquire_target( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_acquire_target";
    long error=NO_ERROR;

    // disable guiding
    //
    logwrite( function, "TODO: disable guiding" );  // TODO
    error = NO_ERROR;

    // send coordinates to TCS
    //
    if ( error == NO_ERROR ) {
      std::stringstream coords;
      std::string reply, ra, dec;

      // convert ra, dec to decimal
      // can't be NaN
      //
      if ( std::isnan( seq.radec_to_decimal( seq.target.ra,  ra  ) ) ||
           std::isnan( seq.radec_to_decimal( seq.target.dec, dec ) ) ) {
        logwrite( function, "ERROR: can't handle NaN value for RA, DEC" );
        seq.thr_error.fetch_or( ERROR );
        return;
      }

      coords << "COORDS " << ra << " " << dec << " 0 0 0 \"" << seq.target.name << "\"";
      error  = seq.tcsd.send( coords.str(), reply );
      int tcsvalue;
      if ( error == NO_ERROR ) error = seq.extract_tcs_value( reply, tcsvalue );
      if ( error == NO_ERROR ) error = seq.parse_tcs_generic( tcsvalue );
    }

    // send casangle
    //
    if ( error == NO_ERROR ) logwrite( function, "TODO send casangle" ); // TODO

    // Before entering loop waiting for telescope arrival,
    // get the current time (in seconds) to be used for timeout.
    //
    bool   arrived       = false;
    double clock_now     = get_clock_time();
    double clock_timeout = clock_now + seq.tcs_timeout;  // must arrive by this time

    // Wait for telescope to arrive...
    //
    while ( error==NO_ERROR && not arrived ) {
      std::string reply;
      error = seq.tcsd.send( "?MOTION", reply );
      if ( error == NO_ERROR) error = seq.parse_tcs_motion( reply );

      // before looping, check for a timeout
      //
      clock_now = get_clock_time();

      if ( clock_now > clock_timeout ) {
        error = ERROR;
        logwrite( function, "ERROR: timeout waiting for telescope to arrive" );
        break;
      }
      usleep( 5000000 );  // don't poll the TCS too fast  TODO use smaller number in real life
      arrived = true;     // force arrival for testing
    }

    if ( arrived ) {

      // trigger A&G acquire sequence
      //
      logwrite( function, "TODO: trigger A&G acquire" );  //TODO
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to acquire target" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCS );

    return;
  }
  /**************** Sequencer::Sequence::dothread_acquire_target **************/


  /**************** Sequencer::Sequence::dothread_focus_set *******************/
  /**
   * @fn         dothread_focus_set
   * @brief  
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_focus_set( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_focus_set";
    long error=NO_ERROR;

    logwrite( function, "delaying 5s for focus" );
    usleep( 5000000 );

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to set focus" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );
    return;
  }
  /**************** Sequencer::Sequence::dothread_focus_set *******************/


  /**************** Sequencer::Sequence::dothread_flexure_set *****************/
  /**
   * @fn         dothread_flexure_set
   * @brief  
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_flexure_set( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_flexure_set";
    long error=NO_ERROR;

    logwrite( function, "delaying 5s for flexure" );
    usleep( 5000000 );

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to set flexure control" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE );
    return;
  }
  /**************** Sequencer::Sequence::dothread_flexure_set *****************/


  /**************** Sequencer::Sequence::dothread_calibrator_set **************/
  /**
   * @fn         dothread_calibrator_set
   * @brief  
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_calibrator_set( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_calibrator_set";
    long error=NO_ERROR;
    logwrite( function, "delaying 6s for calibrator" );
    usleep( 6000000 );

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to set calibrator" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );
    return;
  }
  /**************** Sequencer::Sequence::dothread_calibrator_set **************/


  /**************** Sequencer::Sequence::dothread_wait_for_state **************/
  /**
   * @fn         dothread_wait_for_state
   * @brief      waits for sequence state to match the requested state
   * @param[in]  ref to Sequencer::Sequence object
   * @param[in]  state_in to wait for
   * @return     none
   *
   * This function is spawned in a thread. It waits forever until the seq.seqstate
   * matches the requested state_in, at which point it sends a cv.notify_all signal
   * to unblock all threads currently waiting on cv.
   *
   */
  void Sequence::dothread_wait_for_state( Sequencer::Sequence &seq, uint32_t state_in ) {
    std::string function = "Sequencer::Sequence::dothread_wait_for_state";
    std::stringstream message;
    std::string reason = "";
    message.str(""); message << "waiting for sequencer state = " << state_in;
    logwrite( function, message.str() );

    // wait forever
    // or until seqstate is the requested state,
    // or stopped,
    // or error.
    //
    while ( 1 ) {
      if ( seq.seqstate.load() == state_in ) { reason = "state reached"; break; }
      if ( seq.seqstate.load() == Sequencer::SEQ_STOPPED ) { reason = "stop requested"; break; }
      if ( seq.thr_error.load() == ERROR ) { reason = "thread signaled an error"; break; }
    }

    // done waiting so send notification
    //
    std::unique_lock<std::mutex> lck(seq.wait_mtx);
    message.str(""); message << reason << ": notifying threads to stop waiting";
    logwrite( function, message.str() );
    seq.cv.notify_all();

    return;
  }
  /**************** Sequencer::Sequence::dothread_wait_for_state **************/


  /**************** Sequencer::Sequence::startup ******************************/
  /**
   * @fn         startup
   * @brief      performs nightly startup
   * @param[in]  reference to Sequence object
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::startup( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::startup";
    std::stringstream message;
    std::string retstring;

    if ( seq.runstate.load() != STOPPED && seq.seqstate.load() != Sequencer::SEQ_STOPPED ) {
      logwrite( function, "ERROR: cannot run nightly startup while sequencer is running" );
      return( ERROR );
    }

    std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );  // create a mutex object for waiting

    // set the SEQ_READY bit
    //
    seq.set_seqstate_bit( Sequencer::SEQ_READY );

    // Everything (except TCS) needs the power control to be running 
    // so initialize the power control first.
    //
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_POWER );   std::thread( dothread_power_init, std::ref(seq) ).detach();

    std::thread( dothread_wait_for_state, std::ref(seq), Sequencer::SEQ_READY ).detach();

    logwrite( function, "waiting for power control to initialize" );

    while ( seq.thr_error.load() == NO_ERROR && seq.seqstate.load() != Sequencer::SEQ_READY ) {
      seq.cv.wait( wait_lock );
    }

    // Don't proceed unless power control initialized successfully
    //
    if ( seq.thr_error.load() == NO_ERROR ) {
      logwrite( function, "power control initialized" );
    }
    else {
      logwrite( function, "ERROR initializing power control. startup aborted" );
      this->ready_to_start = false;
      seq.clr_seqstate_bit( Sequencer::SEQ_READY );
      return( ERROR );
    }

    // The following can be done in parallel.
    // Set the state bit before starting each thread, then
    // the thread will clear their bit when they complete.
    //
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );    std::thread( dothread_slit_init, std::ref(seq) ).detach();
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );   std::thread( dothread_calib_init, std::ref(seq) ).detach();
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_TCS );     std::thread( dothread_tcs_init, std::ref(seq) ).detach();
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );  std::thread( dothread_camera_init, std::ref(seq) ).detach();
//  seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );   std::thread( dothread_focus_init, std::ref(seq) ).detach();
//  seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE ); std::thread( dothread_flexure_init, std::ref(seq) ).detach();

    // Now that the threads are running, wait until they are all finished.
    // When the SEQ_READY bit is the only bit set then we are ready.
    //
    std::thread( dothread_wait_for_state, std::ref(seq), Sequencer::SEQ_READY ).detach();

    logwrite( function, "waiting for init threads to complete" );

    while ( seq.thr_error.load() == NO_ERROR && seq.seqstate.load() != Sequencer::SEQ_READY ) {
      seq.cv.wait( wait_lock );
    }

    logwrite( function, "init threads completed" );

    // if any thread returned an error then we are not ready
    //
    if ( seq.thr_error.load() != NO_ERROR ) {
      logwrite( function, "one or more threads returned an error. startup failed" );
      this->ready_to_start = false;
      seq.clr_seqstate_bit( Sequencer::SEQ_READY );
      return( ERROR );
    }
    else {
      logwrite( function, "ready to start" );
      this->ready_to_start = true;
      return( NO_ERROR );
    }
  }
  /**************** Sequencer::Sequence::startup ******************************/


  /**************** Sequencer::Sequence::parse_state *************************/
  /**
   * @fn         parse_state
   * @brief      parse the true|false state from a string reply
   * @param[in]  whoami string is name of function using this
   * @param[in]  reply string
   * @param[out] ref to bool state
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::parse_state( std::string whoami, std::string reply, bool &state ) {
    std::string function = "Sequencer::Sequence::parse_state";
    std::stringstream message;

    // Tokenize the reply --
    // There should be a response of only two tokens, "<state> <err>"
    // where <state> = {true|false} and <err> = {DONE|ERROR}
    //
    std::vector<std::string> tokens;
    Tokenize( reply, tokens, " " );
    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR reading true|false state from string \"" << reply << "\" from " << whoami << ": expected two tokens";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // If we're still here then just 2 tokens, good.
    // Parse the tokens to get the homed state and set a flag if homing is needed..
    //
    try {
      if ( tokens.at(1) != "DONE" ) { 
        message.str(""); message << "ERROR reading state from " << whoami;
        logwrite( function, message.str() );
        return( ERROR );
      }
      if ( tokens.at(0) == "true" ) state = true;
      else
      if ( tokens.at(0) == "false" ) state = false;
      else {
        message.str(""); message << "ERROR unknown state \"" << tokens.at(0) << "\" from " << whoami << ": expected {true|false}";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "out of range parsing message \"" << reply << "\" from " << whoami << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    return( NO_ERROR );
  }
  /**************** Sequencer::Sequence::parse_state *************************/


  /**************** Sequencer::Sequence::radec_to_decimal ********************/
  /**
   * @fn         radec_to_decimal
   * @brief      convert string RA or DEC to decimal number
   * @param[in]  str_in, input string to convert
   * @return     double representation of string, or NaN on error
   *
   * Input string could be  HH:MM:SS.sss
   *                        HH MM SS.sss
   *                       ??DD:MM:SS.sss
   *                       ??DD MM SS.sss
   *                       ?? D:MM:SS.sss
   *                       ?? D MM SS.sss
   *
   * Convert the input string into a decimal (double) number, HH.hhh or ??DD.dddd
   *
   * If the string is empty or otherwise cannot be converted then return NaN.
   *
   */
  double Sequence::radec_to_decimal( std::string str_in ) {
    std::string dontcare;
    return( this->radec_to_decimal( str_in, dontcare ) );
  }
  /**************** Sequencer::Sequence::radec_to_decimal ********************/


  /**************** Sequencer::Sequence::radec_to_decimal ********************/
  /**
   * @fn         radec_to_decimal
   * @brief      convert string RA or DEC to decimal number
   * @param[in]  str_in, input string to convert
   * @param[out] retstring, string representation of return value
   * @return     double representation of string, or NaN on error
   *
   * This function is overloaded.
   * This version accepts a reference to a return string, to return a string
   * version of the decimal (double) return value.
   *
   */
  double Sequence::radec_to_decimal( std::string str_in, std::string &retstring ) {
    std::string function = "Sequencer::Sequence::radec_to_decimal";
    std::stringstream message;
    std::vector<std::string> tokens;
    double sign=1.0;

    // can't convert an empty string to a value other than NaN
    //
    if ( str_in.empty() ) {
      logwrite( function, "ERROR: empty input string returns NaN" );
      return( NAN );
    }

    // If there's a minus sign (-) in the input string then set the sign
    // multiplier negative, then remove the sign.
    //
    // This is done because tokenizing on space or colon would result in three separate
    // tokens (HH MM SS or DD MM SS) except for the case where the degree is a single 
    // digit, then it's possible that tokenizing " + D MM SS.sss" it could result in four 
    // tokens, "+", "D", "MM", "SS.sss" so determine the sign then get rid of it.
    //
    if ( str_in.find( '-' ) != std::string::npos ) sign = -1.0;
    str_in.erase( std::remove( str_in.begin(), str_in.end(), '-' ), str_in.end() );
    str_in.erase( std::remove( str_in.begin(), str_in.end(), '+' ), str_in.end() );

    Tokenize( str_in, tokens, " :" );  // tokenize on space or colon

    if ( tokens.size() != 3 ) {
      message.str(""); message << "expected 3 tokens but received " << tokens.size() << ": " << str_in;
      logwrite( function, message.str() );
      return( NAN );
    }

    double hh, mm, ss, dec;
    std::stringstream ret;
    try {
      hh = std::stod( tokens.at(0) );
      mm = std::stod( tokens.at(1) ) / 60.0;
      ss = std::stod( tokens.at(2) ) / 3600.0;
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "out of range parsing input string " << str_in << ": " << e.what();
      logwrite( function, message.str() );
      return( NAN );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "invalid argument parsing input string " << str_in << ": " << e.what();
      logwrite( function, message.str() );
      return( NAN );
    }

    dec = sign * hh + mm + ss;
    ret << std::fixed << std::setprecision(6) << dec;
    retstring = ret.str();

    return( dec );
  }
  /**************** Sequencer::Sequence::radec_to_decimal ********************/


  /**************** Sequencer::Sequence::extract_tcs_value *******************/
  /**
   * @fn         extract_tcs_value
   * @brief      extract the value from a tcsd reply <val> DONE|ERROR
   * @param[in]  reply, string from the tcs daemon
   * @param[out] value, reference to integer containing the extracted value
   * @return     ERROR or NO_ERROR
   *
   * Replies from tcsd will include the direct response from the TCS followed
   * by a space and DONE or ERROR. That reply is tokenized (to get the direct
   * TCS response) and returned.
   *
   */
  long Sequence::extract_tcs_value( std::string reply, int &value ) {
    std::string function = "Sequencer::Sequence::extract_tcs_value";
    std::stringstream message;
    std::vector<std::string> tokens;
    long error = ERROR;

    // reply string cannot be empty
    //
    if ( reply.empty() ) {
      logwrite( function, "ERROR: empty reply string" );
      value = TCS_UNDEFINED;
      return( ERROR );
    }

    Tokenize( reply, tokens, " " );

    // If there's only one token then it's either DONE|ERROR with no value,
    // or something really weird.
    //
    if ( tokens.size() == 1 ) {
      if ( tokens.at(0) == "DONE" ) {
        logwrite( function, "no return value from tcsd" );
        error = NO_ERROR;
      }
      else
      if ( tokens.at(0) == "ERROR" ) {
        logwrite( function, "error received from tcsd" );
        error = ERROR;
      }
      else {
        message.str(""); message << "unrecognized reply: " << reply << " from tcsd";
        logwrite( function, message.str() );
        error = ERROR;
      }
      value = TCS_UNDEFINED;
    }
    else

    // When all is right, expecting 2 tokens, 
    // <tcsreply> DONE
    //
    if ( tokens.size() == 2 ) {
      try {
        value = std::stoi( tokens.at(0) );
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR out of range parsing input string " << reply << ": " << e.what();
        logwrite( function, message.str() );
        value = TCS_UNDEFINED;
        return( ERROR );
      }
      catch( std::invalid_argument &e ) {
        message.str(""); message << "ERROR converting " << reply << " to integer: " << e.what();
        logwrite( function, message.str() );
        value = TCS_UNDEFINED;
        return( ERROR );
      }
      error = NO_ERROR;
    }

    // more than 2 tokens should be impossible but deal with it just in case
    //
    else {
      message.str(""); message << "ERROR: expected 2 but received " << tokens.size() << " tokens in reply: " << reply;
      logwrite( function, message.str() );
      value = TCS_UNDEFINED;
      error = ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] from reply \"" << reply << "\" extracted value: " << value << " error=" << error;
    logwrite( function, message.str() );
#endif

    return( error );
  }
  /**************** Sequencer::Sequence::extract_tcs_value *******************/


  /**************** Sequencer::Sequence::parse_tcs_generic *******************/
  /**
   * @fn         parse_tcs_generic
   * @brief      parses the generic tcs reply to most commands
   * @param[in]  reply, string from the tcs daemon
   * @return     ERROR or NO_ERROR
   *
   * Replies from tcsd will include the direct response from the TCS followed
   * by a space and DONE or ERROR. That reply is tokenized (to get the direct
   * TCS response) and parsed here.
   *
   */
  long Sequence::parse_tcs_generic( int value ) {
    std::string function = "Sequencer::Sequence::parse_tcs_generic";
    std::stringstream message;
    std::string tcsreply;
    std::vector<std::string> tokens;

    if ( value == 0 ) {
#ifdef LOGLEVEL_DEBUG
      logwrite( function, "[DEBUG] TCS successful completion" );
#endif
      return( NO_ERROR );
    }
    else {
      if ( value == -1 ) {
        logwrite( function, "ERROR: TCS unrecognized command" );
      }
      else
      if ( value == -2 ) {
        logwrite( function, "ERROR: TCS invalid parameter(s)" );
      }
      else
      if ( value == -3 ) {
        logwrite( function, "ERROR: TCS unable to execute" );
      }
      else
      if ( value == -4 ) {
        logwrite( function, "ERROR: TCS Sparc host unavailable" );
      }
      else {
        logwrite( function, "ERROR: unrecognized TCS value" );
      }
      return( ERROR );
    }
  }
  /**************** Sequencer::Sequence::parse_tcs_generic *******************/


  /**************** Sequencer::Sequence::parse_tcs_motion ********************/
  /**
   * @fn         parse_tcs_motion
   * @brief      parses the tcs reply to ?MOTION command
   * @param[in]  reply, string from the TCS
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::parse_tcs_motion( std::string reply ) {
    std::string function = "Sequencer::Sequence::parse_tcs_motion";
    std::stringstream message;
    return( NO_ERROR );
  }
  /**************** Sequencer::Sequence::parse_tcs_motion ********************/


  /**************** Sequencer::Sequence::test ********************************/
  /**
   * @fn         test
   * @brief      
   * @param[in]  
   * @param[out] 
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::test( std::string args, std::string &retstring ) {
    std::string function = "Sequencer::Sequence::test";
    std::stringstream message;
    std::vector<std::string> tokens;
    long error = NO_ERROR;

    Tokenize( args, tokens, " " );

    if ( tokens.size() < 1 ) {
      logwrite( function, "no test name provided" );
      return ERROR;
    }

    std::string testname = tokens[0];                                // the first token is the test name

    // ----------------------------------------------------
    // getnext -- get the next pending target from the database
    // ----------------------------------------------------
    //
    if ( testname == "seqstate" ) {
      error = this->get_seqstate();
      retstring = std::to_string( error );
      retstring.append( " " );
      error = NO_ERROR;
    }
    else

    // ----------------------------------------------------
    // getnext -- get the next pending target from the database
    // ----------------------------------------------------
    //
    if ( testname == "getnext" ) {
      TargetInfo::TargetState ret = this->target.get_next();
      error = NO_ERROR;
      if ( ret == TargetInfo::TargetState::TARGET_FOUND ) retstring = "found";
      else
      if ( ret == TargetInfo::TargetState::TARGET_NOT_FOUND ) retstring = "not_found";
      else
      if ( ret == TargetInfo::TargetState::TARGET_ERROR ) error = ERROR;
    }
    else

    // ----------------------------------------------------
    // addrow -- insert a (fixed, hard-coded) row into the database
    // ----------------------------------------------------
    //
    if ( testname == "addrow" ) {
      error = this->target.add_row();   // 
    }
    else

    // ----------------------------------------------------
    // update -- update the target state of the current row
    // ----------------------------------------------------
    //
    if ( testname == "update" ) {
      if ( tokens.size() < 2 ) {
        logwrite( function, "update needs a state: { pending | complete | unassigned }" );
        return( ERROR );
      }
      else {
        if ( tokens[1] != "pending" && tokens[1] != "complete" && tokens[1] != "unassigned" ) {
          logwrite( function, "update expected { pending | complete | unassigned }" );
          return( ERROR );
        }
        error = this->target.update_state( tokens[1] );   // 
      }
    }
    else

    // ----------------------------------------------------
    // radec -- 
    // ----------------------------------------------------
    //
    if ( testname == "radec" ) {
      double ra,dec;
      ra  = this->radec_to_decimal( target.ra );
      dec = this->radec_to_decimal( target.dec );
      message.str(""); message << "ra " << target.ra << " -> " 
                               << std::fixed << std::setprecision(6) 
                               << ra << "  dec " << target.dec << " -> " << dec;
      logwrite( function, message.str() );
    }

    // ----------------------------------------------------
    // invalid test name
    // ----------------------------------------------------
    //
    else {
      message.str(""); message << "ERROR: test " << testname << " unknown";;
      logwrite(function, message.str());
      error = ERROR;
    }

    return( error );
  }
  /**************** Sequencer::Sequence::test ********************************/
}
