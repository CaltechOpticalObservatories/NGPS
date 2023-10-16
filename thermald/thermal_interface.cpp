/**
 * @file    thermal_interface.cpp
 * @brief   this contains the thermal interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Thermal namespace,
 * and is how the thermal daemon interfaces to the temperature control hardware.
 *
 */

#include "thermal_interface.h"

namespace Thermal {

  /***** Thermal::Interface::Interface ****************************************/
  /**
   * @brief      class constructor
   *
   */
  Interface::Interface() {
  }
  /***** Thermal::Interface::Interface ****************************************/


  /***** Thermal::Interface::~Interface ***************************************/
  /**
   * @brief      class deconstructor
   *
   */
  Interface::~Interface() {
  }
  /***** Thermal::Interface::~Interface ***************************************/


  /***** Thermal::Interface::initialize_class ********************************/
  /**
   * @brief      initializes the class from configure_thermald()
   * @return     ERROR or NO_ERROR
   *
   * This is called by Thermal::Server::configure_thermald() after reading the
   * configuration file to apply the config file setting.
   *
   */
  long Interface::initialize_class() {
    std::string function = "Thermal::Interface::initialize_class";
    std::stringstream message;

    logwrite( function, "initialized" );

    return( NO_ERROR );
  }
  /***** Thermal::Interface::initialize_class *********************************/


