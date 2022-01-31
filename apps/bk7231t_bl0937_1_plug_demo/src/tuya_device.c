/**
 * @File: tuya_device.c 
 * @Author: caojq 
 * @Last Modified time: 2020-07-29 
 * @Description: Power statistics single plug demo solution
 *
 */
#define _TUYA_DEVICE_GLOBAL

#include "tuya_device.h"
#include "tuya_iot_wifi_api.h"
#include "tuya_hal_system.h"
#include "tuya_iot_com_api.h"
#include "tuya_cloud_com_defs.h"
#include "gw_intf.h"
#include "mem_pool.h"
#include "uni_log.h"

#include "app_dltj.h"
#include "app_switch.h"
#include "gpio_test.h"

/***********************************************************
*************************variable define********************
************************************************************/
extern HW_TABLE g_hw_table;
extern APP_DLTJ_CFG g_dltj;
/***********************************************************
*************************function define********************
***********************************************************/
STATIC VOID get_free_heap(VOID)
{
    INT_T free_size = tuya_hal_system_getheapsize();
    PR_NOTICE("device_init is OK!free_size_heap:%d",free_size);
}
/***********************************************************
*   Function:  gpio_test
*   Input:     none
*   Output:    none
*   Return:    none
*   Notice:    gpio test
***********************************************************/
BOOL_T gpio_test(IN CONST CHAR_T *in, OUT CHAR_T *out)
{
    return gpio_test_all(in, out);
}

/***********************************************************
*   Function:  mf_user_callback
*   Input:     none
*   Output:    none
*   Return:    none
*   Notice:    Production test authorization callback function
***********************************************************/
VOID mf_user_callback(VOID)
{
    hw_reset_flash_data();
    return;
}

VOID prod_test(BOOL_T flag,CHAR_T rssi)
{
    OPERATE_RET op_ret;
    op_ret = app_switch_init(APP_SW_MODE_PRODTEST);
    if(op_ret != OPRT_OK) {
        return;
    }
    PR_NOTICE("have scaned the ssid, and enter product test.rssi value:%d",rssi);
    if(rssi < -60) {
        set_wfl_state(WFL_FLASH_VERY_FAST);
        PR_ERR("no authorization, product test err!");
        return;
    }
    if(FALSE == flag) {
        set_wfl_state(WFL_FLASH_VERY_FAST);
        PR_ERR("current rssi value is less than -60 dB, product test err!");
        return;
    }
    if(g_dltj.if_have){
        set_wfl_state(WFL_FLASH_SLOW);
        set_pt_key_en(FALSE);
        UCHAR_T i;
        for(i = 0;i < g_hw_table.channel_num;i++){
            ctrl_switch_state(i, CTRL_SW_OPEN);
        }
        tuya_hal_system_sleep(1500);
        op_ret = app_dltj_init(APP_DLTJ_PRODTEST);
        for(i = 0;i < g_hw_table.channel_num;i++){
            ctrl_switch_state(i, CTRL_SW_CLOSE);
        }
        set_pt_key_en(TRUE);
        if(op_ret != OPRT_OK) {
            set_wfl_state(WFL_OFF);
        }else{
            set_wfl_state(WFL_ON);
        }
    }
}

VOID app_init(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;
    INT_T pt_end = 0;

    app_cfg_set(hw_get_wifi_mode(),prod_test);
    PR_DEBUG("wifi_mode:%d",hw_get_wifi_mode());

    op_ret = get_prod_test_data(&pt_end);
    if((OPRT_OK == op_ret) && (1 == pt_end)){
        set_prod_ssid("tuya_mdev_test2");
        PR_NOTICE("need scan ssid: tuya_mdev_test2 to enter product test repeatedly.");
    } else {
        PR_NOTICE("need scan ssid: tuya_mdev_test1 to enter product test.");
    }
}

/***********************************************************
*   Function:  pre_device_init
*   Input:     none
*   Output:    none
*   Return:    none
*   Notice:    Printing Information Level and Firmware Basic Information
***********************************************************/
VOID pre_device_init(VOID)
{
    PR_DEBUG("%s",tuya_iot_get_sdk_info());
    PR_DEBUG("%s:%s",APP_BIN_NAME,DEV_SW_VERSION);
    PR_NOTICE("firmware compiled at %s %s", __DATE__, __TIME__);
    SetLogManageAttr(TY_LOG_LEVEL_INFO);
}

/***********************************************************
*   Function: status_changed_cb
*   Input:    status : wifi acticed state
*   Output:   none
*   Return:   none
*   Notice: wifi acticed state changed call back
***********************************************************/
VOID status_changed_cb(IN CONST GW_STATUS_E status)
{
    PR_NOTICE("status_changed_cb is status:%d",status);

    if(GW_NORMAL == status) {
        hw_report_all_dp_status();
    }else if(GW_RESET == status) {
        PR_NOTICE("status is GW_RESET");
    }
}

/***********************************************************
*   Function: upgrade_notify_cb
*   Input: 
*   Output: 
*   Return: 
***********************************************************/
VOID upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    PR_DEBUG("download  Finish");
    PR_DEBUG("download_result:%d", download_result);
}

/***********************************************************
*  Function: get_file_data_cb
*  Input: 
*  Output: 
*  Return: 
***********************************************************/
OPERATE_RET get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                     IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    PR_DEBUG("Rev File Data");
    PR_DEBUG("Total_len:%d ", total_len);
    PR_DEBUG("Offset:%d Len:%d", offset, len);

    return OPRT_OK;
}

