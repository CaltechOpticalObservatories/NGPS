/**
 * @file    andor.cpp
 * @brief   this file contains the code for the Andor interface
 * @author  
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
    this->exposure_time=0;
  }
  /***** Andor::Interface::Interface ******************************************/


  /***** Andor::Interface::open ***********************************************/
  /**
   * @brief      open a connection to the Andor
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open() {
    std::string function = "Andor::Interface::open";
    std::stringstream message;

    unsigned long ret;

    // Initialize the Andor SDK
    //
    logwrite( function, "initializing Andor SDK" );
    ret = Initialize( (char*)ANDOR_SDK );

    switch ( ret ) {
      case DRV_VXDNOTINSTALLED:
        message << "ERROR VxD not loaded";
        break;
      case DRV_INIERROR:
        message << "ERROR Unable to load DETECTOR.INI";
        break;
      case DRV_COFERROR:
        message << "ERROR Unable to load *.COF";
        break;
      case DRV_FLEXERROR:
        message << "ERROR Unable to load *.RBF";
        break;
      case DRV_ERROR_ACK:
        message << "ERROR Unable to communicate with device";
        break;
      case DRV_ERROR_FILELOAD:
        message << "ERROR Unable to load *.COF or *.RBF files";
        break;
      case DRV_ERROR_PAGELOCK:
        message << "ERROR Unable to acquire lock on requested memory";
        break;
      case DRV_USBERROR:
        message << "ERROR Unable to detect USB device or not USB2.0";
        break;
      case DRV_ERROR_NOCAMERA:
        message << "ERROR no camera found";
        break;
      default:
        message << "ERROR unrecognized return code: " << ret;
        break;
    }

    if ( ret != DRV_SUCCESS ) {
      this->initialized = false;
      logwrite( function, message.str() );
      return ERROR;
    }

    // The Andor SDK takes several seconds to initialize but returns
    // immediately, so one has to actually wait some period of time.
    //
    usleep( 2000000 );

    this->initialized = true;

    ret = GetCameraSerialNumber( &this->camera_info.serial_number );

    // put a bunch of stuff here in open() just for now, to be moved later
    //
    SetReadMode( 4 );         // image mode
    SetAcquisitionMode( 1 );  // single scan mode
    SetExposureTime( 0.1 );
    GetDetector( &this->camera_info.cols, &this->camera_info.rows );         // get size of detector in pixels
    SetShutter( 1, 0, 50, 50 );                                              // TTL high fully auto shutter
    SetImage( 1, 1, 1, this->camera_info.cols, 1, this->camera_info.rows );  // no binning, full array

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

    unsigned long ret = StartAcquisition();

    switch ( ret ) {
      case DRV_SUCCESS:
        message << "acquisition started";
        break;
      case DRV_NOT_INITIALIZED:
        message << "ERROR not initialized";
        break;
      case DRV_ACQUIRING:
        message << "ERROR acquisition already in progress";
        break;
      case DRV_VXDNOTINSTALLED:
        message << "ERROR VxD not loaded";
        break;
      case DRV_ERROR_ACK:
        message << "ERROR unable to communicate with device";
        break;
      case DRV_INIERROR:
        message << "ERROR Error reading DETECTOR.INI";
        break;
      case DRV_ERROR_PAGELOCK:
        message << "ERROR unable to allocate memory";
        break;
      case DRV_INVALID_FILTER:
        message << "ERROR filter not available for current acquisition";
        break;
      case DRV_BINNING_ERROR:
        message << "ERROR range not multiple of horizontal binning";
        break;
      case DRV_SPOOLSETUPERROR:
        message << "ERROR with spool settings";
        break;
      default:
        message << "ERROR unrecognized return code " << ret;
    }
    logwrite( function, message.str() );
    if ( ret == DRV_SUCCESS ) return NO_ERROR; else return ERROR;
  }
  /***** Andor::Interface::start_acquisition **********************************/


  /***** Andor::Interface::status_string **************************************/
  /**
   * @brief      
   * @return     
   *
   */
  std::string Interface::status_string( int status ) {
    std::string function = "Andor::Interface::status_string";
    std::stringstream message;

    switch ( status ) {
      case DRV_SUCCESS:         message << "SUCCESS";           break;
      case DRV_IDLE:            message << "IDLE";              break;
      case DRV_ACQUIRING:       message << "ACQUIRING";         break;
      case DRV_ERROR_ACK:       message << "ERROR_ACK";         break;
      case DRV_NOT_INITIALIZED: message << "NOT INITIALIZED";   break;
      default:                  message << "UNKNOWN";           break;
    }

    message << ": " << status;
    return message.str();
  }
  /***** Andor::Interface::status_string **************************************/


  /***** Andor::Interface::get_status *****************************************/
  /**
   * @brief      returns the current status of the Andor SDK
   * @param[in]  status  reference to int to contain Andor SDK status
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Interface::get_status( int &status ) {
    std::string function = "Andor::Interface::get_status";
    std::stringstream message;

    GetStatus( &status );

    logwrite( function, this->status_string( status ) );

    if ( status == DRV_SUCCESS ) return NO_ERROR; else return ERROR;
  }
  /***** Andor::Interface::get_status *****************************************/


  /***** Andor::Interface::get_status *****************************************/
  /**
   * @brief      returns the current status of the Andor SDK
   * @return     Andor SDK status
   *
   * This function is overloaded
   *
   */
  unsigned int Interface::get_status() {
    std::string function = "Andor::Interface::get_status";
    std::stringstream message;

    int status;
    GetStatus( &status );

    logwrite( function, this->status_string( status ) );

    return status;
  }
  /***** Andor::Interface::get_status *****************************************/


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

    SetAcquisitionMode( 1 );  // single scan mode
    return this->start_acquisition();
  }
  /***** Andor::Interface::acquire_one ****************************************/


  /***** Andor::Interface::test ***********************************************/
  /**
   * @brief      test
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::test() {
    std::string function = "Andor::Interface::test";
    std::stringstream message;

    int status;
    SetExposureTime( this->camera_info.exposure_time );
    StartAcquisition();
    int32_t* image_data = new int32_t[ this->camera_info.cols * this->camera_info.rows ];
    GetStatus( &status );
    while ( status == DRV_ACQUIRING ) GetStatus( &status );
    GetAcquiredData( image_data, this->camera_info.cols * this->camera_info.rows );

    SaveAsFITS( (char*)"/tmp/andor.fits", 1 );
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

    // If an exposure time was passed in then
    // try to convert it (string) to a double
    //
    if ( ! exptime_in.empty() ) {
      try {
        if ( ( exptime_try = std::stod( exptime_in ) ) < 0 ) {
          logwrite( function, "ERROR: exptime must be >= 0" );
        }
        else {
          this->camera_info.exposure_time = exptime_try;
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
    return( NO_ERROR );

  }
  /***** Andor::Interface::exptime ********************************************/

}
