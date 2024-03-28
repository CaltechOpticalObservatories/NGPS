/**
 * @file    sim.cpp
 * @brief   this file contains the simulator code for the Andor interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "atmcdLXd.h"
#include "andor.h"
#include "logentry.h"

namespace Andor {

  std::map<int, std::vector<float>> sim_hsspeeds = { { 0, { 30.0, 20.0, 10.0, 1.0 } },
                                                     { 1, { 1.0, 0.1 } } };

  std::vector<float> sim_vsspeeds = { 0.6, 1.13, 2.2, 4.33 };


  /***** Andor::Sim::_GetAcquiredData16 ***************************************/
  /**
   * @brief      
   * @details    16-bit version of GetAcquiredData. buf must be large enough
   *             to hold the complete data set.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_GetAcquiredData16( uint16_t* buf, unsigned long bufsize ) {
    std::string function = "Andor::Sim::_GetAcquiredData16";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetAcquiredData16 ***************************************/


  /***** Andor::Sim::_GetAvailableCameras *************************************/
  /**
   * @brief      wrapper for Andor Sim GetAvailableCameras
   * @details    Returns the total number of installed Andor cameras.
   *             Can be called before any cameras are initialized.
   * @param[out] number  reference to integer to return number of cameras
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_GetAvailableCameras( int &number ) {
    std::string function = "Andor::Sim::_GetAvailableCameras";
    std::stringstream message;

    number = 1;

    logwrite( function, std::to_string( number ) );

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetAvailableCameras *************************************/


  /***** Andor::Sim::_GetCameraHandle *****************************************/
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
  long Sim::_GetCameraHandle( int index, int* handle ) {
    std::string function = "Andor::Sim::_GetCameraHandle";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetCameraHandle *****************************************/


  /***** Andor::Sim::_GetCameraSerialNumber ***********************************/
  /**
   * @brief      wrapper for Andor Sim GetCameraSerialNumber
   * @details    checks return value
   * @param[out] number
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_GetCameraSerialNumber( int &number ) {
    std::string function = "Andor::Sim::_GetCameraSerialNumber";
    std::stringstream message;

    number = 12345;

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetCameraSerialNumber ***********************************/


  /***** Andor::Sim::_GetDetector *********************************************/
  /**
   * @brief      wrapper for Andor Sim GetDetector
   * @details    Returns the size of the detector in pixels
   * @param[out] xpix  number of horizontal pixels
   * @param[out] ypix  number of vertical pixels
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_GetDetector( int &xpix, int &ypix ) {
    std::string function = "Andor::Sim::_GetDetector";
    std::stringstream message;

    xpix = 1024;
    ypix = 1024;

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetDetector *********************************************/


  /***** Andor::Sim::_GetStatus ***********************************************/
  /**
   * @brief      wrapper for Andor Sim GetStatus
   * @details    checks return value
   * @param[out] status  reference to string to contain the status message
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   * This function is overloaded. This version does not provide the status message string.
   *
   */
  long Sim::_GetStatus( std::string &status ) {
    std::string function = "Andor::Sim::_GetStatus";
    int dontcare;
    return this->_GetStatus( dontcare, status );
  }
  /***** Andor::Sim::_GetStatus ***********************************************/


  /***** Andor::Sim::_GetStatus ***********************************************/
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
  long Sim::_GetStatus( int &status_id ) {
    std::string function = "Andor::Sim::_GetStatus";
    std::string dontcare;
    return this->_GetStatus( status_id, dontcare );
  }
  /***** Andor::Sim::_GetStatus ***********************************************/


  /***** Andor::Sim::_GetStatus ***********************************************/
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
  long Sim::_GetStatus( int &status_id, std::string &status_msg ) {
    std::string function = "Andor::Sim::_GetStatus";
    std::stringstream message;

    status_id  = DRV_IDLE;
    status_msg = "IDLE";

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetStatus ***********************************************/


  /***** Andor::Sim::_GetNumberADChannels *************************************/
  /**
   * @brief      wrapper for Andor Sim GetNumberADChannels
   * @details    returns the number of AD converter channels available
   * @param[out] channels  reference to return number of channels
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_GetNumberADChannels( int &channels ) {
    std::string function = "Andor::Sim::_GetNumberADChannels";
    std::stringstream message;

    channels = 1;

    logwrite( function, std::to_string( channels ) );

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetNumberADChannels *************************************/


  /***** Andor::Sim::_GetNumberHSSpeeds ***************************************/
  /**
   * @brief      wrapper for Andor Sim GetNumberHSSpeeds
   * @details    returns the number of horizontal shift speeds available
   * @param[in]  adchan  AD channel
   * @param[in]  type    output amplification type {0=EM 1=conventional}
   * @param[out] speeds  reference to int contains number of HS speeds
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_GetNumberHSSpeeds( int adchan, int type, int &speeds ) {
    std::string function = "Andor::Sim::_GetNumberHSSpeeds";
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
  /***** Andor::Sim::_GetNumberHSSpeeds ***************************************/


  /***** Andor::Sim::_GetNumberVSSpeeds ***************************************/
  /**
   * @brief      wrapper for Andor Sim GetNumberVSSpeeds
   * @details    returns the number of vertical shift speeds available
   * @param[out] speeds  reference to int contains number of VS speeds
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_GetNumberVSSpeeds( int &speeds ) {
    std::string function = "Andor::Sim::_GetNumberVSSpeeds";
    std::stringstream message;

    speeds = sim_vsspeeds.size();

    logwrite( function, std::to_string( speeds ) );

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetNumberVSSpeeds ***************************************/


  /***** Andor::Sim::_GetHSSpeed **********************************************/
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
  long Sim::_GetHSSpeed( int chan, int type, int index, float &speed ) {
    std::string function = "Andor::Sim::_GetHSSpeed";
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
  /***** Andor::Sim::_GetHSSpeed **********************************************/


  /***** Andor::Sim::_SetHSSpeed **********************************************/
  /**
   * @brief      wrapper for Andor Sim SetHSSpeed
   * @details    Set the speed at which pixels are shifted into output node
   *             during readout phase of an acquisition.
   * @param[in]  type   output amplification type {0=EM 1=conventional}
   * @param[in]  index  index of hsspeeds vector
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_SetHSSpeed( int type, int index ) {
    std::string function = "Andor::Sim::_SetHSSpeed";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetHSSpeed **********************************************/


  /***** Andor::Sim::_GetVSSpeed **********************************************/
  /**
   * @brief      wrapper for Andor Sim GetVSSpeed
   * @details    returns the vertical shift speed for specified index
   * @param[in]  index
   * @param[out] speed  reference to float to contain speed in MHz
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_GetVSSpeed( int index, float &speed ) {
    std::string function = "Andor::Sim::_GetVSSpeed";
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
  /***** Andor::Sim::_GetVSSpeed **********************************************/


  /***** Andor::Sim::_SetVSSpeed **********************************************/
  /**
   * @brief      wrapper for Andor Sim SetVSSpeed
   * @details    set the vertical speed used for subsequent acquisitions
   * @param[in]  index  index of vsspeeds vector
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_SetVSSpeed( int index ) {
    std::string function = "Andor::Sim::_SetVSSpeed";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetVSSpeed **********************************************/


  /***** Andor::Sim::_SetEMCCDGain ********************************************/
  /**
   * @brief      wrapper for Andor Sim SetEMCCDGain
   * @details    set gain value
   * @param[in]  gain  new gain
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_SetEMCCDGain( int gain ) {
    std::string function = "Andor::Sim::_SetEMCCDGain";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetEMCCDGain ********************************************/


  /***** Andor::Sim::_GetEMCCDGain ********************************************/
  /**
   * @brief      wrapper for Andor Sim GetEMCCDGain
   * @details    return gain setting (by reference) and save in class
   * @param[out] gain  reference to variable to contain gain setting
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_GetEMCCDGain( int &gain ) {
    std::string function = "Andor::Sim::_GetEMCCDGain";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetEMCCDGain ********************************************/


  /***** Andor::Sim::_GetEMGainRange ******************************************/
  /**
   * @brief      wrapper for Andor Sim GetEMGainRange
   * @details    return minimum and maximum values for selected EM gain mode
   *             and temperature of the sensor
   * @param[out] temp  pointer to variable to contain current temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_GetEMGainRange( int &low, int &high ) {
    std::string function = "Andor::Sim::_GetEMGainRange";
    std::stringstream message;

    low  =   1;
    high = 300;

    message << low << " " << high;
    logwrite( function, message.str() );

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetEMGainRange ******************************************/


  /***** Andor::Sim::_SetOutputAmplifier **************************************/
  /**
   * @brief      wrapper for Andor Sim SetOutputAmplifier
   * @details    set the type of amplifier to be used
   * @param[in]  type  amplifier type 0=EMCCD 1=conventional
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_SetOutputAmplifier( int type ) {
    std::string function = "Andor::Sim::_SetOutputAmplifier";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetOutputAmplifier **************************************/


  /***** Andor::Sim::_GetTemperature ******************************************/
  /**
   * @brief      wrapper for Andor Sim GetTemperature
   * @details    returns temperature of detector to the nearest degree C
   * @param[out] temp  pointer to variable to contain current temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_GetTemperature( int &temp, std::string_view &status ) {
    std::string function = "Andor::Sim::_GetTemperature";
    std::stringstream message;

    temp   = -120;
    status = "STABILIZED";

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetTemperature ******************************************/


  /***** Andor::Sim::_GetTemperatureRange *************************************/
  /**
   * @brief      wrapper for Andor Sim GetTemperature checks return value
   * @details    returns the valid range of temperatures
   * @param[out] min  reference to variable to contain minimum temperature
   * @param[out] max  reference to variable to contain maximum temperature
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_GetTemperatureRange( int &min, int &max ) {
    std::string function = "Andor::Sim::_GetTemperatureRange";
    std::stringstream message;

    min = -120;
    max = 20;

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetTemperatureRange *************************************/


  /***** Andor::Sim::_CoolerON ************************************************/
  /**
   * @brief      wrapper for Andor Sim CoolerON
   * @details    turns on the cooling
   * @details    turns the cooler on
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_CoolerON() {
    std::string function = "Andor::Sim::_CoolerON";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_CoolerON ************************************************/


  /***** Andor::Sim::_CoolerOFF ***********************************************/
  /**
   * @brief      wrapper for Andor Sim CoolerOFF
   * @details    turns off the cooling
   * @details    turns the cooler off
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_CoolerOFF() {
    std::string function = "Andor::Sim::_CoolerOFF";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_CoolerOFF ***********************************************/


  /***** Andor::Sim::_SetTemperature ******************************************/
  /**
   * @brief      wrapper for Andor Sim SetTemperature
   * @details    set the temperature of the detector (does NOT control cooling)
   * @param[in]  temp  new temperature setpoint
   * @return     NO_ERROR or ERROR
   *
   */
  long Sim::_SetTemperature( int temp ) {
    std::string function = "Andor::Sim::_SetTemperature";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetTemperature ******************************************/


  /***** Andor::Sim::_GetVersionInfo ******************************************/
  /**
   * @brief      wrapper for Andor Sim GetVersionInfo
   * @details    Retrieves version information about different aspects of the
   *             Andor system, copied into the passed buffer.
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_GetVersionInfo( AT_VersionInfoId arr, char* info, at_u32 len ) {
    std::string function = "Andor::Sim::_GetVersionInfo";
    std::stringstream message;

    if ( info != nullptr ) strcpy( info, "simulator" );

    return NO_ERROR;
  }
  /***** Andor::Sim::_GetVersionInfo ******************************************/


  /***** Andor::Sim::_Initialize **********************************************/
  /**
   * @brief      wrapper for Andor Sim Initialize
   * @details    initialize the Andor Sim system
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_Initialize() {
    std::string function = "Andor::Sim::_Initialize";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_Initialize **********************************************/


  /***** Andor::Sim::_SetAcquisitionMode **************************************/
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
  long Sim::_SetAcquisitionMode( int mode ) {
    std::string function = "Andor::Sim::_SetAcquisitionMode";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetAcquisitionMode **************************************/


  /***** Andor::Sim::_SetCurrentCamera ****************************************/
  /**
   * @brief      wrapper for Andor Sim SetCurrentCamera
   * @details    When multiple cameras are installed this allows selecting which
   *             camera is active. Once selected, all other functions apply to
   *             the selected camera.
   * @param[in]  handle  camera handle from GetCameraHandle()
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_SetCurrentCamera( int handle ) {
    std::string function = "Andor::Sim::_SetCurrentCamera";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetCurrentCamera ****************************************/


  /***** Andor::Sim::_SetExposureTime *****************************************/
  /**
   * @brief      wrapper for Andor Sim SetExposureTime
   * @details    Set the exposure time to the nearest valid value not less
   *             than the given value. Actual exposure time is obtained by
   *             GetAcquisitionTimings.
   * @param[in]  exptime  exposure time in seconds
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_SetExposureTime( double exptime ) {
    std::string function = "Andor::Sim::_SetExposureTime";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetExposureTime *****************************************/


  /***** Andor::Sim::_SetImageFlip ********************************************/
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
  long Sim::_SetImageFlip( int hflip, int vflip ) {
    std::string function = "Andor::Sim::_SetImageFlip";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetImageFlip ********************************************/


  /***** Andor::Sim::_SetImageRotate ******************************************/
  /**
   * @brief      wrapper for Andor Sim SetImageRotate
   * @details    This causes data output from Sim to be rotated in one or both
   *             axes. This rotate is not done in the camera, it occurs after
   *             the data are retrieved and will increase processing overhead.
   * @param[in]  rotdir  0=no rotation, 1=90 CW, 2=90 CCW
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_SetImageRotate( int rotdir ) {
    std::string function = "Andor::Sim::_SetImageRotate";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetImageRotate ******************************************/


  /***** Andor::Sim::_SetImage ************************************************/
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
  long Sim::_SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend ) {
    std::string function = "Andor::Sim::_SetImage";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetImage ************************************************/


  /***** Andor::Sim::_SetReadMode *********************************************/
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
  long Sim::_SetReadMode( int mode ) {
    std::string function = "Andor::Sim::_SetReadMode";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetReadMode *********************************************/


  /***** Andor::Sim::_SetShutter **********************************************/
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
  long Sim::_SetShutter( int type, int mode, int closetime, int opentime ) {
    std::string function = "Andor::Sim::_SetShutter";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_SetShutter **********************************************/


  /***** Andor::Sim::_StartAcquisition ****************************************/
  /**
   * @brief      wrapper for Andor Sim StartAcquisition
   * @details    starts the acquisition
   * @details    this simply checks return value
   * @return     NO_ERROR on DRV_SUCCESS, otherwise ERROR
   *
   */
  long Sim::_StartAcquisition() {
    std::string function = "Andor::Sim::_StartAcquisition";
    std::stringstream message;

    return NO_ERROR;
  }
  /***** Andor::Sim::_StartAcquisition ****************************************/


}
