/***********************************************************
*  File: tuya_led.c
*  Author: nzy
*  Date: 20171117
***********************************************************/
#define __TUYA_LED_GLOBALS
#include "mem_pool.h"
#include "tuya_led.h"
#include "sys_timer.h"
#include "tuya_hal_mutex.h"
#include "uni_log.h"


/***********************************************************
*************************micro define***********************
***********************************************************/
typedef struct led_cntl {
    struct led_cntl *next;
    TY_GPIO_PORT_E port;
    LED_LT_E type;
    USHORT_T flash_cycle; // ms
    USHORT_T expire_time;
    USHORT_T flh_sum_time; // if(flh_sum_time == 0xffff) then flash forever;
}LED_CNTL_S;

#define LED_TIMER_VAL_MS 100
#define LED_TIMER_UNINIT 0xffff

/***********************************************************
*************************variable define********************
***********************************************************/
STATIC LED_CNTL_S *led_cntl_list = NULL;
STATIC TIMER_ID led_timer = LED_TIMER_UNINIT;
STATIC MUTEX_HANDLE mutex = NULL;

/***********************************************************
*************************function define********************
***********************************************************/
STATIC VOID __led_timer_cb(UINT_T timerID,PVOID_T pTimerArg);

/***********************************************************
*  Function: tuya_create_led_handle
*  Input: port
*         high->default output port high/low
*  Output: handle
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET tuya_create_led_handle(IN CONST TY_GPIO_PORT_E port,IN CONST BOOL_T high,OUT LED_HANDLE *handle)
{
    if(NULL == handle) {
        return OPRT_INVALID_PARM;
    }
    OPERATE_RET op_ret = OPRT_OK;
    // create mutex
    if(NULL == mutex) {
        op_ret = tuya_hal_mutex_create_init(&mutex);
        if(OPRT_OK != op_ret) {
            PR_ERR("tuya_hal_mutex_create_init err:%d",op_ret);
            return op_ret;
        }
    }

    // create flash timer
    if(led_timer == LED_TIMER_UNINIT) {
        op_ret = sys_add_timer(__led_timer_cb,NULL,&led_timer);
        if(OPRT_OK != op_ret) {
            return op_ret;
        }else {
            op_ret = sys_start_timer(led_timer,LED_TIMER_VAL_MS,TIMER_CYCLE);
            if(op_ret != OPRT_OK)
            {
                PR_ERR("Start timer fails");
                return op_ret;
            }
        }
    }
    op_ret = tuya_gpio_inout_set(port,FALSE);
    if(OPRT_OK != op_ret) {
        return op_ret;
    }
    op_ret = tuya_gpio_write(port,high);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_gpio_write err:%d",op_ret);
        return op_ret;
    }
    LED_CNTL_S *len_cntl = (LED_CNTL_S *)Malloc(SIZEOF(LED_CNTL_S));
    if(NULL == len_cntl) {
        return OPRT_MALLOC_FAILED;
    }
    memset(len_cntl,0,sizeof(LED_CNTL_S));
    len_cntl->port = port;

    *handle = (LED_HANDLE)len_cntl;
    tuya_hal_mutex_lock(mutex);
    if(led_cntl_list) {
        len_cntl->next = led_cntl_list;
    }
    led_cntl_list = len_cntl;
    tuya_hal_mutex_unlock(mutex);

    return OPRT_OK;
}

/***********************************************************
*  Function: tuya_create_led_handle_select
*  Input: port
*         high->default output port high/low
*  Output: handle
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET tuya_create_led_handle_select(IN CONST TY_GPIO_PORT_E port,IN CONST BOOL_T high,OUT LED_HANDLE *handle)
{
    if(NULL == handle) {
        return OPRT_INVALID_PARM;
    }
    OPERATE_RET op_ret = OPRT_OK;
    // create mutex
    if(NULL == mutex) {
        op_ret = tuya_hal_mutex_create_init(&mutex);
        if(OPRT_OK != op_ret) {
            PR_ERR("tuya_hal_mutex_create_init err:%d",op_ret);
            return op_ret;
        }
    }

    // create flash timer
    if(led_timer == LED_TIMER_UNINIT) {
        op_ret = sys_add_timer(__led_timer_cb,NULL,&led_timer);
        if(OPRT_OK != op_ret) {
            return op_ret;
        }else {
            op_ret = sys_start_timer(led_timer,LED_TIMER_VAL_MS,TIMER_CYCLE);
            if(op_ret != OPRT_OK)
            {
                PR_ERR("Start timer fails");
                return op_ret;
            }
        }
    }

    op_ret = tuya_gpio_inout_set_select(port,FALSE,high);
    if(OPRT_OK != op_ret) {
        return op_ret;
    }
    #if 0
    op_ret = tuya_gpio_write(port,high);
    if(OPRT_OK != op_ret) {
        PR_ERR("tuya_gpio_write err:%d",op_ret);
        return op_ret;
    }
    #endif
    LED_CNTL_S *len_cntl = (LED_CNTL_S *)Malloc(SIZEOF(LED_CNTL_S));
    if(NULL == len_cntl) {
        return OPRT_MALLOC_FAILED;
    }
    memset(len_cntl,0,sizeof(LED_CNTL_S));
    len_cntl->port = port;

    *handle = (LED_HANDLE)len_cntl;
    tuya_hal_mutex_lock(mutex);
    if(led_cntl_list) {
        len_cntl->next = led_cntl_list;
    }
    led_cntl_list = len_cntl;
    tuya_hal_mutex_unlock(mutex);

    return OPRT_OK;
}

STATIC VOID __set_led_light_type(IN CONST LED_HANDLE handle,IN CONST LED_LT_E type,\
                                           IN CONST USHORT_T flh_mstime,IN CONST USHORT_T flh_ms_sumtime)
{
    LED_CNTL_S *led_cntl = (LED_CNTL_S *)handle;
//     PR_DEBUG("port = %d",led_cntl->port);
    led_cntl->type = type;
    if(OL_LOW == type) {
        tuya_gpio_write(led_cntl->port,FALSE);
    }else if(OL_HIGH == type) {
       
        if(tuya_gpio_write(led_cntl->port,TRUE) != OPRT_OK){
            PR_ERR("tuya_gpio_write err");
        }
    }else {
        led_cntl->flh_sum_time = flh_ms_sumtime;
        led_cntl->flash_cycle = flh_mstime;
        led_cntl->expire_time = 0;
    }
}

/***********************************************************
*  Function: tuya_set_led_light_type
*  Input: handle type flh_mstime flh_ms_sumtime
*  Output: none
*  Return: OPERATE_RET
*  note: if(OL_FLASH == type) then flh_mstime and flh_ms_sumtime is valid
***********************************************************/
VOID tuya_set_led_light_type(IN CONST LED_HANDLE handle,IN CONST LED_LT_E type,\
                                        IN CONST USHORT_T flh_mstime,IN CONST USHORT_T flh_ms_sumtime)
{
    tuya_hal_mutex_lock(mutex);
    __set_led_light_type(handle,type,flh_mstime,flh_ms_sumtime);
    tuya_hal_mutex_unlock(mutex);
}

