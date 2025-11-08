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

  namespace Channel {
    constexpr const char* U = "U";
    constexpr const char* G = "G";
    constexpr const char* R = "R";
    constexpr const char* I = "I";
  }

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

  enum VectorType : size_t {
    POSITION_COEFFICIENTS,
    FLEXURE_POLYNOMIALS
  };

  /***** Flexure::Compensator *************************************************/
  /**
   * @brief    contains functions and data for performing compensation
   * @details  this does not compensate anything, just informs how to compensate
   *
   */
  class Compensator {
    private:
      using vector_map_t = std::map<std::pair<std::string, std::string>, std::vector<double>>;

      vector_map_t position_coefficients;
      vector_map_t flexure_polynomials;

      std::map<std::string, TrigFunction> trigfunction;

      std::vector<double> rxe, rye, ixe, iye, pix, prx, piy, pry;

      void calculate_compensation();

      // From Matt Matuszewski
      //
      void collimator_position(double x, double y, double *rx, double *ry, double *ix, double *iy);
      double flexure_polynomial_fit(double vec, double inputvar, size_t offset);
      double calculate_shift(std::pair<std::string,std::string> which);
//    double calculate_shift(double modified_cass, double elevation,  double *p);
      void compute_flexure_compensation(double ha, double dec, double cassring, double exptime,
                                        double *prx, double *pry, double *pix, double *piy,
                                        double nrx, double nry, double nix, double niy,
                                        double *arx, double *ary, double *aix, double *aiy);

    public:
      Compensator();

      static std::string datavec_name(DataVectorType vectype);
      long load_vector_from_config(std::string &config, VectorType type);

      long test();

      long calculate(/* TBD */);  // presumably the equivalent of Matt's main()
  };
  /***** Flexure::Compensator *************************************************/

}
