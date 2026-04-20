/**
 * @file    sequence_operations.cpp
 * @brief   implementation of operations
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * Each Ops factory method here returns a populated Sequence::Operation that
 * wraps a Sequence:: member function and identifies itself via its THR_ bit.
 * These are the "Internal" sequencer-level operations (database-driven,
 * may coordinate multiple daemons). Per-daemon "Passthrough" operations
 * (e.g. "camera exptime 30000") are built inline by init_operation_builders()
 * and therefore do not appear here.
 *
 */

#include "sequence.h"

namespace Sequencer {

  Sequence::Ops::Ops(Sequence* seq) : seq(seq) { }

  /***** Sequencer::Sequence::Ops::acam_init *********************************/
  /**
   * @brief      defines the acam_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::acam_init() {
    Sequence::Operation op;
    op.thr = THR_ACAM_INIT;
    op.func = [this]() { return seq->acam_init(); };
    op.params = { };
    op.max_attempts = 3;
    op.retry_delay = 2000;
    op.on_retry = [this]() {
      long error=NO_ERROR;
      error |= seq->set_power_switch(OFF, POWER_ACAM_CAM, std::chrono::seconds(5));
      error |= seq->daemon_restart(seq->acamd);
      return error;
    };
    return op;
  }
  /***** Sequencer::Sequence::Ops::acam_init *********************************/


  /***** Sequencer::Sequence::Ops::calib_init ********************************/
  /**
   * @brief      defines the calib_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::calib_init() {
    Sequence::Operation op;
    op.thr = THR_CALIB_INIT;
    op.func = [this]() { return seq->calib_init(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::calib_init ********************************/


  /***** Sequencer::Sequence::Ops::calib_set *********************************/
  /**
   * @brief      defines the calib_set operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::calib_set() {
    Sequence::Operation op;
    op.thr  = THR_CALIB_SET;
    op.func = [this]() { return seq->calib_set(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::calib_set *********************************/


  /***** Sequencer::Sequence::Ops::camera_init *******************************/
  /**
   * @brief      defines the camera_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::camera_init() {
    Sequence::Operation op;
    op.thr = THR_CAMERA_INIT;
    op.func = [this]() { return seq->camera_init(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::camera_init *******************************/


  /***** Sequencer::Sequence::Ops::camera_set ********************************/
  /**
   * @brief      defines the camera_set operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::camera_set() {
    Sequence::Operation op;
    op.thr  = THR_CAMERA_SET;
    op.func = [this]() { return seq->camera_set(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::camera_set ********************************/


  /***** Sequencer::Sequence::Ops::do_expose *********************************/
  /**
   * @brief      defines the expose operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::do_expose() {
    Sequence::Operation op;
    op.thr  = THR_EXPOSURE;
    op.func = [this]() {
      const std::string caller("Sequencer::Sequence::Ops::do_expose");
      return seq->do_exposure(caller);
    };
    return op;
  }
  /***** Sequencer::Sequence::Ops::do_expose *********************************/


  /***** Sequencer::Sequence::Ops::do_shutdown *******************************/
  /**
   * @brief      defines the shutdown operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::do_shutdown() {
    Sequence::Operation op;
    op.thr  = THR_SHUTDOWN;
    op.func = [this]() { return seq->shutdown(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::do_shutdown *******************************/


  /***** Sequencer::Sequence::Ops::do_startup ********************************/
  /**
   * @brief      defines the startup operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::do_startup() {
    Sequence::Operation op;
    op.thr  = THR_STARTUP;
    op.func = [this]() { return seq->startup(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::do_startup ********************************/


  /***** Sequencer::Sequence::Ops::flexure_init ******************************/
  /**
   * @brief      defines the flexure_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::flexure_init() {
    Sequence::Operation op;
    op.thr = THR_FLEXURE_INIT;
    op.func = [this]() { return seq->flexure_init(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::flexure_init ******************************/


  /***** Sequencer::Sequence::Ops::flexure_set *******************************/
  /**
   * @brief      defines the flexure_set operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::flexure_set() {
    Sequence::Operation op;
    op.thr  = THR_FLEXURE_SET;
    op.func = [this]() { return seq->flexure_set(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::flexure_set *******************************/


