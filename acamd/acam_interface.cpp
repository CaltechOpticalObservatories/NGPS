/**
 * @file    acam_interface.cpp
 * @brief   this contains the acam interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Acam namespace,
 * and is how the acam daemon interfaces to the acam hardware.
 *
 */

#include "acam_interface.h"

namespace Acam {


  /***** Acam::Camera::emulator ***********************************************/
  /**
   * @brief      enable/disable Andor emulator
   * @param[in]  args       optional state { ? help true false }
   * @param[out] retstring  return status { true false }
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::emulator( std::string args, std::string &retstring ) {
    std::string function = "Acam::Camera::emulator";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_EMULATOR;
      retstring.append( " [ true | false ]\n" );
      retstring.append( "   Enable Andor emulator.\n" );
      retstring.append( "   If the optional { true false } argument is omitted then the current\n" );
      retstring.append( "   state is returned.\n" );
      return HELP;
    }

    // Set the Andor state
    //
    if ( args == "true" ) {
      this->andor.andor_emulator( true );
    }
    else
    if ( args == "false" ) {
      this->andor.andor_emulator( false );
    }
    else
    if ( ! args.empty() ) {
      message.str(""); message << "ERROR unrecognized arg " << args << ": expected \"true\" or \"false\"";
      logwrite( function, message.str() );
      return ERROR;
    }

    // Set the return string
    //
    std::string_view which_andor = this->andor.get_andor_object();

    if ( which_andor == Andor::ANDOR_OBJ_EMULATOR ) retstring="true";
    else
    if ( which_andor == Andor::ANDOR_OBJ_SDK ) retstring="false";
    else {
      retstring="unknown";
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Acam::Camera::emulator ***********************************************/


  /***** Acam::Camera::open ***************************************************/
  /**
   * @brief      open connection to Andor and initialize SDK
   * @param[in]  args  Andor s/n to open
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::open( std::string args ) {
    std::string function = "Acam::Camera::open";
    std::stringstream message;
    long error=NO_ERROR;

    // Opens the Andor and initializes SDK
    //
    if ( this->andor.is_open() ) {
      logwrite( function, "already initialized" );
    }
    else {
      error = this->andor.open( args );
    }

    // Setup camera for continuous readout
    //
    error |= this->gain(2);
    error |= this->andor.set_vsspeed( 4.33 );          // vertical shift speed
    error |= this->andor.set_hsspeed( 30.0 );          // horizontal shift speed
    error |= this->andor.set_read_mode( 4 );           // image mode
    error |= this->andor.set_acquisition_mode( 5 );    // run till abort
    error |= this->andor.set_frame_transfer( "off" );  // disable Frame Transfer mode
    error |= this->andor.set_kinetic_cycle_time( 0 );
    error |= this->andor.set_shutter( "open" );        // shutter always open
    error |= this->set_exptime(1);

    logwrite( function, (error==NO_ERROR ? "camera open" : "ERROR opening camera") );

    return error;
  }
  /***** Acam::Camera::open ***************************************************/


  /***** Acam::Camera::close **************************************************/
  /**
   * @brief      close connection to Andor
   * @details    Any acquisition in progress will be stopped, the shutter will
   *             be closed, and the Andor system will be shut down.
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::close() {
    return this->andor.close();
  }
  /***** Acam::Camera::close **************************************************/


  /***** Acam::Camera::bin ****************************************************/
  /**
   * @brief      set or get camera binning
   * @param[in]  args       optionally contains <hbin> <vbin>
   * @param[out] retstring  return string contains <hbin> <vbin>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::bin( std::string args, std::string &retstring ) {
    std::string function = "Acam::Camera::bin";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_BIN;
      retstring.append( " [ <hbin> <vbin> ]\n" );
      retstring.append( "   Set or get CCD binning.\n" );
      retstring.append( "   <hbin> and <vbin> are the number of pixels to bin horizontally\n" );
      retstring.append( "   and vertically, respectively. When setting either, both must\n" );
      retstring.append( "   be supplied. If both omitted then the current binning is returned.\n" );
      return HELP;
    }

    // Andor must be connected
    //
    if ( !this->andor.is_open() ) {
      logwrite( function, "ERROR no connection to camera" );
      return ERROR;
    }

    // Parse args if present
    //
    if ( !args.empty() ) {

      int hbin, vbin;

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // There can be only one arg (the requested EM gain)
      //
      if ( tokens.size() != 2 ) {
        logwrite( function, "ERROR expected <hbin> <vbin>" );
        return ERROR;
      }

      // Parse the gain from the token
      //
      try {
        hbin = std::stoi( tokens.at(0) );
        vbin = std::stoi( tokens.at(1) );
      }
      catch ( std::out_of_range &e ) {
        message.str(""); message << "ERROR reading arguments: " << e.what();
        error = ERROR;
      }
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR reading arguments: " << e.what();
        error = ERROR;
      }

      if (error==ERROR) logwrite( function, message.str() );

      // Set the binning parameters now
      //
      if (error!=ERROR ) error = this->andor.set_binning( hbin, vbin );
    }

    // return the current binning parameters
    //
    message.str(""); message << this->andor.camera_info.hbin << " " << this->andor.camera_info.vbin;
    retstring = message.str();

    logwrite( function, retstring );

    return error;
  }
  /***** Acam::Camera::bin ****************************************************/


  /***** Acam::Camera::imflip *************************************************/
  /**
   * @brief      set or get camera image flip
   * @param[in]  args       optionally contains <hflip> <vflip> (0=false 1=true)
   * @param[out] retstring  return string contains <hflip> <vflip>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::imflip( std::string args, std::string &retstring ) {
    std::string function = "Acam::Camera::imflip";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_IMFLIP;
      retstring.append( " [ <hflip> <vflip> ]\n" );
      retstring.append( "   Set or get CCD image flip.\n" );
      retstring.append( "   <hflip> and <vflip> indicate to flip horizontally and\n" );
      retstring.append( "   vertically, respectively. Set these =1 to enable flipping,\n" );
      retstring.append( "   or =0 to disable flipping the indicated axis. When setting\n" );
      retstring.append( "   either, both must be supplied. If both omitted then the\n" );
      retstring.append( "   current flip states are returned.\n" );
      return HELP;
    }

    // Andor must be connected
    //
    if ( !this->andor.is_open() ) {
      logwrite( function, "ERROR no connection to camera" );
      return ERROR;
    }

    // Parse args if present
    //
    if ( !args.empty() ) {

      int hflip, vflip;

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // There can be only one arg (the requested EM gain)
      //
      if ( tokens.size() != 2 ) {
        logwrite( function, "ERROR expected <hflip> <vflip>" );
        return ERROR;
      }

      // Parse the gain from the token
      //
      try {
        hflip = std::stoi( tokens.at(0) );
        vflip = std::stoi( tokens.at(1) );
      }
      catch ( std::out_of_range &e ) {
        message.str(""); message << "ERROR reading arguments: " << e.what();
        error = ERROR;
      }
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR reading arguments: " << e.what();
        error = ERROR;
      }

      if (error==ERROR) logwrite( function, message.str() );

      // Set the flip parameters now
      //
      if (error!=ERROR ) error = this->andor.set_imflip( hflip, vflip );
    }

    // return the current image flip states
    //
    message.str(""); message << this->andor.camera_info.hflip << " " << this->andor.camera_info.vflip;
    retstring = message.str();

    logwrite( function, retstring );

    return error;
  }
  /***** Acam::Camera::imflip *************************************************/


  /***** Acam::Camera::imrot **************************************************/
  /**
   * @brief      set camera image rotation
   * @param[in]  args       optionally contains <rotdir> "cw" or "ccw"
   * @param[out] retstring  return string contains <hrot> <vrot>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::imrot( std::string args, std::string &retstring ) {
    std::string function = "Acam::Camera::imrot";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_IMROT;
      retstring.append( " [ <rotdir> ]\n" );
      retstring.append( "   Set CCD image rotation where <rotdir> is { none cw ccw }\n" );
      retstring.append( "   and \"cw\"   will rotate 90 degrees clockwise,\n" );
      retstring.append( "       \"ccw\"  will rotate 90 degrees counter-clockwise,\n" );
      retstring.append( "       \"none\" will set the rotation to none.\n" );
      retstring.append( "   If used in conjuction with \"" + ACAMD_IMFLIP + "\" the rotation will\n" );
      retstring.append( "   occur before the flip regardless of which order the commands are\n" );
      retstring.append( "   sent. 180 degree rotation can be achieved using the \"" + ACAMD_IMFLIP + "\"\n" );
      retstring.append( "   command by selecting both horizontal and vertical flipping.\n" );
      return HELP;
    }

    // Andor must be connected
    //
    if ( !this->andor.is_open() ) {
      logwrite( function, "ERROR no connection to camera" );
      return ERROR;
    }

    // Parse args if present
    //
    if ( !args.empty() ) {

      int rotdir;

      std::transform( args.begin(), args.end(), args.begin(), ::tolower );  // convert to lowercase

      if ( args == "none" ) rotdir = 0;
      else
      if ( args == "cw" )   rotdir = 1;
      else
      if ( args == "ccw" )  rotdir = 2;
      else {
        message.str(""); message << "ERROR bad arg " << args << ": expected { none cw ccw }";
        logwrite( function, message.str() );
        return ERROR;
      }

      // Set the rotation now
      //
      error = this->andor.set_imrot( rotdir );
    }

    return error;
  }
  /***** Acam::Camera::imrot **************************************************/


  /***** Acam::Camera::set_exptime ********************************************/
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

    // Ensure aquisition has stopped
    //
    long error = this->andor.abort_acquisition();

    // Set the exposure time on the Andor.
    // This will modify fval with actual exptime.
    //
    if (error==NO_ERROR) error = this->andor.set_exptime( fval );

    return error;
  }
  /***** Acam::Camera::set_exptime ********************************************/
  /**
   * @brief      set exposure time
   * @details    This overloaded version takes an rvalue reference to accept a
   *             temporary float used to call the other set_exptime function.
   *             Use this to set exptime with an rvalue instead of an lvalue.
   * @param[in]  fval  rvalue reference to exposure time
   * @return     ERROR | NO_ERROR
   */
  long Camera::set_exptime( float &&fval ) {
    float retval=fval;
    return set_exptime(retval);
  }
  /***** Acam::Camera::set_exptime ********************************************/


  /***** Acam::Camera::gain ***************************************************/
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
  long Camera::gain( std::string args, std::string &retstring ) {
    std::string function = "Acam::Camera::gain";
    std::stringstream message;
    long error = NO_ERROR;
    int gain = -999;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_GAIN;
      retstring.append( " [ <gain> ]\n" );
      retstring.append( "   Set or get CCD Gain.\n" );
      if ( this->andor.is_open() ) {
        int low, high;
        this->andor.get_emgain_range( low, high );
        retstring.append( "   Select <gain> = 1 for conventional or in range { " );
        retstring.append( std::to_string( low ) );
        retstring.append( " " );
        retstring.append( std::to_string( high ) );
        retstring.append( " } for EMCCD.\n" );
      }
      else {
        retstring.append( "   Open connection to camera to see gain range.\n" );
      }
      retstring.append( "   If <gain> is omitted then the current gain is returned.\n" );
      retstring.append( "   Output amplifier is automatically selected as conventional for unity gain,\n" );
      retstring.append( "   or EM for non-unity gain.\n" );
      return HELP;
    }

    // Andor must be connected
    //
    if ( !this->andor.is_open() ) {
      logwrite( function, "ERROR no connection to camera" );
      retstring="not_open";
      return ERROR;
    }

    // Parse args if present
    //
    if ( !args.empty() ) {

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // There can be only one arg (the requested EM gain)
      //
      if ( tokens.size() != 1 ) {
        logwrite( function, "ERROR too many arguments" );
        retstring="invalid_argument";
        return ERROR;
      }

      // Parse the gain from the token
      //
      try {
        gain = std::stoi( tokens.at(0) );
      }
      catch ( std::out_of_range &e ) {
        message.str(""); message << "ERROR setting gain: " << e.what();
        error = ERROR;
      }
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR setting gain: " << e.what();
        error = ERROR;
      }

      if (error==ERROR) logwrite( function, message.str() );

      // get min/max values for gain
      //
      int low, high;
      this->andor.get_emgain_range( low, high );

      // set gain and output amplifier as necessary
      //
      message.str("");
      if ( error==NO_ERROR && gain < 1 ) {
        message << "ERROR: gain " << gain << " outside range { 1, " << low << ":" << high << " }";
        error = ERROR;
      }
      else
      // gain of 1 uses Conventional Amplifier, no EM gain
      //
      if ( error==NO_ERROR && gain == 1 ) {
        error = this->andor.set_output_amplifier( Andor::AMPTYPE_CONV );
        if (error==NO_ERROR) this->andor.camera_info.gain = gain;
        else { message << "ERROR gain not set"; }
      }
      else
      // gain > 1 switches to EMCCD output amplifier
      // and sets the EM gain
      //
      if ( error==NO_ERROR && gain >= low && gain <= high ) {
        error = this->andor.set_output_amplifier( Andor::AMPTYPE_EMCCD );
        if (error==NO_ERROR) this->andor.set_emgain( gain );
        if (error==NO_ERROR) this->andor.camera_info.gain = gain;
        else { message << "ERROR gain not set"; }
      }
      else
      if ( error==NO_ERROR ) {
        message.str(""); message << "ERROR: gain " << gain << " outside range { 1, "
                                 << low << ":" << high << " }";
        error = ERROR;
      }
      if ( !message.str().empty() ) logwrite( function, message.str() );
    }

    // Regardless of setting the gain, always return it.
    //
    if ( this->andor.camera_info.gain < 1 ) {
      retstring = "ERR";
    }
    else {
      retstring = std::to_string( this->andor.camera_info.gain );
    }

    logwrite( function, retstring );

    return error;
  }
  /***** Acam::Camera::gain ***************************************************/
  /**
   * @brief      overloaded version accepts int and no return value
   * @param[in]  value  integer value of new gain
   * @return     ERROR | NO_ERROR
   */
  long Camera::gain( int value ) {
    std::string dontcare;
    return this->gain( std::to_string(value), dontcare );
  }
  /***** Acam::Camera::gain ***************************************************/


  /***** Acam::Camera::gain ***************************************************/
  /**
   * @brief      wrapper to get exposure time as a double
   * @return     double exposure time
   *
   */
  int Camera::gain() {
    std::string function = "Acam::Camera::gain";
    std::string svalue;
    int ivalue=0;
    this->gain( "", svalue );
    try {
      ivalue=std::stoi( svalue );
    }
    catch( std::out_of_range &e )     { logwrite( "Acam::Camera::gain", "ERROR "+std::string(e.what()) ); }
    catch( std::invalid_argument &e ) { logwrite( "Acam::Camera::gain", "ERROR "+std::string(e.what()) ); }
    return ivalue;
  }
  /***** Acam::Camera::gain ***************************************************/


  /***** Acam::Camera::speed **************************************************/
  /**
   * @brief      set or get the CCD clocking speeds
   * @param[in]  args       optionally contains new clocking speeds
   * @param[out] retstring  return string contains clocking speeds
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::speed( std::string args, std::string &retstring ) {
    std::string function = "Acam::Camera::speed";
    std::stringstream message;
    long error = NO_ERROR;
    float hori=-1, vert=-1;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_SPEED;
      retstring.append( " [ <hori> <vert> ]\n" );
      retstring.append( "   Set or get CCD clocking speeds for horizontal <hori> and vertical <vert>\n" );
      retstring.append( "   If speeds are omitted then the current speeds are returned.\n" );
      if ( this->andor.is_open() ) {
        retstring.append( "   Current amp type is " );
        retstring.append( ( this->andor.camera_info.amptype == Andor::AMPTYPE_EMCCD ? "EMCCD\n" : "conventional\n" ) );
        retstring.append( "   Select <hori> from {" );
        for ( const auto &hspeed : this->andor.camera_info.hsspeeds[ this->andor.camera_info.amptype] ) {
          retstring.append( " " );
          retstring.append( std::to_string( hspeed ) );
        }
        retstring.append( " }\n" );
        retstring.append( "   Select <vert> from {" );
        for ( const auto &vspeed : this->andor.camera_info.vsspeeds ) {
          retstring.append( " " );
          retstring.append( std::to_string( vspeed ) );
        }
        retstring.append( " }\n" );
        retstring.append( "   Units are MHz\n" );
      }
      else {
        retstring.append( "   Open connection to camera to see possible speeds.\n" );
      }
      return HELP;
    }

    // Andor must be connected
    //
    if ( !this->andor.is_open() ) {
      logwrite( function, "ERROR no connection to camera" );
      return ERROR;
    }

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

      if (error!=ERROR ) error = this->andor.set_hsspeed( hori );
      if (error!=ERROR ) error = this->andor.set_vsspeed( vert );
    }

    if ( ( this->andor.camera_info.hspeed < 0 ) ||
         ( this->andor.camera_info.vspeed < 0 ) ) {
      logwrite( function, "ERROR speeds not set" );
      error = ERROR;
    }

    retstring = std::to_string( this->andor.camera_info.hspeed ) + " "
              + std::to_string( this->andor.camera_info.vspeed );

    logwrite( function, retstring );

    return error;
  }
  /***** Acam::Camera::speed **************************************************/


  /***** Acam::Camera::temperature ********************************************/
  /**
   * @brief      set or get the camera temperature setpoint
   * @param[in]  args       optionally contains new setpoint
   * @param[out] retstring  return string contains <temp> <setpoint> <status>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::temperature( std::string args, std::string &retstring ) {
    std::string function = "Acam::Camera::temperature";
    std::stringstream message;
    long error = NO_ERROR;
    int temp = 999;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_TEMP;
      retstring.append( " [ <setpoint> ]\n" );
      retstring.append( "  Set or get camera temperature in integer degrees C,\n" );
      if ( this->andor.is_open() ) {
        retstring.append( "  where <setpoint> is in range { " );
        retstring.append( std::to_string( this->andor.camera_info.temp_min ) );
        retstring.append( "  " );
        retstring.append( std::to_string( this->andor.camera_info.temp_max ) );
        retstring.append( " }.\n" );
      }
      else {
        retstring.append( "  open connection to camera to see acceptable range.\n" );
      }
      retstring.append( "  If optional <setpoint> is provided then the camera setpoint is changed,\n" );
      retstring.append( "  else the current temperature, setpoint, and status are returned.\n" );
      retstring.append( "  Format of return value is <temp> <setpoint> <status>\n" );
      retstring.append( "  Camera cooling is turned on/off automatically, as needed.\n" );
      return HELP;
    }

    // Andor must be connected
    //
    if ( !this->andor.is_open() ) {
      logwrite( function, "ERROR no connection to camera" );
      return ERROR;
    }

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
        error = this->andor.set_temperature( temp );
      }
      catch ( std::out_of_range &e ) {
        message.str(""); message << "ERROR setting temperature: " << e.what();
        error = ERROR;
      }
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR setting temperature: " << e.what();
        error = ERROR;
      }
      catch ( std::bad_alloc &e ) {
        message.str(""); message << "ERROR setting temperature: " << e.what();
        error = ERROR;
      }
    }
    if (error==ERROR) logwrite( function, message.str() );

    // Regardless of setting the temperature, always read it.
    //
    error |= this->andor.get_temperature( temp );

    message.str(""); message << temp << " " << this->andor.camera_info.setpoint << " " << this->andor.camera_info.temp_status;
    logwrite( function, message.str() );

    retstring = message.str();

    return error;
  }
  /***** Acam::Camera::temperature ********************************************/


  /***** Acam::Camera::get_frame **********************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::get_frame() {
    unsigned long framepix = this->andor.camera_info.cols * this->andor.camera_info.rows;

    // Allocate storage if needed
    //
    if ( this->image_data != nullptr &&
         this->andor.camera_info.npix == framepix ) {
    }
    else
    if ( this->image_data != nullptr &&
         this->andor.camera_info.npix != framepix ) {
      delete [] this->image_data;
      this->image_data = nullptr;
    }

    if ( this->image_data == nullptr ) {
      this->image_data = (uint16_t*)new int16_t[ framepix ];
    }

    unsigned int ret = this->andor.get_last_frame( (uint16_t*)this->image_data );

    if ( ret == DRV_SUCCESS ) return NO_ERROR; else return ERROR;
  }
  /***** Acam::Camera::get_frame **********************************************/


  /***** Acam::Camera::write_frame ********************************************/
  /**
   * @brief      write the Andor image data to FITS file
   * @return     ERROR or NO_ERROR
   *
   */
  long Camera::write_frame( std::string source_file, std::string &outfile, const bool _tcs_online ) {
    std::string function = "Acam::Camera::write_frame";
    std::stringstream message;

    long error = NO_ERROR;

    // Nothing to do if not Andor image data
    //
    if ( andor.get_image_data() == nullptr ) {
      logwrite( function, "ERROR no image data available" );
      return ERROR;
    }

    fitsinfo.fits_name = outfile;
    fitsinfo.datatype = USHORT_IMG;
    fitsinfo.section_size = andor.camera_info.axes[0] * andor.camera_info.axes[1];

    fits_file.copy_info( fitsinfo );      // copy from fitsinfo to the fits_file

    error = fits_file.open_file();        // open the fits file for writing

    if ( !source_file.empty() ) {
      message.str(""); message << "[DEBUG] copy header from " << source_file; logwrite(function,message.str());
      if (error==NO_ERROR) error = fits_file.copy_header_from( source_file );
    }
    else {
      if (error==NO_ERROR) error = fits_file.create_header();  // create basic header
    }

//  fits_file.copy_header( wcs_in );      // if supplied, copy the header from the input file
//  fits_file.copy_header( "/home/developer/cshapiro/acam_skyinfo/skyheader.fits" );

/*
    std::unique_ptr<uint16_t[]> &data = andor.get_image_data();
    if ( data == nullptr ) {
      logwrite( function, "ERROR missing data buffer" );
      return ERROR;
    }
    fits_file.write_image( data.get() );  // write the image data
*/

    if (error==NO_ERROR) fits_file.write_image( andor.get_image_data() );  // write the image data

    fits_file.close_file();                           // close the file

    // This is the one extra call that is outside the normal workflow.
    // If emulator is enabled then the skysim generator will create a simulated
    // image. The image written above by fits_file.write_image() is used as
    // input to skysim because it contains the correct WCS headers, but will
    // ultimately be overwritten by the simulated image.
    //
    if ( andor.is_emulated() && _tcs_online ) andor.simulate_frame( fitsinfo.fits_name,
                                                                    false, // not multi-extension
                                                                    this->simsize );

    outfile = fitsinfo.fits_name;

    return error;
  }
  /***** Acam::Camera::write_frame ********************************************/


  /***** Acam::Camera::test_image *********************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::test_image( ) {
    std::string function = "Acam::Interface::test_image";
    std::stringstream message;
    long error = NO_ERROR;

    error = this->camera.andor.test();

    return error;
  }
  /***** Acam::Camera::test_image *********************************************/


  /***** Acam::Astrometry::initialize_python **********************************/
  /**
   * @brief      Initializes the Python astrometry module
   * @details    If daemonized, this should not be called by a parent process.
   *             Ensure this is called only by a child process.
   * @return     ERROR | NO_ERROR
   *
   */
  long Astrometry::initialize_python() {
    std::string function = "Acam::Astrometry::initialize_python";

    if ( ! py_instance.is_initialized() ) {
      logwrite( function, "ERROR could not initialize Python" );
      return ERROR;
    }

    PyGILState_STATE gstate = PyGILState_Ensure();   // Acquire the GIL

    PyObject* pModuleNameAstrometry = PyUnicode_FromString( PYTHON_ASTROMETRY_MODULE );
    PyObject* pModuleNameQuality    = PyUnicode_FromString( PYTHON_IMAGEQUALITY_MODULE );

    if ( !pModuleNameAstrometry || !pModuleNameQuality ) {
      logwrite( function, "ERROR could not create module name strings" );
      py_instance.print_python_error( function );
      Py_XDECREF( pModuleNameAstrometry );
      Py_XDECREF( pModuleNameQuality );
      PyGILState_Release(gstate);                    // Release the GIL
      return ERROR;
    }

    this->pAstrometryModule = PyImport_Import( pModuleNameAstrometry );
    this->pQualityModule    = PyImport_Import( pModuleNameQuality );

    if ( this->pAstrometryModule == nullptr || this->pQualityModule == nullptr ) {
      std::stringstream message;
      message << "ERROR could not import Python module(s):";
      if ( this->pAstrometryModule == nullptr ) message << " " << PYTHON_ASTROMETRY_MODULE;
      if ( this->pQualityModule == nullptr )    message << " " << PYTHON_IMAGEQUALITY_MODULE;
      logwrite( function, message.str() );
      py_instance.print_python_error( function );
      Py_XDECREF( pModuleNameAstrometry );
      Py_XDECREF( pModuleNameQuality );
      PyGILState_Release(gstate);                    // Release the GIL
      return ERROR;
    }

    PyGILState_Release(gstate);                      // Release the GIL

    this->python_initialized = true;

    logwrite( function, "initialized Python astrometry module" );

    return NO_ERROR;
  }
  /***** Acam::Astrometry::initialize_python **********************************/


  /***** Acam::Astrometry::image_quality **************************************/
  /**
   * @brief      call the Python astrometry image_quality function
   * @return     ERROR or NO_ERROR
   *
   * This should be called only after a solve() and needs to be called only once
   * for each solve.
   *
   */
  long Astrometry::image_quality( ) {
    std::string function = "Acam::Astrometry::image_quality";
    std::stringstream message;

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return ERROR;
    }

    if ( this->pQualityModule == nullptr ) {
      logwrite( function, "ERROR: Python image_quality module not imported" );
      return ERROR;
    }

    PyGILState_STATE gstate = PyGILState_Ensure();   // Acquire the GIL

    // Call the Python image_quality function here
    //
    PyObject* pFunction = PyObject_GetAttrString( this->pQualityModule, PYTHON_IMAGEQUALITY_FUNCTION );

    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR: Python image_quality function not callable" );
      py_instance.print_python_error( function );
      Py_XDECREF( pFunction );
      PyGILState_Release( gstate );                  // release the GIL before returning
      return ERROR;
    }

    PyObject* pReturn = PyObject_CallNoArgs( pFunction );
    Py_DECREF( pFunction );

