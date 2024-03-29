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


  /***** Acam::Camera::simandor ***********************************************/
  /**
   * @brief      enable/disable Andor simulator
   * @param[in]  args       optional state { ? help true false }
   * @param[out] retstring  return status { true false }
   * @return     ERROR or NO_ERROR
   *
   */
  long Camera::simandor( std::string args, std::string &retstring ) {
    std::string function = "Acam::Camera::simandor";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = ACAMD_SIMANDOR;
      retstring.append( " [ true | false ]\n" );
      retstring.append( "   Enable Andor simulator.\n" );
      retstring.append( "   If the optional { true false } argument is omitted then the current\n" );
      retstring.append( "   state is returned.\n" );
      return( NO_ERROR );
    }

    // Set the Andor state
    //
    if ( args == "true" ) {
      this->andor.sim_andor( true );
    }
    else
    if ( args == "false" ) {
      this->andor.sim_andor( false );
    }
    else
    if ( ! args.empty() ) {
      message.str(""); message << "ERROR unrecognized arg " << args << ": expected \"true\" or \"false\"";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Set the return string
    //
    std::string_view which_andor = this->andor.get_andor_object();

    if ( which_andor == "sim" ) retstring="true";
    else
    if ( which_andor == "sdk" ) retstring="false";
    else {
      retstring="unknown";
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** Acam::Camera::simandor ***********************************************/


  /***** Acam::Camera::open ***************************************************/
  /**
   * @brief      open connection to Andor and initialize SDK
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::open( std::string args ) {
    std::string function = "Acam::Camera::open";
    std::stringstream message;
    long error=NO_ERROR;

    if ( this->andor.is_initialized() ) {
      logwrite( function, "already initialized" );
    }
    else {
      error = this->andor.open( args );
    }
    return error;
  }
  /***** Acam::Camera::open ***************************************************/


  /***** Acam::Camera::close **************************************************/
  /**
   * @brief      close connection to Andor
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::close() {
    return this->andor.close();
  }
  /***** Acam::Camera::close **************************************************/


  /***** Acam::Camera::start_acquisition **************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::start_acquisition() {
    return ERROR;
  }
  /***** Acam::Camera::start_acquisition **************************************/


  /***** Acam::Camera::get_status *********************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::get_status() {
    return this->andor.get_status();
  }
  /***** Acam::Camera::get_status *********************************************/


  /***** Acam::Camera::bin ****************************************************/
  /**
   * @brief      set or get camera binning
   * @param[in]  args       optionally contains <hbin> <vbin>
   * @param[out] retstring  return string contains <hbin> <vbin>
   * @return     ERROR or NO_ERROR
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
      return( NO_ERROR );
    }

    // Andor must be connected
    //
    if ( !this->andor.is_initialized() ) {
      logwrite( function, "ERROR no connection to camera" );
      return( ERROR );
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
        return( ERROR );
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

    return( error );
  }
  /***** Acam::Camera::bin ****************************************************/


  /***** Acam::Camera::imflip *************************************************/
  /**
   * @brief      set or get camera image flip
   * @param[in]  args       optionally contains <hflip> <vflip> (0=false 1=true)
   * @param[out] retstring  return string contains <hflip> <vflip>
   * @return     ERROR or NO_ERROR
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
      return( NO_ERROR );
    }

    // Andor must be connected
    //
    if ( !this->andor.is_initialized() ) {
      logwrite( function, "ERROR no connection to camera" );
      return( ERROR );
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
        return( ERROR );
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

    return( error );
  }
  /***** Acam::Camera::imflip *************************************************/


  /***** Acam::Camera::imrot **************************************************/
  /**
   * @brief      set camera image rotation
   * @param[in]  args       optionally contains <rotdir> "cw" or "ccw"
   * @param[out] retstring  return string contains <hrot> <vrot>
   * @return     ERROR or NO_ERROR
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
      return( NO_ERROR );
    }

    // Andor must be connected
    //
    if ( !this->andor.is_initialized() ) {
      logwrite( function, "ERROR no connection to camera" );
      return( ERROR );
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
        return( ERROR );
      }

      // Set the rotation now
      //
      error = this->andor.set_imrot( rotdir );
    }

    return( error );
  }
  /***** Acam::Camera::imrot **************************************************/


  /***** Acam::Camera::gain ***************************************************/
  /**
   * @brief      set or get the CCD gain
   * @details    The output amplifier is automatically set based on gain.
   *             If gain=1 then set to conventional amp and if gain > 1
   *             then set the EMCCD gain register.
   * @param[in]  args       optionally contains new gain
   * @param[out] retstring  return string contains temp, setpoint, and status
   * @return     ERROR or NO_ERROR
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
      if ( this->andor.is_initialized() ) {
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
      return( NO_ERROR );
    }

    // Andor must be connected
    //
    if ( !this->andor.is_initialized() ) {
      logwrite( function, "ERROR no connection to camera" );
      return( ERROR );
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
        return( ERROR );
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
      if ( error==NO_ERROR && gain == 1 ) {
        error = this->andor.set_output_amplifier( Andor::AMPTYPE_CONV );
        if (error==NO_ERROR) this->andor.camera_info.gain = gain;
        else { message << "ERROR gain not set"; }
      }
      else
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
    retstring = std::to_string( this->andor.camera_info.gain );

    logwrite( function, retstring );

    return error;
  }
  /***** Acam::Camera::gain ***************************************************/


  /***** Acam::Camera::speed **************************************************/
  /**
   * @brief      set or get the CCD clocking speeds
   * @param[in]  args       optionally contains new clocking speeds
   * @param[out] retstring  return string contains clocking speeds
   * @return     ERROR or NO_ERROR
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
      if ( this->andor.is_initialized() ) {
        retstring.append( "   Current amp type is " );
        retstring.append( ( this->andor.camera_info.amptype == Andor::AMPTYPE_EMCCD ? "EMCCD\n" : "conventional\n" ) );
        retstring.append( "   Select <hori> from {" );
        for ( auto &hspeed : this->andor.camera_info.hsspeeds[ this->andor.camera_info.amptype] ) {
          retstring.append( " " );
          retstring.append( std::to_string( hspeed ) );
        }
        retstring.append( " }\n" );
        retstring.append( "   Select <vert> from {" );
        for ( auto &vspeed : this->andor.camera_info.vsspeeds ) {
          retstring.append( " " );
          retstring.append( std::to_string( vspeed ) );
        }
        retstring.append( " }\n" );
        retstring.append( "   Units are MHz\n" );
      }
      else {
        retstring.append( "   Open connection to camera to see possible speeds.\n" );
      }
      return( NO_ERROR );
    }

    // Andor must be connected
    //
    if ( !this->andor.is_initialized() ) {
      logwrite( function, "ERROR no connection to camera" );
      return( ERROR );
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
        return( ERROR );
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
   * @return     ERROR or NO_ERROR
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
      if ( this->andor.is_initialized() ) {
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
      return( NO_ERROR );
    }

    // Andor must be connected
    //
    if ( !this->andor.is_initialized() ) {
      logwrite( function, "ERROR no connection to camera" );
      return( ERROR );
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
        return( ERROR );
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
    if ( this->image_data != NULL &&
         this->andor.camera_info.npix == framepix ) {
    }
    else
    if ( this->image_data != NULL &&
         this->andor.camera_info.npix != framepix ) {
      delete [] (uint16_t*)this->image_data;
      this->image_data = NULL;
    }

    if ( this->image_data == NULL ) {
      this->image_data = (uint16_t*)new int16_t[ framepix ];
    }

    unsigned int ret = this->andor.get_last_frame( (uint16_t*)this->image_data );

    if ( ret == DRV_SUCCESS ) return NO_ERROR; else return ERROR;
  }
  /***** Acam::Camera::get_frame **********************************************/


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

    return( error );
  }
  /***** Acam::Camera::test_image *********************************************/


  /***** Acam::CameraServer::CameraServer *************************************/
  /**
   * @brief      default constructor
   *
   */
  CameraServer::CameraServer() {
    this->name = "AndorCameraServer";
    this->host = "";
    this->port = -1;
  }
  /***** Acam::CameraServer::CameraServer *************************************/


  /***** Acam::CameraServer::CameraServer *************************************/
  /**
   * @brief      default constructor
   * @param[in]  host
   * @param[in]  port
   *
   */
  CameraServer::CameraServer( std::string host, int port ) {
    this->name = "AndorCameraServer";
    this->host = host;
    this->port = port;
  }
  /***** Acam::CameraServer::CameraServer *************************************/


  /***** Acam::CameraServer::CameraServer *************************************/
  /**
   * @brief      default constructor
   * @param[in]  name  name for this component (info only)
   * @param[in]  host  host for this component
   * @param[in]  port  port number for this component
   *
   */
  CameraServer::CameraServer( std::string name, std::string host, int port ) {
    this->name = name;
    this->host = host;
    this->port = port;
  }
  /***** Acam::CameraServer::CameraServer *************************************/


  /***** Acam::CameraServer::~CameraServer ************************************/
  /**
   * @brief      default deconstructor
   *
   */
  CameraServer::~CameraServer( ) {
  }
  /***** Acam::CameraServer::~CameraServer ************************************/


  /***** Acam::CameraServer::open *********************************************/
  /**
   * @brief      open a connection to the acam server
   * @return     ERROR or NO_ERROR
   *
   */
  long CameraServer::open() {
    std::string function = "Acam::CameraServer::open";
    std::stringstream message;

    if ( this->server.isconnected() ) {
      logwrite( function, "connection already open" );
      return( NO_ERROR );
    }

    Network::TcpSocket s( this->host, this->port );
    this->server = s;

    // Initialize connection to the acam camera server
    //
    logwrite( function, "opening connection to external camera server" );

    if ( this->server.Connect() != 0 ) {
      logwrite( function, "ERROR connecting to external camera server" );
      return( ERROR );
    }

    message.str(""); message << "socket connection to "
                             << this->server.gethost() << ":" << this->server.getport()
                             << " established on fd " << this->server.getfd();
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Acam::CameraServer::open *********************************************/


  /***** Acam::CameraServer::close ********************************************/
  /**
   * @brief      close the connection to the acam camera server
   * @return     ERROR or NO_ERROR
   *
   */
  long CameraServer::close() {
    std::string function = "Acam::CameraServer::close";
    std::stringstream message;

    if ( !this->server.isconnected() ) {
      logwrite( function, "connection already closed" );
      return( NO_ERROR );
    }

    long error = this->server.Close();

    if ( error == NO_ERROR ) {
      logwrite( function, "connection to acam camera server closed" );
    }
    else {
      logwrite( function, "ERROR closing acam camera server connection" );
    }

    return( error );
  }
  /***** Acam::CameraServer::close ********************************************/


  /***** Acam::CameraServer::acquire ******************************************/
  /**
   * @brief      acquire an image from the acam camera server
   * @param[in]  wcsname    if provided, the name of a previous image with WCS info
   * @param[out] imagename  filename of the new image taken by the server
   * @return     ERROR or NO_ERROR
   *
   */
  long CameraServer::acquire( std::string wcsname, std::string &imagename ) {
    std::string function = "Acam::CameraServer::acquire";
    std::stringstream message;
    std::string cmd = "acquire";

    // Nothing to do if not connected
    //
    if ( !this->server.isconnected() ) {
      logwrite( function, "ERROR: no connection open to the acam camera server" );
      return( ERROR );
    }

    // Include the filename of a WCS-corrected file, if provided
    //
    if ( ! wcsname.empty() ) { cmd.append( " " ); cmd.append( wcsname ); }

    // Send command to the server here
    //
    long error = this->send_command( cmd, imagename );

    message.str(""); message << "[DEBUG] wcsname=" << wcsname << " imagename=" << imagename;
    logwrite( function, message.str() );

    return( error );
  }
  /***** Acam::CameraServer::acquire ******************************************/


  /***** Acam::CameraServer::coords *******************************************/
  /**
   * @brief      send coords to the camera server
   * @param[in]  coords_in  string containing coords to write
   * @return     ERROR or NO_ERROR
   *
   */
  long CameraServer::coords( std::string coords_in ) {
    std::string function = "Acam::CameraServer::coords";
    std::stringstream message;

#ifdef ACAM_ANDOR_SOURCE_SERVER
    // Nothing to do if not connected
    //
    if ( !this->server.isconnected() ) {
      logwrite( function, "ERROR: no connection open to the acam camera server" );
      return( ERROR );
    }

    // Check coords_in string
    //
    std::vector<std::string> tokens;
    Tokenize( coords_in, tokens, " " );
    if ( tokens.size() != 3 ) {
      logwrite( function, "ERROR expected 3 arguments: RA DEC PA" );
      return( ERROR );
    }

    // Otherwise send the command to the camera server
    //
    std::stringstream cmd;
    std::string reply;      // reply not used
    cmd << ACAMD_CAMERASERVER_COORDS << " " << coords_in;
    return this->send_command( cmd.str(), reply );
#elif ACAM_ANDOR_SOURCE_ANDOR
    logwrite( function, "not implemented for Andor" );
    return( ERROR );
#else
    logwrite( function, "ERROR: ACAM source not defined in CMakeLists.txt" );
    return( ERROR );
#endif
  }
  /***** Acam::CameraServer::coords *******************************************/


  /***** Acam::CameraServer::send_command *************************************/
  /**
   * @brief      send a command string to the camera server
   * @param[in]  cmd  string command
   * @return     ERROR or NO_ERROR
   *
   * The needed linefeed \n is added here
   *
   * This function is overloaded with a version that accepts a return string.
   * This version sends a command only and does not read back any reply.
   *
   */
  long CameraServer::send_command( std::string cmd ) {
    std::string function = "Acam::CameraServer::send_command";
    std::stringstream message;

    logwrite( function, cmd );

    cmd.append( "\n" );                        // add the newline character

    int written = this->server.Write( cmd );   // write the command

    if ( written <= 0 ) return( ERROR );       // return error if error writing to socket

    return( NO_ERROR );
  }
  /***** Acam::CameraServer::send_command *************************************/


  /***** Acam::CameraServer::send_command *************************************/
  /**
   * @brief      send a command string to the camera server
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply
   * @return     ERROR or NO_ERROR
   *
   * The needed linefeed \n is added here
   *
   * This function is overloaded.
   *
   * This version is called with a reference to return string, in which case 
   * after writing the command the reply is read and placed into the return
   * string.
   *
   */
  long CameraServer::send_command( std::string cmd, std::string &retstring ) {
    std::string function = "Acam::CameraServer::send_command";
    std::stringstream message;
    std::string reply;
    long error=NO_ERROR;
    long retval=0;

    // send the command
    //
    error = this->send_command( cmd );

    if ( error == ERROR ) {
      message.str(""); message << "ERROR sending command: " << cmd;
      logwrite( function, message.str() );
    }

    // read the reply
    //
    while ( error == NO_ERROR && retval >= 0 ) {

      if ( ( retval=this->server.Poll() ) <= 0 ) {
        if ( retval==0 ) { message.str(""); message << "TIMEOUT on fd " << this->server.getfd() << ": " << strerror(errno);
                           error = TIMEOUT; }
        if ( retval <0 ) { message.str(""); message << "ERROR on fd " << this->server.getfd() << ": " << strerror(errno);
                           error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      if ( ( retval = this->server.Read( reply, '\n' ) ) < 0 ) {
        message.str(""); message << "error reading from camera server: " << strerror( errno );
        logwrite( function, message.str() );
        break;
      }

      // remove any newline characters and get out
      //
      reply.erase(std::remove(reply.begin(), reply.end(), '\r' ), reply.end());
      reply.erase(std::remove(reply.begin(), reply.end(), '\n' ), reply.end());
      break;

    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] got back reply=" << reply; logwrite( function, message.str() );
#endif

    retstring = reply;

    return( error );
  }
  /***** Acam::CameraServer::send_command *************************************/


  /***** Acam::Astrometry::Astrometry *****************************************/
  /**
   * @brief      class constructor
   *
   */
  Astrometry::Astrometry() {
    std::string function = "Astrometry::Astrometry";
    std::stringstream message;

    if ( !this->py_instance.is_initialized() ) {
      logwrite( function, "ERROR could not initialize Python" );
      return;
    }

    CPyObject pModuleName;

    pModuleName             = PyUnicode_FromString( PYTHON_ASTROMETRY_MODULE );
    this->pAstrometryModule = PyImport_Import( pModuleName );

    pModuleName.Release();

    pModuleName           = PyUnicode_FromString( PYTHON_IMAGEQUALITY_MODULE );
    this->pQualityModule  = PyImport_Import( pModuleName );

    pModuleName.Release();

    if ( this->pAstrometryModule == NULL || this->pQualityModule == NULL ) {
      PyErr_Print();
      this->python_initialized = false;
      return;
    }
    else this->python_initialized = true;

    this->isacquire=true;

    logwrite( "Astrometry::Astrometry", "initialized Python astrometry module" );
    return;
  }
  /***** Acam::Astrometry::Astrometry *****************************************/


  /***** Acam::Astrometry::~Astrometry ****************************************/
  /**
   * @brief      class deconstructor
   *
   */
  Astrometry::~Astrometry() {
  }
  /***** Acam::Astrometry::~Astrometry ****************************************/


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

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] PyGILState_Check=" << PyGILState_Check(); logwrite( function, message.str() );
#endif

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return( ERROR );
    }

    if ( this->pQualityModule==NULL ) {
      logwrite( function, "ERROR: Python image_quality module not imported" );
      return( ERROR );
    }

    // Call the Python image_quality function here
    //
    PyObject* pFunction = PyObject_GetAttrString( this->pQualityModule, PYTHON_IMAGEQUALITY_FUNCTION );

    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR: Python image_quality function not callable" );
      return( ERROR );
    }

    PyObject* pReturn = PyObject_CallNoArgs( pFunction );

//  "airmass" can be passed as an keyword. So can "EXPOSURE_TIME" but as a rule, supply neither.
//  Instead, these will both come from the FITS header.
//
//  PyObject *kwargs     = PyDict_New(); PyDict_SetItemString( kwargs, "airmass", PyLong_FromLong(1) );
//  PyObject *pEmptyArgs = PyTuple_New(0);
//  PyObject *pReturn    = PyObject_Call( pFunction, pEmptyArgs, kwargs );

    // Check the return values from Python here
    //
    if ( ! pReturn ) {
      logwrite( function, "ERROR calling Python image_quality" );
      return( ERROR );
    }

    // Expected a Dictionary
    //
    if ( ! PyDict_Check( pReturn ) ) {
      logwrite( function, "ERROR: Python image_quality function did not return expected dictionary" );
      return( ERROR );
    }

    // Get the error from the returned dictionary keyword
    //
    PyObject *_error          = PyDict_GetItemString( pReturn, "ERROR" );
    const char *err           = PyUnicode_AsUTF8( _error );
    std::string errstr(err);

    // If there is any error then report it and do not proceed
    //
    if ( ! errstr.empty() ) {
      this->seeing            = -1;
      this->seeing_zenith     = -1;
      this->extinction        = -1;
      this->background_med    = -1;
      this->background_std    = -1;
      message.str(""); message << "ERROR from Python image_quality: " << errstr;
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Get the values out of the returned dictionary
    //
    PyObject *_seeing         = PyDict_GetItemString( pReturn, "seeing" );
    PyObject *_seeing_zenith  = PyDict_GetItemString( pReturn, "seeing_zenith" );
    PyObject *_extinction     = PyDict_GetItemString( pReturn, "extinction" );
    PyObject *_background_med = PyDict_GetItemString( pReturn, "background_med" );
    PyObject *_background_std = PyDict_GetItemString( pReturn, "background_std" );

    // Store them in the class
    //
    this->seeing              = PyFloat_AsDouble( _seeing );
    this->seeing_zenith       = PyFloat_AsDouble( _seeing_zenith );
    this->extinction          = PyFloat_AsDouble( _extinction );
    this->background_med      = PyFloat_AsDouble( _background_med );
    this->background_std      = PyFloat_AsDouble( _background_std );

    message.str("");
    message << "seeing=" << this->seeing
            << " seeing_zenith=" << this->seeing_zenith
            << " extinction=" << this->extinction
            << " background_med=" << this->background_med
            << " background_std=" << this->background_std;
    logwrite( function, message.str() );

    return( NO_ERROR );
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

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] PyGILState_Check=" << PyGILState_Check(); logwrite( function, message.str() );
#endif

    if ( !this->python_initialized ) {
      logwrite( function, "ERROR Python is not initialized" );
      return( ERROR );
    }

    if ( this->pAstrometryModule==NULL ) {
      logwrite( function, "ERROR: Python astrometry module not imported" );
      return( ERROR );
    }

    // Can't proceed unless there is an imagename
    //
    if ( imagename_in.empty() ) {
      logwrite( function, "ERROR: imagename cannot be empty" );
      return ERROR;
    }

    PyObject* pFunction = PyObject_GetAttrString( this->pAstrometryModule, PYTHON_ASTROMETRY_FUNCTION );

    const char* imagename = imagename_in.c_str();

    PyObject* pArgList = Py_BuildValue( "(s)", imagename );

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
#ifdef LOGLEVEL_DEBUG
          message.str(""); message << "[DEBUG] add solver arg keyword=" << keyval.at(0) << " value=" << keyval.at(1);
          logwrite( function, message.str() );
#endif
          PyDict_SetItemString( pKeywords, pkeyname, pvalue );
        }
      }
      catch( std::out_of_range const& ) {
        logwrite( function, "ERROR: out of range parsing key=value pair from arglist" );
        return ERROR;
      }
      catch( ... ) {
        logwrite( function, "ERROR unknown exception parsing key=value pair from arglist" );
        return ERROR;
      }
    }

    double t0=0, t1=0;

    // Call the Python astrometry function here
    //
    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR: Python astrometry function not callable" );
      return( ERROR );
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] calling Python function: " << PYTHON_ASTROMETRY_FUNCTION;
    logwrite( function, message.str() );
