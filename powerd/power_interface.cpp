/**
 * @file    power_interface.cpp
 * @brief   
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#include "power_interface.h"

namespace Power {

  /**************** Power::NpsInfo::NpsInfo ********8**************************/
  /**
   * @fn         NpsInfo
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  NpsInfo::NpsInfo() {
    this->npsnum=-1;
    this->maxplugs=-1;
    this->host="";
    this->port=-1;
  }
  /**************** Power::NpsInfo::NpsInfo ********8**************************/


  /**************** Power::NpsInfo::~NpsInfo *******8**************************/
  /**
   * @fn         ~NpsInfo
   * @brief      class de-constructor
   * @param[in]  none
   * @return     none
   *
   */
  NpsInfo::~NpsInfo() {
  }
  /**************** Power::NpsInfo::~NpsInfo *******8**************************/


  /**************** Power::Interface::Interface *******************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
    this->class_initialized = false;   /// set true by the initialize_class() function
  }
  /**************** Power::Interface::Interface *******************************/


  /**************** Power::Interface::~Interface ******************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Power::Interface::~Interface ******************************/


  /**************** Power::Interface::configure_interface *********************/
  /**
   * @fn         configure_interface
   * @brief      configure the NPS interface with info from configuration file
   * @param[in]  nps, NpsInfo class
   * @return     none
   *
   * The NpsInfo object passed in here is a temporary object created while
   * reading the configuration file. This function is called only after checking
   * the validity of the info in that class object, which has already been
   * pushed into the nps_info vector.
   *
   * This function will create an interface to the NPS identified in the nps
   * object, and push that into the nps vector.
   *
   */
  void Interface::configure_interface( Power::NpsInfo npsinfo ) {
    std::string function = "Power::Interface::configure_interface";
    std::stringstream message;

    // The nps name is "nps#" where # is npsnum
    //
    std::stringstream name;
    name << "nps" << npsinfo.npsnum;

    // Create a temporary WTI::Interface object using this host and port
    //
    WTI::Interface iface( name.str(), npsinfo.host, npsinfo.port );

    // Create a temporary WTI::NPS object with this interface
    //
    WTI::NPS wti_nps;
    wti_nps.interface = iface;

    // Insert this NPS object into the STL map of WTI::NPS Objects
    //
    this->nps.insert( { npsinfo.npsnum, wti_nps } );

    return;
  }
  /**************** Power::Interface::configure_interface *********************/


  /**************** Power::Interface::initialize_class ************************/
  /**
   * @fn         initialize
   * @brief      initialize class variables
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   * This function is called by the configure_powerd() function, after the 
   * configuration file has been read.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Power::Interface::initialize_class";
    std::stringstream message;
    long error = NO_ERROR;

    this->numdev = this->nps_info.size();   /// this is the number of configured NPS units

    if ( this->numdev > 0 ) {
      this->class_initialized = true;
    }
    else {
      logwrite( function, "ERROR no configured NPS units" );
      this->class_initialized = false;
      error = ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    for ( auto npsinfo_it = this->nps_info.begin(); npsinfo_it != this->nps_info.end(); ++npsinfo_it ) {
      message.str(""); message << "[DEBUG] initialized NPS unit " << npsinfo_it->first << " with " << npsinfo_it->second.maxplugs
                               << " outlets @ " << npsinfo_it->second.host << ":" << npsinfo_it->second.port;
      logwrite( function, message.str() );
    }
#endif

    return( error );
  }
  /**************** Power::Interface::initialize_class ************************/


  /**************** Power::Interface::open ************************************/
  /**
   * @fn         open
   * @brief      open the NPS socket connection
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   * This function will attempt to open sockets to the NPS unit(s). If
   * any one unit returns an error, the function will still attempt to
   * open any remaining units, but will return an error. Only if all
   * available units are opened will NO_ERROR be returned.
   *
   */
  long Interface::open() {
    std::string function = "Power::Interface::open";
    std::stringstream message;
    long error = NO_ERROR;   // this can only be set to error, never cleared in this function

    // Iterate over all NPS units
    //
    for ( auto nps_it = this->nps.begin(); nps_it != this->nps.end(); ++nps_it ) {

      // First make sure that the class object was initialized properly
      // (pretty much has to be)
      //
      if ( ! nps_it->second.interface.is_initialized() ) {
        message.str(""); message << "ERROR: " << nps_it->second.interface.get_name() << " class was not initialized";
        logwrite( function, message.str() );
        error = ERROR;
        continue;
      }

      // Try to open the nps hardware interface
      //
      if ( nps_it->second.interface.open() != NO_ERROR ) {
        message.str(""); message << "ERROR opening connection to " << nps_it->second.interface.get_name();
        logwrite( function, message.str() );
        error = ERROR;
        continue;
      }
      else {
        message.str(""); message << "opened connection to " << nps_it->second.interface.get_name();
        logwrite( function, message.str() );
      }
    }

    return( error );
  }
  /**************** Power::Interface::open ************************************/


  /**************** Power::Interface::close ***********************************/
  /**
   * @fn         close
   * @brief      closes the NPS socket connection
   * @param[in]  none
   * @return     ERROR or NO_ERROR
   *
   * This function will attempt to close all NPS socket connections. If
   * any one NPS returns an error, the function will still attempt to
   * close any remaining NPS units.
   *
   * An error will be returned if the connections are already closed.
   *
   */
  long Interface::close( ) {
    std::string function = "Power::Interface::close";
    std::stringstream message;
    long error = NO_ERROR;   // this can only be set to error, never cleared in this function

    // Iterate over all NPS units
    //
    for ( auto nps_it = this->nps.begin(); nps_it != this->nps.end(); ++nps_it ) {

      // First make sure that the class object was initialized properly
      // (pretty much has to be)
      //
      if ( ! nps_it->second.interface.is_initialized() ) {
        message.str(""); message << "ERROR: " << nps_it->second.interface.get_name() << " class was not initialized";
        logwrite( function, message.str() );
        error = ERROR;
        continue;
      }

      // Is the connection even open?
      //
      if ( ! nps_it->second.isconnected() ) {
        message.str(""); message << "ERROR: " << nps_it->second.interface.get_name() << " is not open";
        logwrite( function, message.str() );
        error = ERROR;
        continue;
      }

      // Try to close the nps hardware interface
      //
      if ( nps_it->second.interface.close() != NO_ERROR ) {
        message.str(""); message << "ERROR closing connection to " << nps_it->second.interface.get_name();
        logwrite( function, message.str() );
        error = ERROR;
        continue;
      }
      else {
        message.str(""); message << "closed connection to " << nps_it->second.interface.get_name();
        logwrite( function, message.str() );
      }
    }

    return( error );
  }
  /**************** Power::Interface::close ***********************************/


  /**************** Power::Interface::isopen **********************************/
  /**
   * @fn         isopen
   * @brief      is the NPS socket connection open?
   * @param[in]  none
   * @return     true or false
   *
   */
  bool Interface::isopen() {
    std::string function = "Power::Interface::isopen";
    std::stringstream message;

    for ( auto it = this->nps.begin(); it != this->nps.end(); ++it ) {
      if ( ! it->second.isconnected() ) {
        message.str(""); message << it->second.interface.get_name() << " not open";
        logwrite( function, message.str() );
        return( false );
      }
    }

    return( true );
  }
  /**************** Power::Interface::isopen **********************************/


  /***** Power::Interface::list ***********************************************/
  /**
   * @brief      list plug devices
   * @param[out] retstring  reference to string to contain the list of plug devices
   *
   */
  void Interface::list( std::string &retstring ) {
    std::string function = "Power::Interface::list";
    std::stringstream message;

    message << "u p plugname\n";

    for ( auto it = this->plugmap.begin(); it != this->plugmap.end(); ++it ) {
      message << it->second.npsnum << " " << it->second.plugnum << " " << it->first << "\n";
    }

    retstring = message.str();

    return;
  }
  /***** Power::Interface::list ***********************************************/


  /***** Power::Interface::command ********************************************/
  /**
   * @brief      parse and form a command to send to the NPS unit
   * @param[in]  cmd, string
   * @param[in]  retstring, string
   * @return     ERROR or NO_ERROR
   *
   * Expected strings are:
   *   "<plugname>"
   *   "<plugname> ON|OFF"
   *   "<unit#> <plug#>"
   *   "<unit#> <plug#> ON|OFF"
   *
   * This function checks the above pemutations and for all cases ultimately
   * extracts the unit# and plug#, then reads or commands that plug on that unit.
   *
   */
  long Interface::command( std::string cmd, std::string &retstring ) {
    std::string function = "Power::Interface::command";
    std::stringstream message;

    std::string name="";        /// name of plug
    int unit=-1;                /// NPS unit number
    int plug=-1;                /// NPS plug number on unit
    int command=-1;             /// command value is {-1,0,1} for {read,off,on}
    std::stringstream cmdstr;   /// command string to send to NPS unit

    std::vector<std::string> tokens;

    Tokenize( cmd, tokens, " " );

    size_t nargs = tokens.size();

    if ( nargs < 1 ) {                            // should be impossible
      logwrite( function, "ERROR no arguments received" );
      return( ERROR );
    }

    // A single token is interpreted as "<plugname>" to read plug state
    //
    if ( nargs == 1 ) {
      auto loc_plugname = this->plugmap.find( tokens.at(0) );
      if ( loc_plugname == this->plugmap.end() ) {
        message.str(""); message << "ERROR plug name " << tokens.at(0) << " not found";
        logwrite( function, message.str() );
        return( ERROR );
      }
      unit = loc_plugname->second.npsnum;
      plug = loc_plugname->second.plugnum;
      name = loc_plugname->first;
    }
    else

    // Two tokens can either be
    //   "<plugname> <command>" to turn plugname ON|OFF
    // or
    //   "<unit#> <plug#>"      to read plug state
    //
    if ( nargs == 2 ) {

      // check the 2nd token for a command
      //
      if ( tokens.at(1) == "OFF" ) command =  0;  // turn off plug
      else
      if ( tokens.at(1) == "ON"  ) command =  1;  // turn on plug
      else
                                   command = -1;  // read plug state

      // request is to read state of <unit#> <plug#>
      //
      if ( command == -1 ) {
        try {
          unit = std::stoi( tokens.at(0) );
          plug = std::stoi( tokens.at(1) );
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR converting unit \"" << tokens.at(0) << "\" or plug \"" << tokens.at(1) 
                                   << "\" to integer. Expected <unit#> <plug#>";
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch( std::out_of_range & ) {
          message.str(""); message << "ERROR out of range converting tokens in \"" << cmd << "\"";
          logwrite( function, message.str() );
          return( ERROR );
        }

        // find this npsnum in the nps_info map
        //
        auto loc_unit = this->nps_info.find( unit );

        if ( loc_unit == this->nps_info.end() ) {
          message.str(""); message << "ERROR requested nps unit " << unit << " not found in configuration";
          logwrite( function, message.str() );
          return( ERROR );
        }

        if ( plug < 1 || plug > loc_unit->second.maxplugs ) {
          message.str(""); message << "ERROR requested plug " << plug << " outside range {1:" << loc_unit->second.maxplugs << "}";
          logwrite( function, message.str() );
          return( ERROR );
        }
      }
      else {  // otherwise the request is "<plugname> <command>" so get the plug info
        auto loc_plugname = this->plugmap.find( tokens.at(0) );
        if ( loc_plugname == this->plugmap.end() ) {
          message.str(""); message << "ERROR plug name " << tokens.at(0) << " not found";
          logwrite( function, message.str() );
          return( ERROR );
        }
        unit = loc_plugname->second.npsnum;
        plug = loc_plugname->second.plugnum;
        name = loc_plugname->first;
      }
    }
    else

    // Three tokens is interpreted as "<unit#> <plug#> <command>" to set plug on|off
    //
    if ( nargs == 3 ) {
      try {
        unit = std::stoi( tokens.at(0) );
        plug = std::stoi( tokens.at(1) );
      }
      catch ( std::invalid_argument &e ) {
        message.str(""); message << "ERROR converting unit \"" << tokens.at(0) << "\" or plug \"" << tokens.at(1) 
                                 << "\" to integer. Expected <unit#> <plug#> <on|off>";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch( std::out_of_range & ) {
        message.str(""); message << "ERROR out of range converting tokens in \"" << cmd << "\"";
        logwrite( function, message.str() );
        return( ERROR );
      }
      if ( tokens.at(2) == "OFF" ) command =  0;  // turn off plug
      else
      if ( tokens.at(2) == "ON"  ) command =  1;  // turn on plug
      else {
        message.str(""); message << "ERROR unrecognized command " << tokens.at(2);
        logwrite( function, message.str() );
        return( ERROR );
      }

      // find the name associated with unit/plug
      //
      for ( auto plug_it = this->plugmap.begin(); plug_it != this->plugmap.end(); ++plug_it ) {
        if ( plug_it->second.npsnum == unit && plug_it->second.plugnum == plug ) {
          name = plug_it->first;
          break;
        }
      }
    }
    else

    // Any other nargs is invalid
    //
    {
      logwrite( function, "ERROR bad number of arguments" );
      return( ERROR );
    }

    // Final check of plug and unit numbers
    //
    auto loc_unit = this->nps_info.find( unit );

    if ( loc_unit == this->nps_info.end() ) {
      message.str(""); message << "ERROR requested nps unit " << unit << " not found in configuration";
      logwrite( function, message.str() );
      return( ERROR );
    }

    if ( plug < 1 || plug > loc_unit->second.maxplugs ) {
      message.str(""); message << "ERROR requested plug " << plug << " outside range {1:" << loc_unit->second.maxplugs << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Must be connected to this NPS unit
    //
    if ( ! this->nps.at(unit).isconnected() ) {
      message.str(""); message << "ERROR not connected to nps" << unit;
      logwrite( function, message.str() );
      return( ERROR );
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] command=" << command << ( command==-1 ? " (get) " : " (set) " )
                             << "unit=" << unit << " plug=" << plug;
    logwrite( function, message.str() );
#endif

    // Send the command
    //
    long error = NO_ERROR;
    switch( command ) {
      case -1: error = this->nps.at(unit).get_switch( plug, retstring );
               break;
      case  0:
      case  1: error = this->nps.at(unit).set_switch( plug, command );
               message.str(""); message << ( error != NO_ERROR ? "ERROR setting " : "set " )
                                        << ( name.empty() ? "empty" : name ) << ( command == 0 ? " OFF" : " ON" );
               logwrite( function, message.str() );
               break;
      default: message.str(""); message << "ERROR bad command " << command;                   // should be impossible
               logwrite( function, message.str() );
               return( ERROR );
               break;
    }

    return( error );
  }
  /***** Power::Interface::command ********************************************/

}
