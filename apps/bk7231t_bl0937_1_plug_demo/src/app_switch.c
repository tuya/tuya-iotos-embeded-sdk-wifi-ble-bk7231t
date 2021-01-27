/**
 * @File: app_switch.c
 * @Author: caojq
 * @Last Modified time: 2020-07-29
 * @Description: 电量统计开关操作
 */
#include "app_switch.h"
#include "tuya_device.h"
#include "uf_file.h"
#include "cJSON.h"
#include "uni_time_queue.h"
#include "app_dltj.h"
#include "gw_intf.h"
#include "tuya_iot_com_api.h"
#include "uni_log.h"
#include "tuya_iot_wifi_api.h"
/***********************************************************
*************************micro define***********************
***********************************************************/
#define _APP_SWITCH_GLOBAL
#define DEFAULT_KEY_RST_TIME_S 3
#define COUNTDOWN_REPT_TIME 30
#define POWER_STAT_KEY "relay_stat_key"
#define PT_END_KEY "pt_end_key"
#define KEY_TIMER_MS 20                         //按键轮询时间
#define PRESS_LONG_TIME 3000                    //按键长按时间

/***********************************************************
*************************variable define********************
************************************************************/
STATIC BOOL_T wf_dir_rl_en = FALSE;     //允许wifi灯指示继电器
STATIC BOOL_T pt_key_en = TRUE;      //允许产测按键使用
STATIC UCHAR_T pt_curr_ch = 0;
STATIC CTRL_SW pt_ch_deal = CTRL_SW_OPEN;

STATIC TIMER_ID countdown_timer;
STATIC TIMER_ID save_stat_timer;
STATIC TIMER_ID enter_pt_timer;
STATIC TIMER_ID end_pt_timer;
STATIC TIMER_ID pt_total_bt_timer;

typedef enum 
{
    SW_DP = 1,
    SW_CD_DP = 9,
}DP_ID_E;

CTRL_CHANNEL_CONFIG ctrl_channels[] = {
    { .led = { .type = IO_DRIVE_LEVEL_NOT_EXIST },
        .relay = { .type = IO_DRIVE_LEVEL_HIGH, .pin = TY_GPIOA_26 },
        .dpid = SW_DP,
        .cd_dpid = SW_CD_DP,
        .init_ch_stat = INIT_CH_CLOSE,
        .cd_sec = 0 }
};

HW_TABLE g_hw_table = {
    .channels = ctrl_channels,
    .wf_led = {
        .wfl_io = { .type = IO_DRIVE_LEVEL_LOW, .pin = TY_GPIOA_6 },
        .press_time = DEFAULT_KEY_RST_TIME_S,
        .wfl_cs = WFL_DIR_RL, /* wifi灯连接时指示状态 */
        .wfl_ucs = WFL_DIR_RL,
        .wcm_mode = GWCM_LOW_POWER,
    },
    .tch = {
        .tled = { .type = IO_DRIVE_LEVEL_NOT_EXIST },
    }
};

/***********************************************************
*************************function define********************
***********************************************************/
STATIC VOID pt_total_bt_timer_cb(UINT_T timerID,PVOID_T pTimerArg);
STATIC VOID pt_hw_key_process(TY_GPIO_PORT_E gpio_no,PUSH_KEY_TYPE_E type,INT_T cnt);
STATIC VOID hw_key_process(TY_GPIO_PORT_E gpio_no,PUSH_KEY_TYPE_E type,INT_T cnt);
STATIC VOID save_stat_timer_cb(UINT_T timerID,PVOID_T pTimerArg);
STATIC VOID end_pt_timer_cb(UINT_T timerID,PVOID_T pTimerArg);
STATIC VOID enter_pt_timer_cb(UINT_T timerID,PVOID_T pTimerArg);
STATIC OPERATE_RET read_saved_stat(VOID);
STATIC OPERATE_RET upload_all_switch_state(VOID);//放到上电同步函数之中
STATIC VOID set_hw_total_led(IN BOOL_T state);
STATIC VOID wfl_direct_relay(VOID);
STATIC OPERATE_RET upload_switch_countdown(IN UCHAR_T channel_num);
STATIC OPERATE_RET upload_switch_state(IN UCHAR_T channel_num);
STATIC VOID channel_event_deal(IN UCHAR_T channel_num,IN CHAN_EVENT_TYPE event_type);
STATIC VOID app_send_countdown_deal(IN UINT_T cd_dp_id,IN INT_T cd_secs);//放到app下发倒计时dp点处理语句之中
STATIC VOID app_send_press_deal(IN UINT_T ch_dp_id,IN BOOL_T ch_state);//放到app下发开关状态dp点处理语句之中
STATIC VOID total_channel_event_deal(IN BOOL_T on_or_off);

VOID deal_dp_proc(IN CONST TY_OBJ_DP_S *root)
{
    UCHAR_T dpid;

    dpid = root->dpid;
    PR_DEBUG("dpid:%d",dpid);

    switch(dpid) {
        case SW_DP:
            PR_DEBUG("set relay_stat :%d",root->value.dp_bool);
            if(root->value.dp_bool) {
                app_send_press_deal(dpid, TRUE);
            }else {
                app_send_press_deal(dpid, FALSE);
            }
        break;

        case SW_CD_DP:
            PR_DEBUG("set relay_cd_sec:%d",root->value.dp_value);
            app_send_countdown_deal(dpid,root->value.dp_value);
        break;

        default:
        break;
    }
    return;

}

