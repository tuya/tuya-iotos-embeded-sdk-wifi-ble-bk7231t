/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors:   
 * @file name: light_driver_adapter.c
 * @Description: light driver adapter proc
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-26 13:55:40
 * @LastEditTime: 2019-10-18 15:30:15
 */

#include "light_set_color.h"
#include "light_printf.h"
#include "light_types.h"

#define LIGHT_SEND_VALUE_MAX                    1000


STATIC DRIVER_MODE_E geDriverMode;
STATIC UCHAR_T gucDriveInitFlag = FALSE;

/**
 * @brief: Light drive init!
 * @param {DRIVER_CONFIG_T *pLightConfig -> init config structure}
 * @attention: DRIVER_MODE_E eMode -> has three mode：
                DRIVER_MODE_PWM = 0

 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightDriveInit(IN DRIVER_CONFIG_T *pLightConfig)
{
    OPERATE_LIGHT opRet = -1;

    if(gucDriveInitFlag != FALSE) {
        PR_NOTICE("Drive init already init ok");
        return LIGHT_OK;
    }
    geDriverMode = pLightConfig ->eMode;
    switch (pLightConfig ->eMode) {
        case DRIVER_MODE_PWM: {
            USER_PWM_INIT_T *pPwmConfig = (USER_PWM_INIT_T *)(&pLightConfig ->uConfig);
            opRet = opUserPWMInit(pPwmConfig);
            if(opRet != LIGHT_OK) {
                PR_ERR("Light drive pwm init error!");
                return LIGHT_INVALID_PARM;
            }
            break;
        }

        default :{
            PR_ERR("vLighDriveInit mode error!");
            break;
        }
    }

    gucDriveInitFlag = TRUE;
    
    return LIGHT_OK;
}

/**
 * @brief: light send control data
 * @param {IN USHORT_T usRed} red color, range 0~1000
 * @param {IN USHORT_T usGreen} Green color, range 0~1000
 * @param {IN USHORT_T usBlue} Blue color, range 0~1000
 * @param {IN USHORT_T usCold} cold white color, range 0~1000
 * @param {IN USHORT_T usWarm} warm white color, range 0~1000
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightSetRGBCW(IN USHORT_T usRed, IN USHORT_T usGreen, IN USHORT_T usBlue, IN USHORT_T usCold, IN USHORT_T usWarm)
{
    OPERATE_LIGHT opRet = -1;
    
    switch(geDriverMode)
    {
        case DRIVER_MODE_PWM:{
                USER_PWM_COLOR_T tPwmColor;
                memset(&tPwmColor, 0, SIZEOF(USER_PWM_COLOR_T));   
                 
                tPwmColor.usRed = usRed;        /* pwm color range 0 ~ 1000 */
                tPwmColor.usGreen = usGreen;
                tPwmColor.usBlue = usBlue;
                tPwmColor.usCold = usCold;
                tPwmColor.usWarm = usWarm;
                opRet = opUserPWMSetRGBCW(&tPwmColor);  /* pwm set color */
                if(opRet != LIGHT_OK) {
                    PR_ERR("PWM send data error!");
                }
            }
            break;

        default:
                PR_ERR("DRIVER mode error!");
            break;
    }   

    if(LIGHT_OK != opRet) {
        PR_ERR("Light set RGBCW ERROR!");
    }
    return opRet;
}


