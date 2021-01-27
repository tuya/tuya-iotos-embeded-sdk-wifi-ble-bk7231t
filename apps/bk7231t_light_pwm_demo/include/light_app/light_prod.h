/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: light_prod.h
 * @Description: light production test include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-26 13:55:40
 * @LastEditTime: 2019-05-08 16:17:44
 */

#ifndef __LIHGT_PROD_H__
#define __LIHGT_PROD_H__


#include "light_types.h"
#include "light_cfg.h"
#include "light_set_color.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/// Weak signal breath display change time(unit ms)
#define WEAK_SINGAL_BREATHR_TIME_INTERVAL           1500
/// Weak signal on/off display change time(unit ms)
#define WEAK_SINGAL_ONOFF_TIME_INTERVAL             250
/// Weak signal on/off blink cnt
#define WEAK_SINGAL_ONOFF_CNT                       2


/// Production detected signal weak judge value
#define AUZ_TEST_WEAK_SIGNAL                        -60 
/// Production unauthorized display on/off time(unit ms)
#define AUZ_TEST_FAIL_TIME_INTERVAL                 3000    


/// Productoin oeverall function single led blink time(unit ms)
#define PORD_CHECK_ONOF_TIMER_INTERVAL              1000
/// Production oberall fuction test time(unit ms)
#define PROD_CHECK_TIMER_INTERVAL                   (2*60*1000)


/// Production aging restart signle led blink timer(unit ms)
#define PROD_AGING_START_ONOFF_TIMER_INTERVAL       500
/// Production aging restart loop cnt()
#define PROD_AGING_START_LOOP_CNT                   5


/// Production repeat test signle led up/down timer(unit ms)
#define PROD_REPEAT_ONOFF_TIMER_INTERVAL        1000

/**
 * @brief: Production factory step
 */
typedef enum{
    PROD_CHECK = 0,
    PROD_AGING,
    PROD_REPEAT
}PROD_STEP_E;


/**
 * @brief Light way type enum 
 *          1~5 ways
 */
typedef enum {
    PROD_LIGHT_C = 1,
    PROD_LIGHT_CW,
    PROD_LIGHT_RGB,
    PROD_LIGHT_RGBC,
    PROD_LIGHT_RGBCW,
    PROD_LIGHT_MAX,
}PROD_LIGHT_WAY_E;

/**
 * @brief Light control drive mode enum
 *          BRIGHT_MODE_CW  -> CW drive by pwm totally
 *          BRIGHT_MODE_CCT -> C value is bright setting essentially , w is the scale of C&W .
 *                          the light will send warm, when w is set by zero.
 */
typedef enum {
    PROD_MODE_CW = 0,
    PROD_MODE_CCT,
    PROD_MODE_MAX,
}PORD_BRIGHT_MODE_E;

#pragma pack(4)
typedef struct {
    PROD_LIGHT_WAY_E    eLightWay;
    PORD_BRIGHT_MODE_E  eBrightMode;
    UCHAR_T             ucLimitCWMax;   /* CW limit Max value */
    UCHAR_T             ucLimitCWMin;   /* CW limit Min value */
    UCHAR_T             ucLimitRGBMax;  /* RGB limit Max value */
    UCHAR_T             ucLimitRGBMin;  /* RGB limit Max value */

#if (LIGHT_CFG_PROD_DRIVER_NEED_INIT == 1)
    DRIVER_CONFIG_T     tDriveCfg;
#endif
}PROD_INIT_T;
#pragma pack()

/**
 * @brief: prodution test process init
 * @param {none}
 * @return: none
 * @retval: none
 */
OPERATE_RET opLightProdInit(VOID);

/**
 * @brief: prodution test process cb
 * @param {BOOL_T bAuthorized -> authorized flag}
 * @param {CHAR_T cSignalStrength -> singal strength}
 * @return: none
 * @retval: none
 */
VOID vProdTestCB(BOOL_T bAuthorized, CHAR_T cSignalStrength);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __LIHGT_PROD_H__ */
