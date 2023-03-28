/**
 * @file    tcs_interface.cpp
 * @brief   this contains the tcs interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the TCS namespace,
 * and is how the tcs daemon interfaces to the actual TCS hardware.
 *
 */

#include "tcs_interface.h"

namespace TCS {

  /***** TCS::Interface::Interface ********************************************/
  /**
   * @brief      class constructor
   *
   */
  Interface::Interface() {
  }
  /***** TCS::Interface::Interface ********************************************/


  /***** TCS::Interface::~Interface *******************************************/
  /**
   * @brief      class deconstructor
   *
   */
  Interface::~Interface() {
  }
  /***** TCS::Interface::~Interface *******************************************/


  /***** TCS::Interface::initialize_class *************************************/
  /**
   * @brief      initialize class variables
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::initialize_class() {
    std::string function = "TCS::Interface::initialize_class";
    std::stringstream message;

    if ( this->port < 0 || host.empty() ) {
      message.str(""); message << "ERROR: host \"" << this->host << "\" and/or port \"" << this->port << "\" invalid";
      logwrite( function, message.str() );
      return( ERROR );
    }

    Network::TcpSocket s( this->host, this->port );
    this->tcs = s;

    logwrite( function, "interface initialized ok" );
    return( NO_ERROR );
  }
  /***** TCS::Interface::initialize_class *************************************/


  /***** TCS::Interface::open *************************************************/
  /**
   * @brief      opens the TCS socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    std::string function = "TCS::Interface::open";
    std::stringstream message;

    if ( this->tcs.isconnected() ) {
      logwrite( function, "connection already open" );
      return( NO_ERROR );
    }

    // initialize connection to the TCS
    //
    logwrite( function, "opening connection to TCS" );

    if ( this->tcs.Connect() != 0 ) {
      message.str(""); message << "ERROR connecting to TCS on " << this->host << ":" << this->port << " ...";
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str(""); message << "socket connection to "
                             << this->tcs.gethost() << ":" << this->tcs.getport()
                             << " established on fd " << this->tcs.getfd();
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /***** TCS::Interface::open *************************************************/


  /***** TCS::Interface::close ************************************************/
  /**
   * @brief      closes the TCS socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    std::string function = "TCS::Interface::close";
    std::stringstream message;

    if ( !this->tcs.isconnected() ) {
      logwrite( function, "connection already closed" );
      return( NO_ERROR );
    }

    long error = this->tcs.Close();

    if ( error == NO_ERROR ) {
      logwrite( function, "connection to TCS closed" );
    }
    else {
      logwrite( function, "ERROR closing connection to TCS" );
    }
    return( error );
  }
  /***** TCS::Interface::close ************************************************/


  /***** TCS::Interface::get_weather_coords ***********************************/
  /**
   * @brief      get the current simulator coords
   * @details    uses the ?WEATHER command, pulls out just the RA and DEC
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_weather_coords( std::string &retstring ) {
    std::string function = "TCS::Interface::get_weather_coords";
    std::stringstream message;
    std::string weather;

    // Send the WEATHER command to the TCS. This returns a string of key=val pairs
    // with each pair separated by a newline character. The first two pairs are 
    // RA and DEC, which is all that is needed here.
    //
    if ( this->send_command( "?WEATHER", weather ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      return ERROR;
    }

    std::vector<std::string> pairs;
    Tokenize( weather, pairs, "\n" );  // tokenize on the newline to break into key=val pairs

    // If someone ever changes the TCS message then this will have to be changed.
    //
    if ( pairs.size() != 13 ) {
      message.str(""); message << "ERROR expected 13 values from the TCS but received " << pairs.size();
      logwrite( function, message.str() );
      return ERROR;
    }

    // Tokenize the first two key=val pairs on the "=" to get the ra, dec values
    //
    try {
      std::vector<std::string> ra_tokens, dec_tokens;

      Tokenize( pairs.at(0), ra_tokens, "=" );
      Tokenize( pairs.at(1), dec_tokens, "=" );

      message.str(""); message << ra_tokens.at(1) << " " << dec_tokens.at(1);
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "ERROR out of range parsing ra,dec from ?WEATHER command" );
      return ERROR;
    }

    logwrite( function, message.str() );

    retstring = message.str();

    return NO_ERROR;
  }
  /***** TCS::Interface::get_weather_coords ***********************************/


  /***** TCS::Interface::get_coords *******************************************/
  /**
   * @brief      get the current coords
   * @details    uses the REQPOS command, pulls out just the RA and DEC,
   *             returns them as "hh:mm:ss.ss dd:mm:ss.ss"
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_coords( std::string &retstring ) {
    std::string function = "TCS::Interface::get_coords";
    std::stringstream message;
/*****
    std::string weather;

    // Send the WEATHER command to the TCS. This returns a string of key=val pairs
    // with each pair separated by a newline character. The first two pairs are 
    // RA and DEC, which is all that is needed here.
    //
    if ( this->send_command( "?WEATHER", weather ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      return ERROR;
    }

    std::vector<std::string> pairs;
    Tokenize( weather, pairs, "\n" );  // tokenize on the newline to break into key=val pairs

    // If someone ever changes the TCS message then this will have to be changed.
    //
    if ( pairs.size() != 13 ) {
      message.str(""); message << "ERROR expected 13 values from the TCS but received " << pairs.size();
      logwrite( function, message.str() );
      return ERROR;
    }

    // Tokenize the first two key=val pairs on the "=" to get the ra, dec values
    //
    try {
      std::vector<std::string> ra_tokens, dec_tokens;

      Tokenize( pairs.at(0), ra_tokens, "=" );
      Tokenize( pairs.at(1), dec_tokens, "=" );

      message.str(""); message << ra_tokens.at(1) << " " << dec_tokens.at(1);
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "ERROR out of range parsing ra,dec from ?WEATHER command" );
      return ERROR;
    }

*****/

