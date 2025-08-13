#include "astrocam_interface.h"
#include "camera_server.h"

namespace Camera {

  AstroCamInterface::AstroCamInterface() {
    this->state_monitor_thread_running = false;
    this->pci_cmd_num.store(0);
    this->nexp=1;
    this->numdev = 0;
    this->nframes = 1;
    this->is_useframes = true;
    this->use_bonn_shutter = true;  // defaults to Bonn shutter present
    this->use_ext_shutter = false;  // defaults to external shutter not present
    this->framethreadcount = 0;

    this->pFits.resize( NUM_EXPBUF );           // pre-allocate FITS_file object pointers for each exposure buffer
    this->fitsinfo.resize( NUM_EXPBUF );        // pre-allocate Camera Information object pointers for each exposure buffer

    this->writes_pending.resize( NUM_EXPBUF );  // pre-allocate writes_pending vector for each exposure buffer
  }

  AstroCamInterface::~AstroCamInterface() = default;


  /***** AstroCam::Interface::add_framethread *********************************/
  /**
   * @brief      call on thread creation to increment framethreadcount
   *
   */
  void AstroCamInterface::add_framethread() {
    std::lock_guard<std::mutex> lock(this->framethreadcount_mutex);
    this->framethreadcount++;
  }
  /***** AstroCam::Interface::add_framethread *********************************/


  /***** AstroCam::Interface::remove_framethread ******************************/
  /**
   * @brief      call on thread destruction to decrement framethreadcount
   *
   */
  void AstroCamInterface::remove_framethread() {
    std::lock_guard<std::mutex> lock(this->framethreadcount_mutex);
    this->framethreadcount--;
  }
  /***** AstroCam::Interface::remove_framethread ******************************/


  /***** AstroCam::Interface::get_framethread *********************************/
  /**
   * @brief      return the number of active threads spawned for handling frames
   * @return     number of threads
   *
   */
  int AstroCamInterface::get_framethread_count() {
    int count;
    std::lock_guard<std::mutex> lock(this->framethreadcount_mutex);
    count = this->framethreadcount;
    return count;
  }
  /***** AstroCam::Interface::get_framethread *********************************/


  /***** Camera::AstroCamInterface::init_framethread_count ********************/
  /**
   * @brief      initialize framethreadcount = 0
   *
   */
  void AstroCamInterface::init_framethread_count() {
    std::lock_guard<std::mutex> lock(this->framethreadcount_mutex);
    this->framethreadcount = 0;
  }
  /***** Camera::AstroCamInterface::init_framethread_count ********************/


  /***** Camera::AstroCamInterface::abort *************************************/
  /**
   * @brief
   * @param[in]  args
   * @param[out] retstring
   * @return     ERROR | NO_ERROR
   *
   */
  long AstroCamInterface::abort( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::abort");
    logwrite(function, "not yet implemented");
    return ERROR;
  }
  /***** Camera::AstroCamInterface::abort *************************************/


  /***** Camera::AstroCamInterface::autodir ***********************************/
  /**
   * @brief
   * @param[in]  args
   * @param[out] retstring  
   * @return     ERROR | NO_ERROR 
   *
   */
  long AstroCamInterface::autodir( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::autodir");
    logwrite(function, "not yet implemented");
    return ERROR;
  } 
  /***** Camera::AstroCamInterface::autodir ***********************************/


  /***** Camera::AstroCamInterface::bias **************************************/
  /**
   * @brief      set a bias voltage
   * @details    not implemented for AstroCam
   * @param[in]  args       contains <module> <chan> <voltage>
   * @param[out] retstring  
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long AstroCamInterface::bias( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::bias");
    logwrite(function, "ERROR not implemented");
    return ERROR;
  }
  /***** Camera::AstroCamInterface::bias **************************************/


  /***** Camera::AstroCamInterface::bin ***************************************/
  /**
   * @brief      set binning
   * @param[in]  args       
   * @param[out] retstring  
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long AstroCamInterface::bin( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::bin");
    logwrite(function, "not yet implemented");
    return ERROR;
  }
  /***** Camera::AstroCamInterface::bin ***************************************/


  /***** Camera::AstroCamInterface::buffer ************************************/
  /**
   * @brief      set/get mapped PCI image buffer
   * @details    This function uses the ARC API to allocate PCI/e buffer space
   *             for doing the DMA transfers.
   * @param[in]  args      string containing <dev>|<chan> [ <bytes> | <rows> <cols> ]
   * @param[out] retstring reference to a string for return values
   * @return     ERROR | NO_ERROR | HELP
   *
   * Function returns only ERROR or NO_ERROR on error or success. The 
   * reference to retstring is used to return the size of the allocated
   * buffer.
   *
   * Since this allocates the amount of memory needed for DMA transfer, it
   * must therefore include the total number of pixels, I.E. overscans too.
   *
   */
  long AstroCamInterface::buffer( std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::buffer");
    std::stringstream message;
    uint32_t try_bufsize=0;

    // Help
    //
    if ( args == "?" || args=="help" ) {
      retstring = CAMERAD_BUFFER;
      retstring.append( " <chan> | <dev#> [ <bytes> | <rows> <cols> ]\n" );
      retstring.append( "  Allocate PCI buffer space for performing DMA transfers for specified device.\n" );
      retstring.append( "  Provide either a single value in <bytes> or two values as <rows> <cols>.\n" );
      retstring.append( "  If no args supplied then buffer size for dev#|chan is returned (in Bytes).\n" );
      retstring.append( "  Specify <chan> from { " );
      message.str("");
      for ( const auto &con : this->controller ) {
        if ( con.second.inactive ) continue;  // skip controllers flagged as inactive
        message << con.second.channel << " ";
      }
      message << "}\n";
      retstring.append( message.str() );
      retstring.append( "       or <dev#> from { " );
      message.str("");
      for ( const auto &con : this->controller ) {
        if ( con.second.inactive ) continue;  // skip controllers flagged as inactive
        message << con.second.devnum << " ";
      }
      message << "}\n";
      retstring.append( message.str() );
      return HELP;
    }

    // If no connected devices then nothing to do here
    //
    if ( this->numdev == 0 ) {
      logwrite(function, "ERROR: no connected devices");
      retstring="not_connected";
      return ERROR;
    }

    int dev;
    std::string chan;

    long error = this->extract_dev_chan( args, dev, chan, retstring );

    if ( chan.empty() ) chan = "(empty)";

    auto it = this->controller.find( dev );

    if ( it == this->controller.end() ) {
      message.str(""); message << "ERROR dev# " << dev << " for chan " << chan << " not configured. "
                               << "Expected <chan> | <dev#> [ <bytes> | <rows> <cols> ]";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    auto &con = it->second;  // need reference so that values can be modified here

    if ( error != NO_ERROR ) return error;

    // If there is a buffer or array size provided then parse it,
    // then try to map the buffer size.
    //
    if ( ! retstring.empty() ) {

      // Still have to tokenize retstring now, to get the requested buffer size.
      //
      // retstring can contain 1 value to allocate that number of bytes
      // retstring can contain 2 values to allocate a buffer of rows x cols
      // Anything other than 1 or 2 tokens is invalid.
      //
      std::vector<std::string> tokens;
      switch( Tokenize( retstring, tokens, " " ) ) {
        case 1:  // allocate specified number of bytes
                 try_bufsize = (uint32_t)parse_val( tokens.at(0) );
                 break;
        case 2:  // allocate (col x row) bytes
                 try_bufsize = (uint32_t)parse_val(tokens.at(0)) * (uint32_t)parse_val(tokens.at(1)) * sizeof(uint16_t);
                 break;
        default: // bad
                 message.str(""); message << "ERROR: invalid arguments: " << retstring << ": expected <bytes> or <rows> <cols>";
                 logwrite(function, message.str());
                 retstring="invalid_argument";
                 return ERROR;
                 break;
      }

      // Now try to re-map the buffer size for the requested device.
      //
      try {
        con.pArcDev->reMapCommonBuffer( try_bufsize );
      }
      catch( const std::exception &e ) {
        message.str(""); message << "ERROR mapping buffer for dev " << dev << " chan " << chan << ": " << e.what();
        logwrite(function, message.str());
        retstring="arc_exception";
        return ERROR;
      }
      catch(...) {
        message.str(""); message << "ERROR unknown exception mapping buffer for dev " << dev << " chan " << chan;
        logwrite(function, message.str());
        retstring="exception";
        return ERROR;
      }

      con.set_bufsize( try_bufsize );  // save the new buffer size to the controller map for this dev
    }  // end if ! retstring.empty()

    retstring = std::to_string( con.get_bufsize() );

//  message.str(""); message << "[DEBUG] allocated " << retstring << " bytes"; logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::buffer ************************************/


  /***** Camera::AstroCamInterface::connect_controller ************************/
  /**
   * @brief
   *
   */
  long AstroCamInterface::connect_controller( const std::string args, std::string &retstring ) {
    std::string function("Camera::AstroCamInterface::do_connect_controller");
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = CAMERAD_OPEN;
      retstring.append( " [ <devnums> ]\n" );
      retstring.append( "  Opens a connection to the indicated PCI/e device(s) where <devnums>\n" );
      retstring.append( "  is an optional space-delimited list of device numbers.\n" );
      retstring.append( "  e.g. \"0 1\" to open PCI devices 0 and 1\n" );
      retstring.append( "  If no list is provided then all detected devices will be opened.\n" );
      retstring.append( "  Opening an ARC device requires that the controller is present and powered on.\n" );
      return HELP;
    }

    // Find the installed devices
    //
    arc::gen3::CArcPCI::findDevices();

    this->numdev = arc::gen3::CArcPCI::deviceCount();

    // Nothing to do if there are no devices detected.
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR no PCI devices found");
      retstring="no_devices";
      return ERROR;
    }

    // Log all PCI devices found
    //
    const std::string* pdevNames;                                             // pointer to device list returned from ARC API
    std::vector<std::string> devNames;                                        // my local copy so I can manipulate it
    pdevNames = arc::gen3::CArcPCI::getDeviceStringList();
    for ( uint32_t i=0; i < arc::gen3::CArcPCI::deviceCount(); i++ ) {
      if ( !pdevNames[i].empty() ) {
        devNames.push_back( pdevNames[i].substr(0, pdevNames[i].size()-1) );  // throw out last character (non-printing)
      }
      message.str(""); message << "found " << devNames.back();
      logwrite(function, message.str());
    }

    // Log PCI devices configured
    //
    if ( this->configured_devnums.empty() ) {
      logwrite( function, "ERROR: no devices configured. Need CONTROLLER keyword in config file." );
      retstring="not_configured";
      return ERROR;
    }

    for ( const auto &dev : this->configured_devnums ) {
      message.str(""); message << "device " << dev << " configured";
      logwrite( function, message.str() );
    }

    // Look at the requested device(s) to open, which are in the
    // space-delimited string args. The devices to open are stored
    // in a public vector "devnums".
    //

    // If no string is given then use vector of configured devices. The configured_devnums
    // vector contains a list of devices defined in the config file with the
    // keyword CONTROLLER=(<PCIDEV> <CHAN> <FT> <FIRMWARE>).
    //
    if ( args.empty() ) this->devnums = this->configured_devnums;
    else {
      // Otherwise, tokenize the device list string and build devnums from the tokens
      //
      this->devnums.clear();                          // empty devnums vector since it's being built here
      std::vector<std::string> tokens;
      Tokenize(args, tokens, " ");
      for ( const auto &n : tokens ) {                // For each token in the args string,
        try {
          int dev = std::stoi( n );                   // convert to int
          if ( std::find( this->devnums.begin(), this->devnums.end(), dev ) == this->devnums.end() ) { // If it's not already in the vector,
            this->devnums.push_back( dev );                                                            // then push into devnums vector.
          }
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR parsing device number " << n << ": " << e.what();
          logwrite(function, message.str());
          retstring="invalid_argument";
          return ERROR;
        }
        catch(...) {
          logwrite(function, "ERROR unknown exception getting device number");
          retstring="exception";
          return ERROR;
        }
      }
    }

    // For each requested dev in devnums, if there is a matching controller in the config file,
    // then get the devname and store it in the controller map.
    //
    for ( const auto &dev : this->devnums ) {
      if ( this->controller.find( dev ) != this->controller.end() ) {
        this->controller[ dev ].devname = devNames[dev];
      }
    }

    // The size of devnums at this point is the number of devices that will
    // be _requested_ to be opened. This should match the number of opened
    // devices at the end of this function.
    //
    size_t requested_device_count = this->devnums.size();

    /*** to be completed ***/

    return error;
  }
  /***** Camera::AstroCamInterface::connect_controller ************************/


