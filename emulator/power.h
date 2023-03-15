/**
 * @file    power.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef POWER_H
#define POWER_H

#include <atomic>
#include <mutex>
#include <chrono>
#include <numeric>
#include <functional>
#include <thread>
#include <fenv.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "logentry.h"
#include "network.h"

/***** PowerEmulator **********************************************************/
/**
 * @namespace PowerEmulator
 * @brief     this namespace contains everything for the power emulator
 *
 */
namespace PowerEmulator {

  /***** PowerEmulator::NpsInfo ***********************************************/
  /**
   * @class  NpsInfo
   * @brief  NPS information class
   *
   * This class defines the information for each emulated "Network Power Switch" (NPS)
   *
   */
  class NpsInfo {
    private:
    public:
      int         npsnum;            ///< an integer assigned to each NPS as an identifier
      int         maxplugs;          ///< the number of outlets on this NPS
      std::string host;              ///< host name/IP for this NPS
      int         port;              ///< port number for this NPS

//    std::vector<int> plugstate;    ///< vector of plug states supported by this NPS
      std::map<int, int> plugstate;  ///< STL map of plug states for this NPS indexed by plugnum

      NpsInfo();
      ~NpsInfo();

      long load_nps_info( std::string &input, int &npsnum ); ///< loads NPS information from the configuration file into the class

      long load_plug_info( std::string &input, int &npsnum, int &plugnum, std::string &plugname );  ///< loads plug information from the configuration file into the class

  };
  /***** PowerEmulator::NpsInfo ***********************************************/


  /***** PowerEmulator::Interface *******************************************/
  /**
   * @class  Interface
   * @brief  interface to the emulated "hardware"
   *
   */
  class Interface {
    private:
    public:
      Interface();
      ~Interface();

      std::map< int, PowerEmulator::NpsInfo > nps_info;                          ///< STL map of NpsInfo objects indexed by NPS unit#

      long parse_command( int npsnum, std::string cmd, std::string &retstring ); ///< parse commands for the emulated NPS

  };
  /***** PowerEmulator::Interface *******************************************/

}
/***** PowerEmulator **********************************************************/
#endif
