#include "astrocam.h"

// The main server object is instantiated in src/server.cpp and
// defined extern here so that static functions can access it. The
// static functions are run in std::threads which means class objects
// are otherwise be unavailable to them. The interface class is accessible
// through this because Camera::Server server inherits AstroCam::Interface.
//
#include "camerad.h"
extern Camera::Server server;

namespace AstroCam {

  /** AstroCam::Callback::exposeCallback **************************************/
  /**
   * @fn     exposeCallback
   * @brief  called by CArcDevice::expose() during the exposure
   * @param  devnum, device number passed to API on expose
   * @param  uiElapsedTime, actually number of millisec remaining
   * @return none
   *
   * This is the callback function invoked by the ARC API,
   * arc::gen3::CArcDevice::expose() during the exposure.
   * After sending the expose (SEX) command, the API polls the controller
   * using the RET command.
   *
   */
  void Callback::exposeCallback( int devnum, std::uint32_t uiElapsedTime ) {
    std::string function = "AstroCam::Callback::exposeCallback";
    std::stringstream message;
    message << "ELAPSEDTIME_" << devnum << ":" << uiElapsedTime;
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
#ifdef LOGLEVEL_DEBUG
    std::cerr << "elapsedtime: " << std::setw(10) << uiElapsedTime << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
#endif
  }
  /** AstroCam::Callback::exposeCallback **************************************/


  /** AstroCam::Callback::readCallback ****************************************/
  /**
   * @fn     readCallback
   * @brief  called by CArcDevice::expose() during readout
   * @param  devnum, device number passed to API on expose
   * @param  uiPixelCount, number of pixels read ( getPixelCount() )
   * @return none
   *
   * This is the callback function invoked by the ARC API,
   * arc::gen3::CArcDevice::expose() when bInReadout is true.
   *
   */
  void Callback::readCallback( int devnum, std::uint32_t uiPixelCount ) {
    std::stringstream message;
    message << "PIXELCOUNT_" << devnum << ":" << uiPixelCount;
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
#ifdef LOGLEVEL_DEBUG
    std::cerr << "pixelcount:  " << std::setw(10) << uiPixelCount << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b";
#endif
  }
  /** AstroCam::Callback::readCallback ****************************************/


  /** AstroCam::Callback::frameCallback ***************************************/
  /**
   * @fn     frameCallback
   * @brief  called by CArcDevice::expose() when a frame has been received
   * @param  devnum, device number passed to API on expose
   * @param  fpbcount, frames per buffer count ( uiFPBCount ) wraps to 0 at FPB
   * @param  fcount, actual frame counter ( uiPCIFrameCount = getFrameCount() )
   * @param  rows, image rows ( uiRows )
   * @param  cols, image columns ( uiCols )
   * @param  buffer, pointer to PCI image buffer
   * @return none
   *
   * This is the callback function invoked by the ARC API,
   * arc::gen3::CArcDevice::expose() when a new frame is received.
   * Spawn a separate thread to handle the incoming frame.
   *
   */
  void Callback::frameCallback( int devnum, std::uint32_t fpbcount, std::uint32_t fcount, std::uint32_t rows, std::uint32_t cols, void* buffer ) {
    if ( ! server.useframes ) fcount=1;  // when firmware doesn't support frames this prevents fcount from being a wild value
    std::stringstream message;
    message << "FRAMECOUNT_" << devnum << ":" << fcount << " rows=" << rows << " cols=" << cols;
    std::thread( std::ref(AstroCam::Interface::handle_queue), message.str() ).detach();
    server.add_framethread();  // framethread_count is incremented because a thread has been added
    std::thread( std::ref(AstroCam::Interface::handle_frame), devnum, fpbcount, fcount, buffer ).detach();
#ifdef LOGLEVEL_DEBUG
    std::cerr << "pixelcount:  " << std::setw(10) << (rows*cols) << "\n";
    std::cerr << "framecount:  " << std::setw(10) << fcount << "\n";
#endif
  }
  /** AstroCam::Callback::frameCallback ***************************************/


  /** AstroCam::Interface::Interface ******************************************/
  /**
   * @fn     Interface
   * @brief  class constructor
   * @param  none
   * @return none
   *
   */
  Interface::Interface() {
    this->modeselected = false;
    this->numdev = 0;
    this->nframes = 1;
    this->nfpseq = 1;
    this->useframes = true;
    this->framethreadcount = 0;

    // Initialize STL map of Readout Amplifiers
    // Indexed by amplifier name.
    // The number is the argument for the Arc command to set this amplifier in the firmware.
    //
    this->readout_source.insert( { "U1",   { U1,      0x5f5531 } } );  // "_U1"
    this->readout_source.insert( { "L1",  { L1,     0x5f4c31 } } );  // "_L1"
    this->readout_source.insert( { "U2",   { U2,      0x5f5532 } } );  // "_U2"
    this->readout_source.insert( { "L2",  { L2,     0x5f4c32 } } );  // "_L2"
    this->readout_source.insert( { "SPLIT1",   { SPLIT1,      0x5f5f31 } } );  // "__1"
    this->readout_source.insert( { "SPLIT2",   { SPLIT2,      0x5f5f32 } } );  // "__2"
    this->readout_source.insert( { "QUAD",        { QUAD,           0x414c4c } } );  // "ALL"
    this->readout_source.insert( { "FT12S2",      { FT12S2,         0x313232 } } );  // "122" -- frame transfer from 1->2, read split2
    this->readout_source.insert( { "FT21S1",      { FT21S1,         0x323131 } } );  // "211" -- frame transfer from 2->1, read split1
//  this->readout_source.insert( { "hawaii1",     { HAWAII_1CH,     0xffffff } } );  // TODO HxRG  1 channel
//  this->readout_source.insert( { "hawaii32",    { HAWAII_32CH,    0xffffff } } );  // TODO HxRG 32 channel
//  this->readout_source.insert( { "hawaii32lr",  { HAWAII_32CH_LR, 0xffffff } } );  // TODO HxRG 32 channel alternate left/right
  }
  /** AstroCam::Interface::Interface ******************************************/


  /** AstroCam::Interface::Interface ******************************************/
  /**
   * @fn     ~Interface
   * @brief  class deconstructor
   * @param  none
   * @return none
   *
   */
  Interface::~Interface() {
  }
  /** AstroCam::Interface::Interface ******************************************/


  /** AstroCam::Interface::interface ******************************************/
  /**
   * @fn     interface
   * @brief  returns the interface
   * @param  reference to std::string iface, to return the interface as a string
   * @return NO_ERROR
   *
   */
  long Interface::interface(std::string &iface) {
    std::string function = "AstroCam::Interface::interface";
    iface = "AstroCam";
    logwrite(function, iface);
    return(NO_ERROR);
  }
  /** AstroCam::Interface::interface ******************************************/


