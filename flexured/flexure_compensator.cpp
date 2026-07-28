/**
 * @file    flexure_compensator.cpp
 * @brief   this contains the flexure compensator code
 * @author  David Hale <dhale@astro.caltech.edu> & Matt
 *          Algorithms by Matt Matuszewski
 *
 */

#include "tcs_info.h"
#include "flexure_compensator.h"

namespace Flexure {

  /***** Flexure::Compensator::Compensator ************************************/
  /**
   * @brief      class constructor
   * @param[in]  info  constructed with a reference to the TcsInfo object
   *                   owned by Interface.
   *
   */
  Compensator::Compensator(TcsInfo &info) : tcs_info(info) {
    // position_coefficients and flexure_polynomials are maps
    // indexed by a pair<chan,axis>. This initializes their indices.
    // Values will be loaded by Compensator::load_position_coefficients().
    //
    for (const auto &chan : { "U", "G", "R", "I" }) {
      for (const auto &axis : { X, Y }) {
        position_coefficients[{chan,axis}] = std::vector<double>();
        flexure_polynomials[{chan,axis}] = std::vector<double>();
      }
    }

    // trig function used for each channel's calculate_shift()
    //
    this->trigfunction["U"] = TrigFunction::Cosine;
    this->trigfunction["G"] = TrigFunction::Cosine;
    this->trigfunction["R"] = TrigFunction::Cosine;
    this->trigfunction["I"] = TrigFunction::Cosine;

  }
  /***** Flexure::Compensator::Compensator ************************************/


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
   *
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
   * @details    Uses inputvar for the fitting variable and 5 coefficients from
   *             supplied vector starting with offset. Coefficients are in units
   *             of sine. Requires that vector have at least 5 coefficients.
   *             For example, if x=sin(inputvar) and the five polynomials from
   *             this->flexure_polynomials[{chan,axis}] are a,b,c,d,e then
   *             return a + bx + cx^2 + dx^3 + ex^4.
   * @param[in]  which     pair { channel, axis }
   * @param[in]  inputvar  independent input variable for the polynomial fit
   * @param[in]  offset    offset in vector to start reading coefficients
   * @return     double (a + bx + cx^2 + dx^3 + ex^4)
   * @throws     std::out_of_range
   *
   */
  double Compensator::flexure_polynomial_fit(const std::pair<std::string,std::string> &which,
                                             double inputvar, size_t offset) {

    const auto it = this->flexure_polynomials.find(which);

    if (it == this->flexure_polynomials.end()) {
      throw std::out_of_range("invalid chan,axis '"+which.first+","+which.second+"'");
    }

    const auto &p = it->second;

    if (offset+5 > p.size()) {
      throw std::out_of_range("not enough flexure polynomial coefficients");
    }

    // coefficients are in units of sine, so use sin(inputvar) here
    //
    const double x = std::sin(inputvar);

    // a + bx + cx^2 + dx^3 + ex^4, where x=sin(inputvar)
    //
    return p[offset] + x*( p[offset+1] + x*( p[offset+2] + x*( p[offset+3] + x*p[offset+4] ) ) );
  }
  /***** Flexure::Compensator::flexure_polynomial_fit *************************/


