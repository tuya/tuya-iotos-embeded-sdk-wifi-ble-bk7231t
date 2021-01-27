/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors:   
 * @file name: user_timer.h
 * @Description: set timer include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-22 20:01:00
 * @LastEditTime: 2019-06-28 18:37:26
 */


#ifndef __USER_TIMER_H__
#define __USER_TIMER_H__


#include "light_types.h"
#include "soc_timer.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/**
 * @brief: user hardware time start
 * @param {IN UINT_T cycle -> hardware timer period, unit:us}
 * @param {IN VOID* callback -> callback proc}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserHWTimerStart(IN UINT_T uiCycleUs, IN VOID* pCallback);

/**
 * @brief: user hardware time stop
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserHWTimerStop(VOID);

/**
 * @berief: user software time start
 * @param {IN UCHAR_T ucTimerID -> software timer ID}
 * @param {IN UINT_T uiCyclyeMs -> timerout time,unit:ms} 
 * @param {IN VOID* pCallback -> timerout handler} 
 * @attention: soft time must can reload in anytime.
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserSWTimerStart(IN UCHAR_T ucTimerID, IN UINT_T uiCyclyeMs, IN VOID* pCallback);

/**
 * @berief: user software time stop
 * @param {IN UCHAR_T ucTimerID -> software timer_id}
 * @return: OPERATE_LIGHT
 * @retval: none
 */
OPERATE_LIGHT opUserSWTimerStop(IN UCHAR_T ucTimerID);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __USER_TIMER_H__ */