//  "airmass" can be passed as an keyword. So can "EXPOSURE_TIME" but as a rule, supply neither.
//  Instead, these will both come from the FITS header.
//
//  PyObject *kwargs     = PyDict_New(); PyDict_SetItemString( kwargs, "airmass", PyLong_FromLong(1) );
//  PyObject *pEmptyArgs = PyTuple_New(0);
//  PyObject *pReturn    = PyObject_Call( pFunction, pEmptyArgs, kwargs );

    // Check the return values from Python here
    //
    if ( !pReturn ) {
      logwrite( function, "ERROR calling Python image_quality" );
      py_instance.print_python_error( function );
      Py_XDECREF( pReturn );
      PyGILState_Release( gstate );                  // release the GIL before returning
      return ERROR;
    }

    // Expected a Dictionary
    //
    if ( ! PyDict_Check( pReturn ) ) {
      logwrite( function, "ERROR: Python image_quality function did not return expected dictionary" );
      py_instance.print_python_error( function );
      Py_DECREF( pReturn );
      PyGILState_Release( gstate );                  // release the GIL before returning
      return ERROR;
    }

    // Get the error from the returned dictionary keyword
    //
    PyObject *_error          = PyDict_GetItemString( pReturn, "ERROR" );
    if ( !_error ) {
      logwrite( function, "ERROR: Python image_quality function did not return 'ERROR' key" );
      py_instance.print_python_error( function );
      Py_DECREF( pReturn );
      PyGILState_Release( gstate );                  // release the GIL before returning
      return ERROR;
    }
    const char *err           = PyUnicode_AsUTF8( _error );
    std::string errstr(err);

    // If there is any error then report it and do not proceed
    //
    if ( ! errstr.empty() ) {
      this->seeing            = NAN;
      this->seeing_std        = NAN;
      this->seeing_zen        = NAN;
      this->extinction        = NAN;
      this->extinction_std    = NAN;
      this->background        = NAN;
      this->background_std    = NAN;
      message.str(""); message << "ERROR from Python image_quality: " << errstr;
      logwrite( function, message.str() );
      Py_DECREF( pReturn );
      PyGILState_Release( gstate );                  // release the GIL before returning
      return ERROR;
    }

    // Get the values out of the returned dictionary
    // (GetItemString returns a borrowed reference, no need to DECREF)
    //
    PyObject *_seeing         = PyDict_GetItemString( pReturn, "seeing" );
    PyObject *_seeing_std     = PyDict_GetItemString( pReturn, "seeing_std" );
    PyObject *_seeing_zen     = PyDict_GetItemString( pReturn, "seeing_zen" );
    PyObject *_extinction     = PyDict_GetItemString( pReturn, "extinction" );
    PyObject *_extinction_std = PyDict_GetItemString( pReturn, "extinction_std" );
    PyObject *_background     = PyDict_GetItemString( pReturn, "background" );
    PyObject *_background_std = PyDict_GetItemString( pReturn, "background_std" );

    // Store them in the class
    //
    this->seeing              = PyFloat_AsDouble( _seeing );
    this->seeing_std          = PyFloat_AsDouble( _seeing_std);
    this->seeing_zen          = PyFloat_AsDouble( _seeing_zen );
    this->extinction          = PyFloat_AsDouble( _extinction );
    this->extinction_std      = PyFloat_AsDouble( _extinction_std );
    this->background          = PyFloat_AsDouble( _background );
    this->background_std      = PyFloat_AsDouble( _background_std );

    Py_DECREF( pReturn );
    PyGILState_Release( gstate );                    // release the GIL before returning

    message.str("");
    message << " seeing=" << this->seeing
            << " seeing_std=" << this->seeing_std
            << " seeing_zen=" << this->seeing_zen
            << " extinction=" << this->extinction
            << " extinction_std=" << this->extinction_std
            << " background=" << this->background
            << " background_std=" << this->background_std;
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Acam::Astrometry::image_quality **************************************/


  /***** Acam::Astrometry::solve **********************************************/
  /**
   * @brief      call the Python astrometry solver
   * @details    This function is overloaded.
   *             This version accepts no command-line solver args.
   * @param[in]  imagename_in   fits filename to give to the solver
   *
   */
  long Astrometry::solve( std::string imagename_in ) {
    std::vector<std::string> no_solverargs;
    return this->solve( imagename_in, no_solverargs );
  }
  /***** Acam::Astrometry::solve **********************************************/


  /***** Acam::Astrometry::solve **********************************************/
  /**
   * @brief      call the Python astrometry solver
   * @details    This function is overloaded.
   *             This version accepts an optional vector of solver args.
   * @param[in]  imagename_in   fits filename to give to the solver
   * @param[in]  solverargs_in  optional command-line solver args
   *
   */
  long Astrometry::solve( std::string imagename_in, std::vector<std::string> solverargs_in ) {
    std::string function = "Acam::Astrometry::solve";
    std::stringstream message;

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return ERROR;
    }

    if ( this->pAstrometryModule == nullptr ) {
      logwrite( function, "ERROR: Python astrometry module not imported" );
      return ERROR;
    }

    // Can't proceed unless there is an imagename
    //
    if ( imagename_in.empty() ) {
      logwrite( function, "ERROR: imagename cannot be empty" );
      return ERROR;
    }

    PyGILState_STATE gstate = PyGILState_Ensure();   // Acquire the GIL

    // There is always at minimum acquire={true|false}
    // and other optional key=val pairs can be added to a space-delimited string.
    //
    PyObject* pKeywords = PyDict_New();
    PyDict_SetItemString( pKeywords, "acquire", this->isacquire ? Py_True : Py_False );

    // The class solver args set by the config file are persistent, while
    // any solver args passed in are one-time-use-only; they are not saved
    // to the class. Before calling the solver, concatenate these two vectors
    // of solver args.
    //
    std::vector<std::string> _solver_args;
    _solver_args.reserve( this->solver_args.size() + solverargs_in.size() );
    _solver_args.insert( _solver_args.end(), this->solver_args.begin(), this->solver_args.end() );
    _solver_args.insert( _solver_args.end(), solverargs_in.begin(), solverargs_in.end() );

    if ( !_solver_args.empty() ) {       // only do this if the arglist is not empty

      // Loop through each "key=val" pair
      //
      try {
        for ( size_t pairn=0; pairn < _solver_args.size(); pairn++ ) {

          // Tokenize each argpair on "=" to separate the key and value
          //
          std::vector<std::string> keyval;
          Tokenize( _solver_args.at(pairn), keyval, "=" );

          // Add the arg pair to the keywords list
          // The key object is created from a const char pointer, and
          // the value must be a PyObject so that takes a little more work,
          // which is done in pyobj_from_string().
          //
          const char* pkeyname = keyval.at(0).c_str();
          PyObject* pvalue;
          this->pyobj_from_string( keyval.at(1), &pvalue );
          PyDict_SetItemString( pKeywords, pkeyname, pvalue );  // takes ownership of pvalue
        }
      }
      catch( std::out_of_range const& ) {
        logwrite( function, "ERROR: out of range parsing key=value pair from arglist" );
        Py_XDECREF( pKeywords );
        PyGILState_Release( gstate );                // release the GIL before returning
        return ERROR;
      }
      catch( ... ) {
        logwrite( function, "ERROR unknown exception parsing key=value pair from arglist" );
        Py_XDECREF( pKeywords );
        return ERROR;
        PyGILState_Release( gstate );                // release the GIL before returning
      }
    }

    double t0=0, t1=0;

    PyObject* pFunction = PyObject_GetAttrString( this->pAstrometryModule, PYTHON_ASTROMETRY_FUNCTION );

    const char* imagename = imagename_in.c_str();

    PyObject* pArgList = Py_BuildValue( "(s)", imagename );

    // Call the Python astrometry function here
    //
    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR: Python astrometry function not callable" );
      py_instance.print_python_error( function );
      Py_DECREF( pArgList );
      Py_XDECREF( pFunction );
      Py_XDECREF( pKeywords );
      PyGILState_Release( gstate );                  // release the GIL before returning
      return ERROR;
    }

    t0=get_clock_time();

    PyObject* pReturn = PyObject_Call( pFunction, pArgList, pKeywords );

    t1=get_clock_time();

    Py_DECREF( pFunction );
    Py_DECREF( pArgList );
    Py_DECREF( pKeywords );

