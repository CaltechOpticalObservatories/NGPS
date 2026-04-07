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
  inline const std::string TARGETINFO = "targetinfo";
  inline const std::string TCSD       = "tcsd";
  inline const std::string SLITD      = "slitd";
  inline const std::string CAMERAD    = "camerad";
  inline const std::string ACAMD      = "acamd";
  inline const std::string SEQ_DAEMONSTATE = "seq_daemonstate";
  inline const std::string SEQ_SEQSTATE = "seq_seqstate";
  inline const std::string SEQ_THREADSTATE = "seq_threadstate";
  inline const std::string SEQ_WAITSTATE = "seq_waitstate";
  inline const std::string SLICECAMD  = "slicecamd";
}

namespace Key {

  inline const std::string SOURCE = "source";

  namespace Sequencer {
    inline const std::string SEQSTATE = "seqstate";
  }

  namespace Camerad {
    inline const std::string READY = "ready";
  }

  namespace Acamd {
    inline const std::string TANDOR       = "tandor";
    inline const std::string FILTER       = "filter";
    inline const std::string COVER        = "cover";
    inline const std::string ACQUIRE_MODE = "acquire_mode";
    inline const std::string IS_ACQUIRED  = "is_acquired";
    inline const std::string NACQUIRED    = "nacquired";
    inline const std::string ATTEMPTS     = "attempts";
    inline const std::string SEEING       = "seeing";
    inline const std::string BACKGROUND   = "background";
  }

  namespace Slicecamd {
    inline const std::string FINEACQUIRE_LOCKED  = "fineacquire_locked";
    inline const std::string FINEACQUIRE_RUNNING = "fineacquire_running";
  }
}
