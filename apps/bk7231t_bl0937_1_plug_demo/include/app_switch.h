/**
 * @File: app_switch.h
 * @Author: caojq
 * @Last Modified time: 2019-10-31
 * @Description: Development-free solution optimization code for electricity statistics platform
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

//IO configuration definition
#ifndef __IO_TYPE_CONFIG__
#define __IO_TYPE_CONFIG__
typedef enum 
{
    IO_DRIVE_LEVEL_HIGH,        // Active high
    IO_DRIVE_LEVEL_LOW,         // Active low
    IO_DRIVE_LEVEL_NOT_EXIST    // The IO does not exist
}IO_DRIVE_TYPE;
typedef struct
{
    IO_DRIVE_TYPE type; // Active level type
    UCHAR_T pin;  // pin number
}IO_CONFIG;
#endif

/***********************************************************
*************************variable define********************
************************************************************/
typedef enum{
    INIT_CH_OPEN,  //Power on by default
    INIT_CH_CLOSE, //Power-on default power-off
    INIT_CH_MEM    //Power-off memory after power-on
}INIT_CH_STAT;

typedef struct
{
    IO_CONFIG       relay;          // relay
    KEY_USER_DEF_S       button;         // control buttons
    IO_CONFIG       led;            // Status Indicator
    LED_HANDLE      led_handle;     // Status light handle
    UINT_T             dpid;           // the dpid that the channel is bound to
    UINT_T            cd_dpid;        // The countdown dpid of the channel binding is less than 0, indicating that it does not exist
    INT_T             cd_sec;         // Channel countdown -1 stop
    BOOL_T            stat;           // Channel status TRUE - valid; FALSE - invalid
    INIT_CH_STAT    init_ch_stat;
}CTRL_CHANNEL_CONFIG;

typedef enum{
    WFL_OFF,    //wifi light is always off
    WFL_ON,     //WiFi light is always on
    WFL_DIR_RL,  //wifi light indicating relay
    WFL_FLASH_VERY_FAST,
    WFL_FLASH_FAST,
    WFL_FLASH_SLOW,
    WFL_ENTER_PT,
    WFL_END_PT
}WFL_STAT;

typedef struct
{
    IO_CONFIG               wfl_io;     //wifi light GPIO port
    LED_HANDLE	            wfl_handle; //wifi light handle
    WFL_STAT                wfl_cs;     //wifi light indicates status when connected
    WFL_STAT                wfl_ucs;    //The wifi light indicates the status when not connected for a long time
    UCHAR_T           press_time;     //Long press to trigger reset time
    UCHAR_T           wcm_mode;
}WIFI_LED;

typedef struct{
    IO_CONFIG               tled;   //Main relay indicator
    KEY_USER_DEF_S               tbt;    //total key
    LED_HANDLE              tled_handle;
}TOTAL_CH;
// HW_TABLE structure type
// Abstract description of hardware configuration
typedef struct
{
    UCHAR_T                   channel_num;
    TOTAL_CH                tch;            //Master control channel
    WIFI_LED                wf_led;         //wifi indicator
    CTRL_CHANNEL_CONFIG     *channels;			// channel list *!* do not repoint elsewhere
}HW_TABLE;

//事件类型定义
typedef enum{
    CHAN_EVENT_CLOSE= 0,   //Channel close + report + memory
    CHAN_EVENT_OPEN,    //Channel open + report + memory
    CHAN_EVENT_TOGGLE   //Channel inversion + report + memory
}CHAN_EVENT_TYPE;

typedef enum{
    KEY_ALL_PRIOR_CLOSE = 0,//Priority all close
    KEY_ALL_PRIOR_OPEN,//Full open priority
    KEY_SINGLE_TOGGLE//Control a single negation
}KEY_TYPE;

typedef enum{
    CTRL_SW_CLOSE = 0,      //channel closed
    CTRL_SW_OPEN,    //channel open
    CTRL_SW_TOGGLE,     //Channel negation
    CTRL_SW_STAT    //Control the channel based on the previous state of the channel
}CTRL_SW;


