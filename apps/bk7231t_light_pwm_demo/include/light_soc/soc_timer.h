/*
 * @Author: wuls
 * @email: wulsu@tuya.com
 * @LastEditors:
 * @file name: soc_timer.h
 * @Description: soc timer proc include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-05-06 10:00:26
 * @LastEditTime: 2019-08-28 16:15:05
 */

#ifndef __SOC_TIMER_H__
#define __SOC_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "light_types.h"

/**
 * @brief: SOC hardware time start
 * @param {IN UINT_T uiCycleUs -> cycle time,unit:us}
 * @param {IN VOID* pCallback -> callback}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opSocHWTimerStart(IN UINT_T uiCycleUs, IN VOID* pCallback);

/**
 * @brief: SOC hardware time stop
 * @param {none} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opSocHWTimerStop(VOID);

/**
 * @brief: SOC software time start
 * @param {IN UCHAR_T ucTimerID -> timer id}
 * @param {IN UINT_T uiCycleMs -> cycle time,unit:ms}
 * @param {IN VOID* pCallback -> callback}
 * @attention: ucTimerID must < SOFTWARE_TIMER_MAX(10)
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opSocSWTimerStart(IN UCHAR_T ucTimerID, IN UINT_T uiCycleMs, IN VOID* pCallback);

/**
 * @brief: SOC software time stop
 * @param {IN UCHAR_T ucTimerID -> timer id} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opSocSWTimerStop(IN UCHAR_T ucTimerID);

/**
 * @brief: Check if the timer is running
 * @param {IN UCHAR_T ucTimerID -> timer_id}
 * @return: BOOL_T: true: running, false : not running
 */
BOOL_T bSocSWTimerStartCheck(IN UCHAR_T ucTimerID);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __SOC_TIMER_H__ */
