/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: soc_timer.c
 * @Description: soc timer proc
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-05-06 10:00:26
 * @LastEditTime: 2019-10-18 09:51:33
 */
#include "FreeRTOS.h"
#include "timers.h"
#include "uni_time_queue.h"
#include "light_types.h"
#include "light_printf.h"
#include "rtos_error.h"
#include "BkDriverTimer.h"
#include "light_tools.h"

#define SOFTWARE_TIMER_MAX      20

/// software timer handle
STATIC TimerHandle_t xSoftWareTimers[SOFTWARE_TIMER_MAX] = {NULL};

/**
 * @brief: SOC hardware time start
 * @param {IN UINT_T cycle_us -> cycle time,unit:us}
 * @param {IN VOID* callback -> callback}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opSocHWTimerStart(IN UINT_T uiCycleUs, IN VOID* pCallback)
{
    UINT_T uiReloadCnt = 0;
    INT_T iRet = 0;
    
    uiReloadCnt = uiCycleUs / 1000 ;        /* use time0,unit:ms */
    iRet = bk_timer_initialize(0, uiReloadCnt, pCallback);
    if(iRet != kNoErr) {
        return OPRT_COM_ERROR;
    }
    
    return OPRT_OK;
}

/**
 * @brief: SOC hardware time stop
 * @param {none} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opSocHWTimerStop(VOID)
{   
    INT_T iRet = 0;

    iRet = bk_timer_stop(0);
    return OPRT_OK;
}

/**
 * @berief: SOC software time start
 * @param {IN UCHAR_T timer_id -> timer id}
 * @param {IN UINT_T cycle_ms -> cycle time,unit:ms}
 * @param {IN VOID* callback -> callback}
 * @attention: timer_id must < SOFTWARE_TIMER_MAX(5)
 * @return: OPERATE_RET
 * @retval: none
 */
OPERATE_LIGHT opSocSWTimerStart(IN UCHAR_T ucTimerID, IN UINT_T uiCycleMs, IN VOID* pCallback)
{
    CHAR_T cTemp[4] = {0};
    
    if(ucTimerID >= SOFTWARE_TIMER_MAX) {
        PR_ERR("Software timer is overflow,max timer count is %d", SOFTWARE_TIMER_MAX);
        return OPRT_INVALID_PARM;
    }

    if(NULL == xSoftWareTimers[ucTimerID]) {
        vNum2Str(0, ucTimerID, 4, cTemp);
        xSoftWareTimers[ucTimerID] = xTimerCreate(cTemp, (uiCycleMs / portTICK_PERIOD_MS), pdFAIL, (VOID *) ucTimerID, (TimerHandle_t)pCallback);
        if(NULL == xSoftWareTimers[ucTimerID]) {
            PR_ERR("Create software time %d error", ucTimerID);
            return OPRT_COM_ERROR;
        }

        if(xTimerStart(xSoftWareTimers[ucTimerID], 0) != pdPASS) {
            PR_ERR("Start software timer %d error", ucTimerID);
            return OPRT_COM_ERROR;
        }

    } else {
        if(xTimerChangePeriod(xSoftWareTimers[ucTimerID], uiCycleMs / portTICK_PERIOD_MS, 0) != pdPASS) {
            PR_ERR("Start software timer %d error", ucTimerID);
            return OPRT_COM_ERROR;
        }
    }

    return OPRT_OK;
}

/**
 * @brief: SOC software time stop
 * @param {IN UCHAR_T timer_id -> timer id} 
 * @retval: OPERATE_RET
 */
OPERATE_LIGHT opSocSWTimerStop(IN UCHAR_T ucTimerID)
{
    if(ucTimerID >= SOFTWARE_TIMER_MAX) {
        PR_ERR("Software timer is overflow,max timer count is %d",SOFTWARE_TIMER_MAX);
        return OPRT_INVALID_PARM;
    }

    if(NULL == xSoftWareTimers[ucTimerID]) {
        return OPRT_OK;
    }
    
    if(xTimerIsTimerActive(xSoftWareTimers[ucTimerID]) != pdFAIL) {
        xTimerDelete(xSoftWareTimers[ucTimerID], 0);
        xSoftWareTimers[ucTimerID] = NULL;
    } else {
        if(xTimerStop(xSoftWareTimers[ucTimerID], 0) != pdPASS) {
            PR_ERR("Software timer stop error!");
            return OPRT_COM_ERROR;
        }
    }

    return OPRT_OK;
}

/**
 * @description: Check if the timer is running
 * @param {timer_id} timer to be check
 * @return: BOOL_T: true: running, false : not running
 */
BOOL_T bSocSWTimerStartCheck(IN UCHAR_T ucTimerID)
{
    return xTimerIsTimerActive(xSoftWareTimers[ucTimerID]);
}




