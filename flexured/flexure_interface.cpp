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

  /***** Flexure::Interface::initialize_class *********************************/
  /**
   * @brief      initializes the class from configure_flexured()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Flexure::Server::configure_flexured() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Flexure::Interface::initialize_class";
    std::stringstream message;

    auto _motormap = this->motorinterface.get_motormap();

    this->numdev = _motormap.size();

    // I don't want to prevent the system from working with a subset of controllers,
    // but the user should be warned, in case it wasn't intentional.
    //
    if ( this->numdev != 2 ) {
      message.str(""); message << "WARNING: " << this->numdev << " PI motor controller"
                               << ( this->numdev == 1 ? "" : "s" ) << " defined! (expected 4)";
      logwrite( function, message.str() );
    }

    return( NO_ERROR );
  }
  /***** Flexure::Interface::initialize_class *********************************/


  /***** Flexure::Interface::open *********************************************/
  /**
   * @brief      opens the PI socket connection
   * @details    the motor interface does all the work
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    long error = NO_ERROR;

    // Open the sockets,
    // clear any error codes on startup, and
    // enable servo mode.
    //
    error |= this->motorinterface.open();
    error |= this->motorinterface.clear_errors();
    error |= this->motorinterface.set_servo( true );
    error |= this->motorinterface.move_to_default();

    return( error );
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
    return this->motorinterface.close();
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
    std::string function = "Flexure::Interface::is_open";
    std::stringstream message;
    long error = NO_ERROR;

    auto _motormap = this->motorinterface.get_motormap();

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

    return( error );
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
    std::string function = "Flexure::Interface::set";
    std::stringstream message;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( args == "?" ) {
      retstring = FLEXURED_SET;
      retstring.append( " <chan> <axis> <pos>\n" );
      retstring.append( "  Set position of indicated <chan> and <axis> to <pos>,\n" );
      retstring.append( "  where <chan> <axis> <min> <max> are as follows:\n" );
      for ( const auto &mot : _motormap ) {
        for ( const auto &axis : mot.second.axes ) {
          retstring.append( "     " );
          retstring.append( mot.first ); retstring.append( " " );
          message.str(""); message << axis.first << " ";
          retstring.append( message.str() );
          message.str(""); message << std::fixed << std::setprecision(3) << axis.second.min << " " << axis.second.max;
          retstring.append( message.str() );
          retstring.append( "\n" );
        }
      }
      return HELP;
    }

    // Tokenize the input arg list,
    // expecting <chan> <axis> <pos>
    //
    std::transform( args.begin(), args.end(), args.begin(), ::toupper );
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 3 ) {
      logwrite( function, "ERROR invalid arguments. expected <chan> <axis> <pos>" );
      retstring="invalid_argument";
      return( ERROR );
    }

    std::string chan;
    int axis;
    float pos;

    try {
      chan = tokens[0];
      axis = std::stoi( tokens[1] );
      pos  = std::stof( tokens[2] );
    }
    catch ( const std::invalid_argument &e ) {
      message.str(""); message << "ERROR parsing args \"" << args << "\" : " << e.what();
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return( ERROR );
    }
    catch ( const std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing args \"" << args << "\" : " << e.what();
      logwrite( function, message.str() );
      retstring="out_of_range";
      return( ERROR );
    }

    return this->motorinterface.moveto( chan, axis, pos, retstring );
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
    std::string function = "Flexure::Interface::get";
    std::stringstream message;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( args == "?" ) {
      retstring = FLEXURED_GET;
      retstring.append( " <chan> <axis>\n" );
      retstring.append( "  Get position of indicated <chan> and <axis>,\n" );
      retstring.append( "  where <chan> <axis> are as follows:\n" );
      for ( const auto &mot : _motormap ) {
        for ( const auto &axis : mot.second.axes ) {
          message.str(""); message << "     " << mot.first << " " << axis.first << "\n";
          retstring.append( message.str() );
        }
      }
      return HELP;
    }

    // Tokenize the input arg list,
    // expecting <chan> <axis>
    //
    std::transform( args.begin(), args.end(), args.begin(), ::toupper );
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      logwrite( function, "ERROR invalid arguments. expected <chan> <axis>" );
      retstring="invalid_argument";
      return( ERROR );
    }

    std::string chan;
    int axis;

    try {
      chan = tokens[0];
      axis = std::stoi( tokens[1] );
    }
    catch ( const std::invalid_argument &e ) {
      message.str(""); message << "ERROR parsing args \"" << args << "\" : " << e.what();
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return( ERROR );
    }
    catch ( const std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing args \"" << args << "\" : " << e.what();
      logwrite( function, message.str() );
      retstring="out_of_range";
      return( ERROR );
    }

    // get the position
    //
    std::string posstring;
    auto addr=this->motorinterface.get_motormap()[chan].addr;
    float position=NAN;
    std::string posname;
    long error = this->motorinterface.get_pos( chan, axis, addr, position, posname );

    // form the return value
    //
    message.str(""); message << chan << " " << axis << " " << std::fixed << std::setprecision(6) << position;
    if ( ! posname.empty() ) { message << " (" << posname << ")"; }
    retstring = message.str();
    logwrite( function, message.str() );

    message.str(""); message << "NOTICE:" << Flexure::DAEMON_NAME << " " << retstring;
    this->async.enqueue( message.str() );

    return( error );
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
    return this->motorinterface.send_command( name, cmd );
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
    if ( cmd.find( "?" ) != std::string::npos ) {
      return( this->motorinterface.send_command( name, cmd, retstring ) );
    }
    else {
      return( this->motorinterface.send_command( name, cmd ) );
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
    auto _motormap = this->motorinterface.get_motormap();

    // loop through all motors in motormap
    for ( const auto &mot : _motormap ) {
      // loop through all axes for each motor
      for ( const auto &axis : mot.second.axes ) {
        auto chan = mot.second.name;
        auto addr = mot.second.addr;
        float position = NAN;
        std::string posname;
        std::string key;
        this->motorinterface.get_pos( chan, axis.second.axisnum, addr, position, posname );
        switch ( axis.second.axisnum ) {
          case 1 : key = "FLXPIS_" + chan; break;
          case 2:  key = "FLXSPE_" + chan; break;
          case 3:  key = "FLXSPA_" + chan; break;
          default: key = "error";
                   message.str(""); message << "ERROR unknown axis " << axis.second.axisnum;
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
    // parse_incoming_telemetry() will parse the serialized json string.
    //
    std::string retstring;
    for ( const auto &provider : this->telemetry_providers ) {
      Common::collect_telemetry( provider, retstring );
      parse_incoming_telemetry(retstring);
    }
    return;
  }
  /***** Flexure::Interface::get_external_telemetry ***************************/


  /***** Flexure::Interface::parse_incoming_telemetry *************************/
  /**
   * @brief      parses incoming telemetry messages
   * @details    Requesting telemetry from another daemon returns a serialized
   *             JSON message which needs to be passed in here to parse it.
   * @param[in]  message_in  incoming serialized JSON message (as a string)
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::parse_incoming_telemetry( std::string message_in ) {
    const std::string function="Flexure::Interface::parse_incoming_telemetry";
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
        double casangle=NAN, alt=NAN, pa=NAN;
        Common::extract_telemetry_value( message_in, "CASANGLE", casangle );
        Common::extract_telemetry_value( message_in, "ALT", alt );
        Common::extract_telemetry_value( message_in, "PA", pa );
        message.str(""); message << "casangle=" << casangle << " alt=" << alt << " PA=" << pa;
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
  /***** Flexure::Interface::parse_incoming_telemetry *************************/


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

    auto _motormap = this->motorinterface.get_motormap();

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
      for ( const auto &mot : _motormap ) {
        retstring.append("\n");
        message.str(""); message << mot.first << " "
                                 << mot.second.host << ":"
                                 << mot.second.port << " "
                                 << mot.second.addr << " "
                                 << mot.second.naxes;
        for ( const auto &axis : mot.second.axes ) {
          message << "\n      " << axis.second.axisnum << " " << axis.second.min << " " << axis.second.max << " "
                  << axis.second.reftype << " " << axis.second.defpos;
        }
        retstring.append( message.str() );
      }
      retstring.append("\n");
    }
    else

    if (testname == "calcshift") {
      if (tokens.size() != 3) {
        logwrite(function, "ERROR expected <chan> <axis>");
        return ERROR;
      }
      double shift;
      this->compensator.test({tokens[1], tokens[2]}, shift);
      message.str(""); message << shift;
      retstring = message.str();
      return NO_ERROR;
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
