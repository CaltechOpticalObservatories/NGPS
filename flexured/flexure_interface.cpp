/**
 * @file    flexure_interface.cpp
 * @brief   this contains the flexure interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Flexure namespace,
 * and is how the flexure daemon interfaces to the flexure hardware.
 *
 */

#include "flexure_interface.h"

namespace Flexure {

  /***** Flexure::Interface::open *********************************************/
  /**
   * @brief      opens the PI socket connection
   * @details    the motor interface does all the work
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    long error = NO_ERROR;

    // Open connection to all motors
    for (auto &mot : this->motors) error |= mot.second.open();

    // clear any error codes on startup
    this->pi_interface->clear_errors();

    // enable motion and servo mode
    for (auto &mot : this->motors) error |= mot.second.enable_motion();

    return error;
  }
  /***** Flexure::Interface::open *********************************************/


  /***** Flexure::Interface::close ********************************************/
  /**
   * @brief      closes the PI socket connection
   * @details    the motor interface does all the work
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    long error=NO_ERROR;
    for (auto &mot : this->motors) error |= mot.second.close();
    return error;
  }
  /***** Flexure::Interface::close ********************************************/


  /***** Flexure::Interface::is_open ******************************************/
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
    const std::string function("Flexure::Interface::is_open");
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = FLEXURED_ISOPEN;
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
  /***** Flexure::Interface::is_open ******************************************/


  /***** Flexure::Interface::set **********************************************/
  /**
   * @brief      set the position of the indicated channel and axis
   * @param[in]  args       string containing <name> <axis> <pos>
   * @param[out] retstring  reference to return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::set( std::string args, std::string &retstring ) {
    const std::string function("Flexure::Interface::set");
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
      retstring = FLEXURED_SET;
      retstring.append( " <chan> <axis> <pos>\n" );
      retstring.append( "  Set position of indicated <chan> and <axis> to <pos>,\n" );
      retstring.append( "  where <chan> <axis> <min> <max> are as follows:\n" );
      try {
        for (auto &mot : this->motors) {
          const auto &motor = mot.second;
          for (int axisnum : motor.axes()) {
            auto ax = motor.axis(axisnum);
            if (!ax) continue;
            retstring.append( "     " );
            retstring.append( mot.first ); retstring.append( " " );
            message.str(""); message << ax->axisnum << " ";
            retstring.append( message.str() );
            message.str(""); message << std::fixed << std::setprecision(3) << ax->min << " " << ax->max;
            retstring.append( message.str() );
            retstring.append( "\n" );
          }
        }
      }
      catch (const std::exception &e) {
        logwrite(function, "ERROR: "+std::string(e.what()));
        return ERROR;
      }
      return HELP;
    }

    make_uppercase(args);
    std::istringstream iss(args);
    std::string chan;
    int axis;
    float pos;

    if (!(iss >> chan >> axis >> pos)) {
      logwrite(function, "ERROR expected <chan> <axis> <pos>");
      return ERROR;
    }

    long error = this->motors.at(chan).moveto(axis, pos, retstring);

    message << (error==NO_ERROR ? "success" : "failed")
            << " moving " << chan << " axis " << axis << " to " << pos;
    logwrite(function, message.str());

    return error;
  }
  /***** Flexure::Interface::set **********************************************/


  /***** Flexure::Interface::get **********************************************/
  /**
   * @brief      get the position of the indicated channel and axis
   * @param[in]  args       string containing <name> <axis>
   * @param[out] retstring  reference to return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::get( std::string args, std::string &retstring ) {
    const std::string function("Flexure::Interface::get");
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
      retstring = FLEXURED_GET;
      retstring.append( " <chan> <axis>\n" );
      retstring.append( "  Get position of indicated <chan> and <axis>,\n" );
      retstring.append( "  where <chan> <axis> are as follows:\n" );
      for (auto &mot : this->motors) {
        const auto &motor = mot.second;
        for (int axisnum : motor.axes()) {
          message.str(""); message << "     " << mot.first << " " << axisnum << "\n";
          retstring.append( message.str() );
        }
      }
      return HELP;
    }

    std::istringstream iss(args);
    std::string chan;
    int axis;

    if (!(iss >> chan >> axis)) {
      logwrite( function, "ERROR parsing arguments. expected <chan> <axis>" );
      retstring="invalid_argument";
      return ERROR;
    }

    // get the position
    //
    std::string posstring;
    auto addr=this->motors.at(chan).get_addr();
    float position=NAN;
    std::string posname;
    long error = this->motors.at(chan).get_pos(axis, addr, position, posname);

    // form the return value
    //
    message.str(""); message << chan << " " << axis << " " << std::fixed << std::setprecision(6) << position;
    if ( ! posname.empty() ) { message << " (" << posname << ")"; }
    retstring = message.str();
    logwrite( function, message.str() );

    message.str(""); message << "NOTICE:" << Flexure::DAEMON_NAME << " " << retstring;
    this->async.enqueue( message.str() );

    return error;
  }
  /***** Flexure::Interface::get **********************************************/


