/**
 * @File: app_dltj.c 
 * @Author: caojq 
 * @Last Modified time: 2020-07-29
 * @Description: Power statistics upper-layer application.
 * Support dp query, support tuya_common_user compilation.
 * 
 */
#define _APP_DLTJ_GLOBAL

#include "tuya_os_adapter.h"
#include "app_dltj.h"
#include "app_switch.h"
#include "uni_log.h"
#include "uf_file.h"
#include "cJSON.h"
#include "uni_thread.h"
#include "uni_time_queue.h"
#include "tuya_iot_com_api.h"
#include "gw_intf.h"
#include "tuya_hal_semaphore.h"

/***********************************************************
*************************micro define***********************
***********************************************************/
#if _APP_DLTJ_DEBUG
#define APP_DLTJ_DEBUG  PR_DEBUG
#else
#define APP_DLTJ_DEBUG(...)
#endif

#define TEM_ELE_SAVE_KEY "tem_ele_save_key"
#define ELE_SAVE_KEY "ele_save_key"

#define TIME_POSIX_2016 1451577600

#define ADD_ELE_THRE 100 //Temporary power accumulation processing threshold
#define ADD_TIME_MIN_THRE 60 //Minimum time interval for power processing
#define ADD_TIME_MAX_THRE 1800 //Maximum time interval for power processing, unit S
#define PVI_REPORT_BIG_THRE  30 //Large interval for reporting current, voltage and power data
#define PVI_REPORT_SMALL_THRE 5 //Small interval for reporting current, voltage and power data
#define PVI_REPORT_MAX_THRE 3600 //The longest interval of current, voltage and power data reporting
#define PVI_REPORT_BIG_THRE_DP_QUERY  1800 //Large interval for reporting current, voltage and power data when there is dp query
#define PVI_REPORT_SMALL_THRE_DP_QUERY 600 //Small interval of current, voltage and power data reporting when dp query exists

#define REPT_THRE_VOL 2 //Voltage reporting change threshold, unit %
#define REPT_THRE_PWR 30 //Power reporting change threshold, unit %
#define ELE_SLEEP_TIME 1 //Power thread sleep interval

#define _IS_OVER_PROTECT 1 //Whether there is an overcurrent protection function
#define LIMIT_CURRENT 17000 //Overcurrent protection limit, unit 1mA
#define _IF_REPT_COE   TRUE     //whether to report power calibration coefficient
#define _DLTJ_REPT_TEST 0      //Report power statistics for debugging, and automatically report data after opening
#define SMALLEST_POWER 30   //When the power is less than this value, the shielding current and power, the unit is 0.1W
#define CPREEPING_POWER_THRE 1 /* Creeping power threshold, some power chips such as bl0937 may measure 0.001 kWh of power without load for 30 minutes, and this part of power will be discarded at this time */
#define MAX_POWER_ABS 50
/***********************************************************
*************************variable define********************
***********************************************************/
typedef struct 
{
    UINT_T unix_time;
    UINT_T ele_energy;
}LOC_DATA;

typedef struct 
{
    UINT_T add_ele; //The accumulated power in the lower power 
    UINT_T cur_posix; //Current timestamp
    UINT_T loc_data_cnt; //The number of elements of the local power data group
    UINT_T cur_current;
    UINT_T cur_vol;
    UINT_T cur_power;
    UINT_T last_current;
    UINT_T last_vol;
    UINT_T last_power;
    INT_T tem_ele_val; //Store the electricity that has been accumulated but has not been reported or stored in local_data
    THRD_HANDLE report_thread;
    LOC_DATA ele_sav_data[10]; //Local power data group
}POWER_CNT;
 STATIC POWER_CNT power_cnt;

typedef enum{
    ELE_NOT_ACTIVE, //The network is not activated after power on
    ELE_NORMAL, //The network has been successfully configured and the current mqtt connection is successful
    ELE_UNCONNECT, //The network has been successfully configured when the current mqtt connection fails
//    ELE_SYS_OTA //The system is being upgraded online
}ELE_THREAD_STATE;

typedef struct{
    TIMER_ID                    app_dltj_timer_id;
//    THRD_HANDLE                      app_dltj_thread;
    TIMER_ID                    app_dltj_timer;
    SEM_HANDLE                  app_dltj_sem;
}AppDltj_S,*pAppDltj_S;
AppDltj_S mAppDltj;

typedef struct {
    BOOL_T is_dp_query;
    UINT_T pvi_small_thre;
    UINT_T pvi_big_thre;
} handle_time_t;

STATIC handle_time_t handle_time = { 
    .is_dp_query = FALSE,
    .pvi_big_thre = PVI_REPORT_BIG_THRE,
    .pvi_small_thre = PVI_REPORT_SMALL_THRE,
};

extern HW_TABLE g_hw_table;