//判断有任何开关处于开启/关闭状态，如果是返回true。
BOOL_T judge_any_sw(IN BOOL_T on_or_off)
{
    BOOL_T ch_state;
    UCHAR_T i;
    if(on_or_off){
        ch_state = FALSE;
        for(i = 0;i < g_hw_table.channel_num; i++){
            ch_state = ch_state || g_hw_table.channels[i].stat;
        }
        return ch_state;
    }else{
        ch_state = TRUE;
        for(i = 0; i < g_hw_table.channel_num; i++){
            ch_state = ch_state && g_hw_table.channels[i].stat;
        }
        return !ch_state;
    }
}

//按键短按处理，三种按键事件（优先全开，优先全关，单通道取反）
STATIC VOID key_short_press_deal(IN INT_T key_num,IN KEY_TYPE key_type)
{
    UCHAR_T i;
    if(key_type == KEY_ALL_PRIOR_OPEN){
        BOOL_T ch_state = judge_any_sw(FALSE);//若有一个通道关闭，则全开
        total_channel_event_deal(ch_state);
    }else if(key_type == KEY_ALL_PRIOR_CLOSE){
        BOOL_T ch_state = judge_any_sw(TRUE);//若有一个通道开通，则全关
        total_channel_event_deal(!ch_state);
    }else{
        for(i = 0;i < g_hw_table.channel_num; i++){
            if(key_num == g_hw_table.channels[i].button.port){
                channel_event_deal(i,CHAN_EVENT_TOGGLE);
                return;
            }
        }
    }
    return;
}

//app短按开关处理
VOID app_send_press_deal(IN         UINT_T ch_dp_id,IN BOOL_T ch_state)
{
    UCHAR_T i;
    PR_DEBUG("app press deal...");
    for(i = 0; i<g_hw_table.channel_num; i++) {
        if(ch_dp_id == g_hw_table.channels[i].dpid) {
            if(ch_state != g_hw_table.channels[i].stat) {
                channel_event_deal(i,ch_state ? CHAN_EVENT_OPEN : CHAN_EVENT_CLOSE);
            }else {
                upload_switch_state(i);
            }
            return;
        }
    }
    return;
}

//app倒计时下发处理
VOID app_send_countdown_deal(IN UINT_T cd_dp_id,IN INT_T cd_secs)
{
    UCHAR_T i;
    for(i = 0; i<g_hw_table.channel_num; i++){
        if(cd_dp_id == g_hw_table.channels[i].cd_dpid){
            g_hw_table.channels[i].cd_sec = cd_secs;
            upload_switch_countdown(i);//如果不上传倒计时时间APP是否会开始计数？
            sys_start_timer(countdown_timer, 1000, TIMER_CYCLE);
            return;
        }
    }
    return;
}

//对通道继电器的纯硬件操作
STATIC VOID set_hw_relay(IN UCHAR_T channel_num,IN BOOL_T state)
{
    if(g_hw_table.channels[channel_num].relay.type == IO_DRIVE_LEVEL_HIGH){
        tuya_gpio_write(g_hw_table.channels[channel_num].relay.pin,(state ? OL_HIGH :OL_LOW));
    }else if(g_hw_table.channels[channel_num].relay.type == IO_DRIVE_LEVEL_LOW){
        tuya_gpio_write(g_hw_table.channels[channel_num].relay.pin,(state ? OL_LOW :OL_HIGH));
    }else if(g_hw_table.channels[channel_num].relay.type == IO_DRIVE_LEVEL_NOT_EXIST){
        //若通道继电器不由模块GPIO控制，在此处控制
    }
    wfl_direct_relay();
    BOOL_T if_on = judge_any_sw(TRUE);
    set_hw_total_led(if_on);
}

//对通道指示灯的纯硬件操作
STATIC VOID set_hw_led(IN UCHAR_T channel_num,IN BOOL_T state)
{
    if(g_hw_table.channels[channel_num].led.type == IO_DRIVE_LEVEL_HIGH){
        tuya_set_led_light_type(g_hw_table.channels[channel_num].led_handle,\
            (state ? OL_HIGH :OL_LOW), 0,0);
    }else if(g_hw_table.channels[channel_num].led.type == IO_DRIVE_LEVEL_LOW){
        tuya_set_led_light_type(g_hw_table.channels[channel_num].led_handle,\
            (state ? OL_LOW :OL_HIGH), 0,0);
    }else if(g_hw_table.channels[channel_num].led.type == IO_DRIVE_LEVEL_NOT_EXIST){
        //若通道LED不由模块GPIO控制，在此处控制
    }
}

