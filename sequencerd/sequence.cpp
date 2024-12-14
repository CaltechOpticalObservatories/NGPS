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
/***
  Sequence::Sequence() {
    this->seq_state.set( Sequencer::SEQ_OFFLINE );   /// offline
    this->req_state.set( Sequencer::SEQ_OFFLINE );   /// offline
    this->broadcast_seqstate();
    this->do_once.store( false );                    /// default to "do all"
/// this->is_tcs_ontarget.store( false );            /// default TCS target not ready to observe
    this->tcs_nowait.store( false );                 /// default to wait
    this->dome_nowait.store( false );                /// default to wait
    this->waiting_for_state.store( false );          /// not currently waiting for a state
    this->ready_to_start = false;                    /// the sequencer is not ready by default (needs nightly startup)
    this->notify_tcs_next_target = false;            /// default do not notify TCS of next target before end of exposure
    this->tcs_preauth_time = 0;                      /// default disable notifing TCS of next target's coords before exposure end
    this->tcs_settle_timeout = 10;                   /// telescope settling timeout (set by config file)
    this->tcs_settle_stable = 1;                     /// telescope stable time (set by config file)
    this->tcs_domeazi_ready = 1;                     /// max degrees azimuth that dome and telescope can differ before ready to observe
    this->tcs_offsetrate_ra = 45;                    /// TCS offset rate RA in arcsec per second
    this->tcs_offsetrate_dec = 45;                   /// TCS offset rate DEC in arcsec per second
    this->acquisition_timeout = 0;                   /// acquisition timeout (set by config file)
    this->acquisition_max_retrys = -1;               /// no retry limit by default (-1 disables) but can override in config file
    this->arm_readout_flag = false;                  /// disarm the readout flag to ignore async messages
    this->last_target="";
    this->test_solver_args="";
    this->tcs_name="offline";
  }
****/
  /***** Sequencer::Sequence **************************************************/


  /***** Sequencer::Sequence::dothread_monitor_ready_state ********************/
  /**
   * @brief      ensures that seqstate is changed (READY/OFFLINE) based on system_not_ready word
   * @details    spawned at startup by main() in sequencerd.cpp
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * @todo       !! this was an experiment and is not currently in use !!
   *             system_not_ready has been replaced with daemon_ready so consider
   *             that if this function gets revived.
   *
   */
  void Sequence::dothread_monitor_ready_state( Sequencer::Sequence &seq ) {
/*****
    // allow only one of these threads to run
    //
    if ( seq.thread_state.is_set( THR_MONITOR_READY_STATE ) ) {
      return;
    }

    seq.thread_state.set( THR_MONITOR_READY_STATE );            // thread running now

    std::string function = "Sequencer::Sequence::dothread_monitor_ready_state";
    std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );     // create a mutex object for waiting
    logwrite( function, "started thread" );

    while( true ) {
      seq.cv.wait( wait_lock );
      if ( seq.system_not_ready ) {
        if ( seq.is_seqstate_set( Sequencer::SEQ_READY ) ) {    // If seqstate is READY then
          seq.seqstate.fetch_or( Sequencer::SEQ_OFFLINE );      // set OFFLINE and
          seq.seqstate.fetch_and( ~Sequencer::SEQ_READY );      // clear READY.
        }
        if ( seq.is_reqstate_set( Sequencer::SEQ_READY ) ) {    // If reqstate is READY then
          seq.reqstate.fetch_or( Sequencer::SEQ_OFFLINE );      // set OFFLINE and
          seq.reqstate.fetch_and( ~Sequencer::SEQ_READY );      // clear READY.
        }
      }
      else {
        if ( seq.is_seqstate_set( Sequencer::SEQ_OFFLINE ) ) {  // If seqstate is OFFLINE then
          seq.seqstate.fetch_or( Sequencer::SEQ_READY );        // set READY and
          seq.seqstate.fetch_and( ~Sequencer::SEQ_OFFLINE );    // clear OFFLINE.
        }
        if ( seq.is_reqstate_set( Sequencer::SEQ_OFFLINE ) ) {  // If reqstate is OFFLINE then
          seq.reqstate.fetch_or( Sequencer::SEQ_READY );        // set READY and
          seq.reqstate.fetch_and( ~Sequencer::SEQ_OFFLINE );    // clear OFFLINE.
        }
      }
      seq.broadcast_seqstate();                                 // broadcast current state
    }

    seq.thread_state.clear( THR_MONITOR_READY_STATE );    // thread terminated
    return;
*****/
  }
  /***** Sequencer::Sequence::dothread_monitor_ready_state ********************/


///***** Sequencer::Sequence::set_seqstate_bit ********************************/
///**
///* @brief      atomically sets the requested bit in the seqstate word
///* @param[in]  mb  masked bit is uint32 word containing the bit to set
///*
///* The new state will be broadcast
///*
///*/
///oid Sequence::set_seqstate_bit( const uint32_t mb ) {
/// this->seqstate.fetch_or( mb );    // set
/// this->seqstate_cv.notify_all();   // notify waiting threads of the change
/// this->broadcast_seqstate();       // broadcast current state
///
///***** Sequencer::Sequence::set_seqstate_bit ********************************/


  /***** Sequencer::Sequence::broadcast_seqstate ******************************/
  /**
   * @brief      writes the seqstate string to the async port
   * @details    This broadcasts the seqstate as a string with the "RUNSTATE:"
   *             message tag.
   *
   */
  void Sequence::broadcast_seqstate() {
    std::stringstream message;
    message << "RUNSTATE: " << this->seq_state.get_set_names();
    this->async.enqueue_and_log( "Sequencer::Sequence::broadcast_seqstate", message.str() );
    return;
  }
  /***** Sequencer::Sequence::broadcast_seqstate ******************************/


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
    seq.thread_state.set( THR_SEQUENCER_ASYNC_LISTENER );      // thread running
    std::string function = "Sequencer::Sequence::dothread_sequencer_async_listener";
    std::stringstream message;

    int retval = udp.Listener();

    if ( retval < 0 ) {
      logwrite(function, "error creating UDP listening socket. thread terminating.");
      seq.thread_state.clear( THR_SEQUENCER_ASYNC_LISTENER );  // thread terminated
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
          double remaining = (double)( seq.target.exptime_req - stol( elapsedstr )/1000. );

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
          seq.seq_state.set_and_clear( Sequencer::SEQ_WAIT_READOUT, Sequencer::SEQ_WAIT_EXPOSE );  // set READOUT, cear EXPOSE
          seq.broadcast_seqstate();
        }

        // ---------------------------------------------
        // clear READOUT flag on the end-of-frame signal
        // ---------------------------------------------
        //
        if ( statstr.compare( 0, 10, "FRAMECOUNT" ) == 0 ) {                                    // async message tag FRAMECOUNT
          if ( seq.seq_state.is_set( Sequencer::SEQ_WAIT_READOUT ) ) {
            seq.seq_state.clear( Sequencer::SEQ_WAIT_READOUT );
            seq.broadcast_seqstate();
          }
        }

        // ---------------------
        // process TEST messages
        // ---------------------
        //
        if ( starts_with( statstr, "TEST:" ) ) {                                                // async message tag TEST
          message.str(""); message << "got test message \"" << statstr << "\"";
          logwrite( function, message.str() );
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

    }  // while true

    seq.thread_state.clear( THR_SEQUENCER_ASYNC_LISTENER );    // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_sequencer_async_listener ***************/


  void Sequence::dothread_test() {
    logwrite( "Sequencer::Sequence::dothread_test", "here I am" );
    std::string targetstatus;
    this->target.get_specified_target( "4430", targetstatus );
    logwrite( "Sequencer::Sequence::dothread_test", targetstatus );
    return;
  }
  /***** Sequencer::Sequence::dothread_sequence_start *************************/
  /**
   * @brief      main sequence start thread
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * This thread is spawned in response to the sequencer receiving the "start"
   * command. This thread will in turn spawn additional needed threads.
   *
   * This thread will run as long as the seq_state is RUNNING
   *
   */
  void Sequence::dothread_sequence_start() {
    this->thread_state.set( THR_SEQUENCE_START );                // thread running
    std::string function = "Sequencer::Sequence::dothread_sequence_start";
    std::stringstream message;
    std::string reply;
    std::string targetstatus;
    TargetInfo::TargetState targetstate;
    long error=NO_ERROR;

    // The daemon shouldn't try to start more than one instance of this but
    // the class has a mutex to guarantee singular access, so lock it now.
    // It will remain locked until it goes out of scope (when the thread ends).
    //
    std::lock_guard<std::mutex> start_lock (this->start_mtx);

    // clear the thread error state
    //
    this->thread_error.clear_all();

    // This is the main loop which runs as long as seq_state is running and
    // has the SEQ_RUNNING bit set and neither ABORTREQ nor STOPREQuested bits set.
    //
    while ( this->seq_state.is_set( Sequencer::SEQ_RUNNING ) &&
            this->seq_state.are_all_clear( Sequencer::SEQ_ABORTREQ, Sequencer::SEQ_STOPREQ ) ) {

      // If the ABORTREQ or STOPREQ bits are set then break the while loop
      //
      if ( this->seq_state.is_any_set( Sequencer::SEQ_ABORTREQ, Sequencer::SEQ_STOPREQ ) ) {
        logwrite( function, "stopping" );
        break;
      }

      logwrite( function, "sequencer running" );

      // Get the next target from the database
      //
      if ( this->cowboy.empty() ) {
        targetstate = this->target.get_next( targetstatus );
      }
      else {
        targetstate = this->target.get_specified_target( this->cowboy, targetstatus );
        this->lastcowboy=this->cowboy;
        this->cowboy.clear();
      }

      message.str(""); message << "NOTICE: " << targetstatus;
      this->async.enqueue( message.str() );                                 // broadcast target status

      if ( targetstate == TargetInfo::TARGET_FOUND ) {                    // target found, get the threads going

        // If the TCS is not ready and the target contains TCS coordinates,
        // then we cannot proceed.
        //
        if ( ! this->daemon_ready.is_set( Sequencer::DAEMON_TCS ) ) {
          if ( ! this->target.ra_hms.empty() || ! this->target.dec_dms.empty() ) {
            message.str(""); message << "ERROR: cannot move to target " << this->target.name << " because TCS has not been connected";
            this->async.enqueue_and_log( function, message.str() );
            this->thread_error.set( THR_SEQUENCE_START );                   // report error
            this->req_state.set( Sequencer::SEQ_STOPREQ );
            this->seq_state.set( Sequencer::SEQ_STOPREQ );
            this->broadcast_seqstate();                                     // broadcast current state
            break;
          }
        }

        error = this->target.update_state( Sequencer::TARGET_ACTIVE );      // set ACTIVE state in database (this says we are using this target)
        if (error!=NO_ERROR) {
          this->thread_error.set( THR_SEQUENCE_START );                     // report any error
          break;
        }

        // let the world know of the state change
        //
        message.str(""); message << "TARGETSTATE:" << this->target.state << " TARGET:" << this->target.name << " OBSID:" << this->target.obsid;
        this->async.enqueue( message.str() );
#ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] target found, starting threads" );
#endif
      }
      else
      if ( targetstate == TargetInfo::TARGET_NOT_FOUND ) {                // no target found is an automatic stop
        logwrite( function, "NOTICE: no targets found. stopping" );
        this->req_state.set( Sequencer::SEQ_STOPREQ );
        this->seq_state.set( Sequencer::SEQ_STOPREQ );
        this->broadcast_seqstate();                                         // broadcast current state
        break;
      }
      else
      if ( targetstate == TargetInfo::TARGET_ERROR ) {                    // request stop on error
        this->async.enqueue_and_log( function, "ERROR getting next target. stopping" );
        this->seq_state.set( Sequencer::SEQ_STOPREQ );
        this->req_state.set( Sequencer::SEQ_STOPREQ );
        this->broadcast_seqstate();                                         // broadcast current state
        break;
      }

      // get the threads going --
      //
      // These things can all be done in parallel, just have to sync up at the end.
      // Set the state bit before starting each thread, then
      // the thread will clear their bit when they complete
      //

      if ( this->target.pointmode == Acam::POINTMODE_ACAM ) {
        this->dotype( "ONE" );
        this->seq_state.set( Sequencer::SEQ_WAIT_TCS );
        this->broadcast_seqstate();
        std::thread( &Sequencer::Sequence::dothread_move_to_target, this ).detach();
        logwrite( function, "[DEBUG] (1) waiting on notification" );
        try {
          this->seq_state.wait_for_states_clear( Sequencer::SEQ_WAIT_TCS );
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "NOTICE: sequence aborted: " << e.what();
          this->async.enqueue_and_log( function, message.str() );
          this->seq_state.set_and_clear( {Sequencer::SEQ_READY}, { Sequencer::SEQ_RUNNING,
                                                                   Sequencer::SEQ_WAIT_TCS } );
          this->broadcast_seqstate();
          this->thread_state.clear( THR_SEQUENCE_START );          // thread terminated
          return;
        }
      }
      else {
        // For pointmode other than ACAM ("SLIT" or empty, which assumes SLIT)
        // set everything else.
        //
        this->seq_state.set( Sequencer::SEQ_WAIT_TCS,
                             Sequencer::SEQ_WAIT_CAMERA,
                             Sequencer::SEQ_WAIT_SLIT,
                             Sequencer::SEQ_WAIT_FOCUS,
                             Sequencer::SEQ_WAIT_FLEXURE,
                             Sequencer::SEQ_WAIT_CALIB );
        this->broadcast_seqstate();
        std::thread( &Sequencer::Sequence::dothread_move_to_target, this ).detach();
        std::thread( &Sequencer::Sequence::dothread_camera_set, this ).detach();
        std::thread( &Sequencer::Sequence::dothread_slit_set, this ).detach();
        std::thread( &Sequencer::Sequence::dothread_focus_set, this ).detach();
        std::thread( &Sequencer::Sequence::dothread_flexure_set, this ).detach();
        std::thread( &Sequencer::Sequence::dothread_calib_set, this ).detach();

        logwrite( function, "[DEBUG] (1) waiting on notification" );

        try {
          this->seq_state.wait_for_states_clear( Sequencer::SEQ_WAIT_TCS,
                                                 Sequencer::SEQ_WAIT_CAMERA,
                                                 Sequencer::SEQ_WAIT_SLIT,
                                                 Sequencer::SEQ_WAIT_FOCUS,
                                                 Sequencer::SEQ_WAIT_FLEXURE,
                                                 Sequencer::SEQ_WAIT_CALIB );
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "NOTICE: sequence aborted: " << e.what();
          this->async.enqueue_and_log( function, message.str() );
          this->seq_state.set_and_clear( {Sequencer::SEQ_READY}, { Sequencer::SEQ_RUNNING,
                                                                   Sequencer::SEQ_WAIT_TCS,
                                                                   Sequencer::SEQ_WAIT_CAMERA,
                                                                   Sequencer::SEQ_WAIT_SLIT,
                                                                   Sequencer::SEQ_WAIT_FOCUS,
                                                                   Sequencer::SEQ_WAIT_FLEXURE,
                                                                   Sequencer::SEQ_WAIT_CALIB } );
          this->broadcast_seqstate();
          this->thread_state.clear( THR_SEQUENCE_START );          // thread terminated
          return;
        }
      }

      logwrite(function, "[DEBUG] DONE waiting on notification");

      logwrite( function, "starting acquisition thread" );             ///< TODO @todo log to telemetry!

      this->seq_state.set( Sequencer::SEQ_WAIT_ACQUIRE );
      this->broadcast_seqstate();
      std::thread( &Sequencer::Sequence::dothread_acquisition, this ).detach();

      // Wait for on-target signal (or abort)
      //
      this->async.enqueue_and_log( function, "NOTICE: waiting for USER to send \"userexpose\" signal" );

      this->seq_state.set( Sequencer::SEQ_WAIT_USER );

      try {
        this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_USER );
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "NOTICE: aborted: " << e.what();
        this->async.enqueue_and_log( function, message.str() );
        this->seq_state.set_and_clear( {Sequencer::SEQ_READY}, { Sequencer::SEQ_WAIT_USER, Sequencer::SEQ_RUNNING } );
        this->broadcast_seqstate();
        this->thread_state.clear( THR_SEQUENCE_START );          // thread terminated
        return;
      }

      this->async.enqueue_and_log( function, "NOTICE: received USER ontarget signal!" );

      if ( this->target.pointmode == Acam::POINTMODE_SLIT ) {
        logwrite( function, "starting exposure" );       ///< TODO @todo log to telemetry!

        // Start the exposure in a thread...
        //
        this->seq_state.set( Sequencer::SEQ_WAIT_CAMERA );                   // set the current state
        this->broadcast_seqstate();

        logwrite( function, "[DEBUG] spawning dothread_trigger_exposure" );
        std::thread( &Sequencer::Sequence::dothread_trigger_exposure, this ).detach();  // trigger exposure in a thread

        this->req_state.set( Sequencer::SEQ_RUNNING );                       // set the requested state

        // ...then wait for it to complete
        //
        logwrite( function, "[DEBUG] (2) waiting on notification" );
        message.str(""); message << "current state: " << this->seq_state.get_set_names()
                                 << " waiting for: " << this->req_state.get_set_names();
        logwrite( function, message.str() );

        try {
          this->seq_state.wait_for_match( this->req_state );  // waits for seq_state == req_state
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "NOTICE: sequence aborted: " << e.what();
          this->async.enqueue_and_log( function, message.str() );
          break;
        }

        logwrite(function, "[DEBUG] (2) DONE waiting on notification");

        // Now that we're done waiting, check for errors or abort
        //
        if ( this->thread_error.is_any_set() ) {
          message.str(""); message << "ERROR stopping sequencer because the following thread(s) had an error: "
                                   << this->thread_error.get_set_names();
          logwrite( function, message.str() );
          break;
        }

        // When an exposure is aborted then it will be marked as UNASSIGNED
        //
        if ( this->seq_state.is_set( Sequencer::SEQ_ABORTREQ ) ) {
          logwrite( function, "ABORT requested" );
          // sets READY, clears ABORTREQ and RUNNING
          this->seq_state.set_and_clear( {Sequencer::SEQ_READY}, { Sequencer::SEQ_ABORTREQ, Sequencer::SEQ_RUNNING } );
          this->req_state.set_and_clear( {Sequencer::SEQ_READY}, { Sequencer::SEQ_ABORTREQ, Sequencer::SEQ_RUNNING } );
          this->broadcast_seqstate();

          error = this->target.update_state( Sequencer::TARGET_UNASSIGNED );
          message.str(""); message << ( error==NO_ERROR ? "" : "ERROR " ) << "marking target " << this->target.name
                                   << " id " << this->target.obsid << " order " << this->target.obsorder
                                   << " as " << Sequencer::TARGET_UNASSIGNED;
          logwrite( function, message.str() );

          // let the world know of the state change
          //
          message.str(""); message << "TARGETSTATE:" << this->target.state << " TARGET:" << this->target.name << " OBSID:" << this->target.obsid;
          this->async.enqueue( message.str() );

          this->thread_error.set( THR_SEQUENCE_START );  // do I really want to do this?
          break;
        }

        // If not aborted then this exposure is now complete
        //
        message.str(""); message << "exposure complete for target " << this->target.name
                                 << " id " << this->target.obsid << " order " << this->target.obsorder;
        logwrite( function, message.str() );

        // before writing to the completed database table, get current
        // telemetry from other daemons.
        //
        this->get_external_telemetry();

        // Update this target's state in the database
        //
        error = this->target.update_state( Sequencer::TARGET_COMPLETE );       // update the active target table
        if (error==NO_ERROR) error = this->target.insert_completed();          // insert into the completed table
        if (error!=NO_ERROR) this->thread_error.set( THR_SEQUENCE_START );     // report any error

        // let the world know of the state change
        //
        message.str(""); message << "TARGETSTATE:" << this->target.state << " TARGET:" << this->target.name << " OBSID:" << this->target.obsid;
        this->async.enqueue( message.str() );
      }  // end if ( this->target.pointmode == Acam::POINTMODE_ACAM )

      // Check the "dotype" --
      // If this was "do one" then do_once is set and get out now.
      //
      if ( this->do_once.load() ) {
        this->seq_state.set( Sequencer::SEQ_STOPREQ );
        this->req_state.set( Sequencer::SEQ_STOPREQ );
        this->broadcast_seqstate();
        logwrite( function, "stopping sequencer because single-step is selected" );
        break;
      }

    } // end while the SEQ_RUNNING bit is set in seqstate
