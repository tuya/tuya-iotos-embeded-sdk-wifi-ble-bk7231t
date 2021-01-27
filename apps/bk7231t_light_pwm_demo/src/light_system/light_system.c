/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @Date: 2019-05-22 10:17:39
 * @LastEditors: wls
 * @LastEditTime: 2020-03-06 12:34:08
 * @file name: light_system.c
 * @Description: system adapter process
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 */

#include "light_system.h"
#include "device_config_load.h"
#include "light_init.h"
#include "light_printf.h"
#include "light_control.h"
#include "light_prod.h"
#include "user_flash.h"
#include "gpio_test.h"
#include "smart_frame.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "tuya_iot_wifi_api.h"
#include "cJSON.h"
#include "gw_intf.h"
#include "tuya_ws_db.h"

#include "user_timer.h"
#include "light_tools.h"
#include "tuya_iot_config.h"
#include "tuya_bt_sdk.h"
#include "uni_log.h"


#define COLOR_DATA_LEN      12

STATIC BOOL_T bFastBootInit = FALSE;

STATIC UINT_T uiCountDownValue = 0;

STATIC UCHAR_T ucCanBlinkFlag = TRUE;

VOID vLightDpUpload(UCHAR_T ucDPNum, VOID *pData);
VOID vLightAllDpUpload(VOID);
STATIC VOID vLightShadeThread(PVOID_T pArg);
VOID pre_app_init(VOID);
UCHAR_T *light_ty_get_enum_str(IN DP_CNTL_S *dp_cntl, IN UCHAR_T enum_id);

SemaphoreHandle_t xSemaphoreShade = NULL;

/**
 * @brief: light software resource init
 * @param {none}
 * @retval: none
 */
OPERATE_LIGHT opLightSysSoftwareInit(VOID)
{
    vSemaphoreCreateBinary(xSemaphoreShade);

    xTaskCreate(vLightShadeThread, "thread_shade", 512, NULL, TRD_PRIO_0, NULL);

    return OPRT_OK;
}


/**
 * @brief: light shade change thread
 * @param {PVOID_T pArg -> NULL}
 * @retval: none
 */
STATIC VOID vLightShadeThread(PVOID_T pArg)
{
	STATIC portBASE_TYPE xHigherPritrityTaskWoken = pdFAIL;
	
	xSemaphoreGiveFromISR(xSemaphoreShade, &xHigherPritrityTaskWoken);
    while(1)
    {
        xSemaphoreTake(xSemaphoreShade, portMAX_DELAY);
        vLightCtrlShadeGradually();
    }
}

/**
 * @brief: light software resource init
 * @param {none}
 * @retval: none
 */
VOID vLightSysHWTimerCB(VOID)
{
    STATIC portBASE_TYPE xHigherPritrityTaskWoken = pdFAIL;

    xSemaphoreGiveFromISR(xSemaphoreShade, &xHigherPritrityTaskWoken);
    if(xHigherPritrityTaskWoken == pdTRUE)
    {

        portEND_SWITCHING_ISR(xHigherPritrityTaskWoken);
    }
}

/**
 * @brief: light reset(re-distribute) proc
 * @param {none}
 * @retval: BOOL_T TRUE -> system reboot
 */
