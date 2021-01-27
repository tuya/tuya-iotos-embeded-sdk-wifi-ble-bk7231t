/*******************************************************************
*  File: tuya_OS_ADAPTER_error_code.h
*  Author: auto generate by tuya code gen system
*  Date: 2019-08-14
*  Description:this file defined the error code of tuya IOT 
*  Device OS module OS_ADAPTER, you can change it manully
*  if needed
*  Copyright(C),2018-2020, tuya inc, www.tuya.comm
*******************************************************************/

#ifndef TUYA_OS_ADAPTER_ERROR_CODE_H
#define TUYA_OS_ADAPTER_ERROR_CODE_H

#include "tuya_hal_system.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_LOG_OUTPUT(fmt,...) \
	do{\
		char buf[1024] = {0};\
		snprintf(buf, 1024, fmt, ##__VA_ARGS__);\
		tuya_hal_output_log(buf);\
		}while(0)


/****************************************************************************
            the error code marco define for module OS_ADAPTER 
****************************************************************************/
#define OPRT_OS_ADAPTER_INIT_MUTEX_ATTR_FAILED -100
#define OPRT_OS_ADAPTER_SET_MUTEX_ATTR_FAILED -101
#define OPRT_OS_ADAPTER_DESTROY_MUTEX_ATTR_FAILED -102
#define OPRT_OS_ADAPTER_INIT_MUTEX_FAILED -103
#define OPRT_OS_ADAPTER_MUTEX_LOCK_FAILED -104
#define OPRT_OS_ADAPTER_MUTEX_TRYLOCK_FAILED  -105
#define OPRT_OS_ADAPTER_MUTEX_LOCK_BUSY -106
#define OPRT_OS_ADAPTER_MUTEX_UNLOCK_FAILED -107
#define OPRT_OS_ADAPTER_MUTEX_RELEASE_FAILED -108
#define OPRT_OS_ADAPTER_CR_MUTEX_ERR -109
#define OPRT_OS_ADAPTER_MEM_PARTITION_EMPTY -110
#define OPRT_OS_ADAPTER_MEM_PARTITION_FULL -111
#define OPRT_OS_ADAPTER_MEM_PARTITION_NOT_FOUND -112
#define OPRT_OS_ADAPTER_INIT_SEM_FAILED -113
#define OPRT_OS_ADAPTER_WAIT_SEM_FAILED -114
#define OPRT_OS_ADAPTER_POST_SEM_FAILED -115
#define OPRT_OS_ADAPTER_THRD_STA_UNVALID -116
#define OPRT_OS_ADAPTER_THRD_CR_FAILED -117
#define OPRT_OS_ADAPTER_THRD_JOIN_FAILED -118
#define OPRT_OS_ADAPTER_THRD_SELF_CAN_NOT_JOIN -119
#define OPRT_OS_ADAPTER_ERRCODE_MAX_CNT 20



#define OS_ADAPTER_CALL_ERR_RETURN_VAL(func, y)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        HAL_LOG_OUTPUT("[%s:%d] call %s return %d\n", __FILE__, __LINE__, #func, rt);\
        return (y);\
    }\
}while(0)


#define OS_ADAPTER_CALL_ERR_RETURN(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
       HAL_LOG_OUTPUT("[%s:%d] call %s return %d\n", __FILE__, __LINE__, #func, rt);\
       return (rt);\
    }\
}while(0)


#define OS_ADAPTER_CALL_ERR_GOTO(func, label)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        HAL_LOG_OUTPUT("[%s:%d] call %s return %d\n", __FILE__, __LINE__, #func, rt);\
        goto (label);\
    }\
}while(0)


#define OS_ADAPTER_CALL_ERR_LOG(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt))\
        HAL_LOG_OUTPUT("[%s:%d] call %s return %d\n", __FILE__, __LINE__, #func, rt);\
}while(0)


#define OS_ADAPTER_CHECK_NULL_RETURN(x, y)\
do{\
    if (NULL == (x))\
        return (y);\
}while(0)


#ifdef __cplusplus
}
#endif
#endif