STATIC ELE_THREAD_STATE ele_state = ELE_NOT_ACTIVE;
STATIC UINT_T ele_handle_time = 0; //The time since the last power
STATIC UINT_T pvi_handle_time = 0;
STATIC UINT_T pvi_report_min_time = PVI_REPORT_BIG_THRE; //Current reporting interval, adjustable
#if 0
#define DLTJ_INIT_CONFIG_DEFAULT()              \
    {                                           \
        .if_have = true,                        \
        .over_curr = LIMIT_CURRENT,             \
        .dp_info = {                            \
            .edpid = DP_ELE,                    \
            .idpid = DP_CURRENT,                \
            .pdpid = DP_POWER,                  \
            .vdpid = DP_VOLTAGE,                \
            .fault_dpid = DP_FAULT,             \
        },                                      \
        .drv_cfg = {                            \
            .epin = 4,                          \
            .ivpin = 5,                         \
            .ivcpin.pin = 12,                   \
            .ivcpin.type = IO_DRIVE_LEVEL_HIGH, \
            .v_ref = V_COE_BL,                  \
            .i_ref = I_COE_BL,                  \
            .p_ref = P_COE_BL,                  \
            .e_ref = E_COE_BL,                  \
            .v_def = V_DEF_220,                 \
            .i_def = I_DEF_220,                 \
            .p_def = P_DEF_220,                 \
            .dp_pt_rslt = DP_PTRSLT,            \
            .dp_vcoe = DP_VREF,                 \
            .dp_icoe = DP_IREF,                 \
            .dp_pcoe = DP_PREF,                 \
            .dp_ecoe = DP_EREF,                 \
        },                                      \
    }
#endif
APP_DLTJ_CFG g_dltj = {
    .if_have = true,
    .over_curr = LIMIT_CURRENT,
    .dp_info = {
        .edpid = DP_ELE,
        .idpid = DP_CURRENT,
        .pdpid = DP_POWER,
        .vdpid = DP_VOLTAGE,
        .fault_dpid = DP_FAULT,
    },
    .drv_cfg = {
        .epin = TY_GPIOA_7,
        .ivpin = TY_GPIOA_8,
        .ivcpin.pin = TY_GPIOA_24,
        .ivcpin.type = IO_DRIVE_LEVEL_HIGH,
        .v_ref = V_COE_BL,
        .i_ref = I_COE_BL,
        .p_ref = P_COE_BL,
        .e_ref = E_COE_BL,
        .v_def = V_DEF_220,
        .i_def = I_DEF_220,
        .p_def = P_DEF_220,
        .dp_pt_rslt = DP_PTRSLT,
        .dp_vcoe = DP_VREF,
        .dp_icoe = DP_IREF,
        .dp_pcoe = DP_PREF,
        .dp_ecoe = DP_EREF,
    },
};

/***********************************************************
*************************function define********************
***********************************************************/
STATIC OPERATE_RET report_pvi_data(VOID);
STATIC OPERATE_RET report_fault_warn(IN UCHAR_T dpid_fault,IN INT_T fault_word);
STATIC VOID addto_local_data(IN UINT_T time,IN UINT_T ele);
STATIC VOID report_local_data(VOID);
STATIC VOID add_ele_handle(VOID);
STATIC BOOL_T value_range_judge(IN UINT_T JudgedValue,IN UINT_T TargetValue,IN UINT_T range);
STATIC BOOL_T same_day_judge(IN UINT_T u_time1,IN UINT_T u_time2);
STATIC BOOL_T if_pvi_need_report(VOID);
VOID over_protect(VOID);
/*********************************************************************************
 * FUNCTION:       get_time_posix
 * DESCRIPTION:    Get the current timestamp
 * INPUT:          none
 * OUTPUT:         curPosix:current timestamp
 * RETURN:         OPERATE_RET:Initialization state, return numeric state value
 * OTHERS:         The unit is milliseconds
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC OPERATE_RET get_time_posix(OUT UINT_T *curPosix)
{
    UINT_T tmp_posix;
    tmp_posix = uni_time_get_posix();
    if(tmp_posix < TIME_POSIX_2016) {
        APP_DLTJ_DEBUG("get curPosix err!");
        return OPRT_INVALID_PARM;
    }
    *curPosix = tmp_posix;
    return OPRT_OK;
}

/*********************************************************************************
 * FUNCTION:       get_tem_ele_val
 * DESCRIPTION:    Get the temporary battery value in flash
 * INPUT:          none
 * OUTPUT:         val:temporary battery value
 * RETURN:         OPERATE_RET:Initialization state, return numeric state value
 * OTHERS:         Format of unconfigured grid power storage in flash
     {“dps”:{“dp_ele”:ele_vlaue},”dpsTime”:{“dp_ele”:s_timer_posix},”ts”:ms_time_posix}
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC OPERATE_RET get_tem_ele_val(OUT INT_T *val)
{
    uFILE * fp = ufopen(TEM_ELE_SAVE_KEY,"r");
    if(NULL == fp) {     /* If it cannot be opened  */
        PR_ERR("cannot open file");
        *val = 0;
        return OPRT_COM_ERROR;
    }
    INT_T buf_len = 256;
    CHAR_T *buf = (CHAR_T *)Malloc(buf_len);
    if(NULL == buf) {
        *val = 0;
        PR_ERR("malloc fail");
        return OPRT_MALLOC_FAILED;
    }

    memset(buf,0,buf_len);
    INT_T Num = ufread(fp, (UCHAR_T *)buf,buf_len);
    if(Num <= 0) {
        *val = 0;
        Free(buf);
        PR_ERR("uf read error %d",Num);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("relay_stat_get_buf:%s", buf);
    ufclose(fp);

    APP_DLTJ_DEBUG("Get tem ele val:%s",buf);
    cJSON * root = cJSON_Parse(buf);
    if(NULL == root) {
        PR_ERR("cjson parse");
        goto JSON_PARSE_ERR;
    }

    cJSON *json = cJSON_GetObjectItem(root,"tem_ele");
    if(NULL == json) {
        PR_ERR("cjson get ");
        goto JSON_PARSE_ERR;
    }

    *val = json->valueint;
    cJSON_Delete(root);
    Free(buf);
    return OPRT_OK;

JSON_PARSE_ERR:
    cJSON_Delete(root);
    Free(buf);
    *val = 0;
    return OPRT_COM_ERROR;
}

