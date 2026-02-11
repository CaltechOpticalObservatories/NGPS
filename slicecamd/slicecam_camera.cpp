/**
 * @file    slicecam_camera.cpp
 * @brief   this contains the implementation for Slicecam::Camera code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Camera class in the Slicecam namespace,
 * which deals directly with the camera.
 *
 */

#include "slicecam_camera.h"

namespace Slicecam {

  /***** Slicecam::Camera::emulator *******************************************/
  /**
   * @brief      enable/disable Andor emulator
   * @param[in]  args       optional state { ? help true false }
   * @param[out] retstring  return status { true false }
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::emulator( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Camera::emulator";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_EMULATOR;
      retstring.append( " [ <cam> ] [ true | false ]\n" );
      retstring.append( "   Enable Andor emulator.\n" );
      retstring.append( "   If the optional <cam> is omitted then command applies to both cameras.\n" );
      retstring.append( "   If the optional { true false } argument is omitted then the current\n" );
      retstring.append( "   state is returned.\n" );
      return HELP;
    }

    std::vector<std::string> tokens;

    Tokenize( args, tokens, " " );

    if ( tokens.size() == 0 ) {
    }
    else
    if ( tokens.size() == 1 ) {
    }
    else
    if ( tokens.size() == 2 ) {
    }
    else {
    }

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // Set the Andor state
    //
    if ( args == "true" ) {
      for ( const auto &pair : this->andor ) {
        pair.second->andor_emulator( true );
      }
    }
    else
    if ( args == "false" ) {
      for ( const auto &pair : this->andor ) {
        pair.second->andor_emulator( false );
      }
    }
    else
    if ( ! args.empty() ) {
      message.str(""); message << "ERROR unrecognized arg " << args << ": expected \"true\" or \"false\"";
      logwrite( function, message.str() );
      return ERROR;
    }

    // Set the return string
    //
    message.str("");
    for ( const auto &pair : this->andor ) {
      std::string_view which_andor = pair.second->get_andor_object();
      if ( which_andor == "sim" ) message << "true ";
      else
      if ( which_andor == "sdk" ) message << "false ";
      else {
        retstring="unknown ";
      }
    }

    retstring = message.str();

    rtrim( retstring );

    return NO_ERROR;
  }
  /***** Slicecam::Camera::emulator *******************************************/


  /***** Slicecam::Camera::open ***********************************************/
  /**
   * @brief      open connection to Andor and initialize SDK
   * @param[in]  which  optionally specify which camera to open
   * @param[in]  args   optional args to send to camera(s)
   * @return     ERROR | NO_ERROR
   * 
   */
  long Camera::open( std::string which, std::string args ) {
    std::string function = "Slicecam::Camera::open";
    std::stringstream message;
    long error=NO_ERROR;

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      return ERROR;
    }

    // Get a map of camera handles, indexed by serial number.
    // This must be called before open() because open() uses handles.
    //
    if ( this->handlemap.size() == 0 ) {
      error = this->andor.begin()->second->get_handlemap( this->handlemap );
    }

    if (error==ERROR) {
      logwrite( function, "ERROR no camera handles found!" );
      return ERROR;
    }

    // make sure each configured Andor has an associated handle for his s/n
    //
    for ( const auto &pair : this->andor ) {
      auto it = this->handlemap.find(pair.second->camera_info.serial_number);
      if ( it == this->handlemap.end() ) {
        message.str(""); message << "ERROR no camera handle found for s/n " << pair.second->camera_info.serial_number;
        logwrite( function, message.str() );
        return ERROR;
      }
      pair.second->camera_info.handle = this->handlemap[pair.second->camera_info.serial_number];
    }

    long ret;

