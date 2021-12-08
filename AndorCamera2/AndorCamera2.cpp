/* *
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 */
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
//#include "atcore.h"
#include "fitsio.h"
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <time.h>
#include "math.h"
//#include <cufft.h>
//#include <cublas_v2.h>
//#include <cuda_runtime_api.h>
//#include <GL/freeglut.h>
//#include <cuda_gl_interop.h>
//#include <cv.h>
//#include <highgui.h>
//#include <helper_functions.h>  // helper for shared functions common to CUDA SDK samples
//#include <helper_cuda.h>       // helper functions for CUDA error checking and initialization
//#include <X11/X.h>
//#include <X11/Xlib.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
//#include "discpp.h"
//#include <boost/bind.hpp>
//#include <boost/smart_ptr.hpp>
//#include <boost/asio.hpp>
//#include <boost/thread/thread.hpp>
//#include "plot.h"
#include "atmcdLXd.h"
#include "AndorCamera2.h"
//=== File Prolog =============================================================
//	This code was developed by California Institute of Technology
//      Caltech Optical Observatories, Palomar Observatory
//
//--- Contents ----------------------------------------------------------------
//	 AndorCamera2.cpp - Java JNI Native Interface to the Andor SDK 2 camera
//   (for example the iXon Ultra 897, 888 and 885
//
//--- Description -------------------------------------------------------------
//--- Notes -------------------------------------------------------------------
//
//--- Development History -----------------------------------------------------
//
//      July 30, 2013 Jennifer Milburn
//        Original Implementation
//
//--- DISCLAIMER---------------------------------------------------------------
//
//	This software is provided "as is" without any warranty of any kind, either
//	express, implied, or statutory, including, but not limited to, any
//	warranty that the software will conform to specification, any implied
//	warranties of merchantability, fitness for a particular purpose, and
//	freedom from infringement, and any warranty that the documentation will
//	conform to the program, or any warranty that the software will be error
//	free.
//
//	In no event shall Caltech be liable for any damages, including, but not
//	limited to direct, indirect, special or consequential damages, arising out
//	of, resulting from, or in any way connected with this software, whether or
//	not based upon warranty, contract, tort or otherwise, whether or not
//	injury was sustained by persons or property or otherwise, and whether or
//	not loss was sustained from or arose out of the results of, or use of,
//	their software or services provided hereunder.
//
//=== End File Prolog =========================================================
/*=============================================================================================
/      class AndorCamera2.cpp
/=============================================================================================*/
/*=============================================================================================
/      READ MODE PARAMETERS
/=============================================================================================*/
const int READ_MODE_IMAGE                 = 4;
const int READ_MODE_SINGLE_TRACK          = 3;
const int READ_MODE_RANDOM_TRACK          = 2;
const int READ_MODE_MULTI_TRACK           = 1;
const int READ_MODE_FULL_VERTICAL_BINNING = 0;
/*=============================================================================================
/      ACQUISITION MODE PARAMETERS
/=============================================================================================*/
const int ACQUISITION_MODE_SINGLE_SCAN     = 1;
const int ACQUISITION_MODE_ACCUMULATE      = 2;
const int ACQUISITION_MODE_KINETICS        = 3;
const int ACQUISITION_MODE_FAST_KINETIC    = 4;
const int ACQUISITION_MODE_RUN_TILL_ABORT  = 5;
/*=============================================================================================
/     SHUTTER PARAMETERS
/=============================================================================================*/
const int SHUTTER_OUTPUT_TTL_HIGH = 1;
const int SHUTTER_OUTPUT_TTL_LOW  = 0;
const int SHUTTER_OPEN            = 1;
const int SHUTTER_CLOSE           = 2;
const int SHUTTER_AUTO            = 0;
const int SHUTTER_CLOSING_TIME    = 50;
const int SHUTTER_OPENING_TIME    = 50;
/*=============================================================================================
/     IMAGE BIT TYPE PARAMETERS
/=============================================================================================*/
const int IMAGE_TYPE_UNSIGNED_16  = 0;
const int IMAGE_TYPE_UNSIGNED_32  = 1;
const int IMAGE_TYPE_SIGNED_16    = 2;
const int IMAGE_TYPE_SIGNED_32    = 3;
const int IMAGE_TYPE_FLOAT        = 4;
/*=============================================================================================
/     FAN PARAMETERS
/=============================================================================================*/
int FAN_ON_FULL = 0;
int FAN_ON_LOW  = 1;
int FAN_OFF     = 2;
/*=============================================================================================
/     READOUT MODE PARAMETERS
/=============================================================================================*/
int READOUT_MODE_FULL_VERTICAL_BINNING = 0;
int READOUT_MODE_MULTI_TRACK           = 1;
int READOUT_MODE_RANDOM_TRACK          = 2;
int READOUT_MODE_SINGLE_TRACK          = 3;
int READOUT_MODE_IMAGE                 = 4;
/*=============================================================================================
/     AMPLIFIER_TYPE PARAMETERS
/=============================================================================================*/
int AMPLIFIER_TYPE_ELECTRON_MULTIPLICATION = 0;
int AMPLIFIER_TYPE_CONVENTIONAL            = 1;
/*=============================================================================================
/     EMGAIN PARAMETERS
/=============================================================================================*/
// Mode = 0 The EM Gain is controlled by DAC settings in the range 0-255.  Default mode
// Mode = 1 The EM Gain is controlled by DAC settings in the range 0-4095
// Mode = 2 Linear mode
// Mode = 3 Real EM gain
int EMGAIN_0 = 0;
int EMGAIN_1 = 1;
int EMGAIN_2 = 2;
int EMGAIN_3 = 3;
/*=============================================================================================
/     AMPLIFIER PARAMETERS
/=============================================================================================*/
 int EMGAIN_AMPLIFIER       = 0;
 int CONVENTIONAL_AMPLIFIER = 1;
/*=============================================================================================
/     EXPOSURE PROGRESS PARAMETERS
/=============================================================================================*/
// Camera 1
double exposure_progress_cam1;
double read_progress_cam1;
double write_progress_cam1;
double observation_progress_cam1;
double error_progress_cam1;
// Camera 2
double exposure_progress_cam2;
double read_progress_cam2;
double write_progress_cam2;
double observation_progress_cam2;
double error_progress_cam2;
// state booleans
bool   exposing;
bool   writing;
bool   reading;
typedef struct{
 unsigned int bit : 1;
}Bit;
/*=============================================================================================
/      CAPABILITIES STRUCTURE
/=============================================================================================*/
typedef struct{
 int  CameraType;
 int  PixelMode;
 int  PCICard;
 // ACQUISTION MODES
 bool AcqModes_SINGLE;
 bool AcqModes_VIDEO;
 bool AcqModes_ACCUMULATE;
 bool AcqModes_KINETIC;
 bool AcqModes_FRAMETRANSFER;
 bool AcqModes_FASTKINETICS;
 bool AcqModes_OVERLAP;
 // READ MODES
 bool ReadModes_FULLIMAGE;
 bool ReadModes_SUBIMAGE;
 bool ReadModes_SINGLETRACK;
 bool ReadModes_FVB;
 bool ReadModes_MULTITRACK;
 bool ReadModes_RANDOMTRACK;
 // FRAME TRANSFER READ MODES
 bool FTReadModes_FULLIMAGE;
 bool FTReadModes_SUBIMAGE;
 bool FTReadModes_SINGLETRACK;
 bool FTReadModes_FVB;
 bool FTReadModes_MULTITRACK;
 bool FTReadModes_RANDOMTRACK;
 // TRIGGER MODES
 bool TriggerModes_INTERNAL;
 bool TriggerModes_EXTERNAL;
 bool TriggerModes_EXTERNAL_FVB_EM;
 bool TriggerModes_CONTINUOUS;
 bool TriggerModes_EXTERNALSTART;
 bool TriggerModes_BULB;
 bool TriggerModes_EXTERNALEXPOSURE;
 bool TriggerModes_INVERTED;
 bool TriggerModes_EXTERNALCHARGESHIFTED;
 // PIXEL MODES
 bool PixelMode_8BIT;
 bool PixelMode_14BIT;
 bool PixelMode_16BIT;
 bool PixelMode_32BIT;
 // SET FUNCTIONS
 bool SetFunction_VREADOUT;
 bool SetFunction_HREADOUT;
 bool SetFunction_TEMPERATURE;
 bool SetFunction_MCPGAIN;
 bool SetFunction_EMCCDGAIN;
 bool SetFunction_BASELINECLAMP;
 bool SetFunction_VSAMPLITUDE;
 bool SetFunction_HIGHCAPACITY;
 bool SetFunction_BASELINEOFFSET;
 bool SetFunction_PREAMPGAIN;
 bool SetFunction_CROPMODE;
 bool SetFunction_DMAPARAMETERS;
 bool SetFunction_HORIZONTALBIN;
 bool SetFunction_MULTITRACKHRANGE;
 bool SetFunction_RANDOMTRACKNOGAPS;
 bool SetFunction_EMADVANCED;
 bool SetFunction_GATEMODE;
 bool SetFunction_DDGTIMES;
 bool SetFunction_IOC;
 bool SetFunction_INTELLIGATE;
 bool SetFunction_INSERTION_DELAY;
 bool SetFunction_GATESTEP;
 bool SetFunction_TRIGGERTERMINATION;
 bool SetFunction_EXTENDEDNIR;
 bool SetFunction_SPOOLTHREADCOUNT;
 // GET FUNCTIONS
 bool GetFunction_TEMPERATURE;
 bool GetFunction_TEMPERATURERANGE;
 bool GetFunction_DETECTORSIZE;
 bool GetFunction_MCPGAIN;
 bool GetFunction_EMCCDGAIN;
 bool GetFunction_GATEMODE;
 bool GetFunction_DDGTIMES;
 bool GetFunction_IOC;
 bool GetFunction_INTELLIGATE;
 bool GetFunction_INSERTION_DELAY;
 bool GetFunction_PHOSPHORSTATUS;
 bool GetFunction_BASELINECLAMP;
 // FEATURES
 bool Features_POLLING;
 bool Features_EVENTS;
 bool Features_SPOOLING;
 bool Features_SHUTTER;
 bool Features_SHUTTEREX;
 bool Features_EXTERNAL_I2C;
 bool Features_SATURATIONEVENT;
 bool Features_FANCONTROL;
 bool Features_MIDFANCONTROL;
 bool Features_TEMPERATURE_DURING_ACQUISITION;
 bool Features_KEEP_CLEAN_CONTROL;
 bool Features_DDGLITE;
 bool Features_FT_EXTERNAL_EXPOSURE;
 bool Features_KINETIC_EXTERNAL_EXPOSURE;
 bool Features_DAC_CONTROL;
 bool Features_METADATA;
 bool Features_IOCONTROL;
 bool Features_PHOTONCOUNTING;
 bool Features_COUNTCONVERT;
 bool Features_DUALMODE;
 // EMGAIN CAPABILITIES
 bool EMGain_8BIT;
 bool EMGain_12BIT;
 bool EMGain_LINEAR12;
 bool EMGain_REAL12;
}Capabilities;
Capabilities           *current_capabilities;
/*=============================================================================================
/     CAMERA ID PARAMETERS
/=============================================================================================*/
const int MAX_HEADER_CARDS  = 200;
int        CAMERA_1       = -1;
int        CAMERA_2       = -1;
int        CAMERA_1_ID    = 0;
int        CAMERA_2_ID    = 1;
bool       ABORT_CAMERA_1 = false;
bool       ABORT_CAMERA_2 = false;
/*=============================================================================================
/     HEADER CARD PARAMETERS
/=============================================================================================*/
char           *header_card_array_cam1[MAX_HEADER_CARDS];
int             header_counter_cam1 = 0;
char           *header_card_array_cam2[MAX_HEADER_CARDS];
int             header_counter_cam2 = 0;
/*=============================================================================================
/     MISCELLANEOUS PARAMETERS
/=============================================================================================*/
int  MISSING_INTEGER = -99;
int  sdk_error      = DRV_SUCCESS;
int  ON             = 1;
int  OFF            = 0;
bool is_initialized = false;
/*=============================================================================================
/      FUNCTION PROTOTYPES
/=============================================================================================*/
bool  getBit(int value,int position);
void  setSDKError(int newError);
int   getSDKError();
void  setInitialized(bool newInitialized);
bool  isInitialized();
/*=============================================================================================
/      getBit
/=============================================================================================*/
bool getBit(int value,int position){
	Bit singleBit;
	bool state = false;
	singleBit.bit = 0;
	if(position == 0){
		singleBit.bit = value & 1;
	}else{
		singleBit.bit = (value &(1<<position))>>position;
	}
	if(singleBit.bit == 0){
		state = false;
	}
	if(singleBit.bit == 1){
		state = true;
	}
	return state;
}
/****************************************************/
/* Print out cfitsio error messages and exit program */
/*****************************************************/
/*--------------------------------------------------------------------------*/
void printerror( int status){
    if (status){
       fits_report_error(stderr, status); /* print error report */
       printf("FITS error %i",status);
//       exit( status );    /* terminate the program, returning error status */
    }
    return;
}
/******************************************************/
/*      Create a FITS primary array containing a 2-D image */
/******************************************************/
void writeAcquireimage(char *filename,int naxis_1,int naxis_2,int *data_array ){
    fitsfile *fptr;       /* pointer to the FITS file, defined in fitsio.h */
    int status;
    long nelements;
    /* initialize FITS image parameters */
//    char filename[] = "atestfil.fits";             /* name for new FITS file */
    int bitpix   =  LONG_IMG; /* 16-bit unsigned short pixel values       */
    long naxis    =   2;  /* 2-dimensional image                            */
    long naxes[2] = { naxis_1, naxis_2 };   /* image is 300 pixels wide by 200 rows */
    remove(filename);               /* Delete old file if it already exists */
    status = 0;         /* initialize status before calling fitsio routines */
    if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
         printerror( status );           /* call printerror if error occurs */
    if ( fits_create_img(fptr,  bitpix, naxis, naxes, &status) )
         printerror( status );
//    fpixel = 1;                               /* first pixel to write      */
    nelements = naxes[0] * naxes[1];          /* number of pixels to write *
    / write the array of unsigned integers to the FITS file */
    if ( fits_write_2d_int(fptr, 0, naxis_1,naxis_1,naxis_2,data_array, &status) )
        printerror( status );
    if ( fits_close_file(fptr, &status) )                /* close the file */
         printerror( status );
    return;
}
/*=============================================================================================
/     setSDKError(int newError)
/=============================================================================================*/
void setSDKError(int newError){
	sdk_error = newError;
}
int  getSDKError(){
	return sdk_error;
}
/*=============================================================================================
/     setInitialized(bool newInitialized)
/=============================================================================================*/
void setInitialized(bool newInitialized){
	is_initialized = newInitialized;
}
bool  isInitialized(){
	return is_initialized;
}
/*=============================================================================================
/     Java_andor_AndorCamera2_1test_acquire
/=============================================================================================*/
JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_acquire(JNIEnv *env, jclass obj, jint handle, jstring fileName, jboolean isWriteFile){
	// Parameter declaration section
	int width, height;
	at_32*     imageData;
	int        status;
	int        datasize;
    at_32      accumulations, kinetics;
    int        error;
    jintArray  jimage_array;
	const char *nativeString;
	char       *current_filename;

	  if(handle == CAMERA_1){
	    exposure_progress_cam1 = 0.0;
	    read_progress_cam1     = 0.0;
	    write_progress_cam1    = 0.0;
	  }
	  if(handle == CAMERA_2){
	    exposure_progress_cam2 = 0.0;
	    read_progress_cam2     = 0.0;
	    write_progress_cam2    = 0.0;
	  }
	  error = SetReadMode(READ_MODE_IMAGE);                      //Set Read Mode to --Image--
	  error = SetAcquisitionMode(ACQUISITION_MODE_SINGLE_SCAN);  //Set Acquisition mode to --Single scan--
	  error = GetDetector(&width, &height);                      //Get Detector dimensions
	  error = SetExposureTime(0.1);                              //Set initial exposure time
	  error = SetShutter(SHUTTER_OUTPUT_TTL_HIGH,SHUTTER_AUTO,SHUTTER_CLOSING_TIME,SHUTTER_OPENING_TIME);//Initialize Shutter
	  error = SetImage(1,1,1,width,1,height);                    //Setup Image dimensions
	  printf("Height = %d Width = %d \n",height,width);
	  jsize  stringlength     = env->GetStringLength(fileName);
	         nativeString     = new char[stringlength];
	         current_filename = new char[stringlength];
	  // Dealing with the problem of passing a const char * to the writeFITS method, copying the native_filename to current_filename
	  nativeString = env->GetStringUTFChars(fileName,0);
	  sprintf(current_filename,"%s",nativeString);
	  printf("Current File Name = %s\n",current_filename);

		imageData = new at_32[width*height];
		error = StartAcquisition();
		//Loop until acquisition finished
		error = GetStatus(&status);
		while(status==DRV_ACQUIRING){
			error = GetStatus(&status);
		}
		error = GetAcquiredData(imageData, width*height);
//		for(int i=0;i<width*height;i++){
//			printf("%i\n",imageData[i]);
//		}
	    if(isWriteFile){
	    	writeAcquireimage(current_filename,width,height,imageData);
	    }
	env->ReleaseStringUTFChars(fileName,nativeString);
	jimage_array = env->NewIntArray(width*height);
	printf("Writing image to Java Array\n");
	env->SetIntArrayRegion(jimage_array,0,width*height,imageData);
	free(imageData);
	free(current_filename);
  return jimage_array;
}
/*=============================================================================================
/      Java_andor_AndorCamera_setCamera(JNIEnv *env, jclass obj,jint handle,jint camera_ID)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    setCamera
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_setCamera(JNIEnv *env, jclass obj, jint handle, jint camera_ID){
    if(camera_ID == CAMERA_1_ID){
      CAMERA_1 = handle;
    }
    if(camera_ID == CAMERA_2_ID){
      CAMERA_2 = handle;
    }
    return 0;
}
/*=============================================================================================
/      Java_andor_AndorCamera2_1test_getProgress(JNIEnv *env, jclass obj, jint handle)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    getProgress
 * Signature: (I)[D
 */