/*********************************************************************************
 * FUNCTION:       set_tem_ele_val
 * DESCRIPTION:    Store temporary battery value in flash
 * INPUT:          none
 * OUTPUT:         val:temporary battery value
 * RETURN:         OPERATE_RET:Initialization state, return numeric state value
 * OTHERS:         Format of unconfigured grid power storage in flash:
     {“dps”:{“dp_ele”:ele_vlaue},”dpsTime”:{“dp_ele”:s_timer_posix},”ts”:ms_time_posix}
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC OPERATE_RET set_tem_ele_val(IN INT_T val)
{
    cJSON *root = NULL;
    CHAR_T *buf = NULL;

    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error");
        return OPRT_CJSON_GET_ERR;
    }
    
    cJSON_AddNumberToObject(root, "tem_ele", val);
    buf = cJSON_PrintUnformatted(root);
    if(NULL == buf) {
        PR_ERR("buf is NULL");
        cJSON_Delete(root);
        return OPRT_COM_ERROR;
    }
    cJSON_Delete(root);
    APP_DLTJ_DEBUG("to set tem ele val:%s",buf);

    uFILE* fp = NULL;
    fp = ufopen(TEM_ELE_SAVE_KEY,"w+");
    if(NULL == fp) {
        Free(buf);
        PR_ERR("write to file error");
        return OPRT_COM_ERROR;
    }else {
        PR_DEBUG("open file OK");
    }

    INT_T Num = ufwrite(fp, (UCHAR_T *)buf, strlen(buf));
    ufclose(fp);
    if( Num <= 0) {
        PR_ERR("uf write fail %d",Num);
        Free(buf);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("uf write ok %d",Num);

    Free(buf);
    return OPRT_OK;    
}

/*********************************************************************************
 * FUNCTION:       get_ele_data
 * DESCRIPTION:    Get the disconnected power value in flash
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         OPERATE_RET:Initialization state, return numeric state value
 * OTHERS:         The format of power storage in flash:
     {“ele_save_key”:[{time:time_posix1,ele:ele_value1},{time:time_posix2,ele:ele_value2}...{time:time_posix10,ele:ele_value10}]}
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC OPERATE_RET get_ele_data(VOID)
{
    uFILE * fp = ufopen(ELE_SAVE_KEY,"r");
    if(NULL == fp) {     /* If it cannot be opened  */
        PR_ERR("cannot open file");
        power_cnt.loc_data_cnt = 0;
        return OPRT_COM_ERROR;
    }
    INT_T buf_len = 1024;
    CHAR_T *buf = (CHAR_T *)Malloc(buf_len);
    if(NULL == buf) {
        power_cnt.loc_data_cnt = 0;
        PR_ERR("malloc fail");
        return OPRT_MALLOC_FAILED;
    }

    memset(buf,0,buf_len);
    INT_T Num = ufread(fp, (UCHAR_T *)buf,buf_len);
    if(Num <= 0) {
        power_cnt.loc_data_cnt = 0;
        Free(buf);
        PR_ERR("uf read error %d",Num);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("relay_stat_get_buf:%s", buf);
    ufclose(fp);

    cJSON *arrayItem = NULL, *json = NULL;
    INT_T i;

    APP_DLTJ_DEBUG("get ele data buf:%s",buf);
    cJSON *array = cJSON_Parse(buf);
    Free(buf);
    if(NULL == array) {
        PR_ERR("cjson parse err");
        power_cnt.loc_data_cnt = 0;
        return OPRT_COM_ERROR;
    }
    power_cnt.loc_data_cnt = cJSON_GetArraySize(array);

    for(i = 0;i < power_cnt.loc_data_cnt; i++){
         arrayItem = cJSON_GetArrayItem(array, i);
        if(arrayItem){
            json = cJSON_GetObjectItem(arrayItem,"time");
            if(NULL == json){
                cJSON_Delete(array);
                return OPRT_COM_ERROR;
            }
            power_cnt.ele_sav_data[i].unix_time = json->valueint;
            json = cJSON_GetObjectItem(arrayItem,"ele");
            if(NULL == json) {
                cJSON_Delete(array);
                return OPRT_COM_ERROR;
            }
            power_cnt.ele_sav_data[i].ele_energy = json->valueint;
            APP_DLTJ_DEBUG("i:%d,time:%d,ele:%d",i,power_cnt.ele_sav_data[i].unix_time,\
            power_cnt.ele_sav_data[i].ele_energy);
        }
    }
    cJSON_Delete(array);
    return OPRT_OK;
}

