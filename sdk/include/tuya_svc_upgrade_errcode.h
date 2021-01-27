/*******************************************************************
*  File: tuya_SVC_UPGRADE_error_code.h
*  Author: auto generate by tuya code gen system
*  Description:this file defined the error code of tuya IOT 
*  Device OS module SVC_UPGRADE, you can change it manully
*  if needed
*  Copyright(C),2018-2020, tuya inc, www.tuya.comm
*******************************************************************/

#ifndef TUYA_SVC_UPGRADE_ERROR_CODE_H
#define TUYA_SVC_UPGRADE_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
            the error code marco define for module SVC_UPGRADE 
****************************************************************************/
#define OPRT_SVC_UPGRADE_NO_FIRMWARE -11400
#define OPRT_SVC_UPGRADE_NO_VALID_FIRMWARE -11401
#define OPRT_SVC_UPGRADE_ERRCODE_MAX_CNT 2



#define SVC_UPGRADE_CALL_ERR_RETURN_VAL(func, y)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d\n", #func, rt);\
        return (y);\
    }\
}while(0)


#define SVC_UPGRADE_CALL_ERR_RETURN(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
       PR_ERR("call %s return %d\n", #func, rt);\
       return (rt);\
    }\
}while(0)


#define SVC_UPGRADE_CALL_ERR_GOTO(func, label)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d\n", #func, rt);\
        goto (label);\
    }\
}while(0)


#define SVC_UPGRADE_CALL_ERR_LOG(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt))\
        PR_ERR("call %s return %d\n", #func, rt);\
}while(0)


#define SVC_UPGRADE_CHECK_NULL_RETURN(x, y)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s is null!\n", #x);\
        return (y);\
    }\
}while(0)


#define SVC_UPGRADE_CHECK_NULL_GOTO(x, label)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s is null!\n", #x);\
        goto (label);\
    }\
}while(0)


#ifdef __cplusplus
}
#endif
#endif
