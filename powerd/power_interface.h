#ifndef POWER_INTERFACE_H
#define POWER_INTERFACE_H

#include <map>

#include "wti.h"
#include "network.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>

namespace Power {

  /** NpsInfo *****************************************************************/
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
      int         npsnum;    /// an integer assigned to each NPS as an identifier
      int         maxplugs;  /// the number of outlets on this NPS
      std::string host;      /// host name/IP for this NPS
      int         port;      /// port number for this NPS

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
  /** NpsInfo *****************************************************************/


  /** Interface ***************************************************************/
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
      size_t numdev;                                       /// number of NPS devices, or "units"

    public:
      Interface();
      ~Interface();

      std::map< int, Power::NpsInfo > nps_info;            /// STL map of NpsInfo objects indexed by NPS unit#
      std::map< int, WTI::NPS >       nps;                 /// STL map of WTI NPS objects indexed by NPS unit#

      typedef struct {
        int npsnum;                                        /// nps number
        int plugnum;                                       /// plug number
      } plug_t;                                            /// structure to contain location of a plug, both nps and plug number
      std::map< std::string, plug_t > plugmap;             /// STL map of plug number indexed by plug name,
                                                           /// allows finding {npsnum,plugnum} by plugname

      void configure_interface( Power::NpsInfo npsinfo );  /// configure the NPS interface vector with info from configuration file
      long initialize_class();                             /// initialize class variables
      long open();                                         /// open the NPS socket connection
      long close();                                        /// close the NPS socket connection
      bool isopen();                                       /// is the NPS socket connection open?
      long command( std::string cmd, std::string &retstring );

  };
  /** Interface ***************************************************************/

}
#endif