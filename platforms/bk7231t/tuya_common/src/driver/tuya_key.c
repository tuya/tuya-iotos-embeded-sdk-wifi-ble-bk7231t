/***********************************************************
*  File: tuya_key.c
*  Author: nzy
*  Date: 20171117
***********************************************************/
#define __TUYA_KEY_GLOBALS
#include <stdio.h>
#include <string.h>
#include "tuya_cloud_types.h"
#include "mem_pool.h"
#include "tuya_key.h"
#include "sys_timer.h"
#include "tuya_hal_mutex.h"
#include "uni_log.h"

/***********************************************************
*************************micro define***********************
***********************************************************/

typedef enum {
    KEY_DOWN = 0,
    KEY_DOWN_CONFIRM,
    KEY_DOWNNING, 
    KEY_UP_CONFIRM,
    KEY_UPING,
    KEY_FINISH,
}KEY_STAT_E;


typedef struct {
    // user define
    KEY_USER_DEF_S kud;

    // run variable
    KEY_STAT_E status;
    BOOL_T long_key_press;
    BOOL_T key_val_last; //press ? TRUE : FALSE and init it with TRUE
    INT_T down_time; // ms
    INT_T up_time;
    INT_T seq_key_cnt;
}KEY_ENTITY_S;

typedef struct ken_en_lst{
    struct ken_en_lst *nxt;
    KEY_ENTITY_S key_ent;
}KEY_EN_LST_S;

typedef struct {
    KEY_ENTITY_S *p_tbl;

    TIMER_ID kd_timer; // key detect timer
    INT_T tbl_cnt;
    INT_T timer_space; // default 20ms

    KEY_EN_LST_S *lst;
    MUTEX_HANDLE mutex;
}KEY_MANAGE_S;

#define TIMER_SPACE_MAX 100 // ms
/***********************************************************
*************************variable define********************
***********************************************************/


STATIC KEY_MANAGE_S *key_mag = NULL;

/***********************************************************
*************************function define********************
***********************************************************/
STATIC VOID __key_timer_cb(UINT_T timerID,PVOID_T pTimerArg);
STATIC VOID __key_handle(VOID);
STATIC VOID __key_ent_proc(INOUT KEY_ENTITY_S *key_ent);

