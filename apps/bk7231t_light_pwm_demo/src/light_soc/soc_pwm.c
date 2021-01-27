/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: soc_pwm.c
 * @Description: soc PWM proc
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-24 20:42:29
 * @LastEditTime: 2019-08-28 21:50:15
 */

#include "BkDriverPwm.h"
#include "soc_pwm.h"
#include "tuya_gpio.h"
#include "light_printf.h"
#include "tuya_pwm.h"
#include "uart_pub.h"

STATIC UCHAR_T *pChannelList = NULL;
STATIC UINT_T uiPeriod = 0;
STATIC BOOL_T bCCTDriveFlag = 0;
STATIC UCHAR_T *pPwmEnablelist = NULL;
STATIC UCHAR_T ucChannelNumer = 0;



/**
 * @brief: PWM SOC Init
 * @param {IN USHORT_T usFrequency --> PWM frequency,unit:HZ, attention: range：500 ~ 20KHz }
 * @param {IN USHORT_T usPositiveDutyCycle --> PWM Init Duty(positive duty),range:0 ~ 1000}
 * @param {IN UCHAR_T ucChannelNum --> PWM channel num}
 * @param {IN UCHART_T *pGpioList --> PWM gpio set}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opSocPwmInit(IN USHORT_T usFrequency, IN USHORT_T usPositiveDutyCycle, IN UCHAR_T ucChannelNum, IN UCHAR_T *pGpioList, IN BOOL_T bCCTFlag)
{
    UCHAR_T i = 0, j = 0;
    USHORT_T uiDuty = 0;

    CONST UCHAR_T ucGpioAllowList[6][2] = {
                                                {6,  0},
                                                {7,  1},
                                                {8,  2},
                                                {9,  3},
                                                {24, 4},
                                                {26, 5}
                                            };

    if(pChannelList != NULL) {
        PR_NOTICE("SOC pwm already init.");
        return OPRT_OK;
    }

    pChannelList = (UCHAR_T *)Malloc( ucChannelNum * SIZEOF(UCHAR_T) );
    pPwmEnablelist = (UCHAR_T *)Malloc( ucChannelNum * SIZEOF(UCHAR_T) );
    
    if( (pChannelList == NULL) || (pPwmEnablelist == NULL) ) {
        PR_ERR("SOC PWM Init Error!");
        goto INIT_ERROR;
    }

    if(ucChannelNum > 5) {
        PR_ERR("bk7231 enable max pwm num is 5! set channel num error!");
        goto INIT_ERROR;
    }

    if((usFrequency < 100) && (usFrequency > 20000)) {      /* frequency check range */
        PR_ERR("bk7231 pwm frequency range: 100 ~ 20KHz, set frequency error!");
        goto INIT_ERROR;
    }
    
    /* check gpio allowed to initilize pwm */
    for(i = 0; i < ucChannelNum; i++) {
        for(j = 0; j < 6; j++) {
            if(pGpioList[i] == ucGpioAllowList[j][0]) {
                pChannelList[i] = ucGpioAllowList[j][1];
                break;
            }
        }
        if( j >= 6 ) {
            break;
        }
    }
    if( i < ucChannelNum ) {
        PR_ERR("PWM GPIO List is illegal!");
        goto INIT_ERROR;
    }
    ucChannelNumer = ucChannelNum;

    uiPeriod = (UINT_T) (26000000 / usFrequency);
    uiDuty = (UINT_T) (usPositiveDutyCycle / 1000.0 * uiPeriod);
    bCCTDriveFlag = bCCTFlag;
    
    if(bCCTDriveFlag) { /* CCT drive mode */
        for(i = 0; i < ucChannelNum; i++ ) {
            bk_pwm_initialize(pChannelList[i], uiPeriod, uiDuty);
            bk_pwm_start(pChannelList[i]);
            *(pPwmEnablelist + i) = TRUE;
        }
        
    }else {
        if(ucChannelNum == 2) {
            tuya_pwm_init(pChannelList[0], uiPeriod, uiDuty);
            tuya_pwm_init(pChannelList[1], uiPeriod, uiDuty);
            *(pPwmEnablelist + 0) = TRUE;
            *(pPwmEnablelist + 1) = TRUE;
        } else if(ucChannelNum == 5) { 
            for(i = 0; i < 3; i++) {
                bk_pwm_initialize(pChannelList[i], uiPeriod, uiDuty);
                *(pPwmEnablelist + i) = TRUE;
                bk_pwm_start(pChannelList[i]);
            }
            tuya_pwm_init(pChannelList[3], uiPeriod, uiDuty);
            tuya_pwm_init(pChannelList[4], uiPeriod, uiDuty);
            *(pPwmEnablelist + 3) = TRUE;
            *(pPwmEnablelist + 4) = TRUE;
        } else {
            for(i = 0; i < ucChannelNum; i++ ) {
                bk_pwm_initialize(pChannelList[i], uiPeriod, uiDuty);
                *(pPwmEnablelist + i) = TRUE;
            }
        }
    }
    
    return OPRT_OK;

INIT_ERROR:

    if(pChannelList != NULL) {
        Free(pChannelList);
        pChannelList = NULL;
    }
    
    if(pPwmEnablelist != NULL) {
        Free(pPwmEnablelist);
        pPwmEnablelist = NULL;
    }

    return OPRT_INVALID_PARM;
}

