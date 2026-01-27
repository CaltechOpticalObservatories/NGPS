/**
 * @file    motion_controller.h
 * @brief   common classes and utilities for motion controllers
 * @details These implementations can be shared between PI and Galil
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "common.h"
#include "network.h"
#include "utilities.h"

#include <string>
#include <vector>
#include <mutex>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <map>

/***** MotionController *******************************************************/
/**
 * @namespace MotionController
 * @brief     common classes and utilities for motion controllers
 *
 */
namespace MotionController {

  /***** MotionController::AxisInfo *******************************************/
  /**
   * @class    AxisInfo
   * @brief    information for a motor axis
   *
   */
  class AxisInfo {
    public:
      std::string motorname;      ///< motor controller name
      int axisnum;                ///< 1-based axis number {1,2,3,...}
      float pos;                  ///< current position of this actuator
      bool ishome;                ///< is axis homed?
      bool ontarget;              ///< is axis on target?
      float min;                  ///< min travel range of motor connected to this controller
      float max;                  ///< max travel range of motor connected to this controller
      float zeropos;              ///< defined zero position of motor
      float defpos;               ///< optional default position to move after referencing (NaN if not specified)
      std::string reftype;        ///< reference type can be { neg, pos, ref }

      AxisInfo() : axisnum(0), pos(NAN), ishome(false), ontarget(false), min(NAN), max(NAN), zeropos(NAN) { }

      long load_axis_info( std::string input );
  };
  /***** MotionController::AxisInfo *******************************************/


  /***** MotionController::PosInfo ********************************************/
  /**
   * @class    PosInfo
   * @brief    information for position maps
   *
   */
  class PosInfo {
    public:
      std::string motorname;
      int axis;
      int posid;
      float position;
      std::string posname;

      long load_pos_info( std::string input );
  };
  /***** MotionController::PosInfo ********************************************/


  /***** MotionController::ControllerInfoBase *********************************/
  /**
   * @class    ControllerInfoBase
   * @brief    Base contoller information class
   * @details  This is the base class which contains members and functions
   *           common to all types of controllers. It is intended to be
   *           inherited by each specific class. This will always be
   *           constructed first.
   *
   */
  class ControllerInfoBase {
    public:
      std::string name;                       ///< controller name
      int addr;                               ///< controller address
      int naxes;
      std::map<int, AxisInfo> axes;           ///< info for each axis
      std::map<std::string, PosInfo> posmap;  ///< map of positions indexed by position name

      ControllerInfoBase() : name(""), addr(-1), naxes(-1) { }

  };
  /***** MotionController::ControllerInfoBase *********************************/


  /***** MotionController::ControllerInfo *************************************/
  /**
   * @class    ControllerInfo
   * @brief    Template class for all controller types
   * @details  This is a template class for types <T> where T is one of the
   *           derived classes representing a specific controller type. You
   *           can create a specialization by instantiating an object, E.G.,
   *           "ControllerInfo<StepperInfo> stepper" to specialize it for a
   *           stepper controller. This will always be constructed last,
   *           after the derived class.
   *
   */
  template <typename T>
  class ControllerInfo : public T {
    public:
      std::string host;                              ///< each motor controller defines its own host and port
      int port;

      ControllerInfo() : host(""), port(-1) { }      ///< constructor

      /***** ControllerInfo::load_controller_info *****************************/
      /**
       * @brief      Loads information from the config file into the class
       * @details    This is the template class version which can parse the
       *             input string for common parameters, then call the
       *             specialized version which will extract parameters specific
       *             to that type of controller. OBSOLETE
       * @param[in]  input  string containing line from config file
       * @return     ERROR or NO_ERROR
       *
       */
      long load_controller_info( std::string input ) {
        return T::load_controller_info( input );
      }
      /***** ControllerInfo::load_controller_info *****************************/


      /***** ControllerInfo::load_pos_info ************************************/
      /**
       * @brief      Loads MOTOR_POS information from config file into the 
       * @details    This is the template class version which will parse the
       *             input string for common parameters.
       * @param[in]  input  string specifies: "<name> <ID> <pos> <posname>"
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the MOTOR_POS key is found
       * in the configuration file, to parse and load all of the information
       * assigned by that key into the appropriate class variables.
       *
       * Currently all motors use the same MOTOR_POS format, but if that changes
       * then a template class function call could be made here.
       *
       */
      long load_pos_info( std::string input ) {
        std::string function = "MotionController::ControllerInfo::load_pos_info";
        int id;
        float position;
        std::string name, posname;

        std::istringstream iss(input);

        if (!(iss >> name
                  >> id
                  >> position
                  >> posname)) {
          logwrite(function, "ERROR bad config input. expected { <name> <ID> <pos> <posname> }");
          return ERROR;
        }

        // ID starts at 0 (can't be negative)
        //
        if ( id < 0 ) {
          std::ostringstream oss;
          oss << "ERROR: id " << id << " for \"" << name << "\" " << "must be >= 0";
          logwrite(function, oss.str());
          return ERROR;
        }

        this->name = name;
        this->posmap[ posname ].id       = id;
        this->posmap[ posname ].position = position;
        this->posmap[ posname ].posname  = posname;

        return NO_ERROR;
      }
      /***** ControllerInfo::load_pos_info ************************************/

  };
  /***** MotionController::ControllerInfo *************************************/