//对wifi灯的纯硬件操作
STATIC VOID set_hw_wifi_led(IN BOOL state)
{
    if(g_hw_table.wf_led.wfl_io.type == IO_DRIVE_LEVEL_HIGH){
        tuya_set_led_light_type(g_hw_table.wf_led.wfl_handle,\
            (state ? OL_HIGH :OL_LOW), 0,0);
    }else if(g_hw_table.wf_led.wfl_io.type == IO_DRIVE_LEVEL_LOW){
        tuya_set_led_light_type(g_hw_table.wf_led.wfl_handle,\
            (state ? OL_LOW :OL_HIGH), 0,0);
    }else if(g_hw_table.wf_led.wfl_io.type == IO_DRIVE_LEVEL_NOT_EXIST){
        //若wifiLED不由模块GPIO控制，在此处控制
    }
}

//对总led灯的纯硬件操作
STATIC VOID set_hw_total_led(IN BOOL_T state)
{
    if(g_hw_table.tch.tled.type == IO_DRIVE_LEVEL_HIGH){
        tuya_set_led_light_type(g_hw_table.tch.tled_handle,\
            (state ? OL_HIGH :OL_LOW), 0,0);
    }else if(g_hw_table.tch.tled.type == IO_DRIVE_LEVEL_LOW){
        tuya_set_led_light_type(g_hw_table.tch.tled_handle,\
            (state ? OL_LOW :OL_HIGH), 0,0);
    }else if(g_hw_table.tch.tled.type == IO_DRIVE_LEVEL_NOT_EXIST){
        //若总LED不由模块GPIO控制，在此处控制
    }
}

//控制某一个通道的 （状态+继电器+指示灯），三者绑定。
VOID ctrl_switch_state(IN UCHAR_T channel_num,IN CTRL_SW ctrl_sw)
{
    if(channel_num < 0 || channel_num >= g_hw_table.channel_num){
        PR_ERR("input channel number error: %d", channel_num);
        return;
    }
    switch(ctrl_sw){
    case CTRL_SW_OPEN:
        g_hw_table.channels[channel_num].stat = TRUE;
    break;
    case CTRL_SW_CLOSE:
        g_hw_table.channels[channel_num].stat = FALSE;
    break;
    case CTRL_SW_TOGGLE:
        g_hw_table.channels[channel_num].stat = !g_hw_table.channels[channel_num].stat;
    break;
    case CTRL_SW_STAT:
    break;
    default:
    break;
    }
    set_hw_relay(channel_num,g_hw_table.channels[channel_num].stat);
    set_hw_led(channel_num,g_hw_table.channels[channel_num].stat);
}

//倒计时定时器回调，在有任何一个通道需要倒计时时开启，都结束后关闭定时器
STATIC VOID countdown_timer_cb(UINT_T timerID,PVOID_T pTimerArg)
{
    UCHAR_T i;
    BOOL_T countdown_state = FALSE;
    //PR_DEBUG("countdown -1s... ");
    for(i = 0; i < g_hw_table.channel_num; i++){
        if(g_hw_table.channels[i].cd_dpid > 0){//若存在倒计时
            if(g_hw_table.channels[i].cd_sec > 0){
                g_hw_table.channels[i].cd_sec--;
                if(g_hw_table.channels[i].cd_sec == 0){
                    channel_event_deal(i, CHAN_EVENT_TOGGLE);
                }
                if(g_hw_table.channels[i].cd_sec % COUNTDOWN_REPT_TIME == 0){
                    upload_switch_countdown(i);
                }
                countdown_state = countdown_state || g_hw_table.channels[i].cd_sec;
            }else{
                g_hw_table.channels[i].cd_sec = 0;
            }
            PR_DEBUG("ch[%d] sec:%d",i,g_hw_table.channels[i].cd_sec);
        }
    }
    if(!countdown_state){
        if(IsThisSysTimerRun(countdown_timer)){
            sys_stop_timer(countdown_timer);
        }
    }
}

//单通道事件,在正常工作时调用，会触发上报及断电记忆储存
STATIC VOID channel_event_deal(IN UCHAR_T channel_num,IN CHAN_EVENT_TYPE event_type)
{
    if(channel_num < 0 || channel_num >= g_hw_table.channel_num){
        PR_ERR("input channel number error: %d", channel_num);
        return;
    }

    switch(event_type){
    case CHAN_EVENT_OPEN:
        ctrl_switch_state(channel_num,CTRL_SW_OPEN);
        break;
    case CHAN_EVENT_CLOSE:
        ctrl_switch_state(channel_num,CTRL_SW_CLOSE);
        break;
    case CHAN_EVENT_TOGGLE:
        ctrl_switch_state(channel_num,CTRL_SW_TOGGLE);
        break;
    default:
        break;
    }
    upload_switch_state(channel_num);
    if(g_hw_table.channels[channel_num].init_ch_stat == INIT_CH_MEM){
        sys_start_timer(save_stat_timer, 5000, TIMER_ONCE);
    }
}

//全通道事件，在正常工作时调用，会触发上报及断电记忆储存
STATIC VOID total_channel_event_deal(IN BOOL_T on_or_off)
{
    UCHAR_T i;
    BOOL_T exist_pwr_mem = FALSE;
    for(i = 0; i<g_hw_table.channel_num; i++){
        ctrl_switch_state(i,on_or_off? CTRL_SW_OPEN : CTRL_SW_CLOSE);
        if(g_hw_table.channels[i].init_ch_stat == INIT_CH_MEM){
            exist_pwr_mem = TRUE;
        }
    }
    if(exist_pwr_mem){
        sys_start_timer(save_stat_timer, 5000, TIMER_ONCE);
    }
    if(OPRT_OK != upload_all_switch_state()){
        PR_DEBUG("upload all switch data err");
    }
}