  /***** Flexure::Interface::compensate ***************************************/
  /**
   * @brief      performs the flexure compensation
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::compensate( std::string args, std::string &retstring ) {
    const std::string function="Flexure::Interface::compensate";
    std::stringstream message;

    if ( args == "?" || args == "help" ) {
      retstring = FLEXURED_COMPENSATE;
      retstring.append( " [ dryrun ]\n" );
      retstring.append( "   Performs the flexure compensation. If the optional dryrun argument\n" );
      retstring.append( "   is supplied then perform the calcuations and show the actions that\n" );
      retstring.append( "   would be taken without actually moving anything.\n" );
      return HELP;
    }

    // get the needed telemetry (telescope position and temperatures)
    //
    this->get_external_telemetry();

    // perform the calculations
    //
    retstring="not_yet_implemented";

    return ERROR;
  }
  /***** Flexure::Interface::compensate ***************************************/


  /***** Flexure::Interface::stop *********************************************/
  /**
   * @brief      send the stop-all-motion command to all controllers
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::stop( ) {
    std::string function = "Flexure::Interface::stop";
    std::stringstream message;
    return( ERROR );
  }
  /***** Flexure::Interface::stop *********************************************/


  /***** Flexure::Interface::send_command *************************************/
  /**
   * @brief      writes the raw command as received to the master controller
   * @param[in]  name  controller name
   * @param[in]  cmd   command to send
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command that expects no reply.
   *
   */
  long Interface::send_command( const std::string &name, std::string cmd ) {
    if (this->motors.find(name)==this->motors.end()) {
      logwrite("Flexure::Interface::send_command", "ERROR '"+name+"' not configured");
      return ERROR;
    }
    return this->motors.at(name).send_command(cmd);
  }
  /***** Flexure::Interface::send_command *************************************/


  /***** Flexure::Interface::send_command *************************************/
  /**
   * @brief      writes the raw command to the master controller, reads back reply
   * @param[in]  name       controller name
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
      logwrite("Flexure::Interface::send_command", "ERROR '"+name+"' not configured");
      return ERROR;
    }
    if ( cmd.find( "?" ) != std::string::npos ) {
      return this->motors.at(name).send_command(cmd, &retstring);
    }
    else {
      return this->motors.at(name).send_command(cmd);
    }
  }
  /***** Flexure::Interface::send_command *************************************/


  /***** Flexure::Interface::make_telemetry_message ***************************/
  /**
   * @brief      assembles a telemetry message
   * @details    This creates a JSON message for telemetry info, then serializes
   *             it into a std::string ready to be sent over a socket.
   * @param[out] retstring  string containing the serialization of the JSON message
   *
   */
  void Interface::make_telemetry_message( std::string &retstring ) {
    const std::string function="Flexure::Interface::make_telemetry_message";
    std::stringstream message;

    // assemble the telemetry into a json message
    // Set a messagetype keyword to indicate what kind of message this is.
    //
    nlohmann::json jmessage;
    jmessage["messagetype"]="flexureinfo";

    // get all flexure actuator positions
    //
    for (auto &mot : this->motors) {
      const auto &motor = mot.second;
      // loop through all axes for each motor
      for (int axisnum : motor.axes()) {
        auto ax = motor.axis(axisnum);
        if (!ax) continue;
        auto chan = mot.first;
        auto addr = mot.second.get_addr();
        float position = NAN;
        std::string posname;
        std::string key;
        mot.second.get_pos(axisnum, addr, position, posname);
        switch ( axisnum ) {
          case 1 : key = "FLXPIS_" + chan; break;
          case 2:  key = "FLXSPE_" + chan; break;
          case 3:  key = "FLXSPA_" + chan; break;
          default: key = "error";
                   message.str(""); message << "ERROR unknown axis " << axisnum;
                   logwrite( function, message.str() );
        }

        // assign the position or NaN to a key in the JSON jmessage
        //
        if ( !std::isnan(position) ) jmessage[key]=position; else jmessage[key]="NAN";
      }
    }

    retstring = jmessage.dump();  // serialize the json message into retstring

    retstring.append(JEOF);       // append the JSON message terminator

    return;
  }
  /***** Flexure::Interface::make_telemetry_message ***************************/


