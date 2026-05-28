/** ---------------------------------------------------------------------------
 * @file    thermal_interface.h
 * @brief   thermal interface include
 * @details defines the classes used by the temperature controller hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "message_keys.h"
#include "network.h"
#include "logentry.h"
#include "common.h"
#include "thermald_commands.h"
#include "database.h"
#include "utilities.h"
#include <string>
#include <cmath>
#include <ctgmath>
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <thread>

#include <fcntl.h>    // controls like O_RDWR
#include <unistd.h>   // write(), read(), close()
#include <termios.h>  // terminal control options
#include <cstring>    // memset

#include <mysqlx/xdevapi.h>
#include <mysqlx/xapi.h>
#include <json.hpp>

/***** Thermal ****************************************************************/
/**
 * @namespace Thermal
 * @brief     namespace for the thermal daemon
 *
 */
namespace Thermal {

  /***** Thermal::Campbell ****************************************************/
  /**
   * @class  Campbell
   * @brief  Campbell interface class
   *
   * This class defines the interface for a Campbell device.
   *
   */
  class Campbell {
    private:
      int fd;              ///< for serial connection
      std::string device;  ///< tty device name

    public:
      Campbell() : fd(-1) { }

      void set_device( const std::string dev ) { this->device=dev; }

      std::mutex datamap_mtx;
      std::map<int, std::string> sensor_names;       ///< STL map of sensor_names indexed by Campbell channel number
      std::map<std::string, mysqlx::Value> datamap;  ///< STL map of sensor values indexed by sensor name

      long open_device();   ///< open serial connection to CR1000
      long close_device();  ///< close serial connection to CR1000
      long read_data();     ///< trigger CR1000 to send data, then read it
      long read_data(bool showoutliers);     ///< trigger CR1000 to send data, then read it
  };
  /***** Thermal::Campbell ****************************************************/


  /***** Thermal::Lakeshore ***************************************************/
  /**
   * @class  Lakeshore
   * @brief  Lakeshore interface class
   *
   * This class defines the interface for a Lakeshore device.
   *
   */
  class Lakeshore {
    public:
      std::map<std::string, std::string> temp_info;  ///< STL map of temp labels indexed by channel
      std::map<std::string, std::string> heat_info;  ///< STL map of heater labels indexed by channel

      Network::Interface *lks;                       ///< pointer to object for interfacing with the Lakeshore

      inline std::string device_name() { return this->lks->get_name(); }  ///< return Lakeshore device name
      inline long open() { return this->lks->open(); }                    ///< open Lakeshore device
      inline long close() { return this->lks->close(); }                  ///< close Lakeshore device

      long read_temp( std::string chan, float &tempval );                ///< read specified temperature channel and return value
      long read_heat( std::string chan, float &heat );                   ///< read specified heater and return value
      long set_setpoint( int output, float setpoint );                   ///< set a setpoint
      long get_setpoint( int output, float &setpoint );                  ///< get a setpoint

  };
  /***** Thermal::Lakeshore ***************************************************/


  /***** Thermal::Interface ***************************************************/
  /**
   * @class  Interface
   * @brief  interface class for a thermal device
   *
   * This class defines the interface for each temperature controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      zmqpp::context context;

    public:
      Interface()
        : context(),
          is_subscriber_thread_running(false),
          should_subscriber_thread_run(false)
      {
        topic_handlers = {
          { Topic::SNAPSHOT, std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_snapshot(msg); } ) },
          { Topic::ACAMD_TEMP, std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_acamd(msg); } ) },
          { Topic::SLICECAMD, std::function<void(const nlohmann::json&)>(
                     [this](const nlohmann::json &msg) { handletopic_slicecamd(msg); } ) }
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
      void handletopic_acamd( const nlohmann::json &jmessage );     ///< stash acam CCD temperature into externaldata
      void handletopic_slicecamd( const nlohmann::json &jmessage ); ///< stash slicecam CCD temperatures into externaldata
      void publish_status();                                        ///< publish thermalinfo on Topic::THERMALD
      void request_snapshot();                                      ///< ask subscribed daemons to publish their status

      Common::Queue async;

      std::map<int, Thermal::Lakeshore> lakeshore;      ///< STL map of all Lakeshores indexed by LKS#

      Thermal::Campbell campbell;                       ///< Campbell object for datalogger

      std::mutex lakeshoredata_mtx;
      std::mutex telemdata_mtx;
      std::mutex externaldata_mtx;

      std::map<std::string, mysqlx::Value> lakeshoredata;  ///< map of Lakeshore readings indexed by label
      std::map<std::string, mysqlx::Value> campbelldata;   ///< map of Campbell readings indexed by label
      std::map<std::string, mysqlx::Value> telemdata;      ///< map of all readings (merge lakeshore+campbell)

      std::map<std::string, mysqlx::Value> externaldata;   ///< map of telemetry received from other daemons

      /**
       * @brief      save a key=value pair to the externaldata map
       * @details    The template allows the compiler to automatically deduce
       *             the type of the value and store it in the externaldata
       *             map. The mysqlx::Value element supports multiple types and
       *             requires correct type assignment.
       * @param[in]  key    jmessage key
       * @param[in]  value  any type of value
       */
      template <typename T>
      void save_to_externaldata( const std::string &key, const T &value ) {
        this->externaldata[key] = value;
      }

      /**
       * @brief      verifies key before saving to externaldata map
       * @param[in]  jmessage  json message
       * @param[in]  key       key to save
       */
      template <typename T>
      void process_key( const nlohmann::json &jmessage, const std::string &key ) {
        if ( jmessage.contains(key) ) {
          if ( !jmessage[key].is_null() ) {
            try {
              this->save_to_externaldata( key, jmessage[key].get<T>() );
            }
            catch ( const nlohmann::json::type_error &e ) {
              logwrite( "Thermal::Interface::process_key", "ERROR key \""+key+"\" not expected type: "+e.what() );
            }
          }
          else {
            logwrite( "Thermal::Interface::process_key", "ERROR bad key \""+key+"\"" );
          }
        }
      }

      /**
       * @typedef thermal_t
       * @brief   structure of thermal data database
       */
      typedef struct {
        std::string device;                         ///< device type e.g. LKS, CAMP, etc.
        int unit;                                   ///< user-assigned unit number
        std::string name;                           ///< user-assigned name
        std::string chan;                           ///< channel number e.g. A, B, C1, H1, 1, etc.
        std::string label;                          ///< channel label (must be unique for indexing by label)
      } thermal_info_t;

      std::map<std::string, thermal_info_t> thermal_info;   ///< thermal info database, indexed by channel label

      long reconnect( std::string args, std::string &retstring );  ///< close,open all hardware devices

      /**
       * Campbell functions
       */
      long open_campbell();
      long close_campbell();

      /**
       * Lakeshore functions
       */
      long open_lakeshores();
      long close_lakeshores();
      long parse_unit_chan( std::string args, int &unit, std::string &chan );
      long lakeshore_readall( );  ///< read all Lakeshores into memory
      long get( std::string args, std::string &retstring );       ///< read specified channel from specified LKS unit
      long print_labels( std::string args, std::string &retstring );     ///< print lakeshore channel labels
      long show_telemdata( std::string args, std::string &retstring );
      long native( std::string cmd, std::string &retstring );     ///< send Lakeshore-native command to specified unit
      long setpoint( std::string args, std::string &retstring );  ///< set or get setpoint for specified output

  };
  /***** Thermal::Interface ***************************************************/

}
/***** Thermal ****************************************************************/