//#ifdef LOGLEVEL_DEBUG
//    message.str(""); message << "[DEBUG] Python call time " << (t1-t0) << " sec";
//    logwrite( function, message.str() );
//#endif

    // Check the return values from Python here
    //
    if ( !pReturn ) {
      logwrite( function, "ERROR calling Python astrometry solver" );
      py_instance.print_python_error( function );
      Py_XDECREF( pReturn );
      PyGILState_Release( gstate );                  // release the GIL before returning
      return ERROR;
    }

    // Expected a dictionary
    //
    if ( ! PyDict_Check( pReturn ) ) {
      logwrite( function, "ERROR: did not return expected dictionary" );
      Py_XDECREF( pReturn );
      PyGILState_Release( gstate );                  // release the GIL before returning
      return ERROR;
    }

    // get the needed values from the returned dictionary
    // (GetItemString returns a borrowed reference, no need to DECREF)
    //
    PyObject* result_   = PyDict_GetItemString( pReturn, "result" );
    PyObject* ra_       = PyDict_GetItemString( pReturn, "solveRA" );
    PyObject* dec_      = PyDict_GetItemString( pReturn, "solveDEC" );
    PyObject* pa_       = PyDict_GetItemString( pReturn, "solveANG" );
    PyObject* matches_  = PyDict_GetItemString( pReturn, "matches" );
    PyObject* rmsarcsec_= PyDict_GetItemString( pReturn, "rms_arcsec" );

    // store them in the class
    //
    const char* cresult = PyUnicode_AsUTF8( result_ );
    this->result.assign( cresult );

    this->ra  = PyFloat_Check( ra_ )  ? PyFloat_AsDouble( ra_ )  : NAN;
    this->dec = PyFloat_Check( dec_ ) ? PyFloat_AsDouble( dec_ ) : NAN;
    this->pa  = PyFloat_Check( pa_ )  ? PyFloat_AsDouble( pa_ )  : NAN;
    this->matches   = PyLong_Check( matches_ )  ? PyLong_AsLong( matches_ )  : -1L;
    this->rmsarcsec = PyFloat_Check( rmsarcsec_ )  ? PyFloat_AsDouble( rmsarcsec_ )  : NAN;

    PyGILState_Release( gstate );                    // release the GIL before returning

    message.str(""); message << "result=" << this->result
                             << " NMATCH="<< this->matches
                             << std::fixed << std::setprecision(6)
                             << " RA="    << this->ra
                             << " DEC="   << this->dec
                             << " PA="    << this->pa
                             << " RMS="   << this->rmsarcsec
                             << " solve time=" << t1-t0 << " sec";
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Acam::Astrometry::solve **********************************************/


  /***** Acam::Astrometry::pyobj_from_string **********************************/
  /**
   * @brief      create a PyObject of the correct type from a std::string
   * @param[in]  str_in  the input string
   * @param[out] obj     pointer to PyObject pointer (must be created outside this function)
   *
   */
  void Astrometry::pyobj_from_string( std::string str_in, PyObject** obj ) {
    std::size_t pos(0);

    // If the entire string is either exactly {true,false,True,False} then it's boolean
    //
    if ( str_in=="true"  || str_in=="True"  ) { *obj = Py_True;  return; }
    if ( str_in=="false" || str_in=="False" ) { *obj = Py_False; return; }

    // skip whitespaces
    //
    pos = str_in.find_first_not_of(' ');

    // check the significand, skip the sign if it exists
    //
    if ( str_in[pos] == '+' || str_in[pos] == '-' ) ++pos;

    // count the number of digits and number of decimal points
    //
    int n_nm, n_pt;
    for ( n_nm = 0, n_pt = 0; std::isdigit(str_in[pos]) || str_in[pos] == '.'; ++pos ) {
      str_in[pos] == '.' ? ++n_pt : ++n_nm;
    }
    if (n_pt>1 || n_nm<1 || pos<str_in.size()){   // no more than one point, no numbers, or a non-digit character
      const char* cstr = str_in.c_str();
      *obj = PyUnicode_FromString( cstr );
      return;
    }

    // skip the trailing whitespaces
    while (str_in[pos] == ' ') {
      ++pos;
    }

    try {
      if (pos == str_in.size()) {
        if (str_in.find(".") == std::string::npos) {  // all numbers and no decimals, it's an integer
          long num = std::stol( str_in );
          *obj = PyLong_FromLong( num );
          return;
        }
        else {                                        // otherwise numbers with a decimal, it's a float
          double num = std::stod( str_in );
          *obj = PyFloat_FromDouble( num );
          return;
        }
      }
      else {                                          // lastly, must be a string
        const char* cstr = str_in.c_str();
        *obj = PyUnicode_FromString( cstr );
        return;
      }
    }
    catch ( std::invalid_argument & ) {
      *obj = PyUnicode_FromString( "ERROR" );
      return;
    }
    catch ( std::out_of_range & ) {
      *obj = PyUnicode_FromString( "ERROR" );
      return;
    }

    // if not caught above then it's an error
    //
    *obj = PyUnicode_FromString( "ERROR" );
    return;
  }
  /***** Acam::Astrometry::pyobj_from_string **********************************/


  /***** Acam::Interface::make_telemetry_message ******************************/
  /**
   * @brief      assembles a telemetry message
   * @details    This creates a JSON message for my telemetry info, then serializes
   *             it into a std::string ready to be sent over a socket so that
   *             outside clients can ask for my telemetry.
   * @param[out] retstring  string containing the serialization of the JSON message
   *
   */
  void Interface::make_telemetry_message( std::string &retstring ) {

    // assemble the telemetry into a json message
    // Set a messagetype keyword to indicate what kind of message this is.
    //
    nlohmann::json jmessage;
    jmessage["messagetype"] = "acaminfo";

    // don't put data in the message if there's no camera connection
    //
    if ( this->isopen("camera") ) {
      int ccdtemp=99;
      this->camera.andor.get_temperature( ccdtemp );
      jmessage["TANDOR_ACAM"] = static_cast<float>(ccdtemp);  // the database wants floats
    }

    retstring = jmessage.dump();  // serialize the json message into retstring

    retstring.append(JEOF);       // append the JSON message terminator

    return;
  }
  /***** Acam::Interface::make_telemetry_message ******************************/


  /***** Acam::Interface::get_external_telemetry ******************************/
  /**
   * @brief      collect telemetry from another daemon
   * @details    This is used for any telemetry that I need to collect from
   *             another daemon. Send the command "sendtelem" to the daemon, which
   *             will respond with a JSON message. The daemon(s) to contact
   *             are configured with the TELEM_PROVIDER key in the config file.
   *
   */
  void Interface::get_external_telemetry() {

    // Instantiate a client to communicate with each daemon,
    // constructed with no name, newline termination on command writes,
    // and JEOF termination on reply reads.
    //
    Common::DaemonClient jclient("", "\n", JEOF );

    // Loop through each configured telemetry provider, which is a map of
    // ports indexed by daemon name, both of which are used to update
    // the jclient object.
    //
    // Send the command "sendtelem" to each daemon and read back the reply into
    // retstring, which will be the serialized JSON telemetry message.
    //
    // handle_json_message() will parse the reply and set the FITS header
    // keys in the telemkeys database.
    //
    std::string retstring;
    for ( const auto &[name, port] : this->telemetry_providers ) {
      jclient.set_name(name);
      jclient.set_port(port);
      jclient.connect();
      jclient.command("sendtelem", retstring);
      jclient.disconnect();
      handle_json_message(retstring);
    }
    return;
  }
  /***** Acam::Interface::get_external_telemetry ******************************/


  /***** Acam::Interface::handle_json_message *********************************/
  /**
   * @brief      parses incoming telemetry messages
   * @details    Requesting telemetry from another daemon returns a serialized
   *             JSON message which needs to be passed in here to parse it.
   * @param[in]  message_in  incoming serialized JSON message (as a string)
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::handle_json_message( std::string message_in ) {
    const std::string function="Acam::Interface::handle_json_message";
    std::stringstream message;

    try {
      nlohmann::json jmessage = nlohmann::json::parse( message_in );
      std::string messagetype;

      // jmessage must not contain key "error" and must contain key "messagetype"
      //
      if ( !jmessage.contains("error") ) {
        if ( jmessage.contains("messagetype") && jmessage["messagetype"].is_string() ) {
          messagetype = jmessage["messagetype"];
        }
        else {
          logwrite( function, "ERROR received JSON message with missing or invalid messagetype" );
          return ERROR;
        }
      }
      else {
        logwrite( function, "ERROR in JSON message" );
        return ERROR;
      }

      // no errors, so disseminate the message contents based on the message type
      //
      if ( messagetype == "tcsinfo" ) {
        this->database.add_from_json<double>( jmessage, "CASANGLE" );
        this->database.add_from_json<std::string>( jmessage, "TELRA", "RAtel" );
        this->database.add_from_json<std::string>( jmessage, "TELDEC", "DECLtel" );
        this->database.add_from_json<double>( jmessage, "AZ" );
        this->database.add_from_json<double>( jmessage, "TELFOCUS", "focus" );
        this->database.add_from_json<double>( jmessage, "AIRMASS" );
      }
      else
      if ( messagetype == "targetinfo" ) {
        this->database.add_from_json<int>( jmessage, "OBS_ID" );
        this->database.add_from_json<std::string>( jmessage, "NAME" );
        this->database.add_from_json<std::string>( jmessage, "POINTMODE" );
        this->database.add_from_json<std::string>( jmessage, "RA" );
        this->database.add_from_json<std::string>( jmessage, "DECL" );
      }
      else
      if ( messagetype == "test" ) {
      }
      else {
        message.str(""); message << "ERROR received unhandled JSON message type \"" << messagetype << "\"";
        logwrite( function, message.str() );
        return ERROR;
      }
    }
    catch ( const nlohmann::json::parse_error &e ) {
      message.str(""); message << "ERROR json exception parsing message: " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR parsing message: " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Acam::Interface::handle_json_message *********************************/


  /***** Acam::Interface::initialize_python_objects ***************************/
  /**
   * @brief      provides interface to initialize Python objects in the class
   * @details    This provides an interface (to the Acam Server) to initialize
   *             any Python modules in objects in the class. Allows Daemons to
   *             ensure Python initialization is done by the child process.
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::initialize_python_objects() {
    return( this->astrometry.initialize_python() );
  }
  /***** Acam::Interface::initialize_python_objects ***************************/


  /***** Acam::Interface::configure_interface *********************************/
  /**
   * @brief      configure the interface from the .cfg file
   * @details    this function can be called at any time, e.g. from HUP or a command
   * @param[in]  config  reference to Config object
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::configure_interface( Config &config ) {
    std::string function = "Acam::Interface::configure_interface";
    std::stringstream message;
    int applied=0;
    long error = NO_ERROR;

    if ( config.read_config(config) != NO_ERROR) {          // read configuration file specified on command line
      logwrite(function, "ERROR: unable to read config file");
      return ERROR;
    }

    this->astrometry.solver_args.clear();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < config.n_entries; entry++) {

      if ( starts_with( config.param[entry], "SOLVER_ARGS" ) ) {
        // tokenize each solverargs entry to check that it is of the format key=value
        //
        std::vector<std::string> tokens;
        int size = Tokenize( config.arg[entry], tokens, "=" );
        if (size==0) continue; else  // allows an empty entry but if anything is there then it has to be right
        if (size != 2) {
          message.str(""); message << "ERROR: bad entry for SOLVER_ARGS: " << config.arg[entry] << ": expected (key=value)";
          logwrite(function, message.str());
          error |= ERROR;
          continue;
        }
        else {
          try {
            this->astrometry.solver_args.push_back( config.arg.at(entry) );
            message.str(""); message << "CONFIG:" << config.param[entry] << "=" << config.arg[entry];
            logwrite( function, message.str() );
            this->async.enqueue( message.str() );
            applied++;
          }
          catch(std::out_of_range &) {  // should be impossible but here for safety
            logwrite(function, "ERROR configuring SOLVER_ARGS: requested tokens out of range");
            error |= ERROR;
          }
        }
      }

      if ( starts_with( config.param[entry], "PIXSCALE" ) ) {
        try {
          this->camera.andor.camera_info.pixel_scale = std::stod( config.arg[entry] );
        }
        catch( std::invalid_argument &e ) {
          message.str(""); message << "ERROR invalid pixel scale: " << e.what();
          logwrite( function, message.str() );
          error |= ERROR;
        }
        catch( std::out_of_range &e ) {
          message.str(""); message << "ERROR invalid pixel scale: " << e.what();
          logwrite( function, message.str() );
          error |= ERROR;
        }
      }

      if ( starts_with( config.param[entry], "PUSH_GUIDER_SETTINGS" ) ) {
        this->guide_manager.set_push_settings( config.arg[entry] );
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

      if ( starts_with( config.param[entry], "PUSH_GUIDER_IMAGE" ) ) {
        this->guide_manager.set_push_image( config.arg[entry] );
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

      if ( starts_with( config.param[entry], "PUSH_GUIDER_MESSAGE" ) ) {
        this->guide_manager.set_push_message( config.arg[entry] );
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

      if ( starts_with( config.param[entry], "ANDOR_SN" ) ) {

        // When the SN is set to "emulate" (case-insensitive) then this enables the Andor emulator
        //
        if ( caseCompareString( config.arg[entry], "emulate" ) ) {
          this->camera.andor.andor_emulator( true );
          applied++;
        }
        else {
          this->camera.andor.andor_emulator( false );
          try {
            this->camera.andor.camera_info.serial_number=std::stoi(config.arg[entry]);
            applied++;
          }
          catch ( const std::exception &e ) {
            message.str(""); message << "ERROR parsing ANDOR_SN: " << e.what();
            logwrite( function, message.str() );
            error=ERROR;
          }
        }
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
      }

      if ( starts_with( config.param[entry], "TCSD_PORT" ) ) {
        int port;
        try {
          port = std::stoi( config.arg[entry] );
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR bad TCSD_PORT " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR bad TCSD_PORT " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        }
        this->tcsd.client.port =  port;
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

      if ( config.param[entry] == "ACQUIRE_TIMEOUT" ) {
        double to;
        try {
          to = std::stod( config.arg[entry] );
        } catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_TIMEOUT " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        } catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_TIMEOUT " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        }
        if ( this->target.set_timeout( to ) != NO_ERROR ) {
          message.str(""); message << "ERROR bad ACQUIRE_TIMEOUT \"" << config.param[entry] << "\" must be >= 0";
          logwrite( function, message.str() );
          return ERROR;
        }
        applied++;
      }

      if ( config.param[entry] == "ACQUIRE_RETRYS" ) {
        int retrys;
        try {
          retrys = std::stoi( config.arg[entry] );
        } catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_RETRYS " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        } catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_RETRYS " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        }
        this->target.set_max_attempts( retrys );
        applied++;
      }

      if ( config.param[entry] == "ACQUIRE_OFFSET_THRESHOLD" ) {
        double threshold;
        try {
          threshold = std::stod( config.arg[entry] );
        } catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_OFFSET_THRESHOLD " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        } catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_OFFSET_THRESHOLD " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        }
        if ( this->target.set_offset_threshold( threshold ) != NO_ERROR ) {
          message.str(""); message << "ERROR bad ACQUIRE_OFFSET_THRESHOLD \"" << config.param[entry] << "\" must be >= 0";
          logwrite( function, message.str() );
          return ERROR;
        }
        applied++;
      }

      if ( config.param[entry] == "ACQUIRE_TCS_MAX_OFFSET" ) {
        double offset;
        try {
          offset = std::stod( config.arg[entry] );
        } catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_TCS_MAX_OFFSET " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        } catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_TCS_MAX_OFFSET " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        }
        if ( this->target.set_tcs_max_offset( offset ) != NO_ERROR ) {
          message.str(""); message << "ERROR bad ACQUIRE_TCS_MAX_OFFSET \"" << config.param[entry] << "\" must be >= 0";
          logwrite( function, message.str() );
          return ERROR;
        }
        applied++;
      }

      if ( config.param[entry] == "ACQUIRE_MIN_REPEAT" ) {
        int repeat;
        try {
          repeat = std::stoi( config.arg[entry] );
        } catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_MIN_REPEAT " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        } catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR bad ACQUIRE_MIN_REPEAT " << config.param[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        }
        this->target.set_min_repeat( repeat );
        applied++;
      }

      if ( config.param[entry] == "SKYSIM_IMAGE_SIZE" ) {
        try {
          this->camera.set_simsize( std::stoi( config.arg[entry] ) );
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR invalid SKYSIM_IMAGE_SIZE " << config.arg[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR invalid SKYSIM_IMAGE_SIZE " << config.arg[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
        message.str(""); message << "ACAMD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        applied++;
      }

    }
    message.str(""); message << "applied " << applied << " configuration lines to the acam interface";
    logwrite(function, message.str());
    return error;
  }
  /***** Acam::Interface::configure_interface *********************************/


  /***** Acam::Interface::open ************************************************/
  /**
   * @brief      wrapper to open all or specified acam external components
   * @param[in]  args       string containing 0 or more args specifying which component to open
   * @param[out] retstring  return string contains true | false | <help>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::open( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::open";
    std::stringstream message;
    long error = NO_ERROR;
    std::vector<std::string> arglist;
    std::string component, camarg;

    // No args opens everything (motion and camera)...
    //
    if ( args.empty() ) {
      component = "all";
      camarg.clear();
    }
    else if ( args == "?" ) {
      retstring = ACAMD_OPEN;
      retstring.append( " [ [motion] [camera [sn]] ]\n" );
      retstring.append( "  Open connections to all devices (by default).\n" );
      retstring.append( "  Optionally indicate motion | camera to open only the indicated component.\n" );
      retstring.append( "  The camera component can take an optional serial number to specify which\n" );
      retstring.append( "  Andor to open, which overrides \"ANDOR_SN="
                           +std::to_string(this->camera.andor.camera_info.serial_number)
                           +"\" from config file.");
      return HELP;
    }
    else { // ...otherwise look at the arg(s):

      std::transform( args.begin(), args.end(), args.begin(), ::tolower );  // convert to lowercase

      Tokenize( args, arglist, " " );

      // args can be [ [motion] [camera [args]] ]
      //
      int ccount=0;
      for ( size_t i=0; i < arglist.size(); i++ ) {
        size_t next = i+1;
        // If one arg is motion then any following args must be camera...
        //
        if ( arglist[i] == "motion" ) {
          component = arglist[i]; ccount++;
          if ( next < arglist.size() && arglist[next] != "camera" ) {
            message.str(""); message << "ERROR: unrecognized arg \"" << arglist[next]
                                     << "\". Expected { [ [motion] [camera [args]] ] }";
            logwrite( function, message.str() );
            retstring="invalid_argument";
            return ERROR;
          }
        }
        // If one arg is camera then the following args are passed to the camera
        //
        if ( arglist[i] == "camera" ) {
          component = arglist[i]; ccount++;
          if ( next < arglist.size() && arglist[next] != "motion" ) {
            camarg.append( arglist[ next ] );
          }
        }
      }

      if ( ccount == 2 ) component = "all";

      if ( component.empty() ) {
        message.str(""); message << "ERROR: unrecognized arg \"" << args
                                 << "\". Expected { [ [motion] [camera [args]] ] }";
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return ERROR;
      }
    }

    if ( component != "all" && component != "motion" && component != "camera" ) {
      message.str(""); message << "ERROR: unrecognized component \"" << component << "\". "
                               << "Expected { motion | camera }";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    // Open motion
    //
    if ( component == "all" || component == "motion" ) {
      // open motion
      error |= this->motion.open();
      // are actuators homed?
      std::string homestr;
      error |= this->motion.is_home("", homestr);
      // home them if needed
      if ( homestr == "false" ) error |= this->motion.home("", homestr);
    }

    // Open camera
    //
    if ( component == "all" || component == "camera" ) {
      // If serial number not specified as an arg then open the s/n specified
      // in the config file.
      //
      if ( camarg.empty() ) camarg = std::to_string( this->camera.andor.camera_info.serial_number );

      if ( this->camera.open( camarg ) == NO_ERROR ) {
        error |= this->framegrab( "start", retstring );   // start frame grabbing if open succeeds
      }
      else error=ERROR;

      // Initialize the database class once the camera is open
      //
      if ( error == NO_ERROR ) {
        this->database.initialize_class( this->db_info );
      }

      this->guider_settings_control("", retstring);
    }

    if ( error != NO_ERROR ) logwrite( function, "ERROR: one or more components failed to open" );

    bool state = this->isopen( component );

    retstring = ( state ? "true" : "false" );

    return error;
  }
  /***** Acam::Interface::open ************************************************/


  /***** Acam::Interface::isopen **********************************************/
  /**
   * @brief      wrapper for acam hardware components to check if connection open
   * @param[in]  component  optional string contains which component to check { camera | motion }
   * @param[out] state      reference to bool to indicate open {true} or not {false}
   * @param[out] retstring  reference to return string
   * @return     ERROR | NO_ERROR | HELP
   *
   * This function is overloaded
   *
   */
  long Interface::isopen( std::string component, bool &state, std::string &retstring ) {
    std::string function = "Acam::Interface::isopen";
    std::stringstream message;

    // Help
    //
    if ( component == "?" ) {
      retstring = ACAMD_ISOPEN;
      retstring.append( " [ camera | motion ]\n" );
      retstring.append( "  Returns the open state of the optionally named component.\n" );
      retstring.append( "  If no arg supplied then both components are checked, returning true\n" );
      retstring.append( "  only if both are open.\n" );
      return HELP;
    }

    if ( component.empty() ) {  // No component checks everything (motion and camera)...
      component = "all";
    }
    else {
      std::transform( component.begin(), component.end(), component.begin(), ::tolower );
    }

    if ( component != "all" && component != "motion" && component != "camera" ) {
      message.str(""); message << "ERROR: unrecognized component \"" << component << "\". "
                               << "Expected { motion | camera }";
      logwrite( function, message.str() );
      state = false;
      retstring="invalid_argument";
      return ERROR;
    }

    if ( component == "all" ) {
      state  = this->motion.is_open();
      state &= this->camera.andor.is_open();
    }
    else
    if ( component == "camera" ) {
      state  = this->camera.andor.is_open();
    }
    else
    if ( component == "motion" ) {
      state  = this->motion.is_open();
    }

    retstring = ( state ? "true" : "false" );

    return NO_ERROR;
  }
  /***** Acam::Interface::isopen **********************************************/


  /***** Acam::Interface::isopen **********************************************/
  /**
   * @brief      returns simple boolean for open state of specified component
   * @param[in]  component  optional string contains which component to check { camera | motion }
   * @return     true | false
   *
   * This function is overloaded
   *
   */
  bool Interface::isopen( std::string component ) {
    bool state=false;
    std::string dontcare;
    long error = this->isopen( component, state, dontcare );
    if (error==NO_ERROR) return state; else return false;
  }
  /***** Acam::Interface::isopen **********************************************/


  /***** Acam::Interface::close ***********************************************/
  /**
   * @brief      wrapper for all acam hardware components
   * @param[in]  component  optionally provide the component name to close { camera | motion }
   * @param[out] help       contains return string for help
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::close( std::string component, std::string &help ) {
    std::string function = "Acam::Interface::close";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( component == "?" ) {
      help = ACAMD_CLOSE;
      help.append( " [ camera | motion ]\n" );
      help.append( "  optionally supply the component name to close only that component\n" );
      help.append( "  closes all components if no arg supplied\n" );
      return HELP;
    }

    if ( component.empty() ) {  // No component closes everything (motion and camera)...
      component = "all";
    }
    else {
      std::transform( component.begin(), component.end(), component.begin(), ::tolower );
    }

    if ( component != "all" && component != "motion" && component != "camera" ) {
      message.str(""); message << "ERROR: unrecognized component \"" << component << "\". "
                               << "Expected { motion | camera }";
      logwrite( function, message.str() );
      return ERROR;
    }

    // close motion
    //
    if ( component == "all" || component == "motion" ) {
//    error |= this->motion.send_command( "close" );  // just needed for the emulator
      error |= this->motion.close();
    }

    // close camera
    //
    if ( component == "all" || component == "camera" ) {
      // disable frame grabbing
      // this won't return until framegrabbing has stopped (or timeout)
      //
      std::string dontcare;
      error |= this->framegrab( "stop", dontcare );

      // close the Andor
      //
      error |= this->camera.close();

      // close the database connection
      // this may throw an exception
      //
      try { this->database.close(); }
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR " << e.what();
        logwrite( function, message.str() );
        return ERROR;
      }
    }

    return error;
  }
  /***** Acam::Interface::close ***********************************************/


  /***** Acam::Interface::tcs_init ********************************************/
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
    std::string function = "Acam::Interface::tcs_init";
    std::stringstream message;
    long error = NO_ERROR;

    // No arg is a query
    //
    if ( args.empty() ) {

      if ( ! this->tcsd.client.is_open() ) {
        // tcsd does not have a connection to the TCS
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
        catch( std::exception &e ) {
          message.str(""); message << "ERROR invalid reply \"" << retstring << "\" from tcsd: " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
      }

      return error;
    }

    // If shutting down then stop the focus monitoring thread first
    //
    if ( args == "shutdown" ) {
      // Request stop
      //
      this->monitor_focus_state.store( Acam::FOCUS_MONITOR_STOP_REQ, std::memory_order_seq_cst );

      // Wait up to 1.5 sec for stop
      //
      auto start = std::chrono::steady_clock::now();
      bool timedout=false;
      while ( this->monitor_focus_state.load( std::memory_order_seq_cst ) != Acam::FOCUS_MONITOR_STOPPED ) {
        auto now = std::chrono::steady_clock::now();
        if ( now - start > std::chrono::milliseconds( 1500 ) ) {
          timedout=true;
          break;
        }
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
      }

      // Stop request timed out. Set the error but continue.
      //
      if ( timedout ) {
        logwrite( function, "ERROR timeout waiting for focus monitor thread to stop" );
        error = ERROR;
      }
    }

    // Send command to tcs daemon client. If help was requested then that
    // request is passed on here to tcsd.init() so this could return HELP.
    //
    error |= this->tcsd.init( args, retstring );   // OR'd to include possible stop-thread-timeout

    // If the TCS initialization is successful, then spawn a detached thread
    // to monitor the focus (if not already running), which is what keeps
    // the Guider GUI updated on focus changes.
    //
    if ( error==NO_ERROR && args != "shutdown" ) {
      this->tcs_online.store( true );
      if ( this->monitor_focus_state.load( std::memory_order_seq_cst ) == Acam::FOCUS_MONITOR_STOPPED ) {
        this->monitor_focus_state.store( Acam::FOCUS_MONITOR_START_REQ, std::memory_order_seq_cst );
        std::thread( this->dothread_monitor_focus, std::ref(*this) ).detach();
      }
    }
    else {
      this->tcs_online.store( false );
    }
    return error;
  }
  /***** Acam::Interface::tcs_init ********************************************/


  /***** Acam::Interface::framegrab_fix ***************************************/
  /**
   * @brief      wrapper to control Andor frame grabbing using last WCSfix filename
   * @param[in]  args       only accepts "?|help" for help
   * @param[out] retstring  return string for help or error status
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::framegrab_fix( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::framegrab_fix";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_FRAMEGRABFIX;
      retstring.append( " \n" );
      retstring.append( "   This is a convenience function to grab a single ACAM image using\n" );
      retstring.append( "   the WCSfix filename from the last solve. If a solve was not run in the\n" );
      retstring.append( "   last 60 seconds then it is considered stale and \"" + ACAMD_FRAMEGRABFIX + "\" will return\n" );
      retstring.append( "   an error.\n\n" );
      retstring.append( "   This is equivalent to \"" + ACAMD_FRAMEGRAB + " /tmp/acam_WCSfix.fis\"\n" );
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
  /***** Acam::Interface::framegrab_fix ***************************************/


  /***** Acam::Interface::framegrab *******************************************/
  /**
   * @brief      wrapper to control Andor frame grabbing
   * @param[in]  args       optional filename as source for header info
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::framegrab( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::framegrab";
    std::stringstream message;
    long error = NO_ERROR;
    std::string _imagename = this->imagename;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_FRAMEGRAB;
      retstring.append( " [ start | stop | one [ <filename> ] | saveone <filename> | status ]\n" );
      retstring.append( "   Start/Stop continuous frame grabbing or grab one single ACAM image.\n" );
      retstring.append( "   \n" );
      retstring.append( "   If an optional <filename> is supplied then that file will be used\n" );
      retstring.append( "   as a source for header information for the frame.\n" );
      retstring.append( "   \n" );
      retstring.append( "   A <filename> provided with the saveone argument will save the framegrab\n" );
      retstring.append( "   to that filename.\n" );
      retstring.append( "   \n" );
      retstring.append( "   No argument or \"status\" returns true|false to indicate running state.\n" );
      return HELP;
    }

    if ( args.empty() || args == "status" ) {
      retstring = ( this->is_framegrab_running.load() ? "true" : "false" );
      return NO_ERROR;
    }

    // Tokenize the args and make sure there's at least one
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    std::string whattodo, sourcefile;

    if ( tokens.size() > 0 ) whattodo   = tokens[0];
    if ( tokens.size() > 1 ) sourcefile = tokens[1];
    if ( tokens.size() > 2 ) {
      logwrite( function, "ERROR too many arguments. expected \"one [ <sourcefile> ] | start | stop | status\"" );
      retstring="invalid_argument";
      return ERROR;
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
//    this->tcs_online.store( true ); // TODO
    }

    std::thread( dothread_framegrab, std::ref(*this), whattodo, sourcefile ).detach();

    // When stopping framegrabbing, wait for it to stop. Timeout after 2 exptimes.
    // Always wait a minimum of 1 second in case exptime is 0.
    //
    if ( whattodo == "stop" ) {
      auto start = std::chrono::steady_clock::now();
      bool timedout=false;
      auto timeout_time = std::chrono::duration<double>( std::max( (2.0*this->camera.andor.camera_info.exptime), 2.0 ) );
      while ( this->is_framegrab_running.load() ) {
        auto now = std::chrono::steady_clock::now();
        if ( now - start > timeout_time ) {
          timedout=true;
          break;
        }
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
      }
      if ( timedout ) logwrite( function, "ERROR timeout waiting for framegrab to stop" );
    }
    return error;
  }
  /***** Acam::Interface::framegrab *******************************************/


  /***** Acam::Interface::dothread_framegrab **********************************/
  /**
   * @brief      performs continuous acquisition
   * @details    This should be spawned in a thread.
   *             Set should_framegrab_run true if the loop should run continuously.
   *             Set is_framegrab_running true when the loop is running.
   * @param[in]  iface       reference to Acam::Interface object
   *
   */
  void Interface::dothread_framegrab( Acam::Interface &iface, const std::string whattodo, std::string sourcefile ) {
    std::string function = "Acam::Interface::dothread_framegrab";
    std::stringstream message;
    long error = NO_ERROR;

    if ( whattodo == "one" || whattodo == "saveone" ) {
      // Clear should_framegrab_run which means the framegrab loop should not run.
      // If it's already running then return, the existing framegrab loop will
      // stop. If it's not already running then drop through, and a single
      // frame will be grabbed.
      //
      iface.should_framegrab_run.store( false );
      if ( iface.is_framegrab_running.load() ) return;
    }
    else
    if ( whattodo == "start" ) {
      if ( iface.is_framegrab_running.load() ) {
        logwrite( function, "thread already running, exiting" );
        return;
      }
      else {
        logwrite( function, "set thread running" );
        iface.should_framegrab_run.store( true );
      }
    }
    else
    if ( whattodo == "stop" ) {
      logwrite( function, "set thread to stop, exiting" );
      iface.should_framegrab_run.store( false );
      return;
    }
    else {
      message.str(""); message << "ERROR invalid argument \"" << whattodo << "\". exiting.";
      logwrite( function, message.str() );
      return;
    }

    iface.wcsname.clear();
    iface.wcsfix_time = std::chrono::steady_clock::time_point::min();

    // The speed of this do loop can potentially be limited by either the
    // exposure time or the acquisition, which can be limited by the solver.
    // In other words, it will acquire images as fast as it needs to, but no
    // faster.
    //
    // If I can get the lock then BoolState sets is_framegrab_running true,
    // and clears it (false) automatically when it goes out of scope.
    //
    {
    std::unique_lock<std::mutex> lock( iface.framegrab_mutex, std::defer_lock );             // try to get the mutex
    if ( lock.try_lock() ) {
      BoolState loop_running( iface.is_framegrab_running );    // sets state true here, clears when it goes out of scope
      if (error==NO_ERROR) error = iface.camera.andor.start_acquisition();
      do {
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );                       // don't use too much CPU

        if ( iface.camera.andor.camera_info.exptime == 0 ) continue;            // wait for non-zero exposure time

        if (error==NO_ERROR) error = iface.camera.andor.get_recent(3000);       // get latest frame from camera into memory
        if (error==NO_ERROR) error = iface.collect_header_info();                            // collect header information
        if (error==NO_ERROR) error = iface.camera.write_frame( sourcefile,
                                                               iface.imagename,
                                                               iface.tcs_online.load() );    // write to FITS file

        iface.framegrab_time = std::chrono::steady_clock::time_point::min();

        iface.guide_manager.push_guider_image( iface.imagename );                            // send frame to Guider GUI

        // acquire target if needed
        //
        if (error==NO_ERROR) {
          // do_acquire() is called with each frame grab and will return NO_ERROR
          // each time, unless it reaches the max number of retries or otherwise
          // fails.
          //
          long did_acquire = iface.target.do_acquire();                                      // acquire target here (if needed)
          if ( did_acquire != NO_ERROR ) {
            iface.target.acquire( Acam::TARGET_NOP );                                        // disable acquire on failure
          }
        }

      } while ( error==NO_ERROR && iface.should_framegrab_run.load() );
      error |= iface.camera.andor.abort_acquisition();
    }
    else {                                                                                   // this shouldn't even happen
      logwrite( function, "ERROR another thread is already running" );
      return;
    }
    }

    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR starting thread" );
      iface.should_framegrab_run.store( false );      // disable frame grabbing
      iface.target.acquire( Acam::TARGET_NOP );       // disable acquisition
    }
    else logwrite( function, "leaving thread" );

    return;
  }
  /***** Acam::Interface::dothread_framegrab **********************************/


  /***** Acam::Interface::guider_settings_control *****************************/
  /**
   * @brief      set or get the guider settings for the SAOImage GUIDER display
   * @details    The guider uses SAOImage for display and control. When ds9 is
   *             used to change any one (or more) parameter(s), it will call
   *             this function to set the parameter(s). This function will
   *             return the current parameters and it will call a shell
   *             command which pushes the parameters to the ds9 display. If
   *             no args are supplied then the parameters will only be returned
   *             and pushed (not set).
   * @param[in]  args       <empty> or contains all: <exptime> <gain> <filter> <focus>
   * @param[out] retstring  return string contains <exptime> <gain> <filter> <focus>
   * @return     ERROR | NO_ERROR | HELP
   *
   * This function is overloaded
   *
   */
  long Interface::guider_settings_control( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::guider_settings_control";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_GUIDESET;
      retstring.append( " [ <exptime> <gain> <filter> <focus> ]\n" );
      retstring.append( "   Set or get guider settings for SAOImage GUIDER display.\n" );
      retstring.append( "   When all arguments are supplied they will be set and then pushed\n" );
      retstring.append( "   back to the display. If no arguments are supplied then the current\n" );
      retstring.append( "   settings are returned and pushed to the display.\n" );
      retstring.append( "   The DONE|ERROR suffix on the return string is suppressed.\n" );
      return HELP;
    }

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    // If something was supplied but not the correct number of args then that's an error
    //
    if ( !tokens.empty() && tokens.size() != 4 ) {
      message.str(""); message << "ERROR received " << tokens.size() << " arguments "
                               << "but expected <exptime> <gain> <filter> <focus>";
      logwrite( function, message.str() );
      retstring="invalid_argument_list";
      return ERROR;
    }

    // Get the original exptime and gain from the Andor Information class
    //
    float exptime_og = this->camera.andor.camera_info.exptime;
    int gain_og = this->camera.andor.camera_info.gain;

    long error = NO_ERROR;
    bool set = false;
    float exptime=NAN;

    // If all args are supplied then set all parameters
    //
    if ( tokens.size() == 4 ) {
      try {
        std::string reply;
        exptime = std::stof( tokens.at(1) );

        // set the exposure time here
        error |= camera.set_exptime( exptime );

        // set the gain here
        error |= camera.gain( tokens.at(1), reply );

        // set the filter (always uppercase) via a thread
        std::transform( tokens.at(2).begin(), tokens.at(2).end(), tokens.at(2).begin(), ::toupper );
        std::thread( dothread_set_filter, std::ref( *this ), tokens.at(2) ).detach();

        // set the focus via a thread
        std::thread( dothread_set_focus, std::ref( *this ), std::stod( tokens.at(3) ) ).detach();

        set=true;
      }
      catch( std::invalid_argument &e ) {
        message.str(""); message << "ERROR parsing \"" << args << "\": " << e.what();
        logwrite( function, message.str() );
        retstring="invalid_argument";
        error = ERROR;
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR parsing \"" << args << "\": " << e.what();
        logwrite( function, message.str() );
        retstring="invalid_argument";
        error = ERROR;
      }
    }

    // Set or not, now read the current values and use the guide_manager
    // to set them in the class.
    //
    guide_manager.exptime = camera.andor.camera_info.exptime;
    guide_manager.gain = camera.andor.camera_info.gain;

    // If read-only (not set) or either exptime or gain changed, then set the flag for updating the GUI.
    //
    if ( !set || guide_manager.exptime != exptime_og || guide_manager.gain != gain_og ) {
      guide_manager.set_update();
    }

    if ( motion.filter( "", guide_manager.filter ) != NO_ERROR ) {
      logwrite( function, "ERROR couldn't read filter" );
      guide_manager.filter="ERR";
      error=ERROR;
    }

    double _focus;
    if ( tcsd.get_focus( _focus ) != NO_ERROR ) {
      logwrite( function, "ERROR couldn't read focus" );
      guide_manager.focus.store(NAN, std::memory_order_seq_cst);
      error=ERROR;
    }

    retstring = guide_manager.get_message_string();

    logwrite( function, retstring );

    std::thread( &Acam::GuideManager::push_guider_settings, &guide_manager ).detach();

    return error;
  }
  /***** Acam::Interface::guider_settings_control *****************************/


  /***** Acam::Interface::guider_settings_control *****************************/
  /**
   * @brief      get the guider settings for the SAOImage GUIDER display
   * @details    Passes an empty string to guider_settings_control() so that
   *             the parameters will only be returned and pushed (not set).
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Interface::guider_settings_control() {
    std::string retstring;
    return ( this->guider_settings_control( std::string(""), retstring ) );
  }
  /***** Acam::Interface::guider_settings_control *****************************/


  /***** Acam::Interface::acquire *********************************************/
  /**
   * @brief      outside interface to set or get the target acquisition
   * @details    This uses the target.acquire() function to gain access to
   *             the Acam::Target class. The input coordinates are validated
   *             and set using the target_coords() function.
   * @param[in]  args       string,  [ <ra> <dec> <angle> | guide | stop ]
   * @param[out] retstring  contains timeout in seconds
   * @return     ERROR | NO_ERROR | HELP
   *
   * See embedded help for details on format of the input args string.
   *
   */
  long Interface::acquire( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::acquire";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_ACQUIRE;
      retstring.append( " [ <ra> <dec> <angle> [ <name> ] [ acam | slit ] | guide | stop ]\n" );
      retstring.append( "   Set or get target acquisition mode.\n" );
      retstring.append( "   Returns ACQUIRE_TIMEOUT, the number of seconds before acquisition sequence\n" );
      retstring.append( "   aborts on failure to acquire.\n" );
      retstring.append( "   If no args supplied then the current status is returned in the form of\n" );
      retstring.append( "   <mode> where <mode> = { stopped acquiring guiding }\n" );
      retstring.append( "   Acquisition requires knowing what to acquire so the coords must be supplied\n" );
      retstring.append( "   via one of the following:\n" );
      retstring.append( "\n" );
      retstring.append( "   <ra> <dec> <angle> acquire target using specified coords in SLIT referece\n" );
      retstring.append( "                      frame, then resume normal open loop tracking.\n" );
      retstring.append( "                      When supplying coordinates, an optional arg \"acam\" or\n" );
      retstring.append( "                      \"slit\" may be provided to set the pointmode, indicating\n" );
      retstring.append( "                      where the target is placed. If not provided, the default\n" );
      retstring.append( "                      is \"slit\".\n" );
      retstring.append( "                      An optional target name <name> may also be provided.\n" );
      retstring.append( "\n" );
      retstring.append( "   guide              requires target acquisition using coords or target,\n" );
      retstring.append( "                      then will repeatedly acquire target and continue to run\n" );
      retstring.append( "                      the solver for subsequent images, sending corrections\n" );
      retstring.append( "                      to the TCS.\n" );
      retstring.append( "\n" );
      retstring.append( "   stop               disables any current guiding or acquisition in process.\n" );
      retstring.append( "\n" );
      retstring.append( "   When supplying target coords the expected input string <ra> <dec> <angle>\n" );
      retstring.append( "   can be decimal or sexagesimal. If decimal then <ra> can be decimal degrees\n" );
      retstring.append( "   or decimal hours. Specify decimal hours by adding a \"h\", as in \"1.234h\".\n" );
      return HELP;
    }

    // No args is a status request only,
    // returns acquire_mode_string = { stopped | acquiring | guiding }
    //
    if ( args.empty() ) {
      message.str(""); message << this->target.acquire_mode_string();
      logwrite( function, message.str() );
      retstring = message.str();
      return NO_ERROR;
    } else

    // If the camera isn't open then this shouldn't be started
    //
    if ( ! isopen( "camera" ) ) {
      logwrite( function, "ERROR no connection to camera" );
      retstring = "camera_not_open";
      return ERROR;
    } else

    // If the exposure time is zero then there won't be any starlight
    //
    if ( this->camera.andor.camera_info.exptime == 0 ) {
      logwrite( function, "ERROR exposure time cannot be zero." );
      retstring = "camera_exptime";
      return ERROR;
    } else

    // If not already in GUIDE mode then enable GUIDE mode
    //
    if ( args == "guide" ) {
      if ( this->target.acquire_mode == Acam::TARGET_GUIDE ) {
        logwrite( function, "target guide mode already selected" );
        return NO_ERROR;
      }
      this->astrometry.isacquire = false;                        // informs the solver
      return( this->target.acquire( Acam::TARGET_GUIDE ) );      // enable Target GUIDE mode
    } else

    // If not already stopped then stop (disable by setting mode to NOP)
    //
    if ( args == "stop" ) {
      if ( this->target.acquire_mode == Acam::TARGET_NOP ) {
        logwrite( function, "target guide/acquisition already stopped" );
        return NO_ERROR;
      }
      return( this->target.acquire( Acam::TARGET_NOP ) );        // disables Target guide/acquisition
    } else

    // Finally, if not already in ACQUIRE mode and not one of the above commands
    // then assume args contains coordinates and pass them to target_coords() which
    // will validate them before setting them in the Target class. If coords are
    // valid then enable ACQUIRE mode.
    //
    {
      // Default pointmode, if not specified, is SLIT
      //
      this->target.set_pointmode( Acam::POINTMODE_SLIT );

      // If pointing mode was provided, set that separately then remove it
      // from the string, so that the remaining string can be sent to
      // target_coords() which interprets <ra> <dec> <angle> [ <name> ]
      //
      make_uppercase( args );
      size_t pos;
      while ( ( pos = args.find( " ACAM" ) ) != std::string::npos ) {
        args.erase( pos, 5 );
        this->target.set_pointmode( Acam::POINTMODE_ACAM );
      }
      while ( ( pos = args.find( " SLIT" ) ) != std::string::npos ) {
        args.erase( pos, 5 );
        this->target.set_pointmode( Acam::POINTMODE_SLIT );
      }
      message.str(""); message << "pointmode " << this->target.get_pointmode() << " selected";
      logwrite( function, message.str() );

      if ( this->target.acquire_mode == Acam::TARGET_ACQUIRE ) {
        logwrite( function, "target acquisition mode already selected" );
        return NO_ERROR;
      }
      std::string coords;
      long error = this->target_coords( args, coords );          // validate and set the target coords
      logwrite( function, coords );
      if ( error == NO_ERROR ) {
        double timeout = this->target.get_timeout();             // from config ACAM_ACQUIRE_TIMEOUT
        retstring=std::to_string( timeout );                     // return the timeout in seconds
        this->astrometry.isacquire = true;                       // informs the solver
        return( this->target.acquire( Acam::TARGET_ACQUIRE ) );  // enable Target ACQUIRE mode
      }
      else return ERROR;
    }
  }
  /***** Acam::Interface::acquire *********************************************/


  /***** Acam::Target::acquire ************************************************/
  /**
   * @brief      controls target acquisition
   * @details    This is the interface between the Interface class and the
   *             Target class, providing outside access to the Target class.
   * @param[in]  requested_mode  this is a TargetAcquisitionModes enum
   * @return     ERROR | NO_ERROR
   *
   */
  long Target::acquire( Acam::TargetAcquisitionModes requested_mode ) {
    std::string function = "Acam::Target::acquire";
    std::stringstream message;

    // If the frame grab thread isn't running then try to start it
    //
    if ( ! iface->is_framegrab_running.load() && iface->should_framegrab_run.load() ) {

      logwrite( function, "starting frame grabber" );
      std::string dontcare;
      iface->framegrab( "start", dontcare );           // start framegrab thread

      auto start = std::chrono::steady_clock::now();

      while ( ! iface->is_framegrab_running.load()  ) {     // shouldn't take long but wait up to 100 msec
        auto now = std::chrono::steady_clock::now();
        if ( now - start > std::chrono::milliseconds( 100 ) ) break;
        std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
      }

      if ( ! iface->is_framegrab_running.load() ) {
        iface->async.enqueue_and_log( function, "ERROR: failed to start frame grabber" );
        return ERROR;
      }
    }

    // Can't start guiding until target has been acquired. In other words,
    // must start with ACQUIRE, because this will send different args to
    // the solver.
    //
    if ( requested_mode == Acam::TARGET_GUIDE && ! this->is_acquired ) {
      logwrite( function, "ERROR: cannot start guiding until target has been acquired" );
      return ERROR;
    }

    // For a stop request, set stop_acquisition true, which will break out of the
    // acquisition loop and prevent further acquisitions from starting.
    //
    if ( requested_mode == Acam::TARGET_NOP ) {
      this->stop_acquisition.store( true );
      logwrite( function, "stop requested" );
    }

    if ( requested_mode == Acam::TARGET_ACQUIRE || requested_mode == Acam::TARGET_GUIDE) {
      // initialize variables for new acquisition
      //
      this->nacquired = 0;
      this->attempts = 0;
      this->sequential_failures = 0;
      this->is_acquired.store( false, std::memory_order_seq_cst );

      // Start the timeout clock, initialized as the time now plus the
      // configured timeout in seconds (duration defaults to seconds).
      //
      this->timeout_time = std::chrono::steady_clock::now()
                         + std::chrono::duration<double>(this->timeout);

      // This informs do_acquire() that everything has been initialized
      // and we're ready to start the acquisition.
      //
      this->stop_acquisition.store( false, std::memory_order_seq_cst );
      logwrite( function, "acquisition initialized" );
    }

    this->acquire_mode = requested_mode;

    return NO_ERROR;
  }
  /***** Acam::Target::acquire ************************************************/


  /***** Acam::Target::do_acquire *********************************************/
  /**
   * @brief      performs the actual target acquisition if conditions are right
   * @details    This is called by dothread_framegrab() in a loop, after each frame.
   *             Based on various conditions, this will either return immediately,
   *             or it will call the solver and move the telescope.
   *
   *             If the acquire_mode is NOP or if stop_acquisition is set, then it
   *             returns immediately.
   *
   *             If the acquire_mode is ACQUIRE then it will continue the solve
   *             and offset until it succeeds or fails or times out.
   *
   *             If the acquire_mode is GUIDE then its just like ACQUIRE except
   *             that it never stops until stopped, or error.
   *
   * @return     ERROR | NO_ERROR | TIMEOUT
   *
   */
  long Target::do_acquire() {
    std::string function = "Acam::Target::do_acquire";
    std::stringstream message;

    // Do nothing, return immediately if no acquisition mode selected
    // or if stop_acquisition is set.
    //
    if ( this->acquire_mode == Acam::TARGET_NOP || this->stop_acquisition.load() ) {
      return NO_ERROR;
    }

    // Here is the acquisition sequence, called after every frame grab.
    //
    // Using that frame, call the solver and send offsets to the TCS. After this
    // will be checks on the success and quality of the solution.
    //
    // ACQUIRE and GUIDE are the same except that GUIDE doesn't time-out,
    // and they send different arguments to the solver.
    //

    // "error" is reserved here for success of the do_acquire() function and is not used
    // for each internal function call. If any function call fails in this do-loop then
    // break immediately (without setting error). This will cause it to get counted as
    // an attempt and will keep trying until max attempts.
    //
    long error = NO_ERROR;

    if ( this->acquire_mode == Acam::TARGET_ACQUIRE || this->acquire_mode == Acam::TARGET_GUIDE ) {
    do {
      // ACQUIRE mode can time out so check that here (GUIDE never times out).
      //
      if ( this->acquire_mode == Acam::TARGET_ACQUIRE ) {
        if ( std::chrono::steady_clock::now() > this->timeout_time ) {
          iface->async.enqueue_and_log( function, "ERROR: failed to acquire target within timeout" );
          return TIMEOUT;
        }
      }

      attempts++;

      // The acam interface is given the target coordinates (from the database)
      // which are in the <pointmode> reference frame.
      //
      // Here compute the "goal" for the acam (in the ACAM reference frame) using
      // the database coordinates and the current cass angle.
      //

      // Convert the current cass rotator angle (which is in the SCOPE frame) to a
      // position angle in the <pointmode> frame.  The RA, DEC coordinates don't matter
      // for just computing the angle. pointmode_string() returns a string of the
      // current pointmode.
      //
      if ( iface->fpoffsets.compute_offset( "SCOPE", pointmode,
                                            0, 0, this->tcs_casangle ) == ERROR ) break;

      // Now convert the target coordinates (from the database) using this angle,
      // from the <pointmode> to the ACAM reference frame.
      //
      double acam_ra_goal, acam_dec_goal, acam_angle_goal;

      if ( iface->fpoffsets.compute_offset_last_angle( pointmode, "ACAM",
                                                       this->coords_slit.ra, this->coords_slit.dec,
                                                       acam_ra_goal, acam_dec_goal, acam_angle_goal ) == ERROR ) break;

      // Apply any dRA, dDEC goal offsets from the "put on slit" action to
      // acam_ra_goal, acam_dec_goal. These dRA,dDEC offsets can come from
      // either the ACAM or slicecam GUIs and are stored in the Target class.
      //
      // The supplied acam_ra_goal,acam_dec_goal are modified by dRA,dDEC
      //
      if ( iface->fpoffsets.apply_offset( acam_ra_goal,  iface->target.dRA,
                                          acam_dec_goal, iface->target.dDEC ) == ERROR ) break;

//    {  // this local section is just for display purposes and its variables are not used elsewhere
//    std::string rastr, decstr;
//    decimal_to_sexa( acam_ra_goal * TO_HOURS, rastr );
//    decimal_to_sexa( acam_dec_goal, decstr );
//    message.str(""); message << "[ACQUIRE] set goals=" << rastr << "  " << decstr << "  "
//                                                       << std::fixed << std::setprecision(6) << acam_angle_goal;
//    logwrite( function, message.str() );
//    }

      // Perform the astrometry calculations on the acquired image (and calculate image quality).
      // Optional solver args can be included here, which currently only come from the test commands.
      //
      bool match_found = false;                    // was a match found?
      long matches=-1;
      double acam_ra=NAN, acam_dec=NAN, acam_angle=NAN, rmsarcsec=NAN;  // calculations from acam solver
      std::string result;                          // acam solver result

      std::string last_imagename = iface->get_imagename();

      // call wrapper for Astrometry::solve()
      //
      if ( iface->astrometry.solve( last_imagename, this->ext_solver_args ) == ERROR ) break;

      // this gets the solution from the solver
      //
      iface->astrometry.get_solution( result, acam_ra, acam_dec, acam_angle, matches, rmsarcsec );

      if ( result=="GOOD" || result=="NOISY" ) {   // treat GOOD and NOISY the same for now
        match_found = true;
        std::string rastr, decstr;
        double _ra = acam_ra * TO_HOURS;
        decimal_to_sexa( _ra, rastr );
        decimal_to_sexa( acam_dec, decstr );
//      message.str(""); message << "[ACQUIRE] solve " << result << ": match found with coords="
//                               << rastr << "  " << decstr << "  " << acam_angle;
//      logwrite( function, message.str() );
        iface->astrometry.image_quality();
      }

      // Add properly-typed telemetry keys/value pairs to a database map object,
      // which accepts the name of the database column and the value, which can be
      // any type of value (string, int, float, bool, etc.).
      // This does not write them to the actual database yet.
      //
      iface->database.add_key_val( "result",   result );
      iface->database.add_key_val( "RAsolve",  acam_ra );
      iface->database.add_key_val( "DECsolve", acam_dec );
      iface->database.add_key_val( "ANGsolve", acam_angle );
      iface->database.add_key_val( "matches", matches );
      iface->database.add_key_val( "rms_arcsec", rmsarcsec );

      // If no match found and exceeded number of attempts then give up and get out.
      // This counts as an attempt so attempts is incremented.
      //
      if ( ! match_found && ( this->max_attempts > 0 ) && ( attempts >= this->max_attempts ) ) {
        iface->async.enqueue_and_log( function, "ERROR: failed to acquire target within max number of attempts" );
        error = ERROR;
        break;
      }

      // get out in any case if no match found
      //
      if ( !match_found ) break;

      // Continue only if there was a match

      // Calculate the offsets to send to the TCS.
      //
      // Offsets calculated as difference between acam goals and the solution from
      // solve. ACAM goals are the SLIT->ACAM translated coords from the DB and
      // are where the ACAM needs to be pointed. Goals never change.
      //
      double ra_off, dec_off;  // calculated offsets will be in degrees

      if ( iface->fpoffsets.solve_offset( acam_ra, acam_dec,
                                          acam_ra_goal, acam_dec_goal,
                                          ra_off, dec_off ) == ERROR ) break;

//    {  // this local section is just for display purposes and its variables are not used elsewhere
//    double __ra, __dec;
//    iface->tcsd.get_weather_coords( __ra, __dec );  // returns RA hours, DEC degrees
//    message.str(""); message << "[ACQUIRE] tcs coords before: " << __ra*TO_DEGREES << " " << __dec << " deg";
//    logwrite( function, message.str() );
//    }

      // Compute the angular separation between the target (acam_ra_*) and calculated slit (acam_*)
      //
      double offset = angular_separation( acam_ra_goal, acam_dec_goal, acam_ra, acam_dec );

      // Add properly-typed telemetry keys/value pairs to a database map object.
      // This does not write them to the actual database yet.
      //
      iface->database.add_key_val( "RAoffset",   ra_off );
      iface->database.add_key_val( "DECLoffset", dec_off );

      this->offset_cal_offset += offset;

//    message.str(""); message << "[ACQUIRE] offset=" << offset << " (arcsec)"; logwrite( function,message.str() );

      // There is a maximum offset allowed to the TCS.
      // This is not a TCS limit (their limit is very large).
      // This is our limit so that we don't accidentally move too far off the
      // slit. However, "putonslit" can include a desired offset which is
      // outside this limit, so when checking the calculated offset, include a
      // delta which is the change introduced by putonslit.
      //

      // this will be the solution plus dRA, dDEC
      // start by initializing with acam_ra,acam_dec
      //
      double acam_ra_dRA   = acam_ra;
      double acam_dec_dDEC = acam_dec;

      // Then acam_ra_dRA, acam_dec_dDEC will be modified by applying dRA, dDEC
      //
      iface->fpoffsets.apply_offset( acam_ra_dRA,   iface->target.dRA,
                                     acam_dec_dDEC, iface->target.dDEC );

      // the offset introduced by putonslit is therefore the separation between
      // acam_ra,acam_dec and acam_ra_dRA,acam_dec_dDEC
      //
      this->putonslit_offset = angular_separation( acam_ra_dRA, acam_dec_dDEC, acam_ra, acam_dec );

      // and the delta is the difference between this and the last time,
      // which gets added to the tcs_max_offset.
      //
      double maxoffset = this->tcs_max_offset + std::fabs(this->putonslit_offset - this->last_putonslit_offset);

      // so remember this for next time
      //
      this->last_putonslit_offset = this->putonslit_offset;

      // Finally, check the requested offset against this putonslit-modified max allowed offset
      //
      if ( offset >= maxoffset ) {
        message.str(""); message << "[WARNING] calculated offset " << offset << " not below max "
                                 << maxoffset << " and will not be sent to the TCS";
        logwrite( function, message.str() );

        // Match found but failure to send an offset is considered an attempt
        // so attempts is incremented.
        //
        if ( attempts >= this->max_attempts ) {
          message.str(""); message << "ERROR: failed to find offset below " << this->tcs_max_offset << " within max number of attempts";
          iface->async.enqueue_and_log( function, message.str() );
          error = ERROR;
          break;
        }
      }
      // otherwise send the offsets to the TCS and keep looping.
      //
      else {
        // send offset to TCS here (returns when offset is complete)
        if ( iface->tcsd.pt_offset( ra_off*3600., dec_off*3600. )==ERROR) break;

        // reset retry counter if match found and offset < max and no errors
        attempts = 0;
      }

      // If the offset is below ACQUIRE_OFFSET_THRESHOLD then increment the nacquired
      // counter. We need ACQUIRE_MIN_REPEAT sequential, successful acquires in order
      // to call this a success, so any failure here resets the counter.
      //
      if ( offset < this->offset_threshold ) {
        // but only in ACQUIRE mode, otherwise this could increment forever in GUIDE mode
        if ( this->acquire_mode == Acam::TARGET_ACQUIRE ) {
          this->nacquired++;
          message.str(""); message << "acquired " << this->nacquired << " of " << this->min_repeat;
          logwrite( function, message.str() );
        }
        else if ( this->acquire_mode == Acam::TARGET_GUIDE ) this->nacquired=this->min_repeat;
      }
      else {
        nacquired=0;     // if an acquire is not below threshold then reset the counter
      }

    } while ( false );  // the do-loop is executed only once. used to allow breaks.
    }                   // end of acquisition sequence

    if ( this->nacquired == 0 && this->max_attempts > 0 ) {
      if ( ++this->sequential_failures >= this->max_attempts ) {
        logwrite( function, "ERROR sequential failures exceeds max attempts" );
        error = ERROR;
      }
    }

    // If target acquired, and if acquisition mode was to acquire,
    // then disable acquisition mode.
    //
    if ( this->acquire_mode == Acam::TARGET_ACQUIRE &&
         this->nacquired >= this->min_repeat ) {
      logwrite( function, "NOTICE: target acquired" );
      this->is_acquired.store( true, std::memory_order_seq_cst );                   // Target Acquired!
      this->acquire_mode = Acam::TARGET_NOP;
    }
    else
    // In guide mode, just silently keep the is_acquired flag set
    //
    if ( this->acquire_mode == Acam::TARGET_GUIDE &&
         this->nacquired == this->min_repeat ) {
      this->is_acquired.store( true, std::memory_order_seq_cst );
    }

    // Leaving this function with an error means target acquisition has failed
    //
    if ( error != NO_ERROR ) {
      iface->async.enqueue_and_log( "ERROR", function, "failed to acquire target" );
    }

    // Add properly-typed telemetry keys/value pairs to a database map object,
    // which accepts the name of the database column and the value, which can be
    // any type of value (string, int, float, bool, etc.).
    // This does not write them to the actual database yet.
    //
    iface->database.add_key_val( "acquired",       this->is_acquired.load() );
    iface->database.add_key_val( "seeing",         iface->astrometry.get_seeing() );
    iface->database.add_key_val( "seeing_std",     iface->astrometry.get_seeing_std() );
    iface->database.add_key_val( "seeing_zen",     iface->astrometry.get_seeing_zen() );
    iface->database.add_key_val( "extinction",     iface->astrometry.get_extinction() );
    iface->database.add_key_val( "extinction_std", iface->astrometry.get_extinction_std() );
    iface->database.add_key_val( "background",     iface->astrometry.get_background() );
    iface->database.add_key_val( "background_std", iface->astrometry.get_background_std() );
    iface->database.add_key_val( "filter",         iface->motion.get_current_filter_name() );

    iface->database.add_key_val( "datetime",       get_datetime() );

    // This will write the collected telemetry key/value pairs to the database,
    // then will clear the map object once they are written.
    //
    try {
      iface->database.write();
    }
    catch ( ... ) {
      logwrite( function, "ERROR writing to database" );
      error=ERROR;
    }

    return error;
  }
  /***** Acam::Target::do_acquire *********************************************/


  /***** Acam::Interface::dothread_set_filter *********************************/
  /**
   * @brief      sets the filter and updates GUIDER GUI
   * @details    This should be spawned in a thread.
   * @param[in]  iface       reference to Acam::Interface object
   * @param[in]  filter_req  requested filter
   *
   */
  void Interface::dothread_set_filter( Acam::Interface &iface, std::string filter_req ) {
    std::string function = "Acam::Interface::dothread_set_filter";
    std::stringstream message;

    // get current filter, used to determine if it changed
    //
    std::string filter_og;
    long error = iface.motion.filter( "", filter_og );

    std::string filter;

    error |= iface.motion.filter( filter_req, filter );

/***
    bool arrived = false;

    // While the difference between the current and requested focus is greater
    // than tolerance, the focus is still moving so keep updating the display.
    //
    do {
      arrived = ( std::abs( focus - focus_req ) < tolerance ? true : false );
      error = iface.tcsd.get_focus( focus );
      iface.guide_settings_manager.set_value( "focus", std::to_string(focus), arrived );
      std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
    } while ( error!=ERROR && !arrived );

***/

    if ( iface.guide_manager.filter != filter_og ) {
      iface.guide_manager.set_update();
      std::thread( &Acam::GuideManager::push_guider_settings, &iface.guide_manager ).detach();
    }

    if ( error==ERROR ) {
      logwrite( function, "ERROR reading filter" );
      iface.guide_manager.filter="error";
    }

    return;
  }
  /***** Acam::Interface::dothread_set_filter *********************************/


  /***** Acam::Interface::dothread_set_focus **********************************/
  /**
   * @brief      sets the telescope focus and updates GUIDER GUI
   * @details    This should be spawned in a thread.
   * @param[in]  iface      reference to Acam::Interface object
   * @param[in]  focus_req  requested focus position
   *
   */
  void Interface::dothread_set_focus( Acam::Interface &iface, double focus_req ) {
/*
    std::string function = "Acam::Interface::dothread_set_focus";
    std::stringstream message;

    // get current focus, used to determine if it changed
    //
    double focus_og;
    long error = iface.tcsd.get_focus( focus_og );

    double tolerance=0.01;
    bool arrived = false;
    int stalled = 0, stall_limit=240;

    error |= iface.tcsd.set_focus( focus_req );

    // While the difference between the current and requested focus is greater
    // than tolerance, the focus is still moving so keep updating the display.
    //
    do {
      double last_focus = iface.guide_manager.focus;
      arrived = ( std::abs( iface.guide_manager.focus - focus_req ) < tolerance ? true : false );
      error |= iface.tcsd.get_focus( iface.guide_manager.focus );
      iface.guide_manager.push_guider_settings();
      std::this_thread::sleep_for( std::chrono::milliseconds( 250 ) );
      stalled = ( std::abs( iface.guide_manager.focus - last_focus ) < tolerance ? stalled+1 : 0 );
    } while ( error!=ERROR && !arrived && stalled < stall_limit );

    if ( stalled == stall_limit ) {
      logwrite( function, "ERROR TCS focus position is not changing" );
    }

    if ( iface.guide_manager.focus != focus_og ) {
      iface.guide_manager.set_update();
      iface.guide_manager.push_guider_settings();
    }

    if ( error==ERROR ) {
      logwrite( function, "ERROR reading TCS focus position" );
      iface.guide_manager.focus=NAN;
    }

*/
    return;
  }
  /***** Acam::Interface::dothread_set_focus **********************************/


  /***** Acam::Interface::dothread_fpoffset ***********************************/
  /**
   * @brief      for testing, calls a Python function from a thread
   *
   */
  void Interface::dothread_fpoffset( Acam::Interface &iface ) {
    std::string function = "Acam::Interface::dothread_fpoffset";
    std::stringstream message;

    message.str(""); message << "calling fpoffsets.compute_offset() from thread: PyGILState=" << PyGILState_Check();
    logwrite( function, message.str() );

    double ra_to, dec_to, angle_to;

    iface.fpoffsets.compute_offset( "SCOPE", "ACAM", 17, -24, 19, ra_to, dec_to, angle_to );

    message.str(""); message << "output = " << ra_to << " " << dec_to << " " << angle_to << " : PyGILState=" << PyGILState_Check();
    logwrite( function, message.str() );

    return;
  }
  /***** Acam::Interface::dothread_fpoffset ***********************************/


  /***** Acam::Interface::dothread_monitor_focus ******************************/
  /**
   * @brief      monitors the telescope focus and updates GUIDER GUI when it changes
   * @details    This should be spawned in a thread that never dies.
   * @param[in]  iface      reference to Acam::Interface object
   *
   */
  void Interface::dothread_monitor_focus( Acam::Interface &iface ) {
    std::string function = "Acam::Interface::dothread_monitor_focus";
    std::stringstream message;

    if ( iface.monitor_focus_state.load(std::memory_order_seq_cst) == Acam::FOCUS_MONITOR_RUNNING ) {
      logwrite( function, "thread already running" );
      return;
    }

    if ( iface.monitor_focus_state.load(std::memory_order_seq_cst) == Acam::FOCUS_MONITOR_START_REQ ) {
      logwrite( function, "thread starting" );
      iface.monitor_focus_state.store( Acam::FOCUS_MONITOR_RUNNING, std::memory_order_seq_cst );
    }

    double focus1, focus2;
    long error = NO_ERROR;

    double tolerance=0.01;

    // Read baseline focus, then
    // keep reading the focus forever looking for change.
    //

    error = iface.tcsd.poll_focus( focus1 );

    // This will force push to guider the first time this thread has started,
    // otherwise updates are not made unless the focus has changed.
    //
    bool initial_pass = true;

    // Sometimes a call to the TCS fails but if you try again it succeeds.
    // Instead of terminating the thread at the first sign of an error,
    // try again up to 3 times. Each good call will reset this counter,
    // and each failed call will increment it.
    //
    int error_counter = 0;

    do {
      std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

      error = iface.tcsd.poll_focus( focus2 );  // focus now

      // If the focus now has changed (within tolerance) then
      // store it in the class and push it to the guider.
      //
      if ( std::isnan( focus1 ) || std::isnan( focus2 ) ) {
        logwrite( function, "ERROR focus value is NaN" );
        error = ERROR;
      }
      else
      if ( initial_pass ||                              // either first time or
           std::abs( focus1 - focus2 ) > tolerance ) {  // focus has changed
        initial_pass = false;
        focus1 = focus2;                                // new baseline focus
        iface.guide_manager.focus.store( focus1, std::memory_order_seq_cst );      // save baseline focus to the class
        std::thread( &Acam::GuideManager::push_guider_settings, &iface.guide_manager ).detach();  // push new focus
      }

      // An error will increment the error counter and clear the error flag
      // up to 3 times.
      //
      if ( error != NO_ERROR && error_counter++ < 3 ) {
        logwrite( function, "ERROR polling TCS focus but will retry" );
        error=NO_ERROR;
      }
      else error_counter=0;

    } while ( error == NO_ERROR && 
              iface.monitor_focus_state.load(std::memory_order_seq_cst) == Acam::FOCUS_MONITOR_RUNNING );

    message.str(""); message << "thread terminated" << ( error != NO_ERROR ? " with error" : "" );
    logwrite( function, message.str() );

    iface.monitor_focus_state.store( Acam::FOCUS_MONITOR_STOPPED, std::memory_order_seq_cst );

    return;
  }
  /***** Acam::Interface::dothread_monitor_focus ******************************/


  /***** Acam::Interface::shutdown ********************************************/
  /**
   * @brief      shutdown all threads and connections
   * @param[in]  args       only used for help
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::shutdown( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::shutdown";
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
      retstring = ACAMD_SHUTDOWN;
      retstring.append( "\n" );
      retstring.append( "  Shutdown all threads which communicate with the camera and TCS,\n" );
      retstring.append( "  and close all connections. The daemon remains running.\n" );
      return HELP;
    }

    long error = NO_ERROR;
    std::string dontcare;

    // stop the framegrab thread
    //
    error |= this->framegrab( "stop", dontcare );

    // request stop the focus monitor
    //
    this->monitor_focus_state.store( Acam::FOCUS_MONITOR_STOP_REQ,
                                     std::memory_order_seq_cst );

    // shutdown TCS connection (this will also stop focus monitor thread)
    //
    error |= this->tcs_init( "shutdown", dontcare );

    // close socket connections to hardware
    //
    error |= this->close( "all", dontcare );

    if ( error == NO_ERROR ) logwrite( function, "acam interfaces shut down" );
    else logwrite( function, "ERROR shutting down acam interfaces" );

    return error;
  }
  /***** Acam::Interface::shutdown ********************************************/


  /***** Acam::Interface::test ************************************************/
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
   */
  long Interface::test( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::test";
    std::stringstream message;
    std::vector<std::string> tokens;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" ) {
      retstring = ACAMD_TEST;
      retstring.append( "\n" );
      retstring.append( "  Test Routines\n" );
      retstring.append( "   camera ? | <args...>\n" );
      retstring.append( "   collect [ ? ] \n" );
      retstring.append( "   database\n" );
      retstring.append( "   fpoffsets ? | <from> <to> <ra> <dec> <angle> (see help for units)\n" );
      retstring.append( "   mode ? | cont | single \n" );
      retstring.append( "   monitorfocus [ ? | stop | start ]\n" );
      retstring.append( "   pointmode\n" );
      retstring.append( "   shouldframegrab [ ? | yes | no ]\n" );
      retstring.append( "   sleep\n" );
      retstring.append( "   solverargs [ ? | <key=val> [...<keyn=valn>] ]\n" );
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

    // --------------------------------
    // set optional external solver args
    //
    if ( testname == "solverargs" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {                              // help
        retstring = ACAMD_TEST;
        retstring.append( " solverargs [ <args > ]\n" );
        retstring.append( "  Set optional args to pass to the solver. If <args> is empty\n" );
        retstring.append( "  then clear the external solver args (otherwise they persist).\n" );
        retstring.append( "  The current external solver args are returned.\n" );
        return HELP;
      }
      if ( tokens.size() > 1 ) {                                                  // any tokens are the solver args
        auto pos = args.find( "solverargs " );                                    // note space at end of test name
        args = args.substr( pos );                                                // get from "solverargs " to the end
        this->target.ext_solver_args.clear();
      }
      else {                                                                      // otherwise erase the solver args
        this->target.ext_solver_args.clear();
      }
      retstring.clear();
      for ( const auto &solver_arg : this->target.ext_solver_args ) {
        retstring.append( solver_arg );
        retstring.append( " " );
      }
      error=NO_ERROR;
    }
    else
    // --------------------------------
    // fpoffsets
    //
    if ( testname == "fpoffsets" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {                              // help
        retstring = ACAMD_TEST;
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
          dec_from = std::stod( dec_str );
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
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR invalid argument parsing \"" << args << "\" : " << e.what();
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return ERROR;
      }
      catch ( std::out_of_range &e ) {
        message.str(""); message << "ERROR out of range parsing \"" << args << "\" : " << e.what();
        logwrite( function, message.str() );
        retstring="out_of_range";
        return ERROR;
      }
    }
    else
    // --------------------------------
    // threadoffset
    //
    if ( testname == "threadoffset" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = ACAMD_TEST;
        retstring.append( " threadoffset\n" );
        retstring.append( "  Spawns a thread which calls a Python function.\n" );
        error=HELP;
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
    // --------------------------------
    // camera
    //
    if ( testname == "camera" ) {
      if ( tokens.size() < 2 ) {
        logwrite( function, "ERROR expected argument" );
        retstring="invalid_argument";
        return ERROR;
      }
      else
      if ( tokens[1] == "?" ) {
        retstring = ACAMD_TEST;
        retstring.append( " camera ...\n" );
        retstring.append( "  acquire         starts acquisition, and gets the acquired image\n" );
        retstring.append( "  adchans         return number of AD converter channels\n" );
        retstring.append( "  abort           aborts acquisition\n" );
        retstring.append( "  acont [ n ]     switch to mode cont and acquire n continuous frame(s)\n" );
        retstring.append( "  ft on|off       gets the most recent image\n" );
        retstring.append( "  getrecent       get recent image from camera\n" );
        retstring.append( "  mode ? | <m>    <m>=single | cont\n" );
        retstring.append( "  shutter <s>     where <s> = open|close|auto\n" );
        retstring.append( "  shuttertimes    gets the shutter minimum opening closing times in ms\n" );
        retstring.append( "  start           starts acquisition\n" );
        retstring.append( "  status          Andor status\n" );
        error=HELP;
      }
      else
      // --------------------------------
      // camera acquire
      //
      if ( tokens[1]=="acquire" ) {
        int nacquires=1;
        if ( tokens.size() > 2 ) {
          try { nacquires = std::stoi(tokens.at(2)); }
          catch( const std::exception &e ) {
            message.str(""); message << "ERROR parsing nacquires: " << e.what();
            logwrite( function, message.str() );
            return ERROR;
          }
        }
        error = this->test("mode single",retstring);
        do {
          logwrite( function, std::to_string(nacquires) );
          error |= this->camera.andor.acquire_one();                     // acquire a single image
          error |= this->collect_header_info();                          // collect header information
          error |= this->camera.write_frame( "",
                                             this->imagename,
                                             this->tcs_online.load() );  // write to FITS file
          this->guide_manager.push_guider_image( this->imagename );      // send frame to Guider GUI
          std::this_thread::sleep_for( std::chrono::milliseconds(500) );
        } while (--nacquires > 0);
      }
      else
      // --------------------------------
      // camera adchans
      //
      if ( tokens[1] == "adchans" ) {
        if ( tokens.size() > 2 && tokens[2] == "?" ) {
          retstring = ACAMD_TEST;
          retstring.append( " camera adchans\n" );
          retstring.append( "  Calls Andor SDK wrapper to return the number of AD converter channels\n" );
          retstring.append( "   available.\n" );
          error=HELP;
        }
        else {
          int chans;
          error = this->camera.andor.sdk._GetNumberADChannels( chans );
          retstring = std::to_string(chans);
        }
      }
      else
      // --------------------------------
      // camera cont
      //
      if ( tokens[1]=="cont" ) {
        int nacquires=1;
        if ( tokens.size() > 2 ) {
          try { nacquires = std::stoi(tokens.at(2)); }
          catch( const std::exception &e ) {
            message.str(""); message << "ERROR parsing nacquires: " << e.what();
            logwrite( function, message.str() );
            return ERROR;
          }
        }
        error  = this->test("mode cont",retstring);
        error |= this->camera.andor.start_acquisition();
        do {
          logwrite( function, std::to_string(nacquires) );
          error |= this->camera.andor.get_recent(3000);                  // 
          error |= this->collect_header_info();                          // collect header information
          error |= this->camera.write_frame( "",
                                             this->imagename,
                                             this->tcs_online.load() );  // write to FITS file
          this->guide_manager.push_guider_image( this->imagename );      // send frame to Guider GUI
        } while (--nacquires > 0);
        error |= this->camera.andor.abort_acquisition();
      }
      else
      if ( tokens[1]=="abort" ) {
        error = this->camera.andor.abort_acquisition();
      }
      else
      // --------------------------------
      // camera ft on | off
      //
      if ( tokens[1]=="ft" ) {
        if ( tokens.size() > 2 ) {
          if ( tokens[2]=="on" || tokens[2]=="off" ) {
            int mode = ( tokens[2]=="on" ? 1 : 0 );
            error = this->camera.andor.sdk._SetFrameTransferMode( mode );
          }
          else {
            logwrite( function, "ERROR expected on|off" );
            return ERROR;
          }
        }
      }
      else
      // --------------------------------
      // camera getrecent
      //
      if ( tokens[1]=="getrecent" ) {
        error = this->camera.andor.get_recent(3000);
        error |= this->collect_header_info();                          // collect header information
        error |= this->camera.write_frame( "",
                                           this->imagename,
                                           this->tcs_online.load() );  // write to FITS file
        this->guide_manager.push_guider_image( this->imagename );      // send frame to Guider GUI
      }
      else
      // --------------------------------
      // camera mode
      //
      if ( tokens[1] == "mode" ) {
        if ( tokens.size() < 3 ) {
          logwrite( function, "ERROR camera mode expected argument" );
          retstring="invalid_argument";
          return ERROR;
        }
        else
        if ( tokens[2] == "?" ) {
          retstring = ACAMD_TEST;
          retstring.append( " camera mode single | cont\n" );
          retstring.append( "  Sets Andor acquisition mode to single scan\n" );
          retstring.append( "  or continuous (run till abort).\n" );
          error=HELP;
        }
        else
        if ( tokens[2] == "single" ) {
          error |= this->camera.andor.set_image( 1, 1, 1, this->camera.andor.camera_info.axes[0], 1, this->camera.andor.camera_info.axes[1] );
          error |= this->camera.andor.set_read_mode( 4 );                  // image mode
          error |= this->camera.andor.set_acquisition_mode( 1 );           // single scan
          error |= this->camera.andor.set_shutter( std::string("auto") );  // shutter auto open,close each exposure
        }
        else
        if ( tokens[2] == "cont" ) {
          error |= this->camera.andor.set_image( 1, 1, 1, this->camera.andor.camera_info.axes[0], 1, this->camera.andor.camera_info.axes[1] );
          error |= this->camera.andor.set_read_mode( 4 );                  // image mode
          error |= this->camera.andor.set_acquisition_mode( 5 );           // run till abort
          error |= this->camera.andor.sdk._SetFrameTransferMode( 1 );      // enable Frame Transfer mode
          error |= this->camera.andor.sdk._SetKineticCycleTime( 0 );
          error |= this->camera.andor.set_shutter( std::string("open") );  // shutter always open
        }
      }
      else
      // --------------------------------
      // camera start
      //
      if ( tokens[1]=="start" ) {
        error = this->camera.andor.start_acquisition();
      }
      else
      // --------------------------------
      // camera status
      //
      if ( tokens[1]=="status" ) {
        error = this->camera.andor.get_status();
      }
      else
      // --------------------------------
      // camera shutter
      //
      if ( tokens[1]=="shutter" ) {
        if ( tokens.size() > 2 ) {
          if ( tokens[2]=="open" || tokens[2]=="close" || tokens[2]=="auto" ) {
            error = this->camera.andor.set_shutter(tokens[2]);
          }
          else {
            logwrite( function, "ERROR expected open|close|auto" );
            return ERROR;
          }
        }
        else {
          logwrite( function, "ERROR expected open|close|auto" );
          return ERROR;
        }
      }
      else
      // --------------------------------
      // camera shuttertimes
      //
      if ( tokens[1]=="shuttertimes" ) {
        int opening, closing;
        error = this->camera.andor.sdk._GetShutterMinTimes( opening, closing );
        message.str(""); message << opening << " " << closing << " msec";
        retstring=message.str();
      }
      else {
        logwrite( function, "ERROR invalid test camera argument" );
        return ERROR;
      }
    }
    else
    // --------------------------------
    // collect
    //
    if ( testname == "collect" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = ACAMD_TEST;
        retstring.append( " collect\n" );
        retstring.append( "  Gather information and add it to the internal keyword database.\n" );
        error=HELP;
      }
      else error = this->collect_header_info();             // collect header information
    }
    else
    // --------------------------------
    // database
    //
    if ( testname == "database" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = ACAMD_TEST;
        retstring.append( " database\n" );
        retstring.append( "  Test save some stuff to the telemetry database.\n" );
        error=HELP;
      }
      else {
        try {
          this->database.add_key_val( "datetime",   get_datetime() );
          this->database.add_key_val( "result",   "result" );
          this->database.add_key_val( "obs_id",   123 );
          this->database.add_key_val( "airmass",   1.23 );
          this->database.add_key_val( "acquired",   true );
          this->database.write();
        }
        catch ( const std::exception &e ) {
          message.str(""); message << "ERROR adding keys to database: " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
      }
    }
    else
    // --------------------------------
    // monitorfocus
    //
    if ( testname == "monitorfocus" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = ACAMD_TEST;
        retstring.append( " monitorfocus [ stop | start ]\n" );
        retstring.append( "  Stop or start a thread which monitors the telescope focus and\n" );
        retstring.append( "  updates GUIDER GUI when it changes. Anything else returns the\n" );
        retstring.append( "  running state of that thread.\n" );
        error=HELP;
      }
      else
      if ( tokens.size() > 1 && tokens[1] == "start" ) {
        this->monitor_focus_state.store( Acam::FOCUS_MONITOR_START_REQ, std::memory_order_seq_cst );
        std::thread( this->dothread_monitor_focus, std::ref(*this) ).detach();
      }
      else
      if ( tokens.size() > 1 && tokens[1] == "stop" ) {
        this->monitor_focus_state.store( Acam::FOCUS_MONITOR_STOP_REQ, std::memory_order_seq_cst );
      }
      else {
        auto state = this->monitor_focus_state.load( std::memory_order_seq_cst );
        switch( state ) {
          case Acam::FOCUS_MONITOR_STOPPED:   retstring = "stopped";
                                              break;
          case Acam::FOCUS_MONITOR_STOP_REQ:  retstring = "stop_req";
                                              break;
          case Acam::FOCUS_MONITOR_START_REQ: retstring = "start_req";
                                              break;
          case Acam::FOCUS_MONITOR_RUNNING:   retstring = "running";
                                              break;
          default:                            retstring = "error";
                                              break;
        }
      }
    }
    else
    // --------------------------------
    // pointmode
    //
    if ( testname == "pointmode" ) {
      retstring = "pointmode " + this->target.get_pointmode() + " selected";
    }
    else
    // --------------------------------
    // shouldframegrab
    //
    if ( testname == "shouldframegrab" ) {
      retstring = ( this->should_framegrab_run.load() ? "yes" : "no" );
    }
    else
    // --------------------------------
    // sleep
    //
    if ( testname == "sleep" ) {
      for ( int i=0; i<10; i++ ) {
        logwrite( function, "sleeping . . ." );
        std::this_thread::sleep_for( std::chrono::seconds(1) );
      }
    }
    else {
      message.str(""); message << "ERROR unknown testname \"" << testname << "\"";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    return error;

  }
  /***** Acam::Interface::test ************************************************/


  /***** Acam::Interface::exptime *********************************************/
  /**
   * @brief      set/get exposure time
   * @param[in]  args
   * @param[out] retstring
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::exptime( const std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::exptime";
    std::stringstream message;
    long error=NO_ERROR;

    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_EXPTIME;
      retstring.append( " [ <exptime> ]\n" );
      retstring.append( "  Set or get camera exposure time in decimal seconds.\n" );
      retstring.append( "  If <exptime> is provided then the camera exposure time is changed,\n" );
      retstring.append( "  else the current exposure time is returned.\n" );
      return HELP;
    }

    // No arg just return the exposure time stored in the Andor Information class
    // without reading from the Andor
    //
    if ( args.empty() ) {
      retstring = std::to_string( this->camera.andor.camera_info.exptime );
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
      message.str(""); message << "exception_"<< e.what();
      retstring=message.str();
      return ERROR;
    }
    // I have a potentially valid float value now

    // If framegrab is running then stop it. This won't return until framegrabbing
    // has stopped (or timeout).
    //
    bool was_framegrab_running = this->is_framegrab_running.load();
    if ( was_framegrab_running ) {
      std::string dontcare;
      error = this->framegrab( "stop", dontcare );
    }

    // setting the exposure time will stop any acquisition
    //
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
  /***** Acam::Interface::exptime *********************************************/


  /***** Acam::Interface::image_quality ***************************************/
  /**
   * @brief      wrapper for Astrometry::image_quality()
   * @param[in]  args
   * @param[out] retstring
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::image_quality( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::image_quality";
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
      retstring = ACAMD_QUALITY;
      retstring.append( "\n" );
      retstring.append( "  Calls the (Python) astrometry image quality function.\n" );
      retstring.append( "  This should be called only after a solve and needs to be called only\n" );
      retstring.append( "  once for each solve.\n" );
      return HELP;
    }

    long error = this->astrometry.image_quality( );
    if ( error != NO_ERROR) this->async.enqueue( "ERROR: calling image_quality" );
    return error;
  }
  /***** Acam::Interface::image_quality ***************************************/


  /***** Acam::Interface::solve ***********************************************/
  /**
   * @brief      wrapper for Astrometry::solve()
   * @details    The input args string can be empty, contain an image name,
   *             and/or one or more key=val pairs to be passed to the solver.
   *             Additional solver args can come from the class, which are set
   *             in the configuration file.
   * @param[in]  args       input args string (see details)
   * @param[out] retstring  contains the return string from the solver
   * @return     ERROR | NO_ERROR | HELP
   *
   * The return string is of the form "<RESULT> <RA> <DEC> <ANGLE>"
   *
   */
  long Interface::solve( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::solve";
    std::stringstream message;
    long error = NO_ERROR;
    std::string _imagename;
    std::string _wcsname;

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] args=" << args << " wcsname=" << this->wcsname << " imagename=" << this->imagename;
    logwrite( function, message.str() );
