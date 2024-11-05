/**
 * @file    astrocam-sim.cpp
 * @brief   this is an instrument-specific class for simulating AstroCam
 * @details This file defines functions specific to simulating the Leach controller.
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "camerad.h"
#include "simulator-arc.h"

namespace AstroCam {

  /***** AstroCam::Interface::connect_controller ******************************/
  /**
   * @brief      opens a connection to the PCI/e device(s)
   * @param[in]  devices_in  optional string containing space-delimited list of devices
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::connect_controller( std::string devices_in, std::string &retstring ) {
    std::string function = "AstroCam::Interface::connect_controller";
    std::stringstream message;

    message.str(""); message << "found " << this->numdev << " simulated ARC device" << (this->numdev != 1 ? "s" : "");
    logwrite(function, message.str());

    // Nothing to do if there are no devices detected.
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR: no simulated devices found -- check ARCSIM_NUMDEV in config file");
      return ERROR;
    }

    // Log PCI devices configured
    //
    if ( this->configured_devnums.empty() ) {
      logwrite( function, "ERROR: no devices configured. Need CONTROLLER keyword in config file." );
      retstring="not_configured";
      return ERROR;
    }

    // If no string is given then use vector of configured devices
    //
    if ( devices_in.empty() ) {
      this->devnums = this->configured_devnums;
    }
    else {
      // Otherwise, tokenize the device list string and build devnums from the tokens
      //
      std::vector<std::string> tokens;
      Tokenize(devices_in, tokens, " ");
      for ( const auto &n : tokens ) {                // For each token in the devices_in string,
        try {
          int dev = std::stoi( n );                   // convert to int
          if ( std::find( this->devnums.begin(), this->devnums.end(), dev ) == this->devnums.end() ) { // If it's not already in the vector,
            this->devnums.push_back( dev );                                                            // then push into devnums vector.
          }
        }
        catch (std::invalid_argument &) {
          message.str(""); message << "ERROR: invalid device number: " << n << ": unable to convert to integer";
          logwrite(function, message.str());
          retstring="invalid_argument";
          return ERROR;
        }
        catch (std::out_of_range &) {
          message.str(""); message << "ERROR: device number " << n << ": out of integer range";
          logwrite(function, message.str());
          retstring="out_of_range";
          return ERROR;
        }
        catch(...) { logwrite(function, "unknown error getting device number"); retstring="exception"; return ERROR; }
      }
    }

    // For each requested dev in devnums, if there is a matching controller in the config file,
    // then get the devname and store it in the controller map.
    //
    for ( const auto &dev : this->devnums ) {
      if ( this->controller.find( dev ) != this->controller.end() ) {
        this->controller[ dev ].devname = "sim"+std::to_string(dev);
      }
    }

    // set the controller connected state true
    //
    for ( const auto &dev : this->devnums ) {
      this->controller[dev].connected = true;
    }

    logwrite( function, "simulated controllers connected" );

    this->camera_info.arcsim = true;  // the ARC device is simulated

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

    // clear the controller connected state
    //
    for ( const auto &dev : this->devnums ) {
      this->controller[dev].connected = false;
    }

    logwrite( function, "simulated controllers disconnected" );

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
      if ( std::find( this->devnums.begin(), this->devnums.end(), fw->first ) != this->devnums.end() ) {
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
          error |= ERROR;
        }
        catch ( std::out_of_range & ) {
          this->camera.log_error( function, "ARCSIM_NUMDEV out of integer range" );
          error |= ERROR;
        }

        this->numdev = num;
        message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->camera.async.enqueue( message.str() );
        applied++;
      }
    }

    error |= this->do_configure_controller();

    return error;
  }
  /***** AstroCam::Interface::configure_controller ****************************/


  /***** AstroCam::Interface::abort *******************************************/
  /**
   * @brief      wrapper for abort
   * @details    does nothing
   * @return     NO_ERROR
   *
   */
  long Interface::abort() {
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::abort *******************************************/


  /***** AstroCam::Interface::bin *********************************************/
  /**
   * @brief      wrapper for do_bin
   * @details    does nothing
   * @param[in]  args       argument string contains <chan> <axis> [ <factor> ]
   * @param[out] retstring  return string
   * @return     NO_ERROR
   *
   * The bindir string can be implementation-specific.
   *
   */
  long Interface::bin( std::string args, std::string &retstring ) {
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::bin *********************************************/


  /***** AstroCam::Interface::geometry ****************************************/
  /**
   * @brief      set/get geometry
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::geometry( std::string args, std::string &retstring ) {
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

    if ( this->camera_info.exposure_time < 0 ) {
      logwrite( function, "ERROR: exposure time is undefined" );
      return ERROR;
    }

    this->camera_info.sim_modet = -1;  // initialize modify exposure time, set only by modexptime command

    std::thread( std::ref( AstroCam::Simulator::dothread_expose ), std::ref(this->camera_info) ).detach();
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
    int exptime_try=0;

    // If an exposure time was passed in then
    // try to convert it (string) to an integer
    //
    if ( ! exptime_in.empty() ) {
      try {
        exptime_try = std::stoi( exptime_in );
      }
      catch ( std::invalid_argument & ) {
        message.str(""); message << "ERROR: EXCEPTION converting exposure time: " << exptime_in << " to integer";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: EXCEPTION exposure time " << exptime_in << " outside integer range";
        logwrite( function, message.str() );
        return( ERROR );
      }

      if ( exptime_try < 0 ) {
        logwrite( function, "ERROR:exposure time must be >= 0" );
        return ERROR;
      }
      else this->camera_info.exposure_time = exptime_try;

    }



    return( NO_ERROR );
  }
  /***** AstroCam::Interface::exptime *****************************************/


  /***** AstroCam::Interface::modify_exptime **********************************/
  /**
   * @brief      modify the exposure time while an exposure is running
   * @param[in]  exptime_in  requested exposure time in msec
   * @param[out] retstring   reference to string contains the exposure time
   * @return     ERROR or NO_ERROR
   *
   * Set exptime_in = -1 to end immediately.
   *
   */
  long Interface::modify_exptime( std::string exptime_in, std::string &retstring ) {
    std::string function = "AstroCam::Interface::modify_exptime";
    std::stringstream message;
    long requested_exptime=0;
    long updated_exptime=0;
    long error = NO_ERROR;

    // A requested exposure time must be specified
    //
    if ( exptime_in.empty() ) {
      logwrite( function, "ERROR: requested exposure time cannot be empty" );
      return( ERROR );
    }

    // Convert the requested exptime from string to long
    //
    try {
      requested_exptime = std::stol( exptime_in );
    }
    catch ( std::invalid_argument & ) {
      message.str(""); message << "ERROR: exception converting exposure time: " << exptime_in << " to long";
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range & ) {
      message.str(""); message << "ERROR: exception exposure time " << exptime_in << " outside long range";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // block changes within the last 2 seconds of exposure
    //
    if ( (error==NO_ERROR) && ( (this->camera_info.exposure_time - this->camera_info.sim_et) < 2000 ) ) {
      message.str(""); message << "ERROR cannot change exposure time with less than 2000 msec exptime remaining";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // check if requested exptime has already elapsed
    //
    if ( (error==NO_ERROR) && (requested_exptime >= 0) && (requested_exptime < this->camera_info.sim_et) ) {
      message.str(""); message << "ERROR elapsed time " << this->camera_info.sim_et 
                               << " already exceeds requested exposure time " << requested_exptime;
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Negative value requested exptime means to stop now (round up to the nearest whole sec plus one)
    //
    if ( (error==NO_ERROR) && (requested_exptime < 0 ) ) {
      updated_exptime = (long)( 1000 * std::ceil( 1.0 + (this->camera_info.sim_et/1000.) ) );
    }

    // otherwise, just use the requested exposure time
    //
    if ( (error==NO_ERROR) && (requested_exptime > 0) ) {
      updated_exptime = requested_exptime;
    }

    // setting this class variable, the dothread_expose() thread will update the exposure time
    //
    this->camera_info.sim_modet = updated_exptime;

  return ERROR;
  }
  /***** AstroCam::Interface::modify_exptime **********************************/


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


  long Interface::_image_size( std::string args, std::string &retstring, const bool save_as_default ) {
    std::string function = "AstroCam::Interface::_image_size";
    std::stringstream message;
    logwrite( function, "NOP" );
    return( NO_ERROR );
  }


  /***** AstroCam::Simulator::dothread_expose *********************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  void Simulator::dothread_expose( Camera::Information &info ) {
    std::string function = "AstroCam::Simulator::dothread_expose";
    std::stringstream message;
    unsigned int imagesize = 4096;

    // send the ELAPSEDTIME message
    //
    for ( info.sim_et = 0; info.sim_et <= info.exposure_time; info.sim_et+=100 ) {
      if ( info.sim_modet >= 0 ) info.exposure_time = info.sim_modet;
      message.str(""); message << "ELAPSEDTIME_" << 0 << ":" << info.sim_et << " EXPTIME:" << info.exposure_time;
      std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
#ifdef LOGLEVEL_DEBUG
      std::cerr << "elapsedtime: " << std::setw(10) << info.sim_et << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
#endif
      usleep(100000);
    }
    message.str(""); message << "ELAPSEDTIME_" << 0 << ":" << info.sim_et << " EXPTIME:" << info.exposure_time;
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();

    // send the PIXELCOUNT message
    //
    for ( unsigned int pc = 0; pc <= 4096; pc+=128 ) {
      std::stringstream message;
      message.str(""); message << "PIXELCOUNT_" << 0 << ":" << pc << " IMAGESIZE: " << imagesize;
      std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
#ifdef LOGLEVEL_DEBUG
      std::cerr << "pixelcount:  " << std::setw(10) << pc << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
#endif
      usleep(100000);
    }

    // send the FRAMECOUNT message
    //
    message.str(""); message << "FRAMECOUNT_" << 0 << ":" << 1 << " rows=" << 1024 << " cols=" << 1024;
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();

    return;
  }
  /***** AstroCam::Simulator::dothread_expose *********************************/

}
