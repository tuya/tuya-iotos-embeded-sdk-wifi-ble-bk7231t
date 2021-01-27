/***********************************************************
*  File: tuya_main.c
*  Author: nzy
*  Date: 20171012
***********************************************************/
#include "tuya_os_adapter.h"
#include "tuya_iot_wifi_api.h"
#include "mf_test.h"
#include "tuya_uart.h"
#include "tuya_gpio.h"
#include "gw_intf.h"
#include "wf_basic_intf.h"
#include "BkDriverUart.h"
#include "uni_log.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#define TEST_SSID "tuya_mdev_test1"
#define WF_SSID_LEN 32
#define UG_PKG_HEAD 0x55aa55aa
#define UG_PKG_TAIL 0xaa55aa55
#define UG_START_ADDR 0x132000   //892k  0x101000------0x1FE000
#define OPRT_WR_FLASH_ERROR (-1005)
#define RT_IMG_WR_UNIT 512

/***********************************************************
*************************variable define********************
***********************************************************/
typedef enum {
    UGS_RECV_HEADER = 0,
    UGS_RECV_IMG_DATA,
    UGS_FINISH
}UG_STAT_E;
typedef struct
{
    u32 header_flag;//0xaa55aa55
    char sw_version[12];//sofrware version
    u32 bin_len;
    u32 bin_sum;
    u32 head_sum;//header_flag + sw_version + bin_len + bin_sum
    u32 tail_flag;//0x55aa55aa
}UPDATE_FILE_HDR_S;
typedef struct {
    UPDATE_FILE_HDR_S file_header;
    u32 flash_addr;
    u32 start_addr;
    u32 recv_data_cnt;
    UG_STAT_E stat;
}UG_PROC_S;

typedef VOID (*APP_PROD_CB)(BOOL_T flag, CHAR_T rssi);
typedef VOID (*after_mf_test_cb)(VOID);
typedef VOID (*SET_OTA_FINISH_NOTIFY)(VOID);
STATIC APP_PROD_CB app_prod_test = NULL;
STATIC GW_WF_CFG_MTHD_SEL gwcm_mode = GWCM_OLD;
STATIC CHAR_T prod_ssid_name[WF_SSID_LEN + 1] = TEST_SSID;
static UG_PROC_S *ug_proc = NULL;

after_mf_test_cb pre_app_cb = NULL;
SET_OTA_FINISH_NOTIFY _ota_finish_notify = NULL;
MF_USER_PRODUCT_TEST_CB user_product_test_cb = NULL;
MF_USER_CALLBACK user_enter_mf_cb = NULL;
/***********************************************************
*************************function define********************
***********************************************************/
extern VOID app_init(VOID);
extern VOID pre_device_init(VOID);
extern OPERATE_RET device_init(VOID);
extern BOOL_T gpio_test(IN CONST CHAR_T *in, OUT CHAR_T *out);
extern VOID mf_user_callback(VOID);
extern OPERATE_RET mf_user_product_test(USHORT_T cmd,UCHAR_T *data, UINT_T len, OUT UCHAR_T **ret_data,OUT USHORT_T *ret_len);
extern VOID user_enter_mf_callback(VOID);
extern void extended_app_waiting_for_launch(void);
extern TY_GPIO_PORT_E swith_ctl_port;
STATIC VOID __gw_ug_inform_cb(INOUT BOOL_T *handled, IN CONST FW_UG_S *fw);
STATIC OPERATE_RET __gw_upgrage_process_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len,IN CONST UINT_T offset,\
                                                      IN CONST BYTE_T *data,IN CONST UINT_T len,OUT UINT_T *remain_len, IN PVOID_T pri_data);
STATIC VOID __gw_upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data);

STATIC OPERATE_RET __mf_gw_upgrade_notify_cb(VOID);
STATIC VOID __mf_gw_ug_inform_cb(UINT_T file_size, UINT_T file_crc);

