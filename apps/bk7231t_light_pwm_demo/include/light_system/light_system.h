/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: light_control.h
 * @Description: light control include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-26 13:55:40
 * @LastEditTime: 2019-05-21 10:13:39
 */

#ifndef __LIHGT_SYSTEM_H__
#define __LIHGT_SYSTEM_H__


#include "light_types.h"
#include "tuya_device.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#define DPID_SWITCH          20     /// switch_led
#define DPID_MODE            21     /// work_mode
#define DPID_BRIGHT          22     /// bright_value
#define DPID_TEMPR           23     /// temp_value
#define DPID_COLOR           24     /// colour_data
#define DPID_SCENE           25     /// scene_data
#define DPID_COUNTDOWN       26     /// countdown
#define DPID_MUSIC           27     /// music_data
#define DPID_CONTROL         28


#define DEFAULT_CONFIG "{Jsonver:1.1.0,module:WB3S,cmod:rgbcw,dmod:0,cwtype:0,onoffmode:0,pmemory:1,title20:0,defcolor:c,defbright:100,deftemp:100,cwmaxp:100,brightmin:10,brightmax:100,colormin:10,colormax:100,wfcfg:spcl,rstmode:0,rstnum:3,rstcor:c,rstbr:50,rsttemp:100,pwmhz:1000,r_pin:9,r_lv:1,g_pin:24,g_lv:1,b_pin:26,b_lv:1,c_pin:6,c_lv:1,w_pin:8,w_lv:1,}"


/**
 * @brief: wifi fast initlize process
 * @param {none} 
 * @attention: this partion can't operate kv flash 
                and other wifi system service
 * @attention: called by pre_device_init()
 * @retval: none
 */
VOID vLightSysPreDeviceinit(VOID);


/**
 * @brief: wifi normal initlize process
 * @param {none}
 * @attention: called by app_init()
 * @retval: none
 */
VOID vLightSysAppInit(VOID);

/**
 * @brief: device init
 * @param {none} 
 * @attention: called by device_init()
 * @retval: none
 */
OPERATE_LIGHT opLightSysDeviceInit(VOID);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __LIHGT_SYSTEM_H__ */

