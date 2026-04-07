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
   * @param[in]  commands      vector of ParsedCommands
   * @param[out] sequence_out  the operation group to execute
   * @return     ERROR|NO_ERROR|ABORT
   *
   */
  long Sequence::build_sequence(const std::vector<ParsedCommand> &commands,
                                std::vector<OperationGroup> &sequence_out) {
    OperationGroup group;

    group.type = OperationType::SERIAL;  // default is serial

    for (const auto &command : commands) {

      if (command.name == "begin_parallel") {
        if (!group.operations.empty()) sequence_out.push_back(group);
        group = { OperationType::PARALLEL, {} };
        continue;
      }
      else

      if (command.name == "end_parallel") {
        sequence_out.push_back(group);
        group = { OperationType::SERIAL, {} };
        continue;
      }
      else

      if (command.name == "move_to_target") {
        group.operations.emplace_back( Operation {
          "move_to_target", THR_MOVE_TO_TARGET,
          [this,params=command.params]() {
            if (params.has("ra") && params.has("dec")) {
              this->target.ra_hms = params.get(std::string("ra"),std::string(""));
              this->target.dec_dms = params.get(std::string("dec"),std::string(""));
            }
            return move_to_target();
          },
          command.params
        });
      }
      else

      if (command.name == "slit_set") {
        group.operations.emplace_back( Operation {
          "slit_set", THR_SLIT_SET,
          [this,params=command.params]() {
            size_t mode = params.get<size_t>("mode", VSM_DATABASE);
            return slit_set(static_cast<VirtualSlitMode>(mode));
          },
          command.params
        });
      }

      else

      if (command.name == "expose") {
        group.operations.emplace_back( Operation {
          "expose", THR_EXPOSURE,
          [this]() {
            return do_exposure("placeholder");
          },
          {}
        });
      }

      else {
        this->async.enqueue_and_log("Sequencer::Sequence::build_sequence",
                                    "ERROR unknown command '"+command.name+"'");
      }
    }
    if (!group.operations.empty()) sequence_out.push_back(group);

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::build_sequence *********************************/

}
