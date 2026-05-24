/** ---------------------------------------------------------------------------
 * @file    flexure_interface.h
 * @brief   flexure interface include
 * @details defines the classes used by the flexure hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "flexured_commands.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>
#include <cmath>

#define FLEXURE_MOVE_TIMEOUT      1000       ///< timeout in msec for moves
#define FLEXURE_POSNAME_TOLERANCE    0.0001  ///< tolerance to determine posname from position

/***** Flexure ****************************************************************/
/**
 * @namespace Flexure
 * @brief     namespace for the flexure daemon
 *
 */
namespace Flexure {

  const std::string DAEMON_NAME = "flexured";    ///< when run as a daemon, this is my name

  /***** Flexure::Interface ***************************************************/
  /**
   * @class  Interface
   * @brief  interface class for a flexure device
   *
   * This class defines the interface for each flexure controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      zmqpp::context context;
      size_t numdev;
      bool class_initialized;

      /**
       * @struct Status
       * @brief  published flexure state: actuator position (um) by FLX<axis>_<chan> key; NaN if unavailable
       */
      struct Status {
        std::map<std::string,double> positions;
        bool operator==(const Status &o) const {
          if ( positions.size() != o.positions.size() ) return false;
          for ( const auto &[k,v] : positions ) {
            auto it = o.positions.find(k);
            if ( it == o.positions.end() ) return false;
            if ( std::isnan(v) && std::isnan(it->second) ) continue;  // NaN==NaN treated equal
            if ( v != it->second ) return false;
          }
          return true;
        }
        bool operator!=(const Status &o) const { return !(*this == o); }
      };
      Status status;                                   ///< current flexure state
      Status last_published_status;                    ///< last published flexure state

    public:

      Interface()
        : context(),
          numdev(-1),
          is_subscriber_thread_running(false),
          should_subscriber_thread_run(false),
          motorinterface( FLEXURE_MOVE_TIMEOUT, 0, FLEXURE_POSNAME_TOLERANCE )
      {
        topic_handlers = {
          { Topic::SNAPSHOT, std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_snapshot(msg); } ) }
        };
      }

      std::unique_ptr<Common::PubSub> publisher;       ///< publisher object
      std::string publisher_address;                   ///< publish socket endpoint
      std::string publisher_topic;                     ///< my default topic for publishing
      std::unique_ptr<Common::PubSub> subscriber;      ///< subscriber object
      std::string subscriber_address;                  ///< subscribe socket endpoint
      std::vector<std::string> subscriber_topics;      ///< list of topics I subscribe to
      std::atomic<bool> is_subscriber_thread_running;  ///< is my subscriber thread running?
      std::atomic<bool> should_subscriber_thread_run;  ///< should my subscriber thread run?
      std::unordered_map<std::string,
                         std::function<void(const nlohmann::json&)>> topic_handlers;
                                                       ///< maps a handler function to each topic

      long init_pubsub(const std::initializer_list<std::string> &topics={}) {
        if (!subscriber) {
          subscriber = std::make_unique<Common::PubSub>(context, Common::PubSub::Mode::SUB);
        }
        return Common::PubSubHandler::init_pubsub(context, *this, topics);
      }
      void start_subscriber_thread() { Common::PubSubHandler::start_subscriber_thread(*this); }
      void stop_subscriber_thread()  { Common::PubSubHandler::stop_subscriber_thread(*this); }

      void handletopic_snapshot( const nlohmann::json &jmessage );  ///< respond to a snapshot request
      void get_status();                                            ///< refresh status from hardware
      void publish_status( bool force=false );                      ///< publish flexure state on change (or force)

      Common::Queue async;

      // PI Interface class for the Piezo type
      //
      Physik_Instrumente::Interface<Physik_Instrumente::PiezoInfo> motorinterface;

      long initialize_class();
      long open();                               ///< opens the PI socket connection
      long close();                              ///< closes the PI socket connection
      long is_open( std::string arg, std::string &retstring );     ///< are motor controllers connected?

      long set( std::string args, std::string &retstring ); ///< set the slit width and offset
      long get( std::string args, std::string &retstring ); ///< get the current width and offset
      long compensate( std::string args, std::string &retstring );  ///< perform flexure compensation

      long stop();                               ///< send the stop-all-motion command to all controllers
      long send_command( const std::string &name, std::string cmd );      ///< writes the raw command as received to the master controller, no reply
      long send_command( const std::string &name, std::string cmd, std::string &retstring );                 ///< writes command?, reads reply
      long test( std::string args, std::string &retstring );                 ///< test routines

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads
      std::mutex wait_mtx;                       ///< mutex object for waiting for threads
      std::condition_variable cv;                ///< condition variable for waiting for threads

  };
  /***** Flexure::Interface ***************************************************/

}
/***** Flexure ****************************************************************/
