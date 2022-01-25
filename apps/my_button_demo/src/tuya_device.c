/*
 * @Author: yj 
 * @email: shiliu.yang@tuya.com
 * @LastEditors: yj 
 * @file name: tuya_device.c
 * @Description: template demo for SDK WiFi & BLE for BK7231T
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-01-23 16:33:00
 * @LastEditTime: 2021-01-27 17:00:00
 */

#define _TUYA_DEVICE_GLOBAL

/* Includes ------------------------------------------------------------------*/
#include "uni_log.h"
#include "tuya_iot_wifi_api.h"
#include "tuya_hal_system.h"
#include "tuya_iot_com_api.h"
#include "tuya_cloud_com_defs.h"
#include "gw_intf.h"
#include "gpio_test.h"
#include "tuya_gpio.h"
#include "tuya_key.h"
#include "tuya_led.h"

/* Private includes ----------------------------------------------------------*/
#include "tuya_device.h"

#include "../../beken378/func/key/multi_button.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* wifi 相关配置 */
#define WIFI_WORK_MODE_SEL          GWCM_OLD_PROD   //wifi 工作模式选择
#define WIFI_CONNECT_OVERTIME_S     180             //wifi 配网超时时间，单位：s

/* 配网按键相关宏,长按进入配网模式 */
#define WIFI_KEY_PIN                TY_GPIOA_14 //按键引脚 
#define WIFI_KEY_TIMER_MS           100         //轮询扫描按键所需的时间，一般默认为 100ms
#define WIFI_KEY_LONG_PRESS_MS      3000        //按键长按时间设置
#define WIFI_KEY_SEQ_PRESS_MS       400         //按键连按时间设置
#define WIFI_KEY_LOW_LEVEL_ENABLE   TRUE        //TRUE：按键按下为低，FALSE：按键按下为高

/* 配网指示灯相关宏 */
#define WIFI_LED_PIN                TY_GPIOA_26 //LED 引脚 
#define WIFI_LED_LOW_LEVEL_ENABLE   FALSE       //TRUE：低电平点亮 led，FALSE：高电平点亮 led
#define WIFI_LED_FAST_FLASH_MS      300         //快速闪烁时间 300ms 
#define WIFI_LED_LOW_FLASH_MS       500         //慢闪时间 500ms

/* Private variables ---------------------------------------------------------*/
LED_HANDLE wifi_led_handle; //定义 wifi led 句柄 

/* Private function prototypes -----------------------------------------------*/
VOID hw_report_all_dp_status(VOID);





#define MY_RELAY              GPIO24
#define MY_LED_RED              GPIO14 

beken_timer_t led_timer;