/***********************************************************
*  Function: key_init
*  Input: p_tbl cnt
*         timer_space->if timer (space == 0) then use default value(20ms)
*  Output: 
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET key_init(IN CONST KEY_USER_DEF_S *p_tbl,IN CONST INT_T cnt,\
                         IN CONST INT_T timer_space)
{
    if(key_mag) {
        return OPRT_OK;
    }

    if(timer_space > TIMER_SPACE_MAX) {
        PR_ERR("Invalid param");
        return OPRT_INVALID_PARM;
    }

    key_mag = (KEY_MANAGE_S *)Malloc(SIZEOF(KEY_MANAGE_S));
    if(NULL == key_mag) {
        PR_ERR("Malloc err");
        return OPRT_MALLOC_FAILED;
    }
    memset(key_mag,0,SIZEOF(KEY_MANAGE_S));

    OPERATE_RET op_ret = OPRT_OK;
    if(cnt != 0){
        key_mag->p_tbl =(KEY_ENTITY_S *)Malloc(SIZEOF(KEY_ENTITY_S) * cnt);
        if(NULL == key_mag->p_tbl) {
            PR_ERR("Malloc err");
            op_ret = OPRT_MALLOC_FAILED;
            goto ERR_EXIT;
        }
        memset(key_mag->p_tbl,0,SIZEOF(KEY_ENTITY_S) * cnt);
    }
    else{
        key_mag->p_tbl = NULL;
    }
    key_mag->tbl_cnt = cnt;
    key_mag->timer_space = timer_space;
    if(0 == key_mag->timer_space) {
        key_mag->timer_space = 20;
    }
    key_mag->lst = NULL;
    // init
    INT_T i = 0;
    for(i = 0;i < cnt;i++) {
        memcpy(&(key_mag->p_tbl[i].kud),&p_tbl[i],SIZEOF(KEY_USER_DEF_S));
        key_mag->p_tbl[i].key_val_last = TRUE;
        op_ret = tuya_gpio_inout_set(key_mag->p_tbl[i].kud.port,TRUE);
        if(OPRT_OK != op_ret) {
            PR_ERR("tuya_gpio_inout_set err:%d",op_ret);
            goto ERR_EXIT;
        }
    }

    op_ret = tuya_hal_mutex_create_init(&key_mag->mutex);
    if(OPRT_OK != op_ret) {
        goto ERR_EXIT;
    }

    op_ret = sys_add_timer(__key_timer_cb,NULL,&key_mag->kd_timer);
    if(op_ret != OPRT_OK) {
        goto ERR_EXIT;
    }
    op_ret = sys_start_timer(key_mag->kd_timer,key_mag->timer_space,TIMER_CYCLE);
    if(op_ret != OPRT_OK) {
        PR_ERR("start timer err");
        goto ERR_EXIT;
    }
    return OPRT_OK;

ERR_EXIT:
    if(key_mag) {
        Free(key_mag->p_tbl);
        Free(key_mag);
        key_mag = NULL;
    }

    return op_ret;
}

/***********************************************************
*  Function: reg_proc_key
*  Input: 
*  Output: 
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET reg_proc_key(IN CONST KEY_USER_DEF_S *key_ud)
{
    if(NULL == key_ud) {
        return OPRT_INVALID_PARM;
    }
    if(NULL == key_ud->call_back) {
        return OPRT_INVALID_PARM;
    }
    // is registered?
    KEY_EN_LST_S *tmp_key_ent_lst = key_mag->lst;
    OPERATE_RET op_ret = OPRT_OK;
    tuya_hal_mutex_lock(key_mag->mutex);
    while(tmp_key_ent_lst) {
        if(tmp_key_ent_lst->key_ent.kud.port == key_ud->port) {
            op_ret = tuya_gpio_inout_set(key_ud->port,TRUE);
            if(OPRT_OK != op_ret) {
                tuya_hal_mutex_unlock(key_mag->mutex);
                return op_ret;
            }
            memcpy(&(tmp_key_ent_lst->key_ent.kud),key_ud,SIZEOF(KEY_USER_DEF_S));
            tuya_hal_mutex_unlock(key_mag->mutex);
            return OPRT_OK;
        }
        tmp_key_ent_lst = tmp_key_ent_lst->nxt;
    }
    tuya_hal_mutex_unlock(key_mag->mutex);
    KEY_EN_LST_S *key_ent_lst = (KEY_EN_LST_S *)Malloc(SIZEOF(KEY_EN_LST_S));
    if(NULL == key_ent_lst) {
        return OPRT_MALLOC_FAILED;
    }
    memset(key_ent_lst,0,SIZEOF(KEY_EN_LST_S));
    memcpy(&(key_ent_lst->key_ent.kud),key_ud,SIZEOF(KEY_USER_DEF_S));
    key_ent_lst->key_ent.key_val_last = TRUE;
    
    op_ret = tuya_gpio_inout_set(key_ud->port,TRUE);
    if(OPRT_OK != op_ret) {
        return op_ret;
    }

    tuya_hal_mutex_lock(key_mag->mutex);
    key_ent_lst->nxt = key_mag->lst;
    key_mag->lst = key_ent_lst;
    tuya_hal_mutex_unlock(key_mag->mutex);

    return OPRT_OK;
}

STATIC VOID __key_handle(VOID)
{
    INT_T i = 0;

    for(i = 0;i < key_mag->tbl_cnt;i++) {
        __key_ent_proc(&key_mag->p_tbl[i]);
    }
    //PR_NOTICE("__key_handle");
    tuya_hal_mutex_lock(key_mag->mutex);
    KEY_EN_LST_S *key_ent_lst = key_mag->lst;
    while(key_ent_lst) {
        __key_ent_proc(&key_ent_lst->key_ent);
        key_ent_lst = key_ent_lst->nxt;
    }
    tuya_hal_mutex_unlock(key_mag->mutex);
}

STATIC VOID __key_timer_cb(UINT_T timerID,PVOID_T pTimerArg)
{
    __key_handle();
}

STATIC BOOL_T __tuya_key_down_verify(IN CONST KEY_ENTITY_S *key_ent)
{
    INT_T gpio_stat = tuya_gpio_read(key_ent->kud.port);
    if(FALSE == key_ent->kud.low_level_detect) {
        return (gpio_stat) ? TRUE:FALSE;
    }else {
        return (gpio_stat) ? FALSE:TRUE;
    }
}

STATIC VOID __key_ent_proc(INOUT KEY_ENTITY_S *key_ent)
{
    switch(key_ent->status) {
        case KEY_DOWN: {
            BOOL_T key_val;
            key_val = __tuya_key_down_verify(key_ent);
            if(key_val && (!key_ent->key_val_last)) {
                key_ent->status = KEY_DOWN_CONFIRM;
                key_ent->key_val_last = TRUE;
            }else {
                key_ent->key_val_last = key_val;
            }
            
            key_ent->down_time = 0;
            key_ent->up_time = 0;
            key_ent->seq_key_cnt = 0;
        }
        break;
        
        case KEY_DOWN_CONFIRM: {
            if(TRUE == __tuya_key_down_verify(key_ent)) {
                key_ent->status = KEY_DOWNNING;
                if((FALLING_EDGE_TRIG == key_ent->kud.lp_tp) || (FALLING_LONG_TRIG == key_ent->kud.lp_tp)) {
                    key_ent->kud.call_back(key_ent->kud.port,NORMAL_KEY,0);
                }
            }else {
                key_ent->status = KEY_DOWN;
                key_ent->down_time = 0;
            }
        }
        break;
        
        case KEY_DOWNNING: {
            #define KEY_DOWN_CONT_TRIG_TIME_MS 300
            if(TRUE == __tuya_key_down_verify(key_ent)) {
                key_ent->down_time += (key_mag->timer_space);
                
                if(((LP_ONCE_TRIG == key_ent->kud.lp_tp) || (FALLING_LONG_TRIG == key_ent->kud.lp_tp)) && \
                   key_ent->down_time >= key_ent->kud.long_key_time && \
                   FALSE == key_ent->long_key_press) {
                   key_ent->kud.call_back(key_ent->kud.port,LONG_KEY,0);
                   key_ent->long_key_press = TRUE;

                } else if(LP_MORE_NORMAL_TRIG == key_ent->kud.lp_tp && \
                         key_ent->down_time >= KEY_DOWN_CONT_TRIG_TIME_MS) {
                    key_ent->kud.call_back(key_ent->kud.port,NORMAL_KEY,0);
                    key_ent->down_time = 0;
                }
            }else {
                key_ent->status = KEY_UP_CONFIRM;
                key_ent->up_time = 0;
            }
        }
        break;
        
        case KEY_UP_CONFIRM: {
            if(FALSE == __tuya_key_down_verify(key_ent)) {
                key_ent->status = KEY_UPING;
            }else {
                key_ent->down_time += key_mag->timer_space;
                key_ent->status = KEY_DOWNNING;
            }
        }
        break;

        case KEY_UPING: {
            if(FALSE == __tuya_key_down_verify(key_ent)) {
                if(0 == key_ent->up_time) {
                    key_ent->up_time = (key_mag->timer_space)*2;
                }else {
                    key_ent->up_time += key_mag->timer_space;
                }
                
                if(key_ent->up_time >= key_ent->kud.seq_key_detect_time) {
                    key_ent->status = KEY_FINISH;
                }
            }else { // is seq key?
                if(key_ent->up_time >= key_ent->kud.seq_key_detect_time) {
                    key_ent->status = KEY_FINISH;
                }else {
                    key_ent->status = KEY_DOWN_CONFIRM;
                    key_ent->up_time = 0;
                    key_ent->seq_key_cnt++;
                }
            }

            if((KEY_FINISH == key_ent->status)) {
                if(key_ent->seq_key_cnt) {
                    key_ent->seq_key_cnt++;
                }

                if(key_ent->seq_key_cnt > 1) {
                    key_ent->kud.call_back(key_ent->kud.port,SEQ_KEY,key_ent->seq_key_cnt);
                } else {
                    if((FALLING_EDGE_TRIG != key_ent->kud.lp_tp) && (FALLING_LONG_TRIG != key_ent->kud.lp_tp)) {
						if(!((LP_ONCE_TRIG == key_ent->kud.lp_tp)&&(key_ent->long_key_press))) {
							key_ent->kud.call_back(key_ent->kud.port,NORMAL_KEY,0);
						}
                    }
                }
                
                if(LP_ONCE_TRIG == key_ent->kud.lp_tp){
                    key_ent->long_key_press = FALSE;
                }
            }
        }
        break;

        case KEY_FINISH: {
            key_ent->status = KEY_DOWN;
        }
        break;

        default:
            break;
    }
}
