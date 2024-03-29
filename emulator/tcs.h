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
#include <map>
#include <cmath>

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "logentry.h"
#include "network.h"
#include "tcs_constants.h"
#include "cpython.h"

#define PYTHON_PATH "/home/developer/Software/Python/acam_skyinfo"
#define PYTHON_FPOFFSETS_MODULE "FPoffsets"
#define PYTHON_APPLYOFFSETDEG_FUNCTION "apply_offset_deg"

/***** TcsEmulator ************************************************************/
/**
 * @namespace TcsEmulator
 * @brief     this namespace contains everything for the TCS emulator
 *
 */
namespace TcsEmulator {

  /***** TcsEmulator::FPOffsets ***********************************************/
  /**
   * @class   FPOffsets
   * @brief   class for calling FPOffsets python functions
   * @details makes use of CPython::CPytInstance defined in cpython.h
   *
   */
  class FPOffsets {
    private:
      char* restore_path;       /// if the PYTHONPATH env variable is changed then remember the original
      bool python_initialized;  /// set true when the Python interpreter has been initialized

    public:
      FPOffsets();
      ~FPOffsets();

      inline bool is_initialized() { return this->python_initialized; };

      CPython::CPyInstance py_instance { PYTHON_PATH };  /// initialize the Python interpreter
      PyObject* pModuleName;
      PyObject* pModule;

      long apply_offset( double ra_in,   double dec_in,
                         double ra_off,  double dec_off,
                         double &ra_out, double &dec_out );
  };
  /***** TcsEmulator::FPOffsets ***********************************************/


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

      FPOffsets fpoffsets;                   ///< instantiate an FPOffsets object needed for calculating offsets

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
      ~Interface();

      Telescope telescope;

      std::map<int, std::string> map_returnval;
      std::map<int, std::string> map_motionval;

      long parse_command( std::string cmd, std::string &retstring );  ///< parse commands for the TCS

  };
  /***** TcsEmulator::Interface ***********************************************/

}
/***** TcsEmulator ************************************************************/
#endif
