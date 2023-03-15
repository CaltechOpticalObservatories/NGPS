/**
 * @file    atmcdLXdsim.cpp
 * @brief   simulated functions from Andor API header atmcdLXd.h
 * @author  David Hale <dhale@caltech.edu>
 *
 * These are functions definitions from /usr/local/include/atmcdLXd.h
 * which are used for the Andor simulator. Their real versions are in
 * the Andor API library. These are their simulated counterparts.
 *
 */

#include "atmcdLXd.h"


unsigned int GetDetector(int * xpixels, int * ypixels){
  return DRV_SUCCESS;
}

unsigned int ShutDown() {
  return DRV_SUCCESS;
}

unsigned int StartAcquisition() {
  return DRV_SUCCESS;
}

unsigned int GetStatus(int * status) {
  return DRV_SUCCESS;
}

unsigned int GetAcquiredData(at_32 * arr, at_u32 size) {
  return DRV_SUCCESS;
}

unsigned int GetAcquiredData16(unsigned short * arr, at_u32 size) {
  return DRV_SUCCESS;
}

unsigned int SaveAsFITS(char * szFileTitle, int typ) {
  return DRV_SUCCESS;
}

unsigned int GetCameraSerialNumber(int * number) {
  *number = 12345;
  return DRV_SUCCESS;
}

unsigned int SetReadMode(int mode) {
  return DRV_SUCCESS;
}

unsigned int SetAcquisitionMode(int mode) {
  return DRV_SUCCESS;
}

unsigned int SetExposureTime(float time) {
  return DRV_SUCCESS;
}

unsigned int SetShutter(int typ, int mode, int closingtime, int openingtime) {
  return DRV_SUCCESS;
}

unsigned int SetImage(int hbin, int vbin, int hstart, int hend, int vstart, int vend) {
  return DRV_SUCCESS;
}

unsigned int Initialize(char * dir) {
  return DRV_SUCCESS;
}


