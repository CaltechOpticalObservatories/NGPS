/**
 * @file    sequence_operations.cpp
 * @brief   implementation of operations
 * @author  David Hale <dhale@astro.caltech.edu>
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


  /***** Sequencer::Sequence::Ops::slicecam_init *****************************/
  /**
   * @brief      defines the slicecam_init operation
   * @return     Operation
   *
   */
  Sequence::Operation Sequence::Ops::slicecam_init() {
    Sequence::Operation op;
    op.thr = THR_ACAM_INIT;
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