    // Loop through all defined Andors
    //
    for ( const auto &pair : this->andor ) {

      // get a copy of the Andor::DefaultValues object for
      // the currently indexed andor
      //
      auto cfg = this->default_config[pair.first];

      // If a "which" was specified AND it's not this one, then skip it
      //
      if ( !which.empty() && pair.first != which ) continue;

      // otherwise, open this camera if not already open
      //
      if ( !pair.second->is_open() ) {
        if ( ( ret=pair.second->open( pair.second->camera_info.handle )) != NO_ERROR ) {
          message.str(""); message << "ERROR opening slicecam " << pair.second->camera_info.camera_name
                                   << " S/N " << pair.second->camera_info.serial_number;
          logwrite( function, message.str() );
          error = ret;  // preserve the error state for the return value but try all
          continue;
        }
      }

      // Now set up for single scan readout -- cannot software-trigger acquisition to
      // support continuous readout for multiple cameras in the same process.
      //
      error |= pair.second->set_acquisition_mode( 1 );           // single scan
      error |= pair.second->set_read_mode( 4 );                  // image mode
      error |= pair.second->set_vsspeed( 4.33 );                 // vertical shift speed
      error |= pair.second->set_hsspeed( 1.0 );                  // horizontal shift speed
      error |= pair.second->set_shutter( "open" );               // shutter always open
      error |= pair.second->set_imrot( cfg.rotstr );             // set imrot to configured value
      error |= pair.second->set_imflip( cfg.hflip, cfg.vflip );  // set imflip to configured value
      error |= pair.second->set_binning( cfg.hbin, cfg.vbin );   // set binning to configured value
      error |= pair.second->set_temperature( cfg.setpoint );     // set temp setpoint to configured value
      error |= this->set_gain(pair.first, 1);
      error |= this->set_exptime(pair.first, 1 );

      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR configuring slicecam " << pair.second->camera_info.camera_name
                                 << " S/N " << pair.second->camera_info.serial_number;
        logwrite( function, message.str() );
      }
    }

    return error;
  }
  /***** Slicecam::Camera::open ***********************************************/


  /***** Slicecam::Camera::close **********************************************/
  /**
   * @brief      close connection to Andor
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::close() {
    std::string function = "Slicecam::Camera::close";
    std::stringstream message;
    long error=NO_ERROR;

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      return ERROR;
    }

    // loop through and close all (configured) Andors
    //
    for ( const auto &pair : this->andor ) {
      long ret = pair.second->close();
      if ( ret != NO_ERROR ) {
        message.str(""); message << "ERROR closing slicecam " << pair.second->camera_info.camera_name
                                 << " S/N " << pair.second->camera_info.serial_number;
        logwrite( function, message.str() );
        error = ret;  // preserve the error state for the return value
      }
    }

    this->handlemap.clear();

    return error;
  }
  /***** Slicecam::Camera::close **********************************************/


  /***** Slicecam::Camera::bin ************************************************/
  /**
   * @brief      set camera binning
   * @param[in]  hbin  horizontal binning factor
   * @param[in]  vbin  vertical binning factor
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::bin( const int hbin, const int vbin ) {
    std::string function = "Slicecam::Camera::bin";
    std::stringstream message;
    long error = NO_ERROR;

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      return ERROR;
    }

    // all configured Andors must have been initialized
    //
    for ( const auto &pair : this->andor ) {
      if ( ! pair.second->is_open() ) {
        message.str(""); message << "ERROR no connection to slicecam " << pair.second->camera_info.camera_name
                                 << " S/N " << pair.second->camera_info.serial_number;
        logwrite( function, message.str() );
        error=ERROR;
      }
    }
    if ( error==ERROR ) return ERROR;

    // Set the binning parameters now for each sequentially
    //
    for ( auto &[name, cam] : this->andor ) {
      error |= cam->set_binning( hbin, vbin );
    }

    return error;
  }
  /***** Slicecam::Camera::bin ************************************************/


  /***** Slicecam::Camera::set_fan ********************************************/
  /**
   * @brief      set fan mode
   * @param[in]  which  { L R }
   * @param[in]  mode   { 0 1 2 }
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::set_fan( std::string which, int mode ) {
    std::string function = "Slicecam::Camera::set_fan";
    std::stringstream message;

    // make sure requested camera is in the map
    //
    auto it = this->andor.find( which );
    if ( it == this->andor.end() ) {
      message.str(""); message << "ERROR invalid camera name \"" << which << "\"";
      logwrite( function, message.str() );
      return ERROR;
    }

    // make sure requested camera is open
    //
    if ( ! this->andor[which]->is_open() ) {
      message.str(""); message << "ERROR no connection to slicecam " << which
                               << " S/N " << this->andor[which]->camera_info.serial_number;
      logwrite( function, message.str() );
      return ERROR;
    }

    // set the mode
    //
    return this->andor[which]->set_fan( mode );
  }
  /***** Slicecam::Camera::set_fan ********************************************/


  /***** Slicecam::Camera::imflip *********************************************/
  /**
   * @brief      set or get camera image flip
   * @param[in]  args       optionally contains <hflip> <vflip> (0=false 1=true)
   * @param[out] retstring  return string contains <hflip> <vflip>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::imflip( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Camera::imflip";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_IMFLIP;
      retstring.append( " [ <name> <hflip> <vflip> ]\n" );
      retstring.append( "   Set or get CCD image flip for camera <name> = L | R.\n" );
      retstring.append( "   <hflip> and <vflip> indicate to flip horizontally and\n" );
      retstring.append( "   vertically, respectively. Set these =1 to enable flipping,\n" );
      retstring.append( "   or =0 to disable flipping the indicated axis. When setting\n" );
      retstring.append( "   either, both must be supplied. If both omitted then the\n" );
      retstring.append( "   current flip states are returned.\n" );
      return HELP;
    }

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // tokenize the args to get the camera name and the flip args
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 3 ) {
      logwrite( function, "ERROR expected 3 args L|R <hflip> <vflip>" );
      retstring="invalid_argument";
      return ERROR;
    }

    std::string which;
    int hflip, vflip;

    try {
      which = tokens.at(0);
      hflip = std::stoi(tokens.at(1));
      vflip = std::stoi(tokens.at(2));
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR processing args: " << e.what();
      logwrite( function, message.str() );
      retstring="argument_exception";
      return ERROR;
    }

    // make sure requested camera is in the map
    //
    auto it = this->andor.find( which );
    if ( it == this->andor.end() ) {
      message.str(""); message << "ERROR invalid camera name \"" << which << "\"";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    // make sure the requested camera is open
    //
    if ( ! this->andor[which]->is_open() ) {
      message.str(""); message << "ERROR no connection to slicecam " << which
                               << " S/N " << this->andor[which]->camera_info.serial_number;
      logwrite( function, message.str() );
      retstring="not_open";
      return ERROR;
    }

    // perform the flip
    //
    error = this->andor[which]->set_imflip( hflip, vflip );

    if ( error == NO_ERROR ) {
      hflip = this->andor[which]->camera_info.hflip;
      vflip = this->andor[which]->camera_info.vflip;
    }

    message.str(""); message << hflip << " " << vflip;
    retstring = message.str();
    logwrite( function, retstring );

    return error;
  }
  /***** Slicecam::Camera::imflip *********************************************/


  /***** Slicecam::Camera::imrot **********************************************/
  /**
   * @brief      set camera image rotation
   * @param[in]  args       optionally contains <rotdir> "cw" or "ccw"
   * @param[out] retstring  return string contains <hrot> <vrot>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::imrot( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Camera::imrot";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_IMROT;
      retstring.append( " [ <name> <rotdir> ]\n" );
      retstring.append( "   Set CCD image rotation for camera <name> where <rotdir> is { none cw ccw }\n" );
      retstring.append( "   <name> is L | R\n" );
      retstring.append( "   and \"cw\"   will rotate 90 degrees clockwise,\n" );
      retstring.append( "       \"ccw\"  will rotate 90 degrees counter-clockwise,\n" );
      retstring.append( "       \"none\" will set the rotation to none.\n" );
      retstring.append( "   If used in conjuction with \"" + SLICECAMD_IMFLIP + "\" the rotation will\n" );
      retstring.append( "   occur before the flip regardless of which order the commands are\n" );
      retstring.append( "   sent. 180 degree rotation can be achieved using the \"" + SLICECAMD_IMFLIP + "\"\n" );
      retstring.append( "   command by selecting both horizontal and vertical flipping.\n" );
      return HELP;
    }

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // tokenize the args to get the camera name and the rot arg
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      logwrite( function, "ERROR expected 2 args L|R <rotdir>" );
      retstring="invalid_argument";
      return ERROR;
    }

    std::string which;
    int rotdir;

    // assign the numeric rotdir value from the string argument
    //
    try {
      which = tokens.at(0);
      // convert to lowercase
      std::transform( tokens.at(1).begin(), tokens.at(1).end(), tokens.at(1).begin(), ::tolower );
      if ( tokens.at(1) == "none" ) rotdir = 0;
      else
      if ( tokens.at(1) == "cw" )   rotdir = 1;
      else
      if ( tokens.at(1) == "ccw" )  rotdir = 2;
      else {
        message.str(""); message << "ERROR bad arg " << tokens.at(1) << ": expected { none cw ccw }";
        logwrite( function, message.str() );
        return ERROR;
      }
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR processing args: " << e.what();
      logwrite( function, message.str() );
      retstring="argument_exception";
      return ERROR;
    }

    // make sure requested camera is in the map
    //
    auto it = this->andor.find( which );
    if ( it == this->andor.end() ) {
      message.str(""); message << "ERROR invalid camera name \"" << which << "\"";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    // make sure requested camera is open
    //
    if ( ! this->andor[which]->is_open() ) {
      message.str(""); message << "ERROR no connection to slicecam " << which
                               << " S/N " << this->andor[which]->camera_info.serial_number;
      logwrite( function, message.str() );
      retstring="not_open";
      return ERROR;
    }

    // perform the image rotation
    //
    error = this->andor[which]->set_imrot( rotdir );

    return error;
  }
  /***** Slicecam::Camera::imrot **********************************************/


  /***** Slicecam::Camera::set_exptime ****************************************/
  /**
   * @brief      set exposure time
   * @details    This will stop an acquisition in progress before setting the
   *             exposure time. The actual exposure time is returned in the
   *             reference argument.
   * @param[in]  fval  reference to exposure time
   * @return     ERROR | NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Camera::set_exptime( float &fval ) {
    return set_exptime("", fval);
  }
  /***** Slicecam::Camera::set_exptime ****************************************/
  long Camera::set_exptime( std::string which, float &fval ) {

    long error = NO_ERROR;

    for ( const auto &pair : this->andor ) {
      // If a "which" was specified AND it's not this one, then skip it
      //
      if ( !which.empty() && pair.first != which ) continue;

      // Ensure aquisition has stopped
      //
      error |= pair.second->abort_acquisition();

      // Set the exposure time on the Andor.
      // This will modify val with actual exptime.
      //
      if (error==NO_ERROR) error |= pair.second->set_exptime( fval );
      std::stringstream message;
      message.str(""); message << "[DEBUG] set exptime to " << fval
                               << " for camera " << pair.second->camera_info.camera_name;
      logwrite( "Slicecam::Camera::set_exptime", message.str() );
    }

    return error;
  }
  /***** Slicecam::Camera::set_exptime ****************************************/
  /**
   * @brief      set exposure time
   * @details    This overloaded version takes an rvalue reference to accept a
   *             temporary float used to call the other set_exptime function.
   *             Use this to set exptime with an rvalue instead of an lvalue.
   * @param[in]  fval  rvalue reference to exposure time
   * @return     ERROR | NO_ERROR
   */
  long Camera::set_exptime( float &&fval ) {
    return set_exptime("", fval);
  }
  long Camera::set_exptime( std::string which, float &&fval ) {
    float retval=fval;
    return set_exptime(which, retval);
  }
  /***** Slicecam::Camera::set_exptime ****************************************/


  /***** Slicecam::Camera::set_gain *******************************************/
  /**
   * @brief      set or get the CCD gain
   * @details    The output amplifier is automatically set based on gain.
   *             If gain=1 then set to conventional amp and if gain > 1
   *             then set the EMCCD gain register.
   * @param[in]  args       optionally contains new gain
   * @param[out] retstring  return string contains temp, setpoint, and status
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::set_gain( int &gain ) {
    return set_gain("", gain);
  }
  long Camera::set_gain( std::string which, int &gain ) {
    std::string function = "Slicecam::Camera::set_gain";
    std::stringstream message;
    long error = NO_ERROR;

    // get gain range
    //
    int low=999, high=-1;
    error = this->andor.begin()->second->get_emgain_range( low, high );

    // Loop through all defined Andors
    //
    for ( const auto &pair : this->andor ) {
      // If a "which" was specified AND it's not this one, then skip it
      //
      if ( !which.empty() && pair.first != which ) continue;

      // camera must be open
      //
      if ( ! pair.second->is_open() ) {
        message.str(""); message << "ERROR no connection to slicecam " << pair.second->camera_info.camera_name
                                 << " S/N " << pair.second->camera_info.serial_number;
        logwrite( function, message.str() );
        error=ERROR;
        continue;
      }

      message.str(""); message << "[DEBUG] set gain to " << gain
                               << " for camera " << pair.second->camera_info.camera_name;
      logwrite( function, message.str() );

      if ( error==NO_ERROR && gain == 1 ) {
        error = pair.second->set_output_amplifier( Andor::AMPTYPE_CONV );
        if (error==NO_ERROR) {
          for ( const auto &pair : this->andor ) {
            pair.second->camera_info.gain = 1;
          }
        }
        else { message << "ERROR gain not set"; }
      }
      else
      if ( error==NO_ERROR && gain >= low && gain <= high ) {
        error |= pair.second->set_output_amplifier( Andor::AMPTYPE_EMCCD );
        if (error==NO_ERROR) pair.second->set_emgain( gain );
        if (error==NO_ERROR) pair.second->camera_info.gain = gain;
        else { message << "ERROR gain not set"; }
      }
      else
      if ( error==NO_ERROR ) {
        message.str(""); message << "ERROR: gain " << gain << " outside range { 1, "
                                 << low << ":" << high << " }";
        error = ERROR;
      }
      if ( !message.str().empty() ) logwrite( function, message.str() );

      // The image gets flipped when the EM gain is used.
      // This flips it back.
      //
      if (error==NO_ERROR && pair.first=="L") {
        if (gain>1) {
          error=this->andor["L"]->set_imflip( (default_config["L"].hflip==1?0:1), default_config["L"].vflip );
        }
        else error=this->andor["L"]->set_imflip( default_config["L"].hflip, default_config["L"].vflip );
      }
      if (error==NO_ERROR && pair.first=="R") {
        if (gain>1) {
          error=this->andor["R"]->set_imflip( (default_config["R"].hflip==1?0:1), default_config["R"].vflip );
        }
        else error=this->andor["R"]->set_imflip( default_config["R"].hflip, default_config["R"].vflip );
      }
    }

    // Regardless of setting the gain, always return what's in the camera
    //
    gain = this->andor.begin()->second->camera_info.gain;

    return error;
  }
  long Camera::set_gain( int &&gain ) {
    return set_gain("", gain);
  }
  long Camera::set_gain( std::string which, int &&gain ) {
    return set_gain(which, gain);
  }
  /***** Slicecam::Camera::set_gain *******************************************/


  /***** Slicecam::Camera::speed **********************************************/
  /**
   * @brief      set or get the CCD clocking speeds
   * @param[in]  args       optionally contains new clocking speeds
   * @param[out] retstring  return string contains clocking speeds
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::speed( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Camera::speed";
    std::stringstream message;
    long error = NO_ERROR;
    float hori=-1, vert=-1;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_SPEED;
      retstring.append( " [ <hori> <vert> ]\n" );
      retstring.append( "   Set or get CCD clocking speeds for horizontal <hori> and vertical <vert>\n" );
      retstring.append( "   If speeds are omitted then the current speeds are returned.\n" );
/***
      if ( !this->andor.empty() && this->andor.begin()->second->is_open() ) {
        auto cam = this->andor.begin()->second;  // make a smart pointer to the first andor in the map
        retstring.append( "   Current amp type is " );
        retstring.append( ( cam->camera_info.amptype == Andor::AMPTYPE_EMCCD ? "EMCCD\n" : "conventional\n" ) );
        retstring.append( "   Select <hori> from {" );
        for ( const auto &hspeed : cam->camera_info.hsspeeds[ cam->camera_info.amptype] ) {
          retstring.append( " " );
          retstring.append( std::to_string( hspeed ) );
        }
        retstring.append( " }\n" );
        retstring.append( "   Select <vert> from {" );
        for ( const auto &vspeed : cam->camera_info.vsspeeds ) {
          retstring.append( " " );
          retstring.append( std::to_string( vspeed ) );
        }
        retstring.append( " }\n" );
        retstring.append( "   Units are MHz\n" );
      }
      else {
        retstring.append( "   Open connection to camera to see possible speeds.\n" );
      }
***/
      return HELP;
    }

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // all configured Andors must have been initialized
    //
