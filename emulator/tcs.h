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

namespace Tcs {

  const int MOTION_STOPPED         =  0;
  const int MOTION_SLEWING         =  1;
  const int MOTION_OFFSETTING      =  2;
  const int MOTION_TRACKING_STABLY =  3;
  const int MOTION_SETTLING        = -1;
  const int MOTION_UNKNOWN         = -2;

  /** Telescope ***************************************************************/
  /**
   * @class  Telescope
   * @brief  
   *
   */
  class Telescope {
    private:
      int telid;

    public:
      Telescope();
      ~Telescope();

      volatile std::atomic<bool> coords_running;

      // default slew and settling times set in constructor but can be overridden by configuration file
      //
      double slewrate_ra;        /// slewrate in s for RA
      double slewrate_dec;       /// slewrate in s for DEC
      double slewrate_casangle;  /// slewrate in s for CASANGLE
      double settle_ra;          /// settling time in s for RA
      double settle_dec;         /// settling time in s for DEC
      double settle_casangle;    /// settling time in s for CASANGLE

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

      volatile std::atomic<int> motionstate;      /// telescope motion state

      static void do_coords( Tcs::Telescope &telescope, std::string args );
  };
  /** Telescope ***************************************************************/


  /** Interface ***************************************************************/
  /**
   * @class  Interface
   * @brief  
   *
   */
  class Interface {
    private:
    public:
      Interface();
      ~Interface();

      Telescope telescope;

      long parse_command( std::string cmd, std::string &retstring );

    };
  /** Interface ***************************************************************/


}
#endif
