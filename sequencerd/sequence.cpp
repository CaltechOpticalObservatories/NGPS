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
    this->reqstate.store( Sequencer::SEQ_STOPPED );
    this->waiting_for_state.store( false );          /// not currently waiting for a state
    this->ready_to_start = false;                    /// the sequencer is not ready by default (needs nightly startup)
    this->notify_tcs_next_target = false;            /// default do not notify TCS of next target before end of exposure
    this->tcs_preauth_time = 0;                      /// default disable notifing TCS of next target's coords before exposure end
    this->tcs_settle_timeout = 0;                    /// telescope settling timeout (set by config file)
    this->last_target_name = "";                     /// default no previous target

    // Initializes the STL map of bit-number-to-string for the SequenceStateBits.
    // This map is used to obtain a human-friendly string of the bits which have been set in seqstate
    //
    this->sequence_state_bits.push_back( STOPPED_BIT ); this->sequence_states[ STOPPED_BIT  ] = "STOPPED";
    this->sequence_state_bits.push_back( ABORTING_BIT );this->sequence_states[ ABORTING_BIT ] = "ABORTING";
    this->sequence_state_bits.push_back( STOPREQ_BIT ); this->sequence_states[ STOPREQ_BIT  ] = "STOPREQ";
    this->sequence_state_bits.push_back( STARTING_BIT); this->sequence_states[ STARTING_BIT ] = "STARTING";
    this->sequence_state_bits.push_back( READY_BIT );   this->sequence_states[ READY_BIT    ] = "READY";
    this->sequence_state_bits.push_back( RUNNING_BIT ); this->sequence_states[ RUNNING_BIT  ] = "RUNNING";
    this->sequence_state_bits.push_back( CALIB_BIT );   this->sequence_states[ CALIB_BIT    ] = "CALIB";
    this->sequence_state_bits.push_back( CAMERA_BIT );  this->sequence_states[ CAMERA_BIT   ] = "CAMERA";
    this->sequence_state_bits.push_back( FILTER_BIT );  this->sequence_states[ FILTER_BIT   ] = "FILTER";
    this->sequence_state_bits.push_back( FLEXURE_BIT ); this->sequence_states[ FLEXURE_BIT  ] = "FLEXURE";
    this->sequence_state_bits.push_back( FOCUS_BIT );   this->sequence_states[ FOCUS_BIT    ] = "FOCUS";
    this->sequence_state_bits.push_back( POWER_BIT );   this->sequence_states[ POWER_BIT    ] = "POWER";
    this->sequence_state_bits.push_back( SLIT_BIT );    this->sequence_states[ SLIT_BIT     ] = "SLIT";
    this->sequence_state_bits.push_back( TCS_BIT );     this->sequence_states[ TCS_BIT      ] = "TCS";

    // Initializes the STL map of bit-number-to-string for the ThreadStatusBits.
    // This map is used to obtain a human-friendly string of which threads are running.
    //
    this->thread_state_bits.push_back( ASYNCLISTENER_BIT ); this->thread_states[ ASYNCLISTENER_BIT ] = "sequencer_async_listener";
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
    this->thread_state_bits.push_back( CALIBINIT_BIT );     this->thread_states[ CALIBINIT_BIT ]     = "calib_init";
    this->thread_state_bits.push_back( TCSINIT_BIT );       this->thread_states[ TCSINIT_BIT ]       = "tcs_init";
    this->thread_state_bits.push_back( SLITINIT_BIT );      this->thread_states[ SLITINIT_BIT ]      = "slit_init";
    this->thread_state_bits.push_back( CAMERAINIT_BIT );    this->thread_states[ CAMERAINIT_BIT ]    = "camera_init";
    this->thread_state_bits.push_back( FLEXUREINIT_BIT );   this->thread_states[ FLEXUREINIT_BIT ]   = "flexure_init";
    this->thread_state_bits.push_back( FOCUSINIT_BIT );     this->thread_states[ FOCUSINIT_BIT ]     = "focus_init";
    this->thread_state_bits.push_back( POWERINIT_BIT );     this->thread_states[ POWERINIT_BIT ]     = "power_init";
  }
  /**************** Sequencer::Sequence ***************************************/


  /**************** Sequencer::Sequence::set_seqstate_bit *********************/
  /**
   * @fn         set_seqstate_bit
   * @brief      atomically sets the requested bit in the seqstate word
   * @param[in]  mb, masked bit is uint32 word containing the bit to set
   * @return     none
   *
   */
  void Sequence::set_seqstate_bit( uint32_t mb ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::set_seqstate_bit";
    std::stringstream message;
    std::uint32_t oldstate = this->seqstate.load();
    std::uint32_t newstate =  oldstate | mb;
    message.str(""); message << "[DEBUG] state changed from " << oldstate << " to " << newstate
                             << ": " << this->seqstate_string( newstate );
    logwrite( function, message.str() );
#endif
    this->seqstate.fetch_or( mb );
  }
  /**************** Sequencer::Sequence::set_seqstate_bit *********************/


  /**************** Sequencer::Sequence::clr_seqstate_bit *********************/
  /**
   * @fn         clr_seqstate_bit
   * @brief      atomically clears the requested bit in the seqstate word
   * @param[in]  mb, masked bit is uint32 word containing the bit to clear
   * @return     none
   *
   */
  void Sequence::clr_seqstate_bit( uint32_t mb ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::clr_seqstate_bit";
    std::stringstream message;
    std::uint32_t oldstate = this->seqstate.load();
    std::uint32_t newstate = oldstate & ~mb;
    message.str(""); message << "[DEBUG] state changed from " << oldstate << " to " << newstate
                             << ": " << this->seqstate_string( newstate );
    logwrite( function, message.str() );
#endif
    this->seqstate.fetch_and( ~mb );
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
    message.str(""); message << "[DEBUG] seqstate is " << this->seqstate.load()
                             << ": " << this->seqstate_string( this->seqstate.load() );
    logwrite( function, message.str() );
#endif
    return( this->seqstate.load() );
  }
  /**************** Sequencer::Sequence::get_seqstate *************************/


  /**************** Sequencer::Sequence::set_reqstate_bit *********************/
  /**
   * @fn         set_reqstate_bit
   * @brief      atomically sets the requested bit in the reqstate word
   * @param[in]  mb, masked bit is uint32 word containing the bit to set
   * @return     none
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
  /**************** Sequencer::Sequence::set_reqstate_bit *********************/


  /**************** Sequencer::Sequence::clr_reqstate_bit *********************/
  /**
   * @fn         clr_reqstate_bit
   * @brief      atomically clears the requested bit in the reqstate word
   * @param[in]  mb, masked bit is uint32 word containing the bit to clear
   * @return     none
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
  /**************** Sequencer::Sequence::clr_reqstate_bit *********************/


  /**************** Sequencer::Sequence::get_reqstate *************************/
  /**
   * @fn         get_reqstate
   * @brief      atomically returns the reqstate word
   * @param[in]  none
   * @return     get_rate
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
  /**************** Sequencer::Sequence::get_reqstate *************************/


  /**************** Sequencer::Sequence::dothread_sequence_async_listener *****/
  /**
   * @fn         dothread_sequence_async_listener
   * @brief      async message listening thread
   * @param[in]  ref to Sequencer::Sequence object
   * @param[in]  UDP socket object, pre-configured
   * @return     none
   *
   * This thread allows the sequencer to listen for asynchronous messages.
   * The UDP socket object that is passed in must have already been configured
   * with the async port and group, which would have been read from a config
   * file by the sequencer daemon.
   *
   * This thread never terminates unless there is an error.
   *
   */
  void Sequence::dothread_sequence_async_listener( Sequencer::Sequence &seq, Network::UdpSocket udp ) {
    seq.set_thrstate_bit( THR_SEQUENCE_ASYNC_LISTENER );
    std::string function = "Sequencer::Sequence::dothread_sequence_async_listener";
    std::stringstream message;

    int retval = udp.Listener();

    if ( retval < 0 ) {
      logwrite(function, "error creating UDP listening socket. thread terminating.");
      seq.clr_thrstate_bit( THR_SEQUENCE_ASYNC_LISTENER );
      return;
    }

    logwrite( function, "running" );

    // forever receive and process UDP async status messages
    //
    while ( true ) {

      if ( not seq.is_seqstate_set( Sequencer::SEQ_RUNNING ) ) continue;  // don't check anything if system is idle

      // Receive the UDP mesage
      //
      std::string statstr="";
      udp.Receive( statstr );

      // Act on the mesage
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
        if ( seq.notify_tcs_next_target && statstr.compare( 0, 11, "ELAPSEDTIME" ) == 0 ) {
          std::string::size_type pos = statstr.find( ":" );
          std::string elapsedstr = statstr.substr( pos + 1 );
          double remaining = (double)( seq.target.exptime - stol( elapsedstr )/1000. );
          if ( remaining <= seq.tcs_preauth_time ) {
            std::thread( dothread_notify_tcs, std::ref(seq) ).detach();
            seq.notify_tcs_next_target = false;  // don't do it again until the next exposure
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
    seq.clr_thrstate_bit( THR_SEQUENCE_ASYNC_LISTENER );
    return;
  }
  /**************** Sequencer::Sequence::dothread_sequence_async_listener *****/


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
   * This thread will run as long as the seqstate is RUNNING
   *
   */
  void Sequence::dothread_sequence_start( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_SEQUENCE_START );
    std::string function = "Sequencer::Sequence::dothread_sequence_start";
    std::stringstream message;
    std::string reply;

    // The daemon shouldn't try to start more than one instance of this but
    // the class has a mutex to guarantee singular access, so lock it now.
    // It will remain locked until it goes out of scope (when the thread ends).
    //
    std::lock_guard<std::mutex> start_lock (seq.start_mtx);

    // clear the thread error state
    //
    seq.thr_error.store( NO_ERROR );

    // This is the main loop which runs as long as seqstate is 
    // has the SEQ_RUNNING bit set and neither ABORTING nor STOPPED nor STOPREQuested bits set.
    //
    while ( seq.is_seqstate_set( Sequencer::SEQ_RUNNING ) &&
          ( not seq.is_seqstate_set( Sequencer::SEQ_STOPPED )  || 
            not seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) || 
            not seq.is_seqstate_set( Sequencer::SEQ_STOPREQ ) ) ) {

      // If the ABORTING or STOPREQ or STOPPED bits are set then break the while loop
      //
      if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) ||
           seq.is_seqstate_set( Sequencer::SEQ_STOPPED )  ||
           seq.is_seqstate_set( Sequencer::SEQ_STOPREQ ) ) {
        logwrite( function, "stopping" );
        break;
      }

      logwrite( function, "sequencer running" );

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
        logwrite( function, "ERROR getting next target. stopping" );
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
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );   std::thread( dothread_calibrator_set, std::ref(seq) ).detach();

      // Now that the threads are running, wait until they are all finished.
      // When the SEQ_RUNNING bit is the only bit set then we are ready.
      //
      seq.set_reqstate_bit( Sequencer::SEQ_RUNNING );                  // set the requested state bit
      std::thread( dothread_wait_for_state, std::ref(seq) ).detach();  // wait for requested state

      logwrite( function, "waiting on notification" );

      std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );  // create a mutex object for waiting

      while ( seq.seqstate.load() != seq.reqstate.load() ) {
        message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
        logwrite( function, message.str() );
        seq.cv.wait( wait_lock );
      }
