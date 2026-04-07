/** ---------------------------------------------------------------------------
 * @file    slicecam_math.h
 * @brief   slicecam math utilities
 * @details Declares structs and the Math class used for centroid detection,
 *          WCS pixel-to-sky conversion, and fine-acquisition offset calculation.
 * @author  David Hale, Christoffer Fremling
 *
 */

#pragma once

#include <utility>
#include <vector>
#include <cmath>
#include <algorithm>
#include "common.h"   ///< for Common::FitsKeys

namespace Slicecam {

  struct Point {      ///< pixel coordinate
    double x = 0.0; double y = 0.0;
    bool is_valid() const noexcept {
      return !std::isnan(x) && !std::isnan(y) &&
             x >= 0.0 && y >= 0.0;
    }
  };

  struct Rect  {      ///< rectangular region
    long x1 = 1; long x2 = 1; long y1 = 1; long y2 = 1;
    bool is_valid() const noexcept {
      return x1>0 && x2>0 && y1>0 && y2>0 && x1 != x2 && y1 != y2;
    }
  };

  struct World {      ///< sky coordinates
    double ra  = 0.0; double dec = 0.0;
    bool is_valid() const noexcept {
      return !std::isnan(ra) && !std::isnan(dec) &&
             ra >= 0.0 && dec >= 0.0;
    }
  };

  /***** Slicecam::Math *******************************************************/
  /**
   * @brief  Static math utilities for slicecam fine acquisition
   *
   */
  class Math {
    public:
      /**
       * @brief      compute the centroid of the brightest source near an aim point
       */
      static long calculate_centroid( const std::vector<float> &image,
                                      long cols, long rows,
                                      Rect background,
                                      Point aimpoint,
                                      Point &centroid );
      /**
       * @brief      convert pixel coordinates to sky coordinates using WCS keys
       */
      static void pix2world( const Common::FitsKeys &keys, Point pix, World &world );

      /**
       * @brief      compute the (dRA, dDEC) offset from a goal position to a star
       */
      static void calculate_acquisition_offsets( World star, World goal,
                                                 std::pair<double, double> &offsets );

  };
  /***** Slicecam::Math *******************************************************/

}
