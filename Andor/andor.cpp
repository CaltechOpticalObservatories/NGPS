/**
 * @file    andor.cpp
 * @brief   this file contains the code for the Andor interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "atmcdLXd.h"
#include "andor.h"
#include "logentry.h"

namespace Andor {

  /***** Andor::Interface::Interface ******************************************/
  /**
   * @brief      default Interface class constructor
   *
   */
  Interface::Interface() {
    this->initialized = false;
    this->image_data = NULL;
  }
  /***** Andor::Interface::Interface ******************************************/


  /***** Andor::Interface::~Interface *****************************************/
  /**
   * @brief      Interface class deconstructor
   *
   */
  Interface::~Interface() {
  };
  /***** Andor::Interface::~Interface *****************************************/


  /***** Andor::Information::Information **************************************/
  /**
   * @brief      default Information class constructor
   *
   */
  Information::Information() {
    this->mode=-1;
    this->modestr="";
    this->exposure_time=0;
  }
  /***** Andor::Interface::Interface ******************************************/


  /***** Andor::Interface::_GetAcquiredData16 *********************************/
  /**
   * @brief      wrapper for Andor SDK GetAcquiredData16 checks return value
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_GetAcquiredData16( uint16_t* buf, unsigned long bufsize ) {
    std::string function = "Andor::Interface::_GetAcquiredData16";
    std::stringstream message;

    unsigned int ret = GetAcquiredData16( buf, bufsize );

    switch ( ret ) {
      case DRV_NOT_INITIALIZED: message << "ERROR system not initialized";            break;
      case DRV_ACQUIRING:       message << "ERROR acquisition already in progress";   break;
      case DRV_VXDNOTINSTALLED: message << "ERROR VxD not loaded";                    break;
      case DRV_ERROR_ACK:       message << "ERROR unable to communicate with device"; break;
      case DRV_P1INVALID:       message << "ERROR invalid pointer";                   break;
      case DRV_P2INVALID:       message << "ERROR wrong array size";                  break;
      case DRV_NO_NEW_DATA:     message << "ERROR no acquisition has taken place";    break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }
    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_GetAcquiredData16 *********************************/


  /***** Andor::Interface::_GetAvailableCameras *******************************/
  /**
   * @brief      wrapper for Andor SDK GetAvailableCameras checks return value
   * @param[out] number  pointer to return number of cameras
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_GetAvailableCameras( int* number ) {
    std::string function = "Andor::Interface::_GetAvailableCameras";
    std::stringstream message;

    unsigned int ret = GetAvailableCameras( number );

    switch ( ret ) {
      case DRV_GENERAL_ERRORS: message << "ERROR unable to get number of cameras";            break;
      default:                 message << "ERROR unrecognized return code: " << ret;          break;
    }
    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_GetAvailableCameras *******************************/


  /***** Andor::Interface::_GetCameraHandle ***********************************/
  /**
   * @brief      wrapper for Andor SDK GetCameraHandle checks return value
   * @param[in]  index   index of installed, available camera
   * @param[out] handle  pointer to the handle received
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_GetCameraHandle( int index, int* handle ) {
    std::string function = "Andor::Interface::_GetCameraHandle";
    std::stringstream message;

    unsigned int ret = GetCameraHandle( index, handle );

    switch (ret) {
      case DRV_P1INVALID: message << "ERROR camera index " << index << " invalid"; break;
      default:            message << "ERROR: unknown error " << ret; break;
    }

    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_GetCameraHandle ***********************************/


  /***** Andor::Interface::_GetCameraSerialNumber *****************************/
  /**
   * @brief      wrapper for Andor SDK GetCameraSerialNumber checks return value
   * @param[out] number
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_GetCameraSerialNumber( int* number ) {
    std::string function = "Andor::Interface::_GetCameraSerialNumber";
    std::stringstream message;

    unsigned int ret = GetCameraSerialNumber( number );

    switch (ret) {
      case DRV_NOT_INITIALIZED:  message << "ERROR: system not initialized"; break;
      default:                   message << "ERROR: unknown error " << ret; break;
    }

    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_GetCameraSerialNumber *****************************/


  /***** Andor::Interface::_GetDetector ***************************************/
  /**
   * @brief      wrapper for Andor SDK GetDetector checks return value
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_GetDetector( int* xpix, int* ypix ) {
    std::string function = "Andor::Interface::_GetDetector";
    std::stringstream message;

    unsigned int ret = GetDetector( xpix, ypix );

    switch ( ret ) {
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized"; break;
      default:                  message << "ERROR unrecognized status code " << ret;
    }
    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_GetDetector ***************************************/


  /***** Andor::Interface::_GetStatus *****************************************/
  /**
   * @brief      wrapper for Andor SDK GetStatus checks return value
   * @param[out] status_id   pointer to int to contain the status ID code
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * This function is overloaded. This version does not provide the status message string.
   *
   */
  long Interface::_GetStatus( int* status_id ) {
    std::string function = "Andor::Interface::_GetStatus";
    std::stringstream dontcare;
    return this->_GetStatus( status_id, dontcare );
  }
  /***** Andor::Interface::_GetStatus *****************************************/


  /***** Andor::Interface::_GetStatus *****************************************/
  /**
   * @brief      wrapper for Andor SDK GetStatus checks return value
   * @param[out] status_id   pointer to int to contain the status ID code
   * @param[out] status_msg  reference to stringstream to contain the status message text
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * This function is overloaded. This version provides the status message string.
   *
   */
  long Interface::_GetStatus( int* status_id, std::stringstream &status_msg ) {
    std::string function = "Andor::Interface::_GetStatus";
    std::stringstream message;

    unsigned int ret = GetStatus( status_id );

    switch ( ret ) {
      case DRV_NOT_INITIALIZED:      message << "ERROR not initialized";                         break;
      default:                       message << "ERROR unrecognized return code " << ret;
    }
    switch ( *status_id ) {
      case DRV_IDLE:                 status_msg << "IDLE waiting on instructions";               break;
      case DRV_TEMPCYCLE:            status_msg << "executing temperature cycle";                break;
      case DRV_ACQUIRING:            status_msg << "acquisition in progress";                    break;
      case DRV_ACCUM_TIME_NOT_MET:   status_msg << "ERROR unable to meet accumulate cycle time"; break;
      case DRV_KINETIC_TIME_NOT_MET: status_msg << "ERROR unable to meet kinetic cycle time";    break;
      case DRV_ERROR_ACK:            status_msg << "ERROR unable to communicate with card";      break;
      case DRV_ACQ_BUFFER:           status_msg << "ERROR unable to read data at required rate"; break;
      case DRV_SPOOLERROR:           status_msg << "ERROR spool buffer overflow";                break;
      default:                       status_msg << "ERROR unrecognized status code " << ret;
    }
    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_GetStatus *****************************************/


  /***** Andor::Interface::_GetTemperature ************************************/
  /**
   * @brief      wrapper for Andor SDK GetTemperature checks return value
   * @return     NO_ERROR or ERROR
   *
   */
  long Interface::_GetTemperature() {
    std::string function = "Andor::Interface::_GetTemperature";
    std::stringstream message;

    int temp=0;
    std::string status;

    unsigned int ret = GetTemperature( &temp );

    switch ( ret ) {
      case DRV_NOT_INITIALIZED:     status = "ERROR"; message << "ERROR not initialized";           break;
      case DRV_ACQUIRING:           status = "ERROR"; message << "ERROR acquisition in progress";   break;
      case DRV_ERROR_ACK:           status = "ERROR"; message << "ERROR communicating with device"; break;
      case DRV_TEMP_OFF:            status = "OFF";         break;
      case DRV_TEMP_STABILIZED:     status = "STABILIZED";  break;
      case DRV_TEMP_NOT_REACHED:    status = "NOT_REACHED"; break;
      case DRV_TEMP_DRIFT:          status = "DRIFTED";     break;
      case DRV_TEMP_NOT_STABILIZED: status = "NOT_STABLE";  break;
      default:                      status = "ERROR"; message << "ERROR unrecognized status code " << ret;
    }
    if ( status.compare("ERROR")==0 ) logwrite( function, message.str() );

    this->camera_info.temp        = temp;
    this->camera_info.temp_status = status;

    return ( (status.compare("ERROR")==0) ? ERROR : NO_ERROR );
  }
  /***** Andor::Interface::_GetTemperature ************************************/


  long Interface::_GetVersionInfo( AT_VersionInfoId arr, char* info, at_u32 len ) {
    std::string function = "Andor::Interface::_GetVersionInfo";
    std::stringstream message;
    unsigned int ret = GetVersionInfo( arr, info, len );
    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }

  /***** Andor::Interface::_Initialize ****************************************/
  /**
   * @brief      wrapper for Andor SDK Initialize checks return value
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_Initialize() {
    std::string function = "Andor::Interface::_Initialize";
    std::stringstream message;

    // Initialize the Andor SDK
    //
    unsigned int ret = Initialize( (char*)ANDOR_SDK );

    switch ( ret ) {
      case DRV_VXDNOTINSTALLED: message << "ERROR VxD not loaded";                             break;
      case DRV_INIERROR:        message << "ERROR Unable to load DETECTOR.INI";                break;
      case DRV_COFERROR:        message << "ERROR Unable to load *.COF";                       break;
      case DRV_FLEXERROR:       message << "ERROR Unable to load *.RBF";                       break;
      case DRV_ERROR_ACK:       message << "ERROR Unable to communicate with device";          break;
      case DRV_ERROR_FILELOAD:  message << "ERROR Unable to load *.COF or *.RBF files";        break;
      case DRV_ERROR_PAGELOCK:  message << "ERROR Unable to acquire lock on requested memory"; break;
      case DRV_USBERROR:        message << "ERROR Unable to detect USB device or not USB2.0";  break;
      case DRV_ERROR_NOCAMERA:  message << "ERROR no camera found";                            break;
      default:                  message << "ERROR unrecognized return code: " << ret;          break;
    }
    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_Initialize ****************************************/


  /***** Andor::Interface::_SetAcquisitionMode ********************************/
  /**
   * @brief      wrapper for Andor SDK SetAcquisitionMode checks return value
   * @param[in]  mode  integer mode {1:5}
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * mode values are:
   * 1 = single scan
   * 2 = accumulate
   * 3 = kinetics
   * 4 = fast kinetics
   * 5 = run till abort
   *
   * Sets the class variable "mode" on success.
   *
   */
  long Interface::_SetAcquisitionMode( int mode ) {
    std::string function = "Andor::Interface::_SetAcquisitionMode";
    std::stringstream message;

    unsigned int ret = SetAcquisitionMode( mode );

    switch ( ret ) {
      case DRV_NOT_INITIALIZED: message << "ERROR system not initialized";                  break;
      case DRV_ACQUIRING:       message << "ERROR acquisition already in progress";         break;
      case DRV_P1INVALID:       message << "ERROR acquisition mode " << mode << " invalid"; break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }
    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );  // on error, log the error
    else {
      this->camera_info.mode = mode;                                // on success, save the mode to the class
      switch ( mode ) {
        case 1:  this->camera_info.modestr = "single scan";    break;
        case 2:  this->camera_info.modestr = "accumulate";     break;
        case 3:  this->camera_info.modestr = "kinetics";       break;
        case 4:  this->camera_info.modestr = "fast kinetics";  break;
        case 5:  this->camera_info.modestr = "run till abort"; break;
        default: this->camera_info.modestr = "unknown";        break;
      }
    }

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_SetAcquisitionMode ********************************/


  /***** Andor::Interface::_SetCurrentCamera **********************************/
  /**
   * @brief      wrapper for Andor SDK SetCurrentCamera checks return value
   * @param[in]  handle  camera handle from GetCameraHandle()
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_SetCurrentCamera( int handle ) {
    std::string function = "Andor::Interface::_SetCurrentCamera";
    std::stringstream message;

    unsigned int ret = SetCurrentCamera( handle );

    switch (ret) {
      case DRV_P1INVALID: message << "ERROR camera handle " << handle << " invalid"; break;
      default:            message << "ERROR: unknown error " << ret; break;
    }

    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_SetCurrentCamera **********************************/


  /***** Andor::Interface::_SetExposureTime ***********************************/
  /**
   * @brief      wrapper for Andor SDK SetExposureTime checks return value
   * @param[in]  exptime  exposure time in seconds
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_SetExposureTime( double exptime ) {
    std::string function = "Andor::Interface::_SetExposureTime";
    std::stringstream message;

    unsigned int ret = SetExposureTime( exptime );

    switch (ret) {
      case DRV_NOT_INITIALIZED:  message << "ERROR: system not initialized"; break;
      case DRV_ACQUIRING:        message << "ERROR: acquisition in progress"; break;
      case DRV_P1INVALID:        message << "ERROR: exposure time " << exptime << " invalid"; break;
      default:                   message << "ERROR: unknown error " << ret; break;
    }

    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_SetExposureTime ***********************************/


  /***** Andor::Interface::_SetImage ******************************************/
  /**
   * @brief      wrapper for Andor SDK SetImage checks return value
   * @param[in]  hbin    number of pixels to bin horizontally
   * @param[in]  vbin    number of pixels to bin vertically
   * @param[in]  hstart  start column, inclusive
   * @param[in]  hend    end column, inclusive
   * @param[in]  vstart  start row, inclusive
   * @param[in]  vend    end row, inclusive
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend ) {
    std::string function = "Andor::Interface::_SetImage";
    std::stringstream message;

    unsigned int ret = SetImage( hbin, vbin, hstart, hend, vstart, vend );

    switch (ret) {
      case DRV_NOT_INITIALIZED: message << "ERROR: system not initialized";    break;
      case DRV_ACQUIRING:       message << "ERROR: acquisition in progress";   break;
      case DRV_P1INVALID:
      case DRV_P2INVALID:       message << "ERROR: invalid binning parameter"; break;
      case DRV_P3INVALID:
      case DRV_P4INVALID:
      case DRV_P5INVALID:
      case DRV_P6INVALID:       message << "ERROR: invalid roi coordinate";    break;
      default:                  message << "ERROR: unknown error " << ret;     break;
    }

    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_SetImage ******************************************/


  /***** Andor::Interface::_SetReadMode ***************************************/
  /**
   * @brief      wrapper for Andor SDK SetShutter checks return value
   * @param[in]  mode
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * valid modes:
   *  0 = full vertical binning
   *  1 = multi-track
   *  2 = random-track
   *  3 = single-track
   *  4 = image
   *
   */
  long Interface::_SetReadMode( int mode ) {
    std::string function = "Andor::Interface::_SetReadMode";
    std::stringstream message;

    unsigned int ret = SetReadMode( mode );

    switch (ret) {
      case DRV_NOT_INITIALIZED:  message << "ERROR: system not initialized";     break;
      case DRV_ACQUIRING:        message << "ERROR: acquisition in progress";    break;
      case DRV_P1INVALID:        message << "ERROR: invalid read mode " << mode; break;
      default:                   message << "ERROR: unknown error " << ret;      break;
    }

    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_SetReadMode ***************************************/


  /***** Andor::Interface::_SetShutter ****************************************/
  /**
   * @brief      wrapper for Andor SDK SetShutter checks return value
   * @param[in]  type
   * @param[in]  mode
   * @param[in]  closetime
   * @param[in]  opentime
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_SetShutter( int type, int mode, int closetime, int opentime ) {
    std::string function = "Andor::Interface::_SetShutter";
    std::stringstream message;

    unsigned int ret = SetShutter( type, mode, closetime, opentime );

    switch (ret) {
      case DRV_NOT_INITIALIZED:  message << "ERROR: system not initialized";            break;
      case DRV_ACQUIRING:        message << "ERROR: acquisition in progress";           break;
      case DRV_ERROR_ACK:        message << "ERROR: unable to communicate with device"; break;
      case DRV_P1INVALID:        message << "ERROR: invalid type " << type;             break;
      case DRV_P2INVALID:        message << "ERROR: invalid mode " << mode;             break;
      case DRV_P3INVALID:        message << "ERROR: invalid open time " << opentime;    break;
      case DRV_P4INVALID:        message << "ERROR: invalid close time " << closetime;  break;
      default:                   message << "ERROR: unknown error " << ret;             break;
    }

    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_SetShutter ****************************************/


  /***** Andor::Interface::_StartAcquisition **********************************/
  /**
   * @brief      wrapper for Andor SDK StartAcquisition checks return value
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Interface::_StartAcquisition() {
    std::string function = "Andor::Interface::_StartAcquisition";
    std::stringstream message;

    unsigned int ret = StartAcquisition();

    switch ( ret ) {
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";                              break;
      case DRV_ACQUIRING:       message << "ERROR acquisition already in progress";              break;
      case DRV_VXDNOTINSTALLED: message << "ERROR VxD not loaded";                               break;
      case DRV_ERROR_ACK:       message << "ERROR unable to communicate with device";            break;
      case DRV_INIERROR:        message << "ERROR Error reading DETECTOR.INI";                   break;
      case DRV_ERROR_PAGELOCK:  message << "ERROR unable to allocate memory";                    break;
      case DRV_INVALID_FILTER:  message << "ERROR filter not available for current acquisition"; break;
      case DRV_BINNING_ERROR:   message << "ERROR range not multiple of horizontal binning";     break;
      case DRV_SPOOLSETUPERROR: message << "ERROR with spool settings";                          break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }
    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::Interface::_StartAcquisition **********************************/


  /***** Andor::Interface::open ***********************************************/
  /**
   * @brief      open a connection to the Andor
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( std::string args ) {
    std::string function = "Andor::Interface::open";
    std::stringstream message;
    long error=NO_ERROR;
    int ncameras;  // number of cameras
    int index=0;   // camera index given to 
    int handle=0;  // camera handle

    // If there was an argument then get the camera index from it
    //
    if ( !args.empty() ) {
      try { index = std::stoi( args ); }
      catch ( std::invalid_argument & ) {
        message.str(""); message << "ERROR: unable to convert requested index " << args << " to integer";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: requested index " << args << " outside integer range";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }
    else index=0;

    // Get the number of installed cameras.
    // This will be used to check the requested index and subsequently set
    // the active camera handle.
    //
    if ( _GetAvailableCameras( &ncameras ) == NO_ERROR ) {
      message.str(""); message << "detected " << ncameras << " cameras";
      logwrite( function, message.str() );
    }
    else logwrite( function, "ERROR reading number of cameras" );

    if ( index >= ncameras ) {
      message.str(""); message << "ERROR requested index " << index << " must be in range {0:" << (ncameras-1) << "}";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Set the current camera handle
    //
    if (error==NO_ERROR) error = _GetCameraHandle( index, &handle );
    if (error==NO_ERROR) error = _SetCurrentCamera( handle );
    if (error!=NO_ERROR) {
      message.str(""); message << "ERROR setting camera handle for camera " << index;
      logwrite( function, message.str() );
      return error;
    }

    char info[128];
    if ( _GetVersionInfo( AT_SDKVersion, info, 128 ) == NO_ERROR ) {
      this->camera_info.sdk_version = std::string(info);
      message.str(""); message << "SDK version " << this->camera_info.sdk_version;
      logwrite( function, message.str() );
    }

    // Initialize the Andor SDK
    //
    logwrite( function, "initializing Andor SDK" );

    if (error==NO_ERROR) error = _Initialize();

    if ( error != NO_ERROR ) {
      this->initialized = false;
      message.str(""); message << "ERROR initializing Andor SDK for camera " << index;
      logwrite( function, message.str() );
      return error;
    }

    if ( _GetVersionInfo( AT_DeviceDriverVersion, info, 128 ) == NO_ERROR ) {
      this->camera_info.driver_version = std::string(info);
      message.str(""); message << "Device Driver version " << this->camera_info.driver_version;
      logwrite( function, message.str() );
    }

    // The Andor SDK takes several seconds to initialize but returns
    // immediately, so one has to actually wait some period of time.
    //
    usleep( 2000000 );

    this->initialized = true;

    error = _GetCameraSerialNumber( &this->camera_info.serial_number );

    // put a bunch of stuff here in open() just for now, to be moved later
    //
    if (error==NO_ERROR) error = _SetReadMode( 4 );         // image mode
    if (error==NO_ERROR) error = _SetAcquisitionMode( 1 );  // single scan mode
    if (error==NO_ERROR) error = _SetExposureTime( 0.1 );
    if (error==NO_ERROR) error = _GetDetector( &this->camera_info.axes[0], &this->camera_info.axes[1] ); 
    if (error==NO_ERROR) error = _SetShutter( 1, 0, 50, 50 );  // TTL high fully auto shutter
    if (error==NO_ERROR) error = _SetImage( 1, 1, 1, this->camera_info.axes[0], 1, this->camera_info.axes[1] );  // no binning, full array

    message.str(""); message << "camera s/n " << this->camera_info.serial_number << " initialized";
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Andor::Interface::open ***********************************************/


  /***** Andor::Interface::close **********************************************/
  /**
   * @brief      close the connection to the Andor
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    std::string function = "Andor::Interface::close";
    std::stringstream message;

    ShutDown();
    this->initialized = false;

    return( NO_ERROR );
  }
  /***** Andor::Interface::close **********************************************/


  /***** Andor::Interface::start_acquisition **********************************/
  /**
   * @brief      start_acquisition
   * @return     ERROR or NO_ERROR
   *
   */
  unsigned int Interface::start_acquisition() {
    std::string function = "Andor::Interface::start_acquisition";
    std::stringstream message;

    long error = _StartAcquisition();

    message << ( error==NO_ERROR ? "acquisition started" : "ERROR starting acquisition" );
    logwrite( function, message.str() );

    return error;
  }
  /***** Andor::Interface::start_acquisition **********************************/


  /***** Andor::Interface::shutter ********************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::shutter() {
    std::string function = "Andor::Interface::shutter";
    std::stringstream message;

    long error = _SetShutter( 0, 2, 0, 0 );

    logwrite( function, "shutter closed" );

    return error;
  }
  /***** Andor::Interface::start_acquisition **********************************/


  /***** Andor::Interface::log_status *****************************************/
  /**
   * @brief      logs the current status of the Andor SDK
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::log_status() {
    std::string function = "Andor::Interface::log_status";
    std::stringstream message;

    int status;
    long error = _GetStatus( &status, message );

    logwrite( function, message.str() );

    return error;
  }
  /***** Andor::Interface::log_status *****************************************/


  /***** Andor::Interface::get_last_frame *************************************/
  /**
   * @brief      get_last_frame
   * @param[in]  buf   pointer to allocated buffer to hold the data
   * @param[in]  size  size of buffer in pixels
   * @return     ERROR or NO_ERROR
   *
   */
/***
  template <typename T>
  unsigned int Interface::get_last_frame( T* buf ) {
    std::string function = "Andor::Interface::get_last_frame";
    std::stringstream message;
    unsigned long ret;

    GetDetector( &this->camera_info.cols, &this->camera_info.rows );
    this->camera_info.npix = this->camera_info.cols * this->camera_info.rows;

    // Use the appropriate API call to get the acquired data
    // according to the type of pointer passed in.
    //
    if constexpr ( std::is_same_v<T, int32_t*> ) {
      ret = GetAcquiredData( buf, this->camera_info.npix );
    }
    else
    if constexpr ( std::is_same_v<T, int16_t*> ) {
      ret = GetAcquiredData16( buf, this->camera_info.npix );
    }
    else {
      ret = DRV_DATATYPE;
    }

    switch ( ret ) {
      case DRV_SUCCESS:
        message << "data acquired";
        break;
      case DRV_NOT_INITIALIZED:
        message << "ERROR not initialized";
        break;
      case DRV_ACQUIRING:
        message << "ERROR acquisition in progress";
        break;
      case DRV_ERROR_ACK:
        message << "ERROR unable to communicate with device";
        break;
      case DRV_P1INVALID:
        message << "ERROR invalid buffer pointer";
        break;
      case DRV_P2INVALID:
        message << "ERROR buffer size incorrect";
        break;
      case DRV_NO_NEW_DATA:
        message << "ERROR no acquisition";
        break;
      case DRV_DATATYPE:
        message << "ERROR unrecognized datatype";
        break;
      default:
        message << "ERROR unrecognized return code " << ret;
    }

    logwrite( function, message.str() );
    return ret;
  }
****/
  /***** Andor::Interface::get_last_frame *************************************/


  /***** Andor::Interface::acquire_one ****************************************/
  /**
   * @brief      
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::acquire_one() {
    std::string function = "Andor::Interface::acquire_one";
    std::stringstream message;
    long error = NO_ERROR;

    // Make sure the camera is in single scan mode
    //
    if ( this->camera_info.mode != 1 ) error = this->_SetAcquisitionMode( 1 );

    // Start Acquisition
    //
    if ( error==NO_ERROR ) error = this->start_acquisition();

    // Wait for acquisition
    //
    int status;
    this->_GetStatus( &status );
    while ( error==NO_ERROR && status == DRV_ACQUIRING ) error = this->_GetStatus( &status ); 

    // Get the acquired image
    //
    if ( this->image_data != NULL ) {
      delete [] this->image_data;
      this->image_data=NULL;
    }
    this->image_data = new uint16_t[ this->camera_info.axes[0] * this->camera_info.axes[1] ];

    if (error==NO_ERROR) error = _GetAcquiredData16( this->image_data, this->camera_info.axes[0] * this->camera_info.axes[1] );

    if (error==NO_ERROR) error = _GetTemperature();

    return error;
  }
  /***** Andor::Interface::acquire_one ****************************************/


  /***** Andor::Interface::save_acquired **************************************/
  /**
   * @brief      save an acquired image to FITS file
   * @param[in]  wcs_in   optional input image contains WCS header info to re-use
   * @param[out] imgname  output filename
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::save_acquired( std::string wcs_in, std::string &imgname ) {
    std::string function = "Andor::Interface::save_acquired";
    std::stringstream message;
    long error = NO_ERROR;

    if ( this->image_data == NULL ) {
      logwrite( function, "ERROR no image data available" );
      return( ERROR );
    }

    this->camera_info.fits_name = "/tmp/andor.fits";
    this->camera_info.datatype = USHORT_IMG;
    this->camera_info.section_size = this->camera_info.axes[0] * this->camera_info.axes[1];

    FITS_file fits_file( this->camera_info );   // instantiate a FITS_file object

    fits_file.open_file();                      // open the fits file for writing

    fits_file.copy_header( wcs_in );            // if supplied, copy the header from the input file

    fits_file.write_image( this->image_data );  // write the image data

    fits_file.close_file();                     // close the file

    imgname = this->camera_info.fits_name;      // return the name of the file created

    return error;
  }
  /***** Andor::Interface::save_acquired **************************************/


  /***** Andor::Interface::test ***********************************************/
  /**
   * @brief      test
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::test() {
    std::string function = "Andor::Interface::test";
    std::stringstream message;
    long error = NO_ERROR;

/***
this->camera_info.fits_name = "/tmp/andor.fits";
this->camera_info.datatype = USHORT_IMG;
this->camera_info.axes[0]=1024;
this->camera_info.axes[1]=1024;
this->camera_info.section_size = this->camera_info.axes[0] * this->camera_info.axes[1];

{
FITS_file fits_file( this->camera_info );

fits_file.open_file();

uint16_t* image_data = new uint16_t[ this->camera_info.axes[0] * this->camera_info.axes[1] ];
for (unsigned long int i=0; i<this->camera_info.section_size; i++) image_data[i]=i;
fits_file.write_image( image_data );
delete [] image_data;

fits_file.close_file();
}

return NO_ERROR;
***/

    int status;
    if (error==NO_ERROR) error = this->_SetExposureTime( this->camera_info.exposure_time );
    if (error==NO_ERROR) error = this->start_acquisition();
    uint16_t* image_data = new uint16_t[ this->camera_info.axes[0] * this->camera_info.axes[1] ];
    if (error==NO_ERROR) error = this->log_status( ); 
    this->_GetStatus( &status );
    while ( error==NO_ERROR && status == DRV_ACQUIRING ) error = this->_GetStatus( &status ); 
    if (error==NO_ERROR) error = _GetAcquiredData16( image_data, this->camera_info.axes[0] * this->camera_info.axes[1] );

    SaveAsFITS( (char*)"/tmp/andor.fits", 2 );
    delete [] image_data;

    return( NO_ERROR );
  }
  /***** Andor::Interface::test ***********************************************/


  /***** Andor::Interface::exptime ********************************************/
  /**
   * @brief      exptime
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::exptime( std::string exptime_in, std::string &retstring ) {
    std::string function = "Andor::Interface::exptime";
    std::stringstream message;
    double exptime_try;
    long error = NO_ERROR;

    // If an exposure time was passed in then
    // try to convert it (string) to a double
    //
    if ( ! exptime_in.empty() ) {
      try {
        if ( ( exptime_try = std::stod( exptime_in ) ) < 0 ) {
          logwrite( function, "ERROR: exptime must be >= 0" );
        }
        else {
          error = _SetExposureTime( exptime_try );
          if ( error == NO_ERROR ) this->camera_info.exposure_time = exptime_try;
        }
      }
      catch ( std::invalid_argument & ) {
        message.str(""); message << "ERROR: unable to convert exposure time: " << exptime_in << " to double";
        logwrite( function, message.str() );
        return( ERROR );
      }
      catch ( std::out_of_range & ) {
        message.str(""); message << "ERROR: exposure time " << exptime_in << " outside double range";
        logwrite( function, message.str() );
        return( ERROR );
      }
    }

    retstring = std::to_string( this->camera_info.exposure_time );

    message.str(""); message << "exposure time is " << retstring << " sec";
    logwrite(function, message.str());
    return( error );
  }
  /***** Andor::Interface::exptime ********************************************/


  /***** Andor::FITS_file::FITS_file ******************************************/
  /**
   * @brief      class constructor
   *
   */
  FITS_file::FITS_file() {
  }
  /***** Andor::FITS_file::FITS_file ******************************************/


  /***** Andor::FITS_file::FITS_file ******************************************/
  /**
   * @brief      class constructor
   *
   */
  FITS_file::FITS_file( Information info ) {
    this->info = info;
  }
  /***** Andor::FITS_file::FITS_file ******************************************/


  /***** Andor::FITS_file::FITS_file ******************************************/
  /**
   * @brief      class deconstructor
   *
   */
  FITS_file::~FITS_file() {
  }
  /***** Andor::FITS_file::FITS_file ******************************************/


  /***** Andor::FITS_file::open_file ******************************************/
  /**
   * @brief      
   *
   */
  long FITS_file::open_file() {
    std::string function = "Andor::FITS_file::open_file";
    std::stringstream message;

    int num_axis=2;
    long axes[2];
    axes[0] = (long)this->info.axes[0];
    axes[1] = (long)this->info.axes[1];

    // Check that we can write the file, because CCFits will crash if it cannot
    //
    std::ofstream checkfile ( this->info.fits_name.c_str() );
    if ( checkfile.is_open() ) {
      checkfile.close();
      std::remove( this->info.fits_name.c_str() );
    }
    else {
      message.str(""); message << "ERROR unable to create file \"" << this->info.fits_name << "\"";
      logwrite(function, message.str());
      return(ERROR);
    }

    try {
      // Create a new FITS object, specifying the data type and axes for the primary image.
      // Simultaneously create the corresponding file.
      //
      this->pFits.reset( new CCfits::FITS(this->info.fits_name, this->info.datatype, num_axis, axes) );
    }
    catch ( CCfits::FITS::CantCreate ){
      message.str(""); message << "ERROR: unable to open FITS file \"" << this->info.fits_name << "\"";
      logwrite(function, message.str());
      return(ERROR);
    }
    catch ( ... ) {
      message.str(""); message << "unknown error opening FITS file \"" << this->info.fits_name << "\"";
      logwrite(function, message.str());
      return(ERROR);
    }

    message.str(""); message << "opened file \"" << this->info.fits_name << "\" for FITS write";
    logwrite(function, message.str());

    this->fits_name = this->info.fits_name;

    return(NO_ERROR);
  }
  /***** Andor::FITS_file::open_file ******************************************/


  /***** Andor::FITS_file::copy_header ****************************************/
  /**
   * @brief      copy the header from another file
   * @param[in]  wcs_in  name of WCS input file from which to read header
   *
   */
  long FITS_file::copy_header( std::string wcs_in ) {
    std::string function = "Andor::FITS_file::copy_header";
    std::stringstream message;

    if ( wcs_in.empty() ) {
      logwrite( function, "no wcs file provided" );
      return( NO_ERROR );
    }
    else {
      message << "read WCS input file " << wcs_in;
      logwrite( function, message.str() );
    }

    std::unique_ptr<CCfits::FITS> pInfile( new CCfits::FITS( wcs_in, CCfits::Read, true ) );

    CCfits::PHDU* hdu = &pInfile->pHDU();

    hdu->readAllKeys();

    this->pFits->pHDU().copyAllKeys( hdu );

    this->pFits->pHDU().addKey( "CREATOR", "acamd", "file creator" );
    this->pFits->pHDU().addKey( "EXPTIME", this->info.exposure_time, "exposure time (sec)" );
    this->pFits->pHDU().addKey( "SERNO", this->info.serial_number, "camera serial number" );
    this->pFits->pHDU().addKey( "ACQMODEN", this->info.mode, "acquisition mode number" );
    this->pFits->pHDU().addKey( "ACQMODE", this->info.modestr, "acquisition mode" );
    this->pFits->pHDU().addKey( "TEMP", this->info.temp, "detector temperature" );
    this->pFits->pHDU().addKey( "TEMPSTAT", this->info.temp_status, "detector temperature status" );
    this->pFits->pHDU().addKey( "FITSNAME", this->info.fits_name, "this filename" );

    return( NO_ERROR );
  }
  /***** Andor::FITS_file::copy_header ****************************************/


  /***** Andor::FITS_file::write_image ****************************************/
  /**
   * @brief      closes fits file
   *
   * Before closing the file, DATE and CHECKSUM keywords are added.
   * Nothing called returns anything so this doesn't return anything.
   *
   */
  template <typename T>
  long FITS_file::write_image( T* data ) {
    std::string function = "Andor::FITS_file::write_image";
    std::stringstream message;

    // Set the FITS system to verbose mode so it writes error messages
    //
    CCfits::FITS::setVerboseMode( true );

    // write the primary image into the FITS file
    //
    try {
      std::valarray<T> array( this->info.section_size );
      for ( unsigned long i=0; i < this->info.section_size; i++ ) array[i] = data[i];
      long fpixel(1);        // start with the first pixel always
      this->pFits->pHDU().write( fpixel, this->info.section_size, array );
      this->pFits->flush();  // make sure the image is written to disk
    }
    catch ( CCfits::FitsError& error ) {
      message.str(""); message << "FITS file error thrown: " << error.message();
      logwrite(function, message.str());
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** Andor::FITS_file::write_image ****************************************/


  /***** Andor::FITS_file::close_file *****************************************/
  /**
   * @brief      closes fits file
   *
   * Before closing the file, DATE and CHECKSUM keywords are added.
   * Nothing called returns anything so this doesn't return anything.
   *
   */
  void FITS_file::close_file() {
    std::string function = "Andor::FITS_file::close_file";
    std::stringstream message;

    try {
      // Write the checksum
      //
      this->pFits->pHDU().writeChecksum();

      // Deallocate the CCfits object and close the FITS file
      //
      this->pFits->destroy();
    }
    catch ( CCfits::FitsError& error ) {
      message.str(""); message << "ERROR writing checksum and closing file: " << error.message();
      logwrite( function, message.str() );
    }

    message.str(""); message << this->fits_name << " closed";
    logwrite( function, message.str() );

    return;
  }
  /***** Andor::FITS_file::close_file *****************************************/


}
