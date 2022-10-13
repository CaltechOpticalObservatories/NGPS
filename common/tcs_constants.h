/**
 * @file    tcs_constants.h
 * @brief   
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef TCS_CONSTANTS_H
#define TCS_CONSTANTS_H
const int TCS_SUCCESS              =  0;
const int TCS_UNRECOGNIZED_COMMAND = -1;
const int TCS_INVALID_PARAMETER    = -2;
const int TCS_UNABLE_TO_EXECUTE    = -3;
const int TCS_HOST_UNAVAILABLE     = -4;
const int TCS_UNDEFINED            = 9999;

const int TCS_MOTION_STOPPED       =  0;
const int TCS_MOTION_SLEWING       =  1;
const int TCS_MOTION_OFSETTING     =  2;
const int TCS_MOTION_TRACKING      =  3;
const int TCS_MOTION_SETTLING      = -1;
const int TCS_MOTION_UNKNOWN       = -2;
#endif