BOOL_T bLightSysHWRebootJudge(VOID)
{
    TY_RST_REASON_E eRstNum = tuya_hal_system_get_rst_info();

    PR_DEBUG("reset info -> reason is %d>>>", eRstNum);
    if(TY_RST_POWER_OFF == eRstNum)
    {
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief: light reset(re-distribute) proc
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightSysResetCntOverCB(VOID)
{
    OPERATE_LIGHT opRet = -1;
    UCHAR_T ucConnectMode = 0;

    ucConnectMode = ucLightCtrlGetConnectMode();
    opRet = tuya_iot_wf_gw_fast_unactive(ucConnectMode, WF_START_SMART_FIRST);

    return opRet;
}

/**
 * @brief: light connect blink display proc
 * @param {IN CONST GW_WIFI_NW_STAT_E stat -> wifi connect mode}
 * @retval: none
 */
STATIC VOID vWifiStatusDisplayCB(IN CONST GW_WIFI_NW_STAT_E stat)
{
    OPERATE_LIGHT opRet = -1;
    STATIC GW_WIFI_NW_STAT_E LastWifiStat = 0xFF;
    STATIC UCHAR_T ucConnectFlag = FALSE;

    if(LastWifiStat != stat)
    {
        PR_DEBUG("last wifi stat:%d, wifi stat %d",LastWifiStat,stat);
        PR_DEBUG("size:%d", tuya_hal_system_getheapsize());

        if(!ucCanBlinkFlag)     /* when app remove, can't blink */
        {
            PR_DEBUG("app remove proc....,can't blink!");
            return;
        }

        switch(stat)
        {
        case STAT_LOW_POWER:
            PR_NOTICE("start to lowpower display!");
            opRet = opLightCtrlAutoBlinkStop();
            if(opRet != OPRT_OK)
            {
                PR_ERR("Stop auto blink timer error!");
            }
            break;

        case STAT_UNPROVISION:
            ucConnectFlag = TRUE;                /* already distribution network */
            vLightCtrlDataReset();

            PR_NOTICE("start ez config auto blink");
            opRet = opLightCtrlAutoBlinkStart(PAIRING_NORMAL_BLINK_FREQ);  /* start blink */
            if(opRet != OPRT_OK)
            {
                PR_ERR("start auto blink timer error!");
            }
            break;

        case STAT_AP_STA_UNCFG:
            ucConnectFlag = TRUE;                /* already distribution network */
            vLightCtrlDataReset();

            PR_NOTICE("start AP config auto blink");

            opRet = opLightCtrlAutoBlinkStart(PAIRING_SLOW_BLINK_FREQ);  /* start blink */
            if(opRet != OPRT_OK)
            {
                PR_ERR("start auto blink timer error!");
            }
            break;

        case STAT_AP_STA_DISC: /*  */
            /* do nothing */
            break;

        case STAT_AP_STA_CONN:  /* priority turn down */
            /* do nothing */
            break;


        case STAT_STA_DISC:
            if(ucConnectFlag != TRUE)    /* only distribution network, need to stop and run ctrl proc */
            {
                break;
            }

            ucConnectFlag = FALSE;          /* to avoid disconnect, set default bright cfg */
            PR_DEBUG("Blink stop!!!!");
            opRet = opLightCtrlAutoBlinkStop();
            if(opRet != OPRT_OK)
            {
                PR_ERR("Stop blink timer error!");
            }
            break;

        case STAT_STA_CONN:     /* priority turn down */
            if(ucConnectFlag != TRUE)    /* only distribution network, need to stop and run ctrl proc */
            {
                break;
            }

            ucConnectFlag = FALSE;
            PR_NOTICE("Blink stop!!!!");
            opRet = opLightCtrlAutoBlinkStop();
            if(opRet != OPRT_OK)
            {
                PR_ERR("Stop blink timer error!");
            }
            break;

        case STAT_CLOUD_CONN:
        case STAT_AP_CLOUD_CONN:

            vLightAllDpUpload();

            break;

        default:
            break;
        }
        LastWifiStat = stat;
    }
}

/**
 * @brief: light system format color control data
 * @param {IN CHAR_T *Date -> wifi color control data}
 * @param {OUT USHORT_T *usVal_R -> R light control format}
 * @param {OUT USHORT_T *usVal_G -> G light control format}
 * @param {OUT USHORT_T *usVal_B -> B light control format}
 * @retval: none
*/
VOID vLightSysColor2RGB(IN CHAR_T *Date, OUT USHORT_T *usRed, OUT USHORT_T *usGreen, OUT USHORT_T *usBlue)
{
    USHORT_T usHue, usSat, usValue;
    USHORT_T usMax = 0, usMin = 0;

    usHue = usLightToolSTR2USHORT( ucLightToolASC2Hex(Date[0]), ucLightToolASC2Hex(Date[1]),\
                                   ucLightToolASC2Hex(Date[2]), ucLightToolASC2Hex(Date[3]) );

    usSat = usLightToolSTR2USHORT( ucLightToolASC2Hex(Date[4]), ucLightToolASC2Hex(Date[5]),\
                                   ucLightToolASC2Hex(Date[6]), ucLightToolASC2Hex(Date[7]) );

    usValue = usLightToolSTR2USHORT( ucLightToolASC2Hex(Date[8]), ucLightToolASC2Hex(Date[9]),\
                                     ucLightToolASC2Hex(Date[10]), ucLightToolASC2Hex(Date[11]) );

    PR_DEBUG("HSV value %d %d %d", usHue, usSat, usValue);

    usMax = 1000 * ( (FLOAT_T) ucLightCtrlGetColorMax() / 100.0 );
    usMin = 1000 * ( (FLOAT_T) ucLightCtrlGetColorMin() / 100.0 );

    usValue = ( usValue - 10 ) * ( usMax - usMin ) / ( 1000 - 10 ) + usMin;

    vLightToolHSV2RGB(usHue, usSat, usValue, usRed, usGreen, usBlue);

}

/**
 * @brief: reponse switch property process, realtek
 * @param {OUT BOOL_T bONOFF -> switch status, TRUE is ON}
 * @retval: none
 */
VOID vLightCtrlDataSwitchRespone(OUT BOOL_T bONOFF)
{

    OPERATE_LIGHT opRet = -1;
    cJSON *root = NULL;
    CHAR_T *out = NULL;
    CHAR_T tmp[4] = {0};
    
#if defined(TY_BT_MOD) && TY_BT_MOD == 1
    klv_node_s *p_node = NULL;
    
    p_node = make_klv_list(p_node, DPID_SWITCH, DT_BOOL, &bONOFF, DT_BOOL_LEN);
    p_node = make_klv_list(p_node, DPID_COUNTDOWN, DT_VALUE, &uiCountDownValue, DT_VALUE_LEN);
  
    PR_DEBUG("*******************ty_bt_klv_report****************");
    ty_bt_klv_report(p_node);
    free_klv_list(p_node);


    GW_WIFI_NW_STAT_E cur_nw_stat;
    get_wf_gw_nw_status(&cur_nw_stat);
    if((cur_nw_stat < STAT_STA_CONN) && (cur_nw_stat != STAT_AP_STA_CONN)) { 
        return;
    }
#endif


    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error...");
        return;
    }

    vNum2Str(0, DPID_SWITCH, 4, tmp);
    cJSON_AddBoolToObject(root, tmp, bONOFF);

    vNum2Str(0, DPID_COUNTDOWN, 4, tmp);
    cJSON_AddNumberToObject(root, tmp, uiCountDownValue);
    
    

    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if(NULL == out) {
        PR_ERR("cJSON_PrintUnformatted err...");
        return;
    }
    PR_DEBUG("upload: %s", out);

    opRet = sf_obj_dp_report_async(get_gw_cntl()->gw_if.id, out, FALSE);
    Free(out);
    out = NULL;
    if(LIGHT_OK != opRet) {
        PR_ERR("sf_obj_dp_report err:%d",opRet);
    }
}

/**
 * @brief: reponse mode property process, realtek
 * @param {OUT LIGHT_MODE_E Mode}
 * @retval: none
 */
VOID vLightCtrlDataModeResponse(OUT LIGHT_MODE_E Mode)
{
    OPERATE_LIGHT opRet = -1;
    cJSON *root = NULL;
    CHAR_T *out = NULL;
    DP_CNTL_S *dp_cntl =  NULL;
    CHAR_T tmp[4] = {0};

#if defined(TY_BT_MOD) && TY_BT_MOD == 1
    klv_node_s *p_node = NULL;
    UINT_T TempMode =0;
    
    TempMode = Mode;
    p_node = make_klv_list(p_node, DPID_MODE, DT_ENUM, &TempMode, DT_ENUM_LEN);
  
    PR_DEBUG("*******************ty_bt_klv_report****************");
    ty_bt_klv_report(p_node);
    free_klv_list(p_node);


    GW_WIFI_NW_STAT_E cur_nw_stat;
    get_wf_gw_nw_status(&cur_nw_stat);
    if((cur_nw_stat < STAT_STA_CONN) && (cur_nw_stat != STAT_AP_STA_CONN)) { 
        return;
    }
#endif


    DEV_CNTL_N_S *dev_cntl = get_gw_cntl()->dev;

    if(dev_cntl == NULL) {
        return;
    }

    dp_cntl = &dev_cntl->dp[1];

    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error...");
        return;
    }


    vNum2Str(0, DPID_MODE, 4, tmp);
    cJSON_AddStringToObject(root, tmp, light_ty_get_enum_str(dp_cntl,(UCHAR_T)Mode));
     

    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if(NULL == out) {
        PR_ERR("cJSON_PrintUnformatted err...");
        return;
    }
    PR_DEBUG("upload: %s", out);

    opRet = sf_obj_dp_report_async(get_gw_cntl()->gw_if.id, out, FALSE);
    Free(out);
    out = NULL;
    if(LIGHT_OK != opRet) {
        PR_ERR("sf_obj_dp_report err:%d",opRet);
    }
}

