/**
 * @file tuya_hal_mutex.h
 * @author maht@tuya.com
 * @brief mutex相关接口封装
 * @version 0.1
 * @date 2019-08-15
 * 
 * @copyright Copyright (c) tuya.inc 2019
 * 
 */


#ifndef _TUYA_HAL_MUTEX_H
#define _TUYA_HAL_MUTEX_H

#ifdef __cplusplus
extern "C" {
#endif

/* tuya sdk definition of mutex context */
typedef void* MUTEX_HANDLE;

/**
 * @brief tuya_hal_mutex_create_init用于创建并初始化tuya mutex
 * 
 * @param[out] pMutexHandle 返回mutex句柄
 * @return int 0=成功，非0=失败
 */
int tuya_hal_mutex_create_init(MUTEX_HANDLE *pMutexHandle);

/**
 * @brief tuya_hal_mutex_lock用于lock tuya mutex
 * 
 * @param[in] mutexHandle tuya mutex句柄
 * @return int 0=成功，非0=失败
 */
int tuya_hal_mutex_lock(const MUTEX_HANDLE mutexHandle);

/**
 * @brief tuya_hal_mutex_unlock用于unlock tuya mutex
 * 
 * @param[in] mutexHandle tuya mutex句柄
 * @return int 0=成功，非0=失败
 */
int tuya_hal_mutex_unlock(const MUTEX_HANDLE mutexHandle);

/**
 * @brief tuya_hal_mutex_release用于释放tuya mutex
 * 
 * @param[in] mutexHandle MUTEX_HANDLE tuya mutex句柄
 * @return int 0=成功，非0=失败
 */
int tuya_hal_mutex_release(const MUTEX_HANDLE mutexHandle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
