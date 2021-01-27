/**
 * @file tuya_os_adapter.h
 * @author maht@tuya.com
 * @brief 
 * @version 0.1
 * @date 2019-08-15
 * 
 * @copyright Copyright (c) tuya.inc 2019
 * 
 */
#ifndef _TUYA_OS_ADAPTER_H
#define _TUYA_OS_ADAPTER_H

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "tuya_hal_network.h"
#include "tuya_hal_thread.h"
#include "tuya_hal_mutex.h"
#include "tuya_hal_semaphore.h"
#include "tuya_hal_system.h"


#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/

/***********************************************************
*************************variable define********************
***********************************************************/

/***********************************************************
*************************function define********************
***********************************************************/

/**
 * @brief tuya_os_adapter_init用于初始化tuya os adapter组件
 * 
 * @return int 0，成功；非0，失败
 */
int tuya_os_adapter_init(void);

#ifdef __cplusplus
}
#endif


#endif /* _TUYA_OS_ADAPTER_H */