#ifdef LOGLEVEL_DEBUG
    logwrite( function, "[DEBUG] I'm out of the main SEQ_RUNNING loop now" );
#endif

    if ( this->thread_error.is_any_set() ) {
      logwrite( function, "requesting stop because an error was detected" );
      if ( this->target.get_next( Sequencer::TARGET_ACTIVE, targetstatus ) == TargetInfo::TARGET_FOUND ) {  // If this target was flagged as active,
        this->target.update_state( Sequencer::TARGET_UNASSIGNED );                                          // then change it to unassigned on error.
      }
      this->thread_error.clear_all();     // clear the thread error state
      this->do_once.store(true);
      this->seq_state.set( Sequencer::SEQ_STOPREQ );
      this->req_state.set( Sequencer::SEQ_STOPREQ );
      this->broadcast_seqstate();
    }

    this->seq_state.set_and_clear( {Sequencer::SEQ_READY}, {Sequencer::SEQ_STOPREQ,Sequencer::SEQ_ABORTREQ,Sequencer::SEQ_RUNNING} );
    this->req_state.set_and_clear( {Sequencer::SEQ_READY}, {Sequencer::SEQ_STOPREQ,Sequencer::SEQ_ABORTREQ,Sequencer::SEQ_RUNNING} );
    this->broadcast_seqstate();

    logwrite( function, "target list processing has stopped" );
    this->thread_state.clear( THR_SEQUENCE_START );              // thread running
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
  void Sequence::dothread_camera_set() {
    this->thread_state.set( THR_CAMERA_SET );                    // thread running
    std::string function = "Sequencer::Sequence::dothread_camera_set";
    std::string reply;
    long error;
    std::stringstream camcmd;

    // send the EXPTIME command to camerad
    //
    // Everywhere is maintained that exptime is specified in sec except
    // the camera takes msec, so convert just before sending the command.
    //
    long exptime_msec = (long)( this->target.exptime_req * 1000 );
    camcmd.str(""); camcmd << CAMERAD_EXPTIME << " " << exptime_msec;

    logwrite( function, "sending "+camcmd.str() );

    error = this->camerad.send( camcmd.str(), reply );

    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR: unable to set exptime" );
      this->thread_error.set( THR_CAMERA_SET );
    }

    this->seq_state.clear( Sequencer::SEQ_WAIT_CAMERA );
    this->broadcast_seqstate();

    this->thread_state.clear( THR_CAMERA_SET );                  // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_camera_set *****************************/


  /***** Sequencer::Sequence::dothread_slit_set *******************************/
  /**
   * @brief      sets the slit according to the parameters in the target entry
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_slit_set() {
    this->thread_state.set( THR_SLIT_SET );                      // thread running
    std::string function = "Sequencer::Sequence::dothread_slit_set";
    std::string reply;
    long error;
    std::stringstream slitcmd;

#ifdef REMOVED_FOR_TESTING
    // send the SET command to slitd
    //
    slitcmd << SLITD_SET << " " << this->target.slitwidth_req << " " << this->target.slitoffset_req;

    logwrite( function, "sending: "+slitcmd.str() );

    error = this->slitd.command_timeout( slitcmd.str(), reply, SLITD_SET_TIMEOUT );

    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR: unable to set slit" );
      this->thread_error.set( THR_SLIT_SET );
    }
#endif

    this->seq_state.clear( Sequencer::SEQ_WAIT_SLIT );
    this->broadcast_seqstate();

    this->thread_state.clear( THR_SLIT_SET );                    // thread terminated
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
    seq.thread_state.set( THR_POWER_INIT );                    // thread running
    std::string function = "Sequencer::Sequence::dothread_power_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;
logwrite( function, "[DEBUG] starting" );  ///< TODO @todo temporary

message.str(""); message << "[DEBUG] powerd socket isconnected=" << seq.powerd.socket.isconnected(); ///< TODO @todo temporary
logwrite( function, message.str() ); ///< TODO @todo temporary

    bool was_opened=false;
    error = seq.check_connected(seq.powerd, was_opened);
/***
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
***/
    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize power control" );
logwrite( function, "[DEBUG] I should be setting thread_error here !!!!!!!!!!!!!!!" );
message.str(""); message << "[DEBUG] *before* thread_error=" << seq.thread_error.get_set_names(); logwrite( function, message.str() );
      seq.thread_error.set( THR_POWER_INIT );
message.str(""); message << "[DEBUG] *after* thread_error=" << seq.thread_error.get_set_names(); logwrite( function, message.str() );
    }
    else {
      seq.daemon_ready.set( Sequencer::DAEMON_POWER );         // this daemon is ready
    }

    logwrite( function, "ready" );
    seq.seq_state.clear( Sequencer::SEQ_WAIT_POWER );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_POWER_INIT );                  // thread terminated
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
    seq.thread_state.set( THR_POWER_SHUTDOWN );           // thread running
    std::string function = "Sequencer::Sequence::dothread_power_shutdown";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // If not connected to the power daemon then connect.
    // Even though we are shutting down, it's worth connecting in order to be able to send
    // the close command, to ensure that the hardware has been closed.
    //
    if ( !seq.powerd.socket.isconnected() ) {
      logwrite( function, "connecting to power daemon" );
      error = seq.powerd.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to power daemon" );
    }

    // close connections between powerd and the hardware with which it communicates
    //
    if ( error==NO_ERROR ) {
      logwrite( function, "closing power hardware" );
      error = seq.powerd.send( POWERD_CLOSE, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: closing connection to power hardware" );
    }

    // disconnect me from powerd irrespectively of any previous error
    //
    logwrite( function, "disconnecting from powerd" );
    seq.powerd.disconnect();

    // set this as not ready
    //
    seq.daemon_ready.clear( Sequencer::DAEMON_POWER );

    // set this thread's error status
    //
    if (error!=NO_ERROR) seq.thread_error.set( THR_POWER_SHUTDOWN );

    seq.seq_state.clear( Sequencer::SEQ_WAIT_POWER );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_POWER_SHUTDOWN );         // thread terminated
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
    seq.thread_state.set( THR_SLIT_INIT );                     // thread running
    std::string function = "Sequencer::Sequence::dothread_slit_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false, ishomed=false;

    // make sure hardware is powered on
    //
    error = seq.check_power_on(POWER_SLIT, std::chrono::seconds(5));

    bool was_opened=false;
    error = seq.check_connected(seq.slitd, was_opened);

/*****
    // if not connected to the slit daemon then connect
    //
    if ( error==NO_ERROR && !seq.slitd.socket.isconnected() ) {
      logwrite( function, "connecting to slit daemon" );
      error = seq.slitd.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to slit daemon" );
    }

    // Ask slitd if hardware connection is open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.slitd.command( SLITD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with slit hardware" );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to slit hardware" );
      error = seq.slitd.command( SLITD_OPEN, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: opening connection to slit hardware" );
    }
*****/

    // Ask slitd if the slit motors are homed,
    //
    if ( error == NO_ERROR ) {
      error  = seq.slitd.command( SLITD_ISHOME, reply );
      error |= seq.parse_state( function, reply, ishomed );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with slit hardware" );
    }

    // and send the HOME command to slitd if needed.
    //
    if ( error==NO_ERROR && !ishomed ) {
      logwrite( function, "sending home command" );
      error = seq.slitd.command_timeout( SLITD_HOME, reply, SLITD_HOME_TIMEOUT );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with slit hardware" );
    }

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: initializing slit control" );
      seq.thread_error.set( THR_SLIT_INIT );
    }
    else {
      seq.daemon_ready.set( Sequencer::DAEMON_SLIT );          // this daemon is ready
    }

    seq.seq_state.clear( Sequencer::SEQ_WAIT_SLIT );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_SLIT_INIT );                   // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_slit_init ******************************/


  /***** Sequencer::Sequence::dothread_slit_shutdown **************************/
  /**
   * @brief      shuts down the slit system
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_slit_shutdown( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_SLIT_SHUTDOWN );                       // thread running
    const std::string function = "Sequencer::Sequence::dothread_slit_shutdown";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // If not connected to the slit daemon then connect.
    // Even though we are shutting down, it's worth connecting in order to be able to send
    // the close command, to ensure that the hardware has been closed.
    //
    if ( !seq.slitd.socket.isconnected() ) {
      logwrite( function, "connecting to slit daemon" );
      error = seq.slitd.connect();                   // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR connecting to slit daemon" );
    }

    // send CLOSE command to slitd to close connections between slitd
    // and the hardware with which it communicates
    //
    if ( error==NO_ERROR ) {
      logwrite( function, "closing slit hardware" );
      error = seq.slitd.command( SLITD_CLOSE, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR closing down slit hardware connection" );
    }

    // disconnect me from slitd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from slitd" );
    seq.slitd.disconnect();

    // Turn off power to slit hardware.
    // Any error here is added to thread_error.
    //
    for ( const auto &plug : seq.power_switch[POWER_SLIT].plugname ) {
      long pwrerr=NO_ERROR;
      std::stringstream cmd;
      cmd << plug << " OFF";
      pwrerr = seq.powerd.send( cmd.str(), reply );
      if ( pwrerr != NO_ERROR ) {
        message.str(""); message << "ERROR turning off plug " << plug;
        seq.async.enqueue_and_log( function, message.str() );
        seq.thread_error.set( THR_SLIT_SHUTDOWN );
      }
    }

    // set this system as not ready
    //
    seq.daemon_ready.clear( Sequencer::DAEMON_SLIT );

    // set this thread's error status
    //
    if (error!=NO_ERROR) seq.thread_error.set( THR_SLIT_SHUTDOWN );

    seq.seq_state.clear( Sequencer::SEQ_WAIT_SLIT );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_SLIT_SHUTDOWN );                 // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_slit_shutdown **************************/


  /***** Sequencer::Sequence::dothread_slicecam_init **************************/
  /**
   * @brief      initializes the slicecam system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_slicecam_init( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_SLICECAM_INIT );                       // thread running
    std::string function = "Sequencer::Sequence::dothread_slicecam_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // make sure hardware is powered on
    //
    error = seq.check_power_on(POWER_SLICECAM, std::chrono::seconds(10));

    bool was_opened=false;
    error = seq.check_connected(seq.slicecamd, was_opened);

/*****
    // if not connected to the slicecam daemon then connect
    //
    if ( !seq.slicecamd.socket.isconnected() ) {
      logwrite( function, "connecting to slicecamd daemon" );
      error = seq.slicecamd.connect();                   // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR connecting sequencerd -> slicecamd" );
    }

    // TCS must have been initialized
    //
    if ( seq.tcs_name == "offline" ) {
      seq.async.enqueue_and_log( function, "ERROR sequencer has not initialized a TCS connection" );
      error = ERROR;
    }

    // Initialize slicecamd's connection to tcsd
    //
    if ( error == NO_ERROR ) {
      std::stringstream cmd;
      cmd << SLICECAMD_TCSINIT << " " << seq.tcs_name;
      error  = seq.slicecamd.send( cmd.str(), reply );
      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR initializing slicecamd <--> tcsd \"" << seq.tcs_name << "\"";
        seq.async.enqueue_and_log( function, message.str() );
      }
    }

    // Ask slicecamd if hardware connections are open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.slicecamd.send( SLICECAMD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR communicating with slicecam hardware" );
    }

    // and open it if necessary.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to slicecam hardware" );
      error = seq.slicecamd.command_timeout( SLICECAMD_OPEN, reply, SLICECAMD_OPEN_TIMEOUT );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR opening connection to slicecam hardware" );
    }
*****/

