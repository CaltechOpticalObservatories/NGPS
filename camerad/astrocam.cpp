/**
 * @file    astrocam.cpp
 * @brief   contains the functions for the AstroCam interface
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details The main server object is instantiated in src/server.cpp and
 *          defined extern here so that static functions can access it. The
 *          static functions are run in std::threads which means class objects
 *          are otherwise be unavailable to them. The interface class is
 *          accessible through this because Camera::Server server inherits
 *          AstroCam::Interface.
 *
 */

#include "camerad.h"
extern Camera::Server server;

namespace AstroCam {

  long NewAstroCam::new_expose( std::string nseq_in ) {
    logwrite( "NewAstroCam::new_expose", nseq_in );
    return( NO_ERROR );
  }


  /**** AstroCam::Interface::exposure_progress ********************************/
  /**
   * @brief      polls and broadcasts shutter timer progress
   *
   */
  void Interface::exposure_progress() {
    long remaintime=0, delaytime=0;
    std::string message;
    message.reserve(32);
    do {
      // poll and report once per second
      std::this_thread::sleep_for( std::chrono::seconds(1) );
      // poll
      this->camera.shutter_timer.progress(remaintime, delaytime);
      if ( delaytime==0 ) return;
      message = "EXPTIME:" + std::to_string(remaintime) + " "
                           + std::to_string(delaytime)  + " "
                           + std::to_string(static_cast<long>(100*(1-static_cast<double>(remaintime)/delaytime)));
      // broadcast
      std::thread( &AstroCam::Interface::handle_queue2, this, std::move(message) ).detach();
    }
    while ( remaintime > 0 );
  }
  /**** AstroCam::Interface::exposure_progress ********************************/


  /***** AstroCam::Callback::exposeCallback ***********************************/
  /**
   * @brief      called by CArcDevice::expose() during the exposure
   * @param[in]  devnum         device number passed to API on expose
   * @param[in]  uiElapsedTime  actually number of millisec remaining
   *
   * This is the callback function invoked by the ARC API,
   * arc::gen3::CArcDevice::expose() during the exposure.
   * After sending the expose (SEX) command, the API polls the controller
   * using the RET command.
   *
   */
  void Callback::exposeCallback( int devnum, std::uint32_t uiElapsedTime, std::uint32_t uiExposureTime ) {
    std::string function = "AstroCam::Callback::exposeCallback";
    std::stringstream message;
    message << "ELAPSEDTIME_" << devnum << ":" << uiElapsedTime;
    if ( uiExposureTime != 0x1BAD1BAD ) message<< " EXPTIME:" << uiExposureTime;
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
    return;
  }
  /***** AstroCam::Callback::exposeCallback ***********************************/


  /***** AstroCam::Callback::readCallback *************************************/
  /**
   * @brief      called by CArcDevice::expose() during readout
   * @param[in]  expbuf        exposure buffer number passed to API on expose
   * @param[in]  devnum        device number passed to API on expose
   * @param[in]  uiPixelCount  number of pixels read ( getPixelCount() )
   *
   * This is the callback function invoked by the ARC API,
   * arc::gen3::CArcDevice::expose() when bInReadout is true.
   *
   */
  void Callback::readCallback( int expbuf, int devnum, std::uint32_t uiPixelCount, std::uint32_t uiImageSize ) {
    std::string message;
    message.reserve(36);
    message = "PIXELCOUNT_" + std::to_string(devnum)       + ":"
                            + std::to_string(uiPixelCount) + " "
                            + std::to_string(uiImageSize)  + " "
                            + std::to_string(static_cast<long>(100*uiPixelCount/uiImageSize));
    std::thread( std::ref(AstroCam::Interface::handle_queue), message ).detach();
    return;
  }
  /***** AstroCam::Callback::readCallback *************************************/


  /***** AstroCam::Callback::frameCallback ************************************/
  /**
   * @brief      called by CArcDevice::expose() when a frame has been received
   * @param[in]  expbuf    exposure buffer number passed to API on expose
   * @param[in]  devnum    device number passed to API on expose
   * @param[in]  fpbcount  frames per buffer count ( uiFPBCount ) wraps to 0 at FPB
   * @param[in]  fcount    actual frame counter ( uiPCIFrameCount = getFrameCount() )
   * @param[in]  rows      image rows ( uiRows )
   * @param[in]  cols      image columns ( uiCols )
   * @param[in]  buffer    pointer to PCI image buffer
   *
   * This is the callback function invoked by the ARC API,
   * arc::gen3::CArcDevice::expose() when a new frame is received.
   * Spawn a separate thread to handle the incoming frame.
   *
   * incoming frame from ARC -> frameCallback()
   *
   */
  void Callback::frameCallback( int expbuf, int devnum, std::uint32_t fpbcount, std::uint32_t fcount,
                                std::uint32_t rows, std::uint32_t cols, void* buffer ) {
    if ( ! server.useframes ) fcount=1;  // prevents fcount from being a bad value when firmware doesn't support frames
    std::stringstream message;

    if ( server.controller.find(devnum) == server.controller.end() ) {
      message.str(""); message << "ERROR in AstroCam::Callback::frameCallback (fatal): devnum " << devnum
                               << " not in controller configuration";
      std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
      return;
    }

    message << "FRAMECOUNT_" << devnum << ":" << fcount << " rows=" << rows << " cols=" << cols;
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
    logwrite( "Callback::frameCallback", message.str() );

    server.add_framethread();            // framethread_count is incremented because a thread has been added

    std::thread( std::ref(AstroCam::Interface::handle_frame), expbuf, devnum, fpbcount, fcount, buffer ).detach();

    return;
  }
  /***** AstroCam::Callback::frameCallback ************************************/


  /***** AstroCam::Callback::ftCallback ***************************************/
  /**
   * @brief      called by CArcDevice::frame_transfer() when a frame transfer has been done
   * @details    set/clear control flags and start the readout waveforms
   * @param[in]  expbuf    exposure buffer number passed to API
   * @param[in]  devnum    device number passed to API
   *
   */
  void Callback::ftCallback( int expbuf, int devnum ) {
    std::stringstream message;
    message << "FRAMETRANSFER_" << devnum << ":complete";
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
    logwrite( "Callback::ftCallback", message.str() );

    if ( server.controller.find(devnum) == server.controller.end() ) {
      message.str(""); message << "ERROR in AstroCam::Callback::ftCallback (fatal): devnum " << devnum
                               << " not in controller configuration";
      std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
      return;
    }

    server.controller[devnum].in_frametransfer = false;
    server.exposure_pending( devnum, false );    // this also does the notify
    server.controller[devnum].in_readout       = true;
    server.state_monitor_condition.notify_all();

    // Trigger the readout waveforms here.
    //
    try {
      server.controller[devnum].pArcDev->readout( expbuf,
                                                  devnum,
                                                  server.controller[devnum].info.axes[_ROW_],
                                                  server.controller[devnum].info.axes[_COL_],
                                                  server.camera.abortstate,
                                                  server.controller[devnum].pCallback
                                                );
    }
    catch ( const std::exception &e ) { // arc::gen3::CArcDevice::readout may throw an exception
      message.str(""); message << "ERROR starting readout for " << server.controller[devnum].devname
                               << " channel " << server.controller[devnum].channel << ": " << e.what();
      std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
      return;
    }

    return;
  }
  /***** AstroCam::Callback::ftCallback ***************************************/


  /***** AstroCam::Interface::Interface ***************************************/
  /**
   * @brief      AstroCam Interface class constructor
   *
   */
  Interface::Interface() {
    this->state_monitor_thread_running = false;
    this->modeselected = false;
    this->pci_cmd_num.store(0);
    this->nexp.store(1);
    this->numdev = 0;
    this->nframes = 1;
    this->nfpseq = 1;
    this->useframes = true;
    this->framethreadcount = 0;

    this->pFits.resize( NUM_EXPBUF );           // pre-allocate FITS_file object pointers for each exposure buffer
    this->fitsinfo.resize( NUM_EXPBUF );        // pre-allocate Camera Information object pointers for each exposure buffer

    this->writes_pending.resize( NUM_EXPBUF );  // pre-allocate writes_pending vector for each exposure buffer

    // Initialize STL map of Readout Amplifiers
    // Indexed by amplifier name.
    // The number is the argument for the Arc command to set this amplifier in the firmware.
    //
    // Format here is: { AMP_NAME, { ENUM_TYPE, ARC_ARG } }
    // where AMP_NAME  is the name of the readout amplifier, the index for this map
    //       ENUM_TYPE is an enum of type ReadoutType
    //       ARC_ARG   is the ARC argument for the SOS command to select this readout source
    //
    this->readout_source.insert( { "U1",          { U1,             0x5f5531 } } );  // "_U1"
    this->readout_source.insert( { "L1",          { L1,             0x5f4c31 } } );  // "_L1"
    this->readout_source.insert( { "U2",          { U2,             0x5f5532 } } );  // "_U2"
    this->readout_source.insert( { "L2",          { L2,             0x5f4c32 } } );  // "_L2"
    this->readout_source.insert( { "SPLIT1",      { SPLIT1,         0x5f5f31 } } );  // "__1"
    this->readout_source.insert( { "SPLIT2",      { SPLIT2,         0x5f5f32 } } );  // "__2"
    this->readout_source.insert( { "QUAD",        { QUAD,           0x414c4c } } );  // "ALL"
    this->readout_source.insert( { "FT2",         { FT2,            0x465432 } } );  // "FT2" -- frame transfer from 1->2, read split2
    this->readout_source.insert( { "FT1",         { FT1,            0x465431 } } );  // "FT1" -- frame transfer from 2->1, read split1
//  this->readout_source.insert( { "hawaii1",     { HAWAII_1CH,     0xffffff } } );  ///< TODO @todo implement HxRG  1 channel deinterlacing
//  this->readout_source.insert( { "hawaii32",    { HAWAII_32CH,    0xffffff } } );  ///< TODO @todo implement HxRG 32 channel deinterlacing
//  this->readout_source.insert( { "hawaii32lr",  { HAWAII_32CH_LR, 0xffffff } } );  ///< TODO @todo implement HxRG 32 channel alternate left/right deinterlacing
  }
  /***** AstroCam::Interface::Interface ***************************************/


  /***** AstroCam::Interface::interface ***************************************/
  /**
   * @brief      returns the interface
   * @param[out] iface  reference to string to return the interface type as a string
   * @return     NO_ERROR
   *
   */
  long Interface::interface(std::string &iface) {
    std::string function = "AstroCam::Interface::interface";
    iface = "AstroCam";
    logwrite(function, iface);
    return(NO_ERROR);
  }
  /***** AstroCam::Interface::interface ***************************************/


  /***** AstroCam::Interface::state_monitor_thread ****************************/
  void Interface::state_monitor_thread(Interface& interface) {
    std::string function = "AstroCam::Interface::state_monitor_thread";
    std::stringstream message;
    std::vector<uint32_t> selectdev;

    // notify that the thread is running
    //
    logwrite(function, "starting");
    {
      std::unique_lock<std::mutex> state_lock(interface.state_lock);
      interface.state_monitor_thread_running = true;
    }
    interface.state_monitor_condition.notify_all();
    logwrite(function, "running");

    while ( true ) {
      std::unique_lock<std::mutex> state_lock(interface.state_lock);
      interface.state_monitor_condition.wait(state_lock);

      while ( interface.is_camera_idle() ) {
        selectdev.clear();
        message.str(""); message << "enabling detector idling for channel(s)";
        for ( const auto &dev : interface.devnums ) {
          logwrite(function, std::to_string(dev));
          if ( interface.controller[dev].connected ) {
            selectdev.push_back(dev);
            message << " " << interface.controller[dev].channel;
          }
        }
        if ( selectdev.size() > 0 ) {
          long ret = interface.do_native( selectdev, std::string("IDL") );
          logwrite( function, (ret==NO_ERROR ? "NOTICE: " : "ERROR")+message.str() );
        }
        // Wait for the conditions to change before checking again
        interface.state_monitor_condition.wait( state_lock );
      }
    }

    // notify that the thread is terminating
    // should be impossible
    //
    {
      std::unique_lock<std::mutex> state_lock(interface.state_lock);
      interface.state_monitor_thread_running = false;
    }
    interface.state_monitor_condition.notify_all();
    logwrite(function, "terminating");

    return;
  }
  /***** AstroCam::Interface::state_monitor_thread ****************************/


  /***** AstroCam::Interface::make_image_keywords *****************************/
  void Interface::make_image_keywords( int dev ) {
    std::string function = "AstroCam::Interface::make_image_keywords";
    std::stringstream message;

    auto chan = this->controller[dev].channel;

    auto rows = this->controller[dev].info.axes[_ROW_];
    auto cols = this->controller[dev].info.axes[_COL_];
    auto osrows = this->controller[dev].osrows;
    auto oscols = this->controller[dev].oscols;
    auto skiprows = this->controller[dev].skiprows;
    auto skipcols = this->controller[dev].skipcols;
    int binspec, binspat;
    this->controller[dev].physical_to_logical(this->controller[dev].info.binning[_ROW_],
                                              this->controller[dev].info.binning[_COL_],
                                              binspat,
                                              binspec);

    this->controller[dev].info.systemkeys.add_key( "AMP_ID", this->controller[dev].info.readout_name, "readout amplifier", EXT, chan  );
    this->controller[dev].info.systemkeys.add_key( "FT", this->controller[dev].have_ft, "frame transfer used", EXT, chan );

    this->controller[dev].info.systemkeys.add_key( "IMG_ROWS", this->controller[dev].info.axes[_ROW_], "image rows", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "IMG_COLS", this->controller[dev].info.axes[_COL_]-oscols, "image cols", EXT, chan );

    this->controller[dev].info.systemkeys.add_key( "OS_ROWS", osrows, "overscan rows", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "OS_COLS", oscols, "overscan cols", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "SKIPROWS", skiprows, "skipped rows", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "SKIPCOLS", skipcols, "skipped cols", EXT, chan );

    int L=0, B=0;
    switch ( this->controller[ dev ].info.readout_type ) {
      case L1:     B=1; break;
      case U1:     L=1; B=1; break;
      case U2:     L=1; break;
      case SPLIT1: B=1; break;
      case FT1:    B=1; break;
      default:     L=0; B=0;
    }

    int ltv2 = B * osrows;
    int ltv1 = L * oscols;

//message.str(""); message << "[DEBUG] B=" << B << " L=" << L << " osrows=" << osrows << " oscols=" << oscols
//                         << " binning_row=" << binspat << " binning_col=" << binspec
//                         << " ltv2=" << ltv2 << " ltv1=" << ltv1;
//logwrite(function,message.str() );

    this->controller[dev].info.systemkeys.add_key( "LTV2", ltv2, "", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "LTV1", ltv1, "", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "CRPIX1A", ltv1+1, "", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "CRPIX2A", ltv2+1, "", EXT, chan );

    this->controller[dev].info.systemkeys.add_key( "BINSPEC", binspec, "binning in spectral direction", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "BINSPAT", binspat, "binning in spatial direction", EXT, chan );

    this->controller[dev].info.systemkeys.add_key( "CDELT1A",
                                                   this->controller[dev].info.dispersion*binspec,
                                                   "Dispersion in Angstrom/pixel", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "CRVAL1A",
                                                   this->controller[dev].info.minwavel,
                                                   "Reference value in Angstrom", EXT, chan );

    // These keys are for proper mosaic display.
    // Adjust GAPY to taste.
    //
    int GAPY=20;
    int crval2 = ( this->controller[dev].info.axes[_ROW_] / binspat + GAPY ) * dev;

    this->controller[dev].info.systemkeys.add_key( "CRPIX1", 0, "", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "CRPIX2", 0, "", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "CRVAL1", 0, "", EXT, chan );
    this->controller[dev].info.systemkeys.add_key( "CRVAL2", crval2, "", EXT, chan );

    // Add ___SEC keywords to the extension header for this channel
    //
    std::stringstream sec;

    /* 01-24-2025 ***
    sec.str(""); sec << "[" << this->controller[dev].info.region_of_interest[0] << ":" << this->controller[dev].info.region_of_interest[1]
                     << "," << this->controller[dev].info.region_of_interest[2] << ":" << this->controller[dev].info.region_of_interest[3] << "]";
    this->controller[dev].info.systemkeys.add_key( "CCDSEC", sec.str(), "physical format of CCD", EXT, chan );

    sec.str(""); sec << "[" << this->controller[dev].info.region_of_interest[0] + skipcols << ":" << cols
                     << "," << this->controller[dev].info.region_of_interest[2] + skiprows << ":" << rows << "]";
    this->controller[dev].info.systemkeys.add_key( "DATASEC", sec.str(), "section containing the CCD data", EXT, chan );

    sec.str(""); sec << '[' << cols << ":" << cols+oscols
                     << "," << this->controller[dev].info.region_of_interest[2] + skiprows << ":" << rows+osrows << "]";
    this->controller[dev].info.systemkeys.add_key( "BIASSEC", sec.str(), "overscan section", EXT, chan );
    *** */
    sec.str(""); sec << "[" << oscols+1-2  // -2 is KLUDGE FACTOR
                     << ":" << cols-2      // -2 is KLUDGE FACTOR
                     << "," << this->controller[dev].info.region_of_interest[2] + skiprows << ":" << rows << "]";
    this->controller[dev].info.systemkeys.add_key( "DATASEC", sec.str(), "section containing the CCD data", EXT, chan );

    sec.str(""); sec << "[" << this->controller[dev].info.region_of_interest[0] << ":" << oscols-2  // -2 is KLUDGE FACTOR
                     << "," << this->controller[dev].info.region_of_interest[2] + skiprows << ":" << rows << "]";
    this->controller[dev].info.systemkeys.add_key( "BIASSEC", sec.str(), "overscan section", EXT, chan );

    return;
  }
  /***** AstroCam::Interface::make_image_keywords *****************************/


