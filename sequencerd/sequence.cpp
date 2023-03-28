/**
 * @file    sequence.cpp
 * @brief   code for the Sequence class, the guts of the actual sequence functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Sequencer::Sequence class functions;
 * these are the functions which perform the sequence operations, called by
 * the sequencer daemon.
 *
 * For example, the nightly startup and the start sequences are contained herein.
 *
 */

#include "sequence.h"

namespace Sequencer {

  /***** Sequencer::Sequence **************************************************/
  /**
   * @brief      Sequence class constructor
   *
   */
  Sequence::Sequence() {
    this->seqstate.store( Sequencer::SEQ_OFFLINE );
    this->reqstate.store( Sequencer::SEQ_OFFLINE );
    this->do_once.store( false );                    /// default to "do all"
    this->waiting_for_state.store( false );          /// not currently waiting for a state
    this->ready_to_start = false;                    /// the sequencer is not ready by default (needs nightly startup)
    this->notify_tcs_next_target = false;            /// default do not notify TCS of next target before end of exposure
    this->tcs_preauth_time = 0;                      /// default disable notifing TCS of next target's coords before exposure end
    this->tcs_settle_timeout = 0;                    /// telescope settling timeout (set by config file)
    this->tcs_offsetrate_ra = 45;                    /// TCS offset rate RA in arcsec per second
    this->tcs_offsetrate_dec = 45;                   /// TCS offset rate DEC in arcsec per second
    this->acquisition_timeout = 0;                   /// acquisition timeout (set by config file)
    this->acquisition_max_retrys = -1;               /// no retry limit by default (-1 disables) but can override in config file
    this->arm_readout_flag = false;                  /// disarm the readout flag to ignore async messages

    // Initializes the STL map of bit-number-to-string for the SequenceStateBits.
    // This map is used to obtain a human-friendly string of the bits which have been set in seqstate
    //
    this->sequence_state_bits.push_back( OFFLINE_BIT ); this->sequence_states[ OFFLINE_BIT  ] = "OFFLINE";
    this->sequence_state_bits.push_back( ABORTREQ_BIT );this->sequence_states[ ABORTREQ_BIT ] = "ABORTREQ";
    this->sequence_state_bits.push_back( STOPREQ_BIT ); this->sequence_states[ STOPREQ_BIT  ] = "STOPREQ";
    this->sequence_state_bits.push_back( READY_BIT );   this->sequence_states[ READY_BIT    ] = "READY";
    this->sequence_state_bits.push_back( RUNNING_BIT ); this->sequence_states[ RUNNING_BIT  ] = "RUNNING";
    this->sequence_state_bits.push_back( SHUTTING_BIT );this->sequence_states[ SHUTTING_BIT ] = "SHUTTING";
    this->sequence_state_bits.push_back( PAUSE_BIT );   this->sequence_states[ PAUSE_BIT    ] = "PAUSED";
    this->sequence_state_bits.push_back( STARTING_BIT); this->sequence_states[ STARTING_BIT ] = "STARTING";
    this->sequence_state_bits.push_back( ACAM_BIT );    this->sequence_states[ ACAM_BIT     ] = "ACAM";
    this->sequence_state_bits.push_back( ACQUIRE_BIT ); this->sequence_states[ ACQUIRE_BIT  ] = "ACQUIRE";
    this->sequence_state_bits.push_back( CALIB_BIT );   this->sequence_states[ CALIB_BIT    ] = "CALIB";
    this->sequence_state_bits.push_back( CAMERA_BIT );  this->sequence_states[ CAMERA_BIT   ] = "CAMERA";
    this->sequence_state_bits.push_back( EXPOSE_BIT );  this->sequence_states[ EXPOSE_BIT   ] = "EXPOSING";
    this->sequence_state_bits.push_back( READOUT_BIT ); this->sequence_states[ READOUT_BIT  ] = "READOUT";
    this->sequence_state_bits.push_back( FILTER_BIT );  this->sequence_states[ FILTER_BIT   ] = "FILTER";
    this->sequence_state_bits.push_back( FLEXURE_BIT ); this->sequence_states[ FLEXURE_BIT  ] = "FLEXURE";
    this->sequence_state_bits.push_back( FOCUS_BIT );   this->sequence_states[ FOCUS_BIT    ] = "FOCUS";
    this->sequence_state_bits.push_back( POWER_BIT );   this->sequence_states[ POWER_BIT    ] = "POWER";
    this->sequence_state_bits.push_back( SLIT_BIT );    this->sequence_states[ SLIT_BIT     ] = "SLIT";
    this->sequence_state_bits.push_back( TCS_BIT );     this->sequence_states[ TCS_BIT      ] = "TCS";
    this->sequence_state_bits.push_back( TCSOP_BIT );   this->sequence_states[ TCSOP_BIT    ] = "TCSOP";
    this->sequence_state_bits.push_back( SETTLE_BIT );  this->sequence_states[ SETTLE_BIT   ] = "SETTLE";
    this->sequence_state_bits.push_back( SLEW_BIT );    this->sequence_states[ SLEW_BIT     ] = "SLEW";

    // Initializes the STL map of bit-number-to-string for the ThreadStatusBits.
    // This map is used to obtain a human-friendly string of which threads are running.
    // There should be a 1:1 correlation between these assignments and the ThreadStatusBits enum in sequence.h
    //
    this->thread_state_bits.push_back( ASYNCLISTENER_BIT ); this->thread_states[ ASYNCLISTENER_BIT ] = "async_listener";
    this->thread_state_bits.push_back( TRIGEXPO_BIT );      this->thread_states[ TRIGEXPO_BIT ]      = "trigger_exposure";
    this->thread_state_bits.push_back( WAITFORSTATE_BIT );  this->thread_states[ WAITFORSTATE_BIT ]  = "wait_for_state";
    this->thread_state_bits.push_back( SEQSTART_BIT );      this->thread_states[ SEQSTART_BIT ]      = "sequence_start";
    this->thread_state_bits.push_back( CALIBSET_BIT );      this->thread_states[ CALIBSET_BIT ]      = "calib_set";
    this->thread_state_bits.push_back( CAMERASET_BIT );     this->thread_states[ CAMERASET_BIT ]     = "camera_set";
    this->thread_state_bits.push_back( SLITSET_BIT );       this->thread_states[ SLITSET_BIT ]       = "slit_set";
    this->thread_state_bits.push_back( MOVETOTARGET_BIT );  this->thread_states[ MOVETOTARGET_BIT ]  = "move_to_target";
    this->thread_state_bits.push_back( NOTIFYTCS_BIT );     this->thread_states[ NOTIFYTCS_BIT ]     = "notify_tcs";
    this->thread_state_bits.push_back( FOCUSSET_BIT );      this->thread_states[ FOCUSSET_BIT ]      = "focus_set";
    this->thread_state_bits.push_back( FLEXURESET_BIT );    this->thread_states[ FLEXURESET_BIT ]    = "flexure_set";
    this->thread_state_bits.push_back( CALIBRATORSET_BIT ); this->thread_states[ CALIBRATORSET_BIT ] = "calibrator_set";
    this->thread_state_bits.push_back( ACAMINIT_BIT );      this->thread_states[ ACAMINIT_BIT ]      = "acam_init";
    this->thread_state_bits.push_back( CALIBINIT_BIT );     this->thread_states[ CALIBINIT_BIT ]     = "calib_init";
    this->thread_state_bits.push_back( TCSINIT_BIT );       this->thread_states[ TCSINIT_BIT ]       = "tcs_init";
    this->thread_state_bits.push_back( SLITINIT_BIT );      this->thread_states[ SLITINIT_BIT ]      = "slit_init";
    this->thread_state_bits.push_back( CAMERAINIT_BIT );    this->thread_states[ CAMERAINIT_BIT ]    = "camera_init";
    this->thread_state_bits.push_back( FLEXUREINIT_BIT );   this->thread_states[ FLEXUREINIT_BIT ]   = "flexure_init";
    this->thread_state_bits.push_back( FOCUSINIT_BIT );     this->thread_states[ FOCUSINIT_BIT ]     = "focus_init";
    this->thread_state_bits.push_back( POWERINIT_BIT );     this->thread_states[ POWERINIT_BIT ]     = "power_init";
    this->thread_state_bits.push_back( ACAMSTOP_BIT );      this->thread_states[ ACAMSTOP_BIT ]      = "acam_shutdown";
    this->thread_state_bits.push_back( CALIBSTOP_BIT );     this->thread_states[ CALIBSTOP_BIT ]     = "calib_shutdown";
    this->thread_state_bits.push_back( POWERSTOP_BIT );     this->thread_states[ POWERSTOP_BIT ]     = "power_shutdown";
    this->thread_state_bits.push_back( MODEXPTIME_BIT );    this->thread_states[ MODEXPTIME_BIT ]    = "modify_exptime";
    this->thread_state_bits.push_back( ACQUISITION_BIT );   this->thread_states[ ACQUISITION_BIT ]   = "acquisition";
    this->thread_state_bits.push_back( GUIDE_BIT );         this->thread_states[ GUIDE_BIT ]         = "guide";
    this->thread_state_bits.push_back( STARTUP_BIT );       this->thread_states[ STARTUP_BIT ]       = "startup";
    this->thread_state_bits.push_back( SHUTDOWN_BIT );      this->thread_states[ SHUTDOWN_BIT ]      = "shutdown";
  }
  /***** Sequencer::Sequence **************************************************/


  /***** Sequencer::Sequence::dothread_daemon_command *************************/
  /**
   * @brief      sends a command to a daemon in a thread
   * @param[in]  daemon  reference to Sequencer::Daemon object
   * @param[in]  args    string containing command and any arguments
   *
   * This thread is spawned in response to the sequencer receiving a command
   * for any daemon. The sequencer returns immediately and this thread sends
   * the command to the daemon. Since there is no waiting for this command to
   * complete, the caller must monitor the UDP broadcast message port for any
   * potential status.
   *
   */
  void Sequence::dothread_daemon_command( Sequencer::Daemon &daemon, std::string args ) {
    daemon.command( args );
    return;
  }
  /***** Sequencer::Sequence::dothread_daemon_command *************************/


  /***** Sequencer::Sequence::set_seqstate_bit ********************************/
  /**
   * @brief      atomically sets the requested bit in the seqstate word
   * @param[in]  mb  masked bit is uint32 word containing the bit to set
   *
   * The new state will be written to the async message port
   *
   */
  void Sequence::set_seqstate_bit( uint32_t mb ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::set_seqstate_bit";
    std::stringstream debugmessage;
    std::uint32_t oldstate = this->seqstate.load();
    std::uint32_t newstate =  oldstate | mb;
    debugmessage << "[DEBUG] state changed from " << oldstate << " to " << newstate
                 << ": " << this->seqstate_string( newstate );
    logwrite( function, debugmessage.str() );
#endif
    this->seqstate.fetch_or( mb );    // clear
    this->report_seqstate();          // report current state
  }
  /***** Sequencer::Sequence::set_seqstate_bit ********************************/


  /***** Sequencer::Sequence::clr_seqstate_bit ********************************/
  /**
   * @brief      atomically clears the requested bit in the seqstate word
   * @param[in]  mb  masked bit is uint32 word containing the bit to clear
   *
   * The new state will be written to the async message port
   *
   */
  void Sequence::clr_seqstate_bit( uint32_t mb ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::clr_seqstate_bit";
    std::stringstream debugmessage;
    std::uint32_t oldstate = this->seqstate.load();
    std::uint32_t newstate = oldstate & ~mb;
    debugmessage << "[DEBUG] state changed from " << oldstate << " to " << newstate
                 << ": " << this->seqstate_string( newstate );
    logwrite( function, debugmessage.str() );
#endif
    this->seqstate.fetch_and( ~mb );  // clear
    this->report_seqstate();          // report current state
  }
  /***** Sequencer::Sequence::clr_seqstate_bit ********************************/


  /***** Sequencer::Sequence::set_clr_seqstate_bits ***************************/
  /**
   * @brief      atomically sets and clear the requested bits in the seqstate word
   * @param[in]  sb  masked bit is uint32 word containing the bit(s) to set
   * @param[in]  mb  masked bit is uint32 word containing the bit(s) to clear
   *
   * The new state will be written to the async message port
   *
   */
  void Sequence::set_clr_seqstate_bit( uint32_t sb, uint32_t cb ) {
    this->seqstate.fetch_or( sb );    // set
    this->seqstate.fetch_and( ~cb );  // clear
    this->report_seqstate();          // report current state
  }
  /***** Sequencer::Sequence::set_clr_seqstate_bits ***************************/


  /***** Sequencer::Sequence::report_seqstate *********************************/
  /**
   * @brief      logs and writes the seqstate string to the async port
   * @return     seqstate string
   *
   */
  std::string Sequence::report_seqstate() {
    std::stringstream message;
    uint32_t ss = this->seqstate.load();
    message << "RUNSTATE: " << this->seqstate_string( ss );
    this->async.enqueue( message.str() );

//  uint32_t rs = this->reqstate.load();
//  message << " | REQSTATE: " << this->seqstate_string( rs );
//
//  uint32_t ts = this->thrstate.load();
//  message << " | THREADS: " << this->thrstate_string( ts );
//  logwrite( "Sequencer::Sequence::report_seqstate", message.str() );

    return( message.str() );
  }
  /***** Sequencer::Sequence::report_seqstate *********************************/


  /***** Sequencer::Sequence::get_seqstate ************************************/
  /**
   * @brief      atomically returns the seqstate word
   * @return     seqstate
   *
   */
  uint32_t Sequence::get_seqstate( ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::get_seqstate";
    std::stringstream message;
    message.str(""); message << "[DEBUG] seqstate is " << this->seqstate.load()
                             << ": " << this->seqstate_string( this->seqstate.load() );
    logwrite( function, message.str() );
#endif
    return( this->seqstate.load() );
  }
  /***** Sequencer::Sequence::get_seqstate ************************************/


  /***** Sequencer::Sequence::set_reqstate_bit ********************************/
  /**
   * @brief      atomically sets the requested bit in the reqstate word
   * @param[in]  mb  masked bit is uint32 word containing the bit to set
   *
   */
  void Sequence::set_reqstate_bit( uint32_t mb ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::set_reqstate_bit";
    std::stringstream message;
    std::uint32_t oldstate = this->reqstate.load();
    std::uint32_t newstate =  oldstate | mb;
    message.str(""); message << "[DEBUG] state changed from " << oldstate << " to " << newstate
                             << ": " << this->seqstate_string( newstate );
    logwrite( function, message.str() );
#endif
    this->reqstate.fetch_or( mb );
  }
  /***** Sequencer::Sequence::set_reqstate_bit ********************************/


