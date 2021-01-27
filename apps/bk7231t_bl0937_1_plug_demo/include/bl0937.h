/**
 * @File: bl0937.h 
 * @Author: caojq 
 * @Last Modified time: 2020-07-20 
 * @Description: bl0937电量统计芯片驱动
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
#define D_ERR_MODE                  0x00        //错误提示模式
#define D_NORMAL_MODE               0x10        //正常工作模式
#define D_CAL_START_MODE            0x21        //校正模式，启动
#define D_CAL_END_MODE              0x23        //校正模式，完成
//--------------------------------------------------------------------------------------------

#ifndef __IO_TYPE_CONFIG__
#define __IO_TYPE_CONFIG__
typedef enum {
    IO_DRIVE_LEVEL_HIGH,        // 高电平有效
    IO_DRIVE_LEVEL_LOW,         // 低电平有效
    IO_DRIVE_LEVEL_NOT_EXIST    // 该IO不存在
}IO_DRIVE_TYPE;

typedef struct{
    IO_DRIVE_TYPE type;         // 有效电平类型
    UCHAR_T pin;                // 引脚号
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
 * DESCRIPTION:    bl0937芯片引脚配置和参数设置
 * INPUT:          dltj：电量统计相关参数结构体
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         bl0937芯片引脚配置和参数设置
 * HISTORY:        2020-03-04
 *******************************************************************************/
_BL0937_EXT \
VOID bl0937_init(DLTJ_CONFIG *dltj);

/*********************************************************************************
 * FUNCTION:       ele_cnt_init
 * DESCRIPTION:    电量脉冲计数初始化
 * INPUT:          mode：计量芯片的模式(计量模式)
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         计量芯片初始化/硬件定时器初始化/中断初始化/脉冲计数初始化
 * HISTORY:        2020-03-04
 *******************************************************************************/
_BL0937_EXT \
OPERATE_RET ele_cnt_init(INT_T mode);

 /*********************************************************************************
  * FUNCTION:       report_coe_data
  * DESCRIPTION:    上报电量统计校准参数及产测结果位
  * INPUT:          none
  * OUTPUT:         none
  * RETURN:         none
  * OTHERS:         电量统计校准参数和产测结果
  * HISTORY:        2020-03-04
  *******************************************************************************/
_BL0937_EXT \
OPERATE_RET report_coe_data(VOID);

/*********************************************************************************
 * FUNCTION:       get_ele_par
 * DESCRIPTION:    获取电量实时参数
 * INPUT:          p/v/i:功率/电压/电流
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         获取电量芯片实时参数
 * HISTORY:        2020-03-04
 *******************************************************************************/
_BL0937_EXT \
VOID get_ele_par(OUT UINT_T *P,OUT UINT_T *V,OUT UINT_T *I);

/*********************************************************************************
 * FUNCTION:       get_ele
 * DESCRIPTION:    获取增加电量参数
 * INPUT:          E:增加电量值
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         获取电量芯片增加电量参数
 * HISTORY:        2020-03-04
 *******************************************************************************/
_BL0937_EXT \
VOID get_ele(OUT UINT_T *E);

#ifdef __cplusplus
}
#endif
#endif