BUTTON_S button_r0;
BUTTON_S button_r1;
BUTTON_S button_r2;
static int cnt = 0;
static void app_led_timer_handler(void *data)
{
	cnt ++;
    bk_gpio_output_reverse(MY_RELAY);
	if( cnt % 2 )
	{
		bk_gpio_output_reverse(MY_LED_RED);
	}
	else
	{
		
	}

    PR_NOTICE("Timer is %i\n",cnt);
}
void button_r0_short_press(void *param)
{
	PR_NOTICE("r0 key_short_press\r\n");
}
void button_r0_double_press(void *param)
{
	PR_NOTICE("r0 key_double_press\r\n");
}
void button_r0_long_press_hold(void *param)
{
	PR_NOTICE("r0 key_long_press_hold\r\n");
}
#define KEY_R0 GPIO7
#define KEY_R1 GPIO8
#define KEY_R2 GPIO9
uint8_t button_r0_get_gpio_value(void)
{
	return bk_gpio_input(KEY_R0);
}
void button_r1_short_press(void *param)
{
	PR_NOTICE("r1 key_short_press\r\n");
}
void button_r1_double_press(void *param)
{
	PR_NOTICE("r1 key_double_press\r\n");
}
void button_r1_long_press_hold(void *param)
{
	PR_NOTICE("r1 key_long_press_hold\r\n");
}
uint8_t button_r1_get_gpio_value(void)
{
	return bk_gpio_input(KEY_R1);
}
void button_r2_short_press(void *param)
{
	PR_NOTICE("r2 key_short_press\r\n");
}
void button_r2_double_press(void *param)
{
	PR_NOTICE("r2 key_double_press\r\n");
}
void button_r2_long_press_hold(void *param)
{
	PR_NOTICE("r2 key_long_press_hold\r\n");
}
uint8_t button_r2_get_gpio_value(void)
{
	return bk_gpio_input(KEY_R2);
}
void myInit()
{

    OSStatus err;
	
	key_configure();

	{
		bk_gpio_config_input_pup(KEY_R0);
		button_init(&button_r0, button_r0_get_gpio_value, 0);
		button_attach(&button_r0, SINGLE_CLICK,     button_r0_short_press);
		button_attach(&button_r0, DOUBLE_CLICK,     button_r0_double_press);
		button_attach(&button_r0, LONG_PRESS_HOLD,  button_r0_long_press_hold);
		button_start(&button_r0);
	}
	{
		bk_gpio_config_input_pup(KEY_R1);
		button_init(&button_r1, button_r1_get_gpio_value, 0);
		button_attach(&button_r1, SINGLE_CLICK,     button_r1_short_press);
		button_attach(&button_r1, DOUBLE_CLICK,     button_r1_double_press);
		button_attach(&button_r1, LONG_PRESS_HOLD,  button_r1_long_press_hold);
		button_start(&button_r1);
	}
	{
		bk_gpio_config_input_pup(KEY_R2);
		button_init(&button_r2, button_r2_get_gpio_value, 0);
		button_attach(&button_r2, SINGLE_CLICK,     button_r2_short_press);
		button_attach(&button_r2, DOUBLE_CLICK,     button_r2_double_press);
		button_attach(&button_r2, LONG_PRESS_HOLD,  button_r2_long_press_hold);
		button_start(&button_r2);
	}
	

    bk_gpio_config_output(MY_RELAY);
    bk_gpio_output(MY_RELAY, 0);
	
    bk_gpio_config_output(MY_LED_RED);
    bk_gpio_output(MY_LED_RED, 0);

    err = rtos_init_timer(&led_timer,
                          1 * 1000,
                          app_led_timer_handler,
                          (void *)0);
    ASSERT(kNoErr == err);

    err = rtos_start_timer(&led_timer);
    ASSERT(kNoErr == err);
}


/* Private functions ---------------------------------------------------------*/
/**
 * @Function: wifi_state_led_reminder
 * @Description: WiFi led指示灯，根据当前 WiFi 状态，做出不同提示 
 * @Input: cur_stat：当前 WiFi 状态 
 * @Output: none
 * @Return: none
 * @Others: 
 */
STATIC VOID wifi_state_led_reminder(IN CONST GW_WIFI_NW_STAT_E cur_stat)
{
    switch (cur_stat)
    {
        case STAT_LOW_POWER:    //wifi 连接超时，进入低功耗模式
            tuya_set_led_light_type(wifi_led_handle, OL_LOW, 0, 0); //关闭提示灯
        break;

        case STAT_UNPROVISION: //SamrtConfig 配网模式，等待连接
            tuya_set_led_light_type(wifi_led_handle, OL_FLASH_HIGH, WIFI_LED_FAST_FLASH_MS, 0xffff); //led 快闪
        break;

        case STAT_AP_STA_UNCFG: //ap 配网模式，等待连接
            tuya_set_led_light_type(wifi_led_handle, OL_FLASH_HIGH, WIFI_LED_LOW_FLASH_MS, 0xffff); //led 慢闪
        break;

        case STAT_AP_STA_DISC:
        case STAT_STA_DISC:     //SamrtConfig/ap 正在连接中
            tuya_set_led_light_type(wifi_led_handle, OL_LOW, 0, 0); //关闭 led 
        break;

        case STAT_CLOUD_CONN:
        case STAT_AP_CLOUD_CONN: //连接到涂鸦云
            tuya_set_led_light_type(wifi_led_handle, OL_HIGH, 0, 0); //led 常量
        break;

        default:
        break;
    }
}

