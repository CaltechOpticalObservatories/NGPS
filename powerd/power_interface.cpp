#include "power_interface.h"

namespace Power {

  /**************** Power::Interface::Interface *******************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /**************** Power::Interface::Interface *******************************/


  /**************** Power::Interface::~Interface ******************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Power::Interface::~Interface ******************************/


  /**************** Power::Interface::initialize_class ************************/
  /**
   * @fn         initialize
   * @brief      
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::initialize_class() {
    std::string function = "Power::Interface::initialize_class";
    std::stringstream message;
    logwrite( function, "not yet implemented" );
    return( NO_ERROR );
  }
  /**************** Power::Interface::initialize_class ************************/


  /**************** Power::Interface::open ************************************/
  /**
   * @fn         open
   * @brief      
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    std::string function = "Power::Interface::open";
    std::stringstream message;
    logwrite( function, "not yet implemented" );
    return( NO_ERROR );
  }
  /**************** Power::Interface::open ************************************/


  /**************** Power::Interface::isopen **********************************/
  /**
   * @fn         isopen
   * @brief      
   * @param[in]  none
   * @return     true or false
   *
   */
  bool Interface::isopen() {
    std::string function = "Power::Interface::isopen";
    std::stringstream message;
    logwrite( function, "not yet implemented" );
    return( true );
  }
  /**************** Power::Interface::isopen **********************************/

}
