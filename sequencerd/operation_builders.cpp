/**
 * @file    operation_builders.cpp
 * @brief   implementation for building operations from command names
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file registers the mapping from a parsed sequencer command name to an
 * OperationBuilder lambda. Builders fall into two categories:
 *
 *   1. Internal    : sequencer-level orchestration (database-driven, may span
 *                    multiple daemons). These delegate to Ops:: factory methods
 *                    which wrap Sequence:: member functions. Parameters are
 *                    carried as key=value pairs on cmd.params and are exposed to
 *                    the member function via Sequence::current_op_params (set by
 *                    Sequence::run() immediately before op.func() is invoked).
 *
 *   2. Passthrough : per-daemon validated passthrough (e.g. "camera exptime 30000"
 *                    or "slit set 2.0 0.0"). These are built inline here (no Ops::
 *                    method) and forward the positional args (cmd.args) to the
 *                    target daemon. The first arg is the daemon subcommand; the
 *                    remainder are forwarded as-is in daemon native syntax.
 *
 * NOTE: Passthrough commands currently forward via Common::DaemonClient::send() as a
 *       PLACEHOLDER. This will be replaced by CommandClient<State>::send()
 *       later, at which point per-command state-machine validation and
 *       arg-count checks will be performed at execution time.
 *
 */

#include "sequence.h"
#include "sequencerd_commands.h"

namespace Sequencer {

  namespace {

    /***** Sequencer::daemon_passthrough *************************************/
    /**
     * @brief      TEMPORARY daemon passthrough helper
     * @param[in]  function  calling function name for logging
     * @param[in]  daemon    DaemonClient to forward to
     * @param[in]  args      positional args from ParsedCommand (first is subcommand)
     * @return     ERROR|NO_ERROR
     * @details    Builds a daemon-native command string from args[0..N-1] and
     *             forwards via Common::DaemonClient::send(). Self-identifies in
     *             log as a placeholder so the pending replacement is unambiguous.
     *
     * TEMPORARY -- REMOVE ONCE CommandClient<State> IS WIRED IN (Steps 2+3).
     *              Each Passthrough op_builder lambda will then call
     *              CommandClient<State>::send() directly, performing per-command
     *              state-machine validation and arg-count checks at execution
     *              time, and this helper becomes unnecessary.
     *
     */
    long daemon_passthrough( const std::string &function,
                             Common::DaemonClient &daemon,
                             const std::vector<std::string> &args ) {
      if (args.empty()) {
        logwrite(function, "ERROR no subcommand provided to "+daemon.name);
        return ERROR;
      }
      std::string cmd_str = args[0];
      for (size_t i=1; i<args.size(); ++i) cmd_str += " " + args[i];
      std::string reply;
      logwrite(function, "PLACEHOLDER passthrough (DaemonClient -> "+daemon.name+
                         "): "+cmd_str);
      long error = daemon.send(cmd_str, reply);
      if (error != NO_ERROR) {
        logwrite(function, "ERROR forwarding \""+cmd_str+"\" to "+daemon.name+": "+reply);
      }
      return error;
    }
    /***** Sequencer::daemon_passthrough *************************************/

  }  // anonymous namespace


