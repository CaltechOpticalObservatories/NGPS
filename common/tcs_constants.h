/**
 * @file    tcs_constants.h
 * @brief   return values from the real TCS
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef TCS_CONSTANTS_H
#define TCS_CONSTANTS_H

// these are return values to commands (except for ?MOTION)
//
const int TCS_SUCCESS              =  0;
const int TCS_UNRECOGNIZED_COMMAND = -1;
const int TCS_INVALID_PARAMETER    = -2;
const int TCS_UNABLE_TO_EXECUTE    = -3;
const int TCS_HOST_UNAVAILABLE     = -4;
const int TCS_UNDEFINED            = 9999;

// human-readable strings to match the above
//
inline static const char TCS_SUCCESS_STR[] = "success";
inline static const char TCS_UNRECOGNIZED_COMMAND_STR[] = "unrecognized";
inline static const char TCS_INVALID_PARAMETER_STR[] = "bad param";
inline static const char TCS_UNABLE_TO_EXECUTE_STR[] = "cannot exectute";
inline static const char TCS_HOST_UNAVAILABLE_STR[] = "unavailable";
inline static const char TCS_UNDEFINED_STR[] = "undefined";

// these are return values for the ?MOTION command
//
const int TCS_MOTION_STOPPED       =  0;
const int TCS_MOTION_SLEWING       =  1;
const int TCS_MOTION_OFFSETTING    =  2;
const int TCS_MOTION_TRACKING      =  3;
const int TCS_MOTION_SETTLING      = -1;
const int TCS_MOTION_UNKNOWN       = -2;

// human-readable strings to match the above
//
inline static const char TCS_MOTION_STOPPED_STR[] = "stopped";
inline static const char TCS_MOTION_SLEWING_STR[] = "slewing";
inline static const char TCS_MOTION_OFFSETTING_STR[] = "offsetting";
inline static const char TCS_MOTION_TRACKING_STR[] = "tracking";
inline static const char TCS_MOTION_SETTLING_STR[] = "settling";
inline static const char TCS_MOTION_UNKNOWN_STR[] = "unknown";
#endif