//  // turn on cooling
//  //
//  if ( error==NO_ERROR && !isopen ) {
//    logwrite( function, "turning on slicecam cooling" );
//    error = seq.slicecamd.send( SLICECAMD_TEMP+" -100", reply );
//    if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR requesting slicecam prologue" );
//  }

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize slicecam" );
      seq.thread_error.set( THR_SLICECAM_INIT );
    }
    else {
      seq.daemon_ready.set( Sequencer::DAEMON_SLICECAM );          // this daemon is ready
    }

    seq.seq_state.clear( Sequencer::SEQ_WAIT_SLICECAM );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_SLICECAM_INIT );                   // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_slicecam_init **************************/


  /***** Sequencer::Sequence::dothread_slicecam_shutdown **********************/
  /**
   * @brief      shuts down the slicecam system
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_slicecam_shutdown( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_SLICECAM_SHUTDOWN );                   // thread running
    const std::string function = "Sequencer::Sequence::dothread_slicecam_shutdown";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // If not connected to the slicecam daemon then connect.
    // Even though we are shutting down, it's worth connecting in order to be able to send
    // the close command, to ensure that the hardware has been closed.
    //
    if ( !seq.slicecamd.socket.isconnected() ) {
      logwrite( function, "connecting to slicecamd daemon" );
      error = seq.slicecamd.connect();                   // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR connecting to slicecamd daemon" );
    }

    // send SLICECAMD_SHUTDOWN command to slicecamd -- this will cause it to nicely
    // close connections between slicecamd and the hardware with which it communicates
    //
    if ( error==NO_ERROR ) {
      logwrite( function, "closing slicecam hardware" );
      error = seq.slicecamd.command_timeout( SLICECAMD_SHUTDOWN, reply, SLICECAMD_SHUTDOWN_TIMEOUT );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR shutting down slicecamd" );
    }

    // disconnect me from slicecamd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from slicecamd" );
    seq.slicecamd.disconnect();

    // Turn off power to slicecam hardware.
    // Any error here is added to thread_error.
    //
    for ( const auto &plug : seq.power_switch[POWER_SLICECAM].plugname ) {
      long pwrerr=NO_ERROR;
      std::stringstream cmd;
      cmd << plug << " OFF";
      pwrerr = seq.powerd.send( cmd.str(), reply );
      if ( pwrerr != NO_ERROR ) {
        message.str(""); message << "ERROR turning off plug " << plug;
        seq.async.enqueue_and_log( function, message.str() );
        seq.thread_error.set( THR_SLICECAM_SHUTDOWN );
      }
    }

    // set this system as not ready
    //
    seq.daemon_ready.clear( Sequencer::DAEMON_SLICECAM );

    // set this thread's error status
    //
    if (error!=NO_ERROR) seq.thread_error.set( THR_SLICECAM_SHUTDOWN );

    seq.seq_state.clear( Sequencer::SEQ_WAIT_SLICECAM );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_SLICECAM_SHUTDOWN );                 // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_slicecam_shutdown **********************/


  /***** Sequencer::Sequence::dothread_andor_init *****************************/
  /**
   * @brief      initializes the acam and slicecam systems in series
   * @details    This initializes the two systems in series to avoid contention
   *             with the USB drivers.
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_andor_init( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_ANDOR_INIT );                          // thread running
    std::string function = "Sequencer::Sequence::dothread_andor_init";
    std::stringstream message;
    std::string reply;

    // First the slicecam. It initializes quicker than acam, and the instrument
    // can operate without it.
    //
    seq.seq_state.set( Sequencer::SEQ_WAIT_SLICECAM );
    seq.broadcast_seqstate();
    std::thread( dothread_slicecam_init, std::ref(seq) ).detach();

    // Wait for slicecam to initialize
    //
    try {
      seq.seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_SLICECAM );
    }
    catch ( const std::exception &e ) {
      seq.async.enqueue_and_log( function, "NOTICE: andor startup aborted" );
      seq.seq_state.clear( Sequencer::SEQ_WAIT_SLICECAM );
      seq.seq_state.clear( Sequencer::SEQ_WAIT_ACAM );
      seq.broadcast_seqstate();
      seq.thread_state.clear( THR_ANDOR_INIT );                      // thread terminated
      return;
    }

    // Continue even if slicecam failed to initialize
    //
    if ( seq.thread_error.is_set( THR_SLICECAM_INIT ) ) {
      seq.thread_error.clear( THR_SLICECAM_INIT );
      seq.async.enqueue_and_log( function, "ERROR initializing slicecam" );
      seq.async.enqueue_and_log( function, "NOTICE: procdeding without slicecam" );
    }
    else logwrite( function, "slicecam started" );

    // acam takes longer because it has to home the filter wheel.
    //
    seq.seq_state.set( Sequencer::SEQ_WAIT_ACAM );
    seq.broadcast_seqstate();
    std::thread( dothread_acam_init, std::ref(seq) ).detach();

    // Wait for acam to initialize
    //
    try {
      seq.seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_ACAM );
    }
    catch ( const std::exception &e ) {
      seq.async.enqueue_and_log( function, "NOTICE: acam startup aborted" );
      seq.seq_state.clear( Sequencer::SEQ_WAIT_ACAM );
      seq.broadcast_seqstate();
      seq.thread_state.clear( THR_ANDOR_INIT );                      // thread terminated
      return;
    }

    // Can't continue without acam
    //
    if ( seq.thread_error.is_set( THR_ACAM_INIT ) ) {
      seq.async.enqueue_and_log( function, "ERROR initializing acam" );
    }
    else logwrite( function, "acam started" );

    seq.thread_state.clear( THR_ANDOR_INIT );                        // thread terminated
  }
  /***** Sequencer::Sequence::dothread_andor_init *****************************/


  /***** Sequencer::Sequence::dothread_acam_init ******************************/
  /**
   * @brief      initializes the acam system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_acam_init( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_ACAM_INIT );                       // thread running
    std::string function = "Sequencer::Sequence::dothread_acam_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // make sure hardware is powered on
    //
    error = seq.check_power_on(POWER_ACAM, std::chrono::seconds(10));

    bool was_opened=false;
    error = seq.check_connected(seq.acamd, was_opened);

/*****
    // if not connected to the acam daemon then connect
    //
    if ( !seq.acamd.socket.isconnected() ) {
      logwrite( function, "connecting to acamd daemon" );
      error = seq.acamd.connect();                   // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR connecting sequencerd -> acamd" );
    }

    // TCS must have been initialized
    //
    if ( seq.tcs_name == "offline" ) {
      seq.async.enqueue_and_log( function, "ERROR sequencer has not initialized a TCS connection" );
      error = ERROR;
    }

    // Initialize acamd's connection to tcsd
    //
    if ( error == NO_ERROR ) {
      std::stringstream cmd;
      cmd << ACAMD_TCSINIT << " " << seq.tcs_name;
      error  = seq.acamd.command( cmd.str(), reply );
      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR initializing acamd <--> tcsd \"" << seq.tcs_name << "\"";
        seq.async.enqueue_and_log( function, message.str() );
      }
    }

    // Ask acamd if hardware connections are open,
    //
    if ( error == NO_ERROR ) {
      error  = seq.acamd.command( ACAMD_ISOPEN, reply );
      error |= seq.parse_state( function, reply, isopen );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR communicating with acam hardware" );
    }

    // and open it if necessary, overriding the timeout because this can
    // take a long time if the filter wheel needs to be homed.
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "connecting to acam hardware" );
      error = seq.acamd.command_timeout( ACAMD_OPEN, ACAMD_OPEN_TIMEOUT );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR opening connection to acam hardware" );
    }
******/

    // turn on cooling
    //
    if ( error==NO_ERROR && !isopen ) {
      logwrite( function, "turning on acam cooling" );
      error = seq.acamd.command( ACAMD_TEMP+" -100", reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR requesting acam prologue" );
    }

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize acam" );
      seq.thread_error.set( THR_ACAM_INIT );
    }
    else {
      seq.daemon_ready.set( Sequencer::DAEMON_ACAM );          // this daemon is ready
    }

    seq.seq_state.clear( Sequencer::SEQ_WAIT_ACAM );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_ACAM_INIT );                   // thread terminated
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
    seq.thread_state.set( THR_ACAM_SHUTDOWN );                       // thread running
    const std::string function = "Sequencer::Sequence::dothread_acam_shutdown";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // If not connected to the acam daemon then connect.
    // Even though we are shutting down, it's worth connecting in order to be able to send
    // the close command, to ensure that the hardware has been closed.
    //
    if ( !seq.acamd.socket.isconnected() ) {
      logwrite( function, "connecting to acamd daemon" );
      error = seq.acamd.connect();                   // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR connecting to acamd daemon" );
    }

    // send ACAMD_SHUTDOWN command to acamd -- this will cause it to nicely
    // close connections between acamd and the hardware with which it communicates
    //
    if ( error==NO_ERROR ) {
      logwrite( function, "closing acam hardware" );
      error = seq.acamd.command_timeout( ACAMD_SHUTDOWN, ACAMD_SHUTDOWN_TIMEOUT );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR shutting down acam" );
    }

    // disconnect me from acamd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from acamd" );
    seq.acamd.disconnect();

    // Turn off power to acam hardware.
    // Any error here is added to thread_error.
    //
    for ( const auto &plug : seq.power_switch[POWER_ACAM].plugname ) {
      long pwrerr=NO_ERROR;
      std::stringstream cmd;
      cmd << plug << " OFF";
      pwrerr = seq.powerd.command( cmd.str() );
      if ( pwrerr != NO_ERROR ) {
        message.str(""); message << "ERROR turning off plug " << plug;
        seq.async.enqueue_and_log( function, message.str() );
        seq.thread_error.set( THR_ACAM_SHUTDOWN );
      }
    }

    // set this system as not ready
    //
    seq.daemon_ready.clear( Sequencer::DAEMON_ACAM );

    // set this thread's error status
    //
    if (error!=NO_ERROR) seq.thread_error.set( THR_ACAM_SHUTDOWN );

    seq.seq_state.clear( Sequencer::SEQ_WAIT_ACAM );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_ACAM_SHUTDOWN );                     // thread terminated
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
    seq.thread_state.set( THR_CALIB_INIT );                    // thread running
    std::string function = "Sequencer::Sequence::dothread_calib_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // make sure hardware is powered on
    //
    error = seq.check_power_on(POWER_CALIB, std::chrono::seconds(5));

    // open connection to calib. this will home motors if necessary.
    //
    bool was_opened=false;
    error = seq.check_connected(seq.calibd, was_opened);

    // if calibd was just opened, close the door and open the cover
    //
    if ( error==NO_ERROR && was_opened ) {
      logwrite( function, "closing calib door and opening slit cover" );
      message.str(""); message << CALIBD_SET << " cover=open door=close";
      error = seq.calibd.command_timeout( message.str(), CALIBD_SET_TIMEOUT );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR moving calib door and/or cover" );
    }

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR unable to initialize calibrator" );
      seq.thread_error.set( THR_CALIB_INIT );
    }
    else {
      seq.daemon_ready.set( Sequencer::DAEMON_CALIB );         // ready
    }

    seq.seq_state.clear( Sequencer::SEQ_WAIT_CALIB );
    seq.broadcast_seqstate();

    if ( error==NO_ERROR ) logwrite( function, "calib ready" );

    seq.thread_state.clear( THR_CALIB_INIT );                  // thread terminated
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
    seq.thread_state.set( THR_CALIB_SHUTDOWN );                // thread running
    std::string function = "Sequencer::Sequence::dothread_calib_shutdown";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // If not connected to the calib daemon then connect.
    // Even though we are shutting down, it's worth connecting in order to be able to send
    // the close command, to ensure that the hardware has been closed.
    //
    if ( !seq.calibd.socket.isconnected() ) {
      logwrite( function, "connecting to calib daemon" );
      error = seq.calibd.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to calib daemon" );
    }

    // ensure door and cover are closed
    //
    if ( error==NO_ERROR ) {
      logwrite( function, "closing calib door and cover" );
      message.str(""); message << CALIBD_SET << " cover=close door=close";
      error = seq.calibd.command_timeout( message.str(), CALIBD_SET_TIMEOUT );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR closing calib door and/or cover" );
    }

    // close connections between calibd and the hardware with which it communicates
    //
    if ( error==NO_ERROR ) {
      logwrite( function, "closing calib hardware" );
      error = seq.calibd.send( CALIBD_CLOSE, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: closing connection to calib hardware" );
    }

    // disconnect me from calibd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from calibd" );
    seq.calibd.disconnect();

    // Turn off power to calib hardware.
    // Any error here is added to thread_error.
    //
    for ( const auto &plug : seq.power_switch[POWER_CALIB].plugname ) {
      long pwrerr=NO_ERROR;
      std::stringstream cmd;
      cmd << plug << " OFF";
      pwrerr = seq.powerd.send( cmd.str(), reply );
      if ( pwrerr != NO_ERROR ) {
        message.str(""); message << "ERROR turning off plug " << plug;
        seq.async.enqueue_and_log( function, message.str() );
        seq.thread_error.set( THR_CALIB_SHUTDOWN );
      }
    }

    // set this system as not ready
    //
    seq.daemon_ready.clear( Sequencer::DAEMON_CALIB );

    // set this thread's error status
    //
    if (error!=NO_ERROR) seq.thread_error.set( THR_CALIB_SHUTDOWN );

    seq.seq_state.clear( Sequencer::SEQ_WAIT_CALIB );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_CALIB_SHUTDOWN );              // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_calib_shutdown *************************/


  /***** Sequencer::Sequence::dothread_tcs_init *******************************/
  /**
   * @brief      initializes the tcs system for control from the Sequencer
   * @param[in]  seq    reference to Sequencer::Sequence object
   * @param[in]  which  which TCS to initialize, real|sim. Check-only if empty.
   *
   * @details    If the "which" parameter is empty then this thread cannot
   *             actually initialize the TCS but will only check if it has
   *             already been initialized. Successful initialization requires
   *             that the which param has been specified {real|sim} to indicate
   *             which TCS device to initialize.
   *
   *             The named devices {real,sim} must have been defined in the
   *             configuration file.
   *
   */
  void Sequence::dothread_tcs_init( Sequencer::Sequence &seq, std::string which ) {
    seq.thread_state.set( THR_TCS_INIT );                      // thread running
    std::string function = "Sequencer::Sequence::dothread_tcs_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isconnected=false;  // are we connected to tcsd?
    bool isopen=false;       // has tcsd opened a connection to the TCS?

    // Check if connected to the tcs daemon
    //
    if ( !seq.tcsd.socket.isconnected() ) {
      logwrite( function, "not connected to tcs daemon" );
      seq.async.enqueue( "TCSD:isopen:false" );            // added here for consistency

      if ( ! which.empty() ) {
        logwrite( function, "connecting to tcs daemon" );
        error = seq.tcsd.connect();                        // connect to the daemon
        if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to tcs daemon" );
        else isconnected = true;
      }
    }
    else {
      isconnected = true;
      logwrite( function, "connected to tcsd" );
    }

    // Ask tcsd if hardware connection is open.
    // The response will be "name|false|ERROR" where name is the name of the TCS device
    // if connected to that device.
    //
    if ( error == NO_ERROR && isconnected ) {
      error = seq.tcsd.send( TCSD_ISOPEN, reply );
      if ( error != ERROR && reply.find( "false" ) != std::string::npos ) isopen = false;
      else
      if ( error != ERROR && reply.find( "ERROR" ) != std::string::npos ) {
        isopen = false;
        error = ERROR;
        logwrite( function, "no connection open to TCS" );
      }
      else if ( error != ERROR ) {
        // if the currently opened device is not the requested device then close it
        //
        if ( reply != which ) {
          error = seq.tcsd.send( TCSD_CLOSE, reply );
          isopen = false;
          message.str(""); message << "closed connection to TCS " << reply;
          logwrite( function, message.str() );
        }
        else {
          isopen = true;
          message.str(""); message << "connection open to TCS " << which;
          logwrite( function, message.str() );
        }
      }
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with TCS" );
    }

    // and open it if necessary.
    //
    // This will only be attempted if the "which" param is supplied.
    // Note that the contents of "which" are not checked here, that is left
    // for tcsd to check -- he knows best.
    //
    if ( error==NO_ERROR && !which.empty() && !isopen ) {
      message.str(""); message << "connecting to " << which << " TCS hardware";
      logwrite( function, message.str() );
      std::stringstream opencmd;
      opencmd << TCSD_OPEN << " " << which;
      error = seq.tcsd.send( opencmd.str(), reply );
      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR:connecting to " << which << " TCS hardware";
        seq.async.enqueue_and_log( function, message.str() );
      }
      else isopen = true;  // don't need to ask again because if TCSD_OPEN returned no error then it's open
    }

    // set the offset rates
    //
    if ( error==NO_ERROR && isconnected && isopen && !which.empty() ) {
      message.str(""); message << TCSD_OFFSETRATE << " " << seq.tcs_offsetrate_ra << " " << seq.tcs_offsetrate_dec;
      error  = seq.tcsd.send( message.str(), reply );
    }

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      if ( which.empty() ) seq.async.enqueue_and_log( function, "ERROR:reading TCS initialization state" );
      else                 seq.async.enqueue_and_log( function, "ERROR:unable to initialize TCS control" );
      seq.thread_error.set( THR_TCS_INIT );
    }

    // Allowing clearing the SEQ_WAIT_TCS bit when connected and open.
    //
    if ( isconnected && isopen ) {
      seq.tcs_name = which;
      seq.seq_state.clear( Sequencer::SEQ_WAIT_TCS );
      seq.broadcast_seqstate();
      message.str(""); message << which << " TCS is initialized";
      logwrite( function, message.str() );
      seq.daemon_ready.set( Sequencer::DAEMON_TCS );           // ready
      seq.cv.notify_all();
    }

    seq.thread_state.clear( THR_TCS_INIT );                    // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_tcs_init *******************************/


  /***** Sequencer::Sequence::dothread_tcs_shutdown ***************************/
  /**
   * @brief      shuts down the tcs connection
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_tcs_shutdown( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_TCS_SHUTDOWN );           // thread running
    std::string function = "Sequencer::Sequence::dothread_tcs_shutdown";
    std::stringstream message;
    std::string reply;
    long error = NO_ERROR;

    // If not connected to the tcs daemon then connect.
    // Even though we are shutting down, it's worth connecting in order to be able to send
    // the close command, to ensure that the hardware has been closed.
    //
    if ( !seq.tcsd.socket.isconnected() ) {
      logwrite( function, "connecting to tcs daemon" );
      error = seq.tcsd.connect();                       // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: connecting to tcs daemon" );
    }

    if ( error == NO_ERROR ) {
      error  = seq.tcsd.send( TCSD_CLOSE, reply );      // send CLOSE command
      error |= seq.tcsd.send( TCSD_ISOPEN, reply );     // send ISOPEN to force a broadcast message of TCS status
    }

    // set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR:shutting down TCS" );
      seq.thread_error.set( THR_TCS_SHUTDOWN );
    }
    else {
      seq.tcs_name = "offline";
      seq.daemon_ready.clear( Sequencer::DAEMON_TCS );
    }

    seq.seq_state.clear( Sequencer::SEQ_WAIT_TCS );     // clear the TCS waiting bit
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_TCS_SHUTDOWN );         // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_tcs_shutdown ***************************/


  /***** Sequencer::Sequence::dothread_flexure_init ***************************/
  /**
   * @brief      initializes the flexure system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_flexure_init( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_FLEXURE_INIT );                  // thread running
    std::string function = "Sequencer::Sequence::dothread_flexure_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // make sure hardware is powered on
    //
    error = seq.check_power_on(POWER_FLEXURE, std::chrono::seconds(21));

    bool was_opened=false;
    error = seq.check_connected(seq.flexured, was_opened);

/*****
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
*****/

    // flag daemon as ready or set thread_error so the main thread knows we had an error or not
    //
    if ( error == NO_ERROR ) {
      seq.daemon_ready.set( Sequencer::DAEMON_FLEXURE );
    }
    else {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize flexure control" );
      seq.thread_error.set( THR_FLEXURE_INIT );
    }

    seq.seq_state.clear( Sequencer::SEQ_WAIT_FLEXURE );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_FLEXURE_INIT );                // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_flexure_init ***************************/


  /***** Sequencer::Sequence::dothread_flexure_shutdown ***********************/
  /**
   * @brief      shuts down the flexure system
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_flexure_shutdown( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_FLEXURE_SHUTDOWN );              // thread running
    std::string function = "Sequencer::Sequence::dothread_flexure_shutdown";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    // If not connected to the flexure daemon then connect.
    // Even though we are shutting down, it's worth connecting in order to be able to send
    // the close command, to ensure that the hardware has been closed.
    //
    if ( !seq.flexured.socket.isconnected() ) {
      logwrite( function, "connecting to flexure daemon" );
      error = seq.flexured.connect();                  // connect to the daemon
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR connecting to flexure daemon" );
    }

    // close connections between flexured and the hardware with which it communicates
    //
    if ( error==NO_ERROR ) {
      logwrite( function, "closing flexure hardware" );
      error = seq.flexured.command( FLEXURED_CLOSE, reply );
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: closing connection to flexure hardware" );
    }

    // disconnect me from flexured, irrespective of any previous error
    //
    logwrite( function, "disconnecting from flexured" );
    seq.flexured.disconnect();

    // Turn off power to flexure hardware.
    // Any error here is added to thread_error.
    //
    for ( const auto &plug : seq.power_switch[POWER_FLEXURE].plugname ) {
      long pwrerr=NO_ERROR;
      std::stringstream cmd;
      cmd << plug << " OFF";
      pwrerr = seq.powerd.send( cmd.str(), reply );
      if ( pwrerr != NO_ERROR ) {
        message.str(""); message << "ERROR turning off plug " << plug;
        seq.async.enqueue_and_log( function, message.str() );
        seq.thread_error.set( THR_FLEXURE_SHUTDOWN );
      }
    }

    // set this system as not ready
    //
    seq.daemon_ready.clear( Sequencer::DAEMON_FLEXURE );

    // set this thread's error status
    //
    if (error!=NO_ERROR) seq.thread_error.set( THR_FLEXURE_SHUTDOWN );

    seq.seq_state.clear( Sequencer::SEQ_WAIT_FLEXURE );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_FLEXURE_SHUTDOWN );            // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_flexure_shutdown ***********************/


  /***** Sequencer::Sequence::dothread_focus_init *****************************/
  /**
   * @brief      initializes the focus system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_focus_init( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_FOCUS_SET );                     // thread running
    std::string function = "Sequencer::Sequence::dothread_focus_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // make sure hardware is powered on
    //
    error = seq.check_power_on(POWER_FOCUS, std::chrono::seconds(5));

    bool was_opened=false;
    error = seq.check_connected(seq.focusd, was_opened);

/*****
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
*****/

    // flag daemon as ready or set thread_error so the main thread knows we had an error or not
    //
    if ( error == NO_ERROR ) {
      seq.daemon_ready.set( Sequencer::DAEMON_FOCUS );
    }
    else {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize focus control" );
      seq.thread_error.set( THR_FOCUS_SET );
    }

    seq.seq_state.clear( Sequencer::SEQ_WAIT_FOCUS );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_FOCUS_SET );                   // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_focus_init *****************************/


  /***** Sequencer::Sequence::dothread_focus_shutdown *************************/
  /**
   * @brief      shuts down the focus system
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_focus_shutdown( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_FOCUS_SHUTDOWN );                     // thread running
    const std::string function = "Sequencer::Sequence::dothread_focus_shutdown";

    // close connections between focusd and the hardware with which it communicates
    //
    if ( seq.focusd.command( FOCUSD_CLOSE ) != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR closing focus hardware" );
      seq.thread_error.set( THR_FOCUS_SHUTDOWN );
    }
    else {
      seq.daemon_ready.clear( Sequencer::DAEMON_FOCUS );
    }

    // disconnect me from focusd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from focusd" );
    seq.focusd.disconnect();

    seq.seq_state.clear( Sequencer::SEQ_WAIT_FOCUS );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_FOCUS_SHUTDOWN );                   // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_focus_shutdown *************************/


  /***** Sequencer::Sequence::dothread_camera_init ****************************/
  /**
   * @brief      initializes the camera system for control from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_camera_init( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_CAMERA_INIT );                   // thread running
    std::string function = "Sequencer::Sequence::dothread_camera_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // make sure hardware is powered on
    //
    error = seq.check_power_on(POWER_CAMERA, std::chrono::seconds(5));

    bool was_opened=false;
    error = seq.check_connected(seq.camerad, was_opened);

/*****
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
      error = seq.camerad.send( CAMERAD_OPEN, reply );   // This will open all devices configured with CONTROLLER key in config file
      if ( error != NO_ERROR ) seq.async.enqueue_and_log( function, "ERROR: communicating with science camera controller(s)" );
    }
*****/

    // send all of the prologue commands
    //
    if ( was_opened) {
      for ( const auto &cmd : seq.camera_prologue ) {
        if (error==NO_ERROR) error = seq.camerad.send( cmd, reply );
      }
    }

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      seq.async.enqueue_and_log( function, "ERROR: unable to initialize camera" );
      seq.thread_error.set( THR_CAMERA_INIT );
    }
    else {
      seq.daemon_ready.set( Sequencer::DAEMON_CAMERA );        // ready
    }

    seq.seq_state.clear( Sequencer::SEQ_WAIT_CAMERA );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_CAMERA_INIT );                 // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_camera_init ****************************/


  /***** Sequencer::Sequence::dothread_camera_shutdown ************************/
  /**
   * @brief      shuts down the camera system from the Sequencer
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   */
  void Sequence::dothread_camera_shutdown( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_CAMERA_SHUTDOWN );                     // thread running
    const std::string function = "Sequencer::Sequence::dothread_camera_shutdown";

    // Are any cameras on?
    //
    bool poweron=false;
    for ( const auto &plug : seq.power_switch[POWER_CAMERA].plugname ) {
      std::string reply;
      if ( seq.powerd.send( plug, reply ) == NO_ERROR ) {
        if ( reply.find("1") != std::string::npos ) {
          poweron=true;
          break;
        }
      }
    }

    // if already off then get out now
    //
    if ( !poweron ) {
      seq.daemon_ready.clear( Sequencer::DAEMON_CAMERA );
      seq.seq_state.clear( Sequencer::SEQ_WAIT_CAMERA );
      seq.broadcast_seqstate();
      seq.thread_state.clear( THR_CAMERA_SHUTDOWN );                   // thread terminated
      return;
    }

    // If not connected to the daemon then connect.
    // Even though we are shutting down, it's worth connecting in order to be able to send
    // the close command, to ensure that the hardware has been closed.
    //
    if ( !seq.camerad.socket.isconnected() ) {
      logwrite( function, "connecting to camera daemon" );
      if ( seq.camerad.connect() == NO_ERROR ) {
        seq.camerad.command( CAMERAD_OPEN );
      }
    }

    // send all of the epilogue commands
    //
    for ( const auto &cmd : seq.camera_epilogue ) {
      seq.camerad.command( cmd );
    }

    // disconnect me from camerad, irrespective of any previous error
    //
    logwrite( function, "disconnecting from camerad" );
    seq.camerad.disconnect();

    // Turn off power to camera hardware.
    // Any error here is added to thread_error.
    //
    for ( const auto &plug : seq.power_switch[POWER_CAMERA].plugname ) {
      std::string reply;
      if ( seq.powerd.send( plug+" OFF", reply ) != NO_ERROR ) {
        seq.async.enqueue_and_log( function, "ERROR turning off plug "+plug );
        seq.thread_error.set( THR_CAMERA_SHUTDOWN );
      }
    }

    // set this system as not ready
    //
    seq.daemon_ready.clear( Sequencer::DAEMON_CAMERA );

    seq.seq_state.clear( Sequencer::SEQ_WAIT_CAMERA );
    seq.broadcast_seqstate();

    seq.thread_state.clear( THR_CAMERA_SHUTDOWN );                   // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_camera_shutdown ************************/


  /***** Sequencer::Sequence::dothread_move_to_target *************************/
  /**
   * @brief      send request to TCS to move to target coordinates
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * Disable guiding and send the new target coordinates to the TCS with
   * the instruction to move immediately.
   *
   */
  void Sequence::dothread_move_to_target() {
    this->thread_state.set( THR_MOVE_TO_TARGET );                // thread running
    std::string function = "Sequencer::Sequence::dothread_move_to_target";
    std::stringstream message;
    long error=NO_ERROR;

    // If RA and DEC fields are both empty then no telescope move
    //
    if ( this->target.ra_hms.empty() && this->target.dec_dms.empty() ) {
      logwrite( function, "no telescope move requested" );
      this->thread_state.clear( THR_MOVE_TO_TARGET );            // thread terminated
      this->broadcast_seqstate();
      return;
    }

    {
/*****
    message.str(""); message << "[ACQUIRE] SLIT COOORDS= " << this->target.ra_hms << " "
                                                           << this->target.dec_dms
                                                           << this->target.slitangle;
    logwrite( function, message.str() );

    // If the difference between the TCS coordinates and the target coordinates are within
    // the resolution of reading the TCS then assume we are already pointed. Otherwise,
    // disable guiding and point the telescope here.
    //
//  if ( this->target.name != this->last_target && ( ra_delta > 0.025 || dec_delta > 0.025 ) ) {

      // disable guiding (if running)
      //
      if ( this->seq_state.is_set( Sequencer::SEQ_GUIDE ) ) {

///     if ( ! this->waiting_for_state.load() ) {
///       std::thread( this->dothread_wait_for_state, std::ref(seq) ).detach();
///     }

        this->req_state.clear( Sequencer::SEQ_GUIDE );
        this->broadcast_seqstate();

        message.str(""); message << "current state: " << this->seq_state.get_set_names()
                                 << " waiting for: " << this->req_state.get_set_names();
        logwrite( function, message.str() );

        try {
          this->seq_state.wait_for_match( this->req_state );  // waits for seq_state == req_state
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "NOTICE: move to target aborted: " << e.what();
          this->async.enqueue_and_log( function, message.str() );
          this->seq_state.clear( Sequencer::SEQ_WAIT_TCS, Sequencer::SEQ_WAIT_TCSOP );
          this->broadcast_seqstate();
          this->thread_state.clear( THR_MOVE_TO_TARGET );          // thread terminated
          return;
        }

///     {
///     std::unique_lock<std::mutex> wait_lock(this->seqstate_mtx);
///     if ( this->seqstate.load() != this->reqstate.load() ) {
///       this->seqstate_cv.wait(wait_lock, [&]() { return this->seqstate.load() == this->reqstate.load(); });
///     }
///     }
        logwrite( function, "DONE WAITING" );
      }
///   else logwrite( function, "NOTICE: not guiding" );
///
///   // clear target acquired flag
///   //
///   this->target.acquired = false;
///
*****/
      error = NO_ERROR;

      // Before sending target coordinates to TCS,
      // convert them to decimal and to scope coordinates.
      // (fpoffsets.coords_* are always in degrees)
      //
      double ra_in    = radec_to_decimal( this->target.ra_hms  ) * TO_DEGREES;
      double dec_in   = radec_to_decimal( this->target.dec_dms );
      double angle_in = this->target.slitangle;

      // can't be NaN
      //
      bool ra_isnan  = std::isnan( ra_in  );
      bool dec_isnan = std::isnan( dec_in );

      if ( ra_isnan || dec_isnan ) {
        message.str(""); message << "ERROR: converting";
        if ( ra_isnan  ) { message << " RA=\"" << this->target.ra_hms << "\""; }
        if ( dec_isnan ) { message << " DEC=\"" << this->target.dec_dms << "\""; }
        message << " to decimal";
        this->async.enqueue_and_log( function, message.str() );
        this->thread_error.set( THR_MOVE_TO_TARGET );
        this->seq_state.clear( Sequencer::SEQ_WAIT_TCS, Sequencer::SEQ_WAIT_TCSOP );
        this->broadcast_seqstate();
        this->thread_state.clear( THR_MOVE_TO_TARGET );          // thread terminated
        return;
      }

      // Before sending the target coords to the TCS,
      // convert them from <pointmode> to scope coordinates.
      //
      double ra_out, dec_out, angle_out;
      error = this->target.fpoffsets.compute_offset( this->target.pointmode, "SCOPE",
                                                   ra_in, dec_in, angle_in,
                                                   ra_out, dec_out, angle_out );

      // For now, announce this error but don't stop the show
      //
      double _solved_angle = ( angle_out < 0 ? angle_out + 360.0 : angle_out );

      if ( std::abs(_solved_angle) - std::abs(this->target.casangle) > 0.01 ) {
        message.str(""); message << "NOTICE: Calculated angle " << angle_out
                                 << " is not equivalent to casangle " << this->target.casangle;
        this->async.enqueue_and_log( function, message.str() );
      }

      // Send coordinates using TCS-native COORDS command.
      // TCS wants decimal hours for RA and fpoffsets.coords are always in degrees
      // so convert that as it's being sent here.
      //
      std::stringstream coords_cmd;
      std::string       coords_reply;

      coords_cmd << "COORDS " << ( ra_out * TO_HOURS )  << " "                       // RA in decimal hours
                              <<   dec_out << " "                                    // DEC in decimal degrees
                              <<   "2000" << " "                                     // equinox always = 2000
                              <<   "0 0"  << " "                                     // RA,DEC proper motion not used
                              << "\"" << this->target.name << "\"";                  // target name in quotes

      {
      std::string rastr, decstr;
      double _ra = ra_out * TO_HOURS;
      decimal_to_sexa( _ra, rastr );
      decimal_to_sexa( dec_out, decstr );
      message.str(""); message << "[DEBUG] moving to SCOPE COORDS= " << rastr << "  " << decstr << "  " << angle_out << " J2000";
      logwrite( function, message.str() );
      }

      error  = this->tcsd.send( coords_cmd.str(), coords_reply );                                                // send to the TCS

      if ( error != NO_ERROR || coords_reply.compare( 0, strlen(TCS_SUCCESS_STR), TCS_SUCCESS_STR ) != 0 ) {     // if not success then report error
        message.str(""); message << "ERROR: sending COORDS command. TCS reply: " << coords_reply;
        this->async.enqueue_and_log( function, message.str() );
        this->thread_error.set( THR_MOVE_TO_TARGET );
///     this->seq_state.clear( Sequencer::SEQ_WAIT_TCS, Sequencer::SEQ_WAIT_TCSOP );
///     this->broadcast_seqstate();
///     this->thread_state.clear( THR_MOVE_TO_TARGET );          // thread terminated
///     return;
      }

#ifdef REMOVE_FOR_TESTING
      // Send casangle using tcsd wrapper for RINGGO command
      //
      std::stringstream ringgo_cmd;
      std::string       ringgo_reply;
      ringgo_cmd << TCSD_RINGGO << " " << angle_out;                              // this is calculated cass angle
      this->async.enqueue_and_log( function, "sending "+ringgo_cmd.str()+" to TCS" );
      error = this->tcsd.send( ringgo_cmd.str(), ringgo_reply );
      if ( error != NO_ERROR || ringgo_reply.compare( 0, strlen(TCS_SUCCESS_STR), TCS_SUCCESS_STR ) != 0 ) {     // if not success then report error
        message.str(""); message << "ERROR: sending RINGGO command. TCS reply: " << ringgo_reply;
        this->async.enqueue_and_log( function, message.str() );
        this->thread_error.set( THR_MOVE_TO_TARGET );
///     this->seq_state.clear( Sequencer::SEQ_WAIT_TCS, Sequencer::SEQ_WAIT_TCSOP );
///     this->broadcast_seqstate();
///     this->thread_state.clear( THR_MOVE_TO_TARGET );          // thread terminated
///     return;
      }
#endif

      // Wait for on-target signal (or abort)
      //
      this->async.enqueue_and_log( function, "NOTICE: waiting for TCS operator to send \"ontarget\" signal" );

      this->seq_state.set( Sequencer::SEQ_WAIT_TCSOP );
      this->broadcast_seqstate();

      try {
        this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_TCSOP );
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "NOTICE: move to target aborted: " << e.what();
        this->async.enqueue_and_log( function, message.str() );
        this->seq_state.clear( Sequencer::SEQ_WAIT_TCS, Sequencer::SEQ_WAIT_TCSOP );
        this->broadcast_seqstate();
        this->thread_state.clear( THR_MOVE_TO_TARGET );          // thread terminated
        return;
      }

      this->async.enqueue_and_log( function, "NOTICE: received ontarget signal!" );

      this->last_target = this->target.name;  // remember the last target that was tracked on

    }

    // If not already acquired then start the acquisition sequence in a separate thread
    //
