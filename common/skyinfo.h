/**
 * @file     skyinfo.h
 * @brief    SkyInfo namespace header file
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "cpython.h"
#include <string>
#include <sstream>
#include "common.h"

#define PYTHON_SKYINFO_PATH "/home/developer/Software/Python/acam_skyinfo"
#define PYTHON_FPOFFSETS_MODULE "FPoffsets"
#define PYTHON_FPOFFSETS_FUNCTION "compute_offset"
#define PYTHON_SOLVEOFFSETDEG_FUNCTION "solve_offset_deg"

namespace SkyInfo {

  /***** SkyInfo::FPOffsets ***************************************************/
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

      CPython::CPyInstance py_instance { PYTHON_SKYINFO_PATH };  /// initialize the Python interpreter
      PyObject* pModuleName;
      PyObject* pModule;

    public:
      FPOffsets();

      inline bool is_initialized() { return this->python_initialized; };

      // store the in/out coordinates in the class
      //
      typedef struct {
        double ra;
        double dec;
        double angle;
      } coords_t;

      coords_t coords_in { 0, 0, 0 };
      coords_t coords_out { 0, 0, 0 };

      inline void set_inputs( double dec, double ra, double angle ) {
        this->coords_in.ra=ra;
        this->coords_in.dec=dec;
        this->coords_in.angle=angle;
        return;
      }

      inline void get_outputs( double &dec, double &ra, double &angle ) {
        dec=this->coords_out.dec;
        ra=this->coords_out.ra;
        angle=this->coords_out.angle;
        return;
      }

      // compute_offset() is overloaded
      //
      // It can be called with no coordinates (using the class variables for input/output).
      // It can be called with inputs only (usng the class variables for output).
      // It can be called with both inputs and a reference to output variables so that outputs are returned directly.
      // In all cases, the class variables will be updated with the results.
      //
      long compute_offset( std::string from, std::string to );
      long compute_offset( std::string from, std::string to, double ra_in, double dec_in, double angle_in );
      long compute_offset( std::string from, std::string to,
                           double ra_in, double dec_in, double angle_in,
                           double &ra_out, double &dec_out, double &angle_out );

      long solve_offset( double ra_acam, double dec_acam,
                         double ra_goal, double dec_goal,
                         double &ra_off, double &dec_off );

  };
  /***** SkyInfo::FPOffsets ***************************************************/

}