#endif

    t0=get_clock_time();
    PyObject* pReturn = PyObject_Call( pFunction, pArgList, pKeywords );
    Py_XDECREF( pFunction );
    t1=get_clock_time();

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] Python call time " << (t1-t0) << " sec";
    logwrite( function, message.str() );
#endif

    // Check the return values from Python here
    //
    if ( ! pReturn ) {
      logwrite( function, "ERROR calling Python astrometry solver" );
      return( ERROR );
    }

    // Expected a dictionary
    //
    if ( ! PyDict_Check( pReturn ) ) {
      logwrite( function, "ERROR: did not return expected dictionary" );
      return( ERROR );
    }

    // get the needed values from the returned dictionary
    //
    PyObject* result_   = PyDict_GetItemString( pReturn, "result" );
    PyObject* ra_       = PyDict_GetItemString( pReturn, "solveRA" );
    PyObject* dec_      = PyDict_GetItemString( pReturn, "solveDEC" );
    PyObject* pa_       = PyDict_GetItemString( pReturn, "solveANG" );

    // store them in the class
    //
    const char* cresult = PyUnicode_AsUTF8( result_ );
    this->result.assign( cresult );

    this->ra  = PyFloat_Check( ra_ )  ? PyFloat_AsDouble( ra_ )  : NAN;
    this->dec = PyFloat_Check( dec_ ) ? PyFloat_AsDouble( dec_ ) : NAN;
    this->pa  = PyFloat_Check( pa_ )  ? PyFloat_AsDouble( pa_ )  : NAN;

    message.str(""); message << "result=" << this->result
                             << std::fixed << std::setprecision(6)
                             << " RA="    << this->ra
                             << " DEC="   << this->dec
                             << " PA="    << this->pa
                             << " solve time=" << t1-t0 << " sec";
    logwrite( function, message.str() );

    return( NO_ERROR );
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


  /***** Acam::Interface::Interface *******************************************/
  /**
   * @brief      class constructor
   * @details    The andor camera will be initialized upon construction
   */
  Interface::Interface() {
    this->cameraserver_host="";
    this->motion_host="";
    this->cameraserver_port=-1;
    this->motion_port=-1;
    this->imagename="";
    this->wcsname="";
  }
  /***** Acam::Interface::Interface *******************************************/


  /***** Acam::Interface::~Interface ******************************************/
  /**
   * @brief      class deconstructor
   * @details    The andor camera will be closed upon de-construction
   *
   */
  Interface::~Interface() {
  }
  /***** Acam::Interface::~Interface ******************************************/


  /***** Acam::Interface::initialize_class ************************************/
  /**
   * @brief      initializes the class from configure_acam()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Acam::Server::configure_acam() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Acam::Interface::initialize_class";
    std::stringstream message;

#ifdef ACAM_ANDOR_SOURCE_SERVER
    if ( this->cameraserver_port < 0 || this->cameraserver_host.empty() ) {
      message.str(""); message << "ERROR: external camera server (" << this->cameraserver_host << ") or port (" 
                               << this->cameraserver_port << ") invalid";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Initialize the network interface to the external camera server
    //
    CameraServer s( this->cameraserver_host, this->cameraserver_port );
    this->camera_server = s;
#elif ACAM_ANDOR_SOURCE_ANDOR
    return( NO_ERROR );
#else
    logwrite( function, "ERROR: ACAM source not defined in CMakeLists.txt" );
    return( ERROR );
#endif
  }
  /***** Acam::Interface::initialize_class ************************************/


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

      if ( config.param[entry].find( "SOLVER_ARGS" ) == 0 ) {
        // tokenize each solverargs entry to check that it is of the format key=value
        //
        std::vector<std::string> tokens;
        int size = Tokenize( config.arg[entry], tokens, "=" );
        if (size==0) continue; else  // allows an empty entry but if anything is there then it has to be right
        if (size != 2) {
          message.str(""); message << "ERROR: bad entry for SOLVER_ARGS: " << config.arg[entry] << ": expected (key=value)";
          logwrite(function, message.str());
          error = ERROR;
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
            error = ERROR;
          }
        }
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
   * @param[in]  args  string containing 0 or more args specifying which component to open
   * @param[out] help  return string containing help on request  
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( std::string args, std::string &help ) {
    std::string function = "Acam::Interface::open";
    std::stringstream message;
    long error = NO_ERROR;
    std::vector<std::string> arglist;
    std::string component, camarg;

    // No args opens everything (motion and camera)...
    //
    if ( args.empty() ) {
      component = "all";
      camarg    = "";
    }
    else if ( args == "?" ) {
      help = ACAMD_OPEN;
      help.append( " [ [motion] [camera [args]] ]\n" );
      help.append( "  Open connections to all devices (by default).\n" );
      help.append( "  Optionally indicate motion | camera to open only the indicated component.\n" );
      help.append( "  The camera component can take an optional arg to pass to the camera.\n" );
      return( NO_ERROR );
    }
    else { // ...otherwise look at the arg(s):

      std::transform( args.begin(), args.end(), args.begin(), ::tolower );  // convert to lowercase

      Tokenize( args, arglist, " " );

      // args can be [ [motion] [camera [args]] ]
      //
      int ccount=0;
      for ( size_t i=0; i < arglist.size(); i++ ) {
        size_t next = i+1;
        if ( arglist[i] == "motion" ) {
          component = arglist[i]; ccount++;
          if ( next < arglist.size() && arglist[next] != "camera" ) {
            message.str(""); message << "ERROR: unrecognized arg \"" << arglist[next]
                                     << "\". Expected { [ [motion] [camera [args]] ] }";
            logwrite( function, message.str() );
            return( ERROR );
          }
        }
        if ( arglist[i] == "camera" ) {
          component = arglist[i]; ccount++;
          if ( next < arglist.size() && arglist[next] != "motion" ) {
            camarg.append( arglist[ next ] );
          }
        }
      }
      if ( ccount == 2 ) component = "all";

      message.str(""); message << "[NOTICE] component=" << component << " camarg=" << camarg;
      logwrite( function, message.str() );

      if ( component.empty() ) {
        message.str(""); message << "ERROR: unrecognized arg \"" << args
                                 << "\". Expected { [ [motion] [camera [args]] ] }";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] component=\"" << component << "\"  camarg=\"" << camarg << "\"";
    logwrite( function, message.str() );
#endif

    if ( component != "all" && component != "motion" && component != "camera" ) {
      message.str(""); message << "ERROR: unrecognized component \"" << component << "\". "
                               << "Expected { motion | camera }";
      logwrite( function, message.str() );
      return( ERROR );
    }

    if ( component == "all" || component == "motion" ) {
      error |= this->motion.open();
    }

    if ( component == "all" || component == "camera" ) {
#ifdef ACAM_ANDOR_SOURCE_SERVER
      error |= this->camera_server.open();   // get images from external server
#elif ACAM_ANDOR_SOURCE_ANDOR
      error |= this->camera.open( camarg );  // get images from Andor directly
#else
      logwrite( function, "ERROR: ACAM source not defined in CMakeLists.txt" );
      return( ERROR );
#endif
    }

    if ( error != NO_ERROR ) logwrite( function, "ERROR: one or more components failed to open" );

    return( error );
  }
  /***** Acam::Interface::open ************************************************/


  /***** Acam::Interface::isopen **********************************************/
  /**
   * @brief      wrapper for acam hardware components to check if connection open
   * @param[in]  component  optional string contains which component to check { camera | motion }
   * @param[out] state      reference to bool to indicate open {true} or not {false}
   * @param[out] help       reference to return string (used just for help)
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::isopen( std::string component, bool &state, std::string &help ) {
    std::string function = "Acam::Interface::isopen";
    std::stringstream message;
    state = true;               // default True, then "and" will make false if any one is false

    // Help
    //
    if ( component == "?" ) {
      help = ACAMD_ISOPEN;
      help.append( " [ camera | motion ]\n" );
      help.append( "  optionally supply the component name to check only that component\n" );
      help.append( "  checks all components if no arg supplied\n" );
      return( NO_ERROR );
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
      return( ERROR );
    }

    if ( component == "all" || component == "motion" ) {
      state &= this->motion.isopen();
    }

    if ( component == "all" || component == "camera" ) {
#ifdef ACAM_ANDOR_SOURCE_SERVER
      state &= this->camera_server.isopen();
#elif ACAM_ANDOR_SOURCE_ANDOR
      state &= this->camera.andor.is_initialized();
#else
      logwrite( function, "ERROR: ACAM source not defined in CMakeLists.txt" );
      return( ERROR );
#endif
    }

    return( NO_ERROR );
  }
  /***** Acam::Interface::isopen **********************************************/


  /***** Acam::Interface::close ***********************************************/
  /**
   * @brief      wrapper for all acam hardware components
   * @param[in]  component  optionally provide the component name to close { camera | motion }
   * @param[out] help       contains return string for help
   * @return     ERROR or NO_ERROR
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
      return( NO_ERROR );
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
      return( ERROR );
    }

    if ( component == "all" || component == "motion" ) {
//    error |= this->motion.send_command( "close" );  // just needed for the emulator
      error |= this->motion.close();
    }

    if ( component == "all" || component == "camera" ) {
#ifdef ACAM_ANDOR_SOURCE_SERVER
      error |= this->camera_server.close();
#elif ACAM_ANDOR_SOURCE_ANDOR
      error |= this->camera.close();
#else
      logwrite( function, "ERROR: ACAM source not defined in CMakeLists.txt" );
      return( ERROR );
#endif
    }

    return( error );
  }
  /***** Acam::Interface::close ***********************************************/


  /***** Acam::Interface::acquire *********************************************/
  /**
   * @brief      wrapper to acquire an Andor image
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::acquire( std::string args, std::string &retstring ) {
    std::string function = "Acam::Interface::acquire";
    std::stringstream message;
    long error = NO_ERROR;
    std::string _imagename = this->imagename;

    // Help
    //
    if ( args == "?" ) {
      retstring = ACAMD_ACQUIRE;
      retstring.append( "\n" );
      retstring.append( "  Acquire a single ACAM image.\n" );
      return( NO_ERROR );
    }

    // Set this true for acquisition (false for guiding)
    //
    this->astrometry.isacquire=true;

message << "[DEBUG] this->wcsnamne=" << this->wcsname; logwrite( function, message.str() );
#ifdef ACAM_ANDOR_SOURCE_SERVER
    this->camera_server.acquire( this->wcsname, _imagename );
    this->imagename = _imagename;
#elif ACAM_ANDOR_SOURCE_ANDOR
    this->camera.andor.acquire_one();
    this->camera.andor.save_acquired( this->wcsname, _imagename );
#else
    logwrite( function, "ERROR: ACAM source not defined in CMakeLists.txt" );
    return( ERROR );
#endif

    message.str(""); message << "[DEBUG] this->wcsname=" << this->wcsname << " this->imagename=" << this->imagename;
    logwrite( function, message.str() );

    return( error );
  }
  /***** Acam::Interface::acquire *********************************************/


  /***** Acam::Interface::exptime *********************************************/
  /**
   * @brief      wrapper to set/get exposure time
   * @param[in]  exptime_in  optional requested exposure time
   * @param[out] retstring   return string contains current exposure time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::exptime( std::string exptime_in, std::string &retstring ) {
    std::string function = "Acam::Interface::exptime";
    std::stringstream message;

    if ( exptime_in == "?" || exptime_in == "help" ) {
      retstring = ACAMD_EXPTIME;
      retstring.append( " [ <exptime> ]\n" );
      retstring.append( "  Set or get camera exposure time in decimal seconds.\n" );
      retstring.append( "  If <exptime> is provided then the camera exposure time is changed,\n" );
      retstring.append( "  else the current exposure time is returned.\n" );
      return( NO_ERROR );
    }

#ifdef ACAM_ANDOR_SOURCE_SERVER
    logwrite( function, "ERROR: not implemented for GUI server" );
    return( ERROR );
#elif ACAM_ANDOR_SOURCE_ANDOR
    return this->camera.andor.exptime( exptime_in, retstring );
#else
    logwrite( function, "ERROR: ACAM source not defined in CMakeLists.txt" );
    return( ERROR );
#endif
  }
  /***** Acam::Interface::exptime *********************************************/


  /***** Acam::Interface::image_quality ***************************************/
  /**
   * @brief      wrapper for Astrometry::image_quality()
   * @param[in]  args
   * @param[out] retstring
   * @return     ERROR or NO_ERROR
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
      return( NO_ERROR );
    }

    long error = this->astrometry.image_quality( );
    if ( error != NO_ERROR) this->async.enqueue( "ERROR: calling image_quality" );
    return( error );
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
   * @return     ERROR or NO_ERROR
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
      return( NO_ERROR );
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
    if ( _imagename.empty() ) _imagename = this->get_imagename();

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
    this->set_wcsname( _wcsname );

    return error;
  }
  /***** Acam::Interface::solve ***********************************************/


}