  /***** Thermal::Interface::parse_unit_chan **********************************/
  /**
   * @brief      parse arg string to extract unit and chan numbers
   * @param[in]  args  input arg string, see details below
   * @param[out] unit  extracted unit number
   * @param[out] chan  extracted chan number
   * @return     ERROR or NO_ERROR
   *
   * @details    Input args can be <label> or <unit> <chan>
   *             This function will figure that out and in either case
   *             return a validated <int> unit and <string> chan
   *
   */
  long Interface::parse_unit_chan( std::string args, int &unit, std::string &chan ) {
    std::string function = "Thermal::Interface::parse_unit_chan";
    std::stringstream message;
    long error = NO_ERROR;

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() == 1 ) {                     // <label>

      auto lks_it = this->info.find( tokens[0] );   // Is the arg a known label?

      if ( lks_it != this->info.end() ) {           // yes, label found
        unit = lks_it->second.unit;                 // here's the unit number
        chan = lks_it->second.chan;                 // here's the channel
      }
      else {                                        // no, label not found
        message.str(""); message << "ERROR: label \"" << tokens[0] << "\" not found. Check configuration.";
        logwrite( function, message.str() );
        error = ERROR;
      }
    }
    else
    if ( tokens.size() == 2 ) {                     // <unit> <chan>
      std::string trychan = tokens[1];
      try { unit = std::stoi( tokens[0] ); }
      catch (std::invalid_argument &) { logwrite(function, "ERROR: exception converting unit to integer"); return(ERROR); }
      catch (std::out_of_range &) { logwrite(function, "ERROR: unit out of integer range"); return(ERROR); }

      auto unit_it = this->lakeshore.find( unit );  // Is the unit in the Lakeshore map?

      if ( unit_it == this->lakeshore.end() ) {     // no, unit not found
        message.str(""); message << "ERROR: unit " << unit << " not found. Check configuration.";
        logwrite( function, message.str() );
        error = ERROR;
      }
      else {                                        // yes, unit found
                                                    // now find the channel in this unit's tempchans vector
        if ( std::find( this->lakeshore[unit].tempchans.begin(),
                        this->lakeshore[unit].tempchans.end(),
                        trychan
                      ) == this->lakeshore[unit].tempchans.end() ) {  // chan not found
          message.str(""); message << "ERROR: chan " << trychan << " not found. Check configuration.";
          logwrite( function, message.str() );
          error = ERROR;
        }
        else {                                      // chan found, we have it all!
          chan = trychan;
        }
      }
    }
    else {
      message.str(""); message << "ERROR: got " << tokens.size() << " args but expected 1 or 2";
      logwrite( function, message.str() );
      error = ERROR;
    }
    return( error );
  }
  /***** Thermal::Interface::parse_unit_chan **********************************/


  /***** Thermal::Interface::get **********************************************/
  /**
   * @brief      read the specified channel from the specified LKS unit
   * @param[in]  args       see details below
   * @param[out] retstring  string containing requested value, or help
   * @return     ERROR or NO_ERROR
   *
   * @details    usage:  <label> | <unit> <chan>
   *
   *             where <label> is a channel label
   *
   *             or    <unit> is the LKS UNIT number,
   *             and   <chan> is the channel number
   *
   * If no arg is provided then the known labels and unit/chans is returned.
   *
   */
  long Interface::get( std::string args, std::string &retstring ) {
    std::string function = "Thermal::Interface::get";
    std::stringstream message, retstream;
    long error=NO_ERROR;

    // "?" or no arg displays usage and possible inputs, then return
    //
    if ( args=="?" || args.empty() ) {
      retstream << THERMALD_GET << " <label> | <unit> <chan>\n"   // usage
                << "  Returns temperature of channel specified by <label> or <unit> <chan> using...\n";

      for ( auto lks_it : this->lakeshore ) {             // loop through all lakeshores
        retstream << "\n";
        retstream << "  known unit: ";
        retstream << lks_it.first << " (" << lks_it.second.lks.get_name() << ")\n";  // unit and name
        retstream << "       chans: ";

        for ( auto &chan : lks_it.second.tempchans ) {    // loop through all chans for this lakeshore
          retstream << chan << " ";                       // chan
        }
        retstream << "\n";
        retstream << "      labels: ";

        for ( auto &label : lks_it.second.templabels ) {  // loop through all labels for this lakeshore
          retstream << label << " ";                      // label
        }
      }
      retstream << "\n";
      retstring = retstream.str();
      return( NO_ERROR );
    }

    int unit;
    std::string chan;
    float tempval;

    // Get the unit and chan from the input args. This will parse and validate the inputs.
    //
    error = this->parse_unit_chan( args, unit, chan );

    // Open the designated Lakeshore, read that channel, close the connection.
    //
    if ( error==NO_ERROR ) error = this->lakeshore[unit].open();
    if ( error==NO_ERROR ) error = this->lakeshore[unit].read_temp( chan, tempval );
    if ( error==NO_ERROR ) retstring = std::to_string( tempval );
    if ( error==NO_ERROR ) error = this->lakeshore[unit].close();

    return( error );
  }
  /***** Thermal::Interface::get **********************************************/


  /***** Thermal::Interface::setpoint *****************************************/
  /**
   * @brief      set or get setpoint for specified output channel
   * @param[in]  args       see details below
   * @param[out] retstring  string containing requested value, or help
   * @return     ERROR or NO_ERROR
   *
   * @details    usage:  <unit> <output> [ <temp> ]
   *
   *             where <unit>   is the LKS UNIT number,
   *                   <output> is the output control loop number
   *                   <temp>   is the optional requested temperature setpoint.
   *                            If <temp> is omitted then return current setpoint.
   *
   */
  long Interface::setpoint( std::string args, std::string &retstring ) {
    std::string function = "Thermal::Interface::setpoint";
    std::stringstream message, retstream;
    long error=NO_ERROR;

    // "?" or no arg displays usage and possible inputs, then return
    //
    if ( args=="?" || args.empty() ) {
      retstream << THERMALD_SETPOINT << " <unit> <output> [ <temp> ]\n\n";  // usage

      retstream << "  known <unit>: ";
      for ( auto lks_it : this->lakeshore ) {             // loop through all lakeshores
        retstream << lks_it.first << " (" << lks_it.second.lks.get_name() << ")\n";  // unit and name
        retstream << "                ";
      }
      retstream << "\n";
      retstream << "  If <temp> is omitted then read and return the current setpoint.\n";
      retstring = retstream.str();
      return( NO_ERROR );
    }

    int unit, output;
    float setval;
    bool get=false, set=false;

    // Tokenize args
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    try {
      if ( tokens.size() == 2 ) {                 // <unit> <output>
        get=true; set=false;                      // get
        unit = std::stoi( tokens[0] );
        output = std::stoi( tokens[1] );
      }
      else
      if ( tokens.size() == 3 ) {                 // <unit> <output> <temp>
        set=true; get=false;                      // set
        unit = std::stoi( tokens[0] );
        output = std::stoi( tokens[1] );
        setval = std::stof( tokens[2] );
      }
      else {                                      // something wrong
        logwrite( function, "ERROR: incorrect number of args. Expected <unit> <output> [ <temp> ]" );
        retstring="bad_args";
        return( ERROR );
      }
    }
    catch (std::invalid_argument &) { logwrite(function, "ERROR: invalid argument exception"); return(ERROR); }
    catch (std::out_of_range &) { logwrite(function, "ERROR: out of range exception"); return(ERROR); }

    auto unit_it = this->lakeshore.find( unit );  // Is the unit in the Lakeshore map?
    if ( unit_it == this->lakeshore.end() ) {     // no, unit not found
      message.str(""); message << "ERROR: unit " << unit << " not found. Check configuration.";
      logwrite( function, message.str() );
      retstring="not_found";
      error=ERROR;
    }

    // Open the designated Lakeshore, read/write that setpoint as needed, close the connection.
    //
    if ( error==NO_ERROR )          error = this->lakeshore[unit].open();
    if ( error==NO_ERROR && set )   error = this->lakeshore[unit].set_setpoint( output, setval );
    if ( error==NO_ERROR && get ) { error = this->lakeshore[unit].get_setpoint( output, setval );
                                    retstream << setval; retstring=retstream.str();
                                  }
    if ( error==NO_ERROR )          error = this->lakeshore[unit].close();

    return( error );
  }
  /***** Thermal::Interface::get **********************************************/


  /***** Thermal::Interface::native *******************************************/
  /**
   * @brief      send native command to specified Lakeshore unit
   * @details    only query-based commands will generate a return string
   * @param[in]  cmd        should contain <unit> followed by Lakeshore-native command and args
   * @param[out] retstring  reference to string to contain any reply
   * @return     ERROR or NO_ERROR
   *
   * Other than parsing the <unit> number out of the cmd input string, this
   * function does no parsing and passes the arg string directly to the Lakeshore
   * specified by <unit> in the configuration file.
   *
   */
  long Interface::native( std::string cmd, std::string &retstring ) {
    std::string function = "Thermal::Interface::native";
    std::stringstream message, retstream;
    int unit;
    long error=NO_ERROR;
    std::vector<std::string> tokens;

    Tokenize( cmd, tokens, " " );                   // Tokenize the input only to get the <unit>

    if ( cmd != "?" && tokens.size() > 0 ) {        // need at least one token that's not a "?"
      try { unit = std::stoi( tokens[0] ); }
      catch (std::invalid_argument &) { logwrite(function, "ERROR: exception converting unit to integer"); return(ERROR); }
      catch (std::out_of_range &) { logwrite(function, "ERROR: unit out of integer range"); return(ERROR); }

      auto unit_it = this->lakeshore.find( unit );  // Is the unit in the Lakeshore map?

      if ( unit_it == this->lakeshore.end() ) {     // no, unit not found
        message.str(""); message << "ERROR: unit " << unit << " not found. Check configuration.";
        logwrite( function, message.str() );
        error = ERROR;
      }
      else {                                        // yes, unit found
        cmd = cmd.substr( cmd.find( tokens[0] ) );  // strip the unit from input cmd string
      }
    }
    else {                                             // Got a "?" or no tokens received, show Help/usage
      retstream << THERMALD_NATIVE;
      retstream << " <unit> <cmd> [ <args> ]\n";
      retstream << "  Sends <cmd> and optional <args> to Lakeshore designated by <unit>\n";
      retstream << "  where <unit>: ";
      for ( auto lks_it : this->lakeshore ) {          // loop through all lakeshores
        retstream << lks_it.first;
        retstream << " (" << lks_it.second.lks.get_name() << ")\n";
        retstream << "                ";
      }
      retstream << "\n";
      retstream << "  No parsing is done except to extract the <unit>. The string following <unit>\n";
      retstream << "  is sent directly to the designated Lakeshore\n";
      retstring = retstream.str();
      return( NO_ERROR );
    }

    if ( error==NO_ERROR ) error = this->lakeshore[unit].open();
    if ( error==NO_ERROR ) error = this->lakeshore[unit].lks.send_command( cmd, retstring );
    if ( error==NO_ERROR ) error = this->lakeshore[unit].close();

    return error;
  }
  /***** Thermal::Interface::native *******************************************/


  /***** Thermal::Interface::read_all *****************************************/
  /**
   * @brief      read all channels of all devices
   * @details    results are stored in the respective classes for each device
   * @param[in]  args       used only for asking for help
   * @param[out] retstring  used only for returning help
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::read_all( std::string args, std::string &retstring ) {
    std::string function = "Thermal::Interface::read_all";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    long error = NO_ERROR;

    if ( args == "?" ) {
      retstring = THERMALD_READALL;
      retstring.append( "\n" );
      retstring.append( "  Read all channels of all devices into memory.\n" );
      retstring.append( "  Pair with \""+THERMALD_LOGALL+"\" command to write the data.\n" );
      return( NO_ERROR );
    }

    std::string datetime = get_timestamp();

    // Iterate through all configured Lakeshores
    //
    for ( auto lks_it = this->lakeshore.begin(); lks_it != this->lakeshore.end(); ++lks_it ) {

      // open a connection to this Lakeshore
      //
      if ( lks_it->second.open() == ERROR ) {
        message.str(""); message << "ERROR: opening Lakeshore " << lks_it->second.device_name()
                                 << ": will be skipped";
        logwrite( function, message.str() );
        error = ERROR;
        continue;
      }

      lks_it->second.data.clear();
      lks_it->second.data.insert( { "datetime", datetime } );

#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] cleared data map for Lakeshore " << lks_it->second.device_name()
                               << " and inserted timestamp " << datetime;
      logwrite( function, message.str() );
#endif

      // loop through all configured temperature channels for this Lakeshore
      //
      for ( auto & tempchan : lks_it->second.tempchans ) {
        if ( lks_it->second.read_temp( tempchan ) == ERROR ) {
          message.str(""); message << "ERROR: reading Lakeshore " << lks_it->second.device_name()
                                   << ": will be skipped";
          logwrite( function, message.str() );
          error = ERROR;
          break;
        }
      }

      // loop through all configured heater channels for this Lakeshore
      //
      for ( auto & heater : lks_it->second.heaters ) {
        if( lks_it->second.read_heat( heater ) == ERROR ) {
          error = ERROR;
          break;
        }
      }

      // close the Lakeshore connection
      //
      lks_it->second.lks.close();
    }

    if ( error != NO_ERROR ) logwrite( function, "ERROR: reading one or more devices" );

    return( error );
  }
  /***** Thermal::Interface::read_all *****************************************/


  /***** Thermal::Interface::log_all ******************************************/
  /**
   * @brief      log all read data (in the class) to disk
   * @param[in]  args       used only for asking for help
   * @param[out] retstring  used only for returning help
   * @return     ERROR or NO_ERROR
   *
   * Data must have been read first, using read_all() function.
   *
   */
  long Interface::log_all( std::string args, std::string &retstring ) {
    std::string function = "Thermal::Interface::log_all";
    std::stringstream message, logstr;

    if ( args == "?" ) {
      retstring = THERMALD_LOGALL;
      retstring.append( "\n" );
      retstring.append( "  Log data in memory read using the \""+THERMALD_READALL+"\" command.\n" );
      return( NO_ERROR );
    }

    // Iterate through all configured Lakeshores
    //
    for ( auto lks_it = this->lakeshore.begin(); lks_it != this->lakeshore.end(); ++lks_it ) {

      // If the datetime key is missing then there must be a problem with the data.
      // 
      if ( lks_it->second.data.find("datetime") == lks_it->second.data.end() ) {
        message.str(""); message << "ERROR: missing datetime key from Lakeshore " << lks_it->second.device_name();
        logwrite( function, message.str() );
        continue;
      }

      logstr.str(""); logstr << lks_it->second.data.at("datetime");

      for ( auto & tempchan : lks_it->second.tempchans ) {
        if ( lks_it->second.data.find( tempchan ) == lks_it->second.data.end() ) {
          message.str(""); message << "ERROR: Lakeshore " << lks_it->second.device_name() << " missing channel " << tempchan;
          logwrite( function, message.str() );
          logstr << ", NaN";
        }
        else {
          logstr << ", " << lks_it->second.data.at( tempchan );
        }
      }

      for ( auto & heater : lks_it->second.heaters ) {
        heater.insert( 0, "H" );
        if ( lks_it->second.data.find( heater ) == lks_it->second.data.end() ) {
          message.str(""); message << "ERROR: Lakeshore " << lks_it->second.device_name() << " missing heater " << heater;
          logwrite( function, message.str() );
          logstr << ", NaN";
        }
        else {
          logstr << ", " << lks_it->second.data.at( heater );
        }
      }

      logwrite( function, logstr.str() );
    }

    return( NO_ERROR );
  }
  /***** Thermal::Interface::close ********************************************/


  /***** Thermal::Lakeshore::read_temp ****************************************/
  /**
   * @brief      read the specified Lakeshore temperature channel into the class
   * @details    This function should be used when reading all channels, for logging.
   * @param[in]  chan     Lakeshore channel name (e.g. "A, B, C1, etc.")
   * @return     ERROR or NO_ERROR
   *
   */
  long Lakeshore::read_temp( std::string chan ) {
    std::string function = "Thermal::Interface::read_temp";
    std::stringstream message;
    float tempval;
    std::string tempstr;

    long error = this->read_temp( chan, tempval );  // read the channel

    try {
      tempstr = ( std::isnan( tempval ) ? "NAN" : to_string_prec( tempval, 2 ) );
    }
    catch ( std::exception &ex ) {
      message.str(""); message << "ERROR: parsing value: " << ex.what();
      logwrite( function, message.str() );
    }

    this->data.insert( { chan, tempstr } );         // save it to the class

    return error;
  }
  /***** Thermal::Lakeshore::read_temp ****************************************/


  /***** Thermal::Lakeshore::read_temp ****************************************/
  /**
   * @brief      read the specified Lakeshore temperature channel into memory
   * @details    This function is called by read_temp( chan ) and it may also
   *             be called by a user, to get an immediate value, because it
   *             does not store the result in the class.
   * @param[in]  chan     Lakeshore channel name (e.g. "A, B, C1, etc.")
   * @param[out] tempval  reference to float variable to hold temperature
   * @return     ERROR or NO_ERROR
   *
   * On error, tempval will be NaN
   *
   */
  long Lakeshore::read_temp( std::string chan, float &tempval ) {
    std::string function = "Thermal::Lakeshore::read_temp";
    std::stringstream message, cmd;
    std::string reply;
    long error = NO_ERROR;

    tempval = NAN;  // set to NaN in case of an error

    // format and send the command to read the temperature
    //
    cmd << "KRDG? " << chan;
    error = this->lks.send_command( cmd.str(), reply );

    if ( error != NO_ERROR ) return error;

    // convert the reply to float
    //
    try {
      tempval = std::stof( reply );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR: " << this->device_name()
                               << " converting reply \"" << reply << "\" to float:" << e.what();
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR: " << this->device_name()
                               << " reply \"" << reply << "\" out of float range:" << e.what();
      return( ERROR );
    }

    return( error );
  }
  /***** Thermal::Lakeshore::read_temp ****************************************/


  /***** Thermal::Lakeshore::read_heat ****************************************/
  /**
   * @brief      read the specified Lakeshore heater channel into the class
   * @details    This function should be used when reading all channels, for logging.
   * @param[in]  chan     Lakeshore channel name
   * @return     ERROR or NO_ERROR
   *
   */
  long Lakeshore::read_heat( std::string chan ) {
    std::string function = "Thermal::Interface::read_heat";
    std::stringstream message;
    float heatval;
    std::string heatstr;

    long error = this->read_heat( chan, heatval );  // read the channel

    try {
      heatstr = ( std::isnan( heatval ) ? "NAN" : to_string_prec( heatval, 1 ) );
    }
    catch ( std::exception &ex ) {
      message.str(""); message << "ERROR: parsing value: " << ex.what();
      logwrite( function, message.str() );
    }

    this->data.insert( { "H"+chan, heatstr } );     // save it to the class

    return error;
  }
  /***** Thermal::Lakeshore::read_heat ****************************************/


  /***** Thermal::Lakeshore::read_heat ****************************************/
  /**
   * @brief      read the specified Lakeshore heater channel into memory
   * @details    This function is called by read_heat( chan ) and it may also
   *             be called by a user, to get an immediate value, because it
   *             does not store the result in the class.
   * @param[in]  chan     Lakeshore channel name
   * @param[out] heatval  reference to float variable to hold heater power
   * @return     ERROR or NO_ERROR
   *
   * On error, tempval will be NaN
   *
   */
  long Lakeshore::read_heat( std::string chan, float &heatval ) {
    std::string function = "Thermal::Lakeshore::read_heat";
    std::stringstream message, cmd;
    std::string reply;
    long error;

    heatval = NAN;  // set to NaN in case of an error

    // format and send the command to read the heater
    //
    cmd << "HTR? " << chan;
    error = this->lks.send_command( cmd.str(), reply );

    if ( error != NO_ERROR ) return error;

    // convert the reply to float
    //
    try {
      heatval = std::stof( reply );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR: converting reply \"" << reply << "\" to float:" << e.what();
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR: reply \"" << reply << "\" out of float range:" << e.what();
      return( ERROR );
    }

    return( error );
  }
  /***** Thermal::Lakeshore::read_heat ****************************************/


  /***** Thermal::Lakeshore::set_setpoint *************************************/
  /**
   * @brief      set the specified Lakeshore temperature setpoint
   * @param[in]  output    Lakeshore output control loop (1..4)
   * @param[in]  setpoint  float variable for new setpoint
   * @return     ERROR or NO_ERROR
   *
   */
  long Lakeshore::set_setpoint( int output, float setpoint ) {
    std::string function = "Thermal::Lakeshore::set_setpoint";
    std::stringstream message, cmd;
    std::string reply;
    long error = NO_ERROR;

    // format and send the command to change the setpoint
    //
    cmd << "SETP " << output << ", " << setpoint;

    return this->lks.send_command( cmd.str() );
  }
  /***** Thermal::Lakeshore::set_setpoint ************************************/


  /***** Thermal::Lakeshore::get_setpoint *************************************/
  /**
   * @brief      get the specified Lakeshore temperature setpoint
   * @param[in]  output    Lakeshore output control loop (1..4)
   * @param[in]  setpoint  reference to float variable for current setpoint
   * @return     ERROR or NO_ERROR
   *
   */
  long Lakeshore::get_setpoint( int output, float &setpoint ) {
    std::string function = "Thermal::Lakeshore::get_setpoint";
    std::stringstream message, cmd;
    std::string reply;
    long error = NO_ERROR;
    setpoint = NAN;

    // format and send the command to change the setpoint
    //
    cmd << "SETP? " << output;

    error = this->lks.send_command( cmd.str(), reply );

    if ( error != NO_ERROR ) return error;

    // convert the reply to float
    //
    try {
      setpoint = std::stof( reply );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR: " << this->device_name()
                               << " converting reply \"" << reply << "\" to float:" << e.what();
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR: " << this->device_name()
                               << " reply \"" << reply << "\" out of float range:" << e.what();
      return( ERROR );
    }
    return error;
  }
  /***** Thermal::Lakeshore::get_setpoint ************************************/


}