/**
 * @brief: reponse bright property process
 * @param {OUT LIGHT_MODE_E Mode} 
 * @param {OUT USHORT_T usBright} 
 * @attention: need reponse mode property,as set bright value, will auto set the Mode to WHITE_MODE!
 * @retval: none
 */
VOID vLightCtrlDataBrightResponse(OUT LIGHT_MODE_E eMode, OUT USHORT_T usBright)
{
    OPERATE_LIGHT opRet = -1;
    cJSON *root = NULL;
    CHAR_T *out = NULL;    
    DP_CNTL_S *dp_cntl =  NULL;
    CHAR_T tmp[4] = {0};

    
#if defined(TY_BT_MOD) && TY_BT_MOD == 1
    klv_node_s *p_node = NULL;
    UINT_T TempMode = 0;
    UINT_T TempBright = 0;
    TempMode = eMode;
    TempBright = usBright;
    p_node = make_klv_list(p_node, DPID_MODE, DT_ENUM, &TempMode, DT_ENUM_LEN);
    p_node = make_klv_list(p_node, DPID_BRIGHT, DT_VALUE, &TempBright, DT_VALUE_LEN);
    
    PR_DEBUG("*******************ty_bt_klv_report****************");
    ty_bt_klv_report(p_node);
    free_klv_list(p_node);


    GW_WIFI_NW_STAT_E cur_nw_stat;
    get_wf_gw_nw_status(&cur_nw_stat);
    if((cur_nw_stat < STAT_STA_CONN) && (cur_nw_stat != STAT_AP_STA_CONN)) { 
        return;
    }
#endif
    

    DEV_CNTL_N_S *dev_cntl = get_gw_cntl()->dev;
    
    if(dev_cntl == NULL) {
        return;
    }
    dp_cntl = &dev_cntl->dp[1];    
    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error...");
        return;
    }

    vNum2Str(0, DPID_MODE, 4, tmp);
    cJSON_AddStringToObject(root, tmp, light_ty_get_enum_str(dp_cntl,(UCHAR_T)eMode));
    
    vNum2Str(0, DPID_BRIGHT, 4, tmp);
    cJSON_AddNumberToObject(root, tmp, usBright);
    
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if(NULL == out) {
        PR_ERR("cJSON_PrintUnformatted err...");
        return;
    }
    PR_DEBUG("upload: %s", out);

    opRet = sf_obj_dp_report_async(get_gw_cntl()->gw_if.id, out, FALSE);
    Free(out);
    out = NULL;
    if(LIGHT_OK != opRet) {
        PR_ERR("sf_obj_dp_report err:%d",opRet);
    }

}

/**
 * @brief: reponse temperature property process, realtek
 * @param {OUT LIGHT_MODE_E Mode}
 * @param {OUT USHORT_T usTemperature}
 * @attention: need reponse mode property,as set temperature value, will auto set the Mode to WHITE_MODE!
 * @retval: none
 */