/**** don't acquire right now
    if ( error==NO_ERROR && !this->target.acquired ) {

      logwrite( function, "starting acquisition thread" );             ///< TODO @todo log to telemetry!

      this->seq_state.set( Sequencer::SEQ_WAIT_ACQUIRE );
      this->broadcast_seqstate();
      std::thread( &Sequencer::Sequence::dothread_acquisition, this ).detach();
    }
    else
    if ( error==NO_ERROR && this->target.acquired ) {
      logwrite( function, "target already acquired, nothing to do" );  ///< TODO @todo log to telemetry!
    }

    // Wait for the acquisition sequence
    //
    try {
      this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_ACQUIRE );
    }
    catch ( std::exception & ) {
      message.str(""); message << "NOTICE: acquisition aborted";
      this->async.enqueue_and_log( function, message.str() );
      this->seq_state.clear( Sequencer::SEQ_WAIT_ACQUIRE );
      this->broadcast_seqstate();
      error = ABORT;
    }

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR: unable to acquire target" );
      this->thread_error.set( THR_MOVE_TO_TARGET );
    }
*****/

    // clear all TCS wait bits
    //
    this->seq_state.clear( Sequencer::SEQ_WAIT_TCS,
                           Sequencer::SEQ_WAIT_TCSOP );
    this->broadcast_seqstate();

    this->thread_state.clear( THR_MOVE_TO_TARGET );              // thread terminated
    this->dome_nowait.store( false );
    this->tcs_nowait.store( false );
