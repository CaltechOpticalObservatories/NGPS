/**
 * @file    power_interface.h
 * @brief   
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef POWER_INTERFACE_H
#define POWER_INTERFACE_H

#include <map>

#include "wti.h"
#include "network.h"
#include "logentry.h"
#include "common.h"
#include "powerd_commands.h"
#include <sys/stat.h>

/***** Power ******************************************************************/
/**
 * @namespace Power
 * @brief     namespace for power control
 *
 */
namespace Power {

  const std::string DAEMON_NAME = "powerd";      ///< when run as a daemon, this is my name

  /***** Power::NpsInfo *******************************************************/
  /**
   * @class  NpsInfo
   * @brief  NPS information class
   *
   * This class defines the information for each NPS
   *
   */
  class NpsInfo {
    private:
    public:
      int         npsnum;    ///< an integer assigned to each NPS as an identifier
      int         maxplugs;  ///< the number of outlets on this NPS
      std::string host;      ///< host name/IP for this NPS
      int         port;      ///< port number for this NPS

      NpsInfo();
      ~NpsInfo();


      /**************** Power::NpsInfo::load_nps_info *************************/
      /**
       * @fn         load_nps_info
       * @brief      loads NPS information from the configuration file into the class
       * @param[in]  input
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the NPS_UNIT key is found in the
       * configuration file, to parse and load all of the information assigned
       * by that key into the appropriate NpsInfo class variables.
       *
       * The input string specifies: "<nps#> <maxplugs> <host> <port>"
       *
       */
      long load_nps_info( std::string &input, int &npsnum ) {
        std::string function = "Power::NpsInfo::load_nps_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 4 ) {
          message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 4";
          logwrite( function, message.str() );
          return( ERROR );
        }

        try {
          npsnum         = std::stoi( tokens.at(0) );
          this->maxplugs = std::stoi( tokens.at(1) );
          this->host     = tokens.at(2);
          this->port     = std::stoi( tokens.at(3) );
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
        }

        if ( npsnum < 0 ) {
          message.str(""); message << "ERROR bad NPS unit number " << npsnum << ": must be >= 0";
          logwrite( function, message.str() );
          return( ERROR );
        }
        else this->npsnum = npsnum;

        return( NO_ERROR );
      }
      /**************** Power::NpsInfo::load_nps_info *************************/


      /**************** Power::NpsInfo::load_plug_info ************************/
      /**
       * @fn         load_plug_info
       * @brief      loads plug information from the configuration file into the class
       * @param[in]  input
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the NPS_PLUG key is found in the
       * configuration file, to parse and load all of the information assigned
       * by that key into the appropriate NpsInfo class variables.
       *
       * The input string specifies: "<nps#> <plug#> <plugname>"
       *
       */
      long load_plug_info( std::string &input, int &npsnum, int &plugnum, std::string &plugname ) {
        std::string function = "Power::NpsInfo::load_plug_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 3 ) {
          message.str(""); message << "ERROR bad number of tokens in \"" << input << "\": expected 3 but received " << tokens.size();
          logwrite( function, message.str() );
          return( ERROR );
        }

        try {
          npsnum      = std::stoi( tokens.at(0) );
          plugnum     = std::stoi( tokens.at(1) );
          plugname    = tokens.at(2);
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
        }

        if ( npsnum < 0 ) {
          message.str(""); message << "ERROR bad NPS unit number " << npsnum << ": must be >= 0";
          logwrite( function, message.str() );
          return( ERROR );
        }

        if ( plugnum < 0 ) {
          message.str(""); message << "ERROR bad plug number " << plugnum << ": must be >= 0";
          logwrite( function, message.str() );
          return( ERROR );
        }

        if ( plugname.empty() ) {
          logwrite( function, "ERROR plug name cannot be empty" );
          return( ERROR );
        }

        return( NO_ERROR );
      }
      /**************** Power::NpsInfo::load_plug_info ************************/

  };
  /***** Power::NpsInfo *******************************************************/


  /***** Power::Interface *****************************************************/
  /**
   * @class  Interface
   * @brief  interface class for the network power switch(es)
   *
   * This class defines the interface for all of the power control hardware and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      bool   class_initialized;
      size_t numdev;                                       ///< number of NPS devices, or "units"
      std::map<std::string, int> telemetry_map;            ///< map of plug status 0|1 indexed by plug nam

    public:
      Interface();
      ~Interface();

      Common::Queue async;                                 ///< asynchronous message queue object

      std::map< int, Power::NpsInfo > nps_info;            ///< STL map of NpsInfo objects indexed by NPS unit#
      std::map< int, WTI::NPS >       npsmap;              ///< STL map of WTI NPS objects indexed by NPS unit#
      std::vector<int>                npsvec;              ///< vector of npsnums for indexing

      /**
       * @struct plug_t
       * @brief  structure to contain location of a plug, both nps and plug number
       */
      typedef struct {
        int npsnum;                                        ///< NPS number
        int plugnum;                                       ///< plug number
      } plug_t;

      std::string missing;                                 ///< reports any missing hardware (configured but unresponsive)

      std::map< std::string, plug_t > plugmap;             ///< STL map of plug number indexed by plug name, allows finding {npsnum,plugnum} by plugname

      std::map< std::string, std::string > plugname;       ///< STL map of plug names indexed by a string made of "unit# plug#", e.g. "1 1" or "2 8" etc.

      void configure_interface( Power::NpsInfo npsinfo );  ///< configure the NPS interface vector with info from configuration file
      long initialize_class();                             ///< initialize class variables
      long open();                                         ///< open the NPS socket connection
      long close();                                        ///< close the NPS socket connection
      bool isopen();                                       ///< is the NPS socket connection open?
      long command( std::string cmd, std::string &retstring ); ///< parse and form a command to send to the NPS unit
      void list( std::string args, std::string &retstring );   ///< list plug devices
      long status( std::string args, std::string &retstring ); ///< status of all plug devices
      void make_telemetry_message( std::string &retstring );   ///< make serialized JSON telemetry message

  };
  /***** Power::Interface *****************************************************/

}
/***** Power ******************************************************************/
#endif