  /***** AstroCam::Interface::parse_spec_info *********************************/
  /**
   * @brief      parse the SPEC_INFO config line
   * @param[in]  args  line from config file, expected "<chan> <disp> <wavel>"
   * @return     ERROR|NO_ERROR
   *
   */
  long Interface::parse_spec_info( std::string args ) {
    const std::string function("AstroCam::Interface::parse_spec_info");
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    if ( tokens.size() != 3 ) {
      logwrite(function, "ERROR invalid number of args. expected SPEC_INFO={CHAN DISP WAVEL}");
      return ERROR;
    }

    // Parse the three values from the args string
    try {
      int dev = devnum_from_chan(tokens.at(0));
      this->controller[dev].info.dispersion = std::stod(tokens.at(1));
      this->controller[dev].info.minwavel   = std::stod(tokens.at(2));
    }
    catch(const std::exception &e) {
      logwrite(function, "ERROR parsing SPEC_INFO config: "+std::string(e.what()));
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** AstroCam::Interface::parse_spec_info *********************************/


  /***** AstroCam::Interface::parse_det_geometry ******************************/
  /**
   * @brief      parse the DETECTOR_GEOMETRY config line
   * @details    Expected "CHAN PHYSSPAT PHYSSPEC ROWS COLS OSROWS OSCOLS BINROWS BINCOLS"
   * @param[in]  args  line from config file
   * @return     ERROR|NO_ERROR
   *
   */
  long Interface::parse_det_geometry( std::string args ) {
    const std::string function("AstroCam::Interface::parse_det_geometry");
    std::ostringstream message;
    std::vector<std::string> tokens;

    Tokenize( args, tokens, " " );

    if ( tokens.size() != 9 ) {
      logwrite(function, "ERROR invalid number of args. expected CHAN PHYSSPAT PHYSSPEC ROWS COLS OSROWS OSCOLS BINROWS BINCOLS");
      return ERROR;
    }

    try {
      // validate channel
      int dev = devnum_from_chan(tokens.at(0));
      std::string spat = to_uppercase(tokens.at(1));
      std::string spec = to_uppercase(tokens.at(2));

      // require physical axis args be "ROW" or "COL" and unique
      if ( ((spat != "ROW") && (spat != "COL")) || ((spec != "ROW") && (spec != "COL")) ) {
        throw std::runtime_error("expected ROW|COL");
      }
      if (spec==spat) throw std::runtime_error("PHYSSPAT/PHYSSPEC must be unique");

      this->controller[dev].spat_axis = (spat=="ROW" ? Controller::ROW : Controller::COL);
      this->controller[dev].spec_axis = (spec=="ROW" ? Controller::ROW : Controller::COL);
    }
    catch(const std::exception &e) {
      logwrite(function, "ERROR parsing DETECTOR_GEOMETRY config: "+std::string(e.what()));
      return ERROR;
    }

    // make a new string that removes the orientation arguments
    // this will become the config line to pass to image_size
    std::string line, retstring;
    for (size_t i=0; i<9; i++) {
      if (i==1||i==2) continue;
      if (!line.empty()) line += ' ';
      line += tokens[i];
    }
    bool save_as_default = true;
    long error = this->image_size(line, retstring, save_as_default);

    return error;
  }
  /***** AstroCam::Interface::parse_det_geometry ******************************/


  /***** AstroCam::Interface::parse_controller_config *************************/
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
  long Interface::parse_controller_config( std::string args ) {
    std::string function = "AstroCam::Interface::parse_controller_config";
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
      return( ERROR );
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
        message.str(""); message << "unrecognized value for FT: " << tokens.at(2) << ". Expected { yes | no }";
        this->camera.log_error( function, message.str() );
        return( ERROR );
      }
    }
    catch (std::invalid_argument &) {
      this->camera.log_error( function, "invalid number: unable to convert to integer" );
      return(ERROR);
    }
    catch (std::out_of_range &) {
      this->camera.log_error( function, "value out of integer range" );
      return(ERROR);
    }

    // Check the PCIDEV number is in expected range
    //
    if ( dev < 0 || dev > 3 ) {
      message.str(""); message << "ERROR: bad PCIDEV " << dev << ". Expected {0,1,2,3}";
      this->camera.log_error( function, message.str() );
      return( ERROR );
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
      return( ERROR );
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

/*
    arc::gen3::CArcDevice* pArcDev = NULL;          // create a generic CArcDevice pointer
    pArcDev = new arc::gen3::CArcPCI();             // point this at a new CArcPCI() object  ///< TODO @todo implement PCIe option
    Callback* pCB = new Callback();                 // create a pointer to a Callback() class object

    this->controller[dev].pArcDev = pArcDev;        // set the pointer to this object in the public vector
    this->controller[dev].pCallback = pCB;          // set the pointer to this object in the public vector
*/
    this->controller[dev].pArcDev = ( new arc::gen3::CArcPCI() );        // set the pointer to this object in the public vector
    this->controller[dev].pCallback = ( new Callback() );          // set the pointer to this object in the public vector
    this->controller[dev].devname = "";             // device name
    this->controller[dev].connected = false;        // not yet connected
    this->controller[dev].is_imsize_set = false;    // need to set image_size
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

//  FITS_file* pFits = new FITS_file();             // create a pointer to a FITS_file class object
//  this->controller[dev].pFits = pFits;            // set the pointer to this object in the public vector

#ifdef LOGLEVEL_DEBUG
    message.str("");
    message << "[DEBUG] pointers for dev " << dev << ": "
            << " pArcDev=" << std::hex << std::uppercase << this->controller[dev].pArcDev
            << " pCB="     << std::hex << std::uppercase << this->controller[dev].pCallback;
    logwrite(function, message.str());
#endif
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::parse_controller_config *************************/


  /***** AstroCam::Interface::devnum_from_chan ********************************/
  /**
   * @brief      return the devnum associated with a channel name
   * @param[out] chan    reference to channel name
   * @return     devnum  device number
   * @throws     std::runtime_error
   *
   */
  int Interface::devnum_from_chan( const std::string &chan ) {
    int devnum=-1;
    for ( const auto &con : this->controller ) {
      if ( con.second.inactive ) continue; // skip controllers flagged as inactive
      if ( con.second.channel == chan ) {  // check to see if it matches a configured channel.
        devnum = con.second.devnum;
        break;
      }
    }
    if (devnum < 0) {
      throw std::runtime_error("no devnum configured for channel \"" + chan + "\"");
    }
    if ( this->controller.find(devnum) == this->controller.end() ) {
      std::ostringstream oss;
      oss << "device " << devnum << " not found in controller configuration";
      throw std::runtime_error(oss.str());
    }
    return devnum;
  }
  /***** AstroCam::Interface::devnum_from_chan ********************************/


  /***** AstroCam::Interface::extract_dev_chan ********************************/
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
  long Interface::extract_dev_chan( std::string args, int &dev, std::string &chan, std::string &retstring ) {
    std::string function = "AstroCam::Interface::extract_dev_chan";
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
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** AstroCam::Interface::extract_dev_chan ********************************/


  long Interface::do_abort() {
    std::string function = "AstroCam::Interface::do_abort";
    std::stringstream message;
//  int this_expbuf = this->get_expbuf();
    for ( const auto &dev : this->devnums ) {
      this->exposure_pending( dev, false );
      for ( int buf=0; buf < NUM_EXPBUF; ++buf ) this->write_pending( buf, dev, false );
    }
    this->state_monitor_condition.notify_all();
    logwrite( function, "[DEBUG] exposure pending set to false" );
    return( NO_ERROR );
  }

  /***** AstroCam::Interface::do_bin ******************************************/
  /**
   * @brief      set/get binning factor
   * @details    Since binning is set together with all image parameters, this
   *             function will call image_size(), and using this function is
   *             a convenience so that the user can change only binning.
   * @param[in]  args        argument string contains <axis> [<factor>]
   * @param[out] retstring   reference to string to return error or help
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::do_bin( std::string args, std::string &retstring ) {
    std::string function = "AstroCam::Interface::do_bin";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( args.empty() || args == "?" ) {
      retstring = CAMERAD_BIN;
      retstring.append( " <axis> [ <binfactor> ]\n" );
      retstring.append( "  set or get binning factor for the specified axis.\n" );
      retstring.append( "  This affects all channels.\n" );
      retstring.append( "  If <binfactor> is omitted then the current binning factor is returned.\n" );
      retstring.append( "  Specify <axis> from { spat spec }\n" );
      retstring.append( "    spec = spectral (dispersion) axis\n" );
      retstring.append( "    spat = spatial (slit) axis\n" );
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

    // Don't make any changes while an exposure is pending.
    //
    if ( this->exposure_pending() ) {
      std::vector<int> pending = this->exposure_pending_list();
      message.str(""); message << "ERROR: cannot change binning while exposure is pending for chan";
      message << ( pending.size() > 1 ? "s " : " " );
      for ( const auto &dev : pending ) message << this->controller[dev].channel << " ";
      this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
      retstring="exposure_in_progress";
      return(ERROR);
    }

    // Tokenize args to get the axis and possible binning factor. There
    // must be either 2 tokens (<axis> <bin> to set) or 1 token (<axis> to get).
    //
    std::vector<std::string> tokens;
    Tokenize( args, tokens, " " );

    std::string logical_axis;
    int binfactor=-1;

    try {
      if ( tokens.size() > 0 ) {
        logical_axis = tokens.at(0);
        if (logical_axis != "spec" && logical_axis != "spat") {
          logwrite(function, "ERROR unknown axis \""+logical_axis+"\". expected { spat spec }");
          retstring="bad_arguments";
          return ERROR;
        }
      }

      if ( tokens.size() == 2 ) {
        binfactor = std::stoi( tokens.at(1) );

        if ( binfactor < 1 ) {
          logwrite( function, "ERROR binfactor " +tokens.at(1)+ " must be greater than 0.");
          retstring="invalid_argument";
          return ERROR;
        }

        // Now, since binning applies equally to all devices and the image
        // must be resized for binning, set the image size for each device.
        // This uses the existing image size parameters and the new binning.
        // The requested overscans are sent here, which can be modified by binning.
        //
        for ( const auto &dev : this->devnums ) {
          Controller* pcontroller = &this->controller[dev];
          // determine which physical axis corresponds to the requested logical axis
          int physical_axis;
          if (logical_axis == "spec") {
            physical_axis = pcontroller->spec_physical_axis();
          }
          else {
            physical_axis = pcontroller->spat_physical_axis();
          }

          // Make a local copy of the class' binning for both (physical) axes
          //
          int _binning[2];
          _binning[_ROW_] = pcontroller->info.binning[_ROW_];
          _binning[_COL_] = pcontroller->info.binning[_COL_];
          // then override only the axis requested here.
          _binning[physical_axis] = binfactor;

          // call image_size() with logical coordinates
          int spat, spec, osspat, osspec, binspat, binspec;
          pcontroller->physical_to_logical(pcontroller->detrows, pcontroller->detcols,
                                           spat, spec);
          pcontroller->physical_to_logical(pcontroller->osrows0, pcontroller->oscols0,
                                           osspat, osspec);
          pcontroller->physical_to_logical(_binning[_ROW_], _binning[_COL_],
                                           binspat, binspec);
          message.str("");
          message << dev     << " "
                  << spat    << " "
                  << spec    << " "
                  << osspat  << " "
                  << osspec  << " "
                  << binspat << " "
                  << binspec;
          error = this->image_size( message.str(), retstring );  // this retstring only used on error
          if (error != NO_ERROR) break;
        }
      }
      else if ( tokens.size() > 2 ) {
        message.str(""); message << "ERROR: expected <axis> [ <binfactor> ] but received \"" << retstring << "\"";
        logwrite( function, message.str() );
        retstring="bad_arguments";
        return( ERROR );
      }

      // return binning for the requested logical axis
      if (this->numdev>0) {
        int dev = this->devnums[0];
        int physical_axis = (logical_axis=="spec") ? this->controller[dev].spec_physical_axis() :
                                                     this->controller[dev].spat_physical_axis();
        message.str(""); message << this->controller[dev].info.binning[physical_axis];
        if ( error == NO_ERROR ) retstring = message.str();
      }
    }
    catch ( std::exception &e ) {
      message.str(""); message << "ERROR: parsing \"" << args << "\": " << e.what();
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return( ERROR );
    }

    logwrite( function, message.str() );

    return( error );
  }
  /***** AstroCam::Interface::do_bin ******************************************/


  /***** AstroCam::Interface::do_connect_controller ***************************/
  /**
   * @brief      opens a connection to the PCI/e device(s)
   * @param[in]  devices_in  optional string containing space-delimited list of devices
   * @param[out] retstring   reference to string to return error or help
   * @return     ERROR | NO_ERROR | HELP
   *
   * Input parameter devices_in defaults to empty string which will attempt to
   * connect to all detected devices.
   *
   * If devices_in is specified (and not empty) then it must contain a space-delimited
   * list of device numbers to open. A public vector devnums will hold these device
   * numbers. This vector will be updated here to represent only the devices that
   * are actually connected.
   *
   * All devices requested must be connected in order to return success. It is
   * considered an error condition if not all requested can be connected. If the
   * user wishes to connect to only the device(s) available then the user must
   * call with the specific device(s). In other words, it's all (requested) or nothing.
   *
   */
  long Interface::do_connect_controller( const std::string devices_in, std::string &retstring ) {
    std::string function = "AstroCam::Interface::do_connect_controller";
    std::stringstream message;
    long error = NO_ERROR;

    // Help
    //
    if ( devices_in == "?" || devices_in == "help" ) {
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
      logwrite(function, "ERROR: no devices found");
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
    // space-delimited string devices_in. The devices to open
    // are stored in a public vector "devnums".
    //

    // If no string is given then use vector of configured devices. The configured_devnums
    // vector contains a list of devices defined in the config file with the
    // keyword CONTROLLER=(<PCIDEV> <CHAN> <FT> <FIRMWARE>).
    //
    if ( devices_in.empty() ) this->devnums = this->configured_devnums;
    else {
      // Otherwise, tokenize the device list string and build devnums from the tokens
      //
      this->devnums.clear();                          // empty devnums vector since it's being built here
      std::vector<std::string> tokens;
      Tokenize(devices_in, tokens, " ");
      for ( const auto &n : tokens ) {                // For each token in the devices_in string,
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
          return(ERROR);
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

    // Open only the devices specified by the devnums vector
    //
    for ( size_t i = 0; i < this->devnums.size(); ) {
      int dev = this->devnums[i];
      auto dev_found = this->controller.find( dev );

      if ( dev_found == this->controller.end() ) {
        message.str(""); message << "ERROR: devnum " << dev << " not found in controller definition. check config file";
        logwrite( function, message.str() );
        this->controller[dev].inactive=true;      // flag the non-connected controller as inactive
        this->do_disconnect_controller(dev);
        retstring="unknown_device";
        error = ERROR;
        break;
      }
      else this->controller[dev].inactive=false;

      try {
        // Open the PCI device if not already open
        // (otherwise just reset and test connection)
        //
        if ( ! this->controller[dev].connected ) {
          message.str(""); message << "opening " << this->controller[dev].devname;
          logwrite(function, message.str());
          this->controller[dev].pArcDev->open(dev);
        }
        else {
          message.str(""); message << this->controller[dev].devname << " already open";
          logwrite(function, message.str());
        }

        // Reset the PCI device
        //
        message.str(""); message << "resetting " << this->controller[dev].devname;
        logwrite(function, message.str());
        try {
          this->controller[dev].pArcDev->reset();
        }
        catch (const std::exception &e) {
          message.str(""); message << "ERROR resetting " << this->controller[dev].devname << ": " << e.what();
          logwrite(function, message.str());
          error = ERROR;
        }

        // Is Controller Connected?  (tested with a TDL command)
        //
        this->controller[dev].connected = this->controller[dev].pArcDev->isControllerConnected();
        message.str(""); message << this->controller[dev].devname << (this->controller[dev].connected ? "" : " not" ) << " connected to ARC controller"
                                 << (this->controller[dev].connected ? " for channel " : "" )
                                 << (this->controller[dev].connected ? this->controller[dev].channel : "" );
        logwrite(function, message.str());

        // If not connected then this should remove it from the devnums list
        //
        if ( !this->controller[dev].connected ) this->do_disconnect_controller(dev);

/****** YOU CAN'T DO THIS
        // Now that controller is open, update it with the current image size
        // that has been stored in the class. Create an arg string in the same
        // format as that found in the config file.
        //
        std::stringstream args;
        std::string retstring;
        args << dev << " "
             << this->controller[dev].detrows << " "
             << this->controller[dev].detcols << " "
             << this->controller[dev].osrows  << " "
             << this->controller[dev].oscols  << " "
             << this->camera_info.binning[_ROW_] << " "
             << this->camera_info.binning[_COL_];

        // If image_size fails then close only this controller,
        // which allows operating without this one if needed.
        //
        if ( this->image_size( args.str(), retstring ) != NO_ERROR ) {  // set IMAGE_SIZE here after opening
          message.str(""); message << "ERROR setting image size for " << this->controller[dev].devname << ": " << retstring;
          this->camera.async.enqueue_and_log( function, message.str() );
          this->controller[dev].inactive=true;      // flag the non-connected controller as inactive
          this->do_disconnect_controller(dev);
          error = ERROR;
        }
 ******/
      }
      catch ( const std::exception &e ) { // arc::gen3::CArcPCI::open and reset may throw exceptions
        message.str(""); message << "ERROR opening " << this->controller[dev].devname
                                 << " channel " << this->controller[dev].channel << ": " << e.what();
        this->camera.async.enqueue_and_log( function, message.str() );
        this->controller[dev].inactive=true;      // flag the non-connected controller as inactive
        this->do_disconnect_controller(dev);
        retstring="exception";
        error = ERROR;
      }
      // A call to do_disconnect_controller() can modify the size of devnums,
      // so only if the loop index i is still valid with respect to the current
      // size of devnums should it be incremented.
      //
      if ( i < devnums.size() ) ++i;
    }

    // Log the list of connected devices
    //
    message.str(""); message << "connected devices { ";
    for (const auto &devcheck : this->devnums) { message << devcheck << " "; } message << "}";
    logwrite(function, message.str());

    // check the size of the devnums now, against the size requested
    //
    if ( this->devnums.size() != requested_device_count ) {
      message.str(""); message << "ERROR: " << this->devnums.size() <<" connected device(s) but "
                               << requested_device_count << " requested";
      logwrite( function, message.str() );

      // disconnect/deconstruct everything --
      //
      // If the user might want to use what is available then the user
      // must call again, requesting only what is available. It is an
      // error if the interface cannot deliver what was requested.
      //
      this->do_disconnect_controller();

      retstring="bad_device_count";
      error = ERROR;
    }

    // Start a thread to monitor the state of things (if not already running)
    //
    if ( !this->state_monitor_thread_running.load() ) {
      std::thread( std::ref(AstroCam::Interface::state_monitor_thread), std::ref(*this) ).detach();
      std::unique_lock<std::mutex> state_lock( this->state_lock );
      if ( !this->state_monitor_condition.wait_for( state_lock,
                                                    std::chrono::milliseconds(1000),
                                                    [this] { return this->state_monitor_thread_running.load(); } ) ) {
        logwrite( function, "ERROR: state_monitor_thread did not start" );
        retstring="internal_error";
        error = ERROR;
      }
    }

    // As the last step to opening the controller, this is where I've chosen
    // to initialize the Shutter class, required before using the shutter.
    //
    if ( this->camera.shutter.init(this->camera.bonn_shutter) != NO_ERROR ) {
      retstring="shutter_error";
      error = ERROR;
    }

    return( error );
  }
  /***** AstroCam::Interface::do_connect_controller ***************************/


  /***** AstroCam::Interface::do_disconnect_controller ************************/
  /**
   * @brief      closes the connection to the specified PCI/e device
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Interface::do_disconnect_controller( int dev ) {
    std::string function = "AstroCam::Interface::do_disconnect_controller";
    std::stringstream message;

    if ( !this->is_camera_idle() ) {
      logwrite( function, "ERROR: cannot close controller while camera is active" );
      return ERROR;
    }

    // close indicated PCI device and remove dev from devnums
    //
    try {
      if ( this->controller.at(dev).pArcDev == nullptr ) {
        message.str(""); message << "ERROR no ARC device for dev " << dev;
        logwrite( function, message.str() );
        return ERROR;
      }
      message.str(""); message << "closing " << this->controller.at(dev).devname;
      logwrite(function, message.str());
      this->controller.at(dev).pArcDev->close();  // throws nothing, no error handling
      this->controller.at(dev).connected=false;
      // remove dev from devnums
      //
      auto it = std::find( this->devnums.begin(), this->devnums.end(), dev );
      if ( it != this->devnums.end() ) {
        this->devnums.erase(it);
      }
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "dev " << dev << " not found: " << e.what();
      this->camera.log_error( function, message.str() );
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** AstroCam::Interface::do_disconnect_controller ************************/


  /***** AstroCam::Interface::do_disconnect_controller ************************/
  /**
   * @brief      closes the connection to all PCI/e devices
   * @return     ERROR or NO_ERROR
   *
   * no error handling. can only fail if the camera is busy.
   *
   * This function is overloaded
   *
   */
  long Interface::do_disconnect_controller() {
    std::string function = "AstroCam::Interface::do_disconnect_controller";
    std::stringstream message;
    long error = NO_ERROR;

    if ( !this->is_camera_idle() ) {
      logwrite( function, "ERROR: cannot close controller while camera is active" );
      return( ERROR );
    }

    // close all of the PCI devices
    //
    for ( auto &con : this->controller ) {
      message.str(""); message << "closing " << con.second.devname;
      logwrite(function, message.str());
      if ( con.second.pArcDev != nullptr ) con.second.pArcDev->close();  // throws nothing
      con.second.connected=false;
    }

    this->devnums.clear();   // no devices open
    this->numdev = 0;        // no devices open
    return error;
  }
  /***** AstroCam::Interface::do_disconnect_controller ************************/


  /***** AstroCam::Interface::is_connected ************************************/
  /**
   * @brief      are all selected controllers connected?
   * @param[out] retstring  reference to string to return "true|false"
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::is_connected( std::string &retstring ) {
    std::string function = "AstroCam::Interface::is_connected";
    std::stringstream message;

    size_t ndev = this->devnums.size();  /// number of connected devices
    size_t nopen=0;                      /// number of open devices (should be equal to ndev if all are open)

    // look through all connected devices
    //
    for ( const auto &dev : this->devnums ) {
      if ( this->controller.find( dev ) != this->controller.end() )
        if ( this->controller[dev].connected ) nopen++;
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] " << this->controller[dev].devname << " is " << ( this->controller[dev].connected ? "connected" : "disconnected" );
        logwrite( function, message.str() );
#endif
    }

    // If all devices in (non-empty) devnums are connected then return true,
    // otherwise return false.
    //
    if ( ndev !=0 && ndev == nopen ) {
      retstring = "true";
    }
    else {
      retstring = "false";
    }

    return( NO_ERROR );
  }
  /***** AstroCam::Interface::is_connected ************************************/


  /***** AstroCam::Interface::do_configure_controller *************************/
  /**
   * @brief      perform initial configuration of controller from .cfg file
   * @return     ERROR or NO_ERROR
   *
   * Called automatically by main() when the server starts up.
   *
   */
  long Interface::do_configure_controller() {
    const std::string function("AstroCam::Interface::do_configure_controller");
    int numapplied=0, lastapplied=0;
    long error = NO_ERROR;

    // Initialize the vector of configured device numbers,
    // which will get built up from parse_controller_config() below.
    //
    this->configured_devnums.clear();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      lastapplied=numapplied;

      if ( this->config.param[entry].find( "CONTROLLER" ) == 0 ) {
        if ( this->parse_controller_config( this->config.arg[entry] ) != ERROR ) {
          numapplied++;
        }
      }
      else

      // DETECTOR_GEOMETRY
      if ( this->config.param[entry] == "DETECTOR_GEOMETRY" ) {
        if ( this->parse_det_geometry(this->config.arg[entry]) != ERROR ) {
          numapplied++;
        }
      }
      else

      if ( this->config.param[entry].find( "IMDIR" ) == 0 ) {
        this->camera.imdir( config.arg[entry] );
        numapplied++;
      }
      else

      if ( config.param[entry].find( "DIRMODE" ) == 0 ) {
        try {
          if ( !config.arg[entry].empty() ) {
            mode_t mode = (mode_t)std::stoi( config.arg[entry] );
            this->camera.set_dirmode( mode );
          }
        }
        catch (const std::exception &e) {
          this->camera.log_error(function, "parsing DIRMODE: "+std::string(e.what()));
          return ERROR;
        }
        numapplied++;
      }
      else

      if ( this->config.param[entry].find( "BASENAME" ) == 0 ) {
        this->camera.basename( config.arg[entry] );
        numapplied++;
      }
      else

      // If the Bonn shutter has been disabled ( = "no" ) then add a FITS keyword
      // to indicate that. If present, which is the normal operating condition,
      // then remove that keyword -- only need to flag this if non-standard condition.
      //
      if ( this->config.param[entry].find( "BONN_SHUTTER" ) == 0 ) {
        std::string bs = config.arg[entry];
        if ( !bs.empty() && bs=="no" ) {
          this->camera.bonn_shutter = false;
          this->camera_info.systemkeys.primary().addkey( "BONNSHUT", false, "Bonn shutter not in use" );
        }
        else
        if ( !bs.empty() && bs=="yes" ) {
          this->camera.bonn_shutter = true;
          this->camera_info.systemkeys.primary().delkey( "BONNSHUT" );
        }
        else {
          this->camera.log_error( function, "BONN_SHUTTER expected yes | no" );
          return( ERROR );
        }
        numapplied++;
      }
      else

      if ( this->config.param[entry] == "SHUTTER_DELAY" ) {
        if ( !config.arg[entry].empty() ) {
          if (this->camera.set_shutter_delay( config.arg[entry] )==NO_ERROR) {
            numapplied++;
          }
          else return ERROR;
        }
      }
      else

      // If an external shutter has been enabled ( = "yes" ) then add a FITS keyword
      // to indicate that. If not present, which is the normal operating condition,
      // then remove that keyword -- only need to flag this is in use.
      //
      if ( this->config.param[entry].find( "EXT_SHUTTER" ) == 0 ) {
        std::string es = config.arg[entry];
        if ( !es.empty() && es=="yes" ) {
          this->camera.ext_shutter = true;
          this->camera_info.systemkeys.primary().addkey( "EXT_SHUT", true, "external shutter trigger in use" );
        }
        else
        if ( !es.empty() && es=="no" ) {
          this->camera.ext_shutter = false;
          this->camera_info.systemkeys.primary().delkey( "EXT_SHUT" );
        }
        else {
          this->camera.log_error( function, "EXT_SHUTTER expected yes | no" );
          return( ERROR );
        }
        numapplied++;
      }
      else

      if ( this->config.param[entry] == "SPEC_INFO" ) {
        if ( this->parse_spec_info( this->config.arg[entry] ) != ERROR ) {
          numapplied++;
        }
      }

      if (numapplied > lastapplied) {
        std::ostringstream oss;
        oss << "CAMERAD:config:" << this->config.param[entry] << "=" << this->config.arg[entry];
        this->camera.async.enqueue_and_log(function, oss.str());
      }
    }

    std::ostringstream message;

    if (numapplied==0) {
      message << "ERROR: ";
      error = ERROR;
    }
    message << "applied " << numapplied << " configuration lines to controller";
    logwrite(function, message.str());

    return error;
  }
  /***** AstroCam::Interface::do_configure_controller *************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to specified or all open Leach controllers
   * @details    See details in overloaded do_native(std::string args, std::string &retstring)
   * @param[in]  args  string containing optional DEV|CHAN plus command and arguments
   * @return     NO_ERROR on success, ERROR on error
   *
   */
  long Interface::do_native( std::string args ) {
    std::string dontcare;
    return this->do_native( args, dontcare );
  }
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
  long Interface::do_native( std::string args, std::string &retstring ) {
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
  long Interface::do_native(std::vector<uint32_t> selectdev, std::string cmdstr) {
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
  long Interface::do_native( int dev, std::string cmdstr, std::string &retstring ) {
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
  long Interface::do_native( std::vector<uint32_t> selectdev, std::string cmdstr, std::string &retstring ) {
    std::string function = "AstroCam::Interface::do_native";
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

    // If no command passed then nothing to do here
    //
    if ( cmdstr == "?" ) {
      retstring = CAMERAD_NATIVE;
      retstring.append( " <CMD> [ <ARG1> [ < ARG2> [ <ARG3> [ <ARG4> ] ] ] ]\n" );
      retstring.append( "  send 3-letter command <CMD> with up to four optional args to all open ARC controllers\n" );
      retstring.append( "  Input <CMD> is not case-sensitive and any values default to base-10\n" );
      retstring.append( "  unless preceeded by 0x to indicate base-16 (e.g. rdm 0x400001).\n" );
      return HELP;
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

//    // Log the complete command (with arg list) that will be sent
//    //
//    message.str("");   message << "sending command:"
//                               << std::setfill('0') << std::setw(2) << std::hex << std::uppercase;
//    for (const auto &arg : cmd) message << " 0x" << arg;
//    logwrite(function, message.str());
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
//    std::thread thr( std::ref(AstroCam::Interface::dothread_native), std::ref(this->controller[dev]), cmd );
      std::thread thr( &AstroCam::Interface::dothread_native, std::ref(*this), dev, cmd );
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
/***
    for ( const auto &dev : selectdev ) {
      // any command that doesn't return DON sets error flag
      if ( this->controller[dev].retval != 0x00444F4E ) {
        error = ERROR;
      }

//    std::string retvalstring;
//    this->retval_to_string( this->controller[dev].retval, retvalstring );
//    message.str(""); message << this->controller[dev].devname << " \"" << cmdstr << "\""
//                             << " returns " << retvalstring
//                             << " (0x" << std::hex << std::uppercase << this->controller[dev].retval << ")";
//    logwrite(function, message.str());
    }
***/
    return error;
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCam::Interface::retval_to_string ********************************/
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
  void Interface::retval_to_string( std::uint32_t retval, std::string& retstring ) {
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
    return;
  }
  /***** AstroCam::Interface::retval_to_string ********************************/


  /***** AstroCam::Interface::dothread_shutter ********************************/
  /**
   * @brief      run in a thread to operate the shutter
   * @param[in]  cam  reference to Camera class object
   *
   * This function uses the std::chrono API to sleep for the requested
   * exposure time. The shutter open and close times are recorded before
   * and after the sleep, and the chrono API's duration class is used
   * to record the actual shutter-open time.
   *
   * When the shutter closes, this function will notify all threads
   * that are waiting on the camera.shutter condition variable.
   *
   * @TODO make the shutter timer able to be stopped/paused/resumed!
   *
   */
  void Interface::dothread_shutter( int expbuf, Interface &interface ) {
    const std::string function("AstroCam::Interface::dothread_shutter");
    std::stringstream message;
    std::string timestring;
    timespec timenow;
    double mjd0, mjd1, mjd;
    double airmass0, airmass1, airmass;

    if ( !interface.in_readout() ) {
      logwrite( function, "NOTICE: sending command to stop clocks!" );
      interface.do_native( "SPC" );
    }

    // get the airmass now
    //
    interface.collect_telemetry_key( "tcsd", "AIRMASS", airmass0 );

    // If configured, send a command to the ARC controller to open
    // the shutter. This is not connected to the shutter but can be
    // used to trigger an external light source for testing. This
    // is done first, before operating the Bonn shutter, so as not
    // to introduce any delay in the real shutter timing (and will
    // be closed after the real shutter has closed).
    //
    if ( interface.camera.ext_shutter ) {
      interface.do_native( "OSH" );
      interface.camera.async.enqueue_and_log( function, "NOTICE:external shutter opened at "+timestring );
    }

    // open the Bonn shutter if enabled
    //
    interface.camera.shutter.set_open();

    // Log shutter open time
    //
    timenow = Time::getTimeNow();            // get the time NOW
    timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
    mjd0 = mjd_from( timenow );              // modified Julian date of start

    if ( interface.camera.shutter.is_enabled ) {
      interface.camera.async.enqueue_and_log( function, "NOTICE:shutter opened at "+timestring );
    }

    // spawn a thread to report exposure progress
    //
    std::thread( &AstroCam::Interface::exposure_progress, std::ref(interface) ).detach();

    // Here is the shutter timer. This is a PreciseTimer object.
    //
    interface.camera.shutter_timer.delay( interface.camera.exposure_time );

    // close the Bonn shutter
    //
    interface.camera.shutter.set_close();

    // Log shutter close time
    //
    timenow = Time::getTimeNow();            // get the time NOW
    timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
    mjd1 = mjd_from( timenow );              // modified Julian date of stop
    mjd = (mjd1+mjd0)/2.;                    // average mjd

    if ( interface.camera.shutter.is_enabled ) {
      interface.camera.async.enqueue_and_log( function, "NOTICE:shutter closed at "+timestring );
    }

    // Send external close shutter command, if configured.
    //
    if ( interface.camera.ext_shutter ) {
      interface.do_native( "CSH" );
      interface.camera.async.enqueue_and_log( function, "NOTICE:external shutter closed at "+timestring );
    }

    // get the airmass again
    //
    interface.collect_telemetry_key( "tcsd", "AIRMASS", airmass1 );

    // average airmass
    //
    airmass = ( airmass1 + airmass0 ) / 2.0;

    interface.camera.shutter.condition.notify_all();  // notify waiting threads that the shutter has closed
    logwrite(function,"[DEBUG] threads notified of shutter closure");

    // Add keywords to primary keyword database for this expbuf.
    // These have to be added to fitsinfo[expbuf] because the exposure has already started,
    // and camera_info keys have already been locked-in to fitsinfo[].
    //
    interface.fitsinfo[expbuf]->systemkeys.primary().addkey( "EXPSTART", timestring, "exposure start time" );
    interface.fitsinfo[expbuf]->systemkeys.primary().addkey( "MJD0", mjd0, "exposure start time (modified Julian Date)" );
    interface.fitsinfo[expbuf]->systemkeys.primary().addkey( "MJD1", mjd1, "exposure stop time (modified Julian Date)" );
    interface.fitsinfo[expbuf]->systemkeys.primary().addkey( "MJD", mjd, "average of MJD0 and MJD1" );
    interface.fitsinfo[expbuf]->systemkeys.primary().addkey(
        "SHUTTIME", (interface.camera.shutter.is_enabled ? interface.camera.shutter.get_duration() : NAN ),
        "actual shutter open time in sec", 3 );
    interface.fitsinfo[expbuf]->systemkeys.primary().addkey( "AIRMASS0", airmass0, "airmass at start of exposure" );
    interface.fitsinfo[expbuf]->systemkeys.primary().addkey( "AIRMASS1", airmass1, "airmass at end of exposure" );
    interface.fitsinfo[expbuf]->systemkeys.primary().addkey( "AIRMASS", airmass, "average of AIRMASS0 and AIRMASS1" );
    message.str(""); message << "[DEBUG] wrote fitsinfo[" << expbuf << "] keys like AIRMASS " << airmass; logwrite(function,message.str());
    return;
  }
  /***** AstroCam::Interface::dothread_shutter ********************************/


  /***** AstroCam::Interface::dothread_read ***********************************/
  /**
   * @brief      run in a thread to trigger the appropriate readout waveforms
   * @details    This thread calls the ARC API directly, for the device
   *             specified by con, so it must be called for each one.
   *             This thread may block until the shutter closes.
   * @param[in]  cam  reference to Camera class object
   * @param[in]  con  reference to Controller class object
   *
   */
  void Interface::dothread_read( Camera::Camera &cam, Controller &con, int expbuf ) {
    const std::string function("AstroCam::Interface::dothread_read");
    std::stringstream message;
    std::string which_waveforms;

#ifdef LOGLEVEL_DEBUG
    message.str("");
    message << "[DEBUG] expbuf " << expbuf << " con.devnum=" << con.devnum << " .channel=" << con.channel
            << " .devname=" << con.devname << " con.section_size=" << con.info.section_size
            << " shutterenable=" << con.info.shutterenable;
    logwrite(function, message.str());
#endif

    // Block, waiting for shutter thread to notify shutter closure.
    // This is conditional (on shutter_state) so if the shutter has already closed
    // then there is no waiting.
    //
    while ( !cam.shutter.isclosed() ) {
      std::unique_lock<std::mutex> shutter_lock( cam.shutter.lock );
      cam.shutter.condition.wait( shutter_lock );
      shutter_lock.unlock();
    }

#ifdef LOGLEVEL_DEBUG
    if ( cam.exposure_time > 0 ) {
      message.str(""); message << "[DEBUG] dev " << con.devnum << " chan " << con.channel << " detected shutter closure!";
    }
    else {
      message.str(""); message << "[DEBUG] dev " << con.devnum << " chan " << con.channel << " no need to wait for shutter closure";
    }
    logwrite(function, message.str());
#endif

    // Wait for any shutter delay before reading out.
    //
    if ( cam.get_shutter_delay() > 0 ) cam.wait_shutter_delay();

    // Trigger the readout waveforms here.
    // A callback function triggered by the ARC API will perform the writing of frames to disk.
    //
    try {
      // set the exposure_pending flag for this controller
      //
      server.exposure_pending( con.devnum, true );
      server.state_monitor_condition.notify_all();
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] expbuf " << expbuf << " dev " << con.devnum << " chan " << con.channel << " exposure_pending=true";
      logwrite( function, message.str() );
#endif

      // allocate PCI buffer and set geometry now
      //
/*
      std::stringstream geostring;
      std::string retstring;
      geostring << con.devnum << " "
                << con.info.axes[_ROW_] << " "
                << con.info.axes[_COL_];

      if ( server.buffer( geostring.str(), retstring ) != NO_ERROR ) {
        message.str(""); message << "ERROR: allocating buffer for chan " << con.channel
                                 << " " << con.devname;
        logwrite( function, message.str() );
        con.error = ERROR;
        return;
      }

      if ( server.do_geometry( geostring.str(), retstring ) != NO_ERROR ) {
        message.str(""); message << "ERROR: setting geometry for chan " << con.channel;
        logwrite( function, message.str() );
        con.error = ERROR;
        return;
      }
*/

/*
      std::stringstream cmd;
      cmd.str(""); cmd << "WRM 0x400006 " << server.camera_info.binning[_ROW_]; // this->camera.binning[_ROW_];            // NPBIN
      if ( server.do_native( con.devnum, cmd.str(), retstring ) != NO_ERROR ) { con.error=ERROR; return; }
      cmd.str(""); cmd << "WRM 0x400029 " << skiprows;                               // NP_SKIP
      if ( server.do_native( con.devnum, cmd.str(), retstring ) != NO_ERROR ) { con.error=ERROR; return; }
*/

      // If this controller is not using Frame Transfer then start the readout waveforms.
      //
      if ( ! con.have_ft ) {
        con.in_readout = true;
        server.state_monitor_condition.notify_all();
        which_waveforms="readout";

        // **********************************************************
        // ********** START CCD READOUT WAVEFORMS HERE **************
        // **********************************************************

        // Send the actual command to start the readout waveforms.
        // This API call will send the SRE command to trigger the readout.
        // The expbuf and devnum are passed in so that the Callback functions know
        // which exposure and device they belong to, respectively.
        //
        con.pArcDev->readout( expbuf,
                              con.devnum,
                              con.info.axes[_ROW_],
                              con.info.axes[_COL_],
                              cam.abortstate,
                              con.pCallback
                            );
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] pArcDev->readout waveforms started on " << con.devname << " for exposure buffer " << expbuf;
        logwrite( function, message.str() );
#endif
      }

      // Otherwise, if this controller is using Frame Transfer, then start the Frame Transfer waveforms.
      //
      else {
        double clock_timeout = get_clock_time() + 60.;  // @todo need to use an appropriate timeout variable here instead of 60
        while ( con.in_readout ) {                      // wait for any previous readout to complete
          if ( get_clock_time() > clock_timeout ) {     // check for a timeout
            con.error = ERROR;
            cam.async.enqueue_and_log( "CAMERAD", function, "ERROR: timeout waiting for previous readout, FT not started" );
            cam.set_abortstate( true );
            return;
          }
        }
        con.in_frametransfer = true;
        server.state_monitor_condition.notify_all();
        which_waveforms="frame_transfer";

        // **********************************************************
        // ********** START CCD FRAME TRANSFER HERE *****************
        // **********************************************************

        // Send the actual command to start the frame transfer waveforms.
        // This API call will send the FRT command to trigger the frame transfer.
        // The expbuf and devnum are passed in so that the Callback functions know
        // which exposure and device they belong to, respectively.
        //
        con.pArcDev->frame_transfer( expbuf,
                                     con.devnum,
                                     con.info.axes[_ROW_],
                                     con.info.axes[_COL_],
                                     con.pCallback
                                   );
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] pArcDev->frame transfer waveforms started on " << con.devname << " for exposure buffer " << expbuf;
        logwrite( function, message.str() );
#endif
      }

    }
    catch (const std::exception &e) {
      // arc::gen3::CArcDevice::expose() will throw an exception for an abort.
      // Look for the word "abort" in the exception message and log "ABORT"
      // instead of "ERROR".
      //
      std::string estring = e.what();
      message.str("");
      if ( estring.find("aborted") != std::string::npos ) {
        message << "ABORT " << which_waveforms << " on " << con.devname << ": " << e.what();
      }
      else {
        message << "ERROR calling " << which_waveforms << " on " << con.devname << ": " << e.what();
      }
      cam.set_abortstate( true );
      logwrite(function, message.str());
      con.error = ERROR;
      return;
    }
    catch(...) {
      message.str(""); message << "ERROR: unknown exception calling " << which_waveforms << " on " << con.devname 
                               << " for exposure buffer " << expbuf << ". forcing abort.";
      logwrite(function, message.str() );
      cam.set_abortstate( true );
      con.error = ERROR;
      return;
    }
    con.error = NO_ERROR;

    return;
  }
  /***** AstroCam::Interface::dothread_read ***********************************/


  /***** AstroCam::Interface::dothread_expose *********************************/
  /**
   * @brief      run in a thread to actually send the command
   * @details    no longer in use, probably should remove
   * @param[in]  con  reference to Controller class object
   *
   */
  void Interface::dothread_expose( Controller &con ) {
    std::string function = "AstroCam::Interface::dothread_expose";
    logwrite( function, "ERROR: NOT IN USE" );
/***
    std::stringstream message;

#ifdef LOGLEVEL_DEBUG
    message.str("");
    message << "[DEBUG] con.devnum=" << con.devnum 
            << " .devname=" << con.devname << " con.section_size=" << con.info.section_size
            << " shutterenable=" << con.info.shutterenable;
    logwrite(function, message.str());
#endif

    // Start the exposure here.
    // A callback function triggered by the ARC API will perform the writing of frames to disk.
    //
    try {
      // get system time just before the actual expose() call
      //
      con.info.start_time = get_timestamp( );  // current system time formatted as YYYY-MM-DDTHH:MM:SS.sss

      // Send the actual command to start the exposure.
      // This API call will send the SEX command to trigger the exposure.
      // The devnum is passed in so that the Callback functions know which device they belong to.
      //
      con.pArcDev->expose( con.devnum, 
                           con.info.exposure_time,
                           con.info.detector_pixels[0], 
                           con.info.detector_pixels[1], 
                           server.camera.abortstate, 
                           con.pCallback, 
                           con.info.shutterenable
                         );
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] pArcDev->expose started on " << con.devname;
      logwrite( function, message.str() );
#endif

      // The system writes a few things in the header
      //
      con.pFits->add_key( true, "FIRMWARE", "STRING",  con.firmware , "controller firmware" );
      con.pFits->add_key( true, "EXPSTART", "STRING", con.info.start_time, "exposure start time" );
      con.pFits->add_key( true, "READOUT", "STRING",  con.info.readout_name, "readout amplifier" );

      con.pFits->add_key( true, "DEVNUM", "INT",  std::to_string(con.devnum), "PCI device number" );
      con.pFits->add_key( true, "EXPTIME", "INT", std::to_string(con.info.exposure_time), "exposure time in msec" );
    }
    catch (const std::exception &e) {
      // arc::gen3::CArcDevice::expose() will throw an exception for an abort.
      // Look for the word "abort" in the exception message and log "ABORT"
      // instead of "ERROR".
      //
      std::string estring = e.what();
      message.str("");
      if ( estring.find("aborted") != std::string::npos ) {
        message << "ABORT on " << con.devname << ": " << e.what();
      }
      else {
        message << "ERROR on " << con.devname << ": " << e.what();
      }
      server.camera.set_abortstate( true );
      logwrite(function, message.str());
      con.error = ERROR;
      return;
    }
    catch(...) {
      message.str(""); message << "ERROR: unknown exception calling pArcDev->expose() on " << con.devname << ". forcing abort.";
      logwrite(function, message.str() );
      server.camera.set_abortstate( true );
      con.error = ERROR;
      return;
    }
    con.error = NO_ERROR;
***/
    return;
  }
  /***** AstroCam::Interface::dothread_expose *********************************/


  /***** AstroCam::Interface::dothread_native *********************************/
  /**
   * @brief      run in a thread to actually send the command
   * @param[in]  con  reference to Controller object
   * @param[in]  cmd  vector containing command and args
   *
   */
//void Interface::dothread_native( Controller &con, std::vector<uint32_t> cmd ) {
  void Interface::dothread_native( int dev, std::vector<uint32_t> cmd ) {
    const std::string function ( "AstroCam::Interface::dothread_native" );
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
      if (cmd.size() == 1) this->controller[dev].retval = this->controller[dev].pArcDev->command( { TIM_ID, cmd.at(0) } );
      else
      if (cmd.size() == 2) this->controller[dev].retval = this->controller[dev].pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1) } );
      else
      if (cmd.size() == 3) this->controller[dev].retval = this->controller[dev].pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2) } );
      else
      if (cmd.size() == 4) this->controller[dev].retval = this->controller[dev].pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2), cmd.at(3) } );
      else
      if (cmd.size() == 5) this->controller[dev].retval = this->controller[dev].pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2), cmd.at(3), cmd.at(4) } );
      else {
        message.str(""); message << "ERROR: invalid number of command arguments: " << cmd.size() << " (expecting 1,2,3,4,5)";
        logwrite(function, message.str());
        this->controller[dev].retval = 0x455252;
      }
    }
    catch(const std::runtime_error &e) {
      message.str(""); message << "ERROR sending (" << this->pci_cmd_num << ") 0x"
                               << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
                               << command << " to " << this->controller[dev].devname << ": " << e.what();
      logwrite(function, message.str());
      this->controller[dev].retval = 0x455252;
      return;
    }
    catch(std::out_of_range &) {  // impossible
      logwrite(function, "ERROR: indexing command argument ("+std::to_string(this->pci_cmd_num)+")");
      this->controller[dev].retval = 0x455252;
      return;
    }
    catch(...) {
      message.str(""); message << "ERROR sending (" << std::dec << this->pci_cmd_num << ") 0x"
                               << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
                               << command << " to " << this->controller[dev].devname << ": unknown";
      logwrite(function, message.str());
      this->controller[dev].retval = 0x455252;
      return;
    }

    std::string retvalstring;
    this->retval_to_string( this->controller[dev].retval, retvalstring );
    message.str(""); message << this->controller[dev].devname << std::dec << " (" << this->pci_cmd_num << ")"
                             << " returns " << retvalstring;
    logwrite( function, message.str() );

    return;
  }
  /***** AstroCam::Interface::dothread_native *********************************/


  /***** AstroCam::Interface::access_useframes ********************************/
  /**
   * @brief      set or get the state of this->useframes
   * @param[in]  useframes  reference to string contains argument and return value
   * @return     ERROR or NO_ERROR
   *
   * The argument is passed in by reference so that it can be used for the
   * return value of this->useframes, while the actual return value
   * indicates an error condition. If the reference contains an empty
   * string then this is a "get" operation. If the reference is not empty
   * then this is a "set" operation.
   *
   * this->useframes indicates whether or not the firmware supports frames,
   * true if it does, false if it does not. 
   *
   */
  long Interface::access_useframes(std::string& useframes) {
    std::string function = "AstroCam::Interface::access_useframes";
    std::stringstream message;
    std::vector<std::string> tokens;

    // If an empty string reference is passed in then that means to return
    // the current value using that reference.
    //
    if ( useframes.empty() ) {
      useframes = ( this->useframes ? "true" : "false" );  // set the return value here on empty
      message << "useframes is " << useframes;
      logwrite(function, message.str());
      return( NO_ERROR );
    }

    Tokenize(useframes, tokens, " ");

    if (tokens.size() != 1) {
      message.str(""); message << "error: expected 1 argument but got " << tokens.size();
      logwrite(function, message.str());
      useframes = ( this->useframes ? "true" : "false" );  // set the return value here on bad number of args
      return(ERROR);
    }

    // Convert to lowercase then compare against "true" and "false"
    // and set boolean class member and return value string.
    //
    std::transform( useframes.begin(), useframes.end(), useframes.begin(), ::tolower );

    if ( useframes.compare("true")  == 0 ) { this->useframes = true;  useframes = "true";  }
    else
    if ( useframes.compare("false") == 0 ) { this->useframes = false; useframes = "false"; }

    // If neither true nor false then log the error,
    // and set the return value to the current value.
    //
    else {
      message << "ERROR: unrecognized argument: " << useframes << ". Expected true or false.";
      logwrite(function, message.str());
      useframes = ( this->useframes ? "true" : "false" );  // set the return value here on invalid arg
      return( ERROR );
    }

    message << "useframes is " << useframes;
    logwrite(function, message.str());
    return( NO_ERROR );
  }
  /***** AstroCam::Interface::access_useframes ********************************/


  /***** AstroCam::Interface::nframes *****************************************/
  /**
   * @brief      set the number of frames per sequence
   * @param[in]  valstring  contains the number of frames
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::access_nframes(std::string valstring) {
    std::string function = "AstroCam::Interface::nframes";
    std::stringstream message;
    std::vector<std::string> tokens;

    logwrite( function, "ERROR: should you be using this?" );
    return( ERROR );

/*****
    Tokenize(valstring, tokens, " ");

    if (tokens.size() != 2) {
      message.str(""); message << "error: expected 1 value but got " << tokens.size()-1;
      logwrite(function, message.str());
      return(ERROR);
    }
    for (const auto &dev : this->devnums) {        // spawn a thread for each device in devnums
      try {
        int rows = this->controller[dev].rows;
        int cols = this->controller[dev].cols;

        this->nfpseq  = parse_val(tokens.at(1));         // requested nframes is nframes/sequence
        this->nframes = this->nfpseq * this->nsequences; // number of frames is (frames/sequence) x (sequences)
        std::stringstream snf;
        snf << "SNF " << this->nframes;                  // SNF sets total number of frames (Y:<N_FRAMES) on timing board

        message.str(""); message << "sending " << snf.str();
        logwrite(function, message.str());

        if ( this->do_native(snf.str()) != 0x444F4E ) return -1;

        std::stringstream fps;
        fps << "FPS " << nfpseq;            // FPS sets number of frames per sequence (Y:<N_SEQUENCES) on timing board

        message.str(""); message << "sending " << fps.str();
        logwrite(function, message.str());

        if ( this->do_native(fps.str()) != 0x444F4E ) return -1;

///< TODO @todo set fitskey NFRAMES
//      fitskeystr.str(""); fitskeystr << "NFRAMES=" << this->nframes << "//number of frames";
//      this->fitskey.set_fitskey(fitskeystr.str()); // TODO

        int _framesize = rows * cols * sizeof(uint16_t);
        if (_framesize < 1) {
          message.str(""); message << "error: bad framesize: " << _framesize;
          logwrite(function, message.str());
          return (-1);
        }
        unsigned int _nfpb = (unsigned int)( this->get_bufsize() / _framesize );

        if ( (_nfpb < 1) ||
             ( (this->nframes > 1) &&
               (this->get_bufsize() < (int)(2*rows*cols*sizeof(uint16_t))) ) ) {
          message.str(""); message << "insufficient buffer size (" 
                                   << this->get_bufsize()
                                   << " bytes) for "
                                   << this->nframes
                                   << " frame"
                                   << (this->nframes>1 ? "s" : "")
                                   << " of "
                                   << rows << " x " << cols << " pixels";
          logwrite(function, message.str());
          message.str(""); message << "minimum buffer size is "
                                   << 2 * this->nframes * rows * cols
                                   << " bytes";
          logwrite(function, message.str());
          return -1;
        }

        std::stringstream fpb;
        fpb << "FPB " << _nfpb;

        message.str(""); message << "sending " << fpb.str();
        logwrite(function, message.str());

        if ( this->do_native(fpb.str()) == 0x444F4E ) return 0; else return -1;
      }
      catch( std::out_of_range & ) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for ( const auto &check : this->devnums ) message << check << " ";
        message << "}";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch(...) { logwrite(function, "unknown error creating fitsname for controller"); return(ERROR); }
    }

    return 0;
*****/
  }
  /***** AstroCam::Interface::nframes *****************************************/


  /***** AstroCam::Interface::dothread_monitor_exposure_pending ***************/
  /**
   * @brief      block on exposure_condition, notifies when ready for exposure
   * @details    This thread is spawned with each new exposure and broadcasts
   *             an async message announcing the state of exposure_pending.
   *             This is where the exposure number gets incremented, and where
   *             do_expose() is repeatedly called when nexp > 1.
   * @param[in]  interface  reference to this->interface
   *
   */
  void Interface::dothread_monitor_exposure_pending( Interface &interface ) {
    const std::string function("AstroCam::Interface::dothread_monitor_exposure_pending");
    std::stringstream message;

    // Log this message once only
    //
    if ( interface.exposure_pending() ) {
      interface.camera.async.enqueue_and_log( function, "NOTICE:exposure pending" );
      interface.camera.async.enqueue( "CAMERAD:READY:false" );
    }

    // Block on exposure_condition until exposure_pending() returns false,
    // which only happens when all devices have reported no exposure pending.
    // This is also when the exposure number gets incremented because the
    // system is ready for a new exposure.
    //
    while ( interface.exposure_pending() ) {
      std::unique_lock<std::mutex> exposure_lock( interface.exposure_lock );
      interface.exposure_condition.wait( exposure_lock );
      exposure_lock.unlock();
    }

    interface.inc_expbuf();  // increment exposure buffer number

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] incremented expbuf buffer to " << interface.get_expbuf(); logwrite( function, message.str() );
#endif

    // Atomically decrement nexp and
    // if greater than zero then start another exposure.
    //
    interface.nexp.fetch_sub(1);
    if ( interface.nexp.load() > 0 ) {
      message.str(""); message << "NOTICE:starting next exposure, " << interface.nexp << " remaining";
      interface.camera.async.enqueue_and_log( function, message.str() );
      interface.do_expose(interface.nexp);
    }
    else {
      interface.camera.async.enqueue_and_log( function, "NOTICE:ready for next exposure" );
      interface.camera.async.enqueue( "CAMERAD:READY:true" );
    }

    return;
  }
  /***** AstroCam::Interface::dothread_monitor_exposure_pending ***************/


  /***** AstroCam::Interface::do_expose ***************************************/
  /**
   * @brief      initiate an exposure
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::do_expose(int nexp_in) {
    const std::string function("AstroCam::Interface::do_expose");
    std::stringstream message;
    std::string devstr;
    std::string _start_time;
    long error;

    if (this->devnums.empty()) {
      logwrite(function, "ERROR no connected controllers");
      return ERROR;
    }

    for (const auto &dev : this->devnums) {
      std::string naughtylist;
      if (!this->controller[dev].is_imsize_set) {
        if (!naughtylist.empty()) naughtylist += ' ';
        naughtylist += this->controller[dev].channel;
      }
      if (!naughtylist.empty()) {
        logwrite(function, "ERROR image_size not set for channel(s): "+naughtylist);
        return ERROR;
      }
    }

    this->nexp.store(nexp_in);

    // Different controllers may have different rules about when to set and clear their
    // exposure_pending flag (depending on whether or not they support frame transfer), but
    // if an exposure is pending for any controller then do not start a new exposure.
    //
    if ( this->exposure_pending() ) {
      std::vector<int> pending = this->exposure_pending_list();
      message.str(""); message << "ERROR: cannot start new exposure while exposure is pending for chan";
      message << ( pending.size() > 1 ? "s " : " " );
      for ( const auto &dev : pending ) message << this->controller[dev].channel << " ";
      this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
      return(ERROR);
    }

    // When using the internal ARC shutter control you cannot overlap exposures
    // with readout because it would require sending a command which cannot
    // be handled during readout.
    //
    if ( this->camera.ext_shutter && !this->is_camera_idle() ) {
      this->camera.async.enqueue_and_log( "CAMERAD", function, "ERROR: overlapping exposure cannot be started when using ARC shutter" );
      return( ERROR );
    }

    // check for valid exposure_time
    //
    if ( this->camera.exposure_time < 0 ) {
      message.str(""); message << "ERROR: exposure time is undefined";
      this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
      return( ERROR );
    }

    logwrite( function, "starting exposure" );

    // This is the exposure buffer number for this exposure.
    // Lock it in now, before another exposure comes in.
    //
    int this_expbuf = this->get_expbuf();
#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] this_expbuf=" << this_expbuf;
    logwrite( function, message.str() );
#endif

    // Collect telemetry, which will be stored in camera_info.telemkeys
    //
    this->collect_telemetry();

    // Make a copy of this->camera_info for this particular exposure buffer number.
    // This expinfo will be used for this particular exposure.
    // Any changes to camera_info hereafter will not be used for this exposure.
    //
    this->fitsinfo[this_expbuf] = std::make_shared<Camera::Information>( this->camera_info );

    // check readout type
    //
    for ( const auto &dev : this->devnums ) {
      if ( this->controller[ dev ].info.readout_name.empty() ) {
        message.str(""); message << "ERROR: readout undefined";
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return( ERROR );
      }
    }

    // Clear the abort flag for a new exposure, in case it was previously set
    //
    this->camera.set_abortstate( false );

    // Initialize framethread count -- this keeps track of threads
    // spawned by frameCallback, to make sure that all have been completed.
    //
    this->init_framethread_count();

    // Set camera.fitstime (YYYYMMDDHHMMSS) used for filename
    // this may be a fraction of a second before the recorded EXPTIME but this
    // rounds to the nearest second and is only used for the filename. The
    // real shutter open time for the exposure start will be recorded by
    // the shutter thread.
    //
    this->camera.set_fitstime( get_timestamp() );

    if ( ( error = this->camera.get_fitsname( this->fitsinfo[this_expbuf]->fits_name ) ) != NO_ERROR ) {
      this->camera.async.enqueue_and_log( "CAMERAD", function, "ERROR: assembling fitsname" );
      return( error );
    }

    this->fitsinfo[this_expbuf]->systemkeys.primary().addkey( "FITSNAME", 
                                                               this->fitsinfo[this_expbuf]->fits_name,
                                                               "this filename" );

    // Spawn a thread to open and write FITS files as the frames come in.
    // Each thread gets the exposure buffer number for the current exposure,
    // and a reference to "this" Interface object.
    //
    for ( const auto &dev : this->devnums ) this->write_pending( this_expbuf, dev, true );
    std::thread( std::ref(AstroCam::Interface::FITS_handler), this_expbuf, std::ref(*this) ).detach();

    {
    // CLEAR if allowed -- go through all devices,
    // if NOT in frame transfer then you can clear the CCD.
    // If it IS in frame transfer then only clear the CCD if the cameras are idle.
    //
    std::string retstr;
    for ( const auto &dev : this->devnums ) {
      if ( this->is_camera_idle( dev ) ) {
        error = this->do_native( dev, "CLR", retstr );  // send the clear command here to this dev
        if ( error != NO_ERROR ) {
          message.str(""); message << "ERROR clearing chan " << this->controller[dev].channel << " CCD: " << retstr;
          logwrite( function, message.str() );
          return( error );
        }
      message.str(""); message << "cleared chan " << this->controller[dev].channel << " CCD";
      logwrite( function, message.str() );
      }
#ifdef LOGLEVEL_DEBUG
      else {
        message.str(""); message << "[DEBUG] chan " << this->controller[dev].channel << " CCD was *not* cleared:"
                                 << " exposure_pending=" << this->exposure_pending()
                                 << " in_readout=" << this->controller[dev].in_readout
                                 << " in_frametransfer=" << this->controller[dev].in_frametransfer;
        logwrite( function, message.str() );
      }
#endif
    }
    }

    // The shutter duration MUST be manually reset to 0 before each exposure.
    // Allowing the shutter to open and close without automatically resetting
    // the exposure timer provides for pause/resume to increment the exposure
    // duration.
    //
    this->camera.shutter.zero_exposure();  // the shutter duration MUST be reset to 0

    // Spawn a thread to operate the shutter if needed.
    //
    if ( this->camera.exposure_time > 0 ) {
      this->camera.shutter.arm();          // puts shutter into pending state to prevent potential race conditions
                                           // while waiting for shutter_state to transition from 1 -> 0
      std::thread( std::ref(AstroCam::Interface::dothread_shutter), this_expbuf, std::ref(*this) ).detach();
    }
    else {
      logwrite( function, "shutter not opened" );
      message.str(""); message << "NOTICE: incremented exposure buffer to " << server.get_expbuf();
      logwrite( function, message.str() );

      // Save shutter-timed keywords to keyword database now, because dothread_shutter won't run
      //
      timespec timenow       = Time::getTimeNow();         // get the time NOW
      std::string timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
      double mjd             = mjd_from( timenow );        // modified Julian date of start
      double airmass=NAN;

      // get the airmass from tcsd telemetry now
      //
      this->collect_telemetry_key( "tcsd", "AIRMASS", airmass );

      this->fitsinfo[this_expbuf]->systemkeys.primary().addkey( "EXPSTART", timestring, "exposure start time" );
      this->fitsinfo[this_expbuf]->systemkeys.primary().addkey( "MJD0", mjd, "exposure start time (modified Julian Date)" );
      this->fitsinfo[this_expbuf]->systemkeys.primary().addkey( "MJD1", mjd, "exposure stop time (modified Julian Date)" );
      this->fitsinfo[this_expbuf]->systemkeys.primary().addkey( "MJD", mjd, "average of MJD0 and MJD1" );
      this->fitsinfo[this_expbuf]->systemkeys.primary().addkey( "SHUTTIME", this->camera.shutter.get_duration(), "actual shutter open time in sec", 3 );
      this->fitsinfo[this_expbuf]->systemkeys.primary().addkey( "AIRMASS0", airmass, "airmass at start of exposure" );
      this->fitsinfo[this_expbuf]->systemkeys.primary().addkey( "AIRMASS1", airmass, "airmass at end of exposure" );
      this->fitsinfo[this_expbuf]->systemkeys.primary().addkey( "AIRMASS", airmass, "average of AIRMASS0 and AIRMASS1" );
    }

    // Shutter or not, an exposure is now pending, so set the exposure_pending flag
    // and spawn a thread to monitor it, which will provide a notification
    // when ready for the next exposure.
    //
    for ( const auto &dev : this->devnums ) this->exposure_pending( dev, true );
    this->state_monitor_condition.notify_all();
    std::thread( std::ref(AstroCam::Interface::dothread_monitor_exposure_pending), std::ref(*this) ).detach();

    // prepare the camera info class object for each controller
    //
    for (const auto &dev : this->devnums) {        // spawn a thread for each device in devnums
      try {

        // Initialize a frame counter for each device. 
        //
        this->controller[dev].init_framecount();

        // Allocate workspace memory for deinterlacing (each dev has its own workbuf)
        //
        if ( ( error = this->controller[dev].alloc_workbuf( ) ) != NO_ERROR ) {
          this->camera.async.enqueue_and_log( "CAMERAD", function, "ERROR: allocating memory for deinterlacing" );
          return( error );
        }

/*** 10/30/23 *** BOB
        // then set the filename for this specific dev
        // Assemble the FITS filename.
        // If naming type = "time" then this will use this->fitstime so that must be set first.
        // If there are multiple devices in the devnums then force the fitsname to include the dev number
        // in order to make it unique for each device.
        //
        if ( this->devnums.size() > 1 ) {
          devstr = std::to_string( dev );  // passing a non-empty devstr will put that in the fitsname
        }
        else {
          devstr = "";
        }
        if ( ( error = this->camera.get_fitsname( devstr, this->controller[dev].info.fits_name ) ) != NO_ERROR ) {
          this->camera.async.enqueue_and_log( "CAMERAD", function, "ERROR: assembling fitsname" );
          return( error );
        }
***/