/// this->is_tcs_ontarget.store( false );
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
    seq.thread_state.set( THR_NOTIFY_TCS );                    // thread running
    std::string function = "Sequencer::Sequence::dothread_notify_tcs";
    std::stringstream message;

    // If this target is the same as the last then this section gets skipped;
    // no need to repoint the telescope and wait for a slew if it's already
    // on target.  Ask where it is pointed:
    //
    double ra_h_now, dec_d_now;
    seq.get_tcs_coords( ra_h_now, dec_d_now );                                          // returns RA in decimal hours, DEC in decimal degrees
    double ra_delta  = std::abs( ra_h_now  - radec_to_decimal( seq.target.ra_hms  ) );  // compare decimal hours
    double dec_delta = std::abs( dec_d_now - radec_to_decimal( seq.target.dec_dms ) );  // compare decimal degrees

    // If the difference between the TCS coordinates and the target coordinates are within
    // the resolution of reading the TCS then assume we are already pointed. Otherwise,
    // disable guiding and point the telescope here.
    //
    if ( ra_delta > 0.01 || dec_delta > 0.01 ) {

      message << "[TODO] (disabled) requesting early pre-auth for next target: " 
              << seq.target.name << " " 
              << seq.target.ra_hms << " " 
              << seq.target.dec_dms;
      logwrite( function, message.str() );

/*******************************************************************************
      // send coordinates to TCS
      //
      long error=NO_ERROR;
      if ( error == NO_ERROR ) {
        std::stringstream coords;
        std::string tcsreply, ra_hms, dec_dms;

        // convert ra, dec to decimal
        // can't be NaN
        //
        if ( std::isnan( radec_to_decimal( seq.target.ra_hms,  ra_hms  ) ) ||
             std::isnan( radec_to_decimal( seq.target.dec_dms, dec_dms ) ) ) {
          seq.async.enqueue_and_log( function, "ERROR: can't handle NaN value for RA, DEC" );
          seq.thread_error.set( THR_NOTIFY_TCS );
          seq.thread_state.clear( THR_NOTIFY_TCS );            // thread terminated
          return;
        }

        coords << "NEXT " << ra_hms << " " << dec_dms << " 0 0 0 \"" << seq.target.name << "\"";
        ///< @todo TCS command "NEXT" not yet implemented
        error  = seq.tcsd.send( coords.str(), tcsreply );
        message.str(""); message << "[TODO] new command not implemented in TCS: " << coords.str();
        logwrite( function, message.str() );
        int tcsvalue;
        if ( error == NO_ERROR ) error = seq.extract_tcs_value( tcsreply, tcsvalue );
        if ( error == NO_ERROR ) error = seq.parse_tcs_generic( tcsvalue );
      }

      // send casangle
      //
      if ( error == NO_ERROR ) logwrite( function, "[TODO] send casangle not yet implemented" ); ///< TODO @todo put send cassangle into its own thread?
*******************************************************************************/
    }
    seq.thread_state.clear( THR_NOTIFY_TCS );                  // thread terminated
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
  void Sequence::dothread_focus_set() {
    this->thread_state.set( THR_FOCUS_SET );                     // thread running
    std::string function = "Sequencer::Sequence::dothread_focus_set";
    long error=NO_ERROR;

    logwrite( function, "[TODO] focus not yet implemented." );

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR: unable to set focus" );
      this->thread_error.set( THR_FOCUS_SET );
    }

    this->seq_state.clear( Sequencer::SEQ_WAIT_FOCUS );
    this->broadcast_seqstate();

    this->thread_state.clear( THR_FOCUS_SET );                   // thread terminated
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
  void Sequence::dothread_flexure_set() {
    this->thread_state.set( THR_FLEXURE_SET );                   // thread running
    std::string function = "Sequencer::Sequence::dothread_flexure_set";
    long error=NO_ERROR;

    logwrite( function, "[TODO] flexure not yet implemented." );

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR: unable to set flexure control" );
      this->thread_error.set( THR_FLEXURE_SET );
    }

    this->seq_state.clear( Sequencer::SEQ_WAIT_FLEXURE );
    this->broadcast_seqstate();

    this->thread_state.clear( THR_FLEXURE_SET );                 // thread terminated
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
  void Sequence::dothread_calib_set() {
    this->thread_state.set( THR_CALIBRATOR_SET );                // thread running
    std::string function = "Sequencer::Sequence::dothread_calib_set";
    long error=NO_ERROR;

    logwrite( function, "[TODO] calibrator not yet implemented." );

    // atomically set thread_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR: unable to set calibrator" );
      this->thread_error.set( THR_CALIBRATOR_SET );
    }

    this->seq_state.clear( Sequencer::SEQ_WAIT_CALIB );
    this->broadcast_seqstate();

    this->thread_state.clear( THR_CALIBRATOR_SET );              // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_calib_set ******************************/


  /***** Sequencer::Sequence::dothread_repeat_exposure ************************/
  /**
   * @brief      repeat the last exposure
   * @param[in]  seq  reference to Sequencer::Sequence object
   * @todo       calibrator not yet implemented
   *
   */
  void Sequence::dothread_repeat_exposure() {
    const std::string function = "Sequencer::Sequence::dothread_repeat_exposure";
    std::stringstream message;

    this->thread_state.set( THR_REPEAT_EXPOSURE );               // thread running

    std::string targetstatus;
    this->target.get_specified_target( this->lastcowboy, targetstatus );

    logwrite( function, targetstatus );

    this->seq_state.set( Sequencer::SEQ_WAIT_CAMERA,
                         Sequencer::SEQ_WAIT_SLIT );
    this->broadcast_seqstate();
    std::thread( &Sequencer::Sequence::dothread_camera_set, this ).detach();
    std::thread( &Sequencer::Sequence::dothread_slit_set, this ).detach();

    // Now that the threads are running, wait until they are all finished.
    // When the SEQ_RUNNING bit is the only bit set then we are ready.
    //
    this->req_state.set( Sequencer::SEQ_RUNNING );                     // set the requested state bit

    logwrite( function, "[DEBUG] waiting for camera and slit" );

    message.str(""); message << "current state: " << this->seq_state.get_set_names()
                             << " waiting for: " << this->req_state.get_set_names();
    logwrite( function, message.str() );

    try {
      this->seq_state.wait_for_match( this->req_state );  // waits for seq_state == req_state
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "NOTICE: sequence aborted: " << e.what();
      this->async.enqueue_and_log( function, message.str() );
      this->thread_state.clear( THR_REPEAT_EXPOSURE );             // thread running
    }

    // Start the exposure in a thread...
    //
    this->seq_state.set( Sequencer::SEQ_WAIT_CAMERA );                   // set the current state
    this->broadcast_seqstate();

    logwrite( function, "[DEBUG] spawning dothread_trigger_exposure" );
    std::thread( &Sequencer::Sequence::dothread_trigger_exposure, this ).detach();  // trigger exposure in a thread

    this->req_state.set( Sequencer::SEQ_RUNNING );                       // set the requested state

    // ...then wait for it to complete
    //
    logwrite( function, "[DEBUG] waiting for camera" );
    message.str(""); message << "current state: " << this->seq_state.get_set_names()
                             << " waiting for: " << this->req_state.get_set_names();
    logwrite( function, message.str() );

    try {
      this->seq_state.wait_for_match( this->req_state );  // waits for seq_state == req_state
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "NOTICE: sequence aborted: " << e.what();
      this->async.enqueue_and_log( function, message.str() );
      this->thread_state.clear( THR_REPEAT_EXPOSURE );             // thread running
    }
    this->thread_state.clear( THR_REPEAT_EXPOSURE );             // thread running
    return;
  }
  /***** Sequencer::Sequence::dothread_repeat_exposure ************************/


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
  void Sequence::dothread_trigger_exposure() {
    this->thread_state.set( THR_TRIGGER_EXPOSURE );              // thread running
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
    if ( this->tcs_preauth_time > 0 ) this->notify_tcs_next_target = true; else this->notify_tcs_next_target = false;

    this->arm_readout_flag = true;                  // enables the async_listener to look for the readout and clear the EXPOSE bit

    logwrite( function, "[DEBUG] sending expose command" );
    error = this->camerad.async( CAMERAD_EXPOSE );  // Send the EXPOSE command to camera daemon on the non-blocking port and don't wait for reply

    if ( error == NO_ERROR ) {
      error = this->target.update_state( Sequencer::TARGET_EXPOSING );   // set EXPOSE state in database
      this->seq_state.set( Sequencer::SEQ_WAIT_EXPOSE );                 // set EXPOSE bit
    }
    else {
      this->async.enqueue_and_log( function, "ERROR sending command: CAMERAD_EXPOSE" );
      this->thread_error.set( THR_TRIGGER_EXPOSURE );                    // tell the world this thread had an error
      this->target.update_state( Sequencer::TARGET_PENDING );            // return the target state to pending
      this->seq_state.clear( Sequencer::SEQ_WAIT_EXPOSE );               // clear EXPOSE bit
      this->broadcast_seqstate();
      this->arm_readout_flag = false;                                    // disarm async_listener from looking for readout
    }

    // let the world know of the state change
    //
    message.str(""); message << "TARGETSTATE:" << this->target.state << " TARGET:" << this->target.name << " OBSID:" << this->target.obsid;
    this->async.enqueue( message.str() );

    this->seq_state.clear( Sequencer::SEQ_WAIT_CAMERA );
    this->broadcast_seqstate();

    this->thread_state.clear( THR_TRIGGER_EXPOSURE );            // thread terminated
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
    seq.thread_state.set( THR_MODIFY_EXPTIME );                // thread running
    std::string function = "Sequencer::Sequence::dothread_modify_exptime";
    std::stringstream message;
    std::string reply="";
    long error = NO_ERROR;
    double updated_exptime=0;

    // This function is only used while exposing
    //
    if ( ! seq.seq_state.is_set( Sequencer::SEQ_WAIT_EXPOSE ) ) {
      seq.async.enqueue_and_log( function, "ERROR cannot update exposure time when not currently exposing" );
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
      seq.target.exptime_req = updated_exptime;
      message.str(""); message << "successfully updated exptime to " << updated_exptime << " sec";
      logwrite( function, message.str() );
    }

    // announce the success or failure in an asynchronous broadcast message
    //
    message.str(""); message << "MODIFY_EXPTIME: " << seq.target.exptime_req << ( error==NO_ERROR ? " DONE" : " ERROR" );
    seq.async.enqueue( message.str() );

    seq.thread_state.clear( THR_MODIFY_EXPTIME );              // thread terminated
    return;
  }
  /***** Sequencer::Sequence::dothread_modify_exptime *************************/


  /***** Sequencer::Sequence::dothread_acquisition ****************************/
  /**
   * @brief      performs the acqusition sequence
   * @details    this gets called by the move_to_target thread
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * This function is spawned in a thread.
   *
   */
  void Sequence::dothread_acquisition() {
// testing
this->seq_state.clear( Sequencer::SEQ_WAIT_ACQUIRE );              // clear ACQUIRE bit
this->broadcast_seqstate();
return;
    this->thread_state.set( THR_ACQUISITION );                   // thread running
    std::string function = "Sequencer::Sequence::dothread_acquisition";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    long error = NO_ERROR;

    // Before sending target coordinates to ACAM,
    // convert them to decimal and to ACAM coordinates.
    // (fpoffsets.coords_* are always in degrees)
    //
    double ra_in    = radec_to_decimal( this->target.ra_hms  ) * TO_DEGREES;
    double dec_in   = radec_to_decimal( this->target.dec_dms );
    double angle_in = this->target.slitangle;

    // can't be NaN
    //
    bool ra_isnan  = std::isnan( ra_in  );
    bool dec_isnan = std::isnan( dec_in );

    if ( ra_isnan || dec_isnan ) {
      message.str(""); message << "ERROR: converting";
      if ( ra_isnan  ) { message << " RA=\"" << this->target.ra_hms << "\""; }
      if ( dec_isnan ) { message << " DEC=\"" << this->target.dec_dms << "\""; }
      message << " to decimal";
      this->async.enqueue_and_log( function, message.str() );
      this->thread_error.set( THR_MOVE_TO_TARGET );
      this->seq_state.clear( Sequencer::SEQ_WAIT_TCS, Sequencer::SEQ_WAIT_TCSOP );
      this->broadcast_seqstate();
      this->thread_state.clear( THR_MOVE_TO_TARGET );          // thread terminated
      return;
    }

//  // Before sending the target coords to the ACAM,
//  // convert them from <pointmode> to ACAM coordinates.
//  //
//  double ra_out, dec_out, angle_out;
//  error = this->target.fpoffsets.compute_offset( this->target.pointmode, "ACAM",
//                                               ra_in, dec_in, angle_in,
//                                               ra_out, dec_out, angle_out );
//
//  // Send the ACQUIRE command to acamd, which requires
//  // the target coordinates (from the database).
//  //
//  message.str(""); message << "starting target acquisition " << ra_out    << " "
//                                                             << dec_out   << " "
//                                                             << angle_out << " "
//                                                             << this->target.name;
    message.str(""); message << "starting target acquisition " << ra_in    << " "
                                                               << dec_in   << " "
                                                               << angle_in << " "
                                                               << this->target.name;
    logwrite( function, message.str() );
    cmd.str(""); cmd << ACAMD_ACQUIRE << " " << ra_in << " "
                                             << dec_in << " "
                                             << angle_in << " ";

    error = this->acamd.command( cmd.str(), reply );

/***** DONT CARE ABOUT ERRORS NOW -- NO CONDITION ON ACQ SUCCESS 
    if ( error != NO_ERROR ) {
      this->thread_error.set( THR_ACQUISITION );                       // report error
      message.str(""); message << "ERROR acquiring target";
      this->async.enqueue_and_log( function, message.str() );
      this->seq_state.clear( Sequencer::SEQ_WAIT_ACQUIRE );            // clear ACQUIRE bit
      this->broadcast_seqstate();
      this->thread_state.clear( THR_ACQUISITION );                     // thread terminated
      return;
    }

    // The reply contains the timeout.
    // Acam's acquisition sequence uses that timeout but the Sequencer
    // will also use it here, so that it knows when to stop asking acamd
    // for its acquisition status.
    //
    double timeout;
    try {
      timeout = std::stod( reply );
    } catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing timeout \"" << reply << "\" from acam: " << e.what();
      logwrite( function, message.str() );
      this->thread_error.set( THR_ACQUISITION );                       // report any error
      return;
    }

    auto timeout_time = std::chrono::steady_clock::now()
                        + std::chrono::duration<double>(timeout);

    reply.clear();

    // Poll acamd while it is acquiring. Once finished, get the state.
    //
    bool acquiring = true;
    do {
      std::this_thread::sleep_for( std::chrono::milliseconds(100) );
      if (error==NO_ERROR) error = this->acamd.command( ACAMD_ACQUIRE, reply );
      acquiring = ( reply.find("acquiring") != std::string::npos );
    } while ( error==NO_ERROR &&
              acquiring       &&
              std::chrono::steady_clock::now() < timeout_time );

    // Acquisition loop complete so get the state
    //
    error = this->acamd.command( ACAMD_ISACQUIRED, reply );
    this->target.acquired = ( reply.find("true") != std::string::npos );

    // set message
    //
    if ( std::chrono::steady_clock::now() >= timeout_time ) {        // Timeout
      this->thread_error.set( THR_ACQUISITION );
      message.str(""); message << "ERROR failed to acquire within timeout";
    }
    else
    if ( error!=NO_ERROR ) {                                         // Error polling
      this->thread_error.set( THR_ACQUISITION );
      message.str(""); message << "ERROR acquiring target";
    }
    else {                                                           // Success
      message.str(""); message << "NOTICE: target " << ( this->target.acquired ? "acquired" : "not acquired" );
    }

    this->async.enqueue_and_log( function, message.str() );            // log message
*****/

    this->seq_state.clear( Sequencer::SEQ_WAIT_ACQUIRE );              // clear ACQUIRE bit
    this->broadcast_seqstate();

    this->thread_state.clear( THR_ACQUISITION );                       // thread terminated
  }
  /***** Sequencer::Sequence::dothread_acquisition ****************************/


  /***** Sequencer::Sequence::dothread_wait_for_state *************************/
  /**
   * @brief      waits for sequence state to match the requested state
   * @param[in]  seq  reference to Sequencer::Sequence object
   *
   * This function is spawned in a thread. It waits forever until the this->seqstate
   * matches the requested state, at which point it sends a cv.notify_all signal
   * to unblock all threads currently waiting on cv. Note that the requested state
   * can be changed at any time.
   *
   * This whole function might be obsolete now with the new StateManager class
   *
   */
  void Sequence::dothread_wait_for_state( Sequencer::Sequence &seq ) {
    seq.thread_state.set( THR_WAIT_FOR_STATE );           // thread running
    std::string function = "Sequencer::Sequence::dothread_wait_for_state";
    std::stringstream message;

    seq.waiting_for_state.store( true );            // let other threads know that I'm already waiting for a state

    message.str(""); message << "sequencer state: " << seq.seq_state.get_set_names()
                             << ". waiting for state: " << seq.req_state.get_set_names();
    logwrite( function, message.str() );

    // wait forever or until seqstate is the requested state.
    // Note that other threads can (atomically) change reqstate at any time.
    //
    seq.seq_state.wait_for_match( seq.req_state );
/***
    while ( true ) {
      uint32_t new_reqstate = seq.req_state.get();         // reqstate is checked only once per loop here
      if ( last_reqstate != new_reqstate ) {               // log if reqstate has changed
        message.str(""); message << "requested state changed: " << seq.req_state.get_set_names();
        logwrite( function, message.str() );
        last_reqstate = new_reqstate;
      }
      if ( seq.seqstate.load() == last_reqstate ) break;   // condition met: seqstate is reqstate
    }
***/

    if ( seq.thread_error.is_any_set() ) {
      message.str(""); message << "ERROR the following thread(s) had an error: "
                               << seq.thread_error.get_set_names();
      logwrite( function, message.str() );
    }

    // done waiting so send notification
    //
    message.str(""); message << "requested state " << seq.seq_state.get_set_names()
                             << " reached: notifying threads";
    logwrite( function, message.str() );
    seq.waiting_for_state.store( false );           // let other threads know that I'm done waiting
    seq.cv.notify_all();

    seq.thread_state.clear( THR_WAIT_FOR_STATE );         // thread terminated
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
    seq.thread_state.set( THR_STARTUP );                       // thread running
    seq.startup( seq );
    seq.thread_state.clear( THR_STARTUP );                     // thread terminated
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
//  if ( not seq.is_seqstate_set( Sequencer::SEQ_OFFLINE ) && not seq.is_seqstate_set( Sequencer::SEQ_READY ) ) {
//    message << "ERROR: runstate " << this->seq_state.get_set_names() << " must be OFFLINE or READY";
//  if ( seq.seq_state.is_clear( Sequencer::SEQ_OFFLINE ) ) {
//    message << "ERROR: runstate " << this->seq_state.get_set_names() << " must be OFFLINE";
//    seq.async.enqueue_and_log( function, message.str() );
//    return ERROR;
//  }

/*****
    // Disallow startup if TCS is not connected and open
    //
    if ( !seq.tcsd.socket.isconnected() ) {
      logwrite( function, "ERROR cannot start: not connected to tcs daemon" );
      seq.async.enqueue( "TCSD:isopen:false" );            // added here for consistency
      return ERROR;
    }
    else {
      // Ask tcsd if hardware connection is open.
      // The response will be "name|false|ERROR" where name is the name of the TCS device
      // if connected to that device, so to be open, require that there is no error and
      // that the reply is neither false nor error.
      //
      std::string reply;
      if ( seq.tcsd.send( TCSD_ISOPEN, reply ) != NO_ERROR ||
           reply.find( "false" ) != std::string::npos      ||
           reply.find( "ERROR" ) != std::string::npos ) {
        logwrite( function, "ERROR cannot start: tcs not initialized" );
        seq.async.enqueue( "TCSD:isopen:false" );            // added here for consistency
        return ERROR;
      }
    }
*****/

    // clear the thread error state
    //
    seq.thread_error.clear_all();

    // Everything (except TCS) needs the power control to be running 
    // so initialize the power control first.
    // For this, set READY and WAIT_POWER bits, and clear OFFLINE bit.
    //
    seq.seq_state.set_and_clear( {Sequencer::SEQ_STARTING, Sequencer::SEQ_WAIT_POWER}, {Sequencer::SEQ_OFFLINE} );
    seq.req_state.set_and_clear( {Sequencer::SEQ_STARTING}, {Sequencer::SEQ_OFFLINE} );
    seq.broadcast_seqstate();

    std::thread( dothread_power_init, std::ref(seq) ).detach();                   // start power initialization thread

/// std::thread( dothread_wait_for_state, std::ref(seq) ).detach();               // wait for requested state

    logwrite( function, "[DEBUG] (3) waiting for power control to initialize" );
    message.str(""); message << "current state: " << seq.seq_state.get_set_names()
                             << " waiting for: " << seq.req_state.get_set_names();
    logwrite( function, message.str() );

    // wait for WAIT_POWER to be cleared
    //
    try {
      seq.seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_POWER );
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "NOTICE: startup aborted: " << e.what();
      seq.async.enqueue_and_log( function, message.str() );
      this->ready_to_start = false;
      seq.seq_state.set_and_clear( Sequencer::SEQ_OFFLINE, Sequencer::SEQ_STARTING );
      seq.req_state.set_and_clear( Sequencer::SEQ_OFFLINE, Sequencer::SEQ_STARTING );
      seq.broadcast_seqstate();
      return NO_ERROR;
    }

/// {
/// std::unique_lock<std::mutex> wait_lock(seq.seqstate_mtx);
/// if ( seq.seqstate.load() != seq.reqstate.load() ) {
///   seq.seqstate_cv.wait(wait_lock, [&]() { return seq.seqstate.load() == seq.reqstate.load(); });
/// }
/// }
    logwrite( function, "[DEBUG] (3) DONE waiting for power control to initialize" );

    // Don't proceed unless power control initialized successfully
    //
    if ( seq.thread_error.is_any_set() ) {
      seq.async.enqueue_and_log( function, "ERROR: initializing power control. startup aborted" );
      this->ready_to_start = false;
      seq.seq_state.set_and_clear( Sequencer::SEQ_OFFLINE, Sequencer::SEQ_STARTING );
      seq.req_state.set_and_clear( Sequencer::SEQ_OFFLINE, Sequencer::SEQ_STARTING );
      seq.broadcast_seqstate();
      return ERROR;
    }
    else {
      logwrite( function, "power control initialized" );
    }

    // The following can be done in parallel.
    // Set the state bit before starting each thread, then
    // the thread will clear their bit when they complete.
    //
    seq.seq_state.set( Sequencer::SEQ_WAIT_TCS,
                       Sequencer::SEQ_WAIT_ACAM,
                       Sequencer::SEQ_WAIT_CALIB,
                       Sequencer::SEQ_WAIT_CAMERA,
                       Sequencer::SEQ_WAIT_FLEXURE,
                       Sequencer::SEQ_WAIT_FOCUS,
                       Sequencer::SEQ_WAIT_SLICECAM,
                       Sequencer::SEQ_WAIT_SLIT );
    seq.broadcast_seqstate();
    std::thread( dothread_tcs_init, std::ref(*this), seq.tcs_which ).detach();      // spawn the tcs_init thread here
    std::thread( dothread_andor_init, std::ref(seq) ).detach();  // this will clear both ACAM and SLICECAM bits
    std::thread( dothread_calib_init, std::ref(seq) ).detach();
    std::thread( dothread_camera_init, std::ref(seq) ).detach();
    std::thread( dothread_flexure_init, std::ref(seq) ).detach();
    std::thread( dothread_focus_init, std::ref(seq) ).detach();
    std::thread( dothread_slit_init, std::ref(seq) ).detach();
//  seq.set_seqstate_bit( Sequencer::SEQ_WAIT_TCS );     std::thread( dothread_tcs_init, std::ref(seq), "" ).detach();

    // Now that the threads are running, wait until they are all finished.
    // When the SEQ_STARTING bit is the only bit set then we are ready.
    //
/// std::thread( dothread_wait_for_state, std::ref(seq) ).detach();  // wait for requested state

    logwrite( function, "[DEBUG] (4) waiting for init threads to complete" );
    message.str(""); message << "current state: " << seq.seq_state.get_set_names()
                             << " waiting for: " << seq.req_state.get_set_names();
    logwrite( function, message.str() );

    try {
      seq.seq_state.wait_for_match( seq.req_state );
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "NOTICE: startup aborted: " << e.what();
      seq.async.enqueue_and_log( function, message.str() );
      seq.seq_state.set_and_clear( {Sequencer::SEQ_OFFLINE}, {Sequencer::SEQ_STARTING,Sequencer::SEQ_READY} );
      seq.req_state.set_and_clear( {Sequencer::SEQ_OFFLINE}, {Sequencer::SEQ_STARTING,Sequencer::SEQ_READY} );
      seq.broadcast_seqstate();
      this->ready_to_start = false;
      seq.thread_error.clear_all();  // clear the thread error state
      return ERROR;
    }

/// {
/// std::unique_lock<std::mutex> wait_lock(seq.seqstate_mtx);
/// if ( seq.seqstate.load() != seq.reqstate.load() ) {
///   seq.seqstate_cv.wait(wait_lock, [&]() { return seq.seqstate.load() == seq.reqstate.load(); });
/// }
/// }
    logwrite( function, "[DEBUG] (4) DONE waiting for init threads to complete" );

    // all done now so clear the STARTING bits
    //
    logwrite( function, "init threads completed" );

    // if any thread returned an error then we are not ready
    //
    if ( seq.thread_error.is_any_set() ) {
      message.str(""); message << "ERROR startup failed because the following thread(s) had an error: "
                               << seq.thread_error.get_set_names();
      seq.async.enqueue_and_log( function, message.str() );
      seq.seq_state.set_and_clear( {Sequencer::SEQ_OFFLINE}, {Sequencer::SEQ_STARTING,Sequencer::SEQ_READY} );
      seq.req_state.set_and_clear( {Sequencer::SEQ_OFFLINE}, {Sequencer::SEQ_STARTING,Sequencer::SEQ_READY} );
      seq.broadcast_seqstate();
      this->ready_to_start = false;
      seq.thread_error.clear_all();  // clear the thread error state
      return ERROR;
    }
    else {
      seq.seq_state.set_and_clear( Sequencer::SEQ_READY, Sequencer::SEQ_STARTING );
      seq.req_state.set_and_clear( Sequencer::SEQ_READY, Sequencer::SEQ_STARTING );
      seq.broadcast_seqstate();
      logwrite( function, "ready to start" );
      this->ready_to_start = true;
      return NO_ERROR;
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
    seq.thread_state.set( THR_SHUTDOWN );                      // thread running
    seq.shutdown( seq );
    seq.thread_state.clear( THR_SHUTDOWN );                    // thread terminated
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

/*** 6/14/23 allow shutdown any time
 *
    // shutdown sequence can only be run while ready
    //
    if ( not seq.is_seqstate_set( Sequencer::SEQ_READY ) ) {
      message << "ERROR: runstate " << this->seq_state.get_set_names() << " must be READY";
      seq.async.enqueue_and_log( function, message.str() );
      return( ERROR );
    }

    // probably redundant
    //
    if ( not this->is_ready() ) {
      seq.async.enqueue_and_log( function, "ERROR: sequencer is already shut down" );
      return( ERROR );
    }
***/

/// std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );                       // create a mutex object for waiting

    // clear the thread error state
    //
    seq.thread_error.clear_all();

    // Everything (except TCS) needs the power control to be running.
    // It should already be running but just make sure that power control
    // is running and initialize it if not.
    // For this, set SHUTTING and WAIT_POWER bits, and clear READY bit.
    //
    seq.seq_state.set_and_clear( {Sequencer::SEQ_SHUTTING, Sequencer::SEQ_WAIT_POWER}, {Sequencer::SEQ_READY, Sequencer::SEQ_OFFLINE} );
    seq.req_state.set_and_clear( {Sequencer::SEQ_SHUTTING}, {Sequencer::SEQ_READY,Sequencer::SEQ_OFFLINE} );
    seq.broadcast_seqstate();

    std::thread( dothread_power_init, std::ref(seq) ).detach();                   // start power initialization thread

/// std::thread( dothread_wait_for_state, std::ref(seq) ).detach();               // wait for requested state

    logwrite( function, "[DEBUG] waiting for power control to initialize" );

    message.str(""); message << "current state: " << seq.seq_state.get_set_names()
                             << " waiting for: " << seq.req_state.get_set_names();
    logwrite( function, message.str() );

    try {
      seq.seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_POWER );
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "NOTICE: shutdown aborted wait for power init: " << e.what();
      seq.async.enqueue_and_log( function, message.str() );
      seq.seq_state.clear( Sequencer::SEQ_WAIT_POWER );
      seq.req_state.clear( Sequencer::SEQ_WAIT_POWER );
      seq.broadcast_seqstate();
      return NO_ERROR;
    }