VOID vLightCtrlDataTemperatureResponse(OUT LIGHT_MODE_E eMode, OUT USHORT_T usTemperature)
{
    OPERATE_LIGHT opRet = -1;
    cJSON *root = NULL;
    CHAR_T *out = NULL;
    DP_CNTL_S *dp_cntl =  NULL;
    CHAR_T tmp[4] = {0};


#if defined(TY_BT_MOD) && TY_BT_MOD == 1
    klv_node_s *p_node = NULL;
    UINT_T TempMode = 0;
    UINT_T TempTemp = 0;
    TempMode = eMode;
    TempTemp = usTemperature;
    p_node = make_klv_list(p_node, DPID_MODE, DT_ENUM, &TempMode, DT_ENUM_LEN);
    p_node = make_klv_list(p_node, DPID_TEMPR, DT_VALUE, &TempTemp, DT_VALUE_LEN);
    
    PR_DEBUG("*******************ty_bt_klv_report****************");
    ty_bt_klv_report(p_node);
    free_klv_list(p_node);


    GW_WIFI_NW_STAT_E cur_nw_stat;
    get_wf_gw_nw_status(&cur_nw_stat);
    if((cur_nw_stat < STAT_STA_CONN) && (cur_nw_stat != STAT_AP_STA_CONN)) { 
        return;
    }
#endif


    DEV_CNTL_N_S *dev_cntl = get_gw_cntl()->dev;
    
    if(dev_cntl == NULL) {
        return;
    }

    dp_cntl = &dev_cntl->dp[1];
    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error...");
        return;
    }

    vNum2Str(0, DPID_MODE, 4, tmp);
    cJSON_AddStringToObject(root, tmp, light_ty_get_enum_str(dp_cntl,(UCHAR_T)eMode));
    
    vNum2Str(0, DPID_TEMPR, 4, tmp);
    cJSON_AddNumberToObject(root, tmp, usTemperature);

    
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if(NULL == out) {
        PR_ERR("cJSON_PrintUnformatted err...");
        return;
    }
    PR_DEBUG("upload: %s", out);

    opRet = sf_obj_dp_report_async(get_gw_cntl()->gw_if.id, out, FALSE);
    Free(out);
    out = NULL;
    if(LIGHT_OK != opRet) {
        PR_ERR("sf_obj_dp_report err:%d",opRet);
    }
}

/**
 * @brief: reponse RGB property process
 * @param {OUT COLOR_ORIGIN_T *ColorOrigin} 
 * @retval: none
 */
VOID vLightCtrlDataRGBResponse(OUT COLOR_ORIGIN_T *ptColorOrigin)
{
    OPERATE_LIGHT opRet = -1;
    cJSON *root = NULL;
    CHAR_T *out = NULL;
    CHAR_T tmp[4] = {0};

#if defined(TY_BT_MOD) && TY_BT_MOD == 1
    klv_node_s *p_node = NULL;

    p_node = make_klv_list(p_node, DPID_COLOR, DT_STRING, ptColorOrigin->ucColorStr, strlen(ptColorOrigin->ucColorStr));
    
    PR_DEBUG("*******************ty_bt_klv_report****************");
    ty_bt_klv_report(p_node);
    free_klv_list(p_node);


    GW_WIFI_NW_STAT_E cur_nw_stat;
    get_wf_gw_nw_status(&cur_nw_stat);
    if((cur_nw_stat < STAT_STA_CONN) && (cur_nw_stat != STAT_AP_STA_CONN)) { 
        return;
    }
#endif


    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error...");
        return;
    }

    vNum2Str(0, DPID_COLOR, 4, tmp);
    cJSON_AddStringToObject(root, tmp, ptColorOrigin->ucColorStr);

    
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if(NULL == out) {
        PR_ERR("cJSON_PrintUnformatted err...");
        return;
    }
    PR_DEBUG("upload: %s", out);

    opRet = sf_obj_dp_report_async(get_gw_cntl()->gw_if.id, out, FALSE);
    Free(out);
    out = NULL;
    if(LIGHT_OK != opRet) {
        PR_ERR("sf_obj_dp_report err:%d",opRet);
    }
}


/**
 * @brief: reponse scene property process, realtek
 * @param {OUT UCHAR_T *SceneData} 
 * @retval: none
 */
VOID vLightCtrlDataSceneResponse(OUT CHAR_T *pSceneData)
{

    OPERATE_LIGHT opRet = -1;
    cJSON *root = NULL;
    CHAR_T *out = NULL;
    CHAR_T tmp[4] = {0};


#if defined(TY_BT_MOD) && TY_BT_MOD == 1
    klv_node_s *p_node = NULL;

    p_node = make_klv_list(p_node, DPID_SCENE, DT_STRING, pSceneData, strlen(pSceneData));
    
    PR_DEBUG("*******************ty_bt_klv_report****************");
    ty_bt_klv_report(p_node);
    free_klv_list(p_node);


    GW_WIFI_NW_STAT_E cur_nw_stat;
    get_wf_gw_nw_status(&cur_nw_stat);
    if((cur_nw_stat < STAT_STA_CONN) && (cur_nw_stat != STAT_AP_STA_CONN)) { 
        return;
    }
#endif


    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error...");
        return;
    }

    vNum2Str(0, DPID_SCENE, 4, tmp);
    cJSON_AddStringToObject(root, tmp, pSceneData);

    
    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if(NULL == out) {
        PR_ERR("cJSON_PrintUnformatted err...");
        return;
    }
    PR_DEBUG("upload: %s", out);

    opRet = sf_obj_dp_report_async(get_gw_cntl()->gw_if.id, out, FALSE);
    Free(out);
    out = NULL;
    if(LIGHT_OK != opRet) {
        PR_ERR("sf_obj_dp_report err:%d",opRet);
    }
}

/**
 * @brief: Light countdown proc
 * @param {OUT UINT_T RemainTimeSec -> remain countdown time,unit:s}
 * @retval: none
 */