#ifdef LOGLEVEL_DEBUG
        message.str("");
        message << "[DEBUG] pointers for dev " << dev << ": "
                << " pArcDev="  << std::hex << this->controller[dev].pArcDev
                << " pCB="      << std::hex << this->controller[dev].pCallback;
//              << " pFits="    << std::hex << this->controller[dev].pFits;
        logwrite(function, message.str());
#endif
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for (const auto &check : this->devnums) message << check << " ";
        message << "}";
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return(ERROR);
      }
      catch(...) {
        message.str(""); message << "ERROR: unknown exception creating fitsname for controller";
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return(ERROR);
      }
    }

    // For each controller n, open a FITS file and then spawn separate parallel
    // threads which will call ARC API read(n).
    // These threads may block (if needed) until the shutter has closed, but
    // it's OK if the shutter thread is so quick that it ends first.
    //
    // This is where camera_info keywords are "locked-in", when they are copied
    // to the expinfo object. The FITS writer is going to get
    // this->controller[devnum].expinfo[expbuf] so that's where all the header
    // keys need to be.
    //
    // After copying camera_info, erase the telemkeys and conditionally erase
    // the userkeys from it.
    //
    this->camera_info.systemkeys.add_key("WCSNAME",  "DISPLAY",  "", EXT, "all");
    this->camera_info.systemkeys.add_key("WCSNAMEA", "SPECTRUM", "", EXT, "all");
    this->camera_info.systemkeys.add_key("CUNIT1A",  "Angstrom", "", EXT, "all");
    this->camera_info.systemkeys.add_key("CUNIT2A",  "arcsec",   "", EXT, "all");
    this->camera_info.systemkeys.add_key("CDELT2A",  0.25*this->camera_info.binspat, "Spatial scale in arcsec/pixel", EXT, "all");
    this->camera_info.systemkeys.add_key("CRVAL2A",  0.0, "Reference value in arcsec", EXT, "all");

    for (const auto &dev : this->devnums) {        // spawn a thread for each device in devnums

      this->make_image_keywords(dev);

      try {

        // copy the info class from controller[dev] to controller[dev].expinfo[expbuf]
        //
        this->controller[dev].expinfo[this_expbuf] = this->controller[dev].info;

        // copy the info class from controller[dev] to controller[dev].expinfo[expbuf]
        // create handy references to the Common::Header objects for expinfo
        //
        auto &_systemkeys = this->controller[dev].expinfo[this_expbuf].systemkeys;
        auto &_telemkeys  = this->controller[dev].expinfo[this_expbuf].telemkeys;
        auto &_userkeys   = this->controller[dev].expinfo[this_expbuf].userkeys;

        // store BOI in this local _systemkeys so that it's overwritten each exposure
        //
        int nboi=0;
        int lastrowread=0;
        int stop=0;
        for ( const auto &[nskip,nread] : this->controller[dev].info.interest_bands ) {
          nboi++;
          std::string boikey = "BOI"+std::to_string(nboi);
          lastrowread += nskip;
          stop = lastrowread+nread;
          std::string boival = std::to_string(lastrowread)+":"+std::to_string(stop);
          _systemkeys.add_key( boikey, boival, "band of interest "+std::to_string(nboi), EXT, this->controller[dev].channel  );
          lastrowread = stop;
        }

        // merge the primary keyword databases from fitsinfo into expinfo
        //
        _systemkeys.primary().merge( this->fitsinfo[this_expbuf]->systemkeys.primary().keydb );
        _telemkeys.primary().merge( this->fitsinfo[this_expbuf]->telemkeys.primary().keydb );
        _userkeys.primary().merge( this->fitsinfo[this_expbuf]->userkeys.primary().keydb );

        // merge in the primary from camera_info
        //
        _systemkeys.primary().merge( this->camera_info.systemkeys.primary().keydb );
        _telemkeys.primary().merge( this->camera_info.telemkeys.primary().keydb );
        _userkeys.primary().merge( this->camera_info.userkeys.primary().keydb );

        // merge in the primary from camera_info
        // handy vector of "all" and the channel for this devnum which is used
        // to reference keywords to be written to all extensions and only the
        // extension for this channel
        //
        auto channel = this->controller[dev].channel;
        std::vector<std::string> channels = { "all", channel };

        // Loop through both "channels" and merge the Header objects from camera_info
        // into expinfo.
        //
        for ( const auto &chan : channels ) {
          if ( this->camera_info.systemkeys.has_chan(chan) ) _systemkeys.merge_extension(chan, this->camera_info.systemkeys );
          if ( this->camera_info.telemkeys.has_chan(chan) )  _telemkeys.merge_extension(chan, this->camera_info.telemkeys );
          if ( this->camera_info.userkeys.has_chan(chan) )   _userkeys.merge_extension(chan, this->camera_info.userkeys );
        }

        // All changes to the primary extension have to be reflected in the fitsinfo object
        // because this is what the fitswriter has.
        //
        this->fitsinfo[this_expbuf]->systemkeys.primary() = _systemkeys.primary();
        this->fitsinfo[this_expbuf]->telemkeys.primary() = _telemkeys.primary();
        this->fitsinfo[this_expbuf]->userkeys.primary() = _userkeys.primary();

        this->controller[dev].expinfo[this_expbuf].fits_name="not_needed";

        std::string hash;
        md5_file( this->controller[dev].firmware, hash );                 // compute the md5 hash

        // erase the per-exposure keyword databases.
        //
        // telemkeys are refreshed for each exposure
        this->camera_info.telemkeys.primary().erase_db();
        this->camera_info.telemkeys.erase_extensions(channel);

        // userkeys are conditionally persistent
        if ( !this->camera.is_userkeys_persist ) {
          this->camera_info.userkeys.primary().erase_db();
          this->camera_info.userkeys.erase_extensions(channel);
        }

        // **********************************************************
        // ********** SPAWN THREADS TO TRIGGER CCD READOUT **********
        // **********************************************************

        // ... then spawn a thread to trigger the appropriate readout waveforms in the ARC controller
        //
        std::thread( std::ref(AstroCam::Interface::dothread_read),
                     std::ref(this->camera),
                     std::ref(this->controller[dev]),
                     this_expbuf
                   ).detach();
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for (const auto &check : this->devnums) message << check << " ";
        message << "}";
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return(ERROR);
      }
      catch(const std::exception &e) {
        message.str(""); message << "ERROR creating read thread: " << e.what();
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return(ERROR);
      }
      catch(...) {
        message.str(""); message << "ERROR: unknown exception creating read thread for controller";
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return(ERROR);
      }
    }

    logwrite( function, "[DEBUG] expose is done now!" );

    for (const auto &dev : this->devnums) {
      message.str(""); 
      message << std::dec
              << "** dev=" << dev << " type_set=" << this->controller[dev].info.type_set << " frame_type=" << this->controller[dev].info.frame_type
              << " detector_pixels[]=" << this->controller[dev].info.detector_pixels[0] << " " << this->controller[dev].info.detector_pixels[1]
              << " section_size=" << this->controller[dev].info.section_size << " image_memory=" << this->controller[dev].info.image_memory
              << " readout_name=" << this->controller[dev].info.readout_name 
              << " readout_name2=" << this->controller[dev].expinfo[this_expbuf].readout_name
              << " readout_type=" << this->controller[dev].info.readout_type
              << " axes[]=" << this->controller[dev].info.axes[0] << " " << this->controller[dev].info.axes[1] << " " << this->controller[dev].info.axes[2]
              << " cubedepth=" << this->controller[dev].info.cubedepth << " fitscubed=" << this->controller[dev].info.fitscubed
              << " phys binning=" << this->controller[dev].info.binning[0] << " " << this->controller[dev].info.binning[1]
              << " axis_pixels[]=" << this->controller[dev].info.axis_pixels[0] << " " << this->controller[dev].info.axis_pixels[1]
              << " ismex=" << this->controller[dev].info.ismex << " extension=" << this->controller[dev].info.extension;
      logwrite( function, message.str() );
    }

    for (const auto &dev : this->devnums) {
      for ( int ii=0; ii<NUM_EXPBUF; ii++ ) {
        message.str(""); 
        message << std::dec
                << "** dev=" << dev << " expbuf=" << ii << " type_set=" << this->controller[dev].expinfo.at(ii).type_set << " frame_type=" << this->controller[dev].expinfo.at(ii).frame_type
                << " detector_pixels[]=" << this->controller[dev].expinfo.at(ii).detector_pixels[0] << " " << this->controller[dev].expinfo.at(ii).detector_pixels[1]
                << " section_size=" << this->controller[dev].expinfo.at(ii).section_size << " image_memory=" << this->controller[dev].expinfo.at(ii).image_memory
                << " readout_name=" << this->controller[dev].expinfo.at(ii).readout_name << " readout_type=" << this->controller[dev].expinfo.at(ii).readout_type
                << " axes[]=" << this->controller[dev].expinfo.at(ii).axes[0] << " " << this->controller[dev].expinfo.at(ii).axes[1] << " " << this->controller[dev].expinfo.at(ii).axes[2]
                << " cubedepth=" << this->controller[dev].expinfo.at(ii).cubedepth << " fitscubed=" << this->controller[dev].expinfo.at(ii).fitscubed
                << " phys binning=" << this->controller[dev].expinfo.at(ii).binning[0] << " " << this->controller[dev].expinfo.at(ii).binning[1]
                << " axis_pixels[]=" << this->controller[dev].expinfo.at(ii).axis_pixels[0] << " " << this->controller[dev].expinfo.at(ii).axis_pixels[1]
                << " ismex=" << this->controller[dev].expinfo.at(ii).ismex << " extension=" << this->controller[dev].expinfo.at(ii).extension;
        logwrite( function, message.str() );
      }
    }

    return error;
  }
  /***** AstroCam::Interface::do_expose ***************************************/


  /***** AstroCam::Interface::make_telemetry_message **************************/
  /**
   * @brief      assembles my telemetry message
   * @details    This creates a JSON message for my telemetry info, then serializes
   *             it into a std::string ready to be sent over a socket.
   * @param[out] retstring  string containing the serialization of the JSON message
   *
   */
  void Interface::make_telemetry_message( std::string &retstring ) {
    // assemble the telemetry I want to report into a json message
    // Set a messagetype keyword to indicate what kind of message this is.
    //
    nlohmann::json jmessage;
    jmessage["messagetype"] = "camerainfo";

    jmessage["SHUTTIME_SEC"] = this->camera.shutter.get_duration();  // shutter open time in sec

    retstring = jmessage.dump();  // serialize the json message into a string

    retstring.append(JEOF);       // append JSON message terminator

    return;
  }
  /***** AstroCam::Interface::make_telemetry_message **************************/


  /***** AstroCam::Interface::collect_telemetry *******************************/
  /**
   * @brief      send the TELEMREQUEST command to each configured daemon to get telemetry
   * @details    This overloaded version accepts a name, for the case where
   *             telemetry is needed from one provider only (e.g. TCS)
   * @param[in]  name       name of provider from TELEM_PROVIDER config key
   * @param[out] retstring  serialized string of json telemetry message
   *
   */
  void Interface::collect_telemetry(const std::string name, std::string &retstring) {
    Common::DaemonClient jclient("", "\n", JEOF );
    auto it = this->telemetry_providers.find(name);
    if ( it != this->telemetry_providers.end() ) {
      jclient.set_name(it->first);
      jclient.set_port(it->second);
      jclient.connect();
      jclient.command(TELEMREQUEST, retstring);
      jclient.disconnect();
    }
    return;
  }
  /***** AstroCam::Interface::collect_telemetry *******************************/
  /**
   * @brief      send the TELEMREQUEST command to each configured daemon to get telemetry
   *
   */
  void Interface::collect_telemetry() {
    std::string retstring;

    // Instantiate a client to communicate with each daemon,
    // constructed with no name, newline termination on command writes,
    // and JEOF termination on reply reads.
    //
    Common::DaemonClient jclient("", "\n", JEOF );

    // Loop through each configured telemetry provider, which is a map of
    // ports indexed by daemon name, both of which are used to update
    // the jclient object.
    //
    // Send the command TELEMREQUEST to each daemon and read back the reply into
    // retstring, which will be the serialized JSON telemetry message.
    //
    // handle_json_message() will parse the reply and set the FITS header
    // keys in the telemkeys database.
    //
    for ( const auto &[name, port] : this->telemetry_providers ) {
      jclient.set_name(name);
      jclient.set_port(port);
      jclient.connect();
      jclient.command(TELEMREQUEST, retstring);
      jclient.disconnect();
      handle_json_message(retstring);
    }

    return;
  }
  /***** AstroCam::Interface::collect_telemetry *******************************/


  /***** AstroCam::Interface::handle_json_message *****************************/
  /**
   * @brief      parses incoming telemetry messages
   * @param[in]  message_in  serialized JSON message string
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::handle_json_message( std::string message_in ) {
    const std::string function="AstroCam::Interface::handle_json_message";
    std::stringstream message;
    std::string messagetype;
    long error;

    // nothing to do if the message is empty
    //
    if ( message_in.empty() ) {
      logwrite( function, "empty JSON message" );
      return ERROR;
    }

    /**
     * @struct  PrimaryInfo
     * @brief   holds info for extracting primary header keys from json message
     * @details The value in jmessage with key jkey will be added to the primary
     *          FITS header, using comment and optional keyword. If keyword is
     *          not specified then the header keyword uses jkey.
     */
    struct PrimaryInfo {
      std::string jkey;     // key to extract from jmessage
      std::string keyword;  // optional FITS keyword (uses jkey if not specified)
      std::string comment;  // FITS key comment
      std::string type="";  // optional keyword datatype
    };

    /**
     * @struct  ExtensionInfo
     * @brief   holds info for extracting extension header keys from json message
     * @details The value in jmessage with key jkey will be added to the FITS
     *          header specified by channel chan, using comment and optional keyword.
     *          If keyword is not specified then the header keyword uses jkey.
     */
    struct ExtensionInfo {
      std::string chan;     // chan name identifies which extension
      std::string jkey;     // key to extract from jmessage
      std::string keyword;  // optional FITS keyword (uses jkey if not specified)
      std::string comment;  // FITS key comment
      std::string type="";  // optional keyword datatype
    };

    auto &telemkeys = this->camera_info.telemkeys;

    // use to select whether to write to extension or primary
    //
    bool ext = true;
    bool pri = !ext;

    size_t eof_pos = message_in.find(JEOF);
    if ( eof_pos != std::string::npos ) message_in.erase(eof_pos);

    try {
      nlohmann::json jmessage = nlohmann::json::parse( message_in );

      // jmessage must not contain key "error" and must contain key "messagetype"
      //
      if ( !jmessage.contains("error") ) {
        if ( jmessage.contains("messagetype") ) {
          messagetype = jmessage["messagetype"];
          error = NO_ERROR;
        }
        else {
          logwrite( function, "ERROR received JSON message with no messagetype" );
          error = ERROR;
        }
      }
      else {
        logwrite( function, "ERROR in JSON message" );
        error = ERROR;
      }

      // If jmessage contained error or no messagetype then get out now.
      //
      if ( error != NO_ERROR ) return error;

      // telemetry from calibd goes in the primary header
      //
      if ( messagetype == "calibinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"MODFEAR",  "", "FeAr lamp modulator pow dut per"},
          {"MODTHAR",  "", "ThAr lamp modulator pow dut per"},
          {"MODBLCON", "", "Blue continuum modulator pow dut per"},
          {"MODBLBYP", "", "Blue bypass modulator pow dut per"},
          {"MODRDCON", "", "Red continuum modulator pow dut per"},
          {"MODRDBYP", "", "Red bypass modulator pow dut per"},
          {"CALCOVER", "", "calib cover state"},
          {"CALDOOR",  "", "calib door state"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, keyinfo.type, pri);
        }
      }
      else

      // telemetry from flexured goes in the extension header corresponding to the channel
      //
      if ( messagetype == "flexureinfo" ) {
        const ExtensionInfo keyarray[] = {
          {"I", "FLXSPE_I", "FLXSPE", "I flexure spectral axis 2 (X) in um"},
          {"I", "FLXSPA_I", "FLXSPA", "I flexure spatial axis 3 (Y) in um"},
          {"I", "FLXPIS_I", "FLXPIS", "I flexure piston axis 1 (Z) in um"},
          {"R", "FLXSPE_R", "FLXSPE", "R flexure spectral axis 2 (X) in um"},
          {"R", "FLXSPA_R", "FLXSPA", "R flexure spatial axis 3 (Y) in um"},
          {"R", "FLXPIS_R", "FLXPIS", "R flexure piston axis 1 (Z) in um"},
          {"G", "FLXSPE_G", "FLXSPE", "G flexure spectral axis 2 (X) in um"},
          {"G", "FLXSPA_G", "FLXSPA", "G flexure spatial axis 3 (Y) in um"},
          {"G", "FLXPIS_G", "FLXPIS", "G flexure piston axis 1 (Z) in um"},
          {"U", "FLXSPE_U", "FLXSPE", "U flexure spectral axis 2 (X) in um"},
          {"U", "FLXSPA_U", "FLXSPA", "U flexure spatial axis 3 (Y) in um"},
          {"U", "FLXPIS_U", "FLXPIS", "U flexure piston axis 1 (Z) in um"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.keyword, keyinfo.comment, keyinfo.type, ext, keyinfo.chan);
        }
      }
      else

      // telemetry from focusd goes in the extension header corresponding to the channel
      //
      if ( messagetype == "focusinfo" ) {
        const ExtensionInfo keyarray[] = {
          {"I", "FOCUSI", "FOCUS", "science camera I focus position in mm" },
          {"R", "FOCUSR", "FOCUS", "science camera R focus position in mm" },
          {"G", "FOCUSG", "FOCUS", "science camera G focus position in mm" },
          {"U", "FOCUSU", "FOCUS", "science camera U focus position in mm" }
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.keyword, keyinfo.comment, keyinfo.type, ext, keyinfo.chan);
        }
      }
      else

      // telemetry from powerd goes in the primary header
      //
      if ( messagetype == "powerinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"LAMPTHAR", "", "is ThAr lamp on"},
          {"LAMPFEAR", "", "is FeAr lamp on"},
          {"LAMPBLUC", "", "is blue Xe continuum lamp on"},
          {"LAMPREDC", "", "is red continuum lamp on"},
          {"LAMPXE",   "", "is Xe lamp on"},
          {"LAMPINCA", "", "is Incandescent lamp on"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, keyinfo.type, pri);
        }
      }
      else

      // telemetry from calibd goes in the primary header
      //
      if ( messagetype == "slitinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"SLITW",    "", "slit width in arcsec"},
          {"SLITO",    "", "slit offset in arcsec"},
          {"SLITPOSA", "", "slit actuator A position in mm"},
          {"SLITPOSA", "", "slit actuator A position in mm"},
          {"SLITPOSB", "", "slit actuator B position in mm"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, keyinfo.type, pri);
        }
      }
      else

      // targetinfo telemetry comes from sequencerd and goes in the primary header
      //
      if ( messagetype == "targetinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"OBS_ID",   "", "Observation ID", "INT"},
          {"NAME",     "", "target name", "STRING"},
//        {"BINSPECT", "", "binning in spectral direction"},
//        {"BINSPAT",  "", "binning in spatial direction"},
          {"SLITA",    "", "slit angle in deg", "FLOAT"},
          {"POINTMDE", "", "pointing mode", "STRING"},
          {"RA",       "", "requested Right Ascension in J2000", "STRING"},
          {"DECL",     "", "requested Declination in J2000", "STRING"}
        };
        for ( const auto &keyinfo : keyarray ) {
          message.str(""); message << "[DEBUG] targetinfo key " << keyinfo.jkey << "=" << jmessage[keyinfo.jkey];
          logwrite(function,message.str());
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, keyinfo.type, pri);
        }
      }
      else

      // telemetry from tcsd goes into primary header
      // AIRMASS is intentionally left out since it is handled differently
      //
      if ( messagetype == "tcsinfo" ) {
        const PrimaryInfo keyarray[] = {
          {"CASANGLE",   "", "TCS reported Cassegrain angle in deg", "FLOAT"},
          {"HA",         "", "hour angle"},
          {"RAOFFSET",   "", "offset Right Ascension"},
          {"DECLOFFS",   "", "offset Declination"},
          {"TELRA",      "", "TCS reported Right Ascension"},
          {"TELDEC",     "", "TCS reported Declination"},
          {"AZ",         "", "TCS reported azimuth"},
          {"ZENANGLE",   "", "TCS reported Zenith angle", "FLOAT"},
          {"DOMEAZ",     "", "TCS reported dome azimuth", "FLOAT"},
          {"DOMESHUT",   "", "dome shutters"},
          {"TELFOCUS",   "", "TCS reported telescope focus position in mm", "FLOAT"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.jkey, keyinfo.comment, keyinfo.type, pri);
        }
      }
      else

      // telemetry from thermald
      //
      if ( messagetype == "thermalinfo" ) {
        const ExtensionInfo keyarray[] = {
          {"I", "TCCD_I", "CCDTEMP", "I CCD temperature in Kelvin", "FLOAT"},
          {"R", "TCCD_R", "CCDTEMP", "R CCD temperature in Kelvin", "FLOAT"},
          {"G", "TCCD_G", "CCDTEMP", "G CCD temperature in Kelvin", "FLOAT"},
          {"U", "TCCD_U", "CCDTEMP", "U CCD temperature in Kelvin", "FLOAT"},

          {"I", "TCOLL_I", "COLTEMP", "I collimator temp in deg C", "FLOAT"},
          {"R", "TCOLL_R", "COLTEMP", "R collimator temp in deg C", "FLOAT"},
          {"G", "TCOLL_G", "COLTEMP", "G collimator temp in deg C", "FLOAT"},

          {"I", "TFOCUS_I", "FOCTEMP", "I focus temp in deg C", "FLOAT"},
          {"R", "TFOCUS_R", "FOCTEMP", "R focus temp in deg C", "FLOAT"},
          {"G", "TFOCUS_G", "FOCTEMP", "G focus temp in deg C", "FLOAT"},
          {"U", "TFOCUS_U", "FOCTEMP", "U focus temp in deg C", "FLOAT"}
        };
        for ( const auto &keyinfo : keyarray ) {
          telemkeys.add_json_key(jmessage, keyinfo.jkey, keyinfo.keyword, keyinfo.comment, keyinfo.type, ext, keyinfo.chan);
        }
      }
      else

      // test message
      //
      if ( messagetype == "test" ) {
        message.str(""); message << "received JSON test message: \"" << jmessage["test"].get<std::string>() << "\"";
        logwrite( function, message.str() );
      }
      else {
        message.str(""); message << "ERROR received unhandled JSON message type \"" << messagetype << "\"";
        logwrite( function, message.str() );
        error = ERROR;
      }
    }
    catch ( const nlohmann::json::parse_error &e ) {
      message.str(""); message << "ERROR json exception parsing message: " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR parsing message: " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }

    return error;
  }
  /***** AstroCam::Interface::handle_json_message *****************************/


  /***** AstroCam::Interface::do_load_firmware ********************************/
  /**
   * @brief      load firmware (.lod) into one or all controller timing boards
   * @param[out] retstring  reference to string to contain return value
   * @return     ERROR or NO_ERROR
   *
   * If a default readout amplifier has been defined then it will be set
   * after loading.
   *
   * This function is overloaded
   *
   * This version is when no parameter is specified, in which case try to
   * load the default lodfiles specified in the config file. To do that,
   * a string is built and the other do_load_firmware(std::string) is called.
   *
   */
  long Interface::do_load_firmware( std::string &retstring ) {
    std::string function = "AstroCam::Interface::do_load_firmware";

    // If no connected devices then nothing to do here
    //
    if ( this->numdev == 0 ) {
      logwrite(function, "ERROR: no connected devices");
      retstring="not_connected";
      return(ERROR);
    }

    long error=NO_ERROR;

    // Loop through all of the configured controllers, build up a string in
    // the form "<dev> <filename>" to pass to the do_load_firmware() function
    // to load each controller with the specified file.
    //
    for ( const auto &con : this->controller ) {
      if ( con.second.inactive ) continue;  // skip controllers flagged as inactive
      // But only use it if the device is open
      //
      if ( con.second.connected ) {
        std::stringstream lodfilestream;
        lodfilestream << con.second.devnum << " " << con.second.firmware;

        // Call do_load_firmware with the built up string.
        // If it returns an error then set error flag to return to the caller.
        //
        if ( this->do_load_firmware(lodfilestream.str(), retstring) == ERROR ) error = ERROR;
      }
    }
    return(error);
  }
  /***** AstroCam::Interface::do_load_firmware ********************************/


  /***** AstroCam::Interface::do_load_firmware ********************************/
  /**
   * @brief      load firmware (.lod) into one or all controller timing boards
   * @param[in]  timlodfile  string may contain a controller number
   * @param[out] retstring   reference to string to contain return value
   * @return     ERROR or NO_ERROR
   *
   * If a default readout amplifier has been defined then it will be set
   * after loading.
   *
   * This function is overloaded
   *
   * The string passed in may contain a space-separated integer to indicate
   * a specific controller to load. In other words:
   *
   * "filename"     -> load filename into all connected devices
   * "0 filename"   -> load filename into device 0
   * "1 3 filename" -> load filename into devices 1 and 3
   *
   */
  long Interface::do_load_firmware(std::string timlodfile, std::string &retstring) {
    std::string function = "AstroCam::Interface::do_load_firmware";
    std::stringstream message;
    std::vector<std::string> tokens;
    std::vector<uint32_t> selectdev;
    struct stat st;
    long error = ERROR;

    // If no connected devices then nothing to do here
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR: no connected devices");
      retstring="not_connected";
      return(ERROR);
    }

    // check the provided timlodfile string
    //
    if (timlodfile.empty()) {
      logwrite(function, "ERROR: no filename provided");
      return( ERROR );
    }

    // Tokenize the input string to separate out the filename from
    // any specified controller numbers.
    //
    Tokenize(timlodfile, tokens, " ");

    // Need at least one token
    //
    if (tokens.size() < 1) {
      logwrite(function, "ERROR: too few arguments");
      retstring="invalid_argument";
      return( ERROR );
    }

    // Can't have more tokens than the number of controllers + 1
    // The +1 is for the file name.
    //
    if ((int)tokens.size() > this->numdev+1) {
      logwrite(function, "ERROR: too many arguments");
      retstring="invalid_argument";
      return( ERROR );
    }

    // If there's only one token then it's the lodfile and load
    // into all controllers in the devnums.
    //
    if (tokens.size() == 1) {
      for (const auto &dev : this->devnums) {
        selectdev.push_back( dev );                        // build selectdev vector from all connected controllers
      }
    }

    // If there's more than one token then there are 1 or more dev numbers
    // in the string. Grab those and put them into a vector.
    // The last token must be the filename.
    //
    if (tokens.size() > 1) {
      for (uint32_t n = 0; n < (tokens.size()-1); n++) {   // tokens.size() - 1 because the last token must be the filename
        selectdev.push_back( (uint32_t)parse_val( tokens.at(n) ) );
      }
      timlodfile = tokens.at( tokens.size() - 1 );         // the last token must be the filename
    }

    // Finally, make sure the file exists before trying to load it
    //
    if (stat(timlodfile.c_str(), &st) != 0) {
      message.str(""); message << "error: " << timlodfile << " does not exist";
      logwrite(function, message.str());
    }
    else {
      // Run through the selectdev vector, spawning threads to do the load for each.
      //
      std::vector<std::thread> threads;               // create a local scope vector for the threads
      int firstdev = -1;                              // first device in the list of connected controllers
      for (const auto &dev : selectdev) {             // spawn a thread for each device in the selectdev list
        if ( firstdev == -1 ) firstdev = dev;         // save the first device from the list of connected controllers
        try {
          if ( this->controller[dev].connected ) {    // but only if connected
            std::thread thr( std::ref(AstroCam::Interface::dothread_load), std::ref(this->controller[dev]), timlodfile );
            threads.push_back ( std::move(thr) );     // push the thread into the local vector
          }
        }
        catch(std::out_of_range &) {
          message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
          for (const auto &check : selectdev) message << check << " ";
          message << "}";
          logwrite(function, message.str());
          retstring="out_of_range";
          return(ERROR);
        }
        catch(const std::exception &e) {
          message.str(""); message << "ERROR threading command: " << e.what();
          logwrite(function, message.str());
          retstring="exception";
          return(ERROR);
        }
        catch(...) { logwrite(function, "unknown error threading command to controller"); retstring="exception"; return(ERROR); }
      }

      try {
        for (std::thread & thr : threads) {           // loop through the vector of threads
          if ( thr.joinable() ) thr.join();           // if thread object is joinable then join to this function (not to each other)
        }
      }
      catch(const std::exception &e) {
        message.str(""); message << "ERROR joining threads: " << e.what();
        logwrite(function, message.str());
        retstring="exception";
        return(ERROR);
      }
      catch(...) { logwrite(function, "unknown error joining threads"); retstring="exception"; return(ERROR); }

      threads.clear();                                // deconstruct the threads vector

      // Check to see if all retvals are the same by comparing them all to the first.
      //
      std::uint32_t check_retval;
      check_retval = this->controller[firstdev].retval;    // save the first one in the controller vector

      bool allsame = true;
      for ( const auto &dev : selectdev ) { if ( this->controller[dev].retval != check_retval ) { allsame = false; } }

      // If all the return values are equal then report only NO_ERROR (if "DON") or ERROR (anything else)
      //
      if ( allsame ) {
        if (check_retval == DON) { error = NO_ERROR; }
        else                     { error = ERROR;    }
      }

      // ...but if any retval is different from another then we have to report each one.
      // This will automatically flag an error. The format will be
      // devnum:retval devnum:retval etc., concatenated together.
      //
      else {
        std::stringstream rss;
        std::string rs;
        for (const auto &dev : selectdev) {
          this->retval_to_string( this->controller[dev].retval, rs );      // convert the retval to string (DON, ERR, etc.)
          rss << this->controller[dev].devnum << ":" << rs << " ";
        }
        retstring = rss.str();
        error = ERROR;
      }
    }

