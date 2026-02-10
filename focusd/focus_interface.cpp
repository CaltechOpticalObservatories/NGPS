/**
 * @file    focus_interface.cpp
 * @brief   this contains the focus interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Focus namespace,
 * and is how the focus daemon interfaces to the focus hardware.
 *
 */

#include "focus_interface.h"

namespace Focus {

  /***** Focus::Interface::open ***********************************************/
  /**
   * @brief      opens the socket connection and enables motion
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    long error = NO_ERROR;

    // Open connection to all motors
    for (auto &mot : this->motors) error |= mot.second.open();

    // clear any error codes on startup (PI-only)
    this->pi_interface->clear_errors();

    // enable motion (servo for PI) for each address in controller_info.
    for (auto &mot : this->motors) error |= mot.second.enable_motion();

    // Focus controllers interface via a terminal server, so it's possible to
    // have open a socket connection and not actually be connected to the motor
    // controllers. Therefore, determine connection success by ensuring that
    // .clear_error() and .set_servo() succeed. If there's any error then close
    // the connection.
    //
    if ( error != NO_ERROR ) this->close();

    return error;
  }
  /***** Focus::Interface::open ***********************************************/


  /***** Focus::Interface::close **********************************************/
  /**
   * @brief      closes the socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    long error=NO_ERROR;
    for (auto &mot : this->motors) error |= mot.second.close();
    return error;
  }
  /***** Focus::Interface::close **********************************************/


  /***** Focus::Interface::is_open ********************************************/
  /**
   * @brief       return the connected state of the motor controllers
   * @param[in]   arg        used only for help
   * @param[out]  retstring  contains the connected state "true" | "false"
   * @return      NO_ERROR | HELP
   *
   * All motors must be connected for this to return "true".
   *
   */
  long Interface::is_open( std::string arg, std::string &retstring ) {
    const std::string function("Focus::Interface::is_open");

    // Help
    //
    if ( arg == "?" ) {
      retstring = FOCUSD_ISOPEN;
      retstring.append( " \n" );
      retstring.append( "  Returns true if all controllers are connected, false if any one is not connected.\n" );
      return HELP;
    }

    // Loop through all motor controllers, checking each if connected,
    // and keeping count of the number that are connected.
    //
    size_t num_open=0;
    std::string unconnected, connected;

    for (auto &mot : this->motors) {
      // am I connected?
      bool _isopen = mot.second.is_connected();
      // count number connected
      num_open += ( _isopen ? 1 : 0 );
      // make lists of connected|unconnected motors
      unconnected.append ( _isopen ? "" : " " ); unconnected.append ( _isopen ? "" : mot.first );
      connected.append   ( _isopen ? " " : "" ); connected.append   ( _isopen ? mot.first : "" );
    }

    // Set the retstring true or false, true only if all controllers are homed.
    //
    if ( num_open == this->motors.size() ) retstring = "true"; else retstring = "false";

    // Log who's connected and not
    //
    if ( !connected.empty() ) {
      logwrite(function, "connected to"+connected);
    }
    if ( !unconnected.empty() ) {
      logwrite(function, "not connected to"+unconnected);
    }

    return NO_ERROR;
  }
  /***** Focus::Interface::is_open ********************************************/


