/**
 * @file    calib.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef CALIB_H
#define CALIB_H

#include <atomic>
#include <chrono>
#include <numeric>
#include <functional>
#include <thread>
#include <fenv.h>

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "logentry.h"
#include "network.h"

namespace Calib {

  class Interface {
    private:
    public:
      Interface();
      ~Interface();

      // Class Objects
      //
      Config config;

    };
}
#endif
