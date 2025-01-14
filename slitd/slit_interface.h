/** ---------------------------------------------------------------------------
 * @file    slit_interface.h
 * @brief   slit interface include
 * @details defines the classes used by the slit hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include "slitd_commands.h"
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>
#include <memory>

#define MOVE_TIMEOUT 25000  ///< number of milliseconds before a move fails
#define HOME_TIMEOUT 25000  ///< number of milliseconds before a home fails

/***** Slit *******************************************************************/
/**
 * @namespace Slit
 * @brief     namespace for the slit daemon
 *
 */
namespace Slit {

  const std::string DAEMON_NAME = "slitd";       ///< when run as a daemon, this is my name

  enum class Unit { MM, ARCSEC };

  /***** Slit::SlitDimension **************************************************/
  /**
   * @class    SlitDimension
   * @brief    helper class encapsulates a slit dimension value with its unit
   * @details  This class allows setting and getting a slit dimension with
   *           its unit. Retrieve the value in the desired unit with the
   *           .mm() and .arcsec() functions.
   *
   */
  class SlitDimension {
    private:
      static float _arcsec_per_mm;  ///< static scale factor shared by all objects
      float _value;                 ///< currernt value in the stored unit
      Unit _unit;                   ///< unit for the current value

      /**
       * cahced variables in case mm() or arcsec()
       * needs to perform a unit conversion
       */
      mutable float _cached_mm;
      mutable float _cached_arcsec;

    public:
      explicit SlitDimension( float v=NAN, Unit u=Unit::MM )
        : _value(v), _unit(u) { }

      SlitDimension( const SlitDimension &other )
        : _value(other._value),
          _unit(other._unit) { }

      static void initialize_arcsec_per_mm( float value ) {
        _arcsec_per_mm = ( (!std::isnan(value) && value) > 0 ? value : NAN );
      }

      /**
       * @brief  return modifiable reference to value in mm
       */
      float &mm() const {
        if (_arcsec_per_mm==0) _arcsec_per_mm=NAN;
        if ( _unit==Unit::MM ) return const_cast<float&>(_value);
        else {
          _cached_mm = _value / _arcsec_per_mm;
          return _cached_mm;
        }
      }

      /**
       * @brief  return modifiable reference to value in arcsec
       */
      float &arcsec() const {
        if (_arcsec_per_mm==0) _arcsec_per_mm=NAN;
        if ( _unit==Unit::ARCSEC ) return const_cast<float&>(_value);
        else {
          _cached_arcsec = _value * _arcsec_per_mm;
          return _cached_arcsec;
        }
      }

      /**
       * overload operators
       */
      SlitDimension operator+( float scalar ) const {
        return SlitDimension( ( _unit==Unit::MM ? mm() : arcsec() )+scalar, _unit );
      }
      SlitDimension operator-( float scalar ) const {
        return SlitDimension( ( _unit==Unit::MM ? mm() : arcsec() )-scalar, _unit );
      }
      SlitDimension operator*( float scalar ) const {
        return SlitDimension( ( _unit==Unit::MM ? mm() : arcsec() )*scalar, _unit );
      }
      SlitDimension operator/( float scalar ) const {
        if (scalar==0) throw std::invalid_argument("division by zero");
        return SlitDimension( ( _unit==Unit::MM ? mm() : arcsec() )/scalar, _unit );
      }
      bool operator<( float scalar ) const {
        return ( ( _unit==Unit::MM ? mm() : arcsec() ) < scalar );
      }
      bool operator>( float scalar ) const {
        return ( ( _unit==Unit::MM ? mm() : arcsec() ) > scalar );
      }
      bool operator==( float scalar ) const {
        return ( ( _unit==Unit::MM ? mm() : arcsec() ) == scalar );
      }
      bool operator<=( float scalar ) const {
        return ( ( _unit==Unit::MM ? mm() : arcsec() ) <= scalar );
      }
      bool operator>=( float scalar ) const {
        return ( ( _unit==Unit::MM ? mm() : arcsec() ) >= scalar );
      }
      bool operator<(const SlitDimension& other) const {
        if (_unit == Unit::MM) return mm() < other.mm();
        else return arcsec() < other.arcsec();
      }
      bool operator>(const SlitDimension& other) const {
        if (_unit == Unit::MM) return mm() > other.mm();
        else return arcsec() > other.arcsec();
      }
      bool operator==(const SlitDimension& other) const {
        if (_unit == Unit::MM) return mm() == other.mm();
        else return arcsec() == other.arcsec();
      }
      bool operator<=(const SlitDimension& other) const {
        if (_unit == Unit::MM) return mm() <= other.mm();
        else return arcsec() <= other.arcsec();
      }
      bool operator>=(const SlitDimension& other) const {
        if (_unit == Unit::MM) return mm() >= other.mm();
        else return arcsec() >= other.arcsec();
      }
      SlitDimension& operator+=(float scalar) {
        _value = ( _unit==Unit::MM ? mm() : arcsec() ) + scalar;
        return *this;
      }
      SlitDimension& operator-=(float scalar) {
        _value = ( _unit==Unit::MM ? mm() : arcsec() ) - scalar;
        return *this;
      }
      SlitDimension& operator*=(float scalar) {
        _value = ( _unit==Unit::MM ? mm() : arcsec() ) * scalar;
        return *this;
      }
      SlitDimension& operator/=(float scalar) {
        if (scalar == 0) throw std::invalid_argument("division by zero");
        _value = ( _unit==Unit::MM ? mm() : arcsec() ) / scalar;
        return *this;
      }
  };
  /***** Slit::SlitDimension **************************************************/


