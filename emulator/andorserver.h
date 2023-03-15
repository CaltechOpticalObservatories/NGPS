/**
 * @file    andorserver.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef ANDORSERVER_H
#define ANDORSERVER_H

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

/***** AndorServerEmulator ****************************************************/
/**
 * @namespace AndorServerEmulator
 * @brief     this namespace contains everything for the andor camera server emulator
 *
 */
namespace AndorServerEmulator {

  /***** AndorServerEmulator::Interface ***************************************/
  /**
   * @class  Interface
   * @brief  andor camera server emulator interface class
   *
   * Interfaces the andorserver emulator daemon to the emulated andorserver
   *
   */
  class Interface {
    private:
    public:
      Interface();
      ~Interface();

      long acquire( std::string cmd, std::string &reply );
      long coords( std::string coordstr );

  };
  /***** AndorServerEmulator::Interface ***************************************/

}
/***** AndorServerEmulator ****************************************************/
#endif