/***
logwrite( function, "NOTICE: firmware loaded" );
for ( const auto &dev : selectdev ) {
  for ( auto it = this->controller[dev].extkeys.keydb.begin();
             it != this->controller[dev].extkeys.keydb.end(); it++ ) {
  message.str(""); message << "NOTICE: dev=" << dev << "key=" << it->second.keyword << " val=" << it->second.keyvalue;
  logwrite( function, message.str() );
  }
}
***/

    for (const auto &dev: selectdev) {
      std::string init=this->controller[dev].channel+" init";
      std::string retstring;
      this->image_size(init, retstring);
    }

    return( error );
  }
  /***** AstroCam::Interface::do_load_firmware ********************************/


  /***** AstroCam::Interface::dothread_load ***********************************/
  /**
   * @brief      run in a thread to actually perform the load
   * @param[in]  con         reference to Controller object
   * @param[in]  timlodfile  string of lodfile name to load
   *
   * If a default readout amplifier has been defined then it will be set
   * after loading.
   *
   * loadControllerFile() doesn't return a value but throws std::runtime_error on error.
   *
   * The retval for each device in the contoller info structure will be set here,
   * to ERR on exception or to DON if no exception is thrown.
   *
   */
  void Interface::dothread_load(Controller &con, std::string timlodfile) {
    std::string function = "AstroCam::Interface::dothread_load";
    std::stringstream message;

    try {
      con.pArcDev->loadControllerFile( timlodfile );        // Do the load here
      con.is_imsize_set = false;                            // need to call image_size now

      if ( !con.info.readout_name.empty() ) {  // Set readout amplifier if defined
        con.pArcDev->selectOutputSource( con.readout_arg );
      }

      std::string hash;
      md5_file( timlodfile, hash );                         // compute the md5 hash

      // add keys to the extension for this channel
      //
      con.info.systemkeys.add_key( "FIRMWARE", timlodfile, "", EXT, con.channel );  // no room for a comment
      con.info.systemkeys.add_key( "FIRM_MD5", hash, "MD5 checksum of firmware", EXT, con.channel );
    }
    catch(const std::exception &e) {
      message.str(""); message << "ERROR: " << con.devname << ": " << e.what();
      logwrite(function, message.str());
      con.retval = ERR;
      con.firmwareloaded = false;
      return;
    }
    catch(...) {
      message.str(""); message << "unknown error loading firmware for " << con.devname;
      logwrite(function, message.str());
      con.retval = ERR;
      con.firmwareloaded = false;
      return;
    }

    message.str(""); message << con.devname << ": loaded firmware " << timlodfile;
                     message << ( con.info.readout_name.empty() ? "" : ", readout " );
                     message << ( con.info.readout_name.empty() ? "" : con.info.readout_name );
    logwrite(function, message.str());

    con.retval = DON;
    con.firmwareloaded = true;
    con.firmware = timlodfile;

    return;
  }
  /***** AstroCam::Interface::dothread_load ***********************************/


  /***** AstroCam::Interface::buffer ******************************************/
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
  long Interface::buffer( std::string args, std::string &retstring ) {
    std::string function = "AstroCam::Interface::buffer";
    std::stringstream message;
    uint32_t try_bufsize=0;

    // Help
    //
    if ( args == "?" ) {
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
      return( ERROR );
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
      return( ERROR );
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
                 return(ERROR);
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
        return( ERROR );
      }
      catch(...) {
        message.str(""); message << "ERROR unknown exception mapping buffer for dev " << dev << " chan " << chan;
        logwrite(function, message.str());
        retstring="exception";
        return( ERROR );
      }

      con.set_bufsize( try_bufsize );  // save the new buffer size to the controller map for this dev
    }  // end if ! retstring.empty()

    retstring = std::to_string( con.get_bufsize() );

