/**
 * @file    calib_defs.h
 * @brief   names of the calibration devices, and their bindings to message keys
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * Three distinct kinds of information are involved in naming a calibration
 * device, and they are easy to conflate:
 *
 *   the device name    the vocabulary used by the cfg files, by the commands
 *                      sent to the owning daemon, and as the map key in every
 *                      daemon that handles the device
 *   the message key    what the providing daemon publishes it under
 *   the hardware id    how the owning daemon addresses the hardware, e.g. the
 *                      TCS lamp number or the modulator's Arduino channel
 *
 * The names live in the Lamp namespaces below. The message keys live in
 * message_keys.h, where their values are arbitrary identifiers. Several of them
 * happen to hold the same string as the device name today, but that is not
 * something to rely on, so the Device tables bind the two explicitly rather than
 * leaving the binding implicit in a constant's value.
 *
 * The hardware id is wiring, not vocabulary, so it stays in the owning daemon's
 * config file -- LAMPMOD_MOD in calibd.cfg.in remains the place that says which
 * Arduino channel a modulator is on. It appears in the tables only because a
 * consumer which knows a device by name sometimes has to command it by number.
 *
 * Everything that names one of these devices derives it from this file:
 * TCS::TcsInfo, Sequencer::CalibrationTarget, and the sequencer's telemetry
 * caches.
 *
 */
#pragma once

#include <string>
#include <vector>

#include "message_keys.h"

/***** Lamp *******************************************************************/
/**
 * @namespace Lamp
 * @brief     names of the calibration lamps and their modulators
 *
 */
namespace Lamp {

  /** @brief  internal calibration lamps. These are powerd plug names. */
  namespace Cal {
    inline const std::string THAR = "LAMPTHAR";
    inline const std::string FEAR = "LAMPFEAR";
    inline const std::string BLUC = "LAMPBLUC";
    inline const std::string REDC = "LAMPREDC";
  }

  /** @brief  TCS dome lamps. These are the names tcsd accepts for TCSD_LAMP. */
  namespace Dome {
    inline const std::string LO    = "LO";
    inline const std::string HI    = "HI";
    inline const std::string ARC   = "ARC";
    inline const std::string ULTRA = "ULTRA";
  }

  /** @brief  lamp modulators. These are the names configured in calibd. */
  namespace Mod {
    inline const std::string FEAR  = "MODFEAR";
    inline const std::string BLETA = "MODBLETA";
    inline const std::string RDETA = "MODRDETA";
    inline const std::string RDCON = "MODRDCON";
    inline const std::string BLCON = "MODBLCON";
    inline const std::string THAR  = "MODTHAR";
  }

}
/***** Lamp *******************************************************************/


/***** CalibDefs **************************************************************/
/**
 * @namespace CalibDefs
 * @brief     binds calibration device names to message keys and hardware ids
 *
 */
namespace CalibDefs {

  /***** CalibDefs::Device ****************************************************/
  /**
   * @struct  Device
   * @brief   binds a device name to the key its provider publishes and to the
   *          hardware identifier used to command it
   *
   */
  struct Device {
    std::string name;  ///< device name used by cfg files, commands and map keys
    std::string jkey;  ///< key published by the providing daemon
    int         num;   ///< hardware id (TCS lamp number, modulator channel), 0 if unused
  };
  /***** CalibDefs::Device ****************************************************/


  /***** CalibDefs::domelamps *************************************************/
  /**
   * @brief      TCS dome lamps
   * @details    Listed in TCS lamp-number order, which is also the order of the
   *             dome lamp tokens in the sequencer's CAL_TARGET config lines.
   * @return     reference to the vector of dome lamp Devices
   *
   */
  inline const std::vector<Device>& domelamps() {
    static const std::vector<Device> devices = {
      { Lamp::Dome::LO,    Key::Tcsd::LAMP_LO,    1 },
      { Lamp::Dome::HI,    Key::Tcsd::LAMP_HI,    2 },
      { Lamp::Dome::ARC,   Key::Tcsd::LAMP_ARC,   3 },
      { Lamp::Dome::ULTRA, Key::Tcsd::LAMP_ULTRA, 4 }
    };
    return devices;
  }
  /***** CalibDefs::domelamps *************************************************/


  /***** CalibDefs::callamps **************************************************/
  /**
   * @brief      internal calibration lamps, which are powerd plugs
   * @details    Listed in the order of the lamp tokens in the sequencer's
   *             CAL_TARGET config lines. There is no hardware id here because
   *             powerd resolves the plug name to a unit and outlet from its own
   *             NPS_PLUG config.
   * @return     reference to the vector of calibration lamp Devices
   *
   */
  inline const std::vector<Device>& callamps() {
    static const std::vector<Device> devices = {
      { Lamp::Cal::THAR, Key::Powerd::LAMPTHAR, 0 },
      { Lamp::Cal::FEAR, Key::Powerd::LAMPFEAR, 0 },
      { Lamp::Cal::BLUC, Key::Powerd::LAMPBLUC, 0 },
      { Lamp::Cal::REDC, Key::Powerd::LAMPREDC, 0 }
    };
    return devices;
  }
  /***** CalibDefs::callamps **************************************************/


  /***** CalibDefs::modulators ************************************************/
  /**
   * @brief      lamp modulators
   * @details    Listed in calibd channel order, which is also the order of the
   *             modulator tokens in the sequencer's CAL_TARGET config lines.
   *             This must agree with the LAMPMOD_MOD lines in calibd.cfg.in,
   *             which is where the channel assignment actually lives. calibd
   *             publishes each modulator's state under its name, so a consumer
   *             joins to that telemetry on the name and needs the channel only
   *             to build a CALIBD_LAMPMOD command.
   *             Channels 7 and 8 are configured as placeholders and are not
   *             listed until they are assigned to real modulators.
   * @return     reference to the vector of modulator Devices
   *
   */
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
  /***** CalibDefs::modulators ************************************************/


  /***** CalibDefs::find ******************************************************/
  /**
   * @brief      look up a Device by name
   * @param[in]  devices  one of the vectors defined above
   * @param[in]  name     device name to find
   * @return     pointer to the Device, or nullptr if name is not defined
   *
   */
  inline const Device* find( const std::vector<Device> &devices,
                             const std::string &name ) {
    for ( const auto &dev : devices ) {
      if ( dev.name == name ) return &dev;
    }
    return nullptr;
  }
  /***** CalibDefs::find ******************************************************/

}
/***** CalibDefs **************************************************************/
