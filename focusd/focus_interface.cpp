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

  /***** Focus::Interface::initialize_class ***********************************/
  /**
   * @brief      initializes the class from configure_focusd()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Focus::Server::configure_focusd() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Focus::Interface::initialize_class";
    std::stringstream message;

    this->numdev = this->motorinterface.get_motormap().size();

    // I don't want to prevent the system from working with a subset of controllers,
    // but the user should be warned, in case it wasn't intentional.
    //
    if ( this->numdev != 3 ) {
      message.str(""); message << "WARNING: " << this->numdev << " PI motor controller"
                               << ( this->numdev == 1 ? "" : "s" ) << " defined!";
      logwrite( function, message.str() );
    }

    return( NO_ERROR );
  }
  /***** Focus::Interface::initialize_class ***********************************/


  /***** Focus::Interface::open ***********************************************/
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

    return( error );
  }
  /***** Focus::Interface::open ***********************************************/


  /***** Focus::Interface::close **********************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    return this->motorinterface.close();
  }
  /***** Focus::Interface::close **********************************************/


  /***** Focus::Interface::is_open ********************************************/
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
    std::string function = "Focus::Interface::is_open";
    std::stringstream message;
    long error = NO_ERROR;

    auto _motormap = this->motorinterface.get_motormap();

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
    std::stringstream message;
    std::string chan, cmd;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( args == "?" ) {
      retstring = FOCUSD_NATIVE;
      retstring.append( " <chan> <cmd>\n" );
      retstring.append( "  Send native command <cmd> to controller indicated by channel name,\n" );
      retstring.append( "  where <chan> is one of { " );
      for ( const auto &mot : _motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "} and <cmd> is any PI-native command and args. This command blocks;\n" );
      retstring.append( "native commands are not run in a separate thread.\n" );
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

    if ( _motormap.find( chan ) == _motormap.end() ) {
      message.str(""); message << "ERROR motor \"" << chan << "\" not found";
      logwrite( function, message.str() );
      retstring="unknown_motor";
      return( ERROR );
    }

    // requires an open connection
    //
    if ( ! this->motorinterface.is_connected( chan ) ) {
      logwrite( function, "ERROR not connected to motor controller" );
      retstring="not_connected";
      return( ERROR );
    }

    message.str("");
    message << _motormap.at(chan).addr << " " << cmd;

    logwrite( function, message.str() );

    return send_command( chan, message.str(), retstring );
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
      auto _motormap = this->motorinterface.get_motormap();
      for ( const auto &mot : _motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "}.\n" );
      return HELP;
    }

    // All the work is done by the PI motor interface class
    //
    return this->motorinterface.home( name_in, retstring );
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

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( args == "?" ) {
      retstring = FOCUSD_SET;
      retstring.append( " <chan> { <pos> | nominal }\n" );
      retstring.append( "  Set focus position of indicated channel to <pos> or to the nominal best focus.\n" );
      retstring.append( "  where <chan> <min> <nominal> <max> are as follows:\n" );
      try {
        for ( auto &mot : _motormap ) {
          retstring.append( "     " );
          retstring.append( mot.first ); retstring.append( " " );
          message.str(""); message << std::fixed << std::setprecision(3) << mot.second.axes[axis].min << " ";
          retstring.append( message.str() );
          message.str(""); message << std::fixed << std::setprecision(3) << mot.second.posmap.at("nominal").position << " ";
          retstring.append( message.str() );
          message.str(""); message << std::fixed << std::setprecision(3) << mot.second.axes[axis].max << " ";
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

    long error = this->motorinterface.moveto( chan, axis, posstr, retstring );

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
      auto _motormap = this->motorinterface.get_motormap();
      for ( const auto &mot : _motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "}.\n" );
      return HELP;
    }

    // Need something, anything
    //
    if ( name.empty() ) {
      logwrite( function, "ERROR expected <chan>" );
      retstring="invalid_argument";
      return( ERROR );
    }

    // get the position for the requested channel
    //
    std::string posstring;
    int axis=1;
    auto addr=this->motorinterface.get_motormap()[name].addr;
    float position=NAN;
    std::string posname;
    error = this->motorinterface.get_pos( name, axis, addr, position, posname );

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
   * This could be called by a thread, so hardware interactions with the PI
   * controller are protected by a mutex.
   *
   */
  long Interface::move_abs( int addr, float pos ) {
    std::string function = "Focus::Interface::move_abs";
    std::stringstream message;
    long error=NO_ERROR;

    auto _motormap = this->motorinterface.get_motormap();

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
//  error = this->motorinterface.move_abs( name, addr, axis, pos );

    if ( error != NO_ERROR ) {
      message.str(""); message << "ERROR moving " << name << " to " << pos;
      logwrite( function, message.str() );
      return error;
    }

    // Wait for motor to be on target
    //
    message.str(""); message << "waiting for " << name;
    logwrite( function, message.str() );
//  error = this->motorinterface.move_axis_wait( name, addr, axis );

    return( error );
  }
  /***** Focus::Interface::move_abs *******************************************/


  /***** Focus::Interface::stop ***********************************************/
  /**
   * @brief      send the stop-all-motion command to all controllers
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::stop( ) {
    std::string function = "Focus::Interface::stop";
    std::stringstream message;

    // send the stop_motion command for each address in controller_info
    //
    auto _motormap = this->motorinterface.get_motormap();

    for ( const auto &mot : _motormap ) {
      this->motorinterface.stop_motion( mot.second.name, mot.second.addr );
    }

    return( NO_ERROR );
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
    return( this->motorinterface.send_command( name, cmd ) );
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
    if ( cmd.find( "?" ) != std::string::npos ) {
      return( this->motorinterface.send_command( name, cmd, retstring ) );
    }
    else {
      return( this->motorinterface.send_command( name, cmd ) );
    }
  }
  /***** Focus::Interface::send_command ***************************************/


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
      retstring="name host:port addr naxes axisnum min max reftype";
      logwrite( function, retstring );
      for ( const auto &mot : _motormap ) {
        retstring.append("\n");
        message.str(""); message << mot.first << " "
                                 << mot.second.host << ":"
                                 << mot.second.port << " "
                                 << mot.second.addr << " "
                                 << mot.second.naxes;
        for ( const auto &axis : mot.second.axes ) {
          message << " " << axis.second.axisnum << " " << axis.second.min << " " << axis.second.max << " " << axis.second.reftype;
        }
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
      for ( const auto &mot : _motormap ) {
        retstring.append("\n");
        message.str(""); message << mot.first << " ";
        for ( const auto &pos : mot.second.posmap ) {
          message << " " << pos.second.axis << " " << pos.second.posid << " " << pos.second.position << " " << pos.second.posname;
        }
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
