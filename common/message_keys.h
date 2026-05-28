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
  inline const std::string ACAMD_TEMP = "acamd_temp";
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

  inline const std::string SOURCE  = "source";
  inline const std::string PUBTIME = "pubtime";

  namespace Broadcast {
    inline const std::string SEVERITY = "severity";
    inline const std::string MESSAGE  = "message";
  }

  namespace Sequencer {
    inline const std::string SEQSTATE           = "seqstate";
    inline const std::string SHOULD_FINEACQUIRE = "should_fineacquire";
  }

  namespace TargetInfo {
    inline const std::string OBS_ID    = "OBS_ID";
    inline const std::string NAME      = "NAME";
    inline const std::string SLITA     = "SLITA";
    inline const std::string BINSPECT  = "BINSPECT";
    inline const std::string BINSPAT   = "BINSPAT";
    inline const std::string POINTMODE = "POINTMODE";
    inline const std::string RA        = "RA";
    inline const std::string DECL      = "DECL";
  }

  namespace Camerad {
    inline const std::string READY         = "ready";
    inline const std::string SHUTTERTIME   = "shuttime_sec";
    inline const std::string EXPTIME       = "exptime";
    inline const std::string IMNUM         = "imnum";
    inline const std::string IMNAME        = "imname";
    inline const std::string FRAMECOUNT    = "framecount";
    inline const std::string FRAMETRANSFER = "frametransfer";
    inline const std::string INREADOUT     = "inreadout";
    inline const std::string EXPOSING      = "exposing";
    inline const std::string PAUSED        = "paused";
    inline const std::string SHUTTEROPEN   = "shutteropen";
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
    inline const std::string HA       = "HA";
    inline const std::string RAOFFSET = "RAOFFSET";
    inline const std::string DECLOFFS = "DECLOFFS";
    inline const std::string ZENANGLE = "ZENANGLE";
    inline const std::string DOMEAZ   = "DOMEAZ";
    inline const std::string DOMESHUT = "DOMESHUT";
    inline const std::string TELFOCUS = "TELFOCUS";
  }

  namespace Calibd {
    inline const std::string MODFEAR  = "MODFEAR";
    inline const std::string MODTHAR  = "MODTHAR";
    inline const std::string MODBLCON = "MODBLCON";
    inline const std::string MODBLBYP = "MODBLBYP";
    inline const std::string MODRDCON = "MODRDCON";
    inline const std::string MODRDBYP = "MODRDBYP";
    inline const std::string CALCOVER = "CALCOVER";
    inline const std::string CALDOOR  = "CALDOOR";
  }

  namespace Flexured {
    inline const std::string FLXSPE_I = "FLXSPE_I";
    inline const std::string FLXSPA_I = "FLXSPA_I";
    inline const std::string FLXPIS_I = "FLXPIS_I";
    inline const std::string FLXSPE_R = "FLXSPE_R";
    inline const std::string FLXSPA_R = "FLXSPA_R";
    inline const std::string FLXPIS_R = "FLXPIS_R";
    inline const std::string FLXSPE_G = "FLXSPE_G";
    inline const std::string FLXSPA_G = "FLXSPA_G";
    inline const std::string FLXPIS_G = "FLXPIS_G";
    inline const std::string FLXSPE_U = "FLXSPE_U";
    inline const std::string FLXSPA_U = "FLXSPA_U";
    inline const std::string FLXPIS_U = "FLXPIS_U";
  }

  namespace Focusd {
    inline const std::string FOCUSI = "FOCUSI";
    inline const std::string FOCUSR = "FOCUSR";
    inline const std::string FOCUSG = "FOCUSG";
    inline const std::string FOCUSU = "FOCUSU";
  }

  namespace Powerd {
    inline const std::string LAMPTHAR = "LAMPTHAR";
    inline const std::string LAMPFEAR = "LAMPFEAR";
    inline const std::string LAMPBLUC = "LAMPBLUC";
    inline const std::string LAMPREDC = "LAMPREDC";
    inline const std::string LAMPXE   = "LAMPXE";
    inline const std::string LAMPINCA = "LAMPINCA";
  }

  namespace Thermald {
    inline const std::string TCCD_I  = "TCCD_I";
    inline const std::string TCCD_R  = "TCCD_R";
    inline const std::string TCCD_G  = "TCCD_G";
    inline const std::string TCCD_U  = "TCCD_U";
    inline const std::string TCOLL_I = "TCOLL_I";
    inline const std::string TCOLL_R = "TCOLL_R";
    inline const std::string TCOLL_G = "TCOLL_G";
    inline const std::string TFOCUS_I = "TFOCUS_I";
    inline const std::string TFOCUS_R = "TFOCUS_R";
    inline const std::string TFOCUS_G = "TFOCUS_G";
    inline const std::string TFOCUS_U = "TFOCUS_U";
  }

}
