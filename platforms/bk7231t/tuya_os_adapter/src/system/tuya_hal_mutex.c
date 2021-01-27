/***********************************************************
*  File: uni_mutex.c
*  Author: nzy
*  Date: 120427
***********************************************************/
#define _UNI_MUTEX_GLOBAL
#include "FreeRTOS.h"

#include "task.h"
#include "semphr.h"
#include "tuya_hal_mutex.h"
#include "tuya_hal_system_internal.h"
#include "../errors_compat.h"
#define FALSE    0
#define TRUE     (!FALSE)
/***********************************************************
*************************micro define***********************
***********************************************************/
typedef xSemaphoreHandle THRD_MUTEX;

typedef struct
{
    THRD_MUTEX mutex;
}MUTEX_MANAGE,*P_MUTEX_MANAGE;

/***********************************************************
*************************variable define********************
***********************************************************/

/***********************************************************
*************************function define********************
***********************************************************/

/***********************************************************
*  Function: CreateMutexAndInit 创建一个互斥量并初始化
*  Input: none
*  Output: pMutexHandle->新建的互斥量句柄
*  Return: OPERATE_RET
*  Date: 120427
***********************************************************/
int tuya_hal_mutex_create_init(MUTEX_HANDLE *pMutexHandle)
{
    if(!pMutexHandle)
        return OPRT_INVALID_PARM;
    
    P_MUTEX_MANAGE pMutexManage;
    pMutexManage = (P_MUTEX_MANAGE)tuya_hal_internal_malloc(sizeof(MUTEX_MANAGE));
    if(!(pMutexManage))
        return OPRT_MALLOC_FAILED;
    
#if defined(CREATE_RECURSION_MUTEX)
    pMutexManage->mutex = xSemaphoreCreateRecursiveMutex();
#else
    pMutexManage->mutex = xSemaphoreCreateMutex();
#endif
    if(NULL == pMutexManage->mutex) {
        return OPRT_INIT_MUTEX_FAILED;
    }

    *pMutexHandle = (MUTEX_HANDLE)pMutexManage;

    return OPRT_OK;
}

/***********************************************************
*  Function: MutexLock 加锁
*  Input: mutexHandle->互斥量句柄
*  Output: none
*  Return: OPERATE_RET
*  Date: 120427
***********************************************************/
int tuya_hal_mutex_lock(const MUTEX_HANDLE mutexHandle)
{
    if(!mutexHandle)
        return OPRT_INVALID_PARM;

    P_MUTEX_MANAGE pMutexManage;
    pMutexManage = (P_MUTEX_MANAGE)mutexHandle;
    
    BaseType_t ret;
    ret = xSemaphoreTake(pMutexManage->mutex, portMAX_DELAY);
    if(pdTRUE != ret) {
        return OPRT_MUTEX_LOCK_FAILED;
    }

    return OPRT_OK;
}

/***********************************************************
*  Function: MutexUnLock 解锁
*  Input: mutexHandle->互斥量句柄
*  Output: none
*  Return: OPERATE_RET
*  Date: 120427
***********************************************************/
int tuya_hal_mutex_unlock(const MUTEX_HANDLE mutexHandle)
{
    if(!mutexHandle)
        return OPRT_INVALID_PARM;
    
    P_MUTEX_MANAGE pMutexManage;
    pMutexManage = (P_MUTEX_MANAGE)mutexHandle;
    
    BaseType_t ret;
    if(FALSE == tuya_hal_system_isrstatus()) {
        ret = xSemaphoreGive(pMutexManage->mutex);
    }else {
        signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        ret = xSemaphoreGiveFromISR(pMutexManage->mutex,\
                                    &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
    if(pdTRUE != ret) {
        return OPRT_MUTEX_UNLOCK_FAILED;
    }


    return OPRT_OK;
}

/***********************************************************
*  Function: ReleaseMutexManage 释放互斥锁管理结构资源
*  Input: mutexHandle->互斥量句柄
*  Output: none
*  Return: OPERATE_RET
*  Date: 120427
***********************************************************/
int tuya_hal_mutex_release(const MUTEX_HANDLE mutexHandle)
{
    if(!mutexHandle)
        return OPRT_INVALID_PARM;

    P_MUTEX_MANAGE pMutexManage;
    pMutexManage = (P_MUTEX_MANAGE)mutexHandle;
    
    vSemaphoreDelete(pMutexManage->mutex);

    tuya_hal_internal_free(mutexHandle);

    return OPRT_OK;
}


