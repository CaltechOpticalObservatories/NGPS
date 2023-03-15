/**
 * @file    acam_pi.h
 * @brief   header file for acam PI emulator
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef ACAM_PI_H
#define ACAM_PI_H

#include <atomic>
#include <mutex>
#include <chrono>
#include <numeric>
#include <functional>
#include <thread>
#include <fenv.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "logentry.h"
#include "network.h"

/***** AcamPIEmulator *********************************************************/
/**
 * @namespace AcamPIEmulator
 * @brief     this namespace contains everything for the acam_pi emulator
 *
 */
namespace AcamPIEmulator {

  /***** AcamPIEmulator::Interface ********************************************/
  /**
   * @class  Interface
   * @brief  
   *
   */
  class Interface {
    private:
    public:
      Interface();
      ~Interface();

      long parse_command( std::string cmd, std::string &retstring );
      long test();

  };
  /***** AcamPIEmulator::Interface ********************************************/

}
/***** AcamPIEmulator *********************************************************/
#endif
