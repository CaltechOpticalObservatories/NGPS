/**
 * @file    tcs.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef TCS_H
#define TCS_H

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

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "logentry.h"
#include "network.h"

/***** TcsEmulator ************************************************************/
/**
 * @namespace TcsEmulator
 * @brief     this namespace contains everything for the TCS emulator
 *
 */
namespace TcsEmulator {

  const int MOTION_STOPPED         =  0;  ///< the real TCS reports this when stopped
  const int MOTION_SLEWING         =  1;  ///< the real TCS reports this when slewing
  const int MOTION_OFFSETTING      =  2;  ///< the real TCS reports this when offsetting
  const int MOTION_TRACKING_STABLY =  3;  ///< the real TCS reports this when tracking stable
  const int MOTION_SETTLING        = -1;  ///< the real TCS reports this when settling
  const int MOTION_UNKNOWN         = -2;  ///< the real TCS reports this when motion unknown

  /***** TcsEmulator::Telescope ***********************************************/
  /**
   * @class  Telescope
   * @brief  The telescope class contains all the information for the fake telescope,
   *         where it's pointed and how to "move" it.
   *
   */
  class Telescope {
    private:
      int telid;

    public:
      Telescope();
      ~Telescope();

      volatile std::atomic<bool> coords_running;  ///< set true during the fake slew, prevents another thread from moving the telescope

      // default slew and settling times set in constructor but can be overridden by configuration file
      //
      double slewrate_ra;        ///< slewrate in s for RA
      double slewrate_dec;       ///< slewrate in s for DEC
      double slewrate_casangle;  ///< slewrate in s for CASANGLE
      double settle_ra;          ///< settling time in s for RA
      double settle_dec;         ///< settling time in s for DEC
      double settle_casangle;    ///< settling time in s for CASANGLE

      double focus;
      double tubelength;
      volatile std::atomic<double> ra;
      volatile std::atomic<double> dec;
      double offset_ra;
      double offset_dec;
      double trackrate_ra;
      double trackrate_dec;
      double casangle;
      double ha;
      double lst;
      double airmass;
      double azimuth;
      double zangle;

      volatile std::atomic<int> motionstate;      ///< telescope motion state

      static void do_coords( TcsEmulator::Telescope &telescope, std::string args ); ///< perform the COORDS command work, which "moves" the telescope
  };
  /***** TcsEmulator::Telescope ***********************************************/


  /***** TcsEmulator::Interface ***********************************************/
  /**
   * @class  Interface
   * @brief  TCS emulator interface class
   *
   * Interfaces the tcs emulator daemon to the emulated tcs
   *
   */
  class Interface {
    private:
    public:
      Interface();
      ~Interface();

      Telescope telescope;

      long parse_command( std::string cmd, std::string &retstring );  ///< parse commands for the TCS

  };
  /***** TcsEmulator::Interface ***********************************************/

}
/***** TcsEmulator ************************************************************/
#endif
