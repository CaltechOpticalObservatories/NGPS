#include "calib_interface.h"

namespace Calib {

  /**************** Calib::Interface::Interface *******************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /**************** Calib::Interface::Interface *******************************/


  /**************** Calib::Interface::~Interface ******************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Calib::Interface::~Interface ******************************/


  /**************** Calib::Interface::initialize_class ************************/
  /**
   * @fn         initialize
   * @brief      
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::initialize_class() {
    std::string function = "Calib::Interface::initialize_class";
    std::stringstream message;
    logwrite( function, "not yet implemented" );
    return( NO_ERROR );
  }
  /**************** Calib::Interface::initialize_class ************************/


  /**************** Calib::Interface::open ************************************/
  /**
   * @fn         open
   * @brief      
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    std::string function = "Calib::Interface::open";
    std::stringstream message;
    logwrite( function, "not yet implemented" );
    return( NO_ERROR );
  }
  /**************** Calib::Interface::open ************************************/


  /**************** Calib::Interface::isopen **********************************/
  /**
   * @fn         isopen
   * @brief      
   * @param[in]  none
   * @return     true or false
   *
   */
  bool Interface::isopen() {
    std::string function = "Calib::Interface::isopen";
    std::stringstream message;
    logwrite( function, "not yet implemented" );
    return( true );
  }
  /**************** Calib::Interface::isopen **********************************/

}