  /***** Sequencer::Sequence::clr_reqstate_bit ********************************/
  /**
   * @brief      atomically clears the requested bit in the reqstate word
   * @param[in]  mb  masked bit is uint32 word containing the bit to clear
   *
   */
  void Sequence::clr_reqstate_bit( uint32_t mb ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::clr_reqstate_bit";
    std::stringstream message;
    std::uint32_t oldstate = this->reqstate.load();
    std::uint32_t newstate = oldstate & ~mb;
    message.str(""); message << "[DEBUG] state changed from " << oldstate << " to " << newstate
                             << ": " << this->seqstate_string( newstate );
    logwrite( function, message.str() );
#endif
    this->reqstate.fetch_and( ~mb );
  }
  /***** Sequencer::Sequence::clr_reqstate_bit ********************************/


  /***** Sequencer::Sequence::get_reqstate ************************************/
  /**
   * @brief      atomically returns the reqstate word
   * @return     reqstate
   *
   */
  uint32_t Sequence::get_reqstate( ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::get_reqstate";
    std::stringstream message;
    message.str(""); message << "[DEBUG] reqstate is " << this->reqstate.load();
    logwrite( function, message.str() );
#endif
    return( this->reqstate.load() );
  }
  /***** Sequencer::Sequence::get_reqstate ************************************/


  /***** Sequencer::Sequence::dothread_sequencer_async_listener ***************/
  /**
   * @brief      async message listening thread
   * @param[in]  seq  reference to Sequencer::Sequence object
   * @param[in]  udp  UDP socket object, pre-configured
   *
   * This thread allows the sequencer to listen for asynchronous messages.
   * The UDP socket object that is passed in must have already been configured
   * with the async message port and message group, which would have been read
   * from a config file by the sequencer daemon.
   *
   * This thread never terminates unless there is an error.
   *
   * This thread listens for the following asynchronous message tags:
   *    ELAPSEDTIME
   *
   */
  void Sequence::dothread_sequencer_async_listener( Sequencer::Sequence &seq, Network::UdpSocket udp ) {
    seq.set_thrstate_bit( THR_SEQUENCER_ASYNC_LISTENER );
    std::string function = "Sequencer::Sequence::dothread_sequencer_async_listener";
    std::stringstream message;

    int retval = udp.Listener();

    if ( retval < 0 ) {
      logwrite(function, "error creating UDP listening socket. thread terminating.");
      seq.clr_thrstate_bit( THR_SEQUENCER_ASYNC_LISTENER );
      return;
    }

    logwrite( function, "running" );

    // forever receive and process UDP async status messages
    //
    while ( true ) {

//    if ( not seq.is_seqstate_set( Sequencer::SEQ_RUNNING | Sequencer::SEQ_STOPREQ | Sequencer::SEQ_ABORTREQ ) ) continue;  // don't check anything if system is idle

      // Receive the UDP message
      //
      std::string statstr="";
      udp.Receive( statstr );

      // Act on the message
      //
      try {

        // ---------------------------------------------------------------------------------------
        // NOTIFY TCS OPERATOR OF NEXT TARGET COORDS FOR PREAUTHORIZATION PRIOR TO END OF EXPOSURE
        // ---------------------------------------------------------------------------------------
        //
        // Monitor the elapsed exposure time. When the remaining time is within TCS_PREAUTH_TIME sec
        // of the end of the exposure time, then alert the TCS operator of the next
        // target's coordinates. notify_tcs_next_target has to be set true by dothread_trigger_exposure().
        //
        // Since the only purpose of this is to alert the TCS near the end of the exposure, it is
        // not important to differentiate which camera's elapsed time that is being read here;
        // they are all going to be close enough for this purpose.
        //
        if ( seq.notify_tcs_next_target && statstr.compare( 0, 11, "ELAPSEDTIME" ) == 0 ) {     // async message tag ELAPSEDTIME
          std::string::size_type pos = statstr.find( ":" );
          std::string elapsedstr = statstr.substr( pos + 1 );
          double remaining = (double)( seq.target.exptime - stol( elapsedstr )/1000. );

          if ( remaining <= seq.tcs_preauth_time ) {
            std::thread( dothread_notify_tcs, std::ref(seq) ).detach();
            seq.notify_tcs_next_target = false;  // don't do it again until the next exposure
          }

          if ( remaining == 0 ) {
///< @todo can I use this to detect when "Exposing" has ended?
          }
        }

        // ------------------------------------------------------------------
        // Set READOUT flag and clear EXPOSE flag when pixels start coming in
        // ------------------------------------------------------------------
        //
        if ( seq.arm_readout_flag && statstr.compare( 0, 10, "PIXELCOUNT" ) == 0 ) {            // async message tag PIXELCOUNT
          seq.arm_readout_flag = false;
          seq.set_clr_seqstate_bit( Sequencer::SEQ_WAIT_READOUT, Sequencer::SEQ_WAIT_EXPOSE );  // set READOUT, clear EXPOSE
        }

        // ---------------------------------------------
        // clear READOUT flag on the end-of-frame signal
        // ---------------------------------------------
        //
        if ( statstr.compare( 0, 10, "FRAMECOUNT" ) == 0 ) {                                    // async message tag FRAMECOUNT
          if ( seq.is_seqstate_set( Sequencer::SEQ_WAIT_READOUT ) ) {
            seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_READOUT );
          }
        }

      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "out of range parsing status string " << statstr << ": " << e.what();
        logwrite( function, message.str() );
      }
      catch( std::invalid_argument &e ) {
        message.str(""); message << "invalid argument parsing status string " << statstr << ": " << e.what();
        logwrite( function, message.str() );
      }

    }
    seq.clr_thrstate_bit( THR_SEQUENCER_ASYNC_LISTENER );
    return;
  }
  /***** Sequencer::Sequence::dothread_sequencer_async_listener ***************/


  /***** Sequencer::Sequence::dothread_sequence_start *************************/
  /**
   * @brief      main sequence start thread
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * This thread is spawned in response to the sequencer receiving the "start"
   * command. This thread will in turn spawn additional needed threads.
   *
   * This thread will run as long as the seqstate is RUNNING
   *
   */
  void Sequence::dothread_sequence_start( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_SEQUENCE_START );
    std::string function = "Sequencer::Sequence::dothread_sequence_start";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // The daemon shouldn't try to start more than one instance of this but
    // the class has a mutex to guarantee singular access, so lock it now.
    // It will remain locked until it goes out of scope (when the thread ends).
    //
    std::lock_guard<std::mutex> start_lock (seq.start_mtx);

    // clear the thread error state
    //
    seq.thr_error.store( THR_NONE );

    // This is the main loop which runs as long as seqstate is 
    // has the SEQ_RUNNING bit set and neither ABORTREQ nor STOPREQuested bits set.
    //
    while ( seq.is_seqstate_set( Sequencer::SEQ_RUNNING ) &&
          ( not seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) || 
            not seq.is_seqstate_set( Sequencer::SEQ_STOPREQ ) ) ) {

      // If the ABORTREQ or STOPREQ bits are set then break the while loop
      //
      if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ||
           seq.is_seqstate_set( Sequencer::SEQ_STOPREQ ) ) {
        logwrite( function, "stopping" );
        break;
      }

      logwrite( function, "sequencer running" );

      // Get the next target from the database
      //
      TargetInfo::TargetState targetstate = seq.target.get_next();

      if ( targetstate == TargetInfo::TARGET_FOUND ) {                    // target found, get the threads going
        error = seq.target.update_state( Sequencer::TARGET_ACTIVE );      // set ACTIVE state in database (this says we are using this target)
        if (error!=NO_ERROR) {
          seq.thr_error.fetch_or( THR_SEQUENCE_START );                   // report any error
          break;
        }

        // let the world know of the state change
        //
        message.str(""); message << "TARGETSTATE:" << seq.target.state << " TARGET:" << seq.target.name << " OBSID:" << seq.target.obsid;
        seq.async.enqueue( message.str() );
#ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] target found, starting threads" );
#endif
      }
      else
      if ( targetstate == TargetInfo::TARGET_NOT_FOUND ) {                // no target found is an automatic stop
        logwrite( function, "no targets found. stopping" );
        seq.set_reqstate_bit( Sequencer::SEQ_STOPREQ );
        seq.set_seqstate_bit( Sequencer::SEQ_STOPREQ );
        break;
      }
      else
      if ( targetstate == TargetInfo::TARGET_ERROR ) {                    // request stop on error
        seq.async.enqueue_and_log( function, "ERROR getting next target. stopping" );
        seq.set_seqstate_bit( Sequencer::SEQ_STOPREQ );
        seq.set_reqstate_bit( Sequencer::SEQ_STOPREQ );
        break;
      }

      // get the threads going --
      //
      // These things can all be done in parallel, just have to sync up at the end.
      // Set the state bit before starting each thread, then
      // the thread will clear their bit when they complete
      //
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );  std::thread( dothread_camera_set, std::ref(seq) ).detach();
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );    std::thread( dothread_slit_set, std::ref(seq) ).detach();
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_TCS );     std::thread( dothread_move_to_target, std::ref(seq) ).detach();
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );   std::thread( dothread_focus_set, std::ref(seq) ).detach();
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE ); std::thread( dothread_flexure_set, std::ref(seq) ).detach();
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );   std::thread( dothread_calib_set, std::ref(seq) ).detach();

      // Now that the threads are running, wait until they are all finished.
      // When the SEQ_RUNNING bit is the only bit set then we are ready.
      //
      seq.set_reqstate_bit( Sequencer::SEQ_RUNNING );                  // set the requested state bit
      std::thread( dothread_wait_for_state, std::ref(seq) ).detach();  // wait for requested state

      logwrite( function, "[DEBUG] (1) waiting on notification" );

      std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );  // create a mutex object for waiting

      while ( seq.seqstate.load() != seq.reqstate.load() ) {
        message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
        logwrite( function, message.str() );
        seq.cv.wait( wait_lock );
      }
      logwrite( function, "[DEBUG] (1) DONE waiting on notification" );
/*
      long error;
      while ( seq.cv.wait_for( wait_lock, std::chrono::seconds(1))==std::cv_status::timeout ) {
        logwrite( function, "waiting" );
      }
*/
      // Now that we're done waiting, check for errors or abort
      //
      uint32_t te = seq.thr_error.load();
      if ( te != THR_NONE ) {
        message.str(""); message << "ERROR the following thread(s) had an error: " << seq.thrstate_string( te );
        logwrite( function, message.str() );
        seq.thr_error.store( THR_NONE );  // clear the thread error state
        break;
      }

/*** I think this can cause a race condition
 *    if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {
 *      logwrite( function, "ABORT requested" );
 *      seq.set_clr_seqstate_bit( Sequencer::SEQ_READY, ( Sequencer::SEQ_ABORTREQ | Sequencer::SEQ_RUNNING ) );  // set and clear together
 *      seq.clr_reqstate_bit( ( Sequencer::SEQ_ABORTREQ | Sequencer::SEQ_RUNNING ) );
 *      seq.set_reqstate_bit( Sequencer::SEQ_READY );
 *      break;
 *    }
 */
      if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {
        logwrite( function, "aborting the start sequence" );
        break;
      }

      logwrite( function, "starting exposure" );       ///< TODO @todo log to telemetry!

      // Start the exposure in a thread...
      //
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );                // set the current state
      std::thread( dothread_trigger_exposure, std::ref(seq) ).detach();  // trigger exposure in a thread

      seq.set_reqstate_bit( Sequencer::SEQ_RUNNING );                    // set the requested state
      std::thread( dothread_wait_for_state, std::ref(seq) ).detach();    // wait for requested state

      // ...then wait for it to complete
      //
      logwrite( function, "[DEBUG] (2) waiting on notification" );
      while ( seq.seqstate.load() != seq.reqstate.load() ) {
        message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
        logwrite( function, message.str() );
        seq.cv.wait( wait_lock );
      }
      logwrite( function, "[DEBUG] (2) DONE waiting on notification" );

      // Now that we're done waiting, check for errors or abort
      //
      te = seq.thr_error.load();
      if ( te != THR_NONE ) {
        message.str(""); message << "ERROR the following thread(s) had an error: " << seq.thrstate_string( te );
        logwrite( function, message.str() );
        seq.thr_error.store( THR_NONE );  // clear the thread error state
        break;
      }

      // When an exposure is aborted then it will be marked as UNASSIGNED
      //
      if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {
        logwrite( function, "ABORT requested" );
logwrite( function, "[DEBUG] setting READY bit" );
        seq.set_clr_seqstate_bit( Sequencer::SEQ_READY, ( Sequencer::SEQ_ABORTREQ | Sequencer::SEQ_RUNNING ) );  // set and clear together
        seq.clr_reqstate_bit( ( Sequencer::SEQ_ABORTREQ | Sequencer::SEQ_RUNNING ) );
        seq.set_reqstate_bit( Sequencer::SEQ_READY );

        error = seq.target.update_state( Sequencer::TARGET_UNASSIGNED );
        message.str(""); message << ( error==NO_ERROR ? "" : "ERROR " ) << "marking target " << seq.target.name 
                                 << " id " << seq.target.obsid << " order " << seq.target.obsorder
                                 << " as " << Sequencer::TARGET_UNASSIGNED;
        logwrite( function, message.str() );

        // let the world know of the state change
        //
        message.str(""); message << "TARGETSTATE:" << seq.target.state << " TARGET:" << seq.target.name << " OBSID:" << seq.target.obsid;
        seq.async.enqueue( message.str() );

        seq.thr_error.fetch_or( THR_SEQUENCE_START );
        break;
      }

      // If not aborted then this exposure is now complete
      //
      message.str(""); message << "exposure complete for target " << seq.target.name 
                               << " id " << seq.target.obsid << " order " << seq.target.obsorder;
      logwrite( function, message.str() );

      // Update this target's state in the database
      //
      error = seq.target.update_state( Sequencer::TARGET_COMPLETE );       // update the active target table
      if (error==NO_ERROR) error = seq.target.insert_completed();          // insert into the completed table
      if (error!=NO_ERROR) seq.thr_error.fetch_or( THR_SEQUENCE_START );   // report any error

      // let the world know of the state change
      //
      message.str(""); message << "TARGETSTATE:" << seq.target.state << " TARGET:" << seq.target.name << " OBSID:" << seq.target.obsid;
      seq.async.enqueue( message.str() );

      // Check the "dotype" --
      // If this was "do one" then do_once is set and get out now.
      //
      if ( seq.do_once.load() ) {
        seq.set_seqstate_bit( Sequencer::SEQ_STOPREQ );
        seq.set_reqstate_bit( Sequencer::SEQ_STOPREQ );
        logwrite( function, "stopping sequencer because single-step is selected" );
        break;
      }

    } // end while the SEQ_RUNNING bit is set in seqstate
