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
  inline const std::string SEQ_PROGRESS = "seq_progress";
}

namespace Key {

  inline const std::string SOURCE = "source";

  namespace Camerad {
    inline const std::string READY = "ready";
  }

  namespace SeqProgress {
    inline const std::string ONTARGET = "ontarget";
    inline const std::string FINE_TUNE_ACTIVE = "fine_tune_active";
    inline const std::string OFFSET_ACTIVE = "offset_active";
    inline const std::string OFFSET_SETTLE = "offset_settle";
    inline const std::string OBSID = "obsid";
    inline const std::string TARGET_STATE = "target_state";
    inline const std::string EVENT = "event";
  }
}
