/**
 * @File: app_dltj.h 
 * @Author: caojq
 * @Last Modified time: 2020-07-29
 * @Description: 电量统计上层应用
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

#define DP_ELE 17 //增加电量上报
#define DP_CURRENT 18 //电流上报
#define DP_POWER 19 //功率上报
#define DP_VOLTAGE 20 //电压上报
//以下可选择上报或不上报
#define DP_PTRSLT 21 //产测结果，0为失败，1为成功，2为未产测
#define DP_VREF 22 //电压系数
#define DP_IREF 23 //电流系数
#define DP_PREF 24 //功率系数
#define DP_EREF 25 //电量系数

#define DP_FAULT 26 

/* 220V 产测电压、电流、功率 */
#define V_DEF_220 2200
#define I_DEF_220 392
#define P_DEF_220 864

/* 120V 产测电压、电流、功率 */
#define V_DEF_120 1200
#define I_DEF_120 833
#define P_DEF_120 1000

/* hlw8012芯片，电流采样电阻1mR，电压采样电阻1KR/2MR,220V产测环境
   下的校准系数 */
#define V_COE_HLW 1680
#define I_COE_HLW 34586
#define P_COE_HLW 98363
#define E_COE_HLW 212

/* bl0937芯片，电流采样电阻1mR，电压采样电阻1KR/2MR,220V产测环境
   下的校准系数 */
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
    UCHAR_T fault_dpid;    //故障告警dpid
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
 * DESCRIPTION:    电量统计初始化
 * INPUT:          mode：电量统计设备模式：正常模式和产测模式
 * OUTPUT:         none
 * RETURN:         OPERATE_RET:初始化状态，返回数值型状态值
 * OTHERS:         bl0937芯片初始化/电量统计参数上报线程
 * HISTORY:        2020-03-04
 *******************************************************************************/
_APP_DLTJ_EXT \
OPERATE_RET app_dltj_init(IN APP_DLTJ_MODE mode);

/*********************************************************************************
 * FUNCTION:       report_over_curr
 * DESCRIPTION:    上报过流事件
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         过流数据上报
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