/*********************************************************************************
 * FUNCTION:       set_ele_data
 * DESCRIPTION:    The disconnected power value stored in flash
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         OPERATE_RET:Initialization state, return numeric state value
 * OTHERS:         The format of power storage in flash: 
     {“ele_save_key”:[{time:time_posix1,ele:ele_value1},{time:time_posix2,ele:ele_value2}...{time:time_posix10,ele:ele_value10}]}
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC OPERATE_RET set_ele_data(VOID)
{
    cJSON *arrayItem = NULL, *array = NULL;
    CHAR_T *buf = NULL;
    INT_T i;

    array = cJSON_CreateArray();
    if(NULL == array) {
        PR_ERR("cJSON_CreateArray err");
        return OPRT_CJSON_GET_ERR;    
    }

    for(i = 0; i < power_cnt.loc_data_cnt; i++){
        arrayItem = cJSON_CreateObject();
        if(NULL == arrayItem) {
            cJSON_Delete(array);
            PR_ERR("cJSON_CreateObject error");
            return OPRT_CJSON_GET_ERR;
        }
        cJSON_AddNumberToObject(arrayItem, "time", power_cnt.ele_sav_data[i].unix_time);
        cJSON_AddNumberToObject(arrayItem, "ele", power_cnt.ele_sav_data[i].ele_energy);
        cJSON_AddItemToArray(array,arrayItem);
    }
    buf = cJSON_PrintUnformatted(array);
    cJSON_Delete(array);
    if(NULL == buf) {
        PR_ERR("buf is NULL");
        return OPRT_COM_ERROR;
    }
    APP_DLTJ_DEBUG("set ele data buf:%s",buf);

    uFILE* fp = NULL;
    fp = ufopen(ELE_SAVE_KEY,"w+");
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

/*********************************************************************************
 * FUNCTION:       dltj_driver_init
 * DESCRIPTION:    bl0937 chip hardware mode pin initialization / power pulse count initialization and hardware sampling configuration
 * INPUT:          mode chip count mode: normal mode and production test mode
 * OUTPUT:         none
 * RETURN:         OPERATE_RET:Initialization state, return numeric state value
 * OTHERS:         bl0937 metering chip configuration
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC OPERATE_RET dltj_driver_init(IN APP_DLTJ_MODE mode)
{
    OPERATE_RET op_ret = OPRT_OK;
    bl0937_init(&(g_dltj.drv_cfg));
    if(mode == APP_DLTJ_NORMAL){
        op_ret = ele_cnt_init(D_NORMAL_MODE);
    }else{
        op_ret = ele_cnt_init(D_CAL_START_MODE);
    }
    return op_ret;
}

/*********************************************************************************
 * FUNCTION:       ele_par_filter
 * DESCRIPTION:    Filter current and power events
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         The current power is less than a certain value, or the current power is filtered and cleared when all channels are closed
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC VOID ele_par_filter(VOID)
{
    if((power_cnt.cur_power <= SMALLEST_POWER)|| !judge_any_sw(true)) {
        power_cnt.cur_current = 0;
        power_cnt.cur_power = 0;
    }
}

/*********************************************************************************
 * FUNCTION:       pvi_data_handle
 * DESCRIPTION:    Battery parameter events 
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Obtain real-time power parameters and corresponding overload events;
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC VOID pvi_data_handle(VOID)
{
#if _DLTJ_REPT_TEST
    static UINT_T p,v,i;
    p+=10;
    v++;
    i += 10;
    if(i > 30000){
        i = 0;
        p = 0;
        v = 0;
    }
    power_cnt.cur_current = i;
    power_cnt.cur_power = p;
    power_cnt.cur_vol = v;
#else
    get_ele_par(&power_cnt.cur_power,&power_cnt.cur_vol,&power_cnt.cur_current);
#endif
    ele_par_filter();
    APP_DLTJ_DEBUG("cur:%d power:%d vol:%d",power_cnt.cur_current,\
    power_cnt.cur_power,power_cnt.cur_vol);
    if(g_dltj.over_curr){
        if(power_cnt.cur_current >= g_dltj.over_curr){
            over_protect(); //overcurrent event 
        }
    }
}

/*********************************************************************************
* FUNCTION:       update_ele_data
* DESCRIPTION:    Report the increased battery value
* INPUT:          time: timestamp; ele_value: increase power value
* OUTPUT:         none
* RETURN:         OPERATE_RET:Initialization state, return numeric state value
* OTHERS:         Power reporting interface function
* HISTORY:        2020-03-04
*******************************************************************************/
STATIC OPERATE_RET update_ele_data(IN UINT_T time,IN UINT_T ele_value)
{
    OPERATE_RET op_ret = OPRT_OK;

    INT_T dp_cnt = 1;

    TY_OBJ_DP_S *dp_arr = (TY_OBJ_DP_S *)Malloc(dp_cnt*SIZEOF(TY_OBJ_DP_S));
    if(NULL == dp_arr) {
        PR_ERR("malloc failed");
        return OPRT_MALLOC_FAILED;
    }

    memset(dp_arr, 0, dp_cnt*SIZEOF(TY_OBJ_DP_S));

    dp_arr[0].dpid = g_dltj.dp_info.edpid;
    dp_arr[0].time_stamp = time;
    dp_arr[0].type = PROP_VALUE;
    dp_arr[0].value.dp_value = ele_value;

    op_ret = dev_report_dp_stat_sync(get_gw_cntl()->gw_if.id,dp_arr,dp_cnt,5);
    Free(dp_arr);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async relay_config data error,err_num",op_ret);
        return op_ret;
    }

    PR_DEBUG("dp_query report_all_dp_data");
    return OPRT_OK;
}

