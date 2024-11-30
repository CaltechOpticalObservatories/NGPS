/**
 * @file    slit_interface.cpp
 * @brief   this contains the slit interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Slit namespace,
 * and is how the slit daemon interfaces to the slit hardware.
 *
 */

#include "slit_interface.h"

namespace Slit {

  float SlitDimension::_arcsec_per_mm = NAN;  /// initialized by Slit::Server::configure_slitd

  /***** Slit::Interface::initialize_class ************************************/
  /**
   * @brief      initializes the class from configure_slitd()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Slit::Server::configure_slitd() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Slit::Interface::initialize_class";
    std::stringstream message;
    long error = NO_ERROR;

    auto _motormap = this->motorinterface.get_motormap();

    this->numdev = _motormap.size();

    if ( this->numdev == 2 ) {
      logwrite( function, "interface initialized ok" );
      error = NO_ERROR;
    }
    else if ( this->numdev > 2 ) {
      message.str(""); message << "ERROR: too many motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }
    else if ( this->numdev == 1 ) {
      logwrite( function, "ERROR: only one motor controller was defined" );
      error = ERROR;
//    logwrite( function, "WARNING: limited slit range with only one motor controller" );  // consider allowing this?
//    error = NO_ERROR;
    }
    else if ( this->numdev < 1 ) {
      message.str(""); message << "ERROR: no motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }

    this->maxwidth = SlitDimension(0, Unit::MM);

    // Loop through the expected motor names, and
    // sum their min/max values which will have to be the min/max slit width.
    //
    std::vector<std::string> motornames = { "A", "B" };  // expecting motors with these names!

    for ( const auto &name : motornames ) {
      auto loc = _motormap.find( name );

      if ( loc != _motormap.end() ) {
        if ( _motormap[name].axes.find(1) != _motormap[name].axes.end() ) {
          this->maxwidth.mm() += (_motormap[name].axes[1].max-this->center.mm());
        }
        else {
          message.str(""); message << "ERROR motor " << name << " missing configuration for axis 1";
          logwrite( function, message.str() );
          error = ERROR;
        }
      }
      else {
        logwrite( function, "ERROR missing configuration for motor \"A\"" );
        error = ERROR;
      }
    }

    return( error );
  }
  /***** Slit::Interface::initialize_class ************************************/


  /***** Slit::Interface::open ************************************************/
  /**
   * @brief      opens the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    long error = NO_ERROR;

    // Open the sockets,
    // clear any error codes on startup, and
    // enable the servo for each address in controller_info.
    //
    error |= this->motorinterface.open();
    error |= this->motorinterface.clear_errors();
    error |= this->motorinterface.set_servo( true );

    return error;
  }
  /***** Slit::Interface::open ************************************************/


  /***** Slit::Interface::close ***********************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    return this->motorinterface.close();
  }
  /***** Slit::Interface::close ***********************************************/


  /***** Slit::Interface::is_open *********************************************/
  /**
   * @brief       return the connected state of the motor controllers
   * @param[in]   arg        used only for help
   * @param[out]  retstring  contains the connected state "true" | "false"
   * @return      ERROR | NO_ERROR | HELP
   *
   * All motors must be connected for this to return "true".
   *
   */
  long Interface::is_open( std::string arg, std::string &retstring ) {
    std::string function = "Slit::Interface::is_open";
    std::stringstream message;
    long error = NO_ERROR;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( arg == "?" ) {
      retstring = SLITD_ISOPEN;
      retstring.append( " \n" );
      retstring.append( "  Returns true if all controllers are connected, false if any one is not connected.\n" );
      return HELP;
    }

    // Loop through all motor controllers, checking each if connected,
    // and keeping count of the number that are connected.
    //
    size_t num_open=0;
    std::string unconnected, connected;

    for ( const auto &mot : _motormap ) {

      bool _isopen = this->motorinterface.is_connected( mot.second.name );

      num_open += ( _isopen ? 1 : 0 );

      unconnected.append ( _isopen ? "" : " " ); unconnected.append ( _isopen ? "" : mot.second.name );
      connected.append   ( _isopen ? " " : "" ); connected.append   ( _isopen ? mot.second.name : "" );
    }

    // Set the retstring true or false, true only if all controllers are homed.
    //
    if ( num_open == _motormap.size() ) retstring = "true"; else retstring = "false";

    // Log who's connected and not
    //
    if ( !connected.empty() ) {
      message.str(""); message << "connected to" << connected;
      logwrite( function, message.str() );
    }
    if ( !unconnected.empty() ) {
      message.str(""); message << "not connected to" << unconnected;
      logwrite( function, message.str() );
    }

    return error;
  }
  /***** Slit::Interface::is_open *********************************************/