typedef enum{
    APP_SW_MODE_NORMAL,     //Switch normal working mode
    APP_SW_MODE_PRODTEST    //Switch production test mode
}APP_SW_MODE;

/***********************************************************
*   Function: deal_dp_proc
*   Input:    root: app delivers data structure
*   Output:   VOID
*   Return:   VOID
*   Notice:   Process the data sent by the app
***********************************************************/
__APP_SWITCH_EXT \
VOID deal_dp_proc(IN CONST TY_OBJ_DP_S *root);

/***********************************************************
*   Function: judge_any_sw
*   Input:    on_or_off：Overall button status
*   Output:   VOID
*   Return:   VOID
*   Notice:   Judging the status of the main switch
***********************************************************/
__APP_SWITCH_EXT \
BOOL_T judge_any_sw(IN BOOL_T on_or_off);

/***********************************************************
*   Function: set_pt_key_en
*   Input:    if_en:key enable
*   Output:   VOID
*   Return:   VOID
*   Notice:   Set key enable
***********************************************************/
__APP_SWITCH_EXT \
VOID set_pt_key_en(IN BOOL_T if_en);          //Allow the use of buttons during production testing

/***********************************************************
*   Function: app_switch_init
*   Input:    mode:switch mode
*   Output:   VOID
*   Return:   VOID
*   Notice:   button initialization
***********************************************************/
__APP_SWITCH_EXT \
OPERATE_RET app_switch_init(IN APP_SW_MODE mode);//After flash initialization

/***********************************************************
*   Function: set_wfl_state
*   Input:    VOID
*   Output:   wfl_stat：wifi indicator status
*   Return:   VOID
*   Notice:   Set production test logo
***********************************************************/
__APP_SWITCH_EXT \
VOID set_wfl_state(IN WFL_STAT wfl_stat);//Set WIFI light status

/***********************************************************
*   Function: save_pt_end_flag
*   Input:    VOID
*   Output:   state：Production test identification
*   Return:   OPERATE_RET
*   Notice:   Set production test logo
***********************************************************/
__APP_SWITCH_EXT \
OPERATE_RET save_pt_end_flag(IN INT_T state);

/***********************************************************
*   Function: get_pt_end_flag
*   Input:    VOID
*   Output:   state：Production test identification
*   Return:   OPERATE_RET
*   Notice:   Get the production test logo
***********************************************************/
__APP_SWITCH_EXT \
OPERATE_RET get_pt_end_flag(OUT INT_T *state);

/***********************************************************
*   Function: reset_power_stat
*   Input:    VOID
*   Output:   VOID
*   Return:   VOID
*   Notice:   Clear relay memory state
***********************************************************/
__APP_SWITCH_EXT \
VOID reset_power_stat(VOID);

/***********************************************************
*   Function: hw_get_wifi_mode
*   Input:    VOID
*   Output:   VOID
*   Return:   GW_WF_CFG_MTHD_SEL: wifi mode
*   Notice:   get wifi mode
***********************************************************/
__APP_SWITCH_EXT \
GW_WF_CFG_MTHD_SEL hw_get_wifi_mode(VOID);

/***********************************************************
*   Function: hw_wifi_led_status
*   Input:    wifi_stat:wifi status
*   Output:   VOID
*   Return:   VOID
*   Notice:   Indicator status display
***********************************************************/
__APP_SWITCH_EXT \
VOID hw_wifi_led_status(GW_WIFI_NW_STAT_E wifi_stat);

/***********************************************************
*   Function: ctrl_switch_state
*   Input:    channel_num: channel, CTRL_SW:
*   Output:   VOID
*   Return:   VOID
*   Notice:   Key Channel Control
***********************************************************/
__APP_SWITCH_EXT \
VOID ctrl_switch_state(IN UCHAR_T channel_num,IN CTRL_SW ctrl_sw);

#endif