  /***** Focus::Interface::native *********************************************/
  /**
   * @brief      send native command to controller identified by channel name
   * @param[in]  args       contains channel name, command and arg(s)
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::native( std::string args, std::string &retstring ) {
    std::string function = "Focus::Interface::native";
    std::string chan, cmd;

    // Help
    //
    if ( args == "?" ) {
      retstring = FOCUSD_NATIVE;
      retstring.append( " <chan> <cmd>\n" );
      retstring.append( "  Send native command <cmd> to controller indicated by channel name,\n" );
      retstring.append( "  where <chan> is one of { " );
      for (auto &mot : this->motors) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "} and <cmd> is any motor controller native command and args. This command\n" );
      retstring.append( "blocks; native commands are not run in a separate thread.\n" );
      return HELP;
    }

    // Need something, anything
    //
    if ( args.empty() ) {
      logwrite( function, "ERROR expected <chan> <cmd>" );
      retstring="invalid_argument";
      return( ERROR );
    }

    std::transform( args.begin(), args.end(), args.begin(), ::toupper );

    std::size_t cmd_sep = args.find_first_of( " " );        // find first space, which separates <chan> from <cmd>

    if ( cmd_sep == std::string::npos ) {                   // no <cmd>
      logwrite( function, "ERROR expected <chan> <cmd>" );
      retstring="invalid_argument";
      return( ERROR );
    }
    else {
      chan = args.substr( 0, cmd_sep );                     // <chan> is first part before space
      cmd  = args.substr( cmd_sep+1  );                     // <cmad> is everything after the space
    }

    if ( this->motors.find(chan) == this->motors.end() ) {
      logwrite(function, "ERROR motor '"+chan+"' not found in configuration");
      retstring="unknown_motor";
      return ERROR;
    }

    // requires an open connection
    //
    if (!this->motors.at(chan).is_connected()) {
      logwrite( function, "ERROR not connected to motor controller" );
      retstring="not_connected";
      return ERROR;
    }

    std::ostringstream oss;
    oss << this->motors.at(chan).get_addr() << " " << cmd;

    logwrite(function, oss.str());

    return send_command( chan, oss.str(), retstring );
  }
  /***** Focus::Interface::native *********************************************/


  /***** Focus::Interface::home ***********************************************/
  /**
   * @brief      home all or indicated daisy-chained motors
   * @param[in]  name_in    optional list of motors to home
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::home( std::string name_in, std::string &retstring ) {

    // Help
    //
    if ( name_in == "?" || name_in == "help" ) {
      retstring = FOCUSD_HOME;
      retstring.append( " [ <axis> ]\n" );
      retstring.append( "  Home all focus motors or single motor indicated by optional <axis>.\n" );
      retstring.append( "  If no argument is supplied then all axes are homed simultaneously,\n" );
      retstring.append( "  or a single axis may be supplied from { " );
      for (auto &mot : this->motors) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "}.\n" );
      return HELP;
    }

    // All the work is done by the motor interface class
    //
    return this->motors.at(name_in).home(&retstring);
  }
  /***** Focus::Interface::home ***********************************************/


  /***** Focus::Interface::is_home ********************************************/
  /**
   * @brief       return the home state of the motors
   * @param[in]   name_in    optionally contains one or more motors to check
   * @param[out]  retstring  contains the home state "true" | "false"
   * @return      ERROR | NO_ERROR | HELP
   *
   * All motors must be homed for this to return "true".
   *
   */
  long Interface::is_home( std::string name_in, std::string &retstring ) {

    // Help
    //
    if ( name_in == "?" ) {
      retstring = FOCUSD_ISHOME;
      retstring.append( " [ " );
      for (auto &mot : this->motors) { retstring.append( mot.first ); retstring.append(" "); }
      retstring.append( "]\n" );
      retstring.append( "  Reads the referencing state from each of the indicated controllers,\n" );
      retstring.append( "  or all controllers if none supplied. Returns true if all (named) are\n" );
      retstring.append( "  homed, false if any one is not homed.\n" );
      return HELP;
    }

    // All the work is done by the motor interface class
    //
    try {
      retstring = (this->motors.at(name_in).is_home() ? "true" : "false");
    }
    catch (const std::exception &e) {
      retstring=std::string(e.what());
      return ERROR;
    }
    return NO_ERROR;
  }
  /***** Focus::Interface::is_home ********************************************/