VOID vLightCtrlDataCountDownResponse(OUT UINT_T uiRemainTimeSec)
{

    OPERATE_LIGHT opRet = -1;
    cJSON *root = NULL;
    CHAR_T *out = NULL;
    CHAR_T tmp[4] = {0};
    BOOL_T bSwitch = 0xFF;
    STATIC BOOL_T bLastSwitch = 0xFF;
    
    
    PR_DEBUG("count down %d...", uiRemainTimeSec);
    uiCountDownValue = uiRemainTimeSec ;

    opRet = opLightCtrlDataSwitchGet(&bSwitch);
    if(opRet != LIGHT_OK) {
        bSwitch = bLastSwitch;   /* get err, to make sure don't upload! */
    }

#if defined(TY_BT_MOD) && TY_BT_MOD == 1
    klv_node_s *p_node = NULL;

    if(bLastSwitch != bSwitch) {
        p_node = make_klv_list(p_node, DPID_SWITCH, DT_BOOL, &bSwitch, DT_BOOL_LEN);
    }
    p_node = make_klv_list(p_node, DPID_COUNTDOWN, DT_VALUE, &uiCountDownValue, DT_VALUE_LEN);
  
    PR_DEBUG("*******************ty_bt_klv_report****************");
    ty_bt_klv_report(p_node);
    free_klv_list(p_node);


    GW_WIFI_NW_STAT_E cur_nw_stat;
    get_wf_gw_nw_status(&cur_nw_stat);
    if((cur_nw_stat < STAT_STA_CONN) && (cur_nw_stat != STAT_AP_STA_CONN)) { 
        bLastSwitch = bSwitch;
        return;
    }
#endif


    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error...");
        return;
    }

    if(bLastSwitch != bSwitch) {        
        vNum2Str(0, DPID_SWITCH, 4, tmp);
        cJSON_AddBoolToObject(root, tmp, bSwitch);
    }    
    vNum2Str(0, DPID_COUNTDOWN, 4, tmp);
    cJSON_AddNumberToObject(root, tmp, uiCountDownValue);
    

    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if(NULL == out) {
        PR_ERR("cJSON_PrintUnformatted err...");
        return;
    }
    PR_DEBUG("upload: %s", out);

    opRet = sf_obj_dp_report_async(get_gw_cntl()->gw_if.id, out, FALSE);
    Free(out);
    out = NULL;
    if(LIGHT_OK != opRet) {
        PR_ERR("sf_obj_dp_report err:%d",opRet);
    }

    bLastSwitch = bSwitch;
}

/**
 * @brief: Light exist lowpower proc
 * @param {none}
 * @retval: none
 */
VOID vLightSysCtrlLowPowerExist(VOID)
{
    OPERATE_LIGHT opRet = -1;

    wf_lowpower_disable();
    system_timer_set_sleep_interval(10);
    tuya_hal_set_lp_mode(FALSE);
    
    opRet = opUserHWTimerStart(HW_TIMER_CYCLE_US, (VOID *)vLightCtrlHWTimerCB);      /* enable hardware time */
    if(opRet != OPRT_OK)
    {
        PR_ERR("Light exist lowpower start hardware timer error!");
    }
}

/**
 * @brief: Light enter lowpower proc
 * @param {none}
 * @retval: none
 */
VOID vLightSysCtrlLowPowerEnter(VOID)
{
    OPERATE_LIGHT opRet = -1;

    PR_DEBUG("enter lowpower!");
    tuya_hal_set_lp_mode(TRUE);
    wf_lowpower_enable();
    system_timer_set_sleep_interval(1000);

    opRet = opUserHWTimerStop();    /* disable hardware time */
    if(opRet != OPRT_OK)
    {
        PR_ERR("Light enter lowpower stop hardware timer error!");
    }
}

/**
 * @brief: Light control datapoint proc
 * @param {CONST TY_OBJ_DP_S *root -> control DP data}
 * @retval: BOOL_T
 */
BOOL_T bLightDpProc(CONST TY_OBJ_DP_S *root)
{
    OPERATE_LIGHT opRet = -1;
    UINT_T ucLen;
    UCHAR_T dpid;
    BOOL_T bActiveFlag = FALSE;
    COLOR_RGB_T tColorData;
    COLOR_ORIGIN_T tColorOrigin;


    dpid = root->dpid;

    PR_DEBUG("light_light_dp_proc dpid=%d",dpid);

    switch(dpid)
    {

    case DPID_SWITCH:
        PR_DEBUG("set switch %d",root->value.dp_bool);

        opRet = opLightCtrlDataSwitchSet(root->value.dp_bool);
        if(OPRT_OK == opRet)
        {
            bActiveFlag = TRUE;
            opRet = opLightCtrlDataCountDownSet(0);
            if(opRet != OPRT_OK)
            {
            }
        }
        break;

    case DPID_MODE:
        if(root->type != PROP_ENUM)
        {
            break;
        }
        PR_DEBUG("mode is %d", root->value.dp_enum);
        opRet = opLightCtrlDataModeSet(root->value.dp_enum);
        if(OPRT_OK == opRet)
        {
            bActiveFlag = TRUE;
        }
        break;

    case DPID_BRIGHT:
        if(root->type != PROP_VALUE)
        {
            break;
        }
        PR_DEBUG("bright set value %d",root->value.dp_value);

        opRet = opLightCtrlDataBrightSet(root->value.dp_value);
        if(OPRT_OK == opRet)
        {
            bActiveFlag = TRUE;
        }
        break;

    case DPID_TEMPR:
        if(root->type != PROP_VALUE)
        {
            break;
        }
        PR_DEBUG("temper set value %d",root->value.dp_value);

        opRet = opLightCtrlDataTemperatureSet(root->value.dp_value);
        if(OPRT_OK == opRet)
        {
            bActiveFlag = TRUE;
        }
        break;

    case DPID_COLOR:
        if(root->type != PROP_STR)
        {
            break;
        }

        ucLen = strlen(root->value.dp_str);
        if(ucLen != COLOR_DATA_LEN)
        {
            PR_ERR("the data length is wrong: %d", ucLen);
            break;
        }

        PR_DEBUG("color data %s",root->value.dp_str);

        vLightSysColor2RGB(root->value.dp_str, &tColorData.usRed, &tColorData.usGreen, &tColorData.usBlue);
        memcpy(tColorOrigin.ucColorStr, root->value.dp_str, SIZEOF(tColorOrigin.ucColorStr));
        opRet = opLightCtrlDataRGBSet(&tColorData, &tColorOrigin);
        if(OPRT_OK == opRet)
        {
            bActiveFlag = TRUE;
        }
        break;

    case DPID_SCENE:
        if(root->type != PROP_STR)
        {
            break;
        }
        PR_DEBUG("scene data %s",root->value.dp_str);
        opRet = opLightCtrlDataSceneSet(root->value.dp_str);
        if(OPRT_OK == opRet)
        {
            bActiveFlag = TRUE;
        }
        break;

    case DPID_COUNTDOWN:
        if(root->type != PROP_VALUE)
        {
            break;
        }
        opRet = opLightCtrlDataCountDownSet(root->value.dp_value);
        if(opRet != OPRT_OK)
        {
            PR_ERR("count down set error!");
        }
        else
        {
            ; //do nothing
        }
        break;

    case DPID_MUSIC:
        if(root->type != PROP_STR)
        {
            break;
        }
        PR_DEBUG("music ctrl data %s",root->value.dp_str);
        opRet = opLightCtrlDataRealTimeAdjustSet(TRUE, root->value.dp_str);
        if(OPRT_OK != opRet)
        {
            break;
        }
        opRet = opLightRealTimeCtrlProc();
        if(opRet != OPRT_OK)
        {
            PR_ERR("real time control error!");
        }
        break;

    case DPID_CONTROL:
        if(root->type != PROP_STR)
        {
            break;
        }
        PR_DEBUG("realtime ctrl data %s",root->value.dp_str);
        opRet = opLightCtrlDataRealTimeAdjustSet(FALSE, root->value.dp_str);
        if(OPRT_OK != opRet)
        {
            break;
        }
        opRet = opLightRealTimeCtrlProc();
        if(opRet != OPRT_OK)
        {
            PR_ERR("real time control error!");
        }
        break;

    default:

        break;
    }
    PR_DEBUG("ctrl data need to proc flag =%d", bActiveFlag);

    return bActiveFlag;
}

