/**
 * @file    tuya_hal_system_internal.h
 * @brief   system 模块内部接口
 * 
 * @copyright Copyright (c) tuya.inc 2019
 * 
 */

#ifndef __TUYA_HAL_SYSTEM_INTERNAL_H__
#define __TUYA_HAL_SYSTEM_INTERNAL_H__


#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief tuya_hal_system_isrstatus用于检查系统是否处于中断上下文
 * 
 * @return true 
 * @return false 
 */
bool tuya_hal_system_isrstatus(void);

/**
 * @brief 内存分配操作
 * 
 */
void* tuya_hal_internal_malloc(const size_t size);

/**
 * @brief 内存释放操作
 * 
 */
void tuya_hal_internal_free(void* ptr);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif


