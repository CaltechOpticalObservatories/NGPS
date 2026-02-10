/** ---------------------------------------------------------------------------
 * @file     tcs_info.h
 * @brief    defines a class in Flexure for holding TCS info
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <chrono>
#include <optional>

namespace Flexure {

  /***** Flexure::TcsInfo *****************************************************/
  /**
   * @brief    contains TCS telemetry
   *
   */
  class TcsInfo {
    private:
      double zenangle;
      double casangle;
      double pa;
      double equivalentcass;

      bool is_timeset;
      std::chrono::system_clock::time_point telemetry_time;

    public:

      TcsInfo()
        : zenangle(0), casangle(0), pa(0), equivalentcass(0), is_timeset(false) { }

      double get_zenangle()       { return this->zenangle; }
      double get_casangle()       { return this->casangle; }
      double get_pa()             { return this->pa;       }
      double get_equivalentcass() { return this->equivalentcass; }

      /***** Flexure::TcsInfo:is_older_than ***********************************/
      /**
       * @brief      is the TCS telemetry more than <sec> seconds old?
       * @details    When the telemetry is stored in this class it is time-stamped.
       *             This returns true if that time stamp is more than <sec>
       *             seconds ago from now.
       * @param[in]  sec  number of seconds
       * @return     true|false
       */
      bool is_older_than(std::chrono::seconds sec) {
        if (!this->is_timeset) return true;
        if (std::chrono::system_clock::now() > this->telemetry_time+sec) {
          return true;
        }
        return false;
      }
      /***** Flexure::TcsInfo:is_older_than ***********************************/

      /***** Flexure::TcsInfo:set *********************************************/
      /**
       * @brief      timestamp telemetry
       * @details    Call this function when
       *             This returns true if that time stamp is more than <sec>
       *
       */
      void store(const double &_zenangle,
                 const double &_casangle,
                 const double &_pa,
                 const std::optional<double> &_equivalentcass=std::nullopt) {

        // store the values received
        this->zenangle = _zenangle;
        this->casangle = _casangle;
        this->pa       = _pa;

        // if equivalentcass provided then use it
        if (_equivalentcass) {
          this->equivalentcass = *_equivalentcass;
        }
        // otherwise calculate it
        else {
          double angle = this->casangle - this->pa;
          if (angle < -180.0) angle += 360.0;
          if (angle >  180.0) angle -= 360.0;
          this->equivalentcass = angle;
        }

        // record the time that telemetry was received
        this->telemetry_time = std::chrono::system_clock::now();
        this->is_timeset = true;
      }
      /***** Flexure::TcsInfo:set *********************************************/

  };
  /***** Flexure::TcsInfo *****************************************************/

}
