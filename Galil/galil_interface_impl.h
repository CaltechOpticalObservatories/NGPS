/**
 * @file    galil_interface_impl.h
 * @brief   this file contains the implementation for the Galil Interface class
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */


namespace Galil {

  /***** Galil::Interface::test ***********************************************/
  /**
   * @brief      test
   * @details    
   *
   */
  template <typename Type>
  void Interface<Type>::test() {
  }
  /***** Galil::Interface::test ***********************************************/


  /***** Galil::Interface::stop ***********************************************/
  /**
   * @brief      stop all motion
   * @param[in]  motor name
   * @return     ERROR|NO_ERROR
   *
   */
  template <typename Type>
  long Interface<Type>::stop(const std::string &name) {
    logwrite("Galil::Interface::stop", "not implemented");
    return ERROR;
  }
  /***** Galil::Interface::stop ***********************************************/


  /***** Galil::Interface::is_home ********************************************/
  /**
   * @brief      checks if homing has been done
   * @param[in]  motor name
   * @return     ERROR|NO_ERROR
   *
   */
  template <typename Type>
  bool Interface<Type>::is_home(const std::string &name) {
    throw std::runtime_error("not implemented");
  }
  /***** Galil::Interface::is_home ********************************************/


  /***** Galil::Interface::moveto *********************************************/
  /**
   * @brief      move to absolute position
   * @param[in]  name       motor name
   * @param[in]  axisnum    axis number
   * @param[in]  posstr     position name
   * @param[out] retstring  reference to return string
   * @return     ERROR|NO_ERROR
   *
   */
  template <typename Type>
  long Interface<Type>::moveto(const std::string &name,
                               int axisnum,
                               const std::string &posstr,
                               std::string &retstring) {
    const std::string function("Galil::Interface::moveto");
    logwrite(function, "ERROR not implemented");
    return ERROR;
  }
  /***** Galil::Interface::moveto *********************************************/


  /***** Galil::Interface::get_pos ********************************************/
  /**
   * @brief      get the current position of a motor
   * @details    This is the outside-callable function for reading a position,
   *             which performs all the safety checks on name, axis, etc.
   * @param[in]  name      controller name
   * @param[in]  axisnum   axis number
   * @param[out] position  reference to position read
   * @param[out] posname   optional reference to position name, if one exists for pos
   * @param[in]  addr      optional address of controller in daisy-chain
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename ControllerType>
  long Interface<ControllerType>::get_pos(const std::string &name, int axisnum, float &position,
                                          std::string* posname, int addr) {
    const std::string function("Galil::Interface::get_pos");
    logwrite(function, "ERROR not implemented");
    return ERROR;
  }
  /***** Galil::Interface::get_pos ********************************************/


  /***** Galil::Interface::move_to_default ************************************/
  /**
   * @brief      move to the default position
   * @details    this is the outside-callable function
   * @return     ERROR | NO_ERROR
   *
   * Move all motor axes to their defaults, if specified.
   *
   */
  template <typename Type>
  long Interface<Type>::move_to_default() {
    logwrite("Galil::Interface::move_to_default", "not yet implemented");
    return ERROR;
  }
  /***** Galil::Interface::move_to_default ************************************/

}