#define TY_UART TY_UART1
STATIC VOID __tuya_mf_uart_init(UINT_T baud,UINT_T bufsz)
{
    ty_uart_init(TY_UART,baud,TYWL_8B,TYP_NONE,TYS_STOPBIT1,bufsz,TRUE);
}
STATIC VOID __tuya_mf_uart_free(VOID)
{
    ty_uart_free(TY_UART);
}

STATIC VOID __tuya_mf_send(IN CONST BYTE_T *data,IN CONST UINT_T len)
{
    ty_uart_send_data(TY_UART,data,len);
}

STATIC UINT_T __tuya_mf_recv(OUT BYTE_T *buf,IN CONST UINT_T len)
{
    return ty_uart_read_data(TY_UART,buf,len);
}
extern OPERATE_RET gw_cfg_flash_reset_fac(VOID);
STATIC BOOL_T scan_test_ssid(VOID)
{
    BOOL_T mf_close;
    mf_close = wd_mf_test_close_if_read();
    if (TRUE == mf_close) {
        PR_NOTICE("have actived over 15 min, not enter mf_init");
        return OPRT_OK;
    }
    OPERATE_RET op_ret = OPRT_OK;
    GW_WORK_STAT_MAG_S read_gw_wsm ;
    
    memset(&read_gw_wsm, 0, sizeof(GW_WORK_STAT_MAG_S));
    op_ret = wd_gw_wsm_read(&read_gw_wsm);

    if((gwcm_mode == GWCM_OLD_PROD ) || (gwcm_mode == GWCM_LOW_POWER_AUTOCFG) || (gwcm_mode == GWCM_SPCL_AUTOCFG)) {
        if(read_gw_wsm.nc_tp >= GWNS_TY_SMARTCFG) {
            return false;
        }
    } else if (gwcm_mode == GWCM_SPCL_MODE || gwcm_mode == GWCM_LOW_POWER) {
        if(read_gw_wsm.nc_tp >= GWNS_UNCFG_SMC) {
            return false;
        }
    } else {
        ;
    }

    wf_wk_mode_set(WWM_STATION);
    AP_IF_S *ap = NULL;
    BOOL_T flag = TRUE;
    PR_NOTICE("current product ssid name:%s", prod_ssid_name);
    op_ret = wf_assign_ap_scan(prod_ssid_name, &ap);//lql
    wf_station_disconnect();
    if(OPRT_OK != op_ret) {
        PR_DEBUG("wf_assign_ap_scan failed(%d)",op_ret);
        return FALSE;
    }
    //check if has authorized
    op_ret = wd_gw_base_if_read(&(get_gw_cntl()->gw_base));
    if(OPRT_OK != op_ret) {
        PR_DEBUG("read flash err");
        flag = FALSE;
    }
    // gateway base info verify
#if TY_SECURITY_CHIP
    if(!get_gw_cntl()->gw_base.has_auth) {
#else
    if(0 == get_gw_cntl()->gw_base.auth_key[0] || \
    0 == get_gw_cntl()->gw_base.uuid[0]) {
#endif
        PR_DEBUG("please write uuid and auth_key first");
        flag = FALSE;
    }

    if(app_prod_test) {
	    PR_DEBUG("gw cfg flash info reset factory!");
        GW_WORK_STAT_MAG_S *wsm = (GW_WORK_STAT_MAG_S *)Malloc(SIZEOF(GW_WORK_STAT_MAG_S));
        if(NULL != wsm){
            memset(wsm,0,SIZEOF(GW_WORK_STAT_MAG_S));
            op_ret = wd_gw_wsm_write(wsm);
            if(OPRT_OK != op_ret){
                PR_ERR("wd_gw_wsm_write err:%d!", op_ret);
            }
            Free(wsm);
        }        
		
        app_prod_test(flag, ap->rssi);
    }
    return TRUE;
}

void app_cfg_set(IN CONST GW_WF_CFG_MTHD_SEL mthd, APP_PROD_CB callback)
{
    app_prod_test = callback;
    gwcm_mode = mthd;
}

TIMER_ID tm_monitor;
#define     APP_MQTT_MONITOR_TIMER          1          //s

OPERATE_RET tuya_bt_close(VOID)
{ 
    extern bool if_start_adv_after_disconnect;
    uint8_t app_status = 0;
    app_status = appm_get_app_status();
    bk_printf("!!!!!!!!!!tuya_bt_close:%d\r\n", app_status);
    if(app_status == 3)
    {
        appm_stop_advertising();
    }
    if(app_status == 9)
    {
        if_start_adv_after_disconnect = false;
        appm_disconnect(0x13);
    }
	
    return OPRT_OK;
}

STATIC VOID __mqtt_monitor_tm_cb(UINT_T timerID,PVOID_T pTimerArg)
{
    STATIC GW_WIFI_NW_STAT_E last_nw_stat = 0xff;    
    GW_WIFI_NW_STAT_E cur_nw_stat = 0;
    STATIC BYTE_T check_cnt = 0;
    
    OPERATE_RET op_ret = OPRT_OK;
    op_ret = get_wf_gw_nw_status(&cur_nw_stat);
    if(OPRT_OK != op_ret) {
        PR_ERR("get_wf_gw_nw_status error:%d",op_ret);
        sys_start_timer(tm_monitor,APP_MQTT_MONITOR_TIMER*1000,TIMER_ONCE);
        return;
    }
    
    if(cur_nw_stat != last_nw_stat) {
        PR_DEBUG("wifi netstat changed to:%d  -->>", cur_nw_stat);
                
        if(last_nw_stat != STAT_CLOUD_CONN && cur_nw_stat == STAT_CLOUD_CONN) {
            //上一状态未连接MQTT，当前已经连接MQTT
            check_cnt ++;
            if(check_cnt >= 3) {
                last_nw_stat = cur_nw_stat;
                check_cnt = 0;
            
                //SCHAR_T rssi = 0;
                //op_ret = wf_station_get_conn_ap_rssi(&rssi);

                //if(OPRT_OK == op_ret && rssi < -75) {
                    //PR_DEBUG("rssi:%d", rssi);
                    //关闭蓝牙
                    PR_DEBUG("++++++++++++++++ disable bluetooth ++++++++++");
                    tuya_bt_close();
                //}
                
                sys_start_timer(tm_monitor, APP_MQTT_MONITOR_TIMER * 1000, TIMER_ONCE);
            } else {
                sys_start_timer(tm_monitor, 2 * APP_MQTT_MONITOR_TIMER * 1000, TIMER_ONCE);
            }
            
            return;
        } 
        else if(last_nw_stat == STAT_CLOUD_CONN && cur_nw_stat != STAT_CLOUD_CONN) {
            //上一状态连接MQTT，当前已经未连接MQTT
            check_cnt ++;
            if(check_cnt >= 3) {
                //打开蓝牙
                last_nw_stat = cur_nw_stat;
                check_cnt = 0;
                
                PR_DEBUG("++++++++++++++++ enable bluetooth ++++++++++");
                tuya_bt_start_adv();
                sys_start_timer(tm_monitor, APP_MQTT_MONITOR_TIMER * 1000, TIMER_ONCE);
            }
            else {
                sys_start_timer(tm_monitor, 2 * APP_MQTT_MONITOR_TIMER * 1000, TIMER_ONCE);
            }
            
            return;
        }
        else
            last_nw_stat = cur_nw_stat;
            
        PR_DEBUG("wifi netstat changed to:%d  <<--", cur_nw_stat);
    }

    check_cnt = 0;
    sys_start_timer(tm_monitor,APP_MQTT_MONITOR_TIMER*1000,TIMER_ONCE);
}

STATIC VOID __app_mqtt_monitor(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;
    op_ret = sys_add_timer(__mqtt_monitor_tm_cb, NULL, &(tm_monitor));
    if(OPRT_OK != op_ret) {
        PR_ERR("sys_add_timer tm_monitor error");
        return;
    }
    
    sys_start_timer(tm_monitor,APP_MQTT_MONITOR_TIMER*1000,TIMER_ONCE);
}

/***********************************************************
*  Function: user_main 
*  Input: none
*  Output: none
*  Return: none
***********************************************************/
void user_main(void)
{
    OPERATE_RET op_ret = OPRT_OK;
    
    TY_INIT_PARAMS_S init_param = {0};
    init_param.init_db = FALSE;
    strcpy(init_param.sys_env, "BK7231S_2M");
    op_ret = tuya_iot_init_params(NULL, &init_param);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_init err:%d",op_ret);
        return;
    }
#if TY_SECURITY_CHIP
    SEChip_I2C_Init(); //加密芯片 I2C 接口初始化
#endif

    pre_device_init();
    tuya_iot_kv_flash_init(NULL);
	if(pre_app_cb)
	{
		pre_app_cb();
	}
    PR_NOTICE("**********[%s] [%s] compiled at %s %s**********", APP_BIN_NAME, USER_SW_VER, __DATE__,__TIME__);

	extended_app_waiting_for_launch();

    // to add prodect test code
    mf_reg_gw_ug_cb(__mf_gw_ug_inform_cb, __gw_upgrage_process_cb, __mf_gw_upgrade_notify_cb);
    MF_IMPORT_INTF_S intf = {
        .uart_init = __tuya_mf_uart_init,
        .uart_free = __tuya_mf_uart_free,
        .uart_send = __tuya_mf_send,
        .uart_recv = __tuya_mf_recv,
        .gpio_test = gpio_test,
        .mf_user_product_test = user_product_test_cb,
        .user_callback = mf_user_callback,
        .user_enter_mf_callback = user_enter_mf_cb,
    };

    BOOL_T mf_close = FALSE;
    mf_close = wd_mf_test_close_if_read();                                  //无法进入产测，请手工注释此行
    if(TRUE != mf_close) {
        op_ret = mf_init(&intf,APP_BIN_NAME,USER_SW_VER,TRUE);
        if(OPRT_OK != op_ret) {
            PR_ERR("mf_init err:%d",op_ret);
            return;
        }
    } else {
        PR_NOTICE("have actived over 15 min, not enter mf_init");
    }
    
    __tuya_mf_uart_free();
    PR_NOTICE("mf_init succ"); 

    // register gw upgrade inform callback
    iot_register_pre_gw_ug_cb(__gw_ug_inform_cb);

    app_init();
    PR_DEBUG("gwcm_mode %d",gwcm_mode);
    if(gwcm_mode != GWCM_OLD) {
        PR_DEBUG("low_power select");
        if(true == scan_test_ssid()) {
            PR_DEBUG("prodtest");
            return;
        }
        PR_DEBUG("no tuya_mdev_test1!");
        op_ret = device_init();
        if(OPRT_OK != op_ret) {
            PR_ERR("device_init error:%d",op_ret);
            return;
        }
    }else {
        PR_DEBUG("device_init in");
        op_ret = device_init();
        if(OPRT_OK != op_ret) {
            PR_ERR("device_init err:%d",op_ret);
            return;
        }
    }

    //bk7231t_mqtt监控服务
    __app_mqtt_monitor();
}

