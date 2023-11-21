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
#ifdef LOGLEVEL_DEBUG
    std::cerr << "elapsedtime: " << std::setw(10) << uiElapsedTime << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
#endif
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
    std::stringstream message;
    message << "PIXELCOUNT_" << devnum << ":" << uiPixelCount << " IMAGESIZE: " << uiImageSize;
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
#ifdef LOGLEVEL_DEBUG
    std::cerr << "pixelcount:  " << std::setw(10) << uiPixelCount << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
#endif
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
  void Callback::frameCallback( int expbuf, int devnum, std::uint32_t fpbcount, std::uint32_t fcount, std::uint32_t rows, std::uint32_t cols, void* buffer ) {
    if ( ! server.useframes ) fcount=1;  // when firmware doesn't support frames this prevents fcount from being a wild value
    std::stringstream message;

    if ( server.controller.find(devnum) == server.controller.end() ) {
      message.str(""); message << "ERROR in AstroCam::Callback::frameCallback (fatal): devnum " << devnum << " not in controller configuration";
      std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
      return;
    }

    message << "FRAMECOUNT_" << devnum << ":" << fcount << " rows=" << rows << " cols=" << cols;
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();

    server.add_framethread();            // framethread_count is incremented because a thread has been added

    std::thread( std::ref(AstroCam::Interface::handle_frame), expbuf, devnum, fpbcount, fcount, buffer ).detach();

#ifdef LOGLEVEL_DEBUG
    std::cerr << "pixelcount:  " << std::setw(10) << (rows*cols) << "\n";
    std::cerr << "framecount:  " << std::setw(10) << fcount << "\n";
#endif
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
    message << "FT_" << devnum << ":complete";
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();

    if ( server.controller.find(devnum) == server.controller.end() ) {
      message.str(""); message << "ERROR in AstroCam::Callback::ftCallback (fatal): devnum " << devnum << " not in controller configuration";
      std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
      return;
    }

    server.controller[devnum].in_frametransfer = false;
    server.exposure_pending( devnum, false );    // this also does the notify
    server.controller[devnum].in_readout       = true;

    // Trigger the readout waveforms here.
    //
    try {
      server.controller[devnum].pArcDev->readout( expbuf,
                                                  devnum,
                                                  server.controller[devnum].info.detector_pixels[0],
                                                  server.controller[devnum].info.detector_pixels[1],
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
    this->modeselected = false;
    this->numdev = 0;
    this->nframes = 1;
    this->nfpseq = 1;
    this->useframes = true;
    this->framethreadcount = 0;

    this->pFits.resize( NUM_EXPBUF );           // pre-allocate FITS_file object pointers for each exposure buffer

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
    this->readout_source.insert( { "U1",   { U1,      0x5f5531 } } );  // "_U1"
    this->readout_source.insert( { "L1",  { L1,     0x5f4c31 } } );  // "_L1"
    this->readout_source.insert( { "U2",   { U2,      0x5f5532 } } );  // "_U2"
    this->readout_source.insert( { "L2",  { L2,     0x5f4c32 } } );  // "_L2"
    this->readout_source.insert( { "SPLIT1",   { SPLIT1,      0x5f5f31 } } );  // "__1"
    this->readout_source.insert( { "SPLIT2",   { SPLIT2,      0x5f5f32 } } );  // "__2"
    this->readout_source.insert( { "QUAD",        { QUAD,           0x414c4c } } );  // "ALL"
    this->readout_source.insert( { "FT2",      { FT2,         0x465432 } } );  // "FT2" -- frame transfer from 1->2, read split2
    this->readout_source.insert( { "FT1",      { FT1,         0x465431 } } );  // "FT1" -- frame transfer from 2->1, read split1
//  this->readout_source.insert( { "hawaii1",     { HAWAII_1CH,     0xffffff } } );  ///< TODO @todo implement HxRG  1 channel deinterlacing
//  this->readout_source.insert( { "hawaii32",    { HAWAII_32CH,    0xffffff } } );  ///< TODO @todo implement HxRG 32 channel deinterlacing
//  this->readout_source.insert( { "hawaii32lr",  { HAWAII_32CH_LR, 0xffffff } } );  ///< TODO @todo implement HxRG 32 channel alternate left/right deinterlacing
  }
  /***** AstroCam::Interface::Interface ***************************************/


  /***** AstroCam::Interface::Interface ***************************************/
  /**
   * @brief      class deconstructor
   *
   */
  Interface::~Interface() {
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
    for ( auto source : this->readout_source ) {
      if ( source.first.compare( amp ) == 0 ) {     // found a match
        readout_valid = true;
        readout_arg  = source.second.readout_arg;   // get the arg associated with this match
        readout_type = source.second.readout_type;  // get the type associated with this match
      }
    }
    if ( !readout_valid || readout_type==-1 || readout_arg==0xBAD ) {
      message.str(""); message << "ERROR: bad READOUT " << amp << " for CHAN " << chan << ". Expected { ";
      for ( auto source : this->readout_source ) message << source.first << " ";
      message << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Create a vector of configured device numbers
    //
    this->configdev.push_back( dev );

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
    this->controller[dev].firmwareloaded = false;   // no firmware loaded

    this->controller[dev].info.readout_name = amp;
    this->controller[dev].info.readout_type = readout_type;
    this->controller[dev].readout_arg = readout_arg;

    this->exposure_pending( dev, false );
    this->controller[dev].in_readout = false;
    this->controller[dev].in_frametransfer = false;

    FITS_file* pFits = new FITS_file();             // create a pointer to a FITS_file class object
    this->controller[dev].pFits = pFits;            // set the pointer to this object in the public vector

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


  /***** AstroCam::Interface::do_connect_controller ***************************/
  /**
   * @brief      opens a connection to the PCI/e device(s)
   * @param[in]  devices_in  optional string containing space-delimited list of devices
   * @param[out] help        reference to string to return help on request
   * @return     ERROR or NO_ERROR
   *
   * Input parameter devices_in defaults to empty string which will attempt to
   * connect to all detected devices.
   *
   * If devices_in is specified (and not empty) then it must contain a space-delimited
   * list of device numbers to open. A public vector devlist will hold these device
   * numbers. This vector will be updated here to represent only the devices that
   * are actually connected.
   *
   * All devices requested must be connected in order to return success. It is
   * considered an error condition if not all requested can be connected. If the
   * user wishes to connect to only the device(s) available then the user must
   * call with the specific device(s). In other words, it's all (requested) or nothing.
   *
   */
  long Interface::do_connect_controller(std::string devices_in, std::string &help) {
    std::string function = "AstroCam::Interface::do_connect_controller";
    std::stringstream message;

    // Help
    //
    if ( devices_in == "?" ) {
      help = CAMERAD_OPEN;
      help.append( " [ <devlist> ]\n" );
      help.append( "  Opens a connection to the indicated PCI/e device(s) where <devlist>\n" );
      help.append( "  is an optional space-delimited list of device numbers.\n" );
      help.append( "  e.g. \"0 1\" to open PCI devices 0 and 1\n" );
      help.append( "  If no list is provided then all detected devices will be opened.\n" );
      help.append( "  Opening an ARC device requires that the controller is present and powered on.\n" );
      return( NO_ERROR );
    }

    // Find the installed devices
    //
    arc::gen3::CArcPCI::findDevices();

    this->numdev = arc::gen3::CArcPCI::deviceCount();

    // Nothing to do if there are no devices detected.
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR: no devices found");
      return(ERROR);
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
    if ( this->configdev.empty() ) {
      logwrite( function, "ERROR: no devices configured. Need CONTROLLER keyword in config file." );
      return( ERROR );
    }

    for ( auto dev : this->configdev ) {
      message.str(""); message << "device " << dev << " configured";
      logwrite( function, message.str() );
    }

    // Look at the requested device(s) to open, which are in the
    // space-delimited string devices_in. The devices to open
    // are stored in a public vector "devlist".
    //

    // If no string is given then use vector of configured devices
    //
    if ( devices_in.empty() ) this->devlist = this->configdev;

    // Otherwise, tokenize the device list string and build devlist from the tokens
    //
    else {
      std::vector<std::string> tokens;
      Tokenize(devices_in, tokens, " ");
      for ( auto n : tokens ) {                       // For each token in the devices_in string,
        try {
          int dev = std::stoi( n );                   // convert to int
          if ( std::find( this->devlist.begin(), this->devlist.end(), dev ) == this->devlist.end() ) { // If it's not already in the vector,
            this->devlist.push_back( dev );                                                            // then push into devlist vector.
          }
        }
        catch (std::invalid_argument &) {
          message.str(""); message << "ERROR: invalid device number: " << n << ": unable to convert to integer";
          logwrite(function, message.str());
          return(ERROR);
        }
        catch (std::out_of_range &) {
          message.str(""); message << "ERROR: device number " << n << ": out of integer range";
          logwrite(function, message.str());
          return(ERROR);
        }
        catch(...) { logwrite(function, "unknown error getting device number"); return(ERROR); }
      }
    }

    // For each requested dev in devlist, if there is a matching controller in the config file,
    // then get the devname and store it in the controller map.
    //
    for ( auto dev : this->devlist ) {
      if ( this->controller.find( dev ) != this->controller.end() ) {
        this->controller[ dev ].devname = devNames[dev];
      }
    }

    // The size of devlist at this point is the number of devices that will
    // be requested to be opened. This should match the number of opened
    // devices at the end of this function.
    //
    size_t requested_device_count = this->devlist.size();

    // Open only the devices specified by the devlist vector
    //
    for ( auto dev : this->devlist ) {

      auto dev_found = this->controller.find( dev );

      if ( dev_found == this->controller.end() ) {
        message.str(""); message << "ERROR: devnum " << dev << " not found in controller definition. check config file";
        logwrite( function, message.str() );
        this->do_disconnect_controller();
        return( ERROR );
      }

      try {
        // Open the PCI device if not already open
        // (otherwise just reset and test connection)
        //
        if ( ! this->controller[dev].connected ) {
          message.str(""); message << "opening " << this->controller[dev].devname; logwrite(function, message.str());
          this->controller[dev].pArcDev->open(dev);
        }
        else {
          message.str(""); message << this->controller[dev].devname << " already open"; logwrite(function, message.str());
        }

        // Reset the PCI device
        //
        message.str(""); message << "resetting " << this->controller[dev].devname; logwrite(function, message.str());
        this->controller[dev].pArcDev->reset();

        // Is Controller Connected?  (tested with a TDL command)
        //
        this->controller[dev].connected = this->controller[dev].pArcDev->isControllerConnected();
        message.str(""); message << this->controller[dev].devname << (this->controller[dev].connected ? "" : " not" ) << " connected to ARC controller"
                                 << (this->controller[dev].connected ? " for channel " : "" )
                                 << (this->controller[dev].connected ? this->controller[dev].channel : "" );
        logwrite(function, message.str());
      }
      catch ( const std::exception &e ) { // arc::gen3::CArcPCI::open and reset may throw exceptions
        message.str(""); message << "ERROR opening " << this->controller[dev].devname
                                 << " channel " << this->controller[dev].channel << ": " << e.what();
        this->camera.async.enqueue_and_log( function, message.str() );
        this->do_disconnect_controller();
        return( ERROR );
      }
    }

    // Update the devlist vector with connected controllers only
    //
    for ( auto dev : this->devlist ) { if ( ! this->controller[dev].connected ) this->devlist.at( dev ) = -1; }

    // Now remove the marked devices (those not connected) 
    // by erasing them from the this->devlist STL map.
    //
    this->devlist.erase( std::remove (this->devlist.begin(), this->devlist.end(), -1), this->devlist.end() );

    // Log the list of connected devices
    //
    message.str(""); message << "connected devices { ";
    for (auto devcheck : this->devlist) { message << devcheck << " "; } message << "}";
    logwrite(function, message.str());

    // check the size of the devlist now, against the size requested
    //
    if ( this->devlist.size() != requested_device_count ) {
      message.str(""); message << "ERROR: " << this->devlist.size() <<" connected device(s) but "
                               << requested_device_count << " requested";
      logwrite( function, message.str() );

      // disconnect/deconstruct everything --
      //
      // If the user might want to use what is available then the user
      // must call again, requesting only what is available. It is an
      // error if the interface cannot deliver what was requested.
      //
      this->do_disconnect_controller();

      return( ERROR );
    }

    // As the last step to opening the controller, this is where I've chosen
    // to initialize the Shutter class, required before using the shutter.
    //
    return( this->camera.bonn_shutter ? this->camera.shutter.init() : NO_ERROR );
  }
  /***** AstroCam::Interface::do_connect_controller ***************************/


  /***** AstroCam::Interface::disconnect_controller ***************************/
  /**
   * @brief      closes the connection to the PCI/e device(s)
   * @return     NO_ERROR
   *
   * no error handling
   *
   */
  long Interface::do_disconnect_controller() {
    std::string function = "AstroCam::Interface::do_disconnect_controller";
    std::stringstream message;

    // close all of the PCI devices
    //
    for ( auto &con : this->controller ) {
      message.str(""); message << "closing " << con.second.devname;
      logwrite(function, message.str());
      con.second.pArcDev->close();  // throws nothing, no error handling
      con.second.connected=false;
    }

    this->devlist.clear();   // no devices open
    this->numdev = 0;        // no devices open
    return NO_ERROR;         // always succeeds
  }
  /***** AstroCam::Interface::disconnect_controller ***************************/


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

    size_t ndev = this->devlist.size();  /// number of connected devices
    size_t nopen=0;                      /// number of open devices (should be equal to ndev if all are open)

    // look through all connected devices
    //
    for ( auto dev : this->devlist ) {
      if ( this->controller.find( dev ) != this->controller.end() ) nopen++;
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] " << this->controller[dev].devname << " is " << ( this->controller[dev].connected ? "connected" : "disconnected" );
        logwrite( function, message.str() );
#endif
    }

    // If all devices in (non-empty) devlist are connected then return true,
    // otherwise return false with a space-delimited list of the disconnected devices.
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
    std::string function = "AstroCam::Interface::do_configure_controller";
    std::stringstream message;
    int applied=0;
    long error = NO_ERROR;

    // Initialize the vector of configured device numbers,
    // which will get built up from parse_controller_config() below.
    //
    this->configdev.clear();

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      if (this->config.param[entry].compare(0, 10, "CONTROLLER")==0) {
        if ( this->parse_controller_config( this->config.arg[entry] ) != ERROR ) {
          message.str(""); message << "CAMERAD:config:" << this->config.param[entry] << "=" << this->config.arg[entry];
          this->camera.async.enqueue_and_log( function, message.str() );
          applied++;
        }
      }

      if (this->config.param[entry].compare(0, 5, "IMDIR")==0) {
        this->camera.imdir( config.arg[entry] );
        message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->camera.async.enqueue( message.str() );
        applied++;
      }

      if (config.param[entry].compare(0, 7, "DIRMODE")==0) {
        try {
          if ( !config.arg[entry].empty() ) {
            mode_t mode = (mode_t)std::stoi( config.arg[entry] );
            this->camera.set_dirmode( mode );
          }
        }
        catch (std::invalid_argument &) {
          this->camera.log_error( function, "unable to convert DIRMODE to integer" );
          return(ERROR);
        }
        catch (std::out_of_range &) {
          this->camera.log_error( function, "DIRMODE out of integer range" );
          return(ERROR);
        }
        message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->camera.async.enqueue( message.str() );
        applied++;
      }

      if (this->config.param[entry].compare(0, 6, "BASENAME")==0) {
        this->camera.basename( config.arg[entry] );
        message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->camera.async.enqueue( message.str() );
        applied++;
      }

      // If the Bonn shutter has been disabled ( = "no" ) then add a FITS keyword
      // to indicate that. If present, which is the normal operating condition,
      // then remove that keyword -- only need to flag this if non-standard condition.
      //
      if (this->config.param[entry].compare(0, 12, "BONN_SHUTTER")==0) {
        std::string bs = config.arg[entry];
        if ( !bs.empty() && bs=="no" ) {
          this->camera.bonn_shutter = false;
          this->camera_info.prikeys.addkey( "BONNSHUT", false, "Bonn shutter not in use" );
        }
        else
        if ( !bs.empty() && bs=="yes" ) {
          this->camera.bonn_shutter = true;
          this->camera_info.prikeys.delkey( "BONNSHUT" );
        }
        else {
          this->camera.log_error( function, "BONN_SHUTTER expected yes | no" );
          return( ERROR );
        }
        message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->camera.async.enqueue( message.str() );
        applied++;
      }

      // If an external shutter has been enabled ( = "yes" ) then add a FITS keyword
      // to indicate that. If not present, which is the normal operating condition,
      // then remove that keyword -- only need to flag this is in use.
      //
      if (this->config.param[entry].compare(0, 11, "EXT_SHUTTER")==0) {
        std::string es = config.arg[entry];
        if ( !es.empty() && es=="yes" ) {
          this->camera.ext_shutter = true;
          this->camera_info.prikeys.addkey( "EXT_SHUT", true, "external shutter trigger in use" );
        }
        else
        if ( !es.empty() && es=="no" ) {
          this->camera.ext_shutter = false;
          this->camera_info.prikeys.delkey( "EXT_SHUT" );
        }
        else {
          this->camera.log_error( function, "EXT_SHUTTER expected yes | no" );
          return( ERROR );
        }
        message.str(""); message << "CAMERAD:config:" << config.param[entry] << "=" << config.arg[entry];
        logwrite( function, message.str() );
        this->camera.async.enqueue( message.str() );
        applied++;
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
  /***** AstroCam::Interface::do_configure_controller *************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to the Leach controller
   * @param[in]  cmdstr  string containing command and arguments
   * @return     NO_ERROR on success, ERROR on error
   *
   */
  long Interface::do_native( std::string cmdstr ) {
    std::string retstring;
    std::vector<uint32_t> selectdev;
    for ( auto dev : this->devlist ) {
      selectdev.push_back( dev );                        // build selectdev vector from all connected controllers
    }
    return this->do_native( selectdev, cmdstr, retstring );
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to the Leach controller
   * @param[in]  selectdev  vector of devices to use
   * @param[in]  cmdstr     string containing command and arguments
   * @return     NO_ERROR on success, ERROR on error
   *
   */
  long Interface::do_native(std::vector<uint32_t> selectdev, std::string cmdstr) {
    std::string retstring;
    return this->do_native( selectdev, cmdstr, retstring );
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to the Leach controller
   * @param[in]  dev        individual device to use
   * @param[in]  cmdstr     string containing command and arguments
   * @param[out] retstring  reference to string to contain reply
   * @return     NO_ERROR on success, ERROR on error
   *
   */
  long Interface::do_native( int dev, std::string cmdstr, std::string &retstring ) {
    std::vector<uint32_t> selectdev;
    selectdev.push_back( dev );
    return this->do_native( selectdev, cmdstr, retstring );
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to the Leach controller
   * @param[in]  cmdstr     string containing command and arguments
   * @param[out] retstring  reference to string to contain reply
   * @return     NO_ERROR on success, ERROR on error
   *
   */
  long Interface::do_native( std::string cmdstr, std::string &retstring ) {
    std::vector<uint32_t> selectdev;
    for ( auto dev : this->devlist ) {
      selectdev.push_back( dev );                        // build selectdev vector from all connected controllers
    }
    return this->do_native( selectdev, cmdstr, retstring );
  }
  /***** AstroCam::Interface::do_native ***************************************/


  /***** AstroCam::Interface::do_native ***************************************/
  /**
   * @brief      send a 3-letter command to the Leach controller
   * @param[in]  selectdev  vector of devices to use
   * @param[in]  cmdstr     string containing command and arguments
   * @param[out] retstring  reference to string to contain reply
   * @return     NO_ERROR on success, ERROR on error
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
      return(ERROR);
    }

    // If no command passed then nothing to do here
    //
    if ( cmdstr.empty() || cmdstr == "?" ) {
      retstring = CAMERAD_NATIVE;
      retstring.append( " <CMD> [ <ARG1> [ < ARG2> [ <ARG3> [ <ARG4> ] ] ] ]\n" );
      retstring.append( "  send 3-letter command <CMD> with up to four optional args to all open ARC controllers\n" );
      retstring.append( "  Input <CMD> is not case-sensitive and any values default to base-10\n" );
      retstring.append( "  unless preceeded by 0x to indicate base-16 (e.g. rdm 0x400001).\n" );
      return( NO_ERROR );
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
      return(ERROR);
    }

    try {
      // first token is command and require a 3-letter command
      //
      if (tokens.at(0).length() != 3) {
        message.str(""); message << "ERROR: bad command " << tokens.at(0) << ": native command requires 3 letters";
        logwrite(function, message.str());
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
      for ( auto tok : tokens ) {
        cmd.push_back( (uint32_t)parse_val( tok ) );  // then push each converted arg into cmd vector
      }

      // Log the complete command (with arg list) that will be sent
      //
      message.str("");   message << "sending command:"
                                 << std::setfill('0') << std::setw(2) << std::hex << std::uppercase;
      for (auto arg : cmd) message << " 0x" << arg;
      logwrite(function, message.str());
    }
    catch(std::out_of_range &) {  // should be impossible but here for safety
      message.str(""); message << "ERROR: unable to parse command ";
      for (auto check : tokens) message << check << " ";
      message << ": out of range";
      logwrite(function, message.str());
      return(ERROR);
    }
    catch(const std::exception &e) {
      message.str(""); message << "ERROR forming command: " << e.what();
      logwrite(function, message.str());
      return(ERROR);
    }
    catch(...) { logwrite(function, "unknown error forming command"); return(ERROR); }

    // Send the command to each selected device via a separate thread
    //
    {                                       // start local scope for this stuff
    std::vector<std::thread> threads;       // local scope vector stores all of the threads created here
    for ( auto dev : selectdev ) {          // spawn a thread for each device in selectdev
      std::thread thr( std::ref(AstroCam::Interface::dothread_native), std::ref(this->controller[dev]), cmd );
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
      return(ERROR);
    }
    catch(...) { logwrite(function, "unknown error joining threads"); return(ERROR); }
    }                                       // end local scope cleans up this stuff

    // Check to see if all retvals are the same by comparing them all to the first.
    //
    std::uint32_t check_retval;
    try {
      check_retval = this->controller[selectdev.at(0)].retval;    // save the first one in the controller vector
    }
    catch(std::out_of_range &) {
      logwrite(function, "ERROR: no device found. Is the controller connected?");
      return(ERROR);
    }

    bool allsame = true;
    for ( auto dev : selectdev ) { if (this->controller[dev].retval != check_retval) { allsame = false; } }

    // If all the return values are equal then return only one value...
    //
    if ( allsame ) {
      this->retval_to_string( check_retval, retstring );        // this sets retstring = to_string( retval )
    }

    // ...but if any retval is different from another then we have to report each one.
    //
    else {
      std::stringstream rs;
      for ( auto dev : selectdev ) {
        this->retval_to_string( this->controller[dev].retval, retstring );          // this sets retstring = to_string( retval )
        rs << std::dec << this->controller[dev].devnum << ":" << retstring << " ";  // build up a stringstream of each controller's reply
      }
      retstring = rs.str();  // re-use retstring to contain all of the replies
    }

    // Log the return values
    ///< TODO @todo need a way to send these back to the calling function
    //
    for ( auto dev : selectdev ) {
      message.str(""); message << this->controller[dev].devname << " returns " << std::dec << this->controller[dev].retval
                               << " (0x" << std::hex << std::uppercase << this->controller[dev].retval << ")";
      logwrite(function, message.str());
    }
    return ( NO_ERROR );
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
    std::string function = "AstroCam::Interface::dothread_shutter";
    std::stringstream message;
    std::string timestring;
    timespec timenow;
    double mjd0, mjd1, mjd;

    // If configured, send a command to the ARC controller to open
    // the shutter. This is not connected to the shutter but can be
    // used to trigger an external light source for testing. This
    // is done first, before operating the Bonn shutter, so as not
    // to introduce any delay in the real shutter timing (and will
    // be closed after the real shutter has closed).
    //
    if ( interface.camera.ext_shutter ) interface.do_native( "OSH" );

    // open the Bonn shutter
    //
    interface.camera.shutter.set_open();

    // Log shutter open time
    //
    timenow = Time::getTimeNow();            // get the time NOW
    timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
    mjd0 = mjd_from( timenow );              // modified Julian date of start

    interface.camera_info.prikeys.addkey( "EXPSTART", timestring, "exposure start time" );
    interface.camera_info.prikeys.addkey( "MJD0", mjd0, "exposure start time (modified Julian Date)" );

    message.str(""); message << "NOTICE:shutter opened at " << timestring;
    interface.camera.async.enqueue_and_log( function, message.str() );

    // Here is the shutter timer
    //
    std::this_thread::sleep_for( std::chrono::milliseconds( interface.camera.exposure_time ) );

    // close the Bonn shutter
    //
    interface.camera.shutter.set_close();

    // Log shutter close time
    //
    timenow = Time::getTimeNow();            // get the time NOW
    timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
    mjd1 = mjd_from( timenow );              // modified Julian date of stop
    mjd = (mjd1+mjd0)/2.;                    // average mjd

    interface.camera_info.prikeys.addkey( "MJD1", mjd1, "exposure stop time (modified Julian Date)" );
    interface.camera_info.prikeys.addkey( "MJD", mjd, "average of MJD0 and MJD1" );

    message.str(""); message << "NOTICE:shutter closed at " << timestring;
    interface.camera.async.enqueue_and_log( function, message.str() );

    // Send external close shutter command, if configured.
    //
    if ( interface.camera.ext_shutter ) interface.do_native( "CSH" );

    interface.camera.shutter.condition.notify_all();  // notify waiting threads that the shutter has closed

    // Save shutter duration to keyword database
    //
    interface.camera_info.prikeys.addkey( "SHUTTIME", interface.camera.shutter.duration(), "actual shutter open time in msec", 3 );

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
    std::string function = "AstroCam::Interface::dothread_read";
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

    // Trigger the readout waveforms here.
    // A callback function triggered by the ARC API will perform the writing of frames to disk.
    //
    try {
      // set the exposure_pending flag for this controller
      //
      server.exposure_pending( con.devnum, true );
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] expbuf " << expbuf << " dev " << con.devnum << " chan " << con.channel << " exposure_pending=true";
      cam.async.enqueue_and_log( "CAMERAD", function, message.str() );
#endif

      // If this controller is not using Frame Transfer then start the readout waveforms.
      //
      if ( ! con.have_ft ) {
        con.in_readout = true;
        which_waveforms="readout";
        // Send the actual command to start the readout waveforms.
        // This API call will send the SRE command to trigger the readout.
        // The expbuf and devnum are passed in so that the Callback functions know
        // which exposure and device they belong to, respectively.
        //
        con.pArcDev->readout( expbuf,
                              con.devnum,
                              con.info.detector_pixels[0],
                              con.info.detector_pixels[1],
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
        which_waveforms="frame_transfer";
        // Send the actual command to start the frame transfer waveforms.
        // This API call will send the FRT command to trigger the frame transfer.
        // The expbuf and devnum are passed in so that the Callback functions know
        // which exposure and device they belong to, respectively.
        //
        con.pArcDev->frame_transfer( expbuf,
                                     con.devnum,
                                     con.info.detector_pixels[0],
                                     con.info.detector_pixels[1],
                                     con.pCallback
                                   );
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] pArcDev->frame transfer waveforms started on " << con.devname << " for exposure buffer " << expbuf;
        logwrite( function, message.str() );
#endif
      }

      // The system writes a few things in the header
      //
/*** 10/30/23 BOB
      con.pFits->add_key( true, "FIRMWARE", "STRING", con.firmware , "controller firmware" );
      con.pFits->add_key( true, "EXPSTART", "STRING", con.info.start_time, "exposure start time" );
***/

//    con.pFits->add_key( true, "READOUT", "STRING",  con.info.readout_name, "readout amplifier" );
      con.expinfo.at(expbuf).prikeys.addkey( "READOUT", con.expinfo.at(expbuf).readout_name, "readout amplifier" );

//    con.pFits->add_key( true, "DEVNUM", "INT",  std::to_string(con.devnum), "PCI device number" );
      con.expinfo.at(expbuf).prikeys.addkey( "DEVNUM", con.devnum, "PCI device number" );

//    con.pFits->add_key( true, "EXPTIME", "INT", std::to_string(cam.exposure_time), "exposure time in msec" );
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
  void Interface::dothread_native( Controller &con, std::vector<uint32_t> cmd ) {
    std::string function = "AstroCam::Interface::dothread_native";
    std::stringstream message;
    uint32_t command;

    try {
      if ( cmd.size() > 0 ) command = cmd.at(0);

      // ARC_API now uses an initialized_list object for the TIM_ID, command, and arguments.
      // The list object must be instantiated with a fixed size at compile time.
      //
      if (cmd.size() == 1) con.retval = con.pArcDev->command( { TIM_ID, cmd.at(0) } );
      else
      if (cmd.size() == 2) con.retval = con.pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1) } );
      else
      if (cmd.size() == 3) con.retval = con.pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2) } );
      else
      if (cmd.size() == 4) con.retval = con.pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2), cmd.at(3) } );
      else
      if (cmd.size() == 5) con.retval = con.pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2), cmd.at(3), cmd.at(4) } );
      else {
        message.str(""); message << "ERROR: invalid number of command arguments: " << cmd.size() << " (expecting 1,2,3,4,5)";
        logwrite(function, message.str());
        con.retval = 0x455252;
      }
    }
    catch(const std::runtime_error &e) {
      message.str(""); message << "ERROR sending 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
                                                     << command << " to " << con.devname << ": " << e.what();
      logwrite(function, message.str());
      con.retval = 0x455252;
      return;
    }
    catch(std::out_of_range &) {  // impossible
      logwrite(function, "ERROR: indexing command argument");
      con.retval = 0x455252;
      return;
    }
    catch(...) {
      message.str(""); message << "unknown error sending 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
                                                             << command << " to " << con.devname;
      logwrite(function, message.str());
      con.retval = 0x455252;
      return;
    }
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

    Tokenize(valstring, tokens, " ");

    if (tokens.size() != 2) {
      message.str(""); message << "error: expected 1 value but got " << tokens.size()-1;
      logwrite(function, message.str());
      return(ERROR);
    }
    for (auto dev : this->devlist) {        // spawn a thread for each device in devlist
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
/**
        fitskeystr.str(""); fitskeystr << "NFRAMES=" << this->nframes << "//number of frames";
        this->fitskey.set_fitskey(fitskeystr.str()); // TODO
**/

        int _framesize = rows * cols * sizeof(unsigned short);
        if (_framesize < 1) {
          message.str(""); message << "error: bad framesize: " << _framesize;
          logwrite(function, message.str());
          return (-1);
        }
        unsigned int _nfpb = (unsigned int)( this->get_bufsize() / _framesize );

        if ( (_nfpb < 1) ||
             ( (this->nframes > 1) &&
               (this->get_bufsize() < (int)(2*rows*cols*sizeof(unsigned short))) ) ) {
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
        for ( auto check : this->devlist ) message << check << " ";
        message << "}";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch(...) { logwrite(function, "unknown error creating fitsname for controller"); return(ERROR); }
    }

    return 0;
  }
  /***** AstroCam::Interface::nframes *****************************************/


  /***** AstroCam::Interface::dothread_monitor_exposure_pending ***************/
  /**
   * @brief      block on exposure_condition, notifies when ready for exposure
   * @details    This thread is spawned with each new exposure and broadcasts
   *             an async message announcing the state of exposure_pending.
   *             This is also where the exposure number gets incremented.
   * @param[in]  interface  reference to this->interface
   *
   */
  void Interface::dothread_monitor_exposure_pending( Interface &interface ) {
    std::string function = "AstroCam::Interface::dothread_monitor_exposure_pending";
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

    interface.camera.async.enqueue_and_log( function, "NOTICE:ready for next exposure" );
    interface.camera.async.enqueue( "CAMERAD:READY:true" );

    return;
  }
  /***** AstroCam::Interface::dothread_monitor_exposure_pending ***************/


  /***** AstroCam::Interface::do_expose ***************************************/
  /**
   * @brief      initiate an exposure
   * @param[in]  nseq_in  if set becomes the number of sequences
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::do_expose(std::string nseq_in) {
    std::string function = "AstroCam::Interface::do_expose";
    std::stringstream message;
    std::string nseqstr;
    std::string devstr;
    std::string _start_time;
    long error;
    int nseq;

    // check for valid exposure_time
    //
    if ( this->camera.exposure_time < 0 ) {
      message.str(""); message << "ERROR: exposure time is undefined";
      this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
      return( ERROR );
    }

    // Different controllers may have different rules about when to set and clear their
    // exposure_pending flag (depending on whether or not they support frame transfer), but
    // if an exposure is pending for any controller then do not start a new exposure.
    //
    if ( this->exposure_pending() ) {
      std::vector<int> pending = this->exposure_pending_list();
      message.str(""); message << "ERROR: cannot start new exposure while exposure is pending for chan";
      message << ( pending.size() > 1 ? "s " : " " );
      for ( auto dev : pending ) message << this->controller[dev].channel << " ";
      this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
      return(ERROR);
    }

    // This is the exposure buffer number for this exposure.
    // Lock it in now, before another exposure comes in.
    //
    int this_expbuf = this->get_expbuf();
#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] this_expbuf=" << this_expbuf;
    logwrite( function, message.str() );
#endif

    // For all connected devices...
    //
    for (auto dev : this->devlist) {
      try {

        // check image size
        //
        int rows = this->controller[dev].rows;
        int cols = this->controller[dev].cols;
        if (rows < 1 || cols < 1) {
          message.str(""); message << "ERROR: image size must be non-zero: rows=" << rows << " cols=" << cols;
          this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
          return(ERROR);
        }

        // check readout type
        //
        if ( this->controller[ dev ].info.readout_name.empty() ) {
          message.str(""); message << "ERROR: readout undefined";
          this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
          return( ERROR );
        }

        // check buffer size
        // need allocation for at least 2 frames if nframes is greater than 1
        //
        int bufsize = this->get_bufsize();
        if ( bufsize < (int)( (this->nframes>1?2:1) * rows * cols * sizeof(unsigned short) ) ) {  ///< TODO @todo type check bufsize: is it big enough?
          message.str(""); message << "ERROR: insufficient buffer size (" << bufsize
                                   << " bytes) for " << this->nframes << " frame" << (this->nframes==1?"":"s")
                                   << " of " << rows << " x " << cols << " pixels";
          logwrite(function, message.str());
          message.str(""); message << "minimum buffer size is " << (this->nframes>1?2:1) * rows * cols * sizeof(unsigned short) << " bytes";
          this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
          return(ERROR);
        }

        // Currently, astrocam doesn't use "modes" like Archon does -- do we need that?
        // For the time being, set a fixed bitpix here. Doesn't seem like a final
        // solution but need to set it someplace for now.
        //
        this->controller[dev].info.detector_pixels[0] = cols;
        this->controller[dev].info.detector_pixels[1] = rows;
        // ROI is the full detector
        this->controller[dev].info.region_of_interest[0] = 1;
        this->controller[dev].info.region_of_interest[1] = this->controller[dev].info.detector_pixels[0];
        this->controller[dev].info.region_of_interest[2] = 1;
        this->controller[dev].info.region_of_interest[3] = this->controller[dev].info.detector_pixels[1];
        // Binning factor (no binning)
        this->controller[dev].info.binning[0] = 1;
        this->controller[dev].info.binning[1] = 1;

        this->controller[dev].info.ismex = true;
        this->controller[dev].info.bitpix = 16;
        this->controller[dev].info.frame_type = Camera::FRAME_RAW;
        if ( this->controller[dev].info.set_axes() != NO_ERROR ) {
          message.str(""); message << "ERROR setting axes for device " << dev;
          this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
          return( ERROR );
        }


/**
        this->controller[dev].info.prikeys.keydb = this->prikeys.keydb;
        this->controller[dev].info.userkeys.keydb   = this->userkeys.keydb;

        this->controller[dev].expinfo.at(this_expbuf).prikeys.keydb = this->prikeys.keydb;
        this->controller[dev].expinfo.at(this_expbuf).userkeys.keydb   = this->userkeys.keydb;

**/
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for ( auto check : this->devlist ) message << check << " ";
        message << "}";
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return( ERROR );
      }
      catch(...) {
        message.str(""); message << "ERROR: unknown exception creating fitsname for controller";
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return(ERROR);
      }
    }

    // If nseq_in is not supplied then set nseq to 1
    //
    if ( nseq_in.empty() ) {
      nseqstr = "1";
      nseq=1;
    }
    else {                                                          // sequence argument passed in
      try {
        nseq = std::stoi( nseq_in );                                // test that nseq_in is an integer
        nseqstr = nseq_in;                                          // before trying to use it
      }
      catch ( std::invalid_argument & ) {
        message.str(""); message << "ERROR: unable to convert sequences: " << nseq_in << " to integer";
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return( ERROR );
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: sequences " << nseq_in << " outside integer range";
        this->camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
        return(ERROR);
      }
      // Set the extension = 0 for each controller
      //
      for ( auto dev : this->devlist ) { this->controller[ dev ].info.extension = 0; }
    }

    if ( nseq > 1 ) {
      message.str(""); message << "NOTICE: multiple exposures not currently supported";
      logwrite( function, message.str() );
      server.camera.async.enqueue( message.str() );
    }

    // Clear the abort flag for a new exposure, in case it was previously set
    //
    this->camera.set_abortstate( false );

    // Initialize framethread count -- this keeps track of threads
    // spawned by frameCallback, to make sure that all have been completed.
    //
    this->init_framethread_count();

    // Copy the userkeys database object into camera_info
    //
