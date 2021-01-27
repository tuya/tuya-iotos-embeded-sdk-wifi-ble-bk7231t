/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: user_pwm.c
 * @Description: PWM driver program
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-16 11:44:21
 * @LastEditTime: 2019-08-13 11:23:01
 */

#include "user_pwm.h"
#include "light_printf.h"
#include "light_tools.h"
#include "soc_pwm.h"
#include "light_set_color.h"


#define PWM_MAX_FREQ    20000                   ///< PWM MAX Frequency 20K
#define PWM_MIN_FREQ    100                     ///< PWM MIN Frequency 100
#define PWM_MAX_DUTY    1000                    ///< PWM MAX Duty 1000 --> Precision 0.1%

STATIC USER_PWM_INIT_T gPwmInitConfig ;          ///< PWM Settings
STATIC UCHAR_T ucUserPwmInitFlag = FALSE;

/**
 * @brief: user pwm init
 * @param {IN USER_PWM_INIT_T *pPwmInit -> Pwm INIT Parm
 *          usFreq -> PWM Frequency unit Hz, range:100~20000Hz
 *          usDuty -> PWM Init duty unit 0.1%, range: 0~1000 
 *          ucList[5] -> RGBCW GPIO list
 *          ucChannelNum -> channel num, rang 1~5 way
 *          bPolarity -> PWM output polarity
 *              TRUE -> positive
 *              FAlse -> negative          
 *          ucCtrlPin -> ctrl pin
 *          bCtrlLevel -> ctrl pin level}
 * @attention ucList[5] parm set
 *              List order is always RGBCW !
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserPWMInit(IN USER_PWM_INIT_T *pPwmInit)
{
    OPERATE_LIGHT opRet = -1;
    UCHAR_T ucListTemp[5] = {0};
    
    if( NULL == pPwmInit ) {
        PR_ERR("USER PWM init is invalid!");
        return LIGHT_INVALID_PARM;
    }
  
    if( ( pPwmInit ->usFreq < PWM_MIN_FREQ ) || ( pPwmInit ->usFreq > PWM_MAX_FREQ ) ) {
        PR_ERR("USER PWM init is invalid!");
        return LIGHT_INVALID_PARM;
    }

    if( pPwmInit ->usDuty > PWM_MAX_DUTY ) {
        PR_ERR("USER PWM init is invalid!");
        return LIGHT_INVALID_PARM;
    }

    if( (pPwmInit ->ucChannelNum <= 0) || (pPwmInit ->ucChannelNum > 5) ){
        PR_ERR("USER PWM init is invalid!");
        return LIGHT_INVALID_PARM;
    }
    
    memcpy(&gPwmInitConfig, pPwmInit, SIZEOF(USER_PWM_INIT_T));

    switch(gPwmInitConfig.ucChannelNum)
    {
        case 1: /* 1 way -- C */
            ucListTemp[0] = gPwmInitConfig.ucList[3];
            memcpy(gPwmInitConfig.ucList, ucListTemp, SIZEOF(gPwmInitConfig.ucList));
            break;

        case 2: /* 2 way -- CW */
            ucListTemp[0] = gPwmInitConfig.ucList[3];
            ucListTemp[1] = gPwmInitConfig.ucList[4];
            memcpy(gPwmInitConfig.ucList, ucListTemp, SIZEOF(gPwmInitConfig.ucList));
            break;

        case 3: /* 3 way -- RGB */
        case 4: /* 4 way -- RGBC */
        case 5: /* 5 way -- RGBCW */
            /* don't need to process **/
            break;
            
        default:
            break;
    }
    
    opRet = opSocPwmInit(gPwmInitConfig.usFreq, gPwmInitConfig.usDuty, gPwmInitConfig.ucChannelNum, gPwmInitConfig.ucList, gPwmInitConfig.bCCTFlag);
    if(opRet != LIGHT_OK) {
        PR_ERR("PWM soc init error!");
        return LIGHT_INVALID_PARM;
    }

    ucUserPwmInitFlag = TRUE;
    
    if(PIN_NOEXIST == gPwmInitConfig.ucCtrlPin) {
        PR_DEBUG("no ctrl pin!");
        return LIGHT_OK;
    }
    
    if(gPwmInitConfig.bCtrlLevel) {       /* shut down ctrl pin! */
        vSocPinReset(gPwmInitConfig.ucCtrlPin);
    } else {
        vSocPinSet(gPwmInitConfig.ucCtrlPin);
    }
    
    return LIGHT_OK;
}