STATIC VOID init_switch_state(VOID)
{
    UCHAR_T i;
    BOOL exist_pwr_mem = FALSE;
    for(i = 0; i < g_hw_table.channel_num; i++){
        switch(g_hw_table.channels[i].init_ch_stat){
        case INIT_CH_CLOSE:
            ctrl_switch_state(i,CTRL_SW_CLOSE);
            break;
        case INIT_CH_OPEN:
            ctrl_switch_state(i,CTRL_SW_OPEN);
            break;
        case INIT_CH_MEM:
            exist_pwr_mem = TRUE;
            break;
        default:
            break;
        }
    }
    if(exist_pwr_mem){//若存在需要断电记忆的通道
        read_saved_stat();
    }
}

/***********************************************************
*   Function: hw_button_init
*   Input:    VOID
*   Output:   VOID
*   Return:   OPERATE_RET:返回值结果
*   Notice:   hardware button init 
***********************************************************/
STATIC OPERATE_RET hw_button_init(KEY_USER_DEF_S *key_info,KEY_CALLBACK key_process)
{
    OPERATE_RET op_ret = OPRT_OK;

    if(NULL == key_info) {
        return OPRT_INVALID_PARM;
    }

    key_info->port = TY_GPIOA_9;//1TX
    key_info->low_level_detect = TRUE;
    key_info->lp_tp = LP_ONCE_TRIG;
    key_info->long_key_time = PRESS_LONG_TIME;
    key_info->seq_key_detect_time = 0;
    key_info->call_back = key_process;

    op_ret = reg_proc_key(key_info);
    if(OPRT_OK != op_ret) {
        PR_ERR("reg press_key fail,err_num:%d",op_ret);
        return op_ret;
    }
    PR_DEBUG("button init sucess,button_pin:%d",key_info->port);
    return OPRT_OK;
}

STATIC OPERATE_RET hw_switch_init(IN APP_SW_MODE mode)
{
    PR_DEBUG("initialize sw hardware...");
    OPERATE_RET op_ret = OPRT_OK;

    op_ret = key_init(NULL,0,KEY_TIMER_MS);
    if(op_ret != OPRT_OK) {
        PR_ERR("key_init err:%d",op_ret);
        return op_ret;
    }

    g_hw_table.channel_num = SIZEOF(ctrl_channels) / SIZEOF(CTRL_CHANNEL_CONFIG);
    PR_DEBUG("number:%d", g_hw_table.channel_num);

    if(g_hw_table.channel_num <= 0){
        PR_ERR("Error number of channel: %d", g_hw_table.channel_num);
        return OPRT_INVALID_PARM;
    }

    // 初始化wifi状态指示灯
    if(g_hw_table.wf_led.wfl_io.type != IO_DRIVE_LEVEL_NOT_EXIST){
        op_ret = tuya_create_led_handle_select(g_hw_table.wf_led.wfl_io.pin ,\
        (g_hw_table.wf_led.wfl_io.type ==IO_DRIVE_LEVEL_HIGH ? FALSE : TRUE),&g_hw_table.wf_led.wfl_handle);
        op_ret = sys_add_timer(enter_pt_timer_cb,NULL,&enter_pt_timer);
        if(OPRT_OK != op_ret) {
            return op_ret;
        }
        op_ret = sys_add_timer(end_pt_timer_cb,NULL,&end_pt_timer);
        if(OPRT_OK != op_ret) {
            return op_ret;
        }
    }
    UCHAR_T i = 0;
    // 初始化控制通道
    for(i = 0; i < g_hw_table.channel_num; i++){
        g_hw_table.channels[i].stat = FALSE; // 初始状态
        g_hw_table.channels[i].cd_sec = 0;
        if(g_hw_table.channels[i].relay.type != IO_DRIVE_LEVEL_NOT_EXIST){
            op_ret = tuya_gpio_inout_set_select(g_hw_table.channels[i].relay.pin,FALSE,\
                (g_hw_table.channels[i].relay.type ==IO_DRIVE_LEVEL_HIGH ? TRUE :FALSE));
        }else {
            //如继电器非由模块GPIO直接控制，在此处初始化
        }
        if(0 == i) {
//            hw_button_init(&g_hw_table.channels[0].button,(mode == APP_SW_MODE_NORMAL ? hw_key_process : pt_hw_key_process));
        }

        if(g_hw_table.channels[i].led.type != IO_DRIVE_LEVEL_NOT_EXIST){
            op_ret = tuya_create_led_handle_select(g_hw_table.channels[i].led.pin ,\
            (g_hw_table.channels[i].led.type == IO_DRIVE_LEVEL_HIGH ? FALSE : TRUE),&g_hw_table.channels[i].led_handle);
        }else {
            //如通道LED非由模块GPIO直接控制，在此处初始化
        }
    }

        hw_button_init(&g_hw_table.tch.tbt,(mode == APP_SW_MODE_NORMAL ? hw_key_process : pt_hw_key_process));
        if(APP_SW_MODE_PRODTEST == mode){
            op_ret = sys_add_timer(pt_total_bt_timer_cb,NULL,&pt_total_bt_timer);
            if(OPRT_OK != op_ret) {
                return op_ret;
            }
        }

    if(g_hw_table.tch.tled.type != IO_DRIVE_LEVEL_NOT_EXIST){
        op_ret = tuya_create_led_handle_select(g_hw_table.tch.tled.pin ,\
        (g_hw_table.tch.tled.type == IO_DRIVE_LEVEL_HIGH ? FALSE : TRUE),&g_hw_table.tch.tled_handle);
        PR_DEBUG("There exist totol relay led:%d.",g_hw_table.tch.tled.pin);
    }else{
        PR_DEBUG("There not exist totol relay led.");
        //如总继电器led非由模块GPIO直接控制，在此处初始化
    }
    return op_ret;
}