extern OPERATE_RET tuya_hal_flash_set_protect(IN CONST BOOL_T enable);
// mf gateway upgrade start 
VOID __mf_gw_ug_inform_cb(UINT_T file_size, UINT_T file_crc)
{
	#if 1
    ug_proc = Malloc(SIZEOF(UG_PROC_S));
    if(NULL == ug_proc) {
        PR_ERR("malloc err");
        return;
    }
    memset(ug_proc,0,SIZEOF(UG_PROC_S));
	#endif
}

// gateway upgrade start
STATIC VOID __gw_ug_inform_cb(INOUT BOOL_T *handled, IN CONST FW_UG_S *fw)
{
	OPERATE_RET op_ret = OPRT_OK;

//	image_seq_t seq;
	if(fw->tp != DEV_NM_ATH_SNGL)
    {
        *handled = FALSE;
        return;
    }
    *handled = TRUE;

	PR_DEBUG("ota_notify_data_begin enter");
	ug_proc = Malloc(SIZEOF(UG_PROC_S));
    if(NULL == ug_proc) {
        PR_ERR("malloc err");
        return;
    }
    memset(ug_proc,0,SIZEOF(UG_PROC_S));

    op_ret = tuya_iot_upgrade_gw(fw,__gw_upgrage_process_cb,__gw_upgrade_notify_cb,NULL);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_iot_upgrade_gw err:%d",op_ret);
        return;
    }
}

