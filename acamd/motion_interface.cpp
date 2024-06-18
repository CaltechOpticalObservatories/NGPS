/**
 * @file    motion_interface.cpp
 * @brief   this contains the motion interface code used by acam
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the MotionInterface class in the Acam namespace,
 * and is how the acam daemon interfaces to the motion hardware.
 *
 */

#include "motion_interface.h"

namespace Acam {

  /***** Acam::MotionInterface::initialize_class ******************************/
  /**
   * @brief      initializes the MotionInterface class from configure_acam()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Acam::Server::configure_acam() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long MotionInterface::initialize_class() {
    std::string function = "Acam::MotionInterface::initialize_class";
    std::stringstream message;

    auto _motormap = this->motorinterface.get_motormap();

    this->numdev = _motormap.size();

    return( NO_ERROR );
  }
  /***** Acam::MotionInterface::initialize_class ******************************/


  /***** Acam::Interface::open ************************************************/
  /**
   * @brief      opens the sockets to all motors
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::open( ) {
    std::string function = "Acam::MotionInterface::open";
    std::stringstream message;
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
  /***** Acam::Interface::open ************************************************/


  /***** Acam::MotionInterface::close *****************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::close( ) {
    return this->motorinterface.close();
  }
  /***** Acam::MotionInterface::close *****************************************/