  /***** Camera::AstroCamInterface::exposure_pending_list *********************/
  /**
   * @brief      returns the exposure_pending vector
   * @return     std::vector<int>
   */
  std::vector<int> AstroCamInterface::exposure_pending_list() {
    std::lock_guard<std::mutex> lock( this->pending_mtx );
    return this->exposures_pending;
  }
  /***** Camera::AstroCamInterface::exposure_pending_list *********************/


  /***** Camera::AstroCamInterface::exposure_pending **************************/
  /**
   * @brief      Is an exposure pending or can a new exposure be started?
   * @returns    bool { true | false }
   *
   * This function is overloaded with a version that allows
   * setting the exposure_pending state.
   *
   */
  bool AstroCamInterface::exposure_pending() {
    std::lock_guard<std::mutex> lock( this->pending_mtx );
    return ( this->exposures_pending.size() > 0 ? true : false );
  }
  /***** Camera::AstroCamInterface::exposure_pending **************************/


  /***** Camera::AstroCamInterface::exposure_pending **************************/
  /**
   * @brief      Set or clear the exposure pending state for a given controller
   * @param[in]  devnum  controller device number
   * @param[in]  add     bool true to add, false to remove from pending vector
   *
   * This function is overloaded with a version that returns a bool
   * to indicate if there are any controllers with a pending exposure.
   *
   */
  void AstroCamInterface::exposure_pending( int devnum, bool add ) {
    std::lock_guard<std::mutex> lock(this->pending_mtx);       // protect exposures_pending vector

    auto it     = std::find( this->exposures_pending.begin(),
                             this->exposures_pending.end(),
                             devnum );                         // iterator of devnum in exposures_pending vector
    bool found  = ( it != this->exposures_pending.end() );     // was devnum found in vector?
    bool remove = !add;                                        // do I remove it?

    if ( add && !found )   { this->exposures_pending.push_back( devnum ); }
    else
    if ( found && remove ) { this->exposures_pending.erase( it ); }

    this->exposure_condition.notify_all();

    return;                                                    // lock_guard releases on exit
  }
  /***** Camera::AstroCamInterface::exposure_pending **************************/


  /**** Camera::AstroCamInterface::exposure_progress **************************/
  /**
   * @brief      polls and broadcasts shutter timer progress
   *
   */
  void AstroCamInterface::exposure_progress() {
    long remaintime=0, delaytime=0;
    std::string message;
    message.reserve(32);
    do {
      // poll and report once per second
      std::this_thread::sleep_for( std::chrono::seconds(1) );
      // poll
      this->shutter_timer.progress(remaintime, delaytime);
      if ( delaytime==0 ) return;
      message = "EXPTIME:" + std::to_string(remaintime) + " "
                           + std::to_string(delaytime)  + " "
                           + std::to_string(static_cast<long>(100*(1-static_cast<double>(remaintime)/delaytime)));
      // broadcast
      std::thread( &Camera::AstroCamInterface::handle_queue, this, std::move(message) ).detach();
    }
    while ( remaintime > 0 );
  }
  /**** Camera::AstroCamInterface::exposure_progress **************************/


  /***** Camera::AstroCamInterface::extract_dev_chan **************************/
  /**
   * @brief      extract a dev#, channel name, and optional string from provided args
   * @details    The dev# | chan must exist in a configured controller object.
   *             The additional string is optional and can be anything.
   * @param[in]  args       expected: <dev#>|<chan> [ <string> ]
   * @param[out] dev        reference to returned dev#
   * @param[out] chan       reference to returned channel
   * @param[out] retstring  carries error message, or any remaining strings, if present
   *
   */
  long AstroCamInterface::extract_dev_chan( std::string args, int &dev, std::string &chan, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::extract_dev_chan");
    std::stringstream message;

    // make sure input references are initialized
    //
    dev=-1;
    chan.clear();
    retstring.clear();

    std::vector<std::string> tokens;
    std::string tryme;                  // either a dev or chan, TBD

    // Tokenize args to extract requested <dev>|<chan> and <string>, as appropriate
    //
    Tokenize( args, tokens, " " );
    size_t ntok = tokens.size();
    if ( ntok < 1 ) {                   // need at least one token
      logwrite( function, "ERROR: bad arguments. expected <dev> | <chan> [ <string> ]" );
      retstring="invalid_argument";
      return ERROR;
    }
    else
    if ( ntok == 1 ) {                  // dev|chan only
      tryme = tokens[0];
    }
    else {                              // dev|chan plus string
      tryme = tokens[0];
      retstring.clear();
      for ( size_t i=1; i<ntok; i++ ) {   // If more than one token in string, then put
        retstring.append( tokens[i] );  // them all back together and return as one string.
        retstring.append( ( i+1 < ntok ? " " : "" ) );
      }
    }

    // Try to convert the "tryme" to integer. If successful then it's a dev#,
    // and if that fails then check if it's a known channel.
    //
    try {
      dev = std::stoi( tryme );   // convert to integer
      if ( dev < 0 ) {            // don't let the user input a negative number
        logwrite( function, "ERROR: dev# must be >= 0" );
        retstring="invalid_argument";
        return ERROR;
      }
    }
    catch ( std::out_of_range & ) { logwrite( function, "ERROR: out of range" ); retstring="exception"; return ERROR; }
    catch ( std::invalid_argument & ) { }    // ignore this for now, it just means "tryme" is not a number

    // If the stoi conversion failed then we're out here with a negative dev.
    // If it succeeded then all we have is a number >= 0.
    // But now check that either the dev# is a known devnum or the tryme is a known channel.
    //
    for ( const auto &con : this->controller ) {
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] con.first=" << con.first << " con.second.channel=" << con.second.channel
                               << " .devnum=" << con.second.devnum << " .inactive=" << (con.second.inactive?"T":"F");
      logwrite( function, message.str() );
#endif
      if ( con.second.inactive ) continue;   // skip controllers flagged as inactive
      if ( con.second.channel == tryme ) {   // check to see if it matches a configured channel.
        dev  = con.second.devnum;
        chan = tryme;
        break;
      }
      if ( con.second.devnum == dev ) {      // check for a known dev#
        chan = con.second.channel;
        break;
      }
    }