#define BUF_SIZE 4096
// mf gateway upgrade result notify
OPERATE_RET __mf_gw_upgrade_notify_cb(VOID)
{
	#if 1
    u32 ret = 0,i = 0,k = 0,rlen = 0,addr = 0;
    u32 flash_checksum=0;
    u8 *pTempbuf;
    pTempbuf = Malloc(BUF_SIZE);
    if(pTempbuf == NULL) {
        PR_ERR("Malloc failed!!");
        return OPRT_MALLOC_FAILED;
    }

    for(i = 0; i < ug_proc->file_header.bin_len; i += BUF_SIZE) {
        rlen  = ((ug_proc->file_header.bin_len - i) >= BUF_SIZE) ? BUF_SIZE : (ug_proc->file_header.bin_len - i);
        addr = ug_proc->start_addr + i;
        tuya_hal_flash_read(addr, pTempbuf, rlen);
        for(k = 0; k < rlen; k++) {
            flash_checksum += pTempbuf[k];
        }
    }
    
    if(flash_checksum != ug_proc->file_header.bin_sum) {
        PR_ERR("verify_ota_checksum err  checksum(0x%x)  file_header.bin_sum(0x%x)",flash_checksum,ug_proc->file_header.bin_sum);
        tuya_hal_flash_set_protect(FALSE);
        tuya_hal_flash_erase(UG_START_ADDR,0x1000);            //擦除OTA区的首地址4K
        tuya_hal_flash_set_protect(TRUE);
    	Free(pTempbuf);
    	Free(ug_proc);
        return OPRT_COM_ERROR;
    }
    
    PR_NOTICE("the gateway upgrade success");
    Free(pTempbuf);
    Free(ug_proc);
    //PR_DEBUG("the gateway upgrade succes,now go to reset!!");
	#endif
    return OPRT_OK;
}

