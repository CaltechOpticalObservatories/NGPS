/**
 * @file    atmcdLXd_stub.cpp
 * @brief   minimal Andor SDK stub implementation for SDK-less builds
 *
 */

#include "atmcdLXd.h"

#include <algorithm>
#include <cstring>

extern "C" {

unsigned int GetCapabilities( AndorCapabilities* caps ) {
  if ( caps ) {
    caps->ulSize = sizeof( AndorCapabilities );
    caps->ulFTReadModes = AC_ACQMODE_FRAMETRANSFER;
  }
  return DRV_SUCCESS;
}

unsigned int GetAcquiredData16( unsigned short* buf, at_u32 bufsize ) {
  if ( !buf ) return DRV_P1INVALID;
  std::fill( buf, buf + bufsize, 0 );
  return DRV_SUCCESS;
}

unsigned int GetMostRecentImage16( unsigned short* buf, at_u32 bufsize ) {
  if ( !buf ) return DRV_P1INVALID;
  std::fill( buf, buf + bufsize, 0 );
  return DRV_SUCCESS;
}

unsigned int GetAcquiredData( int* buf, at_u32 bufsize ) {
  if ( !buf ) return DRV_P1INVALID;
  std::fill( buf, buf + bufsize, 0 );
  return DRV_SUCCESS;
}

unsigned int GetAvailableCameras( at_32* number ) {
  if ( number ) *number = 1;
  return DRV_SUCCESS;
}

unsigned int GetCameraHandle( at_32 index, at_32* handle ) {
  (void)index;
  if ( handle ) *handle = 0;
  return DRV_SUCCESS;
}

unsigned int GetCameraSerialNumber( int* number ) {
  if ( number ) *number = 0;
  return DRV_SUCCESS;
}

unsigned int GetDetector( int* xpix, int* ypix ) {
  if ( xpix ) *xpix = 1024;
  if ( ypix ) *ypix = 1024;
  return DRV_SUCCESS;
}

unsigned int GetStatus( int* status_id ) {
  if ( status_id ) *status_id = DRV_IDLE;
  return DRV_SUCCESS;
}

unsigned int GetTotalNumberImagesAcquired( at_32* index ) {
  if ( index ) *index = 0;
  return DRV_SUCCESS;
}

unsigned int GetSizeOfCircularBuffer( at_32* index ) {
  if ( index ) *index = 1;
  return DRV_SUCCESS;
}

unsigned int GetNumberADChannels( int* channels ) {
  if ( channels ) *channels = 1;
  return DRV_SUCCESS;
}

unsigned int GetNumberHSSpeeds( int chan, int type, int* speeds ) {
  (void)chan;
  (void)type;
  if ( speeds ) *speeds = 1;
  return DRV_SUCCESS;
}

unsigned int GetNumberVSSpeeds( int* speeds ) {
  if ( speeds ) *speeds = 1;
  return DRV_SUCCESS;
}

unsigned int GetHSSpeed( int chan, int type, int index, float* speed ) {
  (void)chan;
  (void)type;
  (void)index;
  if ( speed ) *speed = 1.0f;
  return DRV_SUCCESS;
}

unsigned int SetHSSpeed( int type, int index ) {
  (void)type;
  (void)index;
  return DRV_SUCCESS;
}

unsigned int GetVSSpeed( int index, float* speed ) {
  (void)index;
  if ( speed ) *speed = 1.0f;
  return DRV_SUCCESS;
}

unsigned int SetVSSpeed( int index ) {
  (void)index;
  return DRV_SUCCESS;
}

unsigned int SetPreAmpGain( int index ) {
  (void)index;
  return DRV_SUCCESS;
}

unsigned int SetEMCCDGain( int gain ) {
  (void)gain;
  return DRV_SUCCESS;
}

unsigned int GetEMCCDGain( int* gain ) {
  if ( gain ) *gain = 0;
  return DRV_SUCCESS;
}

unsigned int GetEMGainRange( int* low, int* high ) {
  if ( low ) *low = 0;
  if ( high ) *high = 300;
  return DRV_SUCCESS;
}

unsigned int SetOutputAmplifier( int type ) {
  (void)type;
  return DRV_SUCCESS;
}

unsigned int SetFrameTransferMode( int mode ) {
  (void)mode;
  return DRV_SUCCESS;
}

unsigned int GetTemperature( int* temp ) {
  if ( temp ) *temp = -70;
  return DRV_TEMP_STABILIZED;
}

unsigned int GetTemperatureRange( int* min, int* max ) {
  if ( min ) *min = -100;
  if ( max ) *max = 25;
  return DRV_SUCCESS;
}

unsigned int CoolerON( void ) {
  return DRV_SUCCESS;
}

unsigned int CoolerOFF( void ) {
  return DRV_SUCCESS;
}

unsigned int SetCoolerMode( int mode ) {
  (void)mode;
  return DRV_SUCCESS;
}

unsigned int SetTemperature( int temp ) {
  (void)temp;
  return DRV_SUCCESS;
}

unsigned int SetTriggerMode( int mode ) {
  (void)mode;
  return DRV_SUCCESS;
}

unsigned int GetVersionInfo( AT_VersionInfoId id, char* info, at_u32 len ) {
  if ( !info || len == 0 ) return DRV_P1INVALID;
  const char* text = ( id == AT_DeviceDriverVersion ) ? "NGPS-STUB-DRIVER" : "NGPS-STUB-SDK";
  std::strncpy( info, text, len - 1 );
  info[ len - 1 ] = '\0';
  return DRV_SUCCESS;
}

unsigned int Initialize( char* dir ) {
  (void)dir;
  return DRV_SUCCESS;
}

unsigned int SetAcquisitionMode( int mode ) {
  (void)mode;
  return DRV_SUCCESS;
}

unsigned int SetCurrentCamera( at_32 handle ) {
  (void)handle;
  return DRV_SUCCESS;
}

unsigned int SetExposureTime( float exptime ) {
  (void)exptime;
  return DRV_SUCCESS;
}

unsigned int SetKineticCycleTime( float time ) {
  (void)time;
  return DRV_SUCCESS;
}

unsigned int SetNumberAccumulations( int number ) {
  (void)number;
  return DRV_SUCCESS;
}

unsigned int SetAccumulationCycleTime( float time ) {
  (void)time;
  return DRV_SUCCESS;
}

unsigned int SetNumberKinetics( int number ) {
  (void)number;
  return DRV_SUCCESS;
}

unsigned int GetAcquisitionTimings( float* exp, float* acc, float* kin ) {
  if ( exp ) *exp = 0.0f;
  if ( acc ) *acc = 0.0f;
  if ( kin ) *kin = 0.0f;
  return DRV_SUCCESS;
}

unsigned int SetImageFlip( int hflip, int vflip ) {
  (void)hflip;
  (void)vflip;
  return DRV_SUCCESS;
}

unsigned int SetImageRotate( int rotdir ) {
  (void)rotdir;
  return DRV_SUCCESS;
}

unsigned int SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend ) {
  (void)hbin;
  (void)vbin;
  (void)hstart;
  (void)hend;
  (void)vstart;
  (void)vend;
  return DRV_SUCCESS;
}

