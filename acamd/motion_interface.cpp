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

  /***** Acam::Interface::open ************************************************/
  /**
   * @brief      opens the sockets to all motors
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::open( ) {
    const std::string function("Acam::MotionInterface::open");
    long error = NO_ERROR;

    // Open connection to all motors
    for (auto &mot : this->motors) error |= mot.second.open();

    // clear any error codes on startup (PI-only)
    this->pi_interface->clear_errors();

    // enable motion (servo for PI) for each address in controller_info.
    for (auto &mot : this->motors) error |= mot.second.enable_motion();

    error |= get_current_filter( this->current_filtername );
    error |= read_cover( this->current_coverpos );

    return error;
  }
  /***** Acam::Interface::open ************************************************/


  /***** Acam::MotionInterface::close *****************************************/
  /**
   * @brief      closes the PI socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long MotionInterface::close( ) {
    long error=NO_ERROR;
    for (auto &mot : this->motors) error |= mot.second.close();
    return error;
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
    const std::string function("Acam::MotionInterface::is_open");
    std::stringstream message;

    // Help
    //
    if ( arg == "?" ) {
      retstring = ACAMD_ISOPEN;
      retstring.append( " \n" );
      retstring.append( "  Returns true if all motor controllers are connected, false if any one is not connected.\n" );
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
      for (auto &mot : this->motors ) {
        retstring.append( mot.first );
        retstring.append( " " );
      }
      retstring.append( "]\n" );
      retstring.append( "  no arg will home all motors simultaneously\n" );
      retstring.append( "  or provide name of motor to home only that motor\n" );
      return( HELP );
    }

    // All the work is done by the motor interface class
    //
    return this->motors.at(name_in).home(&retstring);
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
    const std::string function("Acam::MotionInterface::is_home");
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( name_in == "?" ) {
      retstring = ACAMD_ISHOME;
      retstring.append( " [ " );
      for (auto &mot : this->motors) { retstring.append( mot.first ); retstring.append( " " ); }
      retstring.append( "]\n" );
      retstring.append( "  Reads the referencing state from each of the indicated controllers,\n" );
      retstring.append( "  or all controllers if none supplied. Returns true if all (named) are\n" );
      retstring.append( "  homed, false if any one is not homed.\n" );
      return HELP;
    }

    // Build a vector of all selected motor controllers, or all motor controllers
    // if name_in was empty.
    //
    std::vector<std::string> name_list;
    if ( name_in.empty() ) {
      for (auto &mot : this->motors) { name_list.push_back( mot.first ); }
    }
    else {
      std::transform( name_in.begin(), name_in.end(), name_in.begin(), ::tolower );
      Tokenize( name_in, name_list, " " );
      if ( name_list.size() > this->motors.size() ) {
        message.str(""); message << "ERROR: too many names specified: " << name_in.size() << " "
                                 << "(max " << this->motors.size() << ")";
        logwrite(function, message.str());
        retstring="bad_args";
        return ERROR;
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
      if ( ! this->motors.at(name).is_connected() ) {
        message.str(""); message << "ERROR not connected to motor " << name;
        logwrite( function, message.str() );
        retstring="not_connected";
        return( ERROR );
      }

      bool _ishome = this->motors.at(name).is_home();
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
    const std::string function("Acam::MotionInterface::motion");
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
      return HELP;
    }

    // No args, print all of the indices of the motormap STL map,
    // which will be a list of the STEPPER_CONTROLLER names.
    //
    if ( args.empty() ) {
      retstream << "{ ";
      for (auto &mot : this->motors) { retstream << mot.first << " "; }
      retstream << "}";
      retstring = retstream.str();
      logwrite( function, retstring );
      return NO_ERROR;
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
      if ( this->motors.find(name) == this->motors.end() ) {
        logwrite(function, "ERROR '"+name+"' not found in configuration");
        retstring="motor_not_found";
        return ERROR;
      }

      if ( arglist.size() == 1 ) {
        retstream << "{ ";
        for (auto &mot : this->motors) {
          for (const auto &pos : mot.second.posnames()) {
            retstream << pos << " ";
          }
        }
        retstream << "}";
        retstring = retstream.str();
        logwrite( function, retstring );
      }
    }

    // Two args is taken to be requesting for info of a posname for the given controller name
    //
    if ( arglist.size() == 2 ) {
      std::string posname = arglist[1];

      const auto* posinfo = this->motors.at(name).posinfo(posname);

      if (posinfo==nullptr) {  // posname not found in the motormap.posmap map
        logwrite(function, "ERROR position '"+posname+"' not found in configuration.");
        retstring="posname_not_found";
        return ERROR;
      }

      retstream << posname << ": "
                << "posid="    << posinfo->posid
                << "position=" << posinfo->position;
      retstring = retstream.str();
      logwrite( function, retstring );
    }

    // More than 2 args is taken to be the "native" command. This allows
    // sending native controller commands directly to the PI controller
    // specified by <name>, e.g. 
    //    motion filter native FRF?
    // or 
    //    motion cover native MOV 1 0
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
        // build the command from the arglist after name
        //
        std::stringstream cmdstream;
        for ( int i=2; i<(int)arglist.size(); i++ ) cmdstream << " " << arglist[i];

        // then send the command
        error = this->send_command( name, cmdstream.str(), retstring );
        logwrite( function, retstring );
      }
    }

    error |= get_current_filter( this->current_filtername );
    error |= read_cover( this->current_coverpos );

    return error;
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
    return this->motors.at(name).send_command(cmd);
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
      return this->motors.at(name).send_command(cmd, &retstring);
    }
    else {
      return this->motors.at(name).send_command(cmd);
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
    const std::string function("Acam::MotionInterface::filter");
    std::string filter = "filter";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( destname == "?" ) {
      retstring = ACAMD_FILTER;
      retstring.append( " [ <filtername> | home | ishome ]\n" );
      retstring.append( "  Move filterwheel to filter <filtername> \n" );
      retstring.append( "  where <filtername> = { " );
      for (const auto &[posname, posinfo] : *this->motors.at(filter).posmap()) {
        retstring.append(posname);
        retstring.append(" ");
      }
      retstring.append( "}\n" );
      retstring.append( "  and return <filtername>.\n" );
      retstring.append( "  Optionally home the filterwheel or check if homed, return true|false.\n" );
      retstring.append( "  If no arg provided, return only the current <filtername>.\n" );
      return HELP;
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
      return ERROR;
    }

    // If destname is supplied then move wheel to that position.
    //
    if ( !destname.empty() ) {
      int currid;
      std::string currname;
      float currpos;
      int axis = 1;

      const auto* posinfo = this->motors.at(filter).posinfo(destname);

      if (posinfo==nullptr) {
        logwrite(function, "ERROR position '"+destname+"' not found in configuration");
        retstring="filter_not_found";
        return ERROR;
      }

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dest=" << posinfo->posname << " "
                               << "posid="        << posinfo->posid << " "
                               << "position="     << posinfo->position;
      logwrite( function, message.str() );
#endif

      // Where are we now?
      //
      error = this->get_current_filter( currname, currid, currpos );

      // Where do we want to go?
      //
      int destid = posinfo->posid;
      float destpos = posinfo->position;
      int newid = mod( ( destid + ( 2 - currid ) ) , 6 );

      // Find the position for id=2 so that we can temporarily remap
      // current position as position #2, the "midpoint" of the wheel.
      // This allows for the shortest movement since we can move in
      // either direction +/- from the middle, without having to go
      // possibly the long way around.
      //
      float remap=NAN;
      const auto* posmap = this->motors.at(filter).posmap();
      if (posmap != nullptr) {
        for (const auto &[name, pos] : *posmap) {
          if (pos.posid == 2) {
            remap = pos.position;  // remap = (position for posid=2)
            break;
          }
        }
      }
      if (std::isnan(remap)) {
        logwrite(function, "ERROR no position found for midpoint position id=2");
        return ERROR;
      }
      std::ostringstream cmd;
      cmd.str(""); cmd << "RON " << axis << " 0";         this->send_command( filter, cmd.str() );  // reference mode on
      cmd.str(""); cmd << "POS " << axis << " " << remap; this->send_command( filter, cmd.str() );  // set position of axis
      cmd.str(""); cmd << "RON " << axis << " 1";         this->send_command( filter, cmd.str() );  // reference mode off

      // Now move to the position with the calculated "newid"
      //
      float newpos=NAN;
      for (const auto &[name, pos] : *posmap) {
        if (pos.posid == newid) {
          newpos = pos.position;   // newpos = (position for posid=newid)
          break;
        }
      }
      if ( ! std::isnan( newpos ) ) error = this->motors.at(filter).moveto( axis, newpos, retstring );
      else {
        message.str(""); message << "ERROR: no position found for filter ID " << newid;
        logwrite( function, message.str() );
        error=ERROR;
      }

      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR moving to ID=" << newid << " pos=" << newpos;
        logwrite( function, message.str() );
      }
      else {
        // Then map back to the correct order for the destination filter,
        // or to the current position if no position was found for newid,
        // which means we never moved.
        //
        remap = ( !std::isnan(newpos) ? destpos : currpos );

        cmd.str(""); cmd << "RON " << axis << " 0";         this->send_command( filter, cmd.str() );  // reference mode on
        cmd.str(""); cmd << "POS " << axis << " " << remap; this->send_command( filter, cmd.str() );  // set position of axis
        cmd.str(""); cmd << "RON " << axis << " 1";         this->send_command( filter, cmd.str() );  // reference mode off
      }
    }

    // Whether or not a filter was supplied, read the current position now.
    //
    error |= this->get_current_filter( retstring );

    // save the name to the class
    //
    this->current_filtername = retstring;

    return error;
  }
  /***** Acam::MotionInterface::filter ****************************************/


  /***** Acam::MotionInterface::get_current_filter ****************************/
  /**
   * @brief      get the current filter position name
   * @param[out] currname  return string contains the current position name
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long MotionInterface::get_current_filter( std::string &currname ) {
    std::string function = "Acam::MotionInterface::currernt_filter";
    int currid;
    float currpos;
    long error = this->get_current_filter( currname, currid, currpos );
    return( error );
  }
  /***** Acam::MotionInterface::get_current_filter ****************************/


  /***** Acam::MotionInterface::get_current_filter ****************************/
  /**
   * @brief      get the current filter position name by reading the controller
   * @param[out] currname  contains the current position name
   * @param[out] currid    contains the current position id
   * @param[out] currpos   contains the current position position
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long MotionInterface::get_current_filter( std::string &currname, int &currid, float &currpos ) {
    const std::string function("Acam::MotionInterface::currernt_filter");
    std::string filter = "filter";
    long error = NO_ERROR;
    float tolerance = 2;

    // get the position for the requested channel
    //
    std::string posstring;
    int axis=1;
    currpos=NAN;
    std::string posname;

    error = this->motors.at(filter).get_pos( axis, currpos, posname, tolerance );

    // save the filter name to the class
    //
    this->current_filtername = posname;

    // return the posname as the currname
    //
    currname = posname;

    // return the id of this position
    //
    const auto* posinfo = this->motors.at(filter).posinfo(posname);
    if (posinfo==nullptr) {
      logwrite(function, "ERROR position '"+posname+"' not found in configuration");
      return ERROR;
    }
    currid = posinfo->posid;

    // log the position and name
    //
    std::ostringstream message;
    message << std::fixed << std::setprecision(3) << currpos;
    if ( ! posname.empty() ) { message << " (" << posname << ")"; }
    logwrite( function, message.str() );

    return error;
  }
  /***** Acam::MotionInterface::get_current_filter ****************************/


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

    // If posname is supplied then move cover to that position.
    //
    if ( !posname.empty() ) {

      // Check that posname is found in the motormap.posmap map
      //
      const auto* posinfo = this->motors.at(cover).posinfo(posname);

      if (posinfo==nullptr) {
        logwrite(function, "ERROR position '"+posname+"' not found in configuration");
        retstring="not_found";
        return ERROR;
      }

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dest=" << posinfo->posname << " "
                               << "posid="        << posinfo->posid << " "
                               << "position="     << posinfo->position;
      logwrite( function, message.str() );
#endif

      this->motors.at(cover).moveto( axis, posinfo->position, retstring );

    }

    // get the position name
    //
    error = this->read_cover( posname );

    // form the return value
    //
    message.str(""); message << posname << " "
                             << std::fixed << std::setprecision(3)
                             << this->current_coverpos;
    if ( ! posname.empty() ) { message << " (" << posname << ")"; }
    logwrite( function, message.str() );

    retstring=message.str();

    return error;
  }
  /***** Acam::MotionInterface::cover *****************************************/


  /***** Acam::MotionInterface::read_cover ************************************/
  /**
   * @brief      read name of cover position
   * @param[out] retstring  return string contains the current position name
   * @return     ERROR | NO_ERROR
   *
   */
  long MotionInterface::read_cover( std::string &retstring ) {
    std::string function = "Acam::MotionInterface::read_cover";
    std::string cover = "cover";

    // get the position
    //
    auto addr=this->motors.at(cover).get_addr();
    float position=NAN;
    int axis = 1;
    float tolerance = 2;

    long error = this->motors.at(cover).get_pos( axis, addr, position, retstring, tolerance );

    this->current_coverpos = retstring;

    return error;
  }
  /***** Acam::MotionInterface::read_cover ************************************/
}
