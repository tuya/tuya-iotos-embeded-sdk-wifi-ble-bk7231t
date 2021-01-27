/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: light_cfg.h
 * @Description: light control support function include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-26 13:55:40
 * @LastEditTime: 2020-03-06 12:11:35
 */

#ifndef __LIHGT_CFG_H__
#define __LIHGT_CFG_H__


#include "light_types.h"


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


/**
 * @brief Light frame support function choose
 */
#define LIGHT_CFG_INIT_PARAM_CHECK          0   //if not check param vaild when light init 
#define LIGHT_CFG_SUPPORT_LOWPOWER          0   //if not support lowpower 
#define LIGHT_CFG_PROD_DRIVER_NEED_INIT     0   //if not drive need init in production
#define LIGHT_CFG_ENABLE_GAMMA              1   // enable gamma 
#define LIGHT_CFG_GAMMA_CAL                 0   // enable gamma calc


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __LIHGT_CFG_H__ */