  /***** Flexure::Compensator::calculate_shift ********************************/
  /**
   * @brief      calculates the shift(chan,axis) of the spectrum on the detector
   * @details    C + A1 * sin(cass-theta) + A2 * sin(2*(cass-theta)) or
   *             C + A1 * cos(cass-theta) + A2 * cos(2*(cass-theta))
   *             Input coefficients are a function of (chan,axis) so the output
   *             shift will also be a function of (chan,axis). The zenangle
   *             limits are applied here.
   * @param[in]  which     pair { channel, axis }
   * @return     double (calculated shift)
   * @throws     std::exception
   *
   */
  double Compensator::calculate_shift(const std::pair<std::string,std::string> &which) {
    const std::string function("Flexure::Compensator::calculate_shift");

    if (this->flexure_polynomials.find(which)==this->flexure_polynomials.end()) {
      throw std::out_of_range("invaid chan,axis '"+which.first+","+which.second+"'");
    }

    double zenangle = this->tcs_info.get_zenangle();

    zenangle = (zenangle>ZENMAX ? ZENMAX : zenangle);  // cap zenangle at max value

    if (zenangle < ZENMIN) return 0.;                  // no correction below minimum zenangle

    double zenangle_rad       = zenangle * DEGTORAD;
    double equivalentcass_rad = this->tcs_info.get_equivalentcass() * DEGTORAD;

    try {
      double c     = this->flexure_polynomial_fit(which, zenangle_rad,  0);
      double a1    = this->flexure_polynomial_fit(which, zenangle_rad,  5);
      double theta = this->flexure_polynomial_fit(which, zenangle_rad, 10);
      double a2    = this->flexure_polynomial_fit(which, zenangle_rad, 15);

      auto [ chan, axis ] = which;

      if (this->trigfunction[chan] == TrigFunction::Sine) {
        return c + a1 * std::sin(  (equivalentcass_rad - theta))
                 + a2 * std::sin(2*(equivalentcass_rad - theta));
      }
      else
      if (this->trigfunction[chan] == TrigFunction::Cosine) {
        return c + a1 * std::cos(  (equivalentcass_rad - theta))
                 + a2 * std::cos(2*(equivalentcass_rad - theta));
      }
      else {
        logwrite(function, "ERROR undefined trig function for channel "+chan);
        return NAN;
      }
    }
    catch (const std::exception &e) {
      logwrite(function, "ERROR: "+std::string(e.what()));
      throw;
    }
  }
  /***** Flexure::Compensator::calculate_shift ********************************/


  /***** Flexure::Compensator::compensate_shift_to_delta *********************/
  /**
   * @brief      calculates the tip-tilt adjustment needed to compensate for shift
   * @details    Given the spectral shift for a specific channel, this calculates
   *             the adjustment needed to compensate for that shift in pixel units.
   *             The shift is applied with the opposite sign in order to correct
   *             for it, so the coefficients are evaluated at { -sx, -sy }. Note
   *             that the constant term [2] is not negated.
   * @param[in]  channel  string channel name
   * @param[in]  shift    pair { sx, sy } representing shift in X, Y
   * @param[out] delta    reference to pair { dx, dy } of pix adjustments to X, Y
   *
   */
  void Compensator::compensate_shift_to_delta(const std::string &channel,
                                              const std::pair<double,double> &shift, std::pair<double,double> &delta) {

    const auto &cx = this->position_coefficients.at({channel, X});
    const auto &cy = this->position_coefficients.at({channel, Y});

    // shift is applied in opposite sign in order to compensate for it
    //
    const double x = -shift.first;
    const double y = -shift.second;

    // delta.first is delta-X = [0]*x + [1]*y + [2]
    delta.first  = cx[0]*x + cx[1]*y + cx[2];

    // delta.second is delta-Y = [0]*x + [1]*y + [2]
    delta.second = cy[0]*x + cy[1]*y + cy[2];
  }
  /***** Flexure::Compensator::compensate_shift_to_delta *********************/


  /***** Flexure::Compensator::calculate_compensation ************************/
  /**
   * @brief      calculates the adjustments needed to compensate for a shift
   * @details    input is output of calculate_shift() in pixel units,
   *             output is correction to apply to flexure actuator position
   *             in collimator position units
   * @param[in]  channel  string channel name
   * @param[out] delta    ref to pair { dx, dy } of collimator adjustments to X, Y
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
      logwrite("Flexure::Compensator::calculate_compensation", "ERROR: "+std::string(e.what()));
      delta = { NAN, NAN };
      throw;
    }
  }
  /***** Flexure::Compensator::calculate_compensation ************************/

}