logwrite( function, "[DEBUG] I'm out of the main SEQ_RUNNING loop now" );

    // The STOPREQ got us out of the while loop. Now that the loop has exited,
    // clear the STOPREQ and RUNNING bits and set the READY bits.
    //
    if ( seq.is_seqstate_set( Sequencer::SEQ_STOPREQ | Sequencer::SEQ_ABORTREQ ) ) {
logwrite( function, "[DEBUG] setting READY bit" );
      seq.set_clr_seqstate_bit( Sequencer::SEQ_READY, ( Sequencer::SEQ_STOPREQ | Sequencer::SEQ_ABORTREQ | Sequencer::SEQ_RUNNING ) );  // set and clear together
      seq.clr_reqstate_bit( ( Sequencer::SEQ_STOPREQ | Sequencer::SEQ_ABORTREQ | Sequencer::SEQ_RUNNING ) );
      seq.set_reqstate_bit( Sequencer::SEQ_READY );
    }

    logwrite( function, "target list processing has stopped" );
    seq.clr_thrstate_bit( THR_SEQUENCE_START );
    return;
  }
  /***** Sequencer::Sequence::dothread_sequence_start *************************/


  /***** Sequencer::Sequence::dothread_camera_set *****************************/
  /**
   * @brief      sets the camera according to the parameters in the target entry
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * At the moment, this is only exposure time.
   *
   */
  void Sequence::dothread_camera_set( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_CAMERA_SET );
    std::string function = "Sequencer::Sequence::dothread_camera_set";
    std::string reply;
    long error;
    std::stringstream camcmd;

    // send the EXPTIME command to camerad
    //
    // Everywhere is maintained that exptime is specified in sec except
    // the camera takes msec, so convert just before sending the command.
    //
    long exptime = (long)( seq.target.exptime * 1000 );
    camcmd.str(""); camcmd << CAMERAD_EXPTIME << " " << exptime;

    error = seq.camerad.send( camcmd.str(), reply );

    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to set exptime" );
      seq.thr_error.fetch_or( THR_CAMERA_SET );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );

    seq.clr_thrstate_bit( THR_CAMERA_SET );
    return;
  }
  /***** Sequencer::Sequence::dothread_camera_set *****************************/


  /***** Sequencer::Sequence::dothread_slit_set *******************************/
  /**
   * @brief      sets the slit according to the parameters in the target entry
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_slit_set( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_SLIT_SET );
    std::string function = "Sequencer::Sequence::dothread_slit_set";
    std::string reply;
    long error;
    std::stringstream slitcmd;

    // send the SET command to slitd
    //
    slitcmd << SLITD_SET << " " << seq.target.slitwidth << " " << seq.target.slitoffset;

    error = seq.slitd.send( slitcmd.str(), reply );

    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to set slit" );
      seq.thr_error.fetch_or( THR_SLIT_SET );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );

    seq.clr_thrstate_bit( THR_SLIT_SET );
    return;
  }
  /***** Sequencer::Sequence::dothread_slit_set *******************************/


  /***** Sequencer::Sequence::dothread_power_init *****************************/
  /**
   * @brief      initializes the power system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_power_init( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_POWER_INIT );
    std::string function = "Sequencer::Sequence::dothread_power_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;
logwrite( function, "[DEBUG] starting" );  ///< TODO @todo temporary

message.str(""); message << "[DEBUG] powerd socket isconnected=" << seq.powerd.socket.isconnected(); ///< TODO @todo temporary
logwrite( function, message.str() ); ///< TODO @todo temporary

    // if not connected to the power daemon then connect
    //
    if ( !seq.powerd.socket.isconnected() ) {
      logwrite( function, "connecting to power daemon" );
      error = seq.powerd.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to power daemon" );
    }

    // Ask powerd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.powerd.send( POWERD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with power hardware" );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to network power switch" );
      error = seq.powerd.send( POWERD_OPEN, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: opening connection to power hardware" );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize power control" );
logwrite( function, "[DEBUG] I should be setting thr_error here !!!!!!!!!!!!!!!" );
message.str(""); message << "[DEBUG] *before* thr_error=" << seq.thr_error.load(); logwrite( function, message.str() );
      seq.thr_error.fetch_or( THR_POWER_INIT );
message.str(""); message << "[DEBUG] *after* thr_error=" << seq.thr_error.load(); logwrite( function, message.str() );
    }

    logwrite( function, "ready" );
    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_POWER );

    seq.clr_thrstate_bit( THR_POWER_INIT );
    return;
  }
  /***** Sequencer::Sequence::dothread_power_init *****************************/


  /***** Sequencer::Sequence::dothread_power_shutdown *************************/
  /**
   * @brief      shuts down the power system
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_power_shutdown( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_POWER_SHUTDOWN );
    std::string function = "Sequencer::Sequence::dothread_power_shutdown";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // nothing to do if not connected
    //
    if ( seq.powerd.socket.isconnected() ) {

      // close connections between powerd and the hardware with which it communicates
      //
      if ( error==NO_ERROR ) {
        logwrite( function, "closing power hardware" );
        error = seq.powerd.send( POWERD_CLOSE, reply );
        if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: closing connection to power hardware" );
      }

      // disconnect me from powerd
      //
      if ( error==NO_ERROR ) {
        logwrite( function, "disconnecting from powerd" );
        error = seq.powerd.disconnect();
        if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: disconnecting from powerd" );
      }
    }

    if (error!=NO_ERROR) seq.thr_error.fetch_or( THR_POWER_SHUTDOWN );
    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_POWER );
    seq.clr_thrstate_bit( THR_POWER_SHUTDOWN );
    return;
  }
  /***** Sequencer::Sequence::dothread_power_shutdown *************************/


  /***** Sequencer::Sequence::dothread_slit_init ******************************/
  /**
   * @brief      initializes the slit for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_slit_init( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_SLIT_INIT );
    std::string function = "Sequencer::Sequence::dothread_slit_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false, ishomed=false;

    // Turn on power to slit hardware.
    //
    for ( auto plug : seq.power_switch[POWER_SLIT].plugname ) {
      std::stringstream cmd;
      cmd << plug << " ON";
      error |= seq.powerd.send( cmd.str(), reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: turning on power to slit hardware" );
    }

    // if not connected to the slit daemon then connect
    //
    if ( error==NO_ERROR && !seq.slitd.socket.isconnected() ) {
      logwrite( function, "connecting to slit daemon" );
      error = seq.slitd.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to slit daemon" );
    }

    seq.slitd.socket.set_totime( 30000 );

    // Ask slitd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.slitd.send( SLITD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with slit hardware" );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to slit hardware" );
      error = seq.slitd.send( SLITD_OPEN, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: opening connection to slit hardware" );
    }

    // Ask slitd if the slit motors are homed,
    //
    if ( error == NO_ERROR ) {
      error  = seq.slitd.send( SLITD_ISHOME, reply );
      error |= seq.parse_state( function, reply, ishomed );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with slit hardware" );
    }

    // and send the HOME command to slitd if needed.
    //
    if ( error==NO_ERROR && !ishomed ) {
      logwrite( function, "sending home command" );
      error = seq.slitd.send( SLITD_HOME, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with slit hardware" );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: initializing slit control" );
      seq.thr_error.fetch_or( THR_SLIT_INIT );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );

    seq.clr_thrstate_bit( THR_SLIT_INIT );
    return;
  }
  /***** Sequencer::Sequence::dothread_slit_init ******************************/


  /***** Sequencer::Sequence::dothread_acam_init ******************************/
  /**
   * @brief      initializes the acam system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_acam_init( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_ACAM_INIT );
    std::string function = "Sequencer::Sequence::dothread_acam_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // Turn on power to acam hardware.
    //
    for ( auto plug : seq.power_switch[POWER_ACAM].plugname ) {
      std::stringstream cmd;
      cmd << plug << " ON";
      error |= seq.powerd.send( cmd.str(), reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: turning on power to acam hardware" );
    }

    // if not connected to the acam daemon then connect
    //
    if ( !seq.acamd.socket.isconnected() ) {
      logwrite( function, "connecting to acamd daemon" );
      error = seq.acamd.connect();                   // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to acamd daemon" );
    }

    // Ask acamd if hardware connections are open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.acamd.send( ACAMD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with acam hardware" );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to acam hardware" );
      error = seq.acamd.send( ACAMD_OPEN, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: opening connection to acam hardware" );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize acam" );
      seq.thr_error.fetch_or( THR_ACAM_INIT );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_ACAM );

    seq.clr_thrstate_bit( THR_ACAM_INIT );
    return;
  }
  /***** Sequencer::Sequence::dothread_acam_init ******************************/


  /***** Sequencer::Sequence::dothread_acam_shutdown **************************/
  /**
   * @brief      shuts down the acam system
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_acam_shutdown( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_ACAM_SHUTDOWN );
    std::string function = "Sequencer::Sequence::dothread_acam_shutdown";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // nothing to do if not connected
    //
    if ( seq.acamd.socket.isconnected() ) {

      // close connections between acamd and the hardware with which it communicates
      //
      if ( error==NO_ERROR ) {
        logwrite( function, "closing acam hardware" );
        error = seq.acamd.send( ACAMD_CLOSE, reply );
        if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: closing connection to acam hardware" );
      }

      // disconnect me from acamd
      //
      if ( error==NO_ERROR ) {
        logwrite( function, "disconnecting from acamd" );
        error = seq.acamd.disconnect();
        if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: disconnecting from acamd" );
      }
    }

    // Turn off power to acam hardware.
    // Any error here is added to thr_error.
    //
    for ( auto plug : seq.power_switch[POWER_ACAM].plugname ) {
      long pwrerr=NO_ERROR;
      std::stringstream cmd;
      cmd << plug << " OFF";
      pwrerr = seq.powerd.send( cmd.str(), reply );
      if ( pwrerr != NO_ERROR ) {
        message.str(""); message << "ERROR: turning off plug " << plug << " for acam hardware";
        seq.async.enqueue_and_log( function, message.str() );
        seq.thr_error.fetch_or( THR_ACAM_SHUTDOWN );
      }
    }

    if (error!=NO_ERROR) seq.thr_error.fetch_or( THR_ACAM_SHUTDOWN );
    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_ACAM );
    seq.clr_thrstate_bit( THR_ACAM_SHUTDOWN );
    return;
  }
  /***** Sequencer::Sequence::dothread_acam_shutdown **************************/


  /***** Sequencer::Sequence::dothread_calib_init *****************************/
  /**
   * @brief      initializes the calibrator system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_calib_init( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_CALIB_INIT );
    std::string function = "Sequencer::Sequence::dothread_calib_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // Turn on power to calib hardware.
    //
    for ( auto plug : seq.power_switch[POWER_CALIB].plugname ) {
      std::stringstream cmd;
      cmd << plug << " ON";
      error |= seq.powerd.send( cmd.str(), reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: turning on power to calib hardware" );
    }

    // if not connected to the calib daemon then connect
    //
    if ( !seq.calibd.socket.isconnected() ) {
      logwrite( function, "connecting to calib daemon" );
      error = seq.calibd.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to calib daemon" );
    }

    // Ask calibd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.calibd.send( CALIBD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with calib hardware" );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to calib hardware" );
      error = seq.calibd.send( CALIBD_OPEN, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: opening connection to calib hardware" );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize calibrator" );
      seq.thr_error.fetch_or( THR_CALIB_INIT );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );

    seq.clr_thrstate_bit( THR_CALIB_INIT );
    return;
  }
  /***** Sequencer::Sequence::dothread_calib_init *****************************/


  /***** Sequencer::Sequence::dothread_calib_shutdown *************************/
  /**
   * @brief      shuts down the calibrator system
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_calib_shutdown( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_CALIB_SHUTDOWN );
    std::string function = "Sequencer::Sequence::dothread_calib_shutdown";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // nothing to do if not connected
    //
    if ( seq.calibd.socket.isconnected() ) {

      // close connections between calibd and the hardware with which it communicates
      //
      if ( error==NO_ERROR ) {
        logwrite( function, "closing calib hardware" );
        error = seq.calibd.send( CALIBD_CLOSE, reply );
        if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: closing connection to calib hardware" );
      }

      // disconnect me from calibd
      //
      if ( error==NO_ERROR ) {
        logwrite( function, "disconnecting from calibd" );
        error = seq.calibd.disconnect();
        if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: disconnecting from calibd" );
      }
    }

    // Turn off power to calib hardware.
    // Any error here is added to thr_error.
    //
    for ( auto plug : seq.power_switch[POWER_CALIB].plugname ) {
      long pwrerr=NO_ERROR;
      std::stringstream cmd;
      cmd << plug << " OFF";
      pwrerr = seq.powerd.send( cmd.str(), reply );
      if ( pwrerr != NO_ERROR ) {
        message.str(""); message << "ERROR: turning off plug " << plug << " for calib hardware";
        seq.async.enqueue_and_log( function, message.str() );
        seq.thr_error.fetch_or( THR_CALIB_SHUTDOWN );
      }
    }

    if (error!=NO_ERROR) seq.thr_error.fetch_or( THR_CALIB_SHUTDOWN );
    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );
    seq.clr_thrstate_bit( THR_CALIB_SHUTDOWN );
    return;
  }
  /***** Sequencer::Sequence::dothread_calib_shutdown *************************/


  /***** Sequencer::Sequence::dothread_tcs_init *******************************/
  /**
   * @brief      initializes the tcs system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_tcs_init( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_TCS_INIT );
    std::string function = "Sequencer::Sequence::dothread_tcs_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if not connected to the tcs daemon then connect
    //
    if ( !seq.tcsd.socket.isconnected() ) {
      logwrite( function, "connecting to tcs daemon" );
      error = seq.tcsd.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to tcs daemon" );
    }

    // Ask tcsd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.tcsd.send( TCSD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with TCS" );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to tcs hardware" );
      error = seq.tcsd.send( TCSD_OPEN, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to TCS" );
    }

    // set the offset rates
    //
    if ( error==NO_ERROR ) {
      message.str(""); message << "MRATES " << seq.tcs_offsetrate_ra << " " << seq.tcs_offsetrate_dec;
      error  = seq.tcsd.send( message.str(), reply );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize TCS control" );
      seq.thr_error.fetch_or( THR_TCS_INIT );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCS );

    seq.clr_thrstate_bit( THR_TCS_INIT );
    return;
  }
  /***** Sequencer::Sequence::dothread_tcs_init *******************************/


  /***** Sequencer::Sequence::dothread_flexure_init ***************************/
  /**
   * @brief      initializes the flexure system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_flexure_init( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_FLEXURE_INIT );
    std::string function = "Sequencer::Sequence::dothread_flexure_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // Turn on power to flexure hardware.
    //
    logwrite( function, "powering on flexure hardware" );
    for ( auto plug : seq.power_switch[POWER_FLEXURE].plugname ) {
      std::stringstream cmd;
      cmd << plug << " ON";
      error |= seq.powerd.send( cmd.str(), reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: turning on power to flexure hardware" );
    }

    // if not connected to the flexure daemon then connect
    //
    if ( !seq.flexured.socket.isconnected() ) {
      logwrite( function, "connecting to flexure daemon" );
      error = seq.flexured.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to flexure daemon" );
    }

    // Ask flexured if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.flexured.send( FLEXURED_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with flexure hardware" );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to flexure hardware" );
      error = seq.flexured.send( FLEXURED_OPEN, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: opening connection to flexure hardware" );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize flexure control" );
      seq.thr_error.fetch_or( THR_FLEXURE_INIT );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE );

    seq.clr_thrstate_bit( THR_FLEXURE_INIT );
    return;
  }
  /***** Sequencer::Sequence::dothread_flexure_init ***************************/


  /***** Sequencer::Sequence::dothread_focus_init *****************************/
  /**
   * @brief      initializes the focus system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_focus_init( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_FOCUS_SET );
    std::string function = "Sequencer::Sequence::dothread_focus_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // Turn on power to focus hardware.
    //
    logwrite( function, "powering on focus hardware" );
    for ( auto plug : seq.power_switch[POWER_FOCUS].plugname ) {
      std::stringstream cmd;
      cmd << plug << " ON";
      error |= seq.powerd.send( cmd.str(), reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: turning on power to focus hardware" );
    }

    // if not connected to the focus daemon then connect
    //
    if ( !seq.focusd.socket.isconnected() ) {
      logwrite( function, "connecting to focus daemon" );
      error = seq.focusd.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to focus daemon" );
    }

    // Ask focusd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.focusd.send( FOCUSD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with focus hardware" );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to focus hardware" );
      error = seq.focusd.send( FOCUSD_OPEN, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: opening connection to focus hardware" );
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize focus control" );
      seq.thr_error.fetch_or( THR_FOCUS_SET );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );

    seq.clr_thrstate_bit( THR_FOCUS_SET );
    return;
  }
  /***** Sequencer::Sequence::dothread_focus_init *****************************/


  /***** Sequencer::Sequence::dothread_camera_init ****************************/
  /**
   * @brief      initializes the camera system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_camera_init( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_CAMERA_INIT );
    std::string function = "Sequencer::Sequence::dothread_camera_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // Turn on power to focus hardware.
    //
    logwrite( function, "powering on science cameras" );
    for ( auto plug : seq.power_switch[POWER_CAMERA].plugname ) {
      std::stringstream cmd;
      cmd << plug << " ON";
      error |= seq.powerd.send( cmd.str(), reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: turning on power to science cameras" );
    }

    // if not connected to the camera daemon then connect
    //
    if ( !seq.camerad.socket.isconnected() ) {
      logwrite( function, "connecting to camera daemon" );
      error = seq.camerad.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to camera daemon" );
    }

    // Ask camerad if hardware connection is open,
    //
    if (error==NO_ERROR) error = seq.camerad.send( CAMERAD_ISOPEN, reply );

    // and open it if necessary.
    //
    if (error==NO_ERROR) error = seq.parse_state( function, reply, isopen );
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to camera hardware" );
//    error = seq.camerad.send( CAMERAD_OPEN, reply );
      error = seq.camerad.send( "open 2 3", reply );     ///< TODO @todo two controllers working for testing
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with science camera controller(s)" );
    }

    ///< TODO @todo send a bunch of stuff, will get cleaned up later
    //
//  if (error==NO_ERROR) { logwrite( function, "loading firmware" );  error = seq.camerad.send( "load /home/developer/ss/DSP/SWIFT/sg2_48khz.lod", reply ); }
    if (error==NO_ERROR) { logwrite( function, "loading firmware" );  error = seq.camerad.send( "load", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting buffer");     error = seq.camerad.send( "buffer 1024 1024", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting geometry" );  error = seq.camerad.send( "geometry 1024 1024", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting readmode" );  error = seq.camerad.send( "readout U1", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting useframes" ); error = seq.camerad.send( "useframes false", reply ); }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize camera" );
      seq.thr_error.fetch_or( THR_CAMERA_INIT );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );

    seq.clr_thrstate_bit( THR_CAMERA_INIT );
    return;
  }
  /***** Sequencer::Sequence::dothread_camera_init ****************************/


  /***** Sequencer::Sequence::dothread_acam_shutdown **************************/
  /**
   * @brief      shutdown the acam by closing dust covers
   * @param[in]  seq  reference to Sequencer::Sequence object
   * @todo       ACAM Shutdown not yet implemented!
   *
   */
