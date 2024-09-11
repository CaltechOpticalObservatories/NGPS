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

    // no args is open both cameras in map
    //
    if ( which.empty() ) {
      long ret;
      for ( const auto &pair : this->andor ) {
        if ( !pair.second->is_initialized() ) ret = pair.second->open( args );
        if ( ret != NO_ERROR ) {
          message.str(""); message << "ERROR opening slicecam " << pair.second->camera_info.camera_name
                                   << " S/N " << pair.second->camera_info.serial_number;
          logwrite( function, message.str() );
          error = ret;  // preserve the error state for the return value but try all
        }
      }
    }
    else {
      // make sure requested camera is in the map
      //
      auto it = this->andor.find( which );
      if ( it == this->andor.end() ) {
        message.str(""); message << "ERROR Andor camera name \"" << which << "\" not found";
        logwrite( function, message.str() );
        return ERROR;
      }
      // then open only that camera
      //
      if ( !this->andor[which]->is_initialized() ) error = this->andor[which]->open( args );
      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR opening slicecam " << this->andor[which]->camera_info.camera_name
                                 << " S/N " << this->andor[which]->camera_info.serial_number;
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

    // loop through all Andors, opening if not already open
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

    return error;
  }
  /***** Slicecam::Camera::close **********************************************/


  /***** Slicecam::Camera::start_acquisition **********************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::start_acquisition() {
    return ERROR;
  }
  /***** Slicecam::Camera::start_acquisition **********************************/


  /***** Slicecam::Camera::get_status *****************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   * 
   */
  long Camera::get_status() {
    std::string function = "Slicecam::Camera::get_status";
    std::stringstream message;
    long error=NO_ERROR;

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      return ERROR;
    }

    // loop through all Andors, opening if not already open
    //
    for ( const auto &pair : this->andor ) {
      long ret = pair.second->get_status();
      if ( ret != NO_ERROR ) {
        message.str(""); message << "ERROR getting status from slicecam " << pair.second->camera_info.camera_name
                                 << " S/N " << pair.second->camera_info.serial_number;
        logwrite( function, message.str() );
        error = ret;  // preserve the error state for the return value
      }
    }

    return error;
  }
  /***** Slicecam::Camera::get_status *****************************************/


  /***** Slicecam::Camera::bin ************************************************/
  /**
   * @brief      set or get camera binning
   * @param[in]  args       optionally contains <hbin> <vbin>
   * @param[out] retstring  return string contains <hbin> <vbin>
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::bin( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Camera::bin";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_BIN;
      retstring.append( " [ <hbin> <vbin> ]\n" );
      retstring.append( "   Set or get CCD binning.\n" );
      retstring.append( "   <hbin> and <vbin> are the number of pixels to bin horizontally\n" );
      retstring.append( "   and vertically, respectively. When setting either, both must\n" );
      retstring.append( "   be supplied. If both omitted then the current binning is returned.\n" );
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
      if ( ! pair.second->is_initialized() ) {
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

      int hbin, vbin;

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // There must be two args <hbin> <vbin>
      //
      if ( tokens.size() != 2 ) {
        logwrite( function, "ERROR expected <hbin> <vbin>" );
        return ERROR;
      }

      // Parse the binning values from the tokens
      //
      try {
        hbin = std::stoi( tokens.at(0) );
        vbin = std::stoi( tokens.at(1) );

        // Set the binning parameters now in separate threads.
        // Use std::async to run the function asynchronously.
        //
        std::vector<std::future<long>> futures;

        for ( const auto &pair : this->andor ) {
          futures.push_back( std::async( std::launch::async, [pair, hbin, vbin]() {
            return pair.second->set_binning( hbin, vbin );
          } ) );
        }

        // Wait for all futures to complete
        //
        for ( auto &future : futures ) {
          error |= future.get(); // This will block until the thread finishes
        }
      }
      catch ( std::out_of_range &e ) {
        message.str(""); message << "ERROR reading arguments: " << e.what();
        error = ERROR;
      }
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR reading arguments: " << e.what();
        error = ERROR;
      }

      if ( error==ERROR ) logwrite( function, message.str() );
    }

    // return the current binning parameters
    //
    int hbin=0, vbin=0;
    for ( const auto &pair : this->andor ) {
      hbin = pair.second->camera_info.hbin;
      vbin = pair.second->camera_info.vbin;
    }
    message.str(""); message << hbin << " " << vbin;
    retstring = message.str();

    logwrite( function, retstring );

    return error;
  }
  /***** Slicecam::Camera::bin ************************************************/


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
      retstring.append( " [ <hflip> <vflip> ]\n" );
      retstring.append( "   Set or get CCD image flip.\n" );
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

    // all configured Andors must have been initialized
    //
    for ( const auto &pair : this->andor ) {
      if ( ! pair.second->is_initialized() ) {
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

      int hflip, vflip;

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // There must be two args <hflip> <vflip>
      //
      if ( tokens.size() != 2 ) {
        logwrite( function, "ERROR expected <hflip> <vflip>" );
        return ERROR;
      }

      // Parse the args from the tokens
      //
      try {
        hflip = std::stoi( tokens.at(0) );
        vflip = std::stoi( tokens.at(1) );

        // Set the flip parameters now in separate threads.
        // Use std::async to run the function asynchronously.
        //
        std::vector<std::future<long>> futures;

        for ( const auto &pair : this->andor ) {
          futures.push_back( std::async( std::launch::async, [pair, hflip, vflip]() {
            return pair.second->set_imflip( hflip, vflip );
          } ) );
        }

        // Wait for all futures to complete
        //
        for ( auto &future : futures ) {
          error |= future.get(); // This will block until the thread finishes
        }
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
    }

    // return the current image flip states
    //
    int hflip=-1, vflip=-1;
    for ( const auto &pair : this->andor ) {
      hflip = pair.second->camera_info.hflip;
      vflip = pair.second->camera_info.vflip;
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
      retstring.append( " [ <rotdir> ]\n" );
      retstring.append( "   Set CCD image rotation where <rotdir> is { none cw ccw }\n" );
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

    // all configured Andors must have been initialized
    //
    for ( const auto &pair : this->andor ) {
      if ( ! pair.second->is_initialized() ) {
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

      // Set the rotation now in separate threads.
      // Use std::async to run the function asynchronously.
      //
      std::vector<std::future<long>> futures;

      for ( const auto &pair : this->andor ) {
        futures.push_back( std::async( std::launch::async, [pair, rotdir]() {
          return pair.second->set_imrot( rotdir );
        } ) );
      }

      // Wait for all futures to complete
      //
      for ( auto &future : futures ) {
        error |= future.get(); // This will block until the thread finishes
      }
    }

    return error;
  }
  /***** Slicecam::Camera::imrot **********************************************/


  /***** Slicecam::Camera::exptime ********************************************/
  /**
   * @brief      wrapper to set/get exposure time
   * @param[in]  exptime_in  optional requested exposure time
   * @param[out] retstring   return string contains current exposure time
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Camera::exptime( std::string exptime_in, std::string &retstring ) {
    std::string function = "Slicecam::Camera::exptime";
    std::stringstream message;

    if ( exptime_in == "?" || exptime_in == "help" ) {
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
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // Set the rotation now in separate threads.
    // Use std::async to run the function asynchronously.
    //
    std::vector<std::future<long>> futures;

    for ( const auto &pair : this->andor ) {
      futures.push_back( std::async( std::launch::async, [pair, exptime_in, &retstring]() {
        return pair.second->set_exptime( exptime_in, retstring );
      } ) );
    }

    // Wait for all futures to complete
    //
    long error = NO_ERROR;
    for ( auto &future : futures ) {
      error |= future.get(); // This will block until the thread finishes
    }

    return error;
  }
  /***** Slicecam::Camera::exptime ********************************************/


  /***** Slicecam::Camera::exptime ********************************************/
  /**
   * @brief      wrapper to get exposure time as a double
   * @return     double exposure time
   *
   */
  double Camera::exptime() {
    std::string function = "Slicecam::Camera::exptime";
    std::string svalue;
    double dvalue=NAN;
    this->exptime( "", svalue );
    try {
      dvalue=std::stod( svalue );
    }
    catch( const std::exception &e ) {
      logwrite( "Slicecam::Camera::exptime", "ERROR "+std::string(e.what()) );
    }
    return dvalue;
  }
  /***** Slicecam::Camera::exptime ********************************************/


  /***** Slicecam::Camera::gain ***********************************************/
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
  long Camera::gain( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Camera::gain";
    std::stringstream message;
    long error = NO_ERROR;
    int gain = -999;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_GAIN;
      retstring.append( " [ <gain> ]\n" );
      retstring.append( "   Set or get CCD Gain.\n" );
      if ( !this->andor.empty() ) {
        if ( this->andor.begin()->second->is_initialized() ) {
          int low, high;
          this->andor.begin()->second->get_emgain_range( low, high );
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
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // all configured Andors must have been initialized
    //
    for ( const auto &pair : this->andor ) {
      if ( ! pair.second->is_initialized() ) {
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

      int low, high;
      this->andor.begin()->second->get_emgain_range( low, high );

      // set gain and output amplifier as necessary
      //
      message.str("");
      if ( error==NO_ERROR && gain < 1 ) {
        message << "ERROR: gain " << gain << " outside range { 1, " << low << ":" << high << " }";
        error = ERROR;
      }
      else
      if ( error==NO_ERROR && gain == 1 ) {
        error = this->andor.begin()->second->set_output_amplifier( Andor::AMPTYPE_CONV );
        if (error==NO_ERROR) {
          for ( const auto &pair : this->andor ) {
            pair.second->camera_info.gain = gain;
          }
        }
        else { message << "ERROR gain not set"; }
      }
      else
      if ( error==NO_ERROR && gain >= low && gain <= high ) {
        for ( const auto &pair : this->andor ) {
          error |= pair.second->set_output_amplifier( Andor::AMPTYPE_EMCCD );
          if (error==NO_ERROR) pair.second->set_emgain( gain );
          if (error==NO_ERROR) pair.second->camera_info.gain = gain;
          else { message << "ERROR gain not set"; }
        }
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
    if ( this->andor.begin()->second->camera_info.gain < 1 ) {
      retstring = "ERR";
    }
    else {
      retstring = std::to_string( this->andor.begin()->second->camera_info.gain );
    }

    logwrite( function, retstring );

    return error;
  }
  /***** Slicecam::Camera::gain ***********************************************/


  /***** Slicecam::Camera::gain ***********************************************/
  /**
   * @brief      wrapper to get exposure time as a double
   * @return     double exposure time
   *
   */
  int Camera::gain() {
    std::string function = "Slicecam::Camera::gain";
    std::string svalue;
    int ivalue=0;
    this->gain( "", svalue );
    try {
      ivalue=std::stoi( svalue );
    }
    catch( std::out_of_range &e )     { logwrite( "Slicecam::Camera::gain", "ERROR "+std::string(e.what()) ); }
    catch( std::invalid_argument &e ) { logwrite( "Slicecam::Camera::gain", "ERROR "+std::string(e.what()) ); }
    return ivalue;
  }
  /***** Slicecam::Camera::gain ***********************************************/


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

logwrite( function, "ERROR not yet implemented" ); return ERROR;
/***
    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_SPEED;
      retstring.append( " [ <hori> <vert> ]\n" );
      retstring.append( "   Set or get CCD clocking speeds for horizontal <hori> and vertical <vert>\n" );
      retstring.append( "   If speeds are omitted then the current speeds are returned.\n" );
      if ( this->andor.is_initialized() ) {
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

    // If the STL map of Andors is empty then something went wrong
    // with the configuration.
    //
    if ( this->andor.empty() ) {
      logwrite( function, "ERROR no cameras defined" );
      retstring="bad_config";
      return ERROR;
    }

    // Andor must be connected
    //
    if ( !this->andor.is_initialized() ) {
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
***/
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

logwrite( function, "ERROR not yet implemented" ); return ERROR;
/***
    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_TEMP;
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

    // Andor must be connected
    //
    if ( !this->andor.is_initialized() ) {
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
***/
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

    // Nothing to do if no Andor image data
    //
    for ( const auto &pair : this->andor ) {
      if ( pair.second->get_image_data() == nullptr ) {
        message.str(""); message << "ERROR no image data available for slicecam "
                                 << pair.second->camera_info.camera_name;
        logwrite( function, message.str() );
        return ERROR;
      }
    }

    fitsinfo.fits_name = "/tmp/slicecam.fits";
    fitsinfo.datatype = USHORT_IMG;

    fits_file.copy_info( fitsinfo );      // copy from fitsinfo to the fits_file class

    error = fits_file.open_file();        // open the fits file for writing

    if ( !source_file.empty() ) {
      if (error==NO_ERROR) error = fits_file.copy_header_from( source_file );
    }
    else {
      if (error==NO_ERROR) error = fits_file.create_header();                  // create basic header
    }

    for ( const auto &pair : this->andor ) {
      pair.second->camera_info.section_size = pair.second->camera_info.axes[0] * pair.second->camera_info.axes[1];
      if ( pair.second->camera_info.section_size == 0 ) {
        message.str(""); message << "ERROR section size 0 for slicecam " << pair.second->camera_info.camera_name;
        logwrite( function, message.str() );
        error = ERROR;
        break;
      }
      fits_file.write_image( pair.second );                    // write the image data
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

      if ( config.param[entry] == "ANDOR" ) {
        std::vector<std::string> tokens;
        Tokenize( config.arg[entry], tokens, " " );
        if ( tokens.size() != 3 ) {
          message.str(""); message << "ERROR invalid args ANDOR=\"" << config.arg[entry] << "\" : expected <NAME> <SN> <SCALE>";
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
          const std::string idx   = tokens.at(0);                          // name of this camera is map index

          if ( tokens.at(1) == "emulate" ) {
            this->camera.andor[idx] = std::make_shared<Andor::Interface>(  tokens.at(0)=="L" ? 1001 : 1002  );  // create an Andor::Interface pointer in the map
          }
          else {
            this->camera.andor[idx] = std::make_shared<Andor::Interface>();  // create an Andor::Interface pointer in the map
          }

          int sn;
          if ( tokens.at(1) == "emulate" ) {
            sn=( tokens.at(0)=="L" ? 1 : 2 );
            this->camera.andor[idx]->andor_emulator( true );
          }
          else {
            sn=std::stoi( tokens.at(1) );
            this->camera.andor[idx]->andor_emulator( false );
          }
          this->camera.andor[idx]->camera_info.camera_name   = idx;
          this->camera.andor[idx]->camera_info.serial_number = sn;
          this->camera.andor[idx]->camera_info.pixel_scale   = std::stod( tokens.at(2) );
        }
        catch( std::invalid_argument &e ) {
          message.str(""); message << "ERROR parsing \"" << config.arg[entry] << "\": " << e.what();
          logwrite( function, message.str() );
          error |= ERROR;
        }
        catch( std::out_of_range &e ) {
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
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR invalid TCSD_PORT " << config.arg[entry] << ": " << e.what();
          logwrite( function, message.str() );
          return(ERROR);
        }
        catch ( std::out_of_range &e ) {
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

    long error;

    if ( this->camera.open( which, args ) == NO_ERROR ) {  // open the camera
      error |= this->framegrab( "start", retstring );      // start frame grabbing if open succeeds
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
        state |= pair.second->is_initialized();
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
      state  = this->camera.andor[which]->is_initialized();
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
   * @brief      wrapper for all slicecam hardware components
   * @param[in]  component  optionally provide the component name to close { camera | motion }
   * @param[out] help       contains return string for help
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::close( std::string component, std::string &help ) {
    std::string function = "Slicecam::Interface::close";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( component == "?" ) {
      help = SLICECAMD_CLOSE;
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

    if ( component == "all" || component == "camera" ) {
      std::string dontcare;
      error |= this->framegrab( "stop", dontcare );
      error |= this->camera.close();
    }

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
        catch( std::exception &e ) {
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
      this->tcs_online.store( true );
    }
    else {
      this->tcs_online.store( false );
    }

    return error;
  }
  /***** Slicecam::Interface::tcs_init ****************************************/


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
      retstring.append( " [ start | stop | one [ <filename> ] | status ]\n" );
      retstring.append( "   Start/Stop continuous frame grabbing or grab one single ACAM image.\n" );
      retstring.append( "   If an optional <filename> is supplied then that file will be used\n" );
      retstring.append( "   as a source for header information for the frame.\n" );
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

    if ( whattodo == "one" ) {
      // Clear should_framegrab_run which means the framegrab loop should not run.
      // If it's already running then return, the existing framegrab loop will
      // stop. If it's not already running then drop through, and a single
      // frame will be grabbed.
      //
      this->should_framegrab_run.store( false );
      if ( this->is_framegrab_running.load() ) return;
    }
    else
    if ( whattodo == "start" ) {
      if ( this->is_framegrab_running.load() ) {
        logwrite( function, "thread already running, exiting" );
        return;
      }
      else {
        logwrite( function, "set thread running" );
        this->should_framegrab_run.store( true );
      }
    }
    else
    if ( whattodo == "stop" ) {
      logwrite( function, "set thread to stop, exiting" );
      this->should_framegrab_run.store( false );
      return;
    }
    else {
      message.str(""); message << "ERROR invalid argument \"" << whattodo << "\". exiting.";
      logwrite( function, message.str() );
      return;
    }

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
    std::unique_lock<std::mutex> lock( this->framegrab_mutex, std::defer_lock );             // try to get the mutex
    if ( lock.try_lock() ) {
      BoolState loop_running( this->is_framegrab_running );    // sets state true here, clears when it goes out of scope
      do {
        std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );                       // don't use too much CPU

        // they will both have the same exptime so just get the first one
        //
        double exptime=0;
        if ( !this->camera.andor.empty() ) {
          exptime = this->camera.andor.begin()->second->camera_info.exposure_time;
        }
        if ( exptime == 0 ) continue;                                       // wait for non-zero exposure time

        if (error==NO_ERROR) error = this->acquire_one_threaded();          // grab frame from both cameras
        if (error==NO_ERROR) error = this->collect_header_info_threaded();  // collect header info for both cameras
        if (error==NO_ERROR) error = this->camera.write_frame( sourcefile,
                                                               this->imagename,
                                                               this->tcs_online.load() );    // write to FITS file

        this->framegrab_time = std::chrono::steady_clock::time_point::min();

        this->gui_manager.push_gui_image( this->imagename );                                 // send frame to GUI

      } while ( error==NO_ERROR && this->should_framegrab_run.load() );
    }
    else {                                                                                   // this shouldn't even happen
      logwrite( function, "ERROR another thread is already running" );
      return;
    }
    }

    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR starting thread" );
      this->should_framegrab_run.store( false );  // disable frame grabbing
    }
    else logwrite( function, "leaving thread" );

    return;
  }
  /***** Slicecam::Interface::dothread_framegrab ******************************/


  /***** Slicecam::Interface::acquire_one_threaded ****************************/
  /**
   * @brief      calls acquire_one for each andor map element
   * @details    Loops through the andor STL map and spawns a thread for each
   *             call, then waits for all threads to complete.
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::acquire_one_threaded() {

    // create a container to hold the threads
    //
    std::vector<std::thread> threads;

    // Loop through all elements in the andor map and create an acquire_one thread for each.
    // The first arg specifies the member function pointer for acquire_one, and
    // the second arg is the instance on which acquire_one should be called.
    //
    for ( const auto &pair : this->camera.andor ) {
      threads.emplace_back( &Andor::Interface::acquire_one, pair.second );
    }

    // Wait for all threads to complete by joining them to this main thread,
    // not to each other.
    //
    for ( auto &thread : threads ) {
      if ( thread.joinable() ) {
        thread.join();
      }
    }

    // Loop again to see if any acquie_one threads had an error. This doesn't
    // need threads because it's only checking a state, so it's fast.
    //
    long error = NO_ERROR;
    for ( const auto &pair : this->camera.andor ) {
      error |= pair.second->read_error();
    }

    return error;
  }
  /***** Slicecam::Interface::acquire_one_threaded ****************************/


  /***** Slicecam::Interface::collect_header_info_threaded ********************/
  /**
   * @brief      calls collect_header_info for each andor map element
   * @details    Loops through the andor STL map and spawns a thread for each
   *             call, then waits for all threads to complete.
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::collect_header_info_threaded() {

    // create a container to hold the threads
    //
    std::vector<std::thread> threads;

    // Loop through all elements in the andor map and create a
    // collect_header_info thread for each, passing the function, "this"
    // and the pointer to the andor.
    //
    for ( const auto &pair : this->camera.andor ) {
      threads.emplace_back( &Slicecam::Interface::collect_header_info, this, pair.second );
    }

    // Wait for all threads to complete by joining them to this main thread,
    // not to each other.
    //
    for ( auto &thread : threads ) {
      if ( thread.joinable() ) {
        thread.join();
      }
    }

    return( this->read_error() );
  }
  /***** Slicecam::Interface::collect_header_info_threaded ********************/


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
   * @param[in]  args       <empty> or contains all: <exptime> <gain>
   * @param[out] retstring  return string contains <exptime> <gain>
   * @return     ERROR | NO_ERROR | HELP
   *
   * This function is overloaded
   *
   */
  long Interface::gui_settings_control( std::string args, std::string &retstring ) {
    std::string function = "Slicecam::Interface::gui_settings_control";
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLICECAMD_GUISET;
      retstring.append( " [ <exptime> <gain> ]\n" );
      retstring.append( "   Set or get settings for SAOImage GUI display.\n" );
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
    if ( !tokens.empty() && tokens.size() != 2 ) {
      message.str(""); message << "ERROR received " << tokens.size() << " arguments "
                               << "but expected <exptime> <gain>";
      logwrite( function, message.str() );
      retstring="invalid_argument_list";
      return ERROR;
    }

    // Get the original exptime and gain
    //
    double exptime_og = camera.exptime();
    int gain_og = camera.gain();

    long error = NO_ERROR;
    bool set = false;

    // If all args are supplied then set all parameters
    //
    if ( tokens.size() == 4 ) {
      try {
        std::string reply;

        // set the exposure time here
        error |= camera.exptime( tokens.at(0), reply );

        // set the gain here
        error |= camera.gain( tokens.at(1), reply );

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

    // Set or not, now read the current values and use the gui_manager
    // to set them in the class.
    //
    gui_manager.exptime = camera.exptime();
    gui_manager.gain = camera.gain();

    // If read-only (not set) or either exptime or gain changed, then set the flag for updating the GUI.
    //
    if ( !set || gui_manager.exptime != exptime_og || gui_manager.gain != gain_og ) {
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
   * fpoffsets <from> <to> <ra> <dec> <angle>
   * adchans
   * emgainrange
   * getemgain
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
      retstring.append( "   sleep\n" );
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
    if ( testname == "threadoffset" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = SLICECAMD_TEST;
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
    if ( testname == "adchans" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = SLICECAMD_TEST;
        retstring.append( " adchans\n" );
        retstring.append( "  Calls Andor SDK wrapper to return the number of AD converter channels\n" );
        retstring.append( "   available.\n" );
        error=HELP;
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
  /***** Slicecam::Interface::test ********************************************/


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
  long Interface::collect_header_info( std::shared_ptr<Andor::Interface> slicecam ) {
    std::string function = "Slicecam::Interface::collect_header_info";
    std::stringstream message;

    bool _tcs = this->tcs_online.load();
    std::string tcsname;

    // Check every time for the TCS because we don't ever want to save
    // an image with ambiguous keywords.
    //
    if ( _tcs && this->tcsd.client.is_open() ) {
      this->tcsd.get_name( tcsname );
    }
    else {
      tcsname = "offline";
      this->tcs_online.store( false );
      _tcs = this->tcs_online.load();
    }

    double angle_scope=NAN, ra_scope=NAN, dec_scope=NAN,
           angle_slit=NAN,  ra_slit=NAN,  dec_slit=NAN;

    // Get the current pointing from the TCS
    //
    if ( _tcs ) this->tcsd.get_cass( angle_scope );
    if ( _tcs ) this->tcsd.get_coords( ra_scope, dec_scope );  // returns RA in decimal hours, DEC in decimal degrees

    // Compute FP offsets from TCS coordinates (SCOPE) to SLIT coodinates.
    // compute_offset() always wants degrees and get_coords() returns RA hours.
    // Results in degrees.
    //
    if ( _tcs ) this->fpoffsets.compute_offset( "SCOPE", "SLIT", (ra_scope*TO_DEGREES), dec_scope, angle_scope,
                                                                             ra_slit, dec_slit, angle_slit );

    // Get some info from the Andor::Information class,
    // which is stored in its camera_info object.
    //
    std::copy( std::begin( slicecam->camera_info.axes ),
                 std::end( slicecam->camera_info.axes ),
               std::begin( this->camera.fitsinfo.axes ) );

    // Add information to the Camera::FitsInfo::FitsKeys database
    // either a prioi or from the Andor::Information class
    //
    slicecam->fitskeys.erase_db();

    slicecam->fitskeys.addkey( "BOB",  "hello", "world" );

    slicecam->fitskeys.addkey( "TCS",  tcsname, "" );

    slicecam->fitskeys.addkey( "CREATOR",  "slicecamd", "file creator" );
    slicecam->fitskeys.addkey( "INSTRUME", "NGPS", "name of instrument" );
    slicecam->fitskeys.addkey( "TELESCOP", "P200", "name of telescope" );

    slicecam->fitskeys.addkey( "EXPSTART", slicecam->camera_info.timestring, "exposure start time" );
    slicecam->fitskeys.addkey( "MJD0",     slicecam->camera_info.mjd0, "exposure start time (modified Julian Date)" );
    slicecam->fitskeys.addkey( "EXPTIME",  slicecam->camera_info.exposure_time, "exposure time (sec)" );
    slicecam->fitskeys.addkey( "SERNO",    slicecam->camera_info.serial_number, "camera serial number" );
    slicecam->fitskeys.addkey( "NAME",     slicecam->camera_info.camera_name, "camera name" );
    slicecam->fitskeys.addkey( "READMODE", slicecam->camera_info.readmodestr, "read mode" );
    slicecam->fitskeys.addkey( "TEMPSETP", slicecam->camera_info.setpoint, "detector temperature setpoint deg C" );
    slicecam->fitskeys.addkey( "TEMPREAD", slicecam->camera_info.ccdtemp, "CCD temperature deg C" );
    slicecam->fitskeys.addkey( "TEMPSTAT", slicecam->camera_info.temp_status, "CCD temperature status" );
    slicecam->fitskeys.addkey( "FITSNAME", slicecam->camera_info.fits_name, "this filename" );
    slicecam->fitskeys.addkey( "HBIN",     slicecam->camera_info.hbin, "horizontal binning pixels" );
    slicecam->fitskeys.addkey( "VBIN",     slicecam->camera_info.vbin, "vertical binning pixels" );
    slicecam->fitskeys.addkey( "HSPEED",   slicecam->camera_info.hspeed, "horizontal clocking speed MHz" );
    slicecam->fitskeys.addkey( "VSPEED",   slicecam->camera_info.vspeed, "vertical clocking speed MHz" );
    slicecam->fitskeys.addkey( "AMPTYPE",  slicecam->camera_info.amptypestr, "CCD amplifier type" );
    slicecam->fitskeys.addkey( "CCDGAIN",  slicecam->camera_info.gain, "CCD amplifier gain" );

    slicecam->fitskeys.addkey( "GAIN",     1, "e-/ADU" );

    slicecam->fitskeys.addkey( "PIXSCALE",  slicecam->camera_info.pixel_scale, "arcsec per pixel" );
    slicecam->fitskeys.addkey( "POSANG",    angle_slit, "" );
    slicecam->fitskeys.addkey( "TELRA",     ra_scope, "Telecscope Right Ascension hours" );
    slicecam->fitskeys.addkey( "TELDEC",    dec_scope, "Telescope Declination degrees" );
    slicecam->fitskeys.addkey( "CASANGLE",  angle_scope, "Cassegrain ring angle" );
    slicecam->fitskeys.addkey( "AIRMASS",   1, "" );
    slicecam->fitskeys.addkey( "WCSAXES",   2, "" );
    slicecam->fitskeys.addkey( "RADESYSA",  "ICRS", "" );
    slicecam->fitskeys.addkey( "CTYPE1",    "RA---TAN", "" );
    slicecam->fitskeys.addkey( "CTYPE2",    "DEC--TAN", "" );


    slicecam->fitskeys.addkey( "CRPIX1",    ( slicecam->camera_info.camera_name=="L" ? slicecam->camera_info.hend : 0 ), "" );
    slicecam->fitskeys.addkey( "CRPIX2",    slicecam->camera_info.vend/2, "" );

    slicecam->fitskeys.addkey( "CRVAL1",    ra_slit, "" );
    slicecam->fitskeys.addkey( "CRVAL2",    dec_slit, "" );
    slicecam->fitskeys.addkey( "CUNIT1",    "deg", "" );
    slicecam->fitskeys.addkey( "CUNIT2",    "deg", "" );

    slicecam->fitskeys.addkey( "CDELT1",    slicecam->camera_info.pixel_scale/3600., "" );
    slicecam->fitskeys.addkey( "CDELT2",    slicecam->camera_info.pixel_scale/3600., "" );

    slicecam->fitskeys.addkey( "PC1_1",     ( -1.0 * cos( angle_slit * PI / 180. ) ), "" );
    slicecam->fitskeys.addkey( "PC1_2",     (        sin( angle_slit * PI / 180. ) ), "" );
    slicecam->fitskeys.addkey( "PC2_1",     (        sin( angle_slit * PI / 180. ) ), "" );
    slicecam->fitskeys.addkey( "PC2_2",     (        cos( angle_slit * PI / 180. ) ), "" );

    return NO_ERROR;
  }
  /***** Slicecam::Interface::collect_header_info *****************************/

}