/**
 * @brief: PWM SOC SET Duty
 * @param {IN UCHAR_T ucChannel -> pwm send out channel num}
 * @param {IN USHORT_T usDuty   -> pwm send duty,rang: 0~1000}
 * @attention usChannel --> according to the initlize order
 * @retval: none
 */
VOID vSocPwmSetDuty(IN UCHAR_T ucChannel, IN USHORT_T usDuty) 
{
    FLOAT_T fPercent = 0.0;
    STATIC UINT_T uiCold = 0;
    
    if( (pChannelList == NULL) || (pPwmEnablelist == NULL) ){
        PR_ERR("PWM not init or init error,please init firstly!");
        return;
    }

    if(ucChannel >= ucChannelNumer) {
        PR_ERR("cann't set pwm, because this channel not init!");
        return;
    }

    fPercent = (FLOAT_T) (usDuty / 1000.0);

    if(bCCTDriveFlag) {    /* CCT drive mode */
        if(usDuty <= 0) {    /* duty <= 0 stop pwm, to reduce power */
            if( TRUE == *(pPwmEnablelist + ucChannel) ) {
                bk_pwm_stop(pChannelList[ucChannel]);
                *(pPwmEnablelist + ucChannel) = FALSE;
            } else {
                ;
            }

        } else {       
            fPercent = (FLOAT_T) (usDuty / 1000.0);
            if( *(pPwmEnablelist + ucChannel) != TRUE ) {    /* channel pwm already stop, init pwm func again! */
                bk_pwm_update_param(pChannelList[ucChannel], uiPeriod, fPercent * uiPeriod);
                *(pPwmEnablelist + ucChannel) = TRUE;
            }
            bk_pwm_update_param(pChannelList[ucChannel], uiPeriod, fPercent * uiPeriod);
            bk_pwm_start(pChannelList[ucChannel]);
        }
    }else {
        if(ucChannelNumer == 2) {       /* CW light */
            if(ucChannel == 0) {
                uiCold = (UINT_T)(fPercent * uiPeriod);
            } else {
                if((uiCold == 0) && ((UINT_T)(fPercent * uiPeriod) == 0)) {
                    tuya_pwm_stop(pChannelList[0], pChannelList[1]);
                } else {
                    //PR_NOTICE("cold %d, warm %d", uiCold,(UINT_T)(fPercent * uiPeriod));
                    tuya_pwm_reset_duty_cycle(pChannelList[0], pChannelList[1], uiCold, (UINT_T)(fPercent * uiPeriod), uiPeriod, 0);
                }
            }
        } else if(ucChannelNumer == 5) {    /* RGBCW light */
            if(ucChannel == 3) {      /* channel is C */
                uiCold = (UINT_T)(fPercent * uiPeriod);
            } else if(ucChannel == 4) {   /* channel is W */
                if((uiCold == 0) && ((UINT_T)(fPercent * uiPeriod) == 0)) {
                    tuya_pwm_stop(pChannelList[3],pChannelList[4]);
                } else {
                    tuya_pwm_reset_duty_cycle(pChannelList[3], pChannelList[4], uiCold, (UINT_T)(fPercent * uiPeriod), uiPeriod, 0);
                }
            } else {
                if(usDuty <= 0) {    /* duty <= 0 stop pwm, to reduce power */
                    if( TRUE == *(pPwmEnablelist + ucChannel) ) {
                        bk_pwm_stop(pChannelList[ucChannel]);
                        *(pPwmEnablelist + ucChannel) = FALSE;
                    } else {
                        ;
                    }

                } else {       
                    fPercent = (FLOAT_T) (usDuty / 1000.0);
                    if( *(pPwmEnablelist + ucChannel) != TRUE ) {    /* channel pwm already stop, init pwm func again! */
                        bk_pwm_update_param(pChannelList[ucChannel], uiPeriod, fPercent * uiPeriod);
                        *(pPwmEnablelist + ucChannel) = TRUE;
                    }
                    bk_pwm_update_param(pChannelList[ucChannel], uiPeriod, fPercent * uiPeriod);
                    bk_pwm_start(pChannelList[ucChannel]);
                }
            }
        } else {
            if(usDuty <= 0) {    /* duty <= 0 stop pwm, to reduce power */
                if( TRUE == *(pPwmEnablelist + ucChannel) ) {
                    bk_pwm_stop(pChannelList[ucChannel]);
                    *(pPwmEnablelist + ucChannel) = FALSE;
                } else {
                    ;
                }

            } else {       
                fPercent = (FLOAT_T) (usDuty / 1000.0);
                if( *(pPwmEnablelist + ucChannel) != TRUE ) {    /* channel pwm already stop, init pwm func again! */
                    bk_pwm_update_param(pChannelList[ucChannel], uiPeriod, fPercent * uiPeriod);
                    *(pPwmEnablelist + ucChannel) = TRUE;
                }
                bk_pwm_update_param(pChannelList[ucChannel], uiPeriod, fPercent * uiPeriod);
                bk_pwm_start(pChannelList[ucChannel]);
            }
        }
    }

}

