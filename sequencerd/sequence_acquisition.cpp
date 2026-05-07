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
      logwrite( function, "ERROR converting target coordinates to decimal" );
      return ERROR;
    }

    std::ostringstream cmd;
    cmd << ACAMD_ACQUIRE << " " << ra_in << " " << dec_in << " " << angle_in;

    this->broadcast.notice( function, "starting ACAM acquisition" );

    // Freshness boundary: any ACAMD status publish strictly newer than this
    // timestamp is considered fresh for this acquire cycle. The guard-band
    // absorbs jitter and the timing race between this command send and
    // ACAM's forced publish triggered by the command.
    //
    const int64_t freshness_boundary_us = get_time_us() - ACAM_FRESHNESS_GUARD_US;

    if ( this->acamd.command( cmd.str(), reply ) != NO_ERROR ) {
      logwrite( function, "ERROR sending acquire command to acamd" );
      return ERROR;
    }

    const bool use_timeout = ( this->acquisition_timeout > 0 );
    const auto timeout_time = std::chrono::steady_clock::now()
                            + std::chrono::duration<double>( this->acquisition_timeout );

    // wait for is_acam_guiding (I subscribe to this) with a fresh ACAMD publish
    // since the acquire command was sent, or cancel, or timeout
    //
    std::unique_lock<std::mutex> lock(this->acam_mtx);
    this->acam_cv.wait(lock, [&]() {
        const bool fresh = this->acam_pubtime.load() > freshness_boundary_us;
        return ( fresh && this->is_acam_guiding.load() ) || this->cancel_flag.load() ||
               ( use_timeout && std::chrono::steady_clock::now() > timeout_time );
    });

    if (this->cancel_flag.load()) return ABORT;

    if (use_timeout && this->acam_pubtime.load() <= freshness_boundary_us) {
      logwrite( function, "ERROR timed out waiting for fresh ACAM status" );
      return TIMEOUT;
    }

    if (use_timeout && !this->is_acam_guiding.load()) {
      logwrite( function, "ERROR ACAM acquisition timed out!" );
      return TIMEOUT;
    }

    this->broadcast.notice( function, "ACAM target acquired" );
    return NO_ERROR;
  }
  /***** Sequencer::Sequence::do_acam_acquire **********************************/


  /***** Sequencer::Sequence::do_acam_stop *************************************/
  /**
   * @brief      stops ACAM guiding
   * @return     NO_ERROR | ERROR | TIMEOUT
   *
   */
  long Sequence::do_acam_stop() {
    const std::string function("Sequencer::Sequence::do_acam_stop");

    // nothing to do
    if (!this->is_acam_guiding.load()) return NO_ERROR;

    if ( wait_state_manager.is_set( Sequencer::SEQ_WAIT_ACAM_ACQUIRE ) ) {
      logwrite( function, "ERROR acquisition still in progress" );
      return ERROR;
    }

    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_ACAM );

    // any acamd status newer than this timestamp is considered fresh
    const int64_t freshness_boundary_us = get_time_us() - ACAM_FRESHNESS_GUARD_US;

    // send ACQUIRE STOP command to ACAM
    std::string reply;
    if ( this->acamd.command( ACAMD_ACQUIRE+" stop", reply ) != NO_ERROR ) {
      logwrite( function, "ERROR stopping guiding" );
      return ERROR;
    }

    if ( reply.find("ERROR") != std::string::npos ) {
      logwrite( function, "ERROR acam: "+reply );
      return ERROR;
    }

    const bool use_timeout = ( this->acquisition_timeout > 0 );
    const auto timeout_time = std::chrono::steady_clock::now()
                            + std::chrono::duration<double>( this->acquisition_timeout );

    // wait for a new is_acam_guiding, or cancel, or timeout
    std::unique_lock<std::mutex> lock(this->acam_mtx);
    this->acam_cv.wait(lock, [&]() {
        const bool fresh = this->acam_pubtime.load() > freshness_boundary_us;
        return ( fresh && !this->is_acam_guiding.load() ) || this->cancel_flag.load() ||
               ( use_timeout && std::chrono::steady_clock::now() > timeout_time );
    });

    // success
    if (!this->is_acam_guiding.load()) {
      this->broadcast.notice( function, "guiding stopped" );
      return NO_ERROR;
    }

    // failure
    if (this->cancel_flag.load()) return ABORT;
    if (use_timeout) {
      logwrite( function, "ERROR timeout stopping guiding" );
      return TIMEOUT;
    }
    logwrite( function, "ERROR stopping guiding" );
    return ERROR;
  }
  /***** Sequencer::Sequence::do_acam_stop *************************************/


  /***** Sequencer::Sequence::do_slicecam_fineacquire **************************/
  /**
   * @brief      trigger SLICECAM fine acquisition and wait until locked
   * @return     NO_ERROR | ERROR | TIMEOUT
   *
   */
  long Sequence::do_slicecam_fineacquire() {
    const std::string function("Sequencer::Sequence::do_slicecam_fineacquire");

    ScopedState wait_state(wait_state_manager, Sequencer::SEQ_WAIT_FINEACQUIRE);

    std::string reply;
    if (this->slicecamd.command( SLICECAMD_FINEACQUIRE+" start", reply ) != NO_ERROR) {
      logwrite( function, "ERROR starting slicecam fine acquisition" );
      return ERROR;
    }

    if ( reply.find("ERROR") != std::string::npos ) {
      logwrite( function, "ERROR slicecam fine acquisition mode: "+reply );
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
      logwrite( function, "ERROR slicecam fine acquisition timed out!" );
      return TIMEOUT;
    }

    this->broadcast.notice( function, "slicecam fine acquisition target acquired" );
    return NO_ERROR;
  }
  /***** Sequencer::Sequence::do_slicecam_fineacquire **************************/


  /***** Sequencer::Sequence::do_slicecam_stop **********************************/
  /**
   * @brief      stops slicecam fineacquire
   * @return     NO_ERROR | ERROR | TIMEOUT
   *
   */
  long Sequence::do_slicecam_stop() {
    const std::string function("Sequencer::Sequence::do_slicecam_stop");

    // nothing to do
    if (!this->is_fineacquire_locked.load()) return NO_ERROR;

    if ( wait_state_manager.is_set( Sequencer::SEQ_WAIT_FINEACQUIRE ) ) {
      logwrite( function, "ERROR fine acquisition still in progress" );
      return ERROR;
    }

    ScopedState wait_state( wait_state_manager, Sequencer::SEQ_WAIT_SLICECAM );

    // send STOP command to SLICECAM
    std::string reply;
    if (this->slicecamd.command( SLICECAMD_FINEACQUIRE+" stop", reply ) != NO_ERROR) {
      logwrite( function, "ERROR stopping fine acquisition" );
      return ERROR;
    }

    if ( reply.find("ERROR") != std::string::npos ) {
      logwrite( function, "ERROR slicecam fine acquisition mode: "+reply );
      return ERROR;
    }

    const bool use_timeout = ( this->acquisition_timeout > 0 );
    const auto timeout_time = std::chrono::steady_clock::now()
                            + std::chrono::duration<double>( this->acquisition_timeout );

    // wait for !is_fineacquire_locked or cancel, or timeout
    std::unique_lock<std::mutex> lock(this->fineacquire_mtx);
    this->fineacquire_cv.wait(lock, [&]() {
        return !this->is_fineacquire_locked.load() || this->cancel_flag.load() ||
               (use_timeout && std::chrono::steady_clock::now() > timeout_time);
    });

    // success
    if (!this->is_fineacquire_locked.load()) {
      this->broadcast.notice( function, "slicecam fine acquisition stopped" );
      return NO_ERROR;
    }

    // failure
    if (this->cancel_flag.load()) return ABORT;
    if (use_timeout) {
      logwrite( function, "ERROR slicecam fine acquisition timed out!" );
      return TIMEOUT;
    }
    logwrite( function, "ERROR stopping fine acquisition" );
    return ERROR;
  }
  /***** Sequencer::Sequence::do_slicecam_stop *********************************/

}
