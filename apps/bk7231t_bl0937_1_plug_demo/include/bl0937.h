/**
 * @File: bl0937.h 
 * @Author: caojq 
 * @Last Modified time: 2020-07-20 
 * @Description: bl0937 power statistics chip driver
 */
#ifndef __BL0937_H__
#define __BL0937_H__

#include "tuya_cloud_types.h"
#include "tuya_cloud_error_code.h"

#ifdef __cplusplus
    extern "C" {
#endif

#ifdef _BL0937_GLOBAL
    #define _BL0937_EXT
#else
    #define _BL0937_EXT extern
#endif
//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
#define D_ERR_MODE                  0x00        //Error prompt mode
#define D_NORMAL_MODE               0x10        //normal working mode
#define D_CAL_START_MODE            0x21        //calibration mode, start
#define D_CAL_END_MODE              0x23        //Calibration mode, complete
//--------------------------------------------------------------------------------------------

#ifndef __IO_TYPE_CONFIG__
#define __IO_TYPE_CONFIG__
typedef enum {
    IO_DRIVE_LEVEL_HIGH,        // Active high
    IO_DRIVE_LEVEL_LOW,         // Active low
    IO_DRIVE_LEVEL_NOT_EXIST    // The IO does not exist
}IO_DRIVE_TYPE;

typedef struct{
    IO_DRIVE_TYPE type;         // Active level type
    UCHAR_T pin;                // pin number
}IO_CONFIG;
#endif

typedef struct{
    UCHAR_T dp_vcoe;
    UCHAR_T dp_icoe;
    UCHAR_T dp_pcoe;
    UCHAR_T dp_ecoe;
    UCHAR_T dp_pt_rslt;
    UCHAR_T epin;
    UCHAR_T ivpin;
    IO_CONFIG ivcpin;
    UINT_T v_ref;
    UINT_T  i_ref;
    UINT_T  p_ref;
    UINT_T  e_ref;
    UINT_T  v_def;
    UINT_T  i_def;
    UINT_T  p_def;
}DLTJ_CONFIG;

/***********************************************************
*************************function define********************
***********************************************************/

/*********************************************************************************
 * FUNCTION:       bl0937_init
 * DESCRIPTION:    bl0937 chip pin configuration and parameter settings
 * INPUT:          dltj: Structure of parameters related to electricity statistics
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         bl0937 chip pin configuration and parameter settings
 * HISTORY:        2020-03-04
 *******************************************************************************/
_BL0937_EXT \
VOID bl0937_init(DLTJ_CONFIG *dltj);

/*********************************************************************************
 * FUNCTION:       ele_cnt_init
 * DESCRIPTION:    Initialization of battery pulse count
 * INPUT:          mode: the mode of the metering chip (metering mode)
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Metering chip initialization/hardware timer initialization/interrupt initialization/pulse count initialization
 * HISTORY:        2020-03-04
 *******************************************************************************/
_BL0937_EXT \
OPERATE_RET ele_cnt_init(INT_T mode);

 /*********************************************************************************
  * FUNCTION:       report_coe_data
  * DESCRIPTION:    Report power statistics calibration parameters and production test result bits
  * INPUT:          none
  * OUTPUT:         none
  * RETURN:         none
  * OTHERS:         Gas statistics calibration parameters and production test results
  * HISTORY:        2020-03-04
  *******************************************************************************/
_BL0937_EXT \
OPERATE_RET report_coe_data(VOID);

/*********************************************************************************
 * FUNCTION:       get_ele_par
 * DESCRIPTION:    Get real-time power parameters
 * INPUT:          p/v/i: power/voltage/current
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Get real-time parameters of power chip
 * HISTORY:        2020-03-04
 *******************************************************************************/
_BL0937_EXT \
VOID get_ele_par(OUT UINT_T *P,OUT UINT_T *V,OUT UINT_T *I);

/*********************************************************************************
 * FUNCTION:       get_ele
 * DESCRIPTION:    Get the increase power parameter
 * INPUT:          E: increase the power value
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Get the power chip increase power parameters
 * HISTORY:        2020-03-04
 *******************************************************************************/
_BL0937_EXT \
VOID get_ele(OUT UINT_T *E);

#ifdef __cplusplus
}
#endif
#endif

