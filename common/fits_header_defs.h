/**
 * @file    fits_header_defs.h
 * @brief   shared FITS header keyword metadata for add_json_key consumers
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#pragma once

#include "message_keys.h"

namespace FitsHeaderKeys {

  /**
   * @struct  Primary
   * @brief   holds metadata for a primary-header keyword extracted from JSON
   */
  struct Primary {
    const char* jkey;     ///< key to extract from JSON message
    const char* keyword;  ///< FITS header keyword (uses jkey if empty)
    const char* comment;  ///< FITS keyword comment
    const char* type;     ///< optional explicit datatype
  };

  /**
   * @struct  Extension
   * @brief   holds metadata for an extension-header keyword extracted from JSON
   */
  struct Extension {
    const char* chan;     ///< extension channel name
    const char* jkey;     ///< key to extract from JSON message
    const char* keyword;  ///< FITS header keyword (uses jkey if empty)
    const char* comment;  ///< FITS keyword comment
    const char* type;     ///< optional explicit datatype
  };

  const Primary CalibInfoKeys[] = {
    { Key::Calibd::MODFEAR.c_str(),  "", "FeAr lamp modulator pow dut per" },
    { Key::Calibd::MODTHAR.c_str(),  "", "ThAr lamp modulator pow dut per" },
    { Key::Calibd::MODBLCON.c_str(), "", "Blue continuum modulator pow dut per" },
    { Key::Calibd::MODBLBYP.c_str(), "", "Blue bypass modulator pow dut per" },
    { Key::Calibd::MODRDCON.c_str(), "", "Red continuum modulator pow dut per" },
    { Key::Calibd::MODRDBYP.c_str(), "", "Red bypass modulator pow dut per" },
    { Key::Calibd::CALCOVER.c_str(), "", "calib cover state" },
    { Key::Calibd::CALDOOR.c_str(),  "", "calib door state" }
  };

  const Extension FlexureInfoKeys[] = {
    { "I", Key::Flexured::FLXSPE_I.c_str(), "FLXSPE", "I flexure spectral axis 2 (X) in um", "" },
    { "I", Key::Flexured::FLXSPA_I.c_str(), "FLXSPA", "I flexure spatial axis 3 (Y) in um", "" },
    { "I", Key::Flexured::FLXPIS_I.c_str(), "FLXPIS", "I flexure piston axis 1 (Z) in um", "" },
    { "R", Key::Flexured::FLXSPE_R.c_str(), "FLXSPE", "R flexure spectral axis 2 (X) in um", "" },
    { "R", Key::Flexured::FLXSPA_R.c_str(), "FLXSPA", "R flexure spatial axis 3 (Y) in um", "" },
    { "R", Key::Flexured::FLXPIS_R.c_str(), "FLXPIS", "R flexure piston axis 1 (Z) in um", "" },
    { "G", Key::Flexured::FLXSPE_G.c_str(), "FLXSPE", "G flexure spectral axis 2 (X) in um", "" },
    { "G", Key::Flexured::FLXSPA_G.c_str(), "FLXSPA", "G flexure spatial axis 3 (Y) in um", "" },
    { "G", Key::Flexured::FLXPIS_G.c_str(), "FLXPIS", "G flexure piston axis 1 (Z) in um", "" },
    { "U", Key::Flexured::FLXSPE_U.c_str(), "FLXSPE", "U flexure spectral axis 2 (X) in um", "" },
    { "U", Key::Flexured::FLXSPA_U.c_str(), "FLXSPA", "U flexure spatial axis 3 (Y) in um", "" },
    { "U", Key::Flexured::FLXPIS_U.c_str(), "FLXPIS", "U flexure piston axis 1 (Z) in um", "" }
  };

  const Extension FocusInfoKeys[] = {
    { "I", Key::Focusd::FOCUSI.c_str(), "FOCUS", "science camera I focus position in mm", "" },
    { "R", Key::Focusd::FOCUSR.c_str(), "FOCUS", "science camera R focus position in mm", "" },
    { "G", Key::Focusd::FOCUSG.c_str(), "FOCUS", "science camera G focus position in mm", "" },
    { "U", Key::Focusd::FOCUSU.c_str(), "FOCUS", "science camera U focus position in mm", "" }
  };

  const Primary PowerInfoKeys[] = {
    { Key::Powerd::LAMPTHAR.c_str(), "", "is ThAr lamp on" },
    { Key::Powerd::LAMPFEAR.c_str(), "", "is FeAr lamp on" },
    { Key::Powerd::LAMPBLUC.c_str(), "", "is blue Xe continuum lamp on" },
    { Key::Powerd::LAMPREDC.c_str(), "", "is red continuum lamp on" },
    { Key::Powerd::LAMPXE.c_str(),   "", "is Xe lamp on" },
    { Key::Powerd::LAMPINCA.c_str(), "", "is Incandescent lamp on" }
  };

  const Primary SlitInfoKeys[] = {
    { Key::Slitd::SLITW.c_str(),    "", "slit width in arcsec" },
    { Key::Slitd::SLITO.c_str(),    "", "slit offset in arcsec" },
    { Key::Slitd::SLITPOSA.c_str(), "", "slit actuator A position in mm" },
    { Key::Slitd::SLITPOSB.c_str(), "", "slit actuator B position in mm" }
  };

  const Primary TargetInfoKeys[] = {
    { "OBS_ID",   "", "Observation ID", "INT" },
    { "NAME",     "", "target name", "STRING" },
    { "SLITA",    "", "slit angle in deg", "FLOAT" },
    { "POINTMDE", "", "pointing mode", "STRING" },
    { "RA",       "", "requested Right Ascension in J2000", "STRING" },
    { "DECL",     "", "requested Declination in J2000", "STRING" }
  };

  const Primary TcsInfoKeys[] = {
    { Key::Tcsd::CASANGLE.c_str(), "", "TCS reported Cassegrain angle in deg", "FLOAT" },
    { Key::Tcsd::HA.c_str(),       "", "hour angle" },
    { Key::Tcsd::RAOFFSET.c_str(), "", "offset Right Ascension" },
    { Key::Tcsd::DECLOFFS.c_str(), "", "offset Declination" },
    { Key::Tcsd::TELRA.c_str(),    "", "TCS reported Right Ascension" },
    { Key::Tcsd::TELDEC.c_str(),   "", "TCS reported Declination" },
    { Key::Tcsd::AZ.c_str(),       "", "TCS reported azimuth" },
    { Key::Tcsd::ZENANGLE.c_str(), "", "TCS reported Zenith angle", "FLOAT" },
    { Key::Tcsd::DOMEAZ.c_str(),   "", "TCS reported dome azimuth", "FLOAT" },
    { Key::Tcsd::DOMESHUT.c_str(), "", "dome shutters" },
    { Key::Tcsd::TELFOCUS.c_str(), "", "TCS reported telescope focus position in mm", "FLOAT" }
  };

  const Extension ThermalInfoKeys[] = {
    { "I", Key::Thermald::TCCD_I.c_str(),  "CCDTEMP", "I CCD temperature in Kelvin", "FLOAT" },
    { "R", Key::Thermald::TCCD_R.c_str(),  "CCDTEMP", "R CCD temperature in Kelvin", "FLOAT" },
    { "G", Key::Thermald::TCCD_G.c_str(),  "CCDTEMP", "G CCD temperature in Kelvin", "FLOAT" },
    { "U", Key::Thermald::TCCD_U.c_str(),  "CCDTEMP", "U CCD temperature in Kelvin", "FLOAT" },

    { "I", Key::Thermald::TCOLL_I.c_str(), "COLTEMP", "I collimator temp in deg C", "FLOAT" },
    { "R", Key::Thermald::TCOLL_R.c_str(), "COLTEMP", "R collimator temp in deg C", "FLOAT" },
    { "G", Key::Thermald::TCOLL_G.c_str(), "COLTEMP", "G collimator temp in deg C", "FLOAT" },

    { "I", Key::Thermald::TFOCUS_I.c_str(), "FOCTEMP", "I focus temp in deg C", "FLOAT" },
    { "R", Key::Thermald::TFOCUS_R.c_str(), "FOCTEMP", "R focus temp in deg C", "FLOAT" },
    { "G", Key::Thermald::TFOCUS_G.c_str(), "FOCTEMP", "G focus temp in deg C", "FLOAT" },
    { "U", Key::Thermald::TFOCUS_U.c_str(), "FOCTEMP", "U focus temp in deg C", "FLOAT" }
  };

  const Primary SlitdTelemKeys[] = {
    { Key::Slitd::SLITO.c_str(), "SLITO", "slit offset in arcsec", "FLOAT" },
    { Key::Slitd::SLITW.c_str(), "SLITW", "slit width in arcsec", "FLOAT" }
  };

}
