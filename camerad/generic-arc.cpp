/**
 * @file    generic-arc.cpp
 * @brief   this class calls functions for the real ARC (Leach) controller
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "camerad.h"

namespace AstroCam {

  /***** AstroCam::Interface::handle_json_message *****************************/
  /**
   * @brief      parses incoming telemetry messages
   * @param[in]  message_in  serialized JSON message string
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::handle_json_message( std::string message_in ) {
    const std::string function="AstroCam::Interface::handle_json_message";
    std::stringstream message;
    std::string messagetype;
    long error;

    struct PrimaryInfo {
      std::string jkey;
      std::string comment;
    };

    struct ExtensionInfo {
      std::string chan;
      std::string jkey;
      std::string keyword;
      std::string comment;
    };

    auto &telemkeys = this->camera_info.telemkeys;

    // selects whether to write to extension or primary
    //
    bool ext = true;
    bool pri = !ext;

    size_t eof_pos = message_in.find(JEOF);
    if ( eof_pos != std::string::npos ) message_in.erase(eof_pos);

    try {
      nlohmann::json jmessage = nlohmann::json::parse( message_in );

      // jmessage must not contain key "error" and must contain key "messagetype"
      //
      if ( !jmessage.contains("error") ) {
        if ( jmessage.contains("messagetype") ) {
          messagetype = jmessage["messagetype"];
          error = NO_ERROR;
        }
        else {
          logwrite( function, "ERROR received JSON message with no messagetype" );
          error = ERROR;
        }
      }
      else {
        logwrite( function, "ERROR in JSON message" );
        error = ERROR;
      }

      // If jmessage contained error or no messagetype then get out now.
      //
      if ( error != NO_ERROR ) return error;

      // telemetry from calibd goes in the primary header
      //
      if ( messagetype == "calibinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"MODFEAR", "FeAr lamp modulator pow dut per"},
          {"MODTHAR", "ThAr lamp modulator pow dut per"},
          {"MODBLCON", "Blue continuum modulator pow dut per"},
          {"MODBLBYP", "Blue bypass modulator pow dut per"},
          {"MODRDCON", "Red continuum modulator pow dut per"},
          {"MODRDBYP", "Red bypass modulator pow dut per"},
          {"CALCOVER", "calib cover state"},
          {"CALDOOR", "calib door state"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, pri);
        }
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received calibinfo" );
        #endif
      }
      else

      // telemetry from flexured goes in the extension header corresponding to the channel
      //
      if ( messagetype == "flexureinfo" ) {
        const ExtensionInfo keyarray[] = {
          {"I", "FLXSPE_I", "FLXSPE", "I flexure spectral axis 2 (X) in um"},
          {"I", "FLXSPA_I", "FLXSPA", "I flexure spatial axis 3 (Y) in um"},
          {"I", "FLXPIS_I", "FLXPIS", "I flexure piston axis 1 (Z) in um"},
          {"R", "FLXSPE_R", "FLXSPE", "R flexure spectral axis 2 (X) in um"},
          {"R", "FLXSPA_R", "FLXSPA", "R flexure spatial axis 3 (Y) in um"},
          {"R", "FLXPIS_R", "FLXPIS", "R flexure piston axis 1 (Z) in um"},
          {"G", "FLXSPE_G", "FLXSPE", "G flexure spectral axis 2 (X) in um"},
          {"G", "FLXSPA_G", "FLXSPA", "G flexure spatial axis 3 (Y) in um"},
          {"G", "FLXPIS_G", "FLXPIS", "G flexure piston axis 1 (Z) in um"},
          {"U", "FLXSPE_U", "FLXSPE", "U flexure spectral axis 2 (X) in um"},
          {"U", "FLXSPA_U", "FLXSPA", "U flexure spatial axis 3 (Y) in um"},
          {"U", "FLXPIS_U", "FLXPIS", "U flexure piston axis 1 (Z) in um"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.keyword, keyinfo.comment, ext, keyinfo.chan);
        }
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received flexureinfo" );
        #endif
      }
      else

      // telemetry from focusd goes in the extension header corresponding to the channel
      //
      if ( messagetype == "focusinfo" ) {
        const ExtensionInfo keyarray[] = {
          {"I", "FOCUSI", "FOCUS", "science camera I focus position in mm" },
          {"R", "FOCUSR", "FOCUS", "science camera R focus position in mm" },
          {"G", "FOCUSG", "FOCUS", "science camera G focus position in mm" },
          {"U", "FOCUSU", "FOCUS", "science camera U focus position in mm" }
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.keyword, keyinfo.comment, ext, keyinfo.chan);
        }
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received focusinfo" );
        #endif
      }
      else

      // telemetry from powerd goes in the primary header
      //
      if ( messagetype == "powerinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"LAMPTHAR", "is ThAr lamp on"},
          {"LAMPFEAR", "is FeAr lamp on"},
          {"LAMPBLUC", "is blue Xe continuum lamp on"},
          {"LAMPREDC", "is red continuum lamp on"},
          {"LAMPXE", "is Xe lamp on"},
          {"LAMPINCA", "is Incandescent lamp on"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, pri);
        }
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received powerinfo" );
        #endif
      }
      else

      // telemetry from calibd goes in the primary header
      //
      if ( messagetype == "slitinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"SLITW", "slit width in arcsec"},
          {"SLITO", "slit offset in arcsec"},
          {"SLITPOSA", "slit actuator A position in mm"},
          {"SLITPOSA", "slit actuator A position in mm"},
          {"SLITPOSB", "slit actuator B position in mm"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, pri);
        }
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received slitinfo" );
        #endif
      }
      else

      // targetinfo telemetry comes from sequencerd and goes in the primary header
      //
      if ( messagetype == "targetinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"OBS_ID", "Observation ID"},
          {"NAME", "target name"},
          {"BINSPECT", "binning in spectral direction"},
          {"BINSPAT", "binning in spatial direction"},
          {"SLITA", "slit angle in deg"},
          {"POINTMDE", "pointing mode"},
          {"RA", "requested Right Ascension in J2000"},
          {"DECL", "requested Declination in J2000"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, pri);
        }
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received targetinfo" );
        #endif
      }
      else

      // telemetry from tcsd goes into primary header
      // AIRMASS is intentionally left out since it is handled differently
      //
      if ( messagetype == "tcsinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"CASANGLE", "TCS reported Cassegrain angle in deg"},
          {"HA", "hour angle"},
          {"RAOFFSET", "offset Right Ascension"},
          {"DECLOFFSET", "offset Declination"},
          {"TELRA", "TCS reported Right Ascension"},
          {"TELDEC", "TCS reported Declination"},
          {"AZ", "TCS reported azimuth"},
          {"ZENANGLE", "TCS reported Zenith angle"},
          {"DOMEAZ", "TCS reported dome azimuth"},
          {"DOMESHUT", "dome shutters"},
          {"TELFOCUS", "TCS reported telescope focus position in mm"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, pri);
        }
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received tcsinfo" );
        #endif
      }
      else

      // telemetry from thermald
      //
      if ( messagetype == "thermalinfo" ) {
        const ExtensionInfo keyarray[] = {
          {"I", "TCCD_I", "CCDTEMP", "I CCD temperature in Kelvin"},
          {"R", "TCCD_R", "CCDTEMP", "R CCD temperature in Kelvin"},
          {"G", "TCCD_G", "CCDTEMP", "G CCD temperature in Kelvin"},
          {"U", "TCCD_U", "CCDTEMP", "U CCD temperature in Kelvin"},

          {"I", "TCOLL_I", "COLTEMP", "I collimator temp in deg C"},
          {"R", "TCOLL_R", "COLTEMP", "R collimator temp in deg C"},
          {"G", "TCOLL_G", "COLTEMP", "G collimator temp in deg C"},

          {"I", "TFOCUS_I", "FOCTEMP", "I focus temp in deg C"},
          {"R", "TFOCUS_R", "FOCTEMP", "R focus temp in deg C"},
          {"G", "TFOCUS_G", "FOCTEMP", "G focus temp in deg C"},
          {"U", "TFOCUS_U", "FOCTEMP", "U focus temp in deg C"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.keyword, keyinfo.comment, ext, keyinfo.chan);
        }
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received thermalinfo" );
        #endif
      }
      else

      // test message
      //
      if ( messagetype == "test" ) {
        message.str(""); message << "received JSON test message: \"" << jmessage["test"].get<std::string>() << "\"";
        logwrite( function, message.str() );
      }
      else {
        message.str(""); message << "ERROR received unhandled JSON message type \"" << messagetype << "\"";
        logwrite( function, message.str() );
        error = ERROR;
      }
    }
    catch ( const nlohmann::json::parse_error &e ) {
      message.str(""); message << "ERROR json exception parsing message: " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR parsing message: " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }

    return error;
  }
  /***** AstroCam::Interface::handle_json_message *****************************/


  long Interface::abort() {
    logwrite( "AstroCam::Interface::abort", "[DEBUG] calling do_abort()" );
    return this->do_abort();
  }


  /***** AstroCam::Interface::bin *********************************************/
  /**
   * @brief      wrapper for do_bin
   * @details    set or get the binning factor for the specified direction
   * @param[in]  args       argument string contains <chan> <axis> [ <factor> ]
   * @param[out] retstring  return string
   * @return     ERROR or NO_ERROR
   *
   * The bindir string can be implementation-specific.
   *
   */
  long Interface::bin( std::string args, std::string &retstring ) {
    return this->do_bin( args, retstring );
  }
  /***** AstroCam::Interface::bin *********************************************/


  /***** AstroCam::Interface::connect_controller ******************************/
  /**
   * @brief      wrapper for do_connect_controller
   * @details    opens a connection to the PCI/e device(s)
   * @param[in]  devices_in  optional string containing space-delimited list of devices
   * @param[out] help        reference to string to return help on request
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::connect_controller(std::string devices_in, std::string &help) {
    return this->do_connect_controller( devices_in, help );
  }
  /***** AstroCam::Interface::connect_controller ******************************/


  /***** AstroCam::Interface::disconnect_controller ***************************/
  /**
   * @brief      wrapper for do_disconnect_controller
   * @details    closes the connection to the PCI/e device(s)
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::disconnect_controller() {
    return this->do_disconnect_controller( );
  }
  /***** AstroCam::Interface::disconnect_controller ***************************/


  /***** AstroCam::Interface::load_firmware ***********************************/
  /**
   * @brief      wrapper for do_load_firmware
   * @details    load firmware (.lod) into one or all controller timing boards
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::load_firmware(std::string &retstring) {
    return this->do_load_firmware( retstring );
  }
  /***** AstroCam::Interface::load_firmware ***********************************/


  /***** AstroCam::Interface::load_firmware ***********************************/
  /**
   * @brief      wrapper for do_load_firmware
   * @details    load firmware (.lod) into one or all controller timing boards
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::load_firmware(std::string timlodfile, std::string &retstring) {
    return this->do_load_firmware( timlodfile, retstring );
  }
  /***** AstroCam::Interface::load_firmware ***********************************/


  /***** AstroCam::Interface::configure_controller ****************************/
  /**
   * @brief      wrapper for do_configure_controller
   * @details    perform initial configuration of controller from .cfg file
   * @return     ERROR or NO_ERROR
   *
   * Called automatically by main() when the server starts up.
   *
   */
  long Interface::configure_controller() {
    return this->do_configure_controller();
  }
  /***** AstroCam::Interface::configure_controller ****************************/


  /***** AstroCam::Interface::expose ****************8*************************/
  /**
   * @brief      wrapper for do_expose
   * @details    initiate an exposure
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::expose( std::string nseq_in ) {
    return this->do_expose( nseq_in );
  }
  /***** AstroCam::Interface::expose ****************8*************************/


  /***** AstroCam::Interface::exptime ***************8*************************/
  /**
   * @brief      wrapper for do_exptime
   * @details    set/get the exposure time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::exptime( std::string exptime_in, std::string &retstring ) {
    return this->do_exptime( exptime_in, retstring );
  }
  /***** AstroCam::Interface::exptime ***************8*************************/


  /***** AstroCam::Interface::modify_exptime ********8*************************/
  /**
   * @brief      wrapper for do_modify_exptime
   * @details    modify the exposure time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::modify_exptime( std::string exptime_in, std::string &retstring ) {
    return this->do_modify_exptime( exptime_in, retstring );
  }
  /***** AstroCam::Interface::modify_exptime ********8*************************/


  /***** AstroCam::Interface::geometry **************8*************************/
  /**
   * @brief      wrapper for do_geometry
   * @details    
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::geometry( std::string args, std::string &retstring ) {
    return this->do_geometry( args, retstring );
  }
  /***** AstroCam::Interface::geometry **************8*************************/


  /***** AstroCam::Interface::readout ***************8*************************/
  /**
   * @brief      wrapper for do_readout
   * @details    set or get type of readout
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::readout( std::string readout_in, std::string &readout_out ) {
    return this->do_readout( readout_in, readout_out );
  }
  /***** AstroCam::Interface::readout ***************8*************************/


  /***** AstroCam::Interface::native ****************8*************************/
  /**
   * @brief      wrapper for do_native
   * @details    send a 3-letter command to the Leach controller
   * @param[in]  cmdstr  command to send
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::native( std::string cmdstr ) {
    return this->do_native( cmdstr );
  }
  /***** AstroCam::Interface::native ****************8*************************/


  /***** AstroCam::Interface::native ****************8*************************/
  /**
   * @brief      wrapper for do_native
   * @details    send a 3-letter command to the Leach controller
   * @param[in]  cmdstr     command to send
   * @param[out] retstring  reference to string to contain reply
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::native( std::string cmdstr, std::string &retstring ) {
    return this->do_native( cmdstr, retstring );
  }
  /***** AstroCam::Interface::native ****************8*************************/

}
