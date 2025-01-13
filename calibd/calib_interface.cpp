/**
 * @file    calib_interface.cpp
 * @brief   defines the Calib namespace Interface class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "calib_interface.h"

namespace Calib {


  /***** Calib::Motion::Motion ************************************************/
  /**
   * @brief      Motion class constructor
   *
   */
  Motion::Motion() {
    this->port=-1;
    this->numdev=0;
  }
  /***** Calib::Motion::Motion ************************************************/


  /***** Calib::Motion::configure_class ***************************************/
  /**
   * @brief      configures the class from configure_calibd()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Calib::Server::configure_calibd() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Motion::configure_class() {
    std::string function = "Calib::Motion::configure_class";
    std::stringstream message;
    long error = ERROR;

    auto _motormap = this->motorinterface.get_motormap();

    this->numdev = _motormap.size();

    if ( this->numdev == 2 ) {
      logwrite( function, "motion interface configured ok" );
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
    }
    else if ( this->numdev < 1 ) {
      message.str(""); message << "ERROR: no motor controllers: " << this->numdev << ". expected 2";
      logwrite( function, message.str() );
      error = ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    for ( const auto &mot : _motormap ) {
      message.str(""); message << "[DEBUG] motion controller " << mot.first
                               << " addr=" << mot.second.addr;
      for ( const auto &pos : mot.second.posmap ) {
        message << " " << pos.second.posname << "=" << pos.second.position;
      }
      logwrite( function, message.str() );
    }
#endif

    return( error );
  }
  /***** Calib::Motion::configure_class ***************************************/


  /***** Calib::Motion::open **************************************************/
  /**
   * @brief      opens the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Motion::open() {
    long error = NO_ERROR;

    // open the sockets,
    // clear any error codes,
    // enable servos.
    //
    error |= this->motorinterface.open();
    error |= this->motorinterface.clear_errors();
    error |= this->motorinterface.set_servo( true );

    return( error );
  }
  /***** Calib::Motion::open **************************************************/


  /***** Calib::Motion::close *************************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Motion::close( ) {
    return this->motorinterface.close();
  }
  /***** Calib::Motion::close *************************************************/


  /***** Calib::Motion::is_open ***********************************************/
  /**
   * @brief       return the connected state of the motor controllers
   * @param[in]   arg        used only for help
   * @param[out]  retstring  contains the connected state "true" | "false"
   * @return      NO_ERROR | HELP
   *
   * All motors must be connected for this to return "true".
   *
   */
  long Motion::is_open( std::string arg, std::string &retstring ) {
    std::string function = "Calib::Motion::is_open";
    std::stringstream message;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( arg == "?" ) {
      retstring = CALIBD_ISOPEN;
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

    return NO_ERROR;
  }
  /***** Calib::Motion::is_open ***********************************************/


  /***** Calib::Motion::home **************************************************/
  /**
   * @brief      home all calib actuators
   * @param[in]  name_in    optional list of motors to home
   * @param[out] retstring  reference to return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Motion::home( std::string name_in, std::string &retstring ) {

    // Help
    //
    if ( name_in == "?" || name_in == "help" ) {
      retstring = CALIBD_HOME;
      retstring.append( " [ " );
      auto _motormap = this->motorinterface.get_motormap();
      for ( const auto &mot : _motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "]\n" );
      retstring.append( "  Home all calib motors or single motor indicated by optional <name>.\n" );
      retstring.append( "  If no argument is supplied then all are homed simultaneously.\n" );
      return HELP;
    }

    // All the work is done by the PI motor interface class
    //
    return this->motorinterface.home( name_in, retstring );
  }
  /***** Calib::Motion::home **************************************************/


  /***** Calib::Motion::is_home ***********************************************/
  /**
   * @brief       return the home state of the motors
   * @param[in]   name_in    optionally contains one or more motors to check
   * @param[out]  retstring  contains the home state "true" | "false"
   * @return      ERROR | NO_ERROR | HELP
   *
   * All motors must be homed for this to return "true".
   *
   */
  long Motion::is_home( std::string name_in, std::string &retstring ) {

    // Help
    //
    if ( name_in == "?" ) {
      retstring = CALIBD_ISHOME;
      retstring.append( " [ " );
      auto _motormap = this->motorinterface.get_motormap();
      for ( const auto &mot : _motormap ) { retstring.append( mot.first ); retstring.append(" "); }
      retstring.append( "]\n" );
      retstring.append( "  Reads the referencing state from each of the indicated controllers,\n" );
      retstring.append( "  or all controllers if none supplied. Returns true if all (named) are\n" );
      retstring.append( "  homed, false if any one is not homed.\n" );
      return HELP;
    }

    // All the work is done by the PI motor interface class
    //
    return this->motorinterface.is_home( name_in, retstring );
  }
  /***** Calib::Motion::is_home ***********************************************/


  /***** Calib::Motion::set ***************************************************/
  /**
   * @brief      move and/or return status of an actuator
   * @details    Input list can be comma or space delimited, but there must be
   *             no space on either side of the equal sign. This function
   *             allows for all listed actuators to be set simultaneously,
   *             each in its own thread.
   * @param[in]  input      list of actuators with optional "=state" i.e. cover=open, door=close
   * @param[out] retstring  actuator state { OPEN | CLOSE }
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Motion::set( std::string input, std::string &retstring ) {
    std::string function = "Calib::Motion::set";
    std::stringstream message;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( input == "?" ) {
      retstring = CALIBD_SET;
      retstring.append( " <actuator>=<posname> [ <actuator>=<posname> ]\n" );
      retstring.append( "  where <posname> is\n" );
      for ( const auto &mot : _motormap ) {
        retstring.append( "                     { " );
        for ( const auto &pos : mot.second.posmap ) {
          retstring.append( pos.second.posname ); retstring.append( " " );
        }
        retstring.append( "} for <actuator> = " );
        retstring.append( mot.first );
        retstring.append( "\n" );
      }
      retstring.append( "  One or both actuators may be set simultaneously.\n" );
      retstring.append( "  There are no space between <actuator>=<posname>, e.g. set door=open\n" );
      return HELP;
    }

    std::stringstream namelist;                      // list of actuator names used for getting position after move

    std::vector<std::string> input_list;             // vector of each "actuator=state"
    std::vector<std::string> motornames;             // vector of all "actuator" (motor names)
    std::vector<std::string> posnames;               // vector of all "state" (position names)
    std::vector<int>         axisnums;               // vector of all axis nums, all 1 in this case

    // Tokenize on comma to see if more than one actuator
    //
    Tokenize( input, input_list, " ," );

    // Iterate through each requested actuator, and
    // build the motornames, posnames vectors to pass to moveto().
    //
    for ( const auto &actuator : input_list ) {

      // Tokenize each item in above vector on "=" to get
      // the actuator name and the desired posname.
      //
      std::vector<std::string> actuator_tokens;      // vector of individual actuator, posname for each actuator entry
      Tokenize( actuator, actuator_tokens, "=" );

      if ( actuator_tokens.size() != 2 ) {
        message.str(""); message << "ERROR: bad input \"" << actuator << "\". expected actuator=state";
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return( ERROR );
      }
      else {
        namelist << actuator_tokens[0] << " ";
        motornames.push_back( actuator_tokens[0] );
        posnames.push_back( actuator_tokens[1] );
        axisnums.push_back( 1 );  // these all have axis=1
      }
    }

    long error = this->motorinterface.moveto( motornames, axisnums, posnames, retstring );

    // read and return the position(s) after successful move
    //
    if ( error == NO_ERROR ) error = this->get( namelist.str(), retstring );

    return( error );
  }
  /***** Calib::Motion::set ***************************************************/


  /***** Calib::Motion::get ***************************************************/
  /**
   * @brief      get the position of the named actuator(s)
   * @param[in]  name_in    name of actuator(s), can be space-delimited list
   * @param[out] retstring  current position of actuator(s) { open | closed }
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Motion::get( std::string name_in, std::string &retstring ) {
    std::string function = "Slit::Motion::get";
    std::stringstream message;
    std::stringstream retstream;
    long error = NO_ERROR;
    std::vector<std::string> name_list;

    auto _motormap = this->motorinterface.get_motormap();  // get a copy of the motormap

    // Help
    //
    if ( name_in == "?" ) {
      retstring = CALIBD_GET;
      retstring.append( " all | [ <actuator> ]\n" );
      retstring.append( "  where <actuator> is { " );
      for ( const auto &mot : _motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "}\n" );
      retstring.append( "  If all is supplied then the position of both is returned.\n" );
      retstring.append( "  Supplying an actuator name returns the position of only the specified actuator.\n" );
      return HELP;
    }

    // If no name(s) supplied then create a vector of all defined actuator names
    //
    if ( name_in == "all" ) {
      for ( const auto &mot : _motormap ) {
        name_list.push_back( mot.first );
      }
    }
    else {
      Tokenize( name_in, name_list, " " );  // otherwise create a vector of the supplied name(s)
    }

    for ( const auto &name : name_list ) {
      std::string posname;

      if ( _motormap.find( name ) == _motormap.end() ) {
        message.str(""); message << "ERROR: actuator \"" << name << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        posname = "error";
        error = ERROR;
      }
      else {
        // and then get the current position of this actuator.
        //
        int addr = _motormap[ name ].addr;
        int axis = 1;
        float position = NAN;

        error = this->motorinterface.get_pos( name, axis, addr, position, posname );

        if ( error == ERROR ) {
          message.str(""); message << "ERROR reading position of actuator \"" << name << "\"";
          logwrite( function, message.str() );
        }
      }

      // If more than one requested then include the name with each posname
      //
      if ( name_list.size() > 1 ) {
        retstream << name << "=" << posname << " ";
      }
      // otherwise just the posname
      //
      else {
        retstream << posname;
      }
    }

    retstring = retstream.str();

    return error;
  }
  /***** Calib::Motion::get ***************************************************/


  /***** Calib::Motion::send_command ******************************************/
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
  long Motion::send_command( const std::string &name, std::string cmd ) {
    return( this->motorinterface.send_command( name, cmd ) );
  }
  /***** Calib::Motion::send_command ******************************************/


  /***** Calib::Motion::send_command ******************************************/
  /**
   * @brief      writes the raw command to the master controller, reads back reply
   * @param[in]  cmd        command to send
   * @param[out] retstring  reply received
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command and then reads a reply if that command contains
   * a question mark, "?".
   *
   */
  long Motion::send_command( const std::string &name, std::string cmd, std::string &retstring ) {
    if ( cmd.find( "?" ) != std::string::npos ) {
      return( this->motorinterface.send_command( name, cmd, retstring ) );
    }
    else {
      return( this->motorinterface.send_command( name, cmd ) );
    }
  }
  /***** Calib::Motion::send_command ******************************************/


  /***** Calib::Interface::open ***********************************************/
  /**
   * @brief      open all calib-related connections
   * @param[in]  args       string optionally containing { ? motion lampmod }
   * @param[out] retstring
   * return      ERROR|NO_ERROR|HELP
   *
   */
  long Interface::open( std::string args, std::string &retstring ) {
    const std::string function="Calib::Interface::open";
    std::string which;

    // No args opens everything (motion and camera)...
    //
    if ( args.empty() ) {
      which = "all";
    }
    else if ( args == "?" || args=="-h" || args=="help" ) {
      retstring = CALIBD_OPEN;
      retstring.append( " [ [motion] [lampmod] ]\n" );
      retstring.append( "  Open connections to all calib devices (by default).\n" );
      retstring.append( "  Optionally indicate motion | lampmod to open only the indicated component.\n" );
      return HELP;
    }
    else { // ...otherwise look at the arg(s):
      std::transform( args.begin(), args.end(), args.begin(), ::tolower );  // convert to lowercase

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // args can be [ [motion] [lampmod] ]
      //
      int ccount=0;
      for ( size_t i=0; i < tokens.size(); i++ ) {
        size_t next = i+1;
        if ( tokens[i] == "motion" ) {
          which = tokens[i]; ccount++;
        }
        if ( tokens[i] == "lampmod" ) {
          which = tokens[i]; ccount++;
        }
      }

      if ( ccount == 2 ) which = "all";

      if ( which.empty() ) {
        logwrite( function, "ERROR: unrecognized arg \""+args+"\". Expected { [ [motion] [lampmod] ] }" );
        retstring="invalid_argument";
        return ERROR;
      }
    }

    // open motion, home if necessary
    //
    if ( which == "all" || which == "motion" ) {
      // open motion
      long error  = this->motion.open();
      // are actuators homed?
      error |= this->motion.is_home("", retstring);
      // home them if needed
      if ( retstring == "false" ) error |= this->motion.home("", retstring);
      if (error!=NO_ERROR) {
        logwrite( function, "ERROR initializing motion component" );
        return ERROR;
      }
    }

    // open lamp modulator
    //
    if ( which == "all" || which == "lampmod" ) {
      long error = this->modulator.control("open", retstring);
      if (error!=NO_ERROR) {
        logwrite( function, "ERROR initializing lampmod component" );
        return ERROR;
      }
    }

    retstring.clear();

    return NO_ERROR;
  }
  /***** Calib::Interface::open ***********************************************/


  /***** Calib::Interface::is_open ********************************************/
  /**
   * @brief      check open state of all calib-related connections
   * @param[in]  args       string optionally containing { ? motion lampmod }
   * @param[out] retstring
   * return      NO_ERROR|HELP
   *
   */
  long Interface::is_open( std::string args, std::string &retstring ) {
    const std::string function="Calib::Interface::is_open";
    std::string which;
    int ccount=0;  // component count (number of components requested)

    // No args checks everything (motion and camera)...
    //
    if ( args.empty() ) {
      which = "all";
      ccount=2;
    }
    else if ( args == "?" || args=="-h" || args=="help" ) {
      retstring = CALIBD_CLOSE;
      retstring.append( " [ [motion] [lampmod] ]\n" );
      retstring.append( "  Close connections to all calib devices (by default).\n" );
      retstring.append( "  Optionally indicate motion | lampmod to check only the indicated component.\n" );
      retstring.append( "  Returns true only if all requested components are open.\n" );
      return HELP;
    }
    else { // ...otherwise look at the arg(s):
      std::transform( args.begin(), args.end(), args.begin(), ::tolower );  // convert to lowercase

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // args can be [ [motion] [lampmod] ]
      //
      for ( size_t i=0; i < tokens.size(); i++ ) {
        size_t next = i+1;
        if ( tokens[i] == "motion" ) {
          which = tokens[i]; ccount++;
        }
        if ( tokens[i] == "lampmod" ) {
          which = tokens[i]; ccount++;
        }
      }

      if ( ccount == 2 ) which = "all";

      if ( which.empty() ) {
        logwrite( function, "ERROR: unrecognized arg \""+args+"\". Expected { [ [motion] [lampmod] ] }" );
        retstring="invalid_argument";
        return ERROR;
      }
    }

    int opencount=0;  // number of components open must match number requested

    // check motion
    //
    if ( which == "all" || which == "motion" ) {
      this->motion.is_open("", retstring);
      if ( retstring=="true" ) opencount++;
    }

    // check lamp modulator
    //
    if ( which == "all" || which == "lampmod" ) {
      this->modulator.control("isopen", retstring);
      if ( retstring=="true" ) opencount++;
    }

    // return true only if all requested components return true (open)
    //
    retstring = ( opencount==ccount ? "true" : "false" );

    return NO_ERROR;
  }
  /***** Calib::Interface::is_open ********************************************/


  /***** Calib::Interface::close **********************************************/
  /**
   * @brief      close all calib-related connections
   * @param[in]  args       string optionally containing { ? motion lampmod }
   * @param[out] retstring
   * return      ERROR|NO_ERROR|HELP
   *
   */
  long Interface::close( std::string args, std::string &retstring ) {
    const std::string function="Calib::Interface::close";
    std::string which;

    // No args closes everything (motion and camera)...
    //
    if ( args.empty() ) {
      which = "all";
    }
    else if ( args == "?" || args=="-h" || args=="help" ) {
      retstring = CALIBD_CLOSE;
      retstring.append( " [ [motion] [lampmod] ]\n" );
      retstring.append( "  Close connections to all calib devices (by default).\n" );
      retstring.append( "  Optionally indicate motion | lampmod to close only the indicated component.\n" );
      return HELP;
    }
    else { // ...otherwise look at the arg(s):
      std::transform( args.begin(), args.end(), args.begin(), ::tolower );  // convert to lowercase

      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // args can be [ [motion] [lampmod] ]
      //
      int ccount=0;
      for ( size_t i=0; i < tokens.size(); i++ ) {
        size_t next = i+1;
        if ( tokens[i] == "motion" ) {
          which = tokens[i]; ccount++;
        }
        if ( tokens[i] == "lampmod" ) {
          which = tokens[i]; ccount++;
        }
      }

      if ( ccount == 2 ) which = "all";

      if ( which.empty() ) {
        logwrite( function, "ERROR: unrecognized arg \""+args+"\". Expected { [ [motion] [lampmod] ] }" );
        retstring="invalid_argument";
        return ERROR;
      }
    }

    long error = NO_ERROR;

    // close motion
    //
    if ( which == "all" || which == "motion" ) {
      // close motion
      long ret = this->motion.close();
      if (ret!=NO_ERROR) {
        logwrite( function, "ERROR closing motion component" );
        error=ERROR;
      }
    }

    // close lamp modulator
    //
    if ( which == "all" || which == "lampmod" ) {
      long ret = this->modulator.control("close", retstring);
      if (ret!=NO_ERROR) {
        logwrite( function, "ERROR closing lampmod component" );
        error=ERROR;
      }
    }

    retstring.clear();

    return error;
  }
  /***** Calib::Interface::close **********************************************/


  /***** Calib::Interface::publish_snapshot ***********************************/
  /**
   * @brief      publishes a snapshot of my telemetry
   * @details    This publishes a JSON message containing a snapshot of my
   *             telemetry.
   *
   */
  void Interface::publish_snapshot() {
    std::string dontcare;
    this->publish_snapshot(dontcare);
  }
  void Interface::publish_snapshot( std::string &retstring ) {
    const std::string function("Calib::Interface::publish_snapshot");
    std::string ret_isopen;

    // assemble the telemetry into a json message
    //
    nlohmann::json jmessage_out;

    jmessage_out["source"] = "calibd";        // source of this telemetry

    // get the status for each modulator in the map
    // assemble message string of "pow dut per" indexed by name
    //
    this->is_open("lampmod",ret_isopen);
    for ( const auto &[num,name] : this->modulator.modmap_num ) {
      // initialize these on each pass
      double dut=NAN;
      double per=NAN;
      int pow=-1;
      std::stringstream retstream;

      if (ret_isopen=="true") this->modulator.status( num, dut, per, pow );

      switch(pow) {
        case 0 : retstream << "off "; break;
        case 1 : retstream << "on " ; break;
        default: retstream << "err "; break;
      }

      if ( std::isnan(dut) ) retstream << "nan "; else retstream << dut << " ";
      if ( std::isnan(per) ) retstream << "nan "; else retstream << per;

      jmessage_out[name] = retstream.str();
    }

    // get a copy of the motormap and
    // loop through all motors, getting their actuator position
    //
    auto _motormap = this->motion.motorinterface.get_motormap();  // local copy of motormap

    std::string retmotion;
    this->is_open("motion",retmotion);
    bool ismotion = (retmotion=="true" ? true : false);

    for ( const auto &mot : _motormap ) {
      if (ismotion) this->motion.get( mot.first, retstring );     // get position of actuator
      std::string key="CAL"+mot.first;                            // key = CALxxxx
      make_uppercase(key);                                        // make key uppercase
      jmessage_out[ key ] = (ismotion?retstring:"not_connected"); // store in JSON message
    }

    // for backwards compatibility
    jmessage_out["messagetype"]="calibinfo";
    retstring = jmessage_out.dump();  // serialize the json message into retstring
    retstring.append(JEOF);           // append the JSON message terminator

    try {
      this->publisher->publish( jmessage_out );
    }
    catch( const std::exception &e ) {
      logwrite( "Calib::Interface::publish_snapshot",
                "ERROR publishing message: "+std::string(e.what()) );
    }
  }
  /***** Calib::Interface::publish_snapshot ***********************************/


  void Interface::handletopic_snapshot( const nlohmann::json &jmessage ) {
    // If my name is in the jmessage then publish my snapshot
    //
    if ( jmessage.contains( Calib::DAEMON_NAME ) ) {
      this->publish_snapshot();
    }
    else
    if ( jmessage.contains( "test" ) ) {
      logwrite( "Calib::Interface::handletopic_snapshot", jmessage.dump() );
    }
  }


  /***** Calib::Modulator::configure_host *************************************/
  /**
   * @brief      parse LAMPMOD_HOST key to configure the lamp modulator Arduino host
   * @param[in]  input  string should contain "<host> <port>"
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::configure_host( std::string input ) {
    std::string function = "Calib::Modulator::configure_host";
    std::stringstream message;
    std::vector<std::string> tokens;
    std::string tryhost;
    int tryport=-1;

    // In case the Arduino has already been configured then
    // make sure the connection is closed.
    //
    if ( this->arduino != nullptr && this->arduino->sock.isconnected() ) {
      message.str(""); message << "closing existing connection to " << this->arduino->get_name() << " "
                               << this->arduino->get_host() << ":" << this->arduino->get_port();
      logwrite( function, message.str() );
      this->arduino->close();
    }

    // Extract the host and port from the input string
    //
    Tokenize( input, tokens, " " );

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR bad number of parameters in \"" << input
                               << "\": expected 2 but received " << tokens.size();
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      tryhost = tokens.at(0);
      tryport = std::stoi( tokens.at(1) );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Check that (potentially) valid values have been extracted
    //
    if ( tryport < 1 ) {
      message.str(""); message << "ERROR port " << tryport << " must be greater than 0";
      logwrite( function, message.str() );
      return( ERROR );
    }
    if ( tryhost.empty() ) {
      logwrite( function, "ERROR host cannot be empty" );
      return( ERROR );
    }

    // Configure the modulator arduino pointer
    //
    this->arduino = std::make_unique<Network::Interface>("modulator", tryhost, tryport, '\n', '\n');

    return( NO_ERROR );

  }
  /***** Calib::Modulator::configure_host *************************************/


  /***** Calib::Modulator::configure_mod **************************************/
  /**
   * @brief      parse LAMPMOD_MOD key to configure the lamp modulators
   * @param[in]  input  string should contain 4 args, "<num> <name> <D> <T>"
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::configure_mod( std::string input ) {
    std::string function = "Calib::Modulator::configure_mod";
    std::stringstream message;
    std::vector<std::string> tokens;
    std::string tryname;
    int trynum=-1;
    double trydc=-1;
    double tryperiod=-1;

    // Extract the host and port from the input string
    //
    Tokenize( input, tokens, " " );

    if ( tokens.size() != 4 ) {
      message.str(""); message << "ERROR bad number of parameters in \"" << input
                               << "\": expected 4 but received " << tokens.size();
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      trynum    = std::stoi( tokens.at(0) );
      tryname   = tokens.at(1);
      trydc     = std::stod( tokens.at(2) );
      tryperiod = std::stod( tokens.at(3) );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Check that (potentially) valid values have been extracted
    //
    if ( trynum < 1 || trynum > MOD_MAX ) {
      message.str(""); message << "ERROR num " << trynum << " outside range {1:" << MOD_MAX << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }
    if ( tryname.empty() ) {
      logwrite( function, "ERROR name cannot be empty" );
      return( ERROR );
    }
    if ( trydc < 0.0 || trydc > 1.0 ) {
      message.str(""); message << "ERROR duty cycle " << trydc << " outside range {0:1}";
      logwrite( function, message.str() );
      return( ERROR );
    }
    if ( tryperiod != 0 && ( tryperiod < 50.0 || tryperiod > 3600000.0 ) ) {
      message.str(""); message << "ERROR period " << tryperiod << " outside range {0,50:3600000}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // By this point, all values are valid so save them in the class

    this->mod_nums.push_back( trynum );                           // used to validate modulator number <n>

    this->modmap_name[ tryname ] = { trynum, trydc, tryperiod };  // map settings indexed by name, used to set defaults
    this->modmap_num[ trynum ] = tryname;                         // map of names indexed by number

    return( NO_ERROR );

  }
  /***** Calib::Modulator::configure_mod **************************************/


  /***** Calib::Modulator::control ********************************************/
  /**
   * @brief      lamp modulator control main parser
   * @param[in]  args       input arg string
   * @param[out] retstring  return string
   * @return     ERROR | NO_ERROR | HELP
   * @details    Provides lamp modulator control to set and/or get settings.
   *             Input args are:
   *             ? | open | isopen | close | reconnect | default | <n> [ [ on|off ] | [ <D> <T> ] ]
   *
   * In other words,
   *             control              : invalid (returns error)
   *             control ?            : help
   *             control open         : open connection to Arduino
   *             control isopen       : is connection open to Arduino?
   *             control close        : close connection to Arduino
   *             control reconnect    : close, open connection to Arduino
   *             control default      : set all modulators as defined in config file
   *             control <n>          : get status of modulator n
   *             control <n> on|off   : set power state only for modulator <n>
   *             control <n> <D> <T>  : set <D> and <T> for modulator <n>
   *
   * where <n> = {0:MOD_MAX} and n=0 means all modulators {1:MOD_MAX}
   *       <D> = fractional duty cycle {0:1}
   *       <T> = period in msec {0,50:3600000} where 0=off
   *
   */
  long Modulator::control( std::string args, std::string &retstring ) {
    std::string function = "Calib::Modulator::control";
    std::stringstream message;
    long error=NO_ERROR;

    if ( args.empty() ) {
      logwrite( function, "ERROR missing args" );
      retstring="invalid_argument";
      return( ERROR );
    }

    if ( args == "?" ) {
      retstring = CALIBD_LAMPMOD;
      retstring.append( " ? | open | isopen | close | reconnect | <n> [ [ on|off ] | [ <D> <T> ] ]\n" );
      retstring.append( "  Lamp modulator control\n" );
      retstring.append( "  open          :  open connection to controller\n" );
      retstring.append( "  isopen        :  is connection open to controller?\n" );
      retstring.append( "  close         :  close connection to controller\n" );
      retstring.append( "  reconnect     :  close, open connection to controller\n" );
      retstring.append( "  default       :  set all modulators as defined in config file\n" );
      retstring.append( "  <n>           :  get status of modulator <n> (see format below)\n" );
      retstring.append( "  <n> on | off  :  set power-only for modulator <n>\n" );
      retstring.append( "  <n> <D> <T>   :  set <D> and <T> for modulator <n>\n" );
      retstring.append( "\n" );
      message.str(""); message << "  <n> = {0:" << MOD_MAX << "} and n=0 means all modulators {1:" << MOD_MAX << "}\n";
      retstring.append( message.str() );
      retstring.append( "  <D> = fractional duty cycle {0:1}\n" );
      retstring.append( "  <T> = period in msec {0,50:3600000} where 0=off\n" );
      retstring.append( "\n" );
      retstring.append( "  modulators open every <T> msec and close every <T> + ( <D> * <T> ) msec\n" );
      retstring.append( "\n" );
      retstring.append( "  Response to status request <n> is \"<n>,on|off,<D>,<T>\"\n" );
      return HELP;
    }

    // Anything after this requires at least arduino to be configured
    //
    if ( ( this->arduino == nullptr ) ||
       ( ( this->arduino != nullptr ) && ( !this->arduino->is_initialized() ) ) ) {
      logwrite( function, "ERROR Arduino not initialized (check config)" );
      retstring="invalid_configuration";
      return( ERROR );
    }

    // If args is one of these simple strings then perform one of
    // the following actions and immediately return. Since these
    // have their own entry points (apart from this function) they
    // each perform their own checks of the arduino state.
    //
    if ( args == "open" ) return( this->open_arduino() );         // open connection to controller

    if ( args == "isopen" ) {
      retstring = ( this->arduino->isopen() ? "true" : "false" );
      return NO_ERROR;
    }

    if ( args == "close" ) return( this->close_arduino() );       // close connection to controller

    if ( args == "reconnect" ) return( this->reopen_arduino() );  // close, open connection to controller

    if ( args == "default" ) return( this->set_defaults() );      // set defaults from config file

    // Cannot proceed unless a connection is open to the Arduino.
    // (The above calls each have their own internal checks.)
    //
    if ( !this->arduino->isopen() ) {
      logwrite( function, "ERROR no connection open to Arduino" );
      retstring="not_connected";
      return( ERROR );
    }

    // Everything else requires tokenizing the args.
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    // Checking tokens size here means not having to catch out_of_range exception
    //
    if ( tokens.size() < 1 || tokens.size() > 3 ) {
      message.str(""); message << "ERROR: expected {1,2,3} arguments but received " << tokens.size();
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return( ERROR );
    }

    // Since token size was already checked, we are guaranteed to have
    // at least one token here. The first one, tokens[0], is always <n>.
    // The range of <n> is checked first, here, against those that were
    // configured, so that no other function needs to do this.
    //
    int n=-1;

    try {
      n = std::stoi( tokens[0] );

      // check the requested mod number <n> against those configured (and 0)
      //
      auto foundn = std::find( this->mod_nums.begin(), this->mod_nums.end(), n );
      if ( n != 0 && foundn==this->mod_nums.end() ) {
       message.str(""); message << "ERROR invalid mod number " << n << ". expected { 0 ";
       for ( const auto &num : this->mod_nums ) message << num << " ";
       message << "} (check config)";
       retstring="invalid_argument";
       return( ERROR );
      }
    }
    catch ( const std::invalid_argument &e ) {
      message.str(""); message << "ERROR invalid argument \"" << tokens[0] << "\" : " << e.what()
                               << ": expected integer {0:" << MOD_MAX << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Only one of the following versions of control() will be called,
    // for 1, 2, or 3 args. Then return the status of that call.

    // One arg gets status of modulator <n>,
    //
    if ( tokens.size() == 1 ) {  // <n>
      error = control( n, retstring );
    }
    else

    // or two args sets power for modulator <n>,
    //
    if ( tokens.size() == 2 ) {  // <n> on|off
      int power=-1;
      if ( tokens[1] == "on" )  { power=1; }
      else
      if ( tokens[1] == "off" ) { power=0; }
      else {
        message.str(""); message << "ERROR invalid power state \"" << tokens[1] << "\". expected {on|off}";
        logwrite( function, message.str() );
        return( ERROR );
      }
      error = control( n, power );
    }
    else

    // or three args sets <D> and <T> for modulator <n>.
    //
    if ( tokens.size() == 3 ) {  // <n> <D> <T>
      double D=-1.0, T=-1.0;
      try {
        D = std::stod( tokens[1] );
        T = std::stod( tokens[2] );
      }
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR invalid argument in \"" << tokens[1] << " " << tokens[2] << "\" : " << e.what()
                                 << ": expected floats";
        logwrite( function, message.str() );
        return( ERROR );
      }
      error = control( n, D, T );
    }

    if (error==ERROR) retstring="arduino_error";

    return( error );
  }
  /***** Calib::Modulator::control ********************************************/


  /***** Calib::Modulator::control ********************************************/
  /**
   * @brief      lamp modulator control return status
   * @param[in]  num     modulator number
   * @param[out] status  return string
   * @return     ERROR or NO_ERROR
   * @details    This function is overloaded.
   *             This version returns the status of modulator <n>
   *
   */
  long Modulator::control( int num, std::string &status ) {
    std::string function = "Calib::Modulator::control";
    std::stringstream message, retstream;
    long error=NO_ERROR;
    int pow;
    double dut;
    double per;

    if ( num==0 ) {  // read all configured modulators
      for ( const auto &modnum : this->mod_nums ) {
        error |= this->status( modnum, dut, per, pow );
        retstream << modnum << "," << ( pow ? "on" : "off" ) << "," << dut << "," << per << " ";
      }
    }
    else {           // read the specific modulator
      error = this->status( num, dut, per, pow );
      retstream << num << "," << ( pow ? "on" : "off" ) << "," << dut << "," << per;
    }

    status = retstream.str();

    return( error );
  }
  /***** Calib::Modulator::control ********************************************/


  /***** Calib::Modulator::control ********************************************/
  /**
   * @brief      lamp modulator control set power
   * @param[in]  num  modulator number
   * @param[in]  pow  requested power state, 1=on, 0=off
   * @return     ERROR or NO_ERROR
   * @details    This function is overloaded.
   *             This version sets the power for modulator <n>.
   *
   */
  long Modulator::control( int num, int pow ) {
    std::string function = "Calib::Modulator::control";
    std::stringstream message;
    long error=NO_ERROR;

    if ( num==0 ) {  // set all configured modulators
      for ( const auto &modnum : this->mod_nums ) error |= this->power( modnum, pow );
    }
    else {           // set the specific modulator
      error = this->power( num, pow );
    }

    return( error );
  }
  /***** Calib::Modulator::control ********************************************/


  /***** Calib::Modulator::control ********************************************/
  /**
   * @brief      lamp modulator control set D and T
   * @param[in]  num     modulator number
   * @param[in]  dut     duty cycle (D)
   * @param[in]  per     period (T)
   * @return     ERROR or NO_ERROR
   * @details    This function is overloaded.
   *             This version sets the duty cycle and period for modulator <n>.
   *
   */
  long Modulator::control( int num, double dut, double per ) {
    std::string function = "Calib::Modulator::control";
    std::stringstream message;
    long error=NO_ERROR;

    if ( num==0 ) {  // set all configured modulators
      for ( const auto &modnum : this->mod_nums ) error |= this->mod( modnum, dut, per );
    }
    else {           // set the specific modulator
      error = this->mod( num, dut, per );
    }

    return( error );
  }
  /***** Calib::Modulator::control ********************************************/


  /***** Calib::Modulator::set_defaults ***************************************/
  /**
   * @brief      set all modulators as defined in the config file
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::set_defaults() {
    std::string function = "Calib::Modulator::set_defaults";
    std::stringstream message;
    long error=NO_ERROR;

    // Cannot proceed unless a connection is open to the Arduino.
    //
    if ( !this->arduino->isopen() ) {
      logwrite( function, "ERROR no connection open to Arduino" );
      return( ERROR );
    }

    // Set all configured modulators
    //
    for ( const auto &modit : this->modmap_name ) {
      if ( this->mod( modit.second.num, modit.second.dut, modit.second.per ) != NO_ERROR ) {
        message.str(""); message << "ERROR setting modulator " << modit.second.num;
        logwrite( function, message.str() );
        error = ERROR;
      }
    }

    return( error );
  }
  /***** Calib::Modulator::set_defaults ***************************************/


  /***** Calib::Modulator::open_arduino ***************************************/
  /**
   * @brief      open connection to Arduino
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::open_arduino() {
    std::string function = "Calib::Modulator::open_arduino";
    std::stringstream message;

    // Try to open arduino. Network interface handles case if already open.
    //
    if ( this->arduino->open() == ERROR ) {
      message.str(""); message << "ERROR opening " << this->arduino->get_name();
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** Calib::Modulator::open_arduino ***************************************/


  /***** Calib::Modulator::close_arduino **************************************/
  /**
   * @brief      close connection to Arduino
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::close_arduino() {
    std::string function = "Calib::Modulator::close_arduino";
    std::stringstream message;

    // Try to close arduino. Network interface handles case if already closed.
    //
    if ( this->arduino->close() == ERROR ) {
      message.str(""); message << "ERROR closing " << this->arduino->get_name();
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** Calib::Modulator::close_arduino **************************************/


  /***** Calib::Modulator::reopen_arduino *************************************/
  /**
   * @brief      close, then open connection to Arduino
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::reopen_arduino() {
    long error=NO_ERROR;
    if ( error==NO_ERROR ) error = this->close_arduino();
    if ( error==NO_ERROR ) error = this->open_arduino();
    return( error );
  }
  /***** Calib::Modulator::reopen_arduino *************************************/


  /***** Calib::Modulator::send_command ***************************************/
  /**
   * @brief      wrapper to send command to Arduino
   * @details    this wraps the wrapper, used when no reply is needed
   * @param[in]  cmd    formatted command to send
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Modulator::send_command( std::string cmd ) {
    std::string dontcare;
    return send_command( cmd, dontcare );
  }
  /***** Calib::Modulator::send_command ***************************************/


  /***** Calib::Modulator::send_command ***************************************/
  /**
   * @brief      wrapper to send command to Arduino
   * @details    this will add the necessary linefeed
   * @param[in]  cmd    formatted command to send
   * @param[out] reply  reference to string to return reply
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Modulator::send_command( std::string cmd, std::string &reply ) {
    std::string function = "Calib::Modulator::send_command";
    std::stringstream message;
    long error;

    // The Arduino is literally looking for these two separate characters
    // to terminate the command, a backslash and an n. Weird.
    //
    cmd.append( "\\" );
    cmd.append( "n" );

    error = this->arduino->send_command( cmd, reply );

    return error;
  }
  /***** Calib::Modulator::send_command ***************************************/


  /***** Calib::Modulator::mod ************************************************/
  /**
   * @brief      send command to change duty cycle and period of requested modulator
   * @param[in]  num  modulator number
   * @param[in]  dut  duty cycle
   * @param[in]  per  period
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::mod( int num, double dut, double per ) {
    std::string function = "Calib::Modulator::mod";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;

    // First set the duty cycle and period

    // Form and send command to Arduino: "mod,<num>,<dut>,<per>"
    // Expect to get back "<dut>,<per>"
    //
    cmd.str(""); cmd << "mod," << num << "," << dut << "," << per;

    if ( this->send_command( cmd.str(), reply ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Tokenize the reply. Should get back what was written.
    //
    std::vector<std::string> tokens;
    Tokenize( reply, tokens, "," );

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR: malformed reply \"" << reply
                               << "\" for modulator " << num << ". expected <dut>,<per>";
      logwrite( function, message.str() );
      return( ERROR );
    }

    double dut_ret=-1, per_ret=-1;

    try {
      dut_ret = std::stod( tokens[0] );
      per_ret = std::stod( tokens[1] );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR bad return value in \"" << reply << "\" for modulator " << num
                               << ": " << e.what() << ": expected floats for <dut>,<per>";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Make sure the returned values match the values that were requested
    //
    if ( dut_ret != dut || per_ret != per ) {
      message.str(""); message << "ERROR: modulator " << num << " returned " << dut_ret << "," << per_ret
                               << " doesn't match requested " << dut << "," << per;
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Finally, set the power state based on the duty cycle and period:
    // if either duty cycle or period are zero, then the power is set off.
    //
    int pow;
    if ( dut==0 || per==0 ) pow=0; else pow=1;

    // Form and send command to Arduino: "power,<num>,<pow>"
    //
    cmd.str(""); cmd << "power," << num << "," << pow;

    if ( this->send_command( cmd.str() ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str(""); message << "modulator " << num << " D,T set to " << dut << "," << per << " ["
                             << ( pow ? "on" : "off" ) << "]";
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /***** Calib::Modulator::mod ************************************************/


  /***** Calib::Modulator::power **********************************************/
  /**
   * @brief      send command to change power state of requested modulator
   * @param[in]  num  modulator number
   * @param[in]  pow  power state
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::power( int num, int pow ) {
    std::string function = "Calib::Modulator::power";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    long error = NO_ERROR;

    // Form and send command to Arduino: "power,<num>,<pow>".
    // There is no reply.
    //
    cmd.str(""); cmd << "power," << num << "," << pow;

    if ( this->send_command( cmd.str(), reply ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Reply from the power command should match what it was set to.
    //
    try {
      if ( pow != std::stoi(reply) ) {
        message.str(""); message << "ERROR return value " << reply << " doesn't match expected value " << pow;
        logwrite( function, message.str() );
        error = ERROR;
      }
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR bad return value in \"" << reply << "\" for modulator " << num
                               << ": " << e.what() << ": expected integer for <pow>";
      logwrite( function, message.str() );
      error = ERROR;
    }

    message.str(""); message << "modulator " << num << " set to [" << ( pow ? "on" : "off" ) << "]";
    logwrite( function, message.str() );
    return( error );
  }
  /***** Calib::Modulator::power **********************************************/


  /***** Calib::Modulator::status *********************************************/
  /**
   * @brief      send command to read status of requested modulator
   * @param[in]  num  modulator number
   * @param[out] dut  reference to duty cycle
   * @param[out] per  reference to period
   * @param[out] pow  reference to power state
   * @return     ERROR or NO_ERROR
   *
   */
  long Modulator::status( int num, double &dut, double &per, int &pow ) {
    std::string function = "Calib::Modulator::status";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    long error = NO_ERROR;

    // First ask the power state

    // Form and send command to Arduino: "power,<num>"
    // Expect to get back "<pow>"
    //
    cmd.str(""); cmd << "power," << num;

    if ( this->send_command( cmd.str(), reply ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      pow = std::stoi( reply );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR bad return value in \"" << reply << "\" for modulator " << num
                               << ": " << e.what() << ": expected integer for <pow>";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Next ask the dut,per

    // Form and send command to Arduino: "mod,<num>"
    // Expect to get back "<dut>,<per>"
    //
    cmd.str(""); cmd << "mod," << num;

    if ( this->send_command( cmd.str(), reply ) != NO_ERROR ) {
      message.str(""); message << "ERROR sending " << cmd.str();
      logwrite( function, message.str() );
      return( ERROR );
    }

    std::vector<std::string> tokens;
    Tokenize( reply, tokens, "," );

    if ( tokens.size() != 2 ) {
      message.str(""); message << "ERROR: malformed reply \"" << reply
                               << "\" for modulator " << num << ". expected <dut>,<per>";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      dut = std::stod( tokens[0] );
      per = std::stod( tokens[1] );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR bad return value in \"" << reply << "\" for modulator " << num
                               << ": " << e.what() << ": expected floats for <dut>,<per>";
      logwrite( function, message.str() );
      error = ERROR;
    }

    return( error );
  }
  /***** Calib::Modulator::status *********************************************/

}
