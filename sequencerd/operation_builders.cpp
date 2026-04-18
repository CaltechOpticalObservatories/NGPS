/**
 * @file    operation_builders.cpp
 * @brief   implementation for building operations from command names
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "sequence.h"

namespace Sequencer {

  /***** Sequencer::Sequence::init_operation_builders ************************/
  /**
   * @brief      initializes registration between name and operation
   * @details    Maps command names to function that populate an Operation
   *             from a ParsedCommand.
   *
   */
  void Sequence::init_operation_builders() {

    // ---------- FOCUS_SET --------------------------------------------------

    op_builders["focus_set"] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_FOCUS_SET;
      op.func = [this]() {
        return focus_set();
      };
      op.params = cmd.params;
    };

    // ---------- MOVE_TO_TARGET ---------------------------------------------

    op_builders["move_to_target"] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_MOVE_TO_TARGET;
      op.func = [this,params=cmd.params]() {
                  if (params.has("ra") && params.has("dec")) {
                    this->target.ra_hms = params.get(std::string("ra"),std::string(""));
                    this->target.dec_dms = params.get(std::string("dec"),std::string(""));
                  }
                  return move_to_target();
                };
      op.params = cmd.params;
    };

    // ---------- SLIT_SET ---------------------------------------------------

    op_builders["slit_set"] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_SLIT_SET;
      op.func = [this,params=cmd.params]() {
                  size_t mode = params.get<size_t>("mode", VSM_DATABASE);
                  return slit_set(static_cast<VirtualSlitMode>(mode));
                };
      op.params = cmd.params;
    };

    // ---------- EXPOSE -----------------------------------------------------

    op_builders["expose"] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_EXPOSURE;
      op.func = [this]() {
                  return do_exposure("operation_builder");
                };
      op.params = cmd.params;
    };

    // ---------- FOCUS_SET --------------------------------------------------

    op_builders["focus_set"] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_FOCUS_SET;
      op.func = [this]() {
                  return focus_set();
                };
      op.params = cmd.params;
    };

  }
  /***** Sequencer::Sequence::init_operation_builders ************************/

}
