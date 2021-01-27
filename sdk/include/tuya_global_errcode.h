/*******************************************************************
*  File: tuya_GLOBAL_error_code.h
*  Author: auto generate by tuya code gen system
*  Description:this file defined the error code of tuya IOT 
*  Device OS module GLOBAL, you can change it manully
*  if needed
*  Copyright(C),2018-2020, tuya inc, www.tuya.comm
*******************************************************************/

#ifndef TUYA_GLOBAL_ERROR_CODE_H
#define TUYA_GLOBAL_ERROR_CODE_H


#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
            the error code marco define for module GLOBAL 
****************************************************************************/
#define OPRT_GENERIC_OK -10000
#define OPRT_GENERIC_COM_ERROR -10001
#define OPRT_GENERIC_INVALID_PARM -10002
#define OPRT_GENERIC_MALLOC_FAILED -10003
#define OPRT_GENERIC_NOT_SUPPORTED -10004
#define OPRT_GENERIC_NETWORK_ERROR -10005
#define OPRT_GENERIC_NOT_FOUND -10006
#define OPRT_GLOBAL_ERRCODE_MAX_CNT 7



#define UNUSED(x) (void)(x)

#define TUYA_CALL_ERR_RETURN_VAL(func, y)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d", #func, rt);\
        return (y);\
    }\
}while(0)


#define TUYA_CALL_ERR_RETURN(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
       PR_ERR("call %s return %d", #func, rt);\
       return (rt);\
    }\
}while(0)


#define TUYA_CALL_ERR_GOTO(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d", #func, rt);\
        goto ERR_EXIT;\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt))\
        PR_ERR("call %s return %d", #func, rt);\
}while(0)


#define TUYA_CHECK_NULL_RETURN(x, y)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s is null!", #x);\
        return (y);\
    }\
}while(0)


#define TUYA_CHECK_NULL_GOTO(x)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s is null!", #x);\
        goto ERR_EXIT;\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG_SEQ_RETURN_VAL(func, y, ls_name, point)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d", #func, rt);\
        INSERT_ERROR_LOG_SEQ_DEC((ls_name), (point), rt);\
        return (y);\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG_SEQ_RETURN(func, ls_name, point)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d", #func, rt);\
        INSERT_ERROR_LOG_SEQ_DEC((ls_name), (point), rt);\
        return (rt);\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG_SEQ_GOTO(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d", #func, rt);\
        INSERT_ERROR_LOG_SEQ_DEC((ls_name), (point), rt);\
        goto ERR_EXIT;\
    }\
}while(0)


#define TUYA_CALL_ERR_LOG_SEQ(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)) {\
        PR_ERR("call %s return %d", #func, rt);\
        INSERT_ERROR_LOG_SEQ_DEC((ls_name), (point), rt);\
    }\
}while(0)


#define TUYA_CHECK_NULL_LOG_SEQ_RETURN(x, y, ls_name, point)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s is null!", #x);\
        INSERT_ERROR_LOG_SEQ_DEC((ls_name), (point), y);\
        return (y);\
    }\
}while(0)


#define TUYA_CHECK_NULL_LOG_SEQ_GOTO(x, ls_name, point)\
do{\
    if (NULL == (x)){\
        PR_ERR("%s is null!", #x);\
        INSERT_ERROR_LOG_SEQ_NULL((ls_name), (point));\
        goto ERR_EXIT;\
    }\
}while(0)


#ifdef __cplusplus
}
#endif
#endif