/**
 * @brief: pwm send data out
 * @param {IN user_pwm_color_t *color_data -> PWM send data 
 *          usRed   -> R send duty,rang from 0 to 1000,unit 0.1%
 *          usGreen -> G send duty,rang from 0 to 1000,unit 0.1%
 *          usBlue  -> B send duty,rang from 0 to 1000,unit 0.1%
 *          usCold  -> C send duty,rang from 0 to 1000,unit 0.1%
 *          usWarm  -> W send duty,rang from 0 to 1000,unit 0.1%}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserPWMSetRGBCW(IN USER_PWM_COLOR_T *pColorData)
{
    if(ucUserPwmInitFlag != TRUE){
        PR_ERR("user pwm not init!");
        return LIGHT_INVALID_PARM;
    }
    
    if( (pColorData ->usRed > PWM_MAX_DUTY) || (pColorData ->usGreen > PWM_MAX_DUTY) || \
        (pColorData ->usBlue > PWM_MAX_DUTY) || (pColorData -> usCold > PWM_MAX_DUTY) || \
        (pColorData ->usWarm > PWM_MAX_DUTY) ) {
        PR_ERR("USER PWM data is invalid!");
        return LIGHT_INVALID_PARM;
    }

    if(gPwmInitConfig.ucCtrlPin != PIN_NOEXIST) {       /* ctrl pin exist */
        if(( 0 != pColorData ->usRed ) || ( 0 != pColorData ->usGreen ) || ( 0 != pColorData ->usBlue) ||
            ( 0 != pColorData ->usCold) || ( 0 != pColorData ->usWarm )) {
            
            if(gPwmInitConfig.bCtrlLevel) {
                vSocPinSet(gPwmInitConfig.ucCtrlPin);           /* turn on ctrl pin */
            } else {
                vSocPinReset(gPwmInitConfig.ucCtrlPin);
            }
        } else {
            if(gPwmInitConfig.bCtrlLevel) {
                vSocPinReset(gPwmInitConfig.ucCtrlPin);         /* shut down ctrl pin */
            } else {
                vSocPinSet(gPwmInitConfig.ucCtrlPin);
            }
        }
    }

    if( gPwmInitConfig.bPolarity == FALSE) {
        pColorData ->usRed = PWM_MAX_DUTY - pColorData ->usRed;
        pColorData ->usGreen = PWM_MAX_DUTY - pColorData ->usGreen;
        pColorData ->usBlue = PWM_MAX_DUTY - pColorData ->usBlue;
        pColorData ->usCold = PWM_MAX_DUTY - pColorData ->usCold;
        pColorData ->usWarm = PWM_MAX_DUTY - pColorData ->usWarm;
    }
    
    switch(gPwmInitConfig.ucChannelNum)
    {
        case 1: /* 1 way -- C */           
            vSocPwmSetDuty(0, pColorData ->usCold);     /* send C value */
            break;
        case 2: /* 2 way -- CW */
            vSocPwmSetDuty(0, pColorData ->usCold);     /* send C value */
            vSocPwmSetDuty(1, pColorData ->usWarm);     /* send W value */
            break;
        case 3: /* 3 way -- RGB */
            vSocPwmSetDuty(0, pColorData ->usRed);      /* send R value */
            vSocPwmSetDuty(1, pColorData ->usGreen);    /* send G value */
            vSocPwmSetDuty(2, pColorData ->usBlue);     /* send B value */
            break;
        case 4: /* 4 way -- RGBC */
            vSocPwmSetDuty(0, pColorData ->usRed);      /* send R value */
            vSocPwmSetDuty(1, pColorData ->usGreen);    /* send G value */
            vSocPwmSetDuty(2, pColorData ->usBlue);     /* send B value */
            vSocPwmSetDuty(3, pColorData ->usCold);     /* send C value */
            break;
        case 5: /* 5 way -- RGBCW */
            vSocPwmSetDuty(0, pColorData ->usRed);      /* send R value */
            vSocPwmSetDuty(1, pColorData ->usGreen);    /* send G value */
            vSocPwmSetDuty(2, pColorData ->usBlue);     /* send B value */
            vSocPwmSetDuty(3, pColorData ->usCold);     /* send C value */
            vSocPwmSetDuty(4, pColorData ->usWarm);     /* send w value */
            break;
        default:
            PR_ERR("pwm drive channel set error!!");
            break;
    }
    return LIGHT_OK;
}

