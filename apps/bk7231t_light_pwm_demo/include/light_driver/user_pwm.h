/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: user_pwm.h
 * @Description: pwm send out include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-22 20:01:00
 * @LastEditTime: 2019-08-13 11:23:10
 */


#ifndef __USER_PWM_H__
#define __USER_PWM_H__


#include "light_types.h"


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * @brief pwm init data structure
 * pwm send data structure
 */
typedef struct 
{
    USHORT_T usFreq;            ///< PWM Frequency
    USHORT_T usDuty;            ///< PWM Init duty
    UCHAR_T ucList[5];          ///< GPIO List 
    UCHAR_T ucChannelNum;       ///< GPIO List length
    BOOL_T bPolarity;           ///< PWM output polarity
    UCHAR_T ucCtrlPin;          ///< CTRL pin parm
    BOOL_T  bCtrlLevel;         ///< Enable level
    BOOL_T  bCCTFlag;           ///< CCT drive mode flag
}USER_PWM_INIT_T;

/**
 * @brief pwm send data structure
 * pwm send data structure
 */
typedef struct 
{
    USHORT_T usRed;         ///< R value,rang from 0 to 1000
    USHORT_T usGreen;       ///< G value,rang from 0 to 1000
    USHORT_T usBlue;        ///< B value,rang from 0 to 1000
    USHORT_T usCold;        ///< C value,rang from 0 to 1000
    USHORT_T usWarm;        ///< W value,rang from 0 to 1000
}USER_PWM_COLOR_T;

/**
 * @brief: user pwm init
 * @param {IN USER_PWM_INIT_T *pPwmInit -> Pwm INIT Parm
 *          usFreq -> PWM Frequency unit Hz, range:100~20000Hz
 *          usDuty -> PWM Init duty unit 0.1%, range: 0~1000 
 *          ucList[5] -> RGBCW GPIO list
 *          ucChannelNum -> channel num, rang 1~5 way
 *          bPolarity -> PWM output polarity
 *              TRUE -> positive
 *              FAlse -> negative
 *          ucCtrlPin -> ctrl pin
 *          bCtrlLevel -> ctrl pin level}
 * @attention ucList[5] parm set
 *              List order is always RGBCW !
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserPWMInit(IN USER_PWM_INIT_T *pPwmInit);

/**
 * @brief: pwm send data out
 * @param {IN USER_PWM_COLOR_T *pColorData -> PWM send data 
 *          usRed   -> R send duty,rang from 0 to 1000,unit 0.1%
 *          usGreen -> G send duty,rang from 0 to 1000,unit 0.1%
 *          usBlue  -> B send duty,rang from 0 to 1000,unit 0.1%
 *          usCold  -> C send duty,rang from 0 to 1000,unit 0.1%
 *          usWarm  -> W send duty,rang from 0 to 1000,unit 0.1%}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserPWMSetRGBCW(IN USER_PWM_COLOR_T *pColorData);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __USER_PWM_H__ */
