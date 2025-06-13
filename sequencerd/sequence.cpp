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

  constexpr long CAMERA_PROLOG_TIMEOUT = 6000;  ///< timeout msec to send camera prolog command

  /***** Sequencer::Sequence::handletopic_snapshot ***************************/
  /**
   * @brief      publishes snapshot of my telemetry
   * @details    This publishes a JSON message containing a snapshot of my
   *             telemetry info when the subscriber receives the "_snapshot"
   *             topic and the payload contains my daemon name.
   * @param[in]  jmessage_in  subscribed-received JSON message
   *
   */
  void Sequence::handletopic_snapshot( const nlohmann::json &jmessage_in ) {
    // If my name is in the jmessage then publish my snapshot
    //
    if ( jmessage_in.contains( Sequencer::DAEMON_NAME ) ) {
      this->publish_snapshot();
    }
    else
    if ( jmessage_in.contains( "test" ) ) {
      logwrite( "Sequencer::Sequence::handletopic_snapshot", jmessage_in.dump() );
    }
  }
  /***** Sequencer::Sequence::handletopic_snapshot ***************************/


  /***** Sequencer::Sequence::publish_snapshot *******************************/
  /**
   * @brief      publishes snapshot of my telemetry
   * @details    This publishes a JSON message containing a snapshot of my
   *             telemetry.
   *
   */
  void Sequence::publish_snapshot() {
    std::string dontcare;
    this->publish_snapshot(dontcare);
  }
  void Sequence::publish_snapshot(std::string &retstring) {
    this->publish_seqstate();
    this->publish_waitstate();
    this->publish_daemonstate();
  }
  /***** Sequencer::Sequence::publish_snapshot *******************************/


  /***** Sequencer::Sequence::publish_seqstate ********************************/
  /**
   * @brief      publishes sequencer state with topic "seq_seqstate"
   * @details    seqstate is a single string
   *
   */
  void Sequence::publish_seqstate() {
    nlohmann::json jmessage_out;
    jmessage_out["source"]    = Sequencer::DAEMON_NAME;

    // sequencer state
    std::string seqstate( this->seq_state_manager.get_set_states() );
    rtrim( seqstate );
    jmessage_out["seqstate"]  = seqstate;

    try {
      this->publisher->publish( jmessage_out, "seq_seqstate" );
    }
    catch ( const std::exception &e ) {
      logwrite( "Sequencer::Sequence::publish_seqstate",
                "ERROR publishing message: "+std::string(e.what()) );
      return;
    }
  }
  /***** Sequencer::Sequence::publish_seqstate ********************************/


  /***** Sequencer::Sequence::publish_waitstate *******************************/
  /**
   * @brief      publishes wait state with topic "seq_waitstate"
   * @details    wait state has a bit for each thing that the sequencer could
   *             be waiting on, set when waiting, publishes true when set,
   *             false when clear
   *
   */
  void Sequence::publish_waitstate() {
    nlohmann::json jmessage_out;
    jmessage_out["source"]    = Sequencer::DAEMON_NAME;

    // iterate through map of daemon state bits, add each as a key in the JSON message,
    // and set true|false if the bit is set or not
    std::string active_states( this->wait_state_manager.get_set_states() );
    for ( const auto &[bit,state] : Sequencer::wait_state_names ) {
      jmessage_out[state] = ( active_states.find(state)!=std::string::npos ? true : false);
    }

    try {
      this->publisher->publish( jmessage_out, "seq_waitstate" );
    }
    catch ( const std::exception &e ) {
      logwrite( "Sequencer::Sequence::publish_waitstate",
                "ERROR publishing message: "+std::string(e.what()) );
      return;
    }
  }
  /***** Sequencer::Sequence::publish_waitstate *******************************/


  /***** Sequencer::Sequence::publish_daemonstate *****************************/
  /**
   * @brief      publishes daemon state with topic "seq_daemonstate"
   * @details    daemon state has a bit for each daemon, publishes true when
   *             that daemon is ready, false when not
   *
   */
  void Sequence::publish_daemonstate() {
    nlohmann::json jmessage_out;
    jmessage_out["source"]    = Sequencer::DAEMON_NAME;

    // iterate through map of daemon state bits, add each as a key in the JSON message,
    // and set true|false if the bit is set or not
    std::string active_states( this->daemon_manager.get_set_states() );
    for ( const auto &[bit,state] : Sequencer::daemon_names ) {
      jmessage_out[state] = ( active_states.find(state)!=std::string::npos ? true : false);
    }

    try {
      this->publisher->publish( jmessage_out, "seq_daemonstate" );
    }
    catch ( const std::exception &e ) {
      logwrite( "Sequencer::Sequence::publish_daemonstate",
                "ERROR publishing message: "+std::string(e.what()) );
      return;
    }
  }
  /***** Sequencer::Sequence::publish_daemonstate *****************************/


  /***** Sequencer::Sequence::publish_threadstate *****************************/
  /**
   * @brief      publishes thread state with topic "seq_threadstate"
   * @details    thread state has a bit for each thread that can run, publishes
   *             true when that thread is running, false when not
   *
   */
  void Sequence::publish_threadstate() {
    nlohmann::json jmessage_out;
    jmessage_out["source"]    = Sequencer::DAEMON_NAME;

    // iterate through map of thread state bits, add each as a key in the JSON message,
    // and set true|false if the bit is set or not
    std::string active_states( this->thread_state_manager.get_set_states() );
    for ( const auto &[bit,state] : Sequencer::thread_names ) {
      jmessage_out[state] = ( active_states.find(state)!=std::string::npos ? true : false);
    }

    try {
      this->publisher->publish( jmessage_out, "seq_threadstate" );
    }
    catch ( const std::exception &e ) {
      logwrite( "Sequencer::Sequence::publish_threadstate",
                "ERROR publishing message: "+std::string(e.what()) );
      return;
    }
  }
  /***** Sequencer::Sequence::publish_threadstate *****************************/


  /***** Sequencer::Sequence::broadcast_daemonstate ***************************/
  /**
   * @brief      publishes daemonstate and can control seqstate
   * @details    If not STARTING or STOPPING and not all daemons ready then
   *             this ensures that the seqstate drops into NOTREADY.
   *
   */
  void Sequence::broadcast_daemonstate() {
    // always publish daemonstate when called
    this->publish_daemonstate();

    // If any daemon isn't ready then the sequencer can't be ready,
    // but don't override STARTING or STOPPING, unless none are ready.
    if ( daemon_manager.are_all_clear() ) {
      seq_state_manager.set_only( {Sequencer::SEQ_NOTREADY} );
    }
    else
    if ( ! seq_state_manager.is_set(SEQ_STARTING) &&
         ! seq_state_manager.is_set(SEQ_STOPPING) &&
         ! daemon_manager.are_all_set() ) {
      seq_state_manager.set_only( {Sequencer::SEQ_NOTREADY} );
    }
  }
  /***** Sequencer::Sequence::broadcast_daemonstate ***************************/


  /***** Sequencer::Sequence::broadcast_seqstate ******************************/
  /**
   * @brief      writes string of seq_state to the async port
   * @details    This broadcasts the seqstate as a string with the "SEQSTATE:"
   *             message tag.
   *
   */
  void Sequence::broadcast_seqstate() {
    this->publish_seqstate();
    this->async.enqueue_and_log( "Sequencer::Sequence::broadcast_seqstate",
                                 "SEQSTATE: "+seq_state_manager.get_set_states() );
    this->cv.notify_all();
  }
  /***** Sequencer::Sequence::broadcast_seqstate ******************************/


  /***** Sequencer::Sequence::broadcast_waitstate *****************************/
  /**
   * @brief      writes string of all set wait_state bits to the asyn port
   * @details    This broadcasts the seqstate as a string with the "WAITSTATE:"
   *             message tag.
   *
   */
  void Sequence::broadcast_waitstate() {
    this->publish_waitstate();
    this->async.enqueue_and_log( "Sequencer::Sequence::broadcast_waitstate",
                                 "WAITSTATE: "+wait_state_manager.get_set_states() );
    this->cv.notify_all();
  }
  /***** Sequencer::Sequence::broadcast_waitstate *****************************/


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
    const std::string function("Sequencer::Sequence::dothread_sequencer_async_listener");

    ScopedState thr_state( seq.thread_state_manager, Sequencer::THR_SEQUENCER_ASYNC_LISTENER );

    int retval = udp.Listener();

    if ( retval < 0 ) {
      logwrite(function, "error creating UDP listening socket. thread terminating.");
      return;
    }

    logwrite( function, "running" );

    // forever receive and process UDP async status messages
    //
    while ( true ) {

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
        // target's coordinates. notify_tcs_next_target has to be set true by trigger_exposure().
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
        if ( seq.arm_readout_flag && statstr.compare( 0, 10, "PIXELCOUNT" )==0 ) {  // async message tag PIXELCOUNT
          seq.arm_readout_flag = false;
          seq.wait_state_manager.set_and_clear( {Sequencer::SEQ_WAIT_READOUT},      // set READOUT
                                                {Sequencer::SEQ_WAIT_EXPOSE} );     // clear EXPOSE
        }

        // ---------------------------------------------
        // clear READOUT flag on the end-of-frame signal
        // ---------------------------------------------
        //
        if ( statstr.compare( 0, 10, "FRAMECOUNT" ) == 0 ) {                        // async message tag FRAMECOUNT
          if ( seq.wait_state_manager.is_set( Sequencer::SEQ_WAIT_READOUT ) ) {
            seq.wait_state_manager.clear( Sequencer::SEQ_WAIT_READOUT );
          }
        }

        // ---------------------
        // process TEST messages
        // ---------------------
        //
        if ( starts_with( statstr, "TEST:" ) ) {                                    // async message tag TEST
          logwrite( function, "got test message \""+statstr+"\"" );
        }

      }
      catch( std::exception &e ) {
        logwrite( function, "ERROR parsing status string "+statstr+": "+std::string(e.what()) );
      }

    }  // while true

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


  /***** Sequencer::Sequence::sequence_start **********************************/
  /**
   * @brief      main sequence start thread
   * @details    This thread is spawned in response to the sequencer receiving
   *             the "start" command. This thread will in turn spawn additional
   *             needed threads.
   *
   *             If dotype is ONE, this thread will run once and return.
   *
   *             If dotype is ALL, this thread will run as long as there are
   *             targets to read, or until cancelled.
   * @param[in]  obsid_in  optional obsid, specify for single-target observation
   *
   */
  void Sequence::sequence_start(std::string obsid_in="") {
    const std::string function("Sequencer::Sequence::sequence_start");
    std::stringstream message;
    std::string reply;
    std::string targetstatus;
    TargetInfo::TargetState targetstate;
    long error=NO_ERROR;

    // The Sequencer can only be started once
    //
    if ( thread_state_manager.is_set( Sequencer::THR_SEQUENCE_START ) ) {
      this->async.enqueue_and_log( function, "ERROR sequencer already running" );
      return;
    }

    // The Sequencer can only be started when state is READY
    //
    if ( ! seq_state_manager.is_set( Sequencer::SEQ_READY ) ) {
      this->async.enqueue_and_log( function, "ERROR cannot start: system not ready" );
      return;
    }

    ScopedState thr_state( thread_state_manager, Sequencer::THR_SEQUENCE_START );  // this thread is running
    ScopedState seq_state( seq_state_manager, Sequencer::SEQ_RUNNING, true );      // state = RUNNING (only)
    seq_state.destruct_set( Sequencer::SEQ_READY );                                // set state=READY on exit

    // if obsid_in was specified then this is a single-target observation
    //
    if ( !obsid_in.empty() ) this->single_obsid=obsid_in;

    // clear the thread error state
    //
    this->thread_error_manager.clear_all();

    // clear stop flags
    //
    this->cancel_flag.store(false);
    this->is_ontarget.store(false);
    this->is_usercontinue.store(false);

    // This is the main loop which runs as long as there are targets in the database,
    // or until cancelled. Or, this will run once in the case of single-target-mode,
    // which is invoked by storing an OBSID in the class variable single_obsid.
    //
    while ( true ) {

      logwrite( function, "sequencer running" );

      // Get the next target from the database when single_obsid is empty
      //
      if ( this->single_obsid.empty() ) {
        targetstate = this->target.get_next( targetstatus );
      }
      // otherwise get the specified target by OBSID, using the value in single_obsid.
      // This class variable is remembered by copying it to prev_single_obsid but is
      // then cleared after reading it here, so that it doesn't automatically run the
      // single target again. But since it's remembered, it can be re-used if a repeat
      // exposure is explicitly requested.
      //
      else {
        targetstate = this->target.get_specified_target( this->single_obsid, targetstatus );
        this->prev_single_obsid = this->single_obsid;  // remember it, in case REPEAT is requested
        this->single_obsid.clear();                    // but clear this so that single-target is a one-off
        this->dotype( "ONE" );                         // single-target mode must set dotype=ONE
      }

      message.str(""); message << "NOTICE: " << targetstatus;
      this->async.enqueue( message.str() );                                 // broadcast target status

      if ( targetstate == TargetInfo::TARGET_FOUND ) {                      // target found, get the threads going

        // If the TCS is not ready and the target contains TCS coordinates,
        // then we cannot proceed.
        //
        if ( ! this->daemon_manager.is_set( Sequencer::DAEMON_TCS ) ) {
          if ( ! this->target.ra_hms.empty() || ! this->target.dec_dms.empty() ) {
            message.str(""); message << "ERROR cannot move to target " << this->target.name
                                     << " because TCS is not connected";
            this->async.enqueue_and_log( function, message.str() );
            this->thread_error_manager.set( THR_SEQUENCE_START );           // report error
            break;
          }
        }

        error = this->target.update_state( Sequencer::TARGET_ACTIVE );      // set ACTIVE state in database (this says we are using this target)
        if (error!=NO_ERROR) {
          this->thread_error_manager.set( THR_SEQUENCE_START );             // report any error
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
      else  // targetstate not TARGET_FOUND
      if ( targetstate == TargetInfo::TARGET_NOT_FOUND ) {                // no target found is an automatic stop
        logwrite( function, "NOTICE: no targets found. stopping" );
        break;
      }
      else
      if ( targetstate == TargetInfo::TARGET_ERROR ) {                    // request stop on error
        this->async.enqueue_and_log( function, "ERROR getting next target. stopping" );
        break;
      }

      // get the threads going --
      //
      // These things can all be done in parallel, just have to sync up at the end.
      //

      // threads to start, pair their ThreadStatusBit with the function to call
      std::vector<std::pair<Sequencer::ThreadStatusBits, std::function<long()>>> worker_threads;

      // If pointmode is ACAM then the user has chosen to put the star on ACAM, in
      // which case the assumption is made that nothing else matters. This special
      // mode of operation only points the telescope.
      //
      if ( this->target.pointmode == Acam::POINTMODE_ACAM ) {
        this->dotype( "ONE" );
        worker_threads = { { THR_MOVE_TO_TARGET, std::bind(&Sequence::move_to_target, this) } };

      }
      // For any other pointmode (SLIT, or empty, which assumes SLIT), all
      // subsystems are readied.
      //
      else {
        // set pointmode explicitly, in case it's empty
        this->target.pointmode = Acam::POINTMODE_SLIT;

        // threads to start, pair their ThreadStatusBit with the function to call
        //
        worker_threads = { { THR_MOVE_TO_TARGET, std::bind(&Sequence::move_to_target, this) },
                           { THR_CAMERA_SET,     std::bind(&Sequence::camera_set, this)     },
                           { THR_SLIT_SET,       std::bind(&Sequence::slit_set, this,
                                                                    Sequencer::VSM_ACQUIRE) },
                           { THR_FOCUS_SET,      std::bind(&Sequence::focus_set, this)      },
                           { THR_FLEXURE_SET,    std::bind(&Sequence::flexure_set, this)    },
                           { THR_CALIB_SET,      std::bind(&Sequence::calib_set, this)      }
                         };
      }

      // pair their ThreadStatusBit with their future
      std::vector<std::pair<Sequencer::ThreadStatusBits, std::future<long>>> worker_futures;

      // start all of the threads
      //
      for ( const auto &[thr, func] : worker_threads ) {
        worker_futures.emplace_back( thr, std::async(std::launch::async, func) );
      }

      // wait for the threads to complete. these can be cancelled.
      //
      for ( auto &[thr, future] : worker_futures) {
        try {
          error |= future.get(); // wait for this worker to finish
          logwrite( function, "NOTICE: worker "+Sequencer::thread_names.at(thr)+" completed");
        }
        catch (const std::exception& e) {
          logwrite( function, "ERROR: worker "+Sequencer::thread_names.at(thr)+" exception: "+std::string(e.what()) );
          return;
        }
      }

      logwrite(function, "DONE waiting on threads");

      if ( this->cancel_flag.load() ) {
        this->async.enqueue_and_log( function, "NOTICE: sequence cancelled" );
        return;
      }

      // For pointmode ACAM, there is nothing to be done so get out
      //
      if ( this->target.pointmode == Acam::POINTMODE_ACAM ) {
        this->async.enqueue_and_log( function, "NOTICE: target list processing has stopped" );
        break;
      }

/*** 12/17/24 move acquisition elsewhere?
 *
 *    logwrite( function, "starting acquisition thread" );             ///< TODO @todo log to telemetry!

 *    this->seq_state.set( Sequencer::SEQ_WAIT_ACQUIRE );
 *    this->broadcast_seqstate();
 *    std::thread( &Sequencer::Sequence::dothread_acquisition, this ).detach();
 ***/

      // If not a calibration target then introduce a pause for the user
      // to make adjustments, send offsets, etc.
      //
      if ( !this->target.iscal ) {

        // waiting for user signal (or cancel)
        //
        // The sequencer is effectively paused waiting for user input. This
        // gives the user a chance to ensure the correct target is on the slit,
        // select offset stars, etc.
        //
        {
        ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_USER );

        this->async.enqueue_and_log( function, "NOTICE: waiting for USER to send \"continue\" signal" );

        while ( !this->cancel_flag.load() && !this->is_usercontinue.load() ) {
          std::unique_lock<std::mutex> lock(cv_mutex);
          this->cv.wait( lock, [this]() { return( this->is_usercontinue.load() || this->cancel_flag.load() ); } );
        }

        this->async.enqueue_and_log( function, "NOTICE: received "
                                               +(this->cancel_flag.load() ? std::string("cancel") : std::string("continue"))
                                               +" signal!" );
        }  // end scope for wait_state = WAIT_USER

        if ( this->cancel_flag.load() ) {
          this->async.enqueue_and_log( function, "NOTICE: sequence cancelled" );
          return;
        }

        this->is_usercontinue.store(false);

        this->async.enqueue_and_log( function, "NOTICE: received USER continue signal!" );
      }

      // Ensure slit offset is in "expose" position
      //
      auto slitset = std::async(std::launch::async, &Sequence::slit_set, this, Sequencer::VSM_EXPOSE);
      try {
        error |= slitset.get();
      }
      catch (const std::exception& e) {
        logwrite( function, "ERROR slit offset exception: "+std::string(e.what()) );
        return;
      }

      logwrite( function, "starting exposure" );       ///< TODO @todo log to telemetry!

      // Start the exposure in a thread...
      // set the EXPOSE bit here, outside of the trigger_exposure function, because that
      // function only triggers the exposure -- it doesn't block waiting for the exposure.
      //
      this->wait_state_manager.set( Sequencer::SEQ_WAIT_EXPOSE );  // set EXPOSE bit
      auto start_exposure = std::async(std::launch::async, &Sequence::trigger_exposure, this);
      try {
        error |= start_exposure.get();
      }
      catch (const std::exception& e) {
        logwrite( function, "ERROR repeat_exposure exception: "+std::string(e.what()) );
        return;
      }

      // wait for the exposure to end (naturally or cancelled)
      //
      logwrite( function, "waiting for exposure" );
      while ( !this->cancel_flag.load() && wait_state_manager.is_set( Sequencer::SEQ_WAIT_EXPOSE ) ) {
        std::unique_lock<std::mutex> lock(cv_mutex);
        this->cv.wait( lock, [this]() { return( !wait_state_manager.is_set(SEQ_WAIT_EXPOSE) || this->cancel_flag.load() ); } );
      }

      // When an exposure is aborted then it will be marked as UNASSIGNED
      //
      if ( this->cancel_flag.load() ) {
        this->async.enqueue_and_log( function, "NOTICE: exposure cancelled" );
        error = this->target.update_state( Sequencer::TARGET_UNASSIGNED );
        message.str(""); message << ( error==NO_ERROR ? "" : "ERROR " ) << "marking target " << this->target.name
                                 << " id " << this->target.obsid << " order " << this->target.obsorder
                                 << " as " << Sequencer::TARGET_UNASSIGNED;
        logwrite( function, message.str() );
        return;
      }

      this->async.enqueue_and_log( function, "NOTICE: done waiting for expose" );
      message.str(""); message << "exposure complete for target " << this->target.name
                               << " id " << this->target.obsid << " order " << this->target.obsorder;
      logwrite( function, message.str() );

      // Now that we're done waiting, check for errors or abort
      //
      if ( this->thread_error_manager.are_any_set() ) {
        message.str(""); message << "ERROR stopping sequencer because the following thread(s) had an error: "
                                 << this->thread_error_manager.get_set_states();
        logwrite( function, message.str() );
        break;
      }

      // before writing to the completed database table, get current
      // telemetry from other daemons.
      //
      this->get_external_telemetry();

      // Update this target's state in the database
      //
      error = this->target.update_state( Sequencer::TARGET_COMPLETE );       // update the active target table
      if (error==NO_ERROR) error = this->target.insert_completed();          // insert into the completed table
      if (error!=NO_ERROR) this->thread_error_manager.set( THR_SEQUENCE_START );     // report any error

      // let the world know of the state change
      //
      message.str(""); message << "TARGETSTATE:" << this->target.state << " TARGET:" << this->target.name << " OBSID:" << this->target.obsid;
      this->async.enqueue( message.str() );

      // Check the "dotype" --
      // If this was "do one" then do_once is set and get out now.
      //
      if ( this->do_once.load() ) {
        logwrite( function, "stopping sequencer because single-step is selected" );
        break;
      }

    } // end while true

    if ( this->thread_error_manager.are_any_set() ) {
      logwrite( function, "requesting stop because an error was detected" );
      if ( this->target.get_next( Sequencer::TARGET_ACTIVE, targetstatus ) == TargetInfo::TARGET_FOUND ) {  // If this target was flagged as active,
        this->target.update_state( Sequencer::TARGET_UNASSIGNED );                                          // then change it to unassigned on error.
      }
      this->thread_error_manager.clear_all();     // clear the thread error state
      this->do_once.store(true);
    }

    logwrite( function, "target list processing has stopped" );
    return;
  }
  /***** Sequencer::Sequence::sequence_start **********************************/


  /***** Sequencer::Sequence::camera_set **************************************/
  /**
   * @brief      sets the camera according to the parameters in the target entry
   * @return     ERROR | NO_ERROR
   *
   * At the moment, this is only exposure time.
   *
   */
  long Sequence::camera_set() {
    const std::string function("Sequencer::Sequence::camera_set");
    std::string reply;
    std::stringstream camcmd;
    long error=NO_ERROR;

    logwrite( function, "setting camera parameters");

    ScopedState thr_state( thread_state_manager, Sequencer::THR_CAMERA_SET );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_CAMERA );

    // send the EXPTIME command to camerad
    //
    // Everywhere is maintained that exptime is specified in sec except
    // the camera takes msec, so convert just before sending the command.
    //
    long exptime_msec = (long)( this->target.exptime_req * 1000 );
    camcmd.str(""); camcmd << CAMERAD_EXPTIME << " " << exptime_msec;
    error |= this->camerad.send( camcmd.str(), reply );

    // send binning parameters
    // this is only good for I/R and will have to change to be more general
    // because not all detectors will be oriented the same!
    //
    camcmd.str(""); camcmd << CAMERAD_BIN << " row " << this->target.binspat;
    error |= this->camerad.send( camcmd.str(), reply );
    camcmd.str(""); camcmd << CAMERAD_BIN << " col " << this->target.binspect;
    error |= this->camerad.send( camcmd.str(), reply );

    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR setting camera" );
      this->thread_error_manager.set( THR_CAMERA_SET );
    }

    return error;
  }
  /***** Sequencer::Sequence::camera_set **************************************/


  /***** Sequencer::Sequence::slit_set ****************************************/
  /**
   * @brief      sets the slit width and offset
   * @details    Slit width is always set according to the value in the target
   *             database entry, offset set according to mode.
   * @param[in]  mode  selects source of slit offset
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::slit_set(VirtualSlitMode mode) {
    const std::string function("Sequencer::Sequence::slit_set");
    std::string reply, modestr;
    std::stringstream slitcmd, message;

    ScopedState thr_state( thread_state_manager, Sequencer::THR_SLIT_SET );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_SLIT );

    // send the SET command to slitd
    //
    slitcmd << SLITD_SET << " ";

    switch (mode) {
      case Sequencer::VSM_EXPOSE:
        // uses width from target database entry and virtual-mode offset for expose
        slitcmd << this->target.slitwidth_req << " " << this->slitoffsetexpose;
        modestr = "EXPOSE";
        break;
      case Sequencer::VSM_ACQUIRE:
        // uses virtual-mode width and offset for acquire
        slitcmd << this->slitwidthacquire << " " << this->slitoffsetacquire;
        modestr = "ACQUIRE";
        break;
      case Sequencer::VSM_DATABASE:
        // uses width and offset from target databsae entry
        slitcmd << this->target.slitwidth_req << " " << this->target.slitoffset_req;
        modestr = "DATABASE";
        break;
    }

    this->async.enqueue( "NOTICE: moving slit to "+modestr+" position" );

    logwrite( function, " sending: "+slitcmd.str() );

    if ( this->slitd.command_timeout( slitcmd.str(), reply, SLITD_SET_TIMEOUT ) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR setting slit" );
      this->thread_error_manager.set( THR_SLIT_SET );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::slit_set ****************************************/


  /***** Sequencer::Sequence::power_init **************************************/
  /**
   * @brief      initializes the power system for control from the Sequencer
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::power_init() {
    const std::string function("Sequencer::Sequence::power_init");

    ScopedState thr_state( thread_state_manager, Sequencer::THR_POWER_INIT );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_POWER );

    this->daemon_manager.clear( Sequencer::DAEMON_POWER );  // powerd not ready

    if ( this->reopen_hardware(this->powerd, POWERD_REOPEN, 10000 ) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR initializing power control" );
      return ERROR;
    }

    this->daemon_manager.set( Sequencer::DAEMON_POWER );  // powerd ready

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::power_init **************************************/


  /***** Sequencer::Sequence::power_shutdown **********************************/
  /**
   * @brief      disconnects from powerd
   * @details    There's nothing with powerd that needs to be shut down.
   *
   */
  long Sequence::power_shutdown() {
    const std::string function("Sequencer::Sequence::power_shutdown");

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_POWER_SHUTDOWN );
    ScopedState wait_state( this->wait_state_manager, Sequencer::SEQ_WAIT_POWER );
    ScopedState daemon_state( this->daemon_manager, Sequencer::DAEMON_POWER );

    // disconnect me from powerd
    logwrite( function, "disconnecting from powerd" );
    this->powerd.disconnect();

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::power_shutdown **********************************/


  /***** Sequencer::Sequence::slit_init ***************************************/
  /**
   * @brief      initializes the slit for control from the Sequencer
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::slit_init() {
    const std::string function("Sequencer::Sequence::slit_init");

    ScopedState thr_state( thread_state_manager, Sequencer::THR_SLIT_INIT );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_SLIT );

    this->daemon_manager.clear( Sequencer::DAEMON_SLIT );  // slitd not ready

    if ( this->set_power_switch(ON, POWER_SLIT, std::chrono::seconds(5)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR powering slit hardware" );
      return ERROR;
    }

    bool was_opened=false;
    if ( this->open_hardware(this->slitd, was_opened) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR connecting to slit" );
      return ERROR;
    }

    // Ask slitd if the slit motors are homed,
    //
    bool ishomed=false;
    std::string reply;
    if ( this->slitd.command( SLITD_ISHOME, reply ) ) {
      this->async.enqueue_and_log( function, "ERROR communicating with slit hardware" );
      return ERROR;
    }
    this->parse_state( function, reply, ishomed );

    // and send the HOME command to slitd if needed.
    //
    if ( !ishomed ) {
      logwrite( function, "sending home command" );
      if ( this->slitd.command_timeout( SLITD_HOME, reply, SLITD_HOME_TIMEOUT ) != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR communicating with slit hardware" );
        return ERROR;
      }
    }

    // send init values only if connection was just opened now
    //
    if ( was_opened && !this->config_init["SLIT"].empty() ) {
      std::string cmd = SLITD_SET+" "+this->config_init["SLIT"];
      if ( this->slitd.command_timeout( cmd, reply, SLITD_SET_TIMEOUT ) != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR sending \""+cmd+"\" to slit" );
        return ERROR;
      }
    }

    this->daemon_manager.set( Sequencer::DAEMON_SLIT );  // slitd ready

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::slit_init ***************************************/


  /***** Sequencer::Sequence::slit_shutdown ***********************************/
  /**
   * @brief      shuts down the slit system
   *
   */
  long Sequence::slit_shutdown() {
    const std::string function("Sequencer::Sequence::slit_shutdown");
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_SLIT_SHUTDOWN );
    ScopedState wait_state( this->wait_state_manager, Sequencer::SEQ_WAIT_SLIT );
    ScopedState daemon_state( this->daemon_manager, Sequencer::DAEMON_SLIT );

    // already off?
    //
    bool poweron=false;
    if ( check_power_switch(ON, POWER_SLIT, poweron ) != NO_ERROR ) {
      logwrite( function, "ERROR checking power switch" );
      return ERROR;
    }

    // if already off then get out now, don't turn them back on
    //
    if ( !poweron ) {
      logwrite( function, "slit already powered off. nothing to do." );
      return NO_ERROR;
    }

    if ( this->connect_to_daemon(this->slitd) != NO_ERROR ) return ERROR;

    // set shutdown state
    //
    if (error==NO_ERROR && !this->config_shutdown["SLIT"].empty() ) {
      std::string cmd = SLITD_SET+" "+this->config_shutdown["SLIT"];
      if ( this->slitd.command_timeout( cmd, reply, SLITD_SET_TIMEOUT ) != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR sending \""+cmd+"\" to slit" );
        return ERROR;
      }
    }

    // close connections between slitd and the hardware with which it communicates
    //
    logwrite( function, "closing slit hardware" );
    error = this->slitd.command( SLITD_CLOSE, reply );
    if ( error != NO_ERROR ) this->async.enqueue_and_log( function, "ERROR closing connection to slit hardware" );

    // disconnect me from slitd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from slitd" );
    this->slitd.disconnect();

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::slit_shutdown ***********************************/


  /***** Sequencer::Sequence::slicecam_init ***********************************/
  /**
   * @brief      initializes the slicecam system for control from the Sequencer
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::slicecam_init() {
    const std::string function("Sequencer::Sequence::slicecam_init");

    this->daemon_manager.clear( Sequencer::DAEMON_SLICECAM );  // slicecamd not ready

    ScopedState thr_state( thread_state_manager, Sequencer::THR_SLICECAM_INIT );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_SLICECAM );

    // make sure hardware is powered on
    //
    if ( this->set_power_switch(ON, POWER_SLICECAM, std::chrono::seconds(10)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR initializing slicecam control" );
      return ERROR;
    }

    // open connection is all that is needed, slicecamd takes care of everything
    //
    if ( this->open_hardware(this->slicecamd, SLICECAMD_OPEN, SLICECAMD_OPEN_TIMEOUT) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR starting slicecam" );
      return ERROR;
    }

    this->daemon_manager.set( Sequencer::DAEMON_SLICECAM );  // slicecamd ready

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::slicecam_init ***********************************/


  /***** Sequencer::Sequence::acam_init ***************************************/
  /**
   * @brief      initializes the slicecam system for control from the Sequencer
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::acam_init() {
    const std::string function("Sequencer::Sequence::acam_init");

    this->daemon_manager.clear( Sequencer::DAEMON_ACAM );  // acamd not ready

    ScopedState thr_state( thread_state_manager, Sequencer::THR_ACAM_INIT );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_ACAM );

    // make sure hardware is powered on
    //
    if ( this->set_power_switch(ON, POWER_ACAM, std::chrono::seconds(10)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR powering acam hardware" );
      return ERROR;
    }

    // open connection is all that is needed, acamd takes care of everything
    //
    bool was_opened=false;
    if ( this->open_hardware(this->acamd, ACAMD_OPEN, ACAMD_OPEN_TIMEOUT, was_opened) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR starting acam" );
      return ERROR;
    }

    // send init values only if connection was just opened now
    //
    if ( was_opened) {
      std::string cmd, reply;
      if ( ! this->config_init["ACAM_FILTER"].empty() ) {
        cmd = ACAMD_FILTER+" "+this->config_init["ACAM_FILTER"];
        if ( this->acamd.command_timeout( cmd, reply, ACAMD_MOVE_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR sending \""+cmd+"\" to acamd: "+reply );
          return ERROR;
        }
      }
      if ( ! this->config_init["ACAM_COVER"].empty() ) {
        cmd = ACAMD_COVER+" "+this->config_init["ACAM_COVER"];
        if ( this->acamd.command_timeout( cmd, reply, ACAMD_MOVE_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR sending \""+cmd+"\" to acamd: "+reply );
          return ERROR;
        }
      }
    }

    this->daemon_manager.set( Sequencer::DAEMON_ACAM );  // acamd ready

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::acam_init ***************************************/


  /***** Sequencer::Sequence::slicecam_shutdown *******************************/
  /**
   * @brief      shuts down the slicecam system
   *
   */
  long Sequence::slicecam_shutdown() {
    const std::string function("Sequencer::Sequence::slicecam_shutdown");
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_SLICECAM_SHUTDOWN );
    ScopedState wait_state( this->wait_state_manager, Sequencer::SEQ_WAIT_SLICECAM );
    ScopedState daemon_state( this->daemon_manager, Sequencer::DAEMON_SLICECAM );

    // already off?
    //
    bool poweron=false;
    if ( check_power_switch(ON, POWER_SLICECAM, poweron ) != NO_ERROR ) {
      logwrite( function, "ERROR checking power switch" );
      return ERROR;
    }

    // if already off then get out now, don't turn them back on
    //
    if ( !poweron ) {
      logwrite( function, "slicecam already powered off. nothing to do." );
      return NO_ERROR;
    }

    if ( this->connect_to_daemon(this->slicecamd) != NO_ERROR ) return ERROR;

    // close connections between slicecamd and the hardware with which it communicates
    //
    logwrite( function, "closing slicecam hardware" );
    error = this->slicecamd.command_timeout( SLICECAMD_SHUTDOWN, reply, SLICECAMD_SHUTDOWN_TIMEOUT );
    if ( error != NO_ERROR ) this->async.enqueue_and_log( function, "ERROR closing connection to slicecam hardware" );

    // disconnect me from slicecamd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from slicecamd" );
    this->slicecamd.disconnect();

    // Turn off power to slicecam hardware.
    //
    if ( this->set_power_switch(OFF, POWER_SLICECAM, std::chrono::seconds(0)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR switching off slicecam" );
      error=ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::slicecam_shutdown *******************************/


  /***** Sequencer::Sequence::acam_shutdown ***********************************/
  /**
   * @brief      shuts down the acam system
   *
   */
  long Sequence::acam_shutdown() {
    const std::string function("Sequencer::Sequence::acam_shutdown");
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_ACAM_SHUTDOWN );
    ScopedState wait_state( this->wait_state_manager, Sequencer::SEQ_WAIT_ACAM );
    ScopedState daemon_state( this->daemon_manager, Sequencer::DAEMON_ACAM );

    // ensure a connection to the daemon
    //
    error = this->connect_to_daemon(this->acamd);

    // set shutdown states
    //
    if ( error==NO_ERROR ) {
      std::string cmd, reply;
      if ( ! this->config_shutdown["ACAM_FILTER"].empty() ) {
        cmd = ACAMD_FILTER+" "+this->config_shutdown["ACAM_FILTER"];
        if ( this->acamd.command_timeout( cmd, reply, ACAMD_MOVE_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR sending \""+cmd+"\" to acamd: "+reply );
          return ERROR;
        }
      }
      if ( ! this->config_shutdown["ACAM_COVER"].empty() ) {
        cmd = ACAMD_COVER+" "+this->config_shutdown["ACAM_COVER"];
        if ( this->acamd.command_timeout( cmd, reply, ACAMD_MOVE_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR sending \""+cmd+"\" to acamd: "+reply );
          return ERROR;
        }
      }
    }

    // send ACAMD_SHUTDOWN command to acamd -- this will cause it to nicely
    // close connections between acamd and the hardware with which it communicates
    //
    if ( error==NO_ERROR ) {
      logwrite( function, "closing acam hardware" );
      error = this->acamd.command_timeout( ACAMD_SHUTDOWN, ACAMD_SHUTDOWN_TIMEOUT );
      if ( error != NO_ERROR ) this->async.enqueue_and_log( function, "ERROR shutting down acam" );
    }

    // disconnect me from acamd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from acamd" );
    this->acamd.disconnect();

    // Turn off power to acam hardware.
    // Any error here is added to thread_error_manager.
    //
    if ( this->set_power_switch(OFF, POWER_ACAM, std::chrono::seconds(0)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR switching off acam" );
      error=ERROR;
    }

    // set this thread's error status
    //
    if (error!=NO_ERROR) this->thread_error_manager.set( THR_ACAM_SHUTDOWN );

    return error;
  }
  /***** Sequencer::Sequence::acam_shutdown ***********************************/


  /***** Sequencer::Sequence::calib_init **************************************/
  /**
   * @brief      initializes the calibrator system for control from the Sequencer
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::calib_init() {
    const std::string function("Sequencer::Sequence::calib_init");

    this->daemon_manager.clear( Sequencer::DAEMON_CALIB );

    ScopedState thr_state( thread_state_manager, Sequencer::THR_CALIB_INIT );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_CALIB );

    // make sure calib hardware is powered
    if ( this->set_power_switch(ON, POWER_CALIB, std::chrono::seconds(5)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR powering focus control" );
      return ERROR;
    }

    // connect to calibd
    bool was_opened=false;
    if ( this->open_hardware(this->calibd, was_opened) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR initializing calib control" );
      this->thread_error_manager.set( THR_CALIB_INIT );
      return ERROR;
    }

    // if calibd was just opened, home if needed,
    // then set the defaults
    if ( was_opened ) {
      // check if homed
      bool ishomed=false;
      std::string reply;
      long error = this->calibd.command( CALIBD_ISHOME, reply );
      if ( error!=NO_ERROR || this->parse_state( function, reply, ishomed ) != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR communicating with calib hardware" );
        return ERROR;
      }
      // home calib actuators if not already homed
      if ( !ishomed ) {
        logwrite( function, "sending home command" );
        if ( this->calibd.command_timeout( CALIBD_HOME, reply, CALIBD_HOME_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR communicating with calib hardware" );
          return ERROR;
        }
      }
      // set init values
      // actuators won't move if already in position
      if ( !this->config_init["CALIB_COVER"].empty() && !this->config_init["CALIB_DOOR"].empty() ) {
        std::stringstream cmd;
        cmd << CALIBD_SET;
        if ( !this->config_init["CALIB_COVER"].empty() ) cmd << " cover=" << this->config_init["CALIB_COVER"];
        if ( !this->config_init["CALIB_DOOR"].empty() )  cmd << " door="  << this->config_init["CALIB_DOOR"];
        logwrite( function, "calib default: "+cmd.str() );
        if ( this->calibd.command_timeout( cmd.str(), CALIBD_SET_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR moving calib door and/or cover" );
          return ERROR;
        }
      }
    }

    // calibd is ready
    this->daemon_manager.set( Sequencer::DAEMON_CALIB );

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::calib_init **************************************/


  /***** Sequencer::Sequence::calib_shutdown **********************************/
  /**
   * @brief      shuts down the calibrator system
   *
   */
  long Sequence::calib_shutdown() {
    const std::string function("Sequencer::Sequence::calib_shutdown");
    long error=NO_ERROR;

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_CALIB_SHUTDOWN );
    ScopedState wait_state( this->wait_state_manager, Sequencer::SEQ_WAIT_CALIB );
    ScopedState daemon_state( this->daemon_manager, Sequencer::DAEMON_CALIB );

    // is calib hardware powered?
    //
    bool poweron=false;
    if ( check_power_switch(ON, POWER_CALIB, poweron ) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR checking calib power switch" );
      error=ERROR;
    }

    // ensure a connection to the daemon
    //
    error |= this->connect_to_daemon(this->calibd);

    // set shutdown values if connected and powered
    //
    if ( poweron && error==NO_ERROR ) {
      if ( !this->config_shutdown["CALIB_COVER"].empty() && !this->config_shutdown["CALIB_DOOR"].empty() ) {
        std::stringstream cmd;
        cmd << CALIBD_SET;
        if ( !this->config_shutdown["CALIB_COVER"].empty() ) cmd << " cover=" << this->config_shutdown["CALIB_COVER"];
        if ( !this->config_shutdown["CALIB_DOOR"].empty() )  cmd << " door="  << this->config_shutdown["CALIB_DOOR"];
        logwrite( function, "calib default: "+cmd.str() );
        if ( this->calibd.command_timeout( cmd.str(), CALIBD_SET_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR moving calib door and/or cover" );
          error=ERROR;
        }
      }
    }

    // close connections between calibd and the hardware with which it communicates
    //
    if ( error==NO_ERROR ) {
      std::string reply;
      logwrite( function, "closing calib hardware" );
      error = this->calibd.send( CALIBD_CLOSE, reply );
      if ( error != NO_ERROR ) this->async.enqueue_and_log( function, "ERROR closing connection to calib hardware" );
    }

    // disconnect me from calibd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from calibd" );
    this->calibd.disconnect();

    // Turn off power to calib hardware.
    //
    if ( this->set_power_switch(OFF, POWER_CALIB, std::chrono::seconds(0)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR switching off calib hardware" );
      error=ERROR;
    }

    // always turn off power to lamps
    //
    if ( this->set_power_switch(OFF, POWER_LAMP, std::chrono::seconds(5)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR powering off lamps" );
      error=ERROR;
    }

    // set this thread's error status
    //
    if (error!=NO_ERROR) this->thread_error_manager.set( THR_CALIB_SHUTDOWN );

    return error;
  }
  /***** Sequencer::Sequence::calib_shutdown **********************************/


  /***** Sequencer::Sequence::tcs_init ****************************************/
  /**
   * @brief      initializes the tcs system for control from the Sequencer
   * @details    This opens a connection to tcsd, then instructs tcsd to open
   *             a connection to the TCS. The default TCS is opened, as specified
   *             in the tcsd config file.
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::tcs_init() {
    ScopedState thr_state( thread_state_manager, Sequencer::THR_TCS_INIT );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_TCS );

    this->daemon_manager.clear( Sequencer::DAEMON_TCS );  // tcsd not ready

    if ( this->open_hardware(this->tcsd) != NO_ERROR ) {
      this->async.enqueue_and_log( "Sequencer::Sequence::tcs_init", "ERROR initializing TCS" );
      this->thread_error_manager.set( THR_TCS_INIT );
      return ERROR;
    }

    ///< @TODO Use a long 300 s timeout here until I implement a better way.
    //         This becomes the timeout between the sequencer and tcsd for
    //         all commands. Even though tcsd has a properly timed call to
    //         the TCS, the connection from seq<->tcsd could timeout prematurly.
    this->tcsd.socket.set_totime( 300000 );

    this->daemon_manager.set( Sequencer::DAEMON_TCS );    // tcsd is ready

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::tcs_init ****************************************/


  /***** Sequencer::Sequence::tcs_shutdown ************************************/
  /**
   * @brief      shuts down the tcs connection
   *
   */
  long Sequence::tcs_shutdown() {
    const std::string function("Sequencer::Sequence::tcs_shutdown");
    std::stringstream message;

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_TCS_SHUTDOWN );
    ScopedState wait_state( this->wait_state_manager, Sequencer::SEQ_WAIT_TCS );
    ScopedState daemon_state( this->daemon_manager, Sequencer::DAEMON_TCS );

    // ensure a connection to the daemon
    //
    long error = this->connect_to_daemon(this->tcsd);

    // close connections between tcsd and the TCS
    //
    if ( error==NO_ERROR ) {
      logwrite( function, "closing connection to TCS" );
      std::string reply;
      error = this->tcsd.send( TCSD_CLOSE, reply );
      if ( error != NO_ERROR ) this->async.enqueue_and_log( function, "ERROR: closing connection to TCS" );
    }

    // disconnect me from tcsd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from tcsd" );
    this->tcsd.disconnect();

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::tcs_shutdown ************************************/


  /***** Sequencer::Sequence::flexure_init ************************************/
  /**
   * @brief      initializes the flexure system for control from the Sequencer
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::flexure_init() {
    const std::string function("Sequencer::Sequence::flexure_init");

    ScopedState thr_state( thread_state_manager, Sequencer::THR_FLEXURE_INIT );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_FLEXURE );

    this->daemon_manager.clear( Sequencer::DAEMON_FLEXURE );  // flexured not ready

    // make sure hardware is powered on
    //
    if ( this->set_power_switch(ON, POWER_FLEXURE, std::chrono::seconds(21)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR powering flexure control" );
      return ERROR;
    }

    if ( this->open_hardware(this->flexured) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR initializing flexure control" );
      return ERROR;
    }

    // default actuator positions are defined in ~/Software/Config/flexured.cfg
    // and are set by flexured

    this->daemon_manager.set( Sequencer::DAEMON_FLEXURE );  // flexured ready

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::flexure_init ************************************/


  /***** Sequencer::Sequence::flexure_shutdown ********************************/
  /**
   * @brief      shuts down the flexure system
   *
   */
  long Sequence::flexure_shutdown() {
    const std::string function("Sequencer::Sequence::flexure_shutdown");
    std::string reply;
    long error=NO_ERROR;

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_FLEXURE_SHUTDOWN );
    ScopedState wait_state( this->wait_state_manager, Sequencer::SEQ_WAIT_FLEXURE );
    ScopedState daemon_state( this->daemon_manager, Sequencer::DAEMON_FLEXURE );

    // already off?
    //
    bool poweron=false;
    if ( check_power_switch(ON, POWER_FLEXURE, poweron ) != NO_ERROR ) {
      logwrite( function, "ERROR checking power switch" );
      return ERROR;
    }

    // if already off then get out now, don't turn them back on
    //
    if ( !poweron ) {
      logwrite( function, "flexure already powered off. nothing to do." );
      return NO_ERROR;
    }

    if ( this->connect_to_daemon(this->flexured) != NO_ERROR ) return ERROR;

    // close connections between flexured and the hardware with which it communicates
    //
    logwrite( function, "closing flexure hardware" );
    error = this->flexured.command( FLEXURED_CLOSE, reply );
    if ( error != NO_ERROR ) this->async.enqueue_and_log( function, "ERROR: closing connection to flexure hardware" );

    // disconnect me from flexured, irrespective of any previous error
    //
    logwrite( function, "disconnecting from flexured" );
    this->flexured.disconnect();

    // Turn off power to flexure hardware.
    //
    if ( this->set_power_switch(OFF, POWER_FLEXURE, std::chrono::seconds(0)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR switching off flexure" );
      error=ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::flexure_shutdown ********************************/


  /***** Sequencer::Sequence::focus_init **************************************/
  /**
   * @brief      initializes the focus system for control from the Sequencer
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::focus_init() {
    const std::string function("Sequencer::Sequence::focus_init");

    ScopedState thr_state( thread_state_manager, Sequencer::THR_FOCUS_INIT );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_FOCUS );

    this->daemon_manager.clear( Sequencer::DAEMON_FOCUS );  // focusd not ready

    if ( this->set_power_switch(ON, POWER_FOCUS, std::chrono::seconds(5)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR powering focus control" );
      return ERROR;
    }

    // connect to focusd
    bool was_opened=false;
    if ( this->open_hardware(this->focusd, was_opened) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR initializing focus control" );
      this->thread_error_manager.set( THR_FOCUS_INIT );
      return ERROR;
    }

    // if focusd was just opened, home if needed,
    // then set nominal positions
    if ( was_opened ) {
      // check if homed
      bool ishomed=false;
      std::string reply;
      long error = this->focusd.command( FOCUSD_ISHOME, reply );
      if ( error!=NO_ERROR || this->parse_state( function, reply, ishomed ) != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR communicating with focus hardware" );
        return ERROR;
      }
      // home focus actuators if not already homed
      if ( !ishomed ) {
        logwrite( function, "sending home command" );
        if ( this->focusd.command_timeout( FOCUSD_HOME, reply, FOCUSD_HOME_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR communicating with focus hardware" );
          return ERROR;
        }
      }
      // send actuators to nominal positions
      logwrite( function, "setting nominal positions" );
      std::vector<std::string> chans = {"I", "R"};
      for ( const auto &chan : chans ) {
        std::string command = "set " + chan + " nominal";
        if ( this->focusd.command_timeout( command, reply, FOCUSD_SET_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR setting focus "+chan );
          return ERROR;
        }
      }
    }

    this->daemon_manager.set( Sequencer::DAEMON_FOCUS );    // focusd is ready

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::focus_init **************************************/


  /***** Sequencer::Sequence::focus_shutdown **********************************/
  /**
   * @brief      shuts down the focus system
   *
   */
  long Sequence::focus_shutdown() {
    const std::string function("Sequencer::Sequence::focus_shutdown");

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_FOCUS_SHUTDOWN );
    ScopedState wait_state( this->wait_state_manager, Sequencer::SEQ_WAIT_FOCUS );
    ScopedState daemon_state( this->daemon_manager, Sequencer::DAEMON_FOCUS );

    // already off?
    //
    bool poweron=false;
    if ( check_power_switch(ON, POWER_FOCUS, poweron ) != NO_ERROR ) {
      logwrite( function, "ERROR checking power switch" );
      return ERROR;
    }

    // if already off then get out now, don't turn them back on
    //
    if ( !poweron ) {
      logwrite( function, "focus already powered off. nothing to do." );
      return NO_ERROR;
    }

    if ( this->connect_to_daemon(this->focusd) != NO_ERROR ) return ERROR;

    // close connections between focusd and the hardware with which it communicates
    //
    logwrite( function, "closing focus hardware" );
    std::string reply;
    long error = this->focusd.command( FOCUSD_CLOSE, reply );
    if ( error != NO_ERROR ) this->async.enqueue_and_log( function, "ERROR closing connection to focus hardware" );

    // disconnect me from focusd, irrespective of any previous error
    //
    logwrite( function, "disconnecting from focusd" );
    this->focusd.disconnect();

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::focus_shutdown **********************************/


  /***** Sequencer::Sequence::camera_init *************************************/
  /**
   * @brief      initializes the camera system for control from the Sequencer
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::camera_init() {
    const std::string function("Sequencer::Sequence::camera_init");

    ScopedState thr_state( thread_state_manager, Sequencer::THR_CAMERA_INIT );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_CAMERA );

    this->daemon_manager.clear( Sequencer::DAEMON_CAMERA );  // camerad not ready

    // make sure hardware is powered on
    //
    if ( this->set_power_switch(ON, POWER_CAMERA, std::chrono::seconds(5)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR powering camera" );
      return ERROR;
    }

    bool was_opened=false;
    if ( this->open_hardware(this->camerad, "open", 12000, was_opened) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR initializing camera" );
      return ERROR;
    }

    // send all of the prologue commands only if camera was just opened now
    //
    if ( was_opened) {
      std::string reply;
      for ( const auto &cmd : this->camera_prologue ) {
        if ( this->camerad.command_timeout( cmd, reply, CAMERA_PROLOG_TIMEOUT ) != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR sending \""+cmd+"\" to camera" );
          return ERROR;
        }
      }
    }

    this->daemon_manager.set( Sequencer::DAEMON_CAMERA );  // camerad ready

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::camera_init *************************************/


  /***** Sequencer::Sequence::camera_shutdown *********************************/
  /**
   * @brief      shuts down the camera system from the Sequencer
   *
   */
  long Sequence::camera_shutdown() {
    const std::string function("Sequencer::Sequence::camera_shutdown");

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_CAMERA_SHUTDOWN );
    ScopedState wait_state( this->wait_state_manager, Sequencer::SEQ_WAIT_CAMERA );
    ScopedState daemon_state( this->daemon_manager, Sequencer::DAEMON_CAMERA );

    // Are any cameras on?
    //
    bool poweron=false;
    if ( check_power_switch(ON, POWER_CAMERA, poweron ) != NO_ERROR ) {
      logwrite( function, "ERROR checking power switch" );
      return ERROR;
    }

    // if already off then get out now, don't turn them back on
    //
    if ( !poweron ) {
      logwrite( function, "cameras already powered off. nothing to do." );
      return NO_ERROR;
    }

    // cameras on, open a connection so that the epilogue commands can be sent
    //
    if ( this->open_hardware(this->camerad, CAMERAD_OPEN, CAMERAD_OPEN_TIMEOUT) != NO_ERROR ) {
      logwrite( function, "ERROR opening camera(s)" );
      return ERROR;
    }

    // send all of the epilogue commands
    //
    for ( const auto &cmd : this->camera_epilogue ) {
      this->camerad.command( cmd );
    }

    // disconnect me from camerad, irrespective of any previous error
    //
    logwrite( function, "disconnecting from camerad" );
    this->camerad.disconnect();

    // turn off power to camera hardware
    //
    if ( this->set_power_switch(OFF, POWER_CAMERA, std::chrono::seconds(5)) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR powering off camera" );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::camera_shutdown *********************************/


  /***** Sequencer::Sequence::move_to_target **********************************/
  /**
   * @brief      send request to TCS to move to target coordinates
   *
   * Disable guiding and send the new target coordinates to the TCS with
   * the instruction to move immediately.
   *
   */
  long Sequence::move_to_target() {
    const std::string function("Sequencer::Sequence::move_to_target");
    std::stringstream message;
    long error=NO_ERROR;

    ScopedState thr_state( thread_state_manager, Sequencer::THR_MOVE_TO_TARGET );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_TCS );

    // If RA and DEC fields are both empty then no telescope move
    //
    if ( this->target.ra_hms.empty() && this->target.dec_dms.empty() ) {
      logwrite( function, "no telescope move requested" );
      return NO_ERROR;
    }

    // No telescope move if target coordinates didn't change
    //
    if ( this->target.ra_hms == this->last_ra_hms &&
         this->target.dec_dms == this->last_dec_dms ) {
      this->async.enqueue_and_log( function, "NOTICE: no move required for repeat target" );
      return NO_ERROR;
    }

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
      this->thread_error_manager.set( THR_MOVE_TO_TARGET );
      return ERROR;
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

    // Send casangle using tcsd wrapper for RINGGO command
    // do not wait for reply
    //
    {
    std::stringstream ringgo_cmd;
    std::string noreply("DONTWAIT");                                               // indicates don't wait for reply
    ringgo_cmd << TCSD_RINGGO << " " << angle_out;                                 // this is calculated cass angle
    this->async.enqueue_and_log( function, "sending "+ringgo_cmd.str()+" to TCS" );
    error = this->tcsd.send( ringgo_cmd.str(), noreply );
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

    error  = this->tcsd.send( coords_cmd.str(), coords_reply );                    // send to the TCS

    // waiting for TCS Operator input (or cancel)
    {
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_TCSOP );

    // if not success then wait 1s and try again
    if ( error != NO_ERROR || coords_reply.compare( 0, strlen(TCS_SUCCESS_STR), TCS_SUCCESS_STR ) != 0 ) {
      std::this_thread::sleep_for( std::chrono::seconds(1) );
      error  = this->tcsd.send( coords_cmd.str(), coords_reply );                  // send to the TCS
      // second failure return error
      if ( error != NO_ERROR || coords_reply.compare( 0, strlen(TCS_SUCCESS_STR), TCS_SUCCESS_STR ) != 0 ) {
        message.str(""); message << "ERROR sending COORDS command. TCS reply: " << coords_reply;
        this->async.enqueue_and_log( function, message.str() );
        this->thread_error_manager.set( THR_MOVE_TO_TARGET );
        return ERROR;
      }
    }

    this->async.enqueue_and_log( function, "NOTICE: waiting for TCS operator to send \"ontarget\" signal" );

    while ( !this->cancel_flag.load() && !this->is_ontarget.load() ) {
      std::unique_lock<std::mutex> lock(cv_mutex);
      this->cv.wait( lock, [this]() { return( this->is_ontarget.load() || this->cancel_flag.load() ); } );
    }

    this->async.enqueue_and_log( function, "NOTICE: received "
                                           +(this->cancel_flag.load() ? std::string("cancel") : std::string("ontarget"))
                                           +" signal!" );
    }

    // If waiting for TCS operator was cancelled then don't continue
    //
    if ( this->cancel_flag.load() ) return NO_ERROR;

    // if ontarget (not cancelled) then acquire target
    //
    if ( !this->cancel_flag.load() ) this->acamd.command( ACAMD_ACQUIRE );

    this->is_ontarget.store(false);

    // remember the last target that was tracked on
    //
    this->last_target  = this->target.name;
    this->last_ra_hms  = this->target.ra_hms;
    this->last_dec_dms = this->target.dec_dms;

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::move_to_target **********************************/


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
    const std::string function("Sequencer::Sequence::dothread_notify_tcs");
    std::stringstream message;

    ScopedState thr_state( seq.thread_state_manager, Sequencer::THR_NOTIFY_TCS );

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
          seq.thread_error_manager.set( THR_NOTIFY_TCS );
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
    return;
  }
  /***** Sequencer::Sequence::dothread_notify_tcs *****************************/


  /***** Sequencer::Sequence::focus_set ***************************************/
  /**
   * @brief      set the focus
   * @return     ERROR | NO_ERROR
   * @todo       focus not yet implemented
   *
   */
  long Sequence::focus_set() {
    const std::string function("Sequencer::Sequence::focus_set");

    ScopedState thr_state( thread_state_manager, Sequencer::THR_FOCUS_SET );

    logwrite( function, "[TODO] focus not yet implemented." );

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::focus_set ***************************************/


  /***** Sequencer::Sequence::flexure_set *************************************/
  /**
   * @brief      set the flexure
   * @return     ERROR | NO_ERROR
   * @todo       flexure not yet implemented
   *
   */
  long Sequence::flexure_set() {
    const std::string function("Sequencer::Sequence::flexure_set");

    ScopedState thr_state( thread_state_manager, Sequencer::THR_FLEXURE_SET );

    logwrite( function, "[TODO] flexure not yet implemented." );

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::flexure_set *************************************/


  /***** Sequencer::Sequence::calib_set ***************************************/
  /**
   * @brief      set the calibrator
   * @return     ERROR | NO_ERROR
   * @todo       calibrator not yet implemented
   *
   */
  long Sequence::calib_set() {
    const std::string function("Sequencer::Sequence::calib_set");
    std::stringstream message;
    long error=NO_ERROR;

    ScopedState thr_state( thread_state_manager, Sequencer::THR_CALIBRATOR_SET );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_CALIB );

    // name will index the caltarget map
    //
    std::string name(this->target.name);

    if ( this->target.iscal ) {
      name = this->target.name;
      this->async.enqueue_and_log( function, "NOTICE: configuring calibrator for "+name );
    }
    else {
      this->async.enqueue_and_log( function, "NOTICE: disabling calibrator for science target "+name );
      name="SCIENCE";  // override for indexing the map
    }

    // Get the calibration target map.
    // This contains a map of all the required settings, indexed by target name.
    //
    auto calinfo = this->caltarget.get_info(name);
    if (!calinfo) {
      logwrite( function, "ERROR unrecognized calibration target: "+name );
      return ERROR;
    }

    // set the calib door and cover
    //
    std::stringstream cmd;
    cmd.str(""); cmd << CALIBD_SET
                     << " door="  << ( calinfo->caldoor  ? "open" : "close" )
                     << " cover=" << ( calinfo->calcover ? "open" : "close" );

    logwrite( function, "calib: "+cmd.str() );
    if ( !this->cancel_flag.load() &&
          this->calibd.command_timeout( cmd.str(), CALIBD_SET_TIMEOUT ) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR moving calib door and/or cover" );
      error=ERROR;
    }

    // set the internal calibration lamps
    //
    for ( const auto &[lamp,state] : calinfo->lamp ) {
      if ( this->cancel_flag.load() ) break;
      cmd.str(""); cmd << lamp << " " << (state?"on":"off");
      message.str(""); message << "power " << cmd.str();
      logwrite( function, message.str() );
      std::string reply;
      if ( this->powerd.send( cmd.str(), reply ) != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR "+message.str() );
        error=ERROR;
      }
    }

//  Not working yet 2025-02-04
//
//  // set the dome lamps
//  //
//  for ( const auto &[lamp,state] : calinfo->domelamp ) {
//    if ( this->cancel_flag.load() ) break;
//    cmd.str(""); cmd << TCSD_NATIVE << " NPS " << lamp << " " << (state?1:0);
//    if ( this->tcsd.command( cmd.str() ) != NO_ERROR ) {
//      this->async.enqueue_and_log( function, "ERROR "+cmd.str() );
//      error=ERROR;
//    }
//  }

    // set the lamp modulators
    //
    for ( const auto &[mod,state] : calinfo->lampmod ) {
      if ( this->cancel_flag.load() ) break;
      cmd.str(""); cmd << CALIBD_LAMPMOD << " " << mod << " " << (state?1:0) << " 1000";
      if ( this->calibd.command( cmd.str() ) != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR "+cmd.str() );
        error=ERROR;
      }
    }

    if ( this->cancel_flag.load() ) {
      this->async.enqueue_and_log( function, "NOTICE: abort may have left calib system partially set" );
    }

    return error;
  }
  /***** Sequencer::Sequence::calib_set ***************************************/


  /***** Sequencer::Sequence::abort_process *********************************/
  /**
   * @brief      tries to abort everything happening
   *
   */
  void Sequence::abort_process() {
    const std::string function("Sequencer::Sequence::abort_process");

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_ABORT_PROCESS );

    this->cancel_flag.store(false);

    // stop any exposure that may be in progress
    //
    auto stopexpose = std::async(std::launch::async, &Sequence::stop_exposure, this);
    try {
      stopexpose.get();
    }
    catch (const std::exception &e) {
      logwrite( function, "ERROR stop_exposure exception: "+std::string(e.what()) );
    }

    // set the cancel flag to stop any cancel-able tasks
    //
    this->cancel_flag.store(true);
    this->cv.notify_all();

    // drop into do-one to prevent auto increment to next target
    //
    this->do_once.store(true);

    this->async.enqueue_and_log( function, "NOTICE: cancel signal sent" );
  }

  /***** Sequencer::Sequence::stop_exposure *********************************/
  /**
   * @brief      stop an exposure in progress
   * @details    This can only stop the exposure timer, not the readout.
   *
   */
  void Sequence::stop_exposure() {
    const std::string function("Sequencer::Sequence::stop_exposure");

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_STOP_EXPOSURE );

    // This function is only used while exposing
    //
    if ( ! this->wait_state_manager.is_set( Sequencer::SEQ_WAIT_EXPOSE ) ) {
      this->async.enqueue_and_log( function, "NOTICE: not currently exposing" );
      return;
    }

    this->cancel_flag.store(false);

    // Send command to the camera to stop the exposure.
    //
    std::string reply;
//  long error = this->camerad.async( CAMERAD_STOP, reply );
    long error = this->camerad.send( CAMERAD_STOP, reply );
    if ( error == NO_ERROR ) {
      logwrite( function, "stop exposure sent to camerad" );
    }
    else
    if ( error == NOTHING ) {
      // if not exposing, this is a way to ensure WAIT_EXPOSE bit can be cleared
      this->async.enqueue_and_log( function, "NOTICE: not exposing" );
      this->wait_state_manager.clear( Sequencer::SEQ_WAIT_EXPOSE );
    }
    else
    if ( error == BUSY ) {
      this->async.enqueue_and_log( function, "NOTICE: too late to stop exposure" );
      // can't stop in the last 5 sec so wait that long and it should stop on its own
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    else {
      logwrite( function, "ERROR sending stop exposure to camerad" );
    }
    return;
  }
  /***** Sequencer::Sequence::stop_exposure *********************************/


  /***** Sequencer::Sequence::repeat_exposure *********************************/
  /**
   * @brief      repeat the last exposure
   * @todo       calibrator not yet implemented
   *
   */
  long Sequence::repeat_exposure() {
    const std::string function("Sequencer::Sequence::repeat_exposure");
    std::stringstream message;
    long error = NO_ERROR;

    // can only repeat when state is READY
    //
    if ( ! seq_state_manager.is_set( Sequencer::SEQ_READY ) ) {
      this->async.enqueue_and_log( function, "ERROR cannot repeat: system not ready" );
      return ERROR;
    }

    ScopedState thr_state( thread_state_manager, Sequencer::THR_REPEAT_EXPOSURE );
    ScopedState seq_state( seq_state_manager, Sequencer::SEQ_RUNNING, true );      // set state=RUNNING (only)
    seq_state.destruct_set( Sequencer::SEQ_READY );                                // set state=READY on exit

    // clear stop flags
    //
    this->cancel_flag.store(false);
    this->is_ontarget.store(false);
    this->is_usercontinue.store(false);

    std::string targetstatus;
    this->target.get_specified_target( this->prev_single_obsid, targetstatus );

    logwrite( function, targetstatus );

    // threads to start, pair their ThreadStatusBit with the function to call
    std::vector<std::pair<Sequencer::ThreadStatusBits, std::function<long()>>> worker_threads;

    worker_threads = { { THR_CAMERA_SET,     std::bind(&Sequence::camera_set, this)  },
//                     { THR_SLIT_SET,       std::bind(&Sequence::slit_set, this) }
                     };

    // pair their ThreadStatusBit with their future
    std::vector<std::pair<Sequencer::ThreadStatusBits, std::future<long>>> worker_futures;

    // start the threads
    for ( const auto &[thr, func] : worker_threads ) {
      worker_futures.emplace_back( thr, std::async(std::launch::async, func) );
    }

    // wait for the threads to complete. these can be cancelled.
    for ( auto &[thr, future] : worker_futures) {
      try {
        error |= future.get(); // wait for this worker to finish
        logwrite( function, "NOTICE: worker "+Sequencer::thread_names.at(thr)+" completed");
      }
      catch (const std::exception& e) {
        logwrite( function, "ERROR: worker "+Sequencer::thread_names.at(thr)+" exception: "+std::string(e.what()) );
        return ERROR;
      }
    }

    if ( this->cancel_flag.load() ) {
      this->async.enqueue_and_log( function, "NOTICE: cancelled repeat exposure" );
      return NO_ERROR;
    }

    // Start the exposure in a thread...
    //
    auto start_exposure = std::async(std::launch::async, &Sequence::trigger_exposure, this);
    try {
      error |= start_exposure.get();
    }
    catch (const std::exception& e) {
      logwrite( function, "ERROR repeat_exposure exception: "+std::string(e.what()) );
      return ERROR;
    }
    return NO_ERROR;
  }
  /***** Sequencer::Sequence::repeat_exposure *********************************/


  /***** Sequencer::Sequence::trigger_exposure ********************************/
  /**
   * @brief      trigger and wait for exposure
   * @return     ERROR | NO_ERROR
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
  long Sequence::trigger_exposure() {
    const std::string function("Sequencer::Sequence::trigger_exposure");
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;

    ScopedState thr_state( thread_state_manager, Sequencer::THR_TRIGGER_EXPOSURE );

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

    // Send the EXPOSE command to camera daemon on the non-blocking port and don't wait for reply
    message.str(""); message << CAMERAD_EXPOSE << " " << this->target.nexp;
//  if ( this->camerad.async( message.str() ) != NO_ERROR ) {
//  if ( this->camerad.send( message.str(), reply ) != NO_ERROR ) {
    if ( this->camerad.command_timeout( message.str(), reply, 12000 ) != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR sending camera "+message.str() );
      this->thread_error_manager.set( THR_TRIGGER_EXPOSURE );            // tell the world this thread had an error
      this->target.update_state( Sequencer::TARGET_PENDING );            // return the target state to pending
      this->wait_state_manager.clear( Sequencer::SEQ_WAIT_EXPOSE );      // clear EXPOSE bit
      this->arm_readout_flag = false;                                    // disarm async_listener from looking for readout
      return ERROR;
    }

    error = this->target.update_state( Sequencer::TARGET_EXPOSING );     // set EXPOSE state in database
    this->wait_state_manager.set( Sequencer::SEQ_WAIT_EXPOSE );          // set EXPOSE bit

    return error;
  }
  /***** Sequencer::Sequence::trigger_exposure ********************************/


  /***** Sequencer::Sequence::modify_exptime **********************************/
  /**
   * @brief      modify the exposure time while an exposure is running
   * @param[in]  exptime_in  requested exposure time in seconds
   *
   * Since there is no process that will need to wait on the success or failure of this
   * thread, but the success or failure might need to be known by a GUI, the results
   * will be broadcast to the asynchronous message port.
   *
   */
  void Sequence::modify_exptime( double exptime_in ) {
    const std::string function("Sequencer::Sequence::modify_exptime");
    std::stringstream message;
    std::string reply="";
    long error = NO_ERROR;
    double updated_exptime=0;

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_MODIFY_EXPTIME );

    // This function is only used while exposing
    //
    if ( ! this->wait_state_manager.is_set( Sequencer::SEQ_WAIT_EXPOSE ) ) {
      this->async.enqueue_and_log( function, "ERROR cannot update exposure time when not currently exposing" );
      error = ERROR;
    }

    // Send command to the camera to modify the exposure time.
    // The camera works in msec to convert exptime_in here.
    //
    std::stringstream cmd;
    cmd << CAMERAD_MODEXPTIME << " " << (long)(1000*exptime_in);
//  if ( error==NO_ERROR ) error = this->camerad.async( cmd.str(), reply );
    if ( error==NO_ERROR ) error = this->camerad.send( cmd.str(), reply );

    // Reply from camera will contain DONE or ERROR
    //
    std::string::size_type pos = reply.find( "DONE" );
    if ( error==NO_ERROR && pos != std::string::npos ) {
      updated_exptime = (double)( std::stol( reply.substr( 0, pos ) ) / 1000. );
    }
    else error = ERROR;

    if ( error==NO_ERROR ) {
      this->target.exptime_req = updated_exptime;
      message.str(""); message << "NOTICE: updated exptime to " << updated_exptime << " sec";
      this->async.enqueue_and_log( function, message.str() );
    }

    // announce the success or failure in an asynchronous broadcast message
    //
    message.str(""); message << "MODIFY_EXPTIME: " << this->target.exptime_req << ( error==NO_ERROR ? " DONE" : " ERROR" );
    this->async.enqueue( message.str() );

    return;
  }
  /***** Sequencer::Sequence::modify_exptime **********************************/


  /***** Sequencer::Sequence::dothread_acquisition ****************************/
  /**
   * @brief      performs the acqusition sequence
   * @details    this gets called by the move_to_target thread
   *
   * This function is spawned in a thread.
   *
   */
  void Sequence::dothread_acquisition() {
    const std::string function("Sequencer::Sequence::dothread_acquisition");
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    long error = NO_ERROR;

    ScopedState thr_state( thread_state_manager, Sequencer::THR_ACQUISITION );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_ACQUIRE );

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
      this->thread_error_manager.set( THR_MOVE_TO_TARGET );
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
      this->thread_error_manager.set( THR_ACQUISITION );               // report error
      message.str(""); message << "ERROR acquiring target";
      this->async.enqueue_and_log( function, message.str() );
      this->seq_state.clear( Sequencer::SEQ_WAIT_ACQUIRE );            // clear ACQUIRE bit
      this->broadcast_seqstate();
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
      this->thread_error_manager.set( THR_ACQUISITION );               // report any error
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
      this->thread_error_manager.set( THR_ACQUISITION );
      message.str(""); message << "ERROR failed to acquire within timeout";
    }
    else
    if ( error!=NO_ERROR ) {                                         // Error polling
      this->thread_error_manager.set( THR_ACQUISITION );
      message.str(""); message << "ERROR acquiring target";
    }
    else {                                                           // Success
      message.str(""); message << "NOTICE: target " << ( this->target.acquired ? "acquired" : "not acquired" );
    }

    this->async.enqueue_and_log( function, message.str() );            // log message
*****/

  }
  /***** Sequencer::Sequence::dothread_acquisition ****************************/


  /***** Sequencer::Sequence::startup *****************************************/
  /**
   * @brief      performs nightly startup
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::startup() {
    const std::string function("Sequencer::Sequence::startup");
    std::stringstream message;
    long error=NO_ERROR;

    if ( ! seq_state_manager.are_any_set( Sequencer::SEQ_READY, Sequencer::SEQ_NOTREADY ) ) {
      message << "ERROR cannot perform system startup while "
              << seq_state_manager.get_set_states();
      this->async.enqueue_and_log( function, message.str() );
      return ERROR;
    }

    ScopedState thread_state( thread_state_manager, Sequencer::THR_STARTUP );   // this thread is running

    // set only STARTING (and clear everything else)
    ScopedState seq_state( seq_state_manager, Sequencer::SEQ_STARTING, true );  // state=STARTING (only)

    this->thread_error_manager.clear_all();                                     // clear the thread error state

    // clear stop flags
    //
    this->cancel_flag.store(false);
    this->is_ontarget.store(false);
    this->is_usercontinue.store(false);

    // Everything (except TCS) needs the power control to be running 
    // so initialize the power control first.
    //
    auto start_power = std::async(std::launch::async, &Sequence::power_init, this);
    error = start_power.get();

    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR starting power control. Will try to continue (but don't hold your breath)" );
    }

    // threads to start, pair their ThreadStatusBit with the function to call
    //
    std::vector<std::pair<Sequencer::ThreadStatusBits, std::function<long()>>> worker_threads = {
      { THR_CALIB_INIT,   std::bind(&Sequence::calib_init, this)   },
      { THR_CAMERA_INIT,  std::bind(&Sequence::camera_init, this)  },
      { THR_FLEXURE_INIT, std::bind(&Sequence::flexure_init, this) },
      { THR_FOCUS_INIT,   std::bind(&Sequence::focus_init, this)   },
      { THR_SLIT_INIT,    std::bind(&Sequence::slit_init, this)    },
      { THR_TCS_INIT,     std::bind(&Sequence::tcs_init, this)     }
    };

    std::vector<std::pair<Sequencer::ThreadStatusBits, std::future<long>>> worker_futures;

    // launch all of the worker threads listed in the vector
    //
    for ( const auto &[thr, func] : worker_threads ) {
      worker_futures.emplace_back( thr, std::async(std::launch::async, func) );
    }

    // get() will block, waiting for the threads to complete
    //
    for ( auto &[thr, future] : worker_futures) {
      try {
        // wait for this worker to finish
        if ( future.get() != NO_ERROR ) {
          logwrite( function, "ERROR from "+Sequencer::thread_names.at(thr));
          error = ERROR;
        }
      }
      catch (const std::exception& e) {
        logwrite( function, "ERROR worker "+Sequencer::thread_names.at(thr)+" exception: "+std::string(e.what()) );
        error = ERROR;
        break;
      }
    }

    // Now the Andor cameras must be done individually, first slicecam,
    try {

      // if slicecam_init returns error, it's worth power cycling and trying again
      if ( std::async(std::launch::async, &Sequence::slicecam_init, this).get() != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR from slicecam_init, will try power cycle, standby" );

        // power off slicecams
        if ( std::async(std::launch::async, &Sequence::slicecam_shutdown, this).get() != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR shutting down slicecam, giving up" );
          error = ERROR;
        }
        else
        // try again to initialize slicecams
        if ( std::async(std::launch::async, &Sequence::slicecam_init, this).get() != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR starting slicecam, giving up" );
          error = ERROR;
        }
        else
        this->async.enqueue_and_log( function, "NOTICE: slicecam power cycle success" );
      }
    }
    catch (const std::exception& e) {
      logwrite( function, "ERROR slicecam_init exception: "+std::string(e.what()) );
      error = ERROR;
    }

    // then the acam
    try {
      // if acam_init returns error, it's worth power cycling and trying again
      if ( std::async(std::launch::async, &Sequence::acam_init, this).get() != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR from acam_init, will try power cycle, standby" );

        // power off acam
        if ( std::async(std::launch::async, &Sequence::acam_shutdown, this).get() != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR shutting down acam, giving up" );
          error = ERROR;
        }
        else
        // try again to initialize acams
        if ( std::async(std::launch::async, &Sequence::acam_init, this).get() != NO_ERROR ) {
          this->async.enqueue_and_log( function, "ERROR starting acam, giving up" );
          error = ERROR;
        }
        else
        this->async.enqueue_and_log( function, "NOTICE: acam power cycle success" );
      }
    }
    catch (const std::exception& e) {
      logwrite( function, "ERROR acam_init exception: "+std::string(e.what()) );
      error = ERROR;
    }

    // change state to READY if all daemons ready w/o error
    if ( error==NO_ERROR && daemon_manager.are_all_set() ) {
      seq_state_manager.set_only( {Sequencer::SEQ_READY} );
    }
    else {
      seq_state_manager.set_only( {Sequencer::SEQ_NOTREADY} );
    }

    return error;
  }
  /***** Sequencer::Sequence::startup *****************************************/


  /***** Sequencer::Sequence::shutdown ****************************************/
  /**
   * @brief      performs nightly shutdown
   * @return     ERROR or NO_ERROR
   * @details    The shutdown sequence puts hardware into safe conditions,
   *             closes connections, and turns off power.
   *
   */
  long Sequence::shutdown() {
    const std::string function("Sequencer::Sequence::shutdown");
    long error=ERROR;

    ScopedState thr_state( this->thread_state_manager, Sequencer::THR_SHUTDOWN );  // this thread is running

    // set only STOPPING (and clear everything else)
    ScopedState seq_state( seq_state_manager, Sequencer::SEQ_STOPPING, true );     // state=STOPPING (only)

    seq_state.destruct_set( Sequencer::SEQ_NOTREADY );                             // set state=NOTREADY on exit

    // stop everything
    //
    this->abort_process();

    // clear stop flags
    //
    this->cancel_flag.store(false);
    this->is_ontarget.store(false);
    this->is_usercontinue.store(false);

    // clear the thread error state
    //
    this->thread_error_manager.clear_all();

    // Everything (except TCS) needs the power control to be running 
    // so make sure power control is initialized before continuing.
    //
    auto start_power = std::async(std::launch::async, &Sequence::power_init, this);
    if ( start_power.get() != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR from power control. Will try to continue (but don't hold your breath)" );
    }

    // container of shutdown threads to launch,
    // pair their ThreadStatusBit with the function to call
    //
    std::vector<std::pair<Sequencer::ThreadStatusBits, std::function<long()>>> worker_threads = {
      { THR_ACAM_SHUTDOWN,     std::bind(&Sequence::acam_shutdown, this)     },
      { THR_CALIB_SHUTDOWN,    std::bind(&Sequence::calib_shutdown, this)    },
      { THR_CAMERA_SHUTDOWN,   std::bind(&Sequence::camera_shutdown, this)   },
      { THR_FLEXURE_SHUTDOWN,  std::bind(&Sequence::flexure_shutdown, this)  },
      { THR_FOCUS_SHUTDOWN,    std::bind(&Sequence::focus_shutdown, this)    },
      { THR_SLICECAM_SHUTDOWN, std::bind(&Sequence::slit_shutdown, this)     },
      { THR_SLIT_SHUTDOWN,     std::bind(&Sequence::slicecam_shutdown, this) },
      { THR_TCS_SHUTDOWN,      std::bind(&Sequence::tcs_shutdown, this)      }
    };

    std::vector<std::pair<Sequencer::ThreadStatusBits, std::future<long>>> worker_futures;

    // launch the shutdown threads
    //
    for ( const auto &[thr, func] : worker_threads ) {
      worker_futures.emplace_back( thr, std::async(std::launch::async, func) );
    }

    // wait for the threads to complete
    //
    for ( auto &[thr, future] : worker_futures) {
      try {
        error=future.get(); // wait for this worker to finish
        logwrite( function, "NOTICE: worker "+Sequencer::thread_names.at(thr)+" completed");
      }
      catch (const std::exception& e) {
        logwrite( function, "ERROR: worker "+Sequencer::thread_names.at(thr)+" exception: "+std::string(e.what()) );
        error=ERROR;
      }
    }

    std::stringstream message;
    if (error==NO_ERROR) {
      message << "NOTICE: instrument is shut down";
    }
    else {
      message << "ERROR occurred during shutdown and may not have completed";
    }

    this->async.enqueue_and_log( function, message.str() );

    return error;
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
    const std::string function("Sequencer::Sequence::parse_state");
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
    const std::string function("Sequencer::Sequence::extract_tcs_value");
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
    const std::string function("Sequencer::Sequence::parse_tcs_generic");
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
    const std::string function("Sequencer::Sequence::dotype");
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
    const std::string function("Sequencer::Sequence::dotype");
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
    const std::string function("Sequencer::Sequence::get_dome_position");
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
    const std::string function("Sequencer::Sequence::get_tcs_motion");
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
    const std::string function("Sequencer::Sequence::get_tcs_coords");
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
    const std::string function("Sequencer::Sequencer::get_tcs_cass");
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


  /***** Sequencer::Sequence::target_offset ***********************************/
  /**
   * @brief      performs target offset
   * @return     ERROR or NO_ERROR
   *
   */
  long Sequence::target_offset() {
    const std::string function("Sequencer::Sequence::target_offset");
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
    if ( this->seq_state_manager.are_any_set( Sequencer::SEQ_READY, Sequencer::SEQ_RUNNING ) ) {
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
    const std::string function("Sequencer::Sequence::handle_json_message");
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
    const std::string function("Sequencer::Sequence::dothread_fpoffset");
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


  long Sequence::check_power_switch( PowerState reqstate, const std::string which, bool &is_set ) { 
    const std::string reqstatestr = ( reqstate==ON ? "1" : "0" );

    for ( const auto &plug : this->power_switch[which].plugname ) {
      std::string reply;
      if ( this->powerd.send( plug, reply ) == NO_ERROR ) {
        if ( reply.find(reqstatestr) != std::string::npos ) {
          is_set=true;
          break;
        }
      }
      else return ERROR;
    }
    return NO_ERROR;
  }

  long Sequence::set_power_switch( PowerState reqstate, const std::string which, std::chrono::seconds delay ) { 
    const std::string function("Sequencer::Sequence::set_power_switch");
    long error=NO_ERROR;
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

      std::string reqstatestr = ( reqstate==ON ? "ON" : "OFF" );

      if ( state != reqstate ) {
        cmd << " " << reqstatestr;
        logwrite( function, "switching plug "+plug+" "+reqstatestr );
        error = this->powerd.send( cmd.str(), reply );
        if ( error != NO_ERROR || reply.find(" DONE") != std::string::npos ) {
          logwrite( function, "ERROR switching plug: "+plug+" "+reqstatestr );
          continue;
        }
        // if anything was turned on then this will add a power-on delay
        // at the end of the group
        need_delay=true;
        // switch all plugs with a 1s delay, for niceness
        std::this_thread::sleep_for( std::chrono::seconds(1) );
      }
      else
      if ( state==reqstate ) {
        logwrite( function, "plug "+plug+" already "+reqstatestr );
        continue;
      }
      else {
        logwrite( function, "ERROR bad reply \""+reply+"\" from plug: "+plug );
        continue;
      }
    }

    // some hardware groups should be powered on for a period of time before use
    if ( need_delay ) {
      logwrite( function, "waiting "+std::to_string(delay.count())+"s for "+which );
      std::this_thread::sleep_for(delay);
    }

    return error;
  }


  long Sequence::open_hardware( Common::DaemonClient &daemon ) {
    bool dontcare;
    return open_hardware( daemon, "open", 6000, dontcare, false );
  }

  long Sequence::open_hardware( Common::DaemonClient &daemon, bool &was_opened ) {
    return open_hardware( daemon, "open", 6000, was_opened, false );
  }

  long Sequence::open_hardware( Common::DaemonClient &daemon,
                                const std::string opencmd, const int opentimeout ) {
    bool dontcare;
    return open_hardware( daemon, opencmd, opentimeout, dontcare, false );
  }

  long Sequence::open_hardware( Common::DaemonClient &daemon,
                                const std::string opencmd, const int opentimeout,
                                bool &was_opened ) {
    return open_hardware( daemon, opencmd, opentimeout, was_opened, false );
  }

  long Sequence::reopen_hardware( Common::DaemonClient &daemon,
                                  const std::string opencmd, const int opentimeout ) {
    bool dontcare;
    return open_hardware( daemon, opencmd, opentimeout, dontcare, true );
  }


  /***** Sequencer::Sequence::open_hardware ***********************************/
  /**
   * @brief      open connection to hardware managed by named daemon
   * @details    If neccessary, a connection to the daemon is first established.
   *             No actions are taken if already open.
   * @param[in]  daemon       daemon client object
   * @param[in]  opencmd      open command to send to daemon
   * @param[in]  opentimeout  timeout in msec for opencmd
   * @param[out] was_opened   reference to return if this function had to open
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::open_hardware( Common::DaemonClient &daemon,
                                const std::string opencmd, const int opentimeout,
                                bool &was_opened, bool forceopen ) {
    const std::string function("Sequencer::Sequence::open_hardware");
    bool isopen=false;
    std::string reply;
    long error;

    // if not connected to the daemon then connect
    //
    if ( this->connect_to_daemon(daemon)==ERROR ) return ERROR;

    // Ask if hardware connection is open
    //
    error  = daemon.send( "isopen", reply );
    error |= this->parse_state( function, reply, isopen );
    if ( error != NO_ERROR ) {
      this->async.enqueue_and_log( function, "ERROR opening "+daemon.name+" hardware" );
      return ERROR;
    }

    // and open it if necessary.
    //
    if ( forceopen || !isopen ) {
      logwrite( function, "opening "+daemon.name+" hardware connections with "
                          +std::to_string(opentimeout)+" ms timeout" );
      if ( daemon.command_timeout( opencmd, reply, opentimeout ) != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR opening connection to "+daemon.name+" hardware" );
        return ERROR;
      }
      was_opened=true;
      logwrite( function, "opened "+daemon.name+" hardware" );
    }
    else logwrite( function, daemon.name+" hardware was already open" );

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::open_hardware ***********************************/


  /***** Sequencer::Sequence::connect_to_daemon *******************************/
  /**
   * @brief      open connection named daemon if not already open
   * @details    This allows the sequencer to communicate with the daemon.
   * @param[in]  daemon  daemon client object
   * @return     ERROR | NO_ERROR
   *
   */
  long Sequence::connect_to_daemon( Common::DaemonClient &daemon ) {
    const std::string function("Sequencer::Sequence::connect_to_daemon");

    // if not connected to the daemon then connect
    //
    if ( !daemon.socket.isconnected() ) {
      logwrite( function, "connecting to "+daemon.name+" daemon" );
      if ( daemon.connect() != NO_ERROR ) {
        this->async.enqueue_and_log( function, "ERROR connecting to "+daemon.name );
        return ERROR;
      }
    }
    logwrite( function, "connected to "+daemon.name );

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::connect_to_daemon *******************************/


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
    const std::string function("Sequencer::Sequence::test");
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
      retstring.append( "   calibset [ ? ]\n" );
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
      retstring.append( "   set [ ? ]\n" );
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
    // isready -- show the daemon_manager word
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
      message.str(""); message << "NOTICE: daemons ready: " << this->daemon_manager.get_set_states();
      this->async.enqueue_and_log( function, message.str() );
      retstring.append( message.str() ); retstring.append( "\n" );

      message.str(""); message << "NOTICE: daemons not ready: " << this->daemon_manager.get_cleared_states();
      this->async.enqueue_and_log( function, message.str() );
      retstring.append( message.str() );

      error = NO_ERROR;
    }
    else

    // ----------------------------------------------------
    // calibset -- sets the calibrator according to the parameters in the target entry
    // ----------------------------------------------------
    //
    if ( testname == "calibset" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = "test calibset\n";
        retstring.append( "  Set only the calib according to the parameters in the target row.\n" );
        return HELP;
      }
      // launch thread and wait for it to return
      auto calibset = std::async(std::launch::async, &Sequence::calib_set, this);
      try {
        error = calibset.get();
      }
      catch (const std::exception& e) {
        logwrite( function, "ERROR calib_set exception: "+std::string(e.what()) );
        return ERROR;
      }
      return error;
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
      // launch thread and wait for it to return
      auto cameraset = std::async(std::launch::async, &Sequence::camera_set, this);
      try {
        error = cameraset.get();
      }
      catch (const std::exception& e) {
        logwrite( function, "ERROR camera_set exception: "+std::string(e.what()) );
        return ERROR;
      }
      return error;
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
      std::thread( &Sequencer::Sequence::trigger_exposure, this ).detach();  // trigger exposure in a thread
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

      message.str(""); message << "STATES: " << this->seq_state_manager.get_set_states();
      this->async.enqueue( message.str() );
      logwrite( function, message.str() );

      message.str(""); message << "THREADS: " << this->thread_state_manager.get_set_states();
      logwrite( function, message.str() );

      message.str(""); message << "DAEMONS NOT READY: " << this->daemon_manager.get_cleared_states();
      logwrite( function, message.str() );
      message.str(""); message << "    DAEMONS READY: " << this->daemon_manager.get_set_states();
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
          << this->target.slitwidth_req << "  "
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

      // clear stop flags
      //
      this->cancel_flag.store(false);
      this->is_ontarget.store(false);
      this->is_usercontinue.store(false);

      logwrite( function, "spawning move_to_target..." );
      std::thread( &Sequencer::Sequence::move_to_target, this ).detach();
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
    // slitset -- spawn slit_set with the selected mode
    // ---------------------------------------------------------
    //
    if ( testname == "slitset" ) {

      if ( tokens.size() > 1 && ( tokens[1] == "?" || tokens[1] == "help" || tokens[1] == "-h" ) ) {
        retstring = "test slitset <mode>\n";
        retstring.append( "  Spawn slit_set for <mode> = { expose, acquire, database }\n" );
        return HELP;
      }

      if ( tokens.size() < 2 ) {
        logwrite( function, "ERROR expected slitset <mode>" );
        retstring="invalid_argument";
        return ERROR;
      }

      Sequencer::VirtualSlitMode mode;

      if ( tokens[1]=="expose" )   { mode = Sequencer::VSM_EXPOSE; }
      else
      if ( tokens[1]=="acquire" )  { mode = Sequencer::VSM_ACQUIRE; }
      else
      if ( tokens[1]=="database" ) { mode = Sequencer::VSM_DATABASE; }
      else {
        logwrite( function, "ERROR invalid mode "+tokens[1]+": expected { expose acquire database }" );
        retstring="invalid_argument";
        return ERROR;
      }

      auto slitset = std::async(std::launch::async, &Sequence::slit_set, this, mode);
      try {
        error = slitset.get();
      }
      catch (const std::exception& e) {
        retstring="slit_set_exception";
        logwrite( function, "ERROR slit set exception: "+std::string(e.what()) );
        return ERROR;
      }
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

      // startup or shutdown?
      //
      bool isinit = true;
      if ( testname == "startup" ) isinit=true; else if ( testname == "shutdown" ) isinit=false;

      if ( tokens[1] == "power" ) {
        std::thread( &Sequencer::Sequence::power_init, this ).detach();
      }
      else
      if ( tokens[1] == "acam" ) {
        std::thread( isinit ? &Sequencer::Sequence::acam_init : &Sequencer::Sequence::acam_shutdown, this ).detach();
      }
      else
      if ( tokens[1] == "calib" ) {
        std::thread( isinit ? &Sequencer::Sequence::calib_init : &Sequencer::Sequence::calib_shutdown, this ).detach();
      }
      else
      if ( tokens[1] == "camera" ) {
        std::thread( isinit ? &Sequencer::Sequence::camera_init : &Sequencer::Sequence::camera_shutdown, this ).detach();
      }
      else
      if ( tokens[1] == "flexure" ) {
        std::thread( isinit ? &Sequencer::Sequence::flexure_init : &Sequencer::Sequence::flexure_shutdown, this ).detach();
      }
      else
      if ( tokens[1] == "focus" ) {
        std::thread( isinit ? &Sequencer::Sequence::focus_init : &Sequencer::Sequence::focus_shutdown, this ).detach();
      }
      else
      if ( tokens[1] == "slicecam" ) {
        std::thread( isinit ? &Sequencer::Sequence::slicecam_init : &Sequencer::Sequence::slicecam_shutdown, this ).detach();
      }
      else
      if ( tokens[1] == "slit" ) {
        std::thread( isinit ? &Sequencer::Sequence::slit_init : &Sequencer::Sequence::slit_shutdown, this ).detach();
      }
      else
      if ( tokens[1] == "tcs" ) {
        std::thread( isinit ? &Sequencer::Sequence::tcs_init : &Sequencer::Sequence::tcs_shutdown, this ).detach();
      }
      else {
        logwrite( function, "ERROR invalid module \""+tokens[1]+"\"" );
        retstring="invalid_argument";
        return ERROR;
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
