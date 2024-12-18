/**
 * @file    slicecam_interface.cpp
 * @brief   this contains the slicecam interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Slicecam namespace,
 * and is how the slicecam daemon interfaces to the slicecam hardware.
 *
 */

#include "slicecam_interface.h"

namespace Slicecam {

  constexpr double OFFSETRATE=20.;

  int npreserve=0;  ///< counter used for Interface::preserve_framegrab()

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


  /***** Slicecam::Interface::bin *********************************************/
  /**
   * @brief      set or get camera binning
   * @param[in]  args       optionally contains <hbin> <vbin>
   * @param[out] retstring  return string contains <hbin> <vbin>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::bin( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::bin";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_BIN;
      retstring.append( " [ <hbin> <vbin> ]\n" );
      retstring.append( "   Set or get CCD binning.\n" );
      retstring.append( "   <hbin> and <vbin> are the number of pixels to bin horizontally\n" );
      retstring.append( "   and vertically, respectively, and must be a power of 2. When\n" );
      retstring.append( "   setting either, both must be supplied. If both omitted then the\n" );
      retstring.append( "   current binning is returned.\n" );
      retstring.append( "   Binning is applied equally to both cameras.\n" );
      return HELP;
    }

    // Parse args if present
    //
    if ( !args.empty() ) {

      int hbin=-1, vbin=-1;

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // There must be two args <hbin> <vbin>
      //
      if ( tokens.size() != 2 ) {
        logwrite( function, "ERROR expected <hbin> <vbin>" );
        retstring="invalid_argument";
        return ERROR;
      }

      // Parse the binning values from the tokens
      //
      try {
        hbin = std::stoi( tokens.at(0) );
        vbin = std::stoi( tokens.at(1) );
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR reading arguments: " << e.what();
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return ERROR;
      }

      if ( hbin < 1 || vbin < 1 || (hbin & (hbin-1)) || (vbin & (vbin-1)) ) {
        logwrite( function, "ERROR bin factors must be a power of 2" );
        retstring="invalid_argument";
        return ERROR;
      }

      // If framegrab is running then stop it. This won't return until framegrabbing
      // has stopped (or timeout).
      //
      bool was_framegrab_running = this->is_framegrab_running.load(std::memory_order_acquire);
      if ( was_framegrab_running ) {
        std::string dontcare;
        error = this->framegrab( "stop", dontcare );
      }

      // Set the binning parameters now
      //
      if (error!=ERROR ) error = this->camera.bin( hbin, vbin );

      // If framegrab was previously running then restart it
      //
      if ( was_framegrab_running ) {
        std::string dontcare;
        error |= this->framegrab( "start", dontcare );
      }
    }

    // return the current binning parameters
    //
    int hbin=0, vbin=0;
    for ( auto &[name, cam] : this->camera.andor ) {
      hbin = cam->camera_info.hbin;
      vbin = cam->camera_info.vbin;
    }

    message.str(""); message << hbin << " " << vbin;
    retstring = message.str();

    logwrite( function, retstring );

    return error;
  }
  /***** Slicecam::Interface::bin *********************************************/


  void Interface::handletopic_snapshot( const nlohmann::json &jmessage ) {
    // If my name is in the jmessage then publish my snapshot
    //
    if ( jmessage.contains( Slicecam::DAEMON_NAME ) ) {
      this->publish_snapshot();
    }
    else
    if ( jmessage.contains( "test" ) ) {
      logwrite( "Slicecam::Interface::handletopic_snapshot", jmessage.dump() );
    }
  }


  void Interface::handletopic_slitd( const nlohmann::json &jmessage ) {
    {
    std::lock_guard<std::mutex> lock(snapshot_mtx);
    snapshot_status["slitd"]=true;
    }
    Common::extract_telemetry_value( jmessage, "SLITO",  telem.slitoffset );
    Common::extract_telemetry_value( jmessage, "SLITW",  telem.slitwidth );

    this->telemkeys.add_json_key(jmessage, "SLITO", "SLITO", "slit offset in arcsec", false);
    this->telemkeys.add_json_key(jmessage, "SLITW", "SLITW", "slit width in arcsec", false);
  }


  void Interface::handletopic_tcsd( const nlohmann::json &jmessage ) {
    {
    std::lock_guard<std::mutex> lock(snapshot_mtx);
    snapshot_status["tcsd"]=true;
    }
    // extract and store values in the class
    //
    Common::extract_telemetry_value( jmessage, "TCSNAME",    telem.tcsname );
    Common::extract_telemetry_value( jmessage, "ISOPEN",     telem.is_tcs_open );
    Common::extract_telemetry_value( jmessage, "CASANGLE",   telem.angle_scope );
    Common::extract_telemetry_value( jmessage, "TELRA",      telem.ra_scope_hms );
    Common::extract_telemetry_value( jmessage, "TELDEC",     telem.dec_scope_dms );
    Common::extract_telemetry_value( jmessage, "RA",         telem.ra_scope_h );
    Common::extract_telemetry_value( jmessage, "DEC",        telem.dec_scope_d );
    Common::extract_telemetry_value( jmessage, "RAOFFSET",   telem.offsetra );
    Common::extract_telemetry_value( jmessage, "DECLOFFSET", telem.offsetdec );
    Common::extract_telemetry_value( jmessage, "AZ",         telem.az );
    Common::extract_telemetry_value( jmessage, "TELFOCUS",   telem.telfocus );
    Common::extract_telemetry_value( jmessage, "AIRMASS",    telem.airmass );
  }


  /***** Slicecam::Interface::publish_snapshot ********************************/
  /**
   * @brief      publishes snapshot of my telemetry
   * @details    This publishes a JSON message containing a snapshot of my
   *             telemetry.
   *
   */
  void Interface::publish_snapshot() {
    nlohmann::json jmessage_out;
    jmessage_out["source"]   = "slicecamd";

    for ( const auto &[name, cam] : this->camera.andor ) {
      std::string key="TANDOR_SCAM_"+name;
      jmessage_out[key] = static_cast<float>(cam->camera_info.ccdtemp);  // the database wants a float
    }
    try {
      this->publisher->publish( jmessage_out );
    }
    catch ( const std::exception &e ) {
      logwrite( "Slicecam::Interface::publish_snapshot",
                "ERROR publishing message: "+std::string(e.what()) );
      return;
    }
  }
  /***** Slicecam::Interface::publish_snapshot ********************************/


  /***** Slicecam::Interface::request_snapshot ********************************/
  /**
   * @brief      sends request for snapshot
   *
   */
  void Interface::request_snapshot() {
    nlohmann::json jmessage_out;
    {
    std::lock_guard<std::mutex> lock(snapshot_mtx);
    for ( const auto &[topic,status] : snapshot_status ) {
      jmessage_out[topic]=false;
    }
    }

    try {
      this->publisher->publish( jmessage_out, "_snapshot" );
    }
    catch ( const std::exception &e ) {
      logwrite( "Slicecam::Interface::request_snapshot",
                "ERROR publishing message: "+std::string(e.what()) );
      return;
    }
  }
  /***** Slicecam::Interface::request_snapshot ********************************/