/**
 * @brief: get dp name(string name)
 * @param {IN TY_OBJ_DP_S *dp_cntl -> control DP data}
 * @param {IN UCHAR_T enum_id -> DP enum id}
 * @return: none
 * @retval: UCHAR_T
 */
UCHAR_T *light_ty_get_enum_str(IN DP_CNTL_S *dp_cntl, IN UCHAR_T enum_id)
{
    if( dp_cntl == NULL )
    {
        return NULL;
    }

    if( enum_id >= dp_cntl->prop.prop_enum.cnt )
    {
        return NULL;
    }

    return dp_cntl->prop.prop_enum.pp_enum[enum_id];
}

/**
 * @brief: Light all dp upload
 * @param {none}
 * @retval: none
 */
VOID vLightAllDpUpload(VOID)
{
    OPERATE_LIGHT opRet = -1;
    cJSON *root = NULL;
    CHAR_T *out = NULL;
    DP_CNTL_S *dp_cntl =  NULL;
    CHAR_T tmp[4] = {0};
    LIGHT_CTRL_DATA_T* Updata = NULL;
    BOOL_T bBtEnable = FALSE;
    
#if defined(TY_BT_MOD) && TY_BT_MOD == 1
    GW_WIFI_NW_STAT_E cur_nw_stat;
    get_wf_gw_nw_status(&cur_nw_stat);
    if((cur_nw_stat < STAT_STA_CONN) && (cur_nw_stat != STAT_AP_STA_CONN)) { 
        return;
    }
#endif

    DEV_CNTL_N_S *dev_cntl = get_gw_cntl()->dev;

    if(dev_cntl == NULL)
    {
        return;
    }

    Updata = (LIGHT_CTRL_DATA_T*)Malloc(SIZEOF(LIGHT_CTRL_DATA_T));
    if(NULL == Updata)
    {
        PR_ERR("Updata malloc failed");
        return;
    }
    dp_cntl = &dev_cntl->dp[1];

    root = cJSON_CreateObject();
    if(NULL == root)
    {
        PR_ERR("cJSON_CreateObject error...");
        Free(Updata);
        return;
    }

    opRet = opLightCtrlDataSwitchGet(&Updata->bSwitch);
    if(opRet == LIGHT_OK)
    {
        vNum2Str(0, DPID_SWITCH, 4, tmp);
        cJSON_AddBoolToObject(root, tmp, Updata->bSwitch);
    }


    opRet = opLightCtrlDataModeGet(&Updata->eMode);
    if(opRet == LIGHT_OK)
    {
        vNum2Str(0, DPID_MODE, 4, tmp);
        cJSON_AddStringToObject(root, tmp, light_ty_get_enum_str(dp_cntl,(UCHAR_T)Updata->eMode));
    }

    opRet = opLightCtrlDataBrightGet(&Updata->usBright);
    if(opRet == LIGHT_OK)
    {
        vNum2Str(0, DPID_BRIGHT, 4, tmp);
        cJSON_AddNumberToObject(root, tmp, Updata->usBright);
    }

    opRet = opLightCtrlDataTemperatureGet(&Updata->usTemper);
    if(opRet == LIGHT_OK)
    {
        vNum2Str(0, DPID_TEMPR, 4, tmp);
        cJSON_AddNumberToObject(root, tmp, Updata->usTemper);
    }

    opRet = opLightCtrlDataRGBGet(&Updata->tColorOrigin);
    if(opRet == LIGHT_OK)
    {
        vNum2Str(0, DPID_COLOR, 4, tmp);
        cJSON_AddStringToObject(root, tmp, Updata->tColorOrigin.ucColorStr);
    }

    opRet = opLightCtrlDataSceneGet(Updata->cScene);
    if(opRet == LIGHT_OK)
    {
        vNum2Str(0, DPID_SCENE, 4, tmp);
        cJSON_AddStringToObject(root, tmp, Updata->cScene);
    }
    Free(Updata);
    Updata = NULL;

    vNum2Str(0, DPID_COUNTDOWN, 4, tmp);
    cJSON_AddNumberToObject(root, tmp, uiCountDownValue);

    out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if(NULL == out)
    {
        PR_ERR("cJSON_PrintUnformatted err...");
        return;
    }

    PR_DEBUG("upload: %s", out);

    opRet = sf_obj_dp_report_async(get_gw_cntl()->gw_if.id, out, FALSE);
    Free(out);
    out = NULL;
    if(LIGHT_OK != opRet)
    {
        PR_ERR("sf_obj_dp_report err:%d",opRet);
        return;
    }

}


