/**
 * @file tuya_sdk.h
 * @author maht@tuya.com
 * @brief SDK通用流程管理，SDK对象管理模块
 * @version 0.1
 * @date 2019-08-28
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#ifndef __TUYA_DEV_H__
#define __TUYA_DEV_H__


#include "tuya_cloud_com_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************
*  Function: get_cloud_dev_tpye
*  Input: local_tp
*  Output: none
*  Return: CLOUD_DEV_TP_DEF_T
***********************************************************/
CLOUD_DEV_TP_DEF_T get_cloud_dev_tpye(IN CONST DEV_TYPE_T local_tp);

/***********************************************************
*  Function: get_local_dev_tpye
*  Input: cloud_tp
*  Output: none
*  Return: DEV_TYPE_T
***********************************************************/
DEV_TYPE_T get_local_dev_tpye(IN CONST CLOUD_DEV_TP_DEF_T cloud_tp);


#ifdef __cplusplus
}
#endif


#endif
