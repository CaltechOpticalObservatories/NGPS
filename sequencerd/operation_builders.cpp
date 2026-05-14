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
 *   2. Passthrough : per-daemon translated passthrough. The sequencer DSL form
 *                    "subsystem [do=verb] [key1=val1 ... keyN=valN]" is
 *                    translated to a daemon-native Command{name, arglist} by
 *                    translate_command(), which dispatches to either a
 *                    daemon-specific builder (e.g. build_powerd_command) or
 *                    build_generic_command(). The translated Command is then
 *                    forwarded via CommandClient<State>::send(), which performs
 *                    per-command arg-count and state-machine validation before
 *                    forwarding to the daemon.
 *
 */

#include <algorithm>
#include <initializer_list>
#include <string_view>
#include <stdexcept>

#include "sequence.h"
#include "sequencerd_commands.h"
#include "powerd_commands.h"

namespace Sequencer {

  /***** Sequencer::Sequence::validate_keys **********************************/
  /**
   * @brief      reject any params key not in the allowed whitelist
   * @param[in]  cmd      ParsedCommand whose params are being checked
   * @param[in]  allowed  initializer list of permitted key names
   * @throws     std::runtime_error if any key in cmd.params is not in allowed
   * @details    Called from daemon-specific builders after the verb and
   *             required keys have been matched. Ensures that DSL lines like
   *             "power do=set name=A state=ON garbage=oops" are rejected
   *             rather than silently forwarded.
   *
   */
  void Sequence::validate_keys( const ParsedCommand &cmd,
                                std::initializer_list<std::string_view> allowed ) {
    for (const auto &[key, val] : cmd.params) {
      if (std::find(allowed.begin(), allowed.end(),
                    std::string_view{key}) == allowed.end()) {
        throw std::runtime_error(
          "subsystem '"+cmd.subsystem+"': unexpected key '"+key+"'");
      }
    }
  }
  /***** Sequencer::Sequence::validate_keys **********************************/


  /***** Sequencer::Sequence::build_generic_command **************************/
  /**
   * @brief      default ParsedCommand -> Command translation
   * @param[in]  cmd  ParsedCommand to translate
   * @return     Command{ name, arglist } ready for CommandClient::send()
   * @throws     std::runtime_error if cmd does not match one of the two
   *             supported generic forms
   * @details    Two cases only:
   *               1. verb non-empty, params empty
   *                    -> Command{ verb, {} }
   *                    e.g. "acam do=isopen"          -> isopen
   *               2. verb empty, params has exactly one key=val
   *                    -> Command{ key, { val } }
   *                    e.g. "camera exptime=30000"    -> exptime 30000
   *             Any other shape (verb + params, multiple params, etc.) is
   *             a translation error; add a daemon-specific builder.
   *
   */
  Command Sequence::build_generic_command( const ParsedCommand &cmd ) {
    if (!cmd.verb.empty() && cmd.params.size() == 0) {
      return Command{ cmd.verb, {} };
    }
    if (cmd.verb.empty() && cmd.params.size() == 1) {
      const auto &[key, val] = *cmd.params.begin();
      return Command{ key, { val } };
    }
    throw std::runtime_error(
      "subsystem '"+cmd.subsystem+"': cannot build command"
      " (verb='"+cmd.verb+"', "+std::to_string(cmd.params.size())+" params)"
      " -- add a daemon-specific translator");
  }
  /***** Sequencer::Sequence::build_generic_command **************************/


  /***** Sequencer::Sequence::build_powerd_command ***************************/
  /**
   * @brief      powerd-specific ParsedCommand -> Command translation
   * @param[in]  cmd  ParsedCommand to translate
   * @return     Command{ name, arglist } in powerd native positional syntax
   * @throws     std::runtime_error if required keys are missing or unknown
   *             keys are present
   * @details    Powerd accepts the following DSL forms:
   *               power do=set name=<plug>  state=<ON|OFF|BOOT>
   *               power do=set unit=<u> plug=<p> state=<ON|OFF|BOOT>
   *               power do=get name=<plug>
   *               power do=get unit=<u> plug=<p>
   *             All other powerd subcommands (open, close, list, status,
   *             isopen, reopen) fall through to build_generic_command().
   *
   */
  Command Sequence::build_powerd_command( const ParsedCommand &cmd ) {
    if (cmd.verb == POWERD_SET) {
      if (cmd.params.contains("name") && cmd.params.contains("state")) {
        validate_keys(cmd, {"name", "state"});
        return Command{ POWERD_SET,
                        { cmd.params.at("name"), cmd.params.at("state") } };
      }
      if (cmd.params.contains("unit") &&
          cmd.params.contains("plug") &&
          cmd.params.contains("state")) {
        validate_keys(cmd, {"unit", "plug", "state"});
        return Command{ POWERD_SET,
                        { cmd.params.at("unit"),
                          cmd.params.at("plug"),
                          cmd.params.at("state") } };
      }
      throw std::runtime_error(
        "powerd "+POWERD_SET+": requires name+state or unit+plug+state");
    }
    if (cmd.verb == POWERD_GET) {
      if (cmd.params.contains("name")) {
        validate_keys(cmd, {"name"});
        return Command{ POWERD_GET, { cmd.params.at("name") } };
      }
      if (cmd.params.contains("unit") && cmd.params.contains("plug")) {
        validate_keys(cmd, {"unit", "plug"});
        return Command{ POWERD_GET,
                        { cmd.params.at("unit"), cmd.params.at("plug") } };
      }
      throw std::runtime_error(
        "powerd "+POWERD_GET+": requires name or unit+plug");
    }
    return build_generic_command(cmd);
  }
  /***** Sequencer::Sequence::build_powerd_command ***************************/


