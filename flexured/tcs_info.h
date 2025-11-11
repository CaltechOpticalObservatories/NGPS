#pragma once

#include <chrono>

namespace Flexure {

  /***** Flexure::TcsInfo *****************************************************/
  /**
   * @brief    contains TCS telemetry
   *
   */
  class TcsInfo {
    private:
      bool is_timeset;
      std::chrono::system_clock::time_point telemetry_time;

    public:
      double zenangle;
      double equivalent_cass;
      double pa;
      double casangle;

      TcsInfo()
        : is_timeset(false), zenangle(0), equivalent_cass(0), pa(0), casangle(0) { }

      bool is_older_than(std::chrono::seconds sec) {
        if (!this->is_timeset) return true;
        if (std::chrono::system_clock::now() > this->telemetry_time+sec) {
          return true;
        }
        return false;
      }

      void set() {
        double angle = this->pa + this->casangle;

        if (angle < -180.0) angle += 360.0;
        if (angle >  180.0) angle -= 360.0;

        this->equivalent_cass = angle;

        // record the time that telemetry was received
        this->telemetry_time = std::chrono::system_clock::now();
        this->is_timeset = true;
      }

  };
  /***** Flexure::TcsInfo *****************************************************/

}
