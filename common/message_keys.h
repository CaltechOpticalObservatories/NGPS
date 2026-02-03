/**
 * @file    message_keys.h
 * @brief   contains keys for JSON messages
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#pragma once

#include <string>

namespace Topic {
  inline const std::string SNAPSHOT   = "_snapshot";
  inline const std::string TCSD       = "tcsd";
  inline const std::string TARGETINFO = "tcsd";
  inline const std::string SLITD      = "slitd";
  inline const std::string CAMERAD    = "camerad";
}

namespace Key {

  inline const std::string SOURCE = "source";

  namespace Camerad {
    inline const std::string READY = "ready";
  }
}
