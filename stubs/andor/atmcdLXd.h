/**
 * @file    atmcdLXd.h
 * @brief   minimal Andor SDK stub header for SDK-less builds
 *
 */

#pragma once

#include <cstdint>

typedef int at_32;
typedef unsigned int at_u32;

typedef struct AndorCapabilities {
  unsigned long ulSize;
  unsigned long ulFTReadModes;
} AndorCapabilities;

typedef enum AT_VersionInfoId {
  AT_SDKVersion = 0,
  AT_DeviceDriverVersion = 1
} AT_VersionInfoId;

#define AC_ACQMODE_FRAMETRANSFER 0x1

#define DRV_SUCCESS 0
#define DRV_ERROR_ACK 1
#define DRV_ACQUIRING 2
#define DRV_IDLE 3
#define DRV_NOT_INITIALIZED 4
#define DRV_NO_NEW_DATA 5
#define DRV_P1INVALID 6
#define DRV_P2INVALID 7
#define DRV_P3INVALID 8
#define DRV_P4INVALID 9
#define DRV_P5INVALID 10
#define DRV_P6INVALID 11
#define DRV_ACCUM_TIME_NOT_MET 12
#define DRV_KINETIC_TIME_NOT_MET 13
#define DRV_ACQ_BUFFER 14
#define DRV_SPOOLERROR 15
#define DRV_SPOOLSETUPERROR 16
#define DRV_VXDNOTINSTALLED 17
#define DRV_USBERROR 18
#define DRV_TEMP_OFF 19
#define DRV_TEMP_STABILIZED 20
#define DRV_TEMP_NOT_REACHED 21
#define DRV_TEMP_DRIFT 22
#define DRV_TEMP_NOT_STABILIZED 23
#define DRV_TEMPCYCLE 24
#define DRV_I2CTIMEOUT 25
#define DRV_I2CDEVNOTFOUND 26
#define DRV_INIERROR 27
#define DRV_ERROR_NOCAMERA 28
#define DRV_GENERAL_ERRORS 29
#define DRV_FLEXERROR 30
#define DRV_COFERROR 31
#define DRV_ERROR_FILELOAD 32
#define DRV_INVALID_FILTER 33
#define DRV_INVALID_MODE 34
#define DRV_NOT_AVAILABLE 35
#define DRV_NOT_SUPPORTED 36
#define DRV_ERROR_PAGELOCK 37
#define DRV_BINNING_ERROR 38
#define DRV_DATATYPE 39

#ifdef __cplusplus
extern "C" {
#endif

unsigned int GetCapabilities( AndorCapabilities* caps );
unsigned int GetAcquiredData16( unsigned short* buf, at_u32 bufsize );
unsigned int GetMostRecentImage16( unsigned short* buf, at_u32 bufsize );
unsigned int GetAcquiredData( int* buf, at_u32 bufsize );
unsigned int GetAvailableCameras( at_32* number );
unsigned int GetCameraHandle( at_32 index, at_32* handle );
unsigned int GetCameraSerialNumber( int* number );
unsigned int GetDetector( int* xpix, int* ypix );
unsigned int GetStatus( int* status_id );
unsigned int GetTotalNumberImagesAcquired( at_32* index );
unsigned int GetSizeOfCircularBuffer( at_32* index );
unsigned int GetNumberADChannels( int* channels );
unsigned int GetNumberHSSpeeds( int chan, int type, int* speeds );
unsigned int GetNumberVSSpeeds( int* speeds );
unsigned int GetHSSpeed( int chan, int type, int index, float* speed );
unsigned int SetHSSpeed( int type, int index );
unsigned int GetVSSpeed( int index, float* speed );
unsigned int SetVSSpeed( int index );
unsigned int SetPreAmpGain( int index );
unsigned int SetEMCCDGain( int gain );
unsigned int GetEMCCDGain( int* gain );
unsigned int GetEMGainRange( int* low, int* high );
unsigned int SetOutputAmplifier( int type );
unsigned int SetFrameTransferMode( int mode );
unsigned int GetTemperature( int* temp );
unsigned int GetTemperatureRange( int* min, int* max );
unsigned int CoolerON( void );
unsigned int CoolerOFF( void );
unsigned int SetCoolerMode( int mode );
unsigned int SetTemperature( int temp );
unsigned int SetTriggerMode( int mode );
unsigned int GetVersionInfo( AT_VersionInfoId id, char* info, at_u32 len );
unsigned int Initialize( char* dir );
unsigned int SetAcquisitionMode( int mode );
unsigned int SetCurrentCamera( at_32 handle );
unsigned int SetExposureTime( float exptime );
unsigned int SetKineticCycleTime( float time );
unsigned int SetNumberAccumulations( int number );
unsigned int SetAccumulationCycleTime( float time );
unsigned int SetNumberKinetics( int number );
unsigned int GetAcquisitionTimings( float* exp, float* acc, float* kin );
unsigned int SetImageFlip( int hflip, int vflip );
unsigned int SetImageRotate( int rotdir );
unsigned int SetImage( int hbin, int vbin, int hstart, int hend, int vstart, int vend );
unsigned int SetReadMode( int mode );
unsigned int GetShutterMinTimes( int* minclosing, int* minopening );
unsigned int SetShutter( int type, int mode, int closetime, int opentime );
unsigned int AbortAcquisition( void );
unsigned int StartAcquisition( void );
unsigned int WaitForAcquisitionTimeOut( int timeout );
unsigned int WaitForAcquisition( void );
unsigned int WaitForAcquisitionByHandleTimeOut( at_32 handle, int timeout );
unsigned int SetFanMode( int mode );
unsigned int SaveAsFITS( char* filename, int type );
unsigned int ShutDown( void );

#ifdef __cplusplus
}
#endif
