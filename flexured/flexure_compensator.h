/** ---------------------------------------------------------------------------
 * @file    flexure_interface.h
 * @brief   flexure interface include
 * @details defines the classes used by the flexure hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "common.h"
#include "tcs_info.h"

namespace Flexure {

  constexpr double PI = 3.14159265358979323846;
  constexpr double DEGTORAD = PI/180.0;

  constexpr const char* X = "X";
  constexpr const char* Y = "Y";

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

  enum VectorType : size_t {
    POSITION_COEFFICIENTS,
    FLEXURE_POLYNOMIALS
  };

  /***** Flexure::Compensator *************************************************/
  /**
   * @brief    contains functions and data for calculating compensation
   * @details  this does not compensate anything, just informs how to compensate
   *
   */
  class Compensator {
    private:
      using vector_map_t = std::map<std::pair<std::string, std::string>, std::vector<double>>;

      TcsInfo &tcs_info;  ///< reference to Interface's TcsInfo

      vector_map_t position_coefficients;
      vector_map_t flexure_polynomials;

      std::map<std::string, TrigFunction> trigfunction;

      double flexure_polynomial_fit(const std::pair<std::string,std::string> &which, double inputvar, size_t offset);
      void compensate_shift_to_delta(const std::string &channel,
                                     const std::pair<double,double> &shift, std::pair<double,double> &delta);

    public:
      Compensator(TcsInfo &info);

      long load_vector_from_config(std::string &config, VectorType type);
      double calculate_shift(const std::pair<std::string,std::string> &which);
      void calculate_compensation(const std::string &channel, std::pair<double,double> &delta);
  };
  /***** Flexure::Compensator *************************************************/

}
