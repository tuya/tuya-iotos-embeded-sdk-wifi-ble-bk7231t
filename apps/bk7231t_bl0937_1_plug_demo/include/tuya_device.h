/**
 * @File: device.h 
 * @Author: caojq 
 * @Last Modified time: 2020-07-029 
 * @Description: 
 */
#ifndef _TUYA_DEVICE_H
#define _TUYA_DEVICE_H
    
#include "tuya_cloud_error_code.h"
    
#ifdef __cplusplus
    extern "C" {
#endif
    
#ifdef _TUYA_DEVICE_GLOBAL
    #define _TUYA_DEVICE_EXT 
#else
    #define _TUYA_DEVICE_EXT extern
#endif
    
/***********************************************************
*************************micro define***********************
***********************************************************/
// device information define
#define DEV_SW_VERSION USER_SW_VER
#define PRODECT_KEY "aumib22ytuftcydk"
/***********************************************************
*************************function define********************
***********************************************************/

/***********************************************************
*  Function: device_init 
*  Input: none
*  Output: none
*  Return: none
***********************************************************/
_TUYA_DEVICE_EXT \
OPERATE_RET device_init(VOID);

#ifdef __cplusplus
}
#endif
#endif

