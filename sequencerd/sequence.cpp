#include "sequence.h"

namespace Sequencer {

  Sequence::Sequence() {
    this->runstate=Sequencer::STOPPED;
  }

  Sequence::~Sequence() {
  }


  /**************** Sequencer::Sequence:: *************************************/
  /**
   * @fn         
   * @brief      
   * @param[in]  
   * @return     none
   *
   */
  void Sequence::set_seqstate_bit( uint32_t state ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::set_seqstate_bit";
    std::stringstream message;
    std::uint32_t oldstate = this->seqstate;
    message.str(""); message << "[DEBUG] state changed from " << oldstate << " to " << this->seqstate;
    logwrite( function, message.str() );
#endif
    this->seqstate |= state;
  }
  /**************** Sequencer::Sequence:: *************************************/


  /**************** Sequencer::Sequence:: *************************************/
  /**
   * @fn     
   * @brief  
   * @param[in]  
   * @return     none
   *
   */
  void Sequence::clr_seqstate_bit( uint32_t state ) {
#ifdef LOGLEVEL_DEBUG
    std::string function = "Sequencer::Sequence::clr_seqstate_bit";
    std::stringstream message;
    std::uint32_t oldstate = this->seqstate;
    message.str(""); message << "[DEBUG] state changed from " << oldstate << " to " << this->seqstate;
    logwrite( function, message.str() );
#endif
    this->seqstate &= ~state;
  }
  /**************** Sequencer::Sequence:: *************************************/


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

    // The daemon shouldn't start this more than once but the class has
    // a mutex to guarantee singular access, so lock it now. It will
    // remain locked until it goes out of scope (when the thread ends).
    //
    std::lock_guard<std::mutex> start_lock (seq.start_mtx);

    logwrite( function, "starting" );

    while ( seq.runstate == Sequencer::RUNNING ) {
      message.str(""); message << "runstate=" << seq.runstate;
      logwrite( function, message.str() );

      if ( seq.get_next_target() == TargetInfo::TARGET_NOT_FOUND ) {
        logwrite( function, "no target found" );
        usleep( 1000000 );
        continue;
      }

      // get the threads going --
      // These things can all be done in parallel,
      // just have to sync up at the end.
      //
      std::thread( dothread_set_slit, std::ref(seq) ).detach();
      std::thread( dothread_acquire_target, std::ref(seq) ).detach();
      std::thread( dothread_set_focus, std::ref(seq) ).detach();
      std::thread( dothread_set_flexure, std::ref(seq) ).detach();
      std::thread( dothread_set_calibrator, std::ref(seq) ).detach();

      seq.set_seqstate_bit( Sequencer::SEQ_READY );

      // now that the threads are running, wait until they are all finished
      //
      std::thread( dothread_wait_for_state, std::ref(seq), Sequencer::SEQ_READY ).detach();

      logwrite( function, "waiting on notification" );

      std::unique_lock<std::mutex> wait_lock( seq.wait_mtx );

      while ( seq.seqstate != Sequencer::SEQ_READY ) seq.cv.wait( wait_lock );

      logwrite( function, " ready!!! " );

      logwrite( function, " starting exposure ... " );
      usleep( 3000000 );


      if ( seq.runstate == Sequencer::STOPPED ) {
        logwrite( function, "runstate is stopped. bye!" );
        break;
      }
      usleep( 1000000 );
    }