    // By now, these must both be known.
    //
    if ( dev < 0 || chan.empty() || this->controller.find(dev)==this->controller.end() ) {
      message.str(""); message << "unrecognized channel or device \"" << tryme << "\"";
      logwrite( function, message.str() );
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dev=" << dev << " chan=" << chan << " controller.find(dev)==controller.end ? "
                               << ((this->controller.find(dev)==this->controller.end()) ? "true" : "false" );
      logwrite( function, message.str() );
#endif
      retstring="invalid_argument";
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::extract_dev_chan **************************/


  /***** Camera::AstroCamInterface::geometry **********************************/
  /**
   * @brief      set/get geometry
   * @param[in]  args       contains <dev>|<chan> [ <rows> <cols> ]
   * @param[out] retstring  reference to string for return value
   * @return     ERROR | NO_ERROR | HELP
   *
   * The <rows> <cols> set here is the total number of rows and cols that
   * will be read, therefore this includes any pre/overscan pixels.
   *
   */
  long AstroCamInterface::geometry(std::string args, std::string &retstring) {
    const std::string function("Camera::AstroCamInterface::geometry");
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args=="help" ) {
      retstring = CAMERAD_GEOMETRY;
      retstring.append( " <chan> | <dev#> [ <rows> <cols> ]\n" );
      retstring.append( "  Configures geometry of the detector for the specified device, including\n" );
      retstring.append( "  any overscans. In other words, these are the number of rows and columns that\n" );
      retstring.append( "  will be read out. Camera controller connection must first be open.\n" );
      retstring.append( "  If no args are supplied then the current geometry is returned.\n" );
      retstring.append( "  Specify <chan> from { " );
      message.str("");
      for ( const auto &con : this->controller ) {
        if ( con.second.inactive ) continue;  // skip controllers flagged as inactive
        message << con.second.channel << " ";
      }
      message << "}\n";
      retstring.append( message.str() );
      retstring.append( "       or <dev#> from { " );
      message.str("");
      for ( const auto &con : this->controller ) {
        if ( con.second.inactive ) continue;  // skip controllers flagged as inactive
        message << con.second.devnum << " ";
      }
      message << "}\n";
      retstring.append( message.str() );
      return HELP;
    }

    // If no connected devices then nothing to do here
    //
    if ( this->numdev == 0 ) {
      logwrite(function, "ERROR: no connected devices");
      retstring="not_connected";
      return( ERROR );
    }

    int dev;
    std::string chan;

    if ( this->extract_dev_chan( args, dev, chan, retstring ) != NO_ERROR ) return ERROR;

    // If geometry was in args then extract_dev_chan() put it in the retstring.
    // If geometry was not supplied then retstring is empty.
    // Tokenize retstring to get the rows and cols. There can be
    // either 2 tokens (set geometry) or 0 tokens (get geometry).
    //
    std::vector<std::string> tokens;
    Tokenize( retstring, tokens, " " );

    if ( tokens.size() == 2 ) {
      int setrows=-1;
      int setcols=-1;
      try {
        setrows = std::stoi( tokens.at( 0 ) );
        setcols = std::stoi( tokens.at( 1 ) );
      }
      catch ( std::exception &e ) {
        message.str(""); message << "ERROR: converting " << args << " to integer: " << e.what();
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return( ERROR );
      }

      if ( setrows < 1 || setcols < 1 ) {
        logwrite( function, "ERROR: rows cols must be > 0" );
        retstring="invalid_argument";
        return( ERROR );
      }

//    message.str(""); message << "[DEBUG] " << this->controller[dev].devname
//                                           << " chan " << this->controller[dev].channel << " rows:" << setrows << " cols:" << setcols;
//    logwrite( function, message.str() );

      // Write the geometry to the selected controllers
      //
      std::stringstream cmd;

      cmd.str(""); cmd << "WRM 0x400001 " << setcols;  // NSR
      if ( this->do_native( dev, cmd.str(), retstring ) != NO_ERROR ) return ERROR;

      cmd.str(""); cmd << "WRM 0x400002 " << setrows;  // NPR
      if ( this->do_native( dev, cmd.str(), retstring ) != NO_ERROR ) return ERROR;
    }
    else if ( tokens.size() != 0 ) {                   // some other number of args besides 0 or 2 is confusing
      message.str(""); message << "ERROR: expected [ rows cols ] but received \"" << retstring << "\"";
      logwrite( function, message.str() );
      retstring="bad_arguments";
      return( ERROR );
    }

    // Now read back the geometry from each controller
    //
    std::stringstream rs;
    std::stringstream cmd;
    std::string getrows, getcols;

    // Return value from this->do_native() is of the form "dev:0xVALUE"
    // so must parse the hex value after the colon.
    //
    cmd.str(""); cmd << "RDM 0x400001 ";
    if ( this->do_native( dev, cmd.str(), getcols ) != NO_ERROR ) return ERROR;
    this->controller[dev].cols = (uint32_t)parse_val( getcols.substr( getcols.find(":")+1 ) );

    cmd.str(""); cmd << "RDM 0x400002 ";
    if ( this->do_native( dev, cmd.str(), getrows ) != NO_ERROR ) return ERROR;
    this->controller[dev].rows = (uint32_t)parse_val( getrows.substr( getrows.find(":")+1 ) );

    rs << this->controller[dev].rows << " " << this->controller[dev].cols;

    retstring = rs.str();    // Form the return string from the read-back rows cols

    return( NO_ERROR );
  }
  /***** Camera::AstroCamInterface::geometry **********************************/


  /***** Camera::AstroCamInterface::handle_frame ******************************/
  /**
   * @brief      process frame received by frameCallback for specified device
   * @param[in]  expbuf    exposure buffer number
   * @param[in]  devnum    controller device number
   * @param[in]  fpbcount  frames per buffer count, returned from the ARC API
   * @param[in]  fcount    number of frames read, returned from the ARC API
   * @param[in]  buffer    pointer to the PCI/e buffer, returned from the ARC API
   *
   * This function is static, spawned in a thread created by frameCallback()
   * which is the callback fired by the ARC API when a frame has been received.
   *
   * incoming frame from ARC -> frameCallback() -> handle_frame()
   *
   */
  void AstroCamInterface::handle_frame( int expbuf, int devnum, uint32_t fpbcount, uint32_t fcount, void* buffer ) {
    const std::string function("Camera::AstroCamInterface::handle_frame");
    std::stringstream message;
    int error = NO_ERROR;

#ifdef LOGLEVEL_DEBUG
    message << "[DEBUG] expbuf=" << expbuf << " devnum=" << devnum << " " << "fpbcount=" << fpbcount
            << " fcount=" << fcount << " PCI buffer=" << std::hex << std::uppercase << buffer;
    logwrite(function, message.str());
#endif

    frameinfo_mutex.lock();                 // protect access to frameinfo structure

    // Check to see if there is another frame with the same fpbcount...
    //
    // fpbcount is the frame counter that counts from 0 to FPB, so if there are 3 Frames Per Buffer
    // then for 10 frames, fcount goes from 0,1,2,3,4,5,6,7,8,9
    // and fpbcount goes from 0,1,2,0,1,2,0,1,2,0
    //
    // When useframes is false, fpbcount=0, fcount=0, framenum=0
    //
    if ( ! controller[devnum].have_ft ) {
      exposure_pending( devnum, false );    // this also does the notify
      state_monitor_condition.notify_all();
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dev " << devnum << " chan " << controller[devnum].channel << " exposure_pending=false";
      async.enqueue_and_log( "CAMERAD", function, message.str() );
#endif
    }
    controller[devnum].in_readout = false;
    state_monitor_condition.notify_all();
#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] dev " << devnum << " chan " << controller[devnum].channel << " in_readout=false";
    async.enqueue_and_log( "CAMERAD", function, message.str() );
#endif

    controller[devnum].frameinfo[fpbcount].tid    = fpbcount;  // create this index in the .frameinfo[] map
    controller[devnum].frameinfo[fpbcount].buf    = buffer;

/***
    if ( controller[devnum].frameinfo.count( fpbcount ) == 0 ) {     // searches .frameinfo[] map for an index of fpbcount (none)
      controller[devnum].frameinfo[ fpbcount ].tid      = fpbcount;  // create this index in the .frameinfo[] map
      controller[devnum].frameinfo[ fpbcount ].buf      = buffer;
      // If useframes is false then set framenum=0 because it doesn't mean anything,
      // otherwise set it to the fcount received from the API.
      //
      controller[devnum].frameinfo[ fpbcount ].framenum = useframes ? fcount : 0;
    }
    else {                                                                  // already have this fpbcount in .frameinfo[] map
      message.str(""); message << "ERROR: frame buffer overrun! Try allocating a larger buffer."
                               << " chan " << controller[devnum].channel;
      logwrite( function, message.str() );
      frameinfo_mutex.unlock();
      return;
    }
***/

    frameinfo_mutex.unlock();               // release access to frameinfo structure

/***
    // Write the frames in numerical order.
    // Don't let one thread get ahead of another when it comes to writing.
    //
    double start_time = get_clock_time();
    do {
      int this_frame = fcount;                     // the current frame
      int last_frame = controller[devnum].get_framecount();    // the last frame that has been written by this device
      int next_frame = last_frame + 1;             // the next frame in line
      if (this_frame != next_frame) {              // if the current frame is NOT the next in line then keep waiting
        usleep(5);
        double time_now = get_clock_time();
        if (time_now - start_time >= 1.0) {        // update progress every 1 sec so the user knows what's going on
          start_time = time_now;
          logwrite(function, "waiting for frames");///< TODO @todo presumably update GUI or set a global that the CLI can access
#ifdef LOGLEVEL_DEBUG
          message.str(""); message << "[DEBUG] this_frame=" << this_frame << " next_frame=" << next_frame;
          logwrite(function, message.str());
#endif
        }
      }
      else {                                       // otherwise it's time to write this frame to disk
        break;                                     // note that frameinfo_mutex remains locked now until completion
      }

      if ( abortstate() ) break; // provide the user a way to get out

    } while ( useframes );                  // some firmware doesn't support frames so get out after one frame if it doesn't
***/

    // If not aborted then write this frame
    //
    if ( ! abortstate() ) {
#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] calling write_frame for devnum=" << devnum << " fpbcount=" << fpbcount;
    logwrite(function, message.str());
#endif
      error = write_frame( expbuf, devnum, controller[devnum].channel, fpbcount );
    }
    else {
      logwrite(function, "aborted!");
    }

    if ( error != NO_ERROR ) {
      message.str(""); message << "ERROR writing frame " << fcount << " for devnum=" << devnum;
      logwrite( function, message.str() );
    }

    // Done with the frame identified by "fpbcount".
    // Erase it from the STL map so it's not seen again.
    //
    frameinfo_mutex.lock();                 // protect access to frameinfo structure
//  controller[devnum].frameinfo.erase( fpbcount );

/*** 10/30/23 BOB
    controller[devnum].close_file( camera.writekeys_when );
***/

    frameinfo_mutex.unlock();

    remove_framethread();  // framethread_count is decremented because a thread has completed

    write_pending( expbuf, devnum, false ); // this also does the notify

    return;
  }
  /***** Camera::AstroCamInterface::handle_frame ******************************/


