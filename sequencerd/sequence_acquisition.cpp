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

    ScopedState thr_state( op_state_manager, Sequencer::OP_ACQUISITION );
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
    // since the acquire command was sent, or acamd stopped without acquiring
    // (failure), or cancel, or timeout.
    // was_acquiring latches true once acamd enters "acquiring" mode, so that
    // !is_acam_acquiring only signals failure after the attempt began — not
    // before acamd's first telemetry publish arrives.
    //
    bool was_acquiring = false;
    std::unique_lock<std::mutex> lock(this->acam_mtx);
    this->acam_cv.wait(lock, [&]() {
        if ( this->is_acam_acquiring.load() ) was_acquiring = true;
        const bool fresh = this->acam_pubtime.load() > freshness_boundary_us;
        return ( fresh && this->is_acam_guiding.load() ) || this->cancel_flag.load() ||
               ( was_acquiring && !this->is_acam_acquiring.load() ) ||
               ( use_timeout && std::chrono::steady_clock::now() > timeout_time );
    });

    if (this->cancel_flag.load()) return ABORT;

    // Determine outcome by reading state. If not guiding, then this is a
    // failure of some kind; distinguish a true timeout from acamd stopping
    // without acquiring by consulting the clock. This ordering ensures that
    // a failure publish arriving at or near the timeout boundary is still
    // reported as a failure (not mis-labeled as a timeout).
    //
    if (!this->is_acam_guiding.load()) {
      if (use_timeout && std::chrono::steady_clock::now() >= timeout_time) {
        logwrite( function, "ERROR ACAM acquisition timed out!" );
        return TIMEOUT;
      }
      logwrite( function, "ERROR ACAM acquisition stopped without acquiring target" );
      return ERROR;
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
    // or slicecam stopped without locking (failure), or cancel, or timeout.
    // was_running latches true once slicecamd confirms it started, so that
    // !is_fineacquire_running only signals failure after the run began — not
    // before slicecamd's first telemetry publish arrives.
    //
    bool was_running = false;
    std::unique_lock<std::mutex> lock(this->fineacquire_mtx);
    this->fineacquire_cv.wait(lock, [&]() {
        if ( this->is_fineacquire_running.load() ) was_running = true;
        return this->is_fineacquire_locked.load() || this->cancel_flag.load() ||
               ( was_running && !this->is_fineacquire_running.load() ) ||
               (use_timeout && std::chrono::steady_clock::now() > timeout_time);
    });

    if (this->cancel_flag.load()) return ABORT;

    // Determine outcome by reading state. If not locked, then this is a
    // failure of some kind; distinguish a true timeout from slicecamd
    // stopping without locking by consulting the clock. This ordering
    // ensures that a failure publish arriving at or near the timeout
    // boundary is still reported as a failure (not mis-labeled as a
    // timeout).
    //
    if (!this->is_fineacquire_locked.load()) {
      if (use_timeout && std::chrono::steady_clock::now() >= timeout_time) {
        logwrite( function, "ERROR slicecam fine acquisition timed out!" );
        return TIMEOUT;
      }
      logwrite( function, "ERROR slicecam fine acquisition stopped without locking" );
      return ERROR;
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


  /***** Sequencer::Sequence::do_target_acquisition ****************************/
  /**
   * @brief      performs target acquisition
   * @details    First acquire on ACAM, then run slicecam fineacquire if enabled.
   *             Waits for user confirmation when fineacquire is disabled or fails.
   * @param[in]  caller  name of calling function for logging
   * @return     NO_ERROR | ABORT
   *
   */
  long Sequence::do_target_acquisition(std::string caller) {

    if (this->target.iscal) return NO_ERROR;

    // ---------- ACAM acquire -----------------------------
    //
    if ( this->do_acam_acquire() != NO_ERROR ) {
      this->broadcast.warning( caller, "acam acquisition failed" );
      if ( this->wait_for_user(caller) == ABORT ) return ABORT;
      return NO_ERROR;  // user chose to continue
    }

    // ---------- SLICECAM fineacquire ---------------------
    //
    bool dofine = this->should_fineacquire.load();
    long ret = NO_ERROR;
    if ( dofine ) ret = this->do_slicecam_fineacquire();
    if ( ret != NO_ERROR ) this->broadcast.warning( caller, "slicecam fine acquisition failed" );

    // wait for user if fineacquire was disabled or failed
    if ( (!dofine || ret != NO_ERROR) &&
         this->wait_for_user(caller) == ABORT ) return ABORT;

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::do_target_acquisition ****************************/


  /***** Sequencer::Sequence::do_target_virtualslit ****************************/
  /**
   * @brief      move to virtual slit position
   * @param[in]  mode  VirtualSlitMode
   * @return     NO_ERROR
   *
   */
  long Sequence::do_target_virtualslit(VirtualSlitMode mode) {
    if (this->target.iscal) return NO_ERROR;
    return this->slit_setup(mode);
  }
  /***** Sequencer::Sequence::do_target_virtualslit ****************************/


  /***** Sequencer::Sequence::do_target_offset *********************************/
  /**
   * @brief      send guide offsets to the TCS after successful fine acquisition
   * @details    Offsets are only sent when fineacquire achieved a lock.
   *             If the offset fails, the operator is given the opportunity to
   *             continue or cancel.
   * @param[in]  caller  name of calling function for logging
   * @return     NO_ERROR | ABORT
   *
   */
  long Sequence::do_target_offset(std::string caller) {

    if (this->target.iscal) return NO_ERROR;

    // offsets are only meaningful when fineacquire achieved a lock
    if ( !this->is_fineacquire_locked.load() ) return NO_ERROR;

    if ( this->target_offset() == ERROR ) {
      this->broadcast.warning( caller, "target offset failed" );
      if ( this->wait_for_user(caller) == ABORT ) return ABORT;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::do_target_offset *********************************/

}