//  this->camera_info.userkeys.keydb = this->userkeys.keydb;

//  this->camera_info.start_time = get_timestamp( );                // current system time formatted as YYYY-MM-DDTHH:MM:SS.sss
//  _start_time = get_timestamp();                                  // format that time as YYYY-MM-DDTHH:MM:SS.sss

    // Set camera.fitstime (YYYYMMDDHHMMSS) used for filename
    // this may be a fraction of a second before the recorded EXPTIME but this
    // rounds to the nearest second and is only used for the filename. The
    // real shutter open time for the exposure start will be recorded by
    // the shutter thread.
    //
    this->camera.set_fitstime( get_timestamp() );

//  message.str(""); message << "EXPSTART=" << _start_time << "// exposure start time";
//  this->camera_info.prikeys.addkey( "EXPSTART", _start_time, "exposure start time" );

    if ( ( error = this->camera.get_fitsname( this->camera_info.fits_name ) ) != NO_ERROR ) {
      this->camera.async.enqueue_and_log( "CAMERAD", function, "ERROR: assembling fitsname" );
      return( error );
    }

    this->camera_info.prikeys.addkey( "FITSNAME", this->camera_info.fits_name, "this filename" );

    this->camera_info.prikeys.addkey( "EXPTIME", this->camera.exposure_time, "exposure time in msec" );

    // Spawn a thread to open and write FITS files as the frames come in
    //
    for ( auto dev : this->devlist ) this->write_pending( this_expbuf, dev, true );
    std::thread( std::ref(AstroCam::Interface::FITS_handler), this_expbuf, std::ref(*this) ).detach();

    // send CLR command from local scope since these strings won't be needed elsewere
    //
    { std::string cmd = "CLR";
      std::string retstr;
      error = this->do_native( cmd, retstr );  // send the clear command here
      if ( error != NO_ERROR ) {
        message.str(""); message << "ERROR clearing CCDs: " << retstr;
        logwrite( function, message.str() );
        return( error );
      }
    }
    logwrite( function, "cleared CCDs" );

    // Spawn a thread to operate the shutter if needed.
    //
    if ( this->camera.exposure_time > 0 ) {
      this->camera.shutter.arm();          // puts shutter into pending state to prevent potential race conditions
                                           // while waiting for shutter_state to transition from 1 -> 0
      std::thread( std::ref(AstroCam::Interface::dothread_shutter), this_expbuf, std::ref(*this) ).detach();
    }
    else {
      logwrite( function, "shutter not opened" );
      message.str(""); message << "NOTICE: incremented exposure buffer to " << server.get_expbuf(); logwrite( function, message.str() );
      // Save shutter duration = 0 to keyword database now, because dothread_shutter won't run
      this->camera_info.prikeys.addkey( "SHUTTIME", 0.0, "actual shutter open time in msec", 3 );
    }

    // Shutter or not, an exposure is now pending, so set the exposure_pending flag
    // and spawn a thread to monitor it, which will provide a notification
    // when ready for the next exposure.
    //
    for ( auto dev : this->devlist ) this->exposure_pending( dev, true );
    std::thread( std::ref(AstroCam::Interface::dothread_monitor_exposure_pending), std::ref(*this) ).detach();

    // prepare the camera info class object for each controller
    //
    for (auto dev : this->devlist) {        // spawn a thread for each device in devlist
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
        // If there are multiple devices in the devlist then force the fitsname to include the dev number
        // in order to make it unique for each device.
        //
        if ( this->devlist.size() > 1 ) {
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
                << " pCB="      << std::hex << this->controller[dev].pCallback
                << " pFits="    << std::hex << this->controller[dev].pFits;
        logwrite(function, message.str());
#endif
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for (auto check : this->devlist) message << check << " ";
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
    for (auto dev : this->devlist) {        // spawn a thread for each device in devlist
      try {
        // copy the info class from controller[dev] to controller[dev].expinfo[expbuf]  -- kludge for now
        //
        this->controller[dev].expinfo.at(this_expbuf) = this->controller[dev].info;

logwrite( function, "copying master keyword databases to expinfo now" );
        this->controller[dev].expinfo.at(this_expbuf).prikeys.keydb = this->camera_info.prikeys.keydb;
        this->controller[dev].expinfo.at(this_expbuf).userkeys.keydb   = this->camera_info.userkeys.keydb;

        this->controller[dev].expinfo.at(this_expbuf).fits_name="not_needed";

        std::string hash;
        md5_file( this->controller[dev].firmware, hash );                 // compute the md5 hash

        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "FIRM_MD5", hash, "MD5 checksum of firmware" );
        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "FIRMWARE", this->controller[dev].firmware, "" );  // no room for a comment
        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "DEV_ID", this->controller[dev].devnum, "detector controller device ID" );
        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "SPEC_ID", this->controller[dev].channel, "spectrograph channel" );
        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "CCD_ID", this->controller[dev].ccd_id, "CCD identifier" );
        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "AMP_ID", this->controller[dev].info.readout_name, "CCD readout amplifier ID" );
        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "FT", this->controller[dev].have_ft, "frame transfer used" );

        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "WCSNAME", "DISPLAY", "" );
        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "WCSNAMEA", "SPECTRUM", "" );
        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "CUNIT1A", "Angstrom", "" );
        this->controller[dev].expinfo.at(this_expbuf).extkeys.addkey( "CUNIT2A", "arcsec", "" );

