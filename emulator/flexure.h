/**
 * @file    flexure.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef FLEXURE_H
#define FLEXURE_H

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

namespace Flexure {

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
