/** ---------------------------------------------------------------------------
 * @file    flexure_interface.h
 * @brief   flexure interface include
 * @details defines the classes used by the flexure hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "common.h"

namespace Flexure {

  constexpr double PI = 3.14159265358979323846;
  constexpr double DEGTORAD = PI/180.0;

  enum class TrigFunction {
    Sine,
    Cosine
  };

  /**
   * @brief 
   */
  enum DataVectorType : size_t {
    RXE,
    RYE,
    IXE,
    IYE,
    PIX,
    PIY,
    PRX,
    PRY
  };

  /***** Flexure::Compensator *************************************************/
  /**
   * @brief    contains functions and data for performing compensation
   * @details  this does not compensate anything, just informs how to compensate
   *
   */
  class Compensator {
    private:
      std::vector<double> rxe, rye, ixe, iye, pix, prx, piy, pry;

      // From Matt Matuszewski
      //
      void collimator_position(double x, double y, double *rx, double *ry, double *ix, double *iy);
      double flexure_polynomial_fit(double elevation, double *p);
      double flexure_fit(double modified_cass, double elevation,  double *p);
      double flexure_fit_cos(double modified_cass, double elevation,  double *p);
      void compute_flexure_compensation(double ha, double dec, double cassring, double exptime,
                                        double *prx, double *pry, double *pix, double *piy,
                                        double nrx, double nry, double nix, double niy,
                                        double *arx, double *ary, double *aix, double *aiy);

    public:
      static std::string datavec_name(DataVectorType vectype);
      long load_vector_from_config(std::string &config, DataVectorType vectype, size_t vecsize);

      long calculate(/* TBD */);  // presumably the equivalent of Matt's main()
  };
  /***** Flexure::Compensator *************************************************/

}
