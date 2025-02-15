/**
 * @file    tcs_interface.cpp
 * @brief   this contains the tcs interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the TCS namespace,
 * and is how the tcs daemon interfaces to the actual TCS hardware.
 *
 */

#include "tcs_interface.h"

namespace TCS {

  void Interface::handletopic_snapshot( const nlohmann::json &jmessage ) {
    // If my name is in the jmessage then publish my snapshot
    //
    if ( jmessage.contains( TCS::DAEMON_NAME ) ) {
      this->publish_snapshot();
    }
    else
    if ( jmessage.contains( "test" ) ) {
      logwrite( "TCS::Interface::handletopic_snapshot", jmessage.dump() );
    }
  }


  /***** TCS::Interface::publish_snapshot *************************************/
  /**
   * @brief      publishes snapshot of my telemetry
   * @details    This publishes a JSON message containing a snapshot of my
   *             telemetry.
   *
   */
  void Interface::publish_snapshot() {
    std::string dontcare;
    this->publish_snapshot(dontcare);
  }
  void Interface::publish_snapshot(std::string &retstring) {
    // fill the tcs_info class with current info
    //
    this->get_tcs_info();

    nlohmann::json jmessage_out;
    jmessage_out["source"]     = "tcsd";

    jmessage_out["ISOPEN"]     = this->tcs_info.isopen;
    jmessage_out["TCSNAME"]    = this->tcs_info.tcsname;

    jmessage_out["CASANGLE"]   = this->tcs_info.cassangle;  // double
    jmessage_out["HA"]         = this->tcs_info.ha;         // string
    jmessage_out["RAOFFSET"]   = this->tcs_info.offsetra;   // double
    jmessage_out["DECLOFFSET"] = this->tcs_info.offsetdec;  // double
    jmessage_out["TELRA"]      = this->tcs_info.ra_hms;     // string "hh:mm:ss.s"
    jmessage_out["TELDEC"]     = this->tcs_info.dec_dms;    // string "dd:mm:ss.s"
    jmessage_out["RA"]         = radec_to_decimal( this->tcs_info.ra_hms );
    jmessage_out["DEC"]        = radec_to_decimal( this->tcs_info.dec_dms );
    jmessage_out["AZ"]         = this->tcs_info.azimuth;
    jmessage_out["ALT"]        = 90. - this->tcs_info.zenithangle;
    jmessage_out["ZENANGLE"]   = this->tcs_info.zenithangle;
    jmessage_out["DOMEAZ"]     = this->tcs_info.domeazimuth;
    jmessage_out["DOMESHUT"]   = this->tcs_info.domeshutters==1?"open":"closed";
    jmessage_out["TELFOCUS"]   = this->tcs_info.focus;
    jmessage_out["AIRMASS"]    = this->tcs_info.airmass;
    jmessage_out["MOTION"]     = this->tcs_info.motion;

    // for backwards compatibility
    jmessage_out["messagetype"] = "tcsinfo";
    retstring=jmessage_out.dump();
    retstring.append(JEOF);

    try {
      this->publisher->publish( jmessage_out );
    }
    catch ( const std::exception &e ) {
      logwrite( "TCS::Interface::publish_snapshot",
                "ERROR publishing message: "+std::string(e.what()) );
      return;
    }
  }
  /***** TCS::Interface::publish_snapshot *************************************/


  /***** TCS::Interface::get_tcs_info *****************************************/
  /**
   * @brief      fills the tcs_info class
   * @return     NO_ERROR | ERROR
   *
   */
  long Interface::get_tcs_info() {
    long error = NO_ERROR;
    std::string retstring;

    // erase the class because it's all or nothing. If something fails partway
    // through, we don't want to mix values from a command now with values from
    // an earlier command. E.G. if reqpos fails here but reqstat and weather
    // succeed, we don't want the class to contain values from this reqstat and
    // an earlier call to reqpos.
    //
    this->tcs_info.init();

    // Call the three native functions and the associated parsing function,
    // which will populate the tcs_info class.
    //
    error |= this->send_command( "REQPOS", retstring, TCS::FAST_RESPONSE );
    std::replace( retstring.begin(), retstring.end(), '\n', ',');
    this->tcs_info.parse_reqpos( retstring );

    error |= this->send_command( "REQSTAT", retstring, TCS::FAST_RESPONSE );
    std::replace( retstring.begin(), retstring.end(), '\n', ',');
    this->tcs_info.parse_reqstat( retstring );

    error |= this->send_command( "?WEATHER", retstring, TCS::FAST_RESPONSE );
    std::replace( retstring.begin(), retstring.end(), '\n', ',');
    this->tcs_info.parse_weather( retstring );

    error |= this->send_command( "?MOTION", retstring, TCS::FAST_RESPONSE );
    std::replace( retstring.begin(), retstring.end(), '\n', ',');
    this->tcs_info.motion = retstring;

    return error;
  }
  /***** TCS::Interface::get_tcs_info *****************************************/


  /***** TCS::Interface::list *************************************************/
  /**
   * @brief      list configured TCS devices
   * @param[in]  arg        arg used only for requesting help
   * @param[out] retstring  reference to string to contain the list of devices
   * @return     NO_ERROR | HELP
   *
   */
  long Interface::list( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::list";
    std::stringstream message;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_LIST;
      retstring.append( " \n" );
      retstring.append( "  List configured TCS devices.\n" );
      retstring.append( "  Format will be <name> <host>:<port> <connected>, one per line.\n" );
      return HELP;
    }

    message << "name host:port connected\n";

    for ( const auto &[key,tcs] : this->tcsmap ) {
      message << tcs->getname() << " " 
              << tcs->gethost() << " " 
              << ( tcs->isconnected() ? "true" : "false" ) << "\n";
    }

    retstring = message.str();

