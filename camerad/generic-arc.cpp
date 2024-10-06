/**
 * @file    generic-arc.cpp
 * @brief   this class calls functions for the real ARC (Leach) controller
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "camerad.h"

namespace AstroCam {

  /***** AstroCam::Interface::handle_json_message *****************************/
  long Interface::handle_json_message( std::string message_in ) {
    const std::string function="AstroCam::Interface::handle_json_message";
    std::stringstream message;
    std::string messagetype;
    long error;

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

      if ( messagetype == "calibinfo" ) {
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "LAMPTHAR", "is ThAr lamp on" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "LAMPFEAR", "is FeAr lamp on" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "LAMPXE", "is Xe lamp on" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "LAMPINCA", "is Incandescent lamp on" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "MODFEAR", "FeAr lamp modulator pow dut per" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "MODTHAR", "ThAr lamp modulator pow dut per" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "MODBLCON", "Blue continuum modulator pow dut per" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "MODBLBYP", "Blue bypass modulator pow dut per" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "MODRDCON", "Red continuum modulator pow dut per" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "MODRDBYP", "Red bypass modulator pow dut per" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "CALCOVER", "calib cover state" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "CALDOOR", "calib door state" );
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received calibinfo" );
        #endif
      }
      else
      if ( messagetype == "flexureinfo" ) {
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXSPE_I", "I flexure spectral axis 2 (X) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXSPA_I", "I flexure spatial axis 3 (Y) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXPIS_I", "I flexure piston axis 1 (Z) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXSPE_R", "R flexure spectral axis 2 (X) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXSPA_R", "R flexure spatial axis 3 (Y) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXPIS_R", "R flexure piston axis 1 (Z) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXSPE_G", "G flexure spectral axis 2 (X) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXSPA_G", "G flexure spatial axis 3 (Y) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXPIS_G", "G flexure piston axis 1 (Z) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXSPE_U", "U flexure spectral axis 2 (X) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXSPA_U", "U flexure spatial axis 3 (Y) in um" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FLXPIS_U", "U flexure piston axis 1 (Z) in um" );
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received flexureinfo" );
        #endif
      }
      else
      if ( messagetype == "focusinfo" ) {
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FOCUSI", "science camera I focus position in mm" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FOCUSR", "science camera R focus position in mm" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FOCUSG", "science camera G focus position in mm" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FOCUSU", "science camera U focus position in mm" );
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received focusinfo" );
        #endif
      }
      else
      if ( messagetype == "powerinfo" ) {
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "LAMPTHAR", "ThAr lamp 1=on 0=off" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "LAMPFEAR", "FeAr lamp 1=on 0=off" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "LAMPBLUC", "Blue Xe continuum lamp 1=on 0=off" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "LAMPREDC", "Red continuum lamp 1=on 0=off" );
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received powerinfo" );
        #endif
      }
      else
      if ( messagetype == "targetinfo" ) {
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "BINSPECT", "binning in spectral direction" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "BINSPAT", "binning in spatial direction" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "SLITW", "slit width in mm" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "RA", "requested Right Ascension in J2000" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "DEC", "requested Declination in J2000" );
      }
      else
      if ( messagetype == "tcsinfo" ) {
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "CASANGLE", "TCS reported Cassegrain angle in deg" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "HA", "hour angle" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "RAOFFS", "offset Right Ascension" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "DECOFFS", "offset Declination" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "TELRA", "TCS reported Right Ascension" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "TELDEC", "TCS reported Declination" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "AZIMUTH", "TCS reported azimuth" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "ZENANGLE", "TCS reported Zenith angle" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "DOMEAZ", "TCS reported dome azimuth" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "DOMESTAT", "Dome shutters 1=open 0=closed" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "FOCUS", "TCS reported focus position in mm" );
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received tcsinfo" );
        #endif
      }
      else
      if ( messagetype == "thermalinfo" ) {
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "CCDTEMPR", "R CCD temperature in Kelvin" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "CCDTEMPI", "I CCD temperature in Kelvin" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "CCDTEMPU", "U CCD temperature in Kelvin" );
        this->camera_info.telemkeys.add_json_key( &Common::Header::primary, jmessage, "CCDTEMPG", "G CCD temperature in Kelvin" );
        #ifdef LOGLEVEL_DEBUG
        logwrite( function, "[DEBUG] received thermalinfo" );
        #endif
      }
      else
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
   * @details    initiate an exposure
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
