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


  /***** Thermal::Interface::read_all *****************************************/
  /**
   * @brief      read all channels of all devices
   * @details    results are stored in the respective classes for each device
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::read_all( ) {
    std::string function = "Thermal::Interface::read_all";
    std::stringstream message;
    std::stringstream cmd;
    std::string reply;
    long error = NO_ERROR;

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

      std::string datetime = get_timestamp();
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
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::log_all( ) {
    std::string function = "Thermal::Interface::log_all";
    std::stringstream message, logstr;

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


}
