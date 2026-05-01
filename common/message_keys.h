/**
 * @file    message_keys.h
 * @brief   contains keys for JSON messages
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#pragma once

#include <string>

namespace Daemon {
  inline const std::string ACAMD     = "acamd";
  inline const std::string CALIBD    = "calibd";
  inline const std::string CAMERAD   = "camerad";
  inline const std::string FLEXURED  = "flexured";
  inline const std::string FOCUSD    = "focusd";
  inline const std::string POWERD    = "powerd";
  inline const std::string SEQUENCER = "sequencerd";
  inline const std::string SLICECAMD = "slicecamd";
  inline const std::string SLITD     = "slitd";
  inline const std::string TCSD      = "tcsd";
  inline const std::string THERMALD  = "thermald";
}

namespace Severity {
  inline const std::string NOTICE  = "NOTICE";
  inline const std::string WARNING = "WARNING";
  inline const std::string ERROR   = "ERROR";
}

namespace Topic {
  inline const std::string SNAPSHOT   = "_snapshot";
  inline const std::string TARGETINFO = "targetinfo";
  inline const std::string BROADCAST  = "broadcast";
  inline const std::string TCSD       = "tcsd";
  inline const std::string SLITD      = "slitd";
  inline const std::string CAMERAD    = "camerad";
  inline const std::string ACAMD      = "acamd";
  inline const std::string CALIBD     = "calibd";
  inline const std::string FLEXURED   = "flexured";
  inline const std::string FOCUSD     = "focusd";
  inline const std::string POWERD     = "powerd";
  inline const std::string THERMALD   = "thermald";
  inline const std::string SEQ_DAEMONSTATE = "seq_daemonstate";
  inline const std::string SEQ_SEQSTATE = "seq_seqstate";
  inline const std::string SEQ_THREADSTATE = "seq_threadstate";
  inline const std::string SEQ_WAITSTATE = "seq_waitstate";
  inline const std::string SLICECAMD  = "slicecamd";
}

namespace Key {

  inline const std::string SOURCE = "source";

  namespace Broadcast {
    inline const std::string SEVERITY = "severity";
    inline const std::string MESSAGE  = "message";
  }

  namespace Sequencer {
    inline const std::string SEQSTATE = "seqstate";
  }

  namespace Camerad {
    inline const std::string READY         = "ready";
    inline const std::string SHUTTERTIME   = "shuttime_sec";
    inline const std::string EXPTIME       = "exptime";
    inline const std::string IMNUM         = "imnum";
    inline const std::string IMNAME        = "imname";
    inline const std::string FRAMECOUNT    = "framecount";
    inline const std::string FRAMETRANSFER = "frametransfer";
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

  namespace Slitd {
    inline const std::string SLITPOSA = "slitposa";
    inline const std::string SLITPOSB = "slitposb";
    inline const std::string SLITW    = "slitw";
    inline const std::string SLITO    = "slito";
    inline const std::string ISOPEN   = "isopen";
    inline const std::string ISHOME   = "ishome";
  }

  namespace Tcsd {
    inline const std::string TELRA    = "TELRA";
    inline const std::string TELDEC   = "TELDEC";
    inline const std::string ALT      = "ALT";
    inline const std::string AZ       = "AZ";
    inline const std::string AIRMASS  = "AIRMASS";
    inline const std::string CASANGLE = "CASANGLE";
    inline const std::string MOTION   = "MOTION";
  }

}
