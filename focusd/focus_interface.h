/** ---------------------------------------------------------------------------
 * @file    focus_interface.h
 * @brief   focus interface include
 * @details defines the classes used by the focus hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "network.h"
#include "galil.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "focusd_commands.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define FOCUS_MOVE_TIMEOUT      5000      ///< timeout in msec for moves
#define FOCUS_HOME_TIMEOUT      5000      ///< timeout in msec for home
#define FOCUS_POSNAME_TOLERANCE    0.001  ///< tolerance to determine posname from position

/***** Focus ******************************************************************/
/**
 * @namespace Focus
 * @brief     namespace for the focus daemon
 *
 */
namespace Focus {

  const std::string DAEMON_NAME = "focusd";      ///< when run as a daemon, this is my name

  /***** Focus::Interface *****************************************************/
  /**
   * @class  Interface
   * @brief  interface class for a focus device
   *
   * This class defines the interface for each focus controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      zmqpp::context context;
      bool class_initialized;
    public:
      Interface()
        : context(),
          subscriber(std::make_unique<Common::PubSub>(context, Common::PubSub::Mode::SUB)),
          is_subscriber_thread_running(false),
          should_subscriber_thread_run(false)
      {
        topic_handlers = {
          { "_snapshot", std::function<void(const nlohmann::json&)>(
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

      Common::Queue async;

      // PI Interface class for the Stepper type
      //
      std::unique_ptr<Physik_Instrumente::Interface<Physik_Instrumente::StepperInfo>> pi_interface;

      // Galil Interface class for the U-channel controller
      //
      std::unique_ptr<Galil::Interface<Galil::SingleAxisInfo>> galil_interface;

      // Container to hold all controllers indexed by name
      //
//    std::map<std::string, MotionController::ControllerInterface*> motors;

      std::map<std::string, MotionController::Name> motors;

      template <typename Function>
      void all_motors(Function f) { for (auto &[name,motor] : motors) f(*motor); };

      // publish/subscribe functions
      //
      long init_pubsub(const std::initializer_list<std::string> &topics={}) {
        return Common::PubSubHandler::init_pubsub(context, *this, topics);
      }
      void start_subscriber_thread() { Common::PubSubHandler::start_subscriber_thread(*this); }
      void stop_subscriber_thread()  { Common::PubSubHandler::stop_subscriber_thread(*this); }

      void handletopic_snapshot( const nlohmann::json &jmessage );

      long open();                                              ///< opens the PI socket connection
      long close();                                             ///< closes the PI socket connection
      long is_open( std::string arg, std::string &retstring );  ///< are motor controllers connected?
      long home( std::string name_in, std::string &retstring ); ///< home all daisy-chained motors
      long is_home( std::string arg, std::string &retstring );  ///< return the home state of the motors

      long set( std::string args, std::string &retstring );     ///< set focus motor position
      long get( std::string name, std::string &retstring );     ///< get focus motor position

      static void dothread_move_abs( Focus::Interface &iface, int addr, float pos ); ///< threaded move_abs function

      long move_abs( int addr, float pos );      ///< send move-absolute command to specified controller
      long move_rel( std::string args );         ///< send move-relative command to specified controller
      long native( std::string args, std::string &retstring );  ///< send native command to a PI controller
      long stop();                               ///< send the stop-all-motion command to all controllers
      long send_command( const std::string &name, std::string cmd );      ///< writes the raw command as received to the master controller, no reply
      long send_command( const std::string &name, std::string cmd, std::string &retstring );  ///< writes command?, reads reply
      void make_telemetry_message( std::string &retstring );  ///< assembles a telemetry message

      long test( std::string args, std::string &retstring );

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads

  };
  /***** Focus::Interface *****************************************************/

}
/***** Focus ******************************************************************/
