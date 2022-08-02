#ifndef POWER_INTERFACE_H
#define POWER_INTERFACE_H

#include <map>

#include "network.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>

namespace Power {

  /** Interface ***************************************************************/
  /**
   * @class  Interface
   * @brief  interface class for the network power switch(es)
   *
   * This class defines the interface for all of the power control hardware and
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

  };
  /** Interface ***************************************************************/

}
#endif