#endif

    // Help
    //
    if ( args == "?" ) {
      retstring = ACAMD_SOLVE;
      retstring.append( " [<filename>] [ <key>=<val> ... ]\n" );
      retstring.append( "  Calls the (Python) astrometry solver.\n" );
      retstring.append( "  If <filename> is provided (fully qualified path with .fits extension)\n" );
      retstring.append( "  then send that filename to the solver; otherwise, if no filename\n" );
      retstring.append( "  then the output file of the previous solve will be used.\n" );
      retstring.append( "\n" );
      retstring.append( "  Any number of optional <key>=<value> pairs may be included, which\n" );
      retstring.append( "  will be passed to the (Python) solver.\n" );
      return HELP;
    }

    // This is the local vector of solver args that are passed with
    // the ACAMD_SOLVE command. This vector is passed to the astrometry
    // solver.
    //
    std::vector<std::string> _solverargs;

    // The input args can be a filename and/or include any number of key=value pairs.
    // Tokenize on the space " " to separate any filename from key=value pairs.
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    for ( size_t tok = 0; tok < tokens.size(); tok++ ) {
      // This extracts the image name, anything with a ".fits" on it
      //
      if ( tokens.at(tok).find(".fits") != std::string::npos ) {
        _imagename = tokens.at(tok);
      }

      // This extracts any key=value pairs, anything with a "=" in it
      //
      if ( tokens.at(tok).find("=") != std::string::npos ) {
        _solverargs.push_back( tokens.at(tok) );                           // add any solver args passed in,
      }
    }  // end for

    // If no .fits file was found then _imagename will be empty,
    // so get it from the class.
    //
    if ( _imagename.empty() ) {
      _imagename = this->get_imagename();

      auto time_now = std::chrono::steady_clock::now();
      auto time_since_framegrab = std::chrono::duration_cast<std::chrono::seconds>( time_now - framegrab_time ).count();

      if ( time_since_framegrab > 60 ) {
        message.str(""); message << "WARNING time since last frame grab is " << time_since_framegrab << " sec";
        logwrite( function, message.str() );
      }
    }

    // They can't both be empty, need an image name from somewhere
    //
    if ( _imagename.empty() ) {
      logwrite( function, "ERROR: imagename not provided and no name from previous solve" );
      return ERROR;
    }

    // Call the astrometry solver
    //
    error = this->astrometry.solve( _imagename, _solverargs );

    if ( error != NO_ERROR) this->async.enqueue( "ERROR: calling astrometry solver" );

    // Put the results into the return string
    //
    retstring = this->astrometry.get_result();

    // Astrometry::solve() will create a new file with "_WCSfix" appended to
    // the filename (before the .fits extension) but it doesn't return that,
    // you just have to know. So that extension is added here and the new
    // name is stored in the class.
    //
    std::string suffix = "_WCSfix";
    
    try {
      size_t dotfits = _imagename.find( ".fits" );
      _wcsname = _imagename;
      _wcsname.insert( dotfits, suffix );
    }
    catch( std::out_of_range const& ) {
      message.str(""); message << "ERROR: malformed filename? Could not find \".fits\" in imagename \"" << _imagename << "\"";
      this->async.enqueue_and_log( function, message.str() );
      return ERROR;
    }
    catch( ... ) {
      message.str(""); message << "ERROR unknown exception inserting suffix into \"" << _wcsname << "\"";
      this->async.enqueue_and_log( function, message.str() );
      return ERROR;
    }

    // Save the wcsname to the class
    //
    this->wcsname = _wcsname;
    this->wcsfix_time = std::chrono::steady_clock::now();