    return NO_ERROR;
  }
  /***** TCS::Interface::list *************************************************/


  /***** TCS::Interface::llist ************************************************/
  /**
   * @brief      line list configured TCS devices
   * @details    same as list except all results come out on a single line
   * @param[in]  arg        arg used only for requesting help
   * @param[out] retstring  reference to string to contain the list of devices
   * @return     NO_ERROR | HELP
   *
   */
  long Interface::llist( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::llist";
    std::stringstream message, asyncmsg;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_LLIST;
      retstring.append( " \n" );
      retstring.append( "  List configured TCS devices.\n" );
      retstring.append( "  Format will be <name> <host>:<port> <connected>, for each device,\n" );
      retstring.append( "  separated by a comma (all on a single line, including the DONE|ERROR).\n" );
      return HELP;
    }

    int count=0;
    for ( const auto &[key,val] : this->tcsmap ) {
      if ( count++ > 0 ) message << ",";  // add a comma separator if coming through here again
      message << val->getname() << " " 
              << val->gethost() << " " 
              << ( val->isconnected() ? "true" : "false" );
    }

    retstring = message.str();

    asyncmsg << "TCSD:llist:" << retstring;
    this->async.enqueue( asyncmsg.str() );

    return NO_ERROR;
  }
  /***** TCS::Interface::llist ************************************************/


  /***** TCS::Interface::open *************************************************/
  /**
   * @brief      opens the TCS socket connection
   * @param[in]  arg  contains what to open, real or sim
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::open( std::string arg, std::string &retstring ) {
    const std::string function("TCS::Interface::open");
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_OPEN;
      retstring.append( " <name>\n" );
      retstring.append( "  Open a connection to the specified TCS device, where\n" );
      retstring.append( "  <name> = { " );
      message.str("");
      for ( const auto &[key,val] : this->tcsmap ) message << val->getname() << " ";
      message << "}\n";
      retstring.append( message.str() );
      return HELP;
    }

    // get the name of the tcs to connect to, or use default if specified
    //
    if ( arg.empty() && this->default_tcs.empty() ) {
      logwrite( function, "ERROR no TCS name specified and no default configured" );
      retstring="missing_argument";
      return ERROR;
    }
    if ( arg.empty() ) {
      message.str(""); message << "NOTICE: no TCS name specified, using default \"" << this->default_tcs << "\"";
      logwrite( function, message.str() );
      arg = this->default_tcs;
    }

    // Typically, a second call to open might not return an error but this function returns an error
    // on a second call to open because the user might be trying to open a different TCS, e.g.
    // opening the real while the sim is already open, so returning an error will catch that.
    //
    for ( const auto &[key,val] : this->tcsmap ) {
      if ( val->isconnected() ) {
        message.str(""); message << "ERROR: connection already open to " << val->getname() << " "
                                 << val->gethost();
        logwrite( function, message.str() );
        retstring="already_open";
        return ERROR;
      }
    }

    // Find the requested name in the tcsmap
    //
    auto tcsloc = this->tcsmap.find( arg );

    if ( tcsloc == this->tcsmap.end() ) {
      message.str(""); message << "ERROR: requested TCS name \"" << arg << "\" not found in configured list";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    // Don't allow using P200 TCS unless I'm located at Palomar
    //
    std::string localhost = get_localhost();
    if ( tcsloc->second->gethost() == TCS::IP_TCS && localhost != TCS::IP_PALOMAR ) {
      logwrite( function, "ERROR cannot connect to P200 TCS from "+localhost );
      return ERROR;
    }

    // initialize connection to the TCS
    //
    message.str(""); message << "initializing socket connections to TCS " << tcsloc->first << " on "
                             << tcsloc->second->gethost();
    logwrite( function, message.str() );

    TcsIO &tcs = *(tcsloc->second);

    tcs.initialize_sockets();

    this->name = tcs.getname();             // save the name of the opened tcs

    error = this->isopen( retstring );      // this will broadcast the state and name

    return error;
  }
  /***** TCS::Interface::open *************************************************/
  /**
   * @brief      overloaded version opens socket connection to default TCS
   * @details    The default_tcs is specified in the config file
   * @return     ERROR | NO_ERROR
   */
  long Interface::open() {
    std::string dontcare;
    return open(this->default_tcs, dontcare);
  }
  /***** TCS::Interface::open *************************************************/


  /***** TCS::Interface::isopen ***********************************************/
  /**
   * @brief      return the open status of the TCS
   * @return     true | false
   *
   */
  bool Interface::isopen() {
    std::string ret;

    this->isopen( ret );                 // this will broadcast the state and name

    return( ret == "false" ? false : true );
  }
  /***** TCS::Interface::isopen ***********************************************/


  /***** TCS::Interface::isopen ***********************************************/
  /**
   * @brief      return the open status of the TCS
   * @param[out] retstring  reference to string contains open status
   * @return     ERROR or NO_ERROR
   *
   * status string is either "false" or the name of the TCS if open.
   *
   */
  long Interface::isopen( std::string &retstring ) {
    return this->isopen( "", retstring );
  }
  /***** TCS::Interface::isopen ***********************************************/


  /***** TCS::Interface::isopen ***********************************************/
  /**
   * @brief      return the open status of the TCS
   * @param[in]  arg        input arg used for help
   * @param[out] retstring  reference to string contains open status
   * @return     ERROR | NO_ERROR | HELP
   *
   * status string is either "false" or the name of the TCS if open.
   *
   */
  long Interface::isopen( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::isopen";
    std::stringstream message, asyncmsg;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_ISOPEN;
      retstring.append( " \n" );
      retstring.append( "  Returns the open status of the TCS, true | false\n" );
      return HELP;
    }

    std::string _name;

    for ( const auto &[key,val] : this->tcsmap ) {
      if ( val->isconnected() ) {
        _name = val->getname();       // Found a connected TCS
        break;                        // so get out now.
      }
    }

    this->tcs_info.isopen  = ( ! _name.empty() ? true : false );
    this->tcs_info.tcsname = _name;

    retstring = ( this->tcs_info.isopen ? "true" : "false" );  // return string is the state

    asyncmsg.str(""); asyncmsg << "TCSD:open:" << retstring;
    this->async.enqueue( asyncmsg.str() );              // broadcast the state

    asyncmsg.str(""); asyncmsg << "TCSD:name:" << ( ! _name.empty() ? _name : "offline" );
    this->async.enqueue( asyncmsg.str() );              // broadcast the name

    return NO_ERROR;
  }
  /***** TCS::Interface::isopen ***********************************************/


  /***** TCS::Interface::close ************************************************/
  /**
   * @brief      closes the TCS socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    std::string function = "TCS::Interface::close";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    for ( const auto &[key,tcs] : this->tcsmap ) {
      if ( tcs->isconnected() ) {
        message.str(""); message << "closing connection to " << tcs->getname() << " "
                                 << tcs->gethost();
        logwrite( function, message.str() );
        error = tcs->close();
        this->name.clear();  // clear the name of the opened tcs
      }
    }

    this->isopen();          // do this to broadcast new state

    return error;
  }
  /***** TCS::Interface::close ************************************************/


  /***** TCS::Interface::get_name *********************************************/
  /**
   * @brief      get the name of the currently connected TCS
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains name of tcs { tcs | sim } or offline
   * @return     NO_ERROR | HELP
   *
   */
  long Interface::get_name( const std::string &arg, std::string &retstring ) {
    const std::string function = "TCS::Interface::get_name";

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_GET_NAME;
      retstring.append( " \n" );
      retstring.append( "  Returns the name of the connected TCS: sim | tcs | offline\n" );
      return HELP;
    }

    bool silent = false;                 // logging enabled by default

    if ( arg == "poll" ) silent = true;  // logging is suppressed for polling

    for ( const auto &[key,tcs] : this->tcsmap ) {
      if ( tcs->isconnected() ) {
        if ( !silent ) {
          logwrite( function, "connection open to "+tcs->getname() );
        }
        retstring = tcs->getname();      // Found the connected TCS
        this->async.enqueue( "TCSD:name:"+retstring );
        return NO_ERROR;
      }
    }
    retstring="offline";

    this->async.enqueue( "TCSD:name:offline" );

    return NO_ERROR;
  }
  /***** TCS::Interface::get_name *********************************************/


  /***** TCS::Interface::get_weather_coords ***********************************/
  /**
   * @brief      get the current simulator coords
   * @details    uses the ?WEATHER command, pulls out just the RA and DEC
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get_weather_coords( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_weather_coords";
    std::stringstream message, asyncmsg;
    std::string weather;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_WEATHER_COORDS;
      retstring.append( " \n" );
      retstring.append( "  Return the current decimal RA DEC as h.hhhhhhhh d.dddddddd\n" );
      return HELP;
    }

    bool silent = false;                 // logging enabled by default

    if ( arg == "poll" ) silent = true;  // logging is suppressed for polling

    // Send the WEATHER command to the TCS. This returns a string of key=val pairs
    // with each pair separated by a newline character. The first two pairs are 
    // RA and DEC, which is all that is needed here.
    //
    if ( error != ERROR && this->send_command( "?WEATHER", weather, TCS::FAST_RESPONSE ) != NO_ERROR ) {
      message << "ERROR getting coords from TCS";
      error = ERROR;
    }

    std::vector<std::string> pairs;
    Tokenize( weather, pairs, "\n" );  // tokenize on the newline to break into key=val pairs

    // If someone ever changes the TCS message then this will have to be changed.
    //
    if ( error != ERROR && pairs.size() != 14 ) {
      message << "ERROR malformed reply from TCS";
      error = ERROR;
    }

    // Tokenize the first two key=val pairs on the "=" to get the ra, dec values
    //
    if ( error != ERROR ) {
    try {
      std::vector<std::string> ra_tokens, dec_tokens;

      Tokenize( pairs.at(0), ra_tokens, "=" );
      Tokenize( pairs.at(1), dec_tokens, "=" );

      message << ra_tokens.at(1) << " " << dec_tokens.at(1);
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "ERROR out of range parsing ra,dec from ?WEATHER command" );
      return ERROR;
    }
    }

    if ( !silent ) logwrite( function, message.str() );

    retstring = message.str();

    asyncmsg << "TCSD:weathercoords:" << ( error==NO_ERROR ? message.str() : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_weather_coords ***********************************/


  /***** TCS::Interface::get_coords *******************************************/
  /**
   * @brief      get the current coords
   * @details    uses the REQPOS command, pulls out just the RA and DEC,
   *             returns them as "hh:mm:ss.ss dd:mm:ss.ss"
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get_coords( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_coords";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_GET_COORDS;
      retstring.append( " \n" );
      retstring.append( "  Return the current RA DEC as hh:mm:ss.ss dd:mm:ss.ss\n" );
      return HELP;
    }

    bool silent = false;                 // logging enabled by default

    if ( arg == "poll" ) silent = true;  // logging is suppressed for polling

    // Send the REQPOS command to the TCS. This returns a string that looks like:
    //
    // tcsreply = "UTC = ddd hh:mm:ss, LST = hh:mm:ss\n
    //             RA = hh:mm:ss.ss, DEC = dd:mm:ss.s, HA=hh:mm:ss.s\n
    //             air mass = aa.aaa"
    //
    std::string tcsreply;
    if ( this->send_command( "REQPOS", tcsreply, TCS::FAST_RESPONSE ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      error = ERROR;
    }

    // First tokenize on newline
    //
    std::vector<std::string> lines;
    Tokenize( tcsreply, lines, "\n" );

    if ( error != ERROR && lines.size() != 3 ) {
      message.str(""); message << "ERROR expected 3 lines from REQPOS string but received " << lines.size();
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Then the 2nd token, lines[1], contains the RA,DEC so try to
    // tokenize that on the comma.
    //
    try {
      std::vector<std::string> radec;
      Tokenize( lines.at(1), radec, "," );  // lines[1] = "RA = hh:mm:ss.ss, DEC = dd:mm:ss.s, HA=hh:mm:ss.s\n"

      if ( error != ERROR && radec.size() != 3 ) {
        message.str(""); message << "ERROR expected 3 tokens for ra, dec, ha but received " << radec.size();
        logwrite( function, message.str() );
        error = ERROR;
      }

      // Now try to tokenize the first two tokens of line1 on the equal sign to extract RA and DEC values
      //
      if ( error != ERROR ) {
        std::vector<std::string> ra, dec;
        Tokenize( radec.at(0), ra,  "=" );
        Tokenize( radec.at(1), dec, "=" );

        if ( ra.size() != 2 && dec.size() != 2 ) {
          message.str(""); message << "ERROR extracting ra,dec from " << lines.at(1);
          logwrite( function, message.str() );
          error = ERROR;
        }
        message.str(""); message << ra.at(1) << " " << dec.at(1);
        retstring = message.str();
      }
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "EXCEPTION: out of range parsing ra,dec from TCS reply string" );
      error = ERROR;
    }

    if ( !retstring.empty() && !silent ) logwrite( function, retstring );

    asyncmsg << "TCSD:coords:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_coords *******************************************/


  /***** TCS::Interface::get_cass *********************************************/
  /**
   * @brief      get the current cass angle
   * @details    uses the REQSTAT command, pulls out just the Cass ring angle,
   *             returns as "ddd.dd"
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains cass ring angle
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get_cass( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_cass";
    std::stringstream message, asyncmsg;
    std::stringstream reply;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_GET_CASS;
      retstring.append( " \n" );
      retstring.append( "  Return the current cass ring angle\n" );
      return HELP;
    }

    bool silent = false;                 // logging enabled by default

    if ( arg == "poll" ) silent = true;  // logging is suppressed for polling

    // Send the REQSTAT command to the TCS. This returns a string that looks like:
    //
    // tcsreply = "UTC = 127 21:58:7.0\n
    //             telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm\n
    //             offset RA = 45.00 arcsec, DEC = 45.00 arcsec\n
    //             rate RA = 0.00 arcsec/hr, DEC = 0.00 arcsec/hr\n
    //             Cass ring angle = 18.00
    //
    std::string tcsreply;
    if ( this->send_command( "REQSTAT", tcsreply, TCS::FAST_RESPONSE ) != NO_ERROR ) {
      logwrite( function, "ERROR getting status from TCS" );
      error = ERROR;
    }

    // First tokenize on comma
    //
    std::vector<std::string> lines;
    Tokenize( tcsreply, lines, "\n" );

    if ( error==NO_ERROR && lines.size() != 5 ) {
      message.str(""); message << "ERROR expected 5 lines from REQSTAT string but received " << lines.size()
                               << " in reply \"" << tcsreply << "\"";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Then the 5th token, lines[4], contains the "Cass ring angle = dd.dd"
    // tokenize that on the equal to get the value.
    //
    std::string cassline;                    // assign this a variable for exception reporting
    try {
      cassline = lines.at(4);                // lines[4] = "Cass ring angle = dd.dd"
      std::vector<std::string> tokens;
      Tokenize( cassline, tokens, "=" );

      if ( error==NO_ERROR && tokens.size() != 2 ) {
        message.str(""); message << "ERROR expected 2 tokens for Cass ring angle = dd.dd but received " << tokens.size();
        logwrite( function, message.str() );
        error = ERROR;
      }

      // Try converting it to a double, not because it's needed as a double but this checks
      // that it is indeed a double
      //
      if ( error==NO_ERROR ) {
        double angle = std::stod( tokens.at(1) );
        reply << std::fixed << std::setprecision(2) << angle;
      }
    }
    catch( const std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing Cass ring angle from TCS reply string \"" << cassline
                               << "\": " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }

    retstring = reply.str();

    if ( !retstring.empty() && !silent ) logwrite( function, retstring );

    asyncmsg << "TCSD:cassangle:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_cass *********************************************/


  /***** TCS::Interface::get_dome *********************************************/
  /**
   * @brief      get the dome and telescope azimuths
   * @details    uses the ?WEATHER command, pulls out just the dome and tel azimuth
   *             returns them as "<domeazi> <telazi>"
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get_dome( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_dome";
    std::stringstream message, asyncmsg;
    std::string weather;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_GET_DOME;
      retstring.append( " \n" );
      retstring.append( "  Return the dome and telescope azimuth positions.\n" );
      return HELP;
    }

    // Send the WEATHER command to the TCS. This returns a string of key=val pairs
    // with each pair separated by a newline character. The first two pairs are 
    // RA and DEC, which is all that is needed here.
    //
    if ( error != ERROR && this->send_command( "?WEATHER", weather, TCS::FAST_RESPONSE ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      error = ERROR;
    }

    std::vector<std::string> pairs;
    Tokenize( weather, pairs, "\n" );  // tokenize on the newline to break into key=val pairs

    // If someone ever changes the TCS message then this will have to be changed.
    //
    if ( error != ERROR && pairs.size() != 14 ) {
      logwrite( function, "ERROR malformed reply" );
      error = ERROR;
    }

    // Tokenize the first two key=val pairs on the "=" to get the ra, dec values
    //
    try {
      if ( error != ERROR ) {
        std::vector<std::string> telazi_tokens, domeazi_tokens;

        Tokenize( pairs.at(5), telazi_tokens, "=" );
        Tokenize( pairs.at(8), domeazi_tokens, "=" );

        message.str(""); message << domeazi_tokens.at(1) << " " << telazi_tokens.at(1);
        retstring = message.str();
      }
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "ERROR out of range parsing azimuth from ?WEATHER command" );
      error = ERROR;
    }

    if ( !retstring.empty() ) logwrite( function, retstring );

    asyncmsg << "TCSD:dome:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_dome *********************************************/


  /***** TCS::Interface::set_focus ********************************************/
  /**
   * @brief      set the focus position
   * @details    uses the FOCUSGO command
   * @param[in]  arg        requested focus value
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::set_focus( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::set_focus";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_SET_FOCUS;
      retstring.append( " <value>\n" );
      retstring.append( "  Set the telescope focus position in mm within range {1:74}\n" );
      return HELP;
    }

    if ( arg.empty() ) { retstring="missing_argument"; return ERROR; }

    double value;

    try {
      value = std::stod( arg );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing focus value from \"" << arg << "\": " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR parsing focus value from \"" << arg << "\": " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }

    if ( std::isnan(value) ) {
      logwrite( function, "ERROR: requested focus is NaN" );
      retstring="bad_value";
      return ERROR;
    }

    if ( value < 1.0 || value > 74.0 ) {
      message.str(""); message << "ERROR requested focus " << value << " outside range { 1:74 }";
      logwrite( function, message.str() );
      retstring="outside_range";
      return ERROR;
    }

    std::stringstream cmd;
    cmd << "FOCUSGO " << std::fixed << std::setprecision(2) << value;

    if ( error != ERROR ) error = this->send_command( cmd.str(), retstring, TCS::SLOW_RESPONSE, 60000 );

    return error;
  }
  /***** TCS::Interface::set_focus ********************************************/


  /***** TCS::Interface::get_focus ********************************************/
  /**
   * @brief      get the current focus position
   * @details    uses this version internally, when help won't be requested
   * @param[out] retstring  contains focus position
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_focus( std::string &retstring ) {
    std::string notneeded;
    return get_focus( notneeded, retstring );
  }
  /***** TCS::Interface::get_focus ********************************************/


  /***** TCS::Interface::get_focus ********************************************/
  /**
   * @brief      get the current focus position
   * @details    uses the REQSTAT command, pulls out just the focus,
   *             returns as "dd.dd". Set arg="poll" to suppress logging.
   * @param[in]  arg        input arg { help | poll }
   * @param[out] retstring  contains focus position.
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get_focus( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_focus";
    std::stringstream message, asyncmsg;
    std::stringstream reply;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_GET_FOCUS;
      retstring.append( " \n" );
      retstring.append( "  Return the current telescope focus position in mm\n" );
      return HELP;
    }

    bool silent = false;                 // logging enabled by default

    if ( arg == "poll" ) silent = true;  // logging is suppressed for polling

    // Send the REQSTAT command to the TCS. This returns a string that looks like:
    //
    // tcsreply = "UTC = 127 21:58:7.0\n
    //             telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm\n
    //             offset RA = 45.00 arcsec, DEC = 45.00 arcsec\n
    //             rate RA = 0.00 arcsec/hr, DEC = 0.00 arcsec/hr\n
    //             Cass ring angle = 18.00
    //
    std::string tcsreply;
    if ( this->send_command( "REQSTAT", tcsreply, TCS::FAST_RESPONSE ) != NO_ERROR ) {
      logwrite( function, "ERROR getting status from TCS" );
      error = ERROR;
    }

    // First tokenize on comma
    //
    std::vector<std::string> lines;
    Tokenize( tcsreply, lines, "\n" );

    if ( error==NO_ERROR && lines.size() != 5 ) {
      message.str(""); message << "ERROR expected 5 lines from REQSTAT string but received " << lines.size()
                               << " in reply \"" << tcsreply << "\"";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Then the 2nd token, lines[1], contains the "telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm\n"
    // tokenize that on the comma to get the params from that line.
    //
    try {
      std::vector<std::string> params, tokens;
      Tokenize( lines.at(1), params, "," );  // lines[1] = "telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm\n"

      if ( error==NO_ERROR && params.size() != 3 ) {
        message.str(""); message << "ERROR expected 3 params on line 2 but received " << params.size();
        logwrite( function, message.str() );
        error = ERROR;
      }
      else Tokenize( params.at(1), tokens, "=" );  // Finally, tokenize on "=" to get the value from params[1] = "focus = 36.71 mm"

      if ( error==NO_ERROR && tokens.size() != 2 ) {
        message.str(""); message << "ERROR expected 2 tokens for focus = dd.dd mm but received " << tokens.size();
        logwrite( function, message.str() );
        error = ERROR;
      }

      // Try converting it to a double, not because it's needed as a double but this checks
      // that it is indeed a double. stod will discard the "mm" after the double value so
      // there is no need to remove it prior.
      //
      if ( error==NO_ERROR ) {
        double focus = std::stod( tokens.at(1) );
        reply << std::fixed << std::setprecision(2) << focus;
      }
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "EXCEPTION: out of range parsing focus from TCS reply string" );
      error = ERROR;
    }
    catch( std::invalid_argument &e ) {
      logwrite( function, "EXCEPTION converting focus to double" );
      error = ERROR;
    }

    retstring = reply.str();

    if ( !retstring.empty() && !silent ) logwrite( function, retstring );

    asyncmsg << "TCSD:focus:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_focus ********************************************/


  /***** TCS::Interface::get_offsets ******************************************/
  /**
   * @brief      get the current offsets
   * @details    This calls the overloaded function which returns the offsets
   *             by reference.
   * @param[in]  arg     input arg { help }
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get_offsets( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_offsets";
    std::stringstream message;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_GET_OFFSETS;
      retstring.append( " \n" );
      retstring.append( "  Return the current telescope offsets in decimal arcseconds\n" );
      return HELP;
    }

    double raoff, decoff;
    if ( this->get_offsets( raoff, decoff ) == ERROR ) {
      return ERROR;
    }
    else {
      message << std::fixed << std::setprecision(4) << raoff << " " << decoff;
      retstring = message.str();
      logwrite( function, retstring );
    }

    message.str(""); message << "TCSD:offsets:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( message.str() );

    return NO_ERROR;
  }
  /***** TCS::Interface::get_offsets ******************************************/


  /***** TCS::Interface::get_offsets ******************************************/
  /**
   * @brief      get the current offsets
   * @details    This overloaded function sends the REQSTAT command to the TCS
   *             and parses the reply to obtain raoff, decoff.
   * @param[out] raoff   reference to ra offset
   * @param[out] decoff  reference to dec offset
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::get_offsets( double &raoff, double &decoff ) {
    std::string function = "TCS::Interface::get_offsets";
    std::stringstream message;
    std::stringstream reply;

    // Send the REQSTAT command to the TCS. This returns a string that looks like:
    //
    // tcsreply = "UTC = 127 21:58:7.0\n
    //             telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm\n
    //             offset RA = 45.00 arcsec, DEC = 45.00 arcsec\n
    //             rate RA = 0.00 arcsec/hr, DEC = 0.00 arcsec/hr\n
    //             Cass ring angle = 18.00
    //
    std::string tcsreply;
    if ( this->send_command( "REQSTAT", tcsreply, TCS::FAST_RESPONSE ) != NO_ERROR ) {
      logwrite( function, "ERROR getting status from TCS" );
      return ERROR;
    }

    // First tokenize on newline
    //
    std::vector<std::string> lines;
    Tokenize( tcsreply, lines, "\n" );

    if ( lines.size() != 5 ) {
      message.str(""); message << "ERROR expected 5 lines from REQSTAT string but received " << lines.size()
                               << " in reply \"" << tcsreply << "\"";
      logwrite( function, message.str() );
      return ERROR;
    }

    // Then the 3rd token, lines[2], contains the "offset RA = 45.00 arcsec, DEC = 45.00 arcsec\n"
    //
    try {
      // remove the "=" and "," so the only delimiter is " "
      //
      std::string line = lines.at(2);                                          // copy the line
      line.erase( std::remove( line.begin(), line.end(), '=' ), line.end() );  // remove all = from copy
      line.erase( std::remove( line.begin(), line.end(), ',' ), line.end() );  // remove all , from copy

      // now line = "offset RA 45.00 arcsec DEC 45.00 arcsec\n"
      // so tokenize on space and take the 3rd and 6th tokens
      //
      std::vector<std::string> tokens;
      Tokenize( line, tokens, " " );

      if ( tokens.size() != 7 ) {
        message.str(""); message << "ERROR expected 7 params on line 3 but received " << tokens.size();
        logwrite( function, message.str() );
        return ERROR;
      }

      // convert those tokens to double and assign to the reference arguments
      //
      raoff  = std::stod( tokens.at(2) );
      decoff = std::stod( tokens.at(5) );
    }
    catch( const std::exception &e ) {
      message.str(""); message << "ERROR parsing offsets from TCS reply string: " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** TCS::Interface::get_offsets ******************************************/


  /***** TCS::Interface::pt_offsetrate ****************************************/
  /**
   * @brief      set the offset tracking rates
   * @details    uses the MRATES command
   * @param[in]  arg        "<raoff> <decoff>" in arcsec/second
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::pt_offsetrate( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::pt_offsetrate";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_OFFSETRATE;
      retstring.append( " [ <rate> ]\n" );
      retstring.append( "  Set or get the telescope offset tracking rates in integer arcsec/second.\n" );
      retstring.append( "  If no args supplied then current values are returned.  Default offset rates\n" );
      retstring.append( "  are set by TCS_OFFSET_RATE in the config file.\n" );
      return HELP;
    }

    // No args is read-only
    //
    if ( arg.empty() ) {
      message.str(""); message << this->offsetrate;
      retstring = message.str();
      logwrite( function, retstring );
      return error;
    }

    int _offsetrate=0;

    std::vector<std::string> tokens;
    Tokenize( arg, tokens, " " );

    if ( tokens.size() != 1 ) {
      logwrite( function, "ERROR expected 1 args: <offsetrate>" );
      retstring="invalid_argument";
      return ERROR;
    }

    try {
      _offsetrate = std::stoi( tokens.at(0) );
    }
    catch( const std::exception &e ) {
      message.str(""); message << "ERROR parsing \"" << arg << "\" expected integer {1:50} : " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }

    if ( _offsetrate  < 1 || _offsetrate  > 50 ) {
      message.str(""); message << "ERROR offset rate " << _offsetrate << " must be integer in range { 1:50 }";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    this->offsetrate = _offsetrate;

    return error;
  }
  /***** TCS::Interface::pt_offsetrate ****************************************/


  /***** TCS::Interface::get_motion *******************************************/
  /**
   * @brief      get the motion status
   * @details    uses the ?MOTION command, returns a string instead of a code
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains the motion state string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get_motion( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_motion";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_GET_MOTION;
      retstring.append( " \n" );
      retstring.append( "  Return the current motion state of the TCS\n" );
      return HELP;
    }

    // Send the command
    //
    if ( this->send_command( "?MOTION", retstring, TCS::FAST_RESPONSE ) != NO_ERROR ) {
      logwrite( function, "ERROR getting motion state from TCS" );
      error = ERROR;
    }

    asyncmsg << "TCSD:motion:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_motion *******************************************/


  /***** TCS::Interface::ringgo ***********************************************/
  /**
   * @brief      wrapper for the native RINGGO command
   * @details    wraps the RINGGO command to ensure the value is within range
   * @param[in]  arg        requested angle string
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::ringgo( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::ringgo";
    std::stringstream message, asyncmsg;
    long error = ERROR;

    double angle = NAN;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_RINGGO;
      retstring.append( " <angle>\n" );
      retstring.append( "  Rotate the cass ring to the specified angle in degrees.\n" );
      retstring.append( "  If the angle is available in more than one turn of the ring,\n" );
      retstring.append( "  the shortest move will be taken.\n" );
      return HELP;
    }

    if ( arg.empty() ) { retstring="missing_argument"; return ERROR; }

    try {
      angle = std::stod( arg );
      error = NO_ERROR;
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "EXCEPTION: out of range parsing requested angle from \"" << arg << "\": " << e.what();
      logwrite( function, message.str() );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "EXCEPTION: converting requested angle from \"" << arg << "\" to double: " << e.what();
      logwrite( function, message.str() );
    }

    if ( std::isnan(angle) ) {
      logwrite( function, "ERROR: requested angle is NaN" );
      error = ERROR;
    }

    // For negative angles, offset them once by 360,
    //
    if ( angle < 0.0 ) angle += 360.0;

    // but if that isn't enough to make it positive then this is probably a bogus value.
    //
    if ( angle < 0.0 ) {
      message.str(""); message << "ERROR: requested angle " << angle << " is too negative";
      logwrite( function, message.str() );
      error = ERROR;
    }

    if ( angle > 359.99 && angle <= 360.0 ) angle = 0;

    if ( angle > 360.0 ) {
      message.str(""); message << "ERROR: requested angle " << angle << " cannot exceed 360";
      logwrite( function, message.str() );
      error = ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] requested cass angle " << angle;
    logwrite( function, message.str() );
#endif

    std::stringstream cmd;
    cmd << "RINGGO " << std::fixed << std::setprecision(2) << angle;

    if ( error != ERROR ) error = this->send_command( cmd.str(), retstring, TCS::SLOW_RESPONSE, 300000 );

    return error;
  }
  /***** TCS::Interface::ringgo ***********************************************/


  /***** TCS::Interface::coords ***********************************************/
  /**
   * @brief      wrapper for the native COORDS command
   * @details    wraps the COORDS command to provide a friendly reply
   * @param[in]  args       requested angle string
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::coords( std::string args, std::string &retstring ) {
    std::string function = "TCS::Interface::coords";
    std::string retcode;
    std::stringstream message, asyncmsg;

    // Help
    //
    if ( args == "?" ) {
      retstring = TCSD_COORDS;
      retstring.append( " <ra> <dec> <equinox> <ramotion> <decmotion> [<motionflag>] [ \"<targetname>\" ]\n" );
      retstring.append( "  Supply a position to the TCS.\n" );
      retstring.append( "  <ra> in decimal hours, <dec> in decimal degrees\n" );
      retstring.append( "  <equinox> = 0 indicates apparent equinox\n" );
      retstring.append( "  Optional <motionflag> defines the meaning of <ramotion> and <decmotion> as\n" );
      retstring.append( "  0 (or absent): proper motion in units of RA 0.0001 sec/yr, DEC 0.001 arcsec/yr\n" );
      retstring.append( "  1: offset tracking rates in arcsec/hr\n" );
      retstring.append( "  2: offset tracking rates in sec/hr, arcsec/hr\n" );
      retstring.append( "  Optional targetname must be in double quotes, if supplied.\n" );
      return HELP;
    }

    // check minimum/maximum number of arguments
    //
    std::vector<std::string> tokens;
    int ntok = Tokenize( args, tokens, " " );
    if ( ntok < 5 || ntok > 7 ) { retstring="invalid_arguments"; return ERROR; }

    std::stringstream cmd;
    cmd << "COORDS " << args;

    long error = this->send_command( cmd.str(), retstring, TCS::FAST_RESPONSE );

    asyncmsg << "TCSD:coords:" << ( error == ERROR ? "ERROR" : retstring );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::coords ***********************************************/


  /***** TCS::Interface::pt_offset ********************************************/
  /**
   * @brief      wrapper for the native PT command
   * @details    wraps the PT command to provide a friendly reply
   * @param[in]  args       contains <ra> <dec> in decimal arcsec
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::pt_offset( std::string args, std::string &retstring ) {
    std::string function = "TCS::Interface::pt_offset";
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
      retstring = TCSD_PTOFFSET;
      retstring.append( " <raoff> <decoff> [ <rate> ]\n" );
      retstring.append( "  Send guider offsets to the TCS. These are great circle distances in\n" );
      retstring.append( "  decimal units of arcsec (degree/3600).\n" );
      retstring.append( "  There is currently no reliable way of accurately knowning when we have\n" );
      retstring.append( "  arrived so this command will wait the expected time based on the TCS\n" );
      retstring.append( "  offset rate, with a minimum of 100 msec, before returning.\n" );
      retstring.append( "  The offset rate can be optionally specified.\n" );
      return HELP;
    }

    // check minimum/maximum number of arguments
    //
    std::vector<std::string> tokens;
    Tokenize( strip_control_characters(args), tokens, " " );

    if ( tokens.size() > 3 ) {
      logwrite( function, "ERROR invalid number of arguments: expected <ra> <dec> [ <rate> ]" );
      retstring="invalid_arguments";
      return ERROR;
    }

    // Try to convert them to double to ensure that they are good numbers
    // before sending them to the TCS.
    //
    double raoff, decoff;
    int rate;
    try {
      raoff  = ( std::stod( tokens.at(0) ) );
      decoff = ( std::stod( tokens.at(1) ) );
      rate   = ( tokens.size()==3 ? std::stoi(tokens.at(2)) : this->offsetrate );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing \"" << args << "\":" << e.what();
      logwrite( function, message.str() );
      retstring="invalid_arguments";
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR parsing \"" << args << "\":" << e.what();
      logwrite( function, message.str() );
      retstring="invalid_arguments";
      return ERROR;
    }

    if ( rate < 1 || rate > 50 ) {
      logwrite( function, "ERROR invalid rate "+std::to_string(rate)+
                          " using default: "+std::to_string(this->offsetrate) );
      // default should be good but check anyway, never want to divide by zero
      rate = ( this->offsetrate < 1 ? 40 : this->offsetrate );
    }

    // offsetrate is offset rates in arcsec/sec
    // so ra_t/offsetrate and dec_t/offsetrate are arcsec / arcsec / sec = sec
    //
    int ra_t  = static_cast<int>( 1000 * std::abs(raoff)  / this->offsetrate  );  // time (ms) to offset RA
    int dec_t = static_cast<int>( 1000 * std::abs(decoff) / this->offsetrate );   // time (ms) to offset DEC
    int max_t = static_cast<int>( 1.5  * std::max( ra_t, dec_t ) );     // greater of the two, plus 50%
    max_t = std::max( max_t, PTOFFSET_MIN_TIMEOUT );                    // minimum timeout

    std::stringstream cmd;
    cmd << "PT " << raoff << " " << decoff << " " << rate;

    long error = this->send_command( cmd.str(), retstring, TCS::SLOW_RESPONSE, max_t );  // perform the offset here

    return error;
  }
  /***** TCS::Interface::pt_offset ********************************************/


  /***** TCS::Interface::zero_offsets *****************************************/
  /**
   * @brief      wrapper for the native Z command
   * @details    wraps the Z command to provide a friendly reply
   * @param[in]  args       help only
   * @param[out] retstring  returns help
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::zero_offsets( const std::string args, std::string &retstring ) {
    std::string function = "TCS::Interface::zero_offsets";
    std::string retcode;
    std::stringstream message, asyncmsg;

    // Help
    //
    if ( args == "?" ) {
      retstring = TCSD_ZERO_OFFSETS;
      retstring.append( "\n" );
      retstring.append( "  Zeros the offsets by sending the \"Z\" command.\n" );
      return HELP;
    }

    std::string cmd("Z");

    return this->send_command( cmd, retstring, TCS::FAST_RESPONSE );
  }
  /***** TCS::Interface::zero_offsets *****************************************/


  /***** TCS::Interface::ret_offsets ******************************************/
  /**
   * @brief      wrapper for the native RET command
   * @param[in]  args       for optional help only
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::ret_offsets( std::string args, std::string &retstring ) {
    std::string function = "TCS::Interface::ret_offset";
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
      retstring = TCSD_RETOFFSETS;
      retstring.append( "\n" );
      retstring.append( "  Send RET command to the TCS. This moves the telescope so the DRA\n" );
      retstring.append( "  and DDEC offsets on the telescope status display go to zero.\n" );
      return HELP;
    }

    // Before sending PT command, check for non-zero offset rates.
    //
    if ( this->offsetrate  < 1 || this->offsetrate >  50 ) {
      message.str(""); message << "ERROR offset rate " << this->offsetrate << " outside range {1:50}";
      logwrite( function, message.str() );
      return ERROR;
    }

    // read the current offsets in order to estimate the time to offset
    //
    double raoff, decoff;
    if ( this->get_offsets( raoff, decoff ) == ERROR ) {
      logwrite( function, "ERROR getting current offsets" );
      return ERROR;
    }

    // offsetrate is offset rates in arcsec/sec
    // so ra_t/offsetrate and dec_t/offsetrate are arcsec / arcsec / sec = sec
    //
    int ra_t  = static_cast<int>( 1000 * raoff  / this->offsetrate  );  // time (ms) to offset RA
    int dec_t = static_cast<int>( 1000 * decoff / this->offsetrate );   // time (ms) to offset DEC
    int max_t = static_cast<int>( 1.2 * std::max( ra_t, dec_t ) );      // greater of those two times + 50%
    max_t = std::max( max_t, 100 );                                     // minimum 100 msec

    long error = this->send_command( "RET", retstring, TCS::FAST_RESPONSE );  // perform the offset here

    std::this_thread::sleep_for( std::chrono::milliseconds( max_t ) );     // delay for offset before returning

    return error;
  }
  /***** TCS::Interface::ret_offsets ******************************************/


  /***** TCS::Interface::send_command *****************************************/
  /**
   * @brief      writes the raw command, as received, to the TCS
   * @param[in]  cmd        string to send to TCS
   * @param[out] retstring  reference to return string
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &retstring, TCS::ConnectionType conn_type ) {
    return send_command( cmd, retstring, conn_type, POLLTIMEOUT );
  }
  long Interface::send_command( std::string cmd, std::string &retstring, TCS::ConnectionType conn_type, int to ) {
    std::string function = "TCS::Interface::send_command";
    std::stringstream message;
    std::string sbuf;

    if ( this->name.empty() ) {
      message.str(""); message << "ERROR sending \"" << cmd << "\": no connection open to the TCS" ;
      logwrite( function, message.str() );
      return ERROR;
    }

    // Find the currently opened TCS (this->name) in the tcsmap.
    // It must be located and connected in order to proceed.
    //
    auto tcsloc = this->tcsmap.find( this->name );

    // Is it configured?
    //
    if ( tcsloc == tcsmap.end() ) {
      message.str(""); message << "ERROR sending \"" << cmd << "\": TCS " << this->name << " not configured";
      logwrite( function, message.str() );
      return ERROR;
    }

    TcsIO &tcs = *(tcsloc->second);

    std::string reply;

message.str(""); message << "DEBUG] sending cmd=" << cmd << " with type=" << (conn_type==TCS::FAST_RESPONSE?"fast":"slow") << " and to=" << to;
logwrite(function,message.str());
    tcs.execute_command( cmd, reply, conn_type, to );
logwrite(function,"[DEBUG] back from cmd="+cmd+" with reply="+reply);

    // Success or failure depends on what's in the TCS reply,
    // which depends on the command.
    //
    if ( cmd == "?MOTION" ) {                           // ?MOTION replys have unique codes
      return( parse_motion_code( reply, retstring ) );  // which are translated into retstring here.
    }
    else                                                // These commands reply with information (not a code)...
    if ( cmd == "?NAME"        ||
         cmd == "?PARALLACTIC" ||
         cmd == "?WEATHER"     ||
         cmd == "RAWDEC"       ||
         cmd == "RAWPOS"       ||
         cmd == "REQPOS"       ||
         cmd == "REQSTAT"         ) {
      retstring = reply;                                // ... so put their information into retstring.
      return NO_ERROR;                                  // It would have failed on send if ever.
    }
    else {                                              // Everything else returns a code,
      return( parse_reply_code( reply, retstring ) );   // which is translated here.
    }
  }
  /***** TCS::Interface::send_command *****************************************/


  /***** TCS::Interface::native ***********************************************/
  /**
   * @brief      send a command directly to the TCS
   * @param[in]  args       command to send to TCS
   * @param[out] retstring  return from TCS
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::native( std::string args, std::string &retstring ) {

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = TCSD_NATIVE;
      retstring.append( " <cmd>\n" );
      retstring.append( "  Send command <cmd> directly to the TCS and return the reply, which is\n" );
      retstring.append( "  not parsed. The cmd will be converted to uppercase before sending.\n" );
      return HELP;
    }

    make_uppercase( args );

    long error = this->send_command( args, retstring, TCS::SLOW_RESPONSE );

    // The TCS contains messages that have fields separated by newlines. Replace those with commas.
    //
    std::replace( retstring.begin(), retstring.end(), '\n', ',');

    return error;
  }
  /***** TCS::Interface::native ***********************************************/


  /***** TCS::Interface::parse_reply_code *************************************/
  /**
   * @brief      parse the TCS reply code into a friendly string
   * @param[in]  codein  input reply code string
   * @param[out] reply   reference to string to contain friendly reply
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::parse_reply_code( std::string codein, std::string &reply ) {
    const std::string function("TCS::Interface::parse_reply_code");
    std::stringstream message;
    int code = TCS_UNDEFINED;

    // Convert the reply code from string to integer
    //
    try {
      code = std::stoi( codein );
    }
    catch( const std::exception &e ) {
      message.str(""); message << "ERROR parsing TCS return value \"" << codein << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    // Convert the reply code integer to a friendly string
    //
    switch ( code ) {
      case TCS_SUCCESS:              reply = TCS_SUCCESS_STR;
                                     return NO_ERROR;
      case TCS_UNRECOGNIZED_COMMAND: reply = TCS_UNRECOGNIZED_COMMAND_STR;
                                     return ERROR;
      case TCS_INVALID_PARAMETER:    reply = TCS_INVALID_PARAMETER_STR;
                                     return ERROR;
      case TCS_UNABLE_TO_EXECUTE:    reply = TCS_UNABLE_TO_EXECUTE_STR;
                                     return ERROR;
      case TCS_HOST_UNAVAILABLE:     reply = TCS_HOST_UNAVAILABLE_STR;
                                     return ERROR;
      case TCS_UNDEFINED:
      default:                       reply = TCS_UNDEFINED_STR;
                                     logwrite( function, "ERROR undefined reply code: "+codein );
                                     return ERROR;
    }
  }
  /***** TCS::Interface::parse_reply_code *************************************/


  /***** TCS::Interface::parse_motion_code ************************************/
  /**
   * @brief      parse the TCS reply code into a friendly string
   * @param[in]  codein     input reply code string
   * @param[out] retstring  reference to return string
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::parse_motion_code( std::string codein, std::string &retstring ) {
    std::string function = "TCS::Interface::parse_motion_code";
    std::stringstream message;
    int motion_code = TCS_MOTION_UNKNOWN;

    // Response will be a number that is converted to int here
    //
    try {
      motion_code = std::stoi( codein );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR converting TCS reply \"" << codein << "\" to integer: " << e.what();
      logwrite( function, message.str() );
      retstring="invalid_reply";
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR converting TCS reply \"" << codein << "\" to integer: " << e.what();
      logwrite( function, message.str() );
      retstring="invalid_reply";
      return ERROR;
    }

    // set the return string based on the TCS motion code
    //
    switch( motion_code ) {
      case TCS_MOTION_STOPPED:    retstring = TCS_MOTION_STOPPED_STR;
                                  return NO_ERROR;
      case TCS_MOTION_SLEWING:    retstring = TCS_MOTION_SLEWING_STR;
                                  return NO_ERROR;
      case TCS_MOTION_OFFSETTING: retstring = TCS_MOTION_OFFSETTING_STR;
                                  return NO_ERROR;
      case TCS_MOTION_TRACKING:   retstring = TCS_MOTION_TRACKING_STR;
                                  return NO_ERROR;
      case TCS_MOTION_SETTLING:   retstring = TCS_MOTION_SETTLING_STR;
                                  return NO_ERROR;
      case TCS_MOTION_UNKNOWN:
      default:                    retstring = TCS_MOTION_UNKNOWN_STR;
                                  retstring = "unknown_reply";
                                  message.str(""); message << "ERROR unrecognized motion code " << motion_code;
                                  logwrite( function, message.str() );
                                  return ERROR;
    }
  }
  /***** TCS::Interface::parse_motion_code ************************************/


  /***** TCS::TcsInfo::parse_weather ******************************************/
  void TcsInfo::parse_weather( std::string &input ) {
    const std::string function("TCS::TcsInfo::parse_weather");
    std::stringstream message;

    // input string is expected to be:
    //
    // RA=0.00000000,DEC=0.00000000,HA=0,LST=0,Air mass=1.000,Azimuth=0,
    // Zenith angle=0,Focus Point=36.710,Dome Azimuth=0,Dome shutters=0,
    // Windscreen position=0,InstPos=0,Instrument=NGPS0,Pumps=0 DONE
    //
    // Remove "DONE"
    //
    size_t pos = input.find("DONE");
    if ( pos != std::string::npos ) input.erase( pos, 4 );

    // Tokenize on the comma "," and expect 14 tokens
    //
    std::vector<std::string> tokens;
    Tokenize( input, tokens, "," );

    if ( tokens.size() != 14 ) {
      message << "ERROR expected 14 tokens but got " << tokens.size() << " from \"" << input << "\"";
      logwrite( function, message.str() );
      return;
    }

    // For each token of interest the value is from the equal sign "="
    // to the end of the string.
    //
    try {
      pos = tokens.at(0).find("="); this->ra_h_dec     = std::stod(tokens.at(0).substr(pos+1));
      pos = tokens.at(1).find("="); this->dec_d_dec    = std::stod(tokens.at(1).substr(pos+1));
      pos = tokens.at(5).find("="); this->azimuth      = std::stod(tokens.at(5).substr(pos+1));
      pos = tokens.at(6).find("="); this->zenithangle  = std::stod(tokens.at(6).substr(pos+1));
      pos = tokens.at(8).find("="); this->domeazimuth  = std::stod(tokens.at(8).substr(pos+1));
      pos = tokens.at(9).find("="); this->domeshutters = std::stoi(tokens.at(9).substr(pos+1));
    }
    catch ( const std::exception &e ) {
      message << "ERROR parsing input: " << e.what();
      logwrite( function, message.str() );
      return;
    }
    return;
  }


  void TcsInfo::parse_reqstat( std::string &input ) {
    const std::string function("TCS::TcsInfo::parse_weather");
    std::stringstream message;

    // input string is expected to be:
    //
    // UTC = 278 23:53:48.0,telescope ID = 200, focus = 36.71 mm,
    // tube length = 22.11 mm,offset RA = 45.00 arcsec, DEC = 45.00 arcsec,
    // rate RA = 0.00 arcsec/hr, DEC = 0.00 arcsec/hr,Cass ring angle = 0.00 DONE
    //
    // Remove "DONE"
    //
    size_t pos = input.find("DONE");
    if ( pos != std::string::npos ) input.erase( pos, 4 );

    // Tokenize on the comma "," and expect 9 tokens
    //
    std::vector<std::string> tokens;
    Tokenize( input, tokens, "," );

    if ( tokens.size() != 9 ) {
      message << "ERROR expected 9 tokens but got " << tokens.size() << " from \"" << input << "\"";
      logwrite( function, message.str() );
      return;
    }

    // For each token of interest the value is from the equal sign "="
    // to the end of the string.
    //
    try {
      pos = tokens.at(2).find("="); this->focus          = std::stod(tokens.at(2).substr(pos+1));
      pos = tokens.at(4).find("="); this->offsetra       = std::stod(tokens.at(4).substr(pos+1));
      pos = tokens.at(5).find("="); this->offsetdec      = std::stod(tokens.at(5).substr(pos+1));
      pos = tokens.at(6).find("="); this->offsetrate     = std::stod(tokens.at(6).substr(pos+1));
      pos = tokens.at(7).find("="); this->offsetrate     = std::stod(tokens.at(7).substr(pos+1));
      pos = tokens.at(8).find("="); this->cassangle      = std::stod(tokens.at(8).substr(pos+1));
    }
    catch ( const std::exception &e ) {
      message << "ERROR parsing input: " << e.what();
      logwrite( function, message.str() );
      return;
    }
    return;
  }

  void TcsInfo::parse_reqpos( std::string &input ) {
    const std::string function="TCS::TcsInfo::parse_weather";
    std::stringstream message;

    // input string is expected to be:
    //
    // UTC = 278 23:56:11.0, LST = 00:00:00,RA = +00:00:00.000,
    // DEC = +00:00:00.000, HA=W00:00:00.0,air mass = 1.000 DONE
    //
    // Remove "DONE"
    //
    size_t pos = input.find("DONE");
    if ( pos != std::string::npos ) input.erase( pos, 4 );

    // Tokenize on the comma "," and expect 6 tokens
    //
    std::vector<std::string> tokens;
    Tokenize( input, tokens, "," );

    if ( tokens.size() != 6 ) {
      message << "ERROR expected 6 tokens but got " << tokens.size() << " from \"" << input << "\"";
      logwrite( function, message.str() );
      return;
    }

    // For each token of interest the value is from the equal sign "="
    // to the end of the string.
    //
    try {
      pos = tokens.at(0).find("="); this->utc     = tokens.at(0).substr(pos+1);
      pos = tokens.at(1).find("="); this->lst     = tokens.at(1).substr(pos+1);
      pos = tokens.at(2).find("="); this->ra_hms  = tokens.at(2).substr(pos+1);
      pos = tokens.at(3).find("="); this->dec_dms = tokens.at(3).substr(pos+1);
      pos = tokens.at(4).find("="); this->ha      = tokens.at(4).substr(pos+1);
      pos = tokens.at(5).find("="); this->airmass = std::stod(tokens.at(5).substr(pos+1));
    }
    catch ( const std::exception &e ) {
      message << "ERROR parsing input: " << e.what();
      logwrite( function, message.str() );
      return;
    }
    return;
  }

}