/**
 * @Function: wifi_key_process
 * @Description: 按键回调函数
 * @Input: port：触发引脚,type：按键触发类型,cnt:触发次数
 * @Output: none
 * @Return: none
 * @Others: 长按触发配网模式
 */
STATIC VOID wifi_key_process(TY_GPIO_PORT_E port,PUSH_KEY_TYPE_E type,INT_T cnt)
{
    PR_DEBUG("port:%d,type:%d,cnt:%d",port,type,cnt);
    OPERATE_RET op_ret = OPRT_OK;
    UCHAR_T ucConnectMode = 0;

    if (port = WIFI_KEY_PIN) {
        if (LONG_KEY == type) { //press long enter linking network
            PR_NOTICE("key long press");
            /* 手动移除设备 */
            tuya_iot_wf_gw_unactive();
        } else if (NORMAL_KEY == type) {
            PR_NOTICE("key normal press");
        } else {
            PR_NOTICE("key type is no deal");
        }
    }

    return;
}

/**
 * @Function: wifi_config_init
 * @Description: 初始化 WiFi 相关设备，按键，led指示灯
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 
 */
STATIC VOID wifi_config_init(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    /* LED 相关初始化 */ 
    op_ret = tuya_create_led_handle(WIFI_LED_PIN, TRUE, &wifi_led_handle);
    if (op_ret != OPRT_OK) {
        PR_ERR("key_init err:%d", op_ret);
        return;
    }
    tuya_set_led_light_type(wifi_led_handle, OL_LOW, 0, 0); //关闭 LED

    /* 按键相关初始化 */
    KEY_USER_DEF_S key_def;

    op_ret = key_init(NULL, 0, WIFI_KEY_TIMER_MS);
    if (op_ret != OPRT_OK) {
        PR_ERR("key_init err:%d", op_ret);
        return;
    }

    /* 初始化 key 相关参数 */
    memset(&key_def, 0, SIZEOF(key_def));
    key_def.port = WIFI_KEY_PIN;                            //按键引脚
    key_def.long_key_time = WIFI_KEY_LONG_PRESS_MS;         //长按时间配置
    key_def.low_level_detect = WIFI_KEY_LOW_LEVEL_ENABLE;   //TRUE:低电平算按下，FALSE：高电平算按下
    key_def.lp_tp = LP_ONCE_TRIG;   //
    key_def.call_back = wifi_key_process;                   //按键按下后回调函数
    key_def.seq_key_detect_time = WIFI_KEY_SEQ_PRESS_MS;    //连按间隔时间配置

    /* 注册按键 */
    op_ret = reg_proc_key(&key_def);
    if (op_ret != OPRT_OK) {
        PR_ERR("reg_proc_key err:%d", op_ret);
    }

    return;
}

/**
 * @Function: hw_report_all_dp_status
 * @Description: 上报所有 dp 点
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 
 */
VOID hw_report_all_dp_status(VOID)
{
    //report all dp status
}

/**
 * @Function:gpio_test 
 * @Description: gpio测试
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: none
 */
BOOL_T gpio_test(IN CONST CHAR_T *in, OUT CHAR_T *out)
{
    return gpio_test_all(in, out);
}

/**
 * @Function: mf_user_callback
 * @Description: 授权回调函数
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 清空flash中存储的数据
 */
VOID mf_user_callback(VOID)
{
    hw_reset_flash_data();
    return;
}