  /***** Slicecam::Interface::wait_for_snapshots ******************************/
  /**
   * @brief      wait for everyone to publish their snaphots
   *
   */
  bool Interface::wait_for_snapshots() {
    auto start_time = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds(3);  // set timeout duration

    while (true) {
      bool all_received = true;

      // loop through all elements of snapshot_status,
      // if any of them is false then clear all_received
      {
      std::lock_guard<std::mutex> lock(snapshot_mtx);
      for (const auto &[topic, status] : snapshot_status) {
        if (!status) {
          all_received = false;
          break;
        }
      }
      }

      if (all_received) return true;

      if (std::chrono::steady_clock::now() - start_time > timeout) {
        std::stringstream message;
        message << "ERROR timeout waiting for telemetry from:";
        for ( const auto &[topic,status] : snapshot_status ) {
          if (!status) message << " " << topic;
        }
        logwrite( "Slicecam::Interface::wait_for_snapshots", message.str() );
        return false;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));  // avoid busy-waiting
    }
  }
  /***** Slicecam::Interface::wait_for_snapshots ******************************/


  /***** Slicecam::Interface::configure_interface *****************************/
  /**
   * @brief      configure the interface from the .cfg file
   * @details    this function can be called at any time, e.g. from HUP or a command
   * @param[in]  config  reference to Config object
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::configure_interface( Config &config ) {
    std::string function = "Slicecam::Interface::configure_interface";
    std::stringstream message;
    int applied=0;
    long error = NO_ERROR;

    if ( config.read_config(config) != NO_ERROR) {          // read configuration file specified on command line
      logwrite(function, "ERROR: unable to read config file");
      return ERROR;
    }

    this->camera.andor.clear();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < config.n_entries; entry++) {

      // ANDOR: expected ( <name> [emulate] <sn> <scale> <hflip> <vflip> <rot> <temp> <hbin> <vbin> )

      if ( config.param[entry] == "ANDOR" ) {
        bool emulator_state;  // true if emulated

        // Is "emulate" in the config arg anywhere? If it is then
        // set a flag and remove it from the args.
        //
        auto it = config.arg[entry].find("emulate");
        if ( it != std::string::npos ) {
          emulator_state=true;
          config.arg[entry].erase(it, strlen("emulate"));  // remove from the arg
        }
        else {
          emulator_state=false;
        }

        // now args should have ( <name> <sn> <scale> <hflip> <vflip> <rot> <temp> <hbin> <vbin> )
        std::vector<std::string> tokens;
        Tokenize( config.arg[entry], tokens, " " );

        if ( tokens.size() != 9 ) {
          message.str(""); message << "ERROR invalid args ANDOR=\"" << config.arg[entry]
                                   << "\" : expected <NAME> [emulate] <SN> <SCALE> <HFLIP> <VFLIP> <ROT> <TEMP> <HBIN> <VBIN>";
          logwrite( function, message.str() );
          error |= ERROR;
          continue;
        }

        if ( tokens[0] != "L" && tokens[0] != "R" ) {
          message.str(""); message << "ERROR invalid ANDOR name \"" << tokens[0] << "\" : expected { L R }";
          logwrite( function, message.str() );
          error |= ERROR;
          continue;
        }

        try {
          const std::string idx   = tokens.at(0);
          this->camera.andor[idx] = std::make_unique<Andor::Interface>();  // create an Andor::Interface pointer in the map
          this->camera.andor[idx]->andor_emulator( emulator_state );
          this->camera.andor[idx]->camera_info.camera_name   = idx;        // name of this camera is map index

          this->camera.andor[idx]->camera_info.serial_number = std::stoi(tokens.at(1));

          // set the defaults
          this->camera.default_config[idx].pixel_scale       = std::stod(tokens.at(2));
          this->camera.default_config[idx].hflip             = std::stoi(tokens.at(3));
          this->camera.default_config[idx].vflip             = std::stoi(tokens.at(4));
          this->camera.default_config[idx].rotstr            = tokens.at(5);
          this->camera.default_config[idx].setpoint          = std::stoi(tokens.at(6));
          this->camera.default_config[idx].hbin              = std::stoi(tokens.at(7));
          this->camera.default_config[idx].vbin              = std::stoi(tokens.at(8));

          // copy them into camera_info
          this->camera.andor[idx]->camera_info.pixel_scale   = camera.default_config[idx].pixel_scale;
          this->camera.andor[idx]->camera_info.hflip         = camera.default_config[idx].hflip;
          this->camera.andor[idx]->camera_info.vflip         = camera.default_config[idx].vflip;
          this->camera.andor[idx]->camera_info.rotstr        = camera.default_config[idx].rotstr;
          this->camera.andor[idx]->camera_info.setpoint      = camera.default_config[idx].setpoint;
          this->camera.andor[idx]->camera_info.hbin          = camera.default_config[idx].hbin;
          this->camera.andor[idx]->camera_info.vbin          = camera.default_config[idx].vbin;
        }
        catch( const std::exception &e ) {
          message.str(""); message << "ERROR parsing \"" << config.arg[entry] << "\": " << e.what();
          logwrite( function, message.str() );
          error |= ERROR;
        }

        message.str(""); message << "SLICECAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

      if ( starts_with( config.param[entry], "PUSH_GUI_SETTINGS" ) ) {
        this->gui_manager.set_push_settings( config.arg[entry] );
        message.str(""); message << "SLICECAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

      if ( starts_with( config.param[entry], "PUSH_GUI_IMAGE" ) ) {
        this->gui_manager.set_push_image( config.arg[entry] );
        message.str(""); message << "SLICECAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

      if ( starts_with( config.param[entry], "TCSD_PORT" ) ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "ERROR invalid TCSD_PORT " << config.arg[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        }
        this->tcsd.client.port =  port;
        message.str(""); message << "SLICECAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

      if ( config.param[entry] == "SKYSIM_IMAGE_SIZE" ) {
        try {
          this->camera.set_simsize( std::stoi( config.arg[entry] ) );
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "ERROR invalid SKYSIM_IMAGE_SIZE " << config.arg[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "SLICECAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

    }
    message.str(""); message << "applied " << applied << " configuration lines to the slicecam interface";
    logwrite(function, message.str());

    return error;
  }
  /***** Slicecam::Interface::configure_interface *****************************/


  /***** Slicecam::Interface::open ********************************************/
  /**
   * @brief      wrapper to open all or specified slicecams
   * @param[in]  args       string containing 0 or more args specifying which camera to open
   * @param[out] retstring  return string contains true | false | <help>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::open( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::open";
    std::stringstream message;

    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_OPEN;
      retstring.append( " [ [ <which> [ <args> ]] ]\n" );
      retstring.append( "  Open connections to all slice cameras (default) or the specified camera\n" );
      retstring.append( "  where <which> is { L R }. The optional args are passed to the camera.\n" );
      return HELP;
    }

    std::vector<std::string> tokens;
    std::string which;
    Tokenize( args, tokens, " "  );

    // If there is at least one token,
    //
    if ( tokens.size() > 0 ) {
      // and if that first token is a camera name, then
      // assign that to "which" and remove it from the args string.
      //
      if ( tokens[0] == "L" || tokens[0] == "R" ) {
        which = tokens[0];
        args.erase(args.begin());
      }
    }

    long error = this->tcs_init( "real", retstring );

    std::this_thread::sleep_for(std::chrono::seconds(3));

    if ( this->camera.open( which, args ) == NO_ERROR ) {  // open the camera
      error |= this->framegrab( "start", retstring );      // start frame grabbing if open succeeds
      std::thread( &Slicecam::GUIManager::push_gui_settings, &gui_manager ).detach();  // force display refresh
    }
    else error=ERROR;

    retstring = ( this->isopen( args ) ? "true" : "false" );

    return error;
  }
  /***** Slicecam::Interface::open ********************************************/


  /***** Slicecam::Interface::isopen ******************************************/
  /**
   * @brief      wrapper for slicecam cameras to check if connection open
   * @param[in]  which  optional string contains which camera to check { L R }
   * @param[out] state  reference to bool to indicate open {true} or not {false}
   * @param[out] retstring  reference to return string
   * @return     ERROR | NO_ERROR | HELP
   *
   * This function is overloaded
   *
   */
  long Interface::isopen( std::string which, bool &state, std::string &retstring ) {
    std::string function = "Slicecam::Interface::isopen";
    std::stringstream message;

    // Help
    //
    if ( which == "?" ) {
      retstring = SLICECAMD_ISOPEN;
      retstring.append( " [ <which> ]\n" );
      retstring.append( "  Returns the open state of both, or optionally the specified camera,\n" );
      retstring.append( "  where <which> is { L R }. If no argument is supplied then both are\n" );
      retstring.append( "  checked and both must be open to return true.\n" );
      return HELP;
    }

    // set false, require positive response(s) to be true
    //
    state=false;

    // no args is check both cameras in map
    //
    if ( which.empty() ) {
      for ( const auto &pair : this->camera.andor ) {
        state |= pair.second->is_open();
      }
    }
    else {
      // make sure requested camera is in the map
      //
      auto it = this->camera.andor.find( which );
      if ( it == this->camera.andor.end() ) {
        message.str(""); message << "ERROR Andor camera name \"" << which << "\" not found";
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return ERROR;
      }
      state  = this->camera.andor[which]->is_open();
    }

    retstring = ( state ? "true" : "false" );

    return NO_ERROR;
  }
  /***** Slicecam::Interface::isopen ******************************************/


  /***** Slicecam::Interface::isopen ******************************************/
  /**
   * @brief      returns simple boolean for open state of specified camera
   * @param[in]  which  optional string contains which camera to check { L R }
   * @return     true | false
   *
   * This function is overloaded
   *
   */
  bool Interface::isopen( std::string which ) {
    bool state=false;
    std::string dontcare;
    long error = this->isopen( which, state, dontcare );
    if (error==NO_ERROR) return state; else return false;
  }
  /***** Slicecam::Interface::isopen ******************************************/


  /***** Slicecam::Interface::close *******************************************/
  /**
   * @brief      closes slicecams
   * @param[in]  args       optionally request help
   * @param[out] retstring  contains return string for help
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  void Interface::close() {
    std::string dontcare;
    this->close("",dontcare);
  }
  long Interface::close( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::close";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" ) {
      retstring = SLICECAMD_CLOSE;
      retstring.append( " \n" );
      retstring.append( "  Closes all configured slicecams.\n" );
      return HELP;
    }

    error |= this->framegrab( "stop", retstring );
    error |= this->camera.close();

    return error;
  }
  /***** Slicecam::Interface::close *******************************************/


  /***** Slicecam::Interface::tcs_init ****************************************/
  /**
   * @brief      initialize connection to TCS
   * @param[in]  args       optional string { real sim shutdown }
   * @param[out] retstring  return string contains { real sim offline }
   * @return     ERROR | NO_ERROR | HELP
   *
   * Request for help "?" is passed on to the tcs daemon client
   * via the tcsd.init() call.
   *
   */
  long Interface::tcs_init( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::tcs_init";
    std::stringstream message;
    long error = NO_ERROR;

    // No arg is a query
    //
    if ( args.empty() ) {

      if ( ! this->tcsd.client.is_open() ) {
        retstring="offline";
        return NO_ERROR;
      }

      // Ask tcsd client for the name of the TCS.
      // Response is expected to be "<name> DONE"
      //
      error = this->tcsd.client.command( TCSD_GET_NAME, retstring );

      // Remove the " DONE" from the reply, which becomes the return string
      //
      if ( error == NO_ERROR ) {
        try {
          auto pos = retstring.find( " DONE" );
          if ( pos != std::string::npos ) {
            retstring.erase( pos );
          }
          else {
            message.str(""); message << "ERROR reading TCS name: \"" << retstring << "\"";
            logwrite( function, message.str() );
            return ERROR;
          }
        }
        catch( const std::exception &e ) {
          message.str(""); message << "ERROR invalid reply \"" << retstring << "\" from tcsd: " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
      }

      return error;
    }

    // Send command to tcs daemon client. If help was requested then that
    // request is passed on here to tcsd.init() so this could return HELP.
    //
    error |= this->tcsd.init( args, retstring );   // OR'd to include possible stop-thread-timeout

    if ( error==NO_ERROR && args != "shutdown" ) {
      this->tcs_online.store( true, std::memory_order_release );
    }
    else {
      this->tcs_online.store( false, std::memory_order_release );
    }

    return error;
  }
  /***** Slicecam::Interface::tcs_init ****************************************/


  /***** Slicecam::Interface::saveframes **************************************/
  /**
   * @brief      set/get number of frame grabs to save during target acquisition
   * @param[in]  args       optional number of frames or help
   * @param[out] retstring  return string for help or error status
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::saveframes( std::string args, std::string &retstring ) {
    const std::string function = "Slicecam::Interface::saveframes";

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_SAVEFRAMES;
      retstring.append( " [ <nsave> <nskip> ]\n" );
      retstring.append( "   Set/get the number of frame grabs to save and skip\n" );
      retstring.append( "   This will save <nsave> frames in a row, followed by <nskip>\n" );
      retstring.append( "   frames not saved, and repeat.\n" );
      retstring.append( "   <nsave> = 0 stops saving\n" );
      retstring.append( "   <nskip> = 0 saves every frame (when <nsave> is > 0)\n" );
      return HELP;
    }

    // args other than help, try to convert to integer
    //
    if ( !args.empty() ) {
      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );
      if ( tokens.size() != 2 ) {
        logwrite( function, "ERROR expected <nsave> <nskip>" );
        retstring="invalid_number_args";
        return ERROR;
      }
      try {
        this->nsave_preserve_frames.store( std::stoi(tokens.at(0)) );
        this->nskip_preserve_frames.store( std::stoi(tokens.at(1)) );
      }
      catch ( std::exception & ) {
        logwrite( function, "ERROR parsing value" );
        retstring="invalid_argument";
        return ERROR;
      }
    }

    retstring = std::to_string(this->nsave_preserve_frames.load())+" "
              + std::to_string(this->nskip_preserve_frames.load());

    return NO_ERROR;
  }
  /***** Slicecam::Interface::saveframes **************************************/


  /***** Slicecam::Interface::framegrab_fix ***********************************/
  /**
   * @brief      wrapper to control Andor frame grabbing using last WCSfix filename
   * @param[in]  args       only accepts "?|help" for help
   * @param[out] retstring  return string for help or error status
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::framegrab_fix( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::framegrab_fix";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_FRAMEGRABFIX;
      retstring.append( " \n" );
      retstring.append( "   This is a convenience function to grab a single ACAM image using\n" );
      retstring.append( "   the WCSfix filename from the last solve. If a solve was not run in the\n" );
      retstring.append( "   last 60 seconds then it is considered stale and \"" + SLICECAMD_FRAMEGRABFIX + "\" will return\n" );
      retstring.append( "   an error.\n\n" );
      retstring.append( "   This is equivalent to \"" + SLICECAMD_FRAMEGRAB + " /tmp/slicecam_WCSfix.fis\"\n" );
      return HELP;
    }
    else
    if ( !args.empty() ) {
      logwrite( function, "ERROR expected no argument" );
      retstring="invalid_argument";
      return ERROR;
    }

    if ( wcsfix_time == std::chrono::steady_clock::time_point::min() ) {
      logwrite( function, "ERROR must run solve first" );
      retstring="missing_wcsfix";
      return ERROR;
    }

    if ( !this->wcsname.empty() ) {
      auto time_now = std::chrono::steady_clock::now();
      auto time_since_wcsfix = std::chrono::duration_cast<std::chrono::seconds>( time_now - wcsfix_time ).count();

      if ( time_since_wcsfix > 60 ) {
        message.str(""); message << "ERROR time since last solve is " << time_since_wcsfix << "s: must be within 60s";
        logwrite( function, message.str() );
        retstring="stale_wcsfix";
        return ERROR;
      }
      else return( framegrab( this->wcsname, retstring ) );
    }
    else {
      logwrite( function, "ERROR must run solve first" );
      retstring="missing_wcsfix";
      return ERROR;
    }
  }
  /***** Slicecam::Interface::framegrab_fix ***********************************/


  /***** Slicecam::Interface::framegrab ***************************************/
  /**
   * @brief      wrapper to control Andor frame grabbing
   * @param[in]  args       optional filename as source for header info
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::framegrab( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::framegrab";
    std::stringstream message;
    std::string _imagename = this->imagename;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_FRAMEGRAB;
      retstring.append( " [ start | stop | one [ <filename> ] | saveone <filename> | status ]\n" );
      retstring.append( "   Start/Stop continuous frame grabbing or grab one single ACAM image.\n" );
      retstring.append( "   If an optional <filename> is supplied then that file will be used\n" );
      retstring.append( "   as a source for header information for the frame.\n" );
      retstring.append( "   A <filename> provided with the saveone argument will save the framegrab\n" );
      retstring.append( "   to that filename.\n" );
      retstring.append( "   No argument or \"status\" returns true|false to indicate running state.\n" );
      return HELP;
    }

    // No argument, or "status" will return the framegrab running state
    // then return, no action.
    //
    if ( args.empty() || args == "status" ) {
      retstring = ( this->is_framegrab_running.load(std::memory_order_acquire) ? "true" : "false" );
      return NO_ERROR;
    }

    // Tokenize the args and make sure there's at least one
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    std::string whattodo, sourcefile;

    // First token is what to do (start, stop, one, saveone)
    // and second token, if present, is an optional filename
    //
    if ( tokens.size() > 0 ) whattodo   = tokens[0];
    if ( tokens.size() > 1 ) sourcefile = tokens[1];
    if ( tokens.size() > 2 ) {
      logwrite( function, "ERROR too many arguments" );
      retstring="invalid_argument";
      return ERROR;
    }

    // When stopping framegrabbing, wait for it to stop. Timeout after 3 exptimes or 5s,
    // whichever is greater
    //
    if ( whattodo == "stop" ) {
      this->should_framegrab_run.store( false, std::memory_order_release );  // tells framegrab loop to stop
      if ( this->is_framegrab_running.load(std::memory_order_acquire) ) {    // wait for it to stop
        int waittime = std::max( static_cast<int>(3000*(this->camera.andor.begin()->second->camera_info.exptime+1)), 5000 );
        if ( this->is_framegrab_running.load(std::memory_order_acquire) ) {
          std::unique_lock<std::mutex> lock(framegrab_mtx);
          if ( !cv.wait_for(lock, std::chrono::milliseconds(waittime), [this]() {
                return !this->is_framegrab_running.load(std::memory_order_acquire); }) ) {
            logwrite( function, "ERROR timeout waiting for framegrab loop to stop" );
            return ERROR;
          }
          else { logwrite(function, "framegrab loop has stopped"); return NO_ERROR; }
        }
        else { logwrite(function, "framegrab loop has stopped"); return NO_ERROR; }
      }
    }

    // For "saveone" a provided filename is used for saving the frame,
    // otherwise the default name is used. When no imagename is provided,
    // set_imagename() will restore the default image name.
    //
    if ( whattodo == "saveone" ) {
      this->set_imagename(sourcefile);  // use sourcefile to set the output image name
      sourcefile.clear();               // then clear sourcefile because it's not meant as a WCS source
    }
    else this->set_imagename("");

    // Unless requresting a stop, if the TCS is not already open then
    // initialize the connection // TODO
    //
    if ( whattodo != "stop" && ! this->tcsd.client.is_open() ) {
      this->async.enqueue_and_log( function, "NOTICE: not connected to TCS" );
      retstring="tcs_offline";
//    this->tcs_online.store( true );  // TODO
    }

    // spawn a thread which will perform the continuous acquisition,
    // or not, based on whattodo
    //
    std::thread( &Slicecam::Interface::dothread_framegrab, this, whattodo, sourcefile ).detach();

    return NO_ERROR;
  }
  /***** Slicecam::Interface::framegrab ***************************************/


  /***** Slicecam::Interface::dothread_framegrab ******************************/
  /**
   * @brief      performs continuous acquisition
   * @details    This should be spawned in a thread.
   *             Set should_framegrab_run true if the loop should run continuously.
   *             Set is_framegrab_running true when the loop is running.
   * @param[in]  iface       reference to Slicecam::Interface object
   * @param[in]  whattodo    what to do
   * @param[in]  sourcefile
   *
   */
  void Interface::dothread_framegrab( const std::string whattodo, const std::string sourcefile ) {
    std::string function = "Slicecam::Interface::dothread_framegrab";
    std::stringstream message;
    long error = NO_ERROR;

    // For any whattodo that will take an image, when running the Andor emulator,
    // there must be a TCS (real or emulated) because TCS info is required for
    // the Andor emulator to work.
    //
    if ( (whattodo=="start" || whattodo=="one" || whattodo=="saveone") &&
         (this->camera.andor.begin()->second->is_emulated() && !this->tcs_online.load(std::memory_order_acquire)) ) {
      logwrite( function, "ERROR Andor emulator requires a TCS connection" );
      return;
    }

    if ( whattodo == "one" || whattodo == "saveone" ) {
      // Clear should_framegrab_run which means the framegrab loop should not run.
      // If it's already running then return, the existing framegrab loop will
      // stop. If it's not already running then drop through, and a single
      // frame will be grabbed.
      //
      this->should_framegrab_run.store( false, std::memory_order_release );
      if ( this->is_framegrab_running.load(std::memory_order_acquire) ) return;
    }
    else
    if ( whattodo == "start" ) {
      if ( this->is_framegrab_running.load(std::memory_order_acquire) ) {
        logwrite( function, "thread already running, exiting" );
        return;
      }
      else {
        logwrite( function, "set thread running" );
        this->should_framegrab_run.store( true, std::memory_order_release );
      }
    }
    else return;

    this->wcsname.clear();
    this->wcsfix_time = std::chrono::steady_clock::time_point::min();

    // The speed of this do loop can potentially be limited by either the
    // exposure time or the acquisition, which can be limited by the solver.
    // In other words, it will acquire images as fast as it needs to, but no
    // faster.
    //
    // If I can get the lock then BoolState sets is_framegrab_running true,
    // and clears it (false) automatically when it goes out of scope.
    //
    {
    BoolState loop_running( this->is_framegrab_running );    // sets state true here, clears when it goes out of scope

    int _nsave = this->nsave_preserve_frames.load();  // number of frames to save in a row
    int _nskip = this->nskip_preserve_frames.load();  // number of frames to skip before saving _nsave frames

    do {
      // they will both have the same exptime so just get the first one
      //
      double exptime=0;
      if ( !this->camera.andor.empty() ) {
        exptime = this->camera.andor.begin()->second->camera_info.exptime;
      }
      if ( exptime == 0 ) continue;                                       // wait for non-zero exposure time

      // Trigger and wait for acquisition on each camera sequentially.
      // Unfortunately the SDK can only do this one at a time within the same process.
      //
      for ( auto &[name, cam] : this->camera.andor ) {
        cam->acquire_one();
      }

      if (error==NO_ERROR) error = this->fpoffsets.get_slicecam_params(); // get slicecam params for both cameras before building header

      // request external telemetry once for both cameras, results in struct telem.
      //
      this->request_snapshot();
      this->wait_for_snapshots();

      for ( auto &[name, cam] : this->camera.andor ) {
        collect_header_info( cam );
      }

      if (error==NO_ERROR) error = this->camera.write_frame( sourcefile,
                                                             this->imagename,
                                                             this->tcs_online.load(std::memory_order_acquire) );    // write to FITS file

      this->framegrab_time = std::chrono::steady_clock::time_point::min();

      this->gui_manager.push_gui_image( this->imagename );                                 // send frame to GUI

      // Normally, framegrabs are overwritten to the same file.
      // This optionally saves them at the requested cadence by
      // copying to a different filename.
      //
      // Save a number of frames in a row
      //
      if ( _nsave-- > 0 ) {
        this->preserve_framegrab();
      }
      // skip the next _nskip number of frames before saving _nsave again
      if ( _nsave<1 && _nskip--<1 ) {
        _nsave = this->nsave_preserve_frames.load();
        _nskip = this->nskip_preserve_frames.load();
      }

    } while ( this->should_framegrab_run.load(std::memory_order_acquire) );
    }

    this->cv.notify_all();  // send notification that the loop has stopped

    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR starting thread" );
      this->should_framegrab_run.store( false, std::memory_order_release );  // disable frame grabbing
    }
    else logwrite( function, "leaving thread" );

    return;
  }
  /***** Slicecam::Interface::dothread_framegrab ******************************/


  /***** Slicecam::Interface::preserve_framegrab ******************************/
  /**
   * @brief      preserve frame grab by making a copy
   *
   */
  void Interface::preserve_framegrab() {
    try {
      size_t loc;
      // filename is after the last "/"
      if ( (loc=this->imagename.find_last_of("/")) == std::string::npos ) {
        logwrite( "Slicecam::Interface::preserve_framegrab", "malformed imagename" );
        return;
      }
      std::string filename=this->imagename.substr(loc+1);

      // insert number before the extension
      if ( (loc=filename.find(".fits"))==std::string::npos ) {
        logwrite( "Slicecam::Interface::preserve_framegrab", "malformed imagename" );
        return;
      }

      // build the filename
      std::string basename = filename.substr(0, loc);
      std::string path = "/data/slicecam/" + get_system_date();

      // make sure the path exists and is writable
      if ( !validate_path( path ) ) {
        logwrite( "Slicecam::Interface::preserve_framegrab", "cannot write to "+path );
        return;
      }

      std::stringstream fn;
      fn << path << "/" << basename << "_" << std::setfill('0') << std::setw(5) << npreserve << ".fits";

      // increment until a unique file is found so that it never overwrites
      struct stat st;
      while ( (stat(fn.str().c_str(), &st) == 0) && npreserve < 100000 ) {
        fn.str("");
        fn << path << "/" << basename << "_" << std::setfill('0') << std::setw(5) << ++npreserve << ".fits";
      }

      // copy this framegrab to the file
      std::filesystem::copy( this->imagename, fn.str() );
    }
    catch ( const std::exception &e ) {
      logwrite( "Slicecam::Interface::preserve_framegrab", "ERROR saving frame: "+std::string(e.what()) );
      return;
    }
    return;
  }
  /***** Slicecam::Interface::preserve_framegrab ******************************/


  /***** Slicecam::Interface::gui_settings_control ****************************/
  /**
   * @brief      set or get the GUI settings for the SAOImage GUI display
   * @details    The GUI uses SAOImage for display and control. When ds9 is
   *             used to change any one (or more) parameter(s), it will call
   *             this function to set the parameter(s). This function will
   *             return the current parameters and it will call a shell
   *             command which pushes the parameters to the ds9 display. If
   *             no args are supplied then the parameters will only be returned
   *             and pushed (not set).
   * @param[in]  args       <empty> or contains all: <exptime> <gain> <bin> [ <navg> <reset> ]
   * @param[out] retstring  return string contains <exptime> <gain> <bin> <navg>
   * @return     ERROR | NO_ERROR | HELP
   *
   * This function is overloaded
   *
   */
  long Interface::gui_settings_control( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::gui_settings_control";
    std::stringstream message;
    auto info = this->camera.andor.begin()->second->camera_info;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_GUISET;
      retstring.append( " [ <exptime> <gain> <bin> [ <navg> <reset> ] ]\n" );
      retstring.append( "   Set or get settings for SAOImage GUI display.\n" );
      retstring.append( "   When setting, provide the first 3, <exptime> <gain> <bin>\n" );
      retstring.append( "   or all 5 arguments.\n" );
      retstring.append( "   Binning must be a power of two and will be square.\n" );
      retstring.append( "   <navg> is a value for performing a weighted average, any value > 0.\n" );
      retstring.append( "   Use <reset> = 1 to reset the average, else = 0\n" );
      retstring.append( "   When all arguments are supplied they will be set and then pushed\n" );
      retstring.append( "   back to the display. If no arguments are supplied then all four of\n" );
      retstring.append( "   the current settings are returned and pushed to the display.\n" );
      retstring.append( "   The DONE|ERROR suffix on the return string is suppressed.\n" );
      return HELP;
    }

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    // If something was supplied but not the correct number of args then that's an error
    //
    if ( !tokens.empty() && (tokens.size() != 3 && tokens.size() != 5) ) {
      message.str(""); message << "ERROR received " << tokens.size() << " arguments "
                               << "but expected <exptime> <gain> <bin> [ <navg> <reset> ]";
      logwrite( function, message.str() );
      retstring="invalid_argument_list";
      return ERROR;
    }

    // Get the original exptime and gain from the Andor Information class.
    // These are used to determine if the requested values are different.
    // If they are the same then the GUI display won't be updated.
    //
    float exptime_og = info.exptime;
    int gain_og = info.gain;
    int bin_og = (info.hbin==info.vbin ? info.hbin : -1 );
    float navg_og = this->camera.andor.begin()->second->get_weight();

    long error = NO_ERROR;
    bool set = false;

    // If args were supplied then use them to minimally set
    // exposure time, gain, and binning.
    //
    if ( !tokens.empty() ) {
      try {
        float exptime = std::stof( tokens.at(0) );
        int gain = std::stoi( tokens.at(1) );
        int bin  = std::stoi( tokens.at(2) );

        // If framegrab is running then stop it. This won't return until framegrabbing
        // has stopped (or timeout).
        //
        bool was_framegrab_running = this->is_framegrab_running.load(std::memory_order_acquire);
        if ( was_framegrab_running ) {
          std::string dontcare;
          error = this->framegrab( "stop", dontcare );
        }

        error |= camera.set_exptime( exptime );
        error |= camera.set_gain( gain );
        error |= camera.bin( bin, bin );

        if ( tokens.size() == 5 ) {
          for ( const auto &pair : this->camera.andor ) {
            pair.second->set_weight(std::stof(tokens.at(3)));
            if ( std::stoi(tokens.at(4))==1 ) {
              logwrite(function,message.str());
              pair.second->reset_avg();
            }
          }
        }

        set=true;

        // If framegrab was previously running then restart it
        //
        if ( was_framegrab_running ) {
          std::string dontcare;
          error = this->framegrab( "start", dontcare );
        }
      }
      catch( const std::exception &e ) {
        message.str(""); message << "ERROR parsing args \"" << args << "\": " << e.what();
        logwrite( function, message.str() );
        return ERROR;
      }
    }

    // Set or not, now read the current values and use the gui_manager
    // to set them in the class.
    //
    gui_manager.exptime = info.exptime;
    gui_manager.gain = info.gain;
    gui_manager.bin = (info.hbin==info.vbin ? info.hbin : -1 );
    gui_manager.navg = this->camera.andor.begin()->second->get_weight();

    // If read-only (not set) or either exptime or gain changed, then set the flag for updating the GUI.
    //
    if ( !set || gui_manager.exptime != exptime_og || gui_manager.gain != gain_og ||
                 gui_manager.bin != bin_og || gui_manager.navg != navg_og ) {
      gui_manager.set_update();
    }

    retstring = gui_manager.get_message_string();

    logwrite( function, retstring );

    std::thread( &Slicecam::GUIManager::push_gui_settings, &gui_manager ).detach();

    return error;
  }
  /***** Slicecam::Interface::gui_settings_control ****************************/


  /***** Slicecam::Interface::gui_settings_control ****************************/
  /**
   * @brief      get the GUI settings for the SAOImage GUI display
   * @details    Passes an empty string to gui_settings_control() so that
   *             the parameters will only be returned and pushed (not set).
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Interface::gui_settings_control() {
    std::string retstring;
    return ( this->gui_settings_control( std::string(""), retstring ) );
  }
  /***** Slicecam::Interface::gui_settings_control ****************************/


  /***** Slicecam::Interface::avg_frames **************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::avg_frames( const std::string args, std::string &retstring ) {
    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_AVGFRAMES;
      retstring.append( " [ reset | <num> ]\n" );
      retstring.append( "   Set or get the number of frames in a running average. If no arg is\n" );
      retstring.append( "   supplied then the current value is returned.\n" );
      return HELP;
    }
    if ( !args.empty() ) {
      if ( args=="reset" ) {
        for ( const auto &pair : this->camera.andor ) {
          pair.second->reset_avg();
        }
      }
      else {
        try {
          float num = std::stoi( args );
          if ( num < 1 ) {
            logwrite( "Slicecam::Interface::avg_frames", "ERROR <num> must be greater than 0" );
            retstring="invalid_argument";
            return ERROR;
          }
          for ( const auto &pair : this->camera.andor ) {
            pair.second->set_weight(num);
          }
        }
        catch( const std::exception &e ) {
          logwrite( "Slicecam::Interface::avg_frames", "ERROR parsing value: "+std::string(e.what()) );
          retstring="bad_value";
          return ERROR;
        }
      }
    }
    retstring=std::to_string(this->camera.andor.begin()->second->get_weight());
    return NO_ERROR;
  }
  /***** Slicecam::Interface::avg_frames **************************************/


  /***** Slicecam::Interface::dothread_fpoffset *******************************/
  /**
   * @brief      for testing, calls a Python function from a thread
   *
   */
  void Interface::dothread_fpoffset( Slicecam::Interface &iface ) {
    std::string function = "Slicecam::Interface::dothread_fpoffset";
    std::stringstream message;

    message.str(""); message << "calling fpoffsets.compute_offset() from thread: PyGILState=" << PyGILState_Check();
    logwrite( function, message.str() );

    double ra_to, dec_to, angle_to;

    iface.fpoffsets.compute_offset( "SCOPE", "ACAM", 17, -24, 19, ra_to, dec_to, angle_to );

    message.str(""); message << "output = " << ra_to << " " << dec_to << " " << angle_to << " : PyGILState=" << PyGILState_Check();
    logwrite( function, message.str() );

    return;
  }
  /***** Slicecam::Interface::dothread_fpoffset *******************************/


  /***** Slicecam::Interface::get_acam_guide_state ****************************/
  /**
   * @brief      asks if ACAM is guiding
   * @details    The only way this can fail is if acam is connected but returns
   *             an error reading the state.
   * @param[out] is_guiding  bool guiding state
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::get_acam_guide_state( bool &is_guiding ) {
    std::string function = "Slicecam::Interface::get_acam_guide_state";
    std::stringstream message;
    long error = NO_ERROR;
    std::string retstring;

    // If not connected to acamd then try to connect to the daemon.
    // If there's an error in doing this then assume acamd is not even
    // running, in which case the guiding cannot be running.
    //
    error = this->acamd.is_connected(retstring);
    if ( error == ERROR ) {
      logwrite( function, "ERROR no response from acamd -- will assume guiding is inactive" );
      is_guiding = false;
      return NO_ERROR;
    }

    // Not connected, try to connect
    //
    if ( retstring.find("false") != std::string::npos ) {
      logwrite( function, "connecting to acamd" );
      error = this->acamd.connect();
      if ( error != NO_ERROR ) {
        logwrite( function, "ERROR unable to connect to acamd -- will assume guiding is inactive" );
        is_guiding=false;
        return NO_ERROR;
      }
      logwrite( function, "connected to acamd" );
    }

    // Is acamd guiding? At this point slicecam is connected to acamd, so
    // consider an error here as a fault and don't continute.
    //
    error = this->acamd.send( ACAMD_ACQUIRE, retstring );
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR getting guiding state from ACAM" );
      return ERROR;
    }

    // If guiding is in the return string then it is enabled.
    //
    if ( retstring.find( "guiding" ) != std::string::npos ) {
      is_guiding = true;
    }
    else is_guiding = false;

    message.str(""); message << "acam is" << ( is_guiding ? " " : " not " ) << "guiding";

    return NO_ERROR;
  }
  /***** Slicecam::Interface::get_acam_guide_state ****************************/


  /***** Slicecam::Interface::put_on_slit *************************************/
  /**
   * @brief      put target on slit
   * @details    Intended to be called by the GUI to move the clicked-on
   *             target to the slit.
   * @param[in]  args       string expects "<slitra> <slitdec> <crossra> <crossdec>"
   * @param[out] retstring  reference to string for any return values
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::put_on_slit( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::put_on_slit";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_PUTONSLIT;
      retstring.append( " <slitra> <slitdec> <crossra> <crossdec>\n" );
      retstring.append( "   Move selected target to the slit. Intended to be called by the GUI to\n" );
      retstring.append( "   move the clicked-on target to the slit. The call must supply the RA,DEC\n" );
      retstring.append( "   coordinates of the slit, and the RA,DEC coordinates of the crosshairs,\n" );
      retstring.append( "   all units in decimal degrees. This will result in a PT command to send\n" );
      retstring.append( "   the required offsets to the TCS.\n" );
      return HELP;
    }

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 4 ) {
      logwrite( function, "ERROR expected <slitra> <slitdec> <crossra> <crossdec>" );
      retstring="invalid_argument";
      return ERROR;
    }

    // outputs: result from solve_offset in degrees
    //
    double ra_off, dec_off;

    try {
      // inputs
      //
      double slitra   = std::stod( tokens.at(0) );
      double slitdec  = std::stod( tokens.at(1) );
      double crossra  = std::stod( tokens.at(2) );
      double crossdec = std::stod( tokens.at(3) );

      // call solve_offset from SkyInfo::FPOffsets class which uses Python
      //
      if ( this->fpoffsets.solve_offset( slitra, slitdec, crossra, crossdec, ra_off, dec_off ) != NO_ERROR ) {
        logwrite( function, "ERROR from Python solve_offset" );
        retstring="fpoffsets_failed";
        return ERROR;
      }
    }
    catch( const std::exception &e ) {
      message.str(""); message << "ERROR parsing input \"" << args << "\": " << e.what();
      logwrite( function, message.str() );
      retstring="argument_exception";
      return ERROR;
    }

    // If ACAM is guiding then slicecam must not move the telescope,
    // but must allow ACAM to perform the offset.
    //
    bool is_guiding;
    long error = this->get_acam_guide_state( is_guiding );

    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR getting guide state" );
      return ERROR;
    }

    // send the offsets now
    //
    if ( is_guiding ) {
      // Send to acamd if guiding
      //
      std::stringstream cmd;
      cmd << ACAMD_OFFSETGOAL << " " << std::fixed << std::setprecision(6) << ra_off << " " << dec_off;
      error = this->acamd.command( cmd.str() );
      if ( error != NO_ERROR ) {
        logwrite( function, "ERROR adding offset to acam goal" );
        return ERROR;
      }
    }
    else
    if ( !is_guiding && this->tcs_online.load(std::memory_order_acquire) && this->tcsd.client.is_open() ) {
      // offsets are in degrees, convert to arcsec (required for PT command)
      //
      ra_off  *= 3600.;
      dec_off *= 3600.;

      // Send them directly to the TCS when not guiding
      //
      if ( this->tcsd.pt_offset( ra_off, dec_off, OFFSETRATE ) != NO_ERROR ) {
        logwrite( function, "ERROR offsetting telescope" );
        retstring="tcs_error";
        return ERROR;
      }
    }
    else if ( !is_guiding ) {
      logwrite( function, "ERROR not connected to tcsd" );
      retstring="tcs_not_connected";
      return ERROR;
    }

    message.str(""); message << "requested offsets dRA=" << ra_off << " dDEC=" << dec_off << " arcsec";
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Slicecam::Interface::put_on_slit *************************************/


  /***** Slicecam::Interface::shutdown ****************************************/
  /**
   * @brief      shutdown all threads and connections
   * @param[in]  args       only used for help
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::shutdown( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::shutdown";
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
      retstring = SLICECAMD_SHUTDOWN;
      retstring.append( "\n" );
      retstring.append( "  Shuts down all threads which communicate with the cameras and\n" );
      retstring.append( "  TCS, and close all connections. The daemon remains running.\n" );
      return HELP;
    }

    long error = NO_ERROR;
    std::string dontcare;

    // this stops framegrabbing and closes socket connections to hardware
    //
    error |= this->close( "", dontcare );

    // shutdown TCS connection
    //
    error |= this->tcs_init( "shutdown", dontcare );

    if ( error == NO_ERROR ) logwrite( function, "slicecam interfaces shut down" );
    else logwrite( function, "ERROR shutting down slicecam interfaces" );

    return error;
  }
  /***** Slicecam::Interface::shutdown ****************************************/


  /***** Slicecam::Interface::test ********************************************/
  /**
   * @brief      test routines
   * @details    This is the place to put various debugging and system testing
   *             tools. The server command is "test", the next parameter is
   *             the test name, and any parameters needed for the particular
   *             test are extracted as tokens from the args string.
   * @param[in]  args       test name and arguments
   * @param[out] retstring  reference to string for any return values
   * @return     ERROR | NO_ERROR | HELP
   *
   * Valid test names are:
   *
   * adchans
   * emgainrange
   * fpoffsets <from> <to> <ra> <dec> <angle>
   * getemgain
   * sleep
   * sliceparams
   * threadoffset
   *
   */
  long Interface::test( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::test";
    std::stringstream message;
    std::vector<std::string> tokens;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" ) {
      retstring = SLICECAMD_TEST;
      retstring.append( "\n" );
      retstring.append( "  Test Routines\n" );
      retstring.append( "   adchans [ ? ]\n" );
      retstring.append( "   emgainrange\n" );
      retstring.append( "   fpoffsets ? | <from> <to> <ra> <dec> <angle> (see help for units)\n" );
      retstring.append( "   getemgain\n" );
      retstring.append( "   handlemap [?]\n" );
      retstring.append( "   isguiding [ ? ]\n" );
      retstring.append( "   offsetgoal ? | <dRA> <dDEC>\n" );
      retstring.append( "   shouldframegrab [?]\n" );
      retstring.append( "   sleep\n" );
      retstring.append( "   sliceparams [ ? ]\n" );
      retstring.append( "   threadoffset [ ? ]\n" );
      return HELP;
    }

    Tokenize( args, tokens, " " );

    if ( tokens.size() < 1 ) {
      logwrite( function, "ERROR no test name provided" );
      retstring="invalid_argument";
      return ERROR;
    }

    std::string testname = tokens[0];

    if ( testname == "fpoffsets" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {                              // help
        retstring = SLICECAMD_TEST;
        retstring.append( " fpoffsets <from> <to> <ra> <dec> <angle>\n" );
        retstring.append( "   Calculate focal plane offsets from <from> frame to <to> frame.\n" );
        retstring.append( "   Coordinates can be decimal or sexagesimal. If decimal then <ra> can be\n" );
        retstring.append( "   decimal degrees or decimal hours. Specify decimal hours by adding \"h\"\n" );
        retstring.append( "   as in \"1.234d\". Angle is always decimal degrees.\n" );
        retstring.append( "   Returned offsets will be in decimal degrees.\n" );
        return HELP;
      }

      if ( tokens.size() != 6 ) {
        message.str(""); message << "ERROR received " << tokens.size() << " args but expected <from> <to> <ra> <dec> <angle>";
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return ERROR;
      }

      double ra_from, dec_from, angle_from;
      double ra_to, dec_to, angle_to;

      // Convert the strings to decimal based on what they look like
      //
      try {
        std::string from    = tokens.at(1);
        std::string to      = tokens.at(2);

        std::string ra_str  = tokens.at(3);
        std::string dec_str = tokens.at(4);

        // Parse RA
        //
        if ( ra_str.find(":") != std::string::npos ) {        // assume sexagesimal if there's a colon in the string
          ra_from = radec_to_decimal( ra_str ) * TO_DEGREES;  // convert to decimal degrees
        }
        else
        if ( ends_with( ra_str, "h" ) ) {                     // incoming decimal hours
          ra_str.pop_back();                                  // remove the h
          ra_from = std::stod( ra_str ) * TO_DEGREES;         // convert to decimal degrees
        }
        else {                                                // leaves only decimal degrees
          ra_from = std::stod( ra_str );
        }

        // Parse DEC
        //
        if ( dec_str.find(":") != std::string::npos ) {       // assume sexagesimal if there's a colon
          dec_from = radec_to_decimal( dec_str );             // convert to decimal degrees
        }
        else {                                                // leaves onnly decimal degrees
          dec_from = std::stod( dec_str ) * TO_DEGREES;
        }

        // Parse Angle
        //
        angle_from = std::stod( tokens.at(5) );               // always in decimal degrees

        // Call SkyInfo::FPOffsets::compute_offset()
        //
        error = this->fpoffsets.compute_offset( from, to, ra_from, dec_from, angle_from, ra_to, dec_to, angle_to );

        message.str("");
        message << from << " -> " << to << "\n"
                << ra_from << " " << dec_from << " " << angle_from << " -> " << ra_to << " " << dec_to << " " << angle_to;
        retstring = message.str();
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR parsing \"" << args << "\" : " << e.what();
        logwrite( function, message.str() );
        retstring="argument_exception";
        return ERROR;
      }
    }
    else
    if ( testname == "threadoffset" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = SLICECAMD_TEST;
        retstring.append( " threadoffset\n" );
        retstring.append( "  Spawns a thread which calls a Python function.\n" );
        return HELP;
      }
      else {
        message.str(""); message << "spawning dothread_fpoffset: PyGILState=" << PyGILState_Check();
        logwrite( function, message.str() );
        std::thread( this->dothread_fpoffset, std::ref(*this) ).detach();
        message.str(""); message << "spawned dothread_fpoffset: PyGILState=" << PyGILState_Check();
        logwrite( function, message.str() );
      }
    }
    else
    if ( testname == "adchans" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = SLICECAMD_TEST;
        retstring.append( " adchans\n" );
        retstring.append( "  Calls Andor SDK wrapper to return the number of AD converter channels\n" );
        retstring.append( "   available.\n" );
        return HELP;
      }
      else {
        int chans;
        for ( const auto &pair : this->camera.andor ) {
          error |= pair.second->sdk._GetNumberADChannels( chans );
          message.str(""); message << pair.second->camera_info.camera_name << "slicecam : " << chans;
          logwrite( function, message.str() );
        }
        retstring = std::to_string(chans);
      }
    }
    else
    if ( testname == "emgainrange" ) {
      int low, high;
      for ( const auto &pair : this->camera.andor ) {
        error |= pair.second->get_emgain_range( low, high );
        message.str(""); message << pair.second->camera_info.camera_name << "slicecam : " << low << " " << high;
        logwrite( function, message.str() );
      }
      retstring = std::to_string(low) + " " + std::to_string(high);
    }
    else
    if ( testname == "getemgain" ) {
      int gain;
      for ( const auto &pair : this->camera.andor ) {
        error |= pair.second->get_emgain( gain );
        message.str(""); message << pair.second->camera_info.camera_name << "slicecam : " << gain;
        logwrite( function, message.str() );
      }
      retstring = std::to_string( gain );
    }
    else
    if ( testname == "handlemap" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = SLICECAMD_TEST;
        retstring.append( " handlemap\n" );
        retstring.append( "  Refreshes the camera handle map. Use with caution.\n" );
        retstring.append( "  Cameras must be closed first.\n" );
        return HELP;
      }
      // make sure no cameras are open
      //
      for ( const auto &pair : this->camera.andor ) {
        if ( pair.second->is_open() ) {
          message.str(""); message << "ERROR slicecam " << pair.second->camera_info.camera_name << " is open";
          logwrite( function, message.str() );
          retstring="not_while_camera_open";
          return ERROR;
        }
      }
      // if I'm here then all closed. erase the map and get a new one
      //
      error = this->camera.init_handlemap();
    }
    else
    if ( testname == "isguiding" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = SLICECAMD_TEST;
        retstring.append( " isguiding\n" );
        retstring.append( "  Return the acam guiding state\n" );
        return HELP;
      }
      bool is_guiding;
      error = this->get_acam_guide_state( is_guiding );
      message.str(""); message << "request returned " << ( error==ERROR ? "ERROR" : "NO_ERROR" ) << ": guiding is " << ( is_guiding ? "on" : "off" );
      retstring = message.str();
    }
    else
    if ( testname == "offsetgoal" ) {
      if ( tokens.size() > 1 && (tokens[1] == "?" ||tokens[1] == "help") ) {
        retstring = SLICECAMD_TEST;
        retstring.append( " offsetgoal <dRA> <dDEC>\n" );
        retstring.append( "  \n" );
        return HELP;
      }
      if ( tokens.size() != 3 ) {
        logwrite( function, "ERROR expected <dRA> <dDEC>" );
        retstring="invalid_argument";
        return ERROR;
      }
      bool is_guiding;
      long error = this->get_acam_guide_state( is_guiding );

      if ( error != NO_ERROR ) {
        logwrite( function, "ERROR getting guide state" );
        retstring="acamd_error";
        return ERROR;
      }

      if ( !is_guiding ) {
        logwrite( function, "ERROR acam is not guiding" );
        retstring="not_guiding";
        return ERROR;
      }
      message.str(""); message << ACAMD_OFFSETGOAL << " " << tokens[1] << " " << tokens[2];
      error = this->acamd.command( message.str() );
      if ( error != NO_ERROR ) {
        logwrite( function, "ERROR adding offset to acam goal" );
        return ERROR;
      }
    }
    else
    // --------------------------------
    // shouldframegrab
    //
    if ( testname == "shouldframegrab" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = SLICECAMD_TEST;
        retstring.append( " shouldframegrab\n" );
        retstring.append( "  Returns state of should_framegrab_run\n" );
        return HELP;
      }
      retstring = ( this->should_framegrab_run.load(std::memory_order_acquire) ? "yes" : "no" );
    }
    else
    if ( testname == "sleep" ) {
      for ( int i=0; i<10; i++ ) {
        logwrite( function, "sleeping . . ." );
        std::this_thread::sleep_for( std::chrono::seconds(1) );
      }
    }
    else
    if ( testname == "sliceparams" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = SLICECAMD_TEST;
        retstring.append( " sliceparams\n" );
        retstring.append( "  Show the slicecam params calculated by Python getSlicevParams function.\n" );
        return HELP;
      }
      // reads slicecam params into fpoffsets class
      this->fpoffsets.get_slicecam_params();
      // log those params
      message.str("");
      for ( const auto &pair : this->camera.andor ) {
        std::string cam = pair.second->camera_info.camera_name;
        message << "camera " << cam << " cdelta1=" << this->fpoffsets.sliceparams[cam].cdelt1
                << " cdelta2=" << this->fpoffsets.sliceparams[cam].cdelt2
                << " crpix1=" << this->fpoffsets.sliceparams[cam].crpix1
                << " crpix2=" << this->fpoffsets.sliceparams[cam].crpix2
                << " thetadeg=" << this->fpoffsets.sliceparams[cam].thetadeg << "\n";
      }
      retstring = message.str();
      return HELP;
    }
    else {
      message.str(""); message << "ERROR unknown testname \"" << testname << "\"";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    return error;

  }
  /***** Slicecam::Interface::test ********************************************/


  /***** Slicecam::Interface::exptime *****************************************/
  /**
   * @brief      wrapper to set/get exposure time
   * @param[in]  args       optional requested exposure time
   * @param[out] retstring  return string contains current exposure time
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::exptime( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::exptime";
    std::stringstream message;

    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_EXPTIME;
      retstring.append( " [ <exptime> ]\n" );
      retstring.append( "  Set or get camera exposure time in decimal seconds.\n" );
      retstring.append( "  If <exptime> is provided then the camera exposure time is changed,\n" );
      retstring.append( "  else the current exposure time is returned.\n" );
      return HELP;
    }

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->camera.andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // No arg just return the exposure time stored in the Andor Information class
    // without reading from the Andor
    //
    if ( args.empty() ) {
      retstring = std::to_string( this->camera.andor.begin()->second->camera_info.exptime );
      return NO_ERROR;
    }

    // make sure the input value is reasonable
    //
    float fval=NAN;
    try {
      fval = std::stof( args );
      if ( std::isnan(fval) || fval < 0 ) {
        logwrite( function, "ERROR invalid exposure time" );
        retstring="invalid_argument";
        return ERROR;
      }
    }
    catch( const std::exception &e ) {
      message.str(""); message << "ERROR parsing exptime value " << args << ": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    // I have a potentially valid float value now

    // If framegrab is running then stop it. This won't return until framegrabbing
    // has stopped (or timeout).
    //
    long error = NO_ERROR;
    bool was_framegrab_running = this->is_framegrab_running.load(std::memory_order_acquire);
    if ( was_framegrab_running ) {
      std::string dontcare;
      error = this->framegrab( "stop", dontcare );
    }

    this->camera.set_exptime( fval );

    // If framegrab was previously running then restart it
    //
    if ( was_framegrab_running ) {
      std::string dontcare;
      error = this->framegrab( "start", dontcare );
    }

    retstring = std::to_string(fval);
    message.str(""); message << fval << " sec";
    logwrite( function, message.str() );

    return error;
  }
  /***** Slicecam::Interface::exptime *****************************************/


  /***** Slicecam::Interface::fan_mode ****************************************/
  /**
   * @brief      set camera fan mode
   * @param[in]  args       {full low off}
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::fan_mode( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::fan_mode";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_FAN;
      retstring.append( " <name> <mode>\n" );
      retstring.append( "   Set camera fan mode for camera <name> where <mode> is { full low off }.\n" );
      retstring.append( "   The fan should only be turned off for short periods of time.\n" );
      retstring.append( "   Fan state cannot be directly read. Reported state is only known\n" );
      retstring.append( "   by remembering the last state set, so if not set manually then no state\n" );
      retstring.append( "   can be reported.\n" );
      return HELP;
    }

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->camera.andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // tokenize the args to get the camera name and the rot arg
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      logwrite( function, "ERROR expected 2 args L|R <mode>" );
      retstring="invalid_argument";
      return ERROR;
    }

    std::string which;
    int mode;

    // assign the numeric mode value from the string argument
    //
    try {
      which = tokens.at(0);
      // convert to lowercase
      std::transform( tokens.at(1).begin(), tokens.at(1).end(), tokens.at(1).begin(), ::tolower );
      if ( tokens.at(1) == "full" ) mode = 0;
      else
      if ( tokens.at(1) == "low" )  mode = 1;
      else
      if ( tokens.at(1) == "off" )  mode = 2;
      else {
        message.str(""); message << "ERROR bad arg " << tokens.at(1) << ": expected { full low off }";
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

    // perform the image rotation
    //
    error = this->camera.andor[which]->set_fan( mode );

    // set the mode
    //
    error = this->camera.set_fan( which, mode );

    if ( error == NO_ERROR ) {
      message.str(""); message << "set fan to " << args;
      logwrite( function, message.str() );
    }
    else logwrite( function, "ERROR setting fan" );

    return error;
  }
  /***** Slicecam::Interface::fan_mode ****************************************/


  /***** Slicecam::Interface::gain ********************************************/
  /**
   * @brief      set or get the CCD gain
   * @details    The output amplifier is automatically set based on gain.
   *             If gain=1 then set to conventional amp and if gain > 1
   *             then set the EMCCD gain register.
   * @param[in]  args       optionally contains new gain
   * @param[out] retstring  return string contains gain
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::gain( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::gain";
    std::stringstream message;
    long error = NO_ERROR;
    int gain = -999;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_GAIN;
      retstring.append( " [ <gain> ]\n" );
      retstring.append( "   Set or get CCD Gain.\n" );
      if ( !this->camera.andor.empty() ) {
        if ( this->camera.andor.begin()->second->is_open() ) {
          int low, high;
          this->camera.andor.begin()->second->get_emgain_range( low, high );
          retstring.append( "   Select <gain> = 1 for conventional or in range { " );
          retstring.append( std::to_string( low ) );
          retstring.append( " " );
          retstring.append( std::to_string( high ) );
          retstring.append( " } for EMCCD.\n" );
        }
        else {
          retstring.append( "   Open connection to camera to see gain range.\n" );
        }
      }
      retstring.append( "   If <gain> is omitted then the current gain is returned.\n" );
      retstring.append( "   Output amplifier is automatically selected as conventional for unity gain,\n" );
      retstring.append( "   or EM for non-unity gain.\n" );
      return HELP;
    }

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->camera.andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // No args, just read gain
    //
    if ( args.empty() ) {
      gain = this->camera.andor.begin()->second->camera_info.gain;
    }
    else {
      // otherwise parse args
      //
      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // There must be only one arg (the requested EM gain)
      //
      if ( tokens.size() != 1 ) {
        logwrite( function, "ERROR too many arguments" );
        retstring="invalid_argument";
        return ERROR;
      }

      // If framegrab is running then stop it. This won't return until framegrabbing
      // has stopped (or timeout).
      //
      bool was_framegrab_running = this->is_framegrab_running.load(std::memory_order_acquire);
      if ( was_framegrab_running ) {
        std::string dontcare;
        error = this->framegrab( "stop", dontcare );
      }

      // Parse the gain from the token
      //
      try {
        gain  = std::stoi( tokens.at(0) );
        error = this->camera.set_gain( gain );  // this returns the set gain as confirmation
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR parsing gain: " << e.what();
        logwrite( function, message.str() );
        retstring="parsing_exception";
        return ERROR;
      }

      // If framegrab was previously running then restart it
      //
      if ( was_framegrab_running ) {
        std::string dontcare;
        error |= this->framegrab( "start", dontcare );
      }

    }

    // return string is the current gain
    //
    retstring = std::to_string( gain );

    message.str(""); message << ( error==ERROR ? "ERROR " : "" ) << retstring;
    logwrite( function, message.str() );

    return error;
  }
  /***** Slicecam::Interface::gain ********************************************/


  /***** Slicecam::Interface::collect_header_info *****************************/
  /**
   * @brief      gather information and add it to the internal keyword database
   * @details    Some of the keys are fixed, some come from the Andor::Information
   *             class (which is stored in the camera.andor.camera_info object),
   *             and some has to be retrieved directly from the TCS. The keyword
   *             database is erased and rebuilt each time through, so there is
   *             no chance for stale keywords.
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::collect_header_info( std::unique_ptr<Andor::Interface> &slicecam ) {
    std::string function = "Slicecam::Interface::collect_header_info";
    std::stringstream message;

    std::string cam = slicecam->camera_info.camera_name;

    bool _tcs = telem.is_tcs_open;
    std::string tcsname = ( _tcs ? telem.tcsname : "offline" );

    double angle_slit=NAN,  ra_slit=NAN,  dec_slit=NAN;

    // Compute FP offsets from TCS coordinates (SCOPE) to SLIT coodinates.
    // compute_offset() always wants degrees and get_coords() returns RA hours.
    // Results in degrees. Add thetadeg to the slit angle.
    //
    if ( _tcs ) this->fpoffsets.compute_offset( "SCOPE", "SLIT", (telem.ra_scope_h*TO_DEGREES), telem.dec_scope_d, telem.angle_scope,
                                                                             ra_slit, dec_slit, angle_slit );

    angle_slit += this->fpoffsets.sliceparams[cam].thetadeg;

    // Get some info from the Andor::Information class,
    // which is stored in its camera_info object.
    //
    std::copy( std::begin( slicecam->camera_info.axes ),
                 std::end( slicecam->camera_info.axes ),
               std::begin( this->camera.fitsinfo.axes ) );

    // adjust WCS parameters for binning
    //
    auto hbin   = slicecam->camera_info.hbin;
    auto vbin   = slicecam->camera_info.vbin;
    auto pixscale = ( hbin==vbin ? this->fpoffsets.sliceparams[cam].pixscale*hbin : NAN );
    auto cdelt1 = this->fpoffsets.sliceparams[cam].cdelt1 * hbin;
    auto cdelt2 = this->fpoffsets.sliceparams[cam].cdelt2 * vbin;

    // and adjust CRPIX1 (horizonatal) for slit width and offset
    //
    double factor = ( cam == "L" ? 1.5 : -1.5 );
    double corr   = factor * telem.slitwidth / pixscale;
    double corr2 = (telem.slitoffset+3)/pixscale/3*1.2; // Compute offset relative to -3"
    auto crpix1   = (this->fpoffsets.sliceparams[cam].crpix1 / hbin) + corr + corr2;
    auto crpix2   = (this->fpoffsets.sliceparams[cam].crpix2 / vbin);

    // adjusting DATASEC is a little convoluted
    // datasec = "[h1:h2,v1:v2]"
    //
    // remove the [ ]
    //
    std::string datasec = this->fpoffsets.sliceparams[cam].datasec;
    datasec.erase( std::remove( datasec.begin(), datasec.end(), '['), datasec.end() );
    datasec.erase( std::remove( datasec.begin(), datasec.end(), ']'), datasec.end() );

    // separate the h and v parts separated by comma into hdatasec,vdatasec
    //
    auto it = datasec.find(",");
    if ( it == std::string::npos ) {
      logwrite( function, "malformed DATASEC from fpoffsets.sliceparams" );
      datasec.clear();
    }
    else {
      // separate the 1 and 2 parts separated by :
      //
      std::string hdatasec = datasec.substr(0,it);
      std::string vdatasec = datasec.substr(it+1);
      auto hit = hdatasec.find(":");
      auto vit = vdatasec.find(":");
      if ( hit == std::string::npos || vit == std::string::npos ) {
        logwrite( function, "malformed DATASEC from fpoffsets.sliceparams" );
        datasec.clear();
      }
      else {
        try {
          // convert to integers
          //
          int h1 = std::stoi(hdatasec.substr(0,hit));
          int h2 = std::stoi(hdatasec.substr(hit+1));
          int v1 = std::stoi(vdatasec.substr(0,vit));
          int v2 = std::stoi(vdatasec.substr(vit+1));

          // scale each by the binning factor
          // quick way to round up
          //
          h1 = ( h1 + hbin - 1 ) / hbin;
          h2 = ( h2 + hbin - 1 ) / hbin;
          v1 = ( v1 + vbin - 1 ) / vbin;
          v2 = ( v2 + vbin - 1 ) / vbin;

          // adjust horiz values for slit offset
          //
          int offcorr = telem.slitoffset/pixscale/3;

          h1 = h1 + ( cam=="L" ?  0 : -1 ) * offcorr;
          h2 = h2 + ( cam=="L" ? -1 :  0 ) * offcorr;

          // put the scaled values back into a string
          //
          message.str(""); message << "[" << h1 << ":" << h2
                                   << "," << v1 << ":" << v2 << "]";
          datasec = message.str();
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "malformed DATASEC from fpoffsets.sliceparams: " << e.what();
          logwrite( function, message.str() );
        }
      }
    }

    // Add information to the Camera::FitsInfo::FitsKeys database
    // either a prioi or from the Andor::Information class
    //
    slicecam->fitskeys.erase_db();

    slicecam->fitskeys = this->telemkeys.primary();

    slicecam->fitskeys.addkey( "TCS",  tcsname, "" );

    slicecam->fitskeys.addkey( "CREATOR",  "slicecamd", "file creator" );
    slicecam->fitskeys.addkey( "INSTRUME", "NGPS", "name of instrument" );
    slicecam->fitskeys.addkey( "TELESCOP", "P200", "name of telescope" );
    slicecam->fitskeys.addkey( "TELFOCUS", telem.telfocus, "telescope focus (mm)" );

    slicecam->fitskeys.addkey( "PIXSCALE",  pixscale, "arcsec per pixel" );
    slicecam->fitskeys.addkey( "CRPIX1",    crpix1, "" );
    slicecam->fitskeys.addkey( "CRPIX2",    crpix2, "" );
    slicecam->fitskeys.addkey( "DATASEC",   datasec, "" );
    slicecam->fitskeys.addkey( "HBIN",      hbin, "horizontal binning pixels" );
    slicecam->fitskeys.addkey( "VBIN",      vbin, "vertical binning pixels" );
    slicecam->fitskeys.addkey( "CDELT1",    cdelt1, "" );
    slicecam->fitskeys.addkey( "CDELT2",    cdelt2, "" );
    slicecam->fitskeys.addkey( "THETADEG",  this->fpoffsets.sliceparams[cam].thetadeg, "slice camera angle relative to slit in deg" );

    slicecam->fitskeys.addkey( "EXPSTART", slicecam->camera_info.timestring, "exposure start time" );
    slicecam->fitskeys.addkey( "MJD0",     slicecam->camera_info.mjd0, "exposure start time (modified Julian Date)" );
    slicecam->fitskeys.addkey( "EXPTIME",  slicecam->camera_info.exptime, "exposure time (sec)" );
    slicecam->fitskeys.addkey( "SERNO",    slicecam->camera_info.serial_number, "camera serial number" );
    slicecam->fitskeys.addkey( "NAME",     cam, "camera name" );
    slicecam->fitskeys.addkey( "READMODE", slicecam->camera_info.readmodestr, "read mode" );
    slicecam->fitskeys.addkey( "IMROT",    slicecam->camera_info.rotstr, "image rotation (cw ccw none)" );
    slicecam->fitskeys.addkey( "HFLIP",    slicecam->camera_info.hflip, "horizontal flip" );
    slicecam->fitskeys.addkey( "VFLIP",    slicecam->camera_info.vflip, "vertical flip" );
    slicecam->fitskeys.addkey( "TEMPSETP", slicecam->camera_info.setpoint, "detector temperature setpoint deg C" );
    slicecam->fitskeys.addkey( "TEMPREAD", slicecam->camera_info.ccdtemp, "CCD temperature deg C" );
    slicecam->fitskeys.addkey( "TEMPSTAT", slicecam->camera_info.temp_status, "CCD temperature status" );
    slicecam->fitskeys.addkey( "FITSNAME", slicecam->camera_info.fits_name, "this filename" );
    slicecam->fitskeys.addkey( "HSPEED",   slicecam->camera_info.hspeed, "horizontal clocking speed MHz" );
    slicecam->fitskeys.addkey( "VSPEED",   slicecam->camera_info.vspeed, "vertical clocking speed MHz" );
    slicecam->fitskeys.addkey( "AMPTYPE",  slicecam->camera_info.amptypestr, "CCD amplifier type" );
    slicecam->fitskeys.addkey( "CCDGAIN",  slicecam->camera_info.gain, "CCD amplifier gain" );

    slicecam->fitskeys.addkey( "GAIN",     1, "e-/ADU" );

    slicecam->fitskeys.addkey( "POSANG",    angle_slit, "" );
    slicecam->fitskeys.addkey( "TELRA",     telem.ra_scope_h, "Telecscope Right Ascension hours" );
    slicecam->fitskeys.addkey( "TELDEC",    telem.dec_scope_d, "Telescope Declination degrees" );
    slicecam->fitskeys.addkey( "CASANGLE",  telem.angle_scope, "Cassegrain ring angle" );
    slicecam->fitskeys.addkey( "RAOFFS",    telem.offsetra, "Telescope RA offset" );
    slicecam->fitskeys.addkey( "DECLOFFS",  telem.offsetdec, "Telescope DEC offset" );
    slicecam->fitskeys.addkey( "AIRMASS",   telem.airmass, "" );
    slicecam->fitskeys.addkey( "WCSAXES",   2, "" );
    slicecam->fitskeys.addkey( "RADESYSA",  "ICRS", "" );
    slicecam->fitskeys.addkey( "CTYPE1",    "RA---TAN", "" );
    slicecam->fitskeys.addkey( "CTYPE2",    "DEC--TAN", "" );


    slicecam->fitskeys.addkey( "CRVAL1",    ra_slit, "" );
    slicecam->fitskeys.addkey( "CRVAL2",    dec_slit, "" );
    slicecam->fitskeys.addkey( "CUNIT1",    "deg", "" );
    slicecam->fitskeys.addkey( "CUNIT2",    "deg", "" );

    slicecam->fitskeys.addkey( "PC1_1",     ( -1.0 * cos( angle_slit * PI / 180. ) ), "" );
    slicecam->fitskeys.addkey( "PC1_2",     (        sin( angle_slit * PI / 180. ) ), "" );
    slicecam->fitskeys.addkey( "PC2_1",     (        sin( angle_slit * PI / 180. ) ), "" );
    slicecam->fitskeys.addkey( "PC2_2",     (        cos( angle_slit * PI / 180. ) ), "" );
    slicecam->fitskeys.addkey( "NAVG", slicecam->get_weight(), "weighting factor for frame averaging" );

    return NO_ERROR;
  }
  /***** Slicecam::Interface::collect_header_info *****************************/

}