/*********************************************************************************
* FUNCTION:       report_local_data
* DESCRIPTION:    Report local storage power value
* INPUT:          none
* OUTPUT:         none
* RETURN:         none
* OTHERS:         Report power value and corresponding timestamp
* HISTORY:        2020-03-04
*******************************************************************************/
STATIC VOID report_local_data(VOID)
{
    INT_T i,j;
    UINT_T val_size = power_cnt.loc_data_cnt;
    INT_T cha_flag = 0;
    if(0 == val_size)
        return ;
    for(i = 0;i < val_size; i++){
        if(update_ele_data(power_cnt.ele_sav_data[0].unix_time,power_cnt.ele_sav_data[0].ele_energy) != OPRT_OK){
            break;
        }
        APP_DLTJ_DEBUG("Upload cnt:%d,curr_i:%d,unix_time:%d,ele_energy:%d success...",power_cnt.loc_data_cnt,i,\
                power_cnt.ele_sav_data[0].unix_time,power_cnt.ele_sav_data[0].ele_energy);
        cha_flag = 1;
        for(j=0; j<val_size-1; j++){
            power_cnt.ele_sav_data[j].unix_time = power_cnt.ele_sav_data[j+1].unix_time;
            power_cnt.ele_sav_data[j].ele_energy = power_cnt.ele_sav_data[j+1].ele_energy;
        }
        power_cnt.loc_data_cnt -= 1;
    }

    if(1 == cha_flag){
        if(OPRT_OK != set_ele_data()){
            PR_ERR("set ele to flash err ...");
        }
    }
}

/*********************************************************************************
* FUNCTION:       same_day_judge
* DESCRIPTION:    Timestamp judgment
* INPUT:          u_time1: timestamp 1; u_time2: timestamp 2
* OUTPUT:         none
* RETURN:         BOOL: FLASE is not the same day, TRUE is the same day
* OTHERS:         Determine if two timestamps are the same day
* HISTORY:        2020-03-04
*******************************************************************************/
STATIC BOOL_T same_day_judge(UINT_T u_time1, UINT_T u_time2)
{
    return (u_time1+28800)/86400 == (u_time2+28800)/86400 ? TRUE: FALSE;
}

/*********************************************************************************
* FUNCTION:       addto_local_data
* DESCRIPTION:    Local power handling
* INPUT:          time: timestamp, ele: battery value
* OUTPUT:         none
* RETURN:         none
* OTHERS:         It can store up to 10 days of electricity value, the storage format is: {"ele_time":time,"ele_value":xxx}
* HISTORY:        2020-03-04
*******************************************************************************/
STATIC VOID addto_local_data(UINT_T time, UINT_T ele)
{
    APP_DLTJ_DEBUG("try to add to local data time:%d,ele:%d",time,ele);

    if(power_cnt.loc_data_cnt != 0){
        if(time == 0){
            power_cnt.ele_sav_data[power_cnt.loc_data_cnt-1].ele_energy += ele;
            return;
        }
        //The power is stored in the flash by the day
        if(same_day_judge(power_cnt.ele_sav_data[power_cnt.loc_data_cnt-1].unix_time,time)){
            power_cnt.ele_sav_data[power_cnt.loc_data_cnt-1].unix_time = time;
            power_cnt.ele_sav_data[power_cnt.loc_data_cnt-1].ele_energy += ele;
            APP_DLTJ_DEBUG("Same day ...%d %d",power_cnt.ele_sav_data[power_cnt.loc_data_cnt-1].ele_energy,ele);
        }else{
            if(power_cnt.loc_data_cnt == 10){
                //for(i=0; i<9; i++){
                //	power_cnt.ele_sav_data[i].unix_time = power_cnt.ele_sav_data[i+1].unix_time;
                //	power_cnt.ele_sav_data[i].ele_energy = power_cnt.ele_sav_data[i+1].ele_energy;
                //}
                //power_cnt.ele_sav_data[power_cnt.loc_data_cnt-1].unix_time = time;
                power_cnt.ele_sav_data[power_cnt.loc_data_cnt-1].ele_energy += ele; //After the maximum storage number is reached, the subsequent power will be stored for the last day
            }else{
                power_cnt.ele_sav_data[power_cnt.loc_data_cnt].unix_time = time;
                power_cnt.ele_sav_data[power_cnt.loc_data_cnt].ele_energy = ele;
                power_cnt.loc_data_cnt++;
            }

        }
    }else{
        power_cnt.ele_sav_data[0].unix_time = time;
        power_cnt.ele_sav_data[0].ele_energy = ele;
        power_cnt.loc_data_cnt++;
    }
    APP_DLTJ_DEBUG("set loc_data_cnt : %d",power_cnt.loc_data_cnt);
    if(OPRT_OK != set_ele_data()){
        APP_DLTJ_DEBUG("set ele to flash err ...");
    }
}