  /***** Flexure::Interface::get_external_telemetry ***************************/
  /**
   * @brief      collect telemetry from another daemon
   * @details    This is used for any telemetry that I need to collect from
   *             another daemon. Send the command "sendtelem" to the daemon, which
   *             will respond with a JSON message. The daemon(s) to contact
   *             are configured with the TELEM_PROVIDER key in the config file.
   *
   */
  void Interface::get_external_telemetry() {

    // Loop through each configured telemetry provider. This requests
    // their telemetry which is returned as a serialized json string
    // held in retstring.
    //
    // handle_json_message() will parse the serialized json string.
    //
    std::string retstring;
    for ( const auto &provider : this->telemetry_providers ) {
      Common::collect_telemetry( provider, retstring );
      handle_json_message(retstring);
    }
    return;
  }
  /***** Flexure::Interface::get_external_telemetry ***************************/


  /***** Flexure::Interface::handle_json_message ******************************/
  /**
   * @brief      parses incoming telemetry messages
   * @details    Requesting telemetry from another daemon returns a serialized
   *             JSON message which needs to be passed in here to parse it.
   * @param[in]  message_in  incoming serialized JSON message (as a string)
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::handle_json_message( std::string message_in ) {
    const std::string function="Flexure::Interface::handle_json_message";
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
      if ( messagetype == "thermalinfo" ) {
        double TCOLL_I=NAN, TCOLL_R=NAN, TCOLL_G=NAN, TCOLL_U=NAN;
        Common::extract_telemetry_value( message_in, "TCOLL_I", TCOLL_I );
        Common::extract_telemetry_value( message_in, "TCOLL_R", TCOLL_R );
        Common::extract_telemetry_value( message_in, "TCOLL_G", TCOLL_G );
        Common::extract_telemetry_value( message_in, "TCOLL_U", TCOLL_U );
        message.str(""); message << "TCOLL_I=" << TCOLL_I << " TCOLL_R=" << TCOLL_R << " TCOLL_G=" << TCOLL_G << " TCOLL_U=" << TCOLL_U;
        logwrite( function, message.str() );
      }
      else
      if ( messagetype == "tcsinfo" ) {
        double casangle=NAN, alt=NAN;
        Common::extract_telemetry_value( message_in, "CASANGLE", casangle );
        Common::extract_telemetry_value( message_in, "ALT", alt );
        message.str(""); message << "casangle=" << casangle << " alt=" << alt;
        logwrite( function, message.str() );
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
  /***** Flexure::Interface::handle_json_message ******************************/


  /***** Flexure::Interface::test *********************************************/
  /**
   * @brief      test commands
   * @param[in]  args
   * @param[out] retstring  reference to any reply
   * @return     ERROR or NO_ERROR
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
    std::string function = "Flexure::Interface::test";
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
      retstring="name host:port addr naxes \n      axisnum min max reftype defpos";
      for (auto &mot : this->motors) {
        retstring.append("\n");
        message.str(""); message << mot.first << " "
                                 << mot.second.get_host() << ":"
                                 << mot.second.get_port() << " "
                                 << mot.second.get_addr() << " "
                                 << mot.second.get_naxes();
        const auto &motor = mot.second;
        for (int axisnum : motor.axes()) {
          auto ax = motor.axis(axisnum);
          if (!ax) continue;
          message << "\n      " << axisnum << " " << ax->min << " " << ax->max << " "
                  << ax->reftype << " " << ax->defpos;
        }
        retstring.append( message.str() );
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
  /***** Flexure::Interface::test *********************************************/

}
