/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: light_set_color.h
 * @Description: light set color(choose drive mode)include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-26 13:55:40
 * @LastEditTime: 2020-03-06 11:36:54
 */

#ifndef __LIHGT_SET_COLOR_H__
#define __LIHGT_SET_COLOR_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "light_types.h"

#include "user_pwm.h"

/* enum need to define in json analysize & get file */
typedef enum {
    DRIVER_MODE_PWM = 0
}DRIVER_MODE_E;



typedef struct{
    DRIVER_MODE_E eMode;
    union{
        USER_PWM_INIT_T tPwmInit;
    } uConfig;
}DRIVER_CONFIG_T;

/**
 * @brief: Light drive init!
 * @param {DRIVER_CONFIG_T *pLightConfig -> init config structure}
 * @attention: DRIVER_MODE_E eMode -> has three mode：
                DRIVER_MODE_PWM = 0
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightDriveInit(IN DRIVER_CONFIG_T *pLightConfig);


/**
 * @brief: light send control data
 * @param {IN USHORT_T usRed} red color, range 0~1000
 * @param {IN USHORT_T usGreen} Green color, range 0~1000
 * @param {IN USHORT_T usBlue} Blue color, range 0~1000
 * @param {IN USHORT_T usCold} cold white color, range 0~1000
 * @param {IN USHORT_T usWarm} warm white color, range 0~1000
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightSetRGBCW(IN USHORT_T usRed, IN USHORT_T usGreen, IN USHORT_T usBlue, IN USHORT_T usCold, IN USHORT_T usWarm);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __LIHGT_SET_COLOR_H__ */
