/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors:   
 * @file name:  soc_gpio.h
 * @Description: soc gpio include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-16 16:46:43
 * @LastEditTime: 2019-10-21 14:27:20
 */
#ifndef __SOC_GPIO_H__
#define __SOC_GPIO_H__

#include "light_types.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * @brief: SOC general pin Reset proc
 * @param {UCHAR_T ucPin -> reset pin}
 * @retval: none
 */
VOID vSocPinReset(UCHAR_T ucPin);

/**
 * @brief: SOC i2c ctrl set proc
 * @param {UCHAR_t ucPin -> set pin}
 * @retval: none
 */
VOID vSocPinSet(UCHAR_T ucPin);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __SOC_GPIO_H__ */
