/**
 * @file    calib_interface.h
 * @brief   contains the Calib::Interface class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef CALIB_INTERFACE_H
#define CALIB_INTERFACE_H

#include <map>

#include "network.h"
#include "pi.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>

/***** Calib ******************************************************************/
/**
 * @namespace Calib
 * @brief     namespace for the calibrator
 *
 */
namespace Calib {


  /***** Calib::Interface *****************************************************/
  /**
   * @class  Interface
   * @brief  interface class for the calib hardware
   *
   * This class defines the interface for all of the calib hardware and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      bool class_initialized;
    public:

      Interface();
      ~Interface();

      long initialize_class();
      bool isopen();
      long open();

      Common::Queue async;                    ///< asynchronous message queue object

      Physik_Instrumente::ServoInterface pi;  ///< object for communicating with the PI

  };
  /***** Calib::Interface *****************************************************/

}
/***** Calib ******************************************************************/
#endif
