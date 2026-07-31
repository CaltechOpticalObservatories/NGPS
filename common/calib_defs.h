/**
 * @file    calib_defs.h
 * @brief   calibration device names bound to their message keys
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * The names are the vocabulary used by the cfg files, by the commands sent to
 * the owning daemon, and as map keys. Some of them hold the same string as
 * their message key, which is not something to rely on, so the Device tables
 * bind the two rather than leaving it implicit.
 *
 * The hardware id is wiring, not vocabulary, so it stays in the owning daemon's
 * config (LAMPMOD_MOD in calibd.cfg.in). It appears here only because a
 * consumer that knows a device by name has to command it by number.
 *
 */
#pragma once

#include <string>
#include <vector>

#include "message_keys.h"

namespace Lamp {

  ///< internal calibration lamps, which are powerd plug names
  namespace Cal {
    inline const std::string THAR = "LAMPTHAR";
    inline const std::string FEAR = "LAMPFEAR";
    inline const std::string BLUC = "LAMPBLUC";
    inline const std::string REDC = "LAMPREDC";
  }

  ///< TCS dome lamps, as accepted by tcsd
  namespace Dome {
    inline const std::string LO    = "LO";
    inline const std::string HI    = "HI";
    inline const std::string ARC   = "ARC";
    inline const std::string ULTRA = "ULTRA";
  }

  ///< lamp modulators, as named in calibd
  namespace Mod {
    inline const std::string FEAR  = "MODFEAR";
    inline const std::string BLETA = "MODBLETA";
    inline const std::string RDETA = "MODRDETA";
    inline const std::string RDCON = "MODRDCON";
    inline const std::string BLCON = "MODBLCON";
    inline const std::string THAR  = "MODTHAR";
  }

}

namespace CalibDefs {

  /**
   * @struct  Device
   * @brief   binds a device name to its message key and hardware id
   */
  struct Device {
    std::string name;  ///< name used by cfg files, commands and map keys
    std::string jkey;  ///< key published by the providing daemon
    int         num;   ///< TCS lamp number or modulator channel, 0 if unused
  };

  ///< dome lamps in TCS lamp-number order, which is the CAL_TARGET token order
  inline const std::vector<Device>& domelamps() {
    static const std::vector<Device> devices = {
      { Lamp::Dome::LO,    Key::Tcsd::LAMP_LO,    1 },
      { Lamp::Dome::HI,    Key::Tcsd::LAMP_HI,    2 },
      { Lamp::Dome::ARC,   Key::Tcsd::LAMP_ARC,   3 },
      { Lamp::Dome::ULTRA, Key::Tcsd::LAMP_ULTRA, 4 }
    };
    return devices;
  }

  ///< calibration lamps in CAL_TARGET token order. powerd resolves the plug
  ///< name to a unit and outlet itself, so there is no hardware id here.
  inline const std::vector<Device>& callamps() {
    static const std::vector<Device> devices = {
      { Lamp::Cal::THAR, Key::Powerd::LAMPTHAR, 0 },
      { Lamp::Cal::FEAR, Key::Powerd::LAMPFEAR, 0 },
      { Lamp::Cal::BLUC, Key::Powerd::LAMPBLUC, 0 },
      { Lamp::Cal::REDC, Key::Powerd::LAMPREDC, 0 }
    };
    return devices;
  }

  ///< modulators in channel order, which is the CAL_TARGET token order. Must
  ///< agree with LAMPMOD_MOD in calibd.cfg.in. Channels 7,8 are placeholders.
  inline const std::vector<Device>& modulators() {
    static const std::vector<Device> devices = {
      { Lamp::Mod::FEAR,  Key::Calibd::MODFEAR,  1 },
      { Lamp::Mod::BLETA, Key::Calibd::MODBLETA, 2 },
      { Lamp::Mod::RDETA, Key::Calibd::MODRDETA, 3 },
      { Lamp::Mod::RDCON, Key::Calibd::MODRDCON, 4 },
      { Lamp::Mod::BLCON, Key::Calibd::MODBLCON, 5 },
      { Lamp::Mod::THAR,  Key::Calibd::MODTHAR,  6 }
    };
    return devices;
  }

  /**
   * @brief      look up a Device by name
   * @param[in]  devices  one of the vectors above
   * @param[in]  name     device name to find
   * @return     pointer to the Device, or nullptr if not defined
   */
  inline const Device* find( const std::vector<Device> &devices,
                             const std::string &name ) {
    for ( const auto &dev : devices ) {
      if ( dev.name == name ) return &dev;
    }
    return nullptr;
  }

}