  /***** MotionController::Interface ******************************************/
  /**
   * @class  Interface
   * @brief  MotionController interface class
   *
   */
  template <typename ControllerType>
  class Interface {
    protected:
      bool is_initialized;
      std::string name;        ///< a name for info purposes
      std::unique_ptr<std::mutex> controller_mutex;
      int move_timeout;
      int home_timeout;
      float tolerance;
      volatile std::atomic<int> motors_running;
      volatile std::atomic<long> thread_error;
      std::map<std::pair<std::string, int>, Network::TcpSocket> socketmap;
      std::map<std::string, ControllerInfo<ControllerType>> motormap;

    private:
      long _open(const std::string& motorname);

    public:

      // every controller must implement these functions
      //
      virtual void test() = 0;

      // these have common implementations
      //
      bool get_initialized() { return this->is_initialized; };
      std::map<std::string, ControllerInfo<ControllerType>> get_motormap() const { return motormap; }
      inline void clear_motormap() { motormap.clear(); }

      long open();
      long close();
      long close(const std::string &name);

      /***** MotionController::Interface::is_connected ************************/
      /**
       * @brief      is the socket connected for the specified motor controller?
       * @param[in]  motorname  name of motor controller
       * @return     true or false
       *
       */
      bool is_connected(const std::string &motorname) {
        try {
          // get a copy of the socket object for this motor and return its connected status
          //
          Network::TcpSocket &socket = this->get_socket( motorname );
          return ( socket.isconnected() ? true : false );
        }
        catch ( std::runtime_error &e ) {
          std::string function = "MotionController::Interface::is_connected";
          std::stringstream message;
          message.str(""); message << "ERROR: " << e.what();
          logwrite( function, message.str() );
          return false;
        }
      }
      /***** MotionController::Interface::is_connected ************************/


      /***** MotionController::Interface::add_posmap **************************/
      /**
       * @brief      add the supplied posinfo to the motormap's posmap
       * @param[in]  posinfo  reference to PosInfo object
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the MOTOR_POS key is found
       * in the configuration file. A temporary PosInfo object can be
       * created and loaded with the PosInfo::load_pos_info() function,
       * and then that object can be passed here to store it in
       * the controller class.
       *
       */
      long add_posmap( const PosInfo &posinfo ) {
        auto it = motormap.find( posinfo.motorname );
        if ( it != motormap.end() ) {
          it->second.posmap[posinfo.posname] = posinfo;  // posmap indexed by position name
        }
        else {
          std::string function = "MotionController::Interface::add_posmap";
          std::stringstream message;
          message << "ERROR can't add position " << posinfo.posname << " because motor "
                  << posinfo.motorname << " has not been configured";
          logwrite( function, message.str() );
          return ERROR;
        }
        return NO_ERROR;
      }
      /***** MotionController::Interface::add_posmap **************************/


      /***** MotionController::Interface::add_axis ****************************/
      /**
       * @brief      add the supplied axis info to the motormap's axes vector
       * @param[in]  axis  reference to AxisInfo object
       * @return     ERROR or NO_ERROR
       *
       */
      long add_axis( const AxisInfo &axis ) {
        // Make sure the motor associated with this axis is already defined
        // in the motormap.
        //
        auto it = motormap.find( axis.motorname );

        // If it is, then push it onto the axes vector for this motor.
        //
        if ( it != motormap.end() ) {
          it->second.axes[axis.axisnum] = axis;
        }
        else {
          std::string function = "MotionController::Interface::add_axis";
          std::stringstream message;
          message << "ERROR can't add axis " << axis.axisnum << " to because motor "
                  << axis.motorname << " has not been configured";
          logwrite( function, message.str() );
          return ERROR;
        }
        return NO_ERROR;
      }
      /***** MotionController::Interface::add_axis ****************************/


      // Default constructor
      //
      Interface()
        : is_initialized(false), name(""), controller_mutex(std::make_unique<std::mutex>()),
          move_timeout(60000), home_timeout(60000), tolerance(0.001),
          motors_running(0), thread_error(NO_ERROR) { }

      // Constructor initializes move and home timeouts
      //
      Interface(int TO_move, int TO_home, float tol)
        : is_initialized(false), name(""), controller_mutex(std::make_unique<std::mutex>()),
          move_timeout(TO_move), home_timeout(TO_home), tolerance(tol),
          motors_running(0), thread_error(NO_ERROR) { }

      // Copy constructor
      //
      Interface( const Interface &other )
        : is_initialized(other.is_initialized),
          name(other.name),
          controller_mutex(std::make_unique<std::mutex>()),
          move_timeout(other.move_timeout),
          home_timeout(other.home_timeout),
          tolerance(other.tolerance),
          motors_running(other.motors_running),
          thread_error(other.thread_error),
          socketmap(other.socketmap),
          motormap(other.motormap) {}

