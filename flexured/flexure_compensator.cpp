/**
 * @file    flexure_compensator.cpp
 * @brief   this contains the flexure compensator code
 * @author  David Hale <dhale@astro.caltech.edu> & Matt
 *
 */

#include "flexure_compensator.h"

namespace Flexure {

  Compensator::Compensator() {
    for (const auto &chan : { "U", "G", "R", "I" }) {
      for (const auto &axis : { "X", "Y" }) {
        position_coefficients[{chan,axis}] = std::vector<double>();
        flexure_polynomials[{chan,axis}] = std::vector<double>();
      }
    }

    this->trigfunction["R"] = TrigFunction::Sine;
    this->trigfunction["I"] = TrigFunction::Cosine;

  }

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
      case PIX: return "PIY";
      case PIY: return "PIY";
      case PRX: return "PRX";
      case PRY: return "PRY";
      default:  return "UNKNOWN";
    }
  }
  /***** Flexure::Compensator::datavec_name ***********************************/


  /***** Flexure::Compensator::load_position_coefficients *********************/
  /**
   * @brief      loads position coefficients from configuration file
   * @param[in]  config  configuration line
   * @param[in]  type    one of VectorType enum to specify which vector map to load
   * @return     ERROR|NO_ERROR
   */
  long Compensator::load_vector_from_config(std::string &config, VectorType type) {
    const std::string function("Flexure::Compensator::load_position_coefficients");
    std::vector<std::string> tokens;
    Tokenize(config, tokens, " ");

    size_t vecsize;
    vector_map_t* vecmap;

    if (type==VectorType::POSITION_COEFFICIENTS) {
      vecmap  = &this->position_coefficients;
      vecsize = 3;
    }
    else
    if (type==VectorType::FLEXURE_POLYNOMIALS) {
      vecmap  = &this->flexure_polynomials;
      vecsize = 20;
    }
    else {
      logwrite(function, "ERROR invalid vector type");
      return ERROR;
    }

    // expect <CHAN> <AXIS> <COEFF> <COEFF> <COEFF> ...
    //
    if (tokens.size() != vecsize ) {
      std::ostringstream oss;
      oss << "ERROR got \"" << config << "\" but expected <chan> <axis> ... (" << vecsize << " values)";
      logwrite(function, oss.str());
      return ERROR;
    }

    // the vector is in a map indexed by channel and axis
    //
    std::string chan = tokens[0];
    std::string axis = tokens[1];

    if (vecmap->find({chan,axis}) == vecmap->end()) {
      logwrite(function, "ERROR invalid chan,axis: "+chan+","+axis);
      return ERROR;
    }

    // erase the vector and load it with all of the provided values
    //
    try {
      (*vecmap)[{chan,axis}].clear();
      for (int tok=2; tok<vecsize; tok++) {
        (*vecmap)[{chan,axis}].push_back(std::stod(tokens[tok]));
      }
    }
    catch (const std::exception &e) {
      logwrite(function, "ERROR parsing \""+config+"\"");
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Flexure::Compensator::load_position_coefficients *********************/


  /***** Flexure::Compensator::flexure_polynomial_fit *************************/
  /**
   * @brief      fits a 4th order polynomial to inputvar
   * @details    Uses inputvar for the fitting variable and 4 coefficients from
   *             supplied vector starting with offset. Requires that vector have
   *             at least 5 coefficients.
   * @param[in]  vec       vector of coefficients
   * @param[in]  inputvar  independent input variable for the polynomial fit
   * @param[in]  offset    offset in vector to start reading coefficients
   * @throws     std::out_of_range
   *
   */
  double Compensator::flexure_polynomial_fit(double vec, double inputvar, size_t offset) {
    const std::string function("Flexure::Compensator::flexure_polynomial_fit");
    if (offset+5 > vec.size()) {
      logwrite(function, "ERROR not enough coefficients in vector for requested offset");
      throw std::out_of_range("not enough coefficients in vector");
    }

    return this->flexure_polynomials.at(which)[offset + 0]
         + this->flexure_polynomials.at(which)[offset + 1] * inputvar
         + this->flexure_polynomials.at(which)[offset + 2] * std::pow(inputvar, 2.0)
         + this->flexure_polynomials.at(which)[offset + 3] * std::pow(inputvar, 3.0)
         + this->flexure_polynomials.at(which)[offset + 4] * std::pow(inputvar, 4.0);
    return vec[offset + 0]
         + vec[offset + 1] * inputvar
         + vec[offset + 2] * std::pow(inputvar, 2.0)
         + vec[offset + 3] * std::pow(inputvar, 3.0)
         + vec[offset + 4] * std::pow(inputvar, 4.0);
  }
  /***** Flexure::Compensator::flexure_polynomial_fit *************************/


  /***** Flexure::Compensator::calculate_shift ********************************/
  /**
   * @brief      calculates the shift(chan,axis) of the spectrum on the detector
   * @details    C + A1 * sin(cass-theta) + A2 * sin(2*(cass-theta)) or
   *             C + A1 * cos(cass-theta) + A2 * cos(2*(cass-theta))
   *             Input coefficients are a function of (chan,axis) so the output
   *             shift will also be a function of (chan,axis).
   * @param[in]  coefficients  vector of coefficients data
   * @param[in]  func  type of trig function to use, Sine or Cosine
   * @return     fitted value
   * @throws     std::exception
   *
   */
  /**
   * @brief      calculates the shift of the spectrum on the detector
   * @details    shift is function of (chan, axis)
   *             needs cass, alt, and coefficients(chan,axis)
   *
   */
  double Compensator::calculate_shift(std::pair<std::string,std::string> which) {
//double Compensator::calculate_shift(double coefficients, TrigFunction func) {
    const std::string function("Flexure::Compensator::calculate_shift");
    try {
      double c     = flexure_polynomial_fit(coefficients, zenith,  0);
      double a1    = flexure_polynomial_fit(coefficients, zenith,  5);
      double theta = flexure_polynomial_fit(coefficients, zenith, 10);
      double a2    = flexure_polynomial_fit(coefficients, zenith, 15);

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
  /***** Flexure::Compensator::calculate_shift ********************************/


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
                                                 double &prx, double &pry, double &pix, double &piy,
                                                 double nrx, double nry, double nix, double niy,
                                                 double &arx, double &ary, double &aix, double &aiy) {

    double alt, az, pa;         // elevation, azimuth, and parallactic angle from TCS
    double zenith;              // zenith angle
    double srx, sry, six, siy;  // intermediate computation step -- expected flexure
    double drx, dry, dix,diy;   // computed correction deltas
    double tx, ty;              // temporary.

    // adjust hour angle for exposure time midpoint
    double adjusted_hour_angle = ha + exptime / 2.0 / 3600.0 * 15.0;

    double equivalent_cass     = (-(pa + cassring) + 180) % 360 - 180;

    // calculate flexure of each axis
    srx = calculate_shift(this->prx, Sine);
    sry = calculate_shift(this->pry, Sine);
    six = calculate_shift(this->pix, Cosine);
    siy = calculate_shift(this->piy, Cosine);


    collimator_position(srx, sry, &drx, &dry, &tx, &ty);
    collimator_position(six, siy, &tx, &ty, &dix, &diy);

    // ajusted stage positions
    arx =  drx + nrx;
    ary = -dry + nry;
    aix =  dix + nix;
    aiy = -diy + niy;
  }
  /***** Flexure::Compensator::compute_flexure_compensation ******************/



  /**
   * @brief      calculates the adjustments needed to compensate for a shift
   * @details    input is output of calculate_shift()
   *             output is correction to apply to flexure actuator position
   *
   */
  void Compensator::calculate_compensation(std::pair<std::string,std::string> which) {

    calculate_shift
  }


  long test() {
    this->calculate_shift( { "R", "X" } );
    return NO_ERROR;
  }

}