/*** 10/30/23 *** BOB
        // Open the fits file, ...
        //
        error = this->controller[dev].open_file( this->camera.writekeys_when );
***/

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
        for (auto check : this->devlist) message << check << " ";
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

    for (auto dev : this->devlist) {
      message.str(""); 
      message << "** dev=" << dev << " type_set=" << this->controller[dev].info.type_set << " frame_type=" << this->controller[dev].info.frame_type
              << " detector_pixels[]=" << this->controller[dev].info.detector_pixels[0] << " " << this->controller[dev].info.detector_pixels[1]
              << " section_size=" << this->controller[dev].info.section_size << " image_memory=" << this->controller[dev].info.image_memory
              << " readout_name=" << this->controller[dev].info.readout_name 
              << " readout_name2=" << this->controller[dev].expinfo.at(this_expbuf).readout_name
              << " readout_type=" << this->controller[dev].info.readout_type
              << " axes[]=" << this->controller[dev].info.axes[0] << " " << this->controller[dev].info.axes[1] << " " << this->controller[dev].info.axes[2]
              << " cubedepth=" << this->controller[dev].info.cubedepth << " fitscubed=" << this->controller[dev].info.fitscubed
              << " binning=" << this->controller[dev].info.binning[0] << " " << this->controller[dev].info.binning[1]
              << " axis_pixels[]=" << this->controller[dev].info.axis_pixels[0] << " " << this->controller[dev].info.axis_pixels[1]
              << " ismex=" << this->controller[dev].info.ismex << " extension=" << this->controller[dev].info.extension;
      logwrite( function, message.str() );
    }

    for (auto dev : this->devlist) {
      for ( int ii=0; ii<NUM_EXPBUF; ii++ ) {
        message.str(""); 
        message << "** dev=" << dev << " expbuf=" << ii << " type_set=" << this->controller[dev].expinfo.at(ii).type_set << " frame_type=" << this->controller[dev].expinfo.at(ii).frame_type
                << " detector_pixels[]=" << this->controller[dev].expinfo.at(ii).detector_pixels[0] << " " << this->controller[dev].expinfo.at(ii).detector_pixels[1]
                << " section_size=" << this->controller[dev].expinfo.at(ii).section_size << " image_memory=" << this->controller[dev].expinfo.at(ii).image_memory
                << " readout_name=" << this->controller[dev].expinfo.at(ii).readout_name << " readout_type=" << this->controller[dev].expinfo.at(ii).readout_type
                << " axes[]=" << this->controller[dev].expinfo.at(ii).axes[0] << " " << this->controller[dev].expinfo.at(ii).axes[1] << " " << this->controller[dev].expinfo.at(ii).axes[2]
                << " cubedepth=" << this->controller[dev].expinfo.at(ii).cubedepth << " fitscubed=" << this->controller[dev].expinfo.at(ii).fitscubed
                << " binning=" << this->controller[dev].expinfo.at(ii).binning[0] << " " << this->controller[dev].expinfo.at(ii).binning[1]
                << " axis_pixels[]=" << this->controller[dev].expinfo.at(ii).axis_pixels[0] << " " << this->controller[dev].expinfo.at(ii).axis_pixels[1]
                << " ismex=" << this->controller[dev].expinfo.at(ii).ismex << " extension=" << this->controller[dev].expinfo.at(ii).extension;
        logwrite( function, message.str() );
      }
    }

    return( error );


    /// - - - - OLD - - - - ///


    // As frames are received, threads are created and a counter keeps track of them,
    // incrementing on creation and decrementing on destruction. Wait here for that
    // counter to be zero, indicating all the threads have completed.
    //
    while ( this->get_framethread_count() > 0 ) ;

    // Check each Controller object for errors that may have occured in the expose threads.
    // That error would have been logged, but not returned here, since the thread functions
    // are necessarily static void.
    //
    for (auto dev : this->devlist) {
      try {
        if ( ( error = this->controller[dev].error ) != NO_ERROR ) break;
      }
      catch(std::out_of_range &) {
      }
    }

    if ( error == NO_ERROR ) {
      this->camera.increment_imnum();                                 // increment image_num when fitsnaming == "number"
      logwrite(function, "all frames written");
    }
    else {
      logwrite( function, "ERROR one or more devices failed to start an exposure" );
//    if ( this->camera.get_abortstate() ) {
//      logwrite( function, "ABORTED" );
//      error = NO_ERROR;
//    }
//    else {
//      logwrite(function, "ERROR: writing image");
//    }
    }

    // close the FITS file
    //
    for (auto dev : this->devlist) {
      try {
        this->controller[dev].close_file( this->camera.writekeys_when );
        if ( this->controller[dev].pFits->iserror() ) error = ERROR;   // allow error to be set (but not cleared)
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR closing FITS file: unable to find device " << dev << " in list: { ";
        for (auto check : this->devlist) message << check << " ";
        message << "}";
        logwrite(function, message.str());
        error = ERROR;
      }
      catch(...) { logwrite(function, "unknown error closing FITS file(s)"); }
    }

    logwrite( function, (error==ERROR ? "ERROR" : "complete") );

    return( error );
  }
  /***** AstroCam::Interface::do_expose ***************************************/


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
      return(ERROR);
    }

    long error=NO_ERROR;

    // Loop through all of the configured controllers, build up a string in
    // the form "<dev> <filename>" to pass to the do_load_firmware() function
    // to load each controller with the specified file.
    //
    for ( auto &con : this->controller ) {
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
      return( ERROR );
    }

    // Can't have more tokens than the number of controllers + 1
    // The +1 is for the file name.
    //
    if ((int)tokens.size() > this->numdev+1) {
      logwrite(function, "ERROR: too many arguments");
      return( ERROR );
    }

    // If there's only one token then it's the lodfile and load
    // into all controllers in the devlist.
    //
    if (tokens.size() == 1) {
      for (auto dev : this->devlist) {
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
      for (auto dev : selectdev) {                    // spawn a thread for each device in the selectdev list
        if ( firstdev == -1 ) firstdev = dev;         // save the first device from the list of connected controllers
        try {
          if ( this->controller[dev].connected ) {    // but only if connected
            std::thread thr( std::ref(AstroCam::Interface::dothread_load), std::ref(this->controller[dev]), timlodfile );
            threads.push_back ( std::move(thr) );     // push the thread into the local vector
          }
        }
        catch(std::out_of_range &) {
          message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
          for (auto check : selectdev) message << check << " ";
          message << "}";
          logwrite(function, message.str());
          return(ERROR);
        }
        catch(const std::exception &e) {
          message.str(""); message << "ERROR threading command: " << e.what();
          logwrite(function, message.str());
          return(ERROR);
        }
        catch(...) { logwrite(function, "unknown error threading command to controller"); return(ERROR); }
      }

      try {
        for (std::thread & thr : threads) {           // loop through the vector of threads
          if ( thr.joinable() ) thr.join();           // if thread object is joinable then join to this function (not to each other)
        }
      }
      catch(const std::exception &e) {
        message.str(""); message << "ERROR joining threads: " << e.what();
        logwrite(function, message.str());
        return(ERROR);
      }
      catch(...) { logwrite(function, "unknown error joining threads"); return(ERROR); }

      threads.clear();                                // deconstruct the threads vector

      // Check to see if all retvals are the same by comparing them all to the first.
      //
      std::uint32_t check_retval;
      check_retval = this->controller[firstdev].retval;    // save the first one in the controller vector

      bool allsame = true;
      for ( auto dev : selectdev ) { if ( this->controller[dev].retval != check_retval ) { allsame = false; } }

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
        for (auto dev : selectdev) {
          this->retval_to_string( this->controller[dev].retval, rs );      // convert the retval to string (DON, ERR, etc.)
          rss << this->controller[dev].devnum << ":" << rs << " ";
        }
        retstring = rss.str();
        error = ERROR;
      }
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

      if ( !con.info.readout_name.empty() ) {  // Set readout amplifier if defined
        con.pArcDev->selectOutputSource( con.readout_arg );
      }

      std::string hash;
      md5_file( timlodfile, hash );                         // compute the md5 hash

      con.prikeys.addkey( "READOUT", con.info.readout_name, "readout amplifier" );
      con.prikeys.addkey( "CCD_ID", con.ccd_id, "CCD identifier" );
      con.prikeys.addkey( "FT", con.have_ft, "frame transfer used" );
      con.prikeys.addkey( "FIRMWARE", timlodfile, "" );     // no room for a comment
      con.prikeys.addkey( "FIRM_MD5", hash, "MD5 checksum of firmware" );
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
   * @brief      set/get mapped image buffer
   * @param[in]  size_in   string containing the buffer allocation info
   * @param[out] retstring reference to a string for return values
   * @return     ERROR or NO_ERROR
   *
   * This function uses the ARC API to allocate PCI/e buffer space for doing
   * the DMA transfers.
   *
   * Function returns only ERROR or NO_ERROR on error or success. The 
   * reference to retstring is used to return the size of the allocated
   * buffer.
   *
   */
  long Interface::buffer(std::string size_in, std::string &retstring) {
    std::string function = "AstroCam::Interface::buffer";
    std::stringstream message;
    uint32_t try_bufsize=0;

    // If no connected devices then nothing to do here
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR: no connected devices");
      return(ERROR);
    }

    // If no buffer size specified then return the current setting
    //
    if (size_in.empty()) {
      retstring = std::to_string( this->bufsize );
      return(NO_ERROR);
    }

    // Tokenize size_in string and set buffer size accordingly --
    // size_in string can contain 1 value to allocate that number of bytes
    // size_in string can contain 2 values to allocate a buffer of rows x cols
    // Anything other than 1 or 2 tokens is invalid.
    //
    std::vector<std::string> tokens;
    switch( Tokenize( size_in, tokens, " " ) ) {
      case 1:  // allocate specified number of bytes
               try_bufsize = (uint32_t)parse_val( tokens.at(0) );
               break;
      case 2:  // allocate (col x row) bytes
               try_bufsize = (uint32_t)parse_val(tokens.at(0)) * (uint32_t)parse_val(tokens.at(1)) * sizeof(uint16_t);
               break;
      default: // bad
               message.str(""); message << "ERROR: invalid arguments: " << size_in << ": expected bytes or cols rows";
               logwrite(function, message.str());
               return(ERROR);
               break;
    }

    // For each connected device, try to re-map the requested buffer size.
    // This is done sequentially for each device.
    //
    for (auto dev : this->devlist) {
      try {
        this->controller[dev].pArcDev->reMapCommonBuffer( try_bufsize );
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for (auto check : this->devlist) message << check << " ";
        message << "}";
        logwrite(function, message.str());
        return(ERROR);
      }
      catch(const std::exception &e) {
        message.str(""); message << "ERROR: device number " << dev << ": " << e.what();
        logwrite(function, message.str());
        return(ERROR);
      }
      catch(...) { logwrite(function, "unknown error mapping memory"); return(ERROR); }

      this->bufsize = try_bufsize;
    }
    retstring = std::to_string( this->bufsize );
    return(NO_ERROR);
  }
  /***** AstroCam::Interface::buffer ******************************************/


  /***** AstroCam::Interface::do_readout **************************************/
  /**
   * @brief      set or get type of readout
   * @param[in]  readout_in   string containing the requested readout type
   * @param[out] readout_out  reference to a string for return values
   * @return     ERROR or NO_ERROR
   *
   * This function sets (or gets) the type of readout. This will encompass
   * which amplifier(s) for a CCD (by name) or infrared (by number).
   * Selecting the readout will also set the appropriate deinterlacing
   * scheme to be used.
   *
   */
  long Interface::do_readout(std::string readout_in, std::string &readout_out) {
    std::string function = "AstroCam::Interface::do_readout";
    std::stringstream message;
    std::vector<std::string> tokens;
    long error = NO_ERROR;

    // Tokenize the args into a dev list and an arg list
    //
    std::vector<uint32_t> selectdev;
    std::vector<std::string> arglist;
    int ndev, narg;
    Tokenize( readout_in, selectdev, ndev, arglist, narg );

    if ( ndev < 0 ) {                       // Tokenize() sets ndev < 0 on error
      message.str(""); message << "ERROR: tokenizing device list from {" << readout_in << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    if ( ndev == 0 ) {                      // No device list, so
      for ( auto dev : this->devlist ) {    // build selectdev vector from all connected controllers.
        selectdev.push_back( dev );
      }
    }

    if ( selectdev.empty() ) {              // dev list will be empty if no connections open
      logwrite( function, "ERROR: no connected devices!" );
      return( ERROR );
    }

    if ( narg > 1 ) {                       // Too many arguments
      message.str(""); message << "ERROR: expected one argument for readout type but received " << narg << ": ";
      for ( auto arg : arglist ) message << arg << " ";
      logwrite( function, message.str() );
      error = ERROR;
    }

    std::string readout_name_req;           // requested readout source name
    uint32_t readout_arg;                   // argument associated with requested type
    ReadoutType readout_type;
    bool readout_name_valid = false;

    // Special case -- if requested a list then return a list of accepted readout amp names
    //
    if ( narg == 1 && arglist.front() == "list" ) {
      std::stringstream rs;
      for ( auto type : this->readout_source ) rs << type.first << " ";
      readout_out = rs.str();
      logwrite( function, rs.str() );
      return( NO_ERROR );
    }
    else

    // Otherwise, the argument is a requested readout type
    //
    if ( narg == 1 ) {
      readout_name_req = arglist.front();

      // Check that the requested readout amplifer has a matches in the list of known readout amps.
      // This list is an STL map. this->readout_source.first is the amplifier name,
      // and .second is the argument for the Arc 3-letter command.
      //
      for ( auto source : this->readout_source ) {
        if ( source.first.compare( readout_name_req ) == 0 ) {  // found a match
          readout_name_valid = true;
          readout_arg  = source.second.readout_arg;             // get the arg associated with this match
          readout_type = source.second.readout_type;            // get the type associated with this match
          break;
        }
      }
      if ( !readout_name_valid ) {
        message.str(""); message << "ERROR: readout " << readout_name_req << " not recognized";
        logwrite( function, message.str() );
        error = ERROR;
      }
      else {  // requested readout type is known, so set it for each of the specified devices
        for ( auto dev : selectdev ) {
          try {
            this->controller[ dev ].info.readout_name = readout_name_req;
            this->controller[ dev ].info.readout_type = readout_type;
            this->controller[ dev ].readout_arg = readout_arg;
            this->controller[ dev ].pArcDev->selectOutputSource( readout_arg );  // this sets the readout amplifier
          }
          catch ( std::out_of_range & ) {
            message.str(""); message << "ERROR: no active controller for device number " << dev;
            logwrite( function, message.str() );
            return( ERROR );
          }
          // Send the amplifier selection command to the connected controllers
          //
          std::stringstream cmd;
          std::string retstr;
          cmd.str(""); cmd << "SOS " << readout_arg;                ///< TODO @todo should this 3-letter command be generalized, or configurable?
          error = this->do_native( selectdev, cmd.str(), retstr );  // send the native command here
          if ( error != NO_ERROR || retstr == "ERR" ) {
            message.str(""); message << "ERROR setting output source 0x" << std::hex << std::uppercase << readout_arg << " for device " << dev;
            logwrite( function, message.str() );
            return( ERROR );
          }
        }
      }
    }

    std::stringstream rs;

    for ( auto dev : selectdev ) {
      std::string mytype;
      if ( this->controller[ dev ].connected ) mytype = this->controller[ dev ].info.readout_name;
      else {
        error = ERROR;
        mytype = "???";
      }
      rs << dev << ":" << mytype << " ";
    }

    message.str(""); message << "readout type " << rs.str();
    logwrite( function, message.str() );

    readout_out = rs.str();

    return( error );
  }
  /***** AstroCam::Interface::do_readout **************************************/


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
  long Interface::write_frame( int expbuf, int devnum, int fpbcount ) {
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
message.str(""); message << "about to write section size " << this->controller[devnum].expinfo.at(expbuf).section_size ; // << " to file \"" << this->pFits.at(expbuf)->fits_name << "\"";
logwrite(function, message.str());

          // Call write_image(),
          // passing a pointer to the workbuf for this controller (which has the deinterlaced image), and
          // a pointer for the info class for this exposure buf on this controller.
          //
          error = this->pFits.at( expbuf )->write_image( (uint16_t *)this->controller[ devnum ].workbuf, this->controller[ devnum ].expinfo.at(expbuf) );

          this->pFits.at( expbuf )->extension++;

message.str(""); message << this->controller[devnum].devname << " exposure buffer " << expbuf << " wrote " << std::hex << this->controller[devnum].workbuf;
logwrite(function, message.str());

//        error = this->controller[devnum].write( );  10/30/23 BOB -- the write is above. .write() called ->write_image(), skip that extra function
          break;
        }
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
      for (auto check : this->devlist) message << check << " ";
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
    int exptime_try=0;

    if (this->controller.size() < 1) {
      logwrite(function, "ERROR: controller not configured");
      return(ERROR);
    }

    // If an exposure time was passed in then
    // try to convert it (string) to an integer
    //
    if ( ! exptime_in.empty() ) {
      try {
        exptime_try = std::stoi( exptime_in );
      }
      catch ( std::invalid_argument & ) {
        message.str(""); message << "ERROR: unable to convert exposure time: " << exptime_in << " to integer";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: exposure time " << exptime_in << " outside integer range";
        logwrite( function, message.str() );
        return( ERROR );
      }

      for ( auto dev : this->devlist ) {
        // Send it to the controller via the SET command.
        //
        std::stringstream cmd;
        cmd << "SET " << exptime_try;
        error = this->do_native( cmd.str() );

        // Set the class variable if SET was successful
        //
        if ( error == NO_ERROR ) this->camera.exposure_time = exptime_try;
        this->controller[dev].info.exposure_unit = "msec";  // chaning unit not currently supported in ARC
      }
    }

    try { retstring = std::to_string( this->camera.exposure_time ); }
    catch ( std::bad_alloc &e ) {
      message.str(""); message << "ERROR: exposure time " << this->camera.exposure_time << " exception: " << e.what();
      logwrite( function, message.str() );
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
   * @return     ERROR or NO_ERROR
   *
   * This function assumes that all connected devices are using the same exposure time.
   *
   * Set exptime_in = -1 to end immediately.
   *
   */
  long Interface::do_modify_exptime( std::string exptime_in, std::string &retstring ) {
    std::string function = "AstroCam::Interface::do_modify_exptime";
    std::stringstream message;
    long error = NO_ERROR;
    long elapsed_time=0;
    long updated_exptime=0;
    long requested_exptime=0;

    // A requested exposure time must be specified
    //
    if ( exptime_in.empty() ) {
      logwrite( function, "ERROR: requested exposure time cannot be empty" );
      return( ERROR );
    }

    // Convert the requested exptime from string to long
    //
    try {
      requested_exptime = std::stol( exptime_in );
    }
    catch ( std::invalid_argument & ) {
      message.str(""); message << "ERROR: exception converting exposure time: " << exptime_in << " to long";
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range & ) {
      message.str(""); message << "ERROR: exception exposure time " << exptime_in << " outside long range";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Send command to connected devices to get remaining exposure time.
    //
    std::string reply;
    error = this->do_native( "RET", reply );

    // The reply can contain space-delimited responses from any number
    // of connected cameras, in the form of:
    //   "dev:value dev:value ... dev:value"
    //
    // where dev is the camera device number and value is a hex value, such as:
    //   "0:0x#### 1:0x#### ... 3:0x####"
    //
    // or if all cameras have the same value (unlikely here!) then the reply will be:
    //   "value"
    //
    // So first tokenize on the space character to get a single camera's response,
    //
    std::vector<std::string> tokens;
    Tokenize( reply, tokens, " " );
    if ( error==NO_ERROR && tokens.size() < 1 ) {
      message.str(""); message << "ERROR reply \"" << reply << "\": has not enough tokens";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Must have gotten at least one token from the reply
    //
    if ( (error==NO_ERROR) && (tokens.size() > 0) ) {
      reply = tokens.at(0);                            // set the reply to the first token,

      std::string::size_type pos = reply.find( ":" );  // then extract the value after the colon.

      // If there is only one token then the response was just "value" so convert that value from hex to uint msec
      //
      if ( tokens.size() == 1 ) {
        elapsed_time = parse_val( reply );
      }
      else

      // If there are 2 or more tokens then the response is expected to be of the form "dev:value dev:value ... dev:value"
      // If a colon was found then convert the portion of reply after the colon from a hex string to uint msec
      //
      if ( tokens.size() >= 2 && pos!=std::string::npos ) {
        elapsed_time = parse_val( reply.substr( pos + 1 ) );
      }

      // Otherwise something is wrong.
      //
      else {
        message.str(""); message << "ERROR malformed reply \"" << reply << "\". expected dev:value";
        logwrite( function, message.str() );
        error = ERROR;
      }
    }

    // block changes within the last 2 seconds of exposure
    //
    if ( (error==NO_ERROR) && ( (this->camera.exposure_time - elapsed_time) < 2000 ) ) {
      message.str(""); message << "ERROR cannot change exposure time with less than 2000 msec exptime remaining";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // check if requested exptime has already elapsed
    //
    if ( (error==NO_ERROR) && (requested_exptime >= 0) && (requested_exptime < elapsed_time) ) {
      message.str(""); message << "ERROR elapsed time " << elapsed_time << " already exceeds requested exposure time " << requested_exptime;
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Negative value requested exptime means to stop now (round up to the nearest whole sec plus one)
    //
    if ( (error==NO_ERROR) && (requested_exptime < 0 ) ) {
      updated_exptime = (long)( 1000 * std::ceil( 1.0 + (elapsed_time/1000.) ) );
    }

    // otherwise, just use the requested exposure time
    //
    if ( (error==NO_ERROR) && (requested_exptime > 0) ) {
      updated_exptime = requested_exptime;
    }

    // then send the command.
    //
    if ( error==NO_ERROR ) error = this->exptime( std::to_string( updated_exptime ), retstring );

    if ( error != NO_ERROR ) logwrite( function, "ERROR modifying exptime" );

    // If there is more than one space-delimited reply in retstring then not all cameras are the same
    // and that is a problem.
    //
    Tokenize( retstring, tokens, " " );
    if ( tokens.size() > 1 ) {
      message.str(""); message << "ERROR not all cameras returned the same value: " << retstring;
      logwrite( function, message.str() );
      error = ERROR;
    }

    if ( error==NO_ERROR ) {
      message.str(""); message << "successfully modified exptime to " << updated_exptime << " msec";
      logwrite( function, message.str() );
    }

    return( error );
  }
  /***** AstroCam::Interface::do_modify_exptime *******************************/


  /***** AstroCam::Interface::shutter *****************************************/
  /**
   * @brief      set or get the shutter enable state
   * @details    shutterenable will be passed to pArcDev->expose() which tells
   *             the ARC API whether or not to open the shutter on expose.
   * @param[in]  shutter_in   requested shutter state
   * @param[out] shutter_out  reference to string for return status
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::shutter(std::string shutter_in, std::string& shutter_out) {
    std::string function = "AstroCam::Interface::shutter";
    std::stringstream message;
    long error = NO_ERROR;
    bool shutten;
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
      shutter_out.append( "  A PCI connection must be opened to a controller before the enable\n" );
      shutter_out.append( "  state can be set or read.\n" );
      shutter_out.append( "\n" );
      shutter_out.append( "  See also \""+CAMERAD_TEST+" shutter\" for manual shutter operation.\n" );
      return( NO_ERROR );
    }

    // Process an input argument to set the shutter enable/disable state
    //
    if ( !shutter_in.empty() ) {
      try {
        std::transform( shutter_in.begin(), shutter_in.end(), shutter_in.begin(), ::tolower );  // make lowercase
        if ( shutter_in == "disable" || shutter_in == "0" ) shutten = false;
        else
        if ( shutter_in == "enable"  || shutter_in == "1" ) shutten = true;
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
      if ( error == NO_ERROR ) for ( auto dev : this->devlist ) {
        this->controller[dev].info.shutterenable = shutten;
      }
    }

    // Get shutterenable state from the controller class
    //
    for ( auto dev : this->devlist ) {
      this->camera_info.shutterenable = this->controller[dev].info.shutterenable;
      shutter_out = this->camera_info.shutterenable ? "enabled" : "disabled";
      break;  // just need one since they're all the same
    }

    // set the return value and report the state now, either setting or getting
    //
    message.str(""); message << "shutter is " << shutter_out;
    logwrite( function, message.str() );

    // Add the shutter enable keyword to the system keys db
    //
    this->camera_info.prikeys.addkey( "SHUTTEN", ( this->camera_info.shutterenable ? "T" : "F" ), "shutter was enabled" );

    return error;
  }
  /***** AstroCam::Interface::shutter *****************************************/


  /***** AstroCam::Interface::do_geometry *************************************/
  /**
   * @brief      set/get geometry
   * @param[in]  args       contains: X Y (i.e. cols rows)
   * @param[out] retstring  reference to string for return value
   * @return     ERROR or NO_ERROR
   *
   * args string can optionally contain a comma-separated list of devices,
   * separated from args by a colon, E.G.
   *   "dev: X Y"
   *   "dev,dev: X Y"
   *
   * or "dev:"
   *    "dev,dev:"
   * to read-only from dev or dev list
   *
   */
  long Interface::do_geometry(std::string args, std::string &retstring) {
    std::string function = "AstroCam::Interface::do_geometry";
    std::stringstream message;
    int _rows=-1;
    int _cols=-1;
    long error = NO_ERROR;

    // Tokenize the args into a dev list and an arg list
    //
    std::vector<uint32_t> selectdev;
    std::vector<std::string> arglist;
    int ndev, narg;
    Tokenize( args, selectdev, ndev, arglist, narg );

    // Tokenize sets ndev < 0 on error
    //
    if ( ndev < 0 ) {
      message.str(""); message << "ERROR: tokenizing device list from {" << args << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    if ( ndev == 0 ) {                      // No device list, so
      for ( auto dev : this->devlist ) {    // build selectdev vector from all connected controllers.
        selectdev.push_back( dev );
      }
    }

    if ( selectdev.empty() ) {              // dev list will be empty if no connections open
      logwrite( function, "ERROR: no connected devices!" );
      return( ERROR );
    }

    // If geometry was passed in then it's in the arglist.
    // Get each value and try to convert them to integers--
    //
    if ( narg == 2 ) {
      try {
        _cols = std::stoi( arglist.at( 0 ) );
        _rows = std::stoi( arglist.at( 1 ) );
      }
      catch ( std::invalid_argument & ) {
        message.str(""); message << "ERROR: unable to convert one or more values to integer: ";
        for ( auto arg : arglist ) message << arg << " ";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: one or more values outside range: ";
        for ( auto arg : arglist ) message << arg << " ";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch(...) { logwrite(function, "unknown error setting geometry"); return( ERROR ); }

      if ( _cols < 1 || _rows < 1 ) {
        logwrite( function, "ERROR: cols rows must be > 0" );
        return( ERROR );
      }

      // Write the geometry to the selected controllers
      //
      std::stringstream cmd;

      cmd.str(""); cmd << "WRM 0x400001 " << _cols;
      if ( error == NO_ERROR && this->do_native( selectdev, cmd.str() ) == ERROR ) error = ERROR;

      cmd.str(""); cmd << "WRM 0x400002 " << _rows;
      if ( error == NO_ERROR && this->do_native( selectdev, cmd.str() ) == ERROR ) error = ERROR;

      if (error == ERROR) logwrite(function, "ERROR: writing geometry to controller");
    }
    else if ( narg != 0 ) {                 // some other number of args besides 0 or 2 is confusing
      message.str(""); message << "ERROR: expected no args (for read) or 2 args (X Y for write) but received " << narg << " arguments";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Now read back the geometry from each controller
    //
    std::stringstream rs;

    for ( auto dev : selectdev ) {
      std::stringstream cmd;
      std::string col_s, row_s;

      try {
        // Return value from this->do_native() is of the form "dev:0xVALUE"
        // so must parse the hex value after the colon.
        //
        cmd.str(""); cmd << "RDM 0x400001 ";
        if ( error == NO_ERROR && this->do_native( dev, cmd.str(), col_s ) == ERROR ) error = ERROR;
        if ( error == NO_ERROR ) this->controller[dev].cols = (uint32_t)parse_val( col_s.substr( col_s.find(":")+1 ) );

        cmd.str(""); cmd << "RDM 0x400002 ";
        if ( error == NO_ERROR && this->do_native( dev, cmd.str(), row_s ) == ERROR ) error = ERROR;
        if ( error == NO_ERROR ) this->controller[dev].rows = (uint32_t)parse_val( row_s.substr( row_s.find(":")+1 ) );

        rs << dev << ":" << this->controller[dev].cols << " " << this->controller[dev].rows << " ";
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for ( auto check : selectdev ) message << check << " ";
        message << "}";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    if ( error == NO_ERROR ) retstring = rs.str();    // Form the return string from the read-back cols rows
    else {
      logwrite(function, "ERROR: reading geometry from controller");
    }

    return( error );
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
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] dev " << devnum << " chan " << server.controller[devnum].channel << " exposure_pending=false";
      server.camera.async.enqueue_and_log( "CAMERAD", function, message.str() );
#endif
    }
    server.controller[devnum].in_readout = false;
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
      error = server.write_frame( expbuf, devnum, fpbcount );
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
    try {
      interface.camera_info.datatype = USHORT_IMG;
      interface.camera_info.type_set = true;
      interface.camera_info.ismex = true;
      interface.pFits.at( expbuf ) = ( new FITS_file() );
      interface.pFits.at( expbuf )->extension=0;
      logwrite( function, "about to call pFits.at( expbuf )->open_file()" );
      long error = interface.pFits.at( expbuf )->open_file( true, interface.camera_info );
      if ( error==NO_ERROR ) {
        message.str(""); message << "opened \"" << interface.camera_info.fits_name << "\" returned " << error;
        logwrite( function, message.str() );
      }
      else {
        interface.pFits.at(expbuf)->close_file( true, interface.camera_info );
        message.str(""); message << "ERROR opening \"" << interface.camera_info.fits_name << "\". FITS handler exiting";
        logwrite( function, message.str() );
        return;
      }
    }
    catch ( std::out_of_range & ) {
      message.str(""); message << "ERROR: exposure buffer " << expbuf << " not in pFits vector";
      logwrite( function, message.str() );
      return;  // @TODO get this error back to do_expose? or readout?
    }

    // Block on write_condition until there are no writes pending
    // (writes_pending vector has zero size) which only happens when
    // all devices have written their frames for this exposure number.
    //
    while ( interface.writes_pending.at( expbuf ).size() > 0 ) {
      std::unique_lock<std::mutex> write_lock( interface.write_lock );

      message.str(""); message << "NOTICE:exposure buffer " << expbuf << " waiting for frames from ";
      std::vector<int> pending = interface.writes_pending.at( expbuf );
      for ( auto dev : pending ) message << interface.controller[dev].channel << " ";
      logwrite( function, message.str() );

      // wait() will repeatedly call this lambda function before actually entering
      // the waiting state, and if false, it won't wait and will continue. The
      // loop will exit only when the predicate returns true. wait() will also
      // automatically release the mutex lock temporarily to allow other threads
      // access the shared data.
      //
      interface.write_condition.wait( write_lock, [&] { return !(interface.writes_pending.at( expbuf ).size()>0); } );
    }

    message.str(""); message << "NOTICE:all frames written for exposure buffer " << expbuf;
    logwrite( function, message.str() );

    interface.pFits.at(expbuf)->close_file( true, interface.camera_info );

    return;
  }
  /***** AstroCam::Interface::FITS_handler ************************************/


  /***** AstroCam::Interface::add_framethread *********************************/
  /**
   * @brief      call on thread creation to increment framethreadcount
   *
   */
  inline void Interface::add_framethread() {
    this->framethreadcount_mutex.lock();
    this->framethreadcount++;
    this->framethreadcount_mutex.unlock();
  }
  /***** AstroCam::Interface::add_framethread *********************************/


  /***** AstroCam::Interface::remove_framethread ******************************/
  /**
   * @brief      call on thread destruction to decrement framethreadcount
   *
   */
  inline void Interface::remove_framethread() {
    this->framethreadcount_mutex.lock();
    this->framethreadcount--;
    this->framethreadcount_mutex.unlock();
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
    this->framethreadcount_mutex.lock();
    count = this->framethreadcount;
    this->framethreadcount_mutex.unlock();
    return( count );
  }
  /***** AstroCam::Interface::get_framethread *********************************/


  /***** AstroCam::Interface::init_framethread_count **************************/
  /**
   * @brief      initialize framethreadcount = 0
   *
   */
  inline void Interface::init_framethread_count() {
    this->framethreadcount_mutex.lock();
    this->framethreadcount = 0;
    this->framethreadcount_mutex.unlock();
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
    this->devnum = 0;
    this->framecount = 0;
    this->pArcDev = NULL;
    this->pCallback = NULL;
    this->connected = false;
    this->firmwareloaded = false;
    this->firmware = "";
    this->info.readout_name = "";
    this->info.readout_type = -1;
    this->readout_arg = 0xBAD;
    this->expinfo.resize( NUM_EXPBUF );  // vector of Camera::Information, one for each exposure buffer
  }
  /***** AstroCam::Interface::Controller::Controller **************************/


  /***** AstroCam::Interface::Controller::open_file ***************************/
  /**
   * @brief      wrapper to open the current fits file object
   * @param[in]  writekeys_in  string containing "before|after" for when to write fits keys relative to exposure
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::Controller::open_file( std::string writekeys_in ) {
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
  }
  /***** AstroCam::Interface::Controller::open_file ***************************/


  /***** AstroCam::Interface::Controller::close_file **************************/
  /**
   * @brief      wrapper to close the current fits file object
   * @param[in]  writekeys_in  string containing "before|after" for when to write fits keys relative to exposure
   *
   */
  void Interface::Controller::close_file( std::string writekeys_in ) {
    std::string function = "AstroCam::Interface::Controller::close_file";
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
  }
  /***** AstroCam::Interface::Controller::close_file **************************/


  /***** AstroCam::Interface::Controller::init_framecount *********************/
  /**
   * @brief      initialize this->framecount=0, protected by mutex
   *
   */
  inline void Interface::Controller::init_framecount() {
    server.framecount_mutex.lock();
    this->framecount = 0;
    server.framecount_mutex.unlock();
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
    server.framecount_mutex.lock();
    count = this->framecount;
    server.framecount_mutex.unlock();
    return( count );
  }
  /***** AstroCam::Interface::Controller::get_framecount **********************/


  /***** AstroCam::Interface::Controller::increment_framecount ****************/
  /**
   * @brief      increments this->framecount, protected by mutex
   *
   */
  inline void Interface::Controller::increment_framecount() {
    server.framecount_mutex.lock();
    this->framecount++;
    server.framecount_mutex.unlock();
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
   * @return     ERROR or NO_ERROR
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

    Tokenize(args, tokens, " ");

    if (tokens.size() < 1) {
      logwrite( function, "ERROR: no test name provided" );
      return ERROR;
    }

    std::string testname = tokens[0];                                // the first token is the test name

    // ----------------------------------------------------
    // fitsname
    // ----------------------------------------------------
    // Show what the fitsname will look like.
    // This is a "test" rather than a regular command so that it doesn't get mistaken
    // for returning a real, usable filename. When using fitsnaming=time, the filename
    // has to be generated at the moment the file is opened.
    //
    if (testname == "fitsname") {
      std::string msg;
      this->camera.set_fitstime( get_timestamp( ) );                 // must set camera.fitstime first
      if ( this->devlist.size() > 1 ) {
        for (auto dev : this->devlist) {
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

    // ----------------------------------------------------
    // async [message]
    // ----------------------------------------------------
    // queue an asynchronous message
    // The [message] param is optional. If not provided then "test" is queued.
    //
    else
    if (testname == "async") {
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

    // ----------------------------------------------------
    // bw <nseq>
    // ----------------------------------------------------
    // Bandwidth test
    // This tests the exposure sequence bandwidth by running a sequence
    // of exposures, including reading the frame buffer -- everything except
    // for the fits file writing.
    //
    else
    if (testname == "bw") {
      message.str(""); message << "ERROR: test " << testname << " not implemented";
      logwrite(function, message.str());
      error = ERROR;
    } // end if (testname==bw)

    // ----------------------------------------------------
    // shutter
    // ----------------------------------------------------
    //
    else
    if (testname == "shutter") {
      if ( tokens.size() == 2 ) {
        error = NO_ERROR;
        if ( tokens[1] == "open" ) {
          error  = this->camera.shutter.set_open();
          usleep( 150000 );
          error |= this->test( "shutter get", retstring );
        }
        else
        if ( tokens[1] == "close" ) {
          error  = this->camera.shutter.set_close();
          usleep( 150000 );
          error |= this->test( "shutter get", retstring );
        }
        else
        if ( tokens[1] == "init" ) {
          error = this->camera.shutter.init();
        }
        else
        if ( tokens[1] == "get" ) {
          int state;
          error = this->camera.shutter.get_state(state);
          switch( state ) {
            case 0:  retstring="closed";  break;
            case 1:  retstring="open";    break;
            default: retstring="unknown"; break;
          }
        }
        else
        if ( tokens[1] == "time" ) {
          double el = this->camera.shutter.duration();
          retstring = std::to_string( el );
        }
        else
        if ( tokens[1] == "?" ) {
          retstring = CAMERAD_TEST;
          retstring.append( " shutter init | open | close | get | time | expose <msec> \n" );
          retstring.append( "  init:           initializes Shutter class and opens USB device, required before use\n" );
          retstring.append( "  open:           manually open shutter now\n" );
          retstring.append( "  close:          manually close now\n" );
          retstring.append( "  get:            returns shutter state\n" );
          retstring.append( "  time:           returns the last shutter open/close time duration\n" );
          retstring.append( "  expose <msec>:  open shutter for integral <msec> milliseconds\n" );
          error = NO_ERROR;
        }
        else {
          logwrite( function, "ERROR: expected { init | open | close | get | time | expose <msec> }" );
          error = ERROR;
        }
      }
      else
      if ( tokens.size() == 3 ) {
        if ( tokens[1] == "expose" ) {
          int sl;
          try { retstring="bad exptime: expected integral number of msec"; sl = std::stoi( tokens[2] ); }
          catch ( std::invalid_argument & ) { return ERROR; } catch ( std::out_of_range & ) { return ERROR; }
          error  = this->camera.shutter.set_open();
          if ( error==NO_ERROR ) std::this_thread::sleep_for(std::chrono::milliseconds(sl));
          error |= this->camera.shutter.set_close();
          double el = this->camera.shutter.duration();
          retstring = ( error==NO_ERROR ? std::to_string( el ) : "NaN" );
        }
      }
      else {
        logwrite( function, "ERROR: expected { init | open | close | get | time | expose <msec> }" );
        error = ERROR;
      }
    } // end if (testname==shutter)

    else 
    if ( testname == "pending" ) {
      if ( tokens.size() > 1 && tokens[1] == "?" ) {
        retstring = CAMERAD_TEST;
        retstring.append( " pending\n" );
        retstring.append( "  returns which channels have an exposure pending and for each\n" );
        retstring.append( "  exposure buffer which devices have writes pending.\n\n" );
        error = NO_ERROR;
      }

      // this shows which channels have an exposure pending
      {
      std::vector<int> pending = this->exposure_pending_list();
      message.str(""); message << "exposures pending: ";
      for ( auto dev : pending ) message << this->controller[dev].channel << " ";
      logwrite( function, message.str() );
      }
      retstring.append( message.str() ); retstring.append( "\n" );

      // loop over all exposure buffers...
      //
      for ( int i=0; i<NUM_EXPBUF; i++ ) {
        message.str(""); message << "writes_pending ";
        std::vector<int> pending = this->writes_pending_list(i);  // internally mutex-protected
        message << i << ": ";

        // ...and for each exposure buffer, show which devices have writes pending
        //
        for ( auto wp : pending ) {
          message << wp << " ";
        }
        logwrite( function, message.str() );
        retstring.append( message.str() ); retstring.append( "\n" );
      }
      error = NO_ERROR;
    }

    // ----------------------------------------------------
    // invalid test name
    // ----------------------------------------------------
    //
    else {
      message.str(""); message << "ERROR: test " << testname << " unknown";;
      logwrite(function, message.str());
      error = ERROR;
    }

    return error;
  }
  /**** AstroCam::Interface::test *********************************************/


  /**** AstroCam::Interface::Controller::write ********************************/
  /**
   * @brief      wrapper to write a fits file
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