  /***** Slit::Interface::home ************************************************/
  /**
   * @brief      home all daisy-chained motors
   * @details    Both motors are homed simultaneously by spawning a thread for
   *             each. This will also apply any zeropos, after homing.
   * @param[in]  arg        optional arg for help only
   * @param[out] retstring  reference to return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::home( std::string arg, std::string &retstring ) {

    // Help
    //
    if ( arg == "?" ) {
      retstring = SLITD_HOME;
      retstring.append( " \n" );
      retstring.append( "  home both slit motors simultaneously\n" );
      return HELP;
    }

    if ( ! arg.empty() ) {
      retstring="invalid_argument";
      logwrite( "Slit::Interface::home", "ERROR expected no arguments" );
      return ERROR;
    }

    // All the work is done by the PI motor interface class
    //
    return this->motorinterface.home( arg, retstring );
  }
  /***** Slit::Interface::home ************************************************/


  /***** Slit::Interface::is_home *********************************************/
  /**
   * @brief       return the home state of the motors
   * @param[in]  arg        optional arg for help only
   * @param[out]  retstring  contains the home state "true" | "false"
   * @return      ERROR | NO_ERROR | HELP
   *
   * All motors must be homed for this to return "true".
   *
   */
  long Interface::is_home( std::string arg, std::string &retstring ) {

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = SLITD_ISHOME;
      retstring.append( " \n" );
      retstring.append( "  returns referencing state of both slit motors\n" );
      return HELP;
    }

    if ( ! arg.empty() ) {
      retstring="invalid_argument";
      logwrite( "Slit::Interface::is_home", "ERROR expected no arguments" );
      return ERROR;
    }