OPERATE_RET app_switch_init(IN APP_SW_MODE mode)
{
    OPERATE_RET op_ret = OPRT_OK;

    op_ret = hw_switch_init(mode);
    if(OPRT_OK != op_ret) {
        return op_ret;
    }
    init_switch_state();
    if(APP_SW_MODE_NORMAL == mode){
        op_ret = sys_add_timer(countdown_timer_cb, NULL, &countdown_timer);
        if(OPRT_OK != op_ret) {
            return op_ret;
        }
        
        op_ret = sys_add_timer(save_stat_timer_cb, NULL, &save_stat_timer);
        if(OPRT_OK != op_ret) {
            return op_ret;
        }
    }else{

    }
    PR_DEBUG("app switch init success");
    return op_ret;
}

//正常工作模式下按键回调
STATIC VOID hw_key_process(TY_GPIO_PORT_E gpio_no,PUSH_KEY_TYPE_E type,INT_T cnt)
{
    PR_DEBUG("gpio_no: %d, type: %d, cnt: %d",gpio_no,type,cnt);
    if(LONG_KEY == type) {
        tuya_iot_wf_gw_unactive();
    }else if(NORMAL_KEY == type) {
        if(gpio_no == g_hw_table.tch.tbt.port){
            key_short_press_deal(gpio_no,KEY_ALL_PRIOR_CLOSE);
        }else{
            key_short_press_deal(gpio_no,KEY_SINGLE_TOGGLE);
        }
    }
}
//产测按键回调函数
STATIC VOID pt_hw_key_process(TY_GPIO_PORT_E gpio_no,PUSH_KEY_TYPE_E type,INT_T cnt)
{
    UCHAR_T i;
    PR_DEBUG("gpio_no: %d, type: %d, cnt: %d",gpio_no,type,cnt);
/*    if(g_hw_table.wf_led.wfl_cs == WFL_DIR_RL || \
    g_hw_table.wf_led.wfl_ucs == WFL_DIR_RL){
        set_wfl_state(WFL_DIR_RL);
    }else{
        set_wfl_state(WFL_OFF);
    }*/
    if(pt_key_en && (NORMAL_KEY == type)) {
        if(gpio_no == g_hw_table.tch.tbt.port){
            if(!IsThisSysTimerRun(pt_total_bt_timer)){
                if(judge_any_sw(TRUE) && judge_any_sw(FALSE)){//如果存在半开半关，先全关
                    for(i = 0; i<g_hw_table.channel_num; i++){
                        ctrl_switch_state(i, CTRL_SW_CLOSE);
                    }
                }
                BOOL ch_state = judge_any_sw(TRUE);
                pt_curr_ch = 0;
                pt_ch_deal = ch_state ? CTRL_SW_CLOSE: CTRL_SW_OPEN;
                sys_start_timer(pt_total_bt_timer,500,TIMER_CYCLE);
            }
        }else{
            for(i = 0; i<g_hw_table.channel_num; i++){
                if(gpio_no == g_hw_table.channels[i].button.port){
                    ctrl_switch_state(i, CTRL_SW_TOGGLE);
                    return;
                }
            }
        }
    }else if(pt_key_en && LONG_KEY == type){
        return;//先按老版本出
        OPERATE_RET op_ret = save_pt_end_flag(1);
        if(OPRT_OK == op_ret){
            PR_DEBUG("save pt end key success!!!");
        }else{
            PR_ERR("save pt end key fail!!!", op_ret);
        }
        set_wfl_state(WFL_END_PT);
    }
}