  /***** Slit::Interface ******************************************************/
  /**
   * @class  Interface
   * @brief  interface class for a slit device
   *
   * This class defines the interface for each slit controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      zmqpp::context context;
      size_t numdev;

    public:
      Interface()
        : context(),
          numdev(0),
          subscriber(std::make_unique<Common::PubSub>(context, Common::PubSub::Mode::SUB)),
          is_subscriber_thread_running(false),
          should_subscriber_thread_run(false) {
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

      SlitDimension maxwidth;
      SlitDimension minwidth;  ///< set by config file
      SlitDimension center;    ///< position of center in actuator units

      typedef struct {
        SlitDimension width;
        SlitDimension offset;
        float posA=NAN;
        float posB=NAN;
        bool ishome=false;
        bool isopen=false;
      } snapshot_t;

      snapshot_t snapshot;

      Common::Queue async;

      // PI Interface class for Servo type
      //
      Physik_Instrumente::Interface<Physik_Instrumente::ServoInfo> motorinterface;

      // publish/subscribe functions
      //
      long init_pubsub(const std::initializer_list<std::string> &topics={}) {
        return Common::PubSubHandler::init_pubsub(context, *this, topics);
      }
      void start_subscriber_thread() { Common::PubSubHandler::start_subscriber_thread(*this); }
      void stop_subscriber_thread()  { Common::PubSubHandler::stop_subscriber_thread(*this); }

      void handletopic_snapshot( const nlohmann::json &jmessage );
      void publish_snapshot();
      void publish_snapshot(std::string &retstring);

      long initialize_class();
      long open();                               ///< opens the PI socket connection
      long close();                              ///< closes the PI socket connection
      long is_open( std::string arg, std::string &retstring );     ///< are motor controllers connected?
      long home( std::string args, std::string &help );            ///< home all daisy-chained motors
      long is_home( std::string arg, std::string &retstring );     ///< return the home state of the motors

      long offset( std::string args, std::string &retstring );     ///< set the slit offset only
      long set( std::string args, std::string &retstring );        ///< set the slit width and offset
      long get( std::string args, std::string &retstring );        ///< get the current width and offset
      long get( std::string &retstring );                          ///< get the current width and offset
      long get();                                                  ///< get the current width and offset
      long read_positions( float &width, float &offset, float &posa, float &posb );
      long read_positions( float &width, float &offset );

      static void dothread_move_abs( Slit::Interface &iface, int addr, float pos ); ///< threaded move_abs function

      long move_abs( int addr, float pos );      ///< send move-absolute command to specified controllers
      long move_rel( std::string args );         ///< send move-relative command to specified controllers
      long stop();                               ///< send the stop-all-motion command to all controllers
      long send_command( std::string args, std::string &retstring );      ///< writes the raw command as received to the master controller

      std::mutex pi_mutex;                       ///< mutex to protect multi-threaded access to PI controller

      volatile std::atomic<int> motors_running;  ///< number of motors that are running in threads
      volatile std::atomic<long> thr_error;      ///< error state of threads
      std::mutex wait_mtx;                       ///< mutex object for waiting for threads
      std::condition_variable cv;                ///< condition variable for waiting for threads
  };
  /***** Slit::Interface ******************************************************/

}
/***** Slit *******************************************************************/
