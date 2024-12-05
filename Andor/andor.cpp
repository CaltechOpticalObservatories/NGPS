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

  /***** Andor::SDK::_GetCapabilities *****************************************/
  /**
   * @brief      wrapper for Andor SDK GetCapabilities
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetCapabilities( AndorCapabilities* caps ) {
    std::string function = "Andor::SDK::_GetCapabilities";
    std::stringstream message;

    caps->ulSize = sizeof(AndorCapabilities);

    unsigned int ret = GetCapabilities( caps );

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                               break;
      case DRV_NOT_INITIALIZED: message << "ERROR system not initialized";            break;
      case DRV_P1INVALID:       message << "ERROR invalid pointer";                   break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    if ( caps == nullptr ) {
      logwrite( function, "ERROR capabilities not returned" );
    }

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetCapabilities *****************************************/


  /***** Andor::SDK::_GetAcquiredData16 ***************************************/
  /**
   * @brief      wrapper for Andor SDK GetAcquiredData16
   * @details    16-bit version of GetAcquiredData. buf must be large enough
   *             to hold the complete data set.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetAcquiredData16( unsigned short* buf, at_u32 bufsize ) {
    std::string function = "Andor::SDK::_GetAcquiredData16";
    std::stringstream message;

    unsigned int ret = GetAcquiredData16( buf, bufsize );

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                               break;
      case DRV_NOT_INITIALIZED: message << "ERROR system not initialized";            break;
      case DRV_ACQUIRING:       message << "ERROR acquisition already in progress";   break;
      case DRV_VXDNOTINSTALLED: message << "ERROR VxD not loaded";                    break;
      case DRV_ERROR_ACK:       message << "ERROR unable to communicate with device"; break;
      case DRV_P1INVALID:       message << "ERROR invalid pointer";                   break;
      case DRV_P2INVALID:       message << "ERROR wrong array size";                  break;
      case DRV_NO_NEW_DATA:     message << "ERROR no acquisition has taken place";    break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    if ( buf == nullptr ) {
      logwrite( function, "ERROR buffer is empty" );
    }

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetAcquiredData16 ***************************************/


  /***** Andor::SDK::_GetMostRecentImage16 ************************************/
  /**
   * @brief      wrapper for Andor SDK GetMostRecentImage16
   * @details    16-bit version of GetMostRecentImage. buf must be large enough
   *             to hold the complete data set.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetMostRecentImage16( unsigned short* buf, at_u32 bufsize ) {
    std::string function = "Andor::SDK::_GetMostRecentImage16";
    std::stringstream message;

    unsigned int ret = GetMostRecentImage16( buf, bufsize );

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                               break;
      case DRV_NOT_INITIALIZED: message << "ERROR system not initialized";            break;
      case DRV_ERROR_ACK:       message << "ERROR unable to communicate with device"; break;
      case DRV_P1INVALID:       message << "ERROR invalid pointer";                   break;
      case DRV_P2INVALID:       message << "ERROR wrong array size";                  break;
      case DRV_NO_NEW_DATA:     message << "ERROR no acquisition has taken place";    break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    if ( buf == nullptr ) {
      logwrite( function, "ERROR buffer is empty" );
    }

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetMostRecentImage16 ************************************/


  /***** Andor::SDK::_GetAvailableCameras *************************************/
  /**
   * @brief      wrapper for Andor SDK GetAvailableCameras
   * @details    Returns the total number of installed Andor cameras.
   *             Can be called before any cameras are initialized.
   * @param[out] number  reference to integer to return number of cameras
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetAvailableCameras( at_32 &number ) {
    std::string function = "Andor::SDK::_GetAvailableCameras";
    std::stringstream message;

    unsigned int ret = GetAvailableCameras( &number );

    switch ( ret ) {
      case DRV_SUCCESS:         message << number;                                    break;
      case DRV_GENERAL_ERRORS:  message << "ERROR unable to get number of cameras";   break;
      default:                  message << "ERROR unrecognized return code: " << ret; break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetAvailableCameras *************************************/


  /***** Andor::SDK::_GetCameraHandle *****************************************/
  /**
   * @brief      wrapper for Andor SDK GetCameraHandle
   * @details    Returns the handle for the camera specified by index. When
   *             multiple Andor cameras are installed, the handle of each camera
   *             must be retrieved in order to select a camera using
   *             SetCurrentCamera.
   * @param[in]  index   index of installed, available camera
   * @param[out] handle  pointer to the handle received
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetCameraHandle( int index, at_32 &handle ) {
    std::string function = "Andor::SDK::_GetCameraHandle";
    std::stringstream message;

    unsigned int ret = GetCameraHandle( index, &handle );

    switch (ret) {
      case DRV_SUCCESS:         /* silent on success */                                  break;
      case DRV_P1INVALID:       message << "ERROR camera index " << index << " invalid"; break;
      default:                  message << "ERROR unrecognized return code: " << ret;    break;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetCameraHandle *****************************************/


  /***** Andor::SDK::_GetCameraSerialNumber ***********************************/
  /**
   * @brief      wrapper for Andor SDK GetCameraSerialNumber
   * @details    checks return value
   * @param[out] number
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetCameraSerialNumber( int &number ) {
    std::string function = "Andor::SDK::_GetCameraSerialNumber";
    std::stringstream message;

    unsigned int ret = GetCameraSerialNumber( &number );

    switch (ret) {
      case DRV_SUCCESS:         message << number;                                       break;
      case DRV_NOT_INITIALIZED: message << "ERROR: system not initialized";              break;
      default:                  message << "ERROR unrecognized return code: " << ret;    break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetCameraSerialNumber ***********************************/


  /***** Andor::SDK::_GetDetector *********************************************/
  /**
   * @brief      wrapper for Andor SDK GetDetector
   * @details    Returns the size of the detector in pixels
   * @param[out] xpix  number of horizontal pixels
   * @param[out] ypix  number of vertical pixels
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetDetector( int &xpix, int &ypix ) {
    std::string function = "Andor::SDK::_GetDetector";
    std::stringstream message;

    unsigned int ret = GetDetector( &xpix, &ypix );

    switch ( ret ) {
      case DRV_SUCCESS:         message << xpix << " " << ypix;                          break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";                      break;
      default:                  message << "ERROR unrecognized return code: " << ret;    break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetDetector *********************************************/


  /***** Andor::SDK::_GetStatus ***********************************************/
  /**
   * @brief      wrapper for Andor SDK GetStatus
   * @details    checks return value
   * @param[out] status  reference to string to contain the status message
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * This function is overloaded. This version does not provide the status message string.
   *
   */
  long SDK::_GetStatus( std::string &status ) {
    std::string function = "Andor::SDK::_GetStatus";
    int dontcare;
    return this->_GetStatus( dontcare, status );
  }
  /***** Andor::SDK::_GetStatus ***********************************************/


  /***** Andor::SDK::_GetStatus ***********************************************/
  /**
   * @brief      wrapper for Andor SDK GetStatus
   * @details    Return status of Andor SDK system. Should be called before an
   *             acquisition is started to ensure it is IDLE, and then call
   *             during acquisition to monitor progress.
   * @param[out] status_id   reference to int to contain the status ID code
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * This function is overloaded. This version does not provide the status message string.
   *
   */
  long SDK::_GetStatus( int &status_id ) {
    std::string function = "Andor::SDK::_GetStatus";
    std::string dontcare;
    return this->_GetStatus( status_id, dontcare );
  }
  /***** Andor::SDK::_GetStatus ***********************************************/


  /***** Andor::SDK::_GetStatus ***********************************************/
  /**
   * @brief      wrapper for Andor SDK GetStatus checks return value
   * @details    Return status of Andor SDK system. Should be called before an
   *             acquisition is started to ensure it is IDLE, and then call
   *             during acquisition to monitor progress.
   * @param[out] status_id   reference to int to contain the status ID code
   * @param[out] status_msg  reference to string to contain the status message text
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * This function is overloaded. This version provides the status message string.
   *
   */
  long SDK::_GetStatus( int &status_id, std::string &status_msg ) {
    std::string function = "Andor::SDK::_GetStatus";
    std::stringstream message;

    unsigned int ret = GetStatus( &status_id );

    switch ( ret ) {
      case DRV_SUCCESS:              /* silent on success */                                    break;
      case DRV_NOT_INITIALIZED:      status_msg = "ERROR not initialized";                      break;
      default:                       status_msg = "ERROR unrecognized return code " + std::to_string( ret );
    }
    switch ( status_id ) {
      case DRV_IDLE:                 status_msg = "IDLE";                                       break;
      case DRV_TEMPCYCLE:            status_msg = "executing temperature cycle";                break;
      case DRV_ACQUIRING:            status_msg = "acquisition in progress";                    break;
      case DRV_NOT_INITIALIZED:      status_msg = "ERROR not initialized";                      break;
      case DRV_ACCUM_TIME_NOT_MET:   status_msg = "ERROR unable to meet accumulate cycle time"; break;
      case DRV_KINETIC_TIME_NOT_MET: status_msg = "ERROR unable to meet kinetic cycle time";    break;
      case DRV_ERROR_ACK:            status_msg = "ERROR unable to communicate with card";      break;
      case DRV_ACQ_BUFFER:           status_msg = "ERROR unable to read data at required rate"; break;
      case DRV_SPOOLERROR:           status_msg = "ERROR spool buffer overflow";                break;
      default:                       status_msg = "ERROR unrecognized status code " + std::to_string( ret );
    }

    if ( status_msg.substr(0,5)=="ERROR" ) logwrite( function, status_msg );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetStatus ***********************************************/


  /***** Andor::SDK::_GetTotalNumberImagesAcquired ****************************/
  /**
   * @brief      wrapper for Andor SDK GetTotalNumberImagesAcquired
   * @details    This function will return the total number of images acquired
   *             since the current acquisition started. If the camera is idle
   *             the value returned is the number of images acquired during
   *             the last acquisition.
   * @param[out] index  reference to return number of images
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetTotalNumberImagesAcquired( at_32 &index ) {
    std::string function = "Andor::SDK::_GetTotalNumberImagesAcquired";
    std::stringstream message;

    unsigned int ret = GetTotalNumberImagesAcquired( &index );

    switch ( ret ) {
      case DRV_SUCCESS:         message << index; break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized"; break;
      default:                  message << "ERROR unrecognized status code " << ret;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetTotalNumberImagesAcquired ****************************/


  /***** Andor::SDK::_GetSizeOfCircularBuffer *********************************/
  /**
   * @brief      wrapper for Andor SDK GetSizeOfCircularBuffer
   * @details    This function will return the maximum number of images the
   *             circular buffer can store based on the current acquisition
   *             settings.
   * @param[out] index  reference to return size of buffer
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetSizeOfCircularBuffer( at_32 &index ) {
    std::string function = "Andor::SDK::_GetSizeOfCircularBuffer";
    std::stringstream message;

    unsigned int ret = GetSizeOfCircularBuffer( &index );

    switch ( ret ) {
      case DRV_SUCCESS:         message << index; break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized"; break;
      default:                  message << "ERROR unrecognized status code " << ret;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetSizeOfCircularBuffer *********************************/


  /***** Andor::SDK::_GetNumberADChannels *************************************/
  /**
   * @brief      wrapper for Andor SDK GetNumberADChannels
   * @details    returns the number of AD converter channels available
   * @param[out] channels  reference to return number of channels
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetNumberADChannels( int &channels ) {
    std::string function = "Andor::SDK::_GetNumberADChannels";
    std::stringstream message;

    unsigned int ret = GetNumberADChannels( &channels );

    switch ( ret ) {
      case DRV_SUCCESS: message << channels;
                        break;
      default:          message << "ERROR unrecognized status code " << ret;
                        break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetNumberADChannels *************************************/


  /***** Andor::SDK::_GetNumberHSSpeeds ***************************************/
  /**
   * @brief      wrapper for Andor SDK GetNumberHSSpeeds
   * @details    returns the number of horizontal shift speeds available
   * @param[in]  adchan  AD channel
   * @param[in]  type    output amplification type {0=EM 1=conventional}
   * @param[out] speeds  reference to int contains number of HS speeds
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetNumberHSSpeeds( int adchan, int type, int &speeds ) {
    std::string function = "Andor::SDK::_GetNumberHSSpeeds";
    std::stringstream message;

    unsigned int ret = GetNumberHSSpeeds( adchan, type, &speeds );

    switch ( ret ) {
      case DRV_SUCCESS:         message << speeds;                                   break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";                  break;
      case DRV_P1INVALID:       message << "ERROR invalid channel " << adchan;       break;
      case DRV_P2INVALID:       message << "ERROR invalid horiz read mode";          break;
      default:                  message << "ERROR unrecognized status code " << ret;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetNumberHSSpeeds ***************************************/


  /***** Andor::SDK::_GetNumberVSSpeeds ***************************************/
  /**
   * @brief      wrapper for Andor SDK GetNumberVSSpeeds
   * @details    returns the number of vertical shift speeds available
   * @param[out] speeds  reference to int contains number of VS speeds
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetNumberVSSpeeds( int &speeds ) {
    std::string function = "Andor::SDK::_GetNumberVSSpeeds";
    std::stringstream message;

    unsigned int ret = GetNumberVSSpeeds( &speeds );

    switch ( ret ) {
      case DRV_SUCCESS:         message << speeds;                                   break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";                  break;
      default:                  message << "ERROR unrecognized status code " << ret;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetNumberVSSpeeds ***************************************/


  /***** Andor::SDK::_GetHSSpeed **********************************************/
  /**
   * @brief      wrapper for Andor SDK GetHSSpeed
   * @details    returns the horizontal shift speed for specified amp and index
   * @param[in]  chan   AD channel
   * @param[in]  type   output amplification type {0=EM 1=conventional}
   * @param[in]  index
   * @param[out] speed  reference to float to contain speed in MHz
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetHSSpeed( int chan, int type, int index, float &speed ) {
    std::string function = "Andor::SDK::_GetHSSpeed";
    std::stringstream message;

    unsigned int ret = GetHSSpeed( chan, type, index, &speed );

    switch ( ret ) {
      case DRV_SUCCESS:         message << chan << " " << type << " " << speed;      break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";                  break;
      case DRV_P1INVALID:       message << "ERROR invalid AD chan " << chan;         break;
      case DRV_P2INVALID:       message << "ERROR invalid amp type " << type;        break;
      case DRV_P3INVALID:       message << "ERROR invalid index " << index;          break;
      default:                  message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetHSSpeed **********************************************/


  /***** Andor::SDK::_SetHSSpeed **********************************************/
  /**
   * @brief      wrapper for Andor SDK SetHSSpeed
   * @details    Set the speed at which pixels are shifted into output node
   *             during readout phase of an acquisition.
   * @param[in]  type   output amplification type {0=EM 1=conventional}
   * @param[in]  index  index of hsspeeds vector
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_SetHSSpeed( int type, int index ) {
    std::string function = "Andor::SDK::_SetHSSpeed";
    std::stringstream message;

    unsigned int ret = SetHSSpeed( type, index );

    switch ( ret ) {
      case DRV_SUCCESS:             /* silent on success */                              break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                  break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";          break;
      case DRV_P1INVALID:           message << "ERROR invalid amp type " << type;        break;
      case DRV_P2INVALID:           message << "ERROR invalid index " << index;          break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetHSSpeed **********************************************/


  /***** Andor::SDK::_GetVSSpeed **********************************************/
  /**
   * @brief      wrapper for Andor SDK GetVSSpeed
   * @details    returns the vertical shift speed for specified index
   * @param[in]  index
   * @param[out] speed  reference to float to contain speed in MHz
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetVSSpeed( int index, float &speed ) {
    std::string function = "Andor::SDK::_GetVSSpeed";
    std::stringstream message;

    unsigned int ret = GetVSSpeed( index, &speed );

    switch ( ret ) {
      case DRV_SUCCESS:         message << speed;                                    break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";                  break;
      case DRV_ACQUIRING:       message << "ERROR acquisition in progress";          break;
      case DRV_P1INVALID:       message << "ERROR invalid index " << index;          break;
      default:                  message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetVSSpeed **********************************************/


  /***** Andor::SDK::_SetVSSpeed **********************************************/
  /**
   * @brief      wrapper for Andor SDK SetVSSpeed
   * @details    set the vertical speed used for subsequent acquisitions
   * @param[in]  index  index of vsspeeds vector
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_SetVSSpeed( int index ) {
    std::string function = "Andor::SDK::_SetVSSpeed";
    std::stringstream message;

    unsigned int ret = SetVSSpeed( index );

    switch ( ret ) {
      case DRV_SUCCESS:             /* silent on success */                              break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                  break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";          break;
      case DRV_P1INVALID:           message << "ERROR invalid index " << index;          break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetVSSpeed **********************************************/


  /***** Andor::SDK::_SetPreAmpGain *******************************************/
  /**
   * @brief      wrapper for Andor SDK SetPreAmpGain
   * @details    This function will set the pre amp gain to be used for
   *             subsequent acquisitions. The actual gain factor that will be
   *             applied can be found through a call to the GetPreAmpGain
   *             function. The number of Pre Amp Gains available is found by
   *             calling the GetNumberPreAmpGains function.
   * @param[in]  index
   * @return     NO_ERROR | ERROR
   *
   */
  long SDK::_SetPreAmpGain( int index ) {
    std::string function = "Andor::SDK::_SetPreAmpGain";
    std::stringstream message;

    unsigned int ret = SetPreAmpGain( index );

    switch ( ret ) {
      case DRV_SUCCESS:             /* silent on success */                              break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                  break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";          break;
      case DRV_P1INVALID:           message << "ERROR invalid index " << index;          break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetPreAmpGain *******************************************/


  /***** Andor::SDK::_SetEMCCDGain ********************************************/
  /**
   * @brief      wrapper for Andor SDK SetEMCCDGain
   * @details    set gain value
   * @param[in]  gain  new gain
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_SetEMCCDGain( int gain ) {
    std::string function = "Andor::SDK::_SetEMCCDGain";
    std::stringstream message;

    unsigned int ret = SetEMCCDGain( gain );

    switch ( ret ) {
      case DRV_SUCCESS:             /* silent on success */                                         break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                             break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";                     break;
      case DRV_I2CTIMEOUT:          message << "ERROR I2C command timed out";                       break;
      case DRV_I2CDEVNOTFOUND:      message << "ERROR I2C device not present";                      break;
      case DRV_ERROR_ACK:           message << "ERROR Unable to communicate with device";           break;
      case DRV_P1INVALID:           message << "ERROR invalid gain " << gain;                       break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetEMCCDGain ********************************************/


  /***** Andor::SDK::_GetEMCCDGain ********************************************/
  /**
   * @brief      wrapper for Andor SDK GetEMCCDGain
   * @details    return gain setting (by reference) and save in class
   * @param[out] gain  reference to variable to contain gain setting
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetEMCCDGain( int &gain ) {
    std::string function = "Andor::SDK::_GetEMCCDGain";
    std::stringstream message;

    std::string status;

    unsigned int ret = GetEMCCDGain( &gain );

    switch ( ret ) {
      case DRV_SUCCESS:             /* silent on success */                     break;
      case DRV_NOT_INITIALIZED:     status = "ERROR not initialized";           break;
      case DRV_ERROR_ACK:           status = "ERROR communicating with device"; break;
      default:                      status = "ERROR unrecognized status code " + std::to_string( ret );
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetEMCCDGain ********************************************/


  /***** Andor::SDK::_GetEMGainRange ******************************************/
  /**
   * @brief      wrapper for Andor SDK GetEMGainRange
   * @details    Returns the minimum and maximum values of the current
   *             selected EM Gain mode and temperature of the sensor.
   * @param[out] temp  pointer to variable to contain current temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetEMGainRange( int &low, int &high ) {
    std::string function = "Andor::SDK::_GetEMGainRange";
    std::stringstream message;

    unsigned int ret = GetEMGainRange( &low, &high );

    switch ( ret ) {
      case DRV_SUCCESS:             message << low << " " << high;                       break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                  break;
      default:                      message << "ERROR unrecognized status code " << ret; break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetEMGainRange ******************************************/


  /***** Andor::SDK::_SetOutputAmplifier **************************************/
  /**
   * @brief      wrapper for Andor SDK SetOutputAmplifier
   * @details    set the type of amplifier to be used
   * @param[in]  type  amplifier type 0=EMCCD 1=conventional
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_SetOutputAmplifier( int type ) {
    std::string function = "Andor::SDK::_SetOutputAmplifier";
    std::stringstream message;

    unsigned int ret = SetOutputAmplifier( type );

    switch ( ret ) {
      case DRV_SUCCESS:          /* silent on success */                              break;
      case DRV_NOT_INITIALIZED:  message << "ERROR not initialized";                  break;
      case DRV_ACQUIRING:        message << "ERROR acquisition in progress";          break;
      case DRV_P1INVALID:        message << "ERROR invalid type " << type;            break;
      default:                   message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetOutputAmplifier **************************************/


  /***** Andor::SDK::_SetFrameTransferMode ************************************/
  /**
   * @brief      wrapper for Andor SDK SetFrameTransferMode
   * @param[out] mode  reference to variable to contain mode setting 1=on, 0=off
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_SetFrameTransferMode( int mode ) {
    std::string function = "Andor::SDK::_SetFrameTransferMode";
    std::stringstream message;

    std::string status;

    unsigned int ret = SetFrameTransferMode( mode );

    switch ( ret ) {
      case DRV_SUCCESS:          message << ( mode==1 ? "on" : "off" );      break;
      case DRV_NOT_INITIALIZED:  message << "ERROR not initialized";         break;
      case DRV_ACQUIRING:        message << "ERROR acquisition in progress"; break;
      case DRV_P1INVALID:        message << "ERROR invalid mode " << mode;   break;
      default:                   message << "ERROR unrecognized return code " << ret;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetFrameTransferMode ************************************/


  /***** Andor::SDK::_GetTemperature ******************************************/
  /**
   * @brief      wrapper for Andor SDK GetTemperature
   * @details    returns temperature of detector to the nearest degree C
   * @param[out] temp  pointer to variable to contain current temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetTemperature( int &temp, std::string &status ) {
    std::string function = "Andor::SDK::_GetTemperature";
    std::stringstream message;

    unsigned int ret = GetTemperature( &temp );

    switch ( ret ) {
      case DRV_NOT_INITIALIZED:     status = "ERROR not initialized";           break;
      case DRV_ACQUIRING:           status = "ERROR acquisition in progress";   break;
      case DRV_ERROR_ACK:           status = "ERROR communicating with device"; break;
      case DRV_TEMP_OFF:            status = "OFF";                             break;
      case DRV_TEMP_STABILIZED:     status = "STABILIZED";                      break;
      case DRV_TEMP_NOT_REACHED:    status = "NOT_REACHED";                     break;
      case DRV_TEMP_DRIFT:          status = "DRIFTED";                         break;
      case DRV_TEMP_NOT_STABILIZED: status = "NOT_STABLE";                      break;
      default:                      status = "ERROR unrecognized status code " + std::to_string( ret );
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( status.substr(0,5)=="ERROR" ? ERROR : NO_ERROR );
  }
  /***** Andor::SDK::_GetTemperature ******************************************/


  /***** Andor::SDK::_GetTemperatureRange *************************************/
  /**
   * @brief      wrapper for Andor SDK GetTemperature checks return value
   * @details    returns the valid range of temperatures
   * @param[out] min  reference to variable to contain minimum temperature
   * @param[out] max  reference to variable to contain maximum temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetTemperatureRange( int &min, int &max ) {
    std::string function = "Andor::SDK::_GetTemperatureRange";
    std::stringstream message;

    unsigned int ret = GetTemperatureRange( &min, &max );

    switch ( ret ) {
      case DRV_SUCCESS:             message << min << " " << max;                    break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";              break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";      break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetTemperatureRange *************************************/


  /***** Andor::SDK::_CoolerON ************************************************/
  /**
   * @brief      wrapper for Andor SDK CoolerON
   * @details    turns on the cooling
   * @details    turns the cooler on
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_CoolerON() {
    std::string function = "Andor::SDK::_CoolerON";
    std::stringstream message;

    unsigned int ret = CoolerON();

    switch ( ret ) {
      case DRV_SUCCESS:             message << "cooler is on";                             break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                    break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";            break;
      case DRV_ERROR_ACK:           message << "ERROR Unable to communicate with device";  break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_CoolerON ************************************************/


  /***** Andor::SDK::_CoolerOFF ***********************************************/
  /**
   * @brief      wrapper for Andor SDK CoolerOFF
   * @details    turns off the cooling
   * @details    turns the cooler off
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_CoolerOFF() {
    std::string function = "Andor::SDK::_CoolerOFF";
    std::stringstream message;

    unsigned int ret = CoolerOFF();

    switch ( ret ) {
      case DRV_SUCCESS:             message << "cooler is off";                            break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                    break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";            break;
      case DRV_ERROR_ACK:           message << "ERROR Unable to communicate with device";  break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_CoolerOFF ***********************************************/


  /***** Andor::SDK::_SetCoolerMode *******************************************/
  /**
   * @brief      wrapper for Andor SDK SetCoolerMode
   * @details    This function determines whether the cooler is switched off
   *             when the camera is shut down.
   * @param[in]  mode 1=temp maintained on shutdown, 0=return to ambient
   * @return     NO_ERROR | ERROR
   *
   */
  long SDK::_SetCoolerMode( int mode ) {
    std::string function = "Andor::SDK::_SetCoolerMode";
    std::stringstream message;

    unsigned int ret = SetCoolerMode( mode );

    switch ( ret ) {
      case DRV_SUCCESS:             /* silent on success */                                  break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                      break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";              break;
      case DRV_P1INVALID:           message << "ERROR invalid mode " << mode;                break;
      case DRV_NOT_SUPPORTED:       message << "ERROR camera doesn't support setting mode";  break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetCoolerMode *******************************************/


  /***** Andor::SDK::_SetTemperature ******************************************/
  /**
   * @brief      wrapper for Andor SDK SetTemperature
   * @details    set the temperature of the detector (does NOT control cooling)
   * @param[in]  temp  new temperature setpoint
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_SetTemperature( int temp ) {
    std::string function = "Andor::SDK::_SetTemperature";
    std::stringstream message;

    unsigned int ret = SetTemperature( temp );

    switch ( ret ) {
      case DRV_SUCCESS:             /* silent on success */                                         break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                             break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";                     break;
      case DRV_ERROR_ACK:           message << "ERROR Unable to communicate with device";           break;
      case DRV_P1INVALID:           message << "ERROR invalid temperature " << temp;                break;
      case DRV_NOT_SUPPORTED:       message << "ERROR camera doesn't support setting temperature";  break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetTemperature ******************************************/


  /***** Andor::SDK::_SetTriggerMode ******************************************/
  /**
   * @brief      wrapper for Andor SDK SetTriggerMode
   * @details    set the trigger mode
   * @param[in]  mode  trigger mode {0,1,6,7,9,10}
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_SetTriggerMode( int mode ) {
    std::string function = "Andor::SDK::_SetTriggerMode";
    std::stringstream message;

    unsigned int ret = SetTriggerMode( mode );

    switch ( ret ) {
      case DRV_SUCCESS:             /* silent on success */                               break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                   break;
      case DRV_ACQUIRING:           message << "ERROR acquisition in progress";           break;
      case DRV_P1INVALID:           message << "ERROR invalid trigger mode " << mode;     break;
      default:                      message << "ERROR unrecognized status code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetTriggerMode ******************************************/


  /***** Andor::SDK::_GetVersionInfo ******************************************/
  /**
   * @brief      wrapper for Andor SDK GetVersionInfo
   * @details    Retrieves version information about different aspects of the
   *             Andor system, copied into the passed buffer.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetVersionInfo( AT_VersionInfoId arr, char* info, at_u32 len ) {
    std::string function = "Andor::SDK::_GetVersionInfo";
    std::stringstream message;

    unsigned int ret = GetVersionInfo( arr, info, len );

    switch ( ret ) {
      case DRV_SUCCESS:                                                                             break;
      case DRV_NOT_INITIALIZED:     message << "ERROR not initialized";                             break;
      case DRV_P1INVALID:           message << "ERROR invalid information type";                    break;
      case DRV_P2INVALID:           message << "ERROR storage pointer is null";                     break;
      case DRV_P3INVALID:           message << "ERROR size of storage is zero";                     break;
      default:                      message << "ERROR unrecognized status code " << ret;            break;
    }

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetVersionInfo ******************************************/


  /***** Andor::SDK::_Initialize **********************************************/
  /**
   * @brief      wrapper for Andor SDK Initialize
   * @details    initialize the Andor SDK system
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_Initialize() {
    std::string function = "Andor::SDK::_Initialize";
    std::stringstream message;

    // Initialize the Andor SDK
    //
    unsigned int ret = Initialize( const_cast<char*>(ANDOR_SDK) );

    switch ( ret ) {
      case DRV_SUCCESS:         message << "success";                                          break;
      case DRV_VXDNOTINSTALLED: message << "ERROR VxD not loaded";                             break;
      case DRV_INIERROR:        message << "ERROR Unable to load DETECTOR.INI";                break;
      case DRV_COFERROR:        message << "ERROR Unable to load *.COF";                       break;
      case DRV_FLEXERROR:       message << "ERROR Unable to load *.RBF";                       break;
      case DRV_ERROR_ACK:       message << "ERROR Unable to communicate with device";          break;
      case DRV_ERROR_FILELOAD:  message << "ERROR Unable to load *.COF or *.RBF files";        break;
      case DRV_ERROR_PAGELOCK:  message << "ERROR Unable to acquire lock on requested memory"; break;
      case DRV_USBERROR:        message << "ERROR Unable to detect USB device or not USB2.0";  break;
      case DRV_ERROR_NOCAMERA:  message << "ERROR no camera found";                            break;

      // DRV_NOT_AVAILABLE is not a documented return code for Initialize() but it can
      // return this code and experience has shown that when it does, the only
      // recovery has been to power cycle the Andor camera.
      //
      case DRV_NOT_AVAILABLE:   message << "ERROR driver not available !!reboot camera!!";     break;

      default:                  message << "ERROR unrecognized return code: " << ret;          break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_Initialize **********************************************/


  /***** Andor::SDK::_SetAcquisitionMode **************************************/
  /**
   * @brief      wrapper for Andor SDK SetAcquisitionMode
   * @details    set acquisitiion mode to be used on next StartAcquisition
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
  long SDK::_SetAcquisitionMode( int mode ) {
    std::string function = "Andor::SDK::_SetAcquisitionMode";
    std::stringstream message;

    unsigned int ret = SetAcquisitionMode( mode );

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                                     break;
      case DRV_NOT_INITIALIZED: message << "ERROR system not initialized";                  break;
      case DRV_ACQUIRING:       message << "ERROR acquisition already in progress";         break;
      case DRV_P1INVALID:       message << "ERROR acquisition mode " << mode << " invalid"; break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetAcquisitionMode **************************************/


  /***** Andor::SDK::_SetCurrentCamera ****************************************/
  /**
   * @brief      wrapper for Andor SDK SetCurrentCamera
   * @details    When multiple cameras are installed this allows selecting which
   *             camera is active. Once selected, all other functions apply to
   *             the selected camera.
   * @param[in]  handle  camera handle from GetCameraHandle()
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetCurrentCamera( at_32 handle ) {
    std::string function = "Andor::SDK::_SetCurrentCamera";
    std::stringstream message;

    unsigned int ret = SetCurrentCamera( handle );

    switch (ret) {
      case DRV_SUCCESS:   /* silent on success */                                    break;
      case DRV_P1INVALID: message << "ERROR camera handle " << handle << " invalid"; break;
      default:            message << "ERROR: unknown error " << ret;                 break;
    }

    if ( !message.str().empty() ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetCurrentCamera ****************************************/


  /***** Andor::SDK::_SetExposureTime *****************************************/
  /**
   * @brief      wrapper for Andor SDK SetExposureTime
   * @details    Set the exposure time to the nearest valid value not less
   *             than the given value. Actual exposure time is obtained by
   *             GetAcquisitionTimings.
   * @param[in]  exptime  exposure time in seconds
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetExposureTime( float exptime ) {
    std::string function = "Andor::SDK::_SetExposureTime";
    std::stringstream message;

    unsigned int ret = SetExposureTime( exptime );

    switch (ret) {
      case DRV_SUCCESS:          /* silent on success */                                      break;
      case DRV_NOT_INITIALIZED:  message << "ERROR: system not initialized";                  break;
      case DRV_ACQUIRING:        message << "ERROR: acquisition in progress";                 break;
      case DRV_P1INVALID:        message << "ERROR: exposure time " << exptime << " invalid"; break;
      default:                   message << "ERROR: unknown error " << ret;                   break;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetExposureTime *****************************************/


  /***** Andor::SDK::_SetKineticCycleTime *************************************/
  /**
   * @brief      wrapper for Andor SDK SetKineticCycleTime 
   * @details    This function will set the kinetic cycle time to the nearest
   *             valid value not less than the given value. The actual time
   *             used is obtained by GetAcquisitionTimings.
   * @param[in]  time  kinetic cycle time in seconds
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetKineticCycleTime( float time ) {
    std::string function = "Andor::SDK::_SetKineticCycleTime";
    std::stringstream message;

    unsigned int ret = SetKineticCycleTime( time );

    switch (ret) {
      case DRV_SUCCESS:          message << time;                                  break;
      case DRV_NOT_INITIALIZED:  message << "ERROR: system not initialized";       break;
      case DRV_ACQUIRING:        message << "ERROR: acquisition in progress";      break;
      case DRV_P1INVALID:        message << "ERROR: time " << time << " invalid";  break;
      default:                   message << "ERROR: unknown error " << ret;        break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetKineticCycleTime *************************************/


  /***** Andor::SDK::_SetNumberAccumulations **********************************/
  /**
   * @brief      wrapper for Andor SDK SetNumberAccumulations
   * @details    This function will set the number of scans accumulated in memory.
   *             This will only take effect if the acquisition mode is either
   *             Accumulate or Kinetic Series.
   * @param[in]  number  number of scans to accumulate
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetNumberAccumulations( int number ) {
    std::string function = "Andor::SDK::_SetNumberAccumulations";
    std::stringstream message;

    unsigned int ret = SetNumberAccumulations( number );

    switch (ret) {
      case DRV_SUCCESS:          message << number;                                       break;
      case DRV_NOT_INITIALIZED:  message << "ERROR system not initialized";               break;
      case DRV_ACQUIRING:        message << "ERROR acquisition in progress";              break;
      case DRV_P1INVALID:        message << "ERROR accumulates " << number << " invalid"; break;
      default:                   message << "ERROR unknown error " << ret;                break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetNumberAccumulations **********************************/


  /***** Andor::SDK::_SetAccumulationCycleTime ********************************/
  /**
   * @brief      wrapper for Andor SDK SetAccumulationCycleTime
   * @details    This function will set the accumulation cycle time to the
   *             nearest valid value not less than the given value. The actual
   *             cycle time used is obtained by GetAcquisitionTimings.
   * @param[in]  time  accumulation time in seconds
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetAccumulationCycleTime( float time ) {
    std::string function = "Andor::SDK::_SetAccumulationCycleTime";
    std::stringstream message;

    unsigned int ret = SetAccumulationCycleTime( time );

    switch (ret) {
      case DRV_SUCCESS:          message << time;                             break;
      case DRV_NOT_INITIALIZED:  message << "ERROR system not initialized";   break;
      case DRV_ACQUIRING:        message << "ERROR acquisition in progress";  break;
      case DRV_P1INVALID:        message << "ERROR exposure time invalid";    break;
      default:                   message << "ERROR unknown error " << ret;    break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetAccumulationCycleTime ********************************/


  /***** Andor::SDK::_SetNumberKinetics ***************************************/
  /**
   * @brief      wrapper for Andor SDK SetNumberKinetics
   * @details    This function will set the number of scans (possibly
   *             accumulated scans) to be taken during a single acquisition
   *             sequence. This will only take effect if the acquisition mode
   *             is Kinetic Series.
   * @param[in]  number  number of scans to accumulate
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetNumberKinetics( int number ) {
    std::string function = "Andor::SDK::_SetNumberKinetics";
    std::stringstream message;

    unsigned int ret = SetNumberKinetics( number );

    switch (ret) {
      case DRV_SUCCESS:          message << number;                                  break;
      case DRV_NOT_INITIALIZED:  message << "ERROR system not initialized";          break;
      case DRV_ACQUIRING:        message << "ERROR acquisition in progress";         break;
      case DRV_P1INVALID:        message << "ERROR number " << number << " invalid"; break;
      default:                   message << "ERROR unknown error " << ret;           break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetNumberKinetics ***************************************/


  /***** Andor::SDK::_GetAcquisitionTimings ***********************************/
  /**
   * @brief      wrapper for Andor SDK GetAcquisitionTimings
   * @details    This function will return the current “valid” acquisition
   *             timing information. This function should be used after all
   *             the acquisitions settings have been set, e.g. SetExposureTime,
   *             SetKineticCycleTime and SetReadMode etc. The values returned
   *             are the actual times used in subsequent acquisitions.
   * @param[out] exp  valid exposure time in seconds
   * @param[out] acc  valid accumulate cycle time in seconds
   * @param[out] kin  valid kinetic cycle time in seconds
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetAcquisitionTimings( float &exp, float &acc, float &kin ) {
    std::string function = "Andor::SDK::_GetAcquisitionTimings";
    std::stringstream message;

    unsigned int ret = GetAcquisitionTimings( &exp, &acc, &kin );

    switch (ret) {
      case DRV_SUCCESS:          message << exp << " " << acc << " " << kin;        break;
      case DRV_NOT_INITIALIZED:  message << "ERROR system not initialized";         break;
      case DRV_ACQUIRING:        message << "ERROR acquisition in progress";        break;
      case DRV_INVALID_MODE:     message << "ERROR acq or read mode not available"; break;
      default:                   message << "ERROR unknown error " << ret;          break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetAcquisitionTimings ***********************************/


  /***** Andor::SDK::_SetImageFlip ********************************************/
  /**
   * @brief      wrapper for Andor SDK SetImageFlip
   * @details    This causes data output from SDK to be flipped in one or both
   *             axes. The flip is not done in the camera, it occurs after the
   *             data are retriever and will increase processing overhead.
   * @param[in]  hflip  1=enable horizontal flipping, 0=disable
   * @param[in]  vflip  1=enable vertical flipping, 0=disable
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetImageFlip( int hflip, int vflip ) {
    std::string function = "Andor::SDK::_SetImageFlip";
    std::stringstream message;

    unsigned int ret = SetImageFlip( hflip, vflip );

    switch (ret) {
      case DRV_SUCCESS:          message << hflip << " " << vflip;                  break;
      case DRV_NOT_INITIALIZED:  message << "ERROR: system not initialized";        break;
      case DRV_P1INVALID:        message << "ERROR: hflip " << hflip << " invalid"; break;
      case DRV_P2INVALID:        message << "ERROR: vflip " << vflip << " invalid"; break;
      default:                   message << "ERROR: unknown error " << ret;         break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetImageFlip ********************************************/


  /***** Andor::SDK::_SetImageRotate ******************************************/
  /**
   * @brief      wrapper for Andor SDK SetImageRotate
   * @details    This causes data output from SDK to be rotated in one or both
   *             axes. This rotate is not done in the camera, it occurs after
   *             the data are retrieved and will increase processing overhead.
   * @param[in]  rotdir  0=no rotation, 1=90 CW, 2=90 CCW
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetImageRotate( int rotdir ) {
    std::string function = "Andor::SDK::_SetImageRotate";
    std::stringstream message;

    unsigned int ret = SetImageRotate( rotdir );

    switch (ret) {
      case DRV_SUCCESS:          message << rotdir;                                         break;
      case DRV_NOT_INITIALIZED:  message << "ERROR: system not initialized";                break;
      case DRV_P1INVALID:        message << "ERROR: rotate param " << rotdir << " invalid"; break;
      default:                   message << "ERROR: unknown error " << ret;                 break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetImageRotate ******************************************/


  /***** Andor::SDK::_SetImage ************************************************/
  /**
   * @brief      wrapper for Andor SDK SetImage checks return value
   * @details    sets the horizontal and vertical binning when taking a
   *             full resolution image
   * @param[in]  hbin    number of pixels to bin horizontally
   * @param[in]  vbin    number of pixels to bin vertically
   * @param[in]  hstart  start column, inclusive
   * @param[in]  hend    end column, inclusive
   * @param[in]  vstart  start row, inclusive
   * @param[in]  vend    end row, inclusive
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend ) {
    std::string function = "Andor::SDK::_SetImage";
    std::stringstream message;

    unsigned int ret = SetImage( hbin, vbin, hstart, hend, vstart, vend );

    switch (ret) {
      case DRV_SUCCESS:         /* silent on success */                        break;
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

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetImage ************************************************/


  /***** Andor::SDK::_SetReadMode *********************************************/
  /**
   * @brief      wrapper for Andor SDK SetReadMode
   * @details    set the readout mode to be used on subsequent acquisitions
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
  long SDK::_SetReadMode( int mode ) {
    std::string function = "Andor::SDK::_SetReadMode";
    std::stringstream message;

    unsigned int ret = SetReadMode( mode );

    switch (ret) {
      case DRV_SUCCESS:         /* silent on success */                         break;
      case DRV_NOT_INITIALIZED: message << "ERROR: system not initialized";     break;
      case DRV_ACQUIRING:       message << "ERROR: acquisition in progress";    break;
      case DRV_P1INVALID:       message << "ERROR: invalid read mode " << mode; break;
      default:                  message << "ERROR: unknown error " << ret;      break;
    }

    if ( message.str().substr(0,5)=="ERROR" ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetReadMode *********************************************/


  /***** Andor::SDK::_GetShutterMinTimes **************************************/
  /**
   * @brief      wrapper for Andor SDK GetShutterMinTimes
   * @details    This function will return the minimum opening and closing times
   *             in milliseconds for the shutter on the current camera.
   * @param[out] minclosing  minimum supported closing time in milliseconds
   * @param[out] minopening  minimum supported opening time in milliseconds
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetShutterMinTimes( int &minclosing, int &minopening ) {
    std::string function = "Andor::SDK::_GetShutterMinTimes";
    std::stringstream message;

    unsigned int ret = GetShutterMinTimes( &minclosing, &minopening );

    switch (ret) {
      case DRV_SUCCESS:         message << minclosing << " " << minopening;  break;
      case DRV_NOT_INITIALIZED: message << "ERROR: system not initialized";  break;
      case DRV_ACQUIRING:       message << "ERROR: acquisition in progress"; break;
      case DRV_P1INVALID:       message << "ERROR: minclosingtime is NULL";  break;
      case DRV_P2INVALID:       message << "ERROR: minopeningtime is NULL";  break;
      default:                  message << "ERROR: unknown error " << ret;   break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetShutterMinTimes **************************************/


  /***** Andor::SDK::_SetShutter **********************************************/
  /**
   * @brief      wrapper for Andor SDK SetShutter
   * @details    controls behavior of the shutter
   * @param[in]  type
   * @param[in]  mode
   * @param[in]  closetime
   * @param[in]  opentime
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetShutter( int type, int mode, int closetime, int opentime ) {
    std::string function = "Andor::SDK::_SetShutter";
    std::stringstream message;

    unsigned int ret = SetShutter( type, mode, closetime, opentime );

    switch (ret) {
      case DRV_SUCCESS:          message << type << " " << mode << " " << closetime << " " << opentime;
                                 break;
      case DRV_NOT_INITIALIZED:  message << "ERROR: system not initialized";            break;
      case DRV_ACQUIRING:        message << "ERROR: acquisition in progress";           break;
      case DRV_ERROR_ACK:        message << "ERROR: unable to communicate with device"; break;
      case DRV_P1INVALID:        message << "ERROR: invalid type " << type;             break;
      case DRV_P2INVALID:        message << "ERROR: invalid mode " << mode;             break;
      case DRV_P3INVALID:        message << "ERROR: invalid open time " << opentime;    break;
      case DRV_P4INVALID:        message << "ERROR: invalid close time " << closetime;  break;
      default:                   message << "ERROR: unknown error " << ret;             break;
    }

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_SetShutter **********************************************/


  /***** Andor::SDK::_AbortAcquisition ****************************************/
  /**
   * @brief      wrapper for Andor SDK AbortAcquisition
   * @details    aborts the acquisition
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_AbortAcquisition() {
    std::string function = "Andor::SDK::_AbortAcquisition";
    std::stringstream message;

    unsigned int ret = AbortAcquisition();

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                                          break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";                              break;
      case DRV_IDLE:            message << "ERROR not acquiring";                                break;
      case DRV_VXDNOTINSTALLED: message << "ERROR VxD not loaded";                               break;
      case DRV_ERROR_ACK:       message << "ERROR unable to communicate with device";            break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }

    if ( ret != DRV_SUCCESS ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_AbortAcquisition ****************************************/


  /***** Andor::SDK::_StartAcquisition ****************************************/
  /**
   * @brief      wrapper for Andor SDK StartAcquisition
   * @details    starts the acquisition
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_StartAcquisition() {
    std::string function = "Andor::SDK::_StartAcquisition";
    std::stringstream message;

    unsigned int ret = StartAcquisition();

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                                          break;
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
  /***** Andor::SDK::_StartAcquisition ****************************************/


  /***** Andor::SDK::_WaitForAcquisition **************************************/
  /**
   * @brief      wrapper for Andor SDK WaitForAcquisition
   * @details    WaitForAcquisition can be called after an acquisition is started
   *             using StartAcquisition to put the calling thread to sleep until
   *             an Acquisition Event occurs. This can be used as a simple
   *             alternative to the functionality provided by the SetDriverEvent
   *             function, as all Event creation and handling is performed
   *             internally by the SDK library.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_WaitForAcquisition() {
    std::string function = "Andor::SDK::_WaitForAcquisition";
    std::stringstream message;

    unsigned int ret = WaitForAcquisition();

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                 break;
      case DRV_NO_NEW_DATA:     /* silent on cancel  */                 break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";     break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }

    if ( !message.str().empty() ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_WaitForAcquisition **************************************/


  /***** Andor::SDK::_WaitForAcquisitionTimeOut *******************************/
  /**
   * @brief      wrapper for Andor SDK WaitForAcquisitionTimeOut
   * @details    WaitForAcquisitionTimeOut can be called after an acquisition
   *             is started using StartAcquisition to put the calling thread
   *             to sleep until an Acquisition Event occurs.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_WaitForAcquisitionTimeOut( int timeout ) {
    std::string function = "Andor::SDK::_WaitForAcquisitionTimeOut";
    std::stringstream message;

    unsigned int ret = WaitForAcquisitionTimeOut(timeout);

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                 break;
      case DRV_NO_NEW_DATA:     /* silent on cancel  */                 break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }

    if ( !message.str().empty() ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_WaitForAcquisitionTimeOut *******************************/


  /***** Andor::SDK::_WaitForAcquisitionByHandleTimeOut ***********************/
  /**
   * @brief      wrapper for Andor SDK WaitForAcquisitionByHandleTimeOut
   * @details    Whilst using multiple cameras WaitForAcquisitionByHandle can
   *             be called after an acquisition is started using StartAcquisition
   *             to put the calling thread to sleep until an Acquisition Event occurs.
   * @param[in]  handle   camera handle
   * @param[in]  timeout  timeout in msec
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_WaitForAcquisitionByHandleTimeOut( at_32 handle, int timeout ) {
    std::string function = "Andor::SDK::_WaitForAcquisitionByHandleTimeOut";
    std::stringstream message;

    unsigned int ret = WaitForAcquisitionByHandleTimeOut( handle, timeout );

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                 break;
      case DRV_NO_NEW_DATA:     /* silent on cancel  */                 break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";     break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }

    if ( !message.str().empty() ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_WaitForAcquisitionByHandleTimeOut ***********************/


  /***** Andor::SDK::_SetFanMode **********************************************/
  /**
   * @brief      wrapper for Andor SDK SetFanMode
   * @details    Allows the user to control the mode of the camera fan. If the
   *             system is cooled, the fan should only be turned off for short
   *             periods of time. During thie time the body of the camera will
   *             warm up would could compromise cooling capabilities.
   * @param[in]  mode  fan mode {0,1,2}
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_SetFanMode( int mode ) {
    std::string function = "Andor::SDK::_SetFanMode";
    std::stringstream message;

    unsigned int ret = SetFanMode( mode );

    switch ( ret ) {
      case DRV_SUCCESS:         /* silent on success */                               break;
      case DRV_NOT_INITIALIZED: message << "ERROR not initialized";                   break;
      case DRV_ACQUIRING:       message << "ERROR acquisition already in progress";   break;
      case DRV_I2CTIMEOUT:      message << "ERROR I2C command timed out";             break;
      case DRV_I2CDEVNOTFOUND:  message << "ERROR I2C device not present";            break;
      case DRV_ERROR_ACK:       message << "ERROR Unable to communicate with device"; break;
      case DRV_P1INVALID:       message << "ERROR invalid mode " << mode;             break;
      default:                  message << "ERROR unrecognized return code " << ret;
    }

    if ( !message.str().empty() ) logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_WaitForAcquisitionByHandleTimeOut ***********************/


  /***** Andor::Interface::get_handlemap **************************************/
  /**
   * @brief      returns a map of camera handles indexed by serial number
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::get_handlemap( std::map<at_32, at_32> &handlemap ) {
    std::string function = "Andor::Interface::get_handlemap";
    std::stringstream message;

    at_32 ncameras=0;          // number of installed cameras
    char info[128];
    long error;

    // just FYI, really
    //
    error = ( andor ? andor->_GetVersionInfo( AT_SDKVersion, info, 128 ) : ERROR );
    if ( error == NO_ERROR ) {
      this->camera_info.sdk_version = std::string(info);
      message.str(""); message << "SDK version " << this->camera_info.sdk_version;
      logwrite( function, message.str() );
    }
    error = ( andor ? andor->_GetVersionInfo( AT_DeviceDriverVersion, info, 128 ) : ERROR );
    if ( error == NO_ERROR ) {
      this->camera_info.driver_version = std::string(info);
      message.str(""); message << "Device Driver version " << this->camera_info.driver_version;
      logwrite( function, message.str() );
    }

    // Get the number of installed cameras
    //
    error = ( andor ? andor->_GetAvailableCameras( ncameras ) : ERROR );
    if ( error == NO_ERROR ) {
      message.str(""); message << "detected " << ncameras << " cameras";
      logwrite( function, message.str() );
    }
    else logwrite( function, "ERROR reading number of cameras" );

    // Get the serial numbers of all installed cameras. To do that you must
    // set the handle and Initialize it; after the s/n has been read it
    // can be closed.
    //
    for ( at_32 index=0; index < ncameras; index++ ) {
      int sn;
      at_32 handle;
      if (error==NO_ERROR) error = ( andor ? andor->_GetCameraHandle( index, handle ) : ERROR );
      if (error==NO_ERROR) error = ( andor ? andor->_SetCurrentCamera( handle ) : ERROR );
      if (error==NO_ERROR) {
        long init = ( andor ? andor->_Initialize() : ERROR );
        // If this handle cannot be initialized then someone else is already using it
        // and it's not an option for me -- move on.
        //
        if (init==ERROR) {
          logwrite( function, "handle "+std::to_string(handle)+" busy, trying next handle");
          continue;
        }
      }
      // otherwise the handle has been Initialized so read the s/n
      //
      if (error==NO_ERROR) error = ( andor ? andor->_GetCameraSerialNumber( sn ) : ERROR );

      // save this handle to a map indexed by serial number
      //
      if (error==NO_ERROR) {
        handlemap[sn] = handle;
        message.str(""); message << "found s/n " << sn << " with handle " << handle;
        logwrite( function, message.str() );
      }

      // close this camera handle
      //
      ShutDown();
    }

    message.str(""); message << ( error ? "ERROR" : "successfully" )
                             << " mapped " << handlemap.size() << " camera handle(s)";
    logwrite( function, message.str() );

    return error;
  }
  /***** Andor::Interface::initialize_handles *********************************/


  /***** Andor::Interface::open ***********************************************/
  /**
   * @brief      open a connection to the Andor
   * @details    This opens a camera by handle so in order to know that, you need
   *             to have called Interface::initialize_handles() first.
   * @param[in]  handle_in  the camera handle to open
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( int handle_in ) {
    std::string function = "Andor::Interface::open";
    std::stringstream message;
    long error=NO_ERROR;

    this->_handle = handle_in;

    // Set the handle and initialize the SDK for this camera
    //
    if (error==NO_ERROR) error = ( andor ? andor->_SetCurrentCamera( this->_handle ) : ERROR );
    if (error==NO_ERROR) error = ( andor ? andor->_Initialize() : ERROR );
    if (error==NO_ERROR) error = ( andor ? andor->_GetCameraSerialNumber( this->camera_info.serial_number ) : ERROR );

    if (error!=NO_ERROR) {
      message.str(""); message << "ERROR initializing camera s/n " << serial << " handle " << this->_handle;
      logwrite( function, message.str() );
      return error;
    }

    this->is_andor_open = true;

    message.str(""); message << "opened s/n " << this->camera_info.serial_number << " handle " << this->_handle;
    logwrite( function, message.str() );

    // collect camera information, set unity gain, close shutter
    //
    if (error==NO_ERROR) error = ( andor ? andor->_GetDetector( this->camera_info.hend, this->camera_info.vend ) : ERROR );
    if (error==NO_ERROR) {
      this->camera_info.axes[0] = (this->camera_info.hend-this->camera_info.hstart+1)/this->camera_info.hbin;
      this->camera_info.axes[1] = (this->camera_info.vend-this->camera_info.vstart+1)/this->camera_info.vbin;
    }
    if (error==NO_ERROR) error = ( andor ? andor->_SetTriggerMode( 0 ) : ERROR );
    if (error==NO_ERROR) error = ( andor ? andor->_SetPreAmpGain( 0 ) : ERROR );
    if (error==NO_ERROR) error = ( andor ? andor->_SetCoolerMode( 1 ) : ERROR );
    if (error==NO_ERROR) error = set_image( this->camera_info.hbin, this->camera_info.vbin,
                                            1, this->camera_info.hend,
                                            1, this->camera_info.vend );
    if (error==NO_ERROR) error = ( andor ? andor->_GetNumberADChannels( this->camera_info.adchans ) : ERROR );
    if (error==NO_ERROR) error = get_speeds();
    if (error==NO_ERROR) error = set_output_amplifier( AMPTYPE_CONV );
    if (error==NO_ERROR)         this->camera_info.gain = 1;
    if (error==NO_ERROR) error = ( andor ? andor->_SetHSSpeed( this->camera_info.amptype, 0 ) : ERROR );

    if (error==NO_ERROR) error = ( andor ? andor->_GetTemperatureRange( this->camera_info.temp_min,
                                                                        this->camera_info.temp_max ) : ERROR );
    if (error==NO_ERROR) error = ( andor ? andor->_SetTemperature( this->camera_info.setpoint ) : ERROR );

    if (error==NO_ERROR) error = update_timings();
    if (error==NO_ERROR) error = set_shutter( "close" );

    message.str(""); message << "camera s/n " << this->camera_info.serial_number << " initialized";
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Andor::Interface::open ***********************************************/


  /***** Andor::Interface::close **********************************************/
  /**
   * @brief      close the connection to the Andor
   * @details    This will close the shutter and wait for the temp to be
   *             above -20 deg C, otherwise a shutdown will cause the detector
   *             temperature to rise faster than certified.
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close() {
    std::string function = "Andor::Interface::close";
    std::stringstream message;

    // closing the shutter will necessarily stop any acquisition in progress
    //
    this->set_shutter( "close" );

    // The temperature must be above -20 deg C before shutting down
    //
    int temp=999;
    long error = this->get_temperature( temp );

    // Turn off the cooler
    //
    if ( error==NO_ERROR &&
         temp >= this->camera_info.temp_min &&
         temp <= -20 ) {
      if ( andor ) {
        select_camera(this->_handle);
        andor->_SetTemperature( 20 );
        andor->_CoolerOFF();
      }
    }

//  // Wait for temperature to rise above -20
//  //
//  while ( error==NO_ERROR &&
//          temp >= this->camera_info.temp_min &&
//          temp <= -20 ) {
//    message.str(""); message << "shutdown waiting for temperature " << temp << " to be above -20 deg C";
//    logwrite( function, message.str() );
//    std::this_thread::sleep_for( std::chrono::seconds(10) );
//    error = this->get_temperature( temp );
//  }

    // close the AndorMCD system down
    //
    this->is_andor_open=false;
    ShutDown();

    std::lock_guard<std::mutex> lock(image_data_mutex);

    if ( this->image_data != nullptr ) {
      delete[] this->image_data;
      this->image_data = nullptr;
      this->camera_info.section_size = 0;
      this->bufsz = 0;
    }
    if ( this->avg_data != nullptr ) {
      delete[] this->avg_data;
      this->avg_data = nullptr;
      this->camera_info.section_size = 0;
      this->bufsz = 0;
    }

    return NO_ERROR;
  }
  /***** Andor::Interface::close **********************************************/


  /***** Andor::Interface::select_camera **************************************/
  /**
   * @brief      select camera by setting the camera handle
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::select_camera( at_32 handle ) {
    if ( ! this->is_andor_open ) {
      logwrite( "Andor::Interface::selct_camera", "ERROR camera handle "+std::to_string(handle)+" not open" );
      return ERROR;
    }
    return ( andor ? andor->_SetCurrentCamera( handle ) : ERROR );
  }
  /***** Andor::Interface::select_camera **************************************/


  /***** Andor::Interface::start_acquisition **********************************/
  /**
   * @brief      start acquisition
   * @return     ERROR or NO_ERROR
   *
   */
  unsigned int Interface::start_acquisition() {
    std::string function = "Andor::Interface::start_acquisition";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_StartAcquisition() : ERROR );

    if ( error != NO_ERROR ) logwrite( function, "ERROR starting acquisition" );

    return error;
  }
  /***** Andor::Interface::start_acquisition **********************************/


  /***** Andor::Interface::abort_acquisition **********************************/
  /**
   * @brief      abort acquisition if acquiring
   * @return     ERROR or NO_ERROR
   *
   */
  unsigned int Interface::abort_acquisition() {
    std::string function = "Andor::Interface::abort_acquisition";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    // Are we acquiring?
    //
    int status;
    long error = ( andor ? andor->_GetStatus( status ) : ERROR );
    if ( error==NO_ERROR && status != DRV_ACQUIRING ) {
      return NO_ERROR;
    }

    // yes, so abort
    //
    error |= ( andor ? andor->_AbortAcquisition() : ERROR );

    // make sure it aborted
    //
    error |= ( andor ? andor->_GetStatus( status ) : ERROR );
    while ( error==NO_ERROR && status == DRV_ACQUIRING ) error |= ( andor ? andor->_GetStatus( status ) : ERROR );

    if ( error != NO_ERROR ) logwrite( function, "ERROR aborting acquisition" );

    return error;
  }
  /***** Andor::Interface::abort_acquisition **********************************/


  /***** Andor::Interface::get_image_data *************************************/
  /**
   * @brief      returns a pointer to the image data
   * @return     uint16_t*
   *
   */
  uint16_t* Interface::get_image_data() {
    select_camera(this->_handle);
    return this->image_data;
  }
  /***** Andor::Interface::get_image_data *************************************/


  /***** Andor::Interface::get_avg_data ***************************************/
  /**
   * @brief      returns a pointer to the image data
   * @return     uint16_t*
   *
   */
  float* Interface::get_avg_data() {
    return this->avg_data;
  }
  /***** Andor::Interface::get_avg_data ***************************************/


  /***** Andor::Interface::set_shutter ****************************************/
  /**
   * @brief      set shutter open/close/auto by string
   * @param[in]  state  string can be { auto open close }
   * @return     ERROR or NO_ERROR
   *
   * This function is overridden
   *
   */
  long Interface::set_shutter( const std::string &state ) {
    std::string function = "Andor::Interface::set_shutter";
    std::stringstream message;
    long error;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( state != "auto" && state != "open" && state != "close" ) {
      logwrite( function, "ERROR expected { auto open close }" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    // stop acquiring
    //
    int status;
    error = ( andor ? andor->_GetStatus( status ) : ERROR );
    if ( error==NO_ERROR && status == DRV_ACQUIRING ) {
      error = ( andor ? andor->_AbortAcquisition() : ERROR );
    }

    // Get the minimum opening and closing times which are required
    // when setting the shutter to auto.
    //
    int closingtime, openingtime;
    error = ( andor ? andor->_GetShutterMinTimes( closingtime, openingtime ) : ERROR );

    // Set shutter based on requested state
    //
    if ( state == "auto" ) {
      error |= ( andor ? andor->_SetShutter( 1, 0, closingtime, openingtime ) : ERROR );
    }
    else
    if ( state == "open" ) {
      error |= ( andor ? andor->_SetShutter( 1, 1, 0, 0 ) : ERROR );
    }
    else
    if ( state == "close" ) {
      error |= ( andor ? andor->_SetShutter( 1, 2, 0, 0 ) : ERROR );
    }

    if ( error == NO_ERROR ) message << "shutter set to " << state;

    logwrite( function, message.str() );

    return error;
  }
  /***** Andor::Interface::set_shutter ****************************************/


  /***** Andor::Interface::set_shutter ****************************************/
  /**
   * @brief      wrapper provides full control setting shutter state
   * @param[in]  type         output TTL {0=low 1=high} signal to open shutter
   * @param[in]  mode         0=auto, 1=open, 2=close
   * @param[in]  closingtime  time shutter takes to close in msec
   * @param[in]  openingtime  time shutter takes to open in msec
   * @return     ERROR or NO_ERROR
   *
   * This function is overridden
   *
   */
  long Interface::set_shutter( const int type, const int mode, const int closingtime, const int openingtime ) {
    std::string function = "Andor::Interface::set_shutter";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // change inputs are within valid ranges
    //
    if ( type < 0 || type > 1 ) {
      message.str(""); message << "ERROR invalid type " << type << ": expected {0 1}";
      logwrite( function, message.str() );
      return ERROR;
    }
    if ( mode < 0 || mode > 2 ) {
      message.str(""); message << "ERROR invalid mode " << mode << ": expected {0 1 2}";
      logwrite( function, message.str() );
      return ERROR;
    }
    if ( closingtime<0 || openingtime<0 ) {
      message.str(""); message << "ERROR closing,opening times " << closingtime << "," << openingtime
                               << " must not be negative";
      logwrite( function, message.str() );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    // send the SetShutter command
    //
    long error = ( andor ? andor->_SetShutter( type, mode, closingtime, openingtime ) : ERROR );

    if ( error==ERROR ) logwrite( function, "ERROR setting shutter" );
    else logwrite( function, "shutter set" );

    return error;
  }
  /***** Andor::Interface::set_shutter ****************************************/


  /***** Andor::Interface::get_detector ***************************************/
  /**
   * @brief      get the size of the detector in pixels
   * @param[out] x  number of horizontal pixels
   * @param[out] y  number of vertical pixels
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_detector( int &x, int &y ) {
    std::string function = "Andor::Interface::get_detector";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    return ( andor ? andor->_GetDetector( x, y ) : ERROR );
  }
  /***** Andor::Interface::get_detector ***************************************/


  /***** Andor::Interface::get_status *****************************************/
  /**
   * @brief      get the current status from the Andor SDK
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_status() {
    std::string function = "Andor::Interface::get_status";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    std::string status;
    long error = NO_ERROR;

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    error |= ( andor ? andor->_GetStatus( status ) : ERROR );  // current status of Andor SDK

    int temp;
    error |= get_temperature(temp);

    logwrite( function, status );

    return error;
  }
  /***** Andor::Interface::get_status *****************************************/


  /***** Andor::Interface::get_buffer_counts **********************************/
  /**
   * @brief      get the horizontal and vertical shift speeds
   * @details    This reads all allowed clocking speeds and stores them in
   *             vectors in the class.
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_buffer_counts() {
    std::string function = "Andor::Interface::get_buffer_counts";
    std::stringstream message;

    long error = NO_ERROR;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    // Get Number of AD Chans and HS Speeds if needed
    //
    at_32 numbuf=-1;
    at_32 sizebuf=-1;
    error |= ( andor ? andor->_GetTotalNumberImagesAcquired( numbuf ) : ERROR );
    error |= ( andor ? andor->_GetSizeOfCircularBuffer( sizebuf ) : ERROR );

    message.str(""); message << "total acquired=" << numbuf << " bufsize=" << sizebuf;
    logwrite( function, message.str() );

    return error;
  }
  /***** Andor::Interface::get_buffer_counts **********************************/


  /***** Andor::Interface::get_speeds *****************************************/
  /**
   * @brief      get the horizontal and vertical shift speeds
   * @details    This reads all allowed clocking speeds and stores them in
   *             vectors in the class.
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_speeds() {
    std::string function = "Andor::Interface::get_speeds";
    std::stringstream message;

    long error = NO_ERROR;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    // Get Number of AD Chans and HS Speeds if needed
    //
    if ( error==NO_ERROR && this->camera_info.adchans < 0 ) {
      error |= ( andor ? andor->_GetNumberADChannels( this->camera_info.adchans ) : ERROR );
    }

    this->camera_info.hsspeeds.clear();

    int index=0;

    // get VS Speeds (independent of readout amp)
    //
    int num_vsspeeds=0;
    std::stringstream vmessage;
    vmessage << "           vert {";
    if ( (error=( andor ? andor->_GetNumberVSSpeeds( num_vsspeeds ) : ERROR ))==NO_ERROR ) {
      this->camera_info.vsspeeds.clear();
      for ( index = 0; index < num_vsspeeds; index++ ) {
        float speed;
        error |= ( andor ? andor->_GetVSSpeed( index, speed ) : ERROR );
        if ( error != NO_ERROR ) break;
        this->camera_info.vsspeeds.push_back( speed );
        vmessage << " " << std::to_string( speed );
      }
    }
    vmessage << " }";
    logwrite( function, vmessage.str() );

    int adchan = this->camera_info.adchan;

    // To get HS Speeds, loop through all (both) amp types
    //
    for ( int type = 0; type <= 1; type++ ) {

      std::stringstream hmessage;

      hmessage.str(""); hmessage << "amp type " << type << " hori {";

      // must get Number of Speeds for this type
      //
      int num_hsspeeds=0;
      if ( (error |= ( andor ? andor->_GetNumberHSSpeeds( this->camera_info.adchan, type, num_hsspeeds ) : ERROR ))==ERROR ) break;

      // loop through number of HS Speeds
      //
      this->camera_info.hsspeeds[type].clear();
      for ( index = 0; index < num_hsspeeds; index++ ) {
        float speed;
        error |= ( andor ? andor->_GetHSSpeed( adchan, type, index, speed ) : ERROR );
        if ( error != NO_ERROR ) break;
        this->camera_info.hsspeeds[type].push_back( speed );
        hmessage << " " << std::to_string( speed );
      }
      hmessage << " }";
      logwrite( function, hmessage.str() );
    }

    return error;
  }
  /***** Andor::Interface::get_speeds *****************************************/


  /***** Andor::Interface::hspeed *********************************************/
  /**
   * @brief      set/get the horizontal shift speed
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::hspeed( const std::string args, std::string &retstring ) {
    std::string function = "Andor::Interface::hspeed";
    std::stringstream message;
    long error = NO_ERROR;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // if an arg was supplied then convert it to float and use that to
    // set the hsspeed
    //
    if ( !args.empty() ) {
      try {
        error = this->set_hsspeed( std::stof( args ) );
      }
      catch( const std::exception &e ) {
        message.str(""); message << "ERROR parsing speed " << args << ": " << e.what();
        logwrite( function, message.str() );
        error = ERROR;
      }
    }

    // return the speed in MHz, whether or not an arg was supplied
    //
    message.str(""); message << this->camera_info.hspeed;
    retstring = message.str();
    message << " MHz";
    logwrite( function, message.str() );

    return error;
  }
  /***** Andor::Interface::hspeed *********************************************/


  /***** Andor::Interface::set_hsspeed ****************************************/
  /**
   * @brief      set horizontal shift speed
   * @param[in]  speed  speed in MHz, must be in hsspeeds vector for selected amptype
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_hsspeed( float speed ) {
    std::string function = "Andor::Interface::set_hsspeed";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // vector of HS speeds
    //
    auto vec = this->camera_info.hsspeeds[ this->camera_info.amptype ];

    // is requested speed in the vector of acceptable speeds?
    //
    auto it = std::find( vec.begin(), vec.end(), speed );
    if ( it == vec.end() ) {
      message.str(""); message << "ERROR HS speed " << speed << " not in {";
      for ( auto &sp : vec ) message << " " << sp;
      message << " }";
      logwrite( function, message.str() );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    // find the index of the speed in the vector
    //
    int index = std::distance( vec.begin(), it );

    long error = ( andor ? andor->_SetHSSpeed( this->camera_info.amptype, index ) : ERROR );

    if (error==NO_ERROR) {
      message.str(""); message << speed << " MHz";
      logwrite( function, message.str() );
      this->camera_info.hspeed = speed;
    }

    return error;
  }
  /***** Andor::Interface::set_hsspeed ****************************************/


  /***** Andor::Interface::set_vsspeed ****************************************/
  /**
   * @brief      set vertical shift speed
   * @param[in]  speed  speed in MHz, must be in vsspeeds vector
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_vsspeed( float speed_in ) {
    std::string function = "Andor::Interface::set_vsspeed";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // vector of VS speeds
    //
    auto vec = this->camera_info.vsspeeds;

    // is requested speed in the vector of acceptable speeds?
    //
    auto it = std::find( vec.begin(), vec.end(), speed_in );
    if ( it == vec.end() ) {
      message.str(""); message << "ERROR VS speed " << speed_in << " not in {";
      for ( auto &sp : vec ) message << " " << sp;
      message << " }";
      logwrite( function, message.str() );
      return ERROR;
    }

    // find the index of the speed in the vector
    //
    int index = std::distance( vec.begin(), it );

    // Check that the vector index is in range
    //
    if ( index < 0 || index >= (int)this->camera_info.vsspeeds.size() ) {
      message.str(""); message << "ERROR invalid index " << index;
      logwrite( function, message.str() );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    float speed = this->camera_info.vsspeeds[index];

    long error = ( andor ? andor->_SetVSSpeed( index ) : ERROR );

    if (error==NO_ERROR) {
      message.str(""); message << speed << " MHz";
      logwrite( function, message.str() );
      this->camera_info.vspeed = speed;
    }

    return error;
  }
  /***** Andor::Interface::set_vsspeed ****************************************/


  /***** Andor::Interface::set_image ******************************************/
  /**
   * @brief      set CCD image mode
   * @param[in]  hbin  number of pixels to bin horizontally
   * @param[in]  vbin  number of pixels to bin vertically
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_image( int hbin, int vbin, int hstart, int hend, int vstart, int vend ) {
    std::string function = "Andor::Interface::set_image";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( hbin < 1 || vbin < 1 || hstart < 1 || hend < 1 || vstart < 1 || vend < 1 ) {
      logwrite( function, "ERROR all image parameters must be >= 1" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_SetImage( hbin, vbin, hstart, hend, vstart, vend ) : ERROR );

    if ( error==NO_ERROR ) {
      message << hbin   << " " << vbin << " "
              << hstart << " " << hend << " "
              << vstart << " " << vend;
      logwrite( function, message.str() );
      this->camera_info.hbin   = hbin;
      this->camera_info.vbin   = vbin;
      this->camera_info.hstart = hstart;
      this->camera_info.hend   = hend;
      this->camera_info.vstart = vstart;
      this->camera_info.vend   = vend;
      this->camera_info.axes[0] = (hend-hstart+1)/hbin;
      this->camera_info.axes[1] = (vend-vstart+1)/vbin;
    }

    // allocate a buffer for saving images
    //
    std::lock_guard<std::mutex> lock(image_data_mutex);

    if ( this->image_data != nullptr ) {
      delete[] this->image_data;
      this->image_data = nullptr;
    }
    if ( this->avg_data != nullptr ) {
      delete[] this->avg_data;
      this->avg_data = nullptr;
    }

    this->bufsz = ( (hend-hstart+1)/hbin ) * ( (vend-vstart+1)/vbin );
    this->image_data = new uint16_t[ this->bufsz ];
    this->camera_info.section_size = this->bufsz;

    this->avg_data = new float[ this->bufsz ];
    this->num_avg_frames=1;

    message.str(""); message << "allocated " << this->bufsz << " bytes for image_data buffer";
    logwrite( function, message.str() );

    return error;
  }
  /***** Andor::Interface::set_image ******************************************/


  /***** Andor::Interface::set_fan ********************************************/
  /**
   * @brief      set camera fan
   * @param[in]  mode  fan mode 0=full, 1=low, 2=off
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_fan( int mode ) {
    std::string function = "Andor::Interface::set_fan";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( mode < 0 || mode < 2 ) {
      message.str(""); message << "ERROR " << mode << " outside range { 0 : 2 }";
      logwrite( function, message.str() );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_SetFanMode( mode ) : ERROR );

    // set class variable if successful
    //
    if ( error == NO_ERROR ) {
      switch ( mode ) {
        case 0:  this->camera_info.fan_mode = "full";
                 break;
        case 1:  this->camera_info.fan_mode = "low";
                 break;
        case 2:  this->camera_info.fan_mode = "off";
                 break;
        default: this->camera_info.fan_mode = "err";
      }
    }

    logwrite( function, this->camera_info.fan_mode );

    return error;
  }
  /***** Andor::Interface::set_fan ********************************************/


  /***** Andor::Interface::set_binning ****************************************/
  /**
   * @brief      set CCD binning parameters
   * @param[in]  hbin  number of pixels to bin horizontally
   * @param[in]  vbin  number of pixels to bin vertically
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_binning( int hbin, int vbin ) {
    std::string function = "Andor::Interface::set_binning";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( hbin < 1 || vbin < 1 ) {
      message.str(""); message << "ERROR " << hbin << " " << vbin << " must be > 0";
      logwrite( function, message.str() );
      return ERROR;
    }

    // Don't change ROI but use the existing values
    //
    int hstart = this->camera_info.hstart;
    int hend   = this->camera_info.hend;
    int vstart = this->camera_info.vstart;
    int vend   = this->camera_info.vend;

    // SetImage using new binning parameters
    //
    return set_image( hbin, vbin, hstart, hend, vstart, vend );
  }
  /***** Andor::Interface::set_binning ****************************************/


  /***** Andor::Interface::set_imflip *****************************************/
  /**
   * @brief      set CCD image flip
   * @param[in]  hflip  1=flip horizontally, 0=no flip
   * @param[in]  vflip  1=flip vertically, 0=no flip
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_imflip( int hflip, int vflip ) {
    std::string function = "Andor::Interface::set_imflip";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    long error = NO_ERROR;

    if ( hflip < 0 || hflip > 1 ) {
      message.str(""); message << "ERROR bad hflip " << hflip << ": expected { 0 1 }";
      logwrite( function, message.str() );
      error = ERROR;
    }

    if ( vflip < 0 || vflip > 1 ) {
      message.str(""); message << "ERROR bad vflip " << vflip << ": expected { 0 1 }";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    if (error==NO_ERROR) error = ( andor ? andor->_SetImageFlip( hflip, vflip ) : ERROR );
    if (error==NO_ERROR) {
      this->camera_info.hflip = hflip;
      this->camera_info.vflip = vflip;
    }

    return error;
  }
  /***** Andor::Interface::set_imflip *****************************************/


  /***** Andor::Interface::set_imrot ******************************************/
  /**
   * @brief      rotate CCD image
   * @param[in]  rotstr  accepts string { none cw ccw }
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_imrot( std::string rotstr ) {
    if ( rotstr == "none" ) return set_imrot(0);
    else
    if ( rotstr == "cw" )   return set_imrot(1);
    else
    if ( rotstr == "ccw" )  return set_imrot(2);
    else {
      logwrite( "Andor::Interface::set_imrot",
                "ERROR invalid arg "+rotstr+": expected { none cw ccw }" );
      return ERROR;
    }
  }
  /***** Andor::Interface::set_imrot ******************************************/
  /**
   * @brief      rotate CCD image
   * @param[in]  rotdir  0=no rotation, 1=90deg CW, 2=90deg CCW
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Interface::set_imrot( int rotdir ) {
    std::string function = "Andor::Interface::set_imrot";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    long error = NO_ERROR;

    switch( rotdir ) {
      case 0:   message << "rotate: none";       break;
      case 1:   message << "rotate: 90 deg CW";  break;
      case 2:   message << "rotate: 90 deg CCW"; break;
      default:  message << "ERROR unknown rotdir " << rotdir << ": expected { 0 1 2 }";
                error = ERROR;
                break;
    }
    logwrite( function, message.str() );

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    if (error==NO_ERROR) error = ( andor ? andor->_SetImageRotate( rotdir ) : ERROR );

    return error;
  }
  /***** Andor::Interface::set_imrot ******************************************/


  /***** Andor::Interface::get_emgain_range ***********************************/
  /**
   * @brief      get EM CCD gain Range
   * @param[out] low   reference to return lowest gain alloed
   * @param[out] high  reference to return highest gain allowed
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_emgain_range( int &low, int &high ) {
    std::string function = "Andor::Interface::get_emgain_range";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    return ( andor ? andor->_GetEMGainRange( low, high ) : ERROR );
  }
  /***** Andor::Interface::get_emgain_range ***********************************/


  /***** Andor::Interface::get_emgain *****************************************/
  /**
   * @brief      get EM CCD gain
   * @param[out] low   reference to return lowest gain alloed
   * @param[out] high  reference to return highest gain allowed
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_emgain( int &gain ) {
    std::string function = "Andor::Interface::get_emgain";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_GetEMCCDGain( gain ) : ERROR );

    if (error==NO_ERROR) {
      message.str(""); message << gain;
      logwrite( function, message.str() );
      this->camera_info.emgain = gain;
    }

    return error;
  }
  /***** Andor::Interface::get_emgain *****************************************/


  /***** Andor::Interface::set_output_amplifier *******************************/
  /**
   * @brief      set CCD output amplifier
   * @param[in]  type  amplifier type 0=EMCCD 1=conventional
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_output_amplifier( int type ) {
    std::string function = "Andor::Interface::set_output_amplifier";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( type != 0 && type != 1 ) {
      message.str(""); message << "ERROR invalid type " << type << ": expected { 0 1 }";
      logwrite( function, message.str() );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_SetOutputAmplifier( type ) : ERROR );

    if (error==NO_ERROR) {
      this->camera_info.amptype    = type;
      this->camera_info.amptypestr = ( type == Andor::AMPTYPE_EMCCD ? "EMCCD" : "conventional" );
      message.str(""); message << type << " (" << this->camera_info.amptypestr << ")";
      logwrite( function, message.str() );
    }

    return error;
  }
  /***** Andor::Interface::set_output_amplifier *******************************/


  /***** Andor::Interface::set_emgain *****************************************/
  /**
   * @brief      set EM CCD gain
   * @details    requested gain must be in allowable range from GetEMGainRange()
   * @param[in]  gain  integer gain to set
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_emgain( int gain ) {
    std::string function = "Andor::Interface::set_emgain";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    long error = NO_ERROR;

    int low, high;
    error = get_emgain_range( low, high );

    if (error==ERROR) return error;

    if ( gain < low || gain > high ) {
      message.str(""); message << "ERROR: gain " << gain << " outside range { "
                               << low << " " << high << " }";
      logwrite( function, message.str() );
      return ERROR;
    }
    else {
      // for multiple camera systems it is necessary to force the handle!
      //
      select_camera(this->_handle);

      error = ( andor ? andor->_SetEMCCDGain( gain ) : ERROR );
    }

    if (error==NO_ERROR) {
      this->camera_info.emgain = gain;
      message.str(""); message << gain;
      logwrite( function, message.str() );
    }

    return ( error );
  }
  /***** Andor::Interface::set_emgain *****************************************/


  /***** Andor::Interface::set_temperature ************************************/
  /**
   * @brief      set CCD temperature
   * @details    this will also enable/disable cooling as necessary
   * @param[in]  temp  temperature setpoint in integer degrees C
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_temperature( int temp ) {
    std::string function = "Andor::Interface::set_temperature";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    long error = NO_ERROR;

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    if ( temp >= this->camera_info.temp_max ) {
      error = ( andor ? andor->_CoolerOFF() : ERROR );
    }
    else {
      error = ( andor ? andor->_CoolerON() : ERROR );
    }

    if ( error==NO_ERROR ) {
      error = ( andor ? andor->_SetTemperature( temp ) : ERROR );
    }

    if ( error==NO_ERROR ) {
      message << "setpoint " << temp << " C";
      logwrite( function, message.str() );
      this->camera_info.setpoint = temp;
    }

    return ( error );
  }
  /***** Andor::Interface::set_temperature ************************************/


  /***** Andor::Interface::get_temperature ************************************/
  /**
   * @brief      read the CCD temperature into the class
   * @details    This version will read the temperature and store it in the
   *             class only; it does not return the temperature.
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   *
   */
  long Interface::get_temperature() {
    int dontcare;
    return get_temperature( dontcare );
  }
  /***** Andor::Interface::get_temperature ************************************/


  /***** Andor::Interface::get_temperature ************************************/
  /**
   * @brief      read and return the CCD temperature
   * @details    This version will return the current temperature by reference,
   *             as well as storing it in the class.
   * @param[in]  temp  reference to return temperature in integer degrees C
   * @return     ERROR or NO_ERROR
   *
   * This function is overloaded.
   *
   */
  long Interface::get_temperature( int &temp ) {
    std::string function = "Andor::Interface::get_temperature";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    std::string status;

    long error = ( andor ? andor->_GetTemperature( temp, status ) : ERROR );

    if (error==NO_ERROR) {
      message.str(""); message << temp << ": " << status;
      logwrite( function, message.str() );
      this->camera_info.temp_status = status;
      this->camera_info.ccdtemp     = temp;
    }

    return error;
  }
  /***** Andor::Interface::get_temperature ************************************/


  /***** Andor::Interface::set_kinetic_cycle_time *****************************/
  /**
   * @brief      set kinetic cycle time
   * @param[in]  time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_kinetic_cycle_time( float time ) {
    std::string function = "Andor::Interface::set_kinetic_cycle_time";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( std::isnan(time) || time < 0 ) {
      logwrite( function, "ERROR time can't be negative" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_SetKineticCycleTime( time ) : ERROR );

    // After chaning the kinetic cycle time the exposure timings must be updated.
    // This will update the Andor Information class.
    //
    error |= update_timings();

    return error;
  }
  /***** Andor::Interface::set_kinetic_cycle_time *****************************/


  /***** Andor::Interface::set_number_accumulations ***************************/
  /**
   * @brief      set number of accumulations
   * @param[in]  number  number of scans to accumulate
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_number_accumulations( int number ) {
    std::string function = "Andor::Interface::set_number_accumulations";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( number < 0 ) {
      logwrite( function, "ERROR number can't be negative" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_SetNumberAccumulations( number ) : ERROR );

    return error;
  }
  /***** Andor::Interface::set_number_accumulations ***************************/


  /***** Andor::Interface::set_accumulation_cycle_time ************************/
  /**
   * @brief      set accumulation cycle time
   * @param[in]  time  accumulation cycle time in seconds
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_accumulation_cycle_time( float time ) {
    std::string function = "Andor::Interface::set_accumulation_cycle_time";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( time < 0 ) {
      logwrite( function, "ERROR time can't be negative" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_SetAccumulationCycleTime( time ) : ERROR );

    return error;
  }
  /***** Andor::Interface::set_accumulation_cycle_time ************************/


  /***** Andor::Interface::set_number_kinetics ********************************/
  /**
   * @brief      set number of kinetic scans
   * @param[in]  number  number of scans to accumulate
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_number_kinetics( int number ) {
    std::string function = "Andor::Interface::set_number_kinetics";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( number < 0 ) {
      logwrite( function, "ERROR number can't be negative" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_SetNumberKinetics( number ) : ERROR );

    return error;
  }
  /***** Andor::Interface::set_number_kinetics ********************************/


  /***** Andor::Interface::set_frame_transfer *********************************/
  /**
   * @brief      set frame transfer mode
   * @param[in]  mode  on | off
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_frame_transfer( std::string mode ) {
    std::string function = "Andor::Interface::set_frame_transfer";
    std::stringstream message;
    int imode;

    // check input and camera open
    //
    if ( mode.empty() || ( mode != "on" && mode != "off" ) ) {
      logwrite( function, "ERROR expected on | off" );
      return ERROR;
    }
    else imode = ( mode == "on" ? 1 : 0 );

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    // Verify that FrameTransfer is supported
    //
    AndorCapabilities caps;
    long error = ( andor ? andor->_GetCapabilities( &caps ) : ERROR );
    if ( error == ERROR ) {
      logwrite( function, "ERROR unable to read Andor capabilities" );
      return ERROR;
    }

    if ( !(caps.ulFTReadModes & AC_ACQMODE_FRAMETRANSFER) ) {
      logwrite( function, "ERROR frame transfer not supported" );
      return ERROR;
    }

    // Set the mode now
    //
    error = ( andor ? andor->_SetFrameTransferMode( imode ) : ERROR );
    if ( error == ERROR ) {
      logwrite( function, "ERROR unable to set frame transfer mode" );
      return ERROR;
    }

    logwrite( function, mode );

    return NO_ERROR;
  }
  /***** Andor::Interface::set_frame_transfer *********************************/


  /***** Andor::Interface::set_read_mode **************************************/
  /**
   * @brief      set read mode
   * @param[in]  mode  integer mode {1:5}
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_read_mode( int mode ) {
    std::string function = "Andor::Interface::set_read_mode";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( mode < 0 || mode > 4 ) {
      message.str("");
      message << "ERROR invalid mode " << mode << ": expected range { 0:4 }";
      logwrite( function, message.str() );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_SetReadMode( mode ) : ERROR );

    if ( error == NO_ERROR ) {
      switch ( mode ) {
        case 0:  this->camera_info.readmodestr = "FULL_VERTICAL_BINNING"; break;
        case 1:  this->camera_info.readmodestr = "MULTI_TRACK";           break;
        case 2:  this->camera_info.readmodestr = "RANDOM_TRACK";          break;
        case 3:  this->camera_info.readmodestr = "SINGLE_TRACK";          break;
        case 4:  this->camera_info.readmodestr = "IMAGE";                 break;
        default: this->camera_info.readmodestr = "unknown";               break;
      }
      this->camera_info.readmode = mode;
    }

    // After chaning the read mode the exposure timings must be updated.
    // This will update the Andor Information class.
    //
    error |= update_timings();

    message << mode << " (" << this->camera_info.readmodestr << ")";

    return error;
  }
  /***** Andor::Interface::set_read_mode **************************************/


  /***** Andor::Interface::set_acquisition_mode *******************************/
  /**
   * @brief      set acquisition mode to be used on next start_acquisition
   * @param[in]  mode  integer mode {1:5}
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_acquisition_mode( int mode ) {
    std::string function = "Andor::Interface::set_acquisition_mode";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    if ( mode < 0 || mode > 5 ) {
      message.str(""); message << "ERROR invalid mode " << mode << ": expected { 1:5 }";
      logwrite( function, message.str() );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_SetAcquisitionMode( mode ) : ERROR );

    if ( error==NO_ERROR ) {
      this->camera_info.acqmode = mode;
      switch ( mode ) {
        case 1:  this->camera_info.acqmodestr = "SINGLE_SCAN";    break;
        case 2:  this->camera_info.acqmodestr = "ACCUMULATE";     break;
        case 3:  this->camera_info.acqmodestr = "KINETICS";       break;
        case 4:  this->camera_info.acqmodestr = "FAST_KINETICS";  break;
        case 5:  this->camera_info.acqmodestr = "RUN_TILL_ABORT"; break;
        default: this->camera_info.acqmodestr = "UNKNOWN";        break;
      }
      message << mode << " (" << this->camera_info.acqmodestr << ")";
      logwrite( function, message.str() );
    }

    return error;
  }
  /***** Andor::Interface::set_acquisition_mode *******************************/


  /***** Andor::Interface::wait_for_acquisition *******************************/
  /**
   * @brief      wait for acquisition on this camera handle, with timeout
   * @param[in]  timeout  timeout in msec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::wait_for_acquisition( int timeout ) {
    std::string function = "Andor::Interface::wait_for_acquisition";
    std::stringstream message;

    if ( timeout < 0 ) {
      logwrite( function, "ERROR timeout cannot be negative" );
      return ERROR;
    }

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_WaitForAcquisitionByHandleTimeOut( this->_handle, timeout ) : ERROR );

    return error;
  }
  /***** Andor::Interface::wait_for_acquisition *******************************/


  /***** Andor::Interface::get_recent *****************************************/
  /**
   * @brief      get the most recent image
   * @param[in]  timeout  timeout in msec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_recent( int timeout ) {
    std::string function = "Andor::Interface::get_recent";
    std::stringstream message;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // Make sure the camera is in single scan mode
    //
    if ( this->camera_info.acqmode != 5 ) {
      logwrite( function, "ERROR not in run till abort mode" );
      return ERROR;
    }

    // Wait for acquisition, this camera, with timeout
    //
    if ( this->wait_for_acquisition(timeout) != NO_ERROR ) {
      logwrite( function, "ERROR waiting for acquisition" );
      return ERROR;
    }

    // Get the acquired image
    //
    std::lock_guard<std::mutex> lock( image_data_mutex );
    if ( this->image_data == nullptr ) {
      logwrite( function, "ERROR image_data buffer not initialized: no image saved" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    long error = ( andor ? andor->_GetMostRecentImage16( this->image_data, this->bufsz ) : ERROR );
    if ( error == ERROR ) {
      logwrite( function, "ERROR getting most recent image" );
      return ERROR;
    }

    // get the temperature reading
    //
    if (error==NO_ERROR) error = ( andor ? andor->_GetTemperature( this->camera_info.ccdtemp,
                                                                   this->camera_info.temp_status ) : ERROR );

    // Store the exposure start time
    //
    timespec timenow             = Time::getTimeNow();         // get the time NOW
    this->camera_info.timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
    this->camera_info.mjd0       = mjd_from( timenow );        // modified Julian date

    return NO_ERROR;
  }
  /***** Andor::Interface::get_recent *****************************************/


  /***** Andor::Interface::get_single *****************************************/
  /**
   * @brief      get a single frame
   * @param[in]  timeout  timeout in msec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_single( int timeout ) {
    std::string function = "Andor::Interface::get_single";
    std::stringstream message;
    long error = NO_ERROR;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // Make sure the camera is in single scan mode
    //
    if ( this->camera_info.acqmode != 1 ) {
      logwrite( function, "ERROR not in single scan mode" );
      return ERROR;
    }

    // Wait for acquisition, this camera, with timeout
    //
    error = this->wait_for_acquisition(timeout);

    // Get the acquired image
    //
    std::lock_guard<std::mutex> lock( image_data_mutex );
    if ( this->image_data == nullptr ) {
      logwrite( function, "ERROR image_data buffer not initialized: no image saved" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    if (error==NO_ERROR) error = ( andor ? andor->_GetMostRecentImage16( this->image_data, this->bufsz ) : ERROR );

    // get the temperature reading
    //
    if (error==NO_ERROR) error = ( andor ? andor->_GetTemperature( this->camera_info.ccdtemp,
                                                                   this->camera_info.temp_status ) : ERROR );

    // Store the exposure start time
    //
    timespec timenow             = Time::getTimeNow();         // get the time NOW
    this->camera_info.timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
    this->camera_info.mjd0       = mjd_from( timenow );        // modified Julian date

    return error;
  }
  /***** Andor::Interface::get_single *****************************************/


  /***** Andor::Interface::acquire_one ****************************************/
  /**
   * @brief      acquire a single scan single frame
   * @details    the image is read here and stored in the class
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::acquire_one() {
    std::string function = "Andor::Interface::acquire_one";
    std::stringstream message;
    long error = NO_ERROR;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // Make sure the camera is in single scan mode
    //
    if ( this->camera_info.acqmode != 1 ) {
      message.str(""); message << "ERROR acquisition mode " << this->camera_info.acqmodestr
                               << " not in single scan mode";
      logwrite( function, message.str() );
      return ERROR;
    }

    // BoolState constructor sets is_acquiring (true)
    // and clears (false) when it goes out of scope.
    {
    BoolState acquire_state( this->is_acquiring );

    // Start Acquisition
    //
    error = this->start_acquisition();

    // Wait for acquisition (timeout set to 5s over exptime)
    //
    int timeout = static_cast<int>( (this->camera_info.exptime + 5.)*1000. );
    if (error==NO_ERROR) error = ( andor ? andor->_WaitForAcquisitionTimeOut( timeout ) : ERROR );
    }
    // ^^ acquire_state cleared upon leaving this scope ^^


    // Get the acquired image
    //
    std::lock_guard<std::mutex> lock( image_data_mutex );

    if ( this->image_data == nullptr || this->avg_data == nullptr ) {
      logwrite( function, "ERROR image_data buffer not initialized: no image saved" );
      return ERROR;
    }
    memset( this->image_data, '\0', this->bufsz*sizeof(uint16_t) );

    // get the data
    //
    if (error==NO_ERROR) error = ( andor ? andor->_GetAcquiredData16( this->image_data, this->bufsz ) : ERROR );

    float alpha = ( this->weight > 0 ? 1.0f/this->weight : 1.0f );

    if ( this->num_avg_frames == 1 ) {
      for ( size_t i=0; i<this->bufsz; ++i ) {
        this->avg_data[i] = static_cast<float>(this->image_data[i]);
      }
      this->num_avg_frames=2;
    }
    else if ( this->num_avg_frames==2 ) {
      for ( size_t i=0; i<this->bufsz; ++i ) {
        this->avg_data[i] = ( alpha * static_cast<float>(this->image_data[i]) + (1.0f-alpha)*this->avg_data[i] );
      }
    }

    // get the temperature reading
    //
    if (error==NO_ERROR) error = ( andor ? andor->_GetTemperature( this->camera_info.ccdtemp,
                                                                   this->camera_info.temp_status ) : ERROR );

    // Store the exposure start time
    //
    timespec timenow             = Time::getTimeNow();         // get the time NOW
    this->camera_info.timestring = timestamp_from( timenow );  // format that time as YYYY-MM-DDTHH:MM:SS.sss
    this->camera_info.mjd0       = mjd_from( timenow );        // modified Julian date

    return error;
  }
  /***** Andor::Interface::acquire_one ****************************************/


  /***** Andor::Interface::save_acquired **************************************/
  /**
   * @brief      save an acquired image to FITS file
   * @details    This saves the image stored in the class. See also acquire_one().
   *             This function is overloaded. This version uses hard-coded
   *             default values of simsize=1024
   * @param[in]  wcs_in   optional input image contains WCS header info to re-use
   * @param[out] imgname  output filename
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::save_acquired( std::string wcs_in, std::string &imgname ) {
    return( save_acquired( wcs_in, imgname, 1024 ) );
  }
  /***** Andor::Interface::save_acquired **************************************/


  /***** Andor::Interface::save_acquired **************************************/
  /**
   * @brief      save an acquired image to FITS file
   * @details    This saves the image stored in the class. See also acquire_one().
   *             This function is overloaded. This version accepts simsize as a
   *             free paramter which will be passed as keyword
   *             arguments to Python.
   * @param[in]  wcs_in      optional input image contains WCS header info to re-use
   * @param[out] imgname     output filename
   * @param[in]  simsize     IMAGE_SIZE keyword value for simFromHeader
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::save_acquired( std::string wcs_in, std::string &imgname, const int simsize ) {
    std::string function = "Andor::Interface::save_acquired";
    std::stringstream message;
    long error = NO_ERROR;

#ifdef LOGLEVEL_DEBUG
    message << "[DEBUG] wcs_in=" << wcs_in;
    logwrite( function, message.str() );
#endif

    std::lock_guard<std::mutex> lock( image_data_mutex );
    if ( this->image_data == nullptr ) {
      logwrite( function, "ERROR no image data available" );
      return ERROR;
    }

    this->camera_info.fits_name = "/tmp/andor.fits";
    this->camera_info.datatype = USHORT_IMG;
    this->camera_info.section_size = this->bufsz;

    FITS_file fits_file( this->camera_info );   // instantiate a FITS_file object

    fits_file.open_file();                      // open the fits file for writing

    get_temperature();

    fits_file.create_header();                  // create basic header

//  fits_file.copy_header( wcs_in );            // if supplied, copy the header from the input file
    fits_file.copy_header( "/home/developer/cshapiro/acam_skyinfo/skyheader.fits" );

    fits_file.write_image( this->image_data );  // write the image data

    fits_file.close_file();                     // close the file

    imgname = this->camera_info.fits_name;      // return the name of the file created

    // If Andor is emulated then the file just created is the input for the
    // sky simulator, which is called now to generate the simulated image.
    //
    if ( is_emulated() ) error = emulator.skysim.generate_image( imgname, "/tmp/andorout2.fits",
                                                                 emulator.get_exptime(),
                                                                 false,  // not multi-extension
                                                                 simsize );

    return error;
  }
  /***** Andor::Interface::save_acquired **************************************/


  /***** Andor::Interface::simulate_frame *************************************/
  /**
   * @brief      calls the skysim generator to create a simulated image
   * @details    This function is overloaded. This version uses hard-coded
   *             default values of ismex=false and simsize=1024
   * @param[in]  name_in  input to skysim, gets overwritten with simulated output
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::simulate_frame( std::string name_in ) {
    return( simulate_frame( name_in, false, 1024 ) );
  }
  /***** Andor::Interface::simulate_frame *************************************/


  /***** Andor::Interface::simulate_frame *************************************/
  /**
   * @brief      calls the skysim generator to create a simulated image
   * @details    This function is overloaded. This version accepts simsize as
   *             a free paramter which will be passed as a keyword
   *             argument to Python.
   * @param[in]  name_in     input to skysim, gets overwritten with simulated output
   * @param[in]  ismex       is multi-extension?
   * @param[in]  simsize     IMAGE_SIZE keyword value for simFromHeader
   * @return     ERROR | NO_ERROR
   *
   */
  long Interface::simulate_frame( std::string name_in, const bool ismex, const int simsize ) {
    std::string function = "Andor::Interface::simulate_frame";
    std::stringstream message;

    if ( is_emulated() ) {
      std::string simfile = generate_temp_filename( "skysim" );    // create a temporary filename for skysim output
      long error = emulator.skysim.generate_image( name_in, simfile,
                                                   emulator.get_exptime(),
                                                   ismex,
                                                   simsize );      // generate simulated image with temp filename
      if ( error == NO_ERROR ) {
        try {
          if ( !std::filesystem::exists( simfile ) ) {
            message.str(""); message << "ERROR temp image " << simfile << " not found";
            logwrite( function, message.str() );
            return ERROR;
          }
          // copy then remove this temp filename as input filename
          // (rename won't work across filesystems)
          //
          std::filesystem::copy( simfile, name_in, std::filesystem::copy_options::overwrite_existing );
          std::filesystem::remove( simfile );                      // remove skysim file
          simfile.append(".list");                                 // byproduct file generated by skysim
          std::filesystem::remove( simfile );                      // remove skysim byproduct file
        }
        catch (std::filesystem::filesystem_error &e) {
          message.str(""); message << "ERROR removing temp image: " << e.what();
          logwrite( function, message.str() );
          return ERROR;
        }
      }
      else {
        logwrite( function, "ERROR generating skysim image" );
      }
      return error;
    }
    else {
      logwrite( function, "ERROR Andor emulator not enabled" );
      return ERROR;
    }
  }
  /***** Andor::Interface::simulate_frame *************************************/


  /***** Andor::Interface::test ***********************************************/
  /**
   * @brief      test
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::test() {
/***
    std::string function = "Andor::Interface::test";
    std::stringstream message;
    long error = NO_ERROR;

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

/*** 4/3/24
    int status;
    if (error==NO_ERROR) error = ( andor ? andor->_SetExposureTime( this->camera_info.exposure_time ) : ERROR );
    if (error==NO_ERROR) error = this->start_acquisition();
    uint16_t* image_data = new uint16_t[ this->camera_info.axes[0] * this->camera_info.axes[1] ];
    if (error==NO_ERROR) error = this->get_status( );
    error = ( andor ? andor->_GetStatus( status ) : ERROR );
    while ( error==NO_ERROR && status == DRV_ACQUIRING ) error = ( andor ? andor->_GetStatus( status ) : ERROR );
    if (error==NO_ERROR) error = ( andor ? andor->_GetAcquiredData16( image_data, this->camera_info.axes[0] * this->camera_info.axes[1] ) : ERROR );

    SaveAsFITS( (char*)"/tmp/andor.fits", 2 );
    delete [] image_data;

4/3/24 ***/
    return NO_ERROR;
  }
  /***** Andor::Interface::test ***********************************************/


  /***** Andor::Interface::set_exptime ****************************************/
  /**
   * @brief      set the exposure time
   * @details    Consider the reference argument exptime a request for an
   *             exposure time, but not a guarantee. After setting the exposure
   *             time, the actual exposure time is read back from the Andor
   *             and returned in the supplied reference.
   *
   *             No changes can be made while an acquisition is in progress
   *             and this function does nothing to stop an acquisition but
   *             will return an error if acquiring.
   *
   * @param[in]  exptime  reference to float is updated with actual exptime
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_exptime( float &exptime ) {
    std::string function = "Andor::Interface::set_exptime";
    std::stringstream message;
    long error = NO_ERROR;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    // Can't change if acquisition in progress
    //
    int status;
    error = ( andor ? andor->_GetStatus( status ) : ERROR );
    if ( error==NO_ERROR && status==DRV_ACQUIRING ) {
      logwrite( function, "ERROR cannot change exposure time while acquiring" );
      return ERROR;
    }

    // simple zeroeth-order value check
    //
    if ( std::isnan(exptime) || exptime < 0 ) {
      logwrite( function, "ERROR requested exptime not a number" );
      return ERROR;
    }

    // set the exposure time
    //
    error = ( andor ? andor->_SetExposureTime( exptime ) : ERROR );

    // Now read the actual exposure time which updates the exptime reference
    //
    error |= read_exptime( exptime );

    this->camera_info.exptime = exptime;

    return error;
  }
  /***** Andor::Interface::set_exptime ****************************************/


  /***** Andor::Interface::update_timings *************************************/
  /**
   * @brief      update Andor Information class exptime, acctime, kintime
   * @details    This calls the read_exptime() overloaded function that reads
   *             the exposure timings from the Andor but doesn't return any
   *             values, only sets the class members.
   * @return     ERROR | NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Interface::update_timings() {
    float exptime, acctime, kintime;
    return this->read_exptime( exptime, acctime, kintime );
  }
  /***** Andor::Interface::update_timings *************************************/


  /***** Andor::Interface::read_exptime ***************************************/
  /**
   * @brief      read the actual exposure time from the Andor
   * @details    This calls the read_exptime() overloaded function that returns
   *             also the acctime and kintime, but accepts only an exptime
   *             reference for when only the exptime is desired.
   * @param[out] exptime  reference to return the exposure time (sec)
   * @return     ERROR | NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Interface::read_exptime( float &exptime ) {
    float acctime, kintime;
    return this->read_exptime( exptime, acctime, kintime );
  }
  /***** Andor::Interface::read_exptime ***************************************/


  /***** Andor::Interface::read_exptime ***************************************/
  /**
   * @brief      read actual exposure/accumulation/kinetic timings from the Andor
   * @param[out] exptime  reference to return the exposure time (sec)
   * @param[out] acctime  reference to return the accumulation cycle time (sec)
   * @param[out] kintime  reference to return the kinetic cycle time (sec)
   * @return     ERROR | NO_ERROR
   *
   * This function is overloaded
   *
   */
  long Interface::read_exptime( float &exptime, float &acctime, float &kintime ) {
    std::string function = "Andor::Interface::read_exptime";
    long error = NO_ERROR;

    if ( ! this->is_andor_open ) {
      logwrite( function, "ERROR camera not open" );
      return ERROR;
    }

    // for multiple camera systems it is necessary to force the handle!
    //
    select_camera(this->_handle);

    // Can't read if acquisition in progress
    //
    int status;
    error = ( andor ? andor->_GetStatus( status ) : ERROR );
    if ( error==NO_ERROR && status==DRV_ACQUIRING ) {
      logwrite( function, "ERROR cannot read while acquiring" );
      return ERROR;
    }

    // Read the exposure timings from the camera, which will be
    // stored in the Information class and returned in the ref args.
    //
    error |= ( andor ? andor->_GetAcquisitionTimings( this->camera_info.exptime,
                                                      this->camera_info.acctime,
                                                      this->camera_info.kintime ) : ERROR );

    // modify the values in the reference arguments
    //
    exptime = this->camera_info.exptime;
    acctime = this->camera_info.acctime;
    kintime = this->camera_info.kintime;

    return error;
  }
  /***** Andor::Interface::read_exptime ***************************************/


  /***** Andor::FITS_file::FITS_file ******************************************/
  /**
   * @brief      class constructor
   * @details    instantiated with existing Information object
   * @param[in]  info  Information object
   *
   */
  FITS_file::FITS_file( Information info ) {
    this->info = info;
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
    axes[0] = static_cast<long>(this->info.axes[0]);
    axes[1] = static_cast<long>(this->info.axes[1]);

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
      return ERROR;
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
      return ERROR;
    }
    catch ( ... ) {
      message.str(""); message << "unknown error opening FITS file \"" << this->info.fits_name << "\"";
      logwrite(function, message.str());
      return ERROR;
    }

    message.str(""); message << "opened file \"" << this->info.fits_name << "\" for FITS write";
    logwrite(function, message.str());

    this->fits_name = this->info.fits_name;

    return NO_ERROR;
  }
  /***** Andor::FITS_file::open_file ******************************************/


  /***** Andor::FITS_file::create_header **************************************/
  /**
   * @brief      create the header
   *
   */
  long FITS_file::create_header() {
    std::string function = "Andor::FITS_file::create_header";
    std::stringstream message;

    this->pFits->pHDU().addKey( "CREATOR", "acamd", "file creator" );
    this->pFits->pHDU().addKey( "INSTRUME", "NGPS", "name of instrument" );
    this->pFits->pHDU().addKey( "TELESCOP", "P200", "name of telescope" );
    this->pFits->pHDU().addKey( "EXPTIME", this->info.exptime, "exposure time (sec)" );
    this->pFits->pHDU().addKey( "SERNO", this->info.serial_number, "camera serial number" );
    this->pFits->pHDU().addKey( "ACQMODE", this->info.acqmodestr, "acquisition mode" );
    this->pFits->pHDU().addKey( "READMODE", this->info.readmodestr, "read mode" );
    this->pFits->pHDU().addKey( "TEMPSETP", this->info.setpoint, "detector temperature setpoint deg C" );
    this->pFits->pHDU().addKey( "TEMPREAD", this->info.ccdtemp, "CCD temperature deg C" );
    this->pFits->pHDU().addKey( "TEMPSTAT", this->info.temp_status, "CCD temperature status" );
    this->pFits->pHDU().addKey( "FITSNAME", this->info.fits_name, "this filename" );
    this->pFits->pHDU().addKey( "HBIN", this->info.hbin, "horizontal binning pixels" );
    this->pFits->pHDU().addKey( "VBIN", this->info.vbin, "vertical binning pixels" );
    this->pFits->pHDU().addKey( "HSPEED", this->info.hspeed, "horizontal clocking speed MHz" );
    this->pFits->pHDU().addKey( "VSPEED", this->info.vspeed, "vertical clocking speed MHz" );
    this->pFits->pHDU().addKey( "AMPTYPE", this->info.amptypestr, "CCD amplifier type" );
    this->pFits->pHDU().addKey( "CCDGAIN", this->info.gain, "CCD amplifier gain" );
    this->pFits->pHDU().addKey( "GAIN", "TBD", "e-/ADU" );

    // TBD
    //
    this->pFits->pHDU().addKey( "EXPSTART", "TBD", "exposure start time YYYY-MM-DDTHH:MM:SS.sss" );
    this->pFits->pHDU().addKey( "MJD0", "TBD", "exposure start time modified Julian date" );
    this->pFits->pHDU().addKey( "CASANGLE", "TBD", "TCS reported Cassegrain angle in deg" );
    this->pFits->pHDU().addKey( "AIRMASS0", "TBD", "TCS reported airmass at start of exposure" );
    this->pFits->pHDU().addKey( "HA", "TBD", "ACAM-center hour angle" );
    this->pFits->pHDU().addKey( "RA", "TBD", "ACAM-center right ascension J2000" );
    this->pFits->pHDU().addKey( "DEC", "TBD", "ACAM-center declination J2000" );
    this->pFits->pHDU().addKey( "RAOFFS", "TBD", "offset right ascension" );
    this->pFits->pHDU().addKey( "DECOFFS", "TBD", "offset declination" );
    this->pFits->pHDU().addKey( "TELRA", "TBD", "TCS reported right ascension" );
    this->pFits->pHDU().addKey( "TELDEC", "TBD", "TCS reported declination" );
    this->pFits->pHDU().addKey( "AZIMUTH", "TBD", "TCS reported azimuth" );
    this->pFits->pHDU().addKey( "ELEVATIO", "TBD", "TCS reported elevation" );
    this->pFits->pHDU().addKey( "FILTER", "TBD", "ACAM filter name" );
    this->pFits->pHDU().addKey( "SATURATE", "TBD", "saturated pixel value" );
    this->pFits->pHDU().addKey( "TELFOCUS", "TBD", "TCS reported focus position in mm" );

    this->pFits->pHDU().addKey( "RADESYSA", "FK5", "telescope pointing system type" );
    this->pFits->pHDU().addKey( "WCSAXES", 2, "number of axes in WCS description" );

    this->pFits->pHDU().addKey( "CRVAL1", "TBD", "display position X" );
    this->pFits->pHDU().addKey( "CRVAL2", "TBD", "display position Y" );
    this->pFits->pHDU().addKey( "CRPIX1", "TBD", "reference pixel X" );
    this->pFits->pHDU().addKey( "CRPIX2", "TBD", "reference pixel Y" );
    this->pFits->pHDU().addKey( "CUNIT1", "deg", "units of CRVAL and CDELT" );
    this->pFits->pHDU().addKey( "CUNIT2", "deg", "units of CRVAL and CDELT" );
    this->pFits->pHDU().addKey( "CTYPE1", "RA---TAN", "projection type" );
    this->pFits->pHDU().addKey( "CTYPE2", "DEC---TAN", "projection type" );
    this->pFits->pHDU().addKey( "PC1_1", "TBD", "rotation matrix element" );
    this->pFits->pHDU().addKey( "PC1_2", "TBD", "rotation matrix element" );
    this->pFits->pHDU().addKey( "PC2_1", "TBD", "rotation matrix element" );
    this->pFits->pHDU().addKey( "PC2_2", "TBD", "rotation matrix element" );
    this->pFits->pHDU().addKey( "CDELT1", "TBD", "pixel scale along axis" );
    this->pFits->pHDU().addKey( "CDELT2", "TBD", "pixel scale along axis" );
    this->pFits->pHDU().addKey( "PIXSCALE", this->info.pixel_scale, "arcsec per pixel" );
    this->pFits->pHDU().addKey( "POSANG", "TBD", "angle of image Y axis relative to North" );

    return NO_ERROR;
  }
  /***** Andor::FITS_file::create_header **************************************/


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
      return NO_ERROR;
    }
    else {
      message << "read WCS input file " << wcs_in;
      logwrite( function, message.str() );
    }

    std::unique_ptr<CCfits::FITS> pInfile( new CCfits::FITS( wcs_in, CCfits::Read, true ) );

    CCfits::PHDU* hdu = &pInfile->pHDU();

    hdu->readAllKeys();

#ifdef LOGLEVEL_DEBUG
    std::map<std::string, std::string> keys;
    for ( const auto &key : hdu->keyWord() ) {
      std::string value;
      message.str(""); message << "[DEBUG] " << key.first << " = " << key.second->value(value);
      logwrite( function, message.str() );
    }
#endif

    pInfile->pHDU().readAllKeys();

    this->pFits->pHDU().copyAllKeys( &pInfile->pHDU(), { TYP_WCS_KEY, TYP_REFSYS_KEY, TYP_USER_KEY } );

    return NO_ERROR;
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
      return ERROR;
    }

    return NO_ERROR;
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
