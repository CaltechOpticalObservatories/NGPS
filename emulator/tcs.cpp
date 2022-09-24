#include "tcs.h"

namespace Tcs {

  /**************** Tcs::Interface::Interface *********************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /**************** Tcs::Interface::Interface *********************************/


  /**************** Tcs::Interface::~Interface ********************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Tcs::Interface::~Interface ********************************/


  /**************** Tcs::Telescope::Telescope *********************************/
  /**
   * @fn         Telescope
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   * sets some default values
   *
   */
  Telescope::Telescope() {
    this->motionstate.store( MOTION_STOPPED );  /// telescope is stopped by default

    // default slew and settling times set here can be overridden by configuration file
    //
    this->slewrate_ra = 0.75;     /// default slew rate in s for RA, override with EMULATOR_SLEWRATE_RA
    this->slewrate_dec = 0.75;    /// default slew rate in s for DEC override with EMULATOR_SLEWRATE_DEC
    this->slewrate_casangle = 2;  /// default slew rate in s for CASANGLE override with EMULATOR_SLEWRATE_CASNGLE
    this->settle_ra = 12;         /// default settling time in s for RA override with EMULATOR_SETTLE_RA
    this->settle_dec = 12;        /// default settling time in s for DEC override with EMULATOR_SETTLE_DEC
    this->settle_casangle = 0.5;  /// default settling time in s for CASANGLE override with EMULATOR_SETTLE__CASNGLE

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
  /**************** Tcs::Telescope::Telescope *********************************/


  /**************** Tcs::Telescope::~Telescope ********************************/
  /**
   * @fn         ~Telescope
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Telescope::~Telescope() {
  }
  /**************** Tcs::Telescope::~Telescope ********************************/


  /**************** Tcs::Telescope::do_coords *********************************/
  /**
   * @fn         do_coords
   * @brief      perform the COORDS command work, which "moves" the telescope
   * @param[in]  telescope
   * @param[in]  args
   * @return     none
   *
   */
  void Telescope::do_coords( Tcs::Telescope &telescope, std::string args ) {
    std::string function = "  (Tcs::Telescope::do_coords) ";

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
    telescope.coords_running.store( true );

    // begin the (virtual) slew
    // slew rates are specified per second -- divide that by 2 and loop every 1/2 second
    // (for finer granularity)
    //
    telescope.motionstate.store( MOTION_SLEWING );
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
    telescope.motionstate.store( MOTION_SETTLING );
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
    // and set motion state to tracking
    //
    telescope.ra.store( newra );
    telescope.dec.store( newdec );
    telescope.motionstate.store( MOTION_TRACKING_STABLY );
    std::cerr << get_timestamp() << function << "tracking stably!  ra " 
              << telescope.ra.load() << "  dec " << telescope.dec.load() << "\n";

    // clear the running flag now that this thread is done
    //
    telescope.coords_running.store( false );

    return;
  }
  /**************** Tcs::Telescope::do_coords *********************************/


  /**************** Tcs::Interface::parse_command *****************************/
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
    std::string function = "  (Tcs::Interface::parse_command) ";

    // The real TCS allows an empty command but I don't,
    // only because I don't forsee anyone using it.
    //
    if ( cmd.empty() ) { retstring = "-1"; return( NO_ERROR ); }

    std::cerr << get_timestamp() << function << "received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    std::string mycmd;                 // command is the 1st token
    size_t nargs;                      // number of args (after command)

    if ( tokens.size() < 1 ) {         // should be impossible since already checked for cmd.empty()
      std::cerr << get_timestamp() << function << "ERROR: no tokens\n";
      retstring = "-2";                // invalid parameters
      return( ERROR );
    }

    try {
      mycmd = tokens.at(0);            // command is the 1st token
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
      retstring = std::to_string( this->telescope.motionstate.load() );
    }
    else

    if ( mycmd == "COORDS" ) {
      // can only run one of these threads at a time
      //
      if ( this->telescope.coords_running.load() ) {
        std::cerr << get_timestamp() << function << "ERROR: coordinates have already been sent\n";
        retstring = "-3";               // unable to execute at this time
      }
      else
      if ( nargs < 5 ) {
        std::cerr << get_timestamp() << function << "ERROR: expected minimum 5 args but received " << nargs << "\n";
        retstring = "-2";              // invalid parameters
      }
      else {
        std::stringstream myargs;
        for ( auto tok : tokens ) myargs << tok << " ";
        std::thread( std::ref( Tcs::Telescope::do_coords ), 
                     std::ref( this->telescope ),
                     myargs.str() ).detach();
        retstring = "0";               // successful completion
      }
    }
    else {
      std::cerr << get_timestamp() << function << "ERROR: unknown command " << cmd << "\n";
      retstring = "-1";                // unrecognized command
    }

    std::cerr << get_timestamp() << function << "reply from TCS emulator: " << retstring << "\n";

    return ( NO_ERROR );
  }
  /**************** Tcs::Interface::parse_command *****************************/

}
