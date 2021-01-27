/**
 * @File: app_switch.h
 * @Author: caojq
 * @Last Modified time: 2019-10-31
 * @Description: 电量统计平台免开发方案优化代码
 */

#ifndef  __APP_SWITCH_H__
#define  __APP_SWITCH_H__

#include "tuya_cloud_types.h"
#include "tuya_cloud_error_code.h"
#include "tuya_key.h"
#include "tuya_led.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_cloud_wifi_defs.h"

#ifdef  _APP_SWITCH_GLOBAL
    #define  __APP_SWITCH_EXT
#else
    #define  __APP_SWITCH_EXT extern
#endif

#define DPID_NOT_EXIST (-1)

//IO配置定义
#ifndef __IO_TYPE_CONFIG__
#define __IO_TYPE_CONFIG__
typedef enum 
{
    IO_DRIVE_LEVEL_HIGH,        // 高电平有效
    IO_DRIVE_LEVEL_LOW,         // 低电平有效
    IO_DRIVE_LEVEL_NOT_EXIST    // 该IO不存在
}IO_DRIVE_TYPE;
typedef struct
{
    IO_DRIVE_TYPE type; // 有效电平类型
    UCHAR_T pin;  // 引脚号
}IO_CONFIG;
#endif

/***********************************************************
*************************variable define********************
************************************************************/
typedef enum{
    INIT_CH_OPEN,  //上电默认通电
    INIT_CH_CLOSE, //上电默认断电
    INIT_CH_MEM    //上电后断电记忆
}INIT_CH_STAT;

typedef struct
{
    IO_CONFIG       relay;          // 继电器
    KEY_USER_DEF_S       button;         // 控制按键
    IO_CONFIG       led;            // 状态指示灯
    LED_HANDLE      led_handle;     // 状态指示灯句柄
    UINT_T             dpid;           // 该通道绑定的dpid
    UINT_T            cd_dpid;        // 该通道绑定的倒计时dpid 小于0表示不存在
    INT_T             cd_sec;         // 通道倒计时 -1 停止
    BOOL_T            stat;           // 通道状态 TRUE - 有效; FALSE - 无效
    INIT_CH_STAT    init_ch_stat;
}CTRL_CHANNEL_CONFIG;

typedef enum{
    WFL_OFF,    //wifi灯常灭
    WFL_ON,     //wifi灯常开
    WFL_DIR_RL,  //wifi灯指示继电器
    WFL_FLASH_VERY_FAST,
    WFL_FLASH_FAST,
    WFL_FLASH_SLOW,
    WFL_ENTER_PT,
    WFL_END_PT
}WFL_STAT;

typedef struct
{
    IO_CONFIG               wfl_io;     //wifi灯GPIO口
    LED_HANDLE	            wfl_handle; //wifi灯句柄
    WFL_STAT                wfl_cs;     //wifi灯连接时指示状态
    WFL_STAT                wfl_ucs;    //wifi灯长时间未连接时指示状态
    UCHAR_T           press_time;     //长按触发重置时间
    UCHAR_T           wcm_mode;
}WIFI_LED;

typedef struct{
    IO_CONFIG               tled;   //总继电器指示灯
    KEY_USER_DEF_S               tbt;    //总按键
    LED_HANDLE              tled_handle;
}TOTAL_CH;
// HW_TABLE结构体类型
// 抽象描述硬件配置
typedef struct
{
    UCHAR_T                   channel_num;
    TOTAL_CH                tch;            //总控通道
    WIFI_LED                wf_led;         //wifi指示灯
    CTRL_CHANNEL_CONFIG     *channels;			// 通道列表 *!* 不要重新指向其他位置
}HW_TABLE;

//事件类型定义
typedef enum{
    CHAN_EVENT_CLOSE= 0,   //通道关闭+上报+记忆
    CHAN_EVENT_OPEN,    //通道打开+上报+记忆
    CHAN_EVENT_TOGGLE   //通道取反+上报+记忆
}CHAN_EVENT_TYPE;

typedef enum{
    KEY_ALL_PRIOR_CLOSE = 0,//优先全关
    KEY_ALL_PRIOR_OPEN,//优先全开
    KEY_SINGLE_TOGGLE//控制单个取反
}KEY_TYPE;