/**
 * @Function: prod_test
 * @Description: 扫描到产测热点，进入回调函数，主要是按键、指示灯、继电器功能测试
 * @Input: flag:授权标识；rssi:信号强度
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID prod_test(BOOL_T flag, SCHAR_T rssi)
{
    if (flag == FALSE || rssi < -60) 
    {
        PR_ERR("Prod test failed... flag:%d, rssi:%d", flag, rssi);
        return;
    }
    PR_NOTICE("flag:%d rssi:%d", flag, rssi);

}

/**
 * @Function: app_init
 * @Description: 设备初始化，设置工作模式
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 无
 */
VOID app_init(VOID)
{
    app_cfg_set(WIFI_WORK_MODE_SEL, prod_test);

    /* 设置配网超时时间，未配网超时后退出配网模式 */
    tuya_iot_wf_timeout_set(WIFI_CONNECT_OVERTIME_S);

    /* WiFi 按键，led 初始化 */
    wifi_config_init();
}

/**
 * @Function: pre_device_init
 * @Description: 设备信息(SDK信息、版本号、固件标识等)打印、重启原因和打印等级设置
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID pre_device_init(VOID)
{
    PR_DEBUG("%s",tuya_iot_get_sdk_info());
    PR_DEBUG("%s:%s",APP_BIN_NAME,DEV_SW_VERSION);
    PR_NOTICE("firmware compiled at %s %s", __DATE__, __TIME__);
    PR_NOTICE("Hello Tuya World!");
    PR_NOTICE("system reset reason:[%s]",tuya_hal_system_get_rst_info());
    /* 打印等级设置 */
    SetLogManageAttr(TY_LOG_LEVEL_DEBUG);
}

/**
 * @Function: status_changed_cb
 * @Description: network status changed callback
 * @Input: status: current status
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID status_changed_cb(IN CONST GW_STATUS_E status)
{
    PR_NOTICE("status_changed_cb is status:%d",status);

    if(GW_NORMAL == status) {
        hw_report_all_dp_status();
    }else if(GW_RESET == status) {
        PR_NOTICE("status is GW_RESET");
    }
}

/**
 * @Function: upgrade_notify_cb
 * @Description: firmware download finish result callback
 * @Input: fw: firmware info
 * @Input: download_result: 0 means download succes. other means fail
 * @Input: pri_data: private data
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    PR_DEBUG("download  Finish");
    PR_DEBUG("download_result:%d", download_result);
}

/**
 * @Function: get_file_data_cb
 * @Description: firmware download content process callback
 * @Input: fw: firmware info
 * @Input: total_len: firmware total size
 * @Input: offset: offset of this download package
 * @Input: data && len: this download package
 * @Input: pri_data: private data
 * @Output: remain_len: the size left to process in next cb
 * @Return: OPRT_OK: success  Other: fail
 * @Others: none
 */
OPERATE_RET get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset, \
                                     IN CONST BYTE_T *data, IN CONST UINT_T len, OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    PR_DEBUG("Rev File Data");
    PR_DEBUG("Total_len:%d ", total_len);
    PR_DEBUG("Offset:%d Len:%d", offset, len);

    return OPRT_OK;
}

/**
 * @Function: gw_ug_inform_cb
 * @Description: gateway ota firmware available nofity callback
 * @Input: fw: firmware info
 * @Output: none
 * @Return: int:
 * @Others: 
 */
INT_T gw_ug_inform_cb(IN CONST FW_UG_S *fw)
{
    PR_DEBUG("Rev GW Upgrade Info");
    PR_DEBUG("fw->fw_url:%s", fw->fw_url);
    PR_DEBUG("fw->sw_ver:%s", fw->sw_ver);
    PR_DEBUG("fw->file_size:%d", fw->file_size);

    return tuya_iot_upgrade_gw(fw, get_file_data_cb, upgrade_notify_cb, NULL);
}

/**
 * @Function: hw_reset_flash_data
 * @Description: hardware reset, erase user data from flash
 * @Input: none
 * @Output: none
 * @Return: none
 * @Others: 
 */
