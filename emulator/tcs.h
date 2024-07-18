/**
 * @file    tcs.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#pragma once

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
#include <cmath>

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "logentry.h"
#include "network.h"
#include "tcs_constants.h"
#include "skyinfo.h"


/***** TcsEmulator ************************************************************/
/**
 * @namespace TcsEmulator
 * @brief     this namespace contains everything for the TCS emulator
 *
 */
namespace TcsEmulator {


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

      SkyInfo::FPOffsets fpoffsets;          /// for calling Python fpoffsets, defined in ~/Software/common/skyinfo.h

      void initialize_python_objects();        /// provides interface to initialize all Python modules for objects in this class

      volatile std::atomic<bool> focussing;  ///< set true during fake focus moves, prevents another thread from focussing the telescope
      volatile std::atomic<bool> telmoving;  ///< set true during fake telescope moves, prevents another thread from moving the telescope
      volatile std::atomic<bool> casmoving;  ///< set true during fake cass rotator moves, prevents another thread from moving the cass ring

      // default slew and settling times set in constructor but can be overridden by configuration file
      //
      std::string name;          ///< optional target name
      double focusrate;          ///< focusrate in mm/s
      double slewrate_ra;        ///< slewrate in s for RA
      double slewrate_dec;       ///< slewrate in s for DEC
      double slewrate_casangle;  ///< slewrate in s for CASANGLE
      double settle_ra;          ///< settling time in s for RA
      double settle_dec;         ///< settling time in s for DEC
      double settle_casangle;    ///< settling time in s for CASANGLE
      double offsetrate_ra;      ///< offset rate RA in arcsec/sec
      double offsetrate_dec;     ///< offset rate DEC in arcsec/sec

      volatile std::atomic<double> focus;
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

      std::string get_time();

      static void do_focus( TcsEmulator::Telescope &telescope, double focusval, int movetype );   ///< perform the FOCUSGO command work, which "moves" the focus
      static void do_ringgo( TcsEmulator::Telescope &telescope, double newring );   ///< perform the RINGGO command work, which "moves" the cass rotator
      static void do_coords( TcsEmulator::Telescope &telescope, std::string args ); ///< perform the COORDS command work, which "moves" the telescope
      static void do_pt( TcsEmulator::Telescope &telescope, std::string args ); ///< perform the PT command work, which "offsets" the telescope
      void weather( std::string &retstring );
      void reqstat( std::string &retstring );
      void reqpos( std::string &retstring );
      void mrates( std::string args, std::string &retstring );
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

      Telescope telescope;

      std::map<int, std::string> map_returnval;
      std::map<int, std::string> map_motionval;

      long parse_command( std::string cmd, std::string &retstring );  ///< parse commands for the TCS

  };
  /***** TcsEmulator::Interface ***********************************************/

}
/***** TcsEmulator ************************************************************/