  /***** Camera::AstroCamInterface::image_size ********************************/
  /**
   * @brief      set/get image size parameters and allocate PCI memory
   * @details    Sets/gets ROWS COLS OSROWS OSCOLS BINROWS BINCOLS for a
   *             given device|channel. This calls geometry() and buffer() to
   *             set the image geometry on the controller and allocate a PCI buffer.
   * @param[in]  args       contains DEV|CHAN [ [ ROWS COLS OSROWS OSCOLS BINROWS BINCOLS ]
   * @param[out] retstring  reference to string for return value
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long AstroCamInterface::image_size(std::string args, std::string &retstring, const bool save_as_default) {
    const std::string function("Camera::AstroCamInterface::image_size");
    std::stringstream message;

    // Help
    //
    if ( args == "?" || args=="help" ) {
      retstring = CAMERAD_IMSIZE;
      retstring.append( " <chan> | <dev#> [ [ <rows> <cols> <osrows> <oscols> <binrows> <bincols> ]\n" );
      retstring.append( "  Configures image parameters used to set image size in the controller,\n" );
      retstring.append( "  allocate needed PCI buffer space and for FITS header keywords.\n" );
      retstring.append( "  <bin____> represents the binning factor for each axis.\n" );
      retstring.append( "  Camera controller connection must first be open.\n" );
      retstring.append( "  If no args are supplied then the current parameters for dev|chan are returned.\n" );
      retstring.append( "  Specify <chan> from { " );
      message.str("");
      for ( const auto &con : this->controller ) {
        if ( con.second.inactive ) continue;  // skip controllers flagged as inactive
        message << con.second.channel << " ";
      }
      message << "}\n";
      retstring.append( message.str() );
      retstring.append( "       or <dev#> from { " );
      message.str("");
      for ( const auto &con : this->controller ) {
        if ( con.second.inactive ) continue;  // skip controllers flagged as inactive
        message << con.second.devnum << " ";
      }
      message << "}\n";
      retstring.append( message.str() );
      return HELP;
    }

    // Don't allow any changes while any exposure activity is running.
    //
    if ( !this->is_camera_idle() ) {
      logwrite( function, "ERROR: all exposure activity must be stopped before changing image parameters" );
      retstring="camera_busy";
      return ERROR;
    }

    // Get the requested dev# and channel from supplied args.
    // After calling extract_dev_chan(), dev can be trusted as
    // a valid index without needing a try/catch.
    //
    int dev=-1;
    std::string chan;
    if ( this->extract_dev_chan( args, dev, chan, retstring ) != NO_ERROR ) return ERROR;

    // retstring now should contain [ ROWS COLS OSROWS OSCOLS BINROWS BINCOLS ]
    // It can contain 0 or 6 tokens.
    //
    std::vector<std::string> tokens;
    Tokenize( retstring, tokens, " " );

    if ( ! tokens.empty() ) {

      // Having other than 6 tokens is automatic disqualification so get out now
      //
      if ( tokens.size() != 6 ) {
        message.str(""); message << "ERROR: invalid arguments: " << retstring << ": expected <rows> <cols> <osrows> <oscols> <binrows> <bincols>";
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return ERROR;
      }

      int rows=-1, cols=-1, osrows=-1, oscols=-1, binrows=-1, bincols=-1;

      try {
        rows    = std::stoi( tokens.at(0) );
        cols    = std::stoi( tokens.at(1) );
        osrows  = std::stoi( tokens.at(2) );
        oscols  = std::stoi( tokens.at(3) );
        binrows = std::stoi( tokens.at(4) );
        bincols = std::stoi( tokens.at(5) );
      }
      catch ( std::exception &e ) {
        message.str(""); message << "ERROR: exception parsing \"" << retstring << "\": " << e.what();
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return ERROR;
      }

      // Check image size
      //
      if ( rows<1 || cols<1 || osrows<0 || oscols<0 || binrows<1 || bincols<1 ) {
        message.str(""); message << "ERROR: invalid image size " << rows << " " << cols << " "
                                 << osrows << " " << oscols << " " << binrows << " " << bincols;
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return ERROR;
      }

      // Store the geometry in the class. This is the new detector geometry,
      // unchanged by binning, so that when reverting to binning=1 from some
      // binnnig factor, this is the default image size to revert to.
      //
      this->controller[dev].detrows = rows;
      this->controller[dev].detcols = cols;
      this->controller[dev].osrows0 = osrows;
      this->controller[dev].oscols0 = oscols;

      // Binning is the same for all devices so it's stored in the camera info class.
      //
      this->camera_info.binning[_ROW_] = binrows;
      this->camera_info.binning[_COL_] = bincols;

      // Assign the binning also to the controller info
      //
      this->controller[dev].info.binning[_ROW_] = this->camera_info.binning[_ROW_];
      this->controller[dev].info.binning[_COL_] = this->camera_info.binning[_COL_];

      // If binned by a non-evenly-divisible factor then skip modulo that
      // many at the start. These will be removed from the image.
      //
      this->controller[dev].skipcols = cols % bincols;
      this->controller[dev].skiprows = rows % binrows;

      cols -= this->controller[dev].skipcols;
      rows -= this->controller[dev].skiprows;

      // Adjust the number of overscans to make them evenly divisible
      // by the binning factor.
      //
      oscols -= ( oscols % bincols );
      osrows -= ( osrows % binrows );

      // Now that the rows/cols and osrows/oscols have been adjusted for
      // binning, store them in the class as detector_pixels for this controller.
      //
      this->controller[dev].info.detector_pixels[_COL_] = cols + oscols;
      this->controller[dev].info.detector_pixels[_ROW_] = rows + osrows;

      // ROI is the full detector
      this->controller[dev].info.region_of_interest[0] = 1;
      this->controller[dev].info.region_of_interest[1] = this->controller[dev].info.detector_pixels[0];
      this->controller[dev].info.region_of_interest[2] = 1;
      this->controller[dev].info.region_of_interest[3] = this->controller[dev].info.detector_pixels[1];

      this->controller[dev].info.ismex = true;
      this->controller[dev].info.bitpix = 16;

      // *** This is where the binned-image dimensions are re-calculated ***
      //
      // A call to set_axes() will store the binned image dimensions
      // in controller[dev].info.axes[*]. Those dimensions will
      // be used to set the image geometry with geometry() and
      // the PCI buffer with buffer().
      //
      if ( this->controller[dev].info.set_axes() != NO_ERROR ) {
        message.str(""); message << "ERROR setting axes for device " << dev;
        this->async.enqueue_and_log( "CAMERAD", function, message.str() );
        return ERROR;
      }

      // if requested, store the imsize values as a string in the class for default recovery
      //
      if ( save_as_default ) {
        message.str(""); message << rows << " " << cols << " " << osrows << " " << oscols << " " << binrows << " " << bincols;
        this->controller[dev].imsize_args = message.str();
        this->controller[dev].defrows = rows;
        this->controller[dev].defcols = cols;
        this->controller[dev].defosrows = osrows;
        this->controller[dev].defoscols = oscols;
        logwrite( function, "saved as default for chan "+chan+": "+message.str() );
      }

      // This is as far as we can get without a connected controller.
      // If not connected then all we've done is save this info to the class,
      // which will be used after the controller is connected.
      //
      if ( this->controller[dev].connected ) {

        // because set_axes() doesn't scale overscan
        //
        oscols /= bincols;
        osrows /= binrows;

        // save the binning-adjusted overscans to the class
        //
        this->controller[dev].osrows = osrows;
        this->controller[dev].oscols = oscols;

        // allocate PCI buffer and set geometry now
        //
        std::stringstream geostring;
        std::string retstring;
        geostring << dev << " "
                  << this->controller[dev].info.axes[_ROW_] << " "
                  << this->controller[dev].info.axes[_COL_];

        if ( this->buffer( geostring.str(), retstring ) != NO_ERROR ) {
          message.str(""); message << "ERROR: allocating buffer for chan " << this->controller[dev].channel
                                   << " " << this->controller[dev].devname;
          logwrite( function, message.str() );
          return ERROR;
        }

        if ( this->geometry( geostring.str(), retstring ) != NO_ERROR ) {
          message.str(""); message << "ERROR: setting geometry for chan " << this->controller[dev].channel;
          logwrite( function, message.str() );
          return ERROR;
        }

        // Set the Bin Parameters to the controller using the 3-letter command,
        // "SBP <NPBIN> <NP_SKIP> <NSBIN> <NS_SKIP>"
        //
        std::stringstream cmd;
        cmd << "SBP "
            << this->camera_info.binning[_ROW_] << " "
            << this->controller[dev].skiprows << " "
            << this->camera_info.binning[_COL_] << " "
            << this->controller[dev].skipcols;
        if ( this->do_native( dev, cmd.str(), retstring ) != NO_ERROR ) return ERROR;
      }
      else {
        message.str(""); message << "saved but not sent to controller because chan " << this->controller[dev].channel << " is not connected";
        logwrite( function, message.str() );
      }

    }  // end if !tokens.empty()

    // Return the values stored in the class
    //
    message.str(""); message << "detrows detcols osrows oscols binrow bincol = ";
    message << this->controller[dev].detrows << " " << this->controller[dev].detcols << " "
            << this->controller[dev].osrows << " " << this->controller[dev].oscols << " "
            << this->camera_info.binning[_ROW_] << " " << this->camera_info.binning[_COL_]
            << ( this->controller[dev].connected ? "" : " [inactive]" );
    logwrite( function, message.str() );
    retstring = message.str();

    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::image_size ********************************/


