/**
 * @file    tcs.cpp
 * @brief   these are the main functions for the TCS emulator
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#include "emulatord_tcs.h"

#define MOVETYPE_FOCUS_GO  0
#define MOVETYPE_FOCUS_INC 1

namespace TcsEmulator {

  TcsEmulator::Server* Server::instance = nullptr;

  /***** TcsEmulator::Server::handle_signal ***********************************/
  /**
   * @brief      handles ctrl-C and other signals
   * @param[in]  signo
   *
   */
  void Server::handle_signal( int signo ) {
    std::string function = "  (TcsEmulator::signal_handler) ";
    switch ( signo ) {
      case SIGTERM:
      case SIGINT:
        std::cerr << get_timestamp() << function << Server::instance->subsystem << " received termination signal\n";
        Server::instance->exit_cleanly();                   // shutdown the daemon
        break;
      case SIGHUP:
        std::cerr << get_timestamp() << function << Server::instance->subsystem << " caught SIGHUP\n";
        Server::instance->configure_emulator();             // TODO can (/should) this be done while running?
        break;
      case SIGPIPE:
        std::cerr << get_timestamp() << function << Server::instance->subsystem << " caught SIGPIPE\n";
        break;
      default:
        std::cerr << get_timestamp() << function << Server::instance->subsystem << " received unknown signal\n";
        Server::instance->exit_cleanly();                   // shutdown the daemon
        break;
    }
    return;
  }
  /***** TcsEmulator::Server::handle_signal ***********************************/


  /***** TcsEmulator::Server::exit_cleanly ************************************/
  /**
   * @brief      closes things nicely and exits
   *
   */
  void Server::exit_cleanly() {
    std::string function = "  (TcsEmulator::Server::exit_cleanly) ";
    std::cerr << get_timestamp() << function << "emulatord." << Server::instance->subsystem << " exiting\n";

    // close connection
    //
    if ( Server::instance->port > 0 ) close( Server::instance->port );
    exit( EXIT_SUCCESS );
  }
  /***** TcsEmulator::Server::exit_cleanly ************************************/


  /***** TcsEmulator::Server::block_main **************************************/
  /**
   * @brief      main function for blocking connection thread
   * @param[in]  server  reference to TcsEmulator::Server object
   * @param[in]  sock    Network::TcpSocket socket object
   *
   * accepts a socket connection and processes the request by
   * calling function doit()
   *
   * This thread never terminates.
   *
   */
  void Server::block_main( TcsEmulator::Server &server, Network::TcpSocket sock ) {
    std::string function = "  (TcsEmulator::Server::block_main) ";
    while(1) {
      sock.Accept();
      std::cerr << get_timestamp() << function << " Accept returns connection on fd = " << sock.getfd() << "\n";
      std::thread( server.doit, std::ref(sock) ).detach();  // spawn a thread to handle this connection
    }
  }
  /***** TcsEmulator::Server::block_main **************************************/


  /***** TcsEmulator::Server::doit ********************************************/
  /**
   * @brief      the workhorse of each thread connetion
   * @param[in]  sock  TcpSocket object
   *
   * stays open until closed by client
   *
   * commands come in the form:
   * <device> [all|<app>] [_BLOCK_] <command> [<arg>]
   *
   */
  void Server::doit( Network::TcpSocket sock ) {
    std::string function = "  (TcsEmulator::Server::doit) ";
    long  ret;
    std::stringstream message;
    std::string cmd, args;        // arg string is everything after command
    std::vector<std::string> tokens;
    char delim = '\r';           /// commands sent to me (the TCS) have been terminated with this
    char term = '\0';     /// my replies (as the TCS) get terminated with this

    bool connection_open=true;

    std::cerr << get_timestamp() << function << "accepted connection on fd " << sock.getfd() << "\n";
    while (connection_open) {

      // Wait (poll) connected socket for incoming data...
      //
      int pollret;
      if ( ( pollret=sock.Poll() ) <= 0 ) {
        if (pollret==0) {
          std::cerr << get_timestamp() << function << Server::instance->subsystem
                    << " Poll timeout on fd " << sock.getfd() << " thread " << sock.id << "\n";
        }
        if (pollret <0) {
          std::cerr << get_timestamp() << function << Server::instance->subsystem << " Poll error on fd " << sock.getfd()
                    << " thread " << sock.id << ": " << strerror(errno) << "\n";
        }
        break;                      // this will close the connection
      }

      // Data available, now read from connected socket...
      //
      std::string sbuf;
      if ( (ret=sock.Read( sbuf, delim )) <= 0 ) {     // read until newline delimiter
        if (ret<0) {                // could be an actual read error
          std::cerr << get_timestamp() << function << Server::instance->subsystem
                    << " Read error on fd " << sock.getfd() << ": " << strerror(errno) << "\n";
        }
        if (ret==-2 && sock.getfd() != -1) std::cerr << get_timestamp() << function << Server::instance->subsystem
                                                     << " timeout reading from fd " << sock.getfd() << "\n";
        break;                      // Breaking out of the while loop will close the connection.
                                    // This probably means that the client has terminated abruptly, 
                                    // having sent FIN but not stuck around long enough
                                    // to accept CLOSE and give the LAST_ACK.
      }

      std::cerr << get_timestamp() << function << "[DEBUG] incomming command is terminated with " << tchar(sbuf) << "\n";

      // convert the input buffer into a string and remove any trailing linefeed
      // and carriage return
      //
      sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\r' ), sbuf.end());
      sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\n' ), sbuf.end());

      try {
        std::size_t cmd_sep = sbuf.find_first_of(" "); // find the first space, which separates command from argument list

        cmd = sbuf.substr(0, cmd_sep);                 // cmd is everything up until that space

        if (cmd.empty()) continue;                     // If no command then skip over everything.

        if (cmd_sep == std::string::npos) {            // If no space was found,
          args="";                                     // then the arg list is empty,
        }
        else {
          args= sbuf.substr(cmd_sep+1);                // otherwise args is everything after that space.
        }

        sock.id = ++Server::instance->cmd_num;
        if ( Server::instance->cmd_num == INT_MAX ) Server::instance->cmd_num = 0;

        std::cerr << get_timestamp() << function << Server::instance->subsystem << " received command on fd " << sock.getfd()
                  << " (" << sock.id << "): " << cmd << " " << args << "\n";
      }
      catch ( std::runtime_error &e ) {
        std::stringstream errstream; errstream << e.what();
        std::cerr << get_timestamp() << function << Server::instance->subsystem
                  << " error parsing arguments: " << errstream.str() << "\n";
        ret = -1;
      }
      catch ( ... ) {
        std::cerr << get_timestamp() << function << Server::instance->subsystem << " unknown error parsing arguments: " << args << "\n";
        ret = -1;
      }

      // process commands here
      //
      ret = NOTHING;
      std::string retstring;      // string for the return value

      retstring.clear();

      if ( cmd.compare( "exit" ) == 0 ) {
                      Server::instance->exit_cleanly();                                   // shutdown the daemon
      }
      else
      if ( cmd.compare( TCSD_CLOSE ) == 0 ) {
                      sock.Close();
                      return;
      }
      else {  // if no matching command found then send it straight to the interface's command parser
        try {
          std::transform( sbuf.begin(), sbuf.end(), sbuf.begin(), ::toupper );   // make uppercase
        }
        catch (...) {
          std::cerr << get_timestamp() << function << "error converting command to uppercase\n";
          ret=ERROR;
        }
        ret = Server::instance->interface.parse_command( sbuf, retstring );               // send the command to the parser
      }

#ifdef LOGLEVEL_DEBUG
      std::cerr << get_timestamp() << function << "[DEBUG] ret=" << ret << " retstring=" << retstring << "\n";
      std::cerr << get_timestamp() << function << "[DEBUG] retstring is terminated with " << tchar(retstring) << "\n";
#endif

      if ( ret != NOTHING && !retstring.empty() ) {
        retstring.push_back( term );       // push_back is overloaded to accept a char which is needed here
        if ( sock.Write( retstring ) <0 ) connection_open=false;
      }

      if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                           // Keep blocking connection open for interactive session.
    }

    connection_open = false;
    sock.Close();
    std::cerr << get_timestamp() << function << "connection closed\n";
    return;
  }
  /***** TcsEmulator::Server::doit ********************************************/


  /***** TcsEmulator::Interface::Interface ************************************/
  /**
   * @brief      Interface class constructor
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


  /***** TcsEmulator::Telescope::initialize_python_objects ********************/
  /**
   * @brief      provides interface to initialize Python objects in the class
   * @details    This provides an interface (to the Acam Server) to initialize
   *             any Python modules in objects in the class. Allows Daemons to
   *             ensure Python initialization is done by the child process.
   *
   */
  void Telescope::initialize_python_objects() {
//  this->initialize_python();
    return;
  }
  /***** TcsEmulator::Telescope::initialize_python_objects ********************/


  /***** TcsEmulator::Telescope::get_time *************************************/
  /**
   * @brief      get the current time
   * @return     string containing the time formatted as ddd hh:mm:ss.s
   *
   */
  std::string Telescope::get_time() {
    time_t t;                        // Container for system time
    struct timespec timenow;         // Time of day container
    struct tm mytime;                // GMT time container
    std::stringstream timestring;

    // Get the system time, return a bad timestamp on error
    //
    if ( clock_gettime( CLOCK_REALTIME, &timenow ) != 0 ) return( timestring.str() );

    t = timenow.tv_sec;
    if ( gmtime_r( &t, &mytime ) == NULL ) return( timestring.str() );

    timestring << std::fixed 
               << std::setfill('0')
               << std::setprecision(0)
               << std::setw(3)
               << mytime.tm_yday << " "
               << std::setw(2)
               << mytime.tm_hour << ":"
               << mytime.tm_min  << ":"
               << mytime.tm_sec  << "."
               << std::setprecision(1)
               << std::setw(1)
               << static_cast<int>(timenow.tv_nsec/1000000000.0);

    return( timestring.str() );
  }
  /***** TcsEmulator::Telescope::get_time *************************************/


  /***** TcsEmulator::Telescope::do_ringgo ************************************/
  /**
   * @brief      perform the RINGGO command work, which "moves" the cass rotator
   * @param[in]  telescope  reference to TcsEmulator::Telescope object
   * @param[in]  newring    new ring position
   *
   */
  void Telescope::do_ringgo( TcsEmulator::Telescope &telescope, double newring ) {
    std::string function = "  (TcsEmulator::Telescope::do_ringgo) ";

    telescope.casmoving.store( true );

    // for now, the cass rotator moves instantaneously
    //
    telescope.casangle = newring;

    telescope.casmoving.store( false );

    return;
  }
  /***** TcsEmulator::Telescope::do_ringgo ************************************/


  /***** TcsEmulator::Telescope::do_focus *************************************/
  /**
   * @brief      perform the FOCUSGO command work, which "moves" the focus
   * @param[in]  telescope  reference to TcsEmulator::Telescope object
   * @param[in]  focusval   new focus position
   * @param[in]  movetype   type of move, for FOCUSGO or FOCUSINC
   *
   */
  void Telescope::do_focus( TcsEmulator::Telescope &telescope, double focusval, int movetype ) {
    std::string function = "  (TcsEmulator::Telescope::do_focus) ";

    double newfocus=0;
    double current_focus = telescope.focus.load();                     // the current focus

    switch ( movetype ) {
      case MOVETYPE_FOCUS_GO:
        newfocus = focusval;
        break;
      case MOVETYPE_FOCUS_INC:
        newfocus = current_focus + focusval;
        break;
      default:
        std::cerr << get_timestamp() << function << "invalid move type " << movetype << "\n";
        return;
    }

    if ( newfocus < 1.00 || newfocus > 74.00 ) {
      std::cerr << get_timestamp() << function << "invalid focus position " << newfocus << "\n";
      return;
    }

    double focus_dir = ( newfocus - current_focus < 0 ? -1.0 : 1.0 );  // direction to move to reach requested focus from current focus
    double focusdistance = std::abs( telescope.focus - newfocus );     // calculate focus distance
    double focustime = focusdistance  / telescope.focusrate;           // calculate focus time
    double focus_end_time = get_clock_time() + focustime;              // get the end time for the focus move

    telescope.focussing.store( true );                                 // prevent another thread from also focussing

    while ( true ) {

      current_focus  = telescope.focus.load();                         // read the current focus

      std::cerr << get_timestamp() << function << "moving focus...  " << current_focus << "\n";

      double delta_focus  = std::abs( current_focus - newfocus );      // add the slewrate to focus

      if ( delta_focus > telescope.focusrate/2.0 ) {                   // move half the rate/s since loop freq is 0.5Hz
        // store it permanently as long as we don't overshoot
        telescope.focus.store( ( current_focus + focus_dir * telescope.focusrate/2.0 ) );
      }

      std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
      if ( get_clock_time() >= focus_end_time ) break;
    }
    telescope.focus.store( newfocus );
    std::cerr << get_timestamp() << function << "moving focus...  " << newfocus << "\n";

    telescope.focussing.store( false );                                // release thread

    return;
  }
  /***** TcsEmulator::Telescope::do_focus *************************************/


  /***** TcsEmulator::Telescope::do_coords ************************************/
  /**
   * @brief      perform the COORDS command work, which "moves" the telescope
   * @param[in]  telescope  reference to Telescope object
   * @param[in]  args       <ra> <dec> <equinox> <ramotion> <decmotion>
   *
   */
  void Telescope::do_coords( TcsEmulator::Telescope &telescope, std::string args ) {
    std::string function = "  (TcsEmulator::Telescope::do_coords) ";

    double req_ra, req_dec, delta_ra, delta_dec;

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    try {
      // already checked that there are at least 5 tokens but all that matters
      // here is RA and DEC
      //
      req_ra  = std::stod( tokens.at(0) );
      req_dec = std::stod( tokens.at(1) );

      // The name is optional
      //
      if ( tokens.size() > 5 ) telescope.name = tokens.at(5);
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "EXCEPTION: unable to convert one or more values from \"" 
                << args << "\": " << e.what() << "\n";
      return;
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "EXCEPTION: one or more value from \"" 
                << args << "\" is out of range: " << e.what() << "\n";
      return;
    }

    // the current coordinates
    //
    double current_ra  = telescope.ra.load();
    double current_dec = telescope.dec.load();

    // direction to move to reach the requested coordinate from the current coordinate
    //
    double  ra_dir = ( req_ra  - current_ra  < 0 ? -1.0 : 1.0 );
    double dec_dir = ( req_dec - current_dec < 0 ? -1.0 : 1.0 );

    // calculate the slew distance and the slew time for each of RA, DEC
    //
    double slewdistance_ra  = std::abs( telescope.ra  - req_ra  );
    double slewdistance_dec = std::abs( telescope.dec - req_dec );

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
    telescope.telmoving.store( true );

    // begin the (virtual) slew
    // slew rates are specified per second -- divide that by 2 and loop every 1/2 second
    // (for finer granularity)
    //
    telescope.motionstate.store( TCS_MOTION_SLEWING );
    while ( true ) {

      // read the current ra, dec
      //
      current_ra  = telescope.ra.load();
      current_dec = telescope.dec.load();

      std::cerr << get_timestamp() << function << "slewing...  ra " << current_ra << "  dec " << current_dec << "\n";

      // add the slewrate to ra, dec
      // store it permanently as long as we don't overshoot
      //
      delta_ra  = std::abs( current_ra  -  req_ra );
      delta_dec = std::abs( current_dec - req_dec );

      if (  delta_ra >  telescope.slewrate_ra/2.0 ) {
        telescope.ra.store( (  current_ra +  ra_dir *  telescope.slewrate_ra/2.0 ) );
      }
      if ( delta_dec > telescope.slewrate_dec/2.0 ) {
        telescope.dec.store( ( current_dec + dec_dir * telescope.slewrate_dec/2.0 ) );
      }

      std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
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
      current_ra  = telescope.ra.load();
      current_dec = telescope.dec.load();

      std::cerr << get_timestamp() << function << "settling...  ra " << current_ra << "  dec " << current_dec << "\n";

      // add the slewrate to ra, dec
      // store it permanently as long as we don't overshoot
      //
      delta_ra  = std::abs( current_ra  -  req_ra );
      delta_dec = std::abs( current_dec - req_dec );

      if (  delta_ra >  telescope.slewrate_ra/ telescope.settle_ra ) {
        telescope.ra.store( (  current_ra +  ra_dir *  telescope.slewrate_ra/ telescope.settle_ra ) );
      }
      if ( delta_dec > telescope.slewrate_dec/telescope.settle_dec ) {
        telescope.dec.store( ( current_dec + dec_dir * telescope.slewrate_dec/telescope.settle_dec ) );
      }

      std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
      if ( get_clock_time() >= clock_end ) break;
    }

    // after settlting, set the ra,dec to those requested
    //
    telescope.ra.store( req_ra );
    telescope.dec.store( req_dec );

    // clear the moving flag now
    //
    telescope.telmoving.store( false );

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
   * @brief      perform the PT command work, which "offsets" the telescope
   * @details    inputs are in arcsec
   * @param[in]  telescope  reference to Telescope object
   * @param[in]  args       string contains "<raoff> <decoff>" in decimal arcsec
   *
   */
  void Telescope::do_pt( TcsEmulator::Telescope &telescope, std::string args ) {
    std::string function = "  (TcsEmulator::Telescope::do_pt) ";

    double ra_off, dec_off;                 // offsets read in (arcsec)
    double ra_now  = telescope.ra.load();   // (hr)
    double dec_now = telescope.dec.load();  // (deg)

    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    try {
      ra_off  = std::stod( tokens.at(0) ) / 3600.;  // convert from arcsec to deg
      dec_off = std::stod( tokens.at(1) ) / 3600.;
      // If there's an (optional) 3rd argument then that's the offset rate
      // which must be within range to be accepted.
      //
      if ( tokens.size()==3 ) {
        double testrate = std::stod( tokens.at(2) );
        if ( testrate < 1 || testrate > 50 ) {
          std::cerr << get_timestamp() << function << "ERROR offset rate " << testrate << " outside range {1:50}\n";
          return;
        }
        telescope.offsetrate_ra  = testrate;
        telescope.offsetrate_dec = testrate;
      }
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

    // Call FPOffsets::apply_offset_deg() to calculate the new RA, DEC coordinates
    // after applying the offsets.
    // ra_now, dec_now will be modified by the offsets.
    //
    std::cerr << get_timestamp() << function << "[DEBUG] before apply_offset call ra_now=" << ra_now << " dec_now=" << dec_now << "\n";

    ra_now *= 15.0; // convert to degrees

    telescope.fpoffsets.apply_offset( ra_now, ra_off, dec_now, dec_off );

    ra_now /= 15.0;                                // apply_offset returns deg, tcs uses hr so convert from deg to hours

    std::cerr << get_timestamp() << function << "[DEBUG] after apply_offset call newra=" << ra_now << " newdec=" << dec_now << "\n";

    // calculate the offset distance and the offset time for each of RA, DEC
    //
    double offsetdistance_ra  = std::abs( telescope.ra  - ra_now  );
    double offsetdistance_dec = std::abs( telescope.dec - dec_now );

    double offsettime_ra      = offsetdistance_ra  / telescope.offsetrate_ra;
    double offsettime_dec     = offsetdistance_dec / telescope.offsetrate_dec;

    // whichever one is greater is the only one that matters here
    //
    double offsettime = ( offsettime_ra > offsettime_dec ? offsettime_ra : offsettime_dec );

    // get the end time for this slew, which is now plus the longest slew time
    //
    double clock_end = get_clock_time() + offsettime;

    // before starting the slew, set a flag to prevent another thread from also starting a slew
    //
    telescope.telmoving.store( true );

    telescope.motionstate.store( TCS_MOTION_OFFSETTING );

    // Begin the offsetting here, loop at 1kHz, move a little at a time.
    // After this loop, set the telescope position to be the new position.
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
      if ( ( ra  += telescope.offsetrate_ra/1000.0 )  <= ra_now )  telescope.ra.store( ra  );
      if ( ( dec += telescope.offsetrate_dec/1000.0 ) <= dec_now ) telescope.dec.store( dec );

      std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
      if ( get_clock_time() >= clock_end ) break;
    }

    // then set the ra,dec to those requested (because the granularity above is not perfect)
    //
    std::cerr << get_timestamp() << function << "[DEBUG] store in telescope class: newra=" << ra_now << " newdec=" << dec_now << "\n";
    telescope.ra.store( ra_now );
    telescope.dec.store( dec_now );

    // clear the moving flag now
    //
    telescope.telmoving.store( false );

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
        << "Air mass="            << std::fixed << std::setprecision(3) << this->airmass << "\n"
        << "Azimuth="             << "25.6\n"
        << "Zenith angle="        << "30.1\n"
        << "Focus Point="         << this->focus.load() << "\n"
        << "Dome Azimuth="        << "25\n"
        << "Dome shutters="       << "0\n"
        << "Windscreen position=" << "0\n"
        << "InstPos="             << "0\n"
        << "Instrument=NGPS"      << "0\n"
        << "Pumps="               << "0\n";

    retstring = ret.str();

    return;
  }
  /***** TcsEmulator::Telescope::weather **************************************/


  /***** TcsEmulator::Telescope::reqstat **************************************/
  /**
   * @brief      perform the REQSTAT command
   * @param[out] retstring  reference to string contains the return string
   *
   * In UTC, ddd is UT day of year. RA and DEC are J2000. HA is apparent.
   *
   */
  void Telescope::reqstat( std::string &retstring ) {
    std::string function = "  (TcsEmulator::Telescope::reqstat) ";
    std::stringstream ret;

    ret << "UTC = " << this->get_time() << "\n"
        << "telescope ID = " << this->telid << ", focus = " 
        << std::fixed << std::setprecision(2) 
        << this->focus.load() << " mm, tube length = "
        << std::fixed << std::setprecision(2) 
        << this->tubelength << " mm\n"
        << "offset RA = " 
        << this->offset_ra << " arcsec, DEC = "
        << this->offset_dec << " arcsec\n"
        << "rate RA = "
        << this->offsetrate_ra << " arcsec/hr, DEC = "
        << this->offsetrate_dec << " arcsec/hr\n"
        << "Cass ring angle = "
        << std::fixed << std::setprecision(2)
        << this->casangle;

    retstring = ret.str();

    return;
  }
  /***** TcsEmulator::Telescope::reqstat **************************************/


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

    std::cerr << get_timestamp() << function << "read decimal position ra "
              << std::fixed << std::setprecision(9)
              << _ra << "  dec " << _dec << "\n";

    // RA
    //
    std::string ra_sign = (  _ra < 0 ? "-" : "+" );                 // sign of RA
    _ra                 = std::fabs( _ra  ) + 0.000000001;          // double positive rational RA hours
    double ra_whole_hr  = std::trunc( _ra );                        // double whole RA hours
    double ra_minutes   = ( _ra - ra_whole_hr ) * 60.0;             // double rational RA minutes
    double ra_whole_min = std::trunc( ra_minutes );                 // double whole RA minutes
    double ra_seconds   = ( ra_minutes - ra_whole_min ) * 60.0;     // double rational RA seconds

    int ra_hh           = static_cast<int>( ra_whole_hr  );         // RA sexagesimal "HH"
    int ra_mm           = static_cast<int>( ra_whole_min );         // RA sexagesimal "MM"

    if ( ra_seconds >= 59.999 ) {                                   // accommodate rounding errors
      ra_seconds = 0.0;
      ra_minutes += 1.0;
      if ( ra_minutes >= 59.999 ) {
        ra_minutes = 0.0;
        ra_hh += 1;
      }
    }
    ra_mm = static_cast<int>( ra_minutes );

    // DEC
    //
    std::string dec_sign = ( _dec < 0 ? "-" : "+" );                // sign of DEC
    _dec                 = std::fabs( _dec ) + 0.000000001;         // double positive rational DEC degrees
    double dec_whole_deg = std::trunc( _dec );                      // double whole DEC degrees
    double dec_minutes   = ( _dec - dec_whole_deg ) * 60.0;         // double rational DEC minutes
    double dec_whole_min = std::trunc( dec_minutes );               // double whole DEC minutes
    double dec_seconds   = ( dec_minutes - dec_whole_min ) * 60.0;  // double rational DEC seconds

    int dec_dd           = static_cast<int>( dec_whole_deg );       // DEC sexagesimal "DD"
    int dec_mm           = static_cast<int>( dec_whole_min );       // DEC sexagesimal "MM"

    if ( dec_seconds >= 59.999 ) {                                  // accommodate rounding errors
      dec_seconds = 0.0;
      dec_minutes += 1.0;
      if ( dec_minutes >= 59.999 ) {
        dec_minutes = 0.0;
        dec_dd += 1;
      }
    }
    dec_mm = static_cast<int>( dec_minutes );

    // assemble the return string
    //
    ret << "UTC = " << this->get_time() << ", "
        << "LST = 00:00:00\n"
        << "RA = " <<  ra_sign << std::fixed
                   << std::setfill('0') << std::setw(2) << ra_hh << ":"
                   << std::setfill('0') << std::setw(2) << ra_mm << ":"
                   << std::fixed << std::setprecision(3) << std::setw(6)
                   << std::setfill('0') << ra_seconds
        << ", "
        << "DEC = " << dec_sign << std::fixed
                    << std::setfill('0') << std::setw(2) << dec_dd  << ":"
                    << std::setfill('0') << std::setw(2) << dec_mm  << ":"
                    << std::fixed << std::setprecision(3) << std::setw(6)
                    << std::setfill('0') << dec_seconds << ", "
        << "HA=W00:00:00.0\n"
        << "air mass = " << std::fixed << std::setprecision(3) << this->airmass;

    retstring = ret.str();

    return;
  }
  /***** TcsEmulator::Telescope::reqpos ***************************************/


  /***** TcsEmulator::Interface::parse_command ********************************/
  /**
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
  long Interface::parse_command( const std::string cmd, std::string &retstring ) {
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

    bool check_retval = true;  // check retval unless otherwise set false

    if ( mycmd == "?MOTION" ) {
      retstring = std::to_string( this->telescope.motionstate.load() );  // return the current motion state
      std::cerr << get_timestamp() << function << "reply from TCS emulator: " 
                                               << this->map_motionval[ this->telescope.motionstate.load() ] << "\n";
    }
    else {

    if ( mycmd == "MRATES" ) {
      this->telescope.mrates( myargs.str(), retstring );
      retstring = "0";                 // successful completion
    }
    else
    if ( mycmd == "COORDS" ) {
      // can only run one of these threads at a time
      //
      if ( this->telescope.telmoving.load() ) {
        std::cerr << get_timestamp() << function << "ERROR: telescope is already moving\n";
        retstring = "-3";              // unable to execute at this time
      }
      else
      if ( nargs < 5 ) {
        std::cerr << get_timestamp() << function << "ERROR: COORDS expected minimum 5 args but received " << nargs << "\n";
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
    if ( mycmd == "RINGGO" ) {
      // can only run one of these threads at a time
      //
      if ( this->telescope.casmoving.load() ) {
        std::cerr << get_timestamp() << function << "ERROR: cass rotator is already moving\n";
        retstring = "-3";              // unable to execute at this time
      }
      else
      if ( nargs != 1 ) {
        std::cerr << get_timestamp() << function << "ERROR: RINGGO expected 1 arg but received " << nargs << "\n";
        retstring = "-2";              // invalid parameters
      }
      else {
        double newring;
        try { newring = std::stod( myargs.str() ); }
        catch( std::invalid_argument &e ) {
          std::cerr << get_timestamp() << function << "EXCEPTION: invalid argument parsing \"" 
                    << myargs.str() << "\" : " << e.what() << "\n";
          retstring = "-2";
          newring = NAN;
        }
        catch( std::out_of_range &e ) {
          std::cerr << get_timestamp() << function << "EXCEPTION: out of range parsing \"" 
                    << myargs.str() << "\" : " << e.what() << "\n";
          retstring = "-2";
          newring = NAN;
        }

        if ( std::isnan(newring) || newring < 0 || newring > 359.99 ) {
          std::cerr << get_timestamp() << function << "ERROR: requested ring angle " << newring << " not in range {0:359.99}\n";
          retstring = "-2";
        }
        else {
          std::thread( std::ref( TcsEmulator::Telescope::do_ringgo ), 
                       std::ref( this->telescope ),
                       newring ).detach();
          retstring = "0";             // successful completion
        }
      }
    }
    else
    if ( mycmd == "FOCUSGO" ) {
      // can only run one of these threads at a time
      //
      if ( this->telescope.focussing.load() ) {
        std::cerr << get_timestamp() << function << "ERROR: focus is already moving\n";
        retstring = "-3";              // unable to execute at this time
      }
      else
      if ( nargs != 1 ) {
        std::cerr << get_timestamp() << function << "ERROR: FOCUSGO expected 1 arg but received " << nargs << "\n";
        retstring = "-2";              // invalid parameters
      }
      else {
        double newfocus;
        try { newfocus = std::stod( myargs.str() ); }
        catch( std::invalid_argument &e ) {
          std::cerr << get_timestamp() << function << "EXCEPTION: invalid argument parsing \"" 
                    << myargs.str() << "\" : " << e.what() << "\n";
          retstring = "-2";
          newfocus = NAN;
        }
        catch( std::out_of_range &e ) {
          std::cerr << get_timestamp() << function << "EXCEPTION: out of range parsing \"" 
                    << myargs.str() << "\" : " << e.what() << "\n";
          retstring = "-2";
          newfocus = NAN;
        }

        if ( std::isnan(newfocus) || newfocus < 1.00 || newfocus > 74.00 ) {
          std::cerr << get_timestamp() << function << "ERROR: requested focus " << newfocus << " not in range {1:74}\n";
          retstring = "-2";
        }
        else {
          std::thread( std::ref( TcsEmulator::Telescope::do_focus ), 
                       std::ref( this->telescope ),
                       newfocus,
                       MOVETYPE_FOCUS_GO ).detach();
          retstring = "0";             // successful completion
        }
      }
    }
    else
    if ( mycmd == "FOCUSINC" ) {
      // can only run one of these threads at a time
      //
      if ( this->telescope.focussing.load() ) {
        std::cerr << get_timestamp() << function << "ERROR: focus is already moving\n";
        retstring = "-3";              // unable to execute at this time
      }
      else
      if ( nargs != 1 ) {
        std::cerr << get_timestamp() << function << "ERROR: FOCUSINC expected 1 arg but received " << nargs << "\n";
        retstring = "-2";              // invalid parameters
      }
      else {
        double focusinc;
        try { focusinc = std::stod( myargs.str() ); }
        catch( std::invalid_argument &e ) {
          std::cerr << get_timestamp() << function << "EXCEPTION: invalid argument parsing \"" 
                    << myargs.str() << "\" : " << e.what() << "\n";
          retstring = "-2";
          focusinc = NAN;
        }
        catch( std::out_of_range &e ) {
          std::cerr << get_timestamp() << function << "EXCEPTION: out of range parsing \"" 
                    << myargs.str() << "\" : " << e.what() << "\n";
          retstring = "-2";
          focusinc = NAN;
        }

        double current_focus = telescope.focus.load();                 // read the current focus
        double newfocus = current_focus + focusinc;
        if ( std::isnan(newfocus) || newfocus < 1.00 || newfocus > 74.00 ) {
          std::cerr << get_timestamp() << function << "ERROR: requested focus inc " << focusinc 
                                                   << " would result in " << newfocus << " out of range {1:74}\n";
          retstring = "-2";
        }
        else {
          std::thread( std::ref( TcsEmulator::Telescope::do_focus ), 
                       std::ref( this->telescope ),
                       focusinc,
                       MOVETYPE_FOCUS_INC ).detach();
          retstring = "0";             // successful completion
        }
      }
    }
    else
    if ( mycmd == "PT" ) {
      // can only run one of these threads at a time
      //
      if ( this->telescope.telmoving.load() ) {
        std::cerr << get_timestamp() << function << "ERROR: telescope is already movin\n";
        retstring = "-3";              // unable to execute at this time
      }
      else
      if ( nargs < 2 ) {
        std::cerr << get_timestamp() << function << "ERROR need at least 2 args but received " << nargs << "\n";
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
    if ( mycmd == "RET" ) {
      retstring = "0";                 // successful completion
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
      check_retval = false;
    }
    else
    if ( mycmd == "REQSTAT" ) {
      this->telescope.reqstat( retstring );
      check_retval = false;
    }
    else
    if ( mycmd == "?NAME" ) {
      retstring = this->telescope.name;
      check_retval = false;
    }
    else
    if ( mycmd == "?WEATHER" ) {
      this->telescope.weather( retstring );
      check_retval = false;
    }
    else {
      std::cerr << get_timestamp() << function << "ERROR: unknown command " << cmd << "\n";
      retstring = "-1";                // unrecognized command
    }

    if ( check_retval ) {
      try {
        int retval=std::stoi( retstring );
        std::cerr << get_timestamp() << function << "reply from TCS emulator: " << this->map_returnval[ retval ] << "\n";
      }
      catch( std::invalid_argument &e ) {
        std::cerr << get_timestamp() << function << "EXCEPTION invalid argument parsing reply from TCS emulator \"" << retstring << "\" : " << e.what() << "\n";
      }
      catch( std::out_of_range &e ) {
        std::cerr << get_timestamp() << function << "EXCEPTION out of range parsing reply from TCS emulator \"" << retstring << "\" : " << e.what() << "\n";
      }
    }

    }

    return ( NO_ERROR );
  }
  /***** TcsEmulator::Interface::parse_command ********************************/


}