/*
      long error;
      while ( seq.cv.wait_for( wait_lock, std::chrono::seconds(1))==std::cv_status::timeout ) {
        logwrite( function, "waiting" );
      }
*/
      // Now that we're done waiting, check for errors or abort
      //
      if ( seq.thr_error.load() != NO_ERROR ) {
        logwrite( function, "ERROR from one or more threads" );
        break;
      }

      if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) ) {
        logwrite( function, "ABORT requested" );
        seq.clr_seqstate_bit( ( Sequencer::SEQ_ABORTING | Sequencer::SEQ_RUNNING ) );
        seq.clr_reqstate_bit( ( Sequencer::SEQ_ABORTING | Sequencer::SEQ_RUNNING ) );
        seq.set_seqstate_bit( Sequencer::SEQ_READY );
        seq.set_reqstate_bit( Sequencer::SEQ_READY );
        break;
      }

      logwrite( function, "starting exposure" );       // TODO log to telemetry!

      // Start the exposure in a thread...
      //
      seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );                // set the current state
      std::thread( dothread_trigger_exposure, std::ref(seq) ).detach();  // trigger exposure in a thread

      seq.set_reqstate_bit( Sequencer::SEQ_RUNNING );                    // set the requested state
      std::thread( dothread_wait_for_state, std::ref(seq) ).detach();    // wait for requested state

      // ...then wait for it to complete
      //
      logwrite( function, "waiting on notification" );
      while ( seq.seqstate.load() != seq.reqstate.load() ) {
        message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
        logwrite( function, message.str() );
        seq.cv.wait( wait_lock );
      }

      // Now that we're done waiting, check for errors or abort
      //
      if ( seq.thr_error.load() != NO_ERROR ) {
        logwrite( function, "ERROR from one or more threads" );
        break;
      }

      // When an exposure is aborted then it will be marked as UNASSIGNED
      //
      if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) ) {
        logwrite( function, "ABORT requested" );
        seq.clr_seqstate_bit( ( Sequencer::SEQ_ABORTING | Sequencer::SEQ_RUNNING ) );
        seq.clr_reqstate_bit( ( Sequencer::SEQ_ABORTING | Sequencer::SEQ_RUNNING ) );
        seq.set_seqstate_bit( Sequencer::SEQ_READY );
        seq.set_reqstate_bit( Sequencer::SEQ_READY );

        long error = seq.target.update_state( Sequencer::TARGET_UNASSIGNED );
        message.str(""); message << ( error==NO_ERROR ? "" : "ERROR " ) << "marking target " << seq.target.name 
                                 << " id " << seq.target.obsid << " order " << seq.target.obsorder
                                 << " as " << Sequencer::TARGET_UNASSIGNED;
        logwrite( function, message.str() );
        seq.thr_error.fetch_or( error );
        break;
      }

      // If not aborted then this exposure is now complete
      //
      message.str(""); message << "exposure complete for target " << seq.target.name 
                               << " id " << seq.target.obsid << " order " << seq.target.obsorder;
      logwrite( function, message.str() );

      // Update this target's state in the database
      //
      long error = seq.target.update_state( Sequencer::TARGET_COMPLETE );  // update the active target table
      if (error==NO_ERROR) error = seq.target.insert_completed();          // insert into the completed table
      seq.thr_error.fetch_or( error );                                     // report any error

    } // end while the SEQ_RUNNING bit is set in seqstate

    // The STOPREQ got us out of the while loop. Now that the loop has exited,
    // clear the STOPREQ and RUNNING bits and set the READY bits.
    //
    if ( seq.is_seqstate_set( Sequencer::SEQ_STOPREQ ) ) {
      seq.clr_seqstate_bit( ( Sequencer::SEQ_STOPREQ | Sequencer::SEQ_RUNNING ) );
      seq.clr_reqstate_bit( ( Sequencer::SEQ_STOPREQ | Sequencer::SEQ_RUNNING ) );
      seq.set_seqstate_bit( Sequencer::SEQ_READY );
      seq.set_reqstate_bit( Sequencer::SEQ_READY );
    }

    logwrite( function, "target list processing has stopped" );
    seq.clr_thrstate_bit( THR_SEQUENCE_START );
    return;
  }
  /**************** Sequencer::Sequence::dothread_sequence_start **************/


  /**************** Sequencer::Sequence::dothread_camera_set ******************/
  /**
   * @fn         dothread_camera_set
   * @brief      sets the camera according to the parameters in the target entry
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
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
      logwrite( function, "ERROR: unable to set exptime" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );

    seq.clr_thrstate_bit( THR_CAMERA_SET );
    return;
  }
  /**************** Sequencer::Sequence::dothread_camera_set ******************/


  /**************** Sequencer::Sequence::dothread_slit_set ********************/
  /**
   * @fn         dothread_slit_set
   * @brief      sets the slit according to the parameters in the target entry
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
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
      logwrite( function, "ERROR: unable to set slit" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );

    seq.clr_thrstate_bit( THR_SLIT_SET );
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
    seq.set_thrstate_bit( THR_POWER_INIT );
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

    logwrite( function, "ready" );
    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_POWER );

    seq.clr_thrstate_bit( THR_POWER_INIT );
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
    seq.set_thrstate_bit( THR_SLIT_INIT );
    std::string function = "Sequencer::Sequence::dothread_slit_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false, ishomed=false;

    // Turn on power to slit hardware.
    //
    // TODO
    std::stringstream cmd;
    cmd << "something";
//  error = seq.powerd.send( cmd.str(), reply ); // TODO send command to turn on slit hardware here
    logwrite( function, "[TODO] power on slit hardware not yet implemented" );

    // if not connected to the slit daemon then connect
    //
    if ( error==NO_ERROR && !seq.slitd.socket.isconnected() ) {
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
      logwrite( function, "ERROR initializing slit control" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );

    seq.clr_thrstate_bit( THR_SLIT_INIT );
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
    seq.set_thrstate_bit( THR_CALIB_INIT );
    std::string function = "Sequencer::Sequence::dothread_calib_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if required systems not powered on then turn them on
    //
    // TODO
    logwrite( function, "[TODO] power on calib hardware not yet implemented" );

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

    seq.clr_thrstate_bit( THR_CALIB_INIT );
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

    seq.clr_thrstate_bit( THR_TCS_INIT );
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
    seq.set_thrstate_bit( THR_FLEXURE_INIT );
    std::string function = "Sequencer::Sequence::dothread_flexure_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if required systems not powered on then turn them on
    //
    // TODO
    logwrite( function, "[TODO] power on flexure hardware not yet implemented" );

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

    seq.clr_thrstate_bit( THR_FLEXURE_INIT );
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
    seq.set_thrstate_bit( THR_FOCUS_SET );
    std::string function = "Sequencer::Sequence::dothread_focus_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if required systems not powered on then turn them on
    //
    // TODO
    logwrite( function, "[TODO] power on focus hardware not yet implemented" );

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

    seq.clr_thrstate_bit( THR_FOCUS_SET );
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
    seq.set_thrstate_bit( THR_CAMERA_INIT );
    std::string function = "Sequencer::Sequence::dothread_camera_init";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    bool isopen=false;

    // if required systems not powered on then turn them on
    //
    // TODO
    logwrite( function, "[TODO] power on cameras not yet implemented" );

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
//    error = seq.camerad.send( CAMERAD_OPEN, reply );
      error = seq.camerad.send( "open 2 3", reply );     // TODO two controllers working for testing
    }

    // send a bunch of stuff, will get cleaned up later
    // TODO
    //
    if (error==NO_ERROR) { logwrite( function, "loading firmware" );  error = seq.camerad.send( "load /home/developer/ss/DSP/SWIFT/sg2_48khz.lod", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting buffer");     error = seq.camerad.send( "buffer 1024 1024", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting geometry" );  error = seq.camerad.send( "geometry 1024 1024", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting readmode" );  error = seq.camerad.send( "readout U1", reply ); }
    if (error==NO_ERROR) { logwrite( function, "setting useframes" ); error = seq.camerad.send( "useframes false", reply ); }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to initialize camera" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );

    seq.clr_thrstate_bit( THR_CAMERA_INIT );
    return;
  }
  /**************** Sequencer::Sequence::dothread_camera_init *****************/


  /**************** Sequencer::Sequence::dothread_move_to_target **************/
  /**
   * @fn         dothread_move_to_target
   * @brief      send request to TCS to move to target coordinates
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
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

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] this target.name=" << seq.target.name << " last_target_name=" << seq.last_target_name;
    logwrite( function, message.str() );
#endif

    // If this target is the same as the last then this section gets skipped;
    // no need to repoint the telescope and wait for a slew if it's already
    // on target.
    //
    if ( seq.target.name != seq.last_target_name ) {

      // disable guiding
      //
      logwrite( function, "[TODO] disable guiding not yet implemented" );  // TODO
      error = NO_ERROR;

      // send coordinates to TCS
      //
      if ( error == NO_ERROR ) {
        std::stringstream coords;
        std::string reply, ra, dec;

        // convert ra, dec to decimal
        // can't be NaN
        //
        bool ra_isnan  = std::isnan( seq.radec_to_decimal( seq.target.ra,  ra  ) );
        bool dec_isnan = std::isnan( seq.radec_to_decimal( seq.target.dec, dec ) );

        if ( ra_isnan || dec_isnan ) {
          message.str(""); message << "ERROR converting";
          if ( ra_isnan  ) { message << " RA=\"" << seq.target.ra << "\""; }
          if ( dec_isnan ) { message << " DEC=\"" << seq.target.dec << "\""; }
          message << " to decimal";
          logwrite( function, message.str() );
          seq.thr_error.fetch_or( ERROR );                 
          seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCS );
          seq.clr_thrstate_bit( THR_MOVE_TO_TARGET );
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
      if ( error == NO_ERROR ) logwrite( function, "TODO send casangle" ); // TODO break this out in its own thread?

      // Wait for telescope slew to start.
      //
      logwrite( function, "waiting for TCS slew to start" );  // TODO log telemetry!

      // This wait is forever, or until an abort, because the slew time can be unpredictable
      // due to the fact that we are still waiting for a human (!) to initiate the slew.
      //
      // After sending the COORDS command, the TCS could be in STOPPED or TRACKING
      // from the last target.
      //
      while ( error==NO_ERROR && 
            ( not seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) || not seq.is_seqstate_set( Sequencer::SEQ_STOPPED ) ) ) {

        // if an abort has been requested then stop polling the TCS
        //
        if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) ) {
          logwrite( function, "abort requested. no longer waiting for TCS" );
          break;
        }

        std::string reply;
        int tcsvalue=TCS_UNDEFINED;
        error = seq.tcsd.send( "poll ?MOTION", reply );  // "poll" prevents excessive logging in tcsd
        if ( error == NO_ERROR) error = seq.extract_tcs_value( reply, tcsvalue );

        if ( tcsvalue == TCS_MOTION_SLEWING ) break;

        usleep( 100000 );  // don't poll the TCS too fast
      }
      logwrite( function, "TCS slew started" );  // TODO log telemetry!

      while ( error==NO_ERROR && 
            ( not seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) || not seq.is_seqstate_set( Sequencer::SEQ_STOPPED ) ) ) {

        // if an abort has been requested then stop polling the TCS
        //
        if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) ) {
          logwrite( function, "abort requested. no longer waiting for TCS" );
          break;
        }

        std::string reply;
        int tcsvalue=TCS_UNDEFINED;
        error = seq.tcsd.send( "poll ?MOTION", reply );  // "poll" prevents excessive logging in tcsd
        if ( error == NO_ERROR) error = seq.extract_tcs_value( reply, tcsvalue );

        if ( tcsvalue == TCS_MOTION_SETTLING ) break;

        usleep( 100000 );  // don't poll the TCS too fast
      }

      if ( error==NO_ERROR && 
         ( not seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) || not seq.is_seqstate_set( Sequencer::SEQ_STOPPED ) ) ) {
        logwrite( function, "TCS slew stopped" );  // TODO log telemetry!
      }

    }  // end if ( seq.target.name != seq.last_target_name ) 

    if ( error==NO_ERROR && 
       ( not seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) || not seq.is_seqstate_set( Sequencer::SEQ_STOPPED ) ) ) {
      logwrite( function, "waiting for TCS to settle" );  // TODO log telemetry!
    }

    // Before entering loop waiting for telescope to settle
    // get the current time (in seconds) to be used for timeout.
    //
    bool   settled       = false;
    double clock_now     = get_clock_time();
    double clock_timeout = clock_now + seq.tcs_settle_timeout;  // must settle by this time

    // Wait for telescope to settle (MOTION_TRACKING).
    // This wait can time out.
    //
    while ( error==NO_ERROR && not settled &&
          ( not seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) || not seq.is_seqstate_set( Sequencer::SEQ_STOPPED ) ) ) {

      // if an abort has been requested then stop polling the TCS
      //
      if ( seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) ) {
        logwrite( function, "abort requested. no longer waiting for TCS" );
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
        logwrite( function, "ERROR: timeout waiting for telescope to settle" );
        break;
      }
      usleep( 1000000 );  // don't poll the TCS too fast  TODO use smaller number in real life
    }

    if ( settled ) {

      logwrite( function, "TCS settled" );  // TODO log to telemetry!

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

    // otherwise, if no errors then save this target
    //
    else {
      seq.last_target_name = seq.target.name;
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCS );
    seq.clr_thrstate_bit( THR_MOVE_TO_TARGET );
    return;
  }
  /**************** Sequencer::Sequence::dothread_move_to_target **************/


  /**************** Sequencer::Sequence::dothread_notify_tcs ******************/
  /**
   * @fn         dothread_notify_tcs
   * @brief      notify TCS operator of next target coords for preauthorization, no move
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
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
    // on target.
    //
    if ( seq.target.name != seq.last_target_name ) {

      message << "requesting early pre-auth for next target: " << seq.target.name << " " << seq.target.ra << " " << seq.target.dec;
      logwrite( function, message.str() );

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
          seq.clr_thrstate_bit( THR_NOTIFY_TCS );
          return;
        }

        coords << "NEXT " << ra << " " << dec << " 0 0 0 \"" << seq.target.name << "\"";
//      error  = seq.tcsd.send( coords.str(), reply ); TODO
        message.str(""); message << "TODO: send command to TCS: " << coords.str();
        logwrite( function, message.str() );
        int tcsvalue;
        if ( error == NO_ERROR ) error = seq.extract_tcs_value( reply, tcsvalue );
        if ( error == NO_ERROR ) error = seq.parse_tcs_generic( tcsvalue );
      }

      // send casangle
      //
      if ( error == NO_ERROR ) logwrite( function, "TODO send casangle" ); // TODO break this out in its own thread?
    }
    seq.clr_thrstate_bit( THR_NOTIFY_TCS );
    return;
  }
  /**************** Sequencer::Sequence::dothread_notify_tcs ******************/


  /**************** Sequencer::Sequence::dothread_focus_set *******************/
  /**
   * @fn         dothread_focus_set
   * @brief  
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_focus_set( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_FOCUS_SET );
    std::string function = "Sequencer::Sequence::dothread_focus_set";
    long error=NO_ERROR;

    logwrite( function, "[TODO] not yet implemented. delaying 5s for focus" );
    usleep( 5000000 );

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to set focus" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );
    seq.clr_thrstate_bit( THR_FOCUS_SET );
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
    seq.set_thrstate_bit( THR_FLEXURE_SET );
    std::string function = "Sequencer::Sequence::dothread_flexure_set";
    long error=NO_ERROR;

    logwrite( function, "[TODO] not yet implemented. delaying 5s for flexure" );
    usleep( 5000000 );

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to set flexure control" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE );
    seq.clr_thrstate_bit( THR_FLEXURE_SET );
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
    seq.set_thrstate_bit( THR_CALIBRATOR_SET );
    std::string function = "Sequencer::Sequence::dothread_calibrator_set";
    long error=NO_ERROR;

    logwrite( function, "[TODO] not yet implemented. delaying 6s for calibrator" );
    usleep( 6000000 );

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to set calibrator" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );
    seq.clr_thrstate_bit( THR_CALIBRATOR_SET );
    return;
  }
  /**************** Sequencer::Sequence::dothread_calibrator_set **************/


  /**************** Sequencer::Sequence::dothread_trigger_exposure ************/
  /**
   * @fn         dothread_trigger_exposure
   * @brief      trigger and wait for exposure
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   * This function updates the target's state in the DB active table to EXPOSING
   * then sends the expose command to the cameras.
   *
   * TODO this is not yet ABORT-able
   *
   */
  void Sequence::dothread_trigger_exposure( Sequencer::Sequence &seq ) {
    seq.set_thrstate_bit( THR_TRIGGER_EXPOSURE );
    std::string function = "Sequencer::Sequence::dothread_trigger_exposure";
    std::stringstream message;
    std::string reply;
    long error;

    // When the preauth_time is non-zero, set this flag to true in order
    // to notify the TCS of the next target, when the remaining exposure
    // time is within TCS_PREAUTH_TIME of the end of the exposure time.
    //
    // When this flag is true, the async_listener thread will spawn a thread
    // to send the command to the TCS at the requested time, to get ready 
    // for the next target.
    //
    if ( seq.tcs_preauth_time > 0 ) seq.notify_tcs_next_target = true; else seq.notify_tcs_next_target = false;

    // Update this target's state in the database
    //
    error = seq.target.update_state( Sequencer::TARGET_EXPOSING );

    // send the EXPOSE command to camerad
    //
    if (error==NO_ERROR) error = seq.camerad.send( CAMERAD_EXPOSE, reply );

    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR: unable to start exposure" );
      seq.thr_error.fetch_or( error );
    }

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CAMERA );

    seq.clr_thrstate_bit( THR_TRIGGER_EXPOSURE );
    return;
  }
  /**************** Sequencer::Sequence::dothread_trigger_exposure ************/


  /**************** Sequencer::Sequence::dothread_wait_for_state **************/
  /**
   * @fn         dothread_wait_for_state
   * @brief      waits for sequence state to match the requested state
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
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

    // there are certain conditions when the startup sequence cannot be run
    //
    if ( seq.is_seqstate_set( Sequencer::SEQ_STOPREQ ) || seq.is_seqstate_set( Sequencer::SEQ_ABORTING ) ) {
      logwrite( function, "ERROR: stop/abort has been requested but not complete. please wait for sequencer to stop." );
      return( ERROR );
    }
    if ( seq.is_seqstate_set( Sequencer::SEQ_RUNNING ) ) {
      logwrite( function, "ERROR: cannot run startup while sequencer is currently running." );
      return( ERROR );
    }

    std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );  // create a mutex object for waiting

    // clear the thread error state
    //
    seq.thr_error.store( NO_ERROR );

    // Everything (except TCS) needs the power control to be running 
    // so initialize the power control first.
    // For this, set READY and WAIT_POWER bits, and clear STOPPED bit.
    //
    seq.clr_seqstate_bit( Sequencer::SEQ_STOPPED );
    seq.set_seqstate_bit( Sequencer::SEQ_STARTING | Sequencer::SEQ_WAIT_POWER );  // set the current state
    seq.clr_reqstate_bit( Sequencer::SEQ_STOPPED );
    seq.set_reqstate_bit( Sequencer::SEQ_STARTING );                              // set the requested state

    std::thread( dothread_power_init, std::ref(seq) ).detach();                   // start power initialization thread

    std::thread( dothread_wait_for_state, std::ref(seq) ).detach();               // wait for requested state

    logwrite( function, "waiting for power control to initialize" );

    while ( seq.seqstate.load() != seq.reqstate.load() ) {
      message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
      logwrite( function, message.str() );
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
      seq.clr_seqstate_bit( Sequencer::SEQ_STARTING );
      seq.set_seqstate_bit( Sequencer::SEQ_STOPPED );
      seq.clr_reqstate_bit( Sequencer::SEQ_STARTING );
      seq.set_reqstate_bit( Sequencer::SEQ_STOPPED );
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
    // When the SEQ_STARTING bit is the only bit set then we are ready.
    //
    std::thread( dothread_wait_for_state, std::ref(seq) ).detach();  // wait for requested state

    logwrite( function, "waiting for init threads to complete" );

    while ( seq.seqstate.load() != seq.reqstate.load() ) {
      message.str(""); message << "wait for state " << seq.seqstate_string( seq.reqstate.load() );
      logwrite( function, message.str() );
      seq.cv.wait( wait_lock );
    }

    // all done now so clear the STARTING bits
    //
    logwrite( function, "init threads completed" );
    seq.clr_seqstate_bit( Sequencer::SEQ_STARTING );
    seq.clr_reqstate_bit( Sequencer::SEQ_STARTING );

    // if any thread returned an error then we are not ready
    //
    if ( seq.thr_error.load() != NO_ERROR ) {
      logwrite( function, "ERROR from one or more threads. startup failed" );
      seq.set_seqstate_bit( Sequencer::SEQ_STOPPED );
      seq.set_reqstate_bit( Sequencer::SEQ_STOPPED );
      this->ready_to_start = false;
      return( ERROR );
    }
    else {
      seq.set_seqstate_bit( Sequencer::SEQ_READY );
      seq.set_reqstate_bit( Sequencer::SEQ_READY );
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
      message.str(""); message << "ERROR reading true|false state from string \"" << reply 
                               << "\" from " << whoami << ": expected two tokens";
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
  /**************** Sequencer::Sequence::radec_to_decimal ********************/


  /**************** Sequencer::Sequence::extract_tcs_value *******************/
  /**
   * @fn         extract_tcs_value
   * @brief      extract the value from a tcsd reply <val> DONE|ERROR
   * @param[in]  reply, string from the tcs daemon
   * @param[out] value, reference to integer containing the extracted value
   * @return     ERROR or NO_ERROR
   *
   * The tcs daemon (tcsd) is the interface to the TCS.  Replies from tcsd 
   * include the response from the TCS followed by a space and DONE or ERROR,
   * for example "<val> DONE". This function extracts just the value <val>
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
   * @param[in]  value, int from extract_tcs_value()
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
  /**************** Sequencer::Sequence::parse_tcs_generic *******************/


  /**************** Sequencer::Sequence::seqstate_string *********************/
  /**
   * @fn         seqstate_string
   * @brief      returns the string form of the states set in state word
   * @param[in]  state word to check
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
  /**************** Sequencer::Sequence::seqstate_string *********************/


  /**************** Sequencer::Sequence::thrstate_string *********************/
  /**
   * @fn         thrstate_string
   * @brief      returns the string form of the states set in thread state word
   * @param[in]  state word to check
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
  /**************** Sequencer::Sequence::thrstate_string *********************/


  /**************** Sequencer::Sequence::test ********************************/
  /**
   * @fn         test
   * @brief      test routines
   * @param[in]  args, input command and arguments
   * @param[out] retstring, any return values
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
      message.str(""); message << "seqstate: " << this->seqstate_string( ss );
      logwrite( function, message.str() );

      // same for the requested state
      //
      message.str(""); message << "reqstate: " << this->seqstate_string( rs );
      logwrite( function, message.str() );

      // and for the thread state
      //
      message.str(""); message << "threads : " << this->thrstate_string( ts );
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
                                                                    << this->target.obsorder; }
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