/// {
/// std::unique_lock<std::mutex> wait_lock(seq.seqstate_mtx);
/// if ( seq.seqstate.load() != seq.reqstate.load() ) {
///   seq.seqstate_cv.wait(wait_lock, [&]() { return seq.seqstate.load() == seq.reqstate.load(); });
/// }
/// }
    logwrite( function, "[DEBUG] DONE waiting for power control to initialize" );

    // Try to proceed even if power control didn't initialize successfully
    //
    if ( seq.thread_error.is_any_set() ) {
      seq.async.enqueue_and_log( function, "ERROR: initializing power control" );
      seq.seq_state.clear( Sequencer::SEQ_WAIT_POWER );
      seq.req_state.clear( Sequencer::SEQ_WAIT_POWER );
      seq.broadcast_seqstate();
    }
    else {
      logwrite( function, "power control initialized" );
    }

    // The following can be done in parallel.
    // Set the state bit before starting each thread, then
    // the thread will clear their bit when they complete.
    //
    seq.seq_state.set( Sequencer::SEQ_WAIT_ACAM,
                       Sequencer::SEQ_WAIT_CALIB,
                       Sequencer::SEQ_WAIT_CAMERA,
                       Sequencer::SEQ_WAIT_FLEXURE,
                       Sequencer::SEQ_WAIT_FOCUS,
                       Sequencer::SEQ_WAIT_SLICECAM,
                       Sequencer::SEQ_WAIT_SLIT );
    seq.broadcast_seqstate();
    std::thread( dothread_acam_shutdown, std::ref(seq) ).detach();
    std::thread( dothread_calib_shutdown, std::ref(seq) ).detach();
    std::thread( dothread_camera_shutdown, std::ref(seq) ).detach();
    std::thread( dothread_flexure_shutdown, std::ref(seq) ).detach();
    std::thread( dothread_focus_shutdown, std::ref(seq) ).detach();
    std::thread( dothread_slicecam_shutdown, std::ref(seq) ).detach();
    std::thread( dothread_slit_shutdown, std::ref(seq) ).detach();

    // Now that the shutdown-threads are running, wait until they are all finished.
    // When the SEQ_SHUTTING bit is the only bit set then we are ready to proceed.
    //
/// std::thread( dothread_wait_for_state, std::ref(seq) ).detach();  // wait for requested state

    logwrite( function, "[DEBUG] (5) waiting for shutdown threads to complete" );

    try {
      seq.seq_state.wait_for_match( seq.req_state );
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "NOTICE: shutdown aborted: " << e.what();
      seq.async.enqueue_and_log( function, message.str() );
      seq.seq_state.clear( Sequencer::SEQ_SHUTTING,
                           Sequencer::SEQ_WAIT_ACAM,
                           Sequencer::SEQ_WAIT_CALIB,
                           Sequencer::SEQ_WAIT_CAMERA,
                           Sequencer::SEQ_WAIT_FLEXURE,
                           Sequencer::SEQ_WAIT_FOCUS,
                           Sequencer::SEQ_WAIT_SLICECAM,
                           Sequencer::SEQ_WAIT_SLIT );
      seq.broadcast_seqstate();
      seq.thread_error.clear_all();  // clear the thread error state
      return ERROR;
    }

/// while ( seq.seqstate.load() != seq.reqstate.load() ) {
///   message.str(""); message << "wait for state " << seq.req_state.get_set_names();
///   logwrite( function, message.str() );
///   seq.cv.wait( wait_lock );
/// }


/*****
   why shut down powerd?

    logwrite( function, "[DEBUG] (5) DONE waiting for shutdown threads to complete" );

    // Everything has undergone their shutdown procedure so safe to shut down the power daemon now
    //
    seq.seq_state.set( Sequencer::SEQ_WAIT_POWER );
    seq.broadcast_seqstate();
    std::thread( dothread_power_shutdown, std::ref(seq) ).detach();

    // Wait for the power shutdown thread.
    // When the SEQ_SHUTTING bit is the only bit set then we are ready.
    //
/// std::thread( dothread_wait_for_state, std::ref(seq) ).detach();  // wait for requested state

    logwrite( function, "[DEBUG] (6) waiting for power daemon to shut down" );

    try {
      seq.seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_POWER );
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "NOTICE: power shutdown aborted: " << e.what();
      seq.async.enqueue_and_log( function, message.str() );
      seq.seq_state.clear( Sequencer::SEQ_SHUTTING,
                           Sequencer::SEQ_WAIT_POWER );
      seq.broadcast_seqstate();
      seq.thread_error.clear_all();  // clear the thread error state
      return ERROR;
    }

/// while ( seq.seqstate.load() != seq.reqstate.load() ) {
///   message.str(""); message << "wait for state " << seq.req_state.get_set_names();
///   logwrite( function, message.str() );
///   seq.cv.wait( wait_lock );
/// }
    logwrite( function, "[DEBUG] (6) DONE waiting for power daemon to shut down" );

*****/

    // Note the error(s) but shutdown will always close the daemon connections
    // so return success.
    //
    if ( seq.thread_error.is_any_set() ) {
      message.str(""); message << "NOTICE: the following thread(s) had an error during shutdown: "
                               << seq.thread_error.get_set_names();
      seq.async.enqueue_and_log( function, message.str() );
      seq.thread_error.clear_all();     // clear the thread error state
    }

    // we are now offline
    //
    seq.seq_state.set_and_clear( Sequencer::SEQ_OFFLINE, Sequencer::SEQ_SHUTTING );  // set and clear seqstate
    seq.req_state.set_and_clear( Sequencer::SEQ_OFFLINE, Sequencer::SEQ_SHUTTING );  // set the requested state
    seq.broadcast_seqstate();

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


  /***** Sequencer::Sequence::get_dome_position *******************************/
  /**
   * @brief      read the dome and telescope positions
   * @param[out] domeazi
   * @param[out] telazi
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::poll_dome_position( double &domeazi, double &telazi ) {
    return this->get_dome_position( true, domeazi, telazi );
  }
  long Sequence::get_dome_position( double &domeazi, double &telazi ) {
    return this->get_dome_position( false, domeazi, telazi );
  }
  long Sequence::get_dome_position( bool poll, double &domeazi, double &telazi ) {
    std::string function = "Sequencer::Sequence::get_dome_position";
    std::stringstream message;

    std::string tcsreply;
    std::stringstream tcscmd;
    tcscmd << ( poll ? "poll " : "" ) << TCSD_GET_DOME;  // optional "poll" prevents excessive logging by tcsd
    if ( this->tcsd.send( tcscmd.str(), tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR getting dome position from tcsd" );
      return( ERROR );
    }

    std::vector<std::string> tcstokens;
    Tokenize( tcsreply, tcstokens, " " );

    // If there's one (or fewer) tokens then it's an error
    //
    if ( tcstokens.size() <= 1 || tcsreply == "ERROR" ) {
      logwrite( function, "ERROR getting dome position from tcsd" );
      return( ERROR );
    }

    // On success GET_DOME returns two numbers, the dome azimuth and the telescope azimuth, followed by DONE
    //
    if ( tcstokens.size() != 3 ) {
      message.str(""); message << "ERROR malformed reply \"" << tcsreply << "\" getting dome position. expected <domeaz> <telaz>";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      domeazi = std::stod( tcstokens.at(0) );
      telazi  = std::stod( tcstokens.at(1) );
    }
    catch( std::out_of_range & ) {
      logwrite( function, "ERROR out of range parsing dome position" );
      return( ERROR );
    }
    catch( std::invalid_argument &e ) {
      logwrite( function, "ERROR invalid argument parsing dome position" );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** Sequencer::Sequence::get_dome_position *******************************/


  /***** Sequencer::Sequence::get_tcs_motion **********************************/
  /**
   * @brief      read the tcs motion state
   * @param[out] state
   * @param[out] telazi
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::poll_tcs_motion( std::string &state_out ) {
    return this->get_tcs_motion( true, state_out );
  }
  long Sequence::get_tcs_motion( std::string &state_out ) {
    return this->get_tcs_motion( false, state_out );
  }
  long Sequence::get_tcs_motion( bool poll, std::string &state_out ) {
    std::string function = "Sequencer::Sequence::get_tcs_motion";
    std::stringstream message;

    std::string tcsreply;
    std::stringstream tcscmd;
    tcscmd << ( poll ? "poll " : "" ) << TCSD_GET_MOTION;  // optional "poll" prevents excessive logging by tcsd
    if ( this->tcsd.send( tcscmd.str(), tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR getting motion state from tcsd" );
      return( ERROR );
    }

    std::vector<std::string> tcstokens;
    Tokenize( tcsreply, tcstokens, " " );

    try {
      state_out = tcstokens.at(0);
    }
    catch( std::out_of_range & ) {
      logwrite( function, "ERROR out of range parsing motion state" );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** Sequencer::Sequence::get_tcs_motion **********************************/


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

    std::string coordstring;

    if ( this->tcsd.send( cmd, coordstring ) != NO_ERROR ) {
      logwrite( function, "ERROR reading TCS coordinates" );
      return ERROR;
    }

    std::vector<std::string> tokens;

    Tokenize( coordstring, tokens, " " );                       // comes back as space-delimited string "hh:mm:ss.ss dd:mm:ss.ss"
    try {                                                       // extract ra dec from coordstring
      if ( cmd.compare( TCSD_GET_COORDS ) == 0 ) {
        ra_h  = radec_to_decimal( tokens.at(0) );               // RA decimal hours
        dec_d = radec_to_decimal( tokens.at(1) );               // DEC decimal degrees
      }
      else
      if ( cmd.compare( TCSD_WEATHER_COORDS ) == 0 ) {
        ra_h  = std::stod( tokens.at(0) );                      // RA decimal hours
        dec_d = std::stod( tokens.at(1) );                      // DEC decimal degrees
      }
      else {
        message.str(""); message << "ERROR unrecognized command \"" << cmd << "\"";
        logwrite( function, message.str() );
        return ERROR;
      }
    }
    catch( std::out_of_range &e ) {
      message << "EXCEPTION: out of range exception parsing \"" << coordstring << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      message << "EXCEPTION: invalid argument exception parsing \"" << coordstring << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::get_tcs_coords **********************************/


  /***** Sequencer::Sequence::get_tcs_cass ************************************/
  /**
   * @brief      read the current TCS cass angle
   * @param[out] cass  cass angle in degrees
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::get_tcs_cass( double &cass ) {
    std::string function = "Sequencer::Sequencer::get_tcs_cass";
    std::stringstream message;
    std::string tcsreply;

    if ( this->tcsd.send( TCSD_GET_CASS, tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR reading TCS cass angle" );
      cass = NAN;
      return ERROR;
    }

    // tcsreply will be " ddd.dd DONE" or "ERROR" so tokenize on space
    // to get the angle
    //
    std::vector<std::string> tokens;
    Tokenize( tcsreply, tokens, " " );              // comes back as space-delimited string "hh:mm:ss.ss dd:mm:ss.ss"

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR parsing angle from TCS reply \"" << tcsreply << "\"";
      logwrite( function, message.str() );
      return ERROR;
    }

    try {                                              // extract ra dec from coordstring
      cass = std::stod( tokens.at(0) );
    }
    catch( std::out_of_range &e ) {
      message << "EXCEPTION: out of range exception parsing \"" << tcsreply << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      message << "EXCEPTION: invalid argument exception parsing \"" << tcsreply << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    message.str(""); message << "currrent cass ring angle = " << cass;
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::get_tcs_cass ************************************/


  /***** Sequencer::Sequence::tcs_init ****************************************/
  /**
   * @brief      initialize the specified tcs device
   * @param[in]  which      optional string indicates what to do {real|sim|shutdown}
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR | HELP
   *
   * @details    This function spawns the dothread_tcs_init thread with a
   *             parameter to specify which TCS device to use {real|sim}, in
   *             order to initialize the named TCS.
   *
   *             Error checking the contents of that parameter are left to tcsd.
   *
   *             Supplying the parameter "shutdown" will invoke the tcs_shutdown thread.
   *
   */
  long Sequence::tcs_init( const std::string which, std::string &retstring ) {
    std::string function = "Sequencer::Sequence::tcs_init";
    std::stringstream message;

    if ( which=="?" || which=="help" ) {
      retstring = SEQUENCERD_TCSINIT;
      retstring.append( " [ real | sim | shutdown ]\n" );
      retstring.append( "   Initialize the specified TCS, which takes place in a separate thread.\n" );
      retstring.append( "   real      Connect to and initialize the real P200 TCS.\n" );
      retstring.append( "   sim       Connect to and initialize the TCS emulator.\n" );
      retstring.append( "   shutdown  Begin the TCS shutdown sequence.\n" );
      retstring.append( "   If no argument is supplied then the name of the currently connected TCS\n" );
      retstring.append( "   is returned, if connected.\n" );
      return HELP;
    }

    bool is_initing  = this->thread_state.is_set( THR_TCS_INIT );               // tcs_init thread already running
    bool is_shutting = this->thread_state.is_set( THR_TCS_SHUTDOWN );           // tcs_shutdown thread already running

message.str(""); message << "[DEBUG]"
                         << " THR_TCS_INIT=" << THR_TCS_INIT
                         << " is_initing=" << is_initing
                         << " THR_TCS_SHUTDOWN=" << THR_TCS_SHUTDOWN
                         << " is_shutting=" << is_shutting;
logwrite( function, message.str() );

    if ( which.empty() ) {
      if ( ! this->tcsd.socket.isconnected() ) {
        logwrite( function, "not connected to tcs daemon" );
        retstring="not_connected";
        return NO_ERROR;
      }
      return( this->tcsd.send( TCSD_GET_NAME, retstring ) );
    }

    if ( is_initing || is_shutting ) {                                          // only allow one init|shutdown thread at a time
      message.str("");
      message << "ERROR:TCS " << ( is_initing ? "initialization" : "shutdown" ) << " is already in progress";
      logwrite( function, message.str() );
      retstring = ( is_initing ? "init" : "shutdown" );
      retstring.append( "_in_progress" );
      return ERROR;
    }
    else {
      if ( which.compare( 0, strlen( "shutdown" ), "shutdown" ) == 0 ) {
        std::thread( dothread_tcs_shutdown, std::ref(*this) ).detach();         // spawn the shutdown thread here
      }
      else {
        if ( !which.empty() ) {
          this->seq_state.set( Sequencer::SEQ_WAIT_TCS );                       // only set the WAIT bit if a device was specified
          logwrite( function, "TCS initialization started" );
        }
        std::thread( dothread_tcs_init, std::ref(*this), which ).detach();      // spawn the tcs_init thread here
      }
      return NO_ERROR;
    }
  }
  /***** Sequencer::Sequence::tcs_init ****************************************/


  /***** Sequencer::Sequence::target_offset ***********************************/
  /**
   * @brief      performs target offset
   * @param[in]  seq  reference to Sequence object
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::target_offset() {
    std::string function = "Sequencer::Sequence::target_offset";
    long error=NO_ERROR;

    error  = this->tcsd.command( TCSD_ZERO_OFFSETS );

    std::stringstream cmd;
    cmd << TCSD_PTOFFSET << " " << this->target.offset_ra << " " << this->target.offset_dec;

    error |= this->tcsd.command( cmd.str() );

    logwrite( function, "sent "+cmd.str() );

    return error;
  }
  /***** Sequencer::Sequence::target_offset ***********************************/


  /***** Sequencer::Sequence::make_telemetry_message **************************/
  /**
   * @brief      assembles a telemetry message
   * @details    This creates a JSON message for my telemetry info, then serializes
   *             it into a std::string ready to be sent over a socket.
   * @param[out] retstring  string containing the serialization of the JSON message
   *
   */
  void Sequence::make_telemetry_message( std::string &retstring ) {
    // assemble the telemetry I want to report into a json message
    // Set a messagetype keyword to indicate what kind of message this is.
    //
    nlohmann::json jmessage;
    jmessage["messagetype"] = "targetinfo";

    // fill telemetry message only when READY or RUNNING
    //
    if ( this->seq_state.is_any_set( Sequencer::SEQ_READY, Sequencer::SEQ_RUNNING ) ) {
      // Store unconfigured values as NAN.
      // NAN values are not logged to the database.
      //
      jmessage["OBS_ID"] = this->target.obsid < 0 ? NAN : this->target.obsid;           //  OBSERVATION_ID
      jmessage["NAME"] = this->target.name;                                             //  NAME
      jmessage["SLITA"] = this->target.slitangle;                                       // *OTMslitangle
      jmessage["BINSPECT"] = this->target.binspect < 1 ? NAN : this->target.binspect;   // *BINSPECT
      jmessage["BINSPAT"] = this->target.binspat < 1 ? NAN : this->target.binspat;      // *BINSPAT
      jmessage["POINTMODE"] = this->target.pointmode;                                   // *POINTMODE
      jmessage["RA"] = this->target.ra_hms;                                             // *RA
      jmessage["DECL"] = this->target.dec_dms;                                          // *DECL
    }

    retstring = jmessage.dump();  // serialize the json message into a string

    retstring.append(JEOF);       // append JSON message terminator

    return;
  }
  /***** Sequencer::Sequence::make_telemetry_message **************************/


  /***** Sequencer::Sequence::get_external_telemetry **************************/
  /**
   * @brief      collect telemetry from other daemon(s)
   * @details    This is used for any telemetry that I need to collect from
   *             another daemon. Common::collect_telemetry() sends a command
   *             to the daemon, which will respond with a JSON message. The
   *             daemon(s) to contact are configured with the TELEM_PROVIDER
   *             key in the config file.
   *
   */
  void Sequence::get_external_telemetry() {
    // Loop through each configured telemetry provider. This requests
    // their telemetry which is returned as a serialized json string
    // held in retstring.
    //
    // handle_json_message() will parse the serialized json string.
    //
    std::string retstring;
    for ( const auto &provider : this->telemetry_providers ) {
      Common::collect_telemetry( provider, retstring );
      handle_json_message(retstring);
    }
    return;
  }
  /***** Sequencer::Sequence::get_external_telemetry **************************/


  /***** Sequencer::Sequence::handle_json_message *****************************/
  /**
   * @brief      parses incoming telemetry messages
   * @details    Requesting telemetry from another daemon returns a serialized
   *             JSON message which needs to be passed in here to parse it.
   * @param[in]  message_in  incoming serialized JSON message (as a string)
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::handle_json_message( const std::string message_in ) {
    const std::string function="Sequencer::Sequence::handle_json_message";
    std::stringstream message;

    if ( message_in.empty() ) {
      logwrite( function, "ERROR empty JSON message" );
      return ERROR;
    }

    try {
      nlohmann::json jmessage = nlohmann::json::parse( message_in );
      std::string messagetype;

      // jmessage must not contain key "error" and must contain key "messagetype"
      //
      if ( !jmessage.contains("error") ) {
        if ( jmessage.contains("messagetype") && jmessage["messagetype"].is_string() ) {
          messagetype = jmessage["messagetype"];
        }
        else {
          logwrite( function, "ERROR received JSON message with missing or invalid messagetype" );
          return ERROR;
        }
      }
      else {
        logwrite( function, "ERROR in JSON message" );
        return ERROR;
      }

      // No errors, so disseminate the message contents based on the message type.
      //
      // column_from_json<T>( colname, jkey, jmessage ) will extract the value of
      // expected type <T> with key jkey from json string jmessage, and assign it
      // to this->target.external_telemetry[colname] map. It is expected that
      // "colname" is the column name in the database.
      //
      if ( messagetype == "camerainfo" ) {
        this->target.column_from_json<double>( "EXPTIME", "SHUTTIME_SEC", jmessage );
      }
      else
      if ( messagetype == "slitinfo" ) {
        this->target.column_from_json<double>( "SLITWIDTH", "SLITW", jmessage );
        this->target.column_from_json<double>( "SLITOFFSET", "SLITO", jmessage );
      }
      else
      if ( messagetype == "tcsinfo" ) {
        this->target.column_from_json<std::string>( "TELRA", "TELRA", jmessage );
        this->target.column_from_json<std::string>( "TELDECL", "TELDEC", jmessage );
        this->target.column_from_json<double>( "ALT", "ALT", jmessage );
        this->target.column_from_json<double>( "AZ", "AZ", jmessage );
        this->target.column_from_json<double>( "AIRMASS", "AIRMASS", jmessage );
        this->target.column_from_json<double>( "CASANGLE", "CASANGLE", jmessage );
      }
      else
      if ( messagetype == "test" ) {
      }
      else {
        message.str(""); message << "ERROR received unhandled JSON message type \"" << messagetype << "\"";
        logwrite( function, message.str() );
        return ERROR;
      }
    }
    catch ( const nlohmann::json::parse_error &e ) {
      message.str(""); message << "ERROR json exception parsing message: " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR parsing message: " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::handle_json_message *****************************/


  /***** Sequencer::Sequence::dothread_test_fpoffset **************************/
  /**
   * @brief      for testing, calls a Python function from a thread
   *
   */
  void Sequence::dothread_test_fpoffset() {
    std::string function = "Sequencer::Sequence::dothread_fpoffset";
    std::stringstream message;

    message.str(""); message << "calling fpoffsets.compute_offset() from thread: PyGILState=" << PyGILState_Check();
    logwrite( function, message.str() );

    double ra_to, dec_to, angle_to;

    this->target.fpoffsets.compute_offset( "SCOPE", "SLIT", 17, -24, 19, ra_to, dec_to, angle_to );

    message.str(""); message << "output = " << ra_to << " " << dec_to << " " << angle_to << " : PyGILState=" << PyGILState_Check();
    logwrite( function, message.str() );

    return;
  }
  /***** Sequencer::Sequence::dothread_test_fpoffset **************************/


  long Sequence::check_power_on( const std::string which, std::chrono::seconds delay ) { 
    const std::string function="Sequencer::Sequence::check_power_on";
    long error;
    bool need_delay=false;

    // loop through all of the plugs for the named device
    //
    for ( const auto &plug : this->power_switch[which].plugname ) {
      std::string reply;
      std::stringstream cmd;
      cmd << plug;
      error = this->powerd.send( cmd.str(), reply );
      if ( error != NO_ERROR || reply.find(" DONE") == std::string::npos ) {
        logwrite( function, "ERROR checking plug: "+plug );
        continue;
      }

      auto it = reply.find(" DONE");
      int state=-1;
      try {
        state = std::stoi(reply.substr(0,it));
      }
      catch( const std::exception &e ) {
        logwrite( function, "ERROR parsing reply \""+reply+"\" from plug: "+plug );
        continue;
      }

      if ( state==0 ) {
        cmd << " ON";
        logwrite( function, "turning on plug "+plug );
        error = this->powerd.send( cmd.str(), reply );
        if ( error != NO_ERROR || reply.find(" DONE") != std::string::npos ) {
          logwrite( function, "ERROR turning on plug: "+plug );
          continue;
        }
        // if anything was turned on then wait for power-on delay
        need_delay=true;
      }
      else
      if ( state==1 ) {
        logwrite( function, "plug "+plug+" already on" );
        continue;
      }
      else {
        logwrite( function, "ERROR bad reply \""+reply+"\" from plug: "+plug );
        continue;
      }
    }

    if ( need_delay ) {
      logwrite( function, "waiting "+std::to_string(delay.count())+"s for "+which+" to boot" );
      std::this_thread::sleep_for(delay);
    }

    return error;
  }


  long Sequence::check_connected( Common::DaemonClient &daemon, bool &was_opened ) {
    const std::string function="Sequencer::Sequence::check_connected";
    bool isopen=false;
    std::string reply;
    long error;

    // if not connected to the power daemon then connect
    //
    if ( !daemon.socket.isconnected() ) {
      logwrite( function, "connecting to "+daemon.name );
      if ( daemon.connect() != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR connecting to "+daemon.name );
        return ERROR;
      }
    }
    logwrite( function, "connected to "+daemon.name );

    // Ask if hardware connection is open
    //
    error  = daemon.send( "isopen", reply );
    error |= this->parse_state( function, reply, isopen );
    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR opening "+daemon.name );
      return ERROR;
    }

    // and open it if necessary.
    //
    if ( !isopen ) {
      logwrite( function, "opening "+daemon.name );
      if ( daemon.send( "open", reply ) != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR opening connection to "+daemon.name );
        return ERROR;
      }
      was_opened=true;
      logwrite( function, "opened "+daemon.name+" hardware" );
    }
    else logwrite( function, daemon.name+" hardware was already open" );

    return NO_ERROR;
  }


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
    // help -- list testnames
    // ----------------------------------------------------
    //
    if ( testname == "?" || testname == "help" ) {
      retstring = "test <testname> ...\n";
      retstring.append( "   addrow ? | <number> <name> <RA> <DEC> <slitangle> <slitwidth> <exptime>\n" );
      retstring.append( "   async [ ? | <message> ]\n" );
      retstring.append( "   acquire [ ? ]\n" );
      retstring.append( "   cameraset [ ? ]\n" );
      retstring.append( "   cancel [ ? ]\n" );
      retstring.append( "   clearlasttarget\n" );
      retstring.append( "   completed [ ? ]\n" );
      retstring.append( "   expose [ ? ]\n" );
      retstring.append( "   fpoffset ? | <from> <to>\n" );
      retstring.append( "   getnext [ ? ]\n" );
      retstring.append( "   gettelem [ ? ]\n" );
      retstring.append( "   isready [ ? ]\n" );
      retstring.append( "   moveto [ ? | <solverargs> ]\n" );
      retstring.append( "   notify [ ? ]\n" );
      retstring.append( "   pause [ ? ]\n" );
      retstring.append( "   pending [ ? ]\n" );
      retstring.append( "   targetinfo [ ? ]\n" );
      retstring.append( "   prologue [ ? ]\n" );
      retstring.append( "   radec [ ? ]\n" );
      retstring.append( "   resume [ ? ]\n" );
      retstring.append( "   single <RA>,<DEC>,<slitangle>,<slitwidth>,<exptime>,<binspect>,<binspat>\n" );
      retstring.append( "   setstate [ ? ]\n" );
      retstring.append( "   startup ? | <module>\n" );
      retstring.append( "   states [ ? ]\n" );
      retstring.append( "   tablenames [ ? ]\n" );
      retstring.append( "   threadoffset [ ? ]\n" );
      retstring.append( "   update ? | { pending | complete | unassigned }\n" );
      return HELP;
    }
    else

    // ----------------------------------------------------
    // async -- queue an asynchronous message
    // ----------------------------------------------------
    //
    if ( testname == "async" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test async [ <message> ]\n";
        retstring.append( "  Queue and broadcast optional <message>. If <message> not supplied\n" );
        retstring.append( "  then broadcast \"test\".\n" );
        return HELP;
      }
      if ( tokens.size() > 1 ) {
        bool first=true;
        message.str("");
        for ( const auto &word : tokens ) {
          if ( first ) { first=false; continue; }  // skip the testname
          message << word << " ";
        }
        logwrite( function, message.str() );
        this->async.enqueue( message.str() );
      }
      else {
        logwrite( function, "test" );
        this->async.enqueue( "test" );
      }
    }
    else

    // ----------------------------------------------------
    // setstate -- set any state
    // ----------------------------------------------------
    //
    if ( testname == "setstate" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test setstate\n";
        retstring.append( "  set any seq_state.\n" );
        return HELP;
      }
      else
      if ( tokens.size() > 1 && tokens[1] == "READY" ) {
        this->seq_state.set( Sequencer::SEQ_READY );
        return NO_ERROR;
      }
      else {
        logwrite( function, "ERROR excpected { READY }" );
        return ERROR;
      }
    }
    else

    // ----------------------------------------------------
    // cancel -- cancel pending states
    // ----------------------------------------------------
    //
    if ( testname == "cancel" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test cancel\n";
        retstring.append( "  Cancel pending wait seq_states.\n" );
        return HELP;
      }

      message.str(""); message << "seq_state bits pending before: " << this->seq_state.get_pending_names();
      logwrite( function, message.str() );
      message.str(""); message << "seq_state bits set before: " << this->seq_state.get_set_names();
      logwrite( function, message.str() );

      this->seq_state.cancel_wait();                // cancels all waiting threads and resets all pending bits
      this->seq_state.set( Sequencer::SEQ_READY );  // return to READY state
      this->broadcast_seqstate();
      try {
        this->seq_state.wait_for_state_set( Sequencer::SEQ_READY );
      }
      catch ( std::exception & ) {
        this->async.enqueue_and_log( function, "NOTICE: all states cancelled" );
      }

      message.str(""); message << "seq_state bits pending after:  " << this->seq_state.get_pending_names();
      logwrite( function, message.str() );
      message.str(""); message << "seq_state bits set after:  " << this->seq_state.get_set_names();
      logwrite( function, message.str() );

