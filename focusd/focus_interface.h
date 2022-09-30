#ifndef FOCUS_INTERFACE_H
#define FOCUS_INTERFACE_H

#include <map>

#include "network.h"
#include "logentry.h"
#include "common.h"
#include <sys/stat.h>

namespace Focus {

  const std::string DAEMON_NAME = "focusd";       // when run as a daemon, this is my name

  class Interface {
    private:
    public:

      Interface();
      ~Interface();

      Common::Queue async;

    };

}
#endif
