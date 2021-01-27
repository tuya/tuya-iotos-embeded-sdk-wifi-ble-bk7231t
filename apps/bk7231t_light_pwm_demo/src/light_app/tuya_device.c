/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @Date: 2019-05-22 10:17:39
 * @LastEditors: wls
 * @LastEditTime: 2020-03-06 12:34:08
 * @file name: light_system.c
 * @Description: system adapter process
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 */

#define _TUYA_DEVICE_GLOBAL
#include "tuya_device.h"
#include "light_system.h"

/**
 * @brief: wifi fast initlize process
 * Input: none
 * Output: none
 * Return: none
 * Note: to initialize device before device_init
 */
VOID pre_device_init(VOID)
{
    vLightSysPreDeviceinit();

}

/**
 * @brief: wifi normal initlize process
 * @param {none} 
 * @retval: none
 * @Note:c alled by user_main
 */
VOID app_init(VOID)
{
    vLightSysAppInit();
}

/**
 * @brief: wifi gpio test
 * @param {none} 
 * @retval: gpio_test_cb
 */
BOOL_T gpio_test(IN CONST CHAR_T *in, OUT CHAR_T *out)
{
    return gpio_test_all(in, out);
}

/**
 * @brief: erase user data when authorization
 * @param {none} 
 * @attention: none
 * @retval: none
 */
VOID mf_user_callback(VOID)
{
    vLightSysEraseFlashData();
}

/**
 * @brief: device_init
 * @param {none} 
 * @attention: none
 * @retval: none
 * @Note: called by tuya_main.c
 */
OPERATE_RET device_init(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;

    op_ret = opLightSysDeviceInit();
    if(op_ret != OPRT_OK) {
        return op_ret;
    }
    return OPRT_OK;
}


