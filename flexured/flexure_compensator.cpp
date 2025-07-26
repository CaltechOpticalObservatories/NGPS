/**
 * @file    flexure_compensator.cpp
 * @brief   this contains the flexure compensator code
 * @author  David Hale <dhale@astro.caltech.edu> & Matt
 *
 */

#include "flexure_compensator.h"

namespace Flexure {

  /***** Flexure::Compensator::datavec_name ***********************************/
  /**
   * @brief  returns the name of a vector type
   * @param[in]  vectype  DataVectorType
   * @return     string
   */
  std::string Compensator::datavec_name(DataVectorType vectype) {
    switch (vectype) {
      case RXE: return "RXE";
      case RYE: return "RYE";
      case IXE: return "IXE";
      case IYE: return "IYE";
      case IX: return "IY";
      case IY: return "IY";
      case RX: return "RX";
      case RY: return "RY";
      default:  return "UNKNOWN";
    }
  }
  /***** Flexure::Compensator::datavec_name ***********************************/


  /***** Flexure::Compensator::load_vector_from_config ************************/
  /**
   * @brief      loads data from Config file into the appropriate vector
   * @param[in]  config   configuration line
   * @param[in]  vectype  DataVectorType specifies which vector to load
   * @param[in]  vecsize  number of elements in this vector
   *
   */
  long Compensator::load_vector_from_config(std::string &config, DataVectorType vectype, size_t vecsize) {
    const std::string function("Flexure::Compensator::load_vector_from_config");

    if (vecsize<1) {
      logwrite(function, "ERROR requested zero vector size for "+datavec_name(vectype));
      return ERROR;
    }

    std::vector<double>* vec=nullptr;  // reference to specified class vector

    // select which vector to load
    switch (vectype) {
      case DataVectorType::RXE: vec = &this->rxe;
                                break;
      case DataVectorType::RYE: vec = &this->rye;
                                break;
      case DataVectorType::IXE: vec = &this->ixe;
                                break;
      case DataVectorType::IYE: vec = &this->iye;
                                break;
      case DataVectorType::IX:  vec = &this->ix;
                                break;
      case DataVectorType::IY:  vec = &this->iy;
                                break;
      case DataVectorType::RX:  vec = &this->rx;
                                break;
      case DataVectorType::RY:  vec = &this->ry;
                                break;
      default:
        logwrite(function, "ERROR invalid vector type");
        return ERROR;
    }

    std::vector<std::string> tokens;
    Tokenize(config, tokens, " ");

    if (tokens.size() != vecsize) {
      logwrite(function, "ERROR "+std::to_string(tokens.size())
                        +" tokens do not match vector size "+std::to_string(vecsize)
                        +" for "+datavec_name(vectype));
      return ERROR;
    }

    // erase and load vector
    try {
      vec->clear();
      for (const auto &tok : tokens) vec->push_back( std::stod(tok) );
    }
    catch (const std::exception &e) {
      logwrite(function, "ERROR parsing data from "+datavec_name(vectype)+" data: "+std::string(e.what()));
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Flexure::Compensator::load_vector_from_config ************************/


  /***** Flexure::Compensator::flexure_polynomial_fit *************************/
  /**
   * @brief      fits a 4th order polynomial to inputvar
   * @details    Uses inputvar for the fitting variable and 4 coefficients from
   *             supplied vector starting with offset. Requires that vector have
   *             at least 5 coefficients.
   * @param[in]  vec       vector of coefficients
   * @param[in]  inputvar  independent input variable for the polynomial fit
   * @param[in]  offset    offset in vector to start reading coefficients
   *
   */
  double Compensator::flexure_polynomial_fit(double vec, double inputvar, size_t offset) {
    const std::string function("Flexure::Compensator::flexure_polynomial_fit");
    if (offset+5 > vec.size()) {
      logwrite(function, "ERROR not enough coefficients in vector for requested offset");
      throw std::out_of_range("not enough coefficients in vector");
    }
    return vec[offset + 0]
         + vec[offset + 1] * inputvar
         + vec[offset + 2] * std::pow(inputvar, 2.0)
         + vec[offset + 3] * std::pow(inputvar, 3.0)
         + vec[offset + 4] * std::pow(inputvar, 4.0);
  }
  /***** Flexure::Compensator::flexure_polynomial_fit *************************/


  /***** Flexure::Compensator::flexure_fit ************************************/
  /**
   * @brief
   * @param[in]  func  type of trig function to use, Sine or Cosine
   * @return
   *
   */
  double Compensator::flexure_fit(TrigFunction func) {
    const std::string function("Flexure::Compensator::flexure_fit");
    try {
      double c     = flexure_polynomial_fit(rx, zenith,  0);
      double a1    = flexure_polynomial_fit(rx, zenith,  5);
      double theta = flexure_polynomial_fit(rx, zenith, 10);
      double a2    = flexure_polynomial_fit(rx, zenith, 15);

      switch (func) {
        case TrigFunction::Sine:
          return c + a1 * std::sin(  (equivalent_cass * DEGTORAD - theta))
                   + a2 * std::sin(2*(equivalent_cass * DEGTORAD - theta));
        case TrigFunction::Cosine:
        default:
          return c + a1 * std::cos(  (equivalent_cass * DEGTORAD - theta))
                   + a2 * std::cos(2*(equivalent_cass * DEGTORAD - theta));
      }
    }
    catch (const std::exception &e) {
      logwrite(function, "ERROR: "+std::string(e.what()));
      throw;
    }
  }
  /***** Flexure::Compensator::flexure_fit ************************************/


  /***** Flexure::Compensator::compute_flexure_compensation ******************/
  /**
   * @brief      
   * @details    
   *
   * @param[in]  ha  hour angle in decimal degrees.
   * @param[in]  dec declination in decimal degrees.
   * @param[in]  cassring
   * @param[in]  exptime  exposure time in seconds for future calculation
   * @param[in]  prx, pry, pix, piy -- polynomial coefficients to compute fit parameters
   * @param[in]  nrx, nry, nix, niy -- nominal stage positions
   * @param[out] arx-- adjusted stage positions 
   * @param[out] ary-- adjusted stage positions 
   * @param[out] aix-- adjusted stage positions 
   * @param[out] aiy -- adjusted stage positions 
   *
   */
  void Compensator::compute_flexure_compensation(double ha, double dec, double cassring, double exptime,
                                                 double *prx, double *pry, double *pix, double *piy,
                                                 double nrx, double nry, double nix, double niy,
                                                 double *arx, double *ary, double *aix, double *aiy) {

    double alt, az, pa;         // elevation, azimuth, and parallactic angle from TCS
    double zenith;              // zenith angle
    double srx, sry, six, siy;  // intermediate computation step -- expected flexure
    double drx, dry, dix,diy;   // computed correction deltas
    double tx, ty;              // temporary.

    // adjust hour angle for exposure time midpoint
    double adjusted_hour_angle = ha + exptime / 2.0 / 3600.0 * 15.0;

    double equivalent_cass     = (-(pa + cassring) + 180) % 360 - 180;

    // calculate flexure of each axis
    srx = flexure_fit(equivalent_cass, alt, prx);
    sry = flexure_fit(equivalent_cass, alt, pry);
    six = flexure_fit_cos(equivalent_cass, alt, pix);
    siy = flexure_fit_cos(equivalent_cass, alt, piy);


    collimator_position(srx, sry, &drx, &dry, &tx, &ty);
    collimator_position(six, siy, &tx, &ty, &dix, &diy);

    // ajusted stage positions
    *arx =  drx + nrx;
    *ary = -dry + nry;
    *aix =  dix + nix;
    *aiy = -diy + niy;
  }
  /***** Flexure::Compensator::compute_flexure_compensation ******************/

}
