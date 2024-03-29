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

  /***** Andor::Information::Information **************************************/
  /**
   * @brief      default Information class constructor
   *
   */
  Information::Information() {
    this->acqmode=-1;
    this->readmode=-1;
    this->adchan=0;
    this->adchans=-1;
    this->exposure_time=0;
    this->setpoint=20;
    this->emgain_low=-999;
    this->emgain_high=-999;
    this->hspeed=-999;
    this->vspeed=-999;
    this->gain=-999;
    this->hbin=1;
    this->vbin=1;
    this->hstart=1;
    this->vstart=1;
    this->hend=1;
    this->vend=1;
    this->hflip=0;
    this->vflip=0;
  }
  /***** Andor::Interface::Interface ******************************************/


  /***** Andor::SDK::_GetAcquiredData16 ***************************************/
  /**
   * @brief      wrapper for Andor SDK GetAcquiredData16
   * @details    16-bit version of GetAcquiredData. buf must be large enough
   *             to hold the complete data set.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetAcquiredData16( uint16_t* buf, unsigned long bufsize ) {
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

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_GetAcquiredData16 ***************************************/


  /***** Andor::SDK::_GetAvailableCameras *************************************/
  /**
   * @brief      wrapper for Andor SDK GetAvailableCameras
   * @details    Returns the total number of installed Andor cameras.
   *             Can be called before any cameras are initialized.
   * @param[out] number  reference to integer to return number of cameras
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long SDK::_GetAvailableCameras( int &number ) {
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
  long SDK::_GetCameraHandle( int index, int* handle ) {
    std::string function = "Andor::SDK::_GetCameraHandle";
    std::stringstream message;

    unsigned int ret = GetCameraHandle( index, handle );

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
   * @details    return minimum and maximum values for selected EM gain mode
   *             and temperature of the sensor
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


  /***** Andor::SDK::_GetTemperature ******************************************/
  /**
   * @brief      wrapper for Andor SDK GetTemperature
   * @details    returns temperature of detector to the nearest degree C
   * @param[out] temp  pointer to variable to contain current temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long SDK::_GetTemperature( int &temp, std::string_view &status ) {
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
    unsigned int ret = Initialize( (char*)ANDOR_SDK );

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
  long SDK::_SetCurrentCamera( int handle ) {
    std::string function = "Andor::SDK::_SetCurrentCamera";
    std::stringstream message;

    unsigned int ret = SetCurrentCamera( handle );

    switch (ret) {
      case DRV_SUCCESS:   /* silent on success */                                    break;
      case DRV_P1INVALID: message << "ERROR camera handle " << handle << " invalid"; break;
      default:            message << "ERROR: unknown error " << ret;                 break;
    }

    logwrite( function, message.str() );

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
  long SDK::_SetExposureTime( double exptime ) {
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

    logwrite( function, message.str() );

    return ( ret==DRV_SUCCESS ? NO_ERROR : ERROR );
  }
  /***** Andor::SDK::_StartAcquisition ****************************************/


  /***** Andor::Interface::open ***********************************************/
  /**
   * @brief      open a connection to the Andor
   * @param[in]  args  optional arg is the camera index to open { 0:ncameras-1 }
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( std::string args ) {
    std::string function = "Andor::Interface::open";
    std::stringstream message;
    long error=NO_ERROR;
    int ncameras=0;  // number of cameras
    int index=0;     // camera index given to
    int handle=0;    // camera handle

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
    error = ( andor ? andor->_GetAvailableCameras( ncameras ) : ERROR );
    if ( error == NO_ERROR ) {
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
    if (error==NO_ERROR) error = ( andor ? andor->_GetCameraHandle( index, &handle ) : ERROR );
    if (error==NO_ERROR) error = ( andor ? andor->_SetCurrentCamera( handle ) : ERROR );
    if (error!=NO_ERROR) {
      message.str(""); message << "ERROR setting camera handle for camera " << index;
      logwrite( function, message.str() );
      return error;
    }

    char info[128];
    error = ( andor ? andor->_GetVersionInfo( AT_SDKVersion, info, 128 ) : ERROR );
    if ( error == NO_ERROR ) {
      this->camera_info.sdk_version = std::string(info);
      message.str(""); message << "SDK version " << this->camera_info.sdk_version;
      logwrite( function, message.str() );
    }

    // Initialize the Andor SDK
    //
    logwrite( function, "initializing Andor SDK" );

    if (error==NO_ERROR) error = ( andor ? andor->_Initialize() : ERROR );

    if ( error != NO_ERROR ) {
      this->initialized = false;
      message.str(""); message << "ERROR initializing Andor SDK for camera " << index;
      logwrite( function, message.str() );
      return error;
    }

    error = ( andor ? andor->_GetVersionInfo( AT_DeviceDriverVersion, info, 128 ) : ERROR );
    if ( error == NO_ERROR ) {
      this->camera_info.driver_version = std::string(info);
      message.str(""); message << "Device Driver version " << this->camera_info.driver_version;
      logwrite( function, message.str() );
    }

    // The Andor SDK takes several seconds to initialize but returns
    // immediately, so one has to actually wait some period of time.
    //
    std::this_thread::sleep_for( std::chrono::seconds(3) );

    this->initialized = true;

    error = ( andor ? andor->_GetCameraSerialNumber( this->camera_info.serial_number ) : ERROR );

    // collect camera information
    //
//  if (error==NO_ERROR) error = sdk._GetDetector( this->camera_info.axes[0], this->camera_info.axes[1] );
//  if (error==NO_ERROR) {
//    this->camera_info.hend = this->camera_info.axes[0];
//    this->camera_info.vend = this->camera_info.axes[1];
//  }
    if (error==NO_ERROR) error = ( andor ? andor->_GetDetector( this->camera_info.hend, this->camera_info.vend ) : ERROR );
    if (error==NO_ERROR) {
      this->camera_info.axes[0] = this->camera_info.hend;
      this->camera_info.axes[1] = this->camera_info.vend;
    }
    if (error==NO_ERROR) error = set_image( 1, 1, 1, this->camera_info.axes[0], 1, this->camera_info.axes[1] );  // no binning, full array
    if (error==NO_ERROR) error = ( andor ? andor->_GetNumberADChannels( this->camera_info.adchans ) : ERROR );
    if (error==NO_ERROR) error = get_speeds();
    if (error==NO_ERROR) error = set_output_amplifier( AMPTYPE_CONV );
    if (error==NO_ERROR)         this->camera_info.gain = 1;
    if (error==NO_ERROR) error = ( andor ? andor->_SetHSSpeed( this->camera_info.amptype, 0 ) : ERROR );
    if (error==NO_ERROR) error = set_vsspeed(0.6);

    if (error==NO_ERROR) error = ( andor ? andor->_GetTemperatureRange( this->camera_info.temp_min, this->camera_info.temp_max ) : ERROR );
    if (error==NO_ERROR) error = ( andor ? andor->_SetTemperature( this->camera_info.setpoint ) : ERROR );

    // put a bunch of stuff here in open() just for now, to be moved later
    //
    if (error==NO_ERROR) error = set_read_mode( 4 );        // image mode
    if (error==NO_ERROR) error = set_acquisition_mode( 1 ); // single scan mode
    if (error==NO_ERROR) error = exptime( 0 );
    if (error==NO_ERROR) error = ( andor ? andor->_SetShutter( 1, 0, 50, 50 ) : ERROR );  // TTL high fully auto shutter

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
   * @brief      start acquisition
   * @return     ERROR or NO_ERROR
   *
   */
  unsigned int Interface::start_acquisition() {
    std::string function = "Andor::Interface::start_acquisition";
    std::stringstream message;

    long error = ( andor ? andor->_StartAcquisition() : ERROR );

    message << ( error==NO_ERROR ? "acquisition started" : "ERROR starting acquisition" );
    logwrite( function, message.str() );

    return error;
  }
  /***** Andor::Interface::start_acquisition **********************************/


  /***** Andor::Interface::shutter ********************************************/
  /**
   * @brief      right now this just closes the shutter
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::shutter() {
    std::string function = "Andor::Interface::shutter";
    std::stringstream message;

    long error = ( andor ? andor->_SetShutter( 0, 2, 0, 0 ) : ERROR );

    logwrite( function, "shutter closed" );

    return error;
  }
  /***** Andor::Interface::start_acquisition **********************************/


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

    std::string status;
    long error = NO_ERROR;

    error |= ( andor ? andor->_GetStatus( status ) : ERROR );  // current status of Andor SDK

    int temp;
    error |= get_temperature(temp);

    logwrite( function, status );

    return error;
  }
  /***** Andor::Interface::get_status *****************************************/


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

    if ( ! is_initialized() ) {
      logwrite( function, "ERROR camera not initialized" );
      return( ERROR );
    }

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
      return( ERROR );
    }

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
      return( ERROR );
    }

    // find the index of the speed in the vector
    //
    int index = std::distance( vec.begin(), it );

    // Check that the vector index is in range
    //
    if ( index < 0 || index >= (int)this->camera_info.vsspeeds.size() ) {
      message.str(""); message << "ERROR invalid index " << index;
      logwrite( function, message.str() );
      return( ERROR );
    }

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

    if ( hbin < 1 || vbin < 1 || hstart < 1 || hend < 1 || vstart < 1 || vend < 1 ) {
      logwrite( function, "ERROR all image parameters must be >= 1" );
      return( ERROR );
    }

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
    }

    return error;
  }
  /***** Andor::Interface::set_image ******************************************/


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

    if ( hbin < 1 || vbin < 1 ) {
      message.str(""); message << "ERROR " << hbin << " " << vbin << " must be > 0";
      logwrite( function, message.str() );
      return( ERROR );
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
   * @param[in]  rotdir  0=no rotation, 1=90deg CW, 2=90deg CCW
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_imrot( int rotdir ) {
    std::string function = "Andor::Interface::set_imrot";
    std::stringstream message;

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

    if ( type != 0 && type != 1 ) {
      message.str(""); message << "ERROR invalid type " << type << ": expected { 0 1 }";
      logwrite( function, message.str() );
      return( ERROR );
    }

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

    long error = NO_ERROR;

    int low, high;
    error = get_emgain_range( low, high );

    if (error==ERROR) return error;

    if ( gain < low || gain > high ) {
      message.str(""); message << "ERROR: gain " << gain << " outside range { "
                               << low << " " << high << " }";
      logwrite( function, message.str() );
      return( ERROR );
    }
    else {
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

    long error = NO_ERROR;

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

    std::string_view status;

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

    if ( mode < 0 || mode > 4 ) {
      message.str("");
      message << "ERROR invalid mode " << mode << ": expected range { 0:4 }";
      logwrite( function, message.str() );
      return( ERROR );
    }

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

    if ( mode < 0 || mode > 5 ) {
      message.str(""); message << "ERROR invalid mode " << mode << ": expected { 1:5 }";
      logwrite( function, message.str() );
      return( ERROR );
    }

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

    GetDetector( this->camera_info.cols, this->camera_info.rows );
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
   * @brief      acquire a single scan single frame
   * @details    the image is read here and stored in the class
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::acquire_one() {
    std::string function = "Andor::Interface::acquire_one";
    std::stringstream message;
    long error = NO_ERROR;

    // Make sure the camera is in single scan mode
    //
    if ( this->camera_info.acqmode != 1 ) error = this->set_acquisition_mode( 1 );

    // Start Acquisition
    //
    if ( error==NO_ERROR ) error = this->start_acquisition();

    // Wait for acquisition
    //
    int status;
    error = ( andor ? andor->_GetStatus( status ) : ERROR );
    while ( error==NO_ERROR && status == DRV_ACQUIRING ) error = ( andor ? andor->_GetStatus( status ) : ERROR );

    // Get the acquired image
    //
    if ( this->image_data != NULL ) {
      delete [] this->image_data;
      this->image_data=NULL;
    }
    this->image_data = new uint16_t[ this->camera_info.axes[0] * this->camera_info.axes[1] ];

    if (error==NO_ERROR) error = ( andor ? andor->_GetAcquiredData16( this->image_data, this->camera_info.axes[0] * this->camera_info.axes[1] ) : ERROR );

//  if (error==NO_ERROR) error = sdk._GetTemperature();

    return error;
  }
  /***** Andor::Interface::acquire_one ****************************************/


  /***** Andor::Interface::save_acquired **************************************/
  /**
   * @brief      save an acquired image to FITS file
   * @details    This saves the image stored in the class. See also acquire_one().
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

    get_temperature();

    fits_file.create_header();                  //

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
    if (error==NO_ERROR) error = ( andor ? andor->_SetExposureTime( this->camera_info.exposure_time ) : ERROR );
    if (error==NO_ERROR) error = this->start_acquisition();
    uint16_t* image_data = new uint16_t[ this->camera_info.axes[0] * this->camera_info.axes[1] ];
    if (error==NO_ERROR) error = this->get_status( );
    error = ( andor ? andor->_GetStatus( status ) : ERROR );
    while ( error==NO_ERROR && status == DRV_ACQUIRING ) error = ( andor ? andor->_GetStatus( status ) : ERROR );
    if (error==NO_ERROR) error = ( andor ? andor->_GetAcquiredData16( image_data, this->camera_info.axes[0] * this->camera_info.axes[1] ) : ERROR );

    SaveAsFITS( (char*)"/tmp/andor.fits", 2 );
    delete [] image_data;

    return( NO_ERROR );
  }
  /***** Andor::Interface::test ***********************************************/


  /***** Andor::Interface::exptime ********************************************/
  /**
   * @brief      set the exposure time
   * @param[in]  exptime_in  integer exposure time
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::exptime( int exptime_in ) {
    std::string dontcare;
    return exptime( std::to_string( exptime_in ), dontcare );
  }
  /***** Andor::Interface::exptime ********************************************/


  /***** Andor::Interface::exptime ********************************************/
  /**
   * @brief      set or get the exposure time
   * @param[in]  exptime_in  string exposure time
   * @param[out] retstring   reference to string return value
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::exptime( std::string exptime_in, std::string &retstring ) {
    std::string function = "Andor::Interface::exptime";
    std::stringstream message;
    double exptime_try=NAN;
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
          error = ( andor ? andor->_SetExposureTime( exptime_try ) : ERROR );
          if ( error==NO_ERROR && exptime_try != NAN ) this->camera_info.exposure_time = exptime_try;
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
    this->pFits->pHDU().addKey( "EXPTIME", this->info.exposure_time, "exposure time (sec)" );
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
    this->pFits->pHDU().addKey( "PIXSCALE", "TBD", "arcsec per pixel" );
    this->pFits->pHDU().addKey( "POSANG", "TBD", "angle of image Y axis relative to North" );

    return( NO_ERROR );
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