//  this->set_wcsname( _wcsname );

    return error;
  }
  /***** Acam::Interface::solve ***********************************************/


  /***** Acam::Interface::collect_header_info *********************************/
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
  long Interface::collect_header_info() {
    std::string function = "Acam::Interface::collect_header_info";
    std::stringstream message;

    bool _tcs = this->tcs_online.load();
    std::string tcsname;

    // Check every time for the TCS because we don't ever want to save
    // an image with ambiguous keywords.
    //
    if ( _tcs && this->tcsd.client.poll_open() ) {
      this->tcsd.poll_name( tcsname );
    }
    else {
      tcsname = "offline";
      this->tcs_online.store( false );
      _tcs = this->tcs_online.load();
    }

    // request external telemetry
    //
    this->get_external_telemetry();

    double angle_scope=NAN, ra_scope=NAN, dec_scope=NAN,
           angle_acam=NAN,  ra_acam=NAN,  dec_acam=NAN;

    // Get the current pointing from the TCS
    //
    if ( _tcs ) this->tcsd.poll_cass( angle_scope );
    if ( _tcs ) this->tcsd.poll_coords( ra_scope, dec_scope );  // returns RA in decimal hours, DEC in decimal degrees

    if ( _tcs ) this->target.save_casangle( angle_scope );      // store in the Target class, required for acquisition

    // Compute FP offsets from TCS coordinates (SCOPE) to ACAM coodinates.
    // compute_offset() always wants degrees and get_coords() returns RA hours.
    // Results in degrees.
    //
    if ( _tcs ) this->fpoffsets.compute_offset( "SCOPE", "ACAM", (ra_scope*TO_DEGREES), dec_scope, angle_scope,
                                                                             ra_acam, dec_acam, angle_acam );

    // Get the focus from the guide manager which is already monitoring it.
    // As long as the focus monitor is running then use the focus value,
    // otherwise set it to NaN. addkey() knows how to handle that.
    //
    auto isfocus = this->monitor_focus_state.load( std::memory_order_seq_cst );
    double focus = this->guide_manager.focus.load( std::memory_order_seq_cst );
    if ( isfocus != Acam::FOCUS_MONITOR_RUNNING ) focus = NAN;

    // Get some info from the Andor::Information class,
    // which is stored in its camera_info object.
    //
    std::copy( std::begin( this->camera.andor.camera_info.axes ),
                 std::end( this->camera.andor.camera_info.axes ),
               std::begin( this->camera.fitsinfo.axes ) );

    // Add information to the Camera::FitsInfo::FitsKeys database
    // either a prioi or from the Andor::Information class
    //
    this->camera.fitsinfo.fitskeys.erase_db();

    this->camera.fitsinfo.fitskeys.addkey( "TCS",  tcsname, "" );

    this->camera.fitsinfo.fitskeys.addkey( "CREATOR",  "acamd", "file creator" );
    this->camera.fitsinfo.fitskeys.addkey( "INSTRUME", "NGPS", "name of instrument" );
    this->camera.fitsinfo.fitskeys.addkey( "TELESCOP", "P200", "name of telescope" );
    this->camera.fitsinfo.fitskeys.addkey( "TELFOCUS", focus, "telescope focus (mm)" );

    this->camera.fitsinfo.fitskeys.addkey( "EXPSTART", this->camera.andor.camera_info.timestring, "exposure start time" );
    this->camera.fitsinfo.fitskeys.addkey( "MJD0",     this->camera.andor.camera_info.mjd0, "exposure start time (modified Julian Date)" );
    this->camera.fitsinfo.fitskeys.addkey( "EXPTIME",  this->camera.andor.camera_info.exptime, "exposure time (sec)" );
    this->camera.fitsinfo.fitskeys.addkey( "SERNO",    this->camera.andor.camera_info.serial_number, "camera serial number" );
    this->camera.fitsinfo.fitskeys.addkey( "ACQMODE",  this->camera.andor.camera_info.acqmodestr, "acquisition mode" );
    this->camera.fitsinfo.fitskeys.addkey( "READMODE", this->camera.andor.camera_info.readmodestr, "read mode" );
    this->camera.fitsinfo.fitskeys.addkey( "TEMPSETP", this->camera.andor.camera_info.setpoint, "detector temperature setpoint deg C" );
    this->camera.fitsinfo.fitskeys.addkey( "TEMPREAD", this->camera.andor.camera_info.ccdtemp, "CCD temperature deg C" );
    this->camera.fitsinfo.fitskeys.addkey( "TEMPSTAT", this->camera.andor.camera_info.temp_status, "CCD temperature status" );
    this->camera.fitsinfo.fitskeys.addkey( "FITSNAME", this->camera.andor.camera_info.fits_name, "this filename" );
    this->camera.fitsinfo.fitskeys.addkey( "HBIN",     this->camera.andor.camera_info.hbin, "horizontal binning pixels" );
    this->camera.fitsinfo.fitskeys.addkey( "VBIN",     this->camera.andor.camera_info.vbin, "vertical binning pixels" );
    this->camera.fitsinfo.fitskeys.addkey( "HSPEED",   this->camera.andor.camera_info.hspeed, "horizontal clocking speed MHz" );
    this->camera.fitsinfo.fitskeys.addkey( "VSPEED",   this->camera.andor.camera_info.vspeed, "vertical clocking speed MHz" );
    this->camera.fitsinfo.fitskeys.addkey( "AMPTYPE",  this->camera.andor.camera_info.amptypestr, "CCD amplifier type" );
    this->camera.fitsinfo.fitskeys.addkey( "CCDGAIN",  this->camera.andor.camera_info.gain, "CCD amplifier gain" );

    this->camera.fitsinfo.fitskeys.addkey( "GAIN",     1, "e-/ADU" );

    this->camera.fitsinfo.fitskeys.addkey( "PIXSCALE",  this->camera.andor.camera_info.pixel_scale, "arcsec per pixel" );
    this->camera.fitsinfo.fitskeys.addkey( "POSANG",    angle_acam, "" );
    this->camera.fitsinfo.fitskeys.addkey( "TARGET",    this->target.get_name(), "target name" );
    this->camera.fitsinfo.fitskeys.addkey( "TELRA",     ra_scope, "Telecscope Right Ascension hours" );
    this->camera.fitsinfo.fitskeys.addkey( "TELDEC",    dec_scope, "Telescope Declination degrees" );
    this->camera.fitsinfo.fitskeys.addkey( "CASANGLE",  angle_scope, "Cassegrain ring angle" );
    this->camera.fitsinfo.fitskeys.addkey( "AIRMASS",   NAN, "" );
    this->camera.fitsinfo.fitskeys.addkey( "WCSAXES",   2, "" );
    this->camera.fitsinfo.fitskeys.addkey( "RADESYSA",  "ICRS", "" );
    this->camera.fitsinfo.fitskeys.addkey( "CTYPE1",    "RA---TAN", "" );
    this->camera.fitsinfo.fitskeys.addkey( "CTYPE2",    "DEC--TAN", "" );
    this->camera.fitsinfo.fitskeys.addkey( "CRPIX1",    this->camera.andor.camera_info.hend/2, "" );
    this->camera.fitsinfo.fitskeys.addkey( "CRPIX2",    this->camera.andor.camera_info.vend/2, "" );
    this->camera.fitsinfo.fitskeys.addkey( "CRVAL1",    ra_acam, "" );
    this->camera.fitsinfo.fitskeys.addkey( "CRVAL2",    dec_acam, "" );
    this->camera.fitsinfo.fitskeys.addkey( "CUNIT1",    "deg", "" );
    this->camera.fitsinfo.fitskeys.addkey( "CUNIT2",    "deg", "" );
    this->camera.fitsinfo.fitskeys.addkey( "CDELT1",    this->camera.andor.camera_info.pixel_scale/3600., "" );
    this->camera.fitsinfo.fitskeys.addkey( "CDELT2",    this->camera.andor.camera_info.pixel_scale/3600., "" );
    this->camera.fitsinfo.fitskeys.addkey( "PC1_1",     ( -1.0 * cos( angle_acam * PI / 180. ) ), "" );
    this->camera.fitsinfo.fitskeys.addkey( "PC1_2",     (        sin( angle_acam * PI / 180. ) ), "" );
    this->camera.fitsinfo.fitskeys.addkey( "PC2_1",     (        sin( angle_acam * PI / 180. ) ), "" );
    this->camera.fitsinfo.fitskeys.addkey( "PC2_2",     (        cos( angle_acam * PI / 180. ) ), "" );

    std::string retstring;
    this->motion.filter("", retstring);
    this->camera.fitsinfo.fitskeys.addkey( "FILTER", retstring, "ACAM filter name" );
    this->motion.cover("", retstring);
    this->camera.fitsinfo.fitskeys.addkey( "COVER", retstring, "ACAM cover position" );

    return NO_ERROR;
  }
  /***** Acam::Interface::collect_header_info *********************************/


  /***** Acam::Interface::target_coords ***************************************/
  /**
   * @brief      interface to set or get target coords
   * @details    acamd needs to know the target coords and since it doesn't
   *             have access to the database, it must be told. This provides
   *             the interface between the outside and the Acam::Target class.
   *
   *             The expected minimum input string of <ra> <dec> <angle>
   *             can be in either decimal or sexagesimal, and if decimal,
   *             then <ra> can be decimal degrees or decimal hours. Specify
   *             decimal hours by adding a "h", as in "1.234h".
   *             An optional target name may be included.
   *
   * @param[in]  args       expected input <ra> <dec> <angle> [<name>] (see details)
   * @param[out] retstring  return string contains coords from Target class
   * @return     ERROR | NO_ERROR | HELP
   *
   * This is for internal use and not intended to be used externally except
   * test and debugging purposes.
   *
   */
  long Interface::target_coords( const std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::target_coords";
    std::stringstream message;

    double _ra=NAN, _dec=NAN, _angle=NAN;
    std::string _name;

    // Help
    //
    if ( args == "?" ) {
      retstring = ACAMD_COORDS;
      retstring.append( " [ <ra> <dec> <angle> [ <name> ] ]\n" );
      retstring.append( "  Set or get the target coords used for acquisition/guiding.\n" );
      retstring.append( "\n" );
      retstring.append( "  NOT INTENDED TO BE USED DIRECTLY DURING NORMAL OPERATION!\n" );
      retstring.append( "  The " + ACAMD_ACQUIRE + " command uses this internally.\n" );
      retstring.append( "\n" );
      retstring.append( "  The acamd acquisition sequence needs the target coords in the\n" );
      retstring.append( "  SLIT reference frame, as stored in the database.\n\n" );
      retstring.append( "  <ra>    can be sexagesimal HH:MM:SS.sss or decimal degrees or decimal\n" );
      retstring.append( "          hours, specified by appending a \"h\" as in \"1.234h\".\n" );
      retstring.append( "  <dec>   can be sexagesimal DD:MM:SS.sss , or decimal degrees.\n" );
      retstring.append( "  <angle> is always specified in decimal degrees.\n" );
      retstring.append( "  <name>  optional target name.\n" );
      retstring.append( "\n" );
      retstring.append( "  If no arg is provided then the last loaded target coords are returned,\n" );
      retstring.append( "  and the return values will be in decimal degrees.\n" );
      return HELP;
    }

    // No arg, get the coords from the Acam::Target class and return them
    //
    if ( args.empty() ) {
      this->target.get_coords( _ra, _dec, _angle );
      message.str(""); message << _ra << " " << _dec << " " << _angle;
      retstring = message.str();
      return NO_ERROR;
    }

    // Otherwise tokenize and parse the args string
    //
    std::vector<std::string> tokens;

    Tokenize( args, tokens, " " );

    // Need at least 3 tokens: ra, dec, angle
    //
    if ( tokens.size() < 3 || tokens.size() > 4 ) {
      message.str(""); message << "ERROR invalid arguments \"" << args << "\": expected <ra> <dec> <angle> [<name>]";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    // Convert the strings to decimal based on what they look like
    //
    try {
      std::string ra_str  = tokens.at(0);
      std::string dec_str = tokens.at(1);

      // Parse RA
      //
      if ( ra_str.find(":") != std::string::npos ) {     // assume sexagesimal if there's a colon in the string
        _ra = radec_to_decimal( ra_str ) * TO_DEGREES;   // convert to decimal degrees
      }
      else
      if ( ends_with( ra_str, "h" ) ) {                  // incoming decimal hours
        ra_str.pop_back();                               // remove the h
        _ra = std::stod( ra_str ) * TO_DEGREES;          // convert to decimal degrees
      }
      else {                                             // leaves only decimal hours
        _ra = std::stod( ra_str );
      }

      // Parse DEC
      //
      if ( dec_str.find(":") != std::string::npos ) {    // assume sexagesimal if there's a colon
        _dec = radec_to_decimal( dec_str );              // convert to decimal degrees
      }
      else {                                             // leaves onnly decimal degrees
        _dec = std::stod( dec_str );
      }

      // Parse Angle
      //
      _angle = std::stod( tokens.at(2) );                // always in decimal degrees

      // Optional Target Name
      //
      if ( tokens.size() == 4 ) _name = tokens.at(3);
    }
    catch ( std::exception &e ) {
      message.str(""); message << "ERROR exception parsing \"" << args << "\" : " << e.what();
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    // If any values are NaN then it's a failure.
    //
    if ( std::isnan( _ra ) || std::isnan( _dec ) || std::isnan( _angle ) ) {
      message.str(""); message << "ERROR one or more NaN values: ra=" << _ra
                               << " dec=" << _dec << " angle=" << _angle;
      logwrite( function, message.str() );
      retstring="bad_value";
      return ERROR;
    }

    // Otherwise, finally, coords are valid so save them to the Acam::Target class
    // which will be used for the target acquisition.
    //
    this->target.set_coords( _ra, _dec, _angle, _name );

    message.str(""); message << _ra << " " << _dec << " " << _angle;
    retstring = message.str();

    return NO_ERROR;
  }
  /***** Acam::Interface::target_coords ***************************************/


  /***** Acam::Interface::offset_cal ******************************************/
  /**
   * @brief      performs the TCS offset calibration, as much as we're allowed
   * @details    The default offsets on the P200 TCS periodically need to be
   *             calibrated and "zeroed". Palomar policy forbids us to send the
   *             needed commands so this procedure cannot be fully automated but
   *             this function performs the needed acquisition and prompts the
   *             user to give the needed verbal queues to the telescope operator.
   * @param[in]  args       none, help only
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::offset_cal( const std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::offset_cal";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_OFFSETCAL;
      retstring.append( " \n" );
      retstring.append( "  Performs as much of the TCS offset calibration as is allowed by policy.\n" );
      retstring.append( "  This will attempt to acquire a target on the ACAM using the current TCS\n" );
      retstring.append( "  coordinates as the target coordinates to acquire. The solver results and\n" );
      retstring.append( "  offsets are logged, displayed (if run on the command line) and pushed to\n" );
      retstring.append( "  the ACAM GUI (if active). The user must provide verbal instruction to the\n" );
      retstring.append( "  telescope operator to zero the offsets.\n" );
      return HELP;
    }
    else if ( !args.empty() ) {
      logwrite( function, "ERROR no arguments expected" );
      retstring="invalid_argument";
      return ERROR;
    }

    // Nothing to do if no TCS.
    //
    if ( ! this->tcs_online.load() ) {
      logwrite( function, "ERROR TCS is not connected" );
      retstring="tcs_offline";
      return ERROR;
    }

    // Move the telescope so the DRA and DDEC offsets on the telescope
    // status display go to zero (this is the RET command).
    //
    tcsd.ret_offsets();

    // get the current TCS coordinates
    // then translate into ACAM coordinates.
    //
    double scope_angle=NAN, scope_ra=NAN, scope_dec=NAN,
           acam_angle=NAN,  acam_ra=NAN,  acam_dec=NAN, rmsarcsec=NAN;
    long matches=-1L;
    long error=NO_ERROR;

    // Get the current pointing from the TCS
    //
    if (error==NO_ERROR) error = this->tcsd.get_cass( scope_angle );
    if (error==NO_ERROR) error = this->tcsd.get_coords( scope_ra, scope_dec );  // returns RA in decimal hours, DEC in decimal degrees

    // and translate that into acam coordinates.
    //
    if (error==NO_ERROR) error = this->fpoffsets.compute_offset( "SCOPE", "ACAM",
                                                                 scope_ra*TO_DEGREES, scope_dec, scope_angle,
                                                                 acam_ra,  acam_dec,  acam_angle );

    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR getting current TCS coordinates" );
      retstring="tcs_fault";
      return ERROR;
    }

    this->target.offset_cal_offset=0.;

    // Before starting the acquire, temporarily increase the max TCS offset limit
    // to the field of view of the ACAM. This needs to be restored on completion.
    //
    double max_offset = this->target.get_tcs_max_offset();  // remember the current max
    this->target.set_tcs_max_offset(300.);                  // increase to 300 arcsec
    message.str(""); message << "changed max offset limit to " << this->target.get_tcs_max_offset() << " arcsec";
    logwrite( function, message.str() );

    // Form and send the acquire command.
    // This will change the target.acquire_mode to TARGET_ACQUIRE while it's acquiring.
    //
    std::stringstream cmd;
    cmd << std::fixed << std::setprecision(6) << acam_ra << " " << acam_dec << " " << acam_angle << " acam";
    error = this->acquire( cmd.str(), retstring );

    // wait for acquire success or failure
    //
    this->async.enqueue_and_log( "NOTICE", function, "waiting for acquisition" );
    while ( this->target.acquire_mode == Acam::TARGET_ACQUIRE ) {
      std::this_thread::sleep_for( std::chrono::milliseconds(100) );
    }
    this->async.enqueue_and_log( "NOTICE", function, this->target.is_acquired ? "target acquired" : "target not acquired" );

    // restore the max offset limit
    //
    this->target.set_tcs_max_offset(max_offset);
    message.str(""); message << "changed max offset limit to " << this->target.get_tcs_max_offset() << " arcsec";
    logwrite( function, message.str() );

    std::string result;

    if (error==NO_ERROR) this->astrometry.get_solution( result, acam_ra, acam_dec, acam_angle, matches, rmsarcsec );

    if (error==NO_ERROR) {
      retstring = "ACAM CALIBRATION RESULT\n";
      retstring.append( "Astrometry result = "+result+"\n" );
      message.str(""); message << "ACAM center = " << acam_ra << " " << acam_dec << " // RA, DEC from solver\n\n";
      retstring.append( message.str() );
      message.str(""); message << "Offset RA = <TBD> arcsec\n";
      retstring.append( message.str() );
      message.str(""); message << "Offset DEC = <TBD> arcsec\n";
      retstring.append( message.str() );
      message.str(""); message << "Total Offset = " << this->target.offset_cal_offset << " arcsec\n\n";
      retstring.append( message.str() );
      retstring.append( "Total Offset GOAL = <TBD>\n\n" );
      retstring.append( "If the total offset is larger than the goal, ask the operator to zero them now.\n" );
      retstring.append( "Then slew off the target, slew back, and press CALIBRATE again to verify offsets\n" );
      retstring.append( "are still small.\n\n" );
      retstring.append( "Calibrating and zeroing the offset improves the efficiency of the Acquisition\n" );
      retstring.append( "and Guide system but does not affect accuracy.\n" );

      // write this to STDOUT and the GUI will pick it up
      //
      std::cout << retstring;
      return HELP;             // returning HELP suppresses DONE|ERROR from being written
    }
    else return error;
  }
  /***** Acam::Interface::offset_cal ******************************************/


  /***** Acam::Interface::offset_goal *****************************************/
  /**
   * @brief      sets the dRA,dDEC offsets to apply to goal
   * @details    The offset is not applied here, this only stores the offsets
   *             in the class. Offsets are applied only while guiding.
   * @param[in]  args       input string expected "<dRA> <dDEC>"
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::offset_goal( const std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::offset_goal";
    std::stringstream message;

    if ( args.empty() ) {
      message << this->target.dRA << " " << this->target.dDEC;
      retstring = message.str();
      return NO_ERROR;
    }

    // Help
    //
    if ( args == "?" ) {
      retstring = ACAMD_OFFSETGOAL;
      retstring.append( " [ <dRA> <dDEC> ]\n" );
      retstring.append( "  Apply offsets <dRA> <dDEC> to the ACAM goal coordinates.\n" );
      retstring.append( "  These offsets are applied only while guiding. If omitted,\n" );
      retstring.append( "  the current offsets are returned. Units are in degrees.\n" );
      return HELP;
    }

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      logwrite( function, "ERROR expected <dRA> <dDEC>" );
      retstring="invalid_argument";
      return ERROR;
    }

    // Convert the input string to double and save in the class
    //
    try {
      double dRA  = std::stod( tokens.at(0) );
      double dDEC = std::stod( tokens.at(1) );

      if (std::isnan(dRA) || std::isnan(dDEC)) throw std::invalid_argument("NaN value encountered");

      this->target.dRA  = dRA;
      this->target.dDEC = dDEC;
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR parsing " << args << ": " << e.what();
      logwrite( function, message.str() );
      retstring="argument_exception";
      return ERROR;
    }

    message.str(""); message << this->target.dRA << " " << this->target.dDEC;
    retstring = message.str();

    return NO_ERROR;
  }
  /***** Acam::Interface::offset_goal *****************************************/


  /***** Acam::Interface::put_on_slit *****************************************/
  /**
   * @brief      move target under crosshairs to slit
   * @param[in]  args       input string expected "<crossra> <crossdec>"
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::put_on_slit( const std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::put_on_slit";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" ) {
      retstring = ACAMD_PUTONSLIT;
      retstring.append( " [ <crossra> <decra> ]\n" );
      retstring.append( "  Moves the target located at <crossra> <crossdec> to the slit.\n" );
      retstring.append( "  This is intended to be called from the GUI where the coordinates\n" );
      retstring.append( "  are supplied by the ds9 crosshairs. The move is performed immediately\n" );
      retstring.append( "  using a PT offset command to the TCS. Units are decimal degrees.\n" );
      return HELP;
    }

    // Nothing to do if no TCS.
    //
    if ( ! this->tcs_online.load() ) {
      logwrite( function, "ERROR TCS is not connected" );
      retstring="tcs_offline";
      return ERROR;
    }

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      logwrite( function, "ERROR expected <crossra> <crossdec>" );
      retstring="invalid_argument";
      return ERROR;
    }

    // Convert the input string of crosshair coords to double
    //
    double cross_ra=NAN;
    double cross_dec=NAN;
    try {
      cross_ra  = std::stod( tokens.at(0) );
      cross_dec = std::stod( tokens.at(1) );
      if (std::isnan(cross_ra) || std::isnan(cross_ra)) throw std::invalid_argument("NaN value encountered");
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR parsing " << args << ": " << e.what();
      logwrite( function, message.str() );
      retstring="argument_exception";
      return ERROR;
    }

    // Get the coordinates of the slit by first getting them from the TCS
    // then translate into slit coordinates.
    //
    double scope_angle=NAN, scope_ra=NAN, scope_dec=NAN,
           slit_angle=NAN,  slit_ra=NAN,  slit_dec=NAN;

    // Get the current pointing from the TCS
    //
    this->tcsd.get_cass( scope_angle );
    this->tcsd.get_coords( scope_ra, scope_dec );  // returns RA in decimal hours, DEC in decimal degrees

    // and translate that into slit coordinates.
    //
    error = this->fpoffsets.compute_offset( "SCOPE", "SLIT",
                                            scope_ra, scope_dec, scope_angle,
                                            slit_ra,  slit_dec,  slit_angle );

    // calculate the offset (dRA,dDEC) between the crosshairs and the slit
    //
    double dRA, dDEC;
    if (error==NO_ERROR) error = this->fpoffsets.solve_offset( cross_ra, cross_dec,
                                                               slit_ra,  slit_dec,
                                                               dRA,      dDEC );
    if (error==NO_ERROR) error = this->tcsd.pt_offset( dRA*3600., dDEC*3600. );

    message.str(""); message << ( error==ERROR ? "ERROR moving " : "moved " )
                             << "target to slit";
    logwrite( function, message.str() );

    return error;
  }
  /***** Acam::Interface::put_on_slit *****************************************/

}