  /***** Acam::Interface::is_open *********************************************/
  /**
   * @brief       return the connected state of the motor controllers
   * @param[in]   arg        used only for help
   * @param[out]  retstring  contains the connected state "true" | "false"
   * @return      ERROR | NO_ERROR | HELP
   *
   * All motors must be connected for this to return "true".
   *
   */
  bool MotionInterface::is_open() {
    std::string state;
    this->is_open( "", state );
    return ( state=="true" ? true : false );
  }
  long MotionInterface::is_open( std::string arg, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::is_open";
    std::stringstream message;
    long error = NO_ERROR;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( arg == "?" ) {
      retstring = ACAMD_ISOPEN;
      retstring.append( " \n" );
      retstring.append( "  Returns true if all motor controllers are connected, false if any one is not connected.\n" );
      return( HELP );
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
  /***** Acam::Interface::is_open *********************************************/


  /***** Acam::MotionInterface::home ******************************************/
  /**
   * @brief      home all daisy-chained motors using the neg limit switch
   * @param[in]  name_in    optional list of motors to home
   * @param[out] retstring  reference to return string
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long MotionInterface::home( std::string name_in, std::string &retstring ) {

    // Help
    //
    if ( name_in == "?" ) {
      retstring = ACAMD_HOME;
      retstring.append( " [ " );
      for ( const auto &mot : this->motorinterface.get_motormap() ) {
        retstring.append( mot.first );
        retstring.append( " " );
      }
      retstring.append( "]\n" );
      retstring.append( "  no arg will home all motors simultaneously\n" );
      retstring.append( "  or provide name of motor to home only that motor\n" );
      return( HELP );
    }

logwrite( "Acam::MotionInterface::home", name_in );
    // All the work is done by the PI motor interface class
    //
    return this->motorinterface.home( name_in, retstring );
  }
  /***** Acam::MotionInterface::home ******************************************/


  /***** Acam::MotionInterface::is_home ***************************************/
  /**
   * @brief       return the home state of the motors
   * @param[in]   name_in  optionally contains one or more motors to check
   * @param[out]  retstring  contains the home state ("true" | "false")
   * @return      ERROR | NO_ERROR | HELP
   *
   * All named motors must be homed for this to return "true".
   *
   */
  long MotionInterface::is_home( std::string name_in, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::is_home";
    std::stringstream message;
    long error = NO_ERROR;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( name_in == "?" ) {
      retstring = ACAMD_ISHOME;
      retstring.append( " [ " );
      for ( const auto &mot : _motormap ) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "]\n" );
      retstring.append( "  Reads the referencing state from each of the indicated controllers,\n" );
      retstring.append( "  or all controllers if none supplied. Returns true if all (named) are\n" );
      retstring.append( "  homed, false if any one is not homed.\n" );
      return( HELP );
    }

    // Build a vector of all selected motor controllers, or all motor controllers
    // if name_in was empty.
    //
    std::vector<std::string> name_list;
    if ( name_in.empty() ) {
      for ( const auto &mot : _motormap ) { name_list.push_back( mot.first ); }
    }
    else {
      std::transform( name_in.begin(), name_in.end(), name_in.begin(), ::tolower );
      Tokenize( name_in, name_list, " " );
      if ( name_list.size() > this->numdev ) {
        message.str(""); message << "ERROR: too many names specified: " << name_in.size() << " "
                                 << "(max " << this->numdev << ")";
        logwrite( function, message.str() );
        retstring="bad_args";
        return( ERROR );
      }
    }

    // Loop through that vector of names, asking each if homed,
    // and keeping count of the number that are homed.
    //
    size_t num_home=0;
    std::stringstream homestream;
    for ( const auto &name : name_list ) {

      // requires an open connection
      //
      if ( ! this->motorinterface.is_connected( name ) ) {
        message.str(""); message << "ERROR not connected to motor " << name;
        logwrite( function, message.str() );
        retstring="not_connected";
        return( ERROR );
      }

      auto addr = _motormap[name].addr;
      int axis = 1;
      bool _ishome;

      error |= this->motorinterface.is_home( name, addr, axis, _ishome );  // error is OR'd so any error is preserved
      homestream << name << ":" << ( _ishome ? "true" : "false" ) << " ";
      if ( _ishome ) num_home++;
    }

    // Set the retstring true or false, true only if all requested controllers are the same
    //
    if ( num_home == name_list.size() ) retstring = "true"; else retstring = "false";

    // If not all are the same state then log that but report false
    //
    if ( num_home > 0 && num_home < name_list.size() ) {
      logwrite( function, homestream.str() );
      retstring = "false";
    }

    return( error );
  }
  /***** Acam::MotionInterface::is_home ***************************************/


  /***** Acam::MotionInterface::motion ****************************************/
  /**
   * @brief      basic motion test commands
   * @param[in]  args       string containing optional args (see details below)
   * @param[out] retstring  return string contains response
   * @return     ERROR | NO_ERROR | HELP
   *
   * expected args: [ [ <name> [ native <cmd> | <posname> ] ] ]
   *
   * Only the native command requires an open connection to hardware;
   * everything else is just information held within the class.
   *
   */
  long MotionInterface::motion( std::string args, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::motion";
    std::stringstream message, retstream;
    long error = NO_ERROR;

    // help
    //
    if ( args == "?" ) {
      retstring = ACAMD_MOTION;
      retstring.append( " [ [ <name> [ native <cmd> | <posname> ] ] ]\n" );
      retstring.append( "  no arg gives list of motor names\n" );
      retstring.append( "  <name> only gives list of posnames\n" );
      retstring.append( "  <name> <posname> gives info for that posname\n" );
      retstring.append( "  <name> native <cmd> sends native commands to that motor\n" );
      return( HELP );
    }

    auto _motormap = this->motorinterface.get_motormap();

    // No args, print all of the indices of the motormap STL map,
    // which will be a list of the STEPPER_CONTROLLER names.
    //
    if ( args.empty() ) {
      retstream << "{ ";
      for ( const auto &mot : _motormap ) { retstream << mot.first << " "; }
      retstream << "}";
      retstring = retstream.str();
      logwrite( function, retstring );
      return( NO_ERROR );
    }

    std::vector<std::string> arglist;

    Tokenize( args, arglist, " " );

    if ( arglist.size() <= 0 ) {  // should be impossible since args ! empty
      logwrite( function, "ERROR: bad argument list" );
      retstring="bad_args";
      return( ERROR );
    }

    // One arg, taken to be the name of a STEPPER_CONTROLLER,
    // print all of the indices of the posmap STL map for the named controller,
    // which will be a list of the STEPPER_POS position names.
    //
    std::string name;
    if ( arglist.size() >= 1 ) {
      name = arglist[0];

      // Check that name is found in the motormap map
      //
      if ( _motormap.find( name ) == _motormap.end() ) {
        message.str(""); message << "ERROR: motor name \"" << name << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        retstring="motor_not_found";
        return( ERROR );
      }

      if ( arglist.size() == 1 ) {
        retstream << "{ ";
        for ( const auto &pos : _motormap[ name ].posmap ) { retstream << pos.first << " "; }
        retstream << "}";
        retstring = retstream.str();
        logwrite( function, retstring );
      }
    }

    // Two args is taken to be requesting for info of a posname for the given controller name
    //
    if ( arglist.size() == 2 ) {
      std::string posname = arglist[1];

      // Check that posname is found in the motormap.posmap map
      //
      if ( _motormap[name].posmap.find( posname ) == _motormap[name].posmap.end() ) {
        message.str(""); message << "ERROR: position name \"" << posname << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        retstring="posname_not_found";
        return( ERROR );
      }

      retstream << _motormap[ name ].posmap[ posname ].posname << ": "
                << "posid="    << _motormap[ name ].posmap[ posname ].posid << " "
                << "position=" << _motormap[ name ].posmap[ posname ].position;
      retstring = retstream.str();
      logwrite( function, retstring );
    }

    // More than 2 args is taken to be the "native" command. This allows
    // sending native controller commands directly to the PI controller
    // specified by <name>, e.g. 
    //    motion filter native FRF?
    // or 
    //    motion dustcover native MOV 1 0
    //
    // Only commands with a "?" will provide a reply in retstring.
    //
    if ( arglist.size() > 2 ) {
      if ( arglist[1] != "native" ) {
        logwrite( function, "ERROR: expected motion [ [ <name> [ native <cmd> | <posname> ] ] ]" );
        retstring="bad_args";
        return( ERROR );
      }
      else {

        int addr = _motormap[ name ].addr;  // Get the address from the class,
        std::stringstream cmdstream;        // then build the command from the arglist.
        cmdstream << addr;
        for ( int i=2; i<(int)arglist.size(); i++ ) cmdstream << " " << arglist[i];
        error = this->send_command( cmdstream.str(), retstring );
        logwrite( function, retstring );
      }
    }

    return( error );
  }
  /***** Acam::MotionInterface::motion ****************************************/


  /***** Acam::MotionInterface::send_command **********************************/
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
  long MotionInterface::send_command( const std::string &name, std::string cmd ) {
    std::string function = "Acam::MotionInterface::send_command";
    std::stringstream message;

    return( this->motorinterface.send_command( name, cmd ) );
  }
  /***** Acam::MotionInterface::send_command **********************************/


  /***** Acam::MotionInterface::send_command **********************************/
  /**
   * @brief      writes the raw command to the master controller, reads back reply
   * @param[in]  name       controller name
   * @param[in]  cmd        command to send
   * @param[out] retstring  reference to reply
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   * This version writes a command and then reads a reply if that command contains
   * a question mark, "?".
   *
   */
  long MotionInterface::send_command( const std::string &name, std::string cmd, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::send_command";
    std::stringstream message;

    if ( cmd.find( "?" ) != std::string::npos ) {
      return ( this->motorinterface.send_command( name, cmd, retstring ) );
    }
    else {
      return( this->motorinterface.send_command( name, cmd ) );
    }
  }
  /***** Acam::MotionInterface::send_command **********************************/


  /***** Acam::MotionInterface::filter ****************************************/
  /**
   * @brief      set or get the filter
   * @param[in]  destname   string containing destination filter position name
   * @param[out] retstring  return string contains the current position name
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long MotionInterface::filter( std::string destname, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::filter";
    std::string filter = "filter";
    std::stringstream message;
    long error = NO_ERROR;

    auto _motormap = this->motorinterface.get_motormap();

    // Help
    //
    if ( destname == "?" ) {
      retstring = ACAMD_FILTER;
      retstring.append( " [ <filtername> | home | ishome ]\n" );
      retstring.append( "  Move filterwheel to filter <filtername> \n" );
      retstring.append( "  where <filtername> = { " );
      for ( const auto &pos : _motormap[ filter ].posmap ) {
        retstring.append( pos.first );
        retstring.append( " " );
      }
      retstring.append( "}\n" );
      retstring.append( "  and return <filtername>.\n" );
      retstring.append( "  Optionally home the filterwheel or check if homed, return true|false.\n" );
      retstring.append( "  If no arg provided, return only the current <filtername>.\n" );
      return( HELP );
    }

    // Option to home the filter or check if homed
    //
    if ( destname == "home" ) {
      this->home( filter, destname );
      destname.clear();
    }
    else
    if ( destname == "ishome" ) {
      return this->is_home( filter, retstring );
    }

    // Otherwise filter motor must be homed
    //
    std::string ishome;
    error = this->is_home( filter, ishome );
    if ( error != NO_ERROR ) return( error );
    else
    if ( error==NO_ERROR && ishome != "true" ) {
      logwrite( function, "ERROR: filter motor is not homed" );
      retstring = "not_homed";
      return( ERROR );
    }

    // If destname is supplied then move wheel to that position.
    //
    if ( !destname.empty() ) {
      int currid;
      std::string currname;
      float currpos;
      int axis = 1;

      // Check that destname is found in the motormap.posmap map
      //
      if ( _motormap[filter].posmap.find( destname ) == _motormap[filter].posmap.end() ) {
        message.str(""); message << "ERROR: position \"" << destname << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        retstring="filter_not_found";
        return( ERROR );
      }

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dest=" << _motormap[filter].posmap[destname].posname << " "
                               << "posid="        << _motormap[filter].posmap[destname].posid << " "
                               << "position="     << _motormap[filter].posmap[destname].position;
      logwrite( function, message.str() );
#endif

      // Where are we now?
      //
      error = this->current_filter( currname, currid, currpos );

      // Where do we want to go?
      //
      int destid = _motormap[ filter ].posmap[ destname ].posid;
      float destpos = _motormap[ filter ].posmap[ destname ].position;
      int newid = mod( ( destid + ( 2 - currid ) ) , 6 );

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] currid=" << currid << " destid=" << destid << " newid=" << newid;
      logwrite( function, message.str() );
#endif

      // Find the position for id=2 so that we can temporarily remap
      // current position as position #2, the "midpoint" of the wheel.
      // This allows for the shortest movement since we can move in
      // either direction +/- from the middle, without having to go
      // possibly the long way around.
      //
      std::stringstream cmd;
      float remap;
      for ( auto fit  = _motormap[filter].posmap.begin();
                 fit != _motormap[filter].posmap.end(); ++fit) {
        if ( fit->second.posid == 2 ) remap = fit->second.position;       // remap = (position for posid=2)
      }
      cmd.str(""); cmd << "RON " << axis << " 0";          this->send_command( filter, cmd.str() );
      cmd.str(""); cmd << "POS " << axis << " " << remap;  this->send_command( filter, cmd.str() );
      cmd.str(""); cmd << "RON " << axis << " 1";          this->send_command( filter, cmd.str() );

      // Now move to the position with the calculated "newid"
      //
      float newpos=NAN;
      for ( auto fit  = _motormap[filter].posmap.begin();
                 fit != _motormap[filter].posmap.end(); ++fit) {
        if ( fit->second.posid == newid ) newpos = fit->second.position;  // newpos = (position for posid=newid)
      }
      if ( ! std::isnan( newpos ) ) error = this->motorinterface.moveto( filter, axis, newpos, retstring );
      else {
        message.str(""); message << "ERROR: no position found for filter ID " << newid;
        logwrite( function, message.str() );
      }

      // Then map back to the correct order for the destination filter,
      // or to the current position if no position was found for newid,
      // which means we never moved.
      //
      remap = ( !std::isnan(newpos) ? destpos : currpos );

      cmd.str(""); cmd << "RON " << axis << " 0";          this->send_command( filter, cmd.str() );
      cmd.str(""); cmd << "POS " << axis << " " << remap;  this->send_command( filter, cmd.str() );
      cmd.str(""); cmd << "RON " << axis << " 1";          this->send_command( filter, cmd.str() );
    }
#ifdef LOGLEVEL_DEBUG
    else {
      logwrite( function, "[DEBUG] destname empty, query-only" );
    }
#endif

    // Whether or not a filter was supplied, read the current position now.
    //
//  int addr = _motormap[ filter ].addr;
//  float pos;
//  error = this->pi.get_pos( addr, axis, pos );

logwrite( function, "[DEBUG] query current filter" );
    error = this->current_filter( retstring );
message.str(""); message << "[DEBUG] current_filter = " << retstring; logwrite( function, message.str() );

    return( error );
  }
  /***** Acam::MotionInterface::filter ****************************************/


  /***** Acam::MotionInterface::current_filter ********************************/
  /**
   * @brief      get the current filter position name
   * @param[out] currname  return string contains the current position name
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long MotionInterface::current_filter( std::string &currname ) {
    std::string function = "Acam::MotionInterface::currernt_filter";
    int currid;
    float currpos;
    long error = this->current_filter( currname, currid, currpos );
    return( error );
  }
  /***** Acam::MotionInterface::current_filter ********************************/


  /***** Acam::MotionInterface::current_filter ********************************/
  /**
   * @brief      get the current filter position name
   * @param[out] currname  contains the current position name
   * @param[out] currid    contains the current position id
   * @param[out] currpos   contains the current position position
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long MotionInterface::current_filter( std::string &currname, int &currid, float &currpos ) {
    std::string function = "Acam::MotionInterface::currernt_filter";
    std::string filter = "filter";
    std::stringstream message;
    long error = NO_ERROR;
    float tolerance = 2;

    // get the position for the requested channel
    //
    std::string posstring;
    int axis=1;
    auto addr=this->motorinterface.get_motormap()[currname].addr;
    currpos=NAN;
    std::string posname;

    error = this->motorinterface.get_pos( filter, axis, addr, currpos, posname, tolerance );

    // form the return value
    //
    message.str(""); message << currname << " " << std::fixed << std::setprecision(3) << currpos;
    if ( ! posname.empty() ) { message << " (" << posname << ")"; }
    logwrite( function, message.str() );

    return( error );
  }
  /***** Acam::MotionInterface::current_filter ********************************/


  /***** Acam::MotionInterface::cover *****************************************/
  /**
   * @brief      set or get the cover position
   * @param[in]  posname    string containing { "open" | "close" | "home" | "ishome" | "?" }
   * @param[out] retstring  return string contains the current position name
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long MotionInterface::cover( std::string posname, std::string &retstring ) {
    std::string function = "Acam::MotionInterface::cover";
    std::string cover = "cover";
    std::stringstream message;
    float tolerance = 2;
    long error = NO_ERROR;

    // Help
    //
    if ( posname == "?" ) {
      retstring = ACAMD_COVER;
      retstring.append( " [ open | close | home | ishome ]\n" );
      retstring.append( "  Move cover to position open | close, or home the cover motor.\n" );
      retstring.append( "  For these three options, return open|closed\n" );
      retstring.append( "  For \"ishome\" return true|false\n" );
      retstring.append( "  If no arg provided, return only the current position { open | closed }\n" );
      return( HELP );
    }

    // Option to home the cover
    //
    if ( posname == "home" ) {
      this->home( cover, posname );
      posname.clear();
    }
    else
    if ( posname == "ishome" ) {
      return this->is_home( cover, retstring );
    }

    // Otherwise cover motor must be homed
    //
    std::string ishome;
    error = this->is_home( cover, ishome );
    if ( error != NO_ERROR ) { retstring="not_homed"; return( error ); }
    else
    if ( error==NO_ERROR && ishome != "true" ) {
      logwrite( function, "ERROR: cover motor is not homed" );
      retstring = "not_homed";
      return( ERROR );
    }

    int axis = 1;
    auto _motormap = this->motorinterface.get_motormap();

    // If posname is supplied then move cover to that position.
    //
    if ( !posname.empty() ) {

      // Check that posname is found in the motormap.posmap map
      //
      if ( _motormap[cover].posmap.find( posname ) == _motormap[cover].posmap.end() ) {
        message.str(""); message << "ERROR: position \"" << posname << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        retstring="not_found";
        return( ERROR );
      }

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dest=" << _motormap[cover].posmap[posname].posname << " "
                               << "posid="        << _motormap[cover].posmap[posname].posid << " "
                               << "position="     << _motormap[cover].posmap[posname].position;
      logwrite( function, message.str() );
#endif

      this->motorinterface.moveto( cover, axis, _motormap[cover].posmap[posname].position, retstring );

    }

    // get the position
    //
    auto addr=this->motorinterface.get_motormap()[cover].addr;
    float position=NAN;

    error = this->motorinterface.get_pos( cover, axis, addr, position, posname, tolerance );

    // form the return value
    //
    message.str(""); message << posname << " " << std::fixed << std::setprecision(3) << position;
    if ( ! posname.empty() ) { message << " (" << posname << ")"; }
    logwrite( function, message.str() );

    retstring=message.str();

    return( error );
  }
  /***** Acam::MotionInterface::cover *****************************************/

}