STATIC VOID __led_timer_cb(UINT_T timerID,PVOID_T pTimerArg)
{
    tuya_hal_mutex_lock(mutex);
    LED_CNTL_S *tmp_led_cntl_list = led_cntl_list;
    if(NULL == tmp_led_cntl_list) {
        tuya_hal_mutex_unlock(mutex);
        return;
    }

    while(tmp_led_cntl_list) {
        if(tmp_led_cntl_list->type >= OL_FLASH_LOW) {
            tmp_led_cntl_list->expire_time += LED_TIMER_VAL_MS;
            if(tmp_led_cntl_list->expire_time >= tmp_led_cntl_list->flash_cycle) {
                tmp_led_cntl_list->expire_time = 0;
                if(tmp_led_cntl_list->type == OL_FLASH_LOW) {
                    tmp_led_cntl_list->type = OL_FLASH_HIGH;
                    tuya_gpio_write(tmp_led_cntl_list->port,FALSE);
                }else {
                    tmp_led_cntl_list->type = OL_FLASH_LOW;
                    tuya_gpio_write(tmp_led_cntl_list->port,TRUE);
                }
            }

            // the led flash and set deadline time
            if(tmp_led_cntl_list->flh_sum_time != 0xffff) {
                if(tmp_led_cntl_list->flh_sum_time >= LED_TIMER_VAL_MS) {
                    tmp_led_cntl_list->flh_sum_time -= LED_TIMER_VAL_MS;
                }else {
                    tmp_led_cntl_list->flh_sum_time = 0;
                }
            
                if(0 == tmp_led_cntl_list->flh_sum_time) {
                    if(tmp_led_cntl_list->type == OL_FLASH_LOW) {
                        __set_led_light_type(tmp_led_cntl_list,OL_LOW,0,0);
                    }else {
                        __set_led_light_type(tmp_led_cntl_list,OL_HIGH,0,0);
                    }
                }
            }
        }
        
        tmp_led_cntl_list = tmp_led_cntl_list->next;
    }
    tuya_hal_mutex_unlock(mutex);
}








