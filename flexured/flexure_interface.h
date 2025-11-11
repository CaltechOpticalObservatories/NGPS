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
#include "tcs_info.h"
#include "flexured_commands.h"
#include "flexure_compensator.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

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

    public:
      Interface() :
        context(),
        numdev(-1),
        motorinterface(FLEXURE_MOVE_TIMEOUT, 0, FLEXURE_POSNAME_TOLERANCE),
        subscriber(std::make_unique<Common::PubSub>(context, Common::PubSub::Mode::SUB)),
        is_subscriber_thread_running(false),
        should_subscriber_thread_run(false),
        tcs_snapshot_status(false),
        tcs_info(),
        compensator(tcs_info)
      {
        topic_handlers = {
          { "_snapshot", std::function<void(const nlohmann::json&)>(
              [this](const nlohmann::json &msg) { handletopic_snapshot(msg); } ) },
          { "tcsd", std::function<void(const nlohmann::json&)>(
              [this](const nlohmann::json &msg) { handletopic_tcsd(msg); } ) }
        };
      }

      // PI Interface class for the Piezo type
      //
      Physik_Instrumente::Interface<Physik_Instrumente::PiezoInfo> motorinterface;

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

      bool tcs_snapshot_status;
      std::mutex snapshot_mutex;

      // publish/subscribe functions
      //
      long init_pubsub(const std::initializer_list<std::string> &topics={}) {
        return Common::PubSubHandler::init_pubsub(context, *this, topics);
      }
      void start_subscriber_thread() { Common::PubSubHandler::start_subscriber_thread(*this); }
      void stop_subscriber_thread()  { Common::PubSubHandler::stop_subscriber_thread(*this); }

      void handletopic_snapshot( const nlohmann::json &jmessage );
      void handletopic_tcsd( const nlohmann::json &jmessage );

      void publish_snapshot();
      void request_tcs_snapshot();
      bool wait_for_tcs_snapshot();

      std::map<std::string, int> telemetry_providers;  ///< map of port[daemon_name] for external telemetry providers

      Common::Queue async;

      TcsInfo tcs_info;                          ///< defined in tcs_info.h

      Compensator compensator;

      long initialize_class();
      long open();                               ///< opens the PI socket connection
      long close();                              ///< closes the PI socket connection
      long is_open( std::string arg, std::string &retstring );     ///< are motor controllers connected?

      long set( std::string args, std::string &retstring ); ///< set the slit width and offset
      long get( std::string args, std::string &retstring ); ///< get the current width and offset
      long compensate( std::string args, std::string &retstring );  ///< perform flexure compensation
      void offset_tiptilt(const std::string &chan, const std::pair<double,double> &delta, bool is_dryrun);

      long stop();                               ///< send the stop-all-motion command to all controllers
      long send_command( const std::string &name, std::string cmd );      ///< writes the raw command as received to the master controller, no reply
      long send_command( const std::string &name, std::string cmd, std::string &retstring );                 ///< writes command?, reads reply
      void make_telemetry_message( std::string &retstring );  ///< assembles a telemetry message
      void get_external_telemetry();                          ///< collect telemetry from other daemon(s)
      long parse_incoming_telemetry( std::string message_in );     ///< parses incoming telemetry messages
      long test( std::string args, std::string &retstring );                 ///< test routines

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads
      std::mutex wait_mtx;                       ///< mutex object for waiting for threads
      std::condition_variable cv;                ///< condition variable for waiting for threads


      void validate_tcs_telemetry() {
        // if TCS telemetry is old then ask it to publish
        //
        if (this->tcs_info.is_older_than(std::chrono::seconds(10))) {
          try {
            this->request_tcs_snapshot();
            this->wait_for_tcs_snapshot();
          }
          catch (const std::exception &e) { throw; }
        }
      }
  };
  /***** Flexure::Interface ***************************************************/

}
/***** Flexure ****************************************************************/