    // Send the REQPOS command to the TCS. This returns a string that looks like:
    //
    // posstr = "UTC = ddd hh:mm:ss, LST = hh:mm:ss\n
    //           RA = hh:mm:ss.ss, DEC = dd:mm:ss.s, HA=hh:mm:ss.s\n
    //           air mass = aa.aaa"
    //
    std::string posstr;
    if ( this->send_command( "REQPOS", posstr ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      return ERROR;
    }

    // First tokenize on newline
    //
    std::vector<std::string> lines;
    Tokenize( posstr, lines, "\n" );

    if ( lines.size() != 3 ) {
      message.str(""); message << "ERROR expected 3 lines from REQPOS string but received " << lines.size();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Then the 2nd token, lines[1], contains the RA,DEC so try to
    // tokenize that on the comma.
    //
    try {
      std::vector<std::string> radec;
      Tokenize( lines.at(1), radec, "," );  // lines[1] = "RA = hh:mm:ss.ss, DEC = dd:mm:ss.s, HA=hh:mm:ss.s\n"

      if ( radec.size() != 3 ) {
        message.str(""); message << "ERROR expected 3 tokens for ra, dec, ha but received " << radec.size();
        logwrite( function, message.str() );
        return( ERROR );
      }

      // Now try to tokenize the first two tokens of line1 on the equal sign to extract RA and DEC values
      //
      std::vector<std::string> ra, dec;
      Tokenize( radec.at(0), ra,  "=" );
      Tokenize( radec.at(1), dec, "=" );

      if ( ra.size() != 2 && dec.size() != 2 ) {
        message.str(""); message << "ERROR extracting ra,dec from " << lines.at(1);
        logwrite( function, message.str() );
        return( ERROR );
      }

      message.str(""); message << ra.at(1) << " " << dec.at(1);
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "ERROR out of range parsing ra,dec from ?WEATHER command" );
      return ERROR;
    }

    logwrite( function, message.str() );

    retstring = message.str();

    return NO_ERROR;
  }
  /***** TCS::Interface::get_coords *******************************************/


  /***** TCS::Interface::send_command *****************************************/
  /**
   * @brief      writes the raw command, as received, to the TCS
   * @param[in]  cmd    string to send to TCS
   * @param[out] reply  reference to string to contain reply
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &reply ) {
    std::string function = "TCS::Interface::send_command";
    std::stringstream message;
    long error=NO_ERROR;
    int ret;
    std::string sbuf;
    char delim       = '\r';   /// replies from the TCS are terminated with this
    std::string term = "\r";   /// all commands to TCS are terminated with this

    if ( !this->tcs.isconnected() ) {
      logwrite( function, "ERROR: no connection open to the TCS" );
      return( ERROR );
    }

#ifdef LOGLEVEL_DEBUG  // this can be a little much when polling
//  message.str(""); message << "[DEBUG] sending to TCS on fd " << this->tcs.getfd() << ": " << cmd;
//  logwrite( function, message.str() );
#endif

    cmd.append( term );        // add terminator before sending command to the TCS

    if ( this->tcs.Write( cmd ) == -1 ) {
      message.str(""); message << "ERROR writing to TCS on fd " << this->tcs.getfd();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // receive the reply
    //
    do {
      // wait for incoming data
      //
      if ( ( ret = this->tcs.Poll() ) <= 0 ) {
        if ( ret==0 ) { message.str(""); message << "TIMEOUT polling fd " << tcs.getfd() 
                                                 << " waiting for response from TCS"; error = TIMEOUT; }
        if ( ret <0 ) { message.str(""); message << "ERROR polling fd "   << tcs.getfd() 
                                                 << " waiting for response from TCS"; error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      // data available, read from socket
      //
      if ( ( ret = this->tcs.Read( sbuf, delim ) ) <= 0 ) {
        if ( ret < 0 ) {
          message.str(""); message << "ERROR reading from TCS on fd " << tcs.getfd() << ": " << strerror(errno);
          logwrite( function, message.str() );
        }
        if ( ret == 0 ) {
          message.str(""); message << "TIMEOUT reading from TCS on fd " << tcs.getfd() << ": " << strerror(errno);
          logwrite( function, message.str() );
        }
        break;
      }
    } while ( ret > 0 && sbuf.find( delim ) == std::string::npos );

    // remove any trailing linefeed and carriage return
    //
    sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\r' ), sbuf.end());
//  sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\n' ), sbuf.end()); // can't strip newline since TCS embeds this

    reply = sbuf;

#ifdef LOGLEVEL_DEBUG  // this can be a little much when polling
//  message.str(""); message << "[DEBUG] received from TCS on fd " << this->tcs.getfd() << ": " << reply;
//  logwrite( function, message.str() );
#endif

    return( error );
  }
  /***** TCS::Interface::send_command *****************************************/

}
