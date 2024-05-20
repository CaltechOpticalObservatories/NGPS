/**
 * @file    pi.h
 * @brief   this file contains the definitions for the PI interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef PHYSIK_INSTRUMENTE_H
#define PHYSIK_INSTRUMENTE_H

#include "common.h"
#include "network.h"
#include "utilities.h"
#include "pi_error.h"

#include <string>
#include <vector>
#include <mutex>

/***** Physik_Instrumente *****************************************************/
/**
 * @namespace Physik_Instrumente
 * @brief     namespace for Physik Instrumente hardware
 * @details   This namespace contains Interface and ControllerInfo classes.
 *            The Interface class contains the functions which communicate
 *            with the hardware. The ControllerInfo class contains the
 *            information from the configuration file.
 *
 */
namespace Physik_Instrumente {


  /***** AxisInfo *************************************************************/
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

      std::string reftype;        ///< reference type can be { neg, pos, ref }

      AxisInfo() : axisnum(0), pos(NAN), ishome(false), ontarget(false), min(NAN), max(NAN), zeropos(NAN) { }

      long load_axis_info( std::string input ) {
        std::string function = "Physik_Instrumente::AxisInfo::load_axis_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 6 ) {
          logwrite( function, "ERROR: bad config input. Expected { <motorname> <axis> <min> <max> <zero> <ref> }" );
          return( ERROR );
        }

        std::string tryname, tryreftype;
        float trymin, trymax, tryzero;
        int trynum;