  /***** Sequencer::Sequence::init_operation_builders ************************/
  /**
   * @brief      initializes registration between name and operation
   * @details    Maps command names to functions that populate an Operation
   *             from a ParsedCommand.
   *
   */
  void Sequence::init_operation_builders() {

    // ---------- INTERNAL ------------------------------------------------------

    // sequencer-level orchestration (Ops factory methods)
    // Command names come from the THR_ -> thread_names map in sequence.h.
    // I probably should rename these from "thread" to something else (operation?)

    op_builders[thread_names.at(THR_ACAM_INIT)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.acam_init();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_CALIB_INIT)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.calib_init();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_CALIB_SET)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.calib_set();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_CAMERA_INIT)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.camera_init();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_CAMERA_SET)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.camera_set();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_EXPOSURE)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.do_expose();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_FLEXURE_INIT)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.flexure_init();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_FLEXURE_SET)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.flexure_set();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_FOCUS_INIT)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.focus_init();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_FOCUS_SET)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.focus_set();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_MOVE_TO_TARGET)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.move_to_target();
      op.params = cmd.params;   // may carry optional ra= / dec= overrides
    };

    op_builders[thread_names.at(THR_POWER_INIT)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.power_init();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_REPEAT_EXPOSURE)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.repeat_exposure();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_SHUTDOWN)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.do_shutdown();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_SLICECAM_INIT)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.slicecam_init();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_SLIT_INIT)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.slit_init();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_SLIT_SET)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.slit_set();
      op.params = cmd.params;   // may carry optional mode= override
    };

    op_builders[thread_names.at(THR_STARTUP)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.do_startup();
      op.params = cmd.params;
    };

    op_builders[thread_names.at(THR_TCS_INIT)] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.tcs_init();
      op.params = cmd.params;
    };

    // target_offset shares THR_MOVE_TO_TARGET with move_to_target, so it needs a
    // distinct CLI/DSL name. Use its Sequence:: member-function name "target_offset"
    // (there is no separate entry in thread_names for it).
    op_builders["target_offset"] = [this](Operation &op, const ParsedCommand &cmd) {
      op = ops.target_offset();
      op.params = cmd.params;
    };

    // ---------- PASSTHROUGH ---------------------------------------------------

    // per-daemon validated passthrough (inline, no Ops method).
    // Command names come from SEQUENCERD_* constants in common/sequencerd_commands.h.
    // PLACEHOLDER: forwards via Common::DaemonClient::send() until Steps 2+3
    // replace with CommandClient<State>::send() (state-machine validated).

    op_builders[SEQUENCERD_ACAM] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_ACAM_INIT;  // passthrough uses nearest daemon THR_ identity
      op.func = [this, args=cmd.args]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_ACAM+"]");
        return daemon_passthrough(function, this->acamd, args);
      };
    };

    op_builders[SEQUENCERD_CALIB] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_CALIB_INIT;
      op.func = [this, args=cmd.args]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_CALIB+"]");
        return daemon_passthrough(function, this->calibd, args);
      };
    };

    op_builders[SEQUENCERD_CAMERA] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_CAMERA_INIT;
      op.func = [this, args=cmd.args]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_CAMERA+"]");
        return daemon_passthrough(function, this->camerad, args);
      };
    };

    op_builders[SEQUENCERD_FLEXURE] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_FLEXURE_INIT;
      op.func = [this, args=cmd.args]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_FLEXURE+"]");
        return daemon_passthrough(function, this->flexured, args);
      };
    };

    op_builders[SEQUENCERD_FOCUS] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_FOCUS_INIT;
      op.func = [this, args=cmd.args]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_FOCUS+"]");
        return daemon_passthrough(function, this->focusd, args);
      };
    };

    op_builders[SEQUENCERD_POWER] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_POWER_INIT;
      op.func = [this, args=cmd.args]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_POWER+"]");
        return daemon_passthrough(function, this->powerd, args);
      };
    };

    op_builders[SEQUENCERD_SLICECAM] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_SLICECAM_INIT;
      op.func = [this, args=cmd.args]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_SLICECAM+"]");
        return daemon_passthrough(function, this->slicecamd, args);
      };
    };

    op_builders[SEQUENCERD_SLIT] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_SLIT_INIT;
      op.func = [this, args=cmd.args]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_SLIT+"]");
        return daemon_passthrough(function, this->slitd, args);
      };
    };

    op_builders[SEQUENCERD_TCS] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_TCS_INIT;
      op.func = [this, args=cmd.args]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_TCS+"]");
        return daemon_passthrough(function, this->tcsd, args);
      };
    };

  }
  /***** Sequencer::Sequence::init_operation_builders ************************/

}
