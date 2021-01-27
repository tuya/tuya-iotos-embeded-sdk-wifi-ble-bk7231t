/***********************************************************
*  File: svc_online_log_seq.h.h
*  Author: nzy
*  Date: 20190422
***********************************************************/
#ifndef __LOG_SEQ_H
#define __LOG_SEQ_H

#include "tuya_cloud_types.h"
#include "tuya_os_adapter.h"


#ifdef __cplusplus
	extern "C" {
#endif


/***********************************************************
*************************micro define***********************
***********************************************************/
#define STR_DATA_MAX_LENGTH 16

typedef BYTE_T LS_DATA_TP_T; // log sequence data type
#define LDT_NULL 0
#define LDT_DEC 1
#define LDT_HEX 2
#define LDT_TIMESTAMP 3
#define LDT_STRING 4

typedef union {
    INT_T dec_data;
    UINT_T hex_data;
    TIME_T tm_data;
    CHAR_T str_data[STR_DATA_MAX_LENGTH+1];
}LOG_DA_TP_U;

typedef struct { // log sequence data structure
    BYTE_T seq_num; // log sequence num
    LS_DATA_TP_T ldt;
    LOG_DA_TP_U data;
}LS_DATA_S;

typedef BYTE_T LS_STAT_T;
#define LSS_INIT 0
#define LSS_RECORDING 1
#define LSS_LOCKED 2 // log_seq_t set locked status when record the error log sequence

//tuya-iot log seq name list
#define DEFAULT_NAME  "def"
#define MQTT_FLOW     "mqtt"
#define HTTP_FLOW     "http"
#define SMT_CFG       "smt_netcfg"
#define AP_CFG        "ap_netcfg"

//net_cfg log seq log_seq list
#define LOG_SEQ_TOKEN_NUM 1
#define LOG_SEQ_CFG_ERR_NUM 2
#define LOG_SEQ_CFG_TIME_NUM 3
#define LOG_SEQ_OTHER_START 5
    
// Calling when record the error log sequence
typedef int (*LOG_SEQ_UPLOAD_CB)(const char *p_log); 

// log sequence record format of string
// ec is error repeat count
// "name(15bytes)" " | " "mm-dd hh:mm:ss" " | " "erc:%d" " | " "[""%d" " | " "%3d:0x%x" " | " "%3d:%d""]"  
// for example:"eazymode | 02-01 10:58:20 | ec:20 | [1 | 2:0x12345678 | 3:1234 | 4:12-19 12:38:59]"
#define LS_DELIMITER " | "
#define LSR_STR_MAX LOG_SEQ_NAME_LEN + sizeof(LS_DELIMITER) + \
                    sizeof("mm-dd hh:mm:ss") + sizeof(LS_DELIMITER) + \
                    sizeof("ec:xxx") + sizeof(LS_DELIMITER) + \
                    sizeof("[]") + LOG_DEF_SIZE*(sizeof("xxx:")+STR_DATA_MAX_LENGTH) + 20 + 20

#define INSERT_LOG_SEQ_NULL(ls,seq) \
do{ \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_NULL; \
    log_seq_insert_log(ls,&ls_data); \
}while(0)

#define INSERT_LOG_SEQ_DEC(ls,seq,dec) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_DEC; \
    ls_data.data.dec_data = dec; \
    log_seq_insert_log(ls,&ls_data); \
}while(0)

#define INSERT_LOG_SEQ_HEX(ls,seq,hex) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_HEX; \
    ls_data.data.hex_data = hex; \
    log_seq_insert_log(ls,&ls_data); \
}while(0)

#define INSERT_LOG_SEQ_TM(ls,seq,tm) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_TIMESTAMP; \
    ls_data.data.tm_data = tm; \
    log_seq_insert_log(ls,&ls_data); \
}while(0)

//string max length is STR_DATA_MAX_LENGTH
#define INSERT_LOG_SEQ_STR(ls,seq,str) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_STRING; \
    memset(ls_data.data.str_data,0,SIZEOF(ls_data.data.str_data)); \
    UCHAR_T len = strlen(str) < STR_DATA_MAX_LENGTH ? strlen(str) : STR_DATA_MAX_LENGTH; \
    memcpy(ls_data.data.str_data,str,len); \
    log_seq_insert_log(ls,&ls_data); \
}while(0)


#define INSERT_ERROR_LOG_SEQ_NULL(ls,seq) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_NULL; \
    log_seq_insert_error_log(ls,&ls_data); \
}while(0)

#define INSERT_ERROR_LOG_SEQ_DEC(ls,seq,dec) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_DEC; \
    ls_data.data.dec_data = dec; \
    log_seq_insert_error_log(ls,&ls_data); \
}while(0)

