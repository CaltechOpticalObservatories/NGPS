#ifndef FLEXURE_INTERFACE_H
#define FLEXURE_INTERFACE_H

#include <map>

#include "network.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>

namespace Flexure {

  const std::string DAEMON_NAME = "flexured";    /// when run as a daemon, this is my name

  class Interface {
    private:
    public:

      Interface();
      ~Interface();

      Common::Queue async;

    };

}
#endif