/***
  void Sequence::dothread_acam_shutdown( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_ACAM_SHUTDOWN );
    std::string function = "Sequencer::Sequence::dothread_acam_shutdown";
    std::stringstream message;
    std::string reply;
    logwrite( function, "[TODO] acam shutdown not yet implemented. sleeping 5s" );
    usleep( 5000000 );
    seq.clr_thrstate_bit( THR_ACAM_SHUTDOWN );
    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_ACAM );
    return;
  }
***/
  /***** Sequencer::Sequence::dothread_acam_shutdown **************************/


  /***** Sequencer::Sequence::dothread_move_to_target *************************/
  /**
   * @brief      send request to TCS to move to target coordinates
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * Disable guiding and send the new target coordinates to the TCS with
   * the instruction to move immediately.
   *
   */
  void Sequence::dothread_move_to_target( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_MOVE_TO_TARGET );
    std::string function = "Sequencer::Sequence::dothread_move_to_target";
    std::stringstream message;
    long error=NO_ERROR;

//#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[ACQUIRE] this target.name=" << seq.target.name 
                             << " RA=" << seq.target.ra_hms 
                             << " DEC=" << seq.target.dec_dms 
                             << " CASSANGLE=" << seq.target.casangle
                             << " SLITANGLE=" << seq.target.slitangle;
    logwrite( function, message.str() );
//#endif

    // If RA and DEC fields are both empty then no telescope move
    //
    if ( seq.target.ra_hms.empty() && seq.target.dec_dms.empty() ) {
      logwrite( function, "no telescope move requested" );
      seq.clr_thrstate_bit( THR_MOVE_TO_TARGET );
      return;
    }

    // Before trying to move the telescope, ask where it is pointed
    //
    double ra_h_now, dec_d_now;
    seq.get_tcs_coords( ra_h_now, dec_d_now );   // returns RA in decimal hours, DEC in decimal degrees
    double ra_delta  = std::abs( ra_h_now  - seq.radec_to_decimal( seq.target.ra_hms  ) );  // compare decimal hours
    double dec_delta = std::abs( dec_d_now - seq.radec_to_decimal( seq.target.dec_dms ) );  // compare decimal degrees

    // If the difference between the TCS coordinates and the target coordinates are within
    // the resolution of reading the TCS then assume we are already pointed. Otherwise,
    // disable guiding and point the telescope here.
    //
    if ( ra_delta > 0.01 || dec_delta > 0.01 ) {

      // disable guiding
      //
      logwrite( function, "[TODO] disable guiding not yet implemented" ); ///< @todo disable guiding not yet implemented

      // clear target acquired flag
      //
      seq.target.acquired = false;

      error = NO_ERROR;

      // Before sending target coordinates to TCS,
      // convert them to decimal and to scope coordinates.
      // (fpoffsets.coords_* are always in degrees)
      //
      std::stringstream coords;
      std::string reply;
      seq.target.fpoffsets.coords_in.ra    = seq.radec_to_decimal( seq.target.ra_hms  ) * TO_DEGREES;
      seq.target.fpoffsets.coords_in.dec   = seq.radec_to_decimal( seq.target.dec_dms );
      seq.target.fpoffsets.coords_in.angle = seq.target.slitangle;

      // can't be NaN
      //
      bool ra_isnan  = std::isnan( seq.target.fpoffsets.coords_in.ra  );
      bool dec_isnan = std::isnan( seq.target.fpoffsets.coords_in.dec );

      if ( ra_isnan || dec_isnan ) {
        message.str(""); message << "ERROR: converting";
        if ( ra_isnan  ) { message << " RA=\"" << seq.target.ra_hms << "\""; }
        if ( dec_isnan ) { message << " DEC=\"" << seq.target.dec_dms << "\""; }
        message << " to decimal";
        seq.async.enqueue_and_log( function, message.str() );
        seq.thr_error.fetch_or( THR_MOVE_TO_TARGET );                 
        seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCS | Sequencer::SEQ_WAIT_TCSOP );
        seq.clr_thrstate_bit( THR_MOVE_TO_TARGET );
        return;
      }

      // The acam GUI would like to know the coordinates of the acam,
      // which can be attained by converting the target coords from slit->acam ...
      //
      std::stringstream acam;
      std::string dontcare;
      if ( error == NO_ERROR ) seq.acamd.command( ACAMD_INIT, dontcare );         // send here, don't need the reply

      error = seq.target.fpoffsets.compute_offset( "SLIT", "ACAM" );              // convert coords_in from SLIT to ACAM

      acam << ACAMD_CAMERASERVER_COORDS << std::fixed << std::setprecision(6)
                                        << " " << seq.target.fpoffsets.coords_out.ra
                                        << " " << seq.target.fpoffsets.coords_out.dec
                                        << " " << seq.target.fpoffsets.coords_out.angle;

      message.str(""); message << "[ACQUIRE] " << acam.str(); logwrite( function, message.str() );

      if ( error == NO_ERROR ) seq.acamd.command( acam.str(), dontcare );         // send here, don't need the reply

      // send casangle
      //
      logwrite( function, "[TODO] send casangle not yet implemented" ); ///< @todo put send cassangle into its own thread?

      // Before sending the target coords to the TCS, convert them from slit->scope...
      //
      error = seq.target.fpoffsets.compute_offset( "SLIT", "SCOPE" );             // convert coords_in from SLIT->SCOPE

      // TCS wants decimal hours for RA and fpoffsets.coords are always in degrees
      // so convert that as it's being sent here.
      //
      coords << "COORDS " << ( seq.target.fpoffsets.coords_out.ra * TO_HOURS )  << " " 
                          <<   seq.target.fpoffsets.coords_out.dec << " 0 0 0 \"" << seq.target.name << "\"";

message.str(""); message << "[ACQUIRE] sending " << coords.str();
logwrite( function, message.str() );

      error  = seq.tcsd.send( coords.str(), reply );                              // send to the TCS

      int tcsvalue;
      if ( error == NO_ERROR ) error = seq.extract_tcs_value( reply, tcsvalue );  // extract the value from the tcsd reply
      if ( error == NO_ERROR ) error = seq.parse_tcs_generic( tcsvalue );         // parse the tcs reply
      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR: sending COORDS command. TCS reply: " << reply;
        seq.async.enqueue_and_log( function, message.str() );
        seq.thr_error.fetch_or( THR_MOVE_TO_TARGET );                 
        seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCS | Sequencer::SEQ_WAIT_TCSOP );
        seq.clr_thrstate_bit( THR_MOVE_TO_TARGET );
        return;
      }

      // Target coords have been sent to the TCS but they don't actually go to the TCS,
      // they go to a human, which has to then command the TCS, so we are now waiting
      // on the TCS Operator...
      //
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_TCSOP );                          // waiting for TCS Operator

      // Wait for telescope slew to start.
      //
      logwrite( function, "waiting for TCS slew to start" );  ///< TODO @todo log telemetry!

      // This wait is forever, or until an abort, because the slew time can be unpredictable
      // due to the fact that we are still waiting for a human (!) to initiate the slew.
      //
      // After sending the COORDS command, the TCS could be in STOPPED or TRACKING
      // from the last target.
      //

      while ( error==NO_ERROR && not seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {

        // If an abort has been requested then stop polling the TCS.
        // This doesn't actually stop the telescope, it just means we stop paying attention to it.
        //
        if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {
          seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCSOP );
          logwrite( function, "abort requested. no longer waiting for TCS Op" );
          break;
        }

        std::string reply;
        int tcsvalue=TCS_UNDEFINED;
        error = seq.tcsd.send( "poll ?MOTION", reply );  // "poll" prevents excessive logging in tcsd
        if ( error == NO_ERROR) error = seq.extract_tcs_value( reply, tcsvalue );

        if ( tcsvalue == TCS_MOTION_SLEWING ) {          // switch from TCSOP to SLEW
          seq.set_clr_seqstate_bit( Sequencer::SEQ_WAIT_SLEW, Sequencer::SEQ_WAIT_TCSOP );
          logwrite( function, "TCS slew started" );      ///< TODO @todo log telemetry!
          break;
        }

        usleep( 100000 );  // don't poll the TCS too fast
      }

      // If not aborted then
      // set TCS slew state bit and clear TCS Operator state now that the telescope is moving.
      //
//    if ( not seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {
//      seq.set_clr_seqstate_bit( Sequencer::SEQ_WAIT_SLEW, Sequencer::SEQ_WAIT_TCSOP );
//      logwrite( function, "TCS slew started" );               ///< TODO @todo log telemetry!
//    }
//    else {
//      logwrite( function, "TCS slew aborted" );               ///< TODO @todo log telemetry!
//      seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCSOP );
//    }

      // Poll the TCS at 10Hz to detect when slewing has stopped.
      // This has no timeout but can be cancelled by user command.
      //
      while ( true ) { // error==NO_ERROR && not seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) 

        // If an abort has been requested then stop polling the TCS.
        // This doesn't actually stop the telescope, it just means we stop paying attention to it.
        //
        if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {
          seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SLEW );
          logwrite( function, "abort requested. no longer waiting for TCS slew" );
          break;
        }

        std::string reply;
        int tcsvalue=TCS_UNDEFINED;
        error = seq.tcsd.send( "poll ?MOTION", reply );  // "poll" prevents excessive logging in tcsd
        if ( error == NO_ERROR) error = seq.extract_tcs_value( reply, tcsvalue );

        if ( tcsvalue == TCS_MOTION_SETTLING ) break;

        usleep( 100000 );  // don't poll the TCS too fast
      }

      // If the slew finished naturally (not aborted) then
      // log that it stopped
      //
      if ( not seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {
        logwrite( function, "TCS slew stopped" );               ///< TODO @todo log telemetry!
      }

    }  // end if ( ra_delta > 0.01 || dec_delta > 0.01 ) 
    else {
      seq.async.enqueue_and_log( function, " NOTICE: telescope already pointed at target, no TCS move requested" );
    }

    // If the slew finished naturally (not aborted) then
    // set TCS settle state bit and clear TCS slew state bit now that the telescope is settling.
    //
    if ( error==NO_ERROR && not seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {
      logwrite( function, "waiting for TCS to settle" );  ///< TODO @todo log telemetry!
      seq.set_clr_seqstate_bit( Sequencer::SEQ_WAIT_SETTLE, Sequencer::SEQ_WAIT_SLEW );
    }

    // Before entering loop waiting for telescope to settle
    // get the current time (in seconds) to be used for timeout.
    //
    bool   settled       = false;                               // settled gets set true only when TCS reports MOTION_TRACKING
    double clock_now     = get_clock_time();
    double clock_timeout = clock_now + seq.tcs_settle_timeout;  // must settle by this time

    // Poll the TCS at 10Hz to detect when telescope has settled (MOTION_TRACKING).
    // This wait can time out.
    //
    while ( not settled ) { // error==NO_ERROR && not settled && not seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) )

      // If an abort has been requested then stop polling the TCS.
      // This doesn't actually stop the telescope, it just means we stop paying attention to it.
      //
      if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTREQ ) ) {
        seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SETTLE );
        logwrite( function, "abort requested. no longer waiting for TCS settle" );
        break;
      }

      std::string reply;
      int tcsvalue=TCS_UNDEFINED;
      error = seq.tcsd.send( "poll ?MOTION", reply );  // "poll" prevents excessive logging in tcsd
      if ( error == NO_ERROR) error = seq.extract_tcs_value( reply, tcsvalue );

      // once the TCS reports TRACKING, then the telescope has settled and it's time to get out
      //
      if ( tcsvalue == TCS_MOTION_TRACKING ) settled = true;

      // before looping, check for a timeout
      //
      clock_now = get_clock_time();

      if ( clock_now > clock_timeout ) {
        error = ERROR;
        seq.async.enqueue_and_log( function, "ERROR: timeout waiting for telescope to settle" );
        seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SETTLE );
        break;
      }
      usleep( 100000 );  // don't poll the TCS too fast
    }

    // If not already acquired then start the acquisition sequence in a separate thread
    //
    if ( settled && !seq.target.acquired ) {

      logwrite( function, "TCS settled, starting acquisition thread" );  ///< TODO @todo log to telemetry!

      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_ACQUIRE ); std::thread( dothread_acquisition, std::ref(seq) ).detach();
    }
    else
    if ( settled && seq.target.acquired ) {
      logwrite( function, "TCS settled, target already acquired" );  ///< TODO @todo log to telemetry!
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to acquire target" );
      seq.thr_error.fetch_or( THR_MOVE_TO_TARGET );
    }

    // clear all TCS wait bits
    //