/*Before network activation, all newly added power is stored in power_cnt.tem_ele_val and stored in TEM_ELE_SAVE_KEY area,
  After activation, report power_cnt.tem_ele_val and clear the TEM_ELE_SAVE_KEY. After that, try the newly added power first.
  Report, if the report fails, it will be stored in the ele_sav_data array. In the networked state, the power thread will try to report the ele_sav_data array every cycle.*/
 /*********************************************************************************
 * FUNCTION:       add_ele_handle
 * DESCRIPTION:    Increment battery value parameter event
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Incremental battery value event processing;
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC VOID add_ele_handle(VOID)
{
    get_ele(&power_cnt.add_ele);
    get_time_posix(&power_cnt.cur_posix);
#if _DLTJ_REPT_TEST
    power_cnt.add_ele = 10;
#endif
    power_cnt.tem_ele_val += power_cnt.add_ele;
    APP_DLTJ_DEBUG("add_ele = %d,tem_ele_val = %d",power_cnt.add_ele,power_cnt.tem_ele_val);
    power_cnt.add_ele = 0;
    //Power handling event trigger
    if((power_cnt.tem_ele_val >= ADD_ELE_THRE && ele_handle_time >= ADD_TIME_MIN_THRE )\
    || ele_handle_time >= ADD_TIME_MAX_THRE){
        ele_handle_time = 0;
        if (power_cnt.tem_ele_val > CPREEPING_POWER_THRE) {
            if(ELE_NOT_ACTIVE == ele_state){
                set_tem_ele_val(power_cnt.tem_ele_val);
                APP_DLTJ_DEBUG("set tem ele val :%d success...",power_cnt.tem_ele_val);
            }else if(ELE_NORMAL == ele_state){
                if(OPRT_OK != update_ele_data(power_cnt.cur_posix, power_cnt.tem_ele_val)){
                    addto_local_data(power_cnt.cur_posix, power_cnt.tem_ele_val);
                }
                power_cnt.tem_ele_val = 0;
            }else if(ELE_UNCONNECT == ele_state){
                addto_local_data(power_cnt.cur_posix, power_cnt.tem_ele_val);
                power_cnt.tem_ele_val = 0;
            }
        } else {
            power_cnt.tem_ele_val = 0;
        }
    }
}

/*********************************************************************************
 * FUNCTION:       small_power_report
 * DESCRIPTION:    low power report
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         BOOL: FALSE (do not report), TRUE (report)
 * OTHERS:         Whether the low power is reported;
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC BOOL_T small_power_report(VOID)
{
    if(abs(power_cnt.cur_power - power_cnt.last_power) > MAX_POWER_ABS) {
        return TRUE;
    }
    return FALSE;
}

/*********************************************************************************
 * FUNCTION:       value_range_judge
 * DESCRIPTION:    Rate of change judgment function
 * INPUT:          JudgedValue: judgment value; TargetValue: standard value; range: rate of change
 * OUTPUT:         none
 * RETURN:         BOOL: FALSE (do not report), TRUE (report)
 * OTHERS:         Whether to report low power；
 * HISTORY:        2020-03-04
 *******************************************************************************/
//Determine if JudgedValue is within the range% of TargetValue's positive and negative range, if so, return 1, if not, return 0
//JudgedValue is the judged value, TargetValue is the target value of the judged value, and range is the floating range of 0-100(%).
STATIC BOOL_T value_range_judge(IN UINT_T JudgedValue,IN UINT_T TargetValue,IN UINT_T range)
{
    if((JudgedValue * 100 >= TargetValue * (100 - range)) && \
        (JudgedValue * 100 <= TargetValue * (100 + range))){
        return TRUE;
    }else{
        return FALSE;
    }
}

/*********************************************************************************
 * FUNCTION:       if_pvi_need_report
 * DESCRIPTION:    Whether real-time power data needs to be reported
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         BOOL: FALSE (no need to report), TRUE (requires report)
 * OTHERS:         Power/voltage rate of change to determine whether to report real-time power data
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC BOOL_T if_pvi_need_report(VOID)
{
    if((!value_range_judge(power_cnt.cur_power,power_cnt.last_power,REPT_THRE_PWR) &&small_power_report())\
        ||!value_range_judge(power_cnt.cur_vol,power_cnt.last_vol,REPT_THRE_VOL)){
        pvi_report_min_time = handle_time.pvi_small_thre;
        return TRUE;
    }else{
        pvi_report_min_time = handle_time.pvi_big_thre;
        return FALSE;
    }
}
/*********************************************************************************
 * FUNCTION:       report_pvi_data
 * DESCRIPTION:    Report real-time power data
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         OPERATE_RET: return value result
 * OTHERS:         Current/voltage power current value
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC OPERATE_RET report_pvi_data(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    INT_T dp_cnt = 3;//Channel dp + power dp (does not include power value)

    TY_OBJ_DP_S *dp_arr = (TY_OBJ_DP_S *)Malloc(dp_cnt*SIZEOF(TY_OBJ_DP_S));
    if(NULL == dp_arr) {
        PR_ERR("malloc failed");
        return OPRT_MALLOC_FAILED;
    }

    memset(dp_arr, 0, dp_cnt*SIZEOF(TY_OBJ_DP_S));
    dp_arr[0].dpid = g_dltj.dp_info.idpid;
    dp_arr[0].time_stamp = 0;
    dp_arr[0].type = PROP_VALUE;
    dp_arr[0].value.dp_value = power_cnt.cur_current;

    dp_arr[1].dpid = g_dltj.dp_info.pdpid;
    dp_arr[1].time_stamp = 0;
    dp_arr[1].type = PROP_VALUE;
    dp_arr[1].value.dp_value = power_cnt.cur_power;

    dp_arr[2].dpid = g_dltj.dp_info.vdpid;
    dp_arr[2].time_stamp = 0;
    dp_arr[2].type = PROP_VALUE;
    dp_arr[2].value.dp_value = power_cnt.cur_vol;

    op_ret = dev_report_dp_json_async(get_gw_cntl()->gw_if.id,dp_arr,dp_cnt);
    Free(dp_arr);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async relay_config data error,err_num",op_ret);
        return op_ret;
    }

    PR_DEBUG("dp_query report_all_dp_data");
    return OPRT_OK;
}
/*********************************************************************************
 * FUNCTION:       save_reported_pvi_data
 * DESCRIPTION:    Update current real-time parameters
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         The global variables of power statistics parameters are updated;
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC VOID save_reported_pvi_data(VOID)
{
    power_cnt.last_current = power_cnt.cur_current;
    power_cnt.last_vol = power_cnt.cur_vol;
    power_cnt.last_power = power_cnt.cur_power;
}

BOOL_T hw_wifi_net_cloud(VOID)
{
    GW_WIFI_NW_STAT_E cur_nw_stat;
    OPERATE_RET op_ret = get_wf_gw_nw_status(&cur_nw_stat);
    if ((op_ret == OPRT_OK)
        && (cur_nw_stat >= STAT_CLOUD_CONN)) {
            return true;
    }
    return false;
}
/*********************************************************************************
 * FUNCTION:       app_dltj_proc
 * DESCRIPTION:    Power statistics reporting thread
 * INPUT:          pArg：线程参数
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Processing power parameters: real-time electrical parameter reporting (current, voltage and power) and increase the storage and reporting of power values;
 * HISTORY:        2020-03-04
 *******************************************************************************/
 #if 0
