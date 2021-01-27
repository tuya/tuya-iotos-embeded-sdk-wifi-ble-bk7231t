/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: sm2135.c
 * @Description: SM2135 IIC driver program
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-16 11:44:21
 * @LastEditTime: 2019-08-28 19:38:13
 */

#include "user_timer.h"
#include "light_types.h"
#include "light_printf.h"

/**
 * @brief: user hardware time start
 * @param {IN UINT_T uiCycleUs -> hardware timer period, unit:us}
 * @param {IN VOID* pCallback -> callback proc}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserHWTimerStart(IN UINT_T uiCycleUs, IN VOID* pCallback)
{
    OPERATE_LIGHT opRet = -1;

    if(NULL == pCallback) {
        PR_ERR("user hardware time callback can't be null!");
        return LIGHT_INVALID_PARM;
    }

    opRet = opSocHWTimerStart(uiCycleUs, pCallback);
    if(opRet != LIGHT_OK) {
        PR_ERR("User hardware start error!");
        return opRet;
    }

    return LIGHT_OK;
}

/**
 * @brief: user hardware time stop
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserHWTimerStop(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opSocHWTimerStop();

    return opRet;
}

/**
 * @brief: user software time start
 * @param {IN UCHAR_T ucTimerID -> software timer ID}
 * @param {IN UINT_T uiCyclyeMs -> timerout time,unit:ms}
 * @param {IN VOID* pCallback -> timerout handler}
 * @attention: soft time must can reload in anytime.
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserSWTimerStart(IN UCHAR_T ucTimerID, IN UINT_T uiCyclyeMs, IN VOID* pCallback)
{
    OPERATE_LIGHT opRet = -1;

    if(NULL == pCallback) {
        PR_ERR("user software time callback can't be null!");
        return LIGHT_INVALID_PARM;
    }

    opRet = opSocSWTimerStart(ucTimerID, uiCyclyeMs, pCallback);
    if(opRet != LIGHT_OK) {
        PR_ERR("User software start error!");
        return opRet;
    }

    return LIGHT_OK;
}

/**
 * @brief: user software time stop
 * @param {IN UCHAR_T ucTimerID -> software timer_id}
 * @return: OPERATE_LIGHT
 * @retval: none
 */
OPERATE_LIGHT opUserSWTimerStop(IN UCHAR_T ucTimerID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opSocSWTimerStop(ucTimerID);
    if(opRet != LIGHT_OK) {
        PR_ERR("User software stop error!");
        return opRet;
    }

    return LIGHT_OK;
}

/**
 * @brief: user hardware time start
 * @param {IN UCHART_T ucTimerID -> software timer_id}
 * @retval: BOOL_T: true: running, false : not running
 */
BOOL_T bUserSWTimerCheckValid(IN UCHAR_T ucTimerID)
{
    return bSocSWTimerStartCheck(ucTimerID);
}


