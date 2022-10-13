/**
 * @file    pi_error.h
 * @brief   defines PI errors
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#ifndef PHYSIK_INSTRUMENTE_ERROR_H
#define PHYSIK_INSTRUMENTE_ERROR_H

/***** Physik_Instrumente *****************************************************/
/**
 * @namespace Physik_Instrumente
 * @brief     namespace for Physik Instrumente hardware
 *
 */
namespace Physik_Instrumente {

  /***** Physik_Instrumente::PIError ******************************************/
  /**
   * @class PIError
   * @brief 
   */
  class PIError {
    const int PI_CNTR_NO_ERROR=0;
    const int PI_CNTR_PARAM_SYNTAX=1;
    const int PI_CNTR_UNKNOWN_COMMAND=2;
    const int PI_CNTR_COMMAND_TOO_LONG=3;
    const int PI_CNTR_SCAN_ERROR=4;
    const int PI_CNTR_MOVE_WITHOUT_REF_OR_NO_SERVO=5;
    const int PI_CNTR_INVALID_SGA_PARAM=6;
    const int PI_CNTR_POS_OUT_OF_LIMITS=7;
    const int PI_CNTR_VEL_OUT_OF_LIMITS=8;
    const int PI_CNTR_SET_PIVOT_NOT_POSSIBLE=9;
    const int PI_CNTR_STOP=10;
    const int PI_CNTR_SST_OR_SCAN_RANGE=11;
    const int PI_CNTR_INVALID_SCAN_AXIS=12;
    const int PI_CNTR_INVALID_NAV_PARAM=13;
    const int PI_CNTR_INVALID_ANALOG_INPUT=14;
    const int PI_CNTR_INVALID_AXIS_IDENTIFIER=15;
    const int PI_CNTR_INVALID_STAGE_NAME=16;
    const int PI_CNTR_PARAMETER_OUT_OF_RANGE=17;
    const int PI_CNTR_INVALID_MACRO_NAME=18;
    const int PI_CNTR_MACRO_RECORD=19;
    const int PI_CNTR_MACRO_NOT_FOUND=20;
    const int PI_CNTR_AXIS_HAS_NO_BRAKE=21;
    const int PI_CNTR_DOUBLE_AXIS=22;
    const int PI_CNTR_ILLEGAL_AXIS=23;
    const int PI_CNTR_PARAM_NR=24;
    const int PI_CNTR_INVALID_REAL_NR=25;
    const int PI_CNTR_MISSING_PARAM=26;
    const int PI_CNTR_SOFT_LIMIT_OUT_OF_RANGE=27;
    const int PI_CNTR_NO_MANUAL_PAD=28;
    const int PI_CNTR_NO_JUMP=29;
    const int PI_CNTR_INVALID_JUMP=30;
  };
  /***** Physik_Instrumente::PIError ******************************************/
}
/***** Physik_Instrumente *****************************************************/
#endif