  /***** Sequencer::Sequence::translate_command ******************************/
  /**
   * @brief      central ParsedCommand -> Command dispatcher
   * @param[in]  cmd  ParsedCommand from the DSL parser
   * @return     Command in daemon native syntax
   * @throws     std::runtime_error from the selected builder
   * @details    Dispatches to a daemon-specific builder when one exists;
   *             otherwise falls through to build_generic_command(). Plain
   *             if/else dispatch -- no registry, no table.
   *
   */
  Command Sequence::translate_command( const ParsedCommand &cmd ) {
    if (cmd.subsystem == SEQUENCERD_POWER) {
      return build_powerd_command(cmd);
    }
    return build_generic_command(cmd);
  }
  /***** Sequencer::Sequence::translate_command ******************************/


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

    // per-daemon translated passthrough (inline, no Ops method).
    // Command names come from SEQUENCERD_* constants in common/sequencerd_commands.h.
    // translate_command() converts the DSL ParsedCommand into a daemon-native
    // Command{name, arglist}, which is then forwarded via
    // CommandClient<State>::send(). The client performs per-command arg-count
    // validation and state-machine validation before forwarding to the daemon.

    op_builders[SEQUENCERD_ACAM] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_ACAM_INIT;  // passthrough uses nearest daemon THR_ identity
      op.func = [this, cmd_copy=cmd]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_ACAM+"]");
        try {
          return this->acamd_cmd.send(translate_command(cmd_copy));
        }
        catch (const std::runtime_error &e) {
          logwrite(function, "ERROR "+std::string(e.what()));
          return ERROR;
        }
      };
    };

    op_builders[SEQUENCERD_CALIB] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_CALIB_INIT;
      op.func = [this, cmd_copy=cmd]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_CALIB+"]");
        try {
          return this->calibd_cmd.send(translate_command(cmd_copy));
        }
        catch (const std::runtime_error &e) {
          logwrite(function, "ERROR "+std::string(e.what()));
          return ERROR;
        }
      };
    };

    op_builders[SEQUENCERD_CAMERA] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_CAMERA_INIT;
      op.func = [this, cmd_copy=cmd]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_CAMERA+"]");
        try {
          return this->camerad_cmd.send(translate_command(cmd_copy));
        }
        catch (const std::runtime_error &e) {
          logwrite(function, "ERROR "+std::string(e.what()));
          return ERROR;
        }
      };
    };

    op_builders[SEQUENCERD_FLEXURE] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_FLEXURE_INIT;
      op.func = [this, cmd_copy=cmd]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_FLEXURE+"]");
        try {
          return this->flexured_cmd.send(translate_command(cmd_copy));
        }
        catch (const std::runtime_error &e) {
          logwrite(function, "ERROR "+std::string(e.what()));
          return ERROR;
        }
      };
    };

    op_builders[SEQUENCERD_FOCUS] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_FOCUS_INIT;
      op.func = [this, cmd_copy=cmd]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_FOCUS+"]");
        try {
          return this->focusd_cmd.send(translate_command(cmd_copy));
        }
        catch (const std::runtime_error &e) {
          logwrite(function, "ERROR "+std::string(e.what()));
          return ERROR;
        }
      };
    };

    op_builders[SEQUENCERD_POWER] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_POWER_INIT;
      op.func = [this, cmd_copy=cmd]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_POWER+"]");
        try {
          return this->powerd_cmd.send(translate_command(cmd_copy));
        }
        catch (const std::runtime_error &e) {
          logwrite(function, "ERROR "+std::string(e.what()));
          return ERROR;
        }
      };
    };

    op_builders[SEQUENCERD_SLICECAM] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_SLICECAM_INIT;
      op.func = [this, cmd_copy=cmd]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_SLICECAM+"]");
        try {
          return this->slicecamd_cmd.send(translate_command(cmd_copy));
        }
        catch (const std::runtime_error &e) {
          logwrite(function, "ERROR "+std::string(e.what()));
          return ERROR;
        }
      };
    };

    op_builders[SEQUENCERD_SLIT] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_SLIT_INIT;
      op.func = [this, cmd_copy=cmd]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_SLIT+"]");
        try {
          return this->slitd_cmd.send(translate_command(cmd_copy));
        }
        catch (const std::runtime_error &e) {
          logwrite(function, "ERROR "+std::string(e.what()));
          return ERROR;
        }
      };
    };

    op_builders[SEQUENCERD_TCS] = [this](Operation &op, const ParsedCommand &cmd) {
      op.thr  = THR_TCS_INIT;
      op.func = [this, cmd_copy=cmd]() {
        const std::string function("Sequencer::Sequence::op_builders["+SEQUENCERD_TCS+"]");
        try {
          return this->tcsd_cmd.send(translate_command(cmd_copy));
        }
        catch (const std::runtime_error &e) {
          logwrite(function, "ERROR "+std::string(e.what()));
          return ERROR;
        }
      };
    };

  }
  /***** Sequencer::Sequence::init_operation_builders ************************/

}
