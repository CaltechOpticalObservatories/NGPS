/**
 * @file    sequence_acquisition.cpp
 * @brief   target acquisition code for the Sequence class
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "sequence.h"

namespace Sequencer {

  /***** Sequencer::Sequence::do_acam_acquire **********************************/
  /**
   * @brief      trigger ACAM acquisition and wait until guiding state reached
   * @return     NO_ERROR | ERROR | TIMEOUT
   *
   */
  long Sequence::do_acam_acquire() {
    const std::string function("Sequencer::Sequence::do_acam_acquire");
    std::string reply;

    ScopedState thr_state( thread_state_manager, Sequencer::THR_ACQUISITION );
    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_ACAM_ACQUIRE );

    // form and send the ACQUIRE command to ACAM
    //
    double ra_in    = radec_to_decimal( this->target.ra_hms  ) * TO_DEGREES;
    double dec_in   = radec_to_decimal( this->target.dec_dms );
    double angle_in = this->target.slitangle;

    if ( std::isnan(ra_in) || std::isnan(dec_in) ) {
      this->broadcast.error( function, "converting target coordinates to decimal" );
      return ERROR;
    }

    std::ostringstream cmd;
    cmd << ACAMD_ACQUIRE << " " << ra_in << " " << dec_in << " " << angle_in;

    this->broadcast.notice( function, "starting ACAM acquisition" );

    if ( this->acamd.command( cmd.str(), reply ) != NO_ERROR ) {
      this->broadcast.error( function, "sending acquire command to acamd" );
      return ERROR;
    }

    const bool use_timeout = ( this->acquisition_timeout > 0 );
    const auto timeout_time = std::chrono::steady_clock::now()
                            + std::chrono::duration<double>( this->acquisition_timeout );

    // wait for is_acam_guiding (I subscribe to this)
    // or cancel, or timeout
    //
    std::unique_lock<std::mutex> lock(this->acam_mtx);
    this->acam_cv.wait(lock, [&]() {
        return this->is_acam_guiding.load() || this->cancel_flag.load() ||
               (use_timeout && std::chrono::steady_clock::now() > timeout_time);
    });

    if (this->cancel_flag.load()) return ABORT;
    if (use_timeout && !this->is_acam_guiding.load()) {
      this->broadcast.error( function, "ACAM acquisition timed out!" );
      return TIMEOUT;
    }

    this->async.enqueue_and_log(function, "ACAM target acquired");
    return NO_ERROR;
  }
  /***** Sequencer::Sequence::do_acam_acquire **********************************/


  /***** Sequencer::Sequence::do_slicecam_fineacquire **************************/
  /**
   * @brief      trigger SLICECAM fine acquisition and wait until locked
   * @return     NO_ERROR | ERROR | TIMEOUT
   *
   */
  long Sequence::do_slicecam_fineacquire() {
    const std::string function("Sequencer::Sequence::do_slicecam_fineacquire");

    ScopedState wait_state(wait_state_manager, Sequencer::SEQ_WAIT_FINEACQUIRE);

    // TODO don't hard-code the arguments here:
    std::string reply;
    if (this->slicecamd.command( SLICECAMD_FINEACQUIRE+" start L", reply ) != NO_ERROR) {
      this->broadcast.error( function, "starting slicecam fine acquisition" );
      return ERROR;
    }

    if ( reply.find("ERROR") != std::string::npos ) {
      this->async.enqueue_and_log(function, "slicecam fine acquisition mode: "+reply);
      return ERROR;
    }

    this->broadcast.notice( function, "slicecam fine acquisition started" );

    const bool use_timeout = ( this->acquisition_timeout > 0 );
    const auto timeout_time = std::chrono::steady_clock::now()
                            + std::chrono::duration<double>( this->acquisition_timeout );

    // wait for is_fineacquire_locked (I subscribe to this)
    // or cancel, or timeout
    //
    std::unique_lock<std::mutex> lock(this->fineacquire_mtx);
    this->fineacquire_cv.wait(lock, [&]() {
        return this->is_fineacquire_locked.load() || this->cancel_flag.load() ||
               (use_timeout && std::chrono::steady_clock::now() > timeout_time);
    });

    if (this->cancel_flag.load()) return ABORT;
    if (use_timeout && !this->is_fineacquire_locked.load()) {
      this->broadcast.error( function, "slicecam fine acquisition timed out!" );
      return TIMEOUT;
    }

    this->async.enqueue_and_log(function, "slicecam fine acquisition target acquired");
    return NO_ERROR;
  }
  /***** Sequencer::Sequence::do_slicecam_fineacquire **************************/


  /***** Sequencer::Sequence::do_target_acquisition ****************************/
  /**
   * @brief      performs target acquisition
   * @details    First acquire on ACAM, then run slicecam fineacquire
   * @return     NO_ERROR | ABORT
   *
   */
  long Sequence::do_target_acquisition(std::string caller) {

    if (this->target.iscal) return NO_ERROR;

    // ---------- ACAM acquire -----------------------------
    //
    if ( this->do_acam_acquire() != NO_ERROR ) {
      this->broadcast.warning( caller, "acam acquisition failed" );

      // on ACAM acquisition failure wait for user to continue or cancel
      if ( this->wait_for_user(caller) == ABORT ) return ABORT;

      return NO_ERROR;  // user chose to continue
    }

    // ---------- SLICECAM fineacquire ---------------------
    //
    if ( this->do_slicecam_fineacquire() != NO_ERROR ) {
      this->broadcast.warning( caller, "slicecam fine acquisition failed" );
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::do_target_acquisition ****************************/


  /***** Sequencer::Sequence::do_target_virtualslit ****************************/
  /**
   * @brief      move to virtual slit position
   * @param[in]  mode  VirtualSlitMode
   * @return     NO_ERROR | NO_ERROR
   *
   */
  long Sequence::do_target_virtualslit(VirtualSlitMode mode) {
    if (this->target.iscal) return NO_ERROR;
    return this->slit_set(mode);
  }
  /***** Sequencer::Sequence::do_target_virtualslit ****************************/
}