typedef enum{
    CTRL_SW_CLOSE = 0,      //通道关闭
    CTRL_SW_OPEN,    //通道打开
    CTRL_SW_TOGGLE,     //通道取反
    CTRL_SW_STAT    //根据通道之前状态控制通道
}CTRL_SW;


typedef enum{
    APP_SW_MODE_NORMAL,     //开关正常工作模式
    APP_SW_MODE_PRODTEST    //开关产测模式
}APP_SW_MODE;

/***********************************************************
*   Function: deal_dp_proc
*   Input:    root:app下发数据结构体
*   Output:   VOID
*   Return:   VOID
*   Notice:   处理app下发数据
***********************************************************/
__APP_SWITCH_EXT \
VOID deal_dp_proc(IN CONST TY_OBJ_DP_S *root);

/***********************************************************
*   Function: judge_any_sw
*   Input:    on_or_off：按键总状态
*   Output:   VOID
*   Return:   VOID
*   Notice:   判断总开关状态
***********************************************************/
__APP_SWITCH_EXT \
BOOL_T judge_any_sw(IN BOOL_T on_or_off);

/***********************************************************
*   Function: set_pt_key_en
*   Input:    if_en:按键使能
*   Output:   VOID
*   Return:   VOID
*   Notice:   设置按键使能
***********************************************************/
__APP_SWITCH_EXT \
VOID set_pt_key_en(IN BOOL_T if_en);          //允许产测时使用按键

/***********************************************************
*   Function: app_switch_init
*   Input:    mode:开关模式
*   Output:   VOID
*   Return:   VOID
*   Notice:   按键初始化
***********************************************************/
__APP_SWITCH_EXT \
OPERATE_RET app_switch_init(IN APP_SW_MODE mode);//放到flash初始化之后

/***********************************************************
*   Function: set_wfl_state
*   Input:    VOID
*   Output:   wfl_stat：wifi指示灯状态
*   Return:   VOID
*   Notice:   设置产测标识
***********************************************************/
__APP_SWITCH_EXT \
VOID set_wfl_state(IN WFL_STAT wfl_stat);//设定WIFI灯状态

/***********************************************************
*   Function: save_pt_end_flag
*   Input:    VOID
*   Output:   state：产测标识
*   Return:   OPERATE_RET
*   Notice:   设置产测标识
***********************************************************/
__APP_SWITCH_EXT \
OPERATE_RET save_pt_end_flag(IN INT_T state);

/***********************************************************
*   Function: get_pt_end_flag
*   Input:    VOID
*   Output:   state：产测标识
*   Return:   OPERATE_RET
*   Notice:   获取产测标识
***********************************************************/
__APP_SWITCH_EXT \
OPERATE_RET get_pt_end_flag(OUT INT_T *state);

/***********************************************************
*   Function: reset_power_stat
*   Input:    VOID
*   Output:   VOID
*   Return:   VOID
*   Notice:   清空继电器存储状态
***********************************************************/
__APP_SWITCH_EXT \
VOID reset_power_stat(VOID);

/***********************************************************
*   Function: hw_get_wifi_mode
*   Input:    VOID
*   Output:   VOID
*   Return:   GW_WF_CFG_MTHD_SEL:wifi模式
*   Notice:   获取wifi模式
***********************************************************/
__APP_SWITCH_EXT \
GW_WF_CFG_MTHD_SEL hw_get_wifi_mode(VOID);

/***********************************************************
*   Function: hw_wifi_led_status
*   Input:    wifi_stat:wifi状态
*   Output:   VOID
*   Return:   VOID
*   Notice:   指示灯状态显示
***********************************************************/
__APP_SWITCH_EXT \
VOID hw_wifi_led_status(GW_WIFI_NW_STAT_E wifi_stat);

/***********************************************************
*   Function: ctrl_switch_state
*   Input:    channel_num：通道，CTRL_SW：
*   Output:   VOID
*   Return:   VOID
*   Notice:   按键通道控制
***********************************************************/
__APP_SWITCH_EXT \
VOID ctrl_switch_state(IN UCHAR_T channel_num,IN CTRL_SW ctrl_sw);

#endif