#define INSERT_ERROR_LOG_SEQ_HEX(ls,seq,hex) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_HEX; \
    ls_data.data.hex_data = hex; \
    log_seq_insert_error_log(ls,&ls_data); \
}while(0)

#define INSERT_ERROR_LOG_SEQ_TM(ls,seq,tm) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_TIMESTAMP; \
    ls_data.data.tm_data = tm; \
    log_seq_insert_error_log(ls,&ls_data); \
}while(0)

//string max length is STR_DATA_MAX_LENGTH
#define INSERT_ERROR_LOG_SEQ_STR(ls,seq,str) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_STRING; \
    memset(ls_data.data.str_data,0,SIZEOF(ls_data.data.str_data)); \
    UCHAR_T len = strlen(str) < STR_DATA_MAX_LENGTH ? strlen(str) : STR_DATA_MAX_LENGTH; \
    memcpy(ls_data.data.str_data,str,len); \
    log_seq_insert_error_log(ls,&ls_data); \
}while(0)

#define INSERT_REPORT_LOG_SEQ_NULL(ls,seq) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_NULL; \
    log_seq_insert_report_log(ls,&ls_data); \
}while(0)

#define INSERT_REPORT_LOG_SEQ_DEC(ls,seq,dec) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_DEC; \
    ls_data.data.dec_data = dec; \
    log_seq_insert_report_log(ls,&ls_data); \
}while(0)

#define INSERT_REPORT_LOG_SEQ_HEX(ls,seq,hex) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_HEX; \
    ls_data.data.hex_data = hex; \
    log_seq_insert_report_log(ls,&ls_data); \
}while(0)

#define INSERT_REPORT_LOG_SEQ_TM(ls,seq,tm) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_TIMESTAMP; \
    ls_data.data.tm_data = tm; \
    log_seq_insert_report_log(ls,&ls_data); \
}while(0)

//string max length is STR_DATA_MAX_LENGTH
#define INSERT_REPORT_LOG_SEQ_STR(ls,seq,str) \
do{  \
    LS_DATA_S ls_data = {0}; \
    ls_data.seq_num = seq; \
    ls_data.ldt = LDT_STRING; \
    memset(ls_data.data.str_data,0,SIZEOF(ls_data.data.str_data)); \
    UCHAR_T len = strlen(str) < STR_DATA_MAX_LENGTH ? strlen(str) : STR_DATA_MAX_LENGTH; \
    memcpy(ls_data.data.str_data,str,len); \
    log_seq_insert_report_log(ls,&ls_data); \
}while(0)


/**
 * @brief 日志序释放接口，用于释放日志序对象
 * 
 * @param[in] ls_name: log seq name，一般为组件名
 *
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_del(const char *ls_name);

/**
 * @brief 日志序修改日志序名称
 * 
 * @param[in] ls_name: log seq name，一般为组件名
 *
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_change_name(const char *old_name, const char *new_name);

/**
 * @brief 日志序重置，清除所有的日志序，在设备重置时候使用
 * 
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_clean();

/**
 * @brief 日志序插入日志
 * 
 * @param[in] ls_name: log seq name，一般为组件名
 * @param[in] ls_data: 日志序数据
 *
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_insert_log(const char *ls_name, const LS_DATA_S *ls_data);

/**
 * @brief 日志序插入错误日志
 * 
 * @param[in] ls_name: log seq name，一般为组件名
 * @param[in] ls_data: 日志序数据
 *
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_insert_error_log(const char *ls_name, const LS_DATA_S *ls_data);

/**
 * @brief 日志序插入报告日志
 * 
 * @param[in] ls_name: log seq name，一般为组件名
 * @param[in] ls_data: 日志序数据
 *
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_insert_report_log(const char *ls_name, const LS_DATA_S *ls_data);

/**
 * @brief 日志序初始化接口
 * 
 * @param[in] log_seq_path: 日志序存储路径
 * @param[in] upload_cb：日志序上传cb函数，由online log模块注册提供
 *
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_init(const char *log_seq_path, LOG_SEQ_UPLOAD_CB upload_cb);

/**
 * @brief 上传客户日志序信息接口
 * 
 * @param[in] log: 需要上传的日志字符串
 *
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_upload_custom_log(const char *p_log);

/**
 * @brief 强制同步所有日志序
 * 
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_force_sync(void);

/**
 * @brief 获取日志序内容
 * 
 * @param[out] log_buff: 日志序缓冲区
 * @param[out] log_len: 日志序日志长度
 *
 * @return int: 0成功，非0，请参照tuya error code描述文档
 */
int log_seq_get_netcfg_log(char **log_buff, int *log_len);



#ifdef __cplusplus
}
#endif
#endif