message.str(""); message << "[DEBUG] clearing all TCS bits. BEFORE: " << seq.seqstate.load();
logwrite( function, message.str() );
    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCS    | 
                          Sequencer::SEQ_WAIT_TCSOP  | 
                          Sequencer::SEQ_WAIT_SLEW   | 
                          Sequencer::SEQ_WAIT_SETTLE );
message.str(""); message << "[DEBUG] clearing all TCS bits. AFTER: " << seq.seqstate.load();
logwrite( function, message.str() );

    seq.clr_thrstate_bit( THR_MOVE_TO_TARGET );
    return;
  }
  /***** Sequencer::Sequence::dothread_move_to_target *************************/


  /***** Sequencer::Sequence::dothread_notify_tcs *****************************/
  /**
   * @brief      notify TCS operator of next target coords for preauthorization, no move
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * This function sends the coordinates of the *next* target to the TCS
   * operator for the purposes of readiness and pre-authorization. The 
   * telescope should *not* move!
   *
   */
  void Sequence::dothread_notify_tcs( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_NOTIFY_TCS );
    std::string function = "Sequencer::Sequence::dothread_notify_tcs";
    std::stringstream message;
    long error=NO_ERROR;

    // If this target is the same as the last then this section gets skipped;
    // no need to repoint the telescope and wait for a slew if it's already
    // on target.  Ask where it is pointed:
    //
    double ra_h_now, dec_d_now;
    seq.get_tcs_coords( ra_h_now, dec_d_now );   // returns RA in decimal hours, DEC in decimal degrees
    double ra_delta  = std::abs( ra_h_now  - seq.radec_to_decimal( seq.target.ra_hms  ) );  // compare decimal hours
    double dec_delta = std::abs( dec_d_now - seq.radec_to_decimal( seq.target.dec_dms ) );  // compare decimal degrees

    // If the difference between the TCS coordinates and the target coordinates are within
    // the resolution of reading the TCS then assume we are already pointed. Otherwise,
    // disable guiding and point the telescope here.
    //
    if ( ra_delta > 0.01 || dec_delta > 0.01 ) {

      message << "requesting early pre-auth for next target: " 
              << seq.target.name << " " 
              << seq.target.ra_hms << " " 
              << seq.target.dec_dms;
      logwrite( function, message.str() );

      // send coordinates to TCS
      //
      if ( error == NO_ERROR ) {
        std::stringstream coords;
        std::string reply, ra_hms, dec_dms;

        // convert ra, dec to decimal
        // can't be NaN
        //
        if ( std::isnan( seq.radec_to_decimal( seq.target.ra_hms,  ra_hms  ) ) ||
             std::isnan( seq.radec_to_decimal( seq.target.dec_dms, dec_dms ) ) ) {
          seq.async.enqueue_and_log( function, "ERROR: can't handle NaN value for RA, DEC" );
          seq.thr_error.fetch_or( THR_NOTIFY_TCS );
          seq.clr_thrstate_bit( THR_NOTIFY_TCS );
          return;
        }

        coords << "NEXT " << ra_hms << " " << dec_dms << " 0 0 0 \"" << seq.target.name << "\"";
        ///< @todo TCS command "NEXT" not yet implemented
//      error  = seq.tcsd.send( coords.str(), reply );
        message.str(""); message << "[TODO] new command not implemented in TCS: " << coords.str();
        logwrite( function, message.str() );
        int tcsvalue;
        if ( error == NO_ERROR ) error = seq.extract_tcs_value( reply, tcsvalue );
        if ( error == NO_ERROR ) error = seq.parse_tcs_generic( tcsvalue );
      }

      // send casangle
      //
      if ( error == NO_ERROR ) logwrite( function, "[TODO] send casangle not yet implemented" ); ///< TODO @todo put send cassangle into its own thread?
    }
    seq.clr_thrstate_bit( THR_NOTIFY_TCS );
    return;
  }
  /***** Sequencer::Sequence::dothread_notify_tcs *****************************/


  /***** Sequencer::Sequence::dothread_focus_set ******************************/
  /**
   * @brief      set the focus
   * @param[in]  seq  reference to Sequencer::Sequence object
   * @todo       focus not yet implemented
   *
   */
  void Sequence::dothread_focus_set( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_FOCUS_SET );
    std::string function = "Sequencer::Sequence::dothread_focus_set";
    long error=NO_ERROR;

    logwrite( function, "[TODO] focus not yet implemented. sleeping 5s" );
    usleep( 5000000 );

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to set focus" );
      seq.thr_error.fetch_or( THR_FOCUS_SET );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );
    seq.clr_thrstate_bit( THR_FOCUS_SET );
    return;
  }
  /***** Sequencer::Sequence::dothread_focus_set ******************************/


  /***** Sequencer::Sequence::dothread_flexure_set ****************************/
  /**
   * @brief      set the flexure
   * @param[in]  seq  reference to Sequencer::Sequence object
   * @todo       flexure not yet implemented
   *
   */
  void Sequence::dothread_flexure_set( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_FLEXURE_SET );
    std::string function = "Sequencer::Sequence::dothread_flexure_set";
    long error=NO_ERROR;

    logwrite( function, "[TODO] flexure not yet implemented. sleeping 5s" );
    usleep( 5000000 );

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to set flexure control" );
      seq.thr_error.fetch_or( THR_FLEXURE_SET );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE );
    seq.clr_thrstate_bit( THR_FLEXURE_SET );
    return;
  }
  /***** Sequencer::Sequence::dothread_flexure_set ****************************/


  /***** Sequencer::Sequence::dothread_calib_set ******************************/
  /**
   * @brief      set the calibrator
   * @param[in]  seq  reference to Sequencer::Sequence object
   * @todo       calibrator not yet implemented
   *
   */
  void Sequence::dothread_calib_set( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_CALIBRATOR_SET );
    std::string function = "Sequencer::Sequence::dothread_calib_set";
    long error=NO_ERROR;

    logwrite( function, "[TODO] calibrator not yet implemented. sleeping 6s" );
    usleep( 6000000 );

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to set calibrator" );
      seq.thr_error.fetch_or( THR_CALIBRATOR_SET );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );
    seq.clr_thrstate_bit( THR_CALIBRATOR_SET );
    return;
  }
  /***** Sequencer::Sequence::dothread_calib_set ******************************/


  /***** Sequencer::Sequence::dothread_trigger_exposure ***********************/
  /**
   * @brief      trigger and wait for exposure
   * @param[in]  seq  reference to Sequencer::Sequence object
   * @todo       trigger exposure not yet abort-able
   *
   * This function updates the target's state in the DB active table to EXPOSING
   * then sends the expose command to the cameras.
   *
   * Sending the expose command to camerad will block until that command completes,
   * which will take all of the exposure and readout time (that's why this has to
   * be done in a separate thread).
   *
   * TODO this is not yet ABORT-able
   *
   */
  void Sequence::dothread_trigger_exposure( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_TRIGGER_EXPOSURE );
    std::string function = "Sequencer::Sequence::dothread_trigger_exposure";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // Check tcs_preauth_time and set notify_tcs_next_target --
    // When the preauth_time is non-zero, set this flag to true in order
    // to notify the TCS of the next target, when the remaining exposure
    // time is within TCS_PREAUTH_TIME of the end of the exposure time.
    //
    // When this flag is true, the async_listener thread will spawn a thread
    // to send the command to the TCS at the requested time, to get ready 
    // for the next target.
    //
    if ( seq.tcs_preauth_time > 0 ) seq.notify_tcs_next_target = true; else seq.notify_tcs_next_target = false;

    seq.arm_readout_flag = true;                  // enables the async_listener to look for the readout and clear the EXPOSE bit

    error = seq.camerad.async( CAMERAD_EXPOSE );  // Send the EXPOSE command to camera daemon on the non-blocking port and don't wait for reply

    if ( error == NO_ERROR ) {
      error = seq.target.update_state( Sequencer::TARGET_EXPOSING );   // set EXPOSE state in database
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_EXPOSE );              // set EXPOSE bit
    }
    else {
      seq.async.enqueue_and_log( function, "ERROR sending command: CAMERAD_EXPOSE" );
      seq.thr_error.fetch_or( THR_TRIGGER_EXPOSURE );                  // tell the world this thread had an error
      seq.target.update_state( Sequencer::TARGET_PENDING );            // return the target state to pending
      seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_EXPOSE );              // clear EXPOSE bit
      seq.arm_readout_flag = false;                                    // disarm async_listener from looking for readout
    }

    // let the world know of the state change
    //
    message.str(""); message << "TARGETSTATE:" << seq.target.state << " TARGET:" << seq.target.name << " OBSID:" << seq.target.obsid;
    seq.async.enqueue( message.str() );

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );

    seq.clr_thrstate_bit( THR_TRIGGER_EXPOSURE );
    return;
  }
  /***** Sequencer::Sequence::dothread_trigger_exposure ***********************/


  /***** Sequencer::Sequence::dothread_modify_exptime *************************/
  /**
   * @brief      modify the exposure time while an exposure is running
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * Since there is no process that will need to wait on the success or failure of this
   * thread, but the success or failure might need to be known by a GUI, the results
   * will be broadcast to the asynchronous message port.
   *
   */
  void Sequence::dothread_modify_exptime( Sequencer::Sequence &seq, double exptime_in ) {
    seq.set_thrstate_bit( THR_MODIFY_EXPTIME );
    std::string function = "Sequencer::Sequence::dothread_modify_exptime";
    std::stringstream message;
    std::string reply="";
    long error = NO_ERROR;
    double updated_exptime=0;

    // This function is only used while exposing
    //
    if ( not seq.is_seqstate_set( Sequencer::SEQ_WAIT_EXPOSE ) ) {
      seq.async.enqueue_and_log( function, "ERROR: cannot update exposure time when not currently exposing" );
      error = ERROR;
    }

    // Send command to the camera to modify the exposure time.
    // The camera works in msec to convert exptime_in here.
    //
    std::stringstream cmd;
    cmd << "modexptime " << (long)(1000*exptime_in);
    if ( error==NO_ERROR ) error = seq.camerad.async( cmd.str(), reply );

    // Reply from camera will contain DONE or ERROR
    //
    std::string::size_type pos = reply.find( "DONE" );
    if ( error==NO_ERROR && pos != std::string::npos ) {
      updated_exptime = (double)( std::stol( reply.substr( 0, pos ) ) / 1000. );
    }
    else error = ERROR;

    if ( error==NO_ERROR ) {
      seq.target.exptime = updated_exptime;
      message.str(""); message << "successfully updated exptime to " << updated_exptime << " sec";
      logwrite( function, message.str() );
    }

    // announce the success or failure in an asynchronous broadcast message
    //
    message.str(""); message << "MODIFY_EXPTIME: " << seq.target.exptime << ( error==NO_ERROR ? " DONE" : " ERROR" );
    seq.async.enqueue( message.str() );

    seq.clr_thrstate_bit( THR_MODIFY_EXPTIME );
    return;
  }
  /***** Sequencer::Sequence::dothread_modify_exptime *************************/


  /***** Sequencer::Sequence::dothread_acquisition ****************************/
  /**
   * @brief      performs the acqusition sequence when signaled
   * @details    this gets called by the move_to_target thread
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * This function is spawned in a thread.
   *
   */
  void Sequence::dothread_acquisition( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_ACQUISITION );
    std::string function = "Sequencer::Sequence::dothread_acquisition";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    long error = NO_ERROR;

    // Initialize the ACAM for acquisition.
    // Need one of these calls for each new target.
    //
    error  = seq.acamd.command( ACAMD_INIT, reply );

    // Begin looping, acquiring images, solving for astrometry, calculating offsets,
    // moving the telescope ... until acquired, timeout, or error.
    //
    double clock_now     = get_clock_time();
    double clock_timeout = clock_now + seq.acquisition_timeout;  // must acquire by this time

    int retrys=0;    // number of acquisition attempts
    int acquired=0;  // number of sequential successful acquisitions, must meet ACQUIRE_MIN_REPEAT for success

    // The goal for the camera is the result from the SLIT->ACAM calculation
    // that was performed in move_to_target() using the DB coords.
    //
    seq.target.fpoffsets.coords_in.ra    = seq.radec_to_decimal( seq.target.ra_hms  ) * TO_DEGREES;
    seq.target.fpoffsets.coords_in.dec   = seq.radec_to_decimal( seq.target.dec_dms );
    seq.target.fpoffsets.coords_in.angle = seq.target.slitangle;

    error = seq.target.fpoffsets.compute_offset( "SLIT", "ACAM" );              // convert coords_in from SLIT to ACAM

    double acam_ra_goal    = seq.target.fpoffsets.coords_out.ra;
    double acam_dec_goal   = seq.target.fpoffsets.coords_out.dec;
    double acam_angle_goal = seq.target.fpoffsets.coords_out.angle;

    message.str(""); message << "[ACQUIRE] set goals: ra=" << std::fixed << std::setprecision(6)
                                                           << acam_ra_goal 
                                                           << " dec=" << acam_dec_goal 
                                                           << " ang=" << acam_angle_goal;
    logwrite( function, message.str() );