STATIC VOID save_stat_timer_cb(UINT_T timerID,PVOID_T pTimerArg)
{
    cJSON *root = NULL;
    CHAR_T *buf = NULL;
    UCHAR_T i = 0;
    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error");
        return;
    }
    CHAR_T dpid_str[8];
    for(i = 0;i < g_hw_table.channel_num;i++){
        if(g_hw_table.channels[i].init_ch_stat == INIT_CH_MEM){
            sprintf(dpid_str, "%d", i);
            cJSON_AddBoolToObject(root,dpid_str,g_hw_table.channels[i].stat);
        }
    }
    buf = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if(NULL == buf) {
        PR_ERR("buf is NULL");
        return;
    }    
    PR_DEBUG("msf_set ch state:%s",buf);

    uFILE* fp = NULL;
    fp = ufopen(POWER_STAT_KEY,"w+");
    if(NULL == fp) {
        PR_ERR("write to file error");
        return;
    }else {
        PR_DEBUG("open file OK");
    }

    INT_T Num = ufwrite(fp, (UCHAR_T *)buf, strlen(buf));
    ufclose(fp);
    if( Num <= 0) {
        PR_ERR("uf write fail %d",Num);
        Free(buf);
        return;
    }
    PR_DEBUG("uf write ok %d",Num);

    Free(buf);
    return;
}

STATIC OPERATE_RET read_saved_stat(VOID)
{
    uFILE * fp = ufopen(POWER_STAT_KEY,"r");
    if(NULL == fp) {     /* 如果无法打开 */
        PR_ERR("cannot open file");
        return OPRT_COM_ERROR;
    }
    INT_T buf_len = 64;
    CHAR_T *buf = (CHAR_T *)Malloc(buf_len);
    if(NULL == buf) {
        PR_ERR("malloc fail");
        return OPRT_MALLOC_FAILED;
    }

    memset(buf,0,buf_len);
    INT_T Num = ufread(fp, (UCHAR_T *)buf,buf_len);
    if(Num <= 0) {
        Free(buf);
        PR_ERR("uf read error %d",Num);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("relay_stat_get_buf:%s", buf);
    ufclose(fp);

    cJSON *json = NULL;

    PR_DEBUG("read ch state:%s",buf);
    cJSON *root = cJSON_Parse(buf);
    if(NULL == root) {
        PR_NOTICE("read save cjson parse");
        goto JSON_PARSE_ERR;
    }
    UCHAR_T i;
    CHAR_T dpid_str[8];
    for(i = 0;i < g_hw_table.channel_num;i++){
        if(g_hw_table.channels[i].init_ch_stat == INIT_CH_MEM){
            sprintf(dpid_str, "%d", i);
            json = cJSON_GetObjectItem(root,dpid_str);
            if(NULL == json) {
                PR_DEBUG("cjson %d ch stat not found", i);
                ctrl_switch_state(i, CTRL_SW_CLOSE);
            }else{
                if(json->type == cJSON_True){
                    ctrl_switch_state(i, CTRL_SW_OPEN);
                }else{
                    ctrl_switch_state(i, CTRL_SW_CLOSE);
                }
            }
        }
    }

    cJSON_Delete(root);
    Free(buf);
    return OPRT_OK;

JSON_PARSE_ERR:
    cJSON_Delete(root);
    Free(buf);
    return OPRT_COM_ERROR;
}

STATIC OPERATE_RET upload_switch_countdown(IN UCHAR_T channel_num)
{
    if(g_hw_table.channels[channel_num].cd_dpid < 0){
        return OPRT_OK;
    }

    OPERATE_RET op_ret = OPRT_OK;

    INT_T dp_cnt = 1;//通道dp+电量dp(不包含电量值)

    TY_OBJ_DP_S *dp_arr = (TY_OBJ_DP_S *)Malloc(dp_cnt*SIZEOF(TY_OBJ_DP_S));
    if(NULL == dp_arr) {
        PR_ERR("malloc failed");
        return OPRT_MALLOC_FAILED;
    }

    memset(dp_arr, 0, dp_cnt*SIZEOF(TY_OBJ_DP_S));

    dp_arr[0].dpid = g_hw_table.channels[channel_num].cd_dpid;
    dp_arr[0].type = PROP_VALUE;
    dp_arr[0].time_stamp = 0;
    dp_arr[0].value.dp_value = g_hw_table.channels[channel_num].cd_sec;

    op_ret = dev_report_dp_json_async(get_gw_cntl()->gw_if.id,dp_arr,dp_cnt);
    Free(dp_arr);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async relay_cd_count data error,err_num",op_ret);
        return op_ret;
    }

    PR_DEBUG("report_dp_count_data");
    return OPRT_OK;
}

/**
 * @brief 
 * 
 * @param channel_num 
 * @return OPERATE_RET 
 * @TODO 拆开倒计时清除逻辑和上报逻辑
 */
STATIC OPERATE_RET upload_switch_state(IN UCHAR_T channel_num)
{
    OPERATE_RET op_ret = OPRT_OK;

    INT_T dp_cnt = 0;
    dp_cnt = 2;//通道dp+电量dp(不包含电量值)

    TY_OBJ_DP_S *dp_arr = (TY_OBJ_DP_S *)Malloc(dp_cnt*SIZEOF(TY_OBJ_DP_S));
    if(NULL == dp_arr) {
        PR_ERR("malloc failed");
        return OPRT_MALLOC_FAILED;
    }

    memset(dp_arr, 0, dp_cnt*SIZEOF(TY_OBJ_DP_S));

    dp_arr[0].dpid = g_hw_table.channels[channel_num].cd_dpid;
    dp_arr[0].type = PROP_VALUE;
    dp_arr[0].time_stamp = 0;
    if(g_hw_table.channels[channel_num].cd_sec > 0) {
        g_hw_table.channels[channel_num].cd_sec = 0;
    }
    dp_arr[0].value.dp_value = 0;

    dp_arr[1].dpid = g_hw_table.channels[channel_num].dpid;
    dp_arr[1].time_stamp = 0;
    dp_arr[1].type = PROP_BOOL;
    dp_arr[1].value.dp_bool = g_hw_table.channels[channel_num].stat;

    op_ret = dev_report_dp_json_async(get_gw_cntl()->gw_if.id,dp_arr,dp_cnt);
    Free(dp_arr);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async relay_config data error,err_num",op_ret);
        return op_ret;
    }

    PR_DEBUG("dp_query report_all_dp_data");
    return OPRT_OK;
}