  /** AstroCam::Interface::connect_controller *********************************/
  /**
   * @fn     connect_controller
   * @brief  opens a connection to the PCI/e device(s)
   * @param  devices_in, optional string containing space-delimited list of devices
   * @return ERROR or NO_ERROR
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
  long Interface::connect_controller(std::string devices_in="") {
    std::string function = "AstroCam::Interface::connect_controller";
    std::stringstream message;

    // Don't allow another open command --
    // Open creates a vector of objects and it's easier to manage by total
    // destruction and construction.
    //
    if (this->controller.size() > 0) {
      logwrite(function, "ERROR: controller connection already open.");
      return(ERROR);
    }

    // Find the installed devices
    //
    arc::gen3::CArcPCI::findDevices();

    this->numdev = arc::gen3::CArcPCI::deviceCount();

    message.str(""); message << "found " << this->numdev << " ARC device" << (this->numdev != 1 ? "s" : "");
    logwrite(function, message.str());

    // Nothing to do if there are no devices detected.
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR: no devices found");
      return(ERROR);
    }

    // Log all PCI devices found
    //
    const std::string* pdevList;
    pdevList = arc::gen3::CArcPCI::getDeviceStringList();
    for ( uint32_t i=0; i < arc::gen3::CArcPCI::deviceCount(); i++ ) {
      message.str(""); message << "found " << pdevList[ i ];
      logwrite(function, message.str());
    }

    // Look at the requested device(s) to open, which are in the
    // space-delimited string devices_in. The devices to open
    // are stored in a public vector "devlist".
    //

    // If no string is given then build devlist from all detected devices
    //
    if ( devices_in.empty() ) {
      for ( auto n = 0; n < this->numdev; n++ ) this->devlist.push_back( n );
    }

    // Otherwise, tokenize the device list string and build devlist from the tokens
    //
    else {
      std::vector<std::string> tokens;
      Tokenize(devices_in, tokens, " ");
      for ( auto n : tokens ) {                       // For each token in the devices_in string,
        try {
          this->devlist.push_back( std::stoi( n ) );  // convert it to int and push into devlist vector.
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

    // Create an object for each device in the system and store a pointer to each object in a vector
    //
    for (int dev = 0; dev < this->numdev; dev++) {
      try {
        // The Controller class holds the Common::Information class and the FITS_file class,
        // as well as wrappers for calling the functions inside the FITS_file class.
        // Then a vector of Controller class objects is created, with one element for each ARC device.
        // This is where that vector is pushed.
        //
        Controller con;                                    // create a Controller object
        this->controller.push_back( con );                 // push it into the vector
        this->controller.at(dev).devnum = dev;             // TODO is this useful?

        arc::gen3::CArcDevice* pArcDev = NULL;             // create a generic CArcDevice pointer
        pArcDev = new arc::gen3::CArcPCI();                // point this at a new CArcPCI() object  //TODO implement PCIe option
        Callback* pCB = new Callback();                    // create a pointer to a Callback() class object

        this->controller.at(dev).pArcDev = pArcDev;        // set the pointer to this object in the public vector
        this->controller.at(dev).pCallback = pCB;          // set the pointer to this object in the public vector
        this->controller.at(dev).devnum = dev;             // device number
        this->controller.at(dev).devname = pdevList[dev];  // device name
        this->controller.at(dev).connected = false;        // not yet connected
        this->controller.at(dev).firmwareloaded = false;   // no firmware loaded

        FITS_file* pFits = new FITS_file();                // create a pointer to a FITS_file class object
        this->controller.at(dev).pFits = pFits;            // set the pointer to this object in the public vector

#ifdef LOGLEVEL_DEBUG
        message.str("");
        message << "[DEBUG] pointers for dev " << dev << ": "
                << " pArcDev=" << std::hex << std::uppercase << this->controller.at(dev).pArcDev 
                << " pCB="     << std::hex << std::uppercase << this->controller.at(dev).pCallback
                << " pFits="   << std::hex << std::uppercase << this->controller.at(dev).pFits;
        logwrite(function, message.str());
#endif
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: creating controller object for device number " << dev << ": out of range";
        logwrite(function, message.str());
        this->disconnect_controller();
        return(ERROR);
      }
      catch(...) { logwrite(function, "unknown error creating controller object"); this->disconnect_controller(); return(ERROR); }
    }

    // The size of devlist at this point is the number of devices that will
    // be requested to be opened. This should match the number of opened
    // devices at the end of this function.
    //
    size_t requested_device_count = this->devlist.size();

    // Open only the devices specified by the devlist vector
    //
    for (auto dev : this->devlist) {
      try {
        // Open the PCI device
        //
        message.str(""); message << "opening device " << dev;
        logwrite(function, message.str());
        this->controller.at(dev).pArcDev->open(dev);

        // Reset the PCI device
        //
        message.str(""); message << "reset PCI board " << dev;
        logwrite(function, message.str());
        this->controller.at(dev).pArcDev->reset();

        // Is Controller Connected?  (tested with a TDL command)
        //
        this->controller.at(dev).connected = this->controller.at(dev).pArcDev->isControllerConnected();
        message.str(""); message << "controller " << dev << " connected = " << (this->controller.at(dev).connected ? "true" : "false");
        logwrite(function, message.str());
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to open  device number " << dev << " because it's not in the list: { ";
        for (auto devcheck : this->devlist) message << devcheck << " ";
        message << "}";
        logwrite(function, message.str());
        this->disconnect_controller();
        return(ERROR);
      }
      catch(const std::exception &e) {
        message.str(""); message << "ERROR: " << this->controller.at(dev).devname << ": " << e.what();
        logwrite(function, message.str());
        this->disconnect_controller();
        return (ERROR);
      }
      catch(...) {
        message.str(""); message << "unknown error connecting to " << this->controller.at(dev).devname;
        logwrite(function, message.str());
        this->disconnect_controller();
        return (ERROR);
      }
    }

    // Update the devlist vector with connected controllers only
    //
    for (auto dev : this->devlist) {
      try {
        if ( ! this->controller.at(dev).connected ) {      // If this devnum is not connected then
          this->devlist.at(dev) = -1;                      // mark it for removal.
        }
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: requested device number " << dev << " not in list: { ";
        for (auto devcheck : this->devlist) message << devcheck << " ";
        message << "}";
        logwrite(function, message.str());
        this->disconnect_controller();
        return(ERROR);
      }
      catch(...) { logwrite(function, "unknown error updaing device list"); this->disconnect_controller(); return(ERROR); }
    }

    // Now remove the marked devices (those not connected) 
    // by erasing them from the this->devlist STL map.
    //
    this->devlist.erase( std::remove (this->devlist.begin(), this->devlist.end(), -1), this->devlist.end() );

    // Log the list of connected devices
    //
    message.str(""); message << "connected devices : { ";
    for (auto devcheck : this->devlist) { message << devcheck << " "; } message << "}";
    logwrite(function, message.str());

    // check the size of the devlist now, against the size requested
    //
    if ( this->devlist.size() != requested_device_count ) {
      message.str(""); message << "ERROR: " << this->devlist.size() <<" connected devices but "
                               << requested_device_count << " requested";
      logwrite( function, message.str() );

      // disconnect/deconstruct everything --
      //
      // If the user might want to use what is available then the user
      // must call again, requesting only what is available. It is an
      // error if the interface cannot deliver what was requested.
      //
      this->disconnect_controller();

      return( ERROR );
    }
    else return( NO_ERROR );
  }
  /** AstroCam::Interface::connect_controller *********************************/


  /** AstroCam::Interface::disconnect_controller ******************************/
  /**
   * @fn     disconnect_controller
   * @brief  closes the connection to the PCI/e device(s)
   * @param  none
   * @return NO_ERROR
   *
   * no error handling
   *
   */
  long Interface::disconnect_controller() {
    std::string function = "AstroCam::Interface::disconnect_controller";
    std::stringstream message;

    // close all of the PCI devices
    //
    for (auto dev : this->controller) {
      message.str(""); message << "closing " << dev.devname;
      logwrite(function, message.str());
      dev.pArcDev->close();  // throws nothing, no error handling
    }

    // destruct the objects in the controller info vector and remove all elements
    //
    this->controller.clear();

    this->devlist.clear();   // no devices open
    this->numdev = 0;        // no devices open
    return NO_ERROR;         // always succeeds
  }
  /** AstroCam::Interface::disconnect_controller ******************************/


  /** AstroCam::Interface::is_connected ***************************************/
  /**
   * @fn         is_connected
   * @brief      are all selected controllers connected?
   * @param[out] retstring, reference to string to return true|false
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
      try {
        if ( this->controller.at(dev).connected ) nopen++;    // increment counter for each open device
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] dev " << dev << " is " << ( this->controller.at(dev).connected ? "connected" : "disconnected" );
        logwrite( function, message.str() );
#endif
      }
      catch( std::out_of_range & ) {
        message.str(""); message << "ERROR: requested device number " << dev << " not in list: { ";
        for ( auto devcheck : this->devlist ) message << devcheck << " ";
        message << "}";
        logwrite(function, message.str());
        return( ERROR );
      }
      catch(...) { logwrite(function, "unknown error accessing device list"); return( ERROR ); }
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
  /** AstroCam::Interface::is_connected ***************************************/


