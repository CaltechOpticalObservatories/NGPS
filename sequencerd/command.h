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

      void validate_args( const Command &cmd ) const {
        auto it = specs.find( cmd.name );
        if (it == specs.end()) throw std::runtime_error("unknown command: "+cmd.name);
        int nargs = cmd.arglist.size();
        if (nargs < it->second.min_args || nargs > it->second.max_args) {
          throw std::runtime_error("invalid arg count for "+cmd.name);
        }
      }

      void validate_order( const Command &cmd ) const {
        for (const auto &transition : transitions) {
          if (transition.from == state && transition.command == cmd.name) {
            return;
          }
        }
        throw std::runtime_error("invalid command order: "+cmd.name);
      }

      void advance_state( const Command &cmd ) {
        for (const auto &transition : transitions) {
          if (transition.from == state && transition.command == cmd.name) {
            state = transition.to;
            return;
          }
        }
      }

    public:
      CommandClient( Common::DaemonClient &client,
                     const CommandSpecMap &specs,
                     State initial_state,
                     const std::vector<Transition<State>> &transitions )
        : client(client),
          specs(specs),
          state(initial_state),
          transitions(transitions) { }

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

      State get_state() const { return state; }
  };
}
/***** Sequencer **************************************************************/
