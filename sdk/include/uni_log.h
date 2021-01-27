/*
uni_log.h
Copyright(C),2018-2020, 涂鸦科技 www.tuya.comm
*/
#ifndef _UNI_LOG_H
#define _UNI_LOG_H

#include "tuya_cloud_types.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _UNI_LOG_GLOBAL
    #define _UNI_LOG_EXT 
#else
    #define _UNI_LOG_EXT extern
#endif

/* tuya sdk definition of log level */
typedef INT_T LOG_LEVEL;
#define TY_LOG_LEVEL_ERR       0  // 错误信息，程序正常运行不应发生的信息 
#define TY_LOG_LEVEL_WARN      1  // 警告信息
#define TY_LOG_LEVEL_NOTICE    2  // 需要注意的信息
#define TY_LOG_LEVEL_INFO      3  // 通知信息
#define TY_LOG_LEVEL_DEBUG     4  // 程序运行调试信息，RELEASE版本中删除
#define TY_LOG_LEVEL_TRACE     5  // 程序运行路径信息，RELEASE版本中删除

#if defined(TUYA_IOT_DEBUG) && (TUYA_IOT_DEBUG==1)
#define TY_DEAULT_LOG_LEVEL    TY_LOG_LEVEL_DEBUG
#else
#define TY_DEAULT_LOG_LEVEL    TY_LOG_LEVEL_INFO
#endif

#define DEF_LOG_BUF_LEN 	1024
#define LOG_LEVEL_MIN		0
#define LOG_LEVEL_MAX		5

#define SAK_PRINT_LOG(module,level,fmt, ...) \
do \
{ \
    PrintLog(module,level, \
             __FILE__,__LINE__,(const char*)__func__, \
             fmt,##__VA_ARGS__); \
}while(0)

// 全局日志输出宏
#ifdef __ANDROID__

#define PR_TRACE(fmt, ...)  __android_log_print(ANDROID_LOG_DEBUG, "tuya_iot_sdk", fmt, ##__VA_ARGS__)
#define PR_TRACE_ENTER() PR_TRACE("enter [%s]", (PCHAR_T)__func__)
#define PR_TRACE_LEAVE() PR_TRACE("leave [%s]", (PCHAR_T)__func__)
#define PR_DEBUG(fmt, ...)  __android_log_print(ANDROID_LOG_DEBUG, "tuya_iot_sdk", fmt, ##__VA_ARGS__)
#define PR_ERR(fmt, ...)    __android_log_print(ANDROID_LOG_ERROR, "tuya_iot_sdk", fmt, ##__VA_ARGS__)
#define PR_WARN(fmt, ...)   __android_log_print(ANDROID_LOG_WARN, "tuya_iot_sdk", fmt, ##__VA_ARGS__)
#define PR_NOTICE(fmt, ...) __android_log_print(ANDROID_LOG_VERBOSE, "tuya_iot_sdk", fmt, ##__VA_ARGS__)
#define PR_INFO(fmt, ...)   __android_log_print(ANDROID_LOG_INFO, "tuya_iot_sdk", fmt, ##__VA_ARGS__)

#else // not __ANDROID_

#define PR_TRACE(fmt, ...) SAK_PRINT_LOG(NULL, TY_LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#define PR_TRACE_ENTER() PR_TRACE("enter [%s]", (const char*)__func__)
#define PR_TRACE_LEAVE() PR_TRACE("leave [%s]", (const char*)__func__)
#define PR_DEBUG(fmt, ...) SAK_PRINT_LOG(NULL,TY_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define PR_ERR(fmt, ...) SAK_PRINT_LOG(NULL,TY_LOG_LEVEL_ERR, fmt, ##__VA_ARGS__)
#define PR_WARN(fmt, ...) SAK_PRINT_LOG(NULL,TY_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define PR_NOTICE(fmt, ...) SAK_PRINT_LOG(NULL,TY_LOG_LEVEL_NOTICE, fmt, ##__VA_ARGS__)
#define PR_INFO(fmt, ...) SAK_PRINT_LOG(NULL,TY_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)

#endif // if __ANDROID_

#define SET_PR_DEBUG_LEVEL(level) SetLogManageAttr(level)
// 基于模块级别的日志输出宏
#define MPR_TRACE(module,fmt, ...) SAK_PRINT_LOG(module,TY_LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#define MPR_DEBUG(module,fmt, ...) SAK_PRINT_LOG(module,TY_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define MPR_ERR(module,fmt, ...) SAK_PRINT_LOG(module,TY_LOG_LEVEL_ERR, fmt, ##__VA_ARGS__)
#define MPR_WARN(module,fmt, ...) SAK_PRINT_LOG(module,TY_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define MPR_NOTICE(module,fmt, ...) SAK_PRINT_LOG(module,TY_LOG_LEVEL_NOTICE, fmt, ##__VA_ARGS__)
#define MPR_INFO(module,fmt, ...) SAK_PRINT_LOG(module,TY_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)

// 直接输出
#if defined(TUYA_IOT_DEBUG) && (TUYA_IOT_DEBUG==1)
#define PR_DEBUG_RAW(fmt, ...) PrintLogRaw(fmt, ##__VA_ARGS__)
#else
#define PR_DEBUG_RAW(fmt, ...) do { } while( 0 )
#endif

typedef VOID (*LOG_OUTPUT)(IN CONST      CHAR_T *str);
/***********************************************************
*************************variable define********************
***********************************************************/

/***********************************************************
*************************function define********************
***********************************************************/
_UNI_LOG_EXT \
OPERATE_RET CreateLogManageAndInit(IN CONST LOG_LEVEL curLogLevel,IN CONST INT_T buf_len,\
                                              IN CONST LOG_OUTPUT output);

_UNI_LOG_EXT \
OPERATE_RET AddOutputTerm(IN CONST CHAR_T *name,IN CONST LOG_OUTPUT term);

_UNI_LOG_EXT \
VOID DelOutputTerm(IN CONST CHAR_T *name);

_UNI_LOG_EXT \
OPERATE_RET SetLogManageAttr(IN CONST LOG_LEVEL curLogLevel);

_UNI_LOG_EXT \
OPERATE_RET SetLogManagemtime(BOOL_T if_ms_level);

_UNI_LOG_EXT \
OPERATE_RET GetLogManageAttr(OUT LOG_LEVEL *pCurLogLevel);

_UNI_LOG_EXT \
OPERATE_RET AddModuleLevel(IN CONST PCHAR_T pModuleName,\
                                  IN CONST LOG_LEVEL logLevel);

_UNI_LOG_EXT \
OPERATE_RET SetModuleLevel(IN CONST PCHAR_T pModuleName,\
                                  LOG_LEVEL log_level);

_UNI_LOG_EXT \
OPERATE_RET GetModuleLevel(IN CONST PCHAR_T pModuleName,\
                                  OUT LOG_LEVEL *pLogLevel);

_UNI_LOG_EXT \
OPERATE_RET DeleteModuleLevel(IN CONST PCHAR_T pModuleName);

int PrintLog(const char* pModuleName,
                        const LOG_LEVEL logLevel,
                        const char* pFile, const int line,
                        const char* pFunc,
                        const char* pFmt,...);

_UNI_LOG_EXT \
OPERATE_RET PrintLogRaw(IN CONST PCHAR_T pFmt,...);

_UNI_LOG_EXT \
VOID ReleaseLogManage(VOID);


void tuya_debug_hex_dump(CHAR_T *title, UINT8_T width, UINT8_T *buf, UINT16_T size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