      // Copy assignment operator
      //
      Interface &operator = ( const Interface &other ) {
        if ( this != &other ) {
          is_initialized = other.is_initialized;
          name = other.name;
          controller_mutex = std::make_unique<std::mutex>();  // Create a new mutex for the copy
          move_timeout = other.move_timeout;
          home_timeout = other.home_timeout;
          tolerance = other.tolerance;
          motors_running = other.motors_running;
          thread_error = other.thread_error;
          socketmap = other.socketmap;
          motormap = other.motormap;
        }
        return *this;
      }

      // Move constructor
      //
      Interface( Interface &&other ) noexcept
        : is_initialized(other.is_initialized),
          name(std::move(other.name)),
          controller_mutex(std::move(other.controller_mutex)),
          move_timeout(std::move(other.move_timeout)),
          home_timeout(std::move(other.home_timeout)),
          tolerance(std::move(other.tolerance)),
          motors_running(std::move(other.motors_running)),
          thread_error(std::move(other.thread_error)),
          socketmap(std::move(other.socketmap)),
          motormap(std::move(other.motormap)) { other.is_initialized = false; }

      // Move assignment operator
      //
      Interface &operator = ( Interface &&other ) noexcept {
        if ( this != &other ) {
          is_initialized = other.is_initialized;
          name = std::move( other.name );
          controller_mutex = std::move( other.controller_mutex );
          move_timeout = std::move( other.move_timeout );
          home_timeout = std::move( other.home_timeout );
          tolerance = std::move( other.tolerance );
          motors_running = std::move( other.motors_running );
          thread_error = std::move( other.thread_error );
          socketmap = std::move( other.socketmap );
          motormap = std::move( other.motormap );
          other.is_initialized = false;
          other.motors_running = 0;
          other.thread_error = NO_ERROR;
        }
        return *this;
      }

      /***** MotionController::Interface::load_controller_config **************/
      /**
       * @brief      loads information from the config file into the class
       * @param[in]  input  config file line (the args, not the key)
       * @return     ERROR or NO_ERROR
       *
       * Expected format of input string is:
       * "<motorname> <host> <port> <addr> <naxes>"
       */
      long load_controller_config( const std::string &input ) {
        std::string function = "MotionController::Interface::load_controller_config";

        std::istringstream iss(input);
        std::string tryname, tryhost;
        int tryport, tryaddr, trynaxes;

        if (!(iss >> tryname
                  >> tryhost
                  >> tryport
                  >> tryaddr
                  >> trynaxes)) {
          logwrite(function, "ERROR: bad config input. Expected { <motorname> <host> <port> <addr> <naxes> }" );
          return ERROR;
        }

        // Create a local (temporary) ControllerInfo object for this ControllerType,
        //
        ControllerInfo<ControllerType> controller;

        // and initialize it with the values just parsed from the config line.
        //
        controller.name  = tryname;
        controller.host  = tryhost;
        controller.port  = tryport;
        controller.addr  = tryaddr;
        controller.naxes = trynaxes;

        // Copy this to the motormap, indexed by motorname.
        //
        this->motormap[tryname] = controller;

        return NO_ERROR;
      }
      /***** MotionController::Interface::load_controller_config **************/


      /***** MotionController::Interface::get_socket **************************/
      /**
       * @brief      return a copy of the socket object for the specified motor
       * @details    This can also be used to create a new entry in the socketmap
       *             because if a socket object doesn't exist for the {host,port}
       *             then it will be created and added to the socketmap.
       * @param[in]  motorname  name of motor controller
       * @return     reference to Network::TcpSocket object
       *
       * On error, this function throws std::runtime_error exception; it does
       * not return an error, so it should be used in a try/catch block. It
       * fails only if the requested motorname is not in the motormap.
       *
       */
      Network::TcpSocket &get_socket( const std::string &motorname ) {
        std::string function = "MotionController::Interface::get_socket";
        std::stringstream message;

        // Find the ControllerInfo for the requested motorname
        //
        auto it = this->motormap.find( motorname );

        if ( it == this->motormap.end() ) {
          message.str(""); message << "motorname \"" << motorname << "\" not found (get_socket)";
          throw std::runtime_error( message.str() );
        }

        // Get the host:port for this motor
        //
        const std::string &host = it->second.host;
        int port = it->second.port;

        // Use this host:port as a key to index the socketmap
        //
        std::pair<std::string, int> key = { host, port };

        // Find the Network::TcpSocket for this motor
        //
        auto socket_it = this->socketmap.find( key );

        // If socket not found in map then create it and add to the socketmap
        //
        if ( socket_it == this->socketmap.end() ) {
          message.str(""); message << "socket not found for motor " << motorname
                                   << " on " << host << ":" << port << " (get_socket)";
          Network::TcpSocket new_socket( host, port );
          this->socketmap[key] = new_socket;
          socket_it = this->socketmap.find( key );  // update the iterator
        }

        return socket_it->second;
      }
      /***** MotionController::Interface::get_socket **************************/

  };
  /***** MotionController::Interface ******************************************/

}
#include "motion_controller_impl.h"  ///< implementations for template code
/***** MotionController *******************************************************/