        try {
          tryname    = tokens.at(0);
          trynum     = std::stoi( tokens.at(1) );
          trymin     = std::stod( tokens.at(2) );
          trymax     = std::stod( tokens.at(3) );
          tryzero    = std::stod( tokens.at(4) );
          tryreftype = tokens.at(5);
        }
        catch( std::out_of_range &e ) {
          message.str(""); message << "ERROR out of range parsing \"" << input << "\": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch( std::invalid_argument &e ) {
          message.str(""); message << "ERROR invalid argument parsing \"" << input << "\": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }

        // The requested reftype must be one of the PI valid reftypes.
        // Use "na" if the axis doesn't support referencing.
        //
        const std::vector<std::string> valid_reftypes{ "neg", "pos", "ref", "na" };

        if ( std::find( valid_reftypes.begin(), valid_reftypes.end(), tryreftype ) == valid_reftypes.end() ) {
          message.str(""); message << "ERROR: reftype \"" << tryreftype << "\" invalid. Expected { ";
          for ( auto ref : valid_reftypes ) message << ref << " ";
          message << "}";
          logwrite( function, message.str() );
          return( ERROR );
        }

        // num must be 1-based
        //
        if ( trynum < 1 ) {
          message.str(""); message << "ERROR axis number must be > 0: \"" << input << "\"";
          logwrite( function, message.str() );
          return( ERROR );
        }

        // min can't be greater than max
        //
        if ( trymin > trymax ) {
          message.str(""); message << "ERROR min can't be greater than max: \"" << input << "\"";
          logwrite( function, message.str() );
          return( ERROR );
        }

        // all valid so set the class variables
        //
        this->motorname = tryname;
        this->axisnum   = trynum;
        this->reftype   = tryreftype;
        this->min       = trymin;
        this->max       = trymax;
        this->zeropos   = tryzero;

        return( NO_ERROR );
      }
  };
  /***** AxisInfo *************************************************************/


  /***** PosInfo **************************************************************/
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

      /***** load_pos_info ****************************************************/
      /**
       * @brief      Loads MOTOR_POS information from config file into the 
       * @details    This is the template class version which will parse the
       *             input string for common parameters.
       * @param[in]  input  string specifies: "<motorname> <ID> <pos> <posname>"
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
        std::string function = "Physik_Instrumente::PosInfo::load_pos_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 5 ) {
          message.str(""); message << "ERROR bad number of arguments: " << tokens.size()
                                   << ". expected 5 { <motorname> <axis> <posid> <pos> <posname> }";
          logwrite( function, message.str() );
          return( ERROR );
        }

        int axis, posid;
        float position;
        std::string motorname, posname;

        try {
          motorname = tokens.at(0);
          axis      = std::stoi( tokens.at(1) );
          posid     = std::stoi( tokens.at(2) );
          position  = std::stof( tokens.at(3) );
          posname   = tokens.at(4);
        }
        catch ( std::exception &e ) {
          message.str(""); message << "ERROR: parsing \"" << input << "\": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }

        // ID starts at 0 (can't be negative)
        //
        if ( posid < 0 ) {
          message.str(""); message << "ERROR: posid " << posid << " for \"" << this->motorname << "\" "
                                   << "must be >= 0";
          logwrite( function, message.str() );
          return( ERROR );
        }

        this->motorname = motorname;
        this->axis      = axis;
        this->posid     = posid;
        this->position  = position;
        this->posname   = posname;

        return NO_ERROR;
      }
      /***** load_pos_info ****************************************************/
  };
  /***** PosInfo **************************************************************/


  /***** ControllerInfoBase ***************************************************/
  /**
   * @class    ControllerInfoBase
   * @brief    Base contoller information class
   * @details  This is the base class which contains members and functions
   *           common to all types of PI controllers. It is intended to be
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
  /***** ControllerInfoBase ***************************************************/


  /***** PiezoInfo ************************************************************/
  /**
   * @class    PiezoInfo
   * @brief    Derived class containing information specific to piezo controllers
   * @details  This is the derived class for piezo controllers, containing
   *           members relevant only to piezo controllers. It inherits the base
   *           class to gain access to common base members. This will always be
   *           constructed after the base class (and before the specialization).
   *
   */
  class PiezoInfo : public ControllerInfoBase {
    public:

      PiezoInfo() { }                                ///< constructor

      // The following are special functions to load info from the config
      // file into the class, for this type of controller.
      //
      long load_controller_info( std::vector<std::string> tokens );
  };
  /***** PiezoInfo ************************************************************/


  /***** ServoInfo ************************************************************/
  /**
   * @class    ServoInfo
   * @brief    Derived class contains information specific to servo controllers
   * @details  This is the derived class for servo controllers, containing
   *           members relevant only to servo controllers. It inherits the base
   *           class to gain access to common base members. This will always be
   *           constructed after the base class (and before the specialization).
   *
   */
  class ServoInfo : public ControllerInfoBase {
    public:
      bool servo;                 ///< servo state (true=on, false=off)

      ServoInfo() : servo(false) { }

      long load_controller_info( std::vector<std::string> tokens );
  };
  /***** ServoInfo ************************************************************/


  /***** StepperInfo **********************************************************/
  /**
   * @class    StepperInfo
   * @brief    Derived class containing information specific to stepper controllers
   * @details  This is the derived class for stepper controllers, containing
   *           members relevant only to stepper controllers. It inherits the base
   *           class to gain access to common base members. This will always be
   *           constructed after the base class (and before the specialization).
   *
   */
  class StepperInfo : public ControllerInfoBase {
    public:

      StepperInfo() { }                              ///< constructor

      // The following are special functions to load info from the config
      // file into the class, for this type of controller.
      //
      long load_controller_info( std::vector<std::string> tokens );
  };
  /***** StepperInfo **********************************************************/


  /***** ControllerInfo *******************************************************/
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
        std::string function = "Physik_Instrumente::ControllerInfo::load_pos_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 4 ) {
          message.str(""); message << "ERROR bad number of arguments: " << tokens.size()
                                   << ". expected 4 { <name> <ID> <pos> <posname> }";
          logwrite( function, message.str() );
          return( ERROR );
        }

        int id;
        float position;
        std::string posname;

        try {
          this->name = tokens.at(0);
          id         = std::stoi( tokens.at(1) );
          position   = std::stof( tokens.at(2) );
          posname    = tokens.at(3);
        }
        catch ( std::exception &e ) {
          message.str(""); message << "ERROR: parsing \"" << input << "\": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }

        // ID starts at 0 (can't be negative)
        //
        if ( id < 0 ) {
          message.str(""); message << "ERROR: id " << id << " for \"" << this->name << "\" "
                                   << "must be >= 0";
          logwrite( function, message.str() );
          return( ERROR );
        }

        this->posmap[ posname ].id       = id;
        this->posmap[ posname ].position = position;
        this->posmap[ posname ].posname  = posname;

        return NO_ERROR;
      }
      /***** ControllerInfo::load_pos_info ************************************/

  };
  /***** ControllerInfo *******************************************************/


  /***** Interface ************************************************************/
  /**
   * @class  Interface
   * @brief  PI interface class
   *
   */
  template <typename ControllerType>
  class Interface {
    private:
      bool initialized;
      std::string name;        ///< a name for info purposes
      std::unique_ptr<std::mutex> pi_mutex;
      int move_timeout;
      int home_timeout;
      float tolerance;
      volatile std::atomic<int> motors_running;
      volatile std::atomic<long> thread_error;
      std::map<std::pair<std::string, int>, Network::TcpSocket> socketmap;
      std::map<std::string, ControllerInfo<ControllerType>> motormap;

      // these functions are for use within the class only
      //

      // moving
      //
      static void _dothread_moveto( Interface<ControllerType> &iface,
                                                   const std::string name,
                                                   const int axis,
                                                   const int addr,
                                                   const float pos );
      long _move_axis_wait( const std::string &name, int addr, int axis );
      long _move_abs( const std::string &name, int addr, float pos );                          ///< send move command in absolute coordinates
      long _move_abs( const std::string &name, int addr, int axis, float pos );                ///< send move command in absolute coordinates for specific axis
      long _move_rel( const std::string &name, int addr, float pos );                          ///< move in relative coordinates
      long _move_rel( const std::string &name, int addr, int axis, float pos );                ///< move in relative coordinates for specific axis

      // homing
      //
      static void _dothread_home( Interface<ControllerType> &iface, const std::string name );  ///< work thread for home
      long _home_axis_wait( const std::string &name, int addr, int axis );
      long _home_axis( const std::string &name, int addr, std::string ref );                   ///< home an axis by moving to reference switch
      long _home_axis( const std::string &name, int addr, int axis, std::string ref );         ///< home an axis by moving to reference switch for specific axis

      long _get_pos( const std::string &name, int addr, float &pos );                          ///< get the current position of a motor
      long _get_pos( const std::string &name, int addr, int axis, float &pos );                ///< get the current position of a motor for specific axis
      long _open( const std::string &name );                          ///< open a connection to the servo controller

    public:
      bool is_initialized() { return this->initialized; };
      bool is_connected( const std::string &motorname );
      long open();                                                   ///< open a connection to the servo controller
      long close();                                                  ///< close the connection to the servo controller
      long close( const std::string &name );                         ///< close the connection to the servo controller
      long clear_errors();
      long get_error( const std::string &name, int addr, int &errcode );                      ///< read the error status
      long set_servo( bool state );                        ///< set all servos on|off
      long set_servo( const std::string &name, int addr, bool state );                        ///< set the servo on|off
      long set_servo( const std::string &name, int addr, int axis, bool state);               ///< set the servo on|off for specific axis
      long moveto( std::string motorname, int axisnum, std::string posname, std::string &retstring );  ///< outside-callable move-to-posname function
      long moveto( std::string motorname, int axusnum, float position, std::string &retstring );  ///< outside-callable move-to-posname function
      long moveto( std::vector<std::string> motornames, std::vector<int> axisnums, std::vector<std::string> posnames, std::string &retstring );  ///< outside-callable move-to-posname function
      long home( std::string input, std::string &retstring );                                 ///< outside-callable home function
      long is_home( std::string input, std::string &retstring );                              ///< outside-callable is_home function
      long is_home( const std::string &name, int addr, int axis );                            ///< queries whether referencing has been done
      long is_home( const std::string &name, int addr, int axis, bool &state );               ///< queries whether referencing has been done
      long on_target( const std::string &name, int addr, bool &retstring );                   ///< query the on target state for given addr
      long on_target( const std::string &name, int addr, int axis, bool &state );             ///< query the on target state for given addr and axis
      long get_pos( const std::string &name, int axisnum, float &position );
      long get_pos( const std::string &name, int axisnum, int addr, float &position, std::string &posname );  ///< outside callable version
      long get_pos( const std::string &name, int axisnum, float &position, std::string &posname );
      long get_pos( const std::string &name, int axisnum, int addr, float &position, std::string &posname, float tol_in );  ///< outside callable version
      long stop_motion( const std::string &name, int addr );                                  ///< stop all movement on all axes
      long send_command( const std::string &motorname, std::string cmd );  ///< send a command string to the controller
      long send_command( const std::string &motorname, std::string cmd, std::string &retstring );  ///< send a command string to the controller

      template <typename ReplyType> long parse_reply( int axis, std::string &reply, ReplyType &retval );

      std::map<std::string, ControllerInfo<ControllerType>> get_motormap() const { return motormap; }
      inline void clear_motormap() { motormap.clear(); }

      /***** Interface::add_posmap ********************************************/
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
          std::string function = "Physik_Instrumente::Interface::add_posmap";
          std::stringstream message;
          message << "ERROR can't add position " << posinfo.posname << " because motor "
                  << posinfo.motorname << " has not been configured";
          logwrite( function, message.str() );
          return ERROR;
        }
        return NO_ERROR;
      }
      /***** Interface::add_posmap ********************************************/


      /***** Interface::add_axis **********************************************/
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
//        it->second.axes.push_back( axis );
        }
        else {
          std::string function = "Physik_Instrumente::Interface::add_axis";
          std::stringstream message;
          message << "ERROR can't add axis " << axis.axisnum << " to because motor "
                  << axis.motorname << " has not been configured";
          logwrite( function, message.str() );
          return ERROR;
        }
        return NO_ERROR;
      }
      /***** Interface::add_axis **********************************************/


      // Default constructor
      //
      Interface() : initialized(false), name(""), pi_mutex(std::make_unique<std::mutex>()),
                    move_timeout(60000), home_timeout(60000), tolerance(0.001) {}

      // Constructor initializes move and home timeouts
      //
      Interface( int TO_move, int TO_home, float tol ) : initialized(false), name(""), pi_mutex(std::make_unique<std::mutex>()),
                     move_timeout(TO_move), home_timeout(TO_home), tolerance(tol) {}

      // Copy constructor
      //
      Interface( const Interface &other ) : initialized(other.initialized),
                                            name(other.name),
                                            pi_mutex(std::make_unique<std::mutex>()),
                                            move_timeout(other.move_timeout),
                                            home_timeout(other.home_timeout),
                                            tolerance(other.tolerance),
                                            socketmap(other.socketmap),
                                            motormap(other.motormap) {}

      // Copy assignment operator
      //
      Interface &operator = ( const Interface &other ) {
        if ( this != &other ) {
          initialized = other.initialized;
          name = other.name;
          pi_mutex = std::make_unique<std::mutex>();  // Create a new mutex for the copy
          move_timeout = other.move_timeout;
          home_timeout = other.home_timeout;
          tolerance = other.tolerance;
          socketmap = other.socketmap;
          motormap = other.motormap;
        }
        return *this;
      }

      // Move constructor
      //
      Interface( Interface &&other ) noexcept : initialized(other.initialized),
                                                name(std::move(other.name)),
                                                pi_mutex(std::move(other.pi_mutex)),
                                                move_timeout(std::move(other.move_timeout)),
                                                home_timeout(std::move(other.home_timeout)),
                                                tolerance(std::move(other.tolerance)),
                                                socketmap(std::move(other.socketmap)),
                                                motormap(std::move(other.motormap)) { other.initialized = false; }

      // Move assignment operator
      //
      Interface &operator = ( Interface &&other ) noexcept {
        if ( this != &other ) {
          initialized = other.initialized;
          name = std::move( other.name );
          pi_mutex = std::move( other.pi_mutex );
          move_timeout = std::move( other.move_timeout );
          home_timeout = std::move( other.home_timeout );
          tolerance = std::move( other.tolerance );
          socketmap = std::move( other.socketmap );
          motormap = std::move( other.motormap );
          other.initialized = false;
        }
        return *this;
      }

      /***** Interface::load_controller_config ********************************/
      /**
       * @brief      loads information from the config file into the class
       * @param[in]  input  config file line (the args, not the key)
       * @return     ERROR or NO_ERROR
       *
       * Expected format of input string is:
       * "<motorname> <host> <port> <addr> <naxes>"
       */
      long load_controller_config( const std::string &input ) {
        std::string function = "Physik_Instrumente::Interface::load_controller_config";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 5 ) {
          logwrite( function, "ERROR: bad config input. Expected { <motorname> <host> <port> <addr> <naxes> }" );
          return( ERROR );
        }

        std::string tryname, tryhost;
        int tryport, tryaddr, trynaxes;

        try {
          tryname    = tokens.at(0);
          tryhost    = tokens.at(1);
          tryport    = std::stoi( tokens.at(2) );
          tryaddr    = std::stoi( tokens.at(3) );
          trynaxes   = std::stoi( tokens.at(4) );
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR invalid argument parsing \"" << input << "\": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch ( std::exception &e ) {
          message.str(""); message << "ERROR invalid argument parsing \"" << input << "\": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
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
      /***** Interface::load_controller_config ********************************/


      /***** Interface::get_socket ********************************************/
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
        std::string function = "Physik_Instrumente::Interface::get_socket";
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
      /***** Interface::get_socket ********************************************/

  };
  /***** Interface ************************************************************/

}
/***** Physik_Instrumente *****************************************************/
#endif