  /** AstroCam::Interface::configure_controller *******************************/
  /**
   * @fn     configure_controller
   * @brief  perform initial configuration of controller from .cfg file
   * @param  none
   * @return ERROR or NO_ERROR
   *
   * Called automatically by main() when the server starts up.
   *
   */
  long Interface::configure_controller() {
    std::string function = "AstroCam::Interface::configure_controller";
    std::stringstream message;
    int applied=0;
    long error = NO_ERROR;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < this->config.n_entries; entry++) {

      if (this->config.param[entry].compare(0, 16, "DEFAULT_FIRMWARE")==0) {
        // tokenize each firmware entry because it contains a controller dev number and a filename
        //
        std::vector<std::string> tokens;
        int size = Tokenize( this->config.arg[entry], tokens, " \t" );
        if (size != 2) {
          message << "ERROR: bad entry for DEFAULT_FIRMWARE: " << this->config.arg[entry] << ": expected (devnum filename)";
          logwrite(function, message.str());
          error = ERROR;
          continue;
        }
        else {
          try {
            this->common.firmware[ parse_val(tokens.at(0)) ] = tokens.at(1);
            applied++;
          }
          catch(std::out_of_range &) {  // should be impossible but here for safety
            logwrite(function, "ERROR configuring DEFAULT_FIRMWARE: requested tokens out of range");
            error = ERROR;
          }
        }
      }

      if (this->config.param[entry].compare(0, 5, "IMDIR")==0) {
        this->common.imdir( config.arg[entry] );
        applied++;
      }

      if (this->config.param[entry].compare(0, 6, "BASENAME")==0) {
        this->common.basename( config.arg[entry] );
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
  /** AstroCam::Interface::configure_controller *******************************/


  /** AstroCam::Interface::native *********************************************/
  /**
   * @fn     native
   * @brief  send a 3-letter command to the Leach controller
   * @param  cmdstr, string containing command and arguments
   * @return NO_ERROR on success, ERROR on error
   *
   */
  long Interface::native(std::string cmdstr) {
    std::string retstring;
    std::vector<uint32_t> selectdev;
    for ( auto dev : this->devlist ) {
      selectdev.push_back( dev );                        // build selectdev vector from all connected controllers
    }
    return this->native( selectdev, cmdstr, retstring );
  }
  long Interface::native(std::vector<uint32_t> selectdev, std::string cmdstr) {
    std::string retstring;
    return this->native( selectdev, cmdstr, retstring );
  }
  long Interface::native( int dev, std::string cmdstr, std::string &retstring ) {
    std::vector<uint32_t> selectdev;
    selectdev.push_back( dev );
    return this->native( selectdev, cmdstr, retstring );
  }
  long Interface::native( std::string cmdstr, std::string &retstring ) {
    std::vector<uint32_t> selectdev;
    for ( auto dev : this->devlist ) {
      selectdev.push_back( dev );                        // build selectdev vector from all connected controllers
    }
    return this->native( selectdev, cmdstr, retstring );
  }
  long Interface::native( std::vector<uint32_t> selectdev, std::string cmdstr, std::string &retstring ) {
    std::string function = "AstroCam::Interface::native";
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
    if ( cmdstr.empty() ) {
      logwrite(function, "ERROR: missing command");
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
    {  // begin local scope
    std::vector<std::thread> threads;       // create a local scope vector for the threads
    for ( auto dev : selectdev ) {          // spawn a thread for each device in selectdev
      try {
        std::thread thr( std::ref(AstroCam::Interface::dothread_native), std::ref(this->controller.at(dev)), cmd );
        threads.push_back(std::move(thr));  // push the thread into a vector
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for ( auto check : this->devlist ) message << check << " ";
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

    threads.clear();                        // deconstruct the threads vector
    }  // end local scope

    // Check to see if all retvals are the same by comparing them all to the first.
    //
    std::uint32_t check_retval;
    try {
      check_retval = this->controller.at(0).retval;    // save the first one in the controller vector
    }
    catch(std::out_of_range &) {
      logwrite(function, "ERROR: no device found. Is the controller connected?");
      return(ERROR);
    }

    bool allsame = true;
    for ( auto dev : selectdev ) {
      try {
        if (this->controller.at(dev).retval != check_retval) {
          allsame = false;
        }
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for ( auto check : this->devlist ) message << check << " ";
        message << "}";
        logwrite(function, message.str());
        return(ERROR);
      }
      catch(...) { logwrite(function, "unknown error looking for return values"); return(ERROR); }
    }

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
        try {
          this->retval_to_string( this->controller.at(dev).retval, retstring );          // this sets retstring = to_string( retval )
          rs << std::dec << this->controller.at(dev).devnum << ":" << retstring << " ";  // build up a stringstream of each controller's reply
        }
        catch(std::out_of_range &) {
          message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
          for ( auto check : this->devlist ) message << check << " ";
          message << "}";
          logwrite(function, message.str());
          return(ERROR);
        }
        catch(...) { logwrite(function, "unknown error looking for return values"); return(ERROR); }
      }
      retstring = rs.str();  // re-use retstring to contain all of the replies
    }

    // Log the return values
    // TODO need a way to send these back to the calling function
    //
    for ( auto dev : selectdev ) {
      try {
        message.str(""); message << this->controller.at(dev).devname << " returns " << std::dec << this->controller.at(dev).retval
                                 << " (0x" << std::hex << std::uppercase << this->controller.at(dev).retval << ")";
        logwrite(function, message.str());
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for ( auto check : this->devlist ) message << check << " ";
        message << "}";
        logwrite(function, message.str());
        return(ERROR);
      }
      catch(...) { logwrite(function, "unknown error looking for return values"); return(ERROR); }
    }
    return ( NO_ERROR );
  }
  /** AstroCam::Interface::native *********************************************/


  /** AstroCam::Interface::retval_to_string ***********************************/
  /**
   * @fn     retval_to_string
   * @brief  private function to convert ARC return values (long) to string
   * @param  retval, input long int retval
   * @param  retstring, reference to a string for return value
   * @return none
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
  }
  /** AstroCam::Interface::retval_to_string ***********************************/


  /** AstroCam::Interface::dothread_expose ************************************/
  /**
   * @fn     dothread_expose
   * @brief  run in a thread to actually send the command
   * @param  _controller, reference to Controller class object
   * @return none
   *
   */
  void Interface::dothread_expose( Controller &_controller ) {
    std::string function = "AstroCam::Interface::dothread_expose";
    std::stringstream message;

#ifdef LOGLEVEL_DEBUG
    message.str("");
    message << "[DEBUG] _controller.devnum=" << _controller.devnum 
            << " .devname=" << _controller.devname << " _controller.section_size=" << _controller.info.section_size
            << " shutterenable=" << _controller.info.shutterenable;
    logwrite(function, message.str());
#endif

    // Start the exposure here.
    // A callback function triggered by the ARC API will perform the writing of frames to disk.
    //
    try {
      // get system time just before the actual expose() call
      //
      _controller.info.start_time = get_timestamp( );  // current system time formatted as YYYY-MM-DDTHH:MM:SS.sss

      // Send the actual command to start the exposure.
      // This API call will send the SEX command to trigger the exposure.
      // The devnum is passed in so that the Callback functions know which device they belong to.
      //
      _controller.pArcDev->expose(_controller.devnum, 
                                  _controller.info.exposure_time, 
                                  _controller.info.detector_pixels[0], 
                                  _controller.info.detector_pixels[1], 
                                  server.common.abortstate, 
                                  _controller.pCallback, 
                                  _controller.info.shutterenable);
#ifdef LOGLEVEL_DEBUG
      message.str(""); message << "[DEBUG] pArcDev->expose started on " << _controller.devname;
      logwrite( function, message.str() );
#endif

      // The system writes a few things in the header
      //
      _controller.pFits->add_key("EXPSTART", "STRING", _controller.info.start_time, "exposure start time");
      _controller.pFits->add_key("READOUT", "STRING",  _controller.info.readout_name, "readout amplifier");
    }
    catch (const std::exception &e) {
      // arc::gen3::CArcDevice::expose() will throw an exception for an abort.
      // Look for the word "abort" in the exception message and log "ABORT"
      // instead of "ERROR".
      //
      std::string estring = e.what();
      message.str("");
      if ( estring.find("aborted") != std::string::npos ) {
        message << "ABORT on " << _controller.devname << ": " << e.what();
      }
      else {
        message << "ERROR on " << _controller.devname << ": " << e.what();
      }
      server.common.set_abortstate( true );
      logwrite(function, message.str());
      _controller.error = ERROR;
      return;
    }
    catch(...) {
      message.str(""); message << "unknown error calling pArcDev->expose() on " << _controller.devname << ". forcing abort.";
      logwrite(function, message.str() );
      server.common.set_abortstate( true );
      _controller.error = ERROR;
      return;
    }
    _controller.error = NO_ERROR;
    return;
  }
  /** AstroCam::Interface::dothread_expose ************************************/


  /** AstroCam::Interface::dothread_native ************************************/
  /**
   * @fn     dothread_native
   * @brief  run in a thread to actually send the command
   * @param  controller, reference to Controller object
   * @param  cmd, vector containing command and args
   * @return none
   *
   */
  void Interface::dothread_native( Controller &controller, std::vector<uint32_t> cmd ) {
    std::string function = "AstroCam::Interface::dothread_native";
    std::stringstream message;
    uint32_t command;

    try {
      if ( cmd.size() > 0 ) command = cmd.at(0);

      // ARC_API now uses an initialized_list object for the TIM_ID, command, and arguments.
      // The list object must be instantiated with a fixed size at compile time.
      //
      if (cmd.size() == 1) controller.retval = controller.pArcDev->command( { TIM_ID, cmd.at(0) } );
      else
      if (cmd.size() == 2) controller.retval = controller.pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1) } );
      else
      if (cmd.size() == 3) controller.retval = controller.pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2) } );
      else
      if (cmd.size() == 4) controller.retval = controller.pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2), cmd.at(3) } );
      else
      if (cmd.size() == 5) controller.retval = controller.pArcDev->command( { TIM_ID, cmd.at(0), cmd.at(1), cmd.at(2), cmd.at(3), cmd.at(4) } );
      else {
        message.str(""); message << "ERROR: invalid number of command arguments: " << cmd.size() << " (expecting 1,2,3,4,5)";
        logwrite(function, message.str());
        controller.retval = 0x455252;
      }
    }
    catch(const std::runtime_error &e) {
      message.str(""); message << "ERROR sending 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
                                                     << command << " to " << controller.devname << ": " << e.what();
      logwrite(function, message.str());
      controller.retval = 0x455252;
      return;
    }
    catch(std::out_of_range &) {  // impossible
      logwrite(function, "ERROR: indexing command argument");
      controller.retval = 0x455252;
      return;
    }
    catch(...) {
      message.str(""); message << "unknown error sending 0x" << std::setfill('0') << std::setw(2) << std::hex << std::uppercase
                                                             << command << " to " << controller.devname;
      logwrite(function, message.str());
      controller.retval = 0x455252;
      return;
    }
  }
  /** AstroCam::Interface::dothread_native ************************************/


  /** AstroCam::Interface::access_useframes ***********************************/
  /**
   * @fn     access_useframes
   * @brief  set or get the state of this->useframes
   * @param  useframes, reference to std::string
   * @return ERROR or NO_ERROR
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
  /** AstroCam::Interface::access_useframes ***********************************/


  /** AstroCam::Interface::nframes ********************************************/
  /**
   * @fn     nframes
   * @brief  
   * @param  
   * @return 
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
        int rows = this->controller.at(dev).rows;
        int cols = this->controller.at(dev).cols;

        this->nfpseq  = parse_val(tokens.at(1));         // requested nframes is nframes/sequence
        this->nframes = this->nfpseq * this->nsequences; // number of frames is (frames/sequence) x (sequences)
        std::stringstream snf;
        snf << "SNF " << this->nframes;                  // SNF sets total number of frames (Y:<N_FRAMES) on timing board

        message.str(""); message << "sending " << snf.str();
        logwrite(function, message.str());

        if ( this->native(snf.str()) != 0x444F4E ) return -1;

        std::stringstream fps;
        fps << "FPS " << nfpseq;            // FPS sets number of frames per sequence (Y:<N_SEQUENCES) on timing board

        message.str(""); message << "sending " << fps.str();
        logwrite(function, message.str());

        if ( this->native(fps.str()) != 0x444F4E ) return -1;

//TODO
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

        if ( this->native(fpb.str()) == 0x444F4E ) return 0; else return -1;
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
  /** AstroCam::Interface::nframes ********************************************/


  /** AstroCam::Interface::expose *********************************************/
  /**
   * @fn     expose
   * @brief  initiate an exposure
   * @param  nseq_in string, if set becomes the number of sequences
   * @return ERROR or NO_ERROR
   *
   */
  long Interface::expose(std::string nseq_in) {
    std::string function = "AstroCam::Interface::expose";
    std::stringstream message;
    std::string nseqstr;
    std::string devstr;
    std::string _start_time;
    long error;
    int nseq;

    for (auto dev : this->devlist) {
      try {
        // check exposure_time
        //
        if ( this->controller.at( dev ).info.exposure_time < 0 ) {
          logwrite( function, "ERROR: exposure time is undefined" );
          return( ERROR );
        }

        // check image size
        //
        int rows = this->controller.at(dev).rows;
        int cols = this->controller.at(dev).cols;
        if (rows < 1 || cols < 1) {
          message.str(""); message << "error: image size must be non-zero: rows=" << rows << " cols=" << cols;
          logwrite(function, message.str());
          return(ERROR);
        }

        // check readout type
        //
        if ( this->controller.at( dev ).info.readout_name.empty() ) {
          logwrite( function, "ERROR: readout undefined" );
          return( ERROR );
        }

        // check buffer size
        // need allocation for at least 2 frames if nframes is greater than 1
        //
        int bufsize = this->get_bufsize();
        if ( bufsize < (int)( (this->nframes>1?2:1) * rows * cols * sizeof(unsigned short) ) ) {  // TODO type check bufsize: is it big enough?
          message.str(""); message << "error: insufficient buffer size (" << bufsize 
                                   << " bytes) for " << this->nframes << " frame" << (this->nframes==1?"":"s")
                                   << " of " << rows << " x " << cols << " pixels";
          logwrite(function, message.str());
          message.str(""); message << "minimum buffer size is " << (this->nframes>1?2:1) * rows * cols * sizeof(unsigned short) << " bytes";
          logwrite(function, message.str());
          return(ERROR);
        }

        // Currently, astrocam doesn't use "modes" like Archon does -- do we need that?
        // For the time being, set a fixed bitpix here. Doesn't seem like a final
        // solution but need to set it someplace for now.
        //
        this->controller.at(dev).info.detector_pixels[0] = cols;
        this->controller.at(dev).info.detector_pixels[1] = rows;
        // ROI is the full detector
        this->controller.at(dev).info.region_of_interest[0] = 1;
        this->controller.at(dev).info.region_of_interest[1] = this->controller.at(dev).info.detector_pixels[0];
        this->controller.at(dev).info.region_of_interest[2] = 1;
        this->controller.at(dev).info.region_of_interest[3] = this->controller.at(dev).info.detector_pixels[1];
        // Binning factor (no binning)
        this->controller.at(dev).info.binning[0] = 1;
        this->controller.at(dev).info.binning[1] = 1;

        this->controller.at(dev).info.bitpix = 16;
        this->controller.at(dev).info.frame_type = Common::FRAME_RAW;
        if ( this->controller.at(dev).info.set_axes() != NO_ERROR ) {
          message.str(""); message << "ERROR setting axes for device " << dev;
          logwrite( function, message.str() );
          return( ERROR );
        }

        this->controller.at(dev).info.systemkeys.keydb = this->systemkeys.keydb;
        this->controller.at(dev).info.userkeys.keydb   = this->userkeys.keydb;
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for ( auto check : this->devlist ) message << check << " ";
        message << "}";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch(...) { logwrite(function, "unknown error creating fitsname for controller"); return(ERROR); }
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
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: sequences " << nseq_in << " outside integer range";
        logwrite(function, message.str());
        return(ERROR);
      }
      // Set the extension = 0 for each controller
      //
      for ( auto dev : this->devlist ) {
        try {
          this->controller.at( dev ).info.extension = 0;
        }
        catch ( std::out_of_range & ) {
          message.str(""); message << "ERROR: no active controller for device number " << dev;
          logwrite( function, message.str() );
          return( ERROR );
        }
      }
    }

    if ( nseq > 1 ) {
      message.str(""); message << "NOTICE: multiple exposures not currently supported";
      logwrite( function, message.str() );
      server.common.message.enqueue( message.str() );
    }

    // Clear the abort flag for a new exposure, in case it was previously set
    //
    this->common.set_abortstate( false );

    // Initialize framethread count -- this keeps track of threads
    // spawned by frameCallback, to make sure that all have been completed.
    //
    this->init_framethread_count();

    // Copy the userkeys database object into camera_info
    //
//  this->camera_info.userkeys.keydb = this->userkeys.keydb;

//  this->camera_info.start_time = get_timestamp( );                // current system time formatted as YYYY-MM-DDTHH:MM:SS.sss
    _start_time = get_timestamp( );                                 // current system time formatted as YYYY-MM-DDTHH:MM:SS.sss
    this->common.set_fitstime( _start_time );                       // sets common.fitstime (YYYYMMDDHHMMSS) used for filename

    message.str(""); message << "starting exposure at " << _start_time;
    logwrite(function, message.str());

    // prepare the camera info class object for each controller
    //
    for (auto dev : this->devlist) {        // spawn a thread for each device in devlist
      try {

        // Initialize a frame counter for each device. 
        //
        this->controller.at(dev).init_framecount();

        // Copy the camera_info into each dev's info class object
        //
//      this->camera.at(dev).info = this->camera_info;  // TODO I think this is obsolete now that I've included Common::Information in the Controller class

        // Allocate workspace memory for deinterlacing (each dev has its own workbuf)
        //
        if ( ( error = this->controller.at(dev).alloc_workbuf() ) != NO_ERROR ) return( error );

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
        if ( ( error = this->common.get_fitsname( devstr, this->controller.at(dev).info.fits_name ) ) != NO_ERROR ) return( error );

#ifdef LOGLEVEL_DEBUG
        message.str("");
        message << "[DEBUG] pointers for dev " << dev << ": "
                << " pArcDev="  << std::hex << this->controller.at(dev).pArcDev 
                << " pCB="      << std::hex << this->controller.at(dev).pCallback
                << " pFits="    << std::hex << this->controller.at(dev).pFits;
        logwrite(function, message.str());
#endif
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for (auto check : this->devlist) message << check << " ";
        message << "}";
        logwrite(function, message.str());
        return(ERROR);
      }
      catch(...) { logwrite(function, "unknown error creating fitsname for controller"); return(ERROR); }
    }

    // Spawn separate threads to call ARC_API's expose() on each connected device, in parallel
    //
    {  // begin local scope
    std::vector<std::thread> threads;       // create a local scope vector for the threads
    for (auto dev : this->devlist) {        // spawn a thread for each device in devlist
      try {
        // Open the fits file, ...
        //
        error = this->controller.at(dev).open_file( this->common.writekeys_when );

        // ... then spawn the expose thread.
        //
        std::thread thr( std::ref(AstroCam::Interface::dothread_expose), std::ref(this->controller.at(dev)) );
        threads.push_back(std::move(thr));  // push the thread into a vector
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for (auto check : this->devlist) message << check << " ";
        message << "}";
        logwrite(function, message.str());
        return(ERROR);
      }
      catch(const std::exception &e) {
        message.str(""); message << "ERROR creating expose thread: " << e.what();
        logwrite(function, message.str());
        return(ERROR);
      }
      catch(...) { logwrite(function, "unknown error creating expose thread for controller"); return(ERROR); }
    }

    // Join the dothread_expose threads to this thread (not to each other)
    // in order to wait for them to complete.
    //
    try {
      for (std::thread & thr : threads) {   // loop through the vector of threads
        if ( thr.joinable() ) thr.join();   // if thread object is joinable then join to this function. (not to each other)
      }
    }
    catch(const std::exception &e) {
      message.str(""); message << "ERROR joining expose threads: " << e.what();
      logwrite(function, message.str());
      return(ERROR);
    }
    catch(...) { logwrite(function, "unknown error joining expose threads"); return(ERROR); }

    threads.clear();                        // deconstruct the threads vector
    }  // end local scope

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
        if ( ( error = this->controller.at(dev).error ) != NO_ERROR ) break;
      }
      catch(std::out_of_range &) {
      }
    }

    if ( error == NO_ERROR ) {
      this->common.increment_imnum();                                 // increment image_num when fitsnaming == "number"
      logwrite(function, "all frames written");
    }
    else {
      logwrite(function, "ERROR: writing image");
    }

    // close the FITS file
    //
    for (auto dev : this->devlist) {
      try {
        this->controller.at(dev).close_file( this->common.writekeys_when );
        if ( this->controller.at(dev).pFits->iserror() ) error = ERROR;   // allow error to be set (but not cleared)
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
  /** AstroCam::Interface::expose *********************************************/


  /** AstroCam::Interface::load_firmware **************************************/
  /**
   * @fn     load_firmware
   * @brief  load firmware (.lod) into one or all controller timing boards
   * @param  none
   * @return ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   * This version is when no parameter is specified, in which case try to
   * load the default lodfiles specified in the config file. To do that,
   * a string is built and the other load_firmware(std::string) is called.
   *
   */
  long Interface::load_firmware(std::string &retstring) {
    std::string function = "AstroCam::Interface::load_firmware";

    // If no connected devices then nothing to do here
    //
    if (this->numdev == 0) {
      logwrite(function, "ERROR: no connected devices");
      return(ERROR);
    }

    long error=NO_ERROR;

    // Loop through all of the default firmware entries found in the configfile
    // which were stored in an STL map so that
    //   fw->first is the devnumber, and
    //   fw->second is the firmware filename.
    // Use these to build a string of the form "<dev> <filename>" to pass to the
    // load_firmware() function to load each controller with the specified file.
    //
    for (auto fw = this->common.firmware.begin(); fw != this->common.firmware.end(); ++fw) {
      std::stringstream lodfilestream;
      // But only use it if the device is open
      //
      if ( std::find( this->devlist.begin(), this->devlist.end(), fw->first ) != this->devlist.end() ) {
        lodfilestream << fw->first << " " << fw->second;

        // Call load_firmware with the built up string.
        // If it returns an error then set error flag to return to the caller.
        //
        if (this->load_firmware(lodfilestream.str(), retstring) == ERROR) error = ERROR;
      }
    }
    return(error);
  }
  /** AstroCam::Interface::load_firmware **************************************/


  /** AstroCam::Interface::load_firmware **************************************/
  /**
   * @fn     load_firmware
   * @brief  load firmware (.lod) into one or all controller timing boards
   * @param  timlodfile, string may contain a controller number
   * @return ERROR or NO_ERROR
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
  long Interface::load_firmware(std::string timlodfile, std::string &retstring) {
    std::string function = "AstroCam::Interface::load_firmware";
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
          if (this->controller.at(dev).connected) {   // but only if connected
            std::thread thr( std::ref(AstroCam::Interface::dothread_load), std::ref(this->controller.at(dev)), timlodfile );
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
      try {
        check_retval = this->controller.at(firstdev).retval;    // save the first one in the controller vector
      }
      catch(std::out_of_range &) {
        message.str(""); message << "ERROR: device " << firstdev << " invalid";
        logwrite( function, message.str() );
        return(ERROR);
      }

      bool allsame = true;
      for (auto dev : selectdev) {
        try {
          if (this->controller.at(dev).retval != check_retval) {
            allsame = false;
          }
        }
        catch(std::out_of_range &) {
          message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
          for (auto check : this->devlist) message << check << " ";
          message << "}";
          logwrite(function, message.str());
          return(ERROR);
        }
        catch(...) { logwrite(function, "unknown error looking for return values"); return(ERROR); }
      }

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
          try {
            this->retval_to_string( this->controller.at(dev).retval, rs );      // convert the retval to string (DON, ERR, etc.)
            rss << this->controller.at(dev).devnum << ":" << rs << " ";
          }
          catch(std::out_of_range &) {
            message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
            for (auto check : this->devlist) message << check << " ";
            message << "}";
            logwrite(function, message.str());
            return(ERROR);
          }
          catch(...) { logwrite(function, "unknown error looking for return values"); return(ERROR); }
        }
        retstring = rss.str();
        error = ERROR;
      }
    }

    return( error );
  }
  /** AstroCam::Interface::load_firmware **************************************/


  /** AstroCam::Interface::dothread_load **************************************/
  /**
   * @fn     dothread_load
   * @brief  run in a thread to actually perform the load
   * @param  contoller, reference to Controller object
   * @param  timlodfile, string of lodfile name to load
   * @return none
   *
   * loadControllerFile() doesn't return a value but throws std::runtime_error on error.
   *
   * The retval for each device in the contoller info structure will be set here,
   * to ERR on exception or to DON if no exception is thrown.
   *
   */
  void Interface::dothread_load(Controller &controller, std::string timlodfile) {
    std::string function = "AstroCam::Interface::dothread_load";
    std::stringstream message;

    try {
      controller.pArcDev->loadControllerFile( timlodfile );  // Do the load here
    }
    catch(const std::exception &e) {
      message.str(""); message << "ERROR: " << controller.devname << ": " << e.what();
      logwrite(function, message.str());
      controller.retval = ERR;
      controller.firmwareloaded = false;
      return;
    }
    catch(...) {
      message.str(""); message << "unknown error loading firmware for " << controller.devname;
      logwrite(function, message.str());
      controller.retval = ERR;
      controller.firmwareloaded = false;
      return;
    }
    message.str(""); message << "devnum " << controller.devnum << ": loaded firmware " << timlodfile;
    logwrite(function, message.str());
    controller.retval = DON;
    controller.firmwareloaded = true;
  }
  /** AstroCam::Interface::dothread_load **************************************/


  /** AstroCam::Interface::buffer *********************************************/
  /**
   * @fn     buffer
   * @brief  set/get mapped image buffer
   * @param  string size_in, string containing the buffer allocation info
   * @param  string &retstring, reference to a string for return values
   * @return ERROR or NO_ERROR
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
        this->controller.at(dev).pArcDev->reMapCommonBuffer( try_bufsize );
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
  /** AstroCam::Interface::buffer *********************************************/


  /** AstroCam::Interface::readout ********************************************/
  /**
   * @fn     readout
   * @brief  
   * @param  string readout_in, string containing the requested readout type
   * @param  string &readout_out, reference to a string for return values
   * @return ERROR or NO_ERROR
   *
   * This function sets (or gets) the type of readout. This will encompass
   * which amplifier(s) for a CCD (by name) or infrared (by number).
   * Selecting the readout will also set the appropriate deinterlacing
   * scheme to be used.
   *
   */
  long Interface::readout(std::string readout_in, std::string &readout_out) {
    std::string function = "AstroCam::Interface::readout";
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
            this->controller.at( dev ).info.readout_name = readout_name_req;
            this->controller.at( dev ).info.readout_type = readout_type;
            this->controller.at( dev ).readout_arg = readout_arg;
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
          cmd.str(""); cmd << "SOS " << readout_arg;              // TODO should this 3-letter command be generalized, or configurable?
          error = this->native( selectdev, cmd.str(), retstr );   // send the native command here
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
      try {
        std::string mytype;
        if ( this->controller.at( dev ).connected ) mytype = this->controller.at( dev ).info.readout_name;
        else {
          error = ERROR;
          mytype = "???";
        }
        rs << dev << ":" << mytype << " ";
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: no active controller for device number " << dev;
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    message.str(""); message << "readout type " << rs.str();
    logwrite( function, message.str() );

    readout_out = rs.str();

    return( error );
  }
  /** AstroCam::Interface::readout ********************************************/


  /**************** AstroCam::Interface::set_camera_mode **********************/
  /**
   * @fn     set_camera_mode
   * @brief  
   * @param  none
   * @return 
   *
   */
  long Interface::set_camera_mode(std::string mode) {
    std::string function = "AstroCam::Interface::set_camera_mode";
    std::stringstream message;
    logwrite(function, "ERROR: not implemented");
    return( ERROR );
  }
  /**************** AstroCam::Interface::set_camera_mode **********************/


  /**************** AstroCam::Interface::write_frame **************************/
  /**
   * @fn     write_frame
   * @brief  writes the image_data buffer to disk
   * @param  int devnum, device number
   * @param  int fpbcount, frame number = also thread ID receiving the frame
   * @return ERROR or NO_ERROR
   *
   * This function is called by the handle_frame thread.
   *
   */
  long Interface::write_frame(int devnum, int fpbcount) {
    std::string function = "AstroCam::Interface::write_frame";
    std::stringstream message;
    long error=NO_ERROR;

    // Use devnum to identify which device has the frame,
    // and fpbcount to index the frameinfo STL map on that devnum,
    // and assign the pointer to that buffer to a local variable.
    //
    void* imbuf = this->controller.at(devnum).frameinfo[fpbcount].buf;

    message << this->controller.at(devnum).devname << " received frame " 
            << this->controller.at(devnum).frameinfo[fpbcount].framenum << " into image buffer "
            << std::hex << this->controller.at(devnum).frameinfo[fpbcount].buf;
    logwrite(function, message.str());

    // Call the class' deinterlace and write functions.
    //
    // The .deinterlace() function is called with the image buffer pointer cast to the appropriate type.
    // The .write() function writes the deinterlaced (work) buffer, which is a class member, so
    // that buffer is already known.
    //
    try {
      switch (this->controller.at(devnum).info.datatype) {
        case USHORT_IMG: {
          this->controller.at(devnum).deinterlace( (uint16_t *)imbuf );
          error = this->controller.at(devnum).write( );
          break;
        }
        case SHORT_IMG: {
          this->controller.at(devnum).deinterlace( (int16_t *)imbuf );
          error = this->controller.at(devnum).write( );
          break;
        }
        case FLOAT_IMG: {
          this->controller.at(devnum).deinterlace( (uint32_t *)imbuf );
          error = this->controller.at(devnum).write( );
          break;
        }
        default:
          message.str("");
          message << "ERROR: unknown datatype: " << this->controller.at(devnum).info.datatype;
          logwrite(function, message.str());
          error = ERROR;
          break;
      }
      // A frame has been written for this device,
      // so increment the framecounter for devnum.
      //
      if (error == NO_ERROR) this->controller.at(devnum).increment_framecount();
    }
    catch (std::out_of_range &) {
      message.str(""); message << "ERROR: unable to find device " << devnum << " in list: { ";
      for (auto check : this->devlist) message << check << " ";
      message << "}";
      logwrite(function, message.str());
      error = ERROR;
    }

//  this->increment_framecount();

#ifdef LOGLEVEL_DEBUG
    message.str("");
    message << "[DEBUG] completed " << (error != NO_ERROR ? "with error. " : "ok. ")
            << "devnum=" << devnum << " " << "fpbcount=" << fpbcount << " "
            << this->controller.at(devnum).devname << " received frame " 
            << this->controller.at(devnum).frameinfo[fpbcount].framenum << " into buffer "
            << std::hex << std::uppercase << this->controller.at(devnum).frameinfo[fpbcount].buf;
    logwrite(function, message.str());
#endif
    return( error );
  }
  /**************** AstroCam::Interface::write_frame **************************/


  /**************** AstroCam::Interface::exptime ******************************/
  /**
   * @fn     exptime
   * @brief  set/get the exposure time
   * @param  string
   * @return ERROR or NO_ERROR
   *
   */
  long Interface::exptime(std::string exptime_in, std::string &retstring) {
    std::string function = "AstroCam::Interface::exptime";
    std::stringstream message;
    long error = NO_ERROR;
    int exptime_try=0;

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
        try {
          // Send it to the controller via the SET command.
          //
          std::stringstream cmd;
          cmd << "SET " << exptime_try;
          error = this->native( cmd.str() );

          // Set the class variable if SET was successful
          //
          if ( error == NO_ERROR ) this->controller.at(dev).info.exposure_time = exptime_try;
          this->controller.at(dev).info.exposure_unit = "msec";  // chaning unit not currently supported in ARC
        }
        catch ( std::out_of_range & ) {
          message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
          for ( auto check : this->devlist ) message << check << " ";
          message << "}";
          logwrite( function, message.str() );
          error = ERROR;
        }
      }
    }

    // Check to see if all exposure times are the same by comparing them to the first
    //
    bool allsame = true;
    for ( auto dev : this->devlist ) {
      try {
        if ( this->controller.at(dev).info.exposure_time != this->controller.front().info.exposure_time ) {
          allsame = false;
        }
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
        for ( auto check : this->devlist ) message << check << " ";
        message << "}";
        logwrite( function, message.str() );
        error = ERROR;
      }
    }

    // If they're all the same then the return value is just the first
    //
    if ( allsame ) {
      retstring = std::to_string( this->controller.front().info.exposure_time );
    }

    // otherwise the return value is a list of all of them
    //
    else {
      std::stringstream rss;
      for ( auto dev : this->devlist ) {
        try {
          rss << this->controller.at(dev).devnum << ":" << this->controller.at(dev).info.exposure_time << " ";
        }
        catch ( std::out_of_range & ) {
          message.str(""); message << "ERROR: unable to find device " << dev << " in list: { ";
          for ( auto check : this->devlist ) message << check << " ";
          message << "}";
          logwrite( function, message.str() );
          error = ERROR;
        }
      }
      retstring = rss.str();
    }

    message.str(""); message << "exposure time is " << retstring << " msec";
    logwrite(function, message.str());
    return( error );
  }
  /**************** AstroCam::Interface::exptime ******************************/


  /** AstroCam::Interface::shutter ********************************************/
  /**
   * @fn     shutter
   * @brief  set or get the shutter enable state
   * @param  std::string shutter_in
   * @param  std::string& shutter_out
   * @return ERROR or NO_ERROR
   *
   */
  long Interface::shutter(std::string shutter_in, std::string& shutter_out) {
    std::string function = "AstroCam::Interface::shutter";
    std::stringstream message;
    long error = NO_ERROR;
    bool shutten = false;

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
        logwrite( function, "error converting shutter_in to lowercase" );
        return( ERROR );
      }
    }

    // Set shutterenable the same for all devices
    // TODO enable setting differently for each device
    //
    for ( auto dev : this->devlist ) {
      this->controller.at(dev).info.shutterenable = shutten;
    }

    // set the return value and report the state now, either setting or getting
    //
    shutter_out = shutten ? "enabled" : "disabled";
    message.str("");
    message << "shutter is " << shutter_out;
    logwrite( function, message.str() );

    // Add the shutter enable keyword to the system keys db
    //
    message.str(""); message << "SHUTTEN=" << ( this->camera_info.shutterenable ? "T" : "F" ) << "// shutter was enabled";
    this->systemkeys.addkey( message.str() );

    return error;
  }
  /** AstroCam::Interface::shutter ********************************************/


  /**************** AstroCam::Interface::geometry *****************************/
  /**
   * @fn     geometry
   * @brief  set/get geometry
   * @param  args contains: X Y (i.e. cols rows)
   * @param  &retstring, reference to string for return value
   * @return ERROR or NO_ERROR
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
  long Interface::geometry(std::string args, std::string &retstring) {
    std::string function = "AstroCam::Interface::geometry";
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
      if ( error == NO_ERROR && this->native( selectdev, cmd.str() ) == ERROR ) error = ERROR;

      cmd.str(""); cmd << "WRM 0x400002 " << _rows;
      if ( error == NO_ERROR && this->native( selectdev, cmd.str() ) == ERROR ) error = ERROR;

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
        // Return value from this->native() is of the form "dev:0xVALUE"
        // so must parse the hex value after the colon.
        //
        cmd.str(""); cmd << "RDM 0x400001 ";
        if ( error == NO_ERROR && this->native( dev, cmd.str(), col_s ) == ERROR ) error = ERROR;
        if ( error == NO_ERROR ) this->controller.at(dev).cols = (uint32_t)parse_val( col_s.substr( col_s.find(":")+1 ) );

        cmd.str(""); cmd << "RDM 0x400002 ";
        if ( error == NO_ERROR && this->native( dev, cmd.str(), row_s ) == ERROR ) error = ERROR;
        if ( error == NO_ERROR ) this->controller.at(dev).rows = (uint32_t)parse_val( row_s.substr( row_s.find(":")+1 ) );

        rs << dev << ":" << this->controller.at(dev).cols << " " << this->controller.at(dev).rows << " ";
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
  /**************** AstroCam::Interface::geometry *****************************/


  /**************** AstroCam::Interface::bias *********************************/
  /**
   * @fn     bias
   * @brief  set a bias
   * @param  args contains: module, channel, bias
   * @return ERROR or NO_ERROR
   *
   */
  long Interface::bias(std::string args, std::string &retstring) {
    std::string function = "AstroCam::Interface::bias";
    std::stringstream message;
    logwrite(function, "ERROR: not implemented");
    return( ERROR );
  }
  /**************** AstroCam::Interface::bias *********************************/


  void Interface::handle_queue(std::string message) {
    server.common.message.enqueue( message );
  }

  /**************** AstroCam::Interface::handle_frame *************************/
  /**
   * @fn     handle_frame
   * @brief  process each frame received by frameCallback for any device
   * @param  fpbcount, frames per buffer count, returned from the ARC API
   * @param  fcount, number of frames read, returned from the ARC API
   * @param  *buffer, pointer to the PCI/e buffer, returned from the ARC API
   * @return none
   *
   * This function is static, spawned in a thread created by frameCallback()
   * which is the callback fired by the ARC API when a frame has been received.
   *
   */
  void Interface::handle_frame(int devnum, uint32_t fpbcount, uint32_t fcount, void* buffer) {
    std::string function = "AstroCam::Interface::handle_frame";
    std::stringstream message;
    int error = NO_ERROR;

#ifdef LOGLEVEL_DEBUG
    message << "[DEBUG] devnum=" << devnum << " " << "fpbcount=" << fpbcount 
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
    try {
      if ( server.controller.at(devnum).frameinfo.count( fpbcount ) == 0 ) {  // ...no
        server.controller.at(devnum).frameinfo[ fpbcount ].tid      = fpbcount;
        server.controller.at(devnum).frameinfo[ fpbcount ].buf      = buffer;
        // If useframes is false then set framenum=0 because it doesn't mean anything,
        // otherwise set it to the fcount received from the API.
        //
        server.controller.at(devnum).frameinfo[ fpbcount ].framenum = server.useframes ? fcount : 0;
      }
      else {                                                                  // ...yes
        logwrite(function, "ERROR: frame buffer overrun! Try allocating a larger buffer");
        server.frameinfo_mutex.unlock();
        return;
      }
    }
    catch (std::out_of_range &) {
      message.str(""); message << "ERROR indexing controller devnum=" << devnum << " or frame at fpb=" << fpbcount;
      logwrite(function, message.str());
      server.frameinfo_mutex.unlock();
      return;
    }

    server.frameinfo_mutex.unlock();               // release access to frameinfo structure

#ifdef LOGLEVEL_DEBUG
    if ( server.useframes ) {
      logwrite(function, "[DEBUG] waiting for frame");
    }
    else {
      logwrite(function, "[DEBUG] firmware doesn't support frames");
    }
#endif

    // Write the frames in numerical order.
    // Don't let one thread get ahead of another when it comes to writing.
    //
    double start_time = get_clock_time();
    do {
      int this_frame = fcount;                     // the current frame
      int last_frame = server.controller.at(devnum).get_framecount();    // the last frame that has been written by this device
      int next_frame = last_frame + 1;             // the next frame in line
      if (this_frame != next_frame) {              // if the current frame is NOT the next in line then keep waiting
        usleep(5);
        double time_now = get_clock_time();
        if (time_now - start_time >= 1.0) {        // update progress every 1 sec so the user knows what's going on
          start_time = time_now;
          logwrite(function, "waiting for frames");//TODO presumably update GUI or set a global that the CLI can access
#ifdef LOGLEVEL_DEBUG
          message.str(""); message << "[DEBUG] this_frame=" << this_frame << " next_frame=" << next_frame;
          logwrite(function, message.str());
#endif
        }
      }
      else {                                       // otherwise it's time to write this frame to disk
        break;                                     // note that frameinfo_mutex remains locked now until completion
      }

      if ( server.common.get_abortstate() ) break; // provide the user a way to get out

    } while ( server.useframes );                  // some firmware doesn't support frames so get out after one frame if it doesn't

    // If not aborted then write this frame
    //
    if ( ! server.common.get_abortstate() ) {
#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] calling server.write_frame for devnum=" << devnum << " fpbcount=" << fpbcount;
    logwrite(function, message.str());
#endif
      error = server.write_frame( devnum, fpbcount );
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
    try {
      server.frameinfo_mutex.lock();               // protect access to frameinfo structure
      server.controller.at(devnum).frameinfo.erase( fpbcount );
    }
    catch (std::out_of_range &) {
      message.str(""); message << "ERROR erasing frameinfo for fpb=" << fpbcount << " at controller devnum=" << devnum;
      logwrite(function, message.str());
    }

    server.frameinfo_mutex.unlock();

    server.remove_framethread();  // framethread_count is decremented because a thread has completed

    return;
  }
  /**************** AstroCam::Interface::handle_frame *************************/


  /**************** AstroCam::Interface::add_framethread **********************/
  /**
   * @fn     add_framethread
   * @brief  call on thread creation to increment framethreadcount
   * @param  none
   * @return none
   *
   */
  inline void Interface::add_framethread() {
    this->framethreadcount_mutex.lock();
    this->framethreadcount++;
    this->framethreadcount_mutex.unlock();
  }
  /**************** AstroCam::Interface::add_framethread **********************/


  /**************** AstroCam::Interface::remove_framethread *******************/
  /**
   * @fn     remove_framethread
   * @brief  call on thread destruction to decrement framethreadcount
   * @param  none
   * @return none
   *
   */
  inline void Interface::remove_framethread() {
    this->framethreadcount_mutex.lock();
    this->framethreadcount--;
    this->framethreadcount_mutex.unlock();
  }
  /**************** AstroCam::Interface::remove_framethread *******************/


  /**************** AstroCam::Interface::get_framethread **********************/
  /**
   * @fn     get_framethread
   * @brief  return the number of active threads spawned for handling frames
   * @param  none
   * @return int, number of threads
   *
   */
  inline int Interface::get_framethread_count() {
    int count;
    this->framethreadcount_mutex.lock();
    count = this->framethreadcount;
    this->framethreadcount_mutex.unlock();
    return( count );
  }
  /**************** AstroCam::Interface::get_framethread **********************/


  /**************** AstroCam::Interface::init_framethread_count ***************/
  /**
   * @fn     init_framethread_count
   * @brief  initialize framethreadcount = 0
   * @param  none
   * @return none
   *
   */
  inline void Interface::init_framethread_count() {
    this->framethreadcount_mutex.lock();
    this->framethreadcount = 0;
    this->framethreadcount_mutex.unlock();
  }
  /**************** AstroCam::Interface::init_framethread_count ***************/


  /** AstroCam::Interface::Controller::Controller *****************************/
  /**
   * @fn     Controller
   * @brief  class constructor
   * @param  none
   * @return none
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
  }
  /** AstroCam::Interface::Controller::Controller *****************************/


  /**************** AstroCam::Interface::Controller::open_file ****************/
  /**
   * @fn     open_file
   * @brief  wrapper to open the current fits file object
   * @param  none
   * @return ERROR or NO_ERROR
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
  /**************** AstroCam::Interface::Controller::open_file ****************/


  /**************** AstroCam::Interface::Controller::close_file ***************/
  /**
   * @fn     close_file
   * @brief  wrapper to close the current fits file object
   * @param  none
   * @return none
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
  /**************** AstroCam::Interface::Controller::close_file ***************/


  /** AstroCam::Interface::Controller::init_framecount ************************/
  /**
   * @fn     init_framecount 
   * @brief  initialize this->framecount=0, protected by mutex
   * @param  none
   * @return none
   *
   */
  inline void Interface::Controller::init_framecount() {
    server.framecount_mutex.lock();
    this->framecount = 0;
    server.framecount_mutex.unlock();
  }
  /** AstroCam::Interface::Controller::init_framecount ************************/


  /** AstroCam::Interface::Controller::get_framecount *************************/
  /**
   * @fn     get_framecount 
   * @brief  returns this->framecount, protected by mutex
   * @param  none
   * @return int framecount
   *
   */
  inline int Interface::Controller::get_framecount() {
    int count;
    server.framecount_mutex.lock();
    count = this->framecount;
    server.framecount_mutex.unlock();
    return( count );
  }
  /** AstroCam::Interface::Controller::get_framecount *************************/


  /** AstroCam::Interface::Controller::increment_framecount *******************/
  /**
   * @fn     increment_framecount 
   * @brief  increments this->framecount, protected by mutex
   * @param  none
   * @return none
   *
   */
  inline void Interface::Controller::increment_framecount() {
    server.framecount_mutex.lock();
    this->framecount++;
    server.framecount_mutex.unlock();
  }
  /** AstroCam::Interface::Controller::increment_framecount *******************/


  /**************** AstroCam::Interface::deinterlace **************************/
  /**
   * @fn     deinterlace
   * @brief  spawns the deinterlacing threads
   * @param  args contains: module, channel, bias
   * @return pointer to workbuf
   *
   * This function spawns threads to perform the actual deinterlacing in order
   * to get it done as quickly as possible.
   * 
   * Called by write_frame(), which is called by the handle_frame thread.
   *
   */
  template <class T>
  T* Interface::Controller::deinterlace(T* imbuf) {
    std::string function = "AstroCam::Interface::deinterlace";
    std::stringstream message;

    int nthreads = cores_available();
nthreads=2;  // TODO *** come back to this !!! ***
logwrite( function, "NOTICE:override nthreads=2 !!!" );
server.common.message.enqueue( "NOTICE:override nthreads=2 !!!" );

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

    message.str(""); message << "deinterlacing for dev " << this->devnum << " complete";
    logwrite(function, message.str());

    return( (T*)this->workbuf );
  }
  /**************** AstroCam::Interface::deinterlace **************************/


  /**************** AstroCam::Interface::dothread_deinterlace *****************/
  /**
   * @fn     dothread_deinterlace
   * @brief  run in a thread to perform the actual deinterlacing
   * @param  T &deinterlace, reference to DeInterlace class object
   * @param  int cols
   * @param  int rows
   * @param  int section, the section of the buffer this thread is working on
   * @param  int nthreads, the number of threads being used for deinterlacing
   * @return none
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

  }
  /**************** AstroCam::Interface::dothread_deinterlace *****************/


  /**************** AstroCam::Interface::test *********************************/
  /**
   * @fn     test
   * @brief  test routines
   * @param  string args contains test name and arguments
   * @param  reference to retstring for any return values
   * @return ERROR or NO_ERROR
   *
   * This is the place to put various debugging and system testing tools.
   * It is placed here, rather than in common, to allow for controller-
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
      logwrite(function, "no test name provided");
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
      this->common.set_fitstime( get_timestamp( ) );                 // must set common.fitstime first
      if ( this->devlist.size() > 1 ) {
        for (auto dev : this->devlist) {
          this->common.get_fitsname( std::to_string(dev), msg );     // get the fitsname (by reference)
          this->common.message.enqueue( msg );                       // queue the fitsname
          logwrite( function, msg );                                 // log ths fitsname
        }
      }
      else {
        this->common.get_fitsname( msg );                            // get the fitsname (by reference)
        this->common.message.enqueue( msg );                         // queue the fitsname
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
        this->common.message.enqueue( tokens[1] );
      }
      else {                                // if no string passed then queue a simple test message
        logwrite(function, "test");
        this->common.message.enqueue("test");
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
  /**************** AstroCam::Interface::test *********************************/


  /**************** AstroCam::Interface::Controller::write ********************/
  /**
   * @fn     write
   * @brief  wrapper to write a fits file
   * @param  none
   * @return ERROR or NO_ERROR
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

if ( server.common.get_abortstate() ) {
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
  /**************** AstroCam::Interface::Controller::write ********************/


  /**************** AstroCam::Interface::Controller::alloc_workbuf ************/
  /**
   * @fn     alloc_workbuf
   * @brief  allocate workspace memory for deinterlacing
   * @param  none
   * @return ERROR or NO_ERROR
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
  /**************** AstroCam::Interface::Controller::alloc_workbuf ************/


  /**************** AstroCam::Interface::Controller::alloc_workbuf ************/
  /**
   * @fn     alloc_workbuf
   * @brief  allocate workspace memory for deinterlacing
   * @param  buf, pointer to template type T
   * @return pointer to the allocated space
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
  /**************** AstroCam::Interface::Controller::alloc_workbuf ************/


  /**************** AstroCam::Interface::Controller::free_workbuf *************/
  /**
   * @fn     free_workbuf
   * @brief  free (delete) memory allocated by alloc_workbuf
   * @param  buf, pointer to template type T
   * @return none
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
  }
  /**************** AstroCam::Interface::Controller::alloc_workbuf ************/

}