  /***** Sequencer::Sequence::Ops::focus_init ********************************/
  /**
   * @brief      defines the focus_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::focus_init() {
    Sequence::Operation op;
    op.thr = THR_FOCUS_INIT;
    op.func = [this]() { return seq->focus_init(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::focus_init ********************************/


  /***** Sequencer::Sequence::Ops::focus_set *********************************/
  /**
   * @brief      defines the focus_set operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::focus_set() {
    Sequence::Operation op;
    op.thr  = THR_FOCUS_SET;
    op.func = [this]() { return seq->focus_set(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::focus_set *********************************/


  /***** Sequencer::Sequence::Ops::move_to_target ****************************/
  /**
   * @brief      defines the move_to_target operation
   * @details    Optional ra= and dec= params on the ParsedCommand override the
   *             target coordinates for this invocation only; the overrides are
   *             read from seq->current_op_params inside Sequence::move_to_target()
   *             (populated by Sequence::run() just before op.func() is called)
   *             and restored on return so the database-backed target is not
   *             permanently mutated.
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::move_to_target() {
    Sequence::Operation op;
    op.thr  = THR_MOVE_TO_TARGET;
    op.func = [this]() { return seq->move_to_target(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::move_to_target ****************************/


  /***** Sequencer::Sequence::Ops::power_init ********************************/
  /**
   * @brief      defines the power_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::power_init() {
    Sequence::Operation op;
    op.thr = THR_POWER_INIT;
    op.func = [this]() { return seq->power_init(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::power_init ********************************/


  /***** Sequencer::Sequence::Ops::repeat_exposure ***************************/
  /**
   * @brief      defines the repeat_exposure operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::repeat_exposure() {
    Sequence::Operation op;
    op.thr  = THR_REPEAT_EXPOSURE;
    op.func = [this]() { return seq->repeat_exposure(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::repeat_exposure ***************************/


  /***** Sequencer::Sequence::Ops::slicecam_init *****************************/
  /**
   * @brief      defines the slicecam_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::slicecam_init() {
    Sequence::Operation op;
    op.thr = THR_SLICECAM_INIT;
    op.func = [this]() { return seq->slicecam_init(); };
    op.params = { };
    op.max_attempts = 3;
    op.retry_delay = 2000;
    op.on_retry = [this]() {
      long error=NO_ERROR;
      error |= seq->set_power_switch(OFF, POWER_SLICECAM, std::chrono::seconds(5));
      error |= seq->daemon_restart(seq->slicecamd);
      return error;
    };
    return op;
  }
  /***** Sequencer::Sequence::Ops::slicecam_init *****************************/


  /***** Sequencer::Sequence::Ops::slit_init *********************************/
  /**
   * @brief      defines the slit_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::slit_init() {
    Sequence::Operation op;
    op.thr = THR_SLIT_INIT;
    op.func = [this]() { return seq->slit_init(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::slit_init *********************************/


  /***** Sequencer::Sequence::Ops::slit_set **********************************/
  /**
   * @brief      defines the slit_set operation
   * @details    Reads optional mode= param from seq->current_op_params. When
   *             absent the default VSM_DATABASE mode is used (drives slit from
   *             database target entry).
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::slit_set() {
    Sequence::Operation op;
    op.thr  = THR_SLIT_SET;
    op.func = [this]() {
      const size_t mode = seq->current_op_params.get<size_t>("mode",
                            static_cast<size_t>(VSM_DATABASE));
      return seq->slit_set(static_cast<VirtualSlitMode>(mode));
    };
    return op;
  }
  /***** Sequencer::Sequence::Ops::slit_set **********************************/


  /***** Sequencer::Sequence::Ops::target_offset *****************************/
  /**
   * @brief      defines the target_offset operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::target_offset() {
    Sequence::Operation op;
    op.thr  = THR_MOVE_TO_TARGET;  // no dedicated THR_ for target_offset; reuse move_to_target identity
    op.func = [this]() { return seq->target_offset(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::target_offset *****************************/


  /***** Sequencer::Sequence::Ops::tcs_init **********************************/
  /**
   * @brief      defines the tcs_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::tcs_init() {
    Sequence::Operation op;
    op.thr = THR_TCS_INIT;
    op.func = [this]() { return seq->tcs_init(); };
    return op;
  }
  /***** Sequencer::Sequence::Ops::tcs_init **********************************/

}