/**
 * @brief: light dp ctrl process callback
 * @param {IN CONST TY_RECV_OBJ_DP_S *dp -> dp ctrl data}
 * @retval: none
 */
STATIC VOID vDeviceCB(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    OPERATE_LIGHT opRet = -1;
    BOOL_T bActiceFlag = FALSE;
    BOOL_T bNeedUpdateFlag = FALSE;
    UCHAR_T i = 0;

    if(NULL == dp)
    {
        PR_ERR("dp error");
        return;
    }

    UCHAR_T nxt = dp->dps_cnt;
    PR_DEBUG("dp_cnt:%d", nxt);

    for(i = 0; i < nxt; i++)
    {
        bActiceFlag = bLightDpProc(&(dp->dps[i]));
        if(bActiceFlag == TRUE)
        {
            bNeedUpdateFlag = TRUE;
        }
    }

    if(bNeedUpdateFlag == TRUE)          /* ctrl */
    {
        opRet = opLightCtrlProc();
        if(opRet != OPRT_OK)
        {
            PR_ERR("ctrl proc deal error!");
        }

        opRet = opLightCtrlDataAutoSaveStart(5000);
        if(opRet != OPRT_OK)
        {
            PR_ERR("Save Dp data error!");
        }
        bActiceFlag = FALSE;
    }
}

/**
 * @brief: gw stauts callback
 * @param {none}
 * @retval: none
 */
STATIC VOID vGWStatusCB(IN CONST GW_STATUS_E status)
{
    if(status == GW_RESET)
    {
        PR_DEBUG("APP remove status");
        ucCanBlinkFlag = FALSE;
        PR_NOTICE("gw reset status heap stack %d", tuya_hal_system_getheapsize());
    }

    PR_NOTICE("gw status heap stack %d", tuya_hal_system_getheapsize());
}

/**
 * @brief: reset proc callback
 * @param {GW_RESET_TYPE_E type -> reset reason}
 * @retval: none
 */
STATIC VOID vResetCB(GW_RESET_TYPE_E type)
{
    PR_NOTICE("gw reset cb heap stack %d", tuya_hal_system_getheapsize());

    /* attention: before restart ,need to save in flash */
    switch(type)
    {
    case GW_LOCAL_RESET_FACTORY:
    case GW_LOCAL_UNACTIVE:
        vLightCtrlDataReset();      /* local reset,just reset ctrl data, as  */
        break;

    case GW_REMOTE_UNACTIVE:
    case GW_REMOTE_RESET_FACTORY:
        vLightCtrlDataReset();
        opLightCtrlDataAutoSave();
        break;

    default:
        break;
    }
}

/**
 * @brief: query dp process
 * @param {none}
 * @retval: none
 */
STATIC VOID vQueryCB(IN CONST TY_DP_QUERY_S *dp_qry)
{
    vLightAllDpUpload();       /* dp data all upload */
}

/**
 * @brief: light smart frame init
 * @param {IN CHAR_T *sw_ver -> bin version}
 * @return: none
 * @retval: none
 */
STATIC OPERATE_LIGHT opLightSysSmartFrameInit(IN CHAR_T *sw_ver)
{
    OPERATE_LIGHT opRet = -1;
    UCHAR_T ucConnectMode = 0;

    TY_IOT_CBS_S wf_cbs =
    {
        .gw_status_cb = vGWStatusCB,
        .gw_ug_cb = NULL,
        .gw_reset_cb = vResetCB,
        .dev_obj_dp_cb = vDeviceCB,
        .dev_raw_dp_cb = NULL,
        .dev_dp_query_cb = vQueryCB,\
        .dev_ug_cb = NULL,
        .dev_reset_cb = NULL,
	#if defined(TUYA_GW_OPERATOR) && (TUYA_GW_OPERATOR==1)
        .ope_get_chcode_cb = NULL,
	#endif
    };

    PR_NOTICE("frame goto init!");
    ucConnectMode = ucLightCtrlGetConnectMode();
    /* wifi smart fram inits */
    opRet = tuya_iot_wf_soc_dev_init_param(ucConnectMode, WF_START_SMART_FIRST, &wf_cbs, NULL, PRODUCT_KEY, sw_ver);
    if(OPRT_OK != opRet)
    {
        PR_ERR("tuya_iot_wf_soc_dev_init err:%02x",opRet);
        return opRet;
    }

    PR_NOTICE("frame init out!");
    opRet = tuya_iot_reg_get_wf_nw_stat_cb(vWifiStatusDisplayCB);   /* register wifi status callback */
    if(OPRT_OK != opRet)
    {
        PR_ERR("tuya_iot_reg_get_wf_nw_stat_cb err:%02x",opRet);
        return opRet;
    }
    PR_NOTICE("frame init ok!");
    return OPRT_OK;
}

/**
 * @brief: wifi reset finsh process
 * @param {none}
 * @attention: none
 * @retval: none
 */
VOID vLightSysResetProc(VOID)
{
    PR_NOTICE("reset finish callback!");
    vLightCtrlResetCntClear();
}

/**
 * @brief: wifi fast initlize process
 * @param {none}
 * @attention: this partion can't operate kv flash
                and other wifi system service
 * @attention: called by pre_device_init()
 * @retval: none
 */