    logwrite( function, "terminated" );
    return;
  }
  /**************** Sequencer::Sequence::dothread_sequence_start **************/


  /**************** Sequencer::Sequence::dothread_set_slit ********************/
  /**
   * @fn         dothread_set_slit
   * @brief      sets the slit according to the parameters in the target entry
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_set_slit( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_set_slit";
    std::string reply;
    logwrite( function, "" );
    std::stringstream slitcmd;

    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );

    // send the SET command to slitd
    //
    slitcmd << SLITD_SET << " " << seq.target.slitwidth << " " << seq.target.slitoffset;

    seq.slitd.send( slitcmd.str(), reply );

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_SLIT );

    return;
  }
  /**************** Sequencer::Sequence::dothread_set_slit ********************/


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

    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_TCS );

    logwrite( function, "delaying 10s for TCS" );
    usleep( 10000000 );

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_TCS );

    return;
  }
  /**************** Sequencer::Sequence::dothread_acquire_target **************/


  /**************** Sequencer::Sequence::dothread_set_focus *******************/
  /**
   * @fn         dothread_set_focus
   * @brief  
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_set_focus( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_set_focus";

    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );

    logwrite( function, "delaying 5s for focus" );
    usleep( 5000000 );

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FOCUS );
    return;
  }
  /**************** Sequencer::Sequence::dothread_set_focus *******************/


  /**************** Sequencer::Sequence::dothread_set_flexure *****************/
  /**
   * @fn         dothread_set_flexure
   * @brief  
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_set_flexure( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_set_flexure";

    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE );

    logwrite( function, "delaying 5s for flexure" );
    usleep( 5000000 );

    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_FLEXURE );
    return;
  }
  /**************** Sequencer::Sequence::dothread_set_flexure *****************/


  /**************** Sequencer::Sequence::dothread_set_calibrator **************/
  /**
   * @fn         dothread_set_calibrator
   * @brief  
   * @param[in]  ref to Sequencer::Sequence object
   * @return     none
   *
   */
  void Sequence::dothread_set_calibrator( Sequencer::Sequence &seq ) {
    std::string function = "Sequencer::Sequence::dothread_set_calibrator";
    logwrite( function, "delaying 6s for calibrator" );
    seq.set_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );
    usleep( 6000000 );
    seq.clr_seqstate_bit( Sequencer::SEQ_WAIT_CALIB );
    return;
  }
  /**************** Sequencer::Sequence::dothread_set_calibrator **************/


  /**************** Sequencer::Sequence::dothread_wait_for_state **************/
  /**
   * @fn         dothread_wait_for_state
   * @brief      waits for sequence state to match the requested state
   * @param[in]  ref to Sequencer::Sequence object
   * @param[in]  state to wait for
   * @return     none
   *
   * This function is spawned in a thread. It waits forever until the seq.seqstate
   * matches the requested state, at which point it sends a cv.notify_all signal
   * to unblock all threads currently waiting on cv.
   *
   */
  void Sequence::dothread_wait_for_state( Sequencer::Sequence &seq, uint32_t state ) {
    std::string function = "Sequencer::Sequence::dothread_wait_for_state";
    std::stringstream message;
    message.str(""); message << "waiting for state=" << state;
    logwrite( function, message.str() );

    // wait forever
    // or until seqstate is the requested state or stopped
    //
    while ( seq.seqstate != state && seq.seqstate != Sequencer::SEQ_STOPPED ) ;

    // done waiting so send notification
    //
    std::unique_lock<std::mutex> lck(seq.wait_mtx);
    logwrite( function, "notifying" );
    seq.cv.notify_all();

    return;
  }
  /**************** Sequencer::Sequence::dothread_wait_for_state **************/


  /**************** Sequencer::Sequence::startup ******************************/
  /**
   * @fn     startup
   * @brief  performs nightly startup
   * @param  none
   * @return ERROR or NO_ERROR
   *
   */
  long Sequence::startup() {
    std::string function = "Sequencer::Sequence::startup";
    std::stringstream message;
    std::string retstring;
    long error=NO_ERROR;

    // if slit not powered then turn on power
    // if calib not powered then turn on power
    // etc.

    // Do each of the following steps,
    // get out when one returns an error.
    //
    while ( error == NO_ERROR ) {

      logwrite( function, "opening slitd" );
      error = this->slitd.command( "open" );

      logwrite( function, "homing slit" );
      error = this->slitd.command( SLITD_HOME );

      break;
    }

    if ( error == NO_ERROR ) {
      this->ready_to_start = true;
    }
    else {
      this->ready_to_start = false;
    }

    return( error );
  }
  /**************** Sequencer::Sequence::startup ******************************/


  /**************** Sequencer::Sequence::get_next_target *********************/
  /**
   * @fn     get_next_target
   * @brief  get the next available target from the DB
   * @param  none
   * @return ERROR or NO_ERROR
   *
   */
  long Sequence::get_next_target() {
    std::string function = "Sequencer::Sequence::get_next_target";
    std::stringstream message;
    long retval;

    logwrite( function, "" );

    retval = this->target.get_next();  // maybe call this directly w/o a separate sequence function

    if ( retval != TargetInfo::TARGET_FOUND ) {
      logwrite( function, "no target found" ); // TODO change this to an ASYNC message
      return retval;
    }

/**  TODO remember to write something to validate the values in the TargetInfo class before trying to use them!
 *
    if ( !this->target.validate_target() ) {
      logwrite( function, "invalid target entry" ); // TODO change this to an ASYNC message
      return TARGET_NOT_FOUND;
    }
**/

    message.str(""); message << "target loaded: "
                             << this->target.name       << ", "
                             << this->target.ra         << ", "
                             << this->target.dec        << ", "
                             << this->target.epoch      << ", "
                             << this->target.slitwidth  << ", "
                             << this->target.slitoffset << ", "
                             << this->target.exptime    << ", "
                             << this->target.repeat     << ", "
                             << this->target.binspect   << ", "
                             << this->target.binspat;

    logwrite( function, message.str() );

    return TargetInfo::TARGET_FOUND;
  }
  /**************** Sequencer::Sequence::get_next_target *********************/
}