//调用此函数会清零所有倒计时，并上报所有开关状态+倒计时。
STATIC OPERATE_RET upload_all_switch_state(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    INT_T dp_cnt = 0,i = 0,j = 0;
    dp_cnt = g_hw_table.channel_num*2;//通道dp+电量dp(不包含电量值)

    TY_OBJ_DP_S *dp_arr = (TY_OBJ_DP_S *)Malloc(dp_cnt*SIZEOF(TY_OBJ_DP_S));
    if(NULL == dp_arr) {
        PR_ERR("malloc failed");
        return OPRT_MALLOC_FAILED;
    }

    memset(dp_arr, 0, dp_cnt*SIZEOF(TY_OBJ_DP_S));

    for(j = 0;j < g_hw_table.channel_num;j++) {
        dp_arr[i].dpid = g_hw_table.channels[j].cd_dpid;
        dp_arr[i].type = PROP_VALUE;
        dp_arr[i].time_stamp = 0;
        if(g_hw_table.channels[j].cd_sec > 0) {
            g_hw_table.channels[j].cd_sec = 0;
        }
        dp_arr[i].value.dp_value = 0;
        i++;
        dp_arr[i].dpid = g_hw_table.channels[j].dpid;
        dp_arr[i].time_stamp = 0;
        dp_arr[i].type = PROP_BOOL;
        dp_arr[i].value.dp_bool = g_hw_table.channels[j].stat;
    }

    op_ret = dev_report_dp_json_async(get_gw_cntl()->gw_if.id,dp_arr,dp_cnt);
    Free(dp_arr);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async relay_config data error,err_num",op_ret);
        return op_ret;
    }

    PR_DEBUG("dp_query report_all_dp_data");
    return OPRT_OK;
}

//进入产测指示灯定时器
STATIC VOID enter_pt_timer_cb(UINT_T timerID,PVOID_T pTimerArg)
{
    STATIC BOOL chg_flag = FALSE;
    if(chg_flag){
        tuya_set_led_light_type(g_hw_table.wf_led.wfl_handle,\
        OL_FLASH_LOW, 100,LED_TIMER_UNINIT);
    }else{
        set_hw_wifi_led(TRUE);
    }
    chg_flag = !chg_flag;
}
//结束产测指示灯定时器
STATIC VOID end_pt_timer_cb(UINT_T timerID,PVOID_T pTimerArg)
{
    set_wfl_state(WFL_OFF);
}

STATIC VOID pt_total_bt_timer_cb(UINT_T timerID,PVOID_T pTimerArg)
{
    ctrl_switch_state(pt_curr_ch, pt_ch_deal);
    pt_curr_ch++;
    if(pt_curr_ch >= g_hw_table.channel_num){
        sys_stop_timer(pt_total_bt_timer);
    }
}
//wifi灯直接指示继电器
STATIC VOID wfl_direct_relay(VOID)
{
    if(wf_dir_rl_en){
        BOOL_T if_on = judge_any_sw(TRUE);
        set_hw_wifi_led(if_on);
    }
}
//设定WIFI灯状态
VOID set_wfl_state(IN WFL_STAT wfl_stat)
{
    if(g_hw_table.wf_led.wfl_io.type == \
    IO_DRIVE_LEVEL_NOT_EXIST){
        return;
    }
    if(wfl_stat == WFL_DIR_RL){
        wf_dir_rl_en = TRUE;
    }else{
        wf_dir_rl_en = FALSE;
    }
    if(wfl_stat != WFL_ENTER_PT){
        if(IsThisSysTimerRun(enter_pt_timer)){
            sys_stop_timer(enter_pt_timer);
        }
    }
    PR_DEBUG("wfl stat:%d", wfl_stat);
    switch(wfl_stat){
    case WFL_ON:
        set_hw_wifi_led(TRUE);
        break;
    case WFL_OFF:
        set_hw_wifi_led(FALSE);
        break;
    case WFL_DIR_RL:
        wfl_direct_relay();
        break;
    case WFL_FLASH_VERY_FAST:
        tuya_set_led_light_type(g_hw_table.wf_led.wfl_handle,\
        OL_FLASH_LOW, 100,LED_TIMER_UNINIT);
        break;
    case WFL_FLASH_FAST:
        tuya_set_led_light_type(g_hw_table.wf_led.wfl_handle,\
        OL_FLASH_LOW, 250,LED_TIMER_UNINIT);
        break;
    case WFL_FLASH_SLOW:
        tuya_set_led_light_type(g_hw_table.wf_led.wfl_handle,\
        OL_FLASH_LOW, 1500,LED_TIMER_UNINIT);
        break;
    case WFL_ENTER_PT:
        tuya_set_led_light_type(g_hw_table.wf_led.wfl_handle,\
        OL_FLASH_LOW, 100,LED_TIMER_UNINIT);
        sys_start_timer(enter_pt_timer,500,TIMER_CYCLE);
        break;
    case WFL_END_PT:
        tuya_set_led_light_type(g_hw_table.wf_led.wfl_handle,\
        OL_FLASH_LOW, 250,LED_TIMER_UNINIT);
        sys_start_timer(end_pt_timer,2500,TIMER_ONCE);
        break;
    }
}
//过流保护事件
VOID over_protect(VOID)
{
    PR_DEBUG("over protect!!!");
    BOOL_T if_on = judge_any_sw(TRUE);
    if(if_on){
        PR_DEBUG("There exist at least one switch ON");
        total_channel_event_deal(FALSE);
        report_over_curr();
    }
}