JNIEXPORT jdoubleArray JNICALL Java_andor2_AndorCamera2_getProgress(JNIEnv *env, jclass obj, jint handle){
double *progress_array;
progress_array = new double[5];
//    printf("Inside getProgress handle = %i CAMERA_1 = %i CAMERA_2 = %i\n",handle,CAMERA_1,CAMERA_2);
if(handle == CAMERA_1){
  progress_array[0] = exposure_progress_cam1;
  progress_array[1] = read_progress_cam1;
  progress_array[2] = write_progress_cam1;
  progress_array[3] = observation_progress_cam1;
  progress_array[4] = error_progress_cam1;

//      printf("Inside getProgress exposure = %d read = %d write = %d observation = %d\n",exposure_progress_cam1,read_progress_cam1,write_progress_cam1,observation_progress_cam1);
}
if(handle == CAMERA_2){
  progress_array[0] = exposure_progress_cam2;
  progress_array[1] = read_progress_cam2;
  progress_array[2] = write_progress_cam2;
  progress_array[3] = observation_progress_cam2;
  progress_array[4] = error_progress_cam2;
//     printf("Inside getProgress exposure = %d read = %d write = %d observation = %d\n",exposure_progress_cam2,read_progress_cam2,write_progress_cam2,observation_progress_cam2);
}
jdoubleArray jprogress_array = env->NewDoubleArray(5);
env->SetDoubleArrayRegion(jprogress_array,0,5,progress_array);
free(progress_array);
return jprogress_array;
}
/*=============================================================================================
/      Java_andor_AndorCamera_addHeaderCard(JNIEnv *env, jobject obj, jstring card)
/=============================================================================================*/
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_addHeaderCard(JNIEnv *env, jobject obj, jstring card,jint handle){
 int             i_err         = 0;
 const char      *native_card;
   jsize stringlength = env->GetStringLength(card);
   native_card        = new char[stringlength];
   char* current_card = (char *)malloc(stringlength* sizeof(char));;
   native_card = env->GetStringUTFChars(card,0);
   sprintf(current_card,"%s",native_card);
   if(handle == CAMERA_1){
     header_card_array_cam1[header_counter_cam1] = current_card;
     header_counter_cam1 = header_counter_cam1+1;
   }
   if(handle == CAMERA_2){
     header_card_array_cam2[header_counter_cam2] = current_card;
     header_counter_cam2 = header_counter_cam2+1;
   }
   printf("%s\n",current_card);
   env->ReleaseStringUTFChars(card,native_card);
 return i_err;
}
/*=============================================================================================
/    Java_andor2_AndorCamera2_initializeCamera(JNIEnv *env, jclass obj, jint selected_cameraIndex)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    GetCameraHandle
 * Signature: (I)J
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_initializeCamera(JNIEnv *env, jclass obj, jint selected_cameraIndex){
   at_32 cameraHandle   = MISSING_INTEGER;
   at_32 cameraHandle_1 = MISSING_INTEGER;
   at_32 cameraHandle_2 = MISSING_INTEGER;
   at_32 lNumCameras    = MISSING_INTEGER;
   int   camera_index   = MISSING_INTEGER;
    int   error = 0;
	  error = GetAvailableCameras(&lNumCameras);
	  if(error == DRV_SUCCESS){
            printf("Number of Available Cameras = %i  error = %i\n",lNumCameras,error);
          }
          if(lNumCameras == 1){
            camera_index = 0;
    	    error = GetCameraHandle(camera_index, &cameraHandle_1);
            printf("Camera 1 Handle = %i  error = %i\n",cameraHandle_1,error);
          }
          if(lNumCameras == 2){
            camera_index = 0;
            error = GetCameraHandle(camera_index, &cameraHandle_1);
            printf("Camera 1 Handle = %i  error = %i\n",cameraHandle_1,error);
            camera_index = 1;
    	    error = GetCameraHandle(camera_index, &cameraHandle_2);
            printf("Camera 2 Handle = %i  error = %i\n",cameraHandle_2,error);
          }
          if(selected_cameraIndex == 1){
            cameraHandle = cameraHandle_1;
          }
          if(selected_cameraIndex == 2){
            cameraHandle = cameraHandle_2;
          }
	  error = SetCurrentCamera(cameraHandle);
	  if(error == DRV_SUCCESS){
	      printf("Setting Current Camera = %i\n",cameraHandle);
	  }
      char drv[] = "/usr/local/etc/andor";
      error = Initialize( drv );
	   if(error == DRV_SUCCESS){
		   setInitialized(true);
		   printf("Driver Library Initialized\n");
	   }
	   if(error!=DRV_SUCCESS){
		   setInitialized(false);
		   printf("Driver Library NOT Initialized!\n");
	   }
   setSDKError(error);
   return error;
}
/*=============================================================================================
/     Java_andor_AndorCamera2_test_Initialize(JNIEnv *env, jclass obj, jstring dir)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2
 * Method:    Initialize
 * Signature: (Ljava/lang/String;)
 * I
 **/
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_Initialize(JNIEnv *env, jclass obj){
// PROBLEM WITH PASSING THE nativeString (which is a const char to a method that requires a character pointer
//   jsize stringlength = env->GetStringLength(dir);
//   const char *nativeString;
//   char       *native_dir;
//   nativeString = new char[stringlength];
//   native_dir   = new char[stringlength];
//   nativeString = env->GetStringUTFChars(dir,0);
//   sprintf(native_dir,"%s",nativeString);

   char drv[] = "/usr/local/etc/andor";
   int error = Initialize( drv );
	   if(error == DRV_SUCCESS){
		   setInitialized(true);
		   printf("Driver Library Initialized\n");
	   }
	   if(error!=DRV_SUCCESS){
		   setInitialized(false);
		   printf("Driver Library NOT Initialized!\n");
	   }
 //  env->ReleaseStringUTFChars(dir,nativeString);
return error;
}
/*=============================================================================================
/     Java_andor_AndorCamera2_1test_GetAvailableCameras
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    GetAvailableCameras
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetAvailableCameras(JNIEnv *env, jclass obj){
  at_32 lNumCameras;
  int   error = 0;
  if(isInitialized()){
	  error = GetAvailableCameras(&lNumCameras);
	  if(error == DRV_SUCCESS){
	    printf("Available Cameras = %i\n",lNumCameras);
	  }
  }
 setSDKError(error);
return lNumCameras;
}
/*=============================================================================================
/    Java_andor_AndorCamera2_1test_GetCameraHandle(JNIEnv *env, jclass obj, jint cameraIndex)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    GetCameraHandle
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_andor2_AndorCamera2_GetCameraHandle(JNIEnv *env, jclass obj, jint cameraIndex){
   at_32 cameraHandle = MISSING_INTEGER;
   int   error = 0;
    if(isInitialized()){
    	error = GetCameraHandle(cameraIndex, &cameraHandle);
    	if(error == DRV_SUCCESS){
 	     printf("Setting Current Camera = %i\n",cameraHandle);
 	   }
    }
   setSDKError(error);
return cameraHandle;
}
/*=============================================================================================
/      IMPLEMENTATION OF THE SDK 2 METHODS
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2
 * Method:    SetCurrentCamera
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetCurrentCamera(JNIEnv *env, jclass obj, jint cameraHandle){
	   int   error = 0;
	    if(isInitialized()){
	    	error = SetCurrentCamera(cameraHandle);
	    	if(error == DRV_SUCCESS){
	    	  printf("Setting Current Camera = %i\n",cameraHandle);
	    	}
	    }
  setSDKError(error);
return error;
}
/*=============================================================================================
/    Java_andor_AndorCamera2_1test_GetCurrentCamera(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    GetCurrentCamera
 * Signature: ()I
 */
//JNIEXPORT jint JNICALL Java_andor_AndorCamera2_1test_GetCurrentCamera
//  (JNIEnv *, jclass);
JNIEXPORT jint JNICALL Java_andor_AndorCamera2_1test_GetCurrentCamera(JNIEnv *env, jclass obj){
   int cameraHandle = MISSING_INTEGER;
   int   error = 0;
       if(isInitialized()){
    	   error = GetCurrentCamera(&cameraHandle);
    	   if(error == DRV_SUCCESS){
    	     printf("Getting Current Camera = %i\n",cameraHandle);
    	   }
       }
   setSDKError(error);
return cameraHandle;
}
/*=============================================================================================
/    Java_andor_AndorCamera2_1test_ShutDown(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    ShutDown
 * Signature: ()I
 */
/*=============================================================================================
/    Java_andor_AndorCamera2_1test_PrepareAcquisition(JNIEnv *env, jclass obj)
/=============================================================================================*/
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_ShutDown(JNIEnv *env, jclass obj){
	  int   error = 0;
	    if(isInitialized()){
	    	error = ShutDown();
	    	if(error == DRV_SUCCESS){
	    	  setInitialized(false);
	    	  printf("Shutting down camera library\n");
	    	}
	    }
	  setSDKError(error);
	return error;
}
/*=============================================================================================
/    Java_andor2_AndorCamera2_EnableKeepCleans
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    EnableKeepCleans
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_EnableKeepCleans(JNIEnv *env, jclass obj, jint mode){
	  int   error = 0;
	  int   native_mode = (int)mode;
	    if(isInitialized()){
	    	error = EnableKeepCleans(native_mode);
	    	if(error == DRV_SUCCESS){
	    	  printf("EnableKeepCleans Success\n");
	    	}
	     	if(error == DRV_NOT_INITIALIZED){
	    	  printf("EnableKeepCleans Camera NOT initialized = %i\n",error);
	     	}
	     	if(error == DRV_NOT_AVAILABLE){
	    	  printf("EnableKeepCleans Feature not available = %i\n",error);
	     	}
	    }
	  setSDKError(error);
	return error;

}
/*=============================================================================================
/    Java_andor_AndorCamera2_1test_PrepareAcquisition(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    PrepareAcquisition
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_PrepareAcquisition(JNIEnv *env, jclass obj){
int   error = 0;
 if(isInitialized()){
 	error = PrepareAcquisition();
 	if(error == DRV_SUCCESS){
 	  printf("PrepareAcquisition SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	      printf("PrepareAcquisition Camera NOT initialized = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	      printf("PrepareAcquisition Acquisition in Progress = %i\n",error);
 	}
 	if(error == DRV_VXDNOTINSTALLED){
	      printf("PrepareAcquisition VxD not loaded = %i\n",error);
 	}
 	if(error == DRV_ERROR_ACK){
	      printf("PrepareAcquisition Unable to communicate with card = %i\n",error);
 	}
 	if(error == DRV_INIERROR){
	      printf("PrepareAcquisition Error reading the DETECTOR.INI = %i\n",error);
 	}
// 	if(error == DRV_ACQERROR){
//	      printf("PrepareAcquisition Acquisition setting invalid = %i\n",error);
// 	}
 	if(error == DRV_ERROR_PAGELOCK){
	      printf("PrepareAcquisition Unable to allocate memory = %i\n",error);
 	}
 	if(error == DRV_INVALID_FILTER){
	      printf("PrepareAcquisition Filter not available for current acquisition = %i\n",error);
 	}
 	if(error == DRV_IOCERROR){
	      printf("PrepareAcquisition Integration On Chip setup error = %i\n",error);
 	}
 	if(error == DRV_BINNING_ERROR){
	      printf("PrepareAcquisition Range not multiple of horizontal binning = %i\n",error);
 	}
 }
setSDKError(error);
return error;
}
/*=============================================================================================
/   Java_andor_AndorCamera2_1test_StartAcquisition(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    StartAcquisition
 * Signature: ()I
 */
