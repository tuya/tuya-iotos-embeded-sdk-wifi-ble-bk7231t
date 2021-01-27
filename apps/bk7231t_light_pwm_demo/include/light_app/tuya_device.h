/***********************************************************
*  File: tuya_device.h
*  Author: nzy
*  Date: 20170909
***********************************************************/
#ifndef _TUYA_DEVICE_H
#define _TUYA_DEVICE_H

#include "tuya_cloud_types.h"

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

/// oem firmware
#define PRODUCT_KEY     "s6vgaf7xvyfes7kb"


/***********************************************************
*************************function define********************
***********************************************************/
/***********************************************************
*  Function: pre_device_init
*  Input: none
*  Output: none
*  Return: none
*  Note: to initialize device before device_init
***********************************************************/
_TUYA_DEVICE_EXT \
VOID pre_device_init(VOID);

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
#endif /* __cplusplus */

#endif