// gateway upgrade result notify
STATIC VOID __gw_upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
    if(OPRT_OK == download_result) { // update success
        // verify 
        u32 ret = 0,i = 0,k = 0,rlen = 0,addr = 0;
        u32 flash_checksum=0;
        u8 *pTempbuf;
        pTempbuf = Malloc(BUF_SIZE);
        if(pTempbuf == NULL){
            PR_ERR("Malloc failed!!");
            return;
        }
        for(i = 0; i < ug_proc->file_header.bin_len; i += BUF_SIZE){
            rlen  = ((ug_proc->file_header.bin_len - i) >= BUF_SIZE) ? BUF_SIZE : (ug_proc->file_header.bin_len - i);
            addr = ug_proc->start_addr + i;
            tuya_hal_flash_read(addr, pTempbuf, rlen);
            for(k = 0; k < rlen; k++){
                flash_checksum += pTempbuf[k];
            }
        }
        if(flash_checksum != ug_proc->file_header.bin_sum){
            PR_ERR("verify_ota_checksum err  checksum(0x%x)  file_header.bin_sum(0x%x)",flash_checksum,ug_proc->file_header.bin_sum);
        	
            tuya_hal_flash_set_protect(FALSE);
        	tuya_hal_flash_erase(UG_START_ADDR,0x1000);            //擦除OTA区的首地址4K
            tuya_hal_flash_set_protect(TRUE);
            
        	Free(pTempbuf);
            return;
        }
        PR_NOTICE("the gateway upgrade success");
        Free(pTempbuf);
        Free(ug_proc);
        //os_printf("the gateway upgrade succes,now go to reset!!\r\n");
        //OTA完成回调
        if(_ota_finish_notify)
            _ota_finish_notify();
        
        tuya_hal_system_reset();
        return;
    }else {
        tuya_hal_flash_set_protect(FALSE);
        tuya_hal_flash_erase(UG_START_ADDR,0x1000);            //擦除OTA区的首地址4K
        tuya_hal_flash_set_protect(TRUE);
        Free(ug_proc);
        PR_ERR("the gateway upgrade failed");
    }
}