VOID vLightSysPreDeviceinit(VOID)
{
    OPERATE_LIGHT opRet = -1;


    SetLogManageAttr(TY_LOG_LEVEL_NOTICE);

    PR_NOTICE("go to pre device!");
    opRet = opLightInit();
    if(opRet != LIGHT_OK)
    {
        PR_NOTICE("Fast boot error!");
        return;
    }

    bFastBootInit = TRUE;

    PR_NOTICE("goto first bright up!");
    opRet = opLightCtrlProc();
    if(opRet != LIGHT_OK)
    {
        PR_ERR("Pre device init set RGBCW error!");
        return;
    }

    /* attention: to make sure light up in 500ms! */
    vLightCtrlHWRebootProc();                       /* write recnt count! reload ctrl data! */
    /* write cnt into flash will take 220ms time */

    //set_gw_reset_fin_cb(vLightSysResetProc);
    pre_app_cfg_set(pre_app_init);  /* set pre app init to judge ifnot reset */

}

/**
 * @brief: wifi reset judge process
 *          to save 500ms(don't need to wait wifi init semaphore) when reset to re-distribute
 * @param {none}
 * @retval: none
 */
VOID pre_app_init(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opLightCtrlResetCntProcess();
    if(opRet != OPRT_OK)
    {
        PR_ERR("Light Reset proc error!");
        return;
    }

}

/**
 * @brief: wifi normal initlize process
 * @param {none}
 * @attention: called by app_init()
 * @retval: none
 */
VOID vLightSysAppInit(VOID)
{
    OPERATE_LIGHT opRet = -1;
    UINT_T len = 0;
    CHAR_T *pConfig = NULL;
    UCHAR_T ucConnectMode = 0;
    LIGHT_PROD_TEST_DATA_FLASH_T tProdResult ;

    //sys_log_uart_on();                                /* 打开打印 */
    PR_NOTICE("%s",tuya_iot_get_sdk_info());            /* output sdk information */
    PR_NOTICE("%s:%s",APP_BIN_NAME,USER_SW_VER);        /* output bin information */

    if(bFastBootInit != TRUE)       /* fast init failure, need to read oem cfg from kvs */
    {
        opRet = wd_user_param_read(&pConfig, &len);
        if(opRet != LIGHT_OK)
        {
            PR_ERR("read kvs oem cfg error!");
            if(LIGHT_NOT_FOUND == opRet)             /* don't has oem config! */
            {
                if(pConfig != NULL)
                {
                    Free(pConfig);
                    pConfig = NULL;
                }

                opRet = opDeviceCfgDataSet(strlen(DEFAULT_CONFIG), DEFAULT_CONFIG);
                if(opRet != LIGHT_OK)
                {
                    PR_ERR("Default oem config error!");
                }

                len = strlen(DEFAULT_CONFIG);
                if(len > 1024)
                {
                    PR_ERR("default oem cfg too large!");
                }
                PR_DEBUG("default oem config len %d", strlen(DEFAULT_CONFIG));

                pConfig = Malloc(1024);
                if(pConfig != NULL)
                {
                    memcpy(pConfig, DEFAULT_CONFIG, strlen(DEFAULT_CONFIG));
                }
                /* load default oem cfg!!!! */
            }
            else
            {
                return;
            }
        }

        PR_DEBUG("kv oem cfg %s", pConfig);
        opRet = opUserFlashWriteOemCfgData(len, pConfig);
        Free(pConfig);
        pConfig = NULL;
        if(opRet != LIGHT_OK)
        {
            PR_ERR("move kvs oem cfg to uf_file error!");
            return;
        }

        PR_NOTICE("oem cfg move to uf file ok!");
        opRet = opDeviceCfgDataLoad();
        if(opRet != LIGHT_OK)
        {
            PR_ERR("oem cfg load error!");
            return;
        }

        opRet = opLightInit();
        if(opRet != LIGHT_OK)
        {
            PR_ERR("Light init error again!");
            return;
        }

        opRet = opLightCtrlProc();
        if(opRet != LIGHT_OK)
        {
            PR_ERR("Pre device init set RGBCW error!");
            return;
        }
    }

#ifdef _IS_OEM
    tuya_iot_oem_set(TRUE);
#endif

    opRet = opLightProdInit();  /* prod init */
    if(opRet != LIGHT_OK)
    {
        PR_ERR("Prod init error!");
        return;
    }

    ucConnectMode = ucLightCtrlGetConnectMode();

    PR_NOTICE("connect mode is %d", ucConnectMode);

    set_wf_cfg_timeout(ucLightCtrlGetConnectTime() * 60);       /* set spcl or spcl_auto connect pairing time */
    app_cfg_set(ucConnectMode, vProdTestCB);                    /* register prod cb */

    opRet = opUserFlashReadProdData(&tProdResult);
    if(opRet != LIGHT_OK)
    {
        return;
    }

    if(PROD_REPEAT == tProdResult.eTestMode)
    {
        PR_DEBUG("Prod test ssid chang to test2!");
        set_prod_ssid("tuya_mdev_test2");
    }
}

/**
 * @brief: device init
 * @param {none}
 * @attention: called by device_init()
 * @retval: none
 */
OPERATE_LIGHT opLightSysDeviceInit(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opLightSysSmartFrameInit(USER_SW_VER);      /* wifi frame init */
    if(opRet != OPRT_OK)
    {
        PR_ERR("smart fram init error");
    }

    return opRet;
}

/**
 * @brief: erase user data when authorization
 * @param {none}
 * @attention: none
 * @retval: none
 */
VOID vLightSysEraseFlashData(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opUserFlashDataErase();
    if(opRet != OPRT_OK)
    {
        PR_ERR("Erase user flash error!");
    }
}


