/*******************************************************************
*  File: tuya_BASE_UTILITIES_error_code.h
*  Author: auto generate by tuya code gen system
*  Description:this file defined the error code of tuya IOT 
*  Device OS module BASE_UTILITIES, you can change it manully
*  if needed
*  Copyright(C),2018-2020, tuya inc, www.tuya.comm
*******************************************************************/

#ifndef TUYA_BASE_UTILITIES_ERROR_CODE_H
#define TUYA_BASE_UTILITIES_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
            the error code marco define for module BASE_UTILITIES 
****************************************************************************/
#define OPRT_BASE_UTILITIES_CR_CJSON_ERR -10100
#define OPRT_BASE_UTILITIES_CJSON_PARSE_ERR -10101
#define OPRT_BASE_UTILITIES_CJSON_GET_ERR -10102
#define OPRT_BASE_UTILITIES_ERRCODE_MAX_CNT 3



#define BASE_UTILITIES_CALL_ERR_RETURN_VAL(func, y)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d\n", #func, rt);\
        return (y);\
    }\
}while(0)


#define BASE_UTILITIES_CALL_ERR_RETURN(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
       PR_ERR("call %s return %d\n", #func, rt);\
       return (rt);\
    }\
}while(0)


#define BASE_UTILITIES_CALL_ERR_GOTO(func, label)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d\n", #func, rt);\
        goto (label);\
    }\
}while(0)


#define BASE_UTILITIES_CALL_ERR_LOG(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt))\
        PR_ERR("call %s return %d\n", #func, rt);\
}while(0)


#define BASE_UTILITIES_CHECK_NULL_RETURN(x, y)\
do{\
    if (NULL == (x))\
        return (y);\
}while(0)


#ifdef __cplusplus
}
#endif
#endif