VOID hw_reset_flash_data(VOID)
{
    return;
}

/**
 * @Function: gw_reset_cb
 * @Description: gateway restart callback, app remove the device 
 * @Input: type:gateway reset type
 * @Output: none
 * @Return: none
 * @Others: reset factory clear flash data
 */
VOID gw_reset_cb(IN CONST GW_RESET_TYPE_E type)
{
    PR_DEBUG("gw_reset_cb type:%d",type);
    if(GW_REMOTE_RESET_FACTORY != type) {
        PR_DEBUG("type is GW_REMOTE_RESET_FACTORY");
        return;
    }

    hw_reset_flash_data();
}

/**
 * @Function: dev_obj_dp_cb
 * @Description: obj dp info cmd callback, tuya cloud dp(data point) received
 * @Input: dp:obj dp info
 * @Output: none
 * @Return: none
 * @Others: app send data by dpid  control device stat
 */
VOID dev_obj_dp_cb(IN CONST TY_RECV_OBJ_DP_S *dp)
{
    PR_DEBUG("dp->cid:%s dp->dps_cnt:%d",dp->cid,dp->dps_cnt);
    UCHAR_T i = 0;

    for(i = 0;i < dp->dps_cnt;i++) {
        //deal_dp_proc(&(dp->dps[i]));
        dev_report_dp_json_async(get_gw_cntl()->gw_if.id, dp->dps, dp->dps_cnt);
    }
}

/**
 * @Function: dev_raw_dp_cb
 * @Description: raw dp info cmd callback, tuya cloud dp(data point) received (hex data)
 * @Input: dp: raw dp info
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID dev_raw_dp_cb(IN CONST TY_RECV_RAW_DP_S *dp)
{
    PR_DEBUG("raw data dpid:%d",dp->dpid);
    PR_DEBUG("recv len:%d",dp->len);
#if 1 
    INT_T i = 0;
    for(i = 0;i < dp->len;i++) {
        PR_DEBUG_RAW("%02X ",dp->data[i]);
    }
#endif
    PR_DEBUG_RAW("\n");
    PR_DEBUG("end");
    return;
}

/**
 * @Function: dev_dp_query_cb
 * @Description: dp info query callback, cloud or app actively query device info
 * @Input: dp_qry: query info
 * @Output: none
 * @Return: none
 * @Others: none
 */
STATIC VOID dev_dp_query_cb(IN CONST TY_DP_QUERY_S *dp_qry) 
{
    PR_NOTICE("Recv DP Query Cmd");

    hw_report_all_dp_status();
}

/**
 * @Function: wf_nw_status_cb
 * @Description: tuya-sdk network state check callback
 * @Input: stat: curr network status
 * @Output: none
 * @Return: none
 * @Others: none
 */
VOID wf_nw_status_cb(IN CONST GW_WIFI_NW_STAT_E stat)
{
    PR_NOTICE("wf_nw_status_cb,wifi_status:%d", stat);
    wifi_state_led_reminder(stat);

    if(stat == STAT_AP_STA_CONN || stat >= STAT_STA_CONN) {
        hw_report_all_dp_status();
    }
}

/**
 * @Function: device_init
 * @Description: device initialization process 
 * @Input: none
 * @Output: none
 * @Return: OPRT_OK: success  Other: fail
 * @Others: none
 */
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

    op_ret = tuya_iot_wf_soc_dev_init_param(WIFI_WORK_MODE_SEL, WF_START_SMART_FIRST, &wf_cbs, NULL, PRODECT_ID, DEV_SW_VERSION);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_wf_soc_dev_init_param error,err_num:%d",op_ret);
        return op_ret;
    }

    op_ret = tuya_iot_reg_get_wf_nw_stat_cb(wf_nw_status_cb);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_reg_get_wf_nw_stat_cb is error,err_num:%d",op_ret);
        return op_ret;
    }
	
	myInit();

    return op_ret;
}