//JNIEXPORT jint JNICALL Java_andor_AndorCamera2_1test_StartAcquisition
//  (JNIEnv *, jclass);
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_StartAcquisition(JNIEnv *env, jclass obj){
int   error = 0;
 if(isInitialized()){
 	error = StartAcquisition();
 	if(error == DRV_SUCCESS){
 	  printf("StartAcquisition SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	      printf("StartAcquisition Camera NOT initialized = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	      printf("StartAcquisition Acquisition in Progress = %i\n",error);
 	}
 	if(error == DRV_VXDNOTINSTALLED){
	      printf("StartAcquisition VxD not loaded = %i\n",error);
 	}
 	if(error == DRV_ERROR_ACK){
	      printf("StartAcquisition Unable to communicate with card = %i\n",error);
 	}
 	if(error == DRV_INIERROR){
	      printf("StartAcquisition Error reading the DETECTOR.INI = %i\n",error);
 	}
//	    	if(error == DRV_ACQERROR){
//		      printf("PrepareAcquisition Acquisition setting invalid = %i\n",error);
//	    	}
 	if(error == DRV_ERROR_PAGELOCK){
	      printf("StartAcquisition Unable to allocate memory = %i\n",error);
 	}
 	if(error == DRV_INVALID_FILTER){
	      printf("StartAcquisition Filter not available for current acquisition = %i\n",error);
 	}
 	if(error == DRV_IOCERROR){
	      printf("StartAcquisition Integration On Chip setup error = %i\n",error);
 	}
 	if(error == DRV_BINNING_ERROR){
	      printf("StartAcquisition Range not multiple of horizontal binning = %i\n",error);
 	}
 }
setSDKError(error);
return error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SendSoftwareTrigger
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2
 * Method:    SendSoftwareTrigger
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SendSoftwareTrigger(JNIEnv *env, jclass obj){
int   error = 0;
 if(isInitialized()){
 	error = SendSoftwareTrigger();
 	if(error == DRV_SUCCESS){
 	  printf("SendSoftwareTrigger SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	      printf("SendSoftwareTrigger Camera NOT initialized = %i\n",error);
 	}
 	if(error == DRV_INVALID_MODE){
	      printf("SendSoftwareTrigger Not in Software Trigger mode = %i\n",error);
 	}
 	if(error == DRV_IDLE){
	      printf("SendSoftwareTrigger Not Acquiring = %i\n",error);
 	}
 	if(error == DRV_ERROR_CODES){
	      printf("SendSoftwareTrigger Error communicating with camera= %i\n",error);
 	}
 	if(error == DRV_ERROR_ACK){
	      printf("SendSoftwareTrigger Previous acquisition not complete = %i\n",error);
 	}
 }
setSDKError(error);
return error;
}
/*=============================================================================================
/   Java_andor_AndorCamera2_1test_AbortAcquisition(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    AbortAcquisition
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_AbortAcquisition(JNIEnv *env, jclass obj){
int   error = 0;
 if(isInitialized()){
 	error = AbortAcquisition();
 	if(error == DRV_SUCCESS){
 	     printf("AbortAcquisition SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	     printf("AbortAcquisition Camera NOT initialized = %i\n",error);
 	}
 	if(error == DRV_IDLE){
	     printf("AbortAcquisition Not Acquiring = %i\n",error);
 	}
 	if(error == DRV_VXDNOTINSTALLED){
	     printf("AbortAcquisition VxD not loaded= %i\n",error);
 	}
 	if(error == DRV_ERROR_ACK){
	     printf("AbortAcquisition Unable to communicate with card = %i\n",error);
 	}
  }
  setSDKError(error);
return error;
}
/*=============================================================================================
/   Java_andor_AndorCamera2_1test_CancelWait(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    CancelWait
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_CancelWait(JNIEnv *env, jclass obj){
int   error = 0;
 if(isInitialized()){
 	error = CancelWait();
 	if(error == DRV_SUCCESS){
 	  printf("CancelWait SUCCESSFUL = %i\n",error);
 	}
 }
setSDKError(error);
return error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_FreeInternalMemory
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    FreeInternalMemory
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_FreeInternalMemory(JNIEnv *env, jclass obj){
int   error = 0;
 if(isInitialized()){
 	error = FreeInternalMemory();
 	if(error == DRV_SUCCESS){
 	  printf("AbortAcquisition SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	      printf("AbortAcquisition Camera NOT initialized = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	      printf("AbortAcquisition Acquisition in progress = %i\n",error);
 	}
 	if(error == DRV_ERROR_ACK){
	      printf("AbortAcquisition Unable to communicate with card = %i\n",error);
 	}
 }
setSDKError(error);
return error;
}
/*=============================================================================================
/   Java_andor_AndorCamera2_1test_SetExposureTime(JNIEnv *env, jclass obj, jfloat exposureTime)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    SetExposureTime
 * Signature: (F)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetExposureTime(JNIEnv *env, jclass obj, jfloat exposureTime){
int   error = 0;
 if(isInitialized()){
 	error = SetExposureTime(exposureTime);
 	if(error == DRV_SUCCESS){
 	  printf("SetExposureTime SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	      printf("SetExposureTime Camera NOT initialized = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	      printf("SetExposureTime Acquisition in progress = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	      printf("SetExposureTime Exposure Time invalid = %i\n",error);
 	}
 }
setSDKError(error);
return error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetFKExposureTime(JNIEnv *, jclass)
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetFKExposureTime
 * Signature: ()F
 */
JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetFKExposureTime(JNIEnv *env, jclass obj){
	int     error = 0;
	float   exposure_time = 0.0;
	 if(isInitialized()){
	 	error = GetFKExposureTime(&exposure_time);
	 	if(error == DRV_SUCCESS){
	 	  printf("GetFKExposureTime SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		  printf("GetFKExposureTime Camera NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		  printf("GetFKExposureTime Acquisition in progress = %i\n",error);
	 	}
	 	if(error == DRV_INVALID_MODE){
		  printf("GetFKExposureTime Fast kinetics is not available = %i\n",error);
	 	}
	 }
	setSDKError(error);
	return exposure_time;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetHighCapacity
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetHighCapacity
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetHighCapacity(JNIEnv *env, jclass obj, jint capacity){
	int   error = 0;
	int   native_capacity = (int)capacity;
	 if(isInitialized()){
	 	error = SetHighCapacity(native_capacity);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetHighCapacity SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		      printf("SetHighCapacity Camera NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		      printf("SetHighCapacity Acquisition in progress = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		      printf("SetHighCapacity State parameter was not set to zero or 1 = %i\n",error);
	 	}
	 }
	setSDKError(error);
	return error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetNumberADChannels
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetNumberADChannels
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetNumberADChannels(JNIEnv *env, jclass obj){
	int   error = 0;
	int   channel = MISSING_INTEGER;
	 if(isInitialized()){
	 	error = GetNumberADChannels(&channel);
	 	if(error == DRV_SUCCESS){
	 	  printf("GetNumberADChannels SUCCESSFUL = %i\n",error);
	 	}
	 }
	setSDKError(error);
	return channel;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetADChannel
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetADChannel
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetADChannel(JNIEnv *env, jclass obj, jint channel){
	int   error = 0;
	int   native_channel = (int)channel;
	 if(isInitialized()){
	 	error = SetADChannel(native_channel);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetADChannel SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		      printf("SetADChannel Index is out of range = %i\n",error);
	 	}
	 }
	setSDKError(error);
	return error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetBitDepth
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetBitDepth
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetBitDepth(JNIEnv *env, jclass obj, jint channel){
int   error = 0;
int   native_channel = channel;
int   depth          = MISSING_INTEGER;
 if(isInitialized()){
 	error = GetBitDepth(channel,&depth);
 	if(error == DRV_SUCCESS){
 	  printf("GetBitDepth SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	      printf("GetBitDepth Camera NOT initialized = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	      printf("GetBitDepth Invalid channel = %i\n",error);
 	}
 }
setSDKError(error);
return (jint)depth;
}
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetMaximumExposure
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetMaximumExposure
 * Signature: ()F
 */
JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetMaximumExposure(JNIEnv *env, jclass obj){
int     error = 0;
float   max_exposure = 0.0;
 if(isInitialized()){
 	error = GetMaximumExposure(&max_exposure);
 	if(error == DRV_SUCCESS){
 	  printf("GetMaximumExposure SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	  printf("GetMaximumExposure Invalid channel = %i\n",error);
 	}
 }
 setSDKError(error);
return (jfloat)max_exposure;
}
/*=============================================================================================
/   Java_andor_AndorCamera2_1test_SetAcquisitionMode(JNIEnv *env, jclass obj, jint mode)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    SetAcquisitionMode
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetAcquisitionMode(JNIEnv *env, jclass obj, jint mode){
	int   error = 0;
	 if(isInitialized()){
	 	error = SetAcquisitionMode(mode);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetAcquisitionMode SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		      printf("SetAcquisitionMode Camera NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		      printf("SetAcquisitionMode Acquisition in progress = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		      printf("SetAcquisitionMode Acquisition Mode invalid = %i\n",error);
	 	}
	 }
	setSDKError(error);
	return error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetNumberAccumulations
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetNumberAccumulations
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetNumberAccumulations(JNIEnv *env, jclass obj, jint accumulations){
	int   native_accumulations = (int)accumulations;
	int   error = 0;
	 if(isInitialized()){
	 	error = SetNumberAccumulations(native_accumulations);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetNumberAccumulations SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		      printf("SetNumberAccumulations System NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		      printf("SetNumberAccumulations Acquisition in progress = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		      printf("SetNumberAccumulations Invalid number of accumulations = %i\n",error);
	 	}
	 }
	setSDKError(error);
	return error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetAccumulationCycleTime
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetAccumulationCycleTime
 * Signature: (F)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetAccumulationCycleTime(JNIEnv *env, jclass obj, jfloat accumulation_cycle_time){
	float   native_accumulation_cycle_time = (float)accumulation_cycle_time;
	int   error = 0;
	 if(isInitialized()){
	 	error = SetAccumulationCycleTime(native_accumulation_cycle_time);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetAccumulationCycleTime SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		      printf("SetAccumulationCycleTime System NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		      printf("SetAccumulationCycleTime Acquisition in progress = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		      printf("SetAccumulationCycleTime Exposure time invalid = %i\n",error);
	 	}
	 }
	setSDKError(error);
	return error;

}

/*=============================================================================================
/   Java_andor2_AndorCamera2_GetCapabilities
/=============================================================================================*/
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetCapabilities(JNIEnv *env, jclass obj, jobject capabilities){
	int error;
	jclass cls;
	AndorCapabilities      *andor_capabilities;
	andor_capabilities   = (AndorCapabilities *)malloc(sizeof(AndorCapabilities));
	current_capabilities = (Capabilities *)malloc(sizeof(Capabilities));
	andor_capabilities->ulSize = sizeof(AndorCapabilities);
	error = GetCapabilities(andor_capabilities);
	printf("%i",error);

	bool test = true;
	cls = (*env).GetObjectClass(capabilities);
	jfieldID fid_AcqModes_SINGLE                   = (*env).GetFieldID(cls,"AcqModes_SINGLE","Z");
	jfieldID fid_AcqModes_VIDEO                    = (*env).GetFieldID(cls,"AcqModes_VIDEO","Z");
	jfieldID fid_AcqModes_ACCUMULATE               = (*env).GetFieldID(cls,"AcqModes_ACCUMULATE","Z");
	jfieldID fid_AcqModes_KINETIC                  = (*env).GetFieldID(cls,"AcqModes_KINETIC","Z");
	jfieldID fid_AcqModes_FRAMETRANSFER            = (*env).GetFieldID(cls,"AcqModes_FRAMETRANSFER","Z");
	jfieldID fid_AcqModes_FASTKINETICS             = (*env).GetFieldID(cls,"AcqModes_FASTKINETICS","Z");
	jfieldID fid_AcqModes_OVERLAP                  = (*env).GetFieldID(cls,"AcqModes_OVERLAP","Z");

	jfieldID fid_EMGain_8BIT                        = (*env).GetFieldID(cls,"EMGain_8BIT","Z");
	jfieldID fid_EMGain_12BIT                       = (*env).GetFieldID(cls,"EMGain_12BIT","Z");
	jfieldID fid_EMGain_LINEAR12                    = (*env).GetFieldID(cls,"EMGain_LINEAR12","Z");
	jfieldID fid_EMGain_REAL12                      = (*env).GetFieldID(cls,"EMGain_REAL12","Z");

	jfieldID fid_ReadModes_FULLIMAGE                = (*env).GetFieldID(cls,"ReadModes_FULLIMAGE","Z");
	jfieldID fid_ReadModes_SUBIMAGE                 = (*env).GetFieldID(cls,"ReadModes_SUBIMAGE","Z");
	jfieldID fid_ReadModes_SINGLETRACK              = (*env).GetFieldID(cls,"ReadModes_SINGLETRACK","Z");
	jfieldID fid_ReadModes_FVB                      = (*env).GetFieldID(cls,"ReadModes_FVB","Z");
	jfieldID fid_ReadModes_MULTITRACK               = (*env).GetFieldID(cls,"ReadModes_MULTITRACK","Z");
	jfieldID fid_ReadModes_RANDOMTRACK              = (*env).GetFieldID(cls,"ReadModes_RANDOMTRACK","Z");

	jfieldID fid_FTReadModes_FULLIMAGE              = (*env).GetFieldID(cls,"FTReadModes_FULLIMAGE","Z");
	jfieldID fid_FTReadModes_SUBIMAGE               = (*env).GetFieldID(cls,"FTReadModes_SUBIMAGE","Z");
	jfieldID fid_FTReadModes_SINGLETRACK            = (*env).GetFieldID(cls,"FTReadModes_SINGLETRACK","Z");
	jfieldID fid_FTReadModes_FVB                    = (*env).GetFieldID(cls,"FTReadModes_FVB","Z");
	jfieldID fid_FTReadModes_MULTITRACK             = (*env).GetFieldID(cls,"FTReadModes_MULTITRACK","Z");
	jfieldID fid_FTReadModes_RANDOMTRACK            = (*env).GetFieldID(cls,"FTReadModes_RANDOMTRACK","Z");

	jfieldID fid_TriggerModes_INTERNAL              = (*env).GetFieldID(cls,"TriggerModes_INTERNAL","Z");
	jfieldID fid_TriggerModes_EXTERNAL              = (*env).GetFieldID(cls,"TriggerModes_EXTERNAL","Z");
	jfieldID fid_TriggerModes_EXTERNAL_FVB_EM       = (*env).GetFieldID(cls,"TriggerModes_EXTERNAL_FVB_EM","Z");
	jfieldID fid_TriggerModes_CONTINUOUS            = (*env).GetFieldID(cls,"TriggerModes_CONTINUOUS","Z");
	jfieldID fid_TriggerModes_EXTERNALSTART         = (*env).GetFieldID(cls,"TriggerModes_EXTERNALSTART","Z");
	jfieldID fid_TriggerModes_EXTERNALEXPOSURE      = (*env).GetFieldID(cls,"TriggerModes_EXTERNALEXPOSURE","Z");
	jfieldID fid_TriggerModes_INVERTED              = (*env).GetFieldID(cls,"TriggerModes_INVERTED","Z");
	jfieldID fid_TriggerModes_EXTERNALCHARGESHIFTED = (*env).GetFieldID(cls,"TriggerModes_EXTERNALCHARGESHIFTED","Z");

	jfieldID fid_PixelMode_8BIT                     = (*env).GetFieldID(cls,"PixelMode_8BIT","Z");
	jfieldID fid_PixelMode_14BIT                    = (*env).GetFieldID(cls,"PixelMode_14BIT","Z");
	jfieldID fid_PixelMode_16BIT                    = (*env).GetFieldID(cls,"PixelMode_16BIT","Z");
	jfieldID fid_PixelMode_32BIT                    = (*env).GetFieldID(cls,"PixelMode_32BIT","Z");

	jfieldID fid_Features_POLLING                   = (*env).GetFieldID(cls,"Features_POLLING","Z");
	jfieldID fid_Features_EVENTS                    = (*env).GetFieldID(cls,"Features_EVENTS","Z");
	jfieldID fid_Features_SPOOLING                  = (*env).GetFieldID(cls,"Features_SPOOLING","Z");
	jfieldID fid_Features_SHUTTER                   = (*env).GetFieldID(cls,"Features_SHUTTER","Z");
	jfieldID fid_Features_SHUTTEREX                 = (*env).GetFieldID(cls,"Features_SHUTTEREX","Z");
	jfieldID fid_Features_EXTERNAL_I2C              = (*env).GetFieldID(cls,"Features_EXTERNAL_I2C","Z");
	jfieldID fid_Features_SATURATIONEVENT           = (*env).GetFieldID(cls,"Features_SATURATIONEVENT","Z");
	jfieldID fid_Features_FANCONTROL                = (*env).GetFieldID(cls,"Features_FANCONTROL","Z");
	jfieldID fid_Features_MIDFANCONTROL             = (*env).GetFieldID(cls,"Features_MIDFANCONTROL","Z");
	jfieldID fid_Features_TEMPERATURE_DURING_ACQUISITION = (*env).GetFieldID(cls,"Features_TEMPERATURE_DURING_ACQUISITION","Z");
	jfieldID fid_Features_KEEP_CLEAN_CONTROL        = (*env).GetFieldID(cls,"Features_KEEP_CLEAN_CONTROL","Z");
	jfieldID fid_Features_DDGLITE                   = (*env).GetFieldID(cls,"Features_DDGLITE","Z");
	jfieldID fid_Features_FT_EXTERNAL_EXPOSURE      = (*env).GetFieldID(cls,"Features_FT_EXTERNAL_EXPOSURE","Z");
	jfieldID fid_Features_KINETIC_EXTERNAL_EXPOSURE = (*env).GetFieldID(cls,"Features_KINETIC_EXTERNAL_EXPOSURE","Z");
	jfieldID fid_Features_DAC_CONTROL               = (*env).GetFieldID(cls,"Features_DAC_CONTROL","Z");
	jfieldID fid_Features_METADATA                  = (*env).GetFieldID(cls,"Features_METADATA","Z");
	jfieldID fid_Features_IOCONTROL                 = (*env).GetFieldID(cls,"Features_IOCONTROL","Z");
	jfieldID fid_Features_PHOTONCOUNTING            = (*env).GetFieldID(cls,"Features_PHOTONCOUNTING","Z");
	jfieldID fid_Features_COUNTCONVERT              = (*env).GetFieldID(cls,"Features_COUNTCONVERT","Z");
	jfieldID fid_Features_DUALMODE                  = (*env).GetFieldID(cls,"Features_DUALMODE","Z");

	jfieldID fid_SetFunction_VREADOUT                 = (*env).GetFieldID(cls,"SetFunction_VREADOUT","Z");
	jfieldID fid_SetFunction_HREADOUT                 = (*env).GetFieldID(cls,"SetFunction_HREADOUT","Z");
	jfieldID fid_SetFunction_TEMPERATURE              = (*env).GetFieldID(cls,"SetFunction_TEMPERATURE","Z");
	jfieldID fid_SetFunction_MCPGAIN                  = (*env).GetFieldID(cls,"SetFunction_MCPGAIN","Z");
	jfieldID fid_SetFunction_EMCCDGAIN                = (*env).GetFieldID(cls,"SetFunction_EMCCDGAIN","Z");
	jfieldID fid_SetFunction_BASELINECLAMP            = (*env).GetFieldID(cls,"SetFunction_BASELINECLAMP","Z");
	jfieldID fid_SetFunction_VSAMPLITUDE              = (*env).GetFieldID(cls,"SetFunction_VSAMPLITUDE","Z");
	jfieldID fid_SetFunction_HIGHCAPACITY             = (*env).GetFieldID(cls,"SetFunction_HIGHCAPACITY","Z");
	jfieldID fid_SetFunction_BASELINEOFFSET           = (*env).GetFieldID(cls,"SetFunction_BASELINEOFFSET","Z");
	jfieldID fid_SetFunction_PREAMPGAIN               = (*env).GetFieldID(cls,"SetFunction_PREAMPGAIN","Z");
	jfieldID fid_SetFunction_CROPMODE                 = (*env).GetFieldID(cls,"SetFunction_CROPMODE","Z");
	jfieldID fid_SetFunction_DMAPARAMETERS            = (*env).GetFieldID(cls,"SetFunction_DMAPARAMETERS","Z");
	jfieldID fid_SetFunction_HORIZONTALBIN            = (*env).GetFieldID(cls,"SetFunction_HORIZONTALBIN","Z");
	jfieldID fid_SetFunction_MULTITRACKHRANGE         = (*env).GetFieldID(cls,"SetFunction_MULTITRACKHRANGE","Z");
	jfieldID fid_SetFunction_RANDOMTRACKNOGAPS        = (*env).GetFieldID(cls,"SetFunction_RANDOMTRACKNOGAPS","Z");
	jfieldID fid_SetFunction_EMADVANCED               = (*env).GetFieldID(cls,"SetFunction_EMADVANCED","Z");
	jfieldID fid_SetFunction_GATEMODE                 = (*env).GetFieldID(cls,"SetFunction_GATEMODE","Z");
	jfieldID fid_SetFunction_DDGTIMES                 = (*env).GetFieldID(cls,"SetFunction_DDGTIMES","Z");
	jfieldID fid_SetFunction_IOC                      = (*env).GetFieldID(cls,"SetFunction_IOC","Z");
	jfieldID fid_SetFunction_INTELLIGATE              = (*env).GetFieldID(cls,"SetFunction_INTELLIGATE","Z");
	jfieldID fid_SetFunction_INSERTION_DELAY          = (*env).GetFieldID(cls,"SetFunction_INSERTION_DELAY","Z");
	jfieldID fid_SetFunction_GATESTEP                 = (*env).GetFieldID(cls,"SetFunction_GATESTEP","Z");
	jfieldID fid_SetFunction_TRIGGERTERMINATION       = (*env).GetFieldID(cls,"SetFunction_TRIGGERTERMINATION","Z");
	jfieldID fid_SetFunction_EXTENDEDNIR              = (*env).GetFieldID(cls,"SetFunction_EXTENDEDNIR","Z");
	jfieldID fid_SetFunction_SPOOLTHREADCOUNT         = (*env).GetFieldID(cls,"SetFunction_SPOOLTHREADCOUNT","Z");

	jfieldID fid_GetFunction_TEMPERATURE              = (*env).GetFieldID(cls,"GetFunction_TEMPERATURE","Z");
	jfieldID fid_GetFunction_TEMPERATURERANGE         = (*env).GetFieldID(cls,"GetFunction_TEMPERATURERANGE","Z");
	jfieldID fid_GetFunction_DETECTORSIZE             = (*env).GetFieldID(cls,"GetFunction_DETECTORSIZE","Z");
	jfieldID fid_GetFunction_MCPGAIN                  = (*env).GetFieldID(cls,"GetFunction_MCPGAIN","Z");
	jfieldID fid_GetFunction_EMCCDGAIN                = (*env).GetFieldID(cls,"GetFunction_EMCCDGAIN","Z");
	jfieldID fid_GetFunction_GATEMODE                 = (*env).GetFieldID(cls,"GetFunction_GATEMODE","Z");
	jfieldID fid_GetFunction_DDGTIMES                 = (*env).GetFieldID(cls,"GetFunction_DDGTIMES","Z");
	jfieldID fid_GetFunction_IOC                      = (*env).GetFieldID(cls,"GetFunction_IOC","Z");
	jfieldID fid_GetFunction_INTELLIGATE              = (*env).GetFieldID(cls,"GetFunction_INTELLIGATE","Z");
	jfieldID fid_GetFunction_INSERTION_DELAY          = (*env).GetFieldID(cls,"GetFunction_INSERTION_DELAY","Z");
	jfieldID fid_GetFunction_PHOSPHORSTATUS           = (*env).GetFieldID(cls,"GetFunction_PHOSPHORSTATUS","Z");
	jfieldID fid_GetFunction_BASELINECLAMP            = (*env).GetFieldID(cls,"GetFunction_BASELINECLAMP","Z");

	jfieldID fid_CameraType                           = (*env).GetFieldID(cls,"CameraType","I");
//	jfieldID fid_PixelMode                            = (*env).GetFieldID(cls,"PixelMode","I");
	jfieldID fid_PCICard                              = (*env).GetFieldID(cls,"PCICard","I");

	if(error == DRV_SUCCESS){
	    int acquisition_modes    = andor_capabilities->ulAcqModes;
	      current_capabilities->AcqModes_SINGLE             = getBit(acquisition_modes,0);
	      current_capabilities->AcqModes_VIDEO              = getBit(acquisition_modes,1);
	      current_capabilities->AcqModes_ACCUMULATE         = getBit(acquisition_modes,2);
	      current_capabilities->AcqModes_KINETIC            = getBit(acquisition_modes,3);
	      current_capabilities->AcqModes_FRAMETRANSFER      = getBit(acquisition_modes,4);
	      current_capabilities->AcqModes_FASTKINETICS       = getBit(acquisition_modes,5);
	      current_capabilities->AcqModes_OVERLAP            = getBit(acquisition_modes,6);
	      current_capabilities->CameraType                  = andor_capabilities->ulCameraType;

	  	 (*env).SetBooleanField(capabilities,fid_AcqModes_SINGLE,        current_capabilities->AcqModes_SINGLE);
	  	 (*env).SetBooleanField(capabilities,fid_AcqModes_VIDEO,         current_capabilities->AcqModes_VIDEO);
	  	 (*env).SetBooleanField(capabilities,fid_AcqModes_ACCUMULATE,    current_capabilities->AcqModes_ACCUMULATE);
	  	 (*env).SetBooleanField(capabilities,fid_AcqModes_KINETIC,       current_capabilities->AcqModes_KINETIC);
	  	 (*env).SetBooleanField(capabilities,fid_AcqModes_FRAMETRANSFER, current_capabilities->AcqModes_FRAMETRANSFER);
	  	 (*env).SetBooleanField(capabilities,fid_AcqModes_FASTKINETICS,  current_capabilities->AcqModes_FASTKINETICS);
	  	 (*env).SetBooleanField(capabilities,fid_AcqModes_OVERLAP,       current_capabilities->AcqModes_OVERLAP);

	  	 (*env).SetIntField(capabilities,fid_CameraType,current_capabilities->CameraType);

	    int em_gain_capability   = andor_capabilities->ulEMGainCapability;
	      current_capabilities->EMGain_8BIT                 = getBit(em_gain_capability,0);
	      current_capabilities->EMGain_12BIT                = getBit(em_gain_capability,1);
	      current_capabilities->EMGain_LINEAR12             = getBit(em_gain_capability,2);
	      current_capabilities->EMGain_REAL12               = getBit(em_gain_capability,3);

		  (*env).SetBooleanField(capabilities,fid_EMGain_8BIT,           current_capabilities->EMGain_8BIT);
		  (*env).SetBooleanField(capabilities,fid_EMGain_12BIT,          current_capabilities->EMGain_12BIT);
		  (*env).SetBooleanField(capabilities,fid_EMGain_LINEAR12,       current_capabilities->EMGain_LINEAR12);
		  (*env).SetBooleanField(capabilities,fid_EMGain_REAL12,         current_capabilities->EMGain_REAL12);

	    int read_mode          = andor_capabilities->ulReadModes;
	      current_capabilities->ReadModes_FULLIMAGE          = getBit(read_mode,0);
	      current_capabilities->ReadModes_SUBIMAGE           = getBit(read_mode,1);
	      current_capabilities->ReadModes_SINGLETRACK        = getBit(read_mode,2);
	      current_capabilities->ReadModes_FVB                = getBit(read_mode,3);
	      current_capabilities->ReadModes_MULTITRACK         = getBit(read_mode,4);
	      current_capabilities->ReadModes_RANDOMTRACK        = getBit(read_mode,5);
		  (*env).SetBooleanField(capabilities,fid_ReadModes_FULLIMAGE,           current_capabilities->ReadModes_FULLIMAGE);
		  (*env).SetBooleanField(capabilities,fid_ReadModes_SUBIMAGE,            current_capabilities->ReadModes_SUBIMAGE);
		  (*env).SetBooleanField(capabilities,fid_ReadModes_SINGLETRACK,         current_capabilities->ReadModes_SINGLETRACK);
		  (*env).SetBooleanField(capabilities,fid_ReadModes_FVB,                 current_capabilities->ReadModes_FVB);
		  (*env).SetBooleanField(capabilities,fid_ReadModes_MULTITRACK,          current_capabilities->ReadModes_MULTITRACK);
		  (*env).SetBooleanField(capabilities,fid_ReadModes_RANDOMTRACK,         current_capabilities->ReadModes_RANDOMTRACK);

	    int ft_read_modes        = andor_capabilities->ulFTReadModes;
	      current_capabilities->FTReadModes_FULLIMAGE       = getBit(ft_read_modes,0);
	      current_capabilities->FTReadModes_SUBIMAGE        = getBit(ft_read_modes,1);
	      current_capabilities->FTReadModes_SINGLETRACK     = getBit(ft_read_modes,2);
	      current_capabilities->FTReadModes_FVB             = getBit(ft_read_modes,3);
	      current_capabilities->FTReadModes_MULTITRACK      = getBit(ft_read_modes,4);
	      current_capabilities->FTReadModes_RANDOMTRACK     = getBit(ft_read_modes,5);
		  (*env).SetBooleanField(capabilities,fid_FTReadModes_FULLIMAGE,           current_capabilities->FTReadModes_FULLIMAGE);
		  (*env).SetBooleanField(capabilities,fid_FTReadModes_SUBIMAGE,            current_capabilities->FTReadModes_SUBIMAGE);
		  (*env).SetBooleanField(capabilities,fid_FTReadModes_SINGLETRACK,         current_capabilities->FTReadModes_SINGLETRACK);
		  (*env).SetBooleanField(capabilities,fid_FTReadModes_FVB,                 current_capabilities->FTReadModes_FVB);
		  (*env).SetBooleanField(capabilities,fid_FTReadModes_MULTITRACK,          current_capabilities->FTReadModes_MULTITRACK);
		  (*env).SetBooleanField(capabilities,fid_FTReadModes_RANDOMTRACK,         current_capabilities->FTReadModes_RANDOMTRACK);

	    int trigger_modes        = andor_capabilities->ulTriggerModes;
	      current_capabilities->TriggerModes_INTERNAL           = getBit(trigger_modes,0);
	      current_capabilities->TriggerModes_EXTERNAL           = getBit(trigger_modes,1);
	      current_capabilities->TriggerModes_EXTERNAL_FVB_EM    = getBit(trigger_modes,2);
	      current_capabilities->TriggerModes_CONTINUOUS         = getBit(trigger_modes,3);
	      current_capabilities->TriggerModes_EXTERNALSTART      = getBit(trigger_modes,4);
	      current_capabilities->TriggerModes_EXTERNALEXPOSURE   = getBit(trigger_modes,5);
	      current_capabilities->TriggerModes_INVERTED           = getBit(trigger_modes,5);
	      current_capabilities->TriggerModes_EXTERNALCHARGESHIFTED = getBit(trigger_modes,7);
		  (*env).SetBooleanField(capabilities,fid_TriggerModes_INTERNAL,              current_capabilities->TriggerModes_INTERNAL);
		  (*env).SetBooleanField(capabilities,fid_TriggerModes_EXTERNAL,              current_capabilities->TriggerModes_EXTERNAL);
		  (*env).SetBooleanField(capabilities,fid_TriggerModes_EXTERNAL_FVB_EM,       current_capabilities->TriggerModes_EXTERNAL_FVB_EM);
		  (*env).SetBooleanField(capabilities,fid_TriggerModes_CONTINUOUS,            current_capabilities->TriggerModes_CONTINUOUS);
		  (*env).SetBooleanField(capabilities,fid_TriggerModes_EXTERNALSTART,         current_capabilities->TriggerModes_EXTERNALSTART);
		  (*env).SetBooleanField(capabilities,fid_TriggerModes_EXTERNALEXPOSURE,      current_capabilities->TriggerModes_EXTERNALEXPOSURE);
		  (*env).SetBooleanField(capabilities,fid_TriggerModes_INVERTED,              current_capabilities->TriggerModes_INVERTED);
		  (*env).SetBooleanField(capabilities,fid_TriggerModes_EXTERNALCHARGESHIFTED, current_capabilities->TriggerModes_EXTERNALCHARGESHIFTED);

	    int pixel_mode         = andor_capabilities->ulPixelMode;
	      current_capabilities->PixelMode_8BIT     = getBit(pixel_mode,0);
	      current_capabilities->PixelMode_14BIT    = getBit(pixel_mode,1);
	      current_capabilities->PixelMode_16BIT    = getBit(pixel_mode,2);
	      current_capabilities->PixelMode_32BIT    = getBit(pixel_mode,3);
		  (*env).SetBooleanField(capabilities,fid_PixelMode_8BIT,            current_capabilities->PixelMode_8BIT);
		  (*env).SetBooleanField(capabilities,fid_PixelMode_14BIT,           current_capabilities->PixelMode_14BIT);
		  (*env).SetBooleanField(capabilities,fid_PixelMode_16BIT,           current_capabilities->PixelMode_16BIT);
		  (*env).SetBooleanField(capabilities,fid_PixelMode_32BIT,           current_capabilities->PixelMode_32BIT);
	//      Bit PIXELMODE_MONO     = getBit(pixel_mode,0);
	//      Bit PIXELMODE_RGB      = getBit(pixel_mode,0);
	//      Bit PIXELMODE_CMY      = getBit(pixel_mode,0);
	    int features           = andor_capabilities->ulFeatures;
	      current_capabilities->Features_POLLING                      = getBit(features,0);
	      current_capabilities->Features_EVENTS                       = getBit(features,1);
	      current_capabilities->Features_SPOOLING                     = getBit(features,2);
	      current_capabilities->Features_SHUTTER                      = getBit(features,3);
	      current_capabilities->Features_SHUTTEREX                    = getBit(features,4);
	      current_capabilities->Features_EXTERNAL_I2C                 = getBit(features,5);
	      current_capabilities->Features_SATURATIONEVENT              = getBit(features,6);
	      current_capabilities->Features_FANCONTROL                   = getBit(features,7);
	      current_capabilities->Features_MIDFANCONTROL                = getBit(features,8);
	      current_capabilities->Features_TEMPERATURE_DURING_ACQUISITION = getBit(features,9);
	      current_capabilities->Features_KEEP_CLEAN_CONTROL           = getBit(features,10);
	      current_capabilities->Features_DDGLITE                      = getBit(features,11);
	      current_capabilities->Features_FT_EXTERNAL_EXPOSURE         = getBit(features,12);
	      current_capabilities->Features_KINETIC_EXTERNAL_EXPOSURE    = getBit(features,13);
	      current_capabilities->Features_DAC_CONTROL                  = getBit(features,14);
	      current_capabilities->Features_METADATA                     = getBit(features,15);
	      current_capabilities->Features_IOCONTROL                    = getBit(features,16);
	      current_capabilities->Features_PHOTONCOUNTING               = getBit(features,17);
	      current_capabilities->Features_COUNTCONVERT                 = getBit(features,18);
	      current_capabilities->Features_DUALMODE                     = getBit(features,19);
		  (*env).SetBooleanField(capabilities,fid_Features_POLLING,                   current_capabilities->Features_POLLING);
		  (*env).SetBooleanField(capabilities,fid_Features_EVENTS,                    current_capabilities->Features_EVENTS);
		  (*env).SetBooleanField(capabilities,fid_Features_SPOOLING,                  current_capabilities->Features_SPOOLING);
		  (*env).SetBooleanField(capabilities,fid_Features_SHUTTER,                   current_capabilities->Features_SHUTTER);
		  (*env).SetBooleanField(capabilities,fid_Features_SHUTTEREX,                 current_capabilities->Features_SHUTTEREX);
		  (*env).SetBooleanField(capabilities,fid_Features_EXTERNAL_I2C,              current_capabilities->Features_EXTERNAL_I2C);
		  (*env).SetBooleanField(capabilities,fid_Features_SATURATIONEVENT,           current_capabilities->Features_SATURATIONEVENT);
		  (*env).SetBooleanField(capabilities,fid_Features_FANCONTROL,                current_capabilities->Features_FANCONTROL);
		  (*env).SetBooleanField(capabilities,fid_Features_MIDFANCONTROL,             current_capabilities->Features_MIDFANCONTROL);
		  (*env).SetBooleanField(capabilities,fid_Features_TEMPERATURE_DURING_ACQUISITION, current_capabilities->Features_TEMPERATURE_DURING_ACQUISITION);
		  (*env).SetBooleanField(capabilities,fid_Features_KEEP_CLEAN_CONTROL,        current_capabilities->Features_KEEP_CLEAN_CONTROL);
		  (*env).SetBooleanField(capabilities,fid_Features_DDGLITE,                   current_capabilities->Features_DDGLITE);
		  (*env).SetBooleanField(capabilities,fid_Features_FT_EXTERNAL_EXPOSURE,      current_capabilities->Features_FT_EXTERNAL_EXPOSURE);
		  (*env).SetBooleanField(capabilities,fid_Features_KINETIC_EXTERNAL_EXPOSURE, current_capabilities->Features_KINETIC_EXTERNAL_EXPOSURE);
		  (*env).SetBooleanField(capabilities,fid_Features_DAC_CONTROL,               current_capabilities->Features_DAC_CONTROL);
		  (*env).SetBooleanField(capabilities,fid_Features_METADATA,                  current_capabilities->Features_METADATA);
		  (*env).SetBooleanField(capabilities,fid_Features_IOCONTROL,                 current_capabilities->Features_IOCONTROL);
		  (*env).SetBooleanField(capabilities,fid_Features_PHOTONCOUNTING,            current_capabilities->Features_PHOTONCOUNTING);
		  (*env).SetBooleanField(capabilities,fid_Features_COUNTCONVERT,              current_capabilities->Features_COUNTCONVERT);
		  (*env).SetBooleanField(capabilities,fid_Features_DUALMODE,                  current_capabilities->Features_DUALMODE);
	    int setFunctions       = andor_capabilities->ulSetFunctions;
	      current_capabilities->SetFunction_VREADOUT                  = getBit(setFunctions,0);
	      current_capabilities->SetFunction_HREADOUT                  = getBit(setFunctions,1);
	      current_capabilities->SetFunction_TEMPERATURE               = getBit(setFunctions,2);
	      current_capabilities->SetFunction_MCPGAIN                   = getBit(setFunctions,3);
	      current_capabilities->SetFunction_EMCCDGAIN                 = getBit(setFunctions,4);
	      current_capabilities->SetFunction_BASELINECLAMP             = getBit(setFunctions,5);
	      current_capabilities->SetFunction_VSAMPLITUDE               = getBit(setFunctions,6);
	      current_capabilities->SetFunction_HIGHCAPACITY              = getBit(setFunctions,7);
	      current_capabilities->SetFunction_BASELINEOFFSET            = getBit(setFunctions,8);
	      current_capabilities->SetFunction_PREAMPGAIN                = getBit(setFunctions,9);
	      current_capabilities->SetFunction_CROPMODE                  = getBit(setFunctions,10);
	      current_capabilities->SetFunction_DMAPARAMETERS             = getBit(setFunctions,11);
	      current_capabilities->SetFunction_HORIZONTALBIN             = getBit(setFunctions,12);
	      current_capabilities->SetFunction_MULTITRACKHRANGE          = getBit(setFunctions,13);
	      current_capabilities->SetFunction_RANDOMTRACKNOGAPS         = getBit(setFunctions,14);
	      current_capabilities->SetFunction_EMADVANCED                = getBit(setFunctions,15);
	      current_capabilities->SetFunction_GATEMODE                  = getBit(setFunctions,16);
	      current_capabilities->SetFunction_DDGTIMES                  = getBit(setFunctions,17);
	      current_capabilities->SetFunction_IOC                       = getBit(setFunctions,18);
	      current_capabilities->SetFunction_INTELLIGATE               = getBit(setFunctions,19);
	      current_capabilities->SetFunction_INSERTION_DELAY           = getBit(setFunctions,20);
	      current_capabilities->SetFunction_GATESTEP                  = getBit(setFunctions,21);
	      current_capabilities->SetFunction_TRIGGERTERMINATION        = getBit(setFunctions,22);
	      current_capabilities->SetFunction_EXTENDEDNIR               = getBit(setFunctions,23);
	      current_capabilities->SetFunction_SPOOLTHREADCOUNT          = getBit(setFunctions,24);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_VREADOUT,          current_capabilities->SetFunction_VREADOUT);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_HREADOUT,          current_capabilities->SetFunction_HREADOUT);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_TEMPERATURE,       current_capabilities->SetFunction_TEMPERATURE);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_MCPGAIN,           current_capabilities->SetFunction_MCPGAIN);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_EMCCDGAIN,         current_capabilities->SetFunction_EMCCDGAIN);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_BASELINECLAMP,     current_capabilities->SetFunction_BASELINECLAMP);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_VSAMPLITUDE,       current_capabilities->SetFunction_VSAMPLITUDE);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_HIGHCAPACITY,      current_capabilities->SetFunction_HIGHCAPACITY);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_BASELINEOFFSET,    current_capabilities->SetFunction_BASELINEOFFSET);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_PREAMPGAIN,        current_capabilities->SetFunction_PREAMPGAIN);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_CROPMODE,          current_capabilities->SetFunction_CROPMODE);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_DMAPARAMETERS,     current_capabilities->SetFunction_DMAPARAMETERS);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_HORIZONTALBIN,     current_capabilities->SetFunction_HORIZONTALBIN);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_MULTITRACKHRANGE,  current_capabilities->SetFunction_MULTITRACKHRANGE);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_RANDOMTRACKNOGAPS, current_capabilities->SetFunction_RANDOMTRACKNOGAPS);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_EMADVANCED,        current_capabilities->SetFunction_EMADVANCED);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_GATEMODE,          current_capabilities->SetFunction_GATEMODE);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_DDGTIMES,          current_capabilities->SetFunction_DDGTIMES);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_IOC,               current_capabilities->SetFunction_IOC);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_INTELLIGATE,       current_capabilities->SetFunction_INTELLIGATE);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_INSERTION_DELAY,   current_capabilities->SetFunction_INSERTION_DELAY);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_GATESTEP,          current_capabilities->SetFunction_GATESTEP);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_TRIGGERTERMINATION,current_capabilities->SetFunction_TRIGGERTERMINATION);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_EXTENDEDNIR,       current_capabilities->SetFunction_EXTENDEDNIR);
		  (*env).SetBooleanField(capabilities,fid_SetFunction_SPOOLTHREADCOUNT,  current_capabilities->SetFunction_SPOOLTHREADCOUNT);

	    int getFunctions       = andor_capabilities->ulGetFunctions;
	      current_capabilities->GetFunction_TEMPERATURE               = getBit(getFunctions,0);
	      current_capabilities->GetFunction_TEMPERATURERANGE          = getBit(getFunctions,2);
	      current_capabilities->GetFunction_DETECTORSIZE              = getBit(getFunctions,3);
	      current_capabilities->GetFunction_MCPGAIN                   = getBit(getFunctions,4);
	      current_capabilities->GetFunction_EMCCDGAIN                 = getBit(getFunctions,5);
	      current_capabilities->GetFunction_GATEMODE                  = getBit(getFunctions,7);
	      current_capabilities->GetFunction_DDGTIMES                  = getBit(getFunctions,8);
	      current_capabilities->GetFunction_IOC                       = getBit(getFunctions,9);
	      current_capabilities->GetFunction_INTELLIGATE               = getBit(getFunctions,10);
	      current_capabilities->GetFunction_INSERTION_DELAY           = getBit(getFunctions,11);
	      current_capabilities->GetFunction_PHOSPHORSTATUS            = getBit(getFunctions,13);
	      current_capabilities->GetFunction_BASELINECLAMP             = getBit(getFunctions,15);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_TEMPERATURE,      current_capabilities->GetFunction_TEMPERATURE);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_TEMPERATURERANGE, current_capabilities->GetFunction_TEMPERATURERANGE);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_DETECTORSIZE,     current_capabilities->GetFunction_DETECTORSIZE);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_MCPGAIN,          current_capabilities->GetFunction_MCPGAIN);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_EMCCDGAIN,        current_capabilities->GetFunction_EMCCDGAIN);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_GATEMODE,         current_capabilities->GetFunction_GATEMODE);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_DDGTIMES,         current_capabilities->GetFunction_DDGTIMES);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_IOC,              current_capabilities->GetFunction_IOC);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_INTELLIGATE,      current_capabilities->GetFunction_INTELLIGATE);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_INSERTION_DELAY,  current_capabilities->GetFunction_INSERTION_DELAY);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_PHOSPHORSTATUS,   current_capabilities->GetFunction_PHOSPHORSTATUS);
		  (*env).SetBooleanField(capabilities,fid_GetFunction_BASELINECLAMP,    current_capabilities->GetFunction_BASELINECLAMP);

	      current_capabilities->PCICard                                 = andor_capabilities->ulPCICard;
		  (*env).SetIntField(capabilities,fid_PCICard,current_capabilities->PCICard);
	    int ulsize             = andor_capabilities->ulSize;
	 }
	 free(andor_capabilities);
	 free(current_capabilities);
	// jclass cls = (*env).FindClass("andor.AndorCapabilities");
	// jfieldID fidNumber = (*env).GetFieldID(cls,"AcqModes_SINGLE","B");
  return error;
}
 /*=============================================================================================
 /  Java_andor2_AndorCamera2_GetControllerCardModel
 /=============================================================================================*/
  JNIEXPORT jstring JNICALL Java_andor2_AndorCamera2_GetControllerCardModel (JNIEnv *env, jclass obj){
     int   length = 10;
	 char  controller_card_model[length];
	 int   error = 0;
		    if(isInitialized()){
		    	error = GetControllerCardModel(controller_card_model);
		    	if(error == DRV_SUCCESS){
		    	  printf("Controller Card Model = %s\n",controller_card_model);
		    	}
		    }
	 setSDKError(error);
	 jchar *java_string = new jchar[length];
	  for(int i=0;i<length;i++){
	     java_string[i] = controller_card_model[i];
	  }
//	  delete controller_card_model;
	  return env->NewString(java_string,length);
  }
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetControllerCardModel
/=============================================================================================*/
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetCameraSerialNumber(JNIEnv *env, jclass obj){
	   int cameraSerialNumber = MISSING_INTEGER;
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetCameraSerialNumber(&cameraSerialNumber);
		    	if(error == DRV_SUCCESS){
		    	  printf("Camera Serial Number = %i\n",cameraSerialNumber);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	  cameraSerialNumber = error;

		    	}
	     setSDKError(error);
	     }
	return (jint)cameraSerialNumber;
 }
  /*=============================================================================================
  /  Java_andor2_AndorCamera2_GetDetector
  /=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetDetector
   * Signature: ()[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetDetector(JNIEnv *env, jclass obj){
	int width, height;
	int dimensions[2];
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetDetector(&width,&height);
		    	if(error == DRV_SUCCESS){
		    	  printf("Detector Size = %i %i\n",width,height);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	}
	     setSDKError(error);
	     }
	     dimensions[0] = width;
	     dimensions[1] = height;
	 	 jintArray j_array = env->NewIntArray(2);
	 	 env->SetIntArrayRegion(j_array,0,2,dimensions);
	return j_array;
  }
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetSoftwareVersion
/=============================================================================================*/
 /*
   * Class:     andor2_AndorCamera2
   * Method:    GetSoftwareVersion
   * Signature: ()[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetSoftwareVersion(JNIEnv *env, jclass obj){
		unsigned int eprom;
		unsigned int cofFile;
		unsigned int vxdRev;
		unsigned int vxdVer;
		unsigned int dllRev;
		unsigned int dllVer;
		int report[6];
		   int error = 0;
		     if(isInitialized()){
		    	 error = GetSoftwareVersion(&eprom,&cofFile,&vxdRev,&vxdVer,&dllRev,&dllVer);
			    	if(error == DRV_SUCCESS){
			    	  printf("Software Version\n eprom = %i \n cofFile = %i \n vxd = %i.%i \n dll = %i.%i\n",eprom,cofFile,vxdVer,vxdRev,dllVer,dllRev);
			    	}
			    	if(error != DRV_SUCCESS){
			    		report[0] = error;
			    		printf("Error returned from method in index zero of array\n");
			    	}
			    	if(error == DRV_NOT_INITIALIZED){
			    	  printf("Driver Not Initialized = %i\n",error);
			    	}
			    	if(error == DRV_ACQUIRING){
			    	  printf("Driver Acquiring = %i\n",error);
			    	}
			    	if(error == DRV_ERROR_ACK){
			    	  printf("Unable to communicate with card = %i\n",error);
			    	}

		     setSDKError(error);
		     }
		     report[0] = eprom;
		     report[1] = cofFile;
		     report[2] = vxdRev;
		     report[3] = vxdVer;
		     report[4] = dllRev;
		     report[5] = dllVer;
		 	 jintArray j_array = env->NewIntArray(6);
		 	 env->SetIntArrayRegion(j_array,0,6,report);
    return j_array;
  }
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetHardwareVersion
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetHardwareVersion
   * Signature: ()[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetHardwareVersion(JNIEnv *env, jclass obj){
		unsigned int PCB;
		unsigned int Decode;
		unsigned int dummy1;
		unsigned int dummy2;
		unsigned int CameraFirmwareVersion;
		unsigned int CameraFirewareBuild;
		int report[6];
		   int error = 0;
		     if(isInitialized()){
		    	 error = GetHardwareVersion(&PCB,&Decode,&dummy1,&dummy2,&CameraFirmwareVersion,&CameraFirewareBuild);
			    	if(error == DRV_SUCCESS){
			    	  printf("Hardware Version\n PCB = %i \n Decode = %i \n dummy = %i.%i \n Camera Firmware = %i.%i\n",PCB,Decode,dummy1,dummy2,CameraFirmwareVersion,CameraFirewareBuild);
			    	}
			    	if(error != DRV_SUCCESS){
			    		report[0] = error;
			    		printf("Error returned from method in index zero of array\n");
			    	}
			    	if(error == DRV_NOT_INITIALIZED){
			    	  printf("Driver Not Initialized = %i\n",error);
			    	}
			    	if(error == DRV_ACQUIRING){
			    	  printf("Driver Acquiring = %i\n",error);
			    	}
			    	if(error == DRV_ERROR_ACK){
			    	  printf("Unable to communicate with card = %i\n",error);
			    	}

		     setSDKError(error);
		     }
		     report[0] = PCB;
		     report[1] = Decode;
		     report[2] = dummy1;
		     report[3] = dummy2;
		     report[4] = CameraFirmwareVersion;
		     report[5] = CameraFirewareBuild;
		 	 jintArray j_array = env->NewIntArray(6);
		 	 env->SetIntArrayRegion(j_array,0,6,report);
  return j_array;
  }
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetHeadModel
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetHeadModel
   * Signature: ()Ljava/lang/String;
   */
  JNIEXPORT jstring JNICALL Java_andor2_AndorCamera2_GetHeadModel(JNIEnv *env, jclass obj){
     int   MAX_PATH = 128;
	 char  head_model[MAX_PATH];
     int   error = 0;
			    if(isInitialized()){
			    	error = GetHeadModel(head_model);
			    	if(error == DRV_SUCCESS){
			    	  printf("Head Model = %s\n",head_model);
			    	}
			    }
		 setSDKError(error);
		 jchar *java_string = new jchar[MAX_PATH];
		  for(int i=0;i<MAX_PATH;i++){
		     java_string[i] = head_model[i];
		  }
    return env->NewString(java_string,MAX_PATH);
  }