unsigned int SetReadMode( int mode ) {
  (void)mode;
  return DRV_SUCCESS;
}

unsigned int GetShutterMinTimes( int* minclosing, int* minopening ) {
  if ( minclosing ) *minclosing = 0;
  if ( minopening ) *minopening = 0;
  return DRV_SUCCESS;
}

unsigned int SetShutter( int type, int mode, int closetime, int opentime ) {
  (void)type;
  (void)mode;
  (void)closetime;
  (void)opentime;
  return DRV_SUCCESS;
}

unsigned int AbortAcquisition( void ) {
  return DRV_SUCCESS;
}

unsigned int StartAcquisition( void ) {
  return DRV_SUCCESS;
}

unsigned int WaitForAcquisitionTimeOut( int timeout ) {
  (void)timeout;
  return DRV_SUCCESS;
}

unsigned int WaitForAcquisition( void ) {
  return DRV_SUCCESS;
}

unsigned int WaitForAcquisitionByHandleTimeOut( at_32 handle, int timeout ) {
  (void)handle;
  (void)timeout;
  return DRV_SUCCESS;
}

unsigned int SetFanMode( int mode ) {
  (void)mode;
  return DRV_SUCCESS;
}

unsigned int SaveAsFITS( char* filename, int type ) {
  (void)filename;
  (void)type;
  return DRV_SUCCESS;
}

unsigned int ShutDown( void ) {
  return DRV_SUCCESS;
}

}  // extern "C"
