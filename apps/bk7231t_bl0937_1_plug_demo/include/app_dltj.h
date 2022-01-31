/**
 * @File: app_dltj.h 
 * @Author: caojq
 * @Last Modified time: 2020-07-29
 * @Description: Upper-layer application of electricity statistics
 *.
 */

#ifndef  __APP_DLTJ_H__
#define  __APP_DLTJ_H__
#include "bl0937.h"
#include "tuya_cloud_types.h"
#include "tuya_cloud_error_code.h"

#ifdef __cplusplus
extern "C" {
#endif
#ifdef _APP_DLTJ_GLOBAL
    #define _APP_DLTJ_EXT
#else
    #define _APP_DLTJ_EXT extern
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
#define _APP_DLTJ_DEBUG 0

#define DP_ELE 17 //Increase power report
#define DP_CURRENT 18 //Current report
#define DP_POWER 19 //Power report
#define DP_VOLTAGE 20 //Voltage report
//The following can choose to report or not to report
#define DP_PTRSLT 21 //Production test result, 0 is failure, 1 is success, 2 is no production test
#define DP_VREF 22 //Voltage coefficient
#define DP_IREF 23 //current coefficient
#define DP_PREF 24 //power factor
#define DP_EREF 25 //Electricity factor

#define DP_FAULT 26 

/* 220V Production test voltage, current, power */
#define V_DEF_220 2200
#define I_DEF_220 392
#define P_DEF_220 864

/* 120V Production test voltage, current, power */
#define V_DEF_120 1200
#define I_DEF_120 833
#define P_DEF_120 1000

/* hlw8012 chip, current sampling resistance 1mR, voltage sampling resistance 1KR/2MR, 220V production test environment 
    calibration factor under */
#define V_COE_HLW 1680
#define I_COE_HLW 34586
#define P_COE_HLW 98363
#define E_COE_HLW 212

/* bl0937 chip, current sampling resistance 1mR, voltage sampling resistance 1KR/2MR, 220V production test environment
    calibration factor under */
#define V_COE_BL 586
#define I_COE_BL 31281
#define P_COE_BL 16192
#define E_COE_BL 1400

/***********************************************************
*************************variable define********************
***********************************************************/
typedef enum{
    APP_DLTJ_NORMAL,
    APP_DLTJ_PRODTEST
}APP_DLTJ_MODE;

typedef struct{
    UCHAR_T edpid;
    UCHAR_T idpid;
    UCHAR_T pdpid;
    UCHAR_T vdpid;
    UCHAR_T fault_dpid;    //fault alarm dpid
}APP_DLTJ_DPID;

typedef struct{
    BOOL_T            if_have;
    UINT_T            over_curr;
    APP_DLTJ_DPID      dp_info;
    DLTJ_CONFIG        drv_cfg;
}APP_DLTJ_CFG;

/***********************************************************
*************************function define********************
***********************************************************/
/*********************************************************************************
 * FUNCTION:       app_dltj_init
 * DESCRIPTION:    Power statistics initialization
 * INPUT:          mode: power statistics device mode: normal mode and production test mode
 * OUTPUT:         none
 * RETURN:         OPERATE_RET: Initialize state, return numeric state value
 * OTHERS:         bl0937 chip initialization / power statistics parameter reporting thread
 * HISTORY:        2020-03-04
 *******************************************************************************/
_APP_DLTJ_EXT \
OPERATE_RET app_dltj_init(IN APP_DLTJ_MODE mode);

/*********************************************************************************
 * FUNCTION:       report_over_curr
 * DESCRIPTION:    Report overcurrent events
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Overcurrent data reporting
 * HISTORY:        2020-03-04
 *******************************************************************************/
_APP_DLTJ_EXT \
VOID report_over_curr(VOID);

_APP_DLTJ_EXT \
VOID reset_clear_temp_ele(VOID);

_APP_DLTJ_EXT \
VOID reset_clear_ele(VOID);

_APP_DLTJ_EXT \
VOID switch_ele_dp_query(VOID);

_APP_DLTJ_EXT \
VOID hw_report_all_dp_status(VOID);

#ifdef __cplusplus
}
#endif
#endif

