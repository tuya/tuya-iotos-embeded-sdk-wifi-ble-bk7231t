/***********************************************************
*  File: smart_frame.h
*  Author: nzy
*  Date: 20170413
***********************************************************/
#ifndef _SMART_FRAME_H
#define _SMART_FRAME_H

#include "tuya_cloud_types.h"
#include "gw_intf.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct {
    DP_CMD_TYPE_E tp;
    ty_cJSON *cmd_js;
}SF_GW_DEV_CMD_S;

typedef struct {
    DP_CMD_TYPE_E tp;
    ty_cJSON *cmd_js;
}SF_GW_SCE_CMD_S;

typedef struct msg_data_s{
    UINT_T serno;
    UINT_T len;
    BYTE_T data[0];
}MSG_DATA_S;

OPERATE_RET smart_frame_init(VOID);

/* 接收并处理来自mqtt，tcp，本地定时的消息 */
OPERATE_RET sf_send_gw_dev_cmd(IN SF_GW_DEV_CMD_S *gd_cmd);
/* 接收并处理来自mqtt，tcp的场景操作命令 */
OPERATE_RET sf_send_scene_oper_cmd(IN SF_GW_SCE_CMD_S *sce_cmd);
/* 开启定时器上送本地dp */
VOID sf_start_sync_obj_dp(VOID);

/* 生成本地待上传的dp数据字符串 */
CHAR_T *sf_pack_local_obj_dp_data(IN DEV_CNTL_N_S *dev_cntl, IN CONST UINT_T max_len, IN CONST BOOL_T addDevId, OUT BOOL_T *p_all_data_packed, BOOL_T reset_flow_ctl, IN CONST BOOL_T is_lan);
VOID sf_dbg_print_dp_stats(VOID);


UINT_T sf_get_serial_no(VOID);
/* MQTT 31协议查询本地DP */
OPERATE_RET sf_respone_obj_dp_query(IN CONST ty_cJSON *pCidArr, IN CONST ty_cJSON *pDpIdArr);


OPERATE_RET sf_obj_dp_report_async(IN CONST CHAR_T *id,IN CONST CHAR_T *data, IN CONST BOOL_T force_send);
OPERATE_RET sf_obj_dp_query_response_async(IN CONST CHAR_T *id,IN CONST CHAR_T *data, IN CONST BOOL_T force_send);
OPERATE_RET sf_raw_dp_report_sync(  IN CONST CHAR_T *id,IN CONST BYTE_T dpid,
                                    IN CONST BYTE_T *data,IN CONST UINT_T len,
                                    IN CONST UINT_T timeout, IN CONST BOOL_T enable_auto_retrans);

OPERATE_RET sf_raw_dp_report_sync_with_time(	IN CONST CHAR_T *id,IN CONST BYTE_T dpid,
                                    IN CONST BYTE_T *data,IN CONST UINT_T len,
                                    IN CONST UINT_T timeout, IN CONST BOOL_T enable_auto_retrans,
                                    IN CONST CHAR_T *time_str);

/* stat */
OPERATE_RET sf_stat_dp_report_sync(IN CONST CHAR_T *dev_id, IN CONST CHAR_T *dp_str, IN CONST CHAR_T *time_str, IN CONST UINT_T timeout, IN CONST BOOL_T enable_auto_retrans);

/***********************************************************
*  Function: sf_dev_online_stat_update
*  Input: dev_id online
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET sf_dev_online_stat_update(IN CONST CHAR_T *dev_id,IN CONST BOOL_T online, IN CONST BOOL_T is_force);

/***********************************************************
*  Function: sf_sys_mag_hb_init
*  Input: hb_send_cb
*         if(NULL == hb_send_cb)
*             passive heartbeat pattern
*         else
*             active heartbeat pattern
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET sf_sys_mag_hb_init(IN CONST DEV_HEARTBEAT_SEND_CB hb_send_cb);

/***********************************************************
*  Function: sf_refresh_dev_hb
*  Input: dev_id
*         hb_timeout_time->if(0xffffffff == hb_timeout_time)
*                          then device online forever
*         max_resend_times-> max resend times
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET sf_set_dev_hb_timeout(IN CONST CHAR_T *dev_id,IN CONST TIME_S hb_timeout_time, IN CONST UINT_T max_resend_times);

/***********************************************************
*  Function: sf_refresh_dev_hb
*  Input: dev_id
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET sf_refresh_dev_hb(IN CONST CHAR_T *dev_id);


/***********************************************************
*  Function: sf_dp_data_get
*  Input:  CHAR_T *id, CHAR_T dp_id
*  Output: none
*  Return: DP_CNTL_S
***********************************************************/
DP_CNTL_S * sf_dp_data_get(IN CONST CHAR_T *id, IN CONST CHAR_T dp_id);


/***********************************************************
*  Function: sf_dp_data_is_equl
*  Input: dp_cmd
*  Output: none
*  Return: bool
***********************************************************/
BOOL_T sf_dp_data_is_equl(IN ty_cJSON *dp_cmd);


OPERATE_RET sf_send_grp_oper_cmd(IN SF_GW_DEV_CMD_S *grp_cmd);
/***********************************************************
*  Function: sf_dp_data_composition
*  Input: dp_cmd
*  Output: none
*  Return: bool
***********************************************************/
OPERATE_RET sf_dp_data_composition(IN CONST CHAR_T *dev_id,IN CONST TY_OBJ_DP_S *dp_data,\
                                                   IN CONST UINT_T cnt,OUT CHAR_T **data,OUT CHAR_T **time);

/***********************************************************
*  Function: sf_special_dp_register_cb
*  Input: handler
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
typedef OPERATE_RET (*sf_special_dp_cb)(IN CONST UINT16_T dpid, IN CONST ty_cJSON *dp_obj);
OPERATE_RET sf_special_dp_register_cb(IN sf_special_dp_cb handler);

#ifdef __cplusplus
}
#endif
#endif

