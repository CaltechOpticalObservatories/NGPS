/**
 * @file    flexure_compensator.cpp
 * @brief   this contains the flexure compensator code
 * @author  David Hale <dhale@astro.caltech.edu> & Matt
 *
 */

#include "flexure_compensator.h"

namespace Flexure {

  Compensator::Compensator() {
    // Initialize the map indices.
    // position_coefficients and flexure_polynomials are maps
    // indexed by a pair<chan,axis>
    // These maps will be loaded by Compensator::load_position_coefficients
    //
    for (const auto &chan : { "U", "G", "R", "I" }) {
      for (const auto &axis : { X, Y }) {
        position_coefficients[{chan,axis}] = std::vector<double>();
        flexure_polynomials[{chan,axis}] = std::vector<double>();
      }
    }

    this->trigfunction["R"] = TrigFunction::Sine;
    this->trigfunction["I"] = TrigFunction::Cosine;

  }


  /***** Flexure::Compensator::load_vector_from_config ************************/
  /**
   * @brief      loads position coefficients from configuration file
   * @details    This parses a configuration file row given the specified VectorType
   *             and loads the class map vector specified by VectorType. This will be
   *             either a vector of POSITION_COEFFICIENTS or FLEXURE_POLYNOMIALS,
   *             which are vectors assigned to a map indexed by pair { chan, axis }.
   * @param[in]  config  configuration line
   * @param[in]  type    one of VectorType enum to specify which vector map to load
   * @return     ERROR|NO_ERROR
   */
  long Compensator::load_vector_from_config(std::string &config, VectorType type) {
    const std::string function("Flexure::Compensator::load_vector_from_config");
    std::vector<std::string> tokens;
    Tokenize(config, tokens, " ");

    size_t vecsize;
    vector_map_t* vecmap;

    // assign the vecmap pointer to the appropriate map based on VectorType,
    // which also has a fixed vector size
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
    if (tokens.size() != vecsize+2 ) {
      std::ostringstream oss;
      oss << "ERROR got \"" << config << "\" but expected <chan> <axis> ... (" << vecsize << " values)";
      logwrite(function, oss.str());
      return ERROR;
    }

    // the vector is in a map indexed by pair { chan, axis }
    std::string chan = tokens[0];
    std::string axis = tokens[1];

    if (vecmap->find({chan,axis}) == vecmap->end()) {
      logwrite(function, "ERROR invalid chan,axis: "+chan+","+axis);
      return ERROR;
    }

    // erase the vector and load it with the values provided by the configuration row
    try {
      (*vecmap)[{chan,axis}].clear();
      for (int tok=2; tok<vecsize+2; tok++) {
        (*vecmap)[{chan,axis}].push_back(std::stod(tokens[tok]));
      }
    }
    catch (const std::exception &e) {
      logwrite(function, "ERROR parsing \""+config+"\"");
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
   *             at least 5 coefficients. For example, if x=inputvar and the
   *             five polynomials from this->flexure_polynomials[{chan,axis}]
   *             are a,b,c,d,e then return a + bx + cx^2 + dx^3 + ex^4.
   * @param[in]  which     pair { channel, axis }
   * @param[in]  inputvar  independent input variable for the polynomial fit
   * @param[in]  offset    offset in vector to start reading coefficients
   * @return     double (a + bx + cx^2 + dx^3 + ex^4)
   * @throws     std::out_of_range
   *
   */
  double Compensator::flexure_polynomial_fit(const std::pair<std::string,std::string> &which, double inputvar, size_t offset) {

    std::cerr << "flexure_polynomial_fit size=" << this->flexure_polynomials.at(which).size() << "\n";
    if (offset+5 > this->flexure_polynomials.at(which).size()) {
      throw std::out_of_range("not enough flexure polynomial coefficients");
    }

    // a + bx + cx^2 + dx^3 + ex^4
    //
    return this->flexure_polynomials.at(which)[offset + 0]
         + this->flexure_polynomials.at(which)[offset + 1] * inputvar
         + this->flexure_polynomials.at(which)[offset + 2] * std::pow(inputvar, 2.0)
         + this->flexure_polynomials.at(which)[offset + 3] * std::pow(inputvar, 3.0)
         + this->flexure_polynomials.at(which)[offset + 4] * std::pow(inputvar, 4.0);
  }
  /***** Flexure::Compensator::flexure_polynomial_fit *************************/


  /***** Flexure::Compensator::calculate_shift ********************************/
  /**
   * @brief      calculates the shift(chan,axis) of the spectrum on the detector
   * @details    C + A1 * sin(cass-theta) + A2 * sin(2*(cass-theta)) or
   *             C + A1 * cos(cass-theta) + A2 * cos(2*(cass-theta))
   *             Input coefficients are a function of (chan,axis) so the output
   *             shift will also be a function of (chan,axis).
   * @param[in]  which     pair { channel, axis }
   * @return     double (calculated shift)
   * @throws     std::exception
   *
   */
  double Compensator::calculate_shift(const std::pair<std::string,std::string> &which) {
    const std::string function("Flexure::Compensator::calculate_shift");
    try {
      double c     = flexure_polynomial_fit(which, this->tcs_info.zenith,  0);
      double a1    = flexure_polynomial_fit(which, this->tcs_info.zenith,  5);
      double theta = flexure_polynomial_fit(which, this->tcs_info.zenith, 10);
      double a2    = flexure_polynomial_fit(which, this->tcs_info.zenith, 15);

      auto [ chan, axis ] = which;

      if (this->trigfunction[chan] == TrigFunction::Sine) {
        return c + a1 * std::sin(  (this->tcs_info.equivalent_cass * DEGTORAD - theta))
                 + a2 * std::sin(2*(this->tcs_info.equivalent_cass * DEGTORAD - theta));
      }
      else
      if (this->trigfunction[chan] == TrigFunction::Cosine) {
        return c + a1 * std::cos(  (this->tcs_info.equivalent_cass * DEGTORAD - theta))
                 + a2 * std::cos(2*(this->tcs_info.equivalent_cass * DEGTORAD - theta));
      }
      else {
        logwrite(function, "ERROR undefined trig function for channel "+chan);
        return NAN;
      }
    }
    catch (const std::exception &e) {
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
/***
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
***/
  /***** Flexure::Compensator::compute_flexure_compensation ******************/


  /***** Flexure::Compensator::compensate_shift_to_delta *********************/
  /**
   * @brief      calculates the tip-tilt adjustment needed to compensate for shift
   * @details    Given the spectral shift for a specific channel, this calculates
   *             the adjustment needed to compensate for that shift.
   * @param[in]  channel  string channel name
   * @param[in]  shift    pair { sx, sy } representing shift in X, Y
   * @param[out] delta    pair { dx, dy } representing adjustments to X, Y
   *
   */
  void Compensator::compensate_shift_to_delta(const std::string &channel,
                                              const std::pair<double,double> &shift, std::pair<double,double> delta) {

    delta.first  = this->position_coefficients.at({channel,X})[0] * shift.first  +
                   this->position_coefficients.at({channel,X})[1] * shift.second +
                   this->position_coefficients.at({channel,X})[2];

    delta.second = this->position_coefficients.at({channel,Y})[0] * shift.first  +
                   this->position_coefficients.at({channel,Y})[1] * shift.second +
                   this->position_coefficients.at({channel,Y})[2];
  }
  /***** Flexure::Compensator::compensate_shift_to_delta *********************/


  /***** Flexure::Compensator::calculate_compensation ************************/
  /**
   * @brief      calculates the adjustments needed to compensate for a shift
   * @details    input is output of calculate_shift()
   *             output is correction to apply to flexure actuator position
   * @param[in]  which     pair { channel, axis }
   * @param[out] delta     pair { dx, dy } representing adjustments to X, Y
   *
   */
  void Compensator::calculate_compensation(const std::string &channel, std::pair<double,double> &delta) {

    try {
      // calculate shift of spectrum on detector
      //
      double shift_x = this->calculate_shift({channel, X});
      double shift_y = this->calculate_shift({channel, Y});

      std::pair<double, double> shift = { shift_x, shift_y };

      // calculate tip-tilt adjustment needed to compenstate for shift
      //
      this->compensate_shift_to_delta(channel, shift, delta);
    }
    catch (const std::exception &e) {
      delta = { NAN, NAN };
      throw;
    }
  }
  /***** Flexure::Compensator::calculate_compensation ************************/


  long Compensator::test(const std::pair<std::string,std::string> which, double &shift) {
    try {
      shift = this->calculate_shift( which );
    }
    catch (const std::exception &e) {
      shift = NAN;
      logwrite("Flexure::Compensator::test", "ERROR: "+std::string(e.what()));
    }
    return NO_ERROR;
  }

}
