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


  /***** Thermal::Interface::open_campbell ***********************************/
  /**
   * @brief      open connection to Cambpbell CR1000
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::open_campbell() {
    return this->campbell.open_device();
  }
  /***** Thermal::Interface::open_campbell ***********************************/


  /***** Thermal::Interface::close_campbell **********************************/
  /**
   * @brief      close connection to Cambpbell CR1000
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::close_campbell() {
    return this->campbell.close_device();
  }
  /***** Thermal::Interface::close_campbell **********************************/


  /***** Thermal::Interface::open_lakeshores *********************************/
  /**
   * @brief      open connection to all configured Lakeshore devices
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open_lakeshores() {
    std::string function = "Thermal::Interface::open_lakeshores";
    std::stringstream message;
    long error=NO_ERROR;

    // Iterate through the this->lakeshore map.
    // Use an iterator (instead of range-based loop) so that the map can be modified.
    //
    for ( auto iter = this->lakeshore.begin(); iter != this->lakeshore.end(); ) {

      // get a reference to the original Thermal::Lakeshore object stored in the map
      //
      Thermal::Lakeshore &my_lakeshore = iter->second;

      // Try to open my_lakeshore
      //
      if ( my_lakeshore.lks->open() == ERROR ) {
        message.str(""); message << "ERROR: opening Lakeshore " << my_lakeshore.lks->get_name()
                                 << " and will not be used.";
        logwrite( function, message.str() );

        // Remove unresponsive device from the this->lakeshore map.
        // erase will return an iterator to the next element.
        //
        iter = this->lakeshore.erase( iter );
        error |= ERROR;
      }
      else ++iter;  // Lakehore open successful, so increment the iterator
    }

    return( error );
  }
  /***** Thermal::Interface::open_lakeshores *********************************/


  /***** Thermal::Interface::close_lakeshores ********************************/
  /**
   * @brief      close connection to all configured Lakeshore devices
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close_lakeshores() {
    std::string function = "Thermal::Interface::close_lakeshores";
    std::stringstream message;
    for ( const auto &lakeshore : this->lakeshore ) {
      if ( lakeshore.second.lks->close() == ERROR ) {
        message.str(""); message << "ERROR: closing Lakeshore " << lakeshore.second.lks->get_name();
        logwrite( function, message.str() );
        return( ERROR );
      }
    }
    return( NO_ERROR );
  }
  /***** Thermal::Interface::close_lakeshores ********************************/


  /***** Thermal::Interface::reconnect ***************************************/
  /**
   * @brief      close, then open all hardware devices
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::reconnect( std::string args, std::string &retstring ) {
    long error;
    error = this->close_lakeshores();
    if ( error==NO_ERROR ) error = this->open_lakeshores();
    error = this->close_campbell();
    if ( error==NO_ERROR ) error = this->open_campbell();
    return( error );
  }
  /***** Thermal::Interface::reconnect ***************************************/


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

      auto lks_it = this->thermal_info.find( tokens[0] );   // Is the arg a known label?

      if ( lks_it != this->thermal_info.end() ) {   // yes, label found
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
        if ( this->lakeshore[unit].temp_info.find( trychan ) == this->lakeshore[unit].temp_info.end() ) {
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
      retstream << THERMALD_GET << " <label> | <unit> <chan> | camp\n"  // usage
                << "  Returns all Cambpell CR1000 readings if arg is \"camp\" or\n"
                << "  returns temperature of channel specified by <label> or <unit> <chan> using...\n";

      for ( const auto &lakeshore_it : this->lakeshore ) {          // loop through all lakeshores
        retstream << "\n";
        retstream << "    unit: ";
        retstream << lakeshore_it.first << " (" << lakeshore_it.second.lks->get_name() << ")\n";  // unit and name
        retstream << "   chans: ";

        for ( const auto &temp : lakeshore_it.second.temp_info ) {  // loop through all tempchans for this lakeshore
          retstream << temp.first << " ";                           // temp chan
        }
        retstream << "\n";
        retstream << "  labels: ";

        for ( const auto &temp : lakeshore_it.second.temp_info ) {  // loop through all tempchans for this lakeshore
          retstream << temp.second << " ";                          // temp label
        }
      }
      retstream << "\n";
      retstring = retstream.str();
      return( NO_ERROR );
    }

    if ( args == "camp" ) {
      this->campbell.read_data();
      return NO_ERROR;
    }

    int unit;
    std::string chan;
    float tempval;

    // Get the unit and chan from the input args. This will parse and validate the inputs.
    //
    error = this->parse_unit_chan( args, unit, chan );

    // read and return the value
    //
    if ( error==NO_ERROR ) error = this->lakeshore[unit].read_temp( chan, tempval );
    if ( error==NO_ERROR ) retstring = std::to_string( tempval );

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
      for ( const auto &lks_it : this->lakeshore ) {             // loop through all lakeshores
        retstream << lks_it.first << " (" << lks_it.second.lks->get_name() << ")\n";  // unit and name
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

    // read/write setpoint as needed
    //
    if ( error==NO_ERROR && set )   error = this->lakeshore[unit].set_setpoint( output, setval );
    if ( error==NO_ERROR && get ) { error = this->lakeshore[unit].get_setpoint( output, setval );
                                    retstream << setval; retstring=retstream.str();
                                  }

    return( error );
  }
  /***** Thermal::Interface::setpoint *****************************************/


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
      for ( const auto &lks_it : this->lakeshore ) {   // loop through all lakeshores
        retstream << lks_it.first;
        retstream << " (" << lks_it.second.lks->get_name() << ")\n";
        retstream << "                ";
      }
      retstream << "\n";
      retstream << "  No parsing is done except to extract the <unit>. The string following <unit>\n";
      retstream << "  is sent directly to the designated Lakeshore\n";
      retstring = retstream.str();
      return( NO_ERROR );
    }

    // send the command
    //
    if ( error==NO_ERROR ) error = this->lakeshore[unit].lks->send_command( cmd, retstring );

    return error;
  }
  /***** Thermal::Interface::native *******************************************/


  /***** Thermal::Interface::lakeshore_readall ********************************/
  /**
   * @brief      read all channels of all Lakeshore devices
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::lakeshore_readall() {
    std::string function = "Thermal::Interface::lakeshore_readall";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    long error = NO_ERROR;

    this->lakeshoredata.clear();

    // lakeshoredata map stores double so store the time as a double,
    // which can be converted back to a datetime string later
    //
    //this->lakeshoredata["time"] = get_time_as_double();

    // Iterate through all configured Lakeshores
    //
    for ( auto lakeshore_it = this->lakeshore.begin(); lakeshore_it != this->lakeshore.end(); ++lakeshore_it ) {

      // Loop separately through the configured temperature, then heater channels for this Lakeshore.
      // The readings, if valid, will be stored in a master STL map indexed by label (which is the
      // database column name). This map will then be passed to the Telemetry object for recording
      // into the telemetry database.
      //

      // Loop through all configured temperature channels for this Lakeshore.
      //
      for ( const auto &temp : lakeshore_it->second.temp_info ) {
        float fvalue = NAN;
        if ( lakeshore_it->second.read_temp( temp.first, fvalue ) == ERROR ) {
          message.str(""); message << "ERROR reading from Lakeshore " 
                                   << lakeshore_it->second.device_name() << ": will be skipped";
          logwrite( function, message.str() );
          error = ERROR;
          break;
        }
        // If this reading is not NaN then save it in the lakeshoredata map,
        // which is indexed by label, converting to string on insertion.
        //
        if ( ! std::isnan( fvalue ) ) { this->lakeshoredata[ temp.second ] = fvalue; }
      }

      // Loop through all configured heater channels for this Lakeshore.
      //
      for ( const auto &heat : lakeshore_it->second.heat_info ) {
        float fvalue = NAN;
        if ( lakeshore_it->second.read_heat( heat.first, fvalue ) == ERROR ) {
          message.str(""); message << "ERROR reading from Lakeshore " 
                                   << lakeshore_it->second.device_name() << ": will be skipped";
          logwrite( function, message.str() );
          error = ERROR;
          break;
        }
        // If this reading is not NaN then save it in the lakeshoredata map,
        // which is indexed by label, converting to string on insertion.
        //
        if ( ! std::isnan( fvalue ) ) { this->lakeshoredata[ heat.second ] = fvalue; }
      }

    }

    if ( error != NO_ERROR ) logwrite( function, "ERROR: reading one or more devices" );

    return( error );
  }
  /***** Thermal::Interface::lakeshore_readall ********************************/


  /***** Thermal::Lakeshore::read_temp ****************************************/
  /**
   * @brief      read the specified Lakeshore temperature channel into memory
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
    error = this->lks->send_command( cmd.str(), reply );

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
   * @brief      read the specified Lakeshore heater channel into memory
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
    error = this->lks->send_command( cmd.str(), reply );

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

    // format and send the command to change the setpoint
    //
    cmd << "SETP " << output << ", " << setpoint;

    return this->lks->send_command( cmd.str() );
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

    error = this->lks->send_command( cmd.str(), reply );

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
  /***** Thermal::Lakeshore::get_setpoint *************************************/


  /***** Thermal::Campbell::open_device ***************************************/
  /*
   * @brief      open and initialize the serial port for the CR1000
   * @details    The Campbell CR1000 is connected to a USB port using a
   *             USB-RS232 serial converter. This requires a proper udev rule
   *             to assign the appropriate USB device to /dev/cr1000.
   * @return     ERROR or NO_ERROR
   *
   */
  long Campbell::open_device()  {
    std::string function = "Thermal::Campbell::open_device";
    std::stringstream message;

    // don't re-open here
    //
    if ( this->fd>=0 ) {
      logwrite( function, "already open" );
      return NO_ERROR;
    }

    // open the USB device
    //
    this->fd = open( this->device.c_str(), O_RDWR | O_NOCTTY );

    if ( this->fd<0 ) {
      message << "ERROR: failed to open device " << this->device << ": " << strerror(errno);
      logwrite( function, message.str() );
      return ERROR;
    }

    // Configure the serial port
    //
    struct termios tty;
    memset( &tty, 0, sizeof(tty) );

    if ( tcgetattr( this->fd, &tty ) != 0 ) {
      message << "ERROR getting attributes for serial port: " << strerror(errno);
      logwrite( function, message.str() );
      close( this->fd );
      this->fd = -1;
      return ERROR;
    }

    // set baud rate to 19200
    //
    cfsetispeed( &tty, B115200 );
    cfsetospeed( &tty, B115200 );

    // 8N1
    //
    tty.c_cflag = ( tty.c_cflag & ~CSIZE ) | CS8;  // 8 data bits
    tty.c_cflag &= ~PARENB;                        // no parity
    tty.c_cflag &= ~CSTOPB;                        // 1 stop
    tty.c_cflag &= ~CRTSCTS;                       // no flow control

    // enable reading and ignore control line
    //
    tty.c_cflag |= ( CLOCAL | CREAD );

    // set raw i/o mode
    //
    tty.c_iflag &= ~( IXON | IXOFF | IXANY );          // disable software flow control
    tty.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG );  // raw input mode
    tty.c_oflag &= ~OPOST;                             // raw output mode

    // set timeout and min bytes
    //
    tty.c_cc[VMIN]  = 0;    // min number of chars for read
    tty.c_cc[VTIME] = 30;   // 3s timeout (in deciseconds)

    // apply the configuration
    //
    if ( tcsetattr( this->fd, TCSANOW, &tty ) != 0 ) {
      message << "ERROR setting attributes for serial port: " << strerror(errno);
      logwrite( function, message.str() );
      close( this->fd );
      this->fd = -1;
      return ERROR;
    }

    logwrite( function, "opened serial connection to CR1000" );

    return NO_ERROR;
  }
  /***** Thermal::Campbell::open_device ***************************************/


  /***** Thermal::Campbell::close_device **************************************/
  /*
   * @brief      close serial port for the CR1000
   * @return     NO_ERROR
   *
   */
  long Campbell::close_device()  {
    if ( this->fd >= 0 ) {
      close( this->fd );
      this->fd = -1;
    }
    return NO_ERROR;
  }
  /***** Thermal::Campbell::close_device **************************************/


  /***** Thermal::Campbell::read_data *****************************************/
  /*
   * @brief      read data from CR1000
   * @details    The Campbell CR1000 is programmed to transmit all values in a
   *             CSV string terminated by newline on reception of any character.
   *             This sends a character to the serial port and reads back the
   *             response into map indexed by name. The names come from the
   *             configuration file.
   * @return     ERROR or NO_ERROR
   *
   */
  long Campbell::read_data()  {
    std::string function = "Thermal::Campbell::read_data";
    std::stringstream message;

    // can't continue if fd not set, which means port not open
    //
    if ( this->fd < 0 ) {
      logwrite( function, "ERROR device not open" );
      return ERROR;
    }

    // send a newline character, which triggers the device to send data
    //
    write( this->fd, (const unsigned char*)"\n", 1 );

    // this will hold the complete data string returned
    //
    std::string replystring;

    // read the serial port one byte at a time to check for the
    // terminating newline
    //
    char buf;
    while (true) {
      ssize_t bytes_read = read(this->fd, &buf, 1);
      if (bytes_read < 0) {
        logwrite( function, "ERROR reading from serial port " );
        return ERROR;
      }
      if (bytes_read == 0) {  // Timeout reached
        logwrite( function, "ERROR Read timeout occurred" );
        return ERROR;
      }
      // build up the replystring with the byte just read,
      // until newline, which signals the end of the stream
      //
      replystring += buf;
      if (buf == '\n') {
        break;
      }
    }

    // The cr1000 returns a comma-separated-variable string (CSV).
    // Tokenize on the command and put each value into the
    // class vector which holds the data.
    //
    std::vector<std::string> tokens;
    Tokenize( replystring, tokens, "," );

    // There needs to be a configured sensor name for each data value
    //
    if ( tokens.size() != this->sensor_names.size() ) {
      message.str(""); message << "ERROR sensor name mismatch: "
                               << this->sensor_names.size() << " sensor names configured "
                               << "and received " << tokens.size() << " data values ";
      logwrite( function, message.str() );
      return ERROR;
    }

    // If correct number of sensor values and names, convert the received
    // strings to numeric values and store in the class along with the
    // time they were read.
    //
    std::string tok;
    try {
      this->datamap.clear();  // erase the map

      // Loop through tokens, which is a vector of sensor readings in order
      // of channel number. datamap is a map of readings indexed by
      // sensor name.
      //
      for ( size_t i=0; i<tokens.size(); i++ ) {
        tok = tokens.at(i);
        int chan = static_cast<int>(i+1);            // sensor_names indexed by int channel
        std::string key = this->sensor_names[chan];  // map key is sensor name
        if ( key == "undef" ) continue;              // don't save the reading if label is "undef"
        double      val = std::stod(tok);            // map value is sensor reading
        this->datamap[key] = val;                    // save this reading to the map
      }
    }
    catch( const std::exception &e ) {
      this->datamap.clear();  // all or nothing, to prevent mismatch with names
      message.str(""); message << "ERROR parsing token \"" << tok << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Thermal::Campbell::read_data *************************************/

}