#ifdef SIMULATED_IMAGES
    // This section is only for cameraserver-generated images.
    // These will always be the latest ACAM coordinates.
    double acam_ra_latest  = acam_ra_goal;
    double acam_dec_latest = acam_dec_goal;
#endif

    while ( (error==NO_ERROR) && acquired < seq.target.min_repeat ) {

      // Acquire an image from the camera.
      // The reply contains a FITS filename that has to be passed to the astrometry solver.
      //
      error  = seq.acamd.command( ACAMD_ACQUIRE, reply );

      // Perform the astrometry calculations on the acquired image.
      //
      cmd.str(""); cmd << ACAMD_SOLVE << " " << reply;
      if ( error == NO_ERROR ) error = seq.acamd.command( ACAMD_SOLVE, reply );

      // Tokenize the reply from the solver.
      // The solver gives us ACAM coordinates.
      //
      std::vector<std::string> tokens;
      Tokenize( reply, tokens, " " );

      bool match_found = false;                 // was a match found?
      double acam_ra=0, acam_dec=0, acam_pa=0;  // calculations from acam solver

      try {
        if ( ( tokens.at(0).compare( "GOOD" )  == 0 ) ||
             ( tokens.at(0).compare( "NOISY" ) == 0 ) ) {  // treat good and noisy the same for now
          match_found = true;
          acam_ra  = stod( tokens.at(1) );
          acam_dec = stod( tokens.at(2) );
          acam_pa  = stod( tokens.at(3) );
          message.str(""); message << "solve " << tokens.at(0) << ": match found with ra="
                                   << acam_ra << " dec=" << acam_dec << " pa=" << acam_pa;
          logwrite( function, message.str() );
        }
        else {
          match_found = false;
          message.str(""); message << "solve " << tokens.at(0) << ": no match found";
          logwrite( function, message.str() );
          acquired=0;  // one bad match resets this counter which requires ACQUIRE_MIN_REPEAT sequential acquires
        }
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "out of range parsing reply \"" << reply << "\" from acam astrometry solver: " << e.what();
        logwrite( function, message.str() );
        error = ERROR;
      }
      catch( std::invalid_argument &e ) {
        message.str(""); message << "invalid argument parsing reply \"" << reply << "\" from acam astrometry solver: " << e.what();
        logwrite( function, message.str() );
        error = ERROR;
      }
      catch( ... ) {
        message.str(""); message << "unknown exception argument parsing reply \"" << reply << "\" from acam astrometry solver";
        logwrite( function, message.str() );
        error = ERROR;
      }

      if ( !match_found && ( seq.acquisition_max_retrys > 0 ) && ( ++retrys >= seq.acquisition_max_retrys ) ) {
        seq.async.enqueue_and_log( function, "ERROR: failed to acquire target within max number of attempts" );
        break;
      }

      if ( error != NO_ERROR ) break;  // any errors at this point get out of the while !acquired loop now

      // Calculate the offsets to send to the TCS.
      // These are in degrees.
      //
      // Offsets calculated as difference between acam goals and the solution from solve.
      // ACAM goals are saved outside this loop. These are the SLIT->ACAM translated coords
      // from the DB and are where the ACAM needs to be pointed.
      // Goals never change.
      //
      double ra_off  = acam_ra_goal  - acam_ra;
      double dec_off = acam_dec_goal - acam_dec;

//#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[ACQUIRE] ra_off=" << std::fixed << std::setprecision(6)
                                                      << ra_off*3600. << " dec_off=" << dec_off*3600. << " (arcsec)";
      logwrite( function, message.str() );
//#endif

      {  // temporary logging
      double __ra, __dec;
      seq.get_tcs_weather_coords( __ra, __dec );
      message.str(""); message << "[ACQUIRE] tcs coords before: " << __ra*TO_DEGREES << " " << __dec << " deg";
      logwrite( function, message.str() );
      }

      // Compute the angular separation between the target (acam_ra_*) and calculated slit (acam_*)
      //
      double offset = seq.angular_separation( acam_ra_goal, acam_dec_goal, acam_ra, acam_dec );

      message.str(""); message << "[ACQUIRE] offset=" << offset << " (arcsec)"; logwrite( function,message.str() );

      // There is a maximum offset allowed to the TCS.
      // This is not a TCS limit (their limit is very large).
      // This is an NGPS limit so that we don't accidentally move too far off the slit.
      //
      if ( offset >= seq.target.max_tcs_offset ) {
        message.str(""); message << "[WARNING] calculated offset " << offset << " not below max "
                                 << seq.target.max_tcs_offset << " and will not be sent to the TCS";
        logwrite( function, message.str() );
      }
      // otherwise send the offsets to the TCS and keep looping.
      //
      else
      if (error==NO_ERROR) error = seq.offset_tcs( ra_off, dec_off );  // send offset to TCS here
      else {
        logwrite( function, "ERROR computing offsets" );
      }

      // If offset below the threshold then the target is acquired.
      // Increment a counter which must exceed the ACQUIRE_MIN_REPEAT before declaring success.
      //
      if ( error!=ERROR && offset < seq.target.offset_threshold ) {
        acquired++;
      }
      else acquired=0;  // one bad match resets this counter which requires ACQUIRE_MIN_REPEAT sequential acquires

      {  // temporary logging
      double __ra, __dec;
      seq.get_tcs_weather_coords( __ra, __dec );
      message.str(""); message << "[ACQUIRE] tcs coords after: " << std::fixed << std::setprecision(6)
                                                                 << __ra*TO_DEGREES << " " << __dec << " deg";
      logwrite( function, message.str() );
      }

      // before looping, check for a timeout
      //
      clock_now = get_clock_time();

      if ( clock_now > clock_timeout ) {
        error = ERROR;
        seq.async.enqueue_and_log( function, "ERROR: failed to acquire target within timeout" );
        break;
      }

#ifdef SIMULATED_IMAGES
      // This section is only for cameraserver-generated images which
      // produces simulated star fields. To generate a simulated image
      // you have to select something in the GUI, send ACAMD_CAMERASERVER_COORDS,
      // wait 1 second, then send ACAMD_ACQUIRE.

      // These will be the latest ACAM coordinates.
      //
      acam_ra_latest  += ra_off;
      acam_dec_latest += dec_off;

      // then send these to the Andor camera server.
      //
      std::stringstream acam;
      std::string dontcare;

      acam.str(""); acam << ACAMD_CAMERASERVER_COORDS << std::fixed << std::setprecision(6)
                                                      << " " << acam_ra_latest
                                                      << " " << acam_dec_latest
                                                      << " " << acam_angle_goal;

      message.str(""); message << "[ACQUIRE] " << acam.str(); logwrite( function, message.str() );

      if ( error == NO_ERROR ) seq.acamd.command( acam.str(), dontcare );  // send to external camera server here

      usleep(1000000);  // delay because cameraserver returns before it's finished
                        // and there's no feedback to know when an image is ready
#endif

    }  // end while ( (error==NO_ERROR) && !acquired ) 

    seq.target.acquired = ( acquired >= seq.target.min_repeat ? true : false );

    message.str(""); message << "target" << ( seq.target.acquired ? " " : " not " ) << "acquired";
    logwrite( function, message.str() );

    if (error!=NO_ERROR) seq.thr_error.fetch_or( THR_ACQUISITION );  // report any error

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_ACQUIRE );             // clear ACQUIRE bit
    seq.clr_thrstate_bit( THR_ACQUISITION );

    return;
  }
  /***** Sequencer::Sequence::dothread_acquisition ****************************/


  /***** Sequencer::Sequence::dothread_guide **********************************/
  /**
   * @brief      performs the guiding sequence when signaled
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * This function is spawned in a thread. It 
   *
   */
  void Sequence::dothread_guide( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_GUIDE );
    std::string function = "Sequencer::Sequence::dothread_guide";
    std::stringstream message;
  }
  /***** Sequencer::Sequence::dothread_guide **********************************/


  /***** Sequencer::Sequence::dothread_wait_for_state *************************/
  /**
   * @brief      waits for sequence state to match the requested state
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * This function is spawned in a thread. It waits forever until the seq.seqstate
   * matches the requested state, at which point it sends a cv.notify_all signal
   * to unblock all threads currently waiting on cv. Note that the requested state
   * can be changed at any time.
   *
   */
  void Sequence::dothread_wait_for_state( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_WAIT_FOR_STATE );
    std::string function = "Sequencer::Sequence::dothread_wait_for_state";
    std::stringstream message;
    uint32_t last_reqstate = seq.reqstate.load();

    seq.waiting_for_state.store( true );            // let other threads know that I'm already waiting for a state

    message.str(""); message << "sequencer state: " << seq.seqstate_string( seq.seqstate.load() )
                             << ". waiting for state: " << seq.seqstate_string( last_reqstate );
    logwrite( function, message.str() );

    // wait forever or until seqstate is the requested state.
    // Note that other threads can (atomically) change reqstate at any time.
    //
    while ( true ) {
      uint32_t new_reqstate = seq.reqstate.load();         // reqstate is checked only once per loop here
      if ( last_reqstate != new_reqstate ) {               // log if reqstate has changed
        message.str(""); message << "requested state changed: " << seq.seqstate_string( new_reqstate );
        logwrite( function, message.str() );
        last_reqstate = new_reqstate;
      }
      if ( seq.seqstate.load() == last_reqstate ) break;   // condition met: seqstate is reqstate
    }

    uint32_t te = seq.thr_error.load();
    if ( te != THR_NONE ) {
      message.str(""); message << "ERROR: the following thread(s) had an error: " << seq.thrstate_string( te );
      logwrite( function, message.str() );
//    seq.thr_error.store( THR_NONE );  // clear the thread error state
    }

    // done waiting so send notification
    //
    std::unique_lock<std::mutex> lck(seq.wait_mtx);
    message.str(""); message << "requested state " << seq.seqstate_string( seq.seqstate.load() ) 
                             << " reached: notifying threads";
    logwrite( function, message.str() );
    seq.waiting_for_state.store( false );           // let other threads know that I'm done waiting
    seq.cv.notify_all();

    seq.clr_thrstate_bit( THR_WAIT_FOR_STATE );
    return;
  }
  /***** Sequencer::Sequence::dothread_wait_for_state *************************/


  /***** Sequencer::Sequence::dothread_startup ********************************/
  /**
   * @brief      
   * @param[in]  
   *
   */
  void Sequence::dothread_startup( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_STARTUP );
    seq.startup( seq );
    seq.clr_thrstate_bit( THR_STARTUP );
    return;
  }
  /***** Sequencer::Sequence::dothread_startup ********************************/


  /***** Sequencer::Sequence::startup *****************************************/
  /**
   * @brief      performs nightly startup
   * @param[in]  seq  reference to Sequence object
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::startup( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::startup";
    std::stringstream message;
    std::string retstring;

    // there are certain conditions when the startup sequence cannot be run
    //
    if ( not seq.is_seqstate_set( Sequencer::SEQ_OFFLINE ) && not seq.is_seqstate_set( Sequencer::SEQ_READY ) ) {
      message << "ERROR: runstate " << this->seqstate_string( this->seqstate.load() ) << " must be OFFLINE or READY";
      seq.async.enqueue_and_log( function, message.str() );
      return( ERROR );
    }

    std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );                       // create a mutex object for waiting

    // clear the thread error state
    //
    seq.thr_error.store( THR_NONE );

    // Everything (except TCS) needs the power control to be running 
    // so initialize the power control first.
    // For this, set READY and WAIT_POWER bits, and clear OFFLINE bit.
    //
    seq.set_clr_seqstate_bit( Sequencer::SEQ_STARTING | Sequencer::SEQ_WAIT_POWER, Sequencer::SEQ_OFFLINE );
    seq.set_reqstate_bit( Sequencer::SEQ_STARTING );
    seq.clr_reqstate_bit( Sequencer::SEQ_OFFLINE );

    std::thread( dothread_power_init, std::ref(seq) ).detach();                   // start power initialization thread

    std::thread( dothread_wait_for_state, std::ref(seq) ).detach();               // wait for requested state

    logwrite( function, "[DEBUG] (3) waiting for power control to initialize" );

    while ( seq.seqstate.load() != seq.reqstate.load() ) {
      message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
      logwrite( function, message.str() );
      seq.cv.wait( wait_lock );
    }
    logwrite( function, "[DEBUG] (3) DONE waiting for power control to initialize" );

    // Don't proceed unless power control initialized successfully
    //
    if ( seq.thr_error.load() == THR_NONE ) {
      logwrite( function, "power control initialized" );
    }
    else {
      seq.async.enqueue_and_log( function, "ERROR: initializing power control. startup aborted" );
      this->ready_to_start = false;
      seq.set_clr_seqstate_bit( Sequencer::SEQ_OFFLINE, Sequencer::SEQ_STARTING );
      seq.set_reqstate_bit( Sequencer::SEQ_OFFLINE );
      seq.clr_reqstate_bit( Sequencer::SEQ_STARTING );
      return( ERROR );
    }

    // Start threads for acquisition and guiding.
    // Nothing is waiting on them, but rather they will be waiting on signals.
    //
    std::thread( dothread_guide, std::ref(seq) ).detach();

    // The following can be done in parallel.
    // Set the state bit before starting each thread, then
    // the thread will clear their bit when they complete.
    //
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );    std::thread( dothread_acam_init, std::ref(seq) ).detach();
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );    std::thread( dothread_slit_init, std::ref(seq) ).detach();
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );   std::thread( dothread_calib_init, std::ref(seq) ).detach();
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_TCS );     std::thread( dothread_tcs_init, std::ref(seq) ).detach();
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );  std::thread( dothread_camera_init, std::ref(seq) ).detach();
//  seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );   std::thread( dothread_focus_init, std::ref(seq) ).detach();
//  seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE ); std::thread( dothread_flexure_init, std::ref(seq) ).detach();

    // Now that the threads are running, wait until they are all finished.
    // When the SEQ_STARTING bit is the only bit set then we are ready.
    //
    std::thread( dothread_wait_for_state, std::ref(seq) ).detach();  // wait for requested state

    logwrite( function, "[DEBUG] (4) waiting for init threads to complete" );

    while ( seq.seqstate.load() != seq.reqstate.load() ) {
      message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
      logwrite( function, message.str() );
      seq.cv.wait( wait_lock );
    }
    logwrite( function, "[DEBUG] (4) DONE waiting for init threads to complete" );

    // all done now so clear the STARTING bits
    //
    logwrite( function, "init threads completed" );

    // if any thread returned an error then we are not ready
    //
    uint32_t te = seq.thr_error.load();
    if ( te != THR_NONE ) {
      message.str(""); message << "ERROR: startup failed because the following thread(s) had an error: " << this->thrstate_string( te );
      seq.async.enqueue_and_log( function, message.str() );
      seq.set_clr_seqstate_bit( Sequencer::SEQ_OFFLINE, Sequencer::SEQ_STARTING );
      seq.set_reqstate_bit( Sequencer::SEQ_OFFLINE );
      seq.clr_reqstate_bit( Sequencer::SEQ_STARTING );
      this->ready_to_start = false;
      seq.thr_error.store( THR_NONE );  // clear the thread error state
      return( ERROR );
    }
    else {
logwrite( function, "[DEBUG] setting READY bit" );
      seq.set_clr_seqstate_bit( Sequencer::SEQ_READY, Sequencer::SEQ_STARTING );
      seq.set_reqstate_bit( Sequencer::SEQ_READY );
      seq.clr_reqstate_bit( Sequencer::SEQ_STARTING );
      logwrite( function, "ready to start" );
      this->ready_to_start = true;
      return( NO_ERROR );
    }
  }
  /***** Sequencer::Sequence::startup *****************************************/


  /***** Sequencer::Sequence::dothread_shutdown *******************************/
  /**
   * @brief      
   * @param[in]  
   *
   */
  void Sequence::dothread_shutdown( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_SHUTDOWN );
    seq.shutdown( seq );
    seq.clr_thrstate_bit( THR_SHUTDOWN );
    return;
  }
  /***** Sequencer::Sequence::dothread_shutdown *******************************/


  /***** Sequencer::Sequence::shutdown ****************************************/
  /**
   * @brief      performs nightly shutdown
   * @param[in]  seq  reference to Sequence object
   * @return     ERROR or NO_ERROR
   * @details
   * The shutdown sequence puts hardware into safe conditions, such as closing dust
   * covers, etc., commands daemons to close their connections to hardware components,
   * disconnects the sequencer's connection to the daemons, then turns off the power
   * to hardware components, then finally closes and disconnects the power daemon.
   * This can only be performed in the READY state.
   *
   */
  long Sequence::shutdown( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::shutdown";
    std::stringstream message;
    std::string retstring;

    // shutdown sequence can only be run while ready
    //
    if ( not seq.is_seqstate_set( Sequencer::SEQ_READY ) ) {
      message << "ERROR: runstate " << this->seqstate_string( this->seqstate.load() ) << " must be READY";
      seq.async.enqueue_and_log( function, message.str() );
      return( ERROR );
    }

    // probably redundant
    //
    if ( not this->is_ready() ) {
      seq.async.enqueue_and_log( function, "ERROR: sequencer is already shut down" );
      return( ERROR );
    }

    std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );                       // create a mutex object for waiting

    // clear the thread error state
    //
    seq.thr_error.store( THR_NONE );

    seq.set_clr_seqstate_bit( Sequencer::SEQ_SHUTTING, Sequencer::SEQ_READY );    // set and clear seqstate bits
    seq.set_reqstate_bit( Sequencer::SEQ_SHUTTING );                              // set the requested state
    seq.clr_reqstate_bit( Sequencer::SEQ_READY );

    // The following can be done in parallel.
    // Set the state bit before starting each thread, then
    // the thread will clear their bit when they complete.
    //
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_ACAM );    std::thread( dothread_acam_shutdown,  std::ref(seq) ).detach();
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );   std::thread( dothread_calib_shutdown, std::ref(seq) ).detach();

    // Now that the shutdown-threads are running, wait until they are all finished.
    // When the SEQ_SHUTTING bit is the only bit set then we are ready to proceed.
    //
    std::thread( dothread_wait_for_state, std::ref(seq) ).detach();  // wait for requested state

    logwrite( function, "[DEBUG] (5) waiting for shutdown threads to complete" );

    while ( seq.seqstate.load() != seq.reqstate.load() ) {
      message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
      logwrite( function, message.str() );
      seq.cv.wait( wait_lock );
    }
    logwrite( function, "[DEBUG] (5) DONE waiting for shutdown threads to complete" );

    // Everything has undergone their shutdown procedure so safe to shut down the power daemon now
    //
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_POWER );   std::thread( dothread_power_shutdown, std::ref(seq) ).detach();

    // Wait for the power shutdown thread.
    // When the SEQ_SHUTTING bit is the only bit set then we are ready.
    //
    std::thread( dothread_wait_for_state, std::ref(seq) ).detach();  // wait for requested state

    logwrite( function, "[DEBUG] (6) waiting for power daemon to shut down" );

    while ( seq.seqstate.load() != seq.reqstate.load() ) {
      message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
      logwrite( function, message.str() );
      seq.cv.wait( wait_lock );
    }
    logwrite( function, "[DEBUG] (6) DONE waiting for power daemon to shut down" );

    // if any thread returned an error then we are not ready
    //
    uint32_t te = seq.thr_error.load();
    if ( te != THR_NONE ) {
      message.str(""); message << "ERROR: shutdown failed because the following thread(s) had an error: " << this->thrstate_string( te );
      seq.async.enqueue_and_log( function, message.str() );
logwrite( function, "[DEBUG] setting READY bit" );
      seq.set_seqstate_bit( Sequencer::SEQ_READY );
      seq.set_reqstate_bit( Sequencer::SEQ_READY );
      seq.thr_error.store( THR_NONE );  // clear the thread error state
      return( ERROR );
    }

    // otherwise we are now offline
    //
    seq.set_clr_seqstate_bit( Sequencer::SEQ_OFFLINE, Sequencer::SEQ_SHUTTING );  // set and clear seqstate
    seq.set_reqstate_bit( Sequencer::SEQ_OFFLINE );                               // set the requested state
    seq.clr_reqstate_bit( Sequencer::SEQ_SHUTTING );

    return( NO_ERROR );
  }
  /***** Sequencer::Sequence::shutdown ****************************************/


  /***** Sequencer::Sequence::parse_state *************************************/
  /**
   * @brief      parse the true|false state from a string reply
   * @param[in]  whoami  string is name of function using this
   * @param[in]  reply   string
   * @param[out] state   reference to bool state
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
      message.str(""); message << "ERROR parsing \"" << reply << "\" from " << whoami << ": expected <state> <error>";
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
  /***** Sequencer::Sequence::parse_state *************************************/


  /***** Sequencer::Sequence::radec_to_decimal ********************************/
  /**
   * @brief      convert string RA or DEC to decimal number
   * @param[in]  str_in  input string to convert
   * @return     double representation of string, or NaN on error
   *
   * Input string could be  HH:MM:SS.sss
   *                        HH MM SS.sss
   *                       ±DD:MM:SS.sss
   *                       ±DD MM SS.sss
   *                       ± D:MM:SS.sss
   *                       ± D MM SS.sss
   *
   * Convert the input string into a decimal (double) number, HH.hhh or ±DD.dddd
   *
   * If the string is empty or otherwise cannot be converted then return NaN.
   *
   */
  double Sequence::radec_to_decimal( std::string str_in ) {
    std::string dontcare;
    return( this->radec_to_decimal( str_in, dontcare ) );
  }
  /***** Sequencer::Sequence::radec_to_decimal ********************************/


  /***** Sequencer::Sequence::radec_to_decimal ********************************/
  /**
   * @brief      convert string RA or DEC to decimal number
   * @param[in]  str_in     input string to convert
   * @param[out] retstring  reference to string representation of return value
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
      message.str(""); message << "ERROR: expected 3 tokens but received " << tokens.size() << ": " << str_in;
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
      message.str(""); message << "ERROR: out of range parsing input string " << str_in << ": " << e.what();
      logwrite( function, message.str() );
      return( NAN );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR: invalid argument parsing input string " << str_in << ": " << e.what();
      logwrite( function, message.str() );
      return( NAN );
    }

    dec = sign * hh + mm + ss;
    ret << std::fixed << std::setprecision(6) << dec;
    retstring = ret.str();

    return( dec );
  }
  /***** Sequencer::Sequence::radec_to_decimal ********************************/


  /***** Sequencer::Sequence::extract_tcs_value *******************************/
  /**
   * @brief      extract the value from a tcsd reply "<val>" DONE|ERROR
   * @param[in]  reply  string from the tcs daemon
   * @param[out] value  reference to integer containing the extracted value
   * @return     ERROR or NO_ERROR
   *
   * The tcs daemon (tcsd) is the interface to the TCS.  Replies from tcsd 
   * include the response from the TCS followed by a space and DONE or ERROR,
   * for example "<val> DONE". This function extracts just the value "<val>"
   * from that reply. The value is returned by reference.
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
        logwrite( function, "ERROR received from tcsd" );
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

#ifdef LOGLEVEL_DEBUG  // this can be a little much when polling
//  message.str(""); message << "[DEBUG] from reply \"" << reply << "\" extracted value: " << value << " error=" << error;
//  logwrite( function, message.str() );
#endif

    return( error );
  }
  /***** Sequencer::Sequence::extract_tcs_value *******************************/


  /***** Sequencer::Sequence::parse_tcs_generic *******************************/
  /**
   * @brief      parses the generic tcs reply to most commands
   * @param[in]  value  int from extract_tcs_value()
   * @return     ERROR or NO_ERROR
   *
   * Parses the TCS reply that has been extracted by extract_tcs_value()
   *
   */
  long Sequence::parse_tcs_generic( int value ) {
    std::string function = "Sequencer::Sequence::parse_tcs_generic";
    std::stringstream message;
    std::string tcsreply;
    std::vector<std::string> tokens;

    if ( value == TCS_SUCCESS ) {
#ifdef LOGLEVEL_DEBUG
      logwrite( function, "[DEBUG] TCS successful completion" );
#endif
      return( NO_ERROR );
    }
    else {
      if ( value == TCS_UNRECOGNIZED_COMMAND ) {
        logwrite( function, "ERROR: TCS unrecognized command" );
      }
      else
      if ( value == TCS_INVALID_PARAMETER ) {
        logwrite( function, "ERROR: TCS invalid parameter(s)" );
      }
      else
      if ( value == TCS_UNABLE_TO_EXECUTE ) {
        logwrite( function, "ERROR: TCS unable to execute" );
      }
      else
      if ( value == TCS_HOST_UNAVAILABLE ) {
        logwrite( function, "ERROR: TCS Sparc host unavailable" );
      }
      else {
        message.str(""); message << "ERROR: " << value << " is not a valid TCS response";
        logwrite( function, message.str() );
      }
      return( ERROR );
    }
  }
  /***** Sequencer::Sequence::parse_tcs_generic *******************************/


  /***** Sequencer::Sequence::seqstate_string *********************************/
  /**
   * @brief      returns the string form of the states set in state word
   * @param[in]  state  word to check
   * @return     string
   *
   * This function serves only to make human-readable strings of the bits set
   * in the seqstate word.
   *
   */
  std::string Sequence::seqstate_string( uint32_t state ) {
    if ( state == 0 ) return "(none)";                     // no bits set
    std::stringstream message;
    for ( auto bit : this->sequence_state_bits ) {         // loop through all of the bits in the vector of bits
      if ( (1<<bit) & state ) {                            // if that bit is set in state then
        if ( not message.str().empty() ) message << " ";
        message << this->sequence_states[ bit ];           // get the string associated with that bit
      }
    }
    return( message.str() );
  }
  /***** Sequencer::Sequence::seqstate_string *********************************/


  /***** Sequencer::Sequence::thrstate_string *********************************/
  /**
   * @brief      returns the string form of the states set in thread state word
   * @param[in]  state  word to check
   * @return     string
   *
   * This function serves only to make human-readable strings of the bits set
   * in the seqstate word.
   *
   */
  std::string Sequence::thrstate_string( uint32_t state ) {
    if ( state == 0 ) return "(none)";                     // no need to check anything if the word is 0
    std::stringstream message;
    for ( auto bit : this->thread_state_bits ) {           // loop through all of the bits in the vector of bits
      if ( (1<<bit) & state ) {                            // if that bit is set in state then
        if ( not message.str().empty() ) message << " ";
        message << this->thread_states[ bit ];             // get the string associated with that bit
      }
    }
    return( message.str() );
  }
  /***** Sequencer::Sequence::thrstate_string *********************************/


  /***** Sequencer::Sequence::dotype ******************************************/
  /**
   * @brief      set or get do type (one/all)
   * @param[in]  args       input command and arguments
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   *
   * This function sets the state of the boolean do_once.
   *
   */
  long Sequence::dotype( std::string args ) {
    std::string function = "Sequencer::Sequence::dotype";
    std::stringstream message;
    std::string dontcare;
    return this->dotype( args, dontcare );
  }
  /***** Sequencer::Sequence::dotype ******************************************/


  /***** Sequencer::Sequence::dotype ******************************************/
  /**
   * @brief      set or get do type (one/all)
   * @param[in]  args       input command and arguments
   * @param[out] retstring  return value, "ONE" or "ALL"
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   *
   * This function sets or gets the state of the boolean do_once.
   * If no args are supplied then return the current state.
   * Although stored as a boolean, the interaction with the user is via
   * a string "one" or "all".
   *
   */
  long Sequence::dotype( std::string args, std::string &retstring ) {
    std::string function = "Sequencer::Sequence::dotype";
    std::stringstream message;
    long error = NO_ERROR;

    // if an arg was supplied then check it against the possible values "one" or "all"
    //
    if ( not args.empty() ) {

      std::transform( args.begin(), args.end(), args.begin(), ::toupper );  // make uppercase for consistency

      if ( args == "ONE" ) this->do_once.store( true );
      else
      if ( args == "ALL" ) this->do_once.store( false );
      else {
        message.str(""); message << "ERROR unrecognized argument " << args << ": expected {ONE|ALL}";
        logwrite( function, message.str() );
        error = ERROR;
      }
    }

    retstring = ( this->do_once.load() ? "ONE" : "ALL" );

    // send an async message with the current type
    //
    message.str(""); message << "DOTYPE: " << retstring;
    this->async.enqueue( message.str() );

    return( error );
  }
  /***** Sequencer::Sequence::dotype ******************************************/


  /***** Sequencer::Sequence::get_tcs_coords **********************************/
  /**
   * @brief      read the current TCS ra, dec
   * @param[in]  ra   RA in decimal hours
   * @param[in]  dec  DEC in decimal degrees
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::get_tcs_coords( double &ra_h, double &dec_d ) {
    return this->get_tcs_coords_type( TCSD_GET_COORDS, ra_h, dec_d );
  }
  long Sequence::get_tcs_weather_coords( double &ra_h, double &dec_d ) {
    return this->get_tcs_coords_type( TCSD_WEATHER_COORDS, ra_h, dec_d );
  }
  long Sequence::get_tcs_coords_type( std::string cmd, double &ra_h, double &dec_d ) {
    std::string function = "Sequencer::Sequence::get_tcs_coords";
    std::stringstream message;

    std::string radec;

    if ( this->tcsd.send( TCSD_GET_COORDS, radec ) != NO_ERROR ) {
      logwrite( function, "ERROR reading TCS coordinates" );
      return ERROR;
    }

    std::vector<std::string> tokens;

    Tokenize( radec, tokens, " " );                    // comes back as space-delimited string "hh:mm:ss.ss dd:mm:ss.ss"
    try {                                              // extract ra dec from radec string
      ra_h  = this->radec_to_decimal( tokens.at(0) );  //decimal hours
      dec_d = this->radec_to_decimal( tokens.at(1) );  // decimal degrees
    }
    catch( std::out_of_range &e ) {
      message << "ERROR out of range exception parsing \"" << radec << "\"";
      logwrite( function, message.str() );
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      message << "ERROR invalid argument exception parsing \"" << radec << "\"";
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::get_tcs_coords **********************************/


  /***** Sequencer::Sequence::angular_separation ******************************/
  /**
   * @brief      compute the angular separation between two points on a sphere
   * @details    used for checking telescope offsets. inputs are in decimal degrees.
   * @param[in]  ra1   RA of point1
   * @param[in]  dec1  DEC of point1
   * @param[in]  ra2   RA of point2
   * @param[in]  dec2  DEC of point2
   * @return     angular separation in arcsec
   *
   */
  double Sequence::angular_separation( double ra1, double dec1, double ra2, double dec2 ) {
    std::string function = "Sequencer::Sequence::angular_separation";
    std::stringstream message;

    message.str(""); message << "[ACQUIRE] goal ra1=" 
                             << std::fixed << std::setprecision(6)
                             << ra1 << " dec1=" << dec1 << " ra2=" << ra2 << " dec2=" << dec2;
    logwrite( function, message.str() );

    double pi = 2 * std::acos(0.0);

    // convert inputs to radians
    //
    ra1 *= pi/180.; dec1 *= pi/180.;
    ra2 *= pi/180.; dec2 *= pi/180.;

    double sdlon = std::sin( ra2 - ra1 );
    double cdlon = std::cos( ra2 - ra1 );
    double slat1 = std::sin( dec1 );
    double slat2 = std::sin( dec2 );
    double clat1 = std::cos( dec1 );
    double clat2 = std::cos( dec2 );

    double num1 = clat2 * sdlon;
    double num2 = clat1 * slat2 - slat1 * clat2 * cdlon;
    double numerator   = std::sqrt( std::pow( num1, 2 ) + std::pow( num2, 2 ) );
    double denominator = slat1 * slat2 + clat1 * clat2 * cdlon;

    double sep = std::atan2( numerator, denominator );  // separation in radians

    sep = sep * 3600. / ( pi/180. );                    // in arcsec

    return sep;
  }
  /***** Sequencer::Sequence::angular_separation ******************************/


  /***** Sequencer::Sequence::offset_tcs **************************************/
  /**
   * @brief      send guider offsets to the TCS using the PT command
   * @param[in]  ra   offset in RA in decimal degrees
   * @param[in]  dec  offset in DEC in decimal degrees
   * @return     ERROR or NO_ERROR
   *
   * There is no way of knowing accurately that we have arrived, and it might not
   * be perfectly reliable to catch the motion state flag transition to OFFSETTING
   * and then the TRACKING, so instead we wait. The PT command is sent and then
   * this function waits for the amount of time that offset should take, based on
   * the TCS_OFFSET_RATEs (aka "MRATE"s) in the config file, plust 25% for margin.
   *
   */
  long Sequence::offset_tcs( double ra_off, double dec_off ) {
    std::string function = "Sequencer::Sequence::offset_tcs";
    std::stringstream message;
    std::stringstream cmd;
    long error = NO_ERROR;

    // convert to arcsec for the TCS, and restrict to max 6000
    //
    ra_off  = ( ra_off*3600.  > 6000 ? 6000. : ra_off*3600.  );
    dec_off = ( dec_off*3600. > 6000 ? 6000. : dec_off*3600. );

    // Form and send the command to the TCS
    //
    cmd << "PT " << std::fixed << std::setprecision(6) << ra_off << " " << dec_off;

    message.str(""); message << "[ACQUIRE] sending " << cmd.str(); logwrite( function, message.str() );

    if ( this->tcsd.command( cmd.str() ) != NO_ERROR ) {
      logwrite( function, "ERROR sending PT offset command to TCS" );
      return ERROR;
    }

    // Now we wait.
    //
    double ratime     = ra_off  / this->tcs_offsetrate_ra;         // time to offset in RA
    double dectime    = dec_off / this->tcs_offsetrate_dec;        // time to offset in DEC
    double offsettime = ( ratime > dectime ? ratime  : dectime );  // only need to wait for the longest one

    offsettime += offsettime*0.25;                                 // add 25% for margin

    long sleeptime = (long) ( ceil(offsettime) * 1000000 );        // round up to next whole usec

    usleep( sleeptime );

    return error;
  }
  /***** Sequencer::Sequence::offset_tcs **************************************/


  /***** Sequencer::Sequence::test ********************************************/
  /**
   * @brief      test routines
   * @param[in]  args       input command and arguments
   * @param[out] retstring  any return values
   * @return     ERROR or NO_ERROR
   *
   * This is the place to put various debugging and system testing tools.
   *
   * The server command is "test", the next parameter is the test name,
   * and any parameters needed for the particular test are extracted as
   * tokens from the args string passed in.
   *
   * The input args string is tokenized and tests are separated by a simple
   * series of if..else.. conditionals.
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
    // async -- queue an asynchronous message
    // ----------------------------------------------------
    //
    if ( testname == "async" ) {
      if ( tokens.size() > 1 ) {
        if ( tokens.size() > 2 ) logwrite( function, "NOTICE multiple strings ignored, only the first will be queued" );
        logwrite( function, tokens[1] );
        this->async.enqueue( tokens[1] );
      }
      else {
        logwrite( function, "test" );
        this->async.enqueue( "test" );
      }
    }
    else

    // ----------------------------------------------------
    // states -- get the current seqstate and reqstate
    // ----------------------------------------------------
    //
    if ( testname == "states" ) {

      // get the seqstate and reqstate and put them (numerically) into the return string
      //
      message.str("");
      uint32_t ss = this->seqstate.load();
      message << ss << " ";
      uint32_t rs = this->reqstate.load();
      message << rs << " ";
      uint32_t ts = this->thrstate.load();
      message << ts << " ";
      retstring = message.str();

      // but now I'm going to write to the log (textually) which bits are set
      //
      message.str(""); message << "RUNSTATE: " << this->seqstate_string( ss );
      this->async.enqueue( message.str() );
      logwrite( function, message.str() );

      // same for the requested state
      //
      message.str(""); message << "REQSTATE: " << this->seqstate_string( rs );
      this->async.enqueue( message.str() );
      logwrite( function, message.str() );

      // and for the thread state
      //
      message.str(""); message << "THREADS: " << this->thrstate_string( ts );
      logwrite( function, message.str() );

      error = NO_ERROR;
    }
    else

    // ----------------------------------------------------
    // getnext -- get the next pending target from the database
    // ----------------------------------------------------
    //
    if ( testname == "getnext" ) {
      TargetInfo::TargetState ret;
      std::stringstream rts;
      if ( tokens.size() > 1 ) {
        ret = this->target.get_next( tokens[1] );  // if a state was supplied then get the next target with the supplied state
      }
      else {
        ret = this->target.get_next();             // otherwise use the default (which is "pending")
      }
      error = NO_ERROR;
      if ( ret == TargetInfo::TargetState::TARGET_FOUND )     { rts << this->target.name  << " " 
                                                                    << this->target.obsid << " " 
                                                                    << this->target.obsorder << " "
                                                                    << this->target.ra_hms << " "
                                                                    << this->target.dec_dms << " "
                                                                    << this->target.casangle << " "
                                                                    << this->target.slitangle; }
      else
      if ( ret == TargetInfo::TargetState::TARGET_NOT_FOUND ) { rts << "(none)"; }
      else
      if ( ret == TargetInfo::TargetState::TARGET_ERROR )     { error = ERROR; }

      retstring = rts.str();
    }
    else

    // ----------------------------------------------------
    // addrow -- insert a (fixed, hard-coded) row into the database
    // ----------------------------------------------------
    //
    if ( testname == "addrow" ) {
      int number=0;
      std::string name="";
      std::string ra="";
      std::string dec="";
      if ( tokens.size() != 5 ) {
        logwrite( function, "ERROR: expected <number> <name> <RA> <DEC>" );
        return( ERROR );
      }
      try {
        number = std::stoi( tokens.at(1) );
        name   = tokens.at(2);
        ra     = tokens.at(3);
        dec    = tokens.at(4);
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "out of range parsing args " << args << ": " << e.what();
        logwrite( function, message.str() );
      }
      catch( std::invalid_argument &e ) {
        message.str(""); message << "invalid argument parsing args " << args << ": " << e.what();
        logwrite( function, message.str() );
      }
      error = this->target.add_row( number, name, ra, dec );
    }
    else

    // ----------------------------------------------------
    // completed -- insert a record into completed observations table
    // ----------------------------------------------------
    //
    if ( testname == "completed" ) {
      error = this->target.update_state( Sequencer::TARGET_COMPLETE );
      if (error==NO_ERROR) error = this->target.insert_completed();

      // let the world know of the state change
      //
      message.str(""); message << "TARGETSTATE:" << this->target.state << " TARGET:" << this->target.name << " OBSID:" << this->target.obsid;
      this->async.enqueue( message.str() );
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

        // let the world know of the state change
        //
        message.str(""); message << "TARGETSTATE:" << this->target.state << " TARGET:" << this->target.name << " OBSID:" << this->target.obsid;
        this->async.enqueue( message.str() );
      }
    }
    else

    // ----------------------------------------------------
    // radec -- 
    // ----------------------------------------------------
    //
    if ( testname == "radec" ) {
      double ra,dec;
      ra  = this->radec_to_decimal( target.ra_hms );
      dec = this->radec_to_decimal( target.dec_dms );
      message.str(""); message << "ra " << target.ra_hms << " -> " 
                               << std::fixed << std::setprecision(6) 
                               << ra << "  dec " << target.dec_dms << " -> " << dec;
      logwrite( function, message.str() );
    }
    else

    // ----------------------------------------------------
    // notify -- send a notification signal to unblock all waiting threads
    // ----------------------------------------------------
    //
    if ( testname == "notify" ) {
      this->cv.notify_all();
    }
    else

    // ----------------------------------------------------
    // tablenames -- print the names of the tables in the DB
    // ----------------------------------------------------
    //
    if ( testname == "tablenames" ) {
      error = this->target.get_table_names();
    }
    else

    // -------------------------------------------------------
    // pause -- send async command to camera to pause exposure
    // -------------------------------------------------------
    //
    if ( testname == "pause" ) {
      error = this->camerad.async( "PEX" );
    }
    else

    // ---------------------------------------------------------
    // resume -- send async command to camera to resume exposure
    // ---------------------------------------------------------
    //
    if ( testname == "resume" ) {
      error = this->camerad.async( "REX" );
    }
    else

    // ---------------------------------------------------------
    // moveto -- spawn thread to move to target
    // ---------------------------------------------------------
    //
    if ( testname == "moveto" ) {
      logwrite( function, "spawning dothread_move_to_target..." );
      std::thread( dothread_move_to_target, std::ref(*this) ).detach();
    }
    else

    // ---------------------------------------------------------
    // acquire -- spawn thread to start acquisition
    // ---------------------------------------------------------
    //
    if ( testname == "acquire" ) {
      // Before starting acquire thread, must first send coordinates to the acam cameraserver
      //
      this->target.fpoffsets.coords_in.ra    = this->radec_to_decimal( this->target.ra_hms  ) * TO_DEGREES;
      this->target.fpoffsets.coords_in.dec   = this->radec_to_decimal( this->target.dec_dms );
      this->target.fpoffsets.coords_in.angle = this->target.slitangle;

      // can't be NaN
      //
      bool ra_isnan  = std::isnan( this->target.fpoffsets.coords_in.ra  );
      bool dec_isnan = std::isnan( this->target.fpoffsets.coords_in.dec );

      if ( ra_isnan || dec_isnan ) {
        message.str(""); message << "ERROR: converting";
        if ( ra_isnan  ) { message << " RA=\"" << this->target.ra_hms << "\""; }
        if ( dec_isnan ) { message << " DEC=\"" << this->target.dec_dms << "\""; }
        message << " to decimal";
        this->async.enqueue_and_log( function, message.str() );
        return ERROR;
      }

      // The acam cameraserver needs the coords converted slit->acam...
      //
      std::stringstream acam;
      std::string dontcare;

      error = this->target.fpoffsets.compute_offset( "SLIT", "ACAM" );               // convert coords_in from SLIT to ACAM

      acam << ACAMD_CAMERASERVER_COORDS << std::fixed << std::setprecision(6)
                                        << " " << this->target.fpoffsets.coords_out.ra
                                        << " " << this->target.fpoffsets.coords_out.dec
                                        << " " << this->target.fpoffsets.coords_out.angle;

      message.str(""); message << "[ACQUIRE] " << acam.str(); logwrite( function, message.str() );

      if ( error == NO_ERROR ) error = this->acamd.command( acam.str(), dontcare );  // send here, don't need the reply

      // Finally, spawn the acquisition thread
      //
      logwrite( function, "spawning dothread_acquisition..." );
      if (error==NO_ERROR) std::thread( dothread_acquisition, std::ref(*this) ).detach();
    }
    else

    // ---------------------------------------------------------
    // fpoffset -- 
    // ---------------------------------------------------------
    //
    if ( testname == "fpoffset" ) {
      std::string from, to;
      double ra,dec;
      ra  = this->radec_to_decimal( target.ra_hms ) * TO_DEGREES ;  // fpoffsets must be in degrees
      dec = this->radec_to_decimal( target.dec_dms );
      try {
        from = tokens.at(1);
        to   = tokens.at(2);
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "out of range parsing from/to from " << args << ": " << e.what();
        logwrite( function, message.str() );
      }
      catch( std::invalid_argument &e ) {
        message.str(""); message << "invalid argument parsing from/to from " << args << ": " << e.what();
        logwrite( function, message.str() );
      }
      this->target.fpoffsets.set_inputs( ra, dec, this->target.casangle );
      error = this->target.fpoffsets.compute_offset( from, to );
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
  /***** Sequencer::Sequence::test ********************************************/
}

/****
 
def angular_separation(lon1, lat1, lon2, lat2):
    ''' units = radians '''

    Angular separation between two points on a sphere.

    sdlon = np.sin(lon2 - lon1)
    cdlon = np.cos(lon2 - lon1)
    slat1 = np.sin(lat1)
    slat2 = np.sin(lat2)
    clat1 = np.cos(lat1)
    clat2 = np.cos(lat2)

    num1 = clat2 * sdlon
    num2 = clat1 * slat2 - slat1 * clat2 * cdlon
    denominator = slat1 * slat2 + clat1 * clat2 * cdlon

    return np.arctan2(np.hypot(num1, num2), denominator)

***/