//  for ( const auto &pair : this->andor ) {
//    if ( ! pair.second->is_open() ) {
    for ( auto &[name, cam] : this->andor ) {
      if ( ! cam->is_open() ) {
        message.str(""); message << "ERROR no connection to slicecam " << cam->camera_info.camera_name
                                 << " S/N " << cam->camera_info.serial_number;
        logwrite( function, message.str() );
        error=ERROR;
      }
    }
    if ( error==ERROR ) return ERROR;

    // Parse args if present
    //
    if ( !args.empty() ) {

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // There must be only two args (the <hori> <vert> speeds)
      //
      if ( tokens.size() != 2 ) {
        logwrite( function, "ERROR expected <hori> <vert> speeds" );
        return ERROR;
      }

      // Parse the gain from the token
      //
      try {
        hori = std::stof( tokens.at(0) );
        vert = std::stof( tokens.at(1) );
      }
      catch ( std::out_of_range &e ) {
        message.str(""); message << "ERROR reading speeds: " << e.what();
        error = ERROR;
      }
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR reading speeds: " << e.what();
        error = ERROR;
      }
      if (error==ERROR) logwrite( function, message.str() );

      for ( const auto &pair : this->andor ) {
        if (error!=ERROR ) error = pair.second->set_hsspeed( hori );
        if (error!=ERROR ) error = pair.second->set_vsspeed( vert );
      }
    }

    message.str("");

    for ( auto &[name, cam] : this->andor ) {
      if ( ( cam->camera_info.hspeed < 0 ) ||
           ( cam->camera_info.vspeed < 0 ) ) {
        message.str(""); message << "ERROR speeds not set for camera " << cam->camera_info.camera_name;
        logwrite( function, message.str() );
        error = ERROR;
      }

      message << cam->camera_info.camera_name << " "
              << cam->camera_info.hspeed << " " << cam->camera_info.vspeed << " ";
    }

    retstring = message.str();
    logwrite( function, retstring );

    return error;
  }
  /***** Slicecam::Camera::speed **********************************************/


  /***** Slicecam::Camera::temperature ****************************************/
  /**
   * @brief      set or get the camera temperature setpoint
   * @param[in]  args       optionally contains new setpoint
   * @param[out] retstring  return string contains <temp> <setpoint> <status>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::temperature( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Camera::temperature";
    std::stringstream message;
    long error = NO_ERROR;
    int temp = 999;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_TEMP;
      retstring.append( " [ <setpoint> ]\n" );
      retstring.append( "  Set or get camera temperature in integer degrees C,\n" );
/***
      if ( !this->andor.empty() && this->andor.begin()->second->is_open() ) {
        auto cam = this->andor.begin()->second;  // make a smart pointer to the first andor in the map
        retstring.append( "  where <setpoint> is in range { " );
        retstring.append( std::to_string( cam->camera_info.temp_min ) );
        retstring.append( "  " );
        retstring.append( std::to_string( cam->camera_info.temp_max ) );
        retstring.append( " }.\n" );
      }
      else {
        retstring.append( "  open connection to camera to see acceptable range.\n" );
      }
***/
      retstring.append( "  If optional <setpoint> is provided then the camera setpoint is changed,\n" );
      retstring.append( "  else the current temperature, setpoint, and status are returned.\n" );
      retstring.append( "  Format of return value is <temp> <setpoint> <status>\n" );
      retstring.append( "  Camera cooling is turned on/off automatically, as needed.\n" );
      return HELP;
    }

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // all configured Andors must have been initialized
    //
    for ( const auto &pair : this->andor ) {
      if ( ! pair.second->is_open() ) {
        message.str(""); message << "ERROR no connection to slicecam " << pair.second->camera_info.camera_name
                                 << " S/N " << pair.second->camera_info.serial_number;
        logwrite( function, message.str() );
        error=ERROR;
      }
    }
    if ( error==ERROR ) return ERROR;

    // Parse args if present
    //
    if ( !args.empty() ) {

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // There can be only one arg (the requested temperature)
      //
      if ( tokens.size() != 1 ) {
        logwrite( function, "ERROR too many arguments" );
        return ERROR;
      }

      // Convert the temperature to int and set the temperature.
      // Cooling will be automatically enabled/disabled as needed.
      //
      try {
        temp = static_cast<int>( std::round( std::stof( tokens.at(0) ) ) );
        for ( const auto &pair : this->andor ) {
          message.str(""); message << "[DEBUG] set temp to " << temp
                                   << " for camera " << pair.second->camera_info.camera_name;
          logwrite( function, message.str() );
          error |= pair.second->set_temperature( temp );
        }
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR setting temperature: " << e.what();
        error = ERROR;
      }
    }
    if (error==ERROR) logwrite( function, message.str() );

    // Regardless of setting the temperature, always read it.
    //
    message.str("");
    for ( const auto &pair : this->andor ) {
      error |= pair.second->get_temperature(temp);
      message << pair.second->camera_info.camera_name << " " << temp << " "
              << pair.second->camera_info.setpoint << " "
              << pair.second->camera_info.temp_status << " ";
    }
    logwrite( function, message.str() );

    retstring = message.str();

    return error;
  }
  /***** Slicecam::Camera::temperature ****************************************/


  /***** Slicecam::Camera::write_frame ****************************************/
  /**
   * @brief      write the Andor image data to FITS file
   * @return     ERROR or NO_ERROR
   *
   */
  long Camera::write_frame( std::string source_file, std::string &outfile, const bool _tcs_online ) {
    std::string function = "Slicecam::Camera::write_frame";
    std::stringstream message;

    long error = NO_ERROR;

    fitsinfo.fits_name = outfile;
//  fitsinfo.datatype = USHORT_IMG;
    fitsinfo.datatype = FLOAT_IMG;

    fits_file.copy_info( fitsinfo );      // copy from fitsinfo to the fits_file class

    error = fits_file.open_file();        // open the fits file for writing

    if ( !source_file.empty() ) {
      if (error==NO_ERROR) error = fits_file.copy_header_from( source_file );
    }
    else {
      if (error==NO_ERROR) error = fits_file.create_header();                  // create basic header
    }

    for ( auto &[name, cam] : this->andor ) {
      cam->camera_info.section_size = cam->camera_info.axes[0] * cam->camera_info.axes[1];
      if ( cam->camera_info.section_size == 0 ) {
        message.str(""); message << "ERROR section size 0 for slicecam " << cam->camera_info.camera_name;
        logwrite( function, message.str() );
        error = ERROR;
        break;
      }
      // cam is passed by reference
      //
      fits_file.write_image( cam );                            // write the image data
    }

    fits_file.close_file();               // close the file

    // This is the one extra call that is outside the normal workflow.
    // If emulator is enabled then the skysim generator will create a simulated
    // image. The image written above by fits_file.write_image() is used as
    // input to skysim because it contains the correct WCS headers, but will
    // ultimately be overwritten by the simulated image.
    //
    // Need only to make one call since it will generate a multi-extension
    // image.
    //
    if ( !this->andor.empty() ) {
      if ( this->andor.begin()->second->is_emulated() && _tcs_online ) {
        this->andor.begin()->second->simulate_frame( fitsinfo.fits_name,
                                                     true,  // multi-extension
                                                     this->simsize );
      }
    }

    outfile = fitsinfo.fits_name;

    return error;
  }
  /***** Slicecam::Camera::write_frame ****************************************/

}