/*=============================================================================================
/  Java_andor2_AndorCamera2_SetMetaData
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    SetMetaData
   * Signature: (I)I
   */
 JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetMetaData(JNIEnv *env, jclass obj, jint state){
	 int native_state = 0;
	 int error        = 0;
	 native_state = (int)state;
	     if(isInitialized()){
	    	 error = SetMetaData(native_state);
		    	if(error == DRV_SUCCESS){
		    	  printf("Set Metadata Succesfull!\n");
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	}
		    	if(error == DRV_ACQUIRING){
		    	  printf("Acquisition in progress = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("Invalid state = %i\n",error);
		    	}
		    	if(error == DRV_NOT_AVAILABLE){
		    	  printf("Feature not available = %i\n",error);
		    	}
	     setSDKError(error);
	     }
    return (jint)error;
 }
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetPixelSize
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetPixelSize
   * Signature: ()[F
   */
  JNIEXPORT jfloatArray JNICALL Java_andor2_AndorCamera2_GetPixelSize(JNIEnv *env, jclass obj){
	float xSize, ySize;
	float *dimensions;
	dimensions = new float[2];
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetPixelSize(&xSize,&ySize);
		    	if(error == DRV_SUCCESS){
		    	  printf("Pixel Size = %i %i\n",xSize,ySize);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	}
	     setSDKError(error);
	     }
	     dimensions[0] = xSize;
	     dimensions[1] = ySize;
	 	 jfloatArray j_array = env->NewFloatArray(2);
	 	 env->SetFloatArrayRegion(j_array,0,2,dimensions);
	 	 free(dimensions);
	return j_array;
  }
/*=============================================================================================
/     Java_andor2_AndorCamera2_GetVersionInfo_1sdk
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetVersionInfo
   * Signature: ()Ljava/lang/String;
   */
  JNIEXPORT jstring JNICALL Java_andor2_AndorCamera2_GetVersionInfo_1sdk(JNIEnv *env, jclass obj){
		int  buffer_length = 128;
		char version_info[buffer_length];
		   int   error = 0;
		    if(isInitialized()){
		    	error = GetVersionInfo(AT_SDKVersion,version_info,buffer_length);
		    	if(error == DRV_SUCCESS){
		    	  printf("SDK Version Number = %s\n",version_info);
		    	}
		    }
	    setSDKError(error);
		jchar *java_string = new jchar[buffer_length];
		  for(int i=0;i<buffer_length;i++){
		     java_string[i] = version_info[i];
		  }
   return env->NewString(java_string,buffer_length);
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetVersionInfo_1device_1driver
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetVersionInfo_device_driver
   * Signature: ()Ljava/lang/String;
   */
  JNIEXPORT jstring JNICALL Java_andor2_AndorCamera2_GetVersionInfo_1device_1driver(JNIEnv *env, jclass obj){
		int  buffer_length = 128;
		char version_info[buffer_length];
		   int   error = 0;
		    if(isInitialized()){
		    	error = GetVersionInfo(AT_DeviceDriverVersion,version_info,buffer_length);
		    	if(error == DRV_SUCCESS){
		    	  printf("Device Driver Version Number = %s\n",version_info);
		    	}
		    }
	    setSDKError(error);
		jchar *java_string = new jchar[buffer_length];
		  for(int i=0;i<buffer_length;i++){
		     java_string[i] = version_info[i];
		  }
   return env->NewString(java_string,buffer_length);
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetSizeOfCircularBuffer
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetSizeOfCircularBuffer
   * Signature: ()I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetSizeOfCircularBuffer(JNIEnv *env, jclass obj){
	   int size_of_circular_buffer = MISSING_INTEGER;
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetSizeOfCircularBuffer(&size_of_circular_buffer);
		    	if(error == DRV_SUCCESS){
		    	  printf("Size of Circular Buffer = %i\n",size_of_circular_buffer);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	  size_of_circular_buffer = error;
		    	}
	     setSDKError(error);
	     }
	return (jint)size_of_circular_buffer;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetMinimumImageLength
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetMinimumImageLength
   * Signature: ()I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetMinimumImageLength(JNIEnv *env, jclass obj){
	   int minimum_image_size = MISSING_INTEGER;
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetMinimumImageLength(&minimum_image_size);
		    	if(error == DRV_SUCCESS){
		    	  printf("Minimum Image Length = %i\n",minimum_image_size);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	  minimum_image_size = error;
		    	}
	     setSDKError(error);
	     }
	return (jint)minimum_image_size;
  } 
/*=============================================================================================
/   Java_andor2_AndorCamera2_IsAmplifierAvailable
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    IsAmplifierAvailable
   * Signature: (I)I
   */
  JNIEXPORT jboolean JNICALL Java_andor2_AndorCamera2_IsAmplifierAvailable(JNIEnv *env, jclass obj, jint amplifier){
	  int  native_amplifier = (int)amplifier;
	  bool state = false;
	  int   error = 0;
	      if(isInitialized()){
	   	   error = IsAmplifierAvailable(native_amplifier);
	   	   if(error == DRV_SUCCESS){
	   	     printf("IsAmplifierAvailable = %i\n",native_amplifier);
	   	     if(native_amplifier == ON){
	   	    	state = true;
	   	     }
	   	     if(native_amplifier == OFF){
	   	    	state = false;
	   	     }
	   	   }
	   	   if(error == DRV_NOT_INITIALIZED){
	   	      printf("IsAmplifierAvailable = %i\n",error);
	   	   }
	       if(error == DRV_INVALID_AMPLIFIER){
	    	  printf("IsAmplifierAvailable  Not a valid amplifier= %i\n",error);
	       }
	      }
	    setSDKError(error);
	    return (jboolean)state;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetNumberAmp
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetNumberAmp
   * Signature: ()I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetNumberAmp(JNIEnv *env, jclass obj){
	   int number_amplifiers = MISSING_INTEGER;
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetNumberAmp(&number_amplifiers);
		    	if(error == DRV_SUCCESS){
		    	  printf("GetNumberAmp number of amplifiers SUCCESS = %i\n",number_amplifiers);
		    	}
	     setSDKError(error);
	     }
	return (jint)number_amplifiers;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetAmpDesc
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetAmpDesc
   * Signature: (I)Ljava/lang/String;
   */
  JNIEXPORT jstring JNICALL Java_andor2_AndorCamera2_GetAmpDesc(JNIEnv *env, jclass obj, jint amplifier_index){
	    char *amplifier_description;
	    int  buffer_length = 21;
		int  native_amplifier_index = (int)amplifier_index;
		amplifier_description = new char[buffer_length];
		int   error = 0;
		    if(isInitialized()){
		    	error = GetAmpDesc(native_amplifier_index,amplifier_description,buffer_length);
		    	if(error == DRV_SUCCESS){
		    	  printf("GetAmpDesc = %s\n",amplifier_description);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("GetAmpDesc System not initialized = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("GetAmpDesc The amplifier index is not valid= %i\n",error);
		    	}
		    	if(error == DRV_P2INVALID){
		    	  printf("GetAmpDesc The desc pointer is null = %i\n",error);
		    	}
		    	if(error == DRV_P2INVALID){
		    	  printf("GetAmpDesc The len parameter is invalid= %i\n",error);
		    	}
		    }
	    setSDKError(error);
		jchar *java_string = new jchar[buffer_length];
		  for(int i=0;i<buffer_length;i++){
		     java_string[i] = amplifier_description[i];
		  }
	    free(amplifier_description);
 return env->NewString(java_string,buffer_length);
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetAmpMaxSpeed
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetAmpMaxSpeed
   * Signature: (I)F
   */
  JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetAmpMaxSpeed(JNIEnv *env, jclass obj, jint index){
	   int   native_index = (int)index;
	   float max_speed    = 0.0;
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetAmpMaxSpeed(native_index,&max_speed);
		    	if(error == DRV_SUCCESS){
		    	  printf("GetAmpMaxSpeed SUCCESS = %f\n",max_speed);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("GetAmpMaxSpeed System not initialized = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("GetAmpMaxSpeed The amplifier index is not valid= %i\n",error);
		    	}
	     setSDKError(error);
	     }
	return (jfloat)max_speed;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetNumberPreAmpGains
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetNumberPreAmpGains
   * Signature: ()I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetNumberPreAmpGains(JNIEnv *env, jclass obj){
	   int number_preamp_gains = MISSING_INTEGER;
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetNumberPreAmpGains(&number_preamp_gains);
		    	if(error == DRV_SUCCESS){
		    	  printf("GetNumberPreAmpGains SUCCESS = %i\n",number_preamp_gains);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("GetNumberPreAmpGains System not initialized = %i\n",error);
		    	}
		    	if(error == DRV_ACQUIRING){
		    	  printf("GetNumberPreAmpGains Acquisition in progre3ss = %i\n",error);
		    	}
	     setSDKError(error);
	     }
	return (jint)number_preamp_gains;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_IsPreAmpGainAvailable
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    IsPreAmpGainAvailable
   * Signature: (IIII)Z
   */
  JNIEXPORT jboolean JNICALL Java_andor2_AndorCamera2_IsPreAmpGainAvailable(JNIEnv *env, jclass obj, jint channel_index, jint amplifier, jint channel_speed, jint preamp_gain_index){
	  int  native_channel_index     = (int)channel_index;
	  int  native_amplifier         = (int)amplifier;
	  int  native_channel_speed     = (int)channel_speed;
	  int  native_preamp_gain_index = (int)preamp_gain_index;
	  bool state = false;
	  int  status                   = MISSING_INTEGER;
	  int   error = 0;
	      if(isInitialized()){
	   	   error = IsPreAmpGainAvailable(native_channel_index,native_amplifier,native_channel_speed,native_preamp_gain_index,&status);
	   	   if(error == DRV_SUCCESS){
	   	     printf("IsPreAmpGainAvailable = %i\n",status);
	   	     if(status == ON){
	   	    	state = true;
	   	     }
	   	     if(status == OFF){
	   	    	state = false;
	   	     }
	   	   }
	   	   if(error == DRV_NOT_INITIALIZED){
	   	      printf("IsPreAmpGainAvailable = %i\n",error);
	   	   }
	       if(error == DRV_ACQUIRING){
	    	  printf("IsPreAmpGainAvailable  Acquisition in progress= %i\n",error);
	       }
	       if(error == DRV_P1INVALID){
	    	  printf("IsPreAmpGainAvailable Invalid channel %i\n",error);
	       }
	       if(error == DRV_P2INVALID){
	    	  printf("IsPreAmpGainAvailable Invalid amplifier = %i\n",error);
	       }
	       if(error == DRV_P3INVALID){
	    	  printf("IsPreAmpGainAvailable Invalid speed index = %i\n",error);
	       }
	       if(error == DRV_P4INVALID){
	    	  printf("IsPreAmpGainAvailable Invalid gain = %i\n",error);
	       }
	      }
	    setSDKError(error);
	 return (jboolean)state;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetPreAmpGain
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    SetPreAmpGain
   * Signature: (I)I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetPreAmpGain(JNIEnv *env, jclass obj, jint index){
		 int native_index = (int)index;
		 int error        = 0;
		     if(isInitialized()){
		    	 error = SetPreAmpGain(native_index);
			    	if(error == DRV_SUCCESS){
			    	  printf("SetPreAmpGain SUCCESS!\n");
			    	}
			    	if(error == DRV_NOT_INITIALIZED){
			    	  printf("SetPreAmpGain Driver Not Initialized = %i\n",error);
			    	}
			    	if(error == DRV_ACQUIRING){
			    	  printf("SetPreAmpGain Acquisition in progress = %i\n",error);
			    	}
			    	if(error == DRV_P1INVALID){
			    	  printf("SetPreAmpGain Invalid index = %i\n",error);
			    	}
		     setSDKError(error);
		     }
	    return (jint)error;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetPreAmpGain
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetPreAmpGain
   * Signature: (I)F
   */
  JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetPreAmpGain(JNIEnv *env, jclass obj, jint index){
  int    native_index = (int)index;
  float  gain         = 0.0;
  int error = 0;
    if(isInitialized()){
   	 error = GetPreAmpGain(native_index,&gain);
	    	if(error == DRV_SUCCESS){
	    	  printf("GetPreAmpGain SUCCESS = %f\n",gain);
	    	}
	    	if(error == DRV_NOT_INITIALIZED){
	    	  printf("GetPreAmpGain System not initialized = %i\n",error);
	    	}
	    	if(error == DRV_ACQUIRING){
	    	  printf("GetPreAmpGain Acquisition in progre3ss = %i\n",error);
	    	}
	    	if(error == DRV_P1INVALID){
	    	  printf("GetPreAmpGain Invalid index = %i\n",error);
	    	}
    setSDKError(error);
    }
   return (jfloat)gain;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_IsInternalMechanicalShutter
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetPreAmpGainText
   * Signature: (I)Ljava/lang/String;
   */
  JNIEXPORT jstring JNICALL Java_andor2_AndorCamera2_GetPreAmpGainText(JNIEnv *env, jclass obj, jint index){
	     int   length = 30;
	     int   native_index = (int)index;
	     char  *preamp_gain_text;
		        preamp_gain_text = new char[length];
		  int   error = 0;
			    if(isInitialized()){
			    	error = GetPreAmpGainText(native_index,preamp_gain_text,length);
			    	if(error == DRV_SUCCESS){
			    	  printf("Controller Card Model = %s\n",preamp_gain_text);
			    	}
			    	if(error == DRV_NOT_INITIALIZED){
			    	  printf("GetPreAmpGainText System not initialized = %i\n",error);
			    	}
			    	if(error == DRV_P1INVALID){
			    	  printf("GetPreAmpGainText Invalid index = %i\n",error);
			    	}
			    	if(error == DRV_P2INVALID){
			    	  printf("GetPreAmpGainText Array size incorrect = %i\n",error);
			    	}
			    	if(error == DRV_NOT_SUPPORTED){
			    	  printf("GetPreAmpGainText Function not supported with this camera = %i\n",error);
			    	}
			    }
		 setSDKError(error);
		 jchar *java_string = new jchar[length];
		  for(int i=0;i<length;i++){
		     java_string[i] = preamp_gain_text[i];
		  }
	  //	  delete controller_card_model;
      free(preamp_gain_text);
   return env->NewString(java_string,length);
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_IsInternalMechanicalShutter
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    IsInternalMechanicalShutter
   * Signature: ()Z
   */
  JNIEXPORT jboolean JNICALL Java_andor2_AndorCamera2_IsInternalMechanicalShutter(JNIEnv *env, jclass obj){
	  bool state          = false;
	  int  internalShutter = MISSING_INTEGER;
	  int  error = 0;
	     if(isInitialized()){
	    	 error = IsInternalMechanicalShutter(&internalShutter);
		    	if(error == DRV_SUCCESS){
		    	  printf("isInternalMechanicalShutter = %i\n",error);
		    	  if(internalShutter == 0){
		    		  state = false;
		    	  }
		    	  if(internalShutter == 1){
		    		  state = true;
		    	  }
		    	}
		    	if(error == DRV_NOT_AVAILABLE){
		    	  printf("Not an iXon camera = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("Parameter is NULL = %i\n",error);
		    	}
	     setSDKError(error);
	  }
	return (jboolean)state;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetShutter
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    SetShutter
   * Signature: (IIII)I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetShutter(JNIEnv *env, jclass obj, jint type, jint mode, jint closingTime, jint openingTime){
	  int  native_type        = (int)type;
	  int  native_mode        = (int)mode;
	  int  native_closingTime = (int)closingTime;
	  int  native_openingTime = (int)openingTime;
	  int  error = 0;
	     if(isInitialized()){
	    	 error = SetShutter(native_type,native_mode,native_closingTime,native_openingTime);
		    	if(error == DRV_SUCCESS){
		    	  printf("SetShutter = %i\n",error);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("SetShutter System not initialized = %i\n",error);
		    	}
		    	if(error == DRV_ACQUIRING){
		    	  printf("Acquisition in progress = %i\n",error);
		    	}
		    	if(error == DRV_ERROR_ACK){
		    	  printf("Unable to communicate with card = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("SetShutter Invalid TTL type = %i\n",error);
		    	}
		    	if(error == DRV_P2INVALID){
		    	  printf("SetShutter Invalid mode = %i\n",error);
		    	}
		    	if(error == DRV_P3INVALID){
		    	  printf("SetShutter Invalid time to open = %i\n",error);
		    	}
		    	if(error == DRV_P4INVALID){
		    	  printf("SetShutter Invalid time to close = %i\n",error);
		    	}
	     setSDKError(error);
	  }
	return (jint)error;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetShutterEx
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    SetShutterEx
   * Signature: (IIIII)I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetShutterEx(JNIEnv *env, jclass obj, jint type, jint mode, jint closingTime, jint openingTime, jint extmode){
  int  native_type        = (int)type;
  int  native_mode        = (int)mode;
  int  native_closingTime = (int)closingTime;
  int  native_openingTime = (int)openingTime;
  int  native_extmode     = (int)extmode;
  int  error = 0;
     if(isInitialized()){
    	 error = SetShutterEx(native_type,native_mode,native_closingTime,native_openingTime,extmode);
	    	if(error == DRV_SUCCESS){
	    	  printf("SetShutter = %i\n",error);
	    	}
	    	if(error == DRV_NOT_INITIALIZED){
	    	  printf("SetShutter System not initialized = %i\n",error);
	    	}
	    	if(error == DRV_ACQUIRING){
	    	  printf("Acquisition in progress = %i\n",error);
	    	}
	    	if(error == DRV_ERROR_ACK){
	    	  printf("Unable to communicate with card = %i\n",error);
	    	}
	    	if(error == DRV_P1INVALID){
	    	  printf("SetShutter Invalid TTL type = %i\n",error);
	    	}
	    	if(error == DRV_P2INVALID){
	    	  printf("SetShutter Invalid mode = %i\n",error);
	    	}
	    	if(error == DRV_P3INVALID){
	    	  printf("SetShutter Invalid time to open = %i\n",error);
	    	}
	    	if(error == DRV_P4INVALID){
	    	  printf("SetShutter Invalid time to close = %i\n",error);
	    	}
	    	if(error == DRV_P5INVALID){
	    	  printf("SetShutter Invalid external mode = %i\n",error);
	    	}
     setSDKError(error);
  }
  return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetTotalNumberImagesAcquired
/=============================================================================================*/
 /*
   * Class:     andor2_AndorCamera2
   * Method:    GetTotalNumberImagesAcquired
   * Signature: ()I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetTotalNumberImagesAcquired(JNIEnv *env, jclass obj){
	   int total_num_images_acquired = MISSING_INTEGER;
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetTotalNumberImagesAcquired(&total_num_images_acquired);
		    	if(error == DRV_SUCCESS){
		    	  printf("Total number of images acquired = %i\n",total_num_images_acquired);
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	  total_num_images_acquired = error;
		    	}
	     setSDKError(error);
	     }
	return (jint)total_num_images_acquired;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetTotalNumberImagesAcquired
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetNumberAvailableImages
   * Signature: ()[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetNumberAvailableImages(JNIEnv *env, jclass obj){
	   int first_available_image = MISSING_INTEGER;
	   int last_available_image  = MISSING_INTEGER;
	   int report[2];
	   int error = 0;
	     if(isInitialized()){
	    	 error = GetNumberAvailableImages(&first_available_image,&last_available_image);
		    	if(error == DRV_SUCCESS){
		    	  printf("Number of Available Images FIRST = %i LAST = %i\n",first_available_image,last_available_image);
		 	      report[0] = first_available_image;
		 	      report[1] = last_available_image;
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	  report[0] = error;
		    	}
	     setSDKError(error);
	     }
	 	 jintArray j_array = env->NewIntArray(2);
	 	 env->SetIntArrayRegion(j_array,0,2,report);
    return j_array;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetNumberNewImages
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetNumberNewImages
   * Signature: ()[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetNumberNewImages(JNIEnv *env, jclass obj){
  int first_available_image = MISSING_INTEGER;
  int last_available_image  = MISSING_INTEGER;
  int report[2];
  int error = 0;
    if(isInitialized()){
   	 error = GetNumberNewImages(&first_available_image,&last_available_image);
	    	if(error == DRV_SUCCESS){
	    	  printf("Number of Available Images FIRST = %i LAST = %i\n",first_available_image,last_available_image);
	 	      report[0] = first_available_image;
	 	      report[1] = last_available_image;
	    	}
	    	if(error == DRV_NOT_INITIALIZED){
	    	  printf("Driver Not Initialized = %i\n",error);
	    	  report[0] = error;
	 	  report[1] = error;
	    	}
	    	if(error == DRV_ERROR_ACK){
	    	  printf("Unable to communicate with card = %i\n",error);
	    	  report[0] = error;
	 	  report[1] = error;
	    	}
	    	if(error == DRV_NO_NEW_DATA){
	    	  printf("There is no new data yet = %i\n",error);
	    	  report[0] = first_available_image;  // modified 3-27-2019 replaced error with first_available image
	 	  report[1] = last_available_image;   // modified 3-27-2019 replaced error with first_available image
	    	}
     setSDKError(error);
     }
	 jintArray j_array = env->NewIntArray(2);
	 env->SetIntArrayRegion(j_array,0,2,report);
  return j_array;
 }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetAcquiredData
/=============================================================================================*/
 /*
   * Class:     andor2_AndorCamera2
   * Method:    GetAcquiredData
   * Signature: ()[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetAcquiredData(JNIEnv *env, jclass obj, jint pixels){
	  jintArray  jimage_array;
	  at_32*     image_array;
	  unsigned long  current_pixels = (unsigned long)pixels;
	                 image_array    = new at_32[current_pixels];
	  int            error          = 0;
	  printf("PIXELS = %i\n",current_pixels);
	  if(isInitialized()){
	   	 error = GetAcquiredData(image_array,current_pixels);
		    	if(error == DRV_SUCCESS){
		    	  printf("Data copied Pixels = %i\n",current_pixels);
		    		jimage_array = env->NewIntArray(current_pixels);
		    		printf("Writing image to Java Array\n");
		    		env->SetIntArrayRegion(jimage_array,0,current_pixels,image_array);
		    		free(image_array);
		   	        setSDKError(error);
		    	  return jimage_array;
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	}
		    	if(error == DRV_ACQUIRING){
		    	  printf("Acquisition in progress = %i\n",error);
		    	}
		    	if(error == DRV_ERROR_ACK){
		    	  printf("Unable to communicate with card = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("Invalid pointer (i.e. NULL) = %i\n",error);
		    	}
		    	if(error == DRV_P2INVALID){
		    	  printf("Array size is incorrect = %i\n",error);
		    	}
		    	if(error == DRV_NO_NEW_DATA){
		    	  printf("No acquisition has taken place = %i\n",error);
		    	}
	     setSDKError(error);
    }
    return NULL;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetNewData
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetNewData
   * Signature: ()[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetNewData(JNIEnv *env, jclass obj, jint pixels){
	  jintArray  jimage_array;
	  at_32*     image_array;
	  unsigned long  current_pixels = (unsigned long)pixels;
	                 image_array    = new at_32[current_pixels];
	  int            error          = 0;
	  printf("PIXELS = %i\n",current_pixels);
	  if(isInitialized()){
	   	 error = GetNewData(image_array,current_pixels);
		    	if(error == DRV_SUCCESS){
		    	  printf("Data copied Pixels = %i\n",current_pixels);
		    		jimage_array = env->NewIntArray(current_pixels);
		    		printf("Writing image to Java Array\n");
		    		env->SetIntArrayRegion(jimage_array,0,current_pixels,image_array);
		    		free(image_array);
		   	        setSDKError(error);
		    	  return jimage_array;
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	}
		    	if(error == DRV_ACQUIRING){
		    	  printf("Acquisition in progress = %i\n",error);
		    	}
		    	if(error == DRV_ERROR_ACK){
		    	  printf("Unable to communicate with card = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("Invalid pointer (i.e. NULL) = %i\n",error);
		    	}
		    	if(error == DRV_P2INVALID){
		    	  printf("Array size is incorrect = %i\n",error);
		    	}
		    	if(error == DRV_NO_NEW_DATA){
		    	  printf("No acquisition has taken place = %i\n",error);
		    	}
	     setSDKError(error);
    }
    return NULL;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetImages
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetImages
   * Signature: (II)[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetImages(JNIEnv *env, jclass obj, jint first_image, jint last_image, jint pixels){
	  jintArray  jimage_array;
	  at_32*     image_array;
	  int        long_first_image = (int)first_image;
	  int        long_last_image  = (int)last_image;
	  int        valid_first      = 0;
	  int        valid_last       = 0;
	  int        num_images       = long_last_image - long_first_image;
	  int  current_pixels = (int)pixels;
	                 image_array    = new at_32[current_pixels*num_images];
	  int            error          = 0;
	  printf("PIXELS = %i\n",current_pixels);
	  if(isInitialized()){
	   	 error = GetImages(long_first_image,long_last_image,image_array,current_pixels,&valid_first,&valid_last);
		    	if(error == DRV_SUCCESS){
		    	  printf("Data copied Pixels = %i\n",current_pixels);
		    		jimage_array = env->NewIntArray(current_pixels);
		    		printf("Writing image to Java Array\n");
		    		env->SetIntArrayRegion(jimage_array,0,current_pixels,image_array);
		    		free(image_array);
		   	        setSDKError(error);
		    	  return jimage_array;
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	}
		    	if(error == DRV_ERROR_ACK){
		    	  printf("Unable to communicate with card = %i\n",error);
		    	}
		    	if(error == DRV_GENERAL_ERRORS){
		    	  printf("The series is out of range = %i\n",error);
		    	}
		    	if(error == DRV_P3INVALID){
		    	  printf("Invalid pointer (i.e. NULL) = %i\n",error);
		    	}
		    	if(error == DRV_P3INVALID){
		    	  printf("Array size is incorrect = %i\n",error);
		    	}
		    	if(error == DRV_NO_NEW_DATA){
		    	  printf("No acquisition has taken place = %i\n",error);
		    	}
	     setSDKError(error);
  }
  return NULL;
 }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetMostRecentImage
/=============================================================================================*/
/*
   * Class:     andor2_AndorCamera2
   * Method:    GetMostRecentImage
   * Signature: ()[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetMostRecentImage(JNIEnv *env, jclass obj, jint pixels){
	  jintArray  jimage_array;
	  at_32*     image_array;
	  unsigned long  current_pixels = (unsigned long)pixels;
	                 image_array    = new at_32[current_pixels];
	  int            error          = 0;
	  printf("PIXELS = %i\n",current_pixels);
	  if(isInitialized()){
	   	 error = GetMostRecentImage(image_array,current_pixels);
		    	if(error == DRV_SUCCESS){
		    	  printf("Data copied Pixels = %i\n",current_pixels);
		    		jimage_array = env->NewIntArray(current_pixels);
		    		printf("Writing image to Java Array\n");
		    		env->SetIntArrayRegion(jimage_array,0,current_pixels,image_array);
		    		free(image_array);
		   	        setSDKError(error);
		    	  return jimage_array;
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	}
		    	if(error == DRV_ACQUIRING){
		    	  printf("Acquisition in progress = %i\n",error);
		    	}
		    	if(error == DRV_ERROR_ACK){
		    	  printf("Unable to communicate with card = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("Invalid pointer (i.e. NULL) = %i\n",error);
		    	}
		    	if(error == DRV_P2INVALID){
		    	  printf("Array size is incorrect = %i\n",error);
		    	}
		    	if(error == DRV_NO_NEW_DATA){
		    	  printf("No acquisition has taken place = %i\n",error);
		    	}
	     setSDKError(error);
    }
    return NULL;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetOldestImage
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetOldestImage
   * Signature: ()[I
   */
  JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetOldestImage(JNIEnv *env, jclass obj, jint pixels){
	  jintArray  jimage_array;
	  at_32*     image_array;
	  unsigned long  current_pixels = (unsigned long)pixels;
	                 image_array    = new at_32[current_pixels];
	  int            error          = 0;
	  printf("PIXELS = %i\n",current_pixels);
	  if(isInitialized()){
	   	 error = GetOldestImage(image_array,current_pixels);
		    	if(error == DRV_SUCCESS){
		    	  printf("Data copied Pixels = %i\n",current_pixels);
		    		jimage_array = env->NewIntArray(current_pixels);
		    		printf("Writing image to Java Array\n");
		    		env->SetIntArrayRegion(jimage_array,0,current_pixels,image_array);
		    		free(image_array);
		   	        setSDKError(error);
		    	  return jimage_array;
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	}
		    	if(error == DRV_ACQUIRING){
		    	  printf("Acquisition in progress = %i\n",error);
		    	}
		    	if(error == DRV_ERROR_ACK){
		    	  printf("Unable to communicate with card = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("Invalid pointer (i.e. NULL) = %i\n",error);
		    	}
		    	if(error == DRV_P2INVALID){
		    	  printf("Array size is incorrect = %i\n",error);
		    	}
		    	if(error == DRV_NO_NEW_DATA){
		    	  printf("No acquisition has taken place = %i\n",error);
		    	}
	     setSDKError(error);
    }
    return NULL;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetOldestImage
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetAcquiredData16
   * Signature: (I)[S
   */
  JNIEXPORT jshortArray JNICALL Java_andor2_AndorCamera2_GetAcquiredData16(JNIEnv *env , jclass obj, jint pixels){
	  jshortArray     jimage_array;
	  unsigned short* image_array;
	  unsigned long   current_pixels = (unsigned long)pixels;
	                  image_array    = new unsigned short[current_pixels];
	  int             error          = 0;
	  printf("PIXELS = %i\n",current_pixels);
	  if(isInitialized()){
	   	 error = GetAcquiredData16(image_array,current_pixels);
		    	if(error == DRV_SUCCESS){
		    	  printf("Data copied Pixels = %i\n",current_pixels);
		    		jimage_array = env->NewShortArray(current_pixels);
		    		printf("Writing image to Java Array\n");
		    		env->SetShortArrayRegion(jimage_array,0,current_pixels,(jshort *)image_array);
		    		free(image_array);
		   	        setSDKError(error);
		    	  return jimage_array;
		    	}
		    	if(error == DRV_NOT_INITIALIZED){
		    	  printf("Driver Not Initialized = %i\n",error);
		    	}
		    	if(error == DRV_ACQUIRING){
		    	  printf("Acquisition in progress = %i\n",error);
		    	}
		    	if(error == DRV_ERROR_ACK){
		    	  printf("Unable to communicate with card = %i\n",error);
		    	}
		    	if(error == DRV_P1INVALID){
		    	  printf("Invalid pointer (i.e. NULL) = %i\n",error);
		    	}
		    	if(error == DRV_P2INVALID){
		    	  printf("Array size is incorrect = %i\n",error);
		    	}
		    	if(error == DRV_NO_NEW_DATA){
		    	  printf("No acquisition has taken place = %i\n",error);
		    	}
	     setSDKError(error);
    }
    return NULL;
  }

  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetNewData16
   * Signature: (I)[S
   */
  JNIEXPORT jshortArray JNICALL Java_andor2_AndorCamera2_GetNewData16
    (JNIEnv *, jclass, jint);

/*=============================================================================================
/   Java_andor2_AndorCamera2_GetImages16
/=============================================================================================*/
 /*
   * Class:     andor2_AndorCamera2
   * Method:    GetImages16
   * Signature: (III)[S
   */
  JNIEXPORT jshortArray JNICALL Java_andor2_AndorCamera2_GetImages16(JNIEnv *env, jclass obj, jint first_image, jint last_image, jint pixels){
  jshortArray  jimage_array;
  unsigned short*     image_array;
  int        long_first_image = (int)first_image;
  int        long_last_image  = (int)last_image;
  int        valid_first      = 0;
  int        valid_last       = 0;
  int        num_images       = long_last_image - long_first_image;
  int        current_pixels   = (int)pixels;
                 image_array    = new unsigned short[current_pixels*num_images];
  int            error          = 0;
  printf("PIXELS = %i\n",current_pixels);
  if(isInitialized()){
   	 error = GetImages16(long_first_image,long_last_image,image_array,current_pixels,&valid_first,&valid_last);
	    	if(error == DRV_SUCCESS){
	    	  printf("Data copied Pixels = %i\n",current_pixels);
	    		jimage_array = env->NewShortArray(current_pixels);
	    		printf("Writing image to Java Array\n");
	    		env->SetShortArrayRegion(jimage_array,0,current_pixels,(jshort *)image_array);
	    		free(image_array);
	   	        setSDKError(error);
	    	  return jimage_array;
	    	}
	    	if(error == DRV_NOT_INITIALIZED){
	    	  printf("Driver Not Initialized = %i\n",error);
	    	}
	    	if(error == DRV_ERROR_ACK){
	    	  printf("Unable to communicate with card = %i\n",error);
	    	}
	    	if(error == DRV_GENERAL_ERRORS){
	    	  printf("The series is out of range = %i\n",error);
	    	}
	    	if(error == DRV_P3INVALID){
	    	  printf("Invalid pointer (i.e. NULL) = %i\n",error);
	    	}
	    	if(error == DRV_P3INVALID){
	    	  printf("Array size is incorrect = %i\n",error);
	    	}
	    	if(error == DRV_NO_NEW_DATA){
	    	  printf("No acquisition has taken place = %i\n",error);
	    	}
     setSDKError(error);
  }
  return NULL;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetMostRecentImage16
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    GetMostRecentImage16
   * Signature: (I)[S
   */
  JNIEXPORT jshortArray JNICALL Java_andor2_AndorCamera2_GetMostRecentImage16(JNIEnv *env, jclass obj, jint pixels){
  jshortArray     jimage_array;
  unsigned short* image_array;
  unsigned long   current_pixels = (unsigned long)pixels;
                  image_array    = new unsigned short[current_pixels];
  int             error          = 0;
  printf("PIXELS = %i\n",current_pixels);
  if(isInitialized()){
   	 error = GetMostRecentImage16(image_array,current_pixels);
	    	if(error == DRV_SUCCESS){
	    	  printf("Data copied Pixels = %i\n",current_pixels);
	    		jimage_array = env->NewShortArray(current_pixels);
	    		printf("Writing image to Java Array\n");
	    		env->SetShortArrayRegion(jimage_array,0,current_pixels,(jshort *)image_array);
	    		free(image_array);
	   	        setSDKError(error);
	    	  return jimage_array;
	    	}
	    	if(error == DRV_NOT_INITIALIZED){
	    	  printf("Driver Not Initialized = %i\n",error);
	    	}
	    	if(error == DRV_ACQUIRING){
	    	  printf("Acquisition in progress = %i\n",error);
	    	}
	    	if(error == DRV_ERROR_ACK){
	    	  printf("Unable to communicate with card = %i\n",error);
	    	}
	    	if(error == DRV_P1INVALID){
	    	  printf("Invalid pointer (i.e. NULL) = %i\n",error);
	    	}
	    	if(error == DRV_P2INVALID){
	    	  printf("Array size is incorrect = %i\n",error);
	    	}
	    	if(error == DRV_NO_NEW_DATA){
	    	  printf("No acquisition has taken place = %i\n",error);
	    	}
     setSDKError(error);
  }
  return NULL;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetOldestImage16
/=============================================================================================*/
 /*
   * Class:     andor2_AndorCamera2
   * Method:    GetOldestImage16
   * Signature: (I)[S
   */
  JNIEXPORT jshortArray JNICALL Java_andor2_AndorCamera2_GetOldestImage16(JNIEnv *env, jclass obj, jint pixels){
  jshortArray     jimage_array;
  unsigned short* image_array;
  unsigned long   current_pixels = (unsigned long)pixels;
                  image_array    = new unsigned short[current_pixels];
  int             error          = 0;
  printf("PIXELS = %i\n",current_pixels);
  if(isInitialized()){
   	 error = GetOldestImage16(image_array,current_pixels);
	    	if(error == DRV_SUCCESS){
	    	  printf("Data copied Pixels = %i\n",current_pixels);
	    		jimage_array = env->NewShortArray(current_pixels);
	    		printf("Writing image to Java Array\n");
	    		env->SetShortArrayRegion(jimage_array,0,current_pixels,(jshort *)image_array);
	    		free(image_array);
	   	        setSDKError(error);
	    	  return jimage_array;
	    	}
	    	if(error == DRV_NOT_INITIALIZED){
	    	  printf("Driver Not Initialized = %i\n",error);
	    	}
	    	if(error == DRV_ACQUIRING){
	    	  printf("Acquisition in progress = %i\n",error);
	    	}
	    	if(error == DRV_ERROR_ACK){
	    	  printf("Unable to communicate with card = %i\n",error);
	    	}
	    	if(error == DRV_P1INVALID){
	    	  printf("Invalid pointer (i.e. NULL) = %i\n",error);
	    	}
	    	if(error == DRV_P2INVALID){
	    	  printf("Array size is incorrect = %i\n",error);
	    	}
	    	if(error == DRV_NO_NEW_DATA){
	    	  printf("No acquisition has taken place = %i\n",error);
	    	}
     setSDKError(error);
  }
  return NULL;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_WaitForAcquisition
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    WaitForAcquisition
   * Signature: ()I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_WaitForAcquisition(JNIEnv *env, jclass obj){
  int   error = 0;
   if(isInitialized()){
   	error = WaitForAcquisition();
   	if(error == DRV_SUCCESS){
   	  printf("WaitForAcquisition SUCCESSFUL = %i\n",error);
   	}
   	if(error == DRV_NOT_INITIALIZED){
  	      printf("WaitForAcquisition Camera NOT initialized = %i\n",error);
   	}
   	if(error == DRV_NO_NEW_DATA){
  	      printf("WaitForAcquisition Non-acquisition Event occured (e.g. CancelWait() = %i\n",error);
   	}
   }
  setSDKError(error);
  return error;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_WaitForAcquisitionByHandle
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    WaitForAcquisitionByHandle
   * Signature: (I)I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_WaitForAcquisitionByHandle(JNIEnv *env, jclass obj, jint handle){
	  long  cameraHandle = (long)handle;
	  int   error = 0;
	    if(isInitialized()){
	    	error = WaitForAcquisitionByHandle(cameraHandle);
	    	if(error == DRV_SUCCESS){
	    	  printf("WaitForAcquisitionByHandle SUCCESSFUL = %i\n",error);
	    	}
	    	if(error == DRV_P1INVALID){
	   	      printf("WaitForAcquisitionByHandle Handle not valid = %i\n",error);
	    	}
	    	if(error == DRV_NO_NEW_DATA){
	   	      printf("WaitForAcquisitionByHandle Non-acquisition Event occurred (e.g. CancelWait() = %i\n",error);
	    	}
	    }
	   setSDKError(error);
	   return error;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_WaitForAcquisitionByHandleTimeOut
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    WaitForAcquisitionByHandleTimeOut
   * Signature: (JI)I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_WaitForAcquisitionByHandleTimeOut(JNIEnv *env, jclass obj, jlong handle, jint iTimeOutMs){
  long  cameraHandle = (long)handle;
  int   timeout      = (int)iTimeOutMs;
  int   error = 0;
    if(isInitialized()){
    	error = WaitForAcquisitionByHandleTimeOut(cameraHandle,timeout);
    	if(error == DRV_SUCCESS){
    	  printf("WaitForAcquisitionByHandleTimeOut SUCCESSFUL = %i\n",error);
    	}
    	if(error == DRV_P1INVALID){
   	      printf("WaitForAcquisitionByHandleTimeOut Handle not valid = %i\n",error);
    	}
    	if(error == DRV_NO_NEW_DATA){
   	      printf("WaitForAcquisitionByHandleTimeOut Non-acquisition Event occurred (e.g. CancelWait() = %i\n",error);
    	}
    }
   setSDKError(error);
   return error;
  }
/*=============================================================================================
/   Java_andor2_AndorCamera2_WaitForAcquisitionTimeOut
/=============================================================================================*/
  /*
   * Class:     andor2_AndorCamera2
   * Method:    WaitForAcquisitionTimeOut
   * Signature: (I)I
   */
  JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_WaitForAcquisitionTimeOut(JNIEnv *env , jclass obj, jint iTimeOutMs){
  int   timeout      = (int)iTimeOutMs;
  int   error = 0;
    if(isInitialized()){
    	error = WaitForAcquisitionTimeOut(timeout);
    	if(error == DRV_SUCCESS){
    	  printf("WaitForAcquisitionTimeOut SUCCESSFUL = %i\n",error);
    	}
    	if(error == DRV_NO_NEW_DATA){
   	      printf("WaitForAcquisitionTimeOut Non-acquisition Event occurred (e.g. CancelWait() = %i\n",error);
    	}
    }
   setSDKError(error);
   return error;
  }
/*=============================================================================================
/   Java_andor_AndorCamera2_1test_GetAcquisitionProgress(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    GetAcquisitionProgress
 * Signature: ()[J
 */
JNIEXPORT jlongArray JNICALL Java_andor2_AndorCamera2_GetAcquisitionProgress(JNIEnv *env, jclass obj){
at_32      accumulations, kinetics;
long      *progress_array;
jlongArray jprogress_array;
progress_array = new long[2];
int   error = 0;
 if(isInitialized()){
 	error = GetAcquisitionProgress(&accumulations, &kinetics);
 	if(error == DRV_SUCCESS){
 	  progress_array[0] = accumulations;
 	  progress_array[1] = kinetics;
 	  printf("GetAcquisitionProgress SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	      printf("GetAcquisitionProgress Camera NOT initialized = %i\n",error);
 	}
 }
 setSDKError(error);
 jprogress_array = env->NewLongArray(2);
 env->SetLongArrayRegion(jprogress_array,0,2,progress_array);
 free(progress_array);
 return jprogress_array;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetStatus
/=============================================================================================*/
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetStatus(JNIEnv *env, jclass obj){
	int   status = 0;
	int   error  = 0;
	 if(isInitialized()){
	 	error = GetStatus(&status);
	 	if(error == DRV_SUCCESS){
//	 	  printf("GetStatus SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetStatus Driver Not Initialized  = %i\n",error);
		   status = error;
	 	}
	 }
	 setSDKError(error);
  return (jint)status;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetAcquisitionTimings(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor_AndorCamera2_test
 * Method:    GetAcquisitionTimings
 * Signature: ()[F
 */
JNIEXPORT jfloatArray JNICALL Java_andor2_AndorCamera2_GetAcquisitionTimings(JNIEnv *env, jclass obj){
float      accumulate, exposure, kinetics;
float      *timing_array;
jfloatArray jtiming_array;
timing_array = new float[3];
int   error = 0;
 if(isInitialized()){
 	error = GetAcquisitionTimings(&exposure, &accumulate, &kinetics);
 	if(error == DRV_SUCCESS){
 		timing_array[0] = exposure;
 		timing_array[1] = accumulate;
 		timing_array[2] = kinetics;
 	  printf("GetAcquisitionTimings SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("GetAcquisitionTimings Camera NOT initialized = %i\n",error);
 	}
 }
 setSDKError(error);
 jtiming_array = env->NewFloatArray(3);
 env->SetFloatArrayRegion(jtiming_array,0,3,timing_array);
 free(timing_array);
 return jtiming_array;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetAcquisitionTimings(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetCameraEventStatus
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetCameraEventStatus(JNIEnv *env, jclass obj){
	// NOTE THIS METHOD IS NOT WORKING - THROWS A SYMBOL NOT DEFINED ERROR
	at_u32       status = 0;
	int          error  = 0;
	 if(isInitialized()){
	 	error = GetCameraEventStatus(&status);
	 	if(error == DRV_SUCCESS){
	 	  printf("GetCameraEventStatus SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetCameraEventStatus Driver Not Initialized  = %i\n",error);
		   status = error;
	 	}
	 }
	 setSDKError(error);
  return (jint)status;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetReadOutTime(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetReadOutTime
 * Signature: ()F
 */
JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetReadOutTime(JNIEnv *env, jclass obj){
	float readout_time = 0;
	int          error  = 0;
	 if(isInitialized()){
	 	error = GetReadOutTime(&readout_time);
	 	if(error == DRV_SUCCESS){
	 	  printf("GetReadOutTime SUCCESSFUL = %i\n",readout_time);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetReadOutTime Driver Not Initialized  = %i\n",error);
	 	}
	 	if(error == DRV_ERROR_CODES){
		   printf("GetReadOutTime error communicating with camera  = %i\n",error);
	 	}
	 }
	 setSDKError(error);
  return (jfloat)readout_time;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetReadOutTime(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetKeepCleanTime
 * Signature: ()F
 */
JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetKeepCleanTime(JNIEnv *env, jclass obj){
	float keep_clean_time = 0;
	int          error  = 0;
	 if(isInitialized()){
	 	error = GetKeepCleanTime(&keep_clean_time);
	 	if(error == DRV_SUCCESS){
	 	  printf("GetKeepCleanTime SUCCESSFUL = %i\n",keep_clean_time);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetKeepCleanTime Driver Not Initialized  = %i\n",error);
	 	}
	 	if(error == DRV_ERROR_CODES){
		   printf("GetKeepCleanTime error communicating with camera  = %i\n",error);
	 	}
	 }
	 setSDKError(error);
  return (jfloat)keep_clean_time;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetNumberHSSpeeds
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetNumberHSSpeeds
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetNumberHSSpeeds(JNIEnv *env, jclass obj, jint channel, jint type){
int   native_channel = (int)channel;
int   native_type    = (int)type;
int   number_of_speeds = 0;
int   error  = 0;
 if(isInitialized()){
 	error = GetNumberHSSpeeds(native_channel,native_type,&number_of_speeds);
 	if(error == DRV_SUCCESS){
 	  printf("GetNumberHSSpeeds SUCCESSFUL = %i\n",number_of_speeds);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("GetNumberHSSpeeds Driver Not Initialized  = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	   printf("GetNumberHSSpeeds Invalid channel  = %i\n",error);
 	}
 	if(error == DRV_P2INVALID){
	   printf("GetNumberHSSpeeds Invalid horizontal read mode  = %i\n",error);
 	}
 }
 setSDKError(error);
return (jint)number_of_speeds;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetNumberVSSpeeds
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetNumberVSSpeeds
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetNumberVSSpeeds(JNIEnv *env, jclass obj){
	int   number_of_speeds = 0;
	int   error  = 0;
	 if(isInitialized()){
	 	error = GetNumberVSSpeeds(&number_of_speeds);
	 	if(error == DRV_SUCCESS){
	 	  printf("GetNumberVSSpeeds SUCCESSFUL = %i\n",number_of_speeds);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetNumberVSSpeeds Driver Not Initialized  = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		   printf("GetNumberVSSpeeds Acquisition in progress = %i\n",error);
	 	}
	 }
	 setSDKError(error);
	return (jint)number_of_speeds;
}

/*=============================================================================================
/   Java_andor2_AndorCamera2_SetVSSpeed
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetVSSpeed
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetVSSpeed(JNIEnv *env, jclass obj, jint index){
int   native_index = (int)index;
int   error  = 0;
 if(isInitialized()){
 	error = SetVSSpeed(native_index);
 	if(error == DRV_SUCCESS){
 	  printf("SetVSSpeed SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("SetVSSpeed Driver Not Initialized  = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	   printf("SetVSSpeed Acquisition in progress = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	   printf("SetVSSpeed Invalid index = %i\n",error);
 	}
 }
 setSDKError(error);
return (jfloat)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetVSSpeed
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetVSSpeed
 * Signature: (I)F
 */
JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetVSSpeed(JNIEnv *env, jclass obj, jint index){
int   native_index = (int)index;
float speed        = 0;
int   error  = 0;
 if(isInitialized()){
 	error = GetVSSpeed(native_index,&speed);
 	if(error == DRV_SUCCESS){
 	  printf("GetVSSpeed SUCCESSFUL = %i\n",speed);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("GetVSSpeed Driver Not Initialized  = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	   printf("GetVSSpeed Acquisition in progress = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	   printf("GetVSSpeed Invalid index = %i\n",error);
 	}
 }
 setSDKError(error);
return (jfloat)speed;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetHSSpeed
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetHSSpeed
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetHSSpeed(JNIEnv *env, jclass obj, jint type, jint index){
int   native_index = (int)index;
int   native_type  = (int)type;
int   error  = 0;
 if(isInitialized()){
 	error = SetHSSpeed(native_type,native_index);
 	if(error == DRV_SUCCESS){
 	  printf("SetHSSpeed SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("SetHSSpeed Driver Not Initialized  = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	   printf("SetHSSpeed Acquisition in progress = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	   printf("SetHSSpeed Mode is invalid = %i\n",error);
 	}
 	if(error == DRV_P2INVALID){
	   printf("SetHSSpeed Index is out of range = %i\n",error);
 	}
 }
 setSDKError(error);
return (jfloat)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetHSSpeed
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetHSSpeed
 * Signature: (III)F
 */
JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetHSSpeed(JNIEnv *env, jclass obj, jint channel, jint type, jint index){
int   native_index   = (int)index;
int   native_channel = (int)channel;
int   native_type    = (int)type;
float speed        = 0;
int   error  = 0;
 if(isInitialized()){
 	error = GetHSSpeed(native_channel,native_type,native_index,&speed);
 	if(error == DRV_SUCCESS){
 	  printf("GetHSSpeed SUCCESSFUL = %i\n",speed);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("GetHSSpeed Driver Not Initialized  = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	   printf("GetHSSpeed Acquisition in progress = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	   printf("GetHSSpeed Invalid channel = %i\n",error);
 	}
 	if(error == DRV_P2INVALID){
	   printf("GetHSSpeed Invalid horizontal read mode = %i\n",error);
 	}
 	if(error == DRV_P3INVALID){
	   printf("GetHSSpeed Invalid index = %i\n",error);
 	}
 }
 setSDKError(error);
return (jfloat)speed;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetFastestRecommendedVSSpeed_1index
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetFastestRecommendedVSSpeed_index
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetFastestRecommendedVSSpeed_1index(JNIEnv *env, jclass obj){
	int   index   = MISSING_INTEGER;
	float speed   = 0;
	int   error  = 0;
	 if(isInitialized()){
	 	error = GetFastestRecommendedVSSpeed(&index,&speed);
	 	if(error == DRV_SUCCESS){
	 	  printf("GetFastestRecommendedVSSpeed SUCCESSFUL = %i\n",index);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetFastestRecommendedVSSpeed Driver Not Initialized  = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		   printf("GetFastestRecommendedVSSpeed Acquisition in progress = %i\n",error);
	 	}
	 }
	 setSDKError(error);
	return (jint)index;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetFastestRecommendedVSSpeed_1speed
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetFastestRecommendedVSSpeed_speed
 * Signature: ()F
 */
JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetFastestRecommendedVSSpeed_1speed(JNIEnv *env, jclass obj){
int   index   = MISSING_INTEGER;
float speed   = 0;
int   error  = 0;
 if(isInitialized()){
 	error = GetFastestRecommendedVSSpeed(&index,&speed);
 	if(error == DRV_SUCCESS){
 	  printf("GetFastestRecommendedVSSpeed SUCCESSFUL = %i\n",index);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("GetFastestRecommendedVSSpeed Driver Not Initialized  = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	   printf("GetFastestRecommendedVSSpeed Acquisition in progress = %i\n",error);
 	}
 }
 setSDKError(error);
return (jfloat)speed;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetVSAmplitude(JNIEnv *env, jclass obj, jint amplitude)
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetVSAmplitude
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetVSAmplitude(JNIEnv *env, jclass obj, jint amplitude){
	   int native_amplitude = amplitude;
	   int   error  = 0;
	    if(isInitialized()){
	    	error = SetVSAmplitude(native_amplitude);
	    	if(error == DRV_SUCCESS){
	    	  printf("SetVSAmplitude SUCCESSFUL = %i\n",error);
	    	}
	    	if(error == DRV_NOT_INITIALIZED){
	   	      printf("SetVSAmplitude Driver Not Initialized  = %i\n",error);
	    	}
	    	if(error == DRV_NOT_AVAILABLE){
	   	      printf("SetVSAmplitude Advanced EM gain not available for this camera  = %i\n",error);
	    	}
	    	if(error == DRV_ACQUIRING){
	   	      printf("SetVSAmplitude Acquisition in progress = %i\n",error);
	    	}
	    	if(error == DRV_P1INVALID){
	   	      printf("SetVSAmplitude Invalid amplitude parameter = %i\n",error);
	    	}
	    }
	setSDKError(error);
	return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetNumberVSAmplitudes(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetNumberVSAmplitudes
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetNumberVSAmplitudes(JNIEnv *env, jclass obj){
	int   number_of_amplitudes   = MISSING_INTEGER;
	int   error  = 0;
	 if(isInitialized()){
	 	error = GetNumberVSAmplitudes(&number_of_amplitudes);
	 	if(error == DRV_SUCCESS){
	 	  printf("GetNumberVSAmplitudes SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetNumberVSAmplitudes Driver Not Initialized  = %i\n",error);
	 	}
	 	if(error == DRV_NOT_AVAILABLE){
		   printf("GetNumberVSAmplitudes Camera does not support this feature = %i\n",error);
	 	}
	 }
	 setSDKError(error);
	return (jint)number_of_amplitudes;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetVSAmplitudeValue(JNIEnv *env, jclass obj, jint index)
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetVSAmplitudeValue
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetVSAmplitudeValue(JNIEnv *env, jclass obj, jint index){
	int   native_index   = (int)index;
	int   value          = 0;
	int   error  = 0;
	 if(isInitialized()){
	 	error = GetVSAmplitudeValue(native_index,&value);
	 	if(error == DRV_SUCCESS){
	 	  printf("GetVSAmplitudeValue SUCCESSFUL = %i\n",index);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetVSAmplitudeValue Driver Not Initialized  = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		   printf("GetVSAmplitudeValues Invalid index = %i\n",error);
	 	}
	 	if(error == DRV_P2INVALID){
		   printf("GetVSAmplitudeValues Invalid value pointer = %i\n",error);
	 	}
	 }
	 setSDKError(error);
	return (jint)value;

}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetVSAmplitudeString(JNIEnv *env, jclass obj, jint index)
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetVSAmplitudeString
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_andor2_AndorCamera2_GetVSAmplitudeString(JNIEnv *env, jclass obj, jint index){
    int   MAX_LENGTH = 10;
    int   native_index = (int)index;
	char *amplitude_string;
	      amplitude_string = new char[MAX_LENGTH];
    int   error = 0;
			    if(isInitialized()){
			    	error = GetVSAmplitudeString(native_index,amplitude_string);
				 	if(error == DRV_SUCCESS){
				 	  printf("GetVSAmplitudeString SUCCESSFUL = %i\n",index);
				 	}
				 	if(error == DRV_NOT_INITIALIZED){
					   printf("GetVSAmplitudeString Driver Not Initialized  = %i\n",error);
				 	}
				 	if(error == DRV_P1INVALID){
					   printf("GetVSAmplitudeString Invalid index = %i\n",error);
				 	}
				 	if(error == DRV_P2INVALID){
					   printf("GetVSAmplitudeString Invalid text pointer = %i\n",error);
				 	}
			    }
		 setSDKError(error);
		 jchar *java_string = new jchar[MAX_LENGTH];
		  for(int i=0;i<MAX_LENGTH;i++){
		     java_string[i] = amplitude_string[i];
		  }
	free(amplitude_string);
   return env->NewString(java_string,MAX_LENGTH);
}

/*
 * Class:     andor2_AndorCamera2
 * Method:    GetVSAmplitudeFromString
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetVSAmplitudeFromString(JNIEnv *env, jclass obj, jstring amplitude_string){
	const char  *native_string;
	char  *current_string;
	jsize  stringlength     = env->GetStringLength(amplitude_string);
	       current_string   = new char[stringlength];
	       native_string = env->GetStringUTFChars(amplitude_string,0);
	sprintf(current_string,"%s",native_string);
    int   index = MISSING_INTEGER;
    int   error = 0;
			    if(isInitialized()){
			    	error = GetVSAmplitudeFromString(current_string,&index);
				 	if(error == DRV_SUCCESS){
				 	  printf("GetVSAmplitudeString SUCCESSFUL = %i\n",index);
				 	}
				 	if(error == DRV_NOT_INITIALIZED){
					   printf("GetVSAmplitudeString Driver Not Initialized  = %i\n",error);
				 	}
				 	if(error == DRV_P1INVALID){
					   printf("GetVSAmplitudeString Invalid index = %i\n",error);
				 	}
				 	if(error == DRV_P2INVALID){
					   printf("GetVSAmplitudeString Invalid text pointer = %i\n",error);
				 	}
			    }
		 setSDKError(error);
	free(current_string);
   return (jint)index;
}

/*=============================================================================================
/   Java_andor2_AndorCamera2_SetEMAdvanced
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetEMAdvanced
 * Signature: (Z)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetEMAdvanced(JNIEnv *env, jclass obj, jboolean state){
   int native_state = 0;
   if(state == true){
	  native_state = 1;
   }
   if(state == false){
	  native_state = 0;
   }
   int   error  = 0;
    if(isInitialized()){
    	error = SetEMAdvanced(native_state);
    	if(error == DRV_SUCCESS){
    	  printf("SetEMAdvanced SUCCESSFUL = %i\n",error);
    	}
    	if(error == DRV_NOT_INITIALIZED){
   	      printf("SetEMAdvanced Driver Not Initialized  = %i\n",error);
    	}
    	if(error == DRV_NOT_AVAILABLE){
   	      printf("SetEMAdvanced Advanced EM gain not available for this camera  = %i\n",error);
    	}
    	if(error == DRV_ACQUIRING){
   	      printf("SetEMAdvanced Acquisition in progress = %i\n",error);
    	}
    	if(error == DRV_P1INVALID){
   	      printf("SetEMAdvanced State parameter was not zero or one = %i\n",error);
    	}
    }
    setSDKError(error);
   return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetEMCCDGain
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetEMCCDGain
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetEMCCDGain(JNIEnv *env, jclass obj, jint emgain){
	int   native_emgain = (int)emgain;
	int   error  = 0;
	 if(isInitialized()){
	 	error = SetEMCCDGain(native_emgain);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetEMCCDGain SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("SetEMCCDGain Driver Not Initialized  = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		   printf("SetEMCCDGain Acquisition in progress = %i\n",error);
	 	}
	 	if(error == DRV_I2CTIMEOUT){
		   printf("SetEMCCDGain I2C command timed out = %i\n",error);
	 	}
	 	if(error == DRV_I2CDEVNOTFOUND){
		   printf("SetEMCCDGain I2C device not present = %i\n",error);
	 	}
	 	if(error == DRV_ERROR_ACK){
		   printf("SetEMCCDGain Unable to communicate with card = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		   printf("SetEMCCDGain Gain value invalid = %i\n",error);
	 	}
	 }
	 setSDKError(error);
	return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetEMGainMode
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetEMGainMode
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetEMGainMode(JNIEnv *env, jclass obj, jint mode){
// Mode = 0 The EM Gain is controlled by DAC settings in the range 0-255.  Default mode
// Mode = 1 The EM Gain is controlled by DAC settings in the range 0-4095
// Mode = 2 Linear mode
// Mode = 3 Real EM gain
int   native_mode = (int)mode;
int   error  = 0;
 if(isInitialized()){
 	error = SetEMGainMode(native_mode);
 	if(error == DRV_SUCCESS){
 	  printf("SetEMGainMode SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("SetEMGainMode Driver Not Initialized  = %i\n",error);
 	}
 	if(error == DRV_ACQUIRING){
	   printf("SetEMGainMode Acquisition in progress = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	   printf("SetEMGainMode EM Gain mode invalid = %i\n",error);
 	}
 }
 setSDKError(error);
return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetEMCCDGain
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetEMCCDGain
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetEMCCDGain(JNIEnv *env, jclass obj){
int   emgain = MISSING_INTEGER;
int   error  = 0;
 if(isInitialized()){
 	error = GetEMCCDGain(&emgain);
 	if(error == DRV_SUCCESS){
 	  printf("GetEMCCDGain SUCCESSFUL = %i\n",emgain);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("GetEMCCDGain Driver Not Initialized  = %i\n",error);
 	}
 	if(error == DRV_ERROR_ACK){
	   printf("GetEMCCDGain Unable to communicate with card = %i\n",error);
 	}
 }
 setSDKError(error);
return (jint)emgain;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetEMGainRange
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetEMGainRange
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetEMGainRange(JNIEnv *env, jclass obj){
		int      lowest_gain;
		int      highest_gain;
		int      *gain_array;
		jintArray jgain_array;
		          gain_array = new int[2];
		int   error = 0;
		 if(isInitialized()){
		 	error = GetEMGainRange(&lowest_gain, &highest_gain);
		 	if(error == DRV_SUCCESS){
		 		gain_array[0] = lowest_gain;
		 		gain_array[1] = highest_gain;
		 	  printf("GetEMGainRange SUCCESSFUL = %i\n",error);
		 	}
		 	if(error == DRV_NOT_INITIALIZED){
			   printf("GetEMGainRange Camera NOT initialized = %i\n",error);
		 	}
		 }
		 setSDKError(error);
		 jgain_array = env->NewIntArray(2);
		 env->SetIntArrayRegion(jgain_array,0,2,gain_array);
    free(gain_array);
   return jgain_array;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetOutputAmplifier
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetOutputAmplifier
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetOutputAmplifier(JNIEnv *env, jclass obj, jint amplifier){
// int EMGAIN_AMPLIFIER       = 0;
// int CONVENTIONAL_AMPLIFIER = 1;
	int   native_amplifier = (int)amplifier;
	int   error = 0;
	 if(isInitialized()){
	 	error = SetOutputAmplifier(native_amplifier);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetOutputAmplifier SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("SetOutputAmplifier Camera NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		   printf("SetEMGainMode Acquisition in progress = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		   printf("SetEMGainMode Output amplifier type invalid = %i\n",error);
	 	}
	 }
	 setSDKError(error);
  return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetAcquisitionTimings(JNIEnv *env, jclass obj)
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    IsCoolerOn
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_andor2_AndorCamera2_IsCoolerOn(JNIEnv *env, jclass obj){
int  mode = MISSING_INTEGER;
bool state = false;
int   error = 0;
    if(isInitialized()){
 	   error = IsCoolerOn(&mode);
 	   if(error == DRV_SUCCESS){
 	     printf("IsCoolerOn = %i\n",mode);
 	     if(mode == ON){
 	    	state = true;
 	     }
 	     if(mode == OFF){
 	    	state = false;
 	     }
 	   }
 	   if(error == DRV_NOT_INITIALIZED){
 	     printf("IsCoolerOn = %i\n",mode);
 	   }
       if(error == DRV_P1INVALID){
  	      printf("IsCoolerOn  Parameter is NULL= %i\n",error);
       }
    }
  setSDKError(error);
  return (jboolean)state;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_CoolerOFF
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    CoolerOFF
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_CoolerOFF(JNIEnv *env, jclass obj){
	int   error = 0;
	    if(isInitialized()){
	 	   error = CoolerOFF();
	 	   if(error == DRV_SUCCESS){
	 	     printf("CoolerOFF = %i\n",error);
	 	   }
	 	   if(error == DRV_NOT_INITIALIZED){
	 	     printf("CoolerOFF System not initialized = %i\n",error);
	 	   }
	 	   if(error == DRV_ACQUIRING){
	 	     printf("CoolerOFF Acquisition in progress = %i\n",error);
	 	   }
	       if(error == DRV_ERROR_ACK){
	  	      printf("CoolerOFF Unable to communicate with card= %i\n",error);
	       }
	       if(error == DRV_TEMP_NOT_SUPPORTED){
	  	      printf("CoolerOFF Camera does not support switching cooler off = %i\n",error);
	       }
	    }
	  setSDKError(error);
  return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_CoolerON
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    CoolerON
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_CoolerON(JNIEnv *env, jclass obj){
int   error = 0;
    if(isInitialized()){
 	   error = CoolerON();
 	   if(error == DRV_SUCCESS){
 	     printf("CoolerOFF = %i\n",error);
 	   }
 	   if(error == DRV_NOT_INITIALIZED){
 	     printf("CoolerOFF System not initialized = %i\n",error);
 	   }
 	   if(error == DRV_ACQUIRING){
 	     printf("CoolerOFF Acquisition in progress = %i\n",error);
 	   }
       if(error == DRV_ERROR_ACK){
  	      printf("CoolerOFF Unable to communicate with card= %i\n",error);
       }
       if(error == DRV_TEMP_NOT_SUPPORTED){
  	      printf("CoolerOFF Camera does not support switching cooler off = %i\n",error);
       }
    }
  setSDKError(error);
  return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetCoolerMode
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetCoolerMode
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetCoolerMode(JNIEnv *env, jclass obj, jint mode){
	int   cooler_mode = (int)mode;
	int   error = 0;
	    if(isInitialized()){
	 	   error = SetCoolerMode(cooler_mode);
	 	   if(error == DRV_SUCCESS){
	 	     printf("SetCoolerMode = %i\n",error);
	 	   }
	 	   if(error == DRV_NOT_INITIALIZED){
	 	     printf("SetCoolerMode System not initialized = %i\n",error);
	 	   }
	 	   if(error == DRV_ACQUIRING){
	 	     printf("SetCoolerMode Acquisition in progress = %i\n",error);
	 	   }
	       if(error == DRV_P1INVALID){
	  	      printf("SetCoolerMode State parameter was not zero or one= %i\n",error);
	       }
	       if(error == DRV_TEMP_NOT_SUPPORTED){
	  	      printf("SetCoolerMode Camera does not support switching cooler off = %i\n",error);
	       }
	    }
	  setSDKError(error);
  return (jint)error;
}
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetTemperature
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetTemperature
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetTemperature(JNIEnv *env, jclass obj){
	int   temperature = MISSING_INTEGER;
	int   error = 0;
	    if(isInitialized()){
	 	   error = GetTemperature(&temperature);
	 	   if(error == DRV_SUCCESS){
	 	     printf("GetTemperature = %i\n",error);
	 	   }
	 	   if(error == DRV_NOT_INITIALIZED){
	 	     printf("GetTemperature System not initialized = %i\n",error);
	 	   }
	 	   if(error == DRV_ACQUIRING){
	 	     printf("GetTemperature Acquisition in progress = %i\n",error);
	 	   }
	 	   if(error == DRV_ERROR_ACK){
	 	     printf("GetTemperature Unable to communicate with card = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_OFF){
	 	     printf("GetTemperature Temperature is OFF = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_STABILIZED){
	 	     printf("GetTemperature Temperature has stabilized at set point = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_NOT_REACHED){
	 	     printf("GetTemperature Temperature has not reached set point = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_DRIFT){
	 	     printf("GetTemperature Temperature has stabilized but has since drifted = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_NOT_STABILIZED){
	 	     printf("GetTemperature Temperature is OFF = %i\n",error);
	 	   }
	    }
	  setSDKError(error);
  return (jint)temperature;
}
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetTemperatureF
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetTemperatureF
 * Signature: ()F
 */
JNIEXPORT jfloat JNICALL Java_andor2_AndorCamera2_GetTemperatureF(JNIEnv *env, jclass obj){
	float temperature = 0.0;
	int   error = 0;
	    if(isInitialized()){
	 	   error = GetTemperatureF(&temperature);
	 	   if(error == DRV_SUCCESS){
	 	     printf("GetTemperatureF = %i\n",error);
	 	   }
	 	   if(error == DRV_NOT_INITIALIZED){
	 	     printf("GetTemperatureF System not initialized = %i\n",error);
	 	   }
	 	   if(error == DRV_ACQUIRING){
	 	     printf("GetTemperatureF Acquisition in progress = %i\n",error);
	 	   }
	 	   if(error == DRV_ERROR_ACK){
	 	     printf("GetTemperatureF Unable to communicate with card = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_OFF){
	 	     printf("GetTemperatureF Temperature is OFF = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_STABILIZED){
	 	     printf("GetTemperatureF Temperature has stabilized at set point = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_NOT_REACHED){
	 	     printf("GetTemperatureF Temperature has not reached set point = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_DRIFT){
	 	     printf("GetTemperatureF Temperature has stabilized but has since drifted = %i\n",error);
	 	   }
	 	   if(error == DRV_TEMP_NOT_STABILIZED){
	 	     printf("GetTemperatureF Temperature is OFF = %i\n",error);
	 	   }
	    }
	  setSDKError(error);
  return (jfloat)temperature;
}
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetTemperature_1status
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetTemperature_status
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetTemperature_1status(JNIEnv *env, jclass obj){
int   temperature = MISSING_INTEGER;
int   error = 0;
    if(isInitialized()){
 	   error = GetTemperature(&temperature);
 	   if(error == DRV_SUCCESS){
 	     printf("GetTemperature = %i\n",error);
 	   }
 	   if(error == DRV_NOT_INITIALIZED){
 	     printf("GetTemperature System not initialized = %i\n",error);
 	   }
 	   if(error == DRV_ACQUIRING){
 	     printf("GetTemperature Acquisition in progress = %i\n",error);
 	   }
 	   if(error == DRV_ERROR_ACK){
 	     printf("GetTemperature Unable to communicate with card = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_OFF){
 	     printf("GetTemperature Temperature is OFF = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_STABILIZED){
 	     printf("GetTemperature Temperature has stabilized at set point = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_NOT_REACHED){
 	     printf("GetTemperature Temperature has not reached set point = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_DRIFT){
 	     printf("GetTemperature Temperature has stabilized but has since drifted = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_NOT_STABILIZED){
 	     printf("GetTemperature Temperature is OFF = %i\n",error);
 	   }
    }
  setSDKError(error);
return (jint)error;
}
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetTemperatureF_1status
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetTemperatureF_status
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetTemperatureF_1status(JNIEnv *env, jclass obj){
float temperature = 0.0;
int   error = 0;
    if(isInitialized()){
 	   error = GetTemperatureF(&temperature);
 	   if(error == DRV_SUCCESS){
 	     printf("GetTemperatureF = %i\n",error);
 	   }
 	   if(error == DRV_NOT_INITIALIZED){
 	     printf("GetTemperatureF System not initialized = %i\n",error);
 	   }
 	   if(error == DRV_ACQUIRING){
 	     printf("GetTemperatureF Acquisition in progress = %i\n",error);
 	   }
 	   if(error == DRV_ERROR_ACK){
 	     printf("GetTemperatureF Unable to communicate with card = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_OFF){
 	     printf("GetTemperatureF Temperature is OFF = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_STABILIZED){
 	     printf("GetTemperatureF Temperature has stabilized at set point = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_NOT_REACHED){
 	     printf("GetTemperatureF Temperature has not reached set point = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_DRIFT){
 	     printf("GetTemperatureF Temperature has stabilized but has since drifted = %i\n",error);
 	   }
 	   if(error == DRV_TEMP_NOT_STABILIZED){
 	     printf("GetTemperatureF Temperature is OFF = %i\n",error);
 	   }
    }
  setSDKError(error);
return (jint)error;
}
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetTemperatureRange
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetTemperatureRange
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetTemperatureRange(JNIEnv *env, jclass obj){
	int       minimum_temperature, maximum_temperature;
	int      *temperature_array;
	jintArray jtemperature_array;
	temperature_array = new int[2];
	int   error = 0;
	 if(isInitialized()){
	 	error = GetTemperatureRange(&minimum_temperature, &maximum_temperature);
	 	if(error == DRV_SUCCESS){
	 		temperature_array[0] = minimum_temperature;
	 		temperature_array[1] = maximum_temperature;
	 	  printf("GetTemperatureRange SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetTemperatureRange Camera NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		   printf("GetTemperatureRange Acquisition in progress = %i\n",error);
	 	}
	 }
	 setSDKError(error);
	 jtemperature_array = env->NewIntArray(2);
	 env->SetIntArrayRegion(jtemperature_array,0,2,temperature_array);
	 free(temperature_array);
	 return jtemperature_array;
}

/*
 * Class:     andor2_AndorCamera2
 * Method:    SetFanMode
 * Signature: (Z)I
 */
/*=============================================================================================
/  Java_andor2_AndorCamera2_GetTemperatureRange
/=============================================================================================*/
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetFanMode(JNIEnv *env, jclass obj, jint mode){
	int   fan_mode = (int)mode;
	int   error = 0;
	    if(isInitialized()){
	 	   error = SetFanMode(fan_mode);
	 	   if(error == DRV_SUCCESS){
	 	     printf("SetFanMode = %i\n",error);
	 	   }
	 	   if(error == DRV_NOT_INITIALIZED){
	 	     printf("SetFanMode System not initialized = %i\n",error);
	 	   }
	 	   if(error == DRV_ACQUIRING){
	 	     printf("SetFanMode Acquisition in progress = %i\n",error);
	 	   }
	       if(error == DRV_I2CTIMEOUT){
	  	      printf("SetFanMode I2C command timed out= %i\n",error);
	       }
	       if(error == DRV_I2CDEVNOTFOUND){
	  	      printf("SetFanMode I2C device not present = %i\n",error);
	       }
	       if(error == DRV_ERROR_ACK){
	  	      printf("SetFanMode Unable to communicate with card = %i\n",error);
	       }
	       if(error == DRV_P1INVALID){
	  	      printf("SetFanMode Mode value invalid = %i\n",error);
	       }
	    }
	  setSDKError(error);
  return (jint)error;
}
/*=============================================================================================
/  Java_andor2_AndorCamera2_SetTemperature
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetTemperature
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetTemperature(JNIEnv *env, jclass obj, jint target_temperature){
	int   temperature = (int)target_temperature;
	int   error = 0;
	    if(isInitialized()){
	 	   error = SetTemperature(temperature);
	 	   if(error == DRV_SUCCESS){
	 	     printf("SetTemperature = %i\n",error);
	 	   }
	 	   if(error == DRV_NOT_INITIALIZED){
	 	     printf("SetTemperature System not initialized = %i\n",error);
	 	   }
	 	   if(error == DRV_ACQUIRING){
	 	     printf("SetTemperature Acquisition in progress = %i\n",error);
	 	   }
	 	   if(error == DRV_ERROR_ACK){
	 	     printf("SetTemperature Unable to communicate with card = %i\n",error);
	 	   }
	 	   if(error == DRV_P1INVALID){
	 	     printf("SetTemperature Temperature invalid = %i\n",error);
	 	   }
	       if(error == DRV_TEMP_NOT_SUPPORTED){
	  	      printf("SetTemperature  Camera does not support setting the temperature = %i\n",error);
	       }
	    }
	  setSDKError(error);
	return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetFullImage
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetReadMode
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetReadMode(JNIEnv *env, jclass obj, jint mode){
	// FULL-VERTICAL BINNING = 0
	// MULTITRACK            = 1
	// RANDOM-TRACK          = 2;
	// SINGLE_TRACK          = 3;
	// IMAGE                 = 4;
	int   native_readmode = (int)mode;
	int   error = 0;
	    if(isInitialized()){
	 	   error = SetReadMode(native_readmode);
	 	   if(error == DRV_SUCCESS){
	 	     printf("SetReadMode = %i\n",error);
	 	   }
	 	   if(error == DRV_NOT_INITIALIZED){
	 	     printf("SetReadMode System not initialized = %i\n",error);
	 	   }
	 	   if(error == DRV_ACQUIRING){
	 	     printf("SetReadMode Acquisition in progress = %i\n",error);
	 	   }
	 	   if(error == DRV_P1INVALID){
	 	     printf("SetReadMode Invalid readout mode passed = %i\n",error);
	 	   }
	    }
	  setSDKError(error);
	return (jint)error;

}

/*=============================================================================================
/   Java_andor2_AndorCamera2_SetFullImage
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetFullImage
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetFullImage(JNIEnv *, jclass, jint hbin, jint vbin){
	int   error = 0;
	int   horizontal_binning = (int)hbin;
	int   vertical_binning   = (int)vbin;
	 if(isInitialized()){
	 	error = SetFullImage(horizontal_binning,vertical_binning);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetImage SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		      printf("SetFullImage Camera NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		      printf("SetFullImage Acquisition in progress = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		      printf("SetFullImage Horizontal binning parameter invalid = %i\n",error);
	 	}
	 	if(error == DRV_P2INVALID){
		      printf("SetFullImage Vertical binning parameter invalid = %i\n",error);
	 	}
	 }
	setSDKError(error);
	return error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_SetImage
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetImage
 * Signature: (IIIIII)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetImage(JNIEnv *, jclass, jint hbin, jint vbin, jint hstart, jint hend, jint vstart, jint vend){
	int   error = 0;
	int   horizontal_binning = (int)hbin;
	int   vertical_binning   = (int)vbin;
	int   horizontal_start   = (int)hstart;
	int   horizontal_end     = (int)hend;
	int   vertical_start     = (int)vstart;
	int   vertical_end       = (int)vend;
	 if(isInitialized()){
	 	error = SetImage(horizontal_binning,vertical_binning,horizontal_start,horizontal_end,vertical_start,vertical_end);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetImage SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		      printf("SetImage Camera NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_ACQUIRING){
		      printf("SetImage Acquisition in progress = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		      printf("SetImage Binning parameter invalid = %i\n",error);
	 	}
	 	if(error == DRV_P2INVALID){
		      printf("SetImage Binning parameter invalid = %i\n",error);
	 	}
	 	if(error == DRV_P3INVALID){
		      printf("SetImage Sub-area co-ordinate is invalid = %i\n",error);
	 	}
	 	if(error == DRV_P4INVALID){
		      printf("SetImage Sub-area co-ordinate is invalid = %i\n",error);
	 	}
	 	if(error == DRV_P5INVALID){
		      printf("SetImage Sub-area co-ordinate is invalid = %i\n",error);
	 	}
	 	if(error == DRV_P6INVALID){
		      printf("SetImage Sub-area co-ordinate is invalid = %i\n",error);
	 	}
	 }
	setSDKError(error);
	return error;
}
/*=============================================================================================
/    Java_andor2_AndorCamera2_GetImageFlip
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetImageFlip
 * Signature: ()[I
 */
JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetImageFlip(JNIEnv *env, jclass obj){
	int      iHFlip;
	int      iVFlip;
	int      *flip_array;
	jintArray jflip_array;
	flip_array = new int[2];
	int   error = 0;
	 if(isInitialized()){
	 	error = GetImageFlip(&iHFlip, &iVFlip);
	 	if(error == DRV_SUCCESS){
	 		flip_array[0] = iHFlip;
	 		flip_array[1] = iVFlip;
	 	  printf("GetImageFlip SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetImageFlip Camera NOT initialized = %i\n",error);
	 	}
	 }
	 setSDKError(error);
	 jflip_array = env->NewIntArray(2);
	 env->SetIntArrayRegion(jflip_array,0,2,flip_array);
	 free(flip_array);
	 return jflip_array;
}
/*=============================================================================================
/    Java_andor2_AndorCamera2_GetImageFlip
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetImageFlip
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetImageFlip(JNIEnv *env, jclass obj, jint iHFlip, jint iVFlip){
	int      native_iHFlip = (int)iHFlip;
	int      native_iVFlip = (int)iVFlip;
	int   error = 0;
	 if(isInitialized()){
	 	error = SetImageFlip(native_iHFlip, native_iVFlip);
	 	if(error == DRV_SUCCESS){
	 	  printf("SetImageFlip SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("SetImageFlip Camera NOT initialized = %i\n",error);
	 	}
	 	if(error == DRV_P1INVALID){
		   printf("SetImageFlip HFlip parameter invalid = %i\n",error);
	 	}
	 	if(error == DRV_P2INVALID){
		   printf("SetImageFlip VFlip parameter invalid = %i\n",error);
	 	}
	 }
	 setSDKError(error);
  return (jint)error;
}
/*=============================================================================================
/   Java_andor2_AndorCamera2_GetImageRotate
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetImageRotate
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_GetImageRotate(JNIEnv *env, jclass obj){
 int   rotation = -1;
 int   error    = 0;
 if(isInitialized()){
 	error = GetImageRotate(&rotation);
 	if(error == DRV_SUCCESS){
 	  printf("GetImageRotate SUCCESSFUL = %i\n",error);
 	}
 	if(error == DRV_NOT_INITIALIZED){
	   printf("GetImageRotate Camera NOT initialized = %i\n",error);
 	}
 	if(error == DRV_P1INVALID){
	   printf("GetImageRotate Rotate parameter invalid = %i\n",error);
 	}
 }
 setSDKError(error);
 return (jint)rotation;
}
/*=============================================================================================
/    Java_andor2_AndorCamera2_SetImageRotate
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetImageRotate
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetImageRotate(JNIEnv *env, jclass obj, jint jrotation){
int   rotation = (int)jrotation;
int   error    = 0;
if(isInitialized()){
	error = SetImageRotate(rotation);
	if(error == DRV_SUCCESS){
	  printf("SetImageRotate SUCCESSFUL = %i\n",error);
	}
	if(error == DRV_NOT_INITIALIZED){
	   printf("SetImageRotate Camera NOT initialized = %i\n",error);
	}
	if(error == DRV_P1INVALID){
	   printf("SetImageRotate Rotate parameter invalid = %i\n",error);
	}
}
setSDKError(error);
return (jint)error;
}
/*=============================================================================================
/    Java_andor2_AndorCamera2_GetMaximumBinning
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    GetMaximumBinning
 * Signature: (I)[I
 */
JNIEXPORT jintArray JNICALL Java_andor2_AndorCamera2_GetMaximumBinning(JNIEnv *env, jclass obj, jint readMode){
    int      native_readMode = (int)readMode;
	int      maxHbinning;
	int      maxVbinning;
	int      *binning_array;
	jintArray jbinning_array;
	int      HORIZONTAL = 0;
	int      VERTICAL   = 1;
	binning_array = new int[2];
	int   error = 0;
	 if(isInitialized()){
	 	error = GetMaximumBinning(native_readMode, HORIZONTAL,&maxHbinning);
	 	if(error == DRV_SUCCESS){
	 		binning_array[0] = maxHbinning;
	 	  printf("GetMaximumBinning SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetMaximumBinning Camera NOT initialized = %i\n",error);
	 	}
		if(error == DRV_P1INVALID){
		   printf("GetMaximumBinning Invalid Readmode = %i\n",error);
		}
		if(error == DRV_P2INVALID){
		   printf("GetMaximumBinning HorzVert not equal to 0 or 1 = %i\n",error);
		}
		if(error == DRV_P3INVALID){
		   printf("GetMaximumBinning Invalid MaxBinning address = %i\n",error);
		}
	 	error = GetMaximumBinning(native_readMode, VERTICAL,&maxVbinning);
	 	if(error == DRV_SUCCESS){
	 		binning_array[1] = maxVbinning;
	 	  printf("GetMaximumBinning SUCCESSFUL = %i\n",error);
	 	}
	 	if(error == DRV_NOT_INITIALIZED){
		   printf("GetMaximumBinning Camera NOT initialized = %i\n",error);
	 	}
		if(error == DRV_P1INVALID){
		   printf("GetMaximumBinning Invalid Readmode = %i\n",error);
		}
		if(error == DRV_P2INVALID){
		   printf("GetMaximumBinning HorzVert not equal to 0 or 1 = %i\n",error);
		}
		if(error == DRV_P3INVALID){
		   printf("GetMaximumBinning Invalid MaxBinning address = %i\n",error);
		}
	 }
	 setSDKError(error);
	 jbinning_array = env->NewIntArray(2);
	 env->SetIntArrayRegion(jbinning_array,0,2,binning_array);
	 free(binning_array);
	 return jbinning_array;
}
/*=============================================================================================
/    Java_andor2_AndorCamera2_SetKineticCycleTime
/=============================================================================================*/
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetKineticCycleTime
 * Signature: (F)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetKineticCycleTime(JNIEnv *env, jclass obj, jfloat jtime){
float   native_time = (float)jtime;
int   error    = 0;
if(isInitialized()){
	error = SetKineticCycleTime(native_time);
	if(error == DRV_SUCCESS){
	   printf("SetKineticCycleTime SUCCESSFUL = %i\n",error);
	}
	if(error == DRV_NOT_INITIALIZED){
	   printf("SetKineticCycleTime Camera NOT initialized = %i\n",error);
	}
 	if(error == DRV_ACQUIRING){
	   printf("SetKineticCycleTime Acquisition in progress = %i\n",error);
 	}
	if(error == DRV_P1INVALID){
	   printf("SetKineticCycleTime Time parameter invalid = %i\n",error);
	}
}
setSDKError(error);
return (jint)error;
}
/*
 * Class:     andor2_AndorCamera2
 * Method:    SetNumberKinetics
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetNumberKinetics(JNIEnv *env, jclass obj, jint jnum_kinetics){
	int native_num_kinetics = (int)jnum_kinetics;
	int error               = 0;
	if(isInitialized()){
		error = SetNumberKinetics(native_num_kinetics);
		if(error == DRV_SUCCESS){
		   printf("SetNumberKinetics SUCCESSFUL = %i\n",error);
		}
		if(error == DRV_NOT_INITIALIZED){
		   printf("SetNumberKinetics Camera NOT initialized = %i\n",error);
		}
	 	if(error == DRV_ACQUIRING){
		   printf("SetNumberKinetics Acquisition in progress = %i\n",error);
	 	}
		if(error == DRV_P1INVALID){
		   printf("SetNumberKinetics Time parameter invalid = %i\n",error);
		}
	}
	setSDKError(error);
	return (jint)error;
}
JNIEXPORT jint JNICALL Java_andor2_AndorCamera2_SetFrameTransferMode(JNIEnv *env, jclass obj, jint jframe_transfer_mode){
	int native_frame_transfer_mode = (int)jframe_transfer_mode;
	int error               = 0;
	if(isInitialized()){
		error = SetFrameTransferMode(native_frame_transfer_mode);
		if(error == DRV_SUCCESS){
		   printf("SetFrameTransferMode SUCCESSFUL = %i\n",error);
		}
		if(error == DRV_NOT_INITIALIZED){
		   printf("SetFrameTransferMode Camera NOT initialized = %i\n",error);
		}
	 	if(error == DRV_ACQUIRING){
		   printf("SetFrameTransferMode Acquisition in progress = %i\n",error);
	 	}
		if(error == DRV_P1INVALID){
		   printf("SetFrameTransferModes Time parameter invalid = %i\n",error);
		}
	}
	setSDKError(error);
	return (jint)error;
}
/*
 * Class:     andor_AndorCamera2
 * Method:    GetControllerCardModel
 * Signature: ()Ljava/lang/String;
 */
/*JNIEXPORT jstring JNICALL Java_andor_AndorCamera2_GetControllerCardModel(JNIEnv *env, jclass obj){
char controller_card_model[10];
   int   error = 0;
    if(isInitialized()){
    	error = GetControllerCardModel(controller_card_model);
    	if(error == DRV_SUCCESS){
    	  printf("Controller Card Model = %s\n",controller_card_model);
    	}
    }
setSDKError(error);
}
*/