  /***** Focus::Interface::set ************************************************/
  /**
   * @brief      set the focus position of the selected channel
   * @param[in]  args       contains channel name and focus position
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::set( std::string args, std::string &retstring ) {
    std::string function = "Focus::Interface::set";
    std::stringstream message;
    int axis=1;

    // Help
    //
    if ( args == "?" ) {
      retstring = FOCUSD_SET;
      retstring.append( " <chan> { <pos> | nominal }\n" );
      retstring.append( "  Set focus position of indicated channel to <pos> or to the nominal best focus.\n" );
      retstring.append( "  where <chan> <min> <nominal> <max> are as follows:\n" );
      /***
      try {
        for (auto &mot : this->motors) {
          auto ax = mot.second.axis(axis);
          auto pos = mot.second.posmap(std::string("nominal"));
          retstring.append( "     " );
          retstring.append( mot.first ); retstring.append( " " );
          message.str(""); message << std::fixed << std::setprecision(3) << (ax ? ax->min : NAN) << " ";
          retstring.append( message.str() );
          message.str(""); message << std::fixed << std::setprecision(3) << (pos ? pos->position : NAN) << " ";
          retstring.append( message.str() );
          message.str(""); message << std::fixed << std::setprecision(3) << (ax ? ax->max : NAN) << " ";
          retstring.append( message.str() );
          retstring.append( "\n" );
        }
      }
      catch( std::invalid_argument &e ) {
        message.str(""); message << "ERROR invalid argument: " << e.what();
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch( std::out_of_range &e ) {
        message.str(""); message << "ERROR out of range: " << e.what();
        logwrite( function, message.str() );
        return( ERROR );
      }
      ***/
      return HELP;
    }

    // Tokenize the arg string. Need two tokens: <chan> and either <pos> or "nominal"
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      logwrite( function, "ERROR expected <chan> { <pos> | nominal }" );
      retstring="invalid_argument";
      return( ERROR );
    }

    std::string chan   = tokens[0];
    std::string posstr = tokens[1];

    long error = this->motors.at(chan).moveto(axis, posstr, retstring);

    message.str("");
    message << ( error==NO_ERROR ? "success" : "failed" )
            << " moving " << chan << " focus to " << posstr;
    logwrite( function, message.str() );

    return error;
  }
  /***** Focus::Interface::set ************************************************/


  /***** Focus::Interface::get ************************************************/
  /**
   * @brief      get the current position of channel indicated by arg
   * @param[in]  name       string contains the channel name
   * @param[out] retstring  reference to string contains focus position
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get( std::string name, std::string &retstring ) {
    std::string function = "Focus::Interface::get";
    std::stringstream message;
    long error  = NO_ERROR;

    // Help
    //
    if ( name == "?" ) {
      retstring = FOCUSD_GET;
      retstring.append( " <chan>\n" );
      retstring.append( "  Get the focus position of the indicated channel\n" );
      retstring.append( "  where <chan> is one of { " );
      for (auto &mot : this->motors) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "}.\n" );
      return HELP;
    }

    // Need something, anything
    //
    if ( name.empty() ) {
      logwrite( function, "ERROR expected <chan>" );
      retstring="invalid_argument";
      return ERROR;
    }

    // okay, not anything, need valid motor name
    //
    if (this->motors.find(name) == this->motors.end()) {
      logwrite(function, "ERROR motor '"+name+"' not found in configuration");
      retstring="unknown_motor";
      return ERROR;
    }

    // get the position for the requested channel
    //
    std::string posstring;
    int axis=1;
    auto addr=this->motors.at(name).get_addr();
    float position=NAN;
    std::string posname;
    error = this->motors.at(name).get_pos(axis, addr, position, posname);

    // form the return value
    //
    message.str(""); message << name << " " << std::fixed << std::setprecision(3) << position;
    if ( ! posname.empty() ) { message << " (" << posname << ")"; }
    retstring = message.str();
    logwrite( function, message.str() );

    message.str(""); message << "NOTICE:" << Focus::DAEMON_NAME << " " << retstring;
    this->async.enqueue( message.str() );

    return( error );
  }
  /***** Focus::Interface::get ************************************************/


  /***** Focus::Interface::move_abs *******************************************/
  /**
   * @brief      send move-absolute command to specified controller
   * @param[in]  addr  controller address
   * @param[in]  pos   motor position
   * @return     ERROR or NO_ERROR
   *
   * This could be called by a thread, so hardware interactions with the
   * controller are protected by a mutex.
   *
   */
  long Interface::move_abs( int addr, float pos ) {
    const std::string function("Focus::Interface::move_abs");
    logwrite(function, "ERROR not implemented");
    return ERROR;
/*****
    std::stringstream message;
    long error=NO_ERROR;

    auto _motormap = this->pi_interface.get_motormap();

    int axis=1;

    // which controller has this addr?
    //
    std::string name;
    for ( const auto &mot : _motormap ) {
      if ( mot.second.addr == addr ) {
        name = mot.second.name;
        break;
      }
    }

    if ( name.empty() ) {
      logwrite( function, "ERROR: no motor controllers defined" );
      return( ERROR );
    }

    // send the move command
    //
//  error = this->pi_interface.move_abs( name, addr, axis, pos );

    if ( error != NO_ERROR ) {
      message.str(""); message << "ERROR moving " << name << " to " << pos;
      logwrite( function, message.str() );
      return error;
    }

    // Wait for motor to be on target
    //
    message.str(""); message << "waiting for " << name;
    logwrite( function, message.str() );
//  error = this->pi_interface.move_axis_wait( name, addr, axis );

    return( error );
*****/
  }
  /***** Focus::Interface::move_abs *******************************************/


  /***** Focus::Interface::stop ***********************************************/
  /**
   * @brief      send the stop-all-motion command to all controllers
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::stop( ) {
    long ret = NO_ERROR;

    for (auto &mot : this->motors) ret |= mot.second.stop();

    return ret;
  }
  /***** Focus::Interface::stop ***********************************************/


  /***** Focus::Interface::send_command ***************************************/
  /**
   * @brief      writes the raw command as received to the master controller
   * @param[in]  name  string contains the controller name
   * @param[in]  cmd   command to send
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command that expects no reply.
   *
   */
  long Interface::send_command( const std::string &name, std::string cmd ) {
    if (this->motors.find(name)==this->motors.end()) {
      logwrite("Focus::Interface::send_command", "ERROR '"+name+"' not configured");
      return ERROR;
    }
    return this->motors.at(name).send_command(cmd);
  }
  /***** Focus::Interface::send_command ***************************************/


  /***** Focus::Interface::send_command ***************************************/
  /**
   * @brief      writes the raw command to the master controller, reads back reply
   * @param[in]  name       string contains the controller name
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply received
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command and then reads a reply if that command contains
   * a question mark, "?".
   *
   */
  long Interface::send_command( const std::string &name, std::string cmd, std::string &retstring ) {
    if (this->motors.find(name)==this->motors.end()) {
      logwrite("Focus::Interface::send_command", "ERROR '"+name+"' not configured");
      return ERROR;
    }
    if ( cmd.find( "?" ) != std::string::npos ) {
      return this->motors.at(name).send_command(cmd, &retstring);
    }
    else {
      return this->motors.at(name).send_command(cmd);
    }
  }
  /***** Focus::Interface::send_command ***************************************/


  /***** Focus::Interface::make_telemetry_message *****************************/
  /**
   * @brief      assembles a telemetry message
   * @details    This creates a JSON message for telemetry info, then serializes
   *             it into a std::string ready to be sent over a socket.
   * @param[out] retstring  optional string ref containing serialization of JSON message
   *
   */
  void Interface::make_telemetry_message(std::string* retstring) {
    const std::string function="Focus::Interface::make_telemetry_message";

    // assemble the telemetry into a json message
    // Set a messagetype keyword to indicate what kind of message this is.
    //
    nlohmann::json jmessage_out;
    jmessage_out["messagetype"]="focusinfo";

    // get focus position for each motor
    //
    for (auto &mot : this->motors) {
      int axis = 1;
      float position = NAN;
      std::string posname;
      mot.second.get_pos(axis, position, posname);

      std::string key = "FOCUS" + mot.first;

      // assign the position or NaN to a key in the JSON jmessage
      //
      if ( !std::isnan(position) ) jmessage_out[key]=position; else jmessage_out[key]="NAN";
    }

    this->publisher->publish(jmessage_out);

    if (retstring) {
      *retstring = jmessage_out.dump();  // serialize the json message into retstring
      retstring->append(JEOF);           // append the JSON message terminator
    }
  }
  /***** Focus::Interface::make_telemetry_message *****************************/


  /***** Focus::Interface::handletopic_snapshot *******************************/
  /**
   * @brief      publishes snapshot of my telemetry
   * @details    This publishes a JSON message containing a snapshot of my
   *             telemetry info when the subscriber receives the "_snapshot"
   *             topic and the payload contains my daemon name.
   * @param[in]  jmessage  received JSON message
   *
   */
  void Interface::handletopic_snapshot( const nlohmann::json &jmessage ) {
    // if my name is in the jmessage then publish my snapshot
    if ( jmessage.contains( Focus::DAEMON_NAME ) ) {
      this->make_telemetry_message();
    }
    else
    if ( jmessage.contains( "test" ) ) {
      logwrite( "Focusd::Interface::handletopic_snapshot", jmessage.dump() );
    }
  }
  /***** Focus::Interface::handletopic_snapshot *******************************/


  /***** Focus::Interface::test ***********************************************/
  /**
   * @brief      test commands
   * @param[in]  args
   * @param[out] retstring  reference to any reply
   * @return     ERROR | NO_ERROR | HELP
   *
   * This is the place to put various debugging and system testing tools.
   *
   * The server command is "test", the next parameter is the test name,
   * and any parameters needed for the particular test are extracted as
   * tokens from the args string passed in.
   *
   * The input args string is tokenized and tests are separated by a simple
   * series of if..else.. conditionals.
   *
   * Valid test names are:
   *   motormap
   *   posmap
   *
   */
  long Interface::test( std::string args, std::string &retstring ) {
    std::string function = "Focus::Interface::test";
    std::stringstream message;
    std::vector<std::string> tokens;
    long error = NO_ERROR;

    Tokenize( args, tokens, " " );

    if ( tokens.size() < 1 ) {
      logwrite( function, "no test name provided" );
      return ERROR;
    }

    std::string testname = tokens[0];                                // the first token is the test name

    if ( testname == "?" || testname == "help" ) {
      retstring.clear();
      retstring.append( "  motormap  return definition of motormap\n" );
      retstring.append( "  posmap    return definition of posmap\n" );
      return HELP;
    }
    else

    // motormap
    //
    if ( testname == "motormap" ) {
      retstring="name host:port addr naxes axisnum min max reftype";
      logwrite( function, retstring );
      for (auto &mot : this->motors) {
        retstring.append("\n");
        message.str(""); message << mot.first << " "
                                 << mot.second.get_host() << ":"
                                 << mot.second.get_port() << " "
                                 << mot.second.get_addr() << " "
                                 << mot.second.get_naxes();
        /*** TODO 2026 FIX THIS LOOP
        for ( const auto &axis : mot.second.axes ) {
          message << " " << axis.second.axisnum << " " << axis.second.min << " " << axis.second.max << " " << axis.second.reftype;
        }
        ***/
        retstring.append( message.str() );
        logwrite( function, message.str() );
      }
      retstring.append("\n");
    }
    else

    // posmap
    //
    if ( testname == "posmap" ) {
      retstring="motorname axis posid pos posname";
      logwrite( function, retstring );
      for (auto &mot : this->motors) {
        retstring.append("\n");
        message.str(""); message << mot.first << " ";
        /*** TODO 2026 FIX THIS LOOP
        for ( const auto &pos : mot.second.posmap ) {
          message << " " << pos.second.axis << " " << pos.second.posid << " " << pos.second.position << " " << pos.second.posname;
        }
        ***/
        retstring.append( message.str() );
        logwrite( function, message.str() );
      }
      retstring.append("\n");
    }

    else {
      message.str(""); message << "ERROR: test " << testname << " unknown";;
      logwrite(function, message.str());
      error = ERROR;
    }

    return( error );
  }
  /***** Focus::Interface::test ***********************************************/

}
