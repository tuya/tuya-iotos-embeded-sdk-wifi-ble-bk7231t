/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: soc_pwm.h
 * @Description: soc PWM include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-24 20:42:29
 * @LastEditTime: 2019-08-28 21:50:23
 */

#ifndef __SOC_PWM_H__
#define __SOC_PWM_H__


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "light_types.h"

/**
 * @brief: PWM SOC Init
 * @param {IN USHORT_T usFrequency --> PWM frequency,unit:HZ, attention: range£º500 ~ 20KHz }
 * @param {IN USHORT_T usPositiveDutyCycle --> PWM Init Duty(positive duty),range:0 ~ 1000}
 * @param {IN UCHAR_T ucChannelNum --> PWM channel num}
 * @param {IN UCHART_T *pGpioList --> PWM gpio set}
 * @retval: OPERATE_RET
 */
OPERATE_RET opSocPwmInit(IN USHORT_T usFrequency, IN USHORT_T usPositiveDutyCycle, IN UCHAR_T ucChannelNum, IN UCHAR_T *pGpioList,IN BOOL_T bCCTFlag);

/**
 * @brief: PWM SOC SET Duty
 * @param {IN UCHAR_T ucChannel -> pwm send out channel num}
 * @param {IN USHORT_T usDuty   -> pwm send duty,rang: 0~1000}
 * @attention usChannel --> according to the initlize order
 * @retval: none
 */
VOID vSocPwmSetDuty(IN UCHAR_T ucChannel, IN USHORT_T usDuty);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __SOC_PWM_H__ */