//  message.str(""); message << "[DEBUG] allocated " << retstring << " bytes"; logwrite( function, message.str() );

    return(NO_ERROR);
  }
  /***** AstroCam::Interface::buffer ******************************************/


  /***** AstroCam::Interface::do_readout **************************************/
  /**
   * @brief      set or get type of readout
   * @param[in]  args       string containing <dev>|<chan> [ <amp> ]
   * @param[out] retstring  reference to a string for return values
   * @return     ERROR | NO_ERROR | HELP
   *
   * Frame transfer mode is set here, based on the first two characters
   * of the readout name ("FT*" enables it).
   *
   */
  long Interface::do_readout( std::string args, std::string &retstring ) {
    std::string function = "AstroCam::Interface::do_readout";
    std::stringstream message;
    std::vector<std::string> tokens;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" ) {
      retstring = CAMERAD_READOUT;
      retstring.append( " <chan> | <dev#> [ <amp> ]\n" );
      retstring.append( "  Set or get readout amp for specified device.\n" );
      retstring.append( "  A connection to the device must already be open to change the readout.\n" );
      retstring.append( "  If optional amp is omitted then current amp is returned.\n" );
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
      message.str("");
      retstring.append( "  Specify <amp> from { " );
      for ( const auto &type : this->readout_source ) message << type.first << " ";
      message << "}\n";
      retstring.append( message.str() );
      return HELP;
    }

    int dev;
    std::string chan;

    std::string readout_name;               // requested readout source name
    uint32_t readout_arg;                   // argument associated with requested type
    ReadoutType readout_type;
    bool readout_name_valid = false;
    bool isft = false;

    error = this->extract_dev_chan( args, dev, chan, readout_name );

    if ( error != NO_ERROR ) { retstring=readout_name; return error; }

    // If supplied, ...
    //
    if ( !readout_name.empty() ) {
      // ... then check that the requested readout amplifer has a matches in the list of known
      // readout amps. This list is an STL map. this->readout_source.first is the amplifier name,
      // and .second is the argument for the Arc 3-letter command.
      //
      for ( const auto &source : this->readout_source ) {
        if ( source.first.compare( readout_name ) == 0 ) {  // found a match
          readout_name_valid = true;
          readout_arg  = source.second.readout_arg;         // get the arg associated with this match
          readout_type = source.second.readout_type;        // get the type associated with this match
          isft = readout_name.substr(0,2) == "FT";          // frametransfer mode set by readout_name
          break;
        }
      }
      if ( !readout_name_valid ) {
        message.str(""); message << "ERROR: readout " << readout_name << " not recognized";
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return( ERROR );
      }
      else {  // requested readout type is known, so set it for each of the specified devices
        try {
          this->controller[ dev ].pArcDev->selectOutputSource( readout_arg );  // this sets the readout amplifier and can throw an exception
          std::stringstream ft;
          ft << dev << ( isft ? " yes" : " no" );
          this->frame_transfer_mode( ft.str() );  // set frametransfer mode
        }
        catch ( const std::exception &e ) {  // arc::gen3::CArcDevice::selectOutputSource() may throw an exception
          message.str(""); message << "ERROR: setting output source for dev " << dev << " chan " << chan << ": " << e.what();
          logwrite( function, message.str() );
          retstring="controller_fault";
          return( ERROR );
        }
        // update the rest of the controller only after selectOutputSource() succeeds
        //
        this->controller[ dev ].info.readout_name = readout_name;
        this->controller[ dev ].info.readout_type = readout_type;
        this->controller[ dev ].readout_arg = readout_arg;
        // add keyword to the extension for this channel
//TCB   this->controller[ dev ].info.systemkeys.add_key( "AMP_ID", readout_name, "readout amplifier", EXT, chan  );
      }
    }

    // In any case, set or not, get the current type
    //
    retstring = this->controller[dev].info.readout_name;

    return( NO_ERROR );
  }
  /***** AstroCam::Interface::do_readout **************************************/


  /***** AstroCam::Interface::band_of_interest ********************************/
  /**
   * @brief      set or get band of interest
   * @details    Up to 10 BOIs can be defined using up to 10 pairs of nskip and
   *             nread values, which define the number of rows to skip and the
   *             number of rows to read. Each successive skip picks up where the
   *             last read left off. This makes use of firmware from NGPS / SWIFT
   *             commit 8080c66aeeae5aafccfd861771e5143ec114e81a
   * @param[in]  args       string containing <chan>|<dev#> [full|<nskip1> <nread1>]
   * @param[out] retstring  reference to a string for return values
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::band_of_interest( std::string args, std::string &retstring ) {
    const std::string function = "AstroCam::Interface::band_of_interest";
    std::stringstream message;
    std::stringstream cmd;

    // Help
    //
    if ( args == "?" || args == "help" ) {
      retstring = CAMERAD_BOI;
      retstring.append( "  <chan>|<dev#> [full|<nskip1> <nread1> [<nskip2> <nread2> [...]]]]\n" );
      retstring.append( "  Set or get band(s) of interest parameters.\n" );
      retstring.append( "  Bands are defined in the SPATIAL direction (along the slit) so\n" );
      retstring.append( "  <nskip> and <nread> refer to spatial pixels to skip and read.\n" );
      retstring.append( "  If no args are supplied then the BOI table for dev|chan is returned.\n" );
      retstring.append( "  Argument \"full\" returns to the full imsize defined in the config file.\n" );
      retstring.append( "\n" );
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
      retstring.append( "\n" );
      retstring.append( "  One or more bands of interest are specified by supplying one or more\n" );
      retstring.append( "  pairs of values, <nskip> <nread> which specifies the number of spatial\n" );
      retstring.append( "  pixels to skip, then read. Successive bands will resume skipping from the\n" );
      retstring.append( "  previous read. Up to 10 such pairs may be specified. The resultant image\n" );
      retstring.append( "  size will be the sum of all <nreads> in the spatial dimension.\n" );
      retstring.append( "\n" );
      retstring.append( "  Camera controller connection must first be open.\n" );
      return HELP;
    }

    if ( args.empty() ) {
      logwrite( function, "ERROR missing <chan> or <dev>" );
      retstring="invalid_argument";
      return ERROR;
    }

    // Don't allow any changes while any exposure activity is running.
    //
    if ( !this->is_camera_idle() ) {
      logwrite( function, "ERROR: all exposure activity must be stopped before changing image parameters" );
      retstring="camera_busy";
      return( ERROR );
    }

    // Get the requested dev# and channel from supplied args.
    // After calling extract_dev_chan(), dev can be trusted as
    // a valid index without needing a try/catch.
    //
    int dev=-1;
    std::string chan;
    if ( this->extract_dev_chan( args, dev, chan, retstring ) != NO_ERROR ) return ERROR;

    Controller* pcontroller = &this->controller[dev];

    // don't continue if that controller is not connected now
    //
    if ( !pcontroller->connected ) {
      logwrite( function, "ERROR controller channel "+chan+" not connected" );
      retstring="not_connected";
      return ERROR;
    }

    // "full" will erase the interest band table and restore the IMAGE_SIZE that
    // was specified in the config file
    //
    if ( args.find("full") != std::string::npos ) {
      pcontroller->info.interest_bands.clear();
      // This native 3-letter command with three zeros "BOI 0 0 0" will initialize
      // the Y:NBOXES address which disables band-of-interest skips/reads in the firmware.
      // It's the 3rd zero that triggers the initialization.
      //
      if ( this->do_native( dev, "BOI 0 0 0", retstring ) != NO_ERROR ) return ERROR;

      // restore the image size from the config file, which was stored in the class
      // when the config file was read
      //
      int spat_default, spec_default,
          osspat_default, osspec_default,
          binspat_default, binspec_default;

      pcontroller->physical_to_logical(pcontroller->defrows, pcontroller->defcols,
                                       spat_default, spec_default);
      pcontroller->physical_to_logical(pcontroller->defosrows, pcontroller->defoscols,
                                       osspat_default, osspec_default);
      pcontroller->physical_to_logical(pcontroller->info.binning[_ROW_], pcontroller->info.binning[_COL_],
                                       binspat_default, binspec_default);

      cmd.str("");
      cmd << chan << " "
          << spat_default << " "
          << spec_default << " "
          << osspat_default << " "
          << osspec_default << " "
          << binspat_default << " "
          << binspec_default;
      if ( this->image_size( cmd.str(), retstring ) != NO_ERROR ) return ERROR;
    }
    // if the only thing passed-in is the chan, then extract_dev_chan will
    // return an empty retstring (which means read-only). If not then there
    // are args to parse...
    //
    else if ( !retstring.empty() ) {

      // Any args(s) other than "full" are parsed here.
      // I'm expecting pairs of <nskip> <nread> ...
      //
      std::vector<std::string> tokens;
      Tokenize( args, tokens, " " );

      // args must come in pairs so check for an even number of tokens
      // after removing one token for the dev argument
      //
      if ( (tokens.size()-1) % 2 != 0 ) {
        logwrite( function, "ERROR expected pairs of values <nskip> <nread>" );
        retstring="invalid_argument";
        return ERROR;
      }

      // initialize the table before writing
      //
      pcontroller->info.interest_bands.clear();
      if ( this->do_native( dev, "BOI 0 0 0", retstring ) != NO_ERROR ) return ERROR;

      // the total number spatial lines in the image will be the sum of all the nreads
      // which is initialized here and summed in the loop
      //
      int spat_total = 0;

      try {
        // This loops through the tokens (which have already been checked to be
        // in pairs). It writes each pair to the BOI table in the controller and
        // places each pair into the vector of interest_bands.
        //
        for ( size_t i=1; i<tokens.size(); i+=2 ) {
          int nskip = std::stoi( tokens.at(i) );
          int nread = std::stoi( tokens.at(i+1) );

          // must read at least 1 spatial line
          //
          if ( nread<=0 ) {
            logwrite( function, "ERROR nread must be greater than 0" );
            retstring="invalid_argument";
            return ERROR;
          }

          // don't have to skip but it can't be negative
          //
          if ( nskip<0 ) {
            logwrite( function, "ERROR nskip cannot be negative" );
            retstring="invalid_argument";
            return ERROR;
          }

          // Load this interest band into a table on the controller.
          // Supply a non-zero 3rd value because the 3rd value = 0 is used
          // to initialize the number of spatial lines in the firmware.
          //
          cmd.str(""); cmd << "BOI " << nskip << " " << nread << " " << 0xFFFF;
          if ( this->do_native( dev, cmd.str(), retstring ) != NO_ERROR ) return ERROR;
          logwrite( function, "chan "+chan+": "+cmd.str() );

          // add this spatial line to the interest_bands table for this controller
          //
          pcontroller->info.interest_bands.emplace_back( nskip, nread );

          // running summation of spatial lines of each band in the table
          //
          spat_total += nread;
        }

        // Before updating the image size, translate the current dimensions (rows/cols)
        // to logical (spat/spec).
        //
        int spec_current, spat_current;
        pcontroller->physical_to_logical(pcontroller->detrows, pcontroller->detcols,
                                         spat_current, spec_current);

        int osspat_current, osspec_current;
        pcontroller->physical_to_logical(pcontroller->osrows, pcontroller->oscols,
                                         osspat_current, osspec_current);

        int binspat_current, binspec_current;
        pcontroller->physical_to_logical(pcontroller->info.binning[_ROW_], pcontroller->info.binning[_COL_],
                                         binspat_current, binspec_current);

        // Now update the image size
        //
        cmd.str("");
        cmd << chan            << " "  // this channel
            << spat_total      << " "  // new spatial dimension is sum of all bands
            << spec_current    << " "  // don't change original spectral dimension
            << 0               << " "  // force no spatial overscans
            << osspec_current  << " "  // don't change original spectral overscans
            << binspat_current << " "  // don't change spatial binning
            << binspec_current;        // don't change spectral binning
        if ( this->image_size( cmd.str(), retstring ) != NO_ERROR ) return ERROR;
      }
      catch( const std::exception &e ) {
        message.str(""); message << "ERROR parsing skip/read pairs: " << e.what();
        logwrite( function, message.str() );
        retstring="parsing_exception";
        return ERROR;
      }
    } // end if args != full

    // Whether setting boi or not, the retstring contains the list of
    // interest bands.
    //
    retstring.clear();
    int boinum=0;
    for ( const auto &[nskip,nread] : pcontroller->info.interest_bands ) {
      message.str(""); message << ++boinum << ": " << nskip << " " << nread << "\n";
      retstring.append( message.str() );
    }
    if ( boinum==0 ) retstring="full";

    return NO_ERROR;
  }
  /***** AstroCam::Interface::band_of_interest ********************************/


  /***** AstroCam::Interface::set_camera_mode *********************************/
  /**
   * @brief      not yet implemented
   * @param[in]  mode  a string to represent the mode name or number
   * @return     ERROR
   * @todo       camera mode not yet implemented
   *
   */
  long Interface::set_camera_mode(std::string mode) {
    std::string function = "AstroCam::Interface::set_camera_mode";
    std::stringstream message;
    logwrite(function, "ERROR: not implemented");
    return( ERROR );
  }
  /***** AstroCam::Interface::set_camera_mode *********************************/


  /***** AstroCam::Interface::write_frame *************************************/
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
  long Interface::write_frame( int expbuf, int devnum, const std::string chan, int fpbcount ) {
    std::string function = "AstroCam::Interface::write_frame";
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
  /***** AstroCam::Interface::write_frame *************************************/


  /***** AstroCam::Interface::do_exptime **************************************/
  /**
   * @brief      set/get the exposure time
   * @param[in]  exptime_in  requested exposure time in msec
   * @param[out] retstring   reference to string contains the exposure time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::do_exptime(std::string exptime_in, std::string &retstring) {
    std::string function = "AstroCam::Interface::do_exptime";
    std::stringstream message;
    long error = NO_ERROR;

    if (this->controller.size() < 1) {
      logwrite(function, "ERROR: controller not configured");
      retstring="not_configured";
      return(ERROR);
    }

    // If an exposure time was passed in then
    // try to convert it (string) to an integer
    //
    if ( ! exptime_in.empty() ) {

      // Can't make changes to the exptime while reading out because this
      // requires writing to the Leach controller and it won't allow that.
      //
      if ( this->in_readout() ) {
        message.str(""); message << "ERROR: cannot change exposure time while reading out chan ";
        for ( const auto &con : this->controller ) {
          if ( con.second.in_readout || con.second.in_frametransfer ) message << con.second.channel << " ";
        }
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return(ERROR);
      }

      try {
        this->camera.exposure_time = std::stoi( exptime_in );
        this->camera_info.systemkeys.primary().addkey( "EXPTIME", this->camera.exposure_time, "exposure time in msec" );
      }
      catch ( const std::exception &e ) {
        message.str(""); message << "ERROR: parsing exposure time \"" << exptime_in << "\": " << e.what();
        logwrite( function, message.str() );
        retstring="invalid_argument";
        return( ERROR );
      }
    }

    try { retstring = std::to_string( this->camera.exposure_time ); }
    catch ( std::bad_alloc &e ) {
      message.str(""); message << "ERROR: exposure time " << this->camera.exposure_time << " exception: " << e.what();
      logwrite( function, message.str() );
      retstring="bad_alloc";
      error = ERROR;
    }

    message.str(""); message << "exposure time is " << retstring << " msec";
    logwrite(function, message.str());
    return( error );
  }
  /***** AstroCam::Interface::do_exptime **************************************/


  /***** AstroCam::Interface::do_modify_exptime *******************************/
  /**
   * @brief      modify the exposure time while an exposure is running
   * @param[in]  exptime_in  requested exposure time in msec
   * @param[out] retstring   reference to string contains the exposure time
   * @return     ERROR | NO_ERROR | BUSY
   *
   */
  long Interface::do_modify_exptime( std::string exptime_in, std::string &retstring ) {
    const std::string function = "AstroCam::Interface::do_modify_exptime";
    std::stringstream message;
    long requested_exptime=0;

    // A requested exposure time must be specified
    //
    if ( exptime_in.empty() ) {
      logwrite( function, "ERROR: requested exposure time cannot be empty" );
      retstring="missing_exptime";
      return ERROR;
    }

    // Convert the requested exptime from string to long
    //
    try {
      requested_exptime = std::stol( exptime_in );
    }
    catch ( const std::exception &e ) {
      message.str(""); message << "ERROR converting exposure time " << exptime_in << ": " << e.what();
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return ERROR;
    }

    // Block changes within the last 5 seconds of exposure.
    // The remaining time is returned by the PreciseTimer object.
    //
    long remaining_time = this->camera.shutter_timer.get_remaining();

    if ( ( remaining_time < 5000 || requested_exptime < 5000 ) ) {
      message.str(""); message << "ERROR cannot change exposure time with less than 5 s exptime remaining";
      logwrite( function, message.str() );
      retstring="too_late";
      return BUSY;
    }

    // check if requested exptime has already elapsed
    //
    if ( (requested_exptime >= 0) && (requested_exptime < (requested_exptime-remaining_time) ) ) {
      message.str(""); message << "ERROR elapsed time " << (requested_exptime-remaining_time)
                               << " already exceeds requested exposure time " << requested_exptime;
      logwrite( function, message.str() );
      retstring="too_late";
      return BUSY;
    }

    // then send the command, input units are whole number milliseconds.
    //
    this->camera.shutter_timer.modify( requested_exptime );

    this->fitsinfo[this->get_expbuf()]->systemkeys.primary().addkey( "EXPTIME", requested_exptime, "exposure time in sec" );

    message.str(""); message << "modified exptime to " << requested_exptime << " msec";
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** AstroCam::Interface::do_modify_exptime *******************************/


  /***** AstroCam::Interface::stop_exposure ***********************************/
  /**
   * @brief      stop the exposure now (as soon as possible)
   * @param[in]  args        not used
   * @param[out] retstring   return string
   * @return     BUSY | NOTHING | NO_ERROR
   *
   */
  long Interface::stop_exposure( std::string args, std::string &retstring ) {
    const std::string function("AstroCam::Interface::stop_exposure");
    std::stringstream message;

    // nothing to do if not exposing
    //
    if ( ! this->exposure_pending() ) {
      logwrite( function, "not exposing" );
      return NOTHING;
    }

    // Block changes within the last 5 seconds of exposure.
    // The remaining time is returned by the PreciseTimer object
    // in units of milliseconds.
    //
    long remaining_time = this->camera.shutter_timer.get_remaining();

    if ( remaining_time < 5000 ) {
      message << "ERROR cannot stop exposure time with less than 5 s exptime remaining"
              << " (" << remaining_time << ")";
      logwrite( function, message.str() );
      retstring="too_late";
      return BUSY;
    }

    // reset nexp to always make this the last exposure
    //
    this->nexp.store(1);

    // The PreciseTimer object .stop(&ms) function modifies remaining_time
    // to reflect the remaining time of the timer.
    //
    this->camera.shutter_timer.stop( remaining_time );

    double actual_exptime = (this->camera.exposure_time - remaining_time)/1000.;

    logwrite( function, "exposure stopped early" );

    this->fitsinfo[this->get_expbuf()]->systemkeys.primary().addkey( "EXPTIME", actual_exptime, "exposure time in sec" );

    return NO_ERROR;
  }
  /***** AstroCam::Interface::stop_exposure ***********************************/


  /***** AstroCam::Interface::pause_exposure **********************************/
  /**
   * @brief      pause the exposure now (as soon as possible)
   * @param[in]  args        not used
   * @param[out] retstring   return string
   * @return     NO_ERROR | BUSY
   *
   */
  long Interface::pause_exposure( std::string args, std::string &retstring ) {
    const std::string function = "AstroCam::Interface::pause_exposure";
    std::stringstream message;

    // Block changes within the last 5 seconds of exposure.
    // The remaining time is returned by the PreciseTimer object
    // in units of milliseconds.
    //
    long remaining_time = this->camera.shutter_timer.get_remaining();

    if ( remaining_time < 5000 ) {
      message.str(""); message << "ERROR cannot pause exposure time with less than 5 s exptime remaining";
      logwrite( function, message.str() );
      retstring="too_late";
      return BUSY;
    }

    // pause the shutter delay timer
    //
    this->camera.shutter_timer.hold();

    // close the Bonn shutter and
    // log shutter close time if applicable
    //
    if ( this->camera.shutter.is_enabled ) {
      this->camera.shutter.set_close();                    // close shutter
      timespec timenow = Time::getTimeNow();               // get the time NOW
      std::string timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
      this->camera.async.enqueue_and_log( function, "NOTICE:shutter closed at "+timestring );
    }

    logwrite( function, "exposure paused" );

    return NO_ERROR;
  }
  /***** AstroCam::Interface::pause_exposure **********************************/


  /***** AstroCam::Interface::resume_exposure *********************************/
  /**
   * @brief      resume the exposure now (as soon as possible)
   * @param[in]  args        not used
   * @param[out] retstring   return string
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::resume_exposure( std::string args, std::string &retstring ) {
    const std::string function = "AstroCam::Interface::resume_exposure";
    std::stringstream message;

    // open the Bonn shutter
    //
    if ( this->camera.shutter.is_enabled ) this->camera.shutter.set_open();

    // resume the shutter delay timer
    //
    this->camera.shutter_timer.resume();

    // log shutter open time if applicable
    //
    if ( this->camera.shutter.is_enabled ) {
      timespec timenow = Time::getTimeNow();               // get the time NOW
      std::string timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
      this->camera.async.enqueue_and_log( function, "NOTICE:shutter opened at "+timestring );
    }

    logwrite( function, "exposure resumed" );

    return NO_ERROR;
  }
  /***** AstroCam::Interface::resume_exposure *********************************/


  /***** AstroCam::Interface::shutter *****************************************/
  /**
   * @brief      set or get the shutter enable state
   * @details    shutterenable will be passed to pArcDev->expose() which tells
   *             the ARC API whether or not to open the shutter on expose.
   * @param[in]  shutter_in   requested shutter state
   * @param[out] shutter_out  reference to string for return status
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::shutter(std::string shutter_in, std::string& shutter_out) {
    std::string function = "AstroCam::Interface::shutter";
    std::stringstream message;
    long error = NO_ERROR;
    shutter_out = "undefined";  // undefined until set below

    // Help
    //
    if ( shutter_in == "?" ) {
      shutter_out = CAMERAD_SHUTTER;
      shutter_out.append( " [ enable | 1 | disable | 0 ]\n" );
      shutter_out.append( "  Sets (and reads) the shutter enable state which controls whether\n" );
      shutter_out.append( "  or not the shutter will open for an exposure. 1 and 0 are equivalent\n" );
      shutter_out.append( "  to enable and disable, respectively. If no arg is given then the\n" );
      shutter_out.append( "  current shutterenable state is returned.\n" );
      shutter_out.append( "\n" );
      if ( this->camera.ext_shutter ) {
        shutter_out.append( "  For external shutter (i.e. controlled by Leach controller) a PCI\n" );
        shutter_out.append( "  connection must be opened to a controller before the enable\n" );
        shutter_out.append( "  state can be set or read.\n" );
        shutter_out.append( "\n" );
      }
      shutter_out.append( "  See also \""+CAMERAD_TEST+" shutter\" for manual shutter operation.\n" );
      return HELP;
    }

    // Process an input argument to set the shutter enable/disable state
    //
    if ( !shutter_in.empty() ) {
      try {
        std::transform( shutter_in.begin(), shutter_in.end(), shutter_in.begin(), ::tolower );  // make lowercase
        if ( shutter_in == "disable" || shutter_in == "0" ) this->camera.shutter.is_enabled = false;
        else
        if ( shutter_in == "enable"  || shutter_in == "1" ) this->camera.shutter.is_enabled = true;
        else {
          message.str(""); message << "ERROR: " << shutter_in << " is invalid. Expecting enable or disable";
          logwrite( function, message.str() );
          error = ERROR;
        }
      }
      catch (...) {
        logwrite( function, "ERROR: program exception converting shutter_in to lowercase" );
        return( ERROR );
      }

      // Set shutterenable the same for all devices
      //
      if ( error==NO_ERROR && this->camera.ext_shutter ) for ( const auto &dev : this->devnums ) {
        this->controller[dev].info.shutterenable = this->camera.shutter.is_enabled;
      }
    }

    // For external shutter (i.e. triggered by Leach controller)
    // read the shutterenable state back from the controller class.
    //
    if ( this->camera.ext_shutter ) for ( const auto &dev : this->devnums ) {
      this->camera.shutter.is_enabled = this->controller[dev].info.shutterenable;
      break;  // just need one since they're all the same
    }
    // otherwise shutterenable state is whatever is in the camera_info class

    shutter_out = this->camera.shutter.is_enabled ? "enabled" : "disabled";

    // set the return value and report the state now, either setting or getting
    //
    message.str(""); message << "shutter is " << shutter_out;
    logwrite( function, message.str() );

    // Add the shutter enable keyword to the system keys db
    //
    this->camera_info.systemkeys.primary().addkey( "SHUTTEN", ( this->camera.shutter.is_enabled ? "T" : "F" ), "shutter was enabled" );

    return error;
  }
  /***** AstroCam::Interface::shutter *****************************************/


  /***** AstroCam::Interface::frame_transfer_mode *****************************/
  /**
   * @brief      set/get frame transfer mode
   * @details    overloaded version to call without retstring
   * @param[in]  args       contains: <dev> | <chan> [ yes | no ]
   * @return     ERROR or NO_ERROR
   *
   * args contains the device# or channel and optionally yes|no
   * If yes|no is omitted then the current state for that chan/dev is returned.
   *
   */
  long Interface::frame_transfer_mode( std::string args ) {
    std::string dontcare;
    return this->frame_transfer_mode( args, dontcare );
  }
  /***** AstroCam::Interface::frame_transfer_mode *****************************/


  /***** AstroCam::Interface::frame_transfer_mode *****************************/
  /**
   * @brief      set/get frame transfer mode
   * @param[in]  args       contains: <dev> | <chan> [ yes | no ]
   * @param[out] retstring  reference to string for return value
   * @return     ERROR | NO_ERROR | HELP
   *
   * args contains the device# or channel and optionally yes|no
   * If yes|no is omitted then the current state for that chan/dev is returned.
   *
   */
  long Interface::frame_transfer_mode( std::string args, std::string &retstring ) {
    std::string function = "AstroCam::Interface::frame_transfer_mode";
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
      retstring = CAMERAD_FRAMETRANSFER;
      retstring.append( " <chan> | <dev#> | all [ yes | no ]\n" );
      retstring.append( "  Set or get frame transfer mode for specified device.\n" );
      retstring.append( "  If \"all\" instead of device then return yes only if all devices are yes." );
      retstring.append( "  If optional yes|no is omitted then current state is returned.\n" );
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

    // return and-state of all cameras,
    // I.E. if all are true, return true, otherwise false
    //
    if (args=="all") {
      bool all_true = true;
      for ( const auto &dev : this->devnums ) {
        if ( !this->controller[dev].have_ft ) {
          all_true=false;
          break;
        }
      }
      retstring = all_true ? "yes" : "no";
      message.str(""); message << (all_true?"all":"not all") << " cameras use frame transfer";
      logwrite(function, message.str());
      return NO_ERROR;
    }

    // Get the devnum and channel from args.
    // Borrow retstring temporarily to carry the state, if supplied.
    //
    int dev;
    std::string chan;

    long error = this->extract_dev_chan( args, dev, chan, retstring );

    if ( error != NO_ERROR ) return error;

    if ( !retstring.empty() && retstring != "yes" && retstring != "no" ) {
      message.str(""); message << "ERROR: bad state \"" << retstring << "\". expected { yes | no }";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return( ERROR );
    }

    // If a state was provided then set it
    //
    if ( ! retstring.empty() ) {
      this->controller[dev].have_ft = ( retstring == "yes" ? true : false );
      // add keyword to the extension for this channel
//TCB this->controller[dev].info.systemkeys.add_key( "FT", this->controller[dev].have_ft, "frame transfer used", EXT, chan );
    }

    // In any case, return the current state
    //
    retstring = ( this->controller[dev].have_ft ? "yes" : "no" );

    return( NO_ERROR );
  }
  /***** AstroCam::Interface::frame_transfer_mode *****************************/


  /***** AstroCam::Interface::image_size **************************************/
  /**
   * @brief      set/get image size parameters and allocate PCI memory
   * @details    Sets/gets ROWS COLS OSROWS OSCOLS BINROWS BINCOLS for a
   *             given device|channel. This calls geometry() and buffer() to
   *             set the image geometry on the controller and allocate a PCI buffer.
   * @param[in]  args       contains DEV|CHAN [ [ SPAT SPEC OSSPAT OSSPEC BINSPAT BINSPEC ]
   * @param[out] retstring  reference to string for return value
   * @return     ERROR | NO_ERROR | HELP
   *
   */
  long Interface::image_size( std::string args, std::string &retstring, const bool save_as_default ) {
    const std::string function("AstroCam::Interface::image_size");
    std::ostringstream message;

    // Help
    if ( args == "?" ) {
      retstring = CAMERAD_IMSIZE;
      retstring.append( " <chan> | <dev#> [ [ <spat> <spec> <osspat> <osspec> <binspat> <binspec> ]\n" );
      retstring.append( "  Configures image parameters in spectral and spatial dimensions.\n" );
      retstring.append( "  <spat>    = spatial dimension (along slit)\n" );
      retstring.append( "  <spec>    = spectral dimension (dispersion direction)\n" );
      retstring.append( "  <osspat>  = spatial overscans\n" );
      retstring.append( "  <osspec>  = spectral overscan pixels\n" );
      retstring.append( "  <binspat> = spatial binning factor\n" );
      retstring.append( "  <binspec> = spectral binning factor\n" );
      retstring.append( "\n" );
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
      return( ERROR );
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

    Controller* pcontroller = &this->controller[dev];

    int spat=-1, spec=-1, osspat=-1, osspec=-1, binspat=-1, binspec=-1;
    // start by loading the values in the class
    pcontroller->physical_to_logical( pcontroller->detrows, pcontroller->detcols, spat, spec );
    pcontroller->physical_to_logical( pcontroller->osrows, pcontroller->oscols, osspat, osspec );
    pcontroller->physical_to_logical( pcontroller->info.binning[_ROW_], pcontroller->info.binning[_COL_],
                                      binspat, binspec );

    // no args returns current settings only
    //
    if (tokens.empty()) {
      message.str(""); message << "spat spec osspat osspec binspat binspec = ";
      message << spat << " " << spec << " " << osspat << " " << osspec << " " << binspat << " " << binspec
              << ( pcontroller->connected ? "" : " [inactive]" );
      logwrite( function, message.str() );
      retstring = message.str();
      return NO_ERROR;
    }
    else

    // tokens not empty, first tok is "init" will use the values in the class
    if (tokens[0]=="init") {
      // use the values from the class gotten above
    }
    else

    // check for complete set of args
    if (tokens.size()==6) {
      try {
        spat    = std::stoi( tokens.at(0) );
        spec    = std::stoi( tokens.at(1) );
        osspat  = std::stoi( tokens.at(2) );
        osspec  = std::stoi( tokens.at(3) );
        binspat = std::stoi( tokens.at(4) );
        binspec = std::stoi( tokens.at(5) );
      }
      catch (const std::exception &e ) {
        logwrite(function, "ERROR parsing \"" + retstring + "\": " + std::string(e.what()));
        retstring="invalid_argument";
        return ERROR;
      }
    }
    else {
      logwrite(function,
          "ERROR invalid number of arguments. expected <spat> <spec> <osspat> <osspec> <binspat> <binspec>");
      retstring="invalid_argument";
      return ERROR;
    }

    // If we got here then spat,spec,etc. have been set,
    // either from the class or the supplied arg list

    // Check image size
    //
    if ( spat<1 || spec<1 || osspat<0 || osspec<0 || binspat<1 || binspec<1 ) {
      message.str(""); message << "ERROR invalid image size " << spat << " " << spec << " "
                               << osspat << " " << osspec << " " << binspat << " " << binspec;
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return( ERROR );
    }

    // Translate supplied (logical) to physical (row/col) coordinates
    int rows, cols, osrows, oscols, binrows, bincols;
    pcontroller->logical_to_physical(spat, spec, rows, cols);
    pcontroller->logical_to_physical(osspat, osspec, osrows, oscols);
    pcontroller->logical_to_physical(binspat, binspec, binrows, bincols);

//  message.str(""); message << "[DEBUG] input imsize: " << rows << " " << cols << " "
//                           << osrows << " " << oscols << " " << binrows << " " << bincols;
//  logwrite( function, message.str() );

    // Store the geometry in the class. This is the new detector geometry,
    // unchanged by binning, so that when reverting to binning=1 from some
    // binnnig factor, this is the default image size to revert to.
    //
    pcontroller->detrows = rows;
    pcontroller->detcols = cols;
    pcontroller->osrows0 = osrows;
    pcontroller->oscols0 = oscols;

    // Binning is the same for all devices so it's stored in the camera info class.
    //
    this->camera_info.binspat = binspat;
    this->camera_info.binspec = binspec;

    // Assign the binning also to the controller info
    //
    pcontroller->info.binning[_ROW_] = binrows;
    pcontroller->info.binning[_COL_] = bincols;

    // If binned by a non-evenly-divisible factor then skip modulo that
    // many at the start. These will be removed from the image.
    //
    pcontroller->skipcols = cols % bincols;
    pcontroller->skiprows = rows % binrows;

//  message.str(""); message << "[DEBUG] skipcols=" << this->controller[dev].skipcols << " skiprows=" << this->controller[dev].skiprows;
//  logwrite( function, message.str() );

    cols -= pcontroller->skipcols;
    rows -= pcontroller->skiprows;

//  message.str(""); message << "[DEBUG] cols=" << cols << " rows=" << rows;
//  logwrite( function, message.str() );

    // Adjust the number of overscans to make them evenly divisible
    // by the binning factor.
    //
    oscols -= ( oscols % bincols );
    osrows -= ( osrows % binrows );

//  message.str(""); message << "[DEBUG] oscols=" << oscols << " osrows=" << osrows;
//  logwrite( function, message.str() );

    // Now that the rows/cols and osrows/oscols have been adjusted for
    // binning, store them in the class as detector_pixels for this controller.
    //
    pcontroller->info.detector_pixels[_COL_] = cols + oscols;
    pcontroller->info.detector_pixels[_ROW_] = rows + osrows;

    // ROI is the full detector
    pcontroller->info.region_of_interest[0] = 1;
    pcontroller->info.region_of_interest[1] = pcontroller->info.detector_pixels[0];
    pcontroller->info.region_of_interest[2] = 1;
    pcontroller->info.region_of_interest[3] = pcontroller->info.detector_pixels[1];

    pcontroller->info.ismex = true;
    pcontroller->info.bitpix = 16;
    pcontroller->info.frame_type = Camera::FRAME_RAW;

//  message.str("");
//  message << "[DEBUG] new binned values before set_axes() to re-calculate:"
//          << " detector_pixels[" << _COL_ << "]=" << this->controller[dev].info.detector_pixels[_COL_]
//          << " detector_pixels[" << _ROW_ << "]=" << this->controller[dev].info.detector_pixels[_ROW_];
//  logwrite(function, message.str());

    // *** This is where the binned-image dimensions are re-calculated ***
    //
    // A call to set_axes() will store the binned image dimensions
    // in controller[dev].info.axes[*]. Those dimensions will
    // be used to set the image geometry with do_geometry() and
    // the PCI buffer with buffer().
    //
    if ( pcontroller->info.set_axes() != NO_ERROR ) {
      message.str(""); message << "ERROR setting axes for device " << dev;
      this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
      return( ERROR );
    }

    // because set_axes() doesn't scale overscan
    //
    oscols /= bincols;
    osrows /= binrows;

    // save the binning-adjusted overscans to the class
    //
    pcontroller->osrows = osrows;
    pcontroller->oscols = oscols;

    // if requested, store the imsize values as a string in the class for default recovery
    //
    if ( save_as_default ) {
      message.str(""); message << rows << " " << cols << " " << osrows << " " << oscols << " " << binrows << " " << bincols;
      pcontroller->imsize_args = message.str();
      pcontroller->defrows = rows;
      pcontroller->defcols = cols;
      pcontroller->defosrows = osrows;
      pcontroller->defoscols = oscols;
      logwrite( function, "saved as default for chan "+chan+": "+message.str() );
    }

    // This is as far as we can get without a connected controller.
    // If not connected then all we've done is save this info to the class,
    // which will be used after the controller is connected.
    //
    if ( pcontroller->connected && pcontroller->firmwareloaded ) {

      // allocate PCI buffer and set geometry now
      //
      std::stringstream geostring;
      std::string retstring;
      geostring << dev << " "
                << pcontroller->info.axes[_ROW_] << " "
                << pcontroller->info.axes[_COL_];

//    message.str("");
//    message << "[DEBUG] allocating buffer for " << pcontroller->info.axes[_ROW_] << " x "
//                                                << pcontroller->info.axes[_COL_];
//    logwrite( function, message.str() );

      if ( this->buffer( geostring.str(), retstring ) != NO_ERROR ) {
        message.str(""); message << "ERROR: allocating buffer for chan " << pcontroller->channel
                                 << " " << pcontroller->devname;
        logwrite( function, message.str() );
        return ERROR;
      }

//    message.str(""); message << "[DEBUG] calling do_geometry( " << geostring.str() << " )";
//    logwrite(function, message.str());

      if ( this->do_geometry( geostring.str(), retstring ) != NO_ERROR ) {
        message.str(""); message << "ERROR: setting geometry for chan " << pcontroller->channel;
        logwrite( function, message.str() );
        return ERROR;
      }

      // Set the Bin Parameters to the controller using the 3-letter command,
      // "SBP <NPBIN> <NP_SKIP> <NSBIN> <NS_SKIP>"
      //
      std::stringstream cmd;
      cmd << "SBP "
          << pcontroller->info.binning[_ROW_] << " "
          << pcontroller->skiprows << " "
          << pcontroller->info.binning[_COL_] << " "
          << pcontroller->skipcols;
      if ( this->do_native( dev, cmd.str(), retstring ) != NO_ERROR ) return ERROR;

      // finally can set this
      pcontroller->is_imsize_set = true;
    }
    else {
      message.str(""); message << "saved but not sent to controller because chan " << pcontroller->channel
                               << (!pcontroller->connected ? " not connected ":"")
                               << (!pcontroller->firmwareloaded ? " firmware not loaded" :"");
      logwrite( function, message.str() );
    }

    // Return the physical values stored in the class as logical (spat/spec)
    //
    pcontroller->physical_to_logical( pcontroller->detrows, pcontroller->detcols, spat, spec );
    pcontroller->physical_to_logical( pcontroller->osrows, pcontroller->oscols, osspat, osspec );
    pcontroller->physical_to_logical( pcontroller->info.binning[_ROW_], pcontroller->info.binning[_COL_],
                                      binspat, binspec );

    message.str(""); message << "spat spec osspat osspec binspat binspec = ";
    message << spat << " " << spec << " " << osspat << " " << osspec << " " << binspat << " " << binspec
            << ( pcontroller->connected ? "" : " [inactive]" );
    logwrite( function, message.str() );

    retstring = message.str();

    message.str(""); message << "[DEBUG] physical rows=" << pcontroller->detrows
                             << " cols=" << pcontroller->detcols;
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /***** AstroCam::Interface::image_size **************************************/


  /***** AstroCam::Interface::do_geometry *************************************/
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
  long Interface::do_geometry(std::string args, std::string &retstring) {
    std::string function = "AstroCam::Interface::do_geometry";
    std::stringstream message;

    // Help
    //
    if ( args == "?" ) {
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
  /***** AstroCam::Interface::do_geometry *************************************/


  /***** AstroCam::Interface::bias ********************************************/
  /**
   * @brief      set a bias
   * @param[in]  args       contains: module, channel, bias
   * @param[out] retstring  reference to string for return value
   * @return     ERROR or NO_ERROR
   * @todo       bias not yet implemented
   *
   * TODO
   *
   */
  long Interface::bias(std::string args, std::string &retstring) {
    std::string function = "AstroCam::Interface::bias";
    std::stringstream message;
    logwrite(function, "ERROR: not implemented");
    return( ERROR );
  }
  /***** AstroCam::Interface::bias ********************************************/


  /***** AstroCam::Interface::handle_queue ************************************/
  /**
   * @brief      inserts a message into the asynchronous message queue
   * @param[in]  message  string to insert into the async message queue
   *
   * This should be spawned in a thread. This is how threads get messages
   * into the async message queue.
   *
   */
  void Interface::handle_queue(std::string message) {
    server.camera.async.enqueue( message );
    return;
  }
  void Interface::handle_queue2(std::string message) {
    server.camera.async.enqueue( message );
    return;
  }
  /***** AstroCam::Interface::handle_queue ************************************/


  /***** AstroCam::Interface::handle_frame ************************************/
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
  void Interface::handle_frame( int expbuf, int devnum, uint32_t fpbcount, uint32_t fcount, void* buffer ) {
    std::string function = "AstroCam::Interface::handle_frame";
    std::stringstream message;
    int error = NO_ERROR;

#ifdef LOGLEVEL_DEBUG
    message << "[DEBUG] expbuf=" << expbuf << " devnum=" << devnum << " " << "fpbcount=" << fpbcount 
            << " fcount=" << fcount << " PCI buffer=" << std::hex << std::uppercase << buffer;
    logwrite(function, message.str());
#endif

    server.frameinfo_mutex.lock();                 // protect access to frameinfo structure

    // Check to see if there is another frame with the same fpbcount...
    //
    // fpbcount is the frame counter that counts from 0 to FPB, so if there are 3 Frames Per Buffer
    // then for 10 frames, fcount goes from 0,1,2,3,4,5,6,7,8,9
    // and fpbcount goes from 0,1,2,0,1,2,0,1,2,0
    //
    // When useframes is false, fpbcount=0, fcount=0, framenum=0
    //
    if ( ! server.controller[devnum].have_ft ) {
      server.exposure_pending( devnum, false );    // this also does the notify
      server.state_monitor_condition.notify_all();
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dev " << devnum << " chan " << server.controller[devnum].channel << " exposure_pending=false";
      server.camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
#endif
    }
    server.controller[devnum].in_readout = false;
    server.state_monitor_condition.notify_all();
#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] dev " << devnum << " chan " << server.controller[devnum].channel << " in_readout=false";
    server.camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
#endif

    server.controller[devnum].frameinfo[fpbcount].tid    = fpbcount;  // create this index in the .frameinfo[] map
    server.controller[devnum].frameinfo[fpbcount].buf    = buffer;

/***
    if ( server.controller[devnum].frameinfo.count( fpbcount ) == 0 ) {     // searches .frameinfo[] map for an index of fpbcount (none)
      server.controller[devnum].frameinfo[ fpbcount ].tid      = fpbcount;  // create this index in the .frameinfo[] map
      server.controller[devnum].frameinfo[ fpbcount ].buf      = buffer;
      // If useframes is false then set framenum=0 because it doesn't mean anything,
      // otherwise set it to the fcount received from the API.
      //
      server.controller[devnum].frameinfo[ fpbcount ].framenum = server.useframes ? fcount : 0;
    }
    else {                                                                  // already have this fpbcount in .frameinfo[] map
      message.str(""); message << "ERROR: frame buffer overrun! Try allocating a larger buffer."
                               << " chan " << server.controller[devnum].channel;
      logwrite( function, message.str() );
      server.frameinfo_mutex.unlock();
      return;
    }
***/

    server.frameinfo_mutex.unlock();               // release access to frameinfo structure

/***
    // Write the frames in numerical order.
    // Don't let one thread get ahead of another when it comes to writing.
    //
    double start_time = get_clock_time();
    do {
      int this_frame = fcount;                     // the current frame
      int last_frame = server.controller[devnum].get_framecount();    // the last frame that has been written by this device
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

      if ( server.camera.get_abortstate() ) break; // provide the user a way to get out

    } while ( server.useframes );                  // some firmware doesn't support frames so get out after one frame if it doesn't
***/

    // If not aborted then write this frame
    //
    if ( ! server.camera.get_abortstate() ) {
#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] calling server.write_frame for devnum=" << devnum << " fpbcount=" << fpbcount;
    logwrite(function, message.str());
#endif
      error = server.write_frame( expbuf, devnum, server.controller[devnum].channel, fpbcount );
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
    server.frameinfo_mutex.lock();                 // protect access to frameinfo structure
//  server.controller[devnum].frameinfo.erase( fpbcount );

/*** 10/30/23 BOB
    server.controller[devnum].close_file( server.camera.writekeys_when );
***/

    server.frameinfo_mutex.unlock();

    server.remove_framethread();  // framethread_count is decremented because a thread has completed

    server.write_pending( expbuf, devnum, false ); // this also does the notify

    return;
  }
  /***** AstroCam::Interface::handle_frame ************************************/


  /***** AstroCam::Interface::FITS_handler ************************************/
  /**
   * @brief      performs the opening and closing of FITS files
   * @details    The FITS file is opened as soon as this thread starts, then
   *             this thread blocks on a condition wait until notified that
   *             all writes have been completed, at which time the FITS file
   *             is closed and this thread exits.
   * @param[in]  expbuf     exposure buffer number
   * @param[in]  interface  reference to Interface class object
   *
   */
  void Interface::FITS_handler( int expbuf, Interface &interface ) {
    std::string function = "AstroCam::Interface::FITS_handler";
    std::stringstream message;

    // Initialize the pFits pointer for this exposure buffer to a new FITS_file object
    //
//  interface.camera_info.datatype = USHORT_IMG;
//  interface.camera_info.type_set = true;
//  interface.camera_info.ismex = true;
    interface.pFits[ expbuf ] = ( new FITS_file() );
    interface.pFits[ expbuf ]->extension=0;
    logwrite( function, "about to call pFits[ expbuf ]->open_file()" );
//  long error = interface.pFits[ expbuf ]->open_file( true, interface.camera_info );
    long error = interface.pFits[ expbuf ]->open_file( true, *interface.fitsinfo[ expbuf ] );
    if ( error==NO_ERROR ) {
//    message.str(""); message << "opened \"" << interface.camera_info.fits_name << "\" returned " << error;
      message.str(""); message << "opened \"" << interface.fitsinfo[expbuf]->fits_name << "\" returned " << error;
      logwrite( function, message.str() );
    }
    else {
//    interface.pFits[expbuf]->close_file( true, interface.camera_info );
//    message.str(""); message << "ERROR opening \"" << interface.camera_info.fits_name << "\". FITS handler exiting";
      interface.pFits[expbuf]->close_file( true, *interface.fitsinfo[expbuf] );
      interface.camera.increment_imnum();
      message.str(""); message << "ERROR opening \"" << interface.fitsinfo[expbuf]->fits_name << "\". FITS handler exiting";
      logwrite( function, message.str() );
      return;
    }

    // Block on write_condition until there are no writes pending
    // (writes_pending vector has zero size) which only happens when
    // all devices have written their frames for this exposure number.
    //
    while ( interface.writes_pending[ expbuf ].size() > 0 ) {
      std::unique_lock<std::mutex> write_lock( interface.write_lock );

      message.str(""); message << "NOTICE:exposure buffer " << expbuf << " waiting for frames from ";
      std::vector<int> pending = interface.writes_pending[ expbuf ];
      for ( const auto &dev : pending ) message << interface.controller[dev].channel << " ";
      logwrite( function, message.str() );

      // wait() will repeatedly call this lambda function before actually entering
      // the waiting state, and if false, it won't wait and will continue. The
      // loop will exit only when the predicate returns true. wait() will also
      // automatically release the mutex lock temporarily to allow other threads
      // access the shared data.
      //
      interface.write_condition.wait( write_lock, [&] { return !(interface.writes_pending[ expbuf ].size()>0); } );
    }

    message.str(""); message << "NOTICE:all frames "
                             << ( interface.camera.get_abortstate() ? "aborted" : "written" )
                             << " for exposure buffer " << expbuf;
    logwrite( function, message.str() );

//  interface.pFits[expbuf]->close_file( true, interface.camera_info );
    interface.pFits[expbuf]->close_file( true, *interface.fitsinfo[expbuf] );
    interface.camera.increment_imnum();

    return;
  }
  /***** AstroCam::Interface::FITS_handler ************************************/


  /***** AstroCam::Interface::add_framethread *********************************/
  /**
   * @brief      call on thread creation to increment framethreadcount
   *
   */
  inline void Interface::add_framethread() {
    std::lock_guard<std::mutex> lock(this->framethreadcount_mutex);
    this->framethreadcount++;
  }
  /***** AstroCam::Interface::add_framethread *********************************/


  /***** AstroCam::Interface::remove_framethread ******************************/
  /**
   * @brief      call on thread destruction to decrement framethreadcount
   *
   */
  inline void Interface::remove_framethread() {
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
  inline int Interface::get_framethread_count() {
    int count;
    std::lock_guard<std::mutex> lock(this->framethreadcount_mutex);
    count = this->framethreadcount;
    return( count );
  }
  /***** AstroCam::Interface::get_framethread *********************************/


  /***** AstroCam::Interface::init_framethread_count **************************/
  /**
   * @brief      initialize framethreadcount = 0
   *
   */
  inline void Interface::init_framethread_count() {
    std::lock_guard<std::mutex> lock(this->framethreadcount_mutex);
    this->framethreadcount = 0;
  }
  /***** AstroCam::Interface::init_framethread_count **************************/


  /***** AstroCam::Interface::Controller::Controller **************************/
  /**
   * @brief      class constructor
   *
   */
  Interface::Controller::Controller() {
    this->workbuf = NULL;
    this->workbuf_size = 0;
    this->bufsize = 0;
    this->rows=0;
    this->cols=0;
    this->devnum = 0;
    this->framecount = 0;
    this->pArcDev = NULL;
    this->pCallback = NULL;
    this->connected = false;
    this->is_imsize_set = false;
    this->firmwareloaded = false;
    this->firmware = "";
    this->info.readout_name = "";
    this->info.readout_type = -1;
    this->readout_arg = 0xBAD;
    this->expinfo.resize( NUM_EXPBUF );  // vector of Camera::Information, one for each exposure buffer
    this->info.exposure_unit = "msec";   // chaning unit not currently supported in ARC
  }
  /***** AstroCam::Interface::Controller::Controller **************************/


  /***** AstroCam::Interface::Controller::logical_to_physical *****************/
  /**
   * @brief      translates logical (spat,spec) to physical (rows,cols)
   * @param[in]  spat
   * @param[in]  spec
   * @param[out] rows
   * @param[out] cols
   *
   */
  void Interface::Controller::logical_to_physical(int spat, int spec, int &rows, int &cols) const {
    if (spec_axis==COL) {
      cols=spec; rows=spat;  // G, R, I
    }
    else {
      rows=spec; cols=spat;  // U
    }
  }
  /***** AstroCam::Interface::Controller::logical_to_physical *****************/


  /***** AstroCam::Interface::Controller::physical_to_logical *****************/
  /**
   * @brief      translates physical (rows,cols) to logical (spat,spec)
   * @param[in]  rows
   * @param[in]  cols
   * @param[out] spat
   * @param[out] spec
   *
   */
  void Interface::Controller::physical_to_logical(int rows, int cols, int &spat, int &spec) const {
    if (spec_axis==COL) {
      spec=cols; spat=rows;  // G, R, I
    }
    else {
      spec=rows; spat=cols;  // U
    }
  }
  /***** AstroCam::Interface::Controller::physical_to_logical *****************/


  /***** AstroCam::Interface::Controller::open_file ***************************/
  /**
   * @brief      OBSOLETE: wrapper to open the current fits file object
   * @param[in]  writekeys_in  string containing "before|after" for when to write fits keys relative to exposure
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::Controller::open_file( std::string writekeys_in ) {
    logwrite( "AstroCam::Interface::Controller::open_file", "ERROR: this function is obsolete!" );
    return( ERROR );
/*
    bool writekeys;
    if ( writekeys_in == "before" ) writekeys = true; else writekeys = false;
#ifdef LOGLEVEL_DEBUG
    std::string function = "AstroCam::Interface::Controller::open_file";
    std::stringstream message;
    message << "[DEBUG] devnum=" << devnum
            << " fits_name=" << this->info.fits_name 
            << " this->pFits=" << std::hex << std::uppercase << this->pFits;
    logwrite(function, message.str());
#endif
    long error = this->pFits->open_file( writekeys, this->info );
    return( error );
*/
  }
  /***** AstroCam::Interface::Controller::open_file ***************************/


  /***** AstroCam::Interface::Controller::close_file **************************/
  /**
   * @brief      OBSOLETE: wrapper to close the current fits file object
   * @param[in]  writekeys_in  string containing "before|after" for when to write fits keys relative to exposure
   *
   */
  void Interface::Controller::close_file( std::string writekeys_in ) {
    std::string function = "AstroCam::Interface::Controller::close_file";
    logwrite( function, "ERROR: this function is obsolete!" );
    return;
/***
    bool writekeys;
    if ( writekeys_in == "after" ) writekeys = true; else writekeys = false;
#ifdef LOGLEVEL_DEBUG
    std::stringstream message;
    message << "[DEBUG] devnum=" << devnum
            << " fits_name=" << this->info.fits_name 
            << " this->pFits=" << std::hex << std::uppercase << this->pFits;
    logwrite(function, message.str());
#endif
    try {
      this->pFits->close_file( writekeys, this->info );
    }
    catch(...) { logwrite(function, "unknown error closing FITS file(s)"); }
    return;
***/
  }
  /***** AstroCam::Interface::Controller::close_file **************************/


  /***** AstroCam::Interface::Controller::init_framecount *********************/
  /**
   * @brief      initialize this->framecount=0, protected by mutex
   *
   */
  inline void Interface::Controller::init_framecount() {
    std::lock_guard<std::mutex> lock(server.framecount_mutex);
    this->framecount = 0;
  }
  /***** AstroCam::Interface::Controller::init_framecount *********************/


  /***** AstroCam::Interface::Controller::get_framecount **********************/
  /**
   * @brief      returns this->framecount, protected by mutex
   * @return     int framecount
   *
   */
  inline int Interface::Controller::get_framecount() {
    int count;
    std::lock_guard<std::mutex> lock(server.framecount_mutex);
    count = this->framecount;
    return( count );
  }
  /***** AstroCam::Interface::Controller::get_framecount **********************/


  /***** AstroCam::Interface::Controller::increment_framecount ****************/
  /**
   * @brief      increments this->framecount, protected by mutex
   *
   */
  inline void Interface::Controller::increment_framecount() {
    std::lock_guard<std::mutex> lock(server.framecount_mutex);
    this->framecount++;
  }
  /***** AstroCam::Interface::Controller::increment_framecount ****************/


  /***** AstroCam::Interface::deinterlace *************************************/
  /**
   * @brief      spawns the deinterlacing threads
   * @param[in]  imbuf  pointer to the PCI image buffer containing data to deinterlace
   * @return     pointer to workbuf
   *
   * This function spawns threads to perform the actual deinterlacing in order
   * to get it done as quickly as possible.
   * 
   * Called by write_frame(), which is called by the handle_frame thread.
   *
   * incoming frame from ARC -> frameCallback() -> handle_frame() -> write_frame() -> deinterlace()
   *
   */
  template <class T>
  T* Interface::Controller::deinterlace( int expbuf, T* imbuf ) {
    std::string function = "AstroCam::Interface::deinterlace";
    std::stringstream message;

    int nthreads = cores_available();
    nthreads=2;  ///< TODO @todo need to optimize this for number of cores
    server.camera.async.enqueue_and_log( "CAMERAD", function, "NOTICE: override nthreads=2 !!!" );

#ifdef LOGLEVEL_DEBUG
    message << "[DEBUG] devnum=" << this->devnum << " nthreads=" << nthreads << " imbuf=" << std::hex << imbuf << " workbuf=" << std::hex << this->workbuf
            << " this->info.readout_name=" << this->info.readout_name << "(" << this->info.readout_type << ")" 
            << " cols=" << std::dec << this->cols << " rows=" << std::dec << this->rows;
    logwrite(function, message.str());
#endif

    // Instantiate a DeInterlace class object,
    // which is constructed with the pointers need for the image and working buffers.
    // This object contains the functions needed for the deinterlacing,
    // which will get called by the threads created here.
    //
    DeInterlace deinterlace( imbuf, (T*)this->workbuf, this->workbuf_size,
                             this->cols,
                             this->rows,
                             this->info.readout_type );

//  deinterlace.bob();

    // spawn threads to handle each sub-section of the image
    //
    {                                       // begin local scope
    std::vector<std::thread> threads;       // create a local scope vector for the threads
#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] spawning deinterlacing threads, from 1 to " << nthreads << "...";
    logwrite( function, message.str() );
#endif
    for ( int section = 1; section <= nthreads; section++ ) {
      std::thread thr( &AstroCam::Interface::Controller::dothread_deinterlace<T>, 
                   std::ref(deinterlace), 
                   this->cols,
                   this->rows,
                   section,
                   nthreads );
      threads.push_back(std::move(thr));    // push the thread into a vector
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] devnum " << this->devnum << " waiting for deinterlacing threads";
    logwrite(function, message.str());
#endif

    // By joining all the deinterlace threads to this thread (NOT to each other!) then
    // this function will not return until all of the deinterlace threads have finished.
    // This is necessary because the DeInterlace object cannot be allowed to go out of scope
    // before all threads are done using it.
    //
    try {
      for (std::thread & thr : threads) {   // loop through the vector of threads
        if ( thr.joinable() ) thr.join();   // if thread object is joinable then join to this function. (not to each other)
      }
    }
    catch(const std::exception &e) {
      message.str(""); message << "ERROR joining threads: " << e.what();
      logwrite(function, message.str());
    }
    catch(...) { logwrite(function, "unknown error joining threads"); }

    threads.clear();                        // deconstruct the threads vector
    }

    message.str(""); message << this->devname << " channel " << this->channel << " deinterlacing complete";
    logwrite(function, message.str());

    return( (T*)this->workbuf );
  }
  /***** AstroCam::Interface::deinterlace *************************************/


  /***** AstroCam::Interface::dothread_deinterlace ****************************/
  /**
   * @brief      run in a thread to perform the actual deinterlacing
   * @param[in]  deinterlace  reference to DeInterlace class object
   * @param[in]  cols         number of columns
   * @param[in]  rows         number of rows
   * @param[in]  section      the section of the buffer this thread is working on
   * @param[in]  nthreads     the number of threads being used for deinterlacing
   *
   * This thread calls the deinterlacer. The actual deinterlacing is performed
   * within the DeInterlace object.
   *
   */
  template <class T>
  void Interface::Controller::dothread_deinterlace( DeInterlace<T> &deinterlace, int cols, int rows, int section, int nthreads ) {
    std::string function = "AstroCam::Interface::Controller::dothread_deinterlace";
    std::stringstream message;

    int rows_per_section = (int)( rows / nthreads );                         // whole number of rows per thread
    int index            = rows_per_section * cols * ( section - 1);         // index from start of buffer, forward
    int index_flip       = rows_per_section * cols * ( nthreads - section + 1);         // index from start of buffer, forward
    int row_start        = rows_per_section * (section-1);                   // first row this thread will deinterlace
    int row_stop         = rows_per_section * section;                       // last row this thread will deinterlace
    int modrows          = rows % nthreads;                                  // are the rows evenly divisible by the number of threads?
    if ( ( modrows != 0 ) && ( section == nthreads ) ) row_stop += modrows;  // add any leftover rows to the last thread if not evenly divisible

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] section=" << section << " " << deinterlace.info()
                             << " row_start=" << row_start << " row_stop=" << row_stop
                             << " modrows=" << modrows << " index=" << index;
    logwrite(function, message.str());
#endif

    deinterlace.do_deinterlace( row_start, row_stop, index, index_flip );

    return;
  }
  /***** AstroCam::Interface::dothread_deinterlace ****************************/


  /**** AstroCam::Interface::test *********************************************/
  /**
   * @brief      test routines
   * @param[in]  args       contains test name and arguments
   * @param[in]  retstring  reference to string for any return values
   * @return     ERROR | NO_ERROR | HELP
   *
   * This is the place to put various debugging and system testing tools.
   * It is placed here, rather than in camera, to allow for controller-
   * specific tests. This means some common tests may need to be duplicated
   * for each controller.
   *
   * The server command is "test", the next parameter is the test name,
   * and any parameters needed for the particular test are extracted as
   * tokens from the args string passed in.
   *
   * The input args string is tokenized and tests are separated by a simple
   * series of if..else.. conditionals.
   *
   */
  long Interface::test(std::string args, std::string &retstring) {
    std::string function = "Archon::Interface::test";
    std::stringstream message;
    std::vector<std::string> tokens;
    long error = NO_ERROR;

    // Help
    //
    if ( args == "?" ) {
      retstring = CAMERAD_TEST;
      retstring.append( "\n" );
      retstring.append( "  Test Routines\n" );
      retstring.append( "   async [ ? | <message> ]\n" );
      retstring.append( "   bw [ ? ]\n" );
      retstring.append( "   fitsname [ ? ]\n" );
      retstring.append( "   frametransfer ? | R | I | U | G \n" );
      retstring.append( "   lpsleep ? | <ms> | stop\n" );
      retstring.append( "   pending [ ? ]\n" );
      retstring.append( "   psleep ? | <ms>\n" );
      retstring.append( "   shdelay ? | <delay> | test\n" );
      retstring.append( "   shutter ? | init | open | close | get | time | expose <msec>\n" );
      retstring.append( "   telem ? | collect | test | calibd | flexured | focusd | tcsd\n" );
      retstring.append( "   isreadout\n" );
      retstring.append( "   pixelcount\n" );
      return HELP;
    }

    Tokenize(args, tokens, " ");

    if (tokens.size() < 1) {
      logwrite( function, "ERROR: no test name provided" );
      return ERROR;
    }

    std::string testname = tokens[0];                                // the first token is the test name

    if ( testname == "newexpose" ) {
      error = server.new_expose( "nseq" );
    }
    else

    // ----------------------------------------------------
    // fitsname
    // ----------------------------------------------------
    // Show what the fitsname will look like.
    // This is a "test" rather than a regular command so that it doesn't get mistaken
    // for returning a real, usable filename. When using fitsnaming=time, the filename
    // has to be generated at the moment the file is opened.
    //
    if (testname == "fitsname") {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {                              // help
        retstring = CAMERAD_TEST;
        retstring.append( " fitsname\n" );
        retstring.append( "  Show what the fitsname will look like.\n" );
        return HELP;
      }
      std::string msg;
      this->camera.set_fitstime( get_timestamp( ) );                 // must set camera.fitstime first
      if ( this->devnums.size() > 1 ) {
        for (const auto &dev : this->devnums) {
          this->camera.get_fitsname( std::to_string(dev), msg );     // get the fitsname (by reference)
          this->camera.async.enqueue( msg );                         // queue the fitsname
          logwrite( function, msg );                                 // log ths fitsname
        }
      }
      else {
        this->camera.get_fitsname( msg );                            // get the fitsname (by reference)
        this->camera.async.enqueue( msg );                           // queue the fitsname
        logwrite( function, msg );                                   // log ths fitsname
      }
    } // end if (testname == fitsname)
    else

    // ----------------------------------------------------
    // async [message]
    // ----------------------------------------------------
    // queue an asynchronous message
    // The [message] param is optional. If not provided then "test" is queued.
    //
    if (testname == "async") {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {                              // help
        retstring = CAMERAD_TEST;
        retstring.append( " async [ <message > ]\n" );
        retstring.append( "  Queue an async broadcast message. If no <message> provided,\n" );
        retstring.append( "  then \"test\" will be queued. Use double-quotes to send\n" );
        retstring.append( "  compound message strings.\n" );
        return HELP;
      }
      if (tokens.size() > 1) {
        if (tokens.size() > 2) {
          logwrite(function, "NOTICE: received multiple strings -- only the first will be queued");
        }
        logwrite( function, tokens[1] );
        this->camera.async.enqueue( tokens[1] );
      }
      else {                                // if no string passed then queue a simple test message
        logwrite(function, "test");
        this->camera.async.enqueue("test");
      }
    } // end if (testname == async)
    else

    // ----------------------------------------------------
    // bw <nseq>
    // ----------------------------------------------------
    // Bandwidth test
    // This tests the exposure sequence bandwidth by running a sequence
    // of exposures, including reading the frame buffer -- everything except
    // for the fits file writing.
    //
    if (testname == "bw") {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {                              // help
        retstring = CAMERAD_TEST;
        retstring.append( " bw\n" );
        retstring.append( "  Tests the exposure sequence bandwidth by running a sequence\n" );
        retstring.append( "  of exposures, including reading the frame buffer -- everything\n" );
        retstring.append( "  except for the FITS file writing.\n" );
        return HELP;
      }
      message.str(""); message << "ERROR: test " << testname << " not implemented";
      logwrite(function, message.str());
      error = ERROR;
    } // end if (testname==bw)
    else

    // ----------------------------------------------------
    // lpsleep
    // ----------------------------------------------------
    //
    if (testname == "lpsleep") {
      long ms;
      if ( tokens.size() > 1 ) {
        if ( tokens[1] == "?" ) {
          retstring = CAMERAD_TEST;
          retstring.append( " lpsleep <ms> | stop | pause | resume | modify <ms>\n" );
          retstring.append( "  Tests the Long Precise Sleep timer by spawning a thread that sleeps\n" );
          retstring.append( "  using the shutter_timer object. Delay time <ms> is a whole number of\n" );
          retstring.append( "  milliseconds and can be any value. This can be interrupted using the\n" );
          retstring.append( "  stop argument. The timer can be modified using the modify argument.\n" );
          error = HELP;
        }
        else
        if ( tokens[1] == "stop" ) {
          long remaining;
          this->camera.shutter_timer.stop(remaining);
          logwrite( function, "TEST: stopped timer test with "+std::to_string(remaining)+" ms remaining" );
          retstring=std::to_string(remaining);
          return NO_ERROR;
        }
        else
        if ( tokens[1] == "pause" ) {
          this->camera.shutter_timer.hold();
          logwrite( function, "TEST: test timer paused" );
          retstring="paused";
          return NO_ERROR;
        }
        else
        if ( tokens[1] == "resume" ) {
          this->camera.shutter_timer.resume();
          logwrite( function, "TEST: test timer resumed" );
          retstring="resumed";
          return NO_ERROR;
        }
        else
        if ( tokens[1] == "modify" && tokens.size() > 2 ) {
          try {
            ms = std::stol( tokens[2] );
            this->camera.shutter_timer.modify(ms);
            retstring="modified";
            return NO_ERROR;
          }
          catch ( const std::exception &e ) {
            logwrite( function, "ERROR parsing ms: "+std::string(e.what()) );
            return ERROR;
          }
        }
        else {
          try {
            ms = std::stol( tokens[1] );
            logwrite( function, "TEST: running timer in a detached thread" );
            std::thread( &AstroCam::Interface::dothread_test_shutter_timer, this, ms ).detach();
            retstring="running";
            return NO_ERROR;
          }
          catch ( const std::exception &e ) {
            logwrite( function, "ERROR parsing ms: "+std::string(e.what()) );
            return ERROR;
          }
        }
      }
      else {
        logwrite( function, "ERROR expected <ms> | stop | modify <ms>" );
        retstring="invalid_argument";
        return ERROR;
      }
    }
    else

    // ----------------------------------------------------
    // psleep
    // ----------------------------------------------------
    //
    if (testname == "psleep") {
      long ms;
      if ( tokens.size() > 1 ) {
        if ( tokens[1] == "?" ) {
          retstring = CAMERAD_TEST;
          retstring.append( " psleep <ms>\n" );
          retstring.append( "  Tests the (short) precise sleep timer directly, not in a thread.\n" );
          retstring.append( "  Delay time <ms> is a whole number of milliseconds and is limited to\n" );
          retstring.append( "  " );
          retstring.append( std::to_string(Camera::MAX_SHUTTER_DELAY) );
          retstring.append( "  ms. This cannot be interrupted.\n" );
          error = HELP;
        }
        else {
          try {
            long ms = std::stol( tokens.at(1) );
            if ( ms > Camera::MAX_SHUTTER_DELAY ) {
              logwrite( function, "ERROR delay cannot exceed 2000 ms" );
              retstring="invalid_argument";
              return ERROR;
            }
            logwrite( function, "TEST: precise sleep started" );
            precise_sleep( 1000*ms );  // precise_sleep() accepts microseconds
            logwrite( function, "TEST: precise sleep ended" );
          }
          catch ( const std::exception &e ) {
            message.str(""); message << "ERROR parsing long from \"" << ms << "\": " << e.what();
            logwrite( function, message.str() );
            return ERROR;
          }
        }
      }
      else {
        logwrite( function, "ERROR expected <ms> delay" );
        retstring="invalid_argument";
        return ERROR;
      }
    }
    else

    // ----------------------------------------------------
    // shutter
    // ----------------------------------------------------
    // Shutter control (see help below)
    // init, open, close, get, time, expose
    //
    if (testname == "shutter") {
      if ( tokens.size() == 2 ) {
        error = NO_ERROR;
        if ( tokens[1] == "open" ) {     // manually open shutter now
          error  = this->camera.shutter.set_open();
          usleep( 150000 );
          error |= this->test( "shutter get", retstring );
        }
        else
        if ( tokens[1] == "close" ) {    // manually close shutter now
          error  = this->camera.shutter.set_close();
          usleep( 150000 );
          error |= this->test( "shutter get", retstring );
        }
        else
        if ( tokens[1] == "init" ) {     // initialize Shutter class and open USB device
          error = this->camera.shutter.init();
        }
        else
        if ( tokens[1] == "get" ) {      // get the shutter state
          int state;
          error = this->camera.shutter.get_state(state);
          switch( state ) {
            case 0:  retstring="closed";  break;
            case 1:  retstring="open";    break;
            default: retstring="unknown"; break;
          }
        }
        else
        if ( tokens[1] == "time" ) {     // report the last recorded exposure duration
          double el = this->camera.shutter.get_duration();
          retstring = std::to_string( el );
        }
        else
        if ( tokens[1] == "?" ) {        // help
          retstring = CAMERAD_TEST;
          retstring.append( " shutter init | open | close | get | time | expose <msec> \n" );
          retstring.append( "  init:           initializes Shutter class and opens USB device, required before use\n" );
          retstring.append( "  open:           manually open shutter now\n" );
          retstring.append( "  close:          manually close now\n" );
          retstring.append( "  get:            returns shutter state\n" );
          retstring.append( "  time:           returns the last shutter open/close time duration\n" );
          retstring.append( "  expose <msec>:  open shutter for integral <msec> milliseconds\n" );
          error = HELP;
        }
        else {
          logwrite( function, "ERROR: expected { init | open | close | get | time | expose <msec> }" );
          error = ERROR;
        }
      }
      else
      if ( tokens.size() == 3 ) {
        if ( tokens[1] == "expose" ) {   // open the shutter for an integral number of msec, report measured exposure time
          int sl;
          try { retstring="bad exptime: expected integral number of msec"; sl = std::stoi( tokens[2] ); }
          catch ( std::invalid_argument & ) { return ERROR; } catch ( std::out_of_range & ) { return ERROR; }
          error  = this->camera.shutter.set_open();
          if ( error==NO_ERROR ) std::this_thread::sleep_for(std::chrono::milliseconds(sl));
          error |= this->camera.shutter.set_close();
          double el = this->camera.shutter.get_duration();
          retstring = ( error==NO_ERROR ? std::to_string( el ) : "NaN" );
        }
      }
      else {
        logwrite( function, "ERROR: expected { init | open | close | get | time | expose <msec> }" );
        error = ERROR;
      }
    } // end if (testname==shutter)
    else

    // ----------------------------------------------------
    // shdelay
    // ----------------------------------------------------
    // set shutter delay
    //
    if (testname == "shdelay") {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {                              // help
        retstring = CAMERAD_TEST;
        retstring.append( " shdelay [ <delay> | test ]\n" );
        retstring.append( "  Set/get the delay in whole milliseconds between the close of shutter\n" );
        retstring.append( "  and start of readout. If no argument supplied then the current delay is\n" );
        retstring.append( "  returned.\n" );
        retstring.append( "  Use \'test\' to spawn a thread which tests the delay by logging a message\n" );
        retstring.append( "  at the start and stop of the delay period.\n" );
        return HELP;
      }

      if ( tokens.size() > 1 ) {
        if ( tokens[1] == "test" ) {
          message.str(""); message << "TEST: spawning shutter timer test thread to check shdelay = " << this->camera.get_shutter_delay();
          logwrite( function, message.str() );
          std::thread( &AstroCam::Interface::dothread_test_shutter_timer, this, this->camera.get_shutter_delay() ).detach();
          retstring="running";
          return NO_ERROR;
        }
        else {
          error = this->camera.set_shutter_delay( tokens[1] );
        }
      }
      retstring = std::to_string( this->camera.get_shutter_delay() );
    }
    else

    // ----------------------------------------------------
    // pending
    // ----------------------------------------------------
    //
    if ( testname == "pending" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = CAMERAD_TEST;
        retstring.append( " pending\n" );
        retstring.append( "  returns which channels have an exposure pending and for each\n" );
        retstring.append( "  exposure buffer which devices have writes pending.\n\n" );
        error = HELP;
      }

      message.str(""); message << "camera.shutter.isclosed=" << (this->camera.shutter.isclosed()?"true":"false");
      logwrite( function, message.str() );
      retstring.append( message.str() ); retstring.append( "\n" );

      message.str(""); message << "this_expbuf=" << this->get_expbuf();
      logwrite( function, message.str() );
      retstring.append( message.str() ); retstring.append( "\n" );

      message.str(""); message << "is_camera_idle=" << ( this->is_camera_idle() ? "true" : "false" );
      logwrite( function, message.str() );
      retstring.append( message.str() ); retstring.append( "\n" );

      // this shows which channels have an exposure pending
      {
      std::vector<int> pending = this->exposure_pending_list();
      message.str(""); message << "exposures pending: ";
      for ( const auto &dev : pending ) message << this->controller[dev].channel << " ";
      logwrite( function, message.str() );
      }
      retstring.append( message.str() ); retstring.append( "\n" );

      message.str(""); message << "in readout: ";
      for ( const auto &dev : this->devnums ) if ( this->controller[dev].in_readout ) message << this->controller[dev].channel << " ";
      logwrite( function, message.str() );
      retstring.append( message.str() ); retstring.append( "\n" );

      message.str(""); message << "in frametransfer: ";
      for ( const auto &dev : this->devnums ) if ( this->controller[dev].in_frametransfer ) message << this->controller[dev].channel << " ";
      logwrite( function, message.str() );
      retstring.append( message.str() ); retstring.append( "\n" );

      // loop over all exposure buffers...
      //
      for ( int i=0; i<NUM_EXPBUF; i++ ) {
        message.str(""); message << "writes_pending ";
        std::vector<int> pending = this->writes_pending_list(i);  // internally mutex-protected
        message << i << ": ";

        // ...and for each exposure buffer, show which devices have writes pending
        //
        for ( const auto &wp : pending ) {
          message << wp << " ";
        }
        logwrite( function, message.str() );
        retstring.append( message.str() ); retstring.append( "\n" );
      }
      error = NO_ERROR;
    } // end if (testname==pending)
    else

    // ----------------------------------------------------
    // frametransfer
    // ----------------------------------------------------
    // run the frame transfer waveforms on the indicated device
    //
    if ( testname == "frametransfer" ) {

      if ( tokens.size()==1 || ( tokens.size() > 1 && tokens[1] == "?" ) ) {
        retstring = CAMERAD_TEST;
        retstring.append( " frametransfer <dev#> | <chan>\n" );
        retstring.append( "  Initiate the frame transfer waveforms on the indicated device.\n" );
        retstring.append( "  Supply dev# or chan from { " );
        message.str("");
        for ( const auto &dd : this->devnums ) {
          message << dd << " " << this->controller[dd].channel << " ";
        }
        if ( this->devnums.empty() ) message << "no_devices_open ";
        message << "}";
        retstring.append( message.str() );
        return HELP;
      }

      // must have at least one device open
      //
      if ( this->devnums.empty() ) {
        logwrite( function, "ERROR: no open devices" );
        retstring="no_devices";
        return( ERROR );
      }

      // check if arg is a channel by comparing to all the defined channels in the devnums
      //
      int dev=-1;
      for ( const auto &dd : this->devnums ) {
        if ( this->controller[dd].channel == tokens[1] ) {
          dev = dd;
          break;
        }
      }

      // not a channel so check if dev#
      //
      if ( dev < 0 && tokens.size()==2 ) {
        // try to convert to integer
        //
        try { dev = std::stoi( tokens.at(1) ); }
        catch ( std::invalid_argument & ) { logwrite( function, "ERROR: invalid argument" ); retstring="invalid_argument"; return ERROR; }
        catch ( std::out_of_range & ) { logwrite( function, "ERROR: out of range" ); retstring="exception"; return ERROR; }

        if ( this->controller.find(dev) == this->controller.end() ) {
          message.str(""); message << "ERROR: " << tokens[1] << " is neither a known channel nor device#";
          logwrite( function, message.str() );
          retstring="invalid_argument";
          return( ERROR );
        }
      }

      // initiate the frame transfer waveforms
      //
      this->controller[dev].pArcDev->frame_transfer( 0,
                                                     this->controller[dev].devnum,
                                                     this->controller[dev].info.axes[_ROW_],
                                                     this->controller[dev].info.axes[_COL_],
                                                     this->controller[dev].pCallback
                                                   );
      retstring=this->controller[dev].channel;
      return( NO_ERROR );
    }
    else

    // ----------------------------------------------------
    // controller
    // ----------------------------------------------------
    //
    if ( testname == "controller" ) {

      for ( auto &con : this->controller ) {
        if ( con.second.inactive ) continue;  // skip controllers flagged as inactive
        message.str(""); message << "controller[" << con.second.devnum << "] connected:" << ( con.second.connected ? "T" : "F" )
                                 << " bufsize:" << con.second.get_bufsize()
                                 << " rows:" << con.second.rows << " cols:" << con.second.cols
                                 << " in_readout:" << ( con.second.in_readout ? "T" : "F" )
                                 << " in_frametransfer:" << ( con.second.in_frametransfer ? "T" : "F" );
        logwrite( function, message.str() );
      }
      return( NO_ERROR );
    }
    else
    if ( testname == "monnotify" ) {
      this->state_monitor_condition.notify_all();
      return( NO_ERROR );
    }
    else
    if ( testname == "monthread" ) {
      std::thread( std::ref(AstroCam::Interface::state_monitor_thread), std::ref(*this) ).detach();
      return( NO_ERROR );
    }
    else
    // ----------------------------------------------------
    // telem
    // ----------------------------------------------------
    // test sending the telem command
    //
    if ( testname == "telem" ) {
      if ( tokens.size() < 2 ) {
        logwrite( function, "ERROR expected an argument" );
        retstring="invalid_argument";
        return ERROR;
      }

      if ( tokens[1] == "?" || tokens[1] == "help" ) {
        retstring = CAMERAD_TEST;
        retstring.append( " telem collect | test | calibd | flexured | focusd | tcsd\n" );
        retstring.append( "  collect   collects telemetry from all daemons\n" );
        retstring.append( "  test      sends a test JSON message back to myself (camerad)\n" );
        retstring.append( "  <xxx>     all other args collect telemetry from named daemon only\n" );
        return HELP;
      }

      if ( tokens[1] == "collect" ) {
        this->collect_telemetry();
        return NO_ERROR;
      }

      Common::DaemonClient jclient("", "\n", JEOF );

      if ( tokens[1]=="calibd" ) {
        jclient.set_name("calibd");
        jclient.set_port(9101);
        jclient.connect();
        jclient.command(TELEMREQUEST, retstring);
        jclient.disconnect();
      }
      else
      if ( tokens[1]=="flexured" ) {
        jclient.set_name("flexured");
        jclient.set_port(9103);
        jclient.connect();
        jclient.command(TELEMREQUEST, retstring);
        jclient.disconnect();
      }
      else
      if ( tokens[1]=="focusd" ) {
        jclient.set_name("focusd");
        jclient.set_port(9104);
        jclient.connect();
        jclient.command(TELEMREQUEST, retstring);
        jclient.disconnect();
      }
      else
      if ( tokens[1]=="tcsd" ) {
        jclient.set_name("tcsd");
        jclient.set_port(9107);
        jclient.connect();
        jclient.command(TELEMREQUEST, retstring);
        jclient.disconnect();
      }
      else
      if ( tokens[1]=="test" ) {
        nlohmann::json jmessage;
        jmessage["messagetype"] = "test";
        jmessage["test"]  = "Hello, world!";
        logwrite( function, "returning JSON test message" );
        retstring = jmessage.dump();
      }
      else {
        jclient.set_name("camerd");
        jclient.set_port(server.nbport);
        jclient.connect();
        jclient.command("test json test", retstring);
        jclient.disconnect();
      }
      this->handle_json_message( retstring );
    }
    else
    // ----------------------------------------------------
    // isreadout
    // ----------------------------------------------------
    // call ARC API isReadout() function directly
    //
    if ( testname == "isreadout" ) {
      error=ERROR;
      retstring.clear();
      try {
      for ( auto &con : this->controller ) {
        if ( con.second.pArcDev != nullptr && con.second.connected ) {
          bool isreadout = con.second.pArcDev->isReadout();
          error=NO_ERROR;
          retstring += (isreadout ? "T " : "F ");
          message.str(""); message << con.second.devname << " isReadout = " << (isreadout ? "true":"false");
          logwrite(function, message.str());
        }
      }
      if (error==ERROR) retstring="no_controllers";
      }
      catch (const std::exception &e) {
        logwrite(function, "ERROR: "+std::string(e.what()));
        return ERROR;
      }
    }
    else
    // ----------------------------------------------------
    // pixelcount
    // ----------------------------------------------------
    // call ARC API getPixelCount() function directly
    //
    if ( testname == "pixelcount" ) {
      error=ERROR;
      retstring="no_controllers";
      try {
      for ( auto &con : this->controller ) {
        if ( con.second.pArcDev != nullptr && con.second.connected ) {
          uint32_t pixelcount = con.second.pArcDev->getPixelCount();
          error=NO_ERROR;
          retstring = std::to_string(pixelcount);
          message.str(""); message << con.second.devname << " PixelCount = " << pixelcount;
          logwrite(function, message.str());
        }
      }
      }
      catch (const std::exception &e) {
        logwrite(function, "ERROR: "+std::string(e.what()));
        return ERROR;
      }
    }
    else {
    // ----------------------------------------------------
    // invalid test name
    // ----------------------------------------------------
    //
      message.str(""); message << "ERROR: test " << testname << " unknown";;
      logwrite(function, message.str());
      error = ERROR;
    }

    return error;
  }
  /**** AstroCam::Interface::test *********************************************/


  /**** AstroCam::Interface::dothread_test_shutter_timer **********************/
  /**
   * @brief      test thread for testing shutter timer
   * @details    This thread sleeps for the specified number of milliseconds
   *             using the shutter timer, a PreciseTimer object, logging before
   *             and after the sleep.
   * @param[in]  ms  number of milliseconds to sleep
   *
   */
  void Interface::dothread_test_shutter_timer(long ms) {
    logwrite( "AstroCam::Interface::dothread_test_shutter_timer", "TEST: starting timer" );
    this->camera.shutter_timer.delay( ms );
    logwrite( "AstroCam::Interface::dothread_test_shutter_timer", "TEST: ending timer" );
    return;
  }
  /**** AstroCam::Interface::dothread_test_shutter_timer **********************/


  /**** AstroCam::Interface::Controller::write ********************************/
  /**
   * @brief      OBSOLETE: wrapper to write a fits file
   * @details    This will call the write_image() member function of the
   *             FITS_file class, pointed to by pFits for this controller.
   * @return     ERROR or NO_ERROR
   *
   * called by Interface::write_frame( ) which is called by the handle_frame thread
   *
   * Since this is part of the Controller class and info is a member
   * object, we already know everything from this->info, including which
   * buffer to write, since that is a member of the Controller class, and there
   * is a vector of Controller objects, one for each device.
   *
   * That pointer is cast to the appropriate type here, based on this->info.datatype,
   * and then the FITS_file class write_image function is called with the correctly
   * typed buffer. write_image is a template class function so it just needs to
   * be called with a buffer of the appropriate type.
   *
   */
  long Interface::Controller::write() {
    std::string function = "AstroCam::Interface::Controller::write";
    logwrite( function, "ERROR: this function is obsolete!" );
    return( ERROR );
/*******************
    std::stringstream message;
    long retval;

if ( server.camera.get_abortstate() ) {
  logwrite(function, "* * * * * GOT ABORT * * * * * skipping write !");
  return( NO_ERROR );
}

#ifdef LOGLEVEL_DEBUG
    message << "[DEBUG] workbuf=" << std::hex << this->workbuf << " this->pFits=" << this->pFits;
    logwrite(function, message.str());
#endif

    // Cast the incoming void pointer and call pFits->write_image() 
    // based on the datatype.
    //
    switch (this->info.datatype) {
      case USHORT_IMG: {
        retval = this->pFits->write_image( (uint16_t *)this->workbuf, this->info);
        break;
      }
      case SHORT_IMG: {
        retval = this->pFits->write_image( (int16_t *)this->workbuf, this->info);
        break;
      }
      case FLOAT_IMG: {
        retval = this->pFits->write_image( (uint32_t *)this->workbuf, this->info);
        break;
      }
      default:
        message.str("");
        message << "ERROR: unknown datatype: " << this->info.datatype;
        logwrite(function, message.str());
        retval = ERROR;
        break;
    }
    return( retval );
*******************/
  }
  /**** AstroCam::Interface::Controller::write ********************************/


  /***** AstroCam::Interface::Controller::alloc_workbuf ***********************/
  /**
   * @brief      allocate workspace memory for deinterlacing
   * @return     ERROR or NO_ERROR
   *
   * This function calls an overloaded template class version with 
   * a generic pointer cast to the correct type.
   *
   */
  long Interface::Controller::alloc_workbuf() {
    std::string function = "AstroCam::Interface::Controller::alloc_workbuf";
    std::stringstream message;
    long retval = NO_ERROR;
    void* ptr=NULL;

message.str(""); message << "devnum=" << this->devnum; logwrite( function, message.str() );
message.str(""); message << "datatype=" << this->info.datatype; logwrite( function, message.str() );

    switch (this->info.datatype) {
      case USHORT_IMG: {
        this->alloc_workbuf( (uint16_t *)ptr );
        break;
      }
      case SHORT_IMG: {
        this->alloc_workbuf( (int16_t *)ptr );
        break;
      }
      case FLOAT_IMG: {
        this->alloc_workbuf( (uint32_t *)ptr );
        break;
      }
      default:
        message.str("");
        message << "ERROR: unknown datatype: " << this->info.datatype;
        logwrite(function, message.str());
        retval = ERROR;
        break;
    }
    return( retval );
  }
  /***** AstroCam::Interface::Controller::alloc_workbuf ***********************/


  /***** AstroCam::Interface::Controller::alloc_workbuf ***********************/
  /**
   * @brief      allocate workspace memory for deinterlacing
   * @param[in]  buf  pointer to template type T
   * @return     pointer to the allocated space
   *
   * The actual allocation occurs in here, based on the template class pointer type.
   *
   */
  template <class T>
  void* Interface::Controller::alloc_workbuf(T* buf) {
    std::string function = "AstroCam::Interface::Controller::alloc_workbuf";
    std::stringstream message;

    // Maybe the size of the existing buffer is already just right
    //
    if ( this->info.section_size == this->workbuf_size ) return( (void*)this->workbuf );

    // But if it's not, then free whatever space is allocated, ...
    //
    if ( this->workbuf != NULL ) this->free_workbuf(buf);

    // ...and then allocate new space.
    //
    this->workbuf = (T*) new T [ this->info.section_size ];
    this->workbuf_size = this->info.section_size;

    message << "allocated " << this->workbuf_size << " bytes for device " << this->devnum << " deinterlacing buffer " 
            << std::hex << this->workbuf;
    logwrite(function, message.str());
    return( (void*)this->workbuf );
  }
  /***** AstroCam::Interface::Controller::alloc_workbuf ***********************/


  /***** AstroCam::Interface::Controller::free_workbuf ************************/
  /**
   * @brief      free (delete) memory allocated by alloc_workbuf
   * @param[in]  buf  pointer to template type T
   *
   * Must pass a pointer of the correct type because delete doesn't work on void.
   *
   */
  template <class T>
  void Interface::Controller::free_workbuf(T* buf) {
    std::string function = "AstroCam::Interface::Controller::free_workbuf";
    std::stringstream message;
    if (this->workbuf != NULL) {
      delete [] (T*)this->workbuf;
      this->workbuf = NULL;
      this->workbuf_size = 0;
      message << "deleted old deinterlacing buffer " << std::hex << this->workbuf;
      logwrite(function, message.str());
    }
    return;
  }
  /***** AstroCam::Interface::Controller::free_workbuf ************************/

}
