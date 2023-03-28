/**
 * @file    tcs.cpp
 * @brief   these are the main functions for the TCS emulator
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#include "tcs.h"

namespace TcsEmulator {

  /***** TcsEmulator::Interface::Interface ************************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
    this->map_returnval[ TCS_SUCCESS ]              = TCS_SUCCESS_STR;
    this->map_returnval[ TCS_UNRECOGNIZED_COMMAND ] = TCS_UNRECOGNIZED_COMMAND_STR;
    this->map_returnval[ TCS_INVALID_PARAMETER ]    = TCS_INVALID_PARAMETER_STR;
    this->map_returnval[ TCS_UNABLE_TO_EXECUTE ]    = TCS_UNABLE_TO_EXECUTE_STR;
    this->map_returnval[ TCS_HOST_UNAVAILABLE ]     = TCS_HOST_UNAVAILABLE_STR;
    this->map_returnval[ TCS_UNDEFINED ]            = TCS_UNDEFINED_STR;

    this->map_motionval[ TCS_MOTION_STOPPED ]    = TCS_MOTION_STOPPED_STR;
    this->map_motionval[ TCS_MOTION_SLEWING ]    = TCS_MOTION_SLEWING_STR;
    this->map_motionval[ TCS_MOTION_OFFSETTING ] = TCS_MOTION_OFFSETTING_STR;
    this->map_motionval[ TCS_MOTION_TRACKING ]   = TCS_MOTION_TRACKING_STR;
    this->map_motionval[ TCS_MOTION_SETTLING ]   = TCS_MOTION_SETTLING_STR;
    this->map_motionval[ TCS_MOTION_UNKNOWN ]    = TCS_MOTION_UNKNOWN_STR;
  }
  /***** TcsEmulator::Interface::Interface ************************************/


  /***** TcsEmulator::Interface::~Interface ***********************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /***** TcsEmulator::Interface::~Interface ***********************************/


  /***** TcsEmulator::Telescope::Telescope ************************************/
  /**
   * @fn         Telescope
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   * sets some default values
   * some of these can be overridden by the configuration file
   *
   */
  Telescope::Telescope() {
    this->motionstate.store( TCS_MOTION_STOPPED );  //! telescope is stopped by default

    // default slew and settling times set here can be overridden by configuration file
    //
    this->slewrate_ra = 0.75;     //! default slew rate is 0.75/s for RA, override with EMULATOR_SLEWRATE_RA
    this->slewrate_dec = 0.75;    //! default slew rate is 0.75/s for DEC override with EMULATOR_SLEWRATE_DEC
    this->slewrate_casangle = 2;  //! default slew rate is 2/s for CASANGLE override with EMULATOR_SLEWRATE_CASNGLE
    this->settle_ra = 12;         //! default settling time is 12s for RA override with EMULATOR_SETTLE_RA
    this->settle_dec = 12;        //! default settling time is 12s for DEC override with EMULATOR_SETTLE_DEC
    this->settle_casangle = 0.5;  //! default settling time is 0.5s for CASANGLE override with EMULATOR_SETTLE__CASNGLE
    this->offsetrate_ra = 9999; //45;     //! default offset RA rate in arcsec/sec
    this->offsetrate_dec = 9999; //45;    //! default offset DEC rate in arcsec/sec

    this->telid = 200;
    this->focus = 36.71;
    this->tubelength = 22.11;
    this->ra.store(0);
    this->dec.store(0);
    this->offset_ra = 0;
    this->offset_dec = 0;
    this->trackrate_ra = 0;
    this->trackrate_dec = 0;
    this->casangle = 0;
    this->ha = 0;
    this->lst = 0;
    this->airmass = 1;
    this->azimuth = 0;
    this->zangle = 0;
  }
  /***** TcsEmulator::Telescope::Telescope ************************************/


  /***** TcsEmulator::Telescope::~Telescope ***********************************/
  /**
   * @fn         ~Telescope
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Telescope::~Telescope() {
  }
  /***** TcsEmulator::Telescope::~Telescope ***********************************/


  /***** TcsEmulator::Telescope::do_coords ************************************/
  /**
   * @fn         do_coords
   * @brief      perform the COORDS command work, which "moves" the telescope
   * @param[in]  telescope
   * @param[in]  args
   * @return     none
   *
   */
  void Telescope::do_coords( TcsEmulator::Telescope &telescope, std::string args ) {
    std::string function = "  (TcsEmulator::Telescope::do_coords) ";

    double newra, newdec;

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    try {
      // already checked that there are at least 5 tokens but all that matters
      // here is RA and DEC
      //
      newra  = std::stof( tokens.at(0) );
      newdec = std::stof( tokens.at(1) );
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "unable to convert one or more values from \"" << args << "\": " << e.what() << "\n";
      return;
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "one or more value from \"" << args << "\" is out of range: " << e.what() << "\n";
      return;
    }

    // calculate the slew distance and the slew time for each of RA, DEC
    //
    double slewdistance_ra  = std::abs( telescope.ra  - newra  );
    double slewdistance_dec = std::abs( telescope.dec - newdec );

    double slewtime_ra      = slewdistance_ra  / telescope.slewrate_ra;
    double slewtime_dec     = slewdistance_dec / telescope.slewrate_dec;

    // whichever one is greater is the only one that matters here
    //
    double slewtime = ( slewtime_ra > slewtime_dec ? slewtime_ra : slewtime_dec );

    // get the end time for this slew, which is now plus the longest slew time
    //
    double clock_end = get_clock_time() + slewtime;

    // before starting the slew, set a flag to prevent another thread from also starting a slew
    //
    telescope.moving.store( true );

    // begin the (virtual) slew
    // slew rates are specified per second -- divide that by 2 and loop every 1/2 second
    // (for finer granularity)
    //
    telescope.motionstate.store( TCS_MOTION_SLEWING );
    while ( true ) {

      // read the current ra, dec
      //
      double ra  = telescope.ra.load();
      double dec = telescope.dec.load();

      std::cerr << get_timestamp() << function << "slewing...  ra " << ra << "  dec " << dec << "\n";

      // add the slewrate to ra, dec
      // store it permanently as long as we don't overshoot
      //
      if ( ( ra  += telescope.slewrate_ra/2.0 )  <= newra )  telescope.ra.store( ra  );
      if ( ( dec += telescope.slewrate_dec/2.0 ) <= newdec ) telescope.dec.store( dec );

      usleep( 500000 );
      if ( get_clock_time() >= clock_end ) break;
    }

    // begin the virtual settling
    // use whichever settling time is longest, ra or dec
    //
    double settle = ( telescope.settle_ra > telescope.settle_dec ? telescope.settle_ra : telescope.settle_dec );
    clock_end = get_clock_time() + settle;
    telescope.motionstate.store( TCS_MOTION_SETTLING );
    while ( true ) {
      if ( get_clock_time() > clock_end ) break;
      // read the current ra, dec
      //
      double ra  = telescope.ra.load();
      double dec = telescope.dec.load();

      std::cerr << get_timestamp() << function << "settling...  ra " << ra << "  dec " << dec << "\n";

      // add the slewrate to ra, dec
      // store it permanently as long as we don't overshoot
      //
      if ( ( ra  += telescope.slewrate_ra/telescope.settle_ra )   <= newra )  telescope.ra.store( ra  );
      if ( ( dec += telescope.slewrate_dec/telescope.settle_dec ) <= newdec ) telescope.dec.store( dec );

      usleep( 1000000 );
      if ( get_clock_time() >= clock_end ) break;
    }

    // after settlting, set the ra,dec to those requested
    //
    telescope.ra.store( newra );
    telescope.dec.store( newdec );

    // clear the moving flag now
    //
    telescope.moving.store( false );

    // and set motion state to tracking
    //
    telescope.motionstate.store( TCS_MOTION_TRACKING );
    std::cerr << get_timestamp() << function << "tracking stably!  ra " 
              << telescope.ra.load() << "  dec " << telescope.dec.load() << "\n";

    return;
  }
  /***** TcsEmulator::Telescope::do_coords ************************************/


  /***** TcsEmulator::Telescope::do_pt ****************************************/
  /**
   * @fn         do_pt
   * @brief      perform the PT command work, which "offsets" the telescope
   * @details    inputs are in arcsec
   * @param[in]  telescope
   * @param[in]  args
   * @return     none
   *
   */
  void Telescope::do_pt( TcsEmulator::Telescope &telescope, std::string args ) {
    std::string function = "  (TcsEmulator::Telescope::do_pt) ";

    double ra_off, dec_off;  // offsets read in
    double newra, newdec;    // these will be the new RA, DEC after offsetting

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    // before starting the slew, set a flag to prevent another thread from also starting a slew
    //
    telescope.moving.store( true );

    telescope.motionstate.store( TCS_MOTION_OFFSETTING );

    try {
      ra_off  = std::stod( tokens.at(0) ) / 3600.;  // convert from arcsec to deg
      dec_off = std::stod( tokens.at(1) ) / 3600.;
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "unable to convert one or more values from \"" 
                                   << args << "\": " << e.what() << "\n";
      return;
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "one or more value from \"" 
                                   << args << "\" is out of range: " << e.what() << "\n";
      return;
    }

    std::cerr << get_timestamp() << function << "offsetting ra " << ra_off << " dec " << dec_off << "\n";

    newra  = telescope.ra.load()  + ra_off;   // this is where we want to end up, after offset is complete
    newdec = telescope.dec.load() + dec_off;

    // calculate the offset distance and the offset time for each of RA, DEC
    //
    double offsetdistance_ra  = std::abs( telescope.ra  - newra  );
    double offsetdistance_dec = std::abs( telescope.dec - newdec );

    double offsettime_ra      = offsetdistance_ra  / telescope.offsetrate_ra;
    double offsettime_dec     = offsetdistance_dec / telescope.offsetrate_dec;

    // whichever one is greater is the only one that matters here
    //
    double offsettime = ( offsettime_ra > offsettime_dec ? offsettime_ra : offsettime_dec );

    // get the end time for this slew, which is now plus the longest slew time
    //
    double clock_end = get_clock_time() + offsettime;

    // Begin the offsetting here, loop at 10Hz
    //
    while ( true ) {

      // read the current ra, dec
      //
      double ra  = telescope.ra.load();
      double dec = telescope.dec.load();

      std::cerr << get_timestamp() << function << "offsetting...  ra " << ra << "  dec " << dec << "\n";

      // add the slewrate to ra, dec
      // store it permanently as long as we don't overshoot
      //
      if ( ( ra  += telescope.offsetrate_ra/10.0 )  <= newra )  telescope.ra.store( ra  );
      if ( ( dec += telescope.offsetrate_dec/10.0 ) <= newdec ) telescope.dec.store( dec );

      usleep( 100000 );
      if ( get_clock_time() >= clock_end ) break;
    }

    // then set the ra,dec to those requested (because the granularity above is not perfect)
    //
    telescope.ra.store( newra );
    telescope.dec.store( newdec );

    // clear the moving flag now
    //
    telescope.moving.store( false );

    telescope.motionstate.store( TCS_MOTION_TRACKING );

    std::cerr << get_timestamp() << function << "tracking stably!  ra " 
              << telescope.ra.load() << "  dec " << telescope.dec.load() << "\n";

    return;
  }
  /***** TcsEmulator::Telescope::do_pt ****************************************/


  /***** TcsEmulator::Telescope::mrates ***************************************/
  /**
   * @brief      perform the MRATES command
   * @param[in]  args       input args should be "ra dec" offset rates
   * @param[out] retstring  reference to string contains the return string
   *
   */
  void Telescope::mrates( std::string args, std::string &retstring ) {
    std::string function = "  (TcsEmulator::Telescope::mrates) ";

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 2 ) {
      std::cerr << get_timestamp() << function << "ERROR expected 2 but received " << tokens.size() << " args\n";
      retstring = "-2";
      return;
    }

    try {
      this->offsetrate_ra = std::stod( tokens.at(0) );
      this->offsetrate_dec = std::stod( tokens.at(1) );
    }
    catch ( std::invalid_argument &e ) {
      std::cerr << "ERROR unable to convert " << args << " to double\n";
      retstring = "-1";
      return;
    }
    catch ( std::out_of_range &e ) {
      std::cerr << "ERROR out of range parsing " << args << "\n";
      retstring = "-1";
      return;
    }

    std::cerr << get_timestamp() << function << "new offsetrate_ra=" << this->offsetrate_ra
                                             << " offsetrate_dec=" << this->offsetrate_dec << "\n";
    return;
  }
  /***** TcsEmulator::Telescope::mrates ***************************************/


  /***** TcsEmulator::Telescope::weather **************************************/
  /**
   * @brief      perform the ?WEATHER command, which returns a long string
   * @param[out] retstring  reference to string contains the return string
   *
   */
  void Telescope::weather( std::string &retstring ) {
    std::string function = "  (TcsEmulator::Telescope::weather) ";

    std::stringstream ret;

    ret << "RA="                  << std::fixed << std::setprecision(8) << this->ra.load()  << "\n"
        << "DEC="                 << std::fixed << std::setprecision(8) << this->dec.load() << "\n"
        << "HA="                  << "0\n"
        << "LST="                 << "0\n"
        << "Air mass="            << "0\n"
        << "Azimuth="             << "0\n"
        << "Zenith angle="        << "0\n"
        << "Focus Point="         << "0\n"
        << "Dome Azimuth="        << "0\n"
        << "Dome shutters="       << "0\n"
        << "Windscreen position=" << "0\n"
        << "InstPos="             << "0\n"
        << "Pumps="               << "0";

    retstring = ret.str();

    return;
  }
  /***** TcsEmulator::Telescope::weather **************************************/


  /***** TcsEmulator::Telescope::reqpos ***************************************/
  /**
   * @brief      perform the REQPOS command, which returns a long string
   * @param[out] retstring  reference to string contains the return string
   *
   * In UTC, ddd is UT day of year. RA and DEC are J2000. HA is apparent.
   *
   */
  void Telescope::reqpos( std::string &retstring ) {
    std::string function = "  (TcsEmulator::Telescope::reqpos) ";

    std::stringstream ret;

    double _ra  = this->ra.load();   // decimal hours, e.g. 01.2345
    double _dec = this->dec.load();  // decimal degrees

    std::string dec_sign = ( _dec < 0 ? "-" : "+" );

    _dec = std::abs(_dec);

    int ra_hh     = (int)_ra;
    double ra_mm  = (_ra - ra_hh) * 60.;
    double ra_ss  = (ra_mm - (int)ra_mm) * 60.;
    double ra_sss = (ra_ss - (int)ra_ss)*100.;

    int dec_dd     = (int)_dec;
    double dec_mm  = (_dec - dec_dd) * 60.;
    double dec_ss  = (dec_mm - (int)dec_mm) * 60.;
    double dec_sss = (dec_ss - (int)dec_ss)*10.;

    ret << "UTC = ddd hh:mm:ss, "
        << "LST = hh:mm:ss\n"
        << "RA = " << std::fixed  << std::setfill('0') << std::setw(2) << (int)_ra     << ":"
                                  << std::setfill('0') << std::setw(2) << (int)ra_mm   << ":"
                                  << std::setfill('0') << std::setw(2) << (int)ra_ss   << "."
                                  << std::setfill('0') << std::setw(2) << (int)std::round(ra_sss)  << ", "
        << "DEC = " << dec_sign << std::fixed << std::setfill('0') << std::setw(2) << (int)_dec    << ":"
                                              << std::setfill('0') << std::setw(2) << (int)dec_mm  << ":"
                                              << std::setfill('0') << std::setw(2) << (int)dec_ss  << "."
                                              << std::setfill('0') << std::setw(1) << (int)std::round(dec_sss) << ", "
        << "HA=Whh:mm:ss.s\n"
        << "air mass = aa.aaa";

    retstring = ret.str();

    return;
  }
  /***** TcsEmulator::Telescope::reqpos ***************************************/


  /***** TcsEmulator::Interface::parse_command ********************************/
  /**
   * @fn         parse_command
   * @brief      parse commands for the TCS, spawn a thread if necessary
   * @param[in]  cmd
   * @param[out] retstring
   * @return     ERROR or NO_ERROR
   *
   * This function parses the commands and arguments received by the emulator which
   * would need to go to the TCS.  Long commands (like COORDS for telescope moves) 
   * are spawned in a separate thread, everything else is responded to immediately here.
   *
   * Always set retstring, don't ever return it empty.
   *
   */
  long Interface::parse_command( std::string cmd, std::string &retstring ) {
    std::string function = "  (TcsEmulator::Interface::parse_command) ";

    // The real TCS allows an empty command but I don't,
    // only because I don't forsee anyone using it.
    //
    if ( cmd.empty() ) { retstring = "-1"; return( NO_ERROR ); }

    std::cerr << get_timestamp() << function << "received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    std::string mycmd;                 // command is the 1st token
    std::stringstream myargs;
    size_t nargs;                      // number of args (after command)

    if ( tokens.size() < 1 ) {         // should be impossible since already checked for cmd.empty()
      std::cerr << get_timestamp() << function << "ERROR: no tokens\n";
      retstring = "-2";                // invalid parameters
      return( ERROR );
    }

    try {
      mycmd = tokens.at(0);            // command is the 1st token
      for ( size_t tok=1; tok<tokens.size(); ++tok ) myargs << tokens.at(tok) << " ";
      tokens.erase( tokens.begin() );  // remove the commmand from the vector
      nargs = tokens.size();           // number of args after removing command
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "unable to convert one or more values: " << e.what() << "\n";
      retstring = "-1";                // unrecognized command
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "one or more values out of range: " << e.what() << "\n";
      retstring = "-1";                // unrecognized command
      return( ERROR );
    }

    /* ------------------------
     * handle the commands here
     * ------------------------
     *
     */

    if ( mycmd == "?MOTION" ) {
      retstring = std::to_string( this->telescope.motionstate.load() );  // return the current motion state
      std::cerr << get_timestamp() << function << "reply from TCS emulator: " 
                                               << this->map_motionval[ this->telescope.motionstate.load() ] << "\n";
    }
    else {

    if ( mycmd == "MRATES" ) {
      this->telescope.mrates( myargs.str(), retstring );
    }
    else
    if ( mycmd == "COORDS" ) {
      // can only run one of these threads at a time
      //
      if ( this->telescope.moving.load() ) {
        std::cerr << get_timestamp() << function << "ERROR: move command already sent\n";
        retstring = "-3";               // unable to execute at this time
      }
      else
      if ( nargs < 5 ) {
        std::cerr << get_timestamp() << function << "ERROR: expected minimum 5 args but received " << nargs << "\n";
        retstring = "-2";              // invalid parameters
      }
      else {
        std::thread( std::ref( TcsEmulator::Telescope::do_coords ), 
                     std::ref( this->telescope ),
                     myargs.str() ).detach();
        retstring = "0";               // successful completion
      }
    }
    else
    if ( mycmd == "PT" ) {
      // can only run one of these threads at a time
      //
      if ( this->telescope.moving.load() ) {
        std::cerr << get_timestamp() << function << "ERROR: move command already sent\n";
        retstring = "-3";               // unable to execute at this time
      }
      else
      if ( nargs != 2 ) {
        std::cerr << get_timestamp() << function << "ERROR: expected 2 args but received " << nargs << "\n";
        retstring = "-2";              // invalid parameters
      }
      else {
        std::thread( std::ref( TcsEmulator::Telescope::do_pt ), 
                     std::ref( this->telescope ),
                     myargs.str() ).detach();
        retstring = "0";               // successful completion
      }
    }
    else
    if ( mycmd == "N" ) {
      retstring = "0";                 // successful completion
    }
    else
    if ( mycmd == "S" ) {
      retstring = "0";                 // successful completion
    }
    else
    if ( mycmd == "E" ) {
      retstring = "0";                 // successful completion
    }
    else
    if ( mycmd == "W" ) {
      retstring = "0";                 // successful completion
    }
    else
    if ( mycmd == "REQPOS" ) {
      this->telescope.reqpos( retstring );
    }
    else
    if ( mycmd == "?WEATHER" ) {
      this->telescope.weather( retstring );
    }
    else {
      std::cerr << get_timestamp() << function << "ERROR: unknown command " << cmd << "\n";
      retstring = "-1";                // unrecognized command
    }

    try {
      int retval=std::stoi( retstring );
      std::cerr << get_timestamp() << function << "reply from TCS emulator: " << this->map_returnval[ retval ] << "\n";
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "reply from TCS emulator: " << retstring << "\n";
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "reply from TCS emulator: " << retstring << "\n";
    }

    }

    return ( NO_ERROR );
  }
  /***** TcsEmulator::Interface::parse_command ********************************/

}
