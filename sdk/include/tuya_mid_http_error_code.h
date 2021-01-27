/*******************************************************************
*  File: tuya_MID_HTTP_error_code.h
*  Author: auto generate by tuya code gen system
*  Description:this file defined the error code of tuya IOT 
*  Device OS module MID_HTTP, you can change it manully
*  if needed
*  Copyright(C),2018-2020, tuya inc, www.tuya.comm
*******************************************************************/

#ifndef TUYA_MID_HTTP_ERROR_CODE_H
#define TUYA_MID_HTTP_ERROR_CODE_H

#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
            the error code marco define for module MID_HTTP 
****************************************************************************/
#define OPRT_MID_HTTP_BUF_NOT_ENOUGH -10900
#define OPRT_MID_HTTP_URL_PARAM_OUT_LIMIT -10901
#define OPRT_MID_HTTP_OS_ERROR -10902
#define OPRT_MID_HTTP_PR_REQ_ERROR -10903
#define OPRT_MID_HTTP_SD_REQ_ERROR -10904
#define OPRT_MID_HTTP_RD_ERROR -10905
#define OPRT_MID_HTTP_AD_HD_ERROR -10906
#define OPRT_MID_HTTP_GET_RESP_ERROR -10907
#define OPRT_MID_HTTP_AES_INIT_ERR -10908
#define OPRT_MID_HTTP_AES_OPEN_ERR -10909
#define OPRT_MID_HTTP_AES_SET_KEY_ERR -10910
#define OPRT_MID_HTTP_AES_ENCRYPT_ERR  -10911
#define OPRT_MID_HTTP_CR_HTTP_URL_H_ERR -10912
#define OPRT_MID_HTTP_HTTPS_HANDLE_FAIL -10913
#define OPRT_MID_HTTP_HTTPS_RESP_UNVALID -10914
#define OPRT_MID_HTTP_ERRCODE_MAX_CNT 15



#define MID_HTTP_CALL_ERR_RETURN_VAL(func, y)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d\n", #func, rt);\
        return (y);\
    }\
}while(0)


#define MID_HTTP_CALL_ERR_RETURN(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
       PR_ERR("call %s return %d\n", #func, rt);\
       return (rt);\
    }\
}while(0)


#define MID_HTTP_CALL_ERR_GOTO(func, label)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt)){\
        PR_ERR("call %s return %d\n", #func, rt);\
        goto (label);\
    }\
}while(0)


#define MID_HTTP_CALL_ERR_LOG(func)\
do{\
    rt = (func);\
    if (OPRT_OK != (rt))\
        PR_ERR("call %s return %d\n", #func, rt);\
}while(0)


#define MID_HTTP_CHECK_NULL_RETURN(x, y)\
do{\
    if (NULL == (x))\
        return (y);\
}while(0)


#ifdef __cplusplus
}
#endif
#endif