VOID set_pt_key_en(IN BOOL_T if_en)
{
    pt_key_en = if_en;
}

//保存产测结束标志位
OPERATE_RET save_pt_end_flag(IN INT_T state)
{
    cJSON *root = NULL;
    UCHAR_T *buf = NULL;

    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error");
        return OPRT_CJSON_GET_ERR;
    }
    cJSON_AddNumberToObject(root, "pt_end", state);
    buf = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if(NULL == buf) {
        PR_ERR("buf is NULL");
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("set buf:%s",buf);

    uFILE* fp = NULL;
    fp = ufopen(PT_END_KEY,"w+");
    if(NULL == fp) {
        PR_ERR("write to file error");
        Free(buf);
        return OPRT_COM_ERROR;
    }else {
        PR_DEBUG("open file OK");
    }

    INT_T Num = ufwrite(fp, (UCHAR_T *)buf, strlen(buf));
    ufclose(fp);
    Free(buf);
    if( Num <= 0) {
        PR_ERR("uf write fail %d",Num);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("uf write ok %d",Num);

    return OPRT_OK;
}
//获取产测结束标志位
OPERATE_RET get_pt_end_flag(OUT INT_T *state)
{
    uFILE * fp = ufopen(PT_END_KEY,"r");
    if(NULL == fp) {     /* 如果无法打开 */
        PR_ERR("cannot open file");
        return OPRT_COM_ERROR;
    }
    INT_T buf_len = 16;
    CHAR_T *buf = (CHAR_T *)Malloc(buf_len);
    if(NULL == buf) {
        PR_ERR("malloc fail");
        return OPRT_MALLOC_FAILED;
    }

    memset(buf,0,buf_len);
    INT_T Num = ufread(fp, (UCHAR_T *)buf,buf_len);
    if(Num <= 0) {
        Free(buf);
        PR_ERR("uf read error %d",Num);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("relay_stat_get_buf:%s", buf);
    ufclose(fp);

    PR_DEBUG("get buf:%s",buf);
    cJSON *root = cJSON_Parse(buf);
    if(NULL == root) {
        PR_ERR("cjson parse");
        goto JSON_PARSE_ERR;
    }

    cJSON *json = cJSON_GetObjectItem(root,"pt_end");
    if(NULL == json) {
        PR_ERR("cjson get ");
        goto JSON_PARSE_ERR;
    }

    *state = json->valueint;
    cJSON_Delete(root);
    Free(buf);
    return OPRT_OK;

JSON_PARSE_ERR:
    cJSON_Delete(root);
    Free(buf);
    return OPRT_COM_ERROR;
}

VOID reset_power_stat(VOID)
{
    CHAR_T save_relay_stat[16] = "relay_stat_key";
    UINT_T channel_num = 0;
    OPERATE_RET op_ret = OPRT_OK;

    if(g_hw_table.channels[channel_num].init_ch_stat == INIT_CH_MEM) {
        op_ret = ufdelete(save_relay_stat);
        if(OPRT_OK != op_ret) {
            PR_ERR("hw_reset_relay_data ufdelete error,err_num:%d",op_ret);
        }
    }
    PR_DEBUG("reset power stat default");
}

GW_WF_CFG_MTHD_SEL hw_get_wifi_mode(VOID)
{
    return g_hw_table.wf_led.wcm_mode;
}

VOID hw_wifi_led_status(GW_WIFI_NW_STAT_E wifi_stat)
{
    PR_DEBUG("wifi_stat:%d",wifi_stat);
        switch(wifi_stat) {
            case STAT_AP_STA_UNCFG:
                set_wfl_state(WFL_FLASH_FAST);
            break;

            case STAT_LOW_POWER:
                set_wfl_state(g_hw_table.wf_led.wfl_ucs);
            break;

            case STAT_AP_STA_DISC:
            case STAT_STA_DISC:
                set_wfl_state(WFL_FLASH_SLOW);
            break;

            case STAT_STA_CONN:
            case STAT_CLOUD_CONN:
            case STAT_AP_CLOUD_CONN:
                set_wfl_state(g_hw_table.wf_led.wfl_cs);
            break;
        }
}

