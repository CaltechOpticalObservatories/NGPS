/**
 * @file    andor_emulator.cpp
 * @brief   this file contains the emulator code for the Andor interface
 * @details most of the emulated SDK wrappers don't do anything but return NO_ERROR
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "atmcdLXd.h"
#include "andor.h"
#include "logentry.h"
#include <Python.h>

namespace Andor {

  std::map<int, std::vector<float>> sim_hsspeeds = { { 0, { 30.0, 20.0, 10.0, 1.0 } },
                                                     { 1, { 1.0, 0.1 } } };

  std::vector<float> sim_vsspeeds = { 0.6, 1.13, 2.2, 4.33 };


  /***** Andor::Emulator::_GetCapabilities ************************************/
  /**
   * @brief      wrapper for GetCapabilities.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetCapabilities( AndorCapabilities* caps ) {
    std::string function = "Andor::Emulator::_GetCapabilities";
    std::stringstream message;

    // set AC_ACQMODE_FRAMETRANSFER bit
    caps->ulFTReadModes = AC_ACQMODE_FRAMETRANSFER;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetCapabilities ************************************/


  /***** Andor::Emulator::_GetAcquiredData16 **********************************/
  /**
   * @brief      
   * @details    16-bit version of GetAcquiredData. buf must be large enough
   *             to hold the complete data set.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetAcquiredData16( unsigned short* buf, at_u32 bufsize ) {
    std::string function = "Andor::Emulator::_GetAcquiredData16";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetAcquiredData16 **********************************/


  /***** Andor::Emulator::_GetMostRecentImage16 *******************************/
  /**
   * @brief      get most recent image
   * @details    16-bit version of GetMostRecentImage16. buf must be large enough
   *             to hold the complete data set.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetMostRecentImage16( unsigned short* buf, at_u32 bufsize ) {
    std::string function = "Andor::Emulator::_GetMostRecentImage16";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetMostRecentImage16 *******************************/


  /***** Andor::Emulator::_GetAvailableCameras ********************************/
  /**
   * @brief      wrapper for Andor Sim GetAvailableCameras
   * @details    Returns the total number of installed Andor cameras.
   *             Can be called before any cameras are initialized.
   * @param[out] number  reference to integer to return number of cameras
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetAvailableCameras( at_32 &number ) {
    std::string function = "Andor::Emulator::_GetAvailableCameras";
    std::stringstream message;

    number = 1;

    logwrite( function, std::to_string( number ) );

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetAvailableCameras ********************************/


  /***** Andor::Emulator::_GetCameraHandle ************************************/
  /**
   * @brief      wrapper for Andor Sim GetCameraHandle
   * @details    Returns the handle for the camera specified by index. When
   *             multiple Andor cameras are installed, the handle of each camera
   *             must be retrieved in order to select a camera using
   *             SetCurrentCamera.
   * @param[in]  index   index of installed, available camera
   * @param[out] handle  pointer to the handle received
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetCameraHandle( int index, at_32 &handle ) {
    std::string function = "Andor::Emulator::_GetCameraHandle";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetCameraHandle ************************************/


  /***** Andor::Emulator::_GetCameraSerialNumber ******************************/
  /**
   * @brief      wrapper for Andor Sim GetCameraSerialNumber
   * @details    checks return value
   * @param[out] number
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetCameraSerialNumber( int &number ) {
    std::string function = "Andor::Emulator::_GetCameraSerialNumber";
    std::stringstream message;

    number = this->serial_number;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetCameraSerialNumber ******************************/


  /***** Andor::Emulator::_GetDetector ****************************************/
  /**
   * @brief      wrapper for Andor Sim GetDetector
   * @details    Returns the size of the detector in pixels
   * @param[out] xpix  number of horizontal pixels
   * @param[out] ypix  number of vertical pixels
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetDetector( int &xpix, int &ypix ) {
    std::string function = "Andor::Emulator::_GetDetector";
    std::stringstream message;

    xpix = 1024;
    ypix = 1024;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetDetector ****************************************/


  /***** Andor::Emulator::_GetStatus ******************************************/
  /**
   * @brief      wrapper for Andor Sim GetStatus
   * @details    checks return value
   * @param[out] status  reference to string to contain the status message
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * This function is overloaded. This version does not provide the status message string.
   *
   */
  long Emulator::_GetStatus( std::string &status ) {
    std::string function = "Andor::Emulator::_GetStatus";
    int dontcare;
    return this->_GetStatus( dontcare, status );
  }
  /***** Andor::Emulator::_GetStatus ******************************************/


  /***** Andor::Emulator::_GetStatus ******************************************/
  /**
   * @brief      wrapper for Andor Sim GetStatus
   * @details    Return status of Andor Sim system. Should be called before an
   *             acquisition is started to ensure it is IDLE, and then call
   *             during acquisition to monitor progress.
   * @param[out] status_id   reference to int to contain the status ID code
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * This function is overloaded. This version does not provide the status message string.
   *
   */
  long Emulator::_GetStatus( int &status_id ) {
    std::string function = "Andor::Emulator::_GetStatus";
    std::string dontcare;
    return this->_GetStatus( status_id, dontcare );
  }
  /***** Andor::Emulator::_GetStatus ******************************************/


  /***** Andor::Emulator::_GetStatus ******************************************/
  /**
   * @brief      wrapper for Andor Sim GetStatus checks return value
   * @details    Return status of Andor Sim system. Should be called before an
   *             acquisition is started to ensure it is IDLE, and then call
   *             during acquisition to monitor progress.
   * @param[out] status_id   reference to int to contain the status ID code
   * @param[out] status_msg  reference to string to contain the status message text
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * This function is overloaded. This version provides the status message string.
   *
   */
  long Emulator::_GetStatus( int &status_id, std::string &status_msg ) {
    std::string function = "Andor::Emulator::_GetStatus";
    std::stringstream message;

    status_id  = DRV_IDLE;
    status_msg = "IDLE";

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetStatus ******************************************/


  /***** Andor::Emulator::_GetNumberADChannels ********************************/
  /**
   * @brief      wrapper for Andor Sim GetNumberADChannels
   * @details    returns the number of AD converter channels available
   * @param[out] channels  reference to return number of channels
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_GetNumberADChannels( int &channels ) {
    std::string function = "Andor::Emulator::_GetNumberADChannels";
    std::stringstream message;

    channels = 1;

    logwrite( function, std::to_string( channels ) );

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetNumberADChannels ********************************/


  /***** Andor::Emulator::_GetNumberHSSpeeds **********************************/
  /**
   * @brief      wrapper for Andor Sim GetNumberHSSpeeds
   * @details    returns the number of horizontal shift speeds available
   * @param[in]  adchan  AD channel
   * @param[in]  type    output amplification type {0=EM 1=conventional}
   * @param[out] speeds  reference to int contains number of HS speeds
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_GetNumberHSSpeeds( int adchan, int type, int &speeds ) {
    std::string function = "Andor::Emulator::_GetNumberHSSpeeds";
    std::stringstream message;
    long error=NO_ERROR;

    auto it = sim_hsspeeds.find( type );

    if ( it == sim_hsspeeds.end() ) {
      speeds=0;
      message << "ERROR type " << type << " not found";
      logwrite( function, message.str() );
      return ERROR;
    }

    speeds = it->second.size();

    logwrite( function, std::to_string( speeds ) );

    return error;
  }
  /***** Andor::Emulator::_GetNumberHSSpeeds **********************************/


  /***** Andor::Emulator::_GetNumberVSSpeeds **********************************/
  /**
   * @brief      wrapper for Andor Sim GetNumberVSSpeeds
   * @details    returns the number of vertical shift speeds available
   * @param[out] speeds  reference to int contains number of VS speeds
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_GetNumberVSSpeeds( int &speeds ) {
    std::string function = "Andor::Emulator::_GetNumberVSSpeeds";
    std::stringstream message;

    speeds = sim_vsspeeds.size();

    logwrite( function, std::to_string( speeds ) );

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetNumberVSSpeeds **********************************/


  /***** Andor::Emulator::_GetHSSpeed *****************************************/
  /**
   * @brief      wrapper for Andor Sim GetHSSpeed
   * @details    returns the horizontal shift speed for specified amp and index
   * @param[in]  chan   AD channel
   * @param[in]  type   output amplification type {0=EM 1=conventional}
   * @param[in]  index
   * @param[out] speed  reference to float to contain speed in MHz
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_GetHSSpeed( int chan, int type, int index, float &speed ) {
    std::string function = "Andor::Emulator::_GetHSSpeed";
    std::stringstream message;

    auto it = sim_hsspeeds.find( type );

    if ( it == sim_hsspeeds.end() ) {
      speed=0;
      message << "ERROR type " << type << " not found";
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( index < 0 || index > (int)it->second.size() ) {
      speed=0;
      message << "ERROR index " << index << " for type " << type << " not found";
      logwrite( function, message.str() );
      return ERROR;
    }

    speed = it->second[index];

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetHSSpeed *****************************************/


  /***** Andor::Emulator::_SetHSSpeed *****************************************/
  /**
   * @brief      wrapper for Andor Sim SetHSSpeed
   * @details    Set the speed at which pixels are shifted into output node
   *             during readout phase of an acquisition.
   * @param[in]  type   output amplification type {0=EM 1=conventional}
   * @param[in]  index  index of hsspeeds vector
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_SetHSSpeed( int type, int index ) {
    std::string function = "Andor::Emulator::_SetHSSpeed";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetHSSpeed *****************************************/


  /***** Andor::Emulator::_GetVSSpeed *****************************************/
  /**
   * @brief      wrapper for Andor Sim GetVSSpeed
   * @details    returns the vertical shift speed for specified index
   * @param[in]  index
   * @param[out] speed  reference to float to contain speed in MHz
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_GetVSSpeed( int index, float &speed ) {
    std::string function = "Andor::Emulator::_GetVSSpeed";
    std::stringstream message;

    if ( index < 0 || index > (int)sim_vsspeeds.size() ) {
      speed=0;
      message << "ERROR index " << index << " not found";
      logwrite( function, message.str() );
      return ERROR;
    }

    speed = sim_vsspeeds[index];

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetVSSpeed *****************************************/


  /***** Andor::Emulator::_SetVSSpeed *****************************************/
  /**
   * @brief      wrapper for Andor Sim SetVSSpeed
   * @details    set the vertical speed used for subsequent acquisitions
   * @param[in]  index  index of vsspeeds vector
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_SetVSSpeed( int index ) {
    std::string function = "Andor::Emulator::_SetVSSpeed";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetVSSpeed *****************************************/


  /***** Andor::Emulator::_SetEMCCDGain ***************************************/
  /**
   * @brief      wrapper for Andor Sim SetEMCCDGain
   * @details    set gain value
   * @param[in]  gain  new gain
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_SetEMCCDGain( int gain ) {
    std::string function = "Andor::Emulator::_SetEMCCDGain";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetEMCCDGain ***************************************/


  /***** Andor::Emulator::_GetEMCCDGain ***************************************/
  /**
   * @brief      wrapper for Andor Sim GetEMCCDGain
   * @details    return gain setting (by reference) and save in class
   * @param[out] gain  reference to variable to contain gain setting
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_GetEMCCDGain( int &gain ) {
    std::string function = "Andor::Emulator::_GetEMCCDGain";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetEMCCDGain ***************************************/


  /***** Andor::Emulator::_GetEMGainRange *************************************/
  /**
   * @brief      wrapper for Andor Sim GetEMGainRange
   * @details    return minimum and maximum values for selected EM gain mode
   *             and temperature of the sensor
   * @param[out] temp  pointer to variable to contain current temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_GetEMGainRange( int &low, int &high ) {
    std::string function = "Andor::Emulator::_GetEMGainRange";
    std::stringstream message;

    low  =   1;
    high = 300;

    message << low << " " << high;
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetEMGainRange *************************************/


  /***** Andor::Emulator::_SetOutputAmplifier *********************************/
  /**
   * @brief      wrapper for Andor Sim SetOutputAmplifier
   * @details    set the type of amplifier to be used
   * @param[in]  type  amplifier type 0=EMCCD 1=conventional
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_SetOutputAmplifier( int type ) {
    std::string function = "Andor::Emulator::_SetOutputAmplifier";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetOutputAmplifier *********************************/


  /***** Andor::Emulator::_SetFrameTransferMode *******************************/
  /**
   * @brief      wrapper for Andor Sim SetFrameTransferMode
   * @param[in]  mode  1=on 0=off
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_SetFrameTransferMode( int mode ) {
    std::string function = "Andor::Emulator::_SetFrameTransferMode";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetFrameTransferMode *******************************/


  /***** Andor::Emulator::_GetTemperature *************************************/
  /**
   * @brief      wrapper for Andor Sim GetTemperature
   * @details    returns temperature of detector to the nearest degree C
   * @param[out] temp  pointer to variable to contain current temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_GetTemperature( int &temp, std::string_view &status ) {
    std::string function = "Andor::Emulator::_GetTemperature";
    std::stringstream message;

    temp   = this->temperature;
    status = "STABILIZED";

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetTemperature *************************************/


  /***** Andor::Emulator::_GetTemperatureRange ********************************/
  /**
   * @brief      wrapper for Andor Sim GetTemperature checks return value
   * @details    returns the valid range of temperatures
   * @param[out] min  reference to variable to contain minimum temperature
   * @param[out] max  reference to variable to contain maximum temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_GetTemperatureRange( int &min, int &max ) {
    std::string function = "Andor::Emulator::_GetTemperatureRange";
    std::stringstream message;

    min = -120;
    max = 20;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetTemperatureRange ********************************/


  /***** Andor::Emulator::_CoolerON *******************************************/
  /**
   * @brief      wrapper for Andor Sim CoolerON
   * @details    turns on the cooling
   * @details    turns the cooler on
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_CoolerON() {
    std::string function = "Andor::Emulator::_CoolerON";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_CoolerON *******************************************/


  /***** Andor::Emulator::_CoolerOFF ******************************************/
  /**
   * @brief      wrapper for Andor Sim CoolerOFF
   * @details    turns off the cooling
   * @details    turns the cooler off
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_CoolerOFF() {
    std::string function = "Andor::Emulator::_CoolerOFF";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_CoolerOFF ******************************************/


  /***** Andor::Emulator::_SetTemperature *************************************/
  /**
   * @brief      wrapper for Andor Sim SetTemperature
   * @details    set the temperature of the detector (does NOT control cooling)
   * @param[in]  temp  new temperature setpoint
   * @return     NO_ERROR or ERROR
   *
   */
  long Emulator::_SetTemperature( int temp ) {
    std::string function = "Andor::Emulator::_SetTemperature";
    std::stringstream message;

    this->temperature = temp;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetTemperature *************************************/


  /***** Andor::Emulator::_GetVersionInfo *************************************/
  /**
   * @brief      wrapper for Andor Sim GetVersionInfo
   * @details    Retrieves version information about different aspects of the
   *             Andor system, copied into the passed buffer.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetVersionInfo( AT_VersionInfoId arr, char* info, at_u32 len ) {
    std::string function = "Andor::Emulator::_GetVersionInfo";
    std::stringstream message;

    if ( info != nullptr ) strcpy( info, "emulator" );

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetVersionInfo *************************************/


  /***** Andor::Emulator::_Initialize *****************************************/
  /**
   * @brief      wrapper for Andor Sim Initialize
   * @details    initialize the Andor Sim system
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_Initialize() {
    std::string function = "Andor::Emulator::_Initialize";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_Initialize *****************************************/


  /***** Andor::Emulator::_SetAcquisitionMode *********************************/
  /**
   * @brief      wrapper for Andor Sim SetAcquisitionMode
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
  long Emulator::_SetAcquisitionMode( int mode ) {
    std::string function = "Andor::Emulator::_SetAcquisitionMode";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetAcquisitionMode *********************************/


  /***** Andor::Emulator::_SetCurrentCamera ***********************************/
  /**
   * @brief      wrapper for Andor Sim SetCurrentCamera
   * @details    When multiple cameras are installed this allows selecting which
   *             camera is active. Once selected, all other functions apply to
   *             the selected camera.
   * @param[in]  handle  camera handle from GetCameraHandle()
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_SetCurrentCamera( at_32 handle ) {
    std::string function = "Andor::Emulator::_SetCurrentCamera";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetCurrentCamera ***********************************/


  /***** Andor::Emulator::_SetExposureTime ************************************/
  /**
   * @brief      wrapper for Andor Sim SetExposureTime
   * @details    Set the exposure time to the nearest valid value not less
   *             than the given value. Actual exposure time is obtained by
   *             GetAcquisitionTimings.
   * @param[in]  exptime_in  exposure time in seconds
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_SetExposureTime( float exptime_in ) {
    std::string function = "Andor::Emulator::_SetExposureTime";
    std::stringstream message;

    if ( std::isnan( exptime_in ) || exptime_in < 0 ) {
      message.str(""); message << "ERROR bad exposure time " << exptime_in;
      logwrite( function, message.str() );
      return ERROR;
    }

    this->exptime = exptime_in;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetExposureTime ************************************/


  /***** Andor::Emulator::_SetKineticCycleTime ********************************/
  /**
   * @brief      wrapper for Andor Sim SetKineticCycleTime
   * @param[in]  time
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_SetKineticCycleTime( float time ) {
    std::string function = "Andor::Emulator::_SetKineticCycleTime";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetKineticCycleTime ********************************/


  /***** Andor::Emulator::_SetNumberAccumulations *****************************/
  /**
   * @brief      wrapper for Andor Sim SetNumberAccumulations
   * @param[in]  number
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_SetNumberAccumulations( int number ) {
    std::string function = "Andor::Emulator::_SetNumberAccumulations";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetNumberAccumulations *****************************/


  /***** Andor::Emulator::_SetAccumulationCycleTime ***************************/
  /**
   * @brief      wrapper for Andor Sim SetAccumulationCycleTime
   * @param[in]  time
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_SetAccumulationCycleTime( float time ) {
    std::string function = "Andor::Emulator::_SetAccumulationCycleTime";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetAccumulationCycleTime ***************************/


  /***** Andor::Emulator::_SetNumberKinetics **********************************/
  /**
   * @brief      wrapper for Andor Sim SetNumberKinetics
   * @param[in]  number
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_SetNumberKinetics( int number ) {
    std::string function = "Andor::Emulator::_SetNumberKinetics";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetNumberKinetics **********************************/


  /***** Andor::Emulator::__GetAcquisitionTimings *****************************/
  /**
   * @brief      wrapper for Andor Sim GetAcquisitionTimings
   * @param[out] exp  valid exposure time in seconds
   * @param[out] acc  valid accumulate cycle time in seconds
   * @param[out] kin  valid kinetic cycle time in seconds
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetAcquisitionTimings( float &exp, float &acc, float &kin ) {
    std::string function = "Andor::Emulator::_GetAcquisitionTimings";
    std::stringstream message;

    exp = this->exptime;
    acc = 1.1*exp;
    kin = 1.1*acc;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetAcquisitionTimings ******************************/


  /***** Andor::Emulator::_SetImageFlip ***************************************/
  /**
   * @brief      wrapper for Andor Sim SetImageFlip
   * @details    This causes data output from Sim to be flipped in one or both
   *             axes. The flip is not done in the camera, it occurs after the
   *             data are retriever and will increase processing overhead.
   * @param[in]  hflip  1=enable horizontal flipping, 0=disable
   * @param[in]  vflip  1=enable vertical flipping, 0=disable
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_SetImageFlip( int hflip, int vflip ) {
    std::string function = "Andor::Emulator::_SetImageFlip";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetImageFlip ***************************************/


  /***** Andor::Emulator::_SetImageRotate *************************************/
  /**
   * @brief      wrapper for Andor Sim SetImageRotate
   * @details    This causes data output from Sim to be rotated in one or both
   *             axes. This rotate is not done in the camera, it occurs after
   *             the data are retrieved and will increase processing overhead.
   * @param[in]  rotdir  0=no rotation, 1=90 CW, 2=90 CCW
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_SetImageRotate( int rotdir ) {
    std::string function = "Andor::Emulator::_SetImageRotate";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetImageRotate *************************************/


  /***** Andor::Emulator::_SetImage *******************************************/
  /**
   * @brief      wrapper for Andor Sim SetImage checks return value
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
  long Emulator::_SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend ) {
    std::string function = "Andor::Emulator::_SetImage";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetImage *******************************************/


  /***** Andor::Emulator::_SetReadMode ****************************************/
  /**
   * @brief      wrapper for Andor Sim SetReadMode
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
  long Emulator::_SetReadMode( int mode ) {
    std::string function = "Andor::Emulator::_SetReadMode";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetReadMode ****************************************/


  /***** Andor::Emulator::_GetShutterMinTimes *********************************/
  /**
   * @brief      wrapper for Andor Sim GetShutterMinTimes
   * @param[out] minclosing
   * @param[out] minopening
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_GetShutterMinTimes( int &minclosing, int &minopening ) {
    std::string function = "Andor::Emulator::__GetShutterMinTimes";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_GetShutterMinTimes *********************************/


  /***** Andor::Emulator::_SetShutter *****************************************/
  /**
   * @brief      wrapper for Andor Sim SetShutter
   * @details    controls behavior of the shutter
   * @param[in]  type
   * @param[in]  mode
   * @param[in]  closetime
   * @param[in]  opentime
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_SetShutter( int type, int mode, int closetime, int opentime ) {
    std::string function = "Andor::Emulator::_SetShutter";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_SetShutter *****************************************/


  /***** Andor::Emulator::_AbortAcquisition ***********************************/
  /**
   * @brief      wrapper for Andor Sim AbortAcquisition
   * @details    aborts the acquisition
   * @details    this simply checks return value
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_AbortAcquisition() {
    std::string function = "Andor::Emulator::_AbortAcquisition";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_AbortAcquisition ***********************************/


  /***** Andor::Emulator::_StartAcquisition ***********************************/
  /**
   * @brief      wrapper for Andor Sim StartAcquisition
   * @details    starts the acquisition
   * @details    this simply checks return value
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_StartAcquisition() {
    std::string function = "Andor::Emulator::_StartAcquisition";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_StartAcquisition ***********************************/


  /***** Andor::Emulator::_WaitForAcquisition *********************************/
  /**
   * @brief      wrapper for Andor Sim WaitForAcquisition
   * @details    starts the acquisition
   * @details    this simply checks return value
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_WaitForAcquisition() {
    std::string function = "Andor::Emulator::_WaitForAcquisition";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_WaitForAcquisition *********************************/


  /***** Andor::Emulator::_WaitForAcquisitionByHandleTimeOut ******************/
  /**
   * @brief      wrapper for Andor Sim WaitForAcquisitionByHandleTimeOut
   * @details    starts the acquisition
   * @details    this simply checks return value
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Emulator::_WaitForAcquisitionByHandleTimeOut( at_32 handle, int timeout ) {
    std::string function = "Andor::Emulator::_WaitForAcquisitionByHandleTimeOut";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Emulator::_WaitForAcquisitionByHandleTimeOut ******************/


  /***** Andor::SkySim::initialize_python *************************************/
  /**
   * @brief      initializes the Python skysim module
   *
   */
  void SkySim::initialize_python() {
    std::string function = "Andor::SkySim::initialize_python";

    if ( ! py_instance.is_initialized() ) {
      logwrite( function, "ERROR could not initialize Python interpreter" );
      py_instance.print_python_error( function );
      return;
    }

    PyGILState_STATE gstate = PyGILState_Ensure();   // Acquire the GIL

    PyObject* pModuleName = PyUnicode_FromString( PYTHON_SKYSIM_MODULE );

    if ( ! pModuleName ) {
      logwrite( function, "ERROR could not create module name string" );
      py_instance.print_python_error( function );
      Py_XDECREF( pModuleName );
      PyGILState_Release( gstate );                  // Release the GIL
      return;
    }

    pSkySimModule = PyImport_Import( pModuleName );

    if ( pSkySimModule == nullptr ) {
      logwrite( function, "ERROR Python skysim module not initialized" );
      py_instance.print_python_error( function );
      Py_XDECREF( pModuleName );
      Py_XDECREF( pSkySimModule );
      PyGILState_Release( gstate );                  // Release the GIL
      return;
    }

    PyGILState_Release( gstate );                    // Release the GIL

    python_initialized = true;

    logwrite( function, "initialized Python skysim module" );

    return;
  }
  /***** Andor::SkySim::initialize_python *************************************/


  /***** Andor::SkySim::generate_image ****************************************/
  /**
   * @brief      calls Python skysim to generate an image
   * @param[in]  headerfile
   * @param[in]  outputfile
   * @param[in]  exptime
   * @param[in]  ismex
   * @param[in]  simsize
   * @return     ERROR | NO_ERROR
   *
   */
  long SkySim::generate_image( const std::string_view &headerfile,
                               const std::string_view &outputfile,
                               const float exptime,
                               const bool ismex,
                               const int simsize ) {
    std::string function = "Andor::SkySim::generate_image";
    std::stringstream message;
    long error = NO_ERROR;

    if ( !python_initialized || pSkySimModule == nullptr ) {
      logwrite( function, "ERROR Python skysim module is not initialized" );
      return ERROR;
    }

    // The exposure time (rounded to nearest msec) is needed only to emulate the delay,
    // otherwise the skysim function uses the exptime from the headerfile to generate an
    // image with the appropriate exposure time.
    //
    // Minimum delay is 10 msec to prevent excessive CPU usage.
    //
    long expdelay = (long)( std::round( exptime * 1000.0 ) );
    if ( expdelay == 0 ) expdelay = 10;
    std::this_thread::sleep_for( std::chrono::milliseconds( expdelay ) );

    PyGILState_STATE gstate = PyGILState_Ensure();   // Acquire the GIL

    // Build Python function name. The function depends on the state of ismex.
    //
    PyObject* pFunction;
    pFunction = ( ismex ? PyObject_GetAttrString( pSkySimModule, PYTHON_SKYSIM_MULTI_FUNCTION )
                        : PyObject_GetAttrString( pSkySimModule, PYTHON_SKYSIM_FUNCTION ) );

    if ( !pFunction || !PyCallable_Check( pFunction ) ) {
      logwrite( function, "ERROR Python skysim function not callable" );
      py_instance.print_python_error( function );
      Py_XDECREF( pFunction );
      PyGILState_Release( gstate );                  // Release the GIL
      return ERROR;
    }

    // Build Python arguments
    //
    PyObject* pHeaderfile = PyUnicode_FromString( headerfile.data() );
    PyObject* pOutputfile = PyUnicode_FromString( outputfile.data() );
    PyObject* pKwArgs     = PyDict_New();

    if ( !pHeaderfile || !pOutputfile || !pKwArgs ) {
      logwrite( function, "ERROR creating Python arguments" );
      py_instance.print_python_error( function );
      Py_XDECREF( pFunction );
      Py_XDECREF( pHeaderfile );
      Py_XDECREF( pOutputfile );
      Py_XDECREF( pKwArgs );
      PyGILState_Release( gstate );                  // Release the GIL
      return ERROR;
    }

    PyDict_SetItemString( pKwArgs, "IMAGE_SIZE", PyLong_FromLong( simsize ) );

    PyObject* pArgs   = PyTuple_Pack( 2, pHeaderfile, pOutputfile );

    if ( !pArgs || PyErr_Occurred() ) {
      logwrite( function, "ERROR packing Python arguments" );
      py_instance.print_python_error( function );
      Py_XDECREF( pFunction );
      Py_XDECREF( pHeaderfile );
      Py_XDECREF( pOutputfile );
      Py_XDECREF( pArgs );
      Py_XDECREF( pKwArgs );
      PyGILState_Release( gstate );                  // Release the GIL
      return ERROR;
    }

    // Call the Python function here
    //
    PyObject* pReturn = PyObject_Call( pFunction, pArgs, pKwArgs );

    if ( !pReturn ) {
      logwrite( function, "ERROR calling Python skysim function" );
      py_instance.print_python_error( function );
      error = ERROR;
    }

    // clean up
    //
    Py_XDECREF( pArgs );
    Py_XDECREF( pFunction );
    Py_XDECREF( pKwArgs );
    Py_XDECREF( pHeaderfile );
    Py_XDECREF( pOutputfile );

    PyGILState_Release( gstate );                    // Release the GIL

    message.str(""); message << "headerfile: " << headerfile << " outputfile: " << outputfile;
    logwrite( function, message.str() );

    return error;
  }
  /***** Andor::SkySim::generate_image ****************************************/

}