/***********************************************************
*   Function: gw_ug_inform_cb
*   Input: 
*   Output: 
*   Return: 
***********************************************************/
VOID gw_ug_inform_cb(IN CONST FW_UG_S *fw)
{
    PR_DEBUG("Rev GW Upgrade Info");
    PR_DEBUG("fw->fw_url:%s", fw->fw_url);
   // PR_DEBUG("fw->fw_md5:%s", fw->fw_md5);
    PR_DEBUG("fw->sw_ver:%s", fw->sw_ver);
    PR_DEBUG("fw->file_size:%d", fw->file_size);

    tuya_iot_upgrade_gw(fw, get_file_data_cb, upgrade_notify_cb, NULL);
}

/***********************************************************
*   Function:  hw_reset_flash_data
*   Input:     none
*   Output:    none
*   Return:    OPERATE_RET
*   Notice:    GW_REMOTE_RESET_FACTORY by app panel control
***********************************************************/
VOID hw_reset_flash_data(VOID)
{
    reset_power_stat();
    reset_clear_temp_ele();
    reset_clear_ele();
}

/***********************************************************
*   Function:  gw_reset_cb
*   Input:     none
*   Output:    none
*   Return:    OPERATE_RET
*   Notice:    GW_REMOTE_RESET_FACTORY by app panel control
***********************************************************/
VOID gw_reset_cb(IN CONST GW_RESET_TYPE_E type)
{
    PR_DEBUG("gw_reset_cb type:%d",type);
    if(GW_REMOTE_RESET_FACTORY != type) {
        PR_DEBUG("type is GW_REMOTE_RESET_FACTORY");
        return;
    }

    hw_reset_flash_data();
    return;
}

/***********************************************************
*   Function:  dev_obj_dp_cb
*   Input:     *dp :the data need to analyse point
*   Output:    return frame
*   Return:    none
*   Notice:    dp data analysis for object data, send return frame
***********************************************************/
VOID dev_obj_dp_cb(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    PR_DEBUG("dp->cid:%s dp->dps_cnt:%d",dp->cid,dp->dps_cnt);
    UCHAR_T i = 0;

    for(i = 0;i < dp->dps_cnt;i++) {
        deal_dp_proc(&(dp->dps[i]));
    }
}

/***********************************************************
*   Function:  dev_raw_dp_cb
*   Input:     *dp :the data need to analyse point
*   Output:    return frame
*   Return:    none
*   Notice:    dp data analysis for raw data, send return frame
***********************************************************/
VOID dev_raw_dp_cb(IN CONST TY_RECV_RAW_DP_S *dp)
{
    PR_DEBUG("raw data dpid:%d",dp->dpid);

    PR_DEBUG("recv len:%d",dp->len);
#if 0
    INT_T i = 0;
    
    for(i = 0;i < dp->len;i++) {
        PR_DEBUG_RAW("%02X ",dp->data[i]);
    }
#endif
    PR_DEBUG_RAW("\n");
    PR_DEBUG("end");
}

/***********************************************************
*   Function: dev_dp_query_cb
*   Input:    dp_qry: received query cmd
*   Output:   none
*   Return:   none
*   Notice:   device query callback
***********************************************************/
STATIC VOID dev_dp_query_cb(IN CONST TY_DP_QUERY_S *dp_qry)
{
    PR_NOTICE("Recv DP Query Cmd");
    switch_ele_dp_query();
    hw_report_all_dp_status();
}

/***********************************************************
*   Function: wf_nw_status_cb
*   Input:    GW_WIFI_NW_STAT_E: wifi状态
*   Output:   none
*   Return:   none
*   Notice:   WiFi status callback function
***********************************************************/
STATIC VOID wf_nw_status_cb(IN CONST GW_WIFI_NW_STAT_E stat)
{
    PR_NOTICE("wf_nw_status_cb,wifi_status:%d",stat);
    hw_wifi_led_status(stat);
    if(stat == STAT_AP_STA_CONN || stat >= STAT_STA_CONN) {
        hw_report_all_dp_status();
    }
}

OPERATE_RET device_init(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    TY_IOT_CBS_S wf_cbs = {
        status_changed_cb,\
        gw_ug_inform_cb,\
        gw_reset_cb,\
        dev_obj_dp_cb,\
        dev_raw_dp_cb,\
        dev_dp_query_cb,\
        NULL,
    };

    op_ret = tuya_iot_wf_soc_dev_init_param(hw_get_wifi_mode(),WF_START_SMART_FIRST,&wf_cbs,NULL,PRODECT_KEY,DEV_SW_VERSION);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_wf_soc_dev_init_param error,err_num:%d",op_ret);
        return op_ret;
    }

    op_ret = tuya_iot_reg_get_wf_nw_stat_cb(wf_nw_status_cb);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_reg_get_wf_nw_stat_cb is error,err_num:%d",op_ret);
        return op_ret;
    }

    op_ret= app_dltj_init(APP_DLTJ_NORMAL);
    if(OPRT_OK != op_ret) {
        PR_ERR("dltj init err!");
        return op_ret;
    }

    op_ret = app_switch_init(APP_SW_MODE_NORMAL);
    if(op_ret != OPRT_OK) {
        return op_ret;
    }

    return op_ret;
}