  /***** Camera::AstroCamInterface::is_camera_idle ****************************/
  /**
   * @brief      is the specified camera idle?
   * @details    Asking if a camera is idle refers to its activity
   *             status -- actively reading out or in frame transfer.
   * @param[in]  dev  device number to check
   * @return     true|false
   *
   */
  bool AstroCamInterface::is_camera_idle( int dev ) {
    int num=0;
    num += ( this->controller[dev].in_readout ? 1 : 0 );
    num += ( this->controller[dev].in_frametransfer ? 1 : 0 );
    std::lock_guard<std::mutex> lock( this->pending_mtx );
    num += this->exposures_pending.size();
    return ( num>0 ? false : true );
  }
  /***** Camera::AstroCamInterface::is_camera_idle ****************************/
  /**
   * @brief      is the specified camera idle?
   * @details    Asking if a camera is idle refers to its activity
   *             status -- actively reading out or in frame transfer.
   * @return     true|false
   *
   */
  bool AstroCamInterface::is_camera_idle() {
    int num=0;
    for ( auto dev : this->devnums ) {
      num += ( this->controller[dev].in_readout ? 1 : 0 );
      num += ( this->controller[dev].in_frametransfer ? 1 : 0 );
    }
    std::lock_guard<std::mutex> lock( this->pending_mtx );
    num += this->exposures_pending.size();
    return ( num>0 ? false : true );
  }
  /***** Camera::AstroCamInterface::is_camera_idle ****************************/