STATIC VOID app_dltj_proc(PVOID_T pArg)
#else
STATIC VOID app_dltj_timer_cb(UINT_T timerID,PVOID_T pTimerArg)
#endif
{
    switch(ele_state){
    case ELE_NOT_ACTIVE:
        if(hw_wifi_net_cloud()) {
            ele_state = ELE_NORMAL;//The first time the network is activated, the temporary power will be reported and the temporary power memory will be cleared.
            pvi_handle_time = pvi_report_min_time;
            ele_handle_time = ADD_TIME_MAX_THRE;
            set_tem_ele_val(0);
            #if _IF_REPT_COE
            if(OPRT_OK != report_coe_data()){
                PR_DEBUG("report coe data fail!!!");
            }
            #endif
        }
        break;
    case ELE_NORMAL:
    case ELE_UNCONNECT:
        if(!hw_wifi_net_cloud()) {
            ele_state = ELE_UNCONNECT;
        }else {
            ele_state = ELE_NORMAL;
        }
        break;
    }
    pvi_data_handle();
    add_ele_handle();
    if(ele_state == ELE_NORMAL){
        if((pvi_handle_time >= pvi_report_min_time && if_pvi_need_report())\
        || pvi_handle_time >= PVI_REPORT_MAX_THRE){
            pvi_handle_time = 0;
            if(OPRT_OK == report_pvi_data()){
                save_reported_pvi_data();
            }
        }
        pvi_handle_time += ELE_SLEEP_TIME;
        if(power_cnt.loc_data_cnt != 0){
            report_local_data();
        }
    }
    PR_DEBUG("remain size:%d",tuya_hal_system_getheapsize());
    APP_DLTJ_DEBUG(" ele state :%d,pvi time:%d, ele time:%d,cur_posix :%d,remain size:%d",\
        ele_state,pvi_handle_time,ele_handle_time,power_cnt.cur_posix,tuya_hal_system_getheapsize());
    ele_handle_time += ELE_SLEEP_TIME;
}

/*********************************************************************************
 * FUNCTION:       clear_ele_data
 * DESCRIPTION:    Clear the increased power value in the flash
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Clear off-grid power value and undistributed grid power value
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC VOID clear_ele_data(VOID)
{
    INT_T tem_ele = 0;
    if((OPRT_OK == get_tem_ele_val(&tem_ele)) && tem_ele != 0) {
        set_tem_ele_val(0);
    }
    if(OPRT_OK == get_ele_data() && (power_cnt.loc_data_cnt != 0)) {
        power_cnt.loc_data_cnt = 0;
        set_ele_data();
    }
}

OPERATE_RET app_dltj_init(IN APP_DLTJ_MODE mode)
{
    OPERATE_RET op_ret = OPRT_OK;
    if(g_dltj.if_have){
        if(APP_DLTJ_NORMAL == mode){
            op_ret = dltj_driver_init(mode);
            if(OPRT_OK != op_ret){
                PR_ERR("mode:%d,dltj driver init err!!!",mode);
                return op_ret;
            }
            if(OPRT_OK != get_ele_data()){
                APP_DLTJ_DEBUG("get ele data err...");
            }
            if(OPRT_OK != get_tem_ele_val(&power_cnt.tem_ele_val)){
                APP_DLTJ_DEBUG("get tem ele data err...");
            }
            #if 0
            THRD_PARAM_S thrd_param;
            thrd_param.priority = TRD_PRIO_2;
            thrd_param.stackDepth = 1024+512;
            thrd_param.thrdname = "app_dltj_task";
            op_ret = CreateAndStart(&mAppDltj.app_dltj_thread,NULL,NULL,app_dltj_proc,NULL,&thrd_param);
            if(op_ret != OPRT_OK) {
                PR_ERR("creat ele thread err...");
                return op_ret;
            }
            #else
            op_ret = sys_add_timer(app_dltj_timer_cb,NULL,&mAppDltj.app_dltj_timer);
            if(OPRT_OK != op_ret) {
                PR_ERR("app_dltj_timer fail,err_num:%d",op_ret);
                return op_ret;
            }else {
                sys_start_timer(mAppDltj.app_dltj_timer,1000,TIMER_CYCLE);
            }
            #endif
        }else{
            clear_ele_data();
            op_ret = dltj_driver_init(mode);
            if(OPRT_OK != op_ret){
                PR_ERR("dltj cal err! product test err!");
                return op_ret;
            }
        }
    }else{
        PR_DEBUG("There not exist dltj");
    }

        return OPRT_OK;
}

/*********************************************************************************
 * FUNCTION:       report_fault_warn
 * DESCRIPTION:    Fault event reporting
 * INPUT:          dpid_fault: fault dp; fault_word: fault value
 * OUTPUT:         none
 * RETURN:         OPERATE_RET: Initialize state, return numeric state value
 * OTHERS:         Fault event reporting
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC OPERATE_RET report_fault_warn(IN UCHAR_T dpid_fault,IN INT_T fault_word)
{
    OPERATE_RET op_ret = OPRT_OK;

    UINT_T dp_cnt = 1;

    TY_OBJ_DP_S *dp_arr = (TY_OBJ_DP_S *)Malloc(dp_cnt*SIZEOF(TY_OBJ_DP_S));
    if(NULL == dp_arr) {
        PR_ERR("malloc failed");
        return OPRT_MALLOC_FAILED;
    }

    memset(dp_arr, 0, dp_cnt*SIZEOF(TY_OBJ_DP_S));

    dp_arr[0].dpid = dpid_fault;
    dp_arr[0].time_stamp = 0;
    dp_arr[0].type = PROP_VALUE;
    dp_arr[0].value.dp_value = fault_word;

//    op_ret = dev_report_dp_json_async(get_gw_cntl()->gw_if.id,dp_arr,dp_cnt);
    op_ret = dev_report_dp_json_async(get_gw_cntl()->gw_if.id,dp_arr,dp_cnt);
    Free(dp_arr);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async relay_config data error,err_num",op_ret);
        return op_ret;
    }

    PR_DEBUG("dp_query report_all_dp_data");
    return OPRT_OK;
}

/*********************************************************************************
 * FUNCTION:       report_over_curr
 * DESCRIPTION:    Report overcurrent events
 * INPUT:          none
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Overcurrent data reporting
 * HISTORY:        2020-03-04
 *******************************************************************************/
