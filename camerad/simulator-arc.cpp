/**
 * @file    astrocam-sim.cpp
 * @brief   this is an instrument-specific class for simulating AstroCam
 * @details This file defines functions specific to simulating the Leach controller.
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "camerad.h"

namespace AstroCam {

  /***** AstroCam::Interface::connect_controller ******************************/
  /**
   * @brief      opens a connection to the PCI/e device(s)
   * @param[in]  devices_in  optional string containing space-delimited list of devices
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::connect_controller(std::string devices_in="") {
    std::string function = "AstroCam::Interface::connect_controller";
    std::stringstream message;

    message.str(""); message << "found " << this->numdev << " simulated ARC device" << (this->numdev != 1 ? "s" : "");
    logwrite(function, message.str());

    // Nothing to do if there are no devices detected.
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR: no simulated devices found -- check ARCSIM_NUMDEV in config file");
      return(ERROR);
    }

    return NO_ERROR;
  }
  /***** AstroCam::Interface::connect_controller ******************************/


  /***** AstroCam::Interface::disconnect_controller ***************************/
  /**
   * @brief      opens a connection to the PCI/e device(s)
   * @param[in]  devices_in  optional string containing space-delimited list of devices
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::disconnect_controller() {
    std::string function = "AstroCam::Interface::disconnect_controller";
    std::stringstream message;
    logwrite( function, "HEY! I'm the simulator" );
    return NO_ERROR;
  }
  /***** AstroCam::Interface::disconnect_controller ***************************/


  /***** AstroCam::Interface::load_firmware ***********************************/
  /**
   * @brief      load firmware (.lod) into one or all controller timing boards
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::load_firmware(std::string &retstring) {
    std::string function = "AstroCam::Interface::load_firmware";
    
    // If no connected devices then nothing to do here
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR: no connected devices");
      return(ERROR);
    }   

    long error=NO_ERROR;
        
    // Loop through all of the default firmware entries found in the configfile
    // which were stored in an STL map so that
    //   fw->first is the devnumber, and
    //   fw->second is the firmware filename.
    // Use these to build a string of the form "<dev> <filename>" to pass to the
    // do_load_firmware() function to load each controller with the specified file.
    //
    for (auto fw = this->camera.firmware.begin(); fw != this->camera.firmware.end(); ++fw) {
      std::stringstream lodfilestream;
      // But only use it if the device is open
      //
      if ( std::find( this->devlist.begin(), this->devlist.end(), fw->first ) != this->devlist.end() ) {
        lodfilestream << fw->first << " " << fw->second;

        // Call do_load_firmware with the built up string.
        // If it returns an error then set error flag to return to the caller.
        //
        if (this->load_firmware(lodfilestream.str(), retstring) == ERROR) error = ERROR;
      }
    }
    return(error);
  }
  /***** AstroCam::Interface::load_firmware ***********************************/


  /***** AstroCam::Interface::load_firmware ***********************************/
  /**
   * @brief      load firmware (.lod) into one or all controller timing boards
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::load_firmware(std::string timlodfile, std::string &retstring) {
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::load_firmware ***********************************/


  /***** AstroCam::Interface::configure_controller ****************************/
  /**
   * @brief      perform initial configuration of controller from .cfg file
   * @return     ERROR or NO_ERROR
   *
   * Called automatically by main() when the server starts up.
   *
   */
  long Interface::configure_controller() {
    std::string function = "AstroCam::Interface::configure_controller";
    std::stringstream message;
    int applied=0;
    long error = NO_ERROR;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      if (this->config.param[entry].compare(0, 13, "ARCSIM_NUMDEV")==0) {
        int num=0;
        try {
          num = std::stoi( config.arg[entry] );
        }
        catch ( std::invalid_argument & ) {
          this->camera.log_error( function, "unable to convert ARCSIM_NUMDEV to integer" );
          error = ERROR;
        }
        catch ( std::out_of_range & ) {
          this->camera.log_error( function, "ARCSIM_NUMDEV out of integer range" );
          error = ERROR;
        }
        this->numdev = num;
        applied++;
      }
    }
    return error;
  }
  /***** AstroCam::Interface::configure_controller ****************************/


  /***** AstroCam::Interface::geometry ****************************************/
  /**
   * @brief      set/get geometry
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::geometry( std::string args, std::string &retstring ) {
    std::string function = "AstroCam::Interface::geometry";
    std::stringstream message;
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::geometry ****************************************/


  /***** AstroCam::Interface::readout *****************************************/
  /**
   * @brief      set or get type of readout
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::readout( std::string readout_in, std::string &readout_out ) {
    std::string function = "AstroCam::Interface::readout";
    std::stringstream message;
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::readout *****************************************/


  /***** AstroCam::Interface::expose ******************************************/
  /**
   * @brief      initiate an exposure
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::expose( std::string nseq_in ) {
    std::string function = "AstroCam::Interface::expose";
    std::stringstream message;
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::expose ******************************************/


  /***** AstroCam::Interface::exptime *****************************************/
  /**
   * @brief      set/get the exposure time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::exptime( std::string exptime_in, std::string &retstring ) {
    std::string function = "AstroCam::Interface::exptime";
    std::stringstream message;
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::exptime *****************************************/


  /***** AstroCam::Interface::native ******************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::native( std::string cmdstr ) {
    std::string function = "AstroCam::Interface::native";
    std::stringstream message;
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::native ******************************************/


  /***** AstroCam::Interface::native ******************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::native( std::string cmdstr, std::string &retstring ) {
    std::string function = "AstroCam::Interface::native";
    std::stringstream message;
    retstring = "DON";
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::native ******************************************/


}
