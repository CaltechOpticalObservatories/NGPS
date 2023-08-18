/** ---------------------------------------------------------------------------
 * @file    thermal_interface.h
 * @brief   thermal interface include
 * @details defines the classes used by the temperature controller hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef THERMAL_INTERFACE_H
#define THERMAL_INTERFACE_H

#include "network.h"
#include "lks.h"
#include "logentry.h"
#include "common.h"
#include <string>
#include <cmath>
#include <ctgmath>
#include <sys/stat.h>
#include <map>
#include <condition_variable>
#include <atomic>

#define MOVE_TIMEOUT 20000  ///< number of milliseconds before a move fails

/***** Thermal ****************************************************************/
/**
 * @namespace Thermal
 * @brief     namespace for the thermal daemon
 *
 */
namespace Thermal {

  const std::string DAEMON_NAME = "thermald";     ///< when run as a daemon, this is my name

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
      std::vector<std::string> tempchans;         ///< vector of temperature channels
      std::vector<std::string> templabels;        ///< vector of temperature channel labels
      std::vector<std::string> heaters;           ///< vector of heater channels
      std::vector<std::string> heatlabels;        ///< vector of heater channel labels

      std::map<std::string, std::string> data;    ///< STL map of readings indexed by channel

      LKS::Interface lks;                         ///< Object for communicating with the Lakeshore

      inline std::string device_name() { return this->lks.get_name(); }  ///< return Lakeshore device name
      inline long open() { return this->lks.open(); }                    ///< open Lakeshore device
      inline long close() { return this->lks.close(); }                  ///< close Lakeshore device

      long read_temp( std::string chan );                                ///< read specified temperature channel into class
      long read_temp( std::string chan, float &tempval );                ///< read specified temperature channel and return value
      long read_heat( std::string chan );                                ///< read specified heater into class
      long read_heat( std::string chan, float &heat );                   ///< read specified heater and return value

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
      bool class_initialized;
    public:
      Interface();
      ~Interface();

      Common::Queue async;

      std::map<int, Thermal::Lakeshore> lakeshore;  ///< STL map of all Lakeshores indexed by LKS#

      std::map<std::string, std::string> data;      ///< STL map of readings indexed by channel

      /**
       * @typedef thermal_t
       * @brief   structure of thermal data database
       */
      typedef struct {
        std::string device;                         ///< device type e.g. LKS, CAMP, etc.
        int unit;                                   ///< user-assigned unit number
        std::string model;                          ///< mfg model number e.g. 318, 325 CR1000, etc.
        std::string chan;                           ///< channel number e.g. A, B, C1, H1, 1, etc.
        std::string label;                          ///< channel label (must be unique)
      } thermal_info_t;

      std::map<std::string, thermal_info_t> info;   ///< thermal info database, indexed by channel label

      long initialize_class();
      long read_all();                              ///< read all thermal data (into memory)
      long log_all();                               ///< log all thermal data (to disk)

  };
  /***** Thermal::Interface ***************************************************/

}
/***** Thermal ****************************************************************/
#endif