  /***** Camera::AstroCamInterface::parse_controller_config *******************/
  /**
   * @brief      parses the CONTROLLER keyword from config file
   * @param[in]  args  expected format is "PCIDEV CHAN FT FIRMWARE READOUT"
   *
   * Each camera controller is defined by
   * '''
   * CONTROLLER=(PCIDEV CHAN FT FIRMWARE READOUT)
   * where PCIDEV    is the PCI device number
   *       CHAN      is the spectrographic channel {U,R,I,G}
   *       ID        is the CCD Identifier (E.G. serial number, name, etc.)
   *       FT        is {yes|no} to indicate if Frame Transfer is supported
   *       FIRMWARE  is the default firmware to load
   *       READOUT   is the default readout amplifier {U1,L1,U2,L2,SPLIT1,SPLIT2,QUAD,FT1,FT2}
   * '''
   *
   * This function parses the arg string, checks for valid inputs, and
   * uses the values to create a controller STL map entry for each device.
   *
   */
  long AstroCamInterface::parse_controller_config( std::string args ) {
    const std::string function("Camera::AstroCamInterface::parse_controller_config");
    std::stringstream message;
    std::vector<std::string> tokens;

    logwrite( function, args );

    int dev, readout_type=-1;
    uint32_t readout_arg=0xBAD;
    std::string chan, id, firm, amp;
    bool ft, readout_valid=false;

    Tokenize( args, tokens, " " );

    if ( tokens.size() != 6 ) {
      message.str(""); message << "ERROR: bad value \"" << args << "\". expected { PCIDEV CHAN ID FT FIRMWARE READOUT }";
      logwrite( function, message.str() );
      return ERROR;
    }

    try {
      dev  = std::stoi( tokens.at(0) );
      chan = tokens.at(1);
      id   = tokens.at(2);
      firm = tokens.at(4);
      amp  = tokens.at(5);
      if ( tokens.at(3) == "yes" ) ft = true;
      else
      if ( tokens.at(3) == "no" )  ft = false;
      else {
        message.str(""); message << "ERROR unrecognized value for FT: " << tokens.at(2) << ". Expected { yes | no }";
        logwrite( function, message.str() );
        return ERROR;
      }
    }
    catch (const std::exception &e) {
      logwrite( function, "ERROR parsing configuration: "+std::string(e.what()));
      return ERROR;
    }

    // Check the PCIDEV number is in expected range
    //
    if ( dev < 0 || dev > 3 ) {
      message.str(""); message << "ERROR bad PCIDEV " << dev << ". Expected {0,1,2,3}";
      logwrite(function, message.str());
      return ERROR;
    }

    // Check that READOUT has a match in the list of known readout amps.
    //
    for ( const auto &source : this->readout_source ) {
      if ( source.first.compare( amp ) == 0 ) {     // found a match
        readout_valid = true;
        readout_arg  = source.second.readout_arg;   // get the arg associated with this match
        readout_type = source.second.readout_type;  // get the type associated with this match
      }
    }
    if ( !readout_valid || readout_type==-1 || readout_arg==0xBAD ) {
      message.str(""); message << "ERROR: bad READOUT " << amp << " for CHAN " << chan << ". Expected { ";
      for ( const auto &source : this->readout_source ) message << source.first << " ";
      message << "}";
      logwrite( function, message.str() );
      return ERROR;
    }

    // Create a vector of configured device numbers
    //
    this->configured_devnums.push_back( dev );

    // The Controller class holds the Camera::Information class and the FITS_file class,
    // as well as wrappers for calling the functions inside the FITS_file class.
    //
    // The first four come from the config file, the rest are defaults
    //
    this->controller[dev].devnum = dev;             // device number
    this->controller[dev].channel = chan;           // spectrographic channel
    this->controller[dev].ccd_id  = id;             // CCD identifier
    this->controller[dev].have_ft = ft;             // frame transfer supported?
    this->controller[dev].firmware = firm;          // firmware file
    this->controller[dev].pArcDev = ( new arc::gen3::CArcPCI() );        // set the pointer to this object in the public vector
    this->controller[dev].pCallback = ( new Callback(this) );            // set the pointer to this object in the public vector
    this->controller[dev].devname = "";             // device name
    this->controller[dev].connected = false;        // not yet connected
    this->controller[dev].firmwareloaded = false;   // no firmware loaded
    this->controller[dev].inactive = false;         // assume active unless shown otherwise

    this->controller[dev].info.readout_name = amp;
    this->controller[dev].info.readout_type = readout_type;
    this->controller[dev].readout_arg = readout_arg;

    this->exposure_pending( dev, false );
    this->controller[dev].in_readout = false;
    this->controller[dev].in_frametransfer = false;
    this->state_monitor_condition.notify_all();

    // Header keys specific to this controller are stored in the controller's extension
    //
    this->controller[dev].info.systemkeys.add_key( "CCD_ID", id, "CCD identifier parse", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "FT", ft, "frame transfer used", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "AMP_ID", amp, "CCD readout amplifier ID", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "SPEC_ID", chan, "spectrograph channel", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "DEV_ID", dev, "detector controller PCI device ID", EXT, chan );

#ifdef LOGLEVEL_DEBUG
    message.str("");
    message << "[DEBUG] pointers for dev " << dev << ": "
            << " pArcDev=" << std::hex << std::uppercase << this->controller[dev].pArcDev
            << " pCB="     << std::hex << std::uppercase << this->controller[dev].pCallback;
    logwrite(function, message.str());
#endif
    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::parse_controller_config *******************/


  /***** Camera::AstroCamInterface::parse_spect_config ************************/
  long AstroCamInterface::parse_spect_config( std::string args ) {
    const std::string function("Camera::AstroCamInterface::parse_spect_config");
    std::stringstream message;
    std::vector<std::string> tokens;

    Tokenize( args, tokens, " " );

    if ( tokens.size() != 3 ) {
      message.str(""); message << "ERROR: bad value \"" << args << "\". expected { CHAN DISPERSION MINWAVELENGTH }";
      logwrite( function, message.str() );
      return ERROR;
    }

    std::string chan;
    double disp=NAN;
    double wavel=NAN;

    // Parse the three values from the args string
    //
    try {
      chan  = tokens.at(0);
      disp  = std::stod(tokens.at(1));
      wavel = std::stod(tokens.at(2));
    }
    catch( const std::exception &e ) {
      message.str(""); message << "ERROR: parsing \"" << args << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    // There must have been a device configured for this channel.
    // Not an error if there's not, but can't continue.
    //
    int dev = devnum_from_chan(chan);

    if ( dev < 0 ) {
      message.str(""); message << "NOTICE: no devnum configured for channel \"" << chan << "\"";
      logwrite( function, message.str() );
      return NO_ERROR;
    }

    if ( this->controller.find(dev) == this->controller.end() ) {
      message.str(""); message << "ERROR dev " << dev << " not found in controller configuration";
      logwrite( function, message.str() );
      return ERROR;
    }

    this->controller[dev].info.dispersion = disp;
    this->controller[dev].info.minwavel = wavel;

    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::parse_spect_config ************************/


  /***** Camera::AstroCamInterface::write_frame *******************************/
  /**
   * @brief      writes the image_data buffer to disk for the specified device
   * @param[in]  expbuf    exposure buffer number
   * @param[in]  devnum    device number
   * @param[in]  fpbcount  frame number = also thread ID receiving the frame
   * @return     ERROR or NO_ERROR
   *
   * This function is called by the handle_frame thread.
   * It will call deinterlace() before calling write().
   *
   * incoming frame from ARC -> frameCallback() -> handle_frame() -> write_frame()
   *
   */
  long AstroCamInterface::write_frame( int expbuf, int devnum, const std::string chan, int fpbcount ) {
    std::string function = "Camera::AstroCamInterface::write_frame";
    std::stringstream message;
    long error=NO_ERROR;

    // This is the PCI image buffer that holds the image.
    //
    // Use devnum to identify which device has the frame,
    // and fpbcount to index the frameinfo STL map on that devnum,
    // and assign the pointer to that buffer to a local variable.
    //
    void* imbuf = this->controller[devnum].frameinfo[fpbcount].buf;

    message << this->controller[devnum].devname << " received exposure "
            << this->controller[devnum].frameinfo[fpbcount].framenum << " into image buffer "
            << std::hex << this->controller[devnum].frameinfo[fpbcount].buf;
    logwrite(function, message.str());

    // Call the class' deinterlace and write functions.
    //
    // The .deinterlace() function is called with the PCI image buffer pointer cast to the appropriate type.
    // The .write() function writes the deinterlaced (work) buffer, which is a class member, so
    // that buffer is already known.
    //
    try {
      switch (this->controller[devnum].info.datatype) {
        case USHORT_IMG: {
          this->controller[devnum].deinterlace( expbuf, (uint16_t *)imbuf );
message.str(""); message << this->controller[devnum].devname << " exposure buffer " << expbuf << " deinterlaced " << std::hex << imbuf;
logwrite(function, message.str());
message.str(""); message << "about to write section size " << this->controller[devnum].expinfo[expbuf].section_size ; // << " to file \"" << this->pFits[expbuf]->fits_name << "\"";
logwrite(function, message.str());

          // Call write_image(),
          // passing a pointer to the workbuf for this controller (which has the deinterlaced image), and
          // a pointer for the info class for this exposure buf on this controller.
          //
          error = this->pFits[ expbuf ]->write_image( (uint16_t *)this->controller[ devnum ].workbuf, this->controller[ devnum ].expinfo[expbuf], chan );

          this->pFits[ expbuf ]->extension++;

message.str(""); message << this->controller[devnum].devname << " exposure buffer " << expbuf << " wrote " << std::hex << this->controller[devnum].workbuf;
logwrite(function, message.str());

//        error = this->controller[devnum].write( );  10/30/23 BOB -- the write is above. .write() called ->write_image(), skip that extra function
          break;
        }
/*******
        case SHORT_IMG: {
          this->controller[devnum].deinterlace( expbuf, (int16_t *)imbuf );
          error = this->controller[devnum].write( );
          break;
        }
        case FLOAT_IMG: {
          this->controller[devnum].deinterlace( expbuf, (uint32_t *)imbuf );
          error = this->controller[devnum].write( );
          break;
        }
********/
        default:
          message.str("");
          message << "ERROR: unknown datatype: " << this->controller[devnum].info.datatype;
          logwrite(function, message.str());
          error = ERROR;
          break;
      }
      // A frame has been written for this device,
      // so increment the framecounter for devnum.
      //
      if (error == NO_ERROR) this->controller[devnum].increment_framecount();
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] framecount(" << devnum << ")="
                               << this->controller[devnum].get_framecount() << " written";
      logwrite( function, message.str() );
#endif
    }
    catch (std::out_of_range &) {
      message.str(""); message << "ERROR: unable to find device " << devnum << " in list: { ";
      for (const auto &check : this->devnums) message << check << " ";
      message << "}";
      logwrite(function, message.str());
      error = ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    message.str("");
    message << "[DEBUG] completed " << (error != NO_ERROR ? "with error. " : "ok. ")
            << "devnum=" << devnum << " " << "fpbcount=" << fpbcount << " "
            << this->controller[devnum].devname << " received exposure "
            << this->controller[devnum].frameinfo[fpbcount].framenum << " into buffer "
            << std::hex << std::uppercase << this->controller[devnum].frameinfo[fpbcount].buf;
    logwrite(function, message.str());
#endif
    return( error );
  }
  /***** Camera::AstroCamInterface::write_frame *******************************/


  /***** Camera::AstroCamInterface::write_pending *****************************/
  /**
   * @brief      Set or clear the write pending state for a given exposure
   * @param[in]  expbuf  exposure buffer number
   * @param[in]  devnum  device number
   * @param[in]  add     bool true to add, false to remove from pending vector
   *
   * This function is overloaded with an inline version that returns a bool
   * to indicate if there are any controllers with a pending write.
   *
   */
  void AstroCamInterface::write_pending( int expbuf, int devnum, bool add ) {

    try {
      std::lock_guard<std::mutex> lock(this->write_lock);               // protect writes_pending vector
      auto it = std::find( this->writes_pending.at( expbuf ).begin(),
                           this->writes_pending.at( expbuf ).end(),
                           devnum );                                    // iterator of devnum in writes_pending[expbuf] vector
      bool found  = ( it != this->writes_pending.at( expbuf ).end() );  // was expbuf found in vector?
      bool remove = !add;                                               // do I remove it?

      if ( add && !found )   { this->writes_pending.at( expbuf ).push_back( devnum ); }
      else
      if ( found && remove ) { this->writes_pending.at( expbuf ).erase( it ); }

    }                                                                   // lock_guard releases when it goes out of scope
    catch ( std::out_of_range & ) { return; }

    this->write_condition.notify_all();

#ifdef LOGLEVEL_DEBUG
    std::stringstream message;
    message << "[DEBUG]";
    for (const auto &dev : this->writes_pending[expbuf]) message << " " << dev;
    logwrite("Camera::AstroCamInterface:::write_pending", message.str());
#endif
    return;
  }
  /***** Camera::AstroCamInterface::write_pending *****************************/


  /***** Camera::AstroCamInterface::configure_camera **************************/
  /**
   * @brief      perform initial configuration of controller from .cfg file
   * @return     ERROR | NO_ERROR
   *
   * Called automatically by main() when the server starts up.
   *
   */
  long AstroCamInterface::configure_camera() {
    const std::string function("Camera::AstroCamInterface::configure_camera");
    std::stringstream message;
    int applied=0;
    long error = NO_ERROR;

    // Initialize the vector of configured device numbers,
    // which will get built up from parse_controller_config() below.
    //
    this->configured_devnums.clear();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < server->config.n_entries; entry++) {

      if ( server->config.param[entry].find( "CONTROLLER" ) == 0 ) {
        if ( this->parse_controller_config( server->config.arg[entry] ) != ERROR ) {
          message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
          this->async.enqueue_and_log( function, message.str() );
          applied++;
        }
      }

      if ( server->config.param[entry] == "IMAGE_SIZE" ) {
        std::string retstring;
        bool save_as_default = true;
        if ( this->image_size( server->config.arg[entry], retstring, save_as_default ) != ERROR ) {
          message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
          this->async.enqueue_and_log( function, message.str() );
          applied++;
        }
      }

      if ( server->config.param[entry].find( "IMDIR" ) == 0 ) {
        std::string dontcare;
        imdir( server->config.arg[entry], dontcare );
        message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
        logwrite( function, message.str() );
        async.enqueue( message.str() );
        applied++;
      }

      if ( server->config.param[entry].find( "DIRMODE" ) == 0 ) {
        try {
          if ( !server->config.arg[entry].empty() ) {
            mode_t mode = (mode_t)std::stoi( server->config.arg[entry] );
            this->set_dirmode( mode );
          }
        }
        catch (const std::exception &e) {
          logwrite( function, "ERROR parsing DIRMODE: "+std::string(server->config.arg[entry]) );
          return ERROR;
        }
        message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
        logwrite( function, message.str() );
        this->async.enqueue( message.str() );
        applied++;
      }

      if ( server->config.param[entry].find( "BASENAME" ) == 0 ) {
        std::string dontcare;
        this->basename( server->config.arg[entry], dontcare );
        message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
        logwrite( function, message.str() );
        this->async.enqueue( message.str() );
        applied++;
      }

      // If the Bonn shutter has been disabled ( = "no" ) then add a FITS keyword
      // to indicate that. If present, which is the normal operating condition,
      // then remove that keyword -- only need to flag this if non-standard condition.
      //
      if ( server->config.param[entry].find( "BONN_SHUTTER" ) == 0 ) {
        std::string bs = server->config.arg[entry];
        if ( !bs.empty() && bs=="no" ) {
          this->use_bonn_shutter = false;
          this->camera_info.systemkeys.primary().addkey( "BONNSHUT", false, "Bonn shutter not in use" );
        }
        else
        if ( !bs.empty() && bs=="yes" ) {
          this->use_bonn_shutter = true;
          this->camera_info.systemkeys.primary().delkey( "BONNSHUT" );
        }
        else {
          logwrite( function, "ERROR BONN_SHUTTER expected yes | no" );
          return ERROR;
        }
        message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
        logwrite( function, message.str() );
        this->async.enqueue( message.str() );
        applied++;
      }

      if ( server->config.param[entry] == "SHUTTER_DELAY" ) {
        if ( !server->config.arg[entry].empty() ) {
          error = this->set_shutter_delay( server->config.arg[entry] );
          message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
          message << (error==ERROR ? " (ERROR)" : "" );
          logwrite( function, message.str() );
          this->async.enqueue( message.str() );
          applied++;
        }
      }

      // If an external shutter has been enabled ( = "yes" ) then add a FITS keyword
      // to indicate that. If not present, which is the normal operating condition,
      // then remove that keyword -- only need to flag this is in use.
      //
      if ( server->config.param[entry].find( "EXT_SHUTTER" ) == 0 ) {
        std::string es = server->config.arg[entry];
        if ( !es.empty() && es=="yes" ) {
          this->use_ext_shutter = true;
          this->camera_info.systemkeys.primary().addkey( "EXT_SHUT", true, "external shutter trigger in use" );
        }
        else
        if ( !es.empty() && es=="no" ) {
          this->use_ext_shutter = false;
          this->camera_info.systemkeys.primary().delkey( "EXT_SHUT" );
        }
        else {
          logwrite( function, "ERROR EXT_SHUTTER expected yes | no" );
          return ERROR;
        }
        message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
        logwrite( function, message.str() );
        this->async.enqueue( message.str() );
        applied++;
      }

      if ( server->config.param[entry] == "SPECT_INFO" ) {
        if ( this->parse_spect_config( server->config.arg[entry] ) != ERROR ) {
          message.str(""); message << "CAMERAD:config:" << server->config.param[entry] << "=" << server->config.arg[entry];
          this->async.enqueue_and_log( function, message.str() );
          applied++;
        }
      }

    }

    message.str("");
    if (applied==0) {
      message << "ERROR: ";
      error = ERROR;
    }
    message << "applied " << applied << " configuration lines to controller";
    logwrite(function, message.str());
    return error;
  }
  /***** Camera::AstroCamInterface::configure_camera **************************/


  /***** Camera::AstroCamInterface::disconnect_controller *********************/
  /**
   * @brief
   *
   */
  long AstroCamInterface::disconnect_controller( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::disconnect_controller");
    logwrite(function, "not yet implemented");
    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::disconnect_controller *********************/


  /***** Camera::AstroCamInterface::exptime ***********************************/
  /**
   * @brief
   *
   */
  long AstroCamInterface::exptime( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::exptime");
    logwrite(function, "not yet implemented");
    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::exptime ***********************************/


  /***** Camera::AstroCamInterface::expose ************************************/
  /**
   * @brief      
   * @param      
   * @param      
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long AstroCamInterface::expose( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::expose");
    logwrite(function, "not yet implemented");

    // Help
    //
    if (args.empty() || args=="?" || args=="help") {
      retstring = CAMERAD_EXPOSE;
      retstring.append( " <tbd>\n" );
      retstring.append( "  TBD\n" );
      return HELP;
    }
    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::expose ************************************/


  /***** Camera::AstroCamInterface::load_firmware *****************************/
  /**
   * @brief
   *
   */
  long AstroCamInterface::load_firmware( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::load_firmware");
    logwrite(function, "not yet implemented");
    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::load_firmware *****************************/


  /***** Camera::AstroCamInterface::native ************************************/
  /**
   * @brief
   *
   */
  long AstroCamInterface::native( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::native");
    logwrite(function, "not yet implemented");
    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::native ************************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to specified or all open Leach controllers
   * @details    See details in overloaded do_native(std::string args, std::string &retstring)
   * @param[in]  args  string containing optional DEV|CHAN plus command and arguments
   * @return     NO_ERROR on success, ERROR on error
   *
   */
  long AstroCamInterface::do_native( std::string args ) {
    std::string dontcare;
    return this->do_native( args, dontcare );
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to specified or all open Leach controllers
   * @details    The input args are checked for the presence of a recognized
   *             channel name or device number. If one is present then the
   *             remaining part of args is sent to that specified controller only,
   *             otherwise the entire args string is sent to all open controllers.
   * @param[in]  args       string containing optional DEV|CHAN plus command and arguments
   * @param[out] retstring  return string
   * @return     NO_ERROR on success, ERROR on error
   *
   * args can be <CMD>
   *             <CMD> <ARG> ...
   *             <DEV|CHAN> <CMD>
   *             <DEV|CHAN> <CMD> <ARG> ...
   */
  long AstroCamInterface::do_native( std::string args, std::string &retstring ) {
    // Try to get the requested dev# and channel from supplied args.
    // If extract_dev_chan() returns NO_ERROR, dev can be trusted as
    // a valid index without needing a try/catch and cmdstr will hold
    // the full command (args minus the dev/chan).
    //
    int dev=-1;
    std::string chan, cmdstr;
    if (this->extract_dev_chan( args, dev, chan, cmdstr )==NO_ERROR) {
      // found a dev so send native command to this dev only
      std::string dontcare;
      return this->do_native( dev, cmdstr, dontcare );
    }
    else {
      // didn't find a dev in args so build vector of all open controllers
      std::vector<uint32_t> selectdev;
      for ( const auto &dev : this->devnums ) {
        if ( this->controller[dev].connected ) selectdev.push_back( dev );
      }
      // this will send the native command to all controllers in that vector
      return this->do_native( selectdev, args, retstring );
    }
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to Leach controllers specified by vector
   * @param[in]  selectdev  vector of devices to use
   * @param[in]  cmdstr     string containing command and arguments
   * @return     NO_ERROR on success, ERROR on error
   *
   */
  long AstroCamInterface::do_native(std::vector<uint32_t> selectdev, std::string cmdstr) {
    // Use the erase-remove idiom to remove disconnected devices from selectdev
    //
    selectdev.erase( std::remove_if( selectdev.begin(), selectdev.end(),
                     [this](uint32_t dev) { return !this->controller[dev].connected; } ),
                     selectdev.end() );

    std::string retstring;
    return this->do_native( selectdev, cmdstr, retstring );
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to individual Leach controller by devnum
   * @param[in]  dev        individual device to use
   * @param[in]  cmdstr     string containing command and arguments
   * @param[out] retstring  reference to string to contain reply
   * @return     NO_ERROR on success, ERROR on error
   *
   */
  long AstroCamInterface::do_native( int dev, std::string cmdstr, std::string &retstring ) {
    std::vector<uint32_t> selectdev;
    if ( this->controller[dev].connected ) selectdev.push_back( dev );
    return this->do_native( selectdev, cmdstr, retstring );
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to Leach controllers specified by vector
   * @param[in]  selectdev  vector of devices to use
   * @param[in]  cmdstr     string containing command and arguments
   * @param[out] retstring  reference to string to contain reply
   * @return     NO_ERROR | ERROR | HELP
   *
   */
  long AstroCamInterface::do_native( std::vector<uint32_t> selectdev, std::string cmdstr, std::string &retstring ) {
    std::string function("Camera::AstroCamInterface::do_native");
    std::stringstream message;
    std::vector<std::string> tokens;

    // If no connected devices then nothing to do here
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR: no connected devices");
      retstring="not_connected";
      return(ERROR);
    }

    if ( cmdstr.empty() ) {
      logwrite(function, "ERROR: missing command");
      retstring="invalid_argument";
      return(ERROR);
    }
    // Ensure command and args are uppercase
    //
    try {
      std::transform( cmdstr.begin(), cmdstr.end(), cmdstr.begin(), ::toupper );
    }
    catch (...) {
      logwrite(function, "ERROR: converting command to uppercase");
      return( ERROR );
    }

    std::vector<uint32_t> cmd;      // this vector will contain the cmd and any arguments
    uint32_t c0, c1, c2;

    Tokenize(cmdstr, tokens, " ");

    int nargs = tokens.size() - 1;  // one token is for the command, this is number of args

    // max 4 arguments
    //
    if (nargs > 4) {
      message.str(""); message << "ERROR: too many arguments: " << nargs << " (max 4)";
      logwrite(function, message.str());
      retstring="invalid_argument";
      return(ERROR);
    }

    try {
      // first token is command and require a 3-letter command
      //
      if (tokens.at(0).length() != 3) {
        message.str(""); message << "ERROR: bad command " << tokens.at(0) << ": native command requires 3 letters";
        logwrite(function, message.str());
        retstring="bad_command";
        return(ERROR);
      }

      // Change the 3-letter (ASCII) command into hex byte representation
      // and push it into cmd vector.
      //
      c0  = (uint32_t)tokens.at(0).at(0); c0 = c0 << 16;
      c1  = (uint32_t)tokens.at(0).at(1); c1 = c1 <<  8;
      c2  = (uint32_t)tokens.at(0).at(2);

      cmd.push_back( c0 | c1 | c2 );

      // convert each arg into a number and push into cmd vector
      //
      tokens.erase( tokens.begin() );                 // first, erase the command from the tokens
      for ( const auto &tok : tokens ) {
        cmd.push_back( (uint32_t)parse_val( tok ) );  // then push each converted arg into cmd vector
      }
    }
    catch(std::out_of_range &) {  // should be impossible but here for safety
      message.str(""); message << "ERROR: unable to parse command ";
      for (const auto &check : tokens) message << check << " ";
      message << ": out of range";
      logwrite(function, message.str());
      retstring="out_of_range";
      return(ERROR);
    }
    catch(const std::exception &e) {
      message.str(""); message << "ERROR forming command: " << e.what();
      logwrite(function, message.str());
      retstring="exception";
      return(ERROR);
    }
    catch(...) { logwrite(function, "unknown error forming command"); retstring="exception"; return(ERROR); }

    // Send the command to each selected device via a separate thread
    //
    {                                       // start local scope for this stuff
    std::vector<std::thread> threads;       // local scope vector stores all of the threads created here
    for ( const auto &dev : selectdev ) {   // spawn a thread for each device in selectdev
      std::thread thr( &AstroCamInterface::dothread_native, std::ref(*this), dev, cmd );
      threads.push_back(std::move(thr));    // push the thread into a vector
    }

    try {
      for (std::thread & thr : threads) {   // loop through the vector of threads
        if ( thr.joinable() ) thr.join();   // if thread object is joinable then join to this function. (not to each other)
      }
    }
    catch(const std::exception &e) {
      message.str(""); message << "ERROR joining threads: " << e.what();
      logwrite(function, message.str());
      retstring="exception";
      return(ERROR);
    }
    catch(...) { logwrite(function, "unknown error joining threads"); retstring="exception"; return(ERROR); }
    }                                       // end local scope cleans up this stuff

    // Check to see if all retvals are the same by comparing them all to the first.
    //
    std::uint32_t check_retval;
    try {
      check_retval = this->controller[selectdev.at(0)].retval;    // save the first one in the controller vector
    }
    catch(std::out_of_range &) {
      logwrite(function, "ERROR: no device found. Is the controller connected?");
      retstring="out_of_range";
      return(ERROR);
    }

    bool allsame = true;
    for ( const auto &dev : selectdev ) { if (this->controller[dev].retval != check_retval) { allsame = false; } }

    // If all the return values are equal then return only one value...
    //
    if ( allsame ) {
      this->retval_to_string( check_retval, retstring );        // this sets retstring = to_string( retval )
    }

    // ...but if any retval is different from another then we have to report each one.
    //
    else {
      std::stringstream rs;
      for ( const auto &dev : selectdev ) {
        this->retval_to_string( this->controller[dev].retval, retstring );          // this sets retstring = to_string( retval )
        rs << std::dec << this->controller[dev].devnum << ":" << retstring << " ";  // build up a stringstream of each controller's reply
      }
      retstring = rs.str();  // re-use retstring to contain all of the replies
    }

    // Log the return values
    ///< TODO @todo need a way to send these back to the calling function
    //
    long error = NO_ERROR;
    return error;
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCamInterface::dothread_native ***********************************/
  /**
   * @brief      run in a thread to actually send the command
   * @param[in]  con  reference to Controller object
   * @param[in]  cmd  vector containing command and args
   *
   */
  void AstroCamInterface::dothread_native( int dev, std::vector<uint32_t> cmd ) {
    const std::string function("Camera::AstroCamInterface::dothread_native");
    std::stringstream message;
    uint32_t command;

    std::lock_guard<std::mutex> lock(this->controller[dev].pcimtx);

    ++this->pci_cmd_num;

    message << "sending command (" << std::dec << this->pci_cmd_num << ") to chan "
            << this->controller[dev].channel << " dev " << dev << ":"
            << std::setfill('0') << std::setw(2) << std::hex << std::uppercase;
    for (const auto &arg : cmd) message << " 0x" << arg;
    logwrite(function, message.str());


    try {
      if ( cmd.size() > 0 ) command = cmd.at(0);

      // ARC_API now uses an initialized_list object for the TIM_ID, command, and arguments.
      // The list object must be instantiated with a fixed size at compile time.
      //
      if (cmd.size() == 1) controller[dev].retval = controller[dev].pArcDev->command( { TIM_ID, cmd.at(0) } );
      else
      if (cmd.size() == 2) controller[dev].retval = controller[dev].pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1) } );
      else
      if (cmd.size() == 3) controller[dev].retval = controller[dev].pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2) } );
      else
      if (cmd.size() == 4) controller[dev].retval = controller[dev].pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2), cmd.at(3) } );
      else
      if (cmd.size() == 5) controller[dev].retval = controller[dev].pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2), cmd.at(3), cmd.at(4) } );
      else {
        message.str(""); message << "ERROR: invalid number of command arguments: " << cmd.size() << " (expecting 1,2,3,4,5)";
        logwrite(function, message.str());
        controller[dev].retval = 0x455252;
      }
    }
    catch(const std::runtime_error &e) {
      message.str(""); message << "ERROR sending (" << this->pci_cmd_num << ") 0x"
                               << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
                               << command << " to " << controller[dev].devname << ": " << e.what();
      logwrite(function, message.str());
      controller[dev].retval = 0x455252;
      return;
    }
    catch(std::out_of_range &) {  // impossible
      logwrite(function, "ERROR: indexing command argument ("+std::to_string(this->pci_cmd_num)+")");
      controller[dev].retval = 0x455252;
      return;
    }
    catch(...) {
      message.str(""); message << "ERROR sending (" << std::dec << this->pci_cmd_num << ") 0x"
                               << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
                               << command << " to " << controller[dev].devname << ": unknown";
      logwrite(function, message.str());
      controller[dev].retval = 0x455252;
      return;
    }

    std::string retvalstring;
    this->retval_to_string( controller[dev].retval, retvalstring );
    message.str(""); message << controller[dev].devname << std::dec << " (" << this->pci_cmd_num << ")"
                             << " returns " << retvalstring;
    logwrite( function, message.str() );

    return;
  }
  /***** AstroCamInterface::dothread_native ***********************************/


  /***** AstroCamInterface::retval_to_string **********************************/
  /**
   * @brief      private function to convert ARC return values (long) to string
   * @param[in]  retval     integer return value from controller
   * @param[out] retstring  reference to a string for return value in ASCII
   *
   * In addition to converting longs to string, if the retval is a particular
   * commonly used code, then set the ASCII characters for those codes,
   * such as DON, ERR, etc. instead of returning a string value of their
   * numeric counterpart.
   *
   */
  void AstroCamInterface::retval_to_string(std::uint32_t retval, std::string& retstring) {
    // replace some common values with their ASCII equivalents
    //
    if ( retval == 0x00455252 ) { retstring = "ERR";  }
    else
    if ( retval == 0x00444F4E ) { retstring = "DON";  }
    else
    if ( retval == 0x544F5554 ) { retstring = "TOUT"; }
    else
    if ( retval == 0x524F5554 ) { retstring = "ROUT"; }
    else
    if ( retval == 0x48455252 ) { retstring = "HERR"; }
    else
    if ( retval == 0x00535952 ) { retstring = "SYR";  }
    else
    if ( retval == 0x00525354 ) { retstring = "RST";  }
    else
    if ( retval == 0x00434E52 ) { retstring = "CNR";  }

    // otherwise just convert the numerical value to a string represented in hex
    //
    else {
      std::stringstream rs;
      rs << "0x" << std::hex << std::uppercase << retval;
      retstring = rs.str();
    }
  }
  /***** AstroCamInterface::retval_to_string **********************************/


  /***** Camera::AstroCamInterface::power *************************************/
  /**
   * @brief
   *
   */
  long AstroCamInterface::power( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::power");
    logwrite(function, "not yet implemented");
    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::power *************************************/


  int AstroCamInterface::devnum_from_chan( const std::string &chan ) {
    int devnum=-1;
    for ( const auto &con : this->controller ) {
      if ( con.second.inactive ) continue; // skip controllers flagged as inactive
      if ( con.second.channel == chan ) {  // check to see if it matches a configured channel.
        devnum = con.second.devnum;
        break;
      }
    }
    return devnum;
  }


  /***** Camera::AstroCamInterface::test **************************************/
  /**
   * @brief
   * @param[in]  args
   * @param[out] retstring
   * @return     ERROR | NO_ERROR
   *
   */
  long AstroCamInterface::test( const std::string args, std::string &retstring ) {
    const std::string function("Camera::AstroCamInterface::test");
    logwrite(function, "not yet implemented");
    return NO_ERROR;
  }
  /***** Camera::AstroCamInterface::test **************************************/

}