// gateway upgrade process
STATIC OPERATE_RET __gw_upgrage_process_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len,IN CONST UINT_T offset,\
                                                      IN CONST BYTE_T *data,IN CONST UINT_T len,OUT UINT_T *remain_len, IN PVOID_T pri_data)
{
    u32 sum_tmp = 0,i=0;
    u32 write_len = 0;
    switch(ug_proc->stat) {
        case UGS_RECV_HEADER: {
            if(len < SIZEOF(UPDATE_FILE_HDR_S)) {
                *remain_len = len;
                break;
            }
            //memcpy((unsigned char*)&ug_proc->file_header,data,SIZEOF(UPDATE_FILE_HDR_S));
            
            ug_proc->file_header.tail_flag = (data[28]<<24)|(data[29]<<16)|(data[30]<<8)|data[31];
            ug_proc->file_header.head_sum = (data[24]<<24)|(data[25]<<16)|(data[26]<<8)|data[27];
            ug_proc->file_header.bin_sum = (data[20]<<24)|(data[21]<<16)|(data[22]<<8)|data[23];
            ug_proc->file_header.bin_len = (data[16]<<24)|(data[17]<<16)|(data[18]<<8)|data[19];
            for(i=0;i<12;i++) {
                ug_proc->file_header.sw_version[i] = data[4 + i];
            }
            
            ug_proc->file_header.header_flag = (data[0]<<24)|(data[1]<<16)|(data[2]<<8)|data[3];
            
            for(i = 0;i < SIZEOF(UPDATE_FILE_HDR_S) - 8;i++) {
                sum_tmp += data[i];
            } 
            PR_NOTICE("header_flag(0x%x) tail_flag(0x%x) head_sum(0x%x-0x%x) bin_sum(0x%x)",ug_proc->file_header.header_flag,ug_proc->file_header.tail_flag,ug_proc->file_header.head_sum,sum_tmp,ug_proc->file_header.bin_sum);
            //PR_DEBUG("sw_version:%s  bin_len = 0x%x   bin_sum = 0x%x\r\n",ug_proc->file_header.sw_version, ug_proc->file_header.bin_len,ug_proc->file_header.bin_sum);
            if((ug_proc->file_header.header_flag !=  UG_PKG_HEAD) || (ug_proc->file_header.tail_flag !=  UG_PKG_TAIL) || (ug_proc->file_header.head_sum != sum_tmp )) {
                memset(&ug_proc->file_header, 0, SIZEOF(UPDATE_FILE_HDR_S));
                PR_ERR("bin_file data header err: header_flag(0x%x) tail_flag(0x%x) bin_sum(0x%x) get_sum(0x%x)",ug_proc->file_header.header_flag,ug_proc->file_header.tail_flag,ug_proc->file_header.head_sum,sum_tmp);
//                return OPRT_OTA_BIN_CHECK_ERROR;
                return OPRT_COM_ERROR;
            }
            if(ug_proc->file_header.bin_len >= (664 * 1024)) {
                //ug文件最大为664K
                memset(&ug_proc->file_header, 0, SIZEOF(UPDATE_FILE_HDR_S));
                PR_ERR("bin_file too large.... %d", ug_proc->file_header.bin_len);
//                return OPRT_OTA_BIN_SIZE_ERROR;
                return OPRT_COM_ERROR;
            }
            
            PR_DEBUG("sw_ver:%s",ug_proc->file_header.sw_version);
            PR_DEBUG("get right bin_file_header!!!");
            ug_proc->start_addr = UG_START_ADDR;
            ug_proc->flash_addr = ug_proc->start_addr;
            ug_proc->stat = UGS_RECV_IMG_DATA;
            ug_proc->recv_data_cnt = 0;
            *remain_len = len - SIZEOF(UPDATE_FILE_HDR_S);
            
            tuya_hal_flash_set_protect(FALSE);
            tuya_hal_flash_erase(ug_proc->start_addr,ug_proc->file_header.bin_len);
            tuya_hal_flash_set_protect(TRUE);
            
        	PR_DEBUG("erase success  remain_len: %d  file size: %d!!!!!",*remain_len,ug_proc->file_header.bin_len);
        } 
        break;
        
        case UGS_RECV_IMG_DATA: {    //dont have set lock for flash! 
//            PR_DEBUG("ug_proc->recv_data_cnt : %d,len : %d",ug_proc->recv_data_cnt,len);
            *remain_len = len;
            if((len < RT_IMG_WR_UNIT) && (ug_proc->recv_data_cnt <= (ug_proc->file_header.bin_len - RT_IMG_WR_UNIT))) {
                break;
            }
            write_len = len;
            while(write_len >= RT_IMG_WR_UNIT) {
                tuya_hal_flash_set_protect(FALSE);
                if(tuya_hal_flash_write(ug_proc->flash_addr, &data[len - write_len], RT_IMG_WR_UNIT)) {
                    tuya_hal_flash_set_protect(TRUE);
                    PR_ERR("Write sector failed");
                    return OPRT_WR_FLASH_ERROR;
                }
                tuya_hal_flash_set_protect(TRUE);
                ug_proc->flash_addr += RT_IMG_WR_UNIT;
                ug_proc->recv_data_cnt += RT_IMG_WR_UNIT;
                write_len -= RT_IMG_WR_UNIT; 
                *remain_len = write_len;
            }
            if((ug_proc->recv_data_cnt > (ug_proc->file_header.bin_len - RT_IMG_WR_UNIT)) \
				&& (write_len >= (ug_proc->file_header.bin_len - ug_proc->recv_data_cnt))) {//last 512 (write directly when get data )
                tuya_hal_flash_set_protect(FALSE);
                if(tuya_hal_flash_write(ug_proc->flash_addr, &data[len - write_len], write_len)) {
                    tuya_hal_flash_set_protect(TRUE);
                    PR_ERR("Write sector failed");
                    return OPRT_WR_FLASH_ERROR;
                }
                tuya_hal_flash_set_protect(TRUE);
                PR_DEBUG("\r\nwrite success!!!");
                ug_proc->flash_addr += write_len;
                ug_proc->recv_data_cnt += write_len;
                write_len = 0;
                *remain_len = 0;
            }
            PR_DEBUG("rcv_cnt:%d，remain_len:%d",ug_proc->recv_data_cnt,*remain_len);
            if(ug_proc->recv_data_cnt >= ug_proc->file_header.bin_len) {
                ug_proc->stat = UGS_FINISH;
                break;
            }
        }
        break;

        case UGS_FINISH: {
            *remain_len = 0;
        }
        break;
    }

    return OPRT_OK;
}


void set_prod_ssid(CHAR_T *ssid)
{
    if (strlen(ssid) > WF_SSID_LEN) {
        PR_ERR("ssid len over max value 32");
        return;
    }
    memset(prod_ssid_name, 0, sizeof(prod_ssid_name));
    strncpy(prod_ssid_name, ssid, WF_SSID_LEN);
    return;
}

void pre_app_cfg_set(after_mf_test_cb callback)
{    
	pre_app_cb = callback;
}


void set_ota_finish_notify_cb(SET_OTA_FINISH_NOTIFY callback)
{
    _ota_finish_notify = callback;
}

void set_user_product_test_cb(MF_USER_PRODUCT_TEST_CB callback)
{
    user_product_test_cb = callback;
}

void set_mf_enter_cb(MF_USER_CALLBACK callback)
{
    user_enter_mf_cb = callback;
}