VOID report_over_curr(VOID)
{
    if(g_dltj.dp_info.fault_dpid){
        report_fault_warn(g_dltj.dp_info.fault_dpid,0x01);
    }else{
        PR_DEBUG("old oem dltj ,no need to report fault");
    }
}

VOID reset_clear_temp_ele(VOID)
{
    CHAR_T save_temp_ele[18] = "tem_ele_save_key";
    OPERATE_RET op_ret = OPRT_OK;

    op_ret = ufdelete(save_temp_ele);
    if(OPRT_OK != op_ret) {
        PR_ERR("reset_clear_temp_ele ufdelete error,err_num:%d",op_ret);
    }
    PR_DEBUG("clear ele_value default");
}

VOID reset_clear_ele(VOID)
{
    CHAR_T save_cur_ele[14] = "ele_save_key";
    OPERATE_RET op_ret = OPRT_OK;
    op_ret = ufdelete(save_cur_ele);
    if(OPRT_OK != op_ret) {
        PR_ERR("reset_clear_ele ufdelete error,err_num:%d",op_ret);
    }
    PR_DEBUG("clear ele_value default");
}

VOID switch_ele_dp_query(VOID)
{
    if (FALSE == handle_time.is_dp_query) {
        handle_time.is_dp_query = TRUE;
        handle_time.pvi_small_thre = PVI_REPORT_SMALL_THRE_DP_QUERY;
        handle_time.pvi_big_thre = PVI_REPORT_BIG_THRE_DP_QUERY;
        PR_NOTICE("support ele dp query, not report actively from now.");
    }
}

/***********************************************************
*   Function:  hw_report_all_dp_status
*   Input:     none
*   Output:    none
*   Return:    none
*   Notice:    Report all dp data
***********************************************************/
VOID hw_report_all_dp_status(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    INT_T dp_cnt = 0;
    dp_cnt = g_hw_table.channel_num*2+3; //Channel dp + power dp (does not include power value)

    TY_OBJ_DP_S *dp_arr = (TY_OBJ_DP_S *)Malloc(dp_cnt*SIZEOF(TY_OBJ_DP_S));
    if(NULL == dp_arr) {
        PR_ERR("malloc failed");
        return;
    }

    memset(dp_arr, 0, dp_cnt*SIZEOF(TY_OBJ_DP_S));

    dp_arr[0].dpid = g_hw_table.channels[0].cd_dpid;
    dp_arr[0].type = PROP_VALUE;
    dp_arr[0].time_stamp = 0;
    if(g_hw_table.channels[0].cd_sec > 0) {
        dp_arr[0].value.dp_value = g_hw_table.channels[0].cd_sec;
    }else {
        dp_arr[0].value.dp_value = 0;
    }

    dp_arr[1].dpid = g_hw_table.channels[0].dpid;
    dp_arr[1].time_stamp = 0;
    dp_arr[1].type = PROP_BOOL;
    dp_arr[1].value.dp_bool = g_hw_table.channels[0].stat;

    dp_arr[2].dpid = g_dltj.dp_info.idpid;
    dp_arr[2].time_stamp = 0;
    dp_arr[2].type = PROP_VALUE;
    dp_arr[2].value.dp_value = power_cnt.cur_current;

    dp_arr[3].dpid = g_dltj.dp_info.pdpid;
    dp_arr[3].time_stamp = 0;
    dp_arr[3].type = PROP_VALUE;
    dp_arr[3].value.dp_value = power_cnt.cur_power;

    dp_arr[4].dpid = g_dltj.dp_info.vdpid;
    dp_arr[4].time_stamp = 0;
    dp_arr[4].type = PROP_VALUE;
    dp_arr[4].value.dp_value = power_cnt.cur_vol;

    op_ret = dev_report_dp_json_async(get_gw_cntl()->gw_if.id,dp_arr,dp_cnt);
    Free(dp_arr);
    if(OPRT_OK != op_ret) {
        PR_ERR("dev_report_dp_json_async relay_config data error,err_num",op_ret);
    }

    PR_DEBUG("dp_query report_all_dp_data");
    return;
}

