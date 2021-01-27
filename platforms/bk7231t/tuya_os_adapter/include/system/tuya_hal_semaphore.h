/**
 * @file tuya_hal_semaphore.h
 * @author maht@tuya.com
 * @brief semaphore相关接口封装
 * @version 0.1
 * @date 2019-08-15
 * 
 * @copyright Copyright (c) tuya.inc 2019
 * 
 */

#ifndef _TUYA_HAL_SEMAPHORE_H
#define _TUYA_HAL_SEMAPHORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/* tuya sdk definition of semaphore */
typedef void* SEM_HANDLE;

/**
 * @brief tuya_hal_semaphore_create_init用于创建并初始化semaphore
 * 
 * @param[out] *pHandle semaphore句柄
 * @param[in] semCnt 
 * @param[in] sem_max 
 * @return int 0=成功，非0=失败
 */
int tuya_hal_semaphore_create_init(SEM_HANDLE *pHandle, const uint32_t semCnt, const uint32_t sem_max);

/**
 * @brief tuya_hal_semaphore_wait用于wait semaphore
 * 
 * @param semHandle semaphore句柄
 * @return int 0=成功，非0=失败
 */
int tuya_hal_semaphore_wait(const SEM_HANDLE semHandle);

/**
 * @brief tuya_hal_semaphore_post用于post semaphore
 * 
 * @param semHandle semaphore句柄
 * @return int 0=成功，非0=失败
 */
int tuya_hal_semaphore_post(const SEM_HANDLE semHandle);

/**
 * @brief tuya_hal_semaphore_release用于release semaphore
 * 
 * @param semHandle 
 * @return int 0=成功，非0=失败
 */
int tuya_hal_semaphore_release(const SEM_HANDLE semHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

