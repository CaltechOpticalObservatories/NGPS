/** ---------------------------------------------------------------------------
 * @file    tcs_interface.h
 * @brief   tcs interface include
 * @details defines the classes used by the tcs hardware interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef SLIT_INTERFACE_H
#define SLIT_INTERFACE_H

#include "network.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>
#include <map>

namespace TCS {

  /** Interface ***************************************************************/
  /**
   * @class  Interface
   * @brief  interface class for a tcs device
   *
   * This class defines the interface for each tcs controller and
   * contains the functions used to communicate with it.
   *
   */
  class Interface {
    private:
      bool class_initialized;
    public:
      const int BUFSZ = 2048;
      std::string host;
      int port;

      Interface();
      ~Interface();

      long initialize_class();
      long open();
      long close();
      long send_command( std::string cmd, std::string &reply );

      Network::TcpSocket tcs;
  };
  /** Interface ***************************************************************/

}
#endif
