/**
 * @file    sequence_builder.cpp
 * @brief   implementation for building sequences from operations
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "sequence.h"

namespace Sequencer {

  /***** Sequencer::Sequence::build_sequence *********************************/
  /**
   * @brief      build a sequence from parsed commands
   * @details    This accepts a vector of ParsedCommands and returns by reference
   *             a sequence, which is one or more OperationGroups. Accepts special
   *             ".dsl" commands for the Sequencer Domain Specific Language.
   *             Start a group with "parallel:" or "serial:", finish group
   *             with "end". Include "on_error stop|continue" to set behavior
   *             of that group when an error occurs in an operation in that
   *             group.
   * @param[in]  commands      vector of ParsedCommands
   * @param[out] sequence_out  vector of Operation Groups (sequence) to execute
   * @return     ERROR|NO_ERROR|ABORT
   *
   */
  long Sequence::build_sequence(const std::vector<ParsedCommand> &commands,
                                std::vector<OperationGroup> &sequence_out) {
    const std::string function("Sequencer::Sequence::build_sequence");

    // local failure-reporting wrapper
    auto fail = [&](const ParsedCommand &cmd, const std::string &msg) {
      logwrite(function, "ERROR line "+std::to_string(cmd.linenum)+": "+msg);
      return ERROR;
    };

    // create temporay group of operations currently being processed
    std::optional<OperationGroup> current_group;

    // empty the sequence
    sequence_out.clear();

    // loop through vector of commands
    //
    for (const auto &command : commands) {

      const std::string &name = command.name;

      // ---------- SPECIAL COMMAND { parallel: | serial: } ------------------

      // start a group
      //
      if (name == "parallel:" || name == "serial:") {

        if (current_group) return fail(command, "group already open");

        // create a new group
        OperationGroup new_group;
        new_group.on_error = OnError::STOP;
        new_group.type = (name=="parallel:") ? OperationType::PARALLEL
                                             : OperationType::SERIAL;
        current_group = std::move(new_group);
        continue;
      }

      // ---------- SPECIAL COMMAND { end } ----------------------------------

      // finish a group
      //
      if (name == "end") {
        if (!current_group) return fail(command, "unexpected 'end' outside group");

        // add it to the sequence now
        sequence_out.push_back(std::move(*current_group));

        current_group.reset();
        continue;
      }

      // ---------- SPECIAL COMMAND { on_error } -----------------------------

      // set how does the group behave on error
      //
      if (name == "on_error") {
        // must be in a group for on_error to mean anything
        if (!current_group) return fail(command, "'on_error' out of a group");
        if (!command.params.has("action")) return fail(command, "on_error requires action=stop|continue");

        const std::string val = command.params.get<std::string>("action","");

        if (val == "stop") current_group->on_error = OnError::STOP;
        else
        if (val == "continue") current_group->on_error = OnError::CONTINUE;
        else {
          return fail(command, "on_error requires action=stop|continue");
        }
        continue;
      }

      // ---------- MAKE OPERATION FOR GIVEN COMMAND -------------------------

      // anything other than { parallel | serial | end | on_error } is a
      // command to parse, so a group must have been started first.
      //
      if (!current_group) return fail(command, "command outside group");

      // find the command in the operation builders map
      //
      auto it = op_builders.find(name);

      if (it == op_builders.end()) return fail(command, "unrecognized command");

      Operation &op = current_group->operations.emplace_back();

      it->second(op, command);
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::build_sequence *********************************/

}
