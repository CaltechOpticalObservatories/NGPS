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

  /***** Interface ************************************************************/
  /**
   * @class  Interface
   * @brief  PI interface class
   *
   */
  class Interface {
    private:
      std::string name;        ///< a name for info purposes
      std::string host;        ///< host name for the motor controller
      int port;                ///< port number for motor controller on host
      bool initialized;

    public:
      bool is_initialized() { return this->initialized; };
      long open();                                                   ///< open a connection to the servo controller
      long close();                                                  ///< close the connection to the servo controller
      long get_error( int addr, int &errcode );                      ///< read the error status
      long set_servo( int addr, bool state );                        ///< set the servo on|off
      long set_servo( int addr, int axis, bool state);               ///< set the servo on|off for specific axis
      long move_abs( int addr, float pos );                          ///< send move command in absolute coordinates
      long move_abs( int addr, int axis, float pos );                ///< send move command in absolute coordinates for specific axis
      long move_rel( int addr, float pos );                          ///< move in relative coordinates
      long move_rel( int addr, int axis, float pos );                ///< move in relative coordinates for specific axis
      long home_axis( int addr, std::string ref );                   ///< home an axis by moving to reference switch
      long home_axis( int addr, int axis, std::string ref );         ///< home an axis by moving to reference switch for specific axis
      long is_home( int addr, int axis, bool &state );               ///< queries whether referencing has been done
      long on_target( int addr, bool &retstring );                   ///< query the on target state for given addr
      long on_target( int addr, int axis, bool &state );             ///< query the on target state for given addr and axis
      long get_pos( int addr, float &pos );                          ///< get the current position of a motor
      long get_pos( int addr, int axis, float &pos );                ///< get the current position of a motor for specific axis
      long stop_motion( int addr );                                  ///< stop all movement on all axes
      long send_command( std::string cmd );                          ///< send a command string to the controller
      long send_command( std::string cmd, std::string &retstring );  ///< send a command string to the controller

      template <typename T> long parse_reply( int axis, std::string &reply, T &retval );

      Interface() : name(""), host(""), port(-1), initialized(false) { }
      Interface( std::string host, int port ) : name(""), host(host), port(port), initialized(true) { }
      Interface( std::string name, std::string host, int port ) : name(name), host(host), port(port), initialized(true) { }

      Network::TcpSocket controller;                                 ///< TCP/IP socket object to communicate with controller

  };
  /***** Interface ************************************************************/


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
      int addr;                   ///< controller address
      std::string name;           ///< controller name
      float pos;                  ///< current position of this actuator
      bool ishome;                ///< is axis homed?
      bool ontarget;              ///< is axis on target?
      std::string reftype;        ///< reference type can be { neg, pos, ref }
      float min;                  ///< min travel range of motor connected to this controller
      float max;                  ///< max travel range of motor connected to this controller
      float zeropos;              ///< defined zero position of motor

      /**
       * @struct  motor_pos_t
       * @brief   structure to contain named positions defined by MOTOR_POS in config file
       * @var     motor_pos_t::id        internal 0-based ID number
       * @var     motor_pos_t::position  real actuator position required to reach location ID
       * @var     motor_pos_t::posname   name of that position (what the user will use)
       */
      struct motor_pos_t {
        int id;
        float position;
        std::string posname;
      };

      std::map<std::string, motor_pos_t> posmap;   ///< STL map of MOTOR_POS indexed by posname

      ControllerInfoBase() : addr(-1), name(""), pos(NAN), ishome(false), ontarget(false),
                             reftype(""), min(NAN), max(NAN), zeropos(NAN) { }

  };
  /***** ControllerInfoBase ***************************************************/


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

      ControllerInfo() { }                           ///< constructor

      /***** load_controller_info *********************************************/
      /**
       * @brief      Loads information from the config file into the class
       * @details    This is the template class version which will parse the
       *             input string for common parameters, then will call the
       *             specialized version which will extract parameters specific
       *             to that type of controller.
       * @param[in]  input  string containing line from config file
       * @return     ERROR or NO_ERROR
       *
       * The input string is expected to contain "<addr> <name> <reftype>..." where the
       * additional "..." will be passed as a vector of tokens to the type
       * specific T:load_controller_info() function.
       *
       */
      long load_controller_info( std::string input ) {
        std::string function = "Physik_Instrumente::ControllerInfo::load_controller_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() < 3 ) {
          logwrite( function, "ERROR: bad config input. Expected { <addr> <name> <reftype> ... }" );
          return( ERROR );
        }

        int tryaddr;
        std::string tryname, tryreftype;

        try {
          tryaddr    = std::stoi( tokens.at(0) );
          tryname    = tokens.at(1);
          tryreftype = tokens.at(2);
        }
        catch ( std::exception &e ) {
          message.str(""); message << "ERROR invalid argument parsing \"" << input << "\": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }

        // The requested reftype must be one of the PI valid reftypes
        //
        const std::vector<std::string> valid_reftypes{ "neg", "pos", "ref" };
        if ( std::find( valid_reftypes.begin(), valid_reftypes.end(), tryreftype ) == valid_reftypes.end() ) {
          message.str(""); message << "ERROR: reftype \"" << tryreftype << "\" invalid. Expected { ";
          for ( auto ref : valid_reftypes ) message << ref << " ";
          message << "}";
          logwrite( function, message.str() );
          return( ERROR );
        }

        // addr must be > 0
        //
        if ( tryaddr < 1 ) {
          message.str(""); message << "ERROR: addr " << tryaddr << " cannot be < 1";
          logwrite( function, message.str() );
          return( ERROR );
        }

        // all valid so set the class variables
        //
        this->addr    = tryaddr;
        this->name    = tryname;
        this->reftype = tryreftype;

        // Remove the first three tokens used here: <addr> <name> <reftype>
        // then rest to the specialized load_controller_info() function
        // declared in the derived class.
        //
        tokens.erase( tokens.begin(), tokens.begin()+3 );
        return T::load_controller_info( tokens );
      }
      /***** load_controller_info *********************************************/


      /***** load_pos_info ****************************************************/
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

        this->posmap[ posname ].id      = id;
        this->posmap[ posname].position = position;
        this->posmap[ posname ].posname = posname;

        return NO_ERROR;
      }
      /***** load_pos_info ****************************************************/

  };
  /***** ControllerInfo *******************************************************/

}
/***** Physik_Instrumente *****************************************************/
#endif