//    logwrite( function, "force clear all states" );
//    this->seq_state.clear_all();
//    this->req_state.clear_all();
//    this->broadcast_seqstate();
    }
    else

    // ----------------------------------------------------
    // pending -- show state bits pending
    // ----------------------------------------------------
    //
    if ( testname == "pending" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test pending\n";
        retstring.append( "  Show state bits pending\n" );
        return HELP;
      }

      message.str(""); message << "seq_state bits pending: " << this->seq_state.get_pending_names();
      logwrite( function, message.str() );

      retstring = message.str();
    }
    else

    // ----------------------------------------------------
    // prologue -- show the camera prologue commands
    // ----------------------------------------------------
    //
    if ( testname == "prologue" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test prologue\n";
        retstring.append( "  Log all of the camera prologue commands.\n" );
        retstring.append( "  These are commands that will be sent to the camera daemon\n" );
        retstring.append( "  on initialization.\n" );
        return HELP;
      }

      for ( const auto &cmd : this->camera_prologue ) {
        logwrite( function, "camera "+cmd );
      }
    }
    else

    // ----------------------------------------------------
    // isready -- show the daemon_ready word
    // ----------------------------------------------------
    //
    if ( testname == "isready" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test isready\n";
        retstring.append( "  Report which systems are ready\n" );
        return HELP;
      }

      // write to the log (textually) which bits are set
      //
      retstring.clear();
      message.str(""); message << "NOTICE: daemons ready: " << this->daemon_ready.get_set_names();
      this->async.enqueue_and_log( function, message.str() );
      retstring.append( message.str() ); retstring.append( "\n" );

      message.str(""); message << "NOTICE: daemons not ready: " << this->daemon_ready.get_clear_names();
      this->async.enqueue_and_log( function, message.str() );
      retstring.append( message.str() );

      error = NO_ERROR;
    }
    else

    // ----------------------------------------------------
    // cameraset -- sets the camera according to the parameters in the target entry
    // ----------------------------------------------------
    //
    if ( testname == "cameraset" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test cameraset\n";
        retstring.append( "  Set only the camera according to the parameters in the target row.\n" );
        return HELP;
      }
      this->seq_state.set( Sequencer::SEQ_WAIT_CAMERA );                   // set the current state
      this->broadcast_seqstate();
      std::thread( &Sequencer::Sequence::dothread_camera_set, this ).detach();        // set camera in a thread
    }
    else

    // ----------------------------------------------------
    // expose -- trigger exposure
    // ----------------------------------------------------
    //
    if ( testname == "expose" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test expose\n";
        retstring.append( "  Trigger an exposure.\n" );
        return HELP;
      }
      this->seq_state.set( Sequencer::SEQ_WAIT_CAMERA );                   // set the current state
      this->broadcast_seqstate();
      std::thread( &Sequencer::Sequence::dothread_trigger_exposure, this ).detach();  // trigger exposure in a thread
    }
    else

    // ----------------------------------------------------
    // states -- get the current seqstate and reqstate
    // ----------------------------------------------------
    //
    if ( testname == "states" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test states\n";
        retstring.append( "  Log the REQSTATE and SEQSTATE bits that are set,\n" );
        retstring.append( "  and a list of the threads currently running\n" );
        return HELP;
      }

      // get the seqstate and reqstate and put them (numerically) into the return string
      //
      message.str("");
      message << this->seq_state.get() << " " << this->req_state.get() << " " << this->thread_state.get();
      retstring = message.str();

      // but now I'm going to write to the log (textually) which bits are set
      //
      message.str(""); message << "RUNSTATE: " << this->seq_state.get_set_names();
      this->async.enqueue( message.str() );
      logwrite( function, message.str() );

      // same for the requested state
      //
      message.str(""); message << "REQSTATE: " << this->req_state.get_set_names();
      this->async.enqueue( message.str() );
      logwrite( function, message.str() );

      // and for the thread state
      //
      message.str(""); message << "THREADS: " << this->thread_state.get_set_names();
      logwrite( function, message.str() );

      error = NO_ERROR;
    }
    else

    // ----------------------------------------------------
    // single -- get command line info for a single observation w/o the database
    //           must specify in CSV order:
    //           RA, DECL, SLITANGLE, SLITWIDTH, EXPTIME, BINSPECT, BINSPAT, POINTMODE
    // ----------------------------------------------------
    //
    if ( testname == "single" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test single <RA>,<DEC>,<slitangle>,<slitwidth>,<exptime>,<binspect>,<binspat>\n";
        retstring.append( "  Get command line info for a single observation without the database.\n" );
        retstring.append( "  Arguments must be comma delimited in the order shown.\n" );
        return HELP;
      }
      std::string::size_type pos = args.find( "single " );         // note space at end of test name!
      std::string arglist = args.substr( pos+strlen("single ") );  // arglist is the rest of the args string after "single "

      // args does not contain "single " (note space! no space means no args)
      //
      if ( pos == std::string::npos ) {
        logwrite( function, "ERROR: expected single <RA>, <DEC>, <slitangle>, <slitwidth>, <exptime>, <binspect>, <binspat>, <pointmode>" );
        return( ERROR );
      }

      // Tokenize arglist on the comma.
      // Expecting 8 tokens: RA, DECL, SLITANGLE, SLITWIDTH, EXPTIME, BINSPECT, BINSPAT, POINTMODE
      //
      Tokenize( arglist, tokens, "," );
      if ( tokens.size() != 8 ) {
        logwrite( function, "ERROR: expected single <RA>, <DEC>, <slitangle>, <slitwidth>, <exptime>, <binspect>, <binspat>, <pointmode>" );
        return( ERROR );
      }

      try {
        // The following items are read from the database with the "get_next()" function,
        // and those marked with an asterick is what is set here. Everything else
        // we don't really need.
        //
        this->target.obsid       = 0;                          //  OBSERVATION_ID
        this->target.obsorder    = 0;                          //  OBS_ORDER
        this->target.name        = "TEST";                     //  NAME
        this->target.state       = Sequencer::TARGET_PENDING;  //  STATE
        this->target.ra_hms      = tokens.at(0);               // *RA
        this->target.dec_dms     = tokens.at(1);               // *DECL
        this->target.casangle    = 0.;                         //  OTMcass
        this->target.slitangle   = std::stod( tokens.at(2) );  // *OTMslitangle
        this->target.slitwidth   = std::stod( tokens.at(3) );  // *OTMslit
        this->target.slitoffset  = 0.;                         //  SLITOFFSET
        this->target.exptime_req = std::stod( tokens.at(4) );  // *OTMexpt
        this->target.targetnum   = 0;                          //  TARGET_NUMBER
        this->target.sequencenum = 0;                          //  SEQUENCE_NUMBER
        this->target.binspect    = std::stoi( tokens.at(5) );  // *BINSPECT
        this->target.binspat     = std::stoi( tokens.at(6) );  // *BINSPAT
        this->target.pointmode   = to_uppercase(tokens.at(7)); // *POINTMODE
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "out of range parsing args " << args << ": " << e.what();
        logwrite( function, message.str() );
        error = ERROR;
      }
      catch( std::invalid_argument &e ) {
        message.str(""); message << "invalid argument parsing args " << args << ": " << e.what();
        logwrite( function, message.str() );
        error = ERROR;
      }
      error = NO_ERROR;
    }
    else

    // ----------------------------------------------------
    // getnext -- get the next pending target from the database
    // ----------------------------------------------------
    //
    if ( testname == "getnext" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test getnext\n";
        retstring.append( "  Read the next PENDING target from the database.\n" );
        retstring.append( "  This is the equivalent (and necessary) step performed prior\n" );
        retstring.append( "  to starting an observation.\n" );
        return HELP;
      }

      TargetInfo::TargetState ret;
      std::stringstream rts;
      std::string targetstatus;
      if ( tokens.size() > 1 ) {
        ret = this->target.get_next( tokens[1], targetstatus );  // if state supplied then get next target with this state
      }
      else {
        ret = this->target.get_next( targetstatus );             // otherwise use the default (which is "pending")
      }
      error = NO_ERROR;

      message.str(""); message << "NOTICE: " << targetstatus;
      this->async.enqueue( message.str() );                      // broadcast target status

      if ( ret == TargetInfo::TargetState::TARGET_FOUND ) {
        rts << "name      obsid  order  ra  dec  casangle  slitangle  airmasslim\n";
        rts << this->target.name  << " "
            << this->target.obsid << "   "
            << this->target.obsorder << "   "
            << this->target.ra_hms << "  "
            << this->target.dec_dms << "  "
            << this->target.casangle << "  "
            << this->target.slitangle << "  "
            << this->target.airmasslimit << "\n";
      }
      else
      if ( ret == TargetInfo::TargetState::TARGET_NOT_FOUND ) { rts << "(none)"; }
      else
      if ( ret == TargetInfo::TargetState::TARGET_ERROR )     { error = ERROR; }

      retstring = rts.str();
    }
    else

    // ----------------------------------------------------
    // targetinfo -- print loaded target info
    // ----------------------------------------------------
    //
    if ( testname == "targetinfo" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test targetinfo\n";
        retstring.append( "  Print loaded target info.\n" );
        return HELP;
      }

      std::stringstream rts;

      rts << "name      obsid  RA  DEC  casangle  slitangle  slitwidth exptime binspat binspect RAoffs DECoffs\n";
      rts << this->target.name  << " "
          << this->target.obsid << "   "
          << this->target.ra_hms << "  "
          << this->target.dec_dms << "  "
          << this->target.casangle << "  "
          << this->target.slitangle << "  "
          << this->target.slitwidth << "  "
          << this->target.exptime_req << "  "
          << this->target.binspat << "  "
          << this->target.binspect << " "
          << this->target.offset_ra << " "
          << this->target.offset_dec << " "
          << "\n";

      // convert to decimal and to scope coordinates.
      // (fpoffsets.coords_* are always in degrees)
      //
      double ra_in    = radec_to_decimal( this->target.ra_hms  ) * TO_DEGREES;
      double dec_in   = radec_to_decimal( this->target.dec_dms );
      double angle_in = this->target.slitangle;

      // can't be NaN
      //
      bool ra_isnan  = std::isnan( ra_in  );
      bool dec_isnan = std::isnan( dec_in );

      if ( ra_isnan || dec_isnan ) {
        message.str(""); message << "ERROR: converting";
        if ( ra_isnan  ) { message << " RA=\"" << this->target.ra_hms << "\""; }
        if ( dec_isnan ) { message << " DEC=\"" << this->target.dec_dms << "\""; }
        message << " to decimal";
        logwrite( function, message.str() );
      }

      // Before sending the target coords to the TCS,
      // convert them from <pointmode> to scope coordinates.
      //
      double ra_out, dec_out, angle_out;
      error = this->target.fpoffsets.compute_offset( this->target.pointmode, "SCOPE",
                                                   ra_in, dec_in, angle_in,
                                                   ra_out, dec_out, angle_out );

      rts << "TCS coords: ra=" << ra_out*TO_HOURS << "  dec=" << dec_out << "  angle=" << angle_out << "\n";
      retstring = rts.str();
    }
    else
    // ----------------------------------------------------
    // gettelem -- get external telemetry
    // ----------------------------------------------------
    //
    if ( testname == "gettelem" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test gettelem\n";
        retstring.append( "  Get external telemetry from other daemons.\n" );
        return HELP;
      }
      this->get_external_telemetry();
      message.str("");
      for ( const auto &[name,data] : this->target.external_telemetry ) {
        message << "name=" << name << " valid=" << (data.valid?"T":"F") << " value=" << data.value << "\n";
      }
      retstring = message.str();
    }
    else

    // ----------------------------------------------------
    // addrow -- insert a (fixed, hard-coded) row into the database
    // ----------------------------------------------------
    //
    if ( testname == "addrow" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test addrow <number> <name> <RA> <DEC> <slitangle> <slitwidth> <exptime>\n";
        retstring.append( "  Insert a fixed row into the database. Arguments are space delimited\n" );
        retstring.append( "  in the order shown.\n" );
        return HELP;
      }
      int number=0;
      std::string name, ra, dec, pmode;
      double etime=0., slitw=1., slita=18.;
      if ( tokens.size() != 9 ) {
        logwrite( function, "ERROR: expected \"addrow <number> <name> <RA> <DEC> <slitangle> <slitwidth> <exptime> <pointmode>\"" );
        return( ERROR );
      }
      try {
        number = std::stoi( tokens.at(1) );
        name   = tokens.at(2);
        ra     = tokens.at(3);
        dec    = tokens.at(4);
        slita  = std::stod( tokens.at(5) );
        slitw  = std::stod( tokens.at(6) );
        etime  = std::stod( tokens.at(7) );
        pmode  = to_uppercase( tokens.at(8) );
      }
      catch( std::exception &e ) {
        message.str(""); message << "ERROR parsing args " << args << ": " << e.what();
        logwrite( function, message.str() );
        return ERROR;
      }
      error = this->target.add_row( number, name, ra, dec, slita, slitw, etime, pmode );
    }
    else

    // ----------------------------------------------------
    // completed -- insert a record into completed observations table
    // ----------------------------------------------------
    //
    if ( testname == "completed" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test completed\n";
        retstring.append( "  Update state of current target in target table to COMPLETE\n" );
        retstring.append( "  and insert a record in the completed observations table.\n" );
        retstring.append( "  A target must have been first read from the database (e.g. test getnext)\n" );
        return HELP;
      }
      error = this->target.update_state( Sequencer::TARGET_COMPLETE );
      if (error==NO_ERROR) error = this->target.insert_completed();

      // let the world know of the state change
      //
      message.str(""); message << "TARGETSTATE:" << this->target.state
                               << " TARGET:"     << this->target.name
                               << " OBSID:"      << this->target.obsid;
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
        if ( tokens[1] == "?" ) {
          retstring = "test update { pending | complete | unassigned }\n";
          retstring.append( "  Update state of current target in target table to PENDING | COMPLETE | UNASSIGNED.\n" );
          retstring.append( "  A target must have been first read from the database (e.g. test getnext)\n" );
          return HELP;
        }
        if ( tokens[1] != "pending" && tokens[1] != "complete" && tokens[1] != "unassigned" ) {
          logwrite( function, "update expected { pending | complete | unassigned }" );
          return( ERROR );
        }
        error = this->target.update_state( tokens[1] );   // 

        // let the world know of the state change
        //
        message.str(""); message << "TARGETSTATE:" << this->target.state
                                 << " TARGET:"     << this->target.name
                                 << " OBSID:"      << this->target.obsid;
        this->async.enqueue( message.str() );
      }
    }
    else

    // ----------------------------------------------------
    // radec -- convert RA,DEC from HH:MM:SS to decimal
    // ----------------------------------------------------
    //
    if ( testname == "radec" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test radec\n";
        retstring.append( "  Convert the RA,DEC of the current target from HH:MM:SS to decimal.\n" );
        retstring.append( "  A target must have been first read from the database (e.g. test getnext)\n" );
        return HELP;
      }
      double ra,dec;
      ra  = radec_to_decimal( target.ra_hms );
      dec = radec_to_decimal( target.dec_dms );
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
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test notify\n";
        retstring.append( "  Send a notification signal to unblock all waiting threads.\n" );
        return HELP;
      }
      this->cv.notify_all();
    }
    else

    // ----------------------------------------------------
    // tablenames -- print the names of the tables in the DB
    // ----------------------------------------------------
    //
    if ( testname == "tablenames" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test tablenames\n";
        retstring.append( "  Print the names of the tables in the database\n" );
        return HELP;
      }
      error = this->target.get_table_names();
    }
    else

    // -------------------------------------------------------
    // pause -- send async command to camera to pause exposure
    // -------------------------------------------------------
    //
    if ( testname == "pause" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test pause\n";
        retstring.append( "  Send an asynchronous command to camerad to pause exposure.\n" );
        return HELP;
      }
      error = this->camerad.async( "PEX" );
    }
    else

    // ---------------------------------------------------------
    // resume -- send async command to camera to resume exposure
    // ---------------------------------------------------------
    //
    if ( testname == "resume" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test resume\n";
        retstring.append( "  Send an asynchronous command to camerad to resume exposure.\n" );
        return HELP;
      }
      error = this->camerad.async( "REX" );
    }
    else

    // ---------------------------------------------------------
    // moveto -- spawn thread to move to target
    // ---------------------------------------------------------
    //
    if ( testname == "moveto" ) {

      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test moveto { <solverargs> }\n";
        retstring.append( "  Spawn a thread to move to the target.\n" );
        retstring.append( "  This will disable guiding, slew the telescope, move the cass ring,\n" );
        retstring.append( "  and start the acquisition sequence. Optional <solverargs> may be\n" );
        retstring.append( "  included to send to the solver.\n" );
        retstring.append( "  A target must have been first read from the database (e.g. test getnext)\n" );
        return HELP;
      }

      // any and all args are taken to be optional solver args
      //
      if ( tokens.size() > 1 ) {
        std::string::size_type pos = args.find( "moveto " );            // note space at end of test name!
        this->test_solver_args = args.substr( pos+strlen("moveto ") );  // remainder of args string after "moveto "
      }
      else this->test_solver_args.clear();                              // clear previous solver args if not specified

      if ( !this->test_solver_args.empty() ) {
        message.str(""); message << "NOTICE: test solver args: " << this->test_solver_args;
      }
      this->async.enqueue_and_log( function, message.str() );

