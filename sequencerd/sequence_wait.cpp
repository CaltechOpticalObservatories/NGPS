/**
 * @file    sequence_wait.cpp
 * @brief   wait wrappers used in the Sequence class
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "sequence.h"

namespace Sequencer {

  /***** Sequencer::Sequence::wait_for_ontarget *******************************/
  /**
   * @brief      waits for the TCS Operator to click 'ontarget'
   * @param[in]  caller  reference to caller's name for logging
   * @return     NO_ERROR on continue | ABORT on cancel
   *
   */
  long Sequence::wait_for_ontarget(std::string caller) {
    // waiting for TCS Operator input (or cancel)
    {
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_TCSOP );

    this->broadcast( caller, Severity::NOTICE, "waiting for TCS operator to send 'ontarget' signal" );

    while ( !this->cancel_flag.load() &&
            !this->is_ontarget.load() ) {

      std::unique_lock<std::mutex> lock(cv_mutex);
      this->cv.wait( lock, [this]() { return( this->is_ontarget.load() ||
                                              this->cancel_flag.load() ); } );
    }

    this->broadcast( caller, Severity::NOTICE, "received "
                                        +(this->cancel_flag.load() ? std::string("cancel")
                                                                   : std::string("ontarget"))
                                        +" signal!" );
    }
    this->is_ontarget.store(false);

    return (this->cancel_flag.load() ? ABORT : NO_ERROR);
  }
  /***** Sequencer::Sequence::wait_for_ontarget *******************************/


  /***** Sequencer::Sequence::wait_for_user ***********************************/
  /**
   * @brief      waits for the user to click a button, or cancel
   * @details    Use this when you just want to slow things down or get a
   *             cup of coffee instead of observing.
   * @param[in]  caller  reference to caller's name for logging
   * @return     NO_ERROR on continue | ABORT on cancel
   *
   */
  long Sequence::wait_for_user(std::string caller) {
    {
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_USER );

    this->broadcast( caller, Severity::NOTICE, "waiting for USER to send 'continue' signal" );

    while ( !this->cancel_flag.load() && !this->is_usercontinue.load() ) {
      std::unique_lock<std::mutex> lock(cv_mutex);
      this->cv.wait( lock, [this]() { return( this->is_usercontinue.load() || this->cancel_flag.load() ); } );
    }

    this->broadcast( caller, Severity::NOTICE, "received "
                                           +(this->cancel_flag.load() ? std::string("cancel") : std::string("continue"))
                                           +" signal!" );
    }  // end scope for wait_state = WAIT_USER

    if ( this->cancel_flag.load() ) {
      this->broadcast( caller, Severity::NOTICE, "sequence cancelled" );
      return ABORT;
    }

    this->is_usercontinue.store(false);

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::wait_for_user ***********************************/


  /***** Sequencer::Sequence::wait_for_exposure *******************************/
  /**
   * @brief      waits for exposure completion, or cancel
   * @param[in]  caller  reference to caller's name for logging
   * @return     NO_ERROR on continue | ABORT on cancel
   *
   */
  long Sequence::wait_for_exposure(std::string caller) {
    logwrite(caller, "waiting for exposure");
    while (!this->cancel_flag.load() &&
           wait_state_manager.is_set(Sequencer::SEQ_WAIT_EXPOSE)) {
      std::unique_lock<std::mutex> lock(cv_mutex);
      this->cv.wait( lock, [this]() { return(!wait_state_manager.is_set(SEQ_WAIT_EXPOSE) ||
                                             this->cancel_flag.load()); } );
    }

    if (this->cancel_flag.load()) {
      this->broadcast( caller, Severity::NOTICE, "exposure cancelled" );
      return ABORT;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::wait_for_exposure *******************************/


  /***** Sequencer::Sequence::wait_for_readout ********************************/
  /**
   * @brief      waits for readout completion, or cancel
   * @param[in]  caller  reference to caller's name for logging
   * @return     NO_ERROR on continue | ABORT on cancel
   *
   */
  long Sequence::wait_for_readout(std::string caller) {

    // don't have to wait for readout when using frame transfer
    if ( this->is_science_frame_transfer ) return NO_ERROR;

    logwrite(caller, "waiting for readout");

    while (!this->cancel_flag.load() &&
           wait_state_manager.is_set(Sequencer::SEQ_WAIT_READOUT)) {
      std::unique_lock<std::mutex> lock(cv_mutex);
      this->cv.wait( lock, [this]() { return(!wait_state_manager.is_set(SEQ_WAIT_READOUT) ||
                                             this->cancel_flag.load()); } );
    }

    if (this->cancel_flag.load()) {
      this->broadcast( caller, Severity::NOTICE, "wait for readout cancelled" );
      return ABORT;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::wait_for_readout ********************************/


  /***** Sequencer::Sequence::wait_for_canexpose ******************************/
  /**
   * @brief      waits for camera to be ready to expose, or cancel
   * @param[in]  caller  reference to caller's name for logging
   * @return     NO_ERROR on continue | ABORT on cancel
   *
   */
  long Sequence::wait_for_canexpose(std::string caller) {

    this->broadcast( caller, Severity::NOTICE, "waiting for camera to be ready to expose" );

    while ( !this->cancel_flag.load() &&
            !this->can_expose.load() ) {

      std::unique_lock<std::mutex> lock(this->camerad_mtx);
      this->camerad_cv.wait( lock, [this]() { return( this->can_expose.load() ||
                                                      this->cancel_flag.load() ); } );
    }

    if (this->cancel_flag.load()) {
      this->broadcast( caller, Severity::NOTICE, "wait for can_expose cancelled" );
      return ABORT;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::wait_for_canexpose ******************************/

}