    // All the work is done by the PI motor interface class
    //
    return this->motorinterface.is_home( arg, retstring );
  }
  /***** Slit::Interface::is_home *********************************************/


  /***** Slit::Interface::set *************************************************/
  /**
   * @brief      set the slit width and offset
   * @param[in]  iface      reference to main Slit::Interface object
   * @param[in]  args       string containing width, or width and offset
   * @param[out] retstring  string contains the width and offset after move
   * @return     ERROR | NO_ERROR | HELP
   *
   * This function moves the "A" and "B" motors to achieve the requested
   * width (and offset, if specified, default 0). Each motor is commanded in its
   * own thread so that they can be moved in parallel.
   *
   * This function requires a reference to the slit interface object because it's
   * going to spawn threads for each motor and the threads, being static, would
   * not otherwise have access to this-> object.
   *
   */
  long Interface::set( Slit::Interface &iface, std::string args, std::string &retstring ) {
    std::string function = "Slit::Interface::set";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLITD_SET;
      retstring.append( " <width> [ <offset> ]\n" );
      retstring.append( "  Set <width> and optionally <offset>. If offset is omitted then\n" );
      retstring.append( "  both motors are moved symmetrically to achieve the requested width.\n" );
      retstring.append( "  Using an offset will reduce the maximum width available.\n\n" );
      retstring.append( "  Default units are arcsec. Add \"mm\" to use actuator units,\n" );
      retstring.append( "  E.G. "+SLITD_SET+" 5mm to set a slit width of 5mm\n\n" );
      message.str(""); message << "  For 0 offset, range of allowable slit widths is { "
                               << minwidth.arcsec() <<" : " << maxwidth.arcsec() << " } arcsec\n";
      retstring.append( message.str() );
      return HELP;
    }

    // Tokenize the input arg list.
    // Expecting either 1 token <width> for default zero offset
    // or 2 tokens <width> <offset>
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() > 2 || tokens.size() < 1 ) {
      message.str(""); message << "bad number of arguments: " << tokens.size() 
                               << ". expected <width> or <width> <offset>";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    SlitDimension reqwidth, reqoffset;

    // tokens.size() is guaranteed to be either 1 OR 2 at this point
    //
    try {
      // default unit is arcsec, can override if "mm" is present with the value
      auto unit = Unit::ARCSEC;

      // Get the reqwidth, reqoffset in the requested unit (default arcsecC)
      //
      switch ( tokens.size() ) {

        case 2:    // the 2nd arg is the setoffset (and if not set here then use default above)
          if ( tokens.at(1).find("mm") != std::string::npos ) unit=Unit::MM; else unit=Unit::ARCSEC;
          reqoffset = SlitDimension( std::stof( tokens.at(1) ), unit );

          // do not break!
          // let this case drop through because if there is a 2nd arg then there's a 1st

        case 1:    // the 1st arg is the reqwidth
          if ( tokens.at(0).find("mm") != std::string::npos ) unit=Unit::MM; else unit=Unit::ARCSEC;
          reqwidth = SlitDimension( std::stof( tokens.at(0) ), unit );
          break;

        default:   // impossible! because I already checked that tokens.size was 1 or 2
          logwrite( function, "ERROR args: "+args );
          retstring="internal_error";
          return ERROR;
      }
    }
    catch( const std::exception &e ) {
      logwrite( function, "ERROR parsing args "+args+": "+e.what() );
      retstring="invalid_argument";
      return ERROR;
    }

    // Now range-check values
    //
    if ( reqwidth.arcsec() > this->maxwidth.arcsec() || reqwidth.arcsec() < this->minwidth.arcsec() ) {
      message.str(""); message << "ERROR: width " << reqwidth.arcsec() << " out of range. expected { "
                               << minwidth.arcsec() <<" : " << maxwidth.arcsec() << " }";
      logwrite( function, message.str() );
      retstring="width_exceeds_range";
      return ERROR;
    }

    // range check requested width and offset
    //
    SlitDimension maxoffset( ( ( this->maxwidth.arcsec() - reqwidth.arcsec() ) / 2.0 ), Unit::ARCSEC );
    if ( std::abs( reqoffset.arcsec() ) > maxoffset.arcsec() ) {
      message.str(""); message << "ERROR: requested offset " << std::abs(reqoffset.arcsec())
                               << " exceeds maximum " << maxoffset.arcsec() << " allowed for this width";
      logwrite( function, message.str() );
      retstring="offset_exceeds_range";
      return ERROR;
    }

    if ( reqoffset.arcsec() < 0 && this->numdev < 2 ) {
      message.str(""); message << "ERROR: negative offset " << reqoffset.arcsec() << " not allowed with only one motor";
      logwrite( function, message.str() );
      retstring="invalid_offset";
      return ERROR;
    }

    // requested actuator positions
    //
    auto posA = SlitDimension( this->center.mm() + reqwidth.mm()/2. + reqoffset.mm(), Unit::MM );
    auto posB = SlitDimension( this->center.mm() + reqwidth.mm()/2. - reqoffset.mm(), Unit::MM );

    // actuator limits
    //
    auto _motormap = this->motorinterface.get_motormap();

    auto minA = SlitDimension( _motormap["A"].axes[1].min, Unit::MM );
    auto maxA = SlitDimension( _motormap["A"].axes[1].max, Unit::MM );
    auto minB = SlitDimension( _motormap["B"].axes[1].min, Unit::MM );
    auto maxB = SlitDimension( _motormap["B"].axes[1].max, Unit::MM );

    if ( posA < minA || posA > maxA ) {
      message.str(""); message << "ERROR requested actuator A position " << posA.arcsec()
                               << " outside range { " << minA.arcsec() << " : " << maxA.arcsec() << " }"
                               << " for this width";
      logwrite( function, message.str() );
      retstring="actuatorA_outside_range";
      return ERROR;
    }

    if ( posB < minB || posB > maxB ) {
      message.str(""); message << "ERROR requested actuator B position " << posB.arcsec()
                               << " outside range { " << minB.arcsec() << " : " << maxB.arcsec() << " }"
                               << " for this width";
      logwrite( function, message.str() );
      retstring="actuatorB_outside_range";
      return ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] posA=" << posA.mm() << " posB=" << posB.mm() << " width=" << reqwidth.arcsec()
                             << " offset=" << reqoffset.arcsec();
    logwrite( function, message.str() );
#endif

    std::stringstream posAstring, posBstring;
    posAstring << std::setprecision(3) << std::fixed << posA.mm();
    posBstring << std::setprecision(3) << std::fixed << posB.mm();

    // move the A and B motors now together --
    // When motorinterface::moveto() receives vectors then it will move
    // all motors in the vector simultaneously in separate threads.
    //
    std::vector<std::string> motornames = { "A", "B" };
    std::vector<int> axisnums = { 1, 1 };
    std::vector<std::string> positions = { posAstring.str(), posBstring.str() };

    error = this->motorinterface.moveto( motornames, axisnums, positions, retstring );

    // after all the moves, read and return the position
    //
    if ( error == NO_ERROR ) error = this->get( retstring );

    return error;
  }
  /***** Slit::Interface::set *************************************************/


  /***** Slit::Interface::get *************************************************/
  /**
   * @brief      get the current width and offset
   * @param[in]  args       input args (currently just for help)
   * @param[out] retstring  string contains the current width and offset
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get( std::string args, std::string &retstring ) {
    std::string function = "Slit::Interface::get";
    std::stringstream message;
    long error  = NO_ERROR;
    float posA=NAN;
    float posB=NAN;
    float poswidth=NAN;
    float posoffset=NAN;

    // Help
    //
    if ( args == "?" || args == "help" || args == "-h" ) {
      retstring = SLITD_GET;
      retstring.append( " \n" );
      retstring.append( "  returns the current slit width and offset in arcseconds\n" );
      return HELP;
    }

    // this call reads the controller and returns the numeric values
    //
    error = this->read_positions( poswidth, posoffset, posA, posB );

    // store the current readings in the class
    //
    this->width  = SlitDimension( poswidth, Unit::MM );
    this->offset = SlitDimension( posoffset, Unit::MM );

    // form the return value
    //
    std::stringstream s;
    s << this->width.arcsec() << " " << this->offset.arcsec();
    retstring = s.str();

    message.str(""); message << "NOTICE:" << Slit::DAEMON_NAME << " " << retstring;
    this->async.enqueue( message.str() );

    return error;
  }
  /***** Slit::Interface::get *************************************************/
  long Interface::get( std::string &retstring ) {
    return this->get( "", retstring );
  }
  /***** Slit::Interface::get *************************************************/


  /***** Slit::Interface::read_positions **************************************/
  /**
   * @brief      read the actuator positions
   * @param[out] poswidth   width in actuator units (mm)
   * @param[out] posoffset  offset in actuator units (mm)
   * @param[out] posA       actuator A position in mm
   * @param[out] posB       actuator B position in mm
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::read_positions( float &poswidth, float &posoffset, float &posA, float &posB ) {

    long error = NO_ERROR;

    // check here to guard against divide-by-zero
    //
    if ( this->numdev == 0 ) {
      logwrite( "Slit::Interface::read_positions", "ERROR no motor controllers defined!" );
      return ERROR;
    }

    // get the actuator position for each address in controller_info
    //
    int axis=1;
    error |= this->motorinterface.get_pos( "A", axis, posA );
    error |= this->motorinterface.get_pos( "B", axis, posB );

    // calculate poswidth and posoffset from actuator positions,
    // which is width and offset in actuator units (mm)
    //
    poswidth  = ( ( posA - this->center.mm() ) + ( posB - this->center.mm() ) );
    posoffset = ( ( posA - posB ) / this->numdev );

    return error;
  }
  /***** Slit::Interface::read_positions **************************************/
  /**
   * @brief      read the actuator positions
   * @details    This overloads read_positions to return only width and offset.
   * @param[out] poswidth   width in actuator units (mm)
   * @param[out] posoffset  offset in actuator units (mm)
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::read_positions( float &poswidth, float &posoffset ) {
    float posA=NAN, posB=NAN;
    return read_positions( poswidth, posoffset, posA, posB );
  }
  /***** Slit::Interface::read_positions **************************************/


  /***** Slit::Interface::stop ************************************************/
  /**
   * @brief      send the stop-all-motion command to all controllers
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::stop( ) {
    std::string function = "Slit::Interface::stop";
    std::stringstream message;

    // send the stop_motion command for each address in controller_info
    //
    auto _motormap = this->motorinterface.get_motormap();

    for ( const auto &mot : _motormap ) {
      this->motorinterface.stop_motion( mot.second.name, mot.second.addr );
    }

    return NO_ERROR;
  }
  /***** Slit::Interface::stop ************************************************/


  /***** Slit::Interface::send_command ****************************************/
  /**
   * @brief      writes the raw command as received to the master controller
   * @param[in]  args       contains <name> <cmd> [ <axis> <arg> ]
   * @param[out] retstring  reference to contain any return string
   * @return     ERROR | NO_ERROR | HELP
   *
   * This function is overloaded.
   * This version writes a command that expects no reply.
   *
   */
  long Interface::send_command( std::string args, std::string &retstring ) {
    std::string function = "Slit::Interface::send_command";
    std::stringstream message;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = SLITD_NATIVE;
      retstring.append( " <name> <cmd> [ <axis> <arg> ] \n" );
      retstring.append( "  sends <cmd> directly to the controller named <name>,\n" );
      retstring.append( "  where <name> is in { " );
      for ( const auto &mot : _motormap ) { retstring.append( mot.first ); retstring.append(" "); }
      retstring.append( "}\n" );
      retstring.append( "  No checks are made as to the validity of the command string. Note that\n" );
      retstring.append( "  some commands require specifying the axisid and may have additional args.\n" );
      retstring.append( "  If <cmd> does not contain a \"?\" then there is no return string.\n" );
      return HELP;
    }

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    // expecting minimally <name> <cmd> tokens
    //
    if ( tokens.size() < 2 ) {
      logwrite( function, "ERROR expected at least two args: <name> <cmd>" );
      retstring="invalid_argument";
      return ERROR;
    }

    // don't need try/catch because already verified there are two elements
    //
    std::string name   = tokens[0];

    // The command may contain multiple string-separated tokens so
    // remove the name, tokens[0] from the args and send the rest as
    // the command.
    //
    auto nameloc = args.find(name);
    std::string cmd_in = args.substr(nameloc+1);

    // find name in the motormap
    //
    if ( _motormap.find(name) == _motormap.end() ) {
      message.str(""); message << "ERROR \"" << name << "\" not found in motormap";
      logwrite( function, message.str() );
      return ERROR;
    }

    // once the name is verified, get the addr because it needs to be
    // sent with the <cmd>
    //
    auto addr = _motormap[name].addr;

    std::stringstream cmd;
    if ( addr > 0 ) cmd << addr << " ";
    cmd << cmd_in;

    if ( cmd.str().find( "?" ) != std::string::npos ) {
      return( this->motorinterface.send_command( name, cmd.str(), retstring ) );
    }
    else {
      return( this->motorinterface.send_command( name, cmd.str() ) );
    }
  }
  /***** Slit::Interface::send_command ****************************************/


  /***** Slit::Interface::make_telemetry_message ******************************/
  /**
   * @brief      assembles a telemetry message
   * @details    This creates a JSON message for my telemetry info, then serializes
   *             it into a std::string ready to be sent over a socket.
   * @param[out] retstring  string containing the serialization of the JSON message
   *
   */
  void Interface::make_telemetry_message( std::string &retstring ) {
    const std::string function="Slit::Interface::make_telemetry_message";

    // assemble my telemetry into a json message
    // Set a messagetype keyword to indicate what kind of message this is.
    //
    nlohmann::json jmessage;
    jmessage["messagetype"]="slitinfo";

    // There needs to be at least one device defined
    //
    if ( this->numdev > 0 ) {
      // get the position for each address in controller_info
      //
      float poswidth=NAN, posoffset=NAN, posA=NAN, posB=NAN;
      this->read_positions( poswidth, posoffset, posA, posB );

      auto width  = SlitDimension( poswidth, Unit::MM );
      auto offset = SlitDimension( posoffset, Unit::MM );

      // fill the jmessage with the positions just retrieved
      //
      if ( !std::isnan(poswidth) )  jmessage["SLITW"]    = width.arcsec();
      if ( !std::isnan(posoffset) ) jmessage["SLITO"]    = offset.arcsec();
      if ( !std::isnan(posA) )      jmessage["SLITPOSA"] = posA;
      if ( !std::isnan(posB) )      jmessage["SLITPOSB"] = posB;
    }

    try {
      this->publisher->publish( jmessage );
    }
    catch ( const std::exception &e ) {
      logwrite( "Slit::Interface::make_telemetry_message",
                "ERROR publishing message: "+std::string(e.what()) );
      return;
    }

    retstring = jmessage.dump();  // serialize the json message into retstring

    retstring.append(JEOF);       // append the JSON message terminator

    return;
  }
  /***** Slit::Interface::make_telemetry_message ******************************/


  void Interface::handletopic_snapshot( const nlohmann::json &jmessage ) {
    if ( jmessage.contains( Slit::DAEMON_NAME ) ) {
      std::string dontcare;
      this->make_telemetry_message(dontcare);
    }
    else
    if ( jmessage.contains( "test" ) ) {
      logwrite( "Slit::Interface::handletopic_snapshot", jmessage.dump() );
    }
  }


  /*
  void Interface::send_telemetry_message() {
    nlohmann::json jmessage;
    jmessage["SLITW"]    = this->width.arcsec();
    jmessage["SLITO"]    = this->offset.arcsec();
    jmessage["SLITPOSA"] = posA;
    jmessage["SLITPOSB"] = posB;
  }
  */
}
