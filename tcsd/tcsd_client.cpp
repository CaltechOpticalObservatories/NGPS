/** ---------------------------------------------------------------------------
 * @file    tcsd_client.cpp
 * @brief   code for TcsDaemonClient
 * @details TcsDaemonClient class is used to create a client for communicating
 *          with the TCS Daemon.
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "tcsd_client.h"


  /***** TcsDaemonClient::init ************************************************/
  /**
   * @brief      initializes the tcs system for control from the client
   * @param[in]  which  which TCS to initialize, real|sim. Check-only if empty.
   * @details    If the "which" parameter is empty then this thread cannot
   *             actually initialize the TCS but will only check if it has
   *             already been initialized. Successful initialization requires
   *             that the which param has been specified {real|sim} to indicate
   *             which TCS device to initialize.
   *
   *             The named devices {real,sim} must have been defined in the
   *             configuration file.
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long TcsDaemonClient::init( std::string_view which, std::string &retstring ) {
    std::string function = "TcsDaemonClient::init";
    std::stringstream message;
    std::string reply, tcsname;
    long error=NO_ERROR;
    bool connected_to_tcs=false;     // has tcsd opened a connection to the TCS?

    // Help
    //
    if ( which == "?" || which == "help" ) {
      retstring = "tcsinit [ sim | real | shutdown ]\n";
      retstring.append( "   Initialize connection to tcs daemon (tcsd) and open the specified TCS.\n" );
      retstring.append( "   If no arg specified then return connection status: sim | real | offline\n" );
      return HELP;
    }

    // Syntax check -- if something is supplied then it must be real|sim
    //
    if ( !which.empty() && which != "real" && which != "sim" && which != "shutdown" ) {
      logwrite( function, "ERROR expected real | sim | shutdown | <empty>" );
      retstring="invalid_argument";
      return ERROR;
    }

    // If not already connected to the tcs daemon then try to connect
    //
    if ( !this->client.socket.isconnected() ) {
      logwrite( function, "connecting to tcsd" );
      error = this->client.connect();                    // connect to the daemon
      if ( error != NO_ERROR ) {
        logwrite( function, "ERROR connecting to tcsd" );
        return ERROR;
      }
    }

    logwrite( function, "connected to tcsd" );

    // Is tcsd connected to the TCS, either real|sim ?
    //
    // The response will be "true|false|ERROR"
    // if connected.
    //
    error = this->client.send( TCSD_ISOPEN, reply );
    // Daemon is connected but an error sending ISOPEN to the TCS.
    // Close connection to daemon.
    //
    if ( error != NO_ERROR || reply.find( "ERROR" ) != std::string::npos ) {
      logwrite( function, "ERROR: tcsd unable to communicate with TCS. Closing daemon connection." );
      this->client.disconnect();
      retstring="offline";
      return ERROR;
    }

    if ( reply.find( "false" ) != std::string::npos ) {
      connected_to_tcs = false;
    }
    else
    if ( reply.find( "true" ) != std::string::npos ) {
      connected_to_tcs = true;
    }
    else {
      message.str(""); message << "ERROR invalid reply \"" << reply << "\" from tcsd. Expected true|false DONE";
      logwrite( function, message.str() );
      this->client.disconnect();
      retstring="offline";
      return ERROR;
    }

    // Already checked for error so the reply will be either "true|false DONE"
    // and I want to extract the state true|false so remove the " DONE" from the reply.
    // std::string::erase can throw an exception
    //
    std::string::size_type pos;
    pos=reply.find(" DONE");
    if (pos != std::string::npos) {
      reply.erase(pos);
    }
    else {
      message.str(""); message << "ERROR invalid reply \"" << reply << "\" from tcsd. Expected true|false DONE";
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( !connected_to_tcs ) {
      logwrite( function, "tcsd not connected to TCS" );
      tcsname = "offline";
    }
    else {
      // reply is "true DONE" here, and this removes the "true " and " DONE"
      //
      error = this->client.send( TCSD_GET_NAME, tcsname );
      pos = tcsname.find(" DONE");
      if (pos != std::string::npos) {
        tcsname.erase(pos);
      }
    }

    // disconnect from tcsd
    //
    if ( which == "shutdown" ) {
      this->client.disconnect();
      return error;
    }

    // if a device is specified and the currently opened device is not the
    // requested device then close it
    //
    if ( !which.empty() && tcsname != which ) {
      error = this->client.send( TCSD_CLOSE, reply );
      connected_to_tcs = false;
      message.str(""); message << "closed connection to TCS " << tcsname;
      logwrite( function, message.str() );
    }
    else if ( !which.empty() && tcsname == which ) {
      connected_to_tcs = true;
      message.str(""); message << "connection open to TCS " << which;
      logwrite( function, message.str() );
      return error;
    }

    // otherwise attempt to connect
    //
    if ( error==NO_ERROR && !connected_to_tcs ) {
      logwrite( function, "connecting to TCS "+std::string(which) );
      std::stringstream opencmd;
      opencmd << TCSD_OPEN << " " << which;
      error = this->client.send( opencmd.str(), reply );
      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR:connecting to " << which << " TCS hardware";
        logwrite( function, message.str() );
      }
      else connected_to_tcs = true;  // don't need to ask again because if TCSD_OPEN returned no error then it's open
    }

    // atomically set thr_error so the main thread knows we had an error
    //
    if ( error != NO_ERROR ) {
      logwrite( function, "ERROR unable to initialize TCS control" );
      retstring="connection_failed";
    }
    else {
      retstring=which;
    }

    return error;
  }
  /***** TcsDaemonClient::init ************************************************/


  /***** TcsDaemonClient::get_name ********************************************/
  /**
   * @brief      ask tcsd for the name of the connected TCS
   * @param[out] name  name of tcs
   * @return     ERROR | NO_ERROR
   *
   */
  long TcsDaemonClient::poll_name( std::string &name ) {
    return get_name( name, true );
  }
  long TcsDaemonClient::get_name( std::string &name ) {
    return get_name( name, false );
  }
  long TcsDaemonClient::get_name( std::string &name, bool poll ) {
    std::string function = "TcsDaemonClient::get_name";
    std::stringstream message;

    // Prepending "poll" onto the command will prevent the server from logging the command
    //
    std::stringstream tcscmd;
    tcscmd << ( poll ? "poll " : "" ) << TCSD_GET_NAME;  // optional "poll" prevents excessive logging by tcsd

    // Ask for the name of the TCS.
    // Response is expected to be "<name> DONE"
    //
    if ( this->client.send( tcscmd.str(), name ) != NO_ERROR ) {
      logwrite( function, "ERROR reading TCS name" );
      name="error";
      return ERROR;
    }

    // Remove the " DONE" from the reply, which becomes the return string
    //
    try {
      auto pos = name.find( " DONE" );
      if ( pos != std::string::npos ) {
        name.erase( pos );
      }
      else {
        message.str(""); message << "ERROR reading TCS name: \"" << name << "\"";
        logwrite( function, message.str() );
        name="error";
        return ERROR;
      }
    }
    catch( std::exception &e ) {
      message.str(""); message << "ERROR invalid reply \"" << name << "\" from tcsd: " << e.what();
      logwrite( function, message.str() );
      name="error";
      return ERROR;
    }
    return NO_ERROR;
  }
  /***** TcsDaemonClient::get_name ********************************************/


  /***** TcsDaemonClient::get_cass ********************************************/
  /**
   * @brief      read the current TCS cass angle
   * @param[out] cass  cass angle in degrees
   * @return     ERROR or NO_ERROR
   *
   */
  long TcsDaemonClient::poll_cass( double &cass ) {
    return this->get_cass( cass, true );
  }
  long TcsDaemonClient::get_cass( double &cass ) {
    return this->get_cass( cass, false );
  }
  long TcsDaemonClient::get_cass( double &cass, bool poll ) {
    std::string function = "TcsDaemonClient::get_cass";
    std::stringstream message;
    std::string tcsreply;
    std::stringstream tcscmd;

    // Prepending "poll" onto the command will prevent the server from logging the command
    //
    tcscmd << ( poll ? "poll " : "" ) << TCSD_GET_CASS;  // optional "poll" prevents excessive logging by tcsd

    // Send the command to tcsd to read the focus into tcsreply
    //
    if ( this->client.send( tcscmd.str(), tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR reading TCS cass angle" );
      cass = NAN;
      return ERROR;
    }

    // tcsreply will be " ddd.dd DONE" or "ERROR" so tokenize on space
    // to get the angle
    //
    std::vector<std::string> tokens;
    Tokenize( tcsreply, tokens, " " );              // comes back as space-delimited string "hh:mm:ss.ss dd:mm:ss.ss"

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR parsing angle from TCS reply \"" << tcsreply << "\"";
      logwrite( function, message.str() );
      return ERROR;
    }

    try {                                              // extract ra dec from coordstring
      cass = std::stod( tokens.at(0) );
    }
    catch( const std::out_of_range &e ) {
      message << "ERROR parsing tcsreply \"" << tcsreply << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( !poll ) {
      message.str(""); message << "currrent cass ring angle = " << cass;
      logwrite( function, message.str() );
    }

    return NO_ERROR;
  }
  /***** TcsDaemonClient::get_cass ********************************************/


  /***** TcsDaemonClient::pt_offset *******************************************/
  /**
   * @brief      send command to tcsd to send guider offsets
   * @param[in]  ra_d   RA in degrees
   * @param[in]  dec_d  DEC in degrees
   * @param[in]  rate   offset rate has default value if not specified
   * @return     ERROR | NO_ERROR
   *
   */
  long TcsDaemonClient::pt_offset( double ra_d, double dec_d, int rate ) {
    const std::string function("TcsDaemonClient::pt_offset");
    std::string tcsreply;
    std::stringstream tcscmd;

    // build the command
    //
    tcscmd << TCSD_PTOFFSET << " " << std::fixed << std::setprecision(6)
           << ra_d << " " << dec_d << " " << rate;

    // estimate the timeout by adding the ra and dec in quadrature,
    // divide by offset rate, and multiply by 1.5 for safety, x1000 for msec
    //
    double quad = std::sqrt( std::pow(ra_d,2) + std::pow(dec_d,2) );
    int to = static_cast<int>( std::max( 5000.0, ( 5000.0 + (quad / rate) * 1000.0 * 1.5 ) ) );
    logwrite( function, "[DEBUG] ra_d="+std::to_string(ra_d)+
                               " dec_d="+std::to_string(dec_d)+
                               " rate="+std::to_string(rate)+
                               " quad="+std::to_string(quad)+
                               " to="+std::to_string(to) );
    logwrite( function, "[DEBUG] sending "+tcscmd.str()+" with timeout="+std::to_string(to)+" ms" );
    if ( this->client.send( tcscmd.str(), tcsreply, to ) != NO_ERROR ) {
      logwrite( function, "ERROR sending guider offsets" );
      return ERROR;
    }

    logwrite( function, tcsreply );

    if ( ends_with( tcsreply, "DONE" ) ) return NO_ERROR;
    else return ERROR;
  }
  /***** TcsDaemonClient::pt_offset *******************************************/


  /***** TcsDaemonClient::ret_offsets *****************************************/
  /**
   * @brief      send RET command to tcsd
   * @details    The RET command moves the telescope so the DRA and DDEC offsets
   *             on the telescope status display go to zero.
   * @return     ERROR | NO_ERROR
   *
   */
  long TcsDaemonClient::ret_offsets() {
    std::string function = "TcsDaemonClient::ret_offsets";
    std::string tcsreply;

    if ( this->client.send( TCSD_RETOFFSETS, tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR sending RET command" );
      return ERROR;
    }

    logwrite( function, tcsreply );

    if ( ends_with( tcsreply, "DONE" ) ) return NO_ERROR;
    else return ERROR;
  }
  /***** TcsDaemonClient::ret_offsets *****************************************/


  /***** TcsDaemonClient::set_focus *******************************************/
  /**
   * @brief      send command to tcsd to set telescope focus value
   * @param[out] value  focus position in mm
   * @return     ERROR or NO_ERROR
   *
   */
  long TcsDaemonClient::set_focus( double &value ) {
    std::string function = "TcsDaemonClient::set_focus";
    std::string tcsreply;
    std::stringstream tcscmd;

    tcscmd << TCSD_SET_FOCUS << " " << value;

    if ( this->client.send( tcscmd.str(), tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR setting focus" );
      return ERROR;
    }

    logwrite( function, tcsreply );

    if ( ends_with( tcsreply, "DONE" ) ) return NO_ERROR;
    else return ERROR;
  }
  /***** TcsDaemonClient::set_focus *******************************************/


  /***** TcsDaemonClient::poll_focus ******************************************/
  /**
   * @brief      poll TCS for focus value
   * @detauls    This calls get_focus with the polling arg=true for no logging.
   * @param[out] value  reference to return value
   * @return     ERROR or NO_ERROR
   *
   */
  long TcsDaemonClient::poll_focus( double &value ) {
    return this->get_focus( value, true );
  }
  /***** TcsDaemonClient::poll_focus ******************************************/


  /***** TcsDaemonClient::get_focus *******************************************/
  /**
   * @brief      send command to tcsd to read current telescope focus value
   * @detauls    This calls get_focus with the polling arg=false to allow logging
   * @param[out] value  reference to return value
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   *
   */
  long TcsDaemonClient::get_focus( double &value ) {
    return this->get_focus( value, false );
  }
  /***** TcsDaemonClient::get_focus *******************************************/


  /***** TcsDaemonClient::get_focus *******************************************/
  /**
   * @brief      send command to tcsd to read current telescope focus value
   * @param[out] value  reference to return value
   * @param[in]  poll   set true to suppress logging for polling purposes
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   *
   */
  long TcsDaemonClient::get_focus( double &value, bool poll ) {
    std::string function = "TcsDaemonClient::get_focus";
    std::stringstream message;
    std::string tcsreply;

    // Prepending "poll" onto the command will prevent the server from logging the command
    //
    std::stringstream command; command << ( poll ? "poll " : "" ) << TCSD_GET_FOCUS;

    // Send the command to tcsd to read the focus into tcsreply
    //
    if ( this->client.send( command.str(), tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR reading TCS focus value" );
      value = NAN;
      return ERROR;
    }

    // tcsreply will be " ddd.dd DONE" or "ERROR" so tokenize on space
    // to get the focus value
    //
    std::vector<std::string> tokens;
    Tokenize( tcsreply, tokens, " " );

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR parsing focus from TCS reply \"" << tcsreply << "\": expected ddd.dd DONE";
      logwrite( function, message.str() );
      return ERROR;
    }

    // convert the value to double
    //
    try {
      value = std::stod( tokens.at(0) );
    }
    catch( std::out_of_range &e ) {
      message << "ERROR out of range parsing tcsreply \"" << tcsreply << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      message << "ERROR invalid argument parsing tcsreply \"" << tcsreply << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    message.str(""); message << "currrent focus = " << value;
    if ( ! poll ) logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** TcsDaemonClient::get_focus *******************************************/


  /***** TcsDaemonClient::extract_value ***************************************/
  /**
   * @brief      read the current TCS cass angle
   * @param[out] cass  cass angle in degrees
   * @return     ERROR or NO_ERROR
   *
   */
  long TcsDaemonClient::extract_value( std::string tcs_message, int &value ) {
    std::string function = "TcsDaemonClient::extract_value";
    std::stringstream message;
    std::vector<std::string> tokens;
    long error = ERROR;

    // tcs_message cannot be empty
    //
    if ( tcs_message.empty() ) {
      logwrite( function, "ERROR: empty tcs_message string" );
      value = TCS_UNDEFINED;
      return ERROR;
    }

    Tokenize( tcs_message, tokens, " " );

    // If there's only one token then it's either DONE|ERROR with no value,
    // or something really weird.
    //
    if ( tokens.size() == 1 ) {
      if ( tokens.at(0) == "DONE" ) {
        logwrite( function, "no return value from tcsd" );
        error = NO_ERROR;
      }
      else
      if ( tokens.at(0) == "ERROR" ) {
        logwrite( function, "ERROR received from tcsd" );
        error = ERROR;
      }
      else {
        message.str(""); message << "unrecognized tcs_message: " << tcs_message << " from tcsd";
        logwrite( function, message.str() );
        error = ERROR;
      }
      value = TCS_UNDEFINED;
    }
    else

    // When all is right, expecting 2 tokens, 
    // <tcsreply> DONE
    //
    if ( tokens.size() == 2 ) {
      try {
        value = std::stoi( tokens.at(0) );
      }
      catch( std::out_of_range &e ) {
        message << "ERROR parsing tcs message \"" << tcs_message << "\": " << e.what();
        logwrite( function, message.str() );
        value = TCS_UNDEFINED;
        return ERROR;
      }
      catch( std::invalid_argument &e ) {
        message << "ERROR parsing tcs message \"" << tcs_message << "\": " << e.what();
        logwrite( function, message.str() );
        value = TCS_UNDEFINED;
        return ERROR;
      }
      error = NO_ERROR;
    }

    // more than 2 tokens should be impossible but deal with it just in case
    //
    else {
      message.str(""); message << "ERROR: expected 2 but received " << tokens.size() << " tokens in tcs_message: " << tcs_message;
      logwrite( function, message.str() );
      value = TCS_UNDEFINED;
      error = ERROR;
    }

#ifdef LOGLEVEL_DEBUG  // this can be a little much when polling
    message.str(""); message << "[DEBUG] from tcs_message \"" << tcs_message << "\" extracted value: " << value << " error=" << error;
    logwrite( function, message.str() );
#endif

    return error;
  }
  /***** TcsDaemonClient::extract_value ***************************************/


  /***** TcsDaemonClient::parse_generic_reply *********************************/
  /**
   * @brief      parses the generic tcs reply to most commands
   * @param[in]  value  int from extract_tcs_value()
   * @return     ERROR or NO_ERROR
   *
   * Parses the TCS reply that has been extracted by extract_tcs_value()
   *
   */
  long TcsDaemonClient::parse_generic_reply( int value ) {
    std::string function = "TcsDaemonClient::parse_generic_reply";
    std::stringstream message;
    std::string tcsreply;
    std::vector<std::string> tokens;

    if ( value == TCS_SUCCESS ) {
#ifdef LOGLEVEL_DEBUG
      logwrite( function, "[DEBUG] TCS successful completion" );
#endif
      return NO_ERROR;
    }
    else {
      if ( value == TCS_UNRECOGNIZED_COMMAND ) {
        logwrite( function, "ERROR: TCS unrecognized command" );
      }
      else
      if ( value == TCS_INVALID_PARAMETER ) {
        logwrite( function, "ERROR: TCS invalid parameter(s)" );
      }
      else
      if ( value == TCS_UNABLE_TO_EXECUTE ) {
        logwrite( function, "ERROR: TCS unable to execute" );
      }
      else
      if ( value == TCS_HOST_UNAVAILABLE ) {
        logwrite( function, "ERROR: TCS Sparc host unavailable" );
      }
      else {
        message.str(""); message << "ERROR: " << value << " is not a valid TCS response";
        logwrite( function, message.str() );
      }
      return ERROR;
    }
  }
  /***** TcsDaemonClient::parse_generic_reply *********************************/


  /***** Sequencer::Sequence::poll_dome_position ******************************/
  /**
   * @brief      poll the TCS dome and telescope positions
   * @detauls    This calls get_dome_position with poll=true to prevent logging
   * @param[out] domeazi  reference to dome azimuth
   * @param[out] telazi   reference to telescope azumuth
   * @return     ERROR or NO_ERROR
   *
   */
  long TcsDaemonClient::poll_dome_position( double &domeazi, double &telazi ) {
    return this->get_dome_position( true, domeazi, telazi );
  }
  /***** Sequencer::Sequence::poll_dome_position ******************************/


  /***** Sequencer::Sequence::get_dome_position *******************************/
  /**
   * @brief      read the dome and telescope positions
   * @detauls    This calls get_dome_position with poll=false to allow logging
   * @param[out] domeazi  reference to dome azimuth
   * @param[out] telazi   reference to telescope azumuth
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   *
   */
  long TcsDaemonClient::get_dome_position( double &domeazi, double &telazi ) {
    return this->get_dome_position( false, domeazi, telazi );
  }
  /***** Sequencer::Sequence::get_dome_position *******************************/


  /***** Sequencer::Sequence::get_dome_position *******************************/
  /**
   * @brief      read the dome and telescope positions
   * @param[in]  poll     set true to suppress logging for polling purposes
   * @param[out] domeazi  reference to dome azimuth
   * @param[out] telazi   reference to telescope azumuth
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   *
   */
  long TcsDaemonClient::get_dome_position( bool poll, double &domeazi, double &telazi ) {
    std::string function = "TcsDaemonClient::get_dome_position";
    std::stringstream message;
    std::string tcsreply;
    std::stringstream tcscmd;

    // Prepending "poll" onto the command will prevent the server from logging the command
    //
    tcscmd << ( poll ? "poll " : "" ) << TCSD_GET_DOME;  // optional "poll" prevents excessive logging by tcsd

    // Send the command to tcsd to read the focus into tcsreply
    //
    if ( this->client.send( tcscmd.str(), tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR getting dome position from tcsd" );
      return ERROR;
    }

    std::vector<std::string> tcstokens;
    Tokenize( tcsreply, tcstokens, " " );

    // If there's one (or fewer) tokens then it's an error
    //
    if ( tcstokens.size() <= 1 || tcsreply == "ERROR" ) {
      logwrite( function, "ERROR getting dome position from tcsd" );
      return ERROR;
    }

    // On success GET_DOME returns two numbers, the dome azimuth and the telescope azimuth, followed by DONE
    //
    if ( tcstokens.size() != 3 ) {
      message.str(""); message << "ERROR malformed reply \"" << tcsreply << "\" getting dome position. expected <domeaz> <telaz>";
      logwrite( function, message.str() );
      return ERROR;
    }

    // convert the values to doubles
    //
    try {
      domeazi = std::stod( tcstokens.at(0) );
      telazi  = std::stod( tcstokens.at(1) );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing tcs reply \"" << tcsreply << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR parsing tcs reply \"" << tcsreply << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::get_dome_position *******************************/


  /***** Sequencer::Sequence::get_coords **************************************/
  /**
   * @brief      get coords from TCS using TCSD_GET_COORDS
   * @param[in]  ra_h   reference to RA in decimal hours
   * @param[in]  dec_d  reference to DEC in decimal degrees
   * @return     ERROR or NO_ERROR
   *
   */
  long TcsDaemonClient::poll_coords( double &ra_h, double &dec_d ) {
    return this->get_coords_type( "poll "+TCSD_GET_COORDS, ra_h, dec_d );
  }
  long TcsDaemonClient::get_coords( double &ra_h, double &dec_d ) {
    return this->get_coords_type( TCSD_GET_COORDS, ra_h, dec_d );
  }
  /***** Sequencer::Sequence::get_coords **************************************/


  /***** Sequencer::Sequence::get_weather_coords ******************************/
  /**
   * @brief      get coords from TCS using TCSD_WEATHER_COORDS
   * @param[in]  ra_h   reference to RA in decimal hours
   * @param[in]  dec_d  reference to DEC in decimal degrees
   * @return     ERROR or NO_ERROR
   *
   */
  long TcsDaemonClient::get_weather_coords( double &ra_h, double &dec_d ) {
    return this->get_coords_type( TCSD_WEATHER_COORDS, ra_h, dec_d );
  }
  /***** Sequencer::Sequence::get_weather_coords ******************************/


  /***** Sequencer::Sequence::get_coords_type *********************************/
  /**
   * @brief      get the coords from the TCS using the specified command
   * @param[in]  cmd    command to send to TCS { TCSD_GET_COORDS | TCSD_WEATHER_COORDS }
   * @param[in]  ra_h   reference to RA in decimal hours
   * @param[in]  dec_d  reference to DEC in decimal degrees
   * @return     ERROR or NO_ERROR
   *
   */
  long TcsDaemonClient::get_coords_type( std::string cmd, double &ra_h, double &dec_d ) {
    std::string function = "TcsDaemonClient::get_coords";
    std::stringstream message;

    std::string coordstring;

    // immediately send the specified command to the tcsd
    //
    if ( this->client.send( cmd, coordstring ) != NO_ERROR ) {
      logwrite( function, "ERROR reading TCS coordinates" );
      return ERROR;
    }

    std::vector<std::string> tokens;

    Tokenize( coordstring, tokens, " " );                       // comes back as space-delimited string "hh:mm:ss.ss dd:mm:ss.ss"
    try {                                                       // extract ra dec from coordstring
      if ( cmd.find( TCSD_GET_COORDS ) != std::string::npos ) {
        ra_h  = this->radec_to_decimal( tokens.at(0) );  // RA decimal hours
        dec_d = this->radec_to_decimal( tokens.at(1) );  // DEC decimal degrees
      }
      else
      if ( cmd.find( TCSD_WEATHER_COORDS ) != std::string::npos ) {
        ra_h  = std::stod( tokens.at(0) );                      // RA decimal hours
        dec_d = std::stod( tokens.at(1) );                      // DEC decimal degrees
      }
      else {
        message.str(""); message << "ERROR unrecognized command \"" << cmd << "\"";
        logwrite( function, message.str() );
        return ERROR;
      }
    }
    catch( std::out_of_range &e ) {
      message << "ERROR parsing \"" << coordstring << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      message << "ERROR parsing \"" << coordstring << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Sequencer::Sequence::get_coords_type *********************************/


  /***** Sequencer::TargetInfo::radec_to_decimal ******************************/
  /**
   * @brief      convert string RA or DEC to decimal number
   * @param[in]  str_in  input string to convert
   * @return     double representation of string, or NaN on error
   *
   * Input string could be  HH:MM:SS.sss
   *                        HH MM SS.sss
   *                       ±DD:MM:SS.sss
   *                       ±DD MM SS.sss
   *                       ± D:MM:SS.sss
   *                       ± D MM SS.sss
   *
   * Convert the input string into a decimal (double) number, HH.hhh or ±DD.dddd
   *
   * If the string is empty or otherwise cannot be converted then return NaN.
   *
   */
  double TcsDaemonClient::radec_to_decimal( std::string str_in ) {
    std::string dontcare;
    return( this->radec_to_decimal( str_in, dontcare ) );
  }
  /***** Sequencer::TargetInfo::radec_to_decimal ******************************/


  /***** Sequencer::TargetInfo::radec_to_decimal ******************************/
  /**
   * @brief      convert string RA or DEC to decimal number
   * @param[in]  str_in     input string to convert
   * @param[out] retstring  reference to string representation of return value
   * @return     double representation of string, or NaN on error
   *
   * This function is overloaded.
   * This version accepts a reference to a return string, to return a string
   * version of the decimal (double) return value.
   *
   */
  double TcsDaemonClient::radec_to_decimal( std::string str_in, std::string &retstring ) {
    std::string function = "TcsDaemonClient::TargetInfo::radec_to_decimal";
    std::stringstream message;
    std::vector<std::string> tokens;
    double sign=1.0;

    // can't convert an empty string to a value other than NaN
    //
    if ( str_in.empty() ) {
      logwrite( function, "ERROR: empty input string returns NaN" );
      return NAN;
    }

    // If there's a minus sign (-) in the input string then set the sign
    // multiplier negative, then remove the sign.
    //
    // This is done because tokenizing on space or colon would result in three separate
    // tokens (HH MM SS or DD MM SS) except for the case where the degree is a single 
    // digit, then it's possible that tokenizing " + D MM SS.sss" it could result in four 
    // tokens, "+", "D", "MM", "SS.sss" so determine the sign then get rid of it.
    //
    if ( str_in.find( '-' ) != std::string::npos ) sign = -1.0;
    try {
      str_in.erase( std::remove( str_in.begin(), str_in.end(), '-' ), str_in.end() );
      str_in.erase( std::remove( str_in.begin(), str_in.end(), '+' ), str_in.end() );
    }
    catch( std::out_of_range &e ) {
      message << "ERROR invalid string \"" << str_in << "\" (missing +/-): " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    Tokenize( str_in, tokens, " :" );  // tokenize on space or colon

    if ( tokens.size() != 3 ) {
      message.str(""); message << "ERROR: expected 3 tokens but received " << tokens.size() << " from str_in \"" << str_in << "\"";
      logwrite( function, message.str() );
      return NAN;
    }

    double hh, mm, ss, dec;
    std::stringstream ret;
    try {
      hh = std::stod( tokens.at(0) );
      mm = std::stod( tokens.at(1) ) / 60.0;
      ss = std::stod( tokens.at(2) ) / 3600.0;
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing input string \"" << str_in << "\": " << e.what();
      logwrite( function, message.str() );
      return NAN;
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR parsing input string \"" << str_in << "\": " << e.what();
      logwrite( function, message.str() );
      return NAN;
    }

    dec = sign * ( hh + mm + ss );
    ret << std::fixed << std::setprecision(6) << dec;
    retstring = ret.str();

    return dec;
  }
  /***** Sequencer::TargetInfo::radec_to_decimal ******************************/
