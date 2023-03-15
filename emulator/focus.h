/**
 * @file    focus.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef FOCUS_H
#define FOCUS_H

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

namespace Focus {

  /** Interface ***************************************************************/
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

  };
  /** Interface ***************************************************************/

}
#endif