///   this->is_tcs_ontarget.store( false );
      this->tcs_nowait.store( false );
      logwrite( function, "spawning dothread_move_to_target..." );
      std::thread( &Sequencer::Sequence::dothread_move_to_target, this ).detach();
    }
    else

    // ---------------------------------------------------------
    // acquire -- spawn thread to start acquisition
    //            this sends the "prepare" command to acamd, then
    //            performs the acquisition sequence.
    // ---------------------------------------------------------
    //
    if ( testname == "acquire" ) {

      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test acquire\n";
        retstring.append( "  Spawn a thread to start the acquisition sequence.\n" );
        retstring.append( "  This only performs the acquisition so the TCS must\n" );
        retstring.append( "  already be tracking on a target.\n" );
        return HELP;
      }

      // Read the current cass angle from the TCS
      //
      double cass_now = NAN;
      error = this->get_tcs_cass( cass_now );

      // This call is just to convert the current cass rotator angle to a slit position angle,
      // so we don't care about the RA, DEC coordinates.
      //
      double ra_out, dec_out, angle_out;
      error = this->target.fpoffsets.compute_offset( "SCOPE", "SLIT", 0, 0, cass_now,
                                                     ra_out, dec_out, angle_out );

      // Before starting acquire thread, must first send coordinates to the acam cameraserver
      //
      double ra_in    = radec_to_decimal( this->target.ra_hms  ) * TO_DEGREES;
      double dec_in   = radec_to_decimal( this->target.dec_dms );
      double angle_in = angle_out;

      // can't be NaN
      //
      bool ra_isnan  = std::isnan( ra_in  );
      bool dec_isnan = std::isnan( dec_in );
      bool cas_isnan = std::isnan( angle_in );

      if ( ra_isnan || dec_isnan || cas_isnan ) {
        message.str(""); message << "ERROR: converting";
        if ( ra_isnan  ) { message << " RA=\"" << this->target.ra_hms << "\""; }
        if ( dec_isnan ) { message << " DEC=\"" << this->target.dec_dms << "\""; }
        if ( cas_isnan ) { message << " CASS=\"" << cass_now << "\""; }
        message << " to decimal";
        this->async.enqueue_and_log( function, message.str() );
        return ERROR;
      }

      // Finally, spawn the acquisition thread
      //
      logwrite( function, "spawning dothread_acquisition..." );
      if (error==NO_ERROR) std::thread( &Sequencer::Sequence::dothread_acquisition, this ).detach();
    }
    else

    // ---------------------------------------------------------
    // clearlasttarget -- clear the last target name, allowing repointing
    //                    to the same target (otherwise move_to_target won't
    //                    repoint the telescope if the name is the same)
    // ---------------------------------------------------------
    //
    if ( testname == "clearlasttarget" ) {
      this->last_target="";
      error=NO_ERROR;
    }
    else

    // ---------------------------------------------------------
    // fpoffset -- convert coordinates of current target
    // ---------------------------------------------------------
    //
    if ( testname == "fpoffset" ) {

      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test fpoffset <from> <to>\n";
        retstring.append( "  Convert the coordinates of the current target from one\n" );
        retstring.append( "  coordinate system to another, as specified by <from> and <to>.\n" );
        retstring.append( "  A target must have been first read from the database (e.g. test getnext)\n" );
        return HELP;
      }

      std::string from, to;
      double ra_in  = radec_to_decimal( target.ra_hms ) * TO_DEGREES ;  // fpoffsets must be in degrees
      double dec_in = radec_to_decimal( target.dec_dms );
      double angle_in = this->target.casangle;
      try {
        from = tokens.at(1);
        to   = tokens.at(2);
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR: out of range parsing from/to from " << args << ": " << e.what();
        logwrite( function, message.str() );
      }
      catch( std::invalid_argument &e ) {
        message.str(""); message << "ERROR: invalid argument parsing from/to from " << args << ": " << e.what();
        logwrite( function, message.str() );
      }
      double ra_out, dec_out, angle_out;
      error = this->target.fpoffsets.compute_offset( from, to,
                                                     ra_in, dec_in, angle_in,
                                                     ra_out, dec_out, angle_out );
      message.str(""); message << ra_in << " " << dec_in << " " << angle_in << " "
                               << from << " -> " << to << " "
                               << ra_out << " " << dec_out << " " << angle_out;
      retstring = message.str();
    }
    else

    // ---------------------------------------------------------
    // script -- run a script -- experimental
    // ---------------------------------------------------------
    //
    if ( testname == "script" ) {

      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test script <file>\n";
        retstring.append( "  Run <file> as a script. EXPERIMENTAL!\n" );
        return HELP;
      }

      if ( tokens.size() != 2 ) {
        logwrite( function, "ERROR: expected one argument" );
        retstring = "bad_arg";
        return( ERROR );
      }

//    if (error==NO_ERROR) std::thread( dothread_runscript, std::ref(*this) ).detach();
    }
    else

    // ---------------------------------------------------------
    // startup -- startup a single specified module
    // ---------------------------------------------------------
    //
    if ( testname == "startup" || testname == "shutdown" ) {

      if ( tokens.size() > 1 && ( tokens[1] == "?" || tokens[1] == "help" ) ) {
        retstring = "test startup | shutdown <module>\n";
        retstring.append( "  Startup or shutdown only a single specified module in a manner similar\n" );
        retstring.append( "  to the startup command, but only the specified module.\n" );
        retstring.append( "  Valid modules are:\n" );
        retstring.append( "    power | acam | andor | calib | camera | flexure | focus |\n" );
        retstring.append( "    slicecam | slit | tcs <which>.\n" );
        retstring.append( "  Note that power must be running before any other module.\n" );
        retstring.append( "  Module tcs requires an additional argument <which> = sim | tcs\n" );
        return HELP;
      }

      if ( tokens.size() < 2 ) {
        logwrite( function, "ERROR expected startup <module>" );
        retstring="invalid_argument";
        return( ERROR );
      }

      bool ispower = false;
      std::string reply;

      // power module must be initialized before any others. If this is not
      // a request for starting power, then check that the power module is
      // initialized. If not connected then it can't be initialized.
      //
      if ( tokens[1] != "power" && ! this->powerd.socket.isconnected() ) {
        logwrite( function, "ERROR power module must be initialized first" );
        retstring = "power_not_initialized";
        return( ERROR );
      }
      else
      if ( tokens[1] != "power" && this->powerd.socket.isconnected() ) {

        // Connected, so ask if it's open to the hardware
        //
        error  = this->powerd.send( POWERD_ISOPEN, reply );
        error |= this->parse_state( function, reply, ispower );

        if ( error != NO_ERROR ) {
          logwrite( function, "ERROR communicating with power module" );
          retstring = "power_not_initialized";
          return( ERROR );
        }

        if ( ! ispower ) {
          logwrite( function, "ERROR power module must be initialized first" );
          retstring = "power_not_initialized";
          return( ERROR );
        }
      }

      // optionally wait for startup to complete before returning,
      // otherwise return immediately
      //
      bool wait=false;
      if ( tokens.size() == 3 && tokens[2]=="wait" ) wait=true;

      // startup or shutdown?
      //
      bool isinit = true;
      if ( testname == "startup" ) isinit=true; else if ( testname == "shutdown" ) isinit=false;

      if ( tokens[1] == "power" ) {
        if ( this->powerd.socket.isconnected() ) {
          std::string reply;
          error  = this->powerd.send( POWERD_ISOPEN, reply );
          error |= this->parse_state( function, reply, ispower );
        }
        if ( ! ispower ) {
          this->seq_state.set( Sequencer::SEQ_WAIT_POWER );
          this->broadcast_seqstate();
          std::thread( dothread_power_init, std::ref(*this) ).detach();
          if ( wait ) {
            try {
              this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_POWER );
            }
            catch ( std::exception & ) {
              this->async.enqueue_and_log( function, "NOTICE: power startup aborted" );
            }
          }
        }
      }
      else
      if ( tokens[1] == "acam" ) {
        this->seq_state.set( Sequencer::SEQ_WAIT_ACAM );
        this->broadcast_seqstate();
        std::thread( (isinit ? dothread_acam_init : dothread_acam_shutdown), std::ref(*this) ).detach();
        if ( wait ) {
          try {
            this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_ACAM );
          }
          catch ( std::exception & ) {
            this->async.enqueue_and_log( function, "NOTICE: acam aborted" );
          }
        }
      }
      else
      if ( tokens[1] == "andor" ) {
        if ( !isinit ) {
          logwrite( function, "ERROR there is no andor shutdown. use acam and slicecam" );
          return ERROR;
        }
        this->seq_state.set( Sequencer::SEQ_WAIT_ACAM, Sequencer::SEQ_WAIT_SLICECAM );
        this->broadcast_seqstate();
        std::thread( dothread_andor_init, std::ref(*this) ).detach();
        if ( wait ) {
          try {
            this->seq_state.wait_for_states_clear( Sequencer::SEQ_WAIT_ACAM, Sequencer::SEQ_WAIT_SLICECAM );
          }
          catch ( std::exception & ) {
            this->async.enqueue_and_log( function, "NOTICE: andor aborted" );
          }
        }
      }
      else
      if ( tokens[1] == "calib" ) {
        this->seq_state.set( Sequencer::SEQ_WAIT_CALIB );
        this->broadcast_seqstate();
        std::thread( isinit ? dothread_calib_init : dothread_calib_shutdown, std::ref(*this) ).detach();
        if ( wait ) {
          try {
            this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_CALIB );
          }
          catch ( std::exception & ) {
            this->async.enqueue_and_log( function, "NOTICE: calib aborted" );
          }
        }
      }
      else
      if ( tokens[1] == "camera" ) {
        this->seq_state.set( Sequencer::SEQ_WAIT_CAMERA );
        this->broadcast_seqstate();
        std::thread( isinit ? dothread_camera_init : dothread_camera_shutdown, std::ref(*this) ).detach();
        if ( wait ) {
          try {
            this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_CAMERA );
          }
          catch ( std::exception & ) {
            this->async.enqueue_and_log( function, "NOTICE: camera aborted" );
          }
        }
      }
      else
      if ( tokens[1] == "flexure" ) {
        this->seq_state.set( Sequencer::SEQ_WAIT_FLEXURE );
        this->broadcast_seqstate();
        std::thread( isinit ? dothread_flexure_init : dothread_flexure_shutdown, std::ref(*this) ).detach();
        if ( wait ) {
          try {
            this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_FLEXURE );
          }
          catch ( std::exception & ) {
            this->async.enqueue_and_log( function, "NOTICE: flexure aborted" );
          }
        }
      }
      else
      if ( tokens[1] == "focus" ) {
        this->seq_state.set( Sequencer::SEQ_WAIT_FOCUS );
        this->broadcast_seqstate();
        std::thread( isinit ? dothread_focus_init : dothread_focus_shutdown, std::ref(*this) ).detach();
        if ( wait ) {
          try {
            this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_FOCUS );
          }
          catch ( std::exception & ) {
            this->async.enqueue_and_log( function, "NOTICE: focus aborted" );
          }
        }
      }
      else
      if ( tokens[1] == "slicecam" ) {
        this->seq_state.set( Sequencer::SEQ_WAIT_SLICECAM );
        this->broadcast_seqstate();
        std::thread( isinit ? dothread_slicecam_init : dothread_slicecam_shutdown, std::ref(*this) ).detach();
        if ( wait ) {
          try {
            this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_SLICECAM );
          }
          catch ( std::exception & ) {
            this->async.enqueue_and_log( function, "NOTICE: slicecam aborted" );
          }
        }
      }
      else
      if ( tokens[1] == "slit" ) {
        this->seq_state.set( Sequencer::SEQ_WAIT_SLIT );
        this->broadcast_seqstate();
        std::thread( isinit ? dothread_slit_init : dothread_slit_shutdown, std::ref(*this) ).detach();
        if ( wait ) {
          try {
            this->seq_state.wait_for_state_clear( Sequencer::SEQ_WAIT_SLIT );
          }
          catch ( std::exception & ) {
            this->async.enqueue_and_log( function, "NOTICE: slit aborted" );
          }
        }
      }
      else
      if ( tokens[1] == "tcs" && tokens.size()==3 ) {
        if ( tokens.size() == 3 ) this->tcs_init( tokens[2], retstring );
        else {
          logwrite( function, "ERROR missing tcs type { real sim }" );
          return ERROR;
        }
      }
      else {
        message.str(""); message << "ERROR invalid module \"" << tokens[1] << "\". expected power|acam|calib|camera|flexure|focus|slicecam|slit|tcs <which>";
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return( ERROR );
      }

      message.str(""); message << "started " << tokens[1] << " module";
      logwrite( function, message.str() );
    }
    else

    // ---------------------------------------------------------
    // threadoffset -- spawn a thread to call a python function
    // ---------------------------------------------------------
    //
    if ( testname == "threadoffset" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = SEQUENCERD_TEST;
        retstring.append( " threadoffset\n" );
        retstring.append( "  Spawns a thread which calls a Python function.\n" );
        error=HELP;
      }
      else {
        message.str(""); message << "spawning dothread_fpoffset: PyGILState=" << PyGILState_Check();
        logwrite( function, message.str() );
        std::thread( &Sequencer::Sequence::dothread_test_fpoffset, this ).detach();
        message.str(""); message << "spawned dothread_fpoffset: PyGILState=" << PyGILState_Check();
        logwrite( function, message.str() );
      }
    }
    else {

    // ----------------------------------------------------
    // invalid test name
    // ----------------------------------------------------
    //
      message.str(""); message << "ERROR: test " << testname << " unknown";;
      logwrite(function, message.str());
      error = ERROR;
    }

    return( error );
  }
  /***** Sequencer::Sequence::test ********************************************/
}
