/**
 * @file     command.h
 * @brief    header-only library for handling commands to daemons
 * @details  This provides a wrapper to form command and arg list strings.
 *           Also a wrapper that stores transition states, and validates that
 *           a command is allowed to be sent while in the current state.
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

/***** Sequencer **************************************************************/
/**
 * @brief     namespace for the observation sequencer
 *
 */
namespace Sequencer {

  /**
   * @brief  wrapper to form string from command and arglist
   */
  struct Command {
    std::string name;
    std::vector<std::string> arglist;

    std::string str() const {
      std::string strung = name;
      for (const auto &arg : arglist) {
        strung += " " + arg;
      }
      return strung;
    }
  };


  /**
   * @brief  command specs right now just holds min/max number of allowed args
   */
  struct CommandSpec {
    int min_args;
    int max_args;
  };

  using CommandSpecMap = std::unordered_map<std::string, CommandSpec>;


  /**
   * @brief  structure contains command and states it can transition from->to
   */
  template <typename State>
  struct Transition {
    State from;
    std::string command;
    State to;
  };


  /**
   * @brief    contains the functionality of the library
   * @details  This pairs specs and transitions with a command and holds
   *           the client object used to communiate with the daemon.
   */
  template <typename State>
  class CommandClient {
    private:
      Common::DaemonClient &client;
      const CommandSpecMap &specs;
      State state;
      const std::vector<Transition<State>> &transitions;
      const std::unordered_map<State, std::string> &state_names;

      /**
       * @brief      rejects cmd if its name is unknown or its arg count is out of range
       * @param[in]  cmd  command to check
       * @throws     std::runtime_error on unknown command name or arg count out of range
       */
      void validate_args( const Command &cmd ) const {
        auto it = specs.find( cmd.name );
        if (it == specs.end()) throw std::runtime_error("unknown command: "+cmd.name);
        int nargs = cmd.arglist.size();
        if (nargs < it->second.min_args || nargs > it->second.max_args) {
          throw std::runtime_error("invalid arg count for "+cmd.name);
        }
      }

      /**
       * @brief      rejects cmd if it is not permitted from the current state
       * @param[in]  cmd  command to check
       * @throws     std::runtime_error
       */
      void validate_order( const Command &cmd ) const {
        for (const auto &transition : transitions) {
          if (transition.from == state && transition.command == cmd.name) return;
        }
        // not valid from current state; collect all valid from-states for the error message
        auto name_of = [&](State s) -> std::string {
          auto it = state_names.find(s);
          return (it != state_names.end()) ? it->second : "?";
        };
        std::string valid;
        for (const auto &transition : transitions) {
          if (transition.command == cmd.name) {
            if (!valid.empty()) valid += ", ";
            valid += name_of(transition.from);
          }
        }
        if (valid.empty()) valid = "(none registered)";
        throw std::runtime_error(
          "invalid command order: '"+cmd.name+"' not valid in state "
          +name_of(state)+"; valid from: {"+valid+"}");
      }

      /**
       * @brief      advances internal state after a successful send
       * @param[in]  cmd  command that was just forwarded to the daemon
       * @details    Searches the transition table for a matching (from==state,
       *             command==cmd.name) entry and sets state to transition.to.
       *             No-ops silently when no matching transition exists.
       */
      void advance_state( const Command &cmd ) {
        for (const auto &transition : transitions) {
          if (transition.from == state && transition.command == cmd.name) {
            state = transition.to;
            return;
          }
        }
      }

    public:
      /**
       * @brief      constructs a CommandClient bound to a daemon connection
       * @param[in]  client         DaemonClient used to forward wire commands
       * @param[in]  specs          per-command arg-count constraints
       * @param[in]  initial_state  starting state of the state machine
       * @param[in]  transitions    valid (from, command, to) state transitions
       * @param[in]  state_names    human-readable names for each State value, used in errors
       */
      CommandClient( Common::DaemonClient &client,
                     const CommandSpecMap &specs,
                     State initial_state,
                     const std::vector<Transition<State>> &transitions,
                     const std::unordered_map<State, std::string> &state_names )
        : client(client),
          specs(specs),
          state(initial_state),
          transitions(transitions),
          state_names(state_names) { }

      /**
       * @brief      primary interface to sending commands
       * @details    This validates the number of args, validates the
       *             transition state, that this command is allowed to be used
       *             in the current state, and sends the command.
       * @param[in]  cmd  Command struct contains command and arglist
       * @return     return value from the client
       *
       */
      long send( const Command &cmd ) {
        validate_args( cmd );
        validate_order( cmd );
        long ret = client.command( cmd.str() );
        advance_state( cmd );
        return ret;
      }

      /** @brief  returns the current state-machine state */
      State get_state() const { return state; }
  };
}
/***** Sequencer **************************************************************/
