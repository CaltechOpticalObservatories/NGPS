/** ---------------------------------------------------------------------------
 * @file    slit_interface.h
 * @brief   slit interface include
 * @details defines the classes used by the slit hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef SLIT_INTERFACE_H
#define SLIT_INTERFACE_H

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>
#include <map>

#define MOVE_TIMEOUT 20000

namespace Slit {

  /** ControllerInfo **********************************************************/
  /**
   * @class  ControllerInfo
   * @brief  slit motor controller information class
   *
   * This class contains the information for each slit controller
   * and a function for loading the information from the configuration file.
   *
   */
  class ControllerInfo {
    public:
      int addr;                   //!< controller address
      float pos;                  //!< current position of this actuator
      std::string name;           //!< controller name
      float min, max;             //!< min,max travel range of motor connected to this controller
      bool ishome;
      bool ontarget;

      ControllerInfo() {
        this->ishome=false;
        this->ontarget=false;
        }

      long load_info( std::string &input ) {
        std::string function = "Slit::ControllerInfo::load_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 4 ) {
          message.str(""); message << "bad number of tokens: " << tokens.size() << ". expected 4";
          logwrite( function, message.str() );
          return( ERROR );
        }

        try {
          this->addr = std::stoi( tokens.at(0) );
          this->name = tokens.at(1);
          this->min =  std::stof( tokens.at(2) );
          this->max =  std::stof( tokens.at(3) );
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "error loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "error loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
        }

        if ( this->addr < 1 ) {
          message.str(""); message << "error: addr " << this->addr << " cannot be < 1";
          logwrite( function, message.str() );
          return( ERROR );
        }

        if ( this->min < 0 ) {
          message.str(""); message << "error: min " << this->min << " cannot be < 0";
          logwrite( function, message.str() );
          return( ERROR );
        }

        return( NO_ERROR );
      }
  };
  /** ControllerInfo **********************************************************/


  /** Interface ***************************************************************/
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
      bool class_initialized;
      int numdev;
      float maxwidth;
      float minwidth;
    public:
      std::string name;
      std::string host;
      int port;
      size_t leftcon;
      size_t rightcon;

      Interface();
      ~Interface();

      // This is a vector of all daisy-chain connected controllers
      //
      std::vector<Slit::ControllerInfo> controller_info;

      long initialize_class();
      long open();
      long close();
      long home();
      long set( std::string args, std::string &retstring );
      long get( std::string &retstring );
      long move_abs( std::string args );
      long move_rel( std::string args );
      long stop();
      long send_command( std::string cmd );
      long send_command( std::string cmd, std::string &retstring );

      Physik_Instrumente::ServoInterface pi;

  };
  /** Interface ***************************************************************/

}
#endif
