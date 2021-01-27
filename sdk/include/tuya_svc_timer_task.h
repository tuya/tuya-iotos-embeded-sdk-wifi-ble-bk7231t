/**
 * @file tuya_svc_timer_task.h
 * @author anby (you@domain.com)
 * @brief tuya定时服务外部接口
 * @version 0.1
 * @date 2019-08-29
 * 
 * @copyright Copyright (c) tuya.inc 2019
 * 
 */

#ifndef __TUYA_SVC_TIME_TASK_H__
#define __TUYA_SVC_TIME_TASK_H__


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief tuya 本地定时服务初始化
 * 
 * @param increase_unit: 定时任务每次申请increase_unit
 * @return OPERATE_RET： 0成功，非0，请参照tuya error code描述文档 
 */
int tuya_svc_timer_task_init(uint32_t increase_unit);

/** 
 * @brief tuya 本地定时服务重置，清空
 * 
 * @return OPERATE_RET： 0成功，非0，请参照tuya error code描述文档 
 */
int tuya_svc_timer_task_reset(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /*__TUYA_SVC_TIME_TASK_H__ */

