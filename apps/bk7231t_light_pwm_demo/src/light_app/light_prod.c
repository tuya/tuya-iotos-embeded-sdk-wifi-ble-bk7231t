/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: light_prod.c
 * @Description: light production test proc
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-26 13:55:40
 * @LastEditTime: 2020-03-06 12:06:57
 */

#include "light_control.h"
#include "light_printf.h"
#include "light_prod.h"
#include "user_flash.h"
#include "device_config_load.h"
#include "user_timer.h"

/// Control calculate range 0 ~ 1000
#define PROD_CAL_VALUE_RANGE                        1000
/// Control cw calc max bright value
#define PROD_CW_BRIGHT_VALUE_MAX                    PROD_CAL_VALUE_RANGE
/// Control rgb calc max bright value
#define PROD_RGB_BRIGHT_VALUE_MAX                   PROD_CAL_VALUE_RANGE


/// Weak signal display change base time(unit ms)
#define WEAK_SINGAL_BASE_INTERVAL                   10

/// Unauthorized display change base time(unit ms)
#define UNAUTHOR_BASE_TIME_INTERVAL                 10

/// Production overall function check base time(unit ms)
#define PROD_CHECK_BASE_INTERVAL                    10

/// Production aging restart check base time(unit ms)
#define PROD_AGING_START_BASE_INTERVAL              10

/// Production aging check base time(unit ms)
#define PROD_AGING_BASE_INTERVAL                    500

/// Production repeat test base time(unit ms)
#define PROD_REPEAT_BASE_INERVAL                    10

/// Production overall function check can repeat timer(unit ms)
#define PROD_CHECK_REPEAT_INTERVAL                  5000

/// Production aging test C or W or RGB max aging time (unit min)
#define PROD_AGING_TIME_MAX                         250

/**
 * @brief software timer use id enum
 */
typedef enum {
    WEAKSINGAL_SW_TIME = 0,
    UNAUTHORIZED_SW_TIME,
    OVERALLCHECK_SW_TIME,
    AGINGSTART_SW_TIME,
    AGINGTEST_SW_TIME,
    REPEATTEST_SW_TIME,
    RESET_CNT_SW_TIME
}PROD_SW_TIMER_ID_E;

/**
 * @brief Light LED enum
 */
typedef enum{
    LED_R = 0,
    LED_G,
    LED_B,
    LED_C,
    LED_W,
    LED_OFF,
    LED_MAX,
    LED_CW,     /* CW AGING END */
    LED_RGB     /* RGB AGING */
}LIGHT_LED_E;

/**
 * @brief Light Aging time enum
 */
typedef enum{
    AGING_C,
    AGING_W,
    AGING_RGB,
    AGING_MAX
}LIGHT_AGING_E;


/**
 * @brief: Production weak singal display status
 */
typedef enum{
    WS_BREATH_UP = 0,
    WS_BREATH_DOWN,
    WS_BLINK_ON,
    WS_BLINK_OFF
}WEAKSINGAL_STATUS_E;

STATIC PROD_INIT_T tProdConfigData;
STATIC UCHAR_T gucProdInitFlag = FALSE;


STATIC VOID vAgingStartProc(VOID);
STATIC VOID vProdAgingProc(VOID);


/**
 * @brief: production send drive light
 * @param {LIGHT_LED_E Bright_LED -> need to light led
 *         USHORT_T usBright_Value -> bright value}
 * @retval: none
 */
STATIC VOID vProdSetRGBCW(LIGHT_LED_E BrightLED, USHORT_T usBrightValue)
{

    switch (BrightLED)
    {
        case LED_R:  /* constant-expression */
            opLightSetRGBCW(usBrightValue, 0, 0, 0, 0);
            break;

        case LED_G:
            opLightSetRGBCW(0, usBrightValue, 0, 0, 0);
            break;

        case LED_B:
            opLightSetRGBCW(0, 0, usBrightValue, 0, 0);
            break;

        case LED_C:
            if(PROD_MODE_CCT == tProdConfigData.eBrightMode) {
                opLightSetRGBCW(0, 0, 0, usBrightValue, PROD_CAL_VALUE_RANGE);    /* CCT Drive mode bright C LED! */
            } else {
                opLightSetRGBCW(0, 0, 0, usBrightValue, 0);
            }
            break;

        case LED_W:
            if(PROD_MODE_CCT == tProdConfigData.eBrightMode) {
                opLightSetRGBCW(0, 0, 0, usBrightValue, 0);                     /* CCT Drive mode bright W LED! */
            } else {
                opLightSetRGBCW(0, 0, 0, 0, usBrightValue);
            }

            break;

        case LED_CW:
            if(PROD_MODE_CCT == tProdConfigData.eBrightMode) {
                opLightSetRGBCW(0, 0, 0, usBrightValue, PROD_CAL_VALUE_RANGE/2);  /* CCT Drive mode C&W LED bright mix */
            } else {
                opLightSetRGBCW(0, 0, 0, usBrightValue, usBrightValue);
            }
            break;

        case LED_RGB:
            opLightSetRGBCW(usBrightValue, usBrightValue, usBrightValue, 0, 0);
            break;

        case LED_OFF:
            opLightSetRGBCW(0, 0, 0, 0, 0);    /* shut down! */
            break;

        case LED_MAX:
        default:
            PR_ERR("Production send led error!");
            opLightSetRGBCW(0, 0, 0, 0, 0);    /* shut down! */
            break;
    }
}

/**
 * @brief: Production factory test detect weak signal display timer callback
 *          display styple
 *              C     ->  C breath up -> breath down -> blink
 *              CW    ->  C breath up -> breath down -> blink
 *              RGB   ->  R breath up -> breath down -> blink
 *              RGBC  ->  C breath up -> breath down -> blink
 *              RGBCW ->  C breath up -> breath down -> blink
 *          above display base on WEAK_SINGAL_BASE_INTERVAL frequency
 * @param {none}
 * @retval: none
 */
STATIC VOID vWeakSingalTimeCB(VOID)
{
    OPERATE_LIGHT opRet = -1;
    STATIC UCHAR_T ucInitFlag = FALSE;
    STATIC UCHAR_T ucStatus = 0;
    STATIC USHORT_T usBrightMin = 0;
    STATIC USHORT_T usBrightMax = 0;
    STATIC USHORT_T usChangeStep = 0;
    STATIC SHORT_T sSendValue = 0;
    STATIC UCHAR_T ucLedList = 0;
    STATIC USHORT_T usStandCnt = 0;
    STATIC UCHAR_T ucONOFFCnt = 0;

    if( FALSE == ucInitFlag ) {
        if(tProdConfigData.eLightWay != PROD_LIGHT_RGB) {
            ucLedList = LED_C;
            usBrightMin = PROD_CW_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitCWMin / 100.0);          /* calc max bright */
            usBrightMax = PROD_CW_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitCWMax / 100.0);          /* calc min bright */
            sSendValue = usBrightMin;
        } else {
            ucLedList = LED_R;
            usBrightMin = PROD_RGB_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitRGBMin / 100.0);          /* calc max bright */
            usBrightMax = PROD_RGB_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitRGBMax / 100.0);          /* calc min bright */
            sSendValue = usBrightMin;
        }

        usChangeStep = (usBrightMax - usBrightMin) / (WEAK_SINGAL_BREATHR_TIME_INTERVAL / WEAK_SINGAL_BASE_INTERVAL);
        ucInitFlag = TRUE;
    }

    switch(ucStatus)
    {
        case WS_BREATH_UP:
            vProdSetRGBCW(ucLedList, sSendValue);     /* breath up */
            sSendValue += usChangeStep;
            if( sSendValue >= usBrightMax ) {
                sSendValue = usBrightMax;
                ucStatus = WS_BREATH_DOWN;
            }
            break;

        case WS_BREATH_DOWN:                             /* breath down */
            vProdSetRGBCW(ucLedList, sSendValue);
            sSendValue -= usChangeStep;
            if( sSendValue <= usBrightMin ) {   /* limit min brightness */
                sSendValue = usBrightMin;
                ucStatus = WS_BLINK_ON;
            }
            break;

        case WS_BLINK_ON:                                /* blink on off */
            usStandCnt++;
            vProdSetRGBCW(ucLedList, usBrightMax);
            if( (usStandCnt * WEAK_SINGAL_BASE_INTERVAL) >= WEAK_SINGAL_ONOFF_TIME_INTERVAL ) {
                usStandCnt = 0;
                ucStatus = WS_BLINK_OFF;
            }
            break;

        case WS_BLINK_OFF:
            usStandCnt++;
            vProdSetRGBCW(ucLedList, 0);
            if( (usStandCnt * WEAK_SINGAL_BASE_INTERVAL) >= WEAK_SINGAL_ONOFF_TIME_INTERVAL ) {
                if( ucONOFFCnt < (WEAK_SINGAL_ONOFF_CNT - 1)) {
                    usStandCnt = 0;
                    ucONOFFCnt++;
                    ucStatus = WS_BLINK_ON;
                } else {
                    usStandCnt = 0;
                    ucONOFFCnt = 0;
                    ucStatus = WS_BREATH_UP;
                }
            }
            break;

        default:
            break;
    }

    opRet = opUserSWTimerStart(WEAKSINGAL_SW_TIME, WEAK_SINGAL_BASE_INTERVAL, vWeakSingalTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("weak signal display restart failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("weak signal display restart failure,light shut down error!");
        }
    }
}

/**
 * @brief: weak signal process init
 *           create timer cb handle
 * @param {none}
 * @retval: none
 */
STATIC VOID vWeakSignalProc(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opUserSWTimerStart(WEAKSINGAL_SW_TIME, WEAK_SINGAL_BASE_INTERVAL, vWeakSingalTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("weak signal display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("weak signal display init failure,light shut down error!");
        }
        return;
    }
}

/**
 * @brief: Production unauthorizeed display timer callback
 *          display styple
 *              C     ->  C on/off mimimus brightness
 *              CW    ->  C on/off mimimus brightness
 *              RGB   ->  R on/off mimimus brightness
 *              RGBC  ->  C on/off mimimus brightness
 *              RGBCW ->  C on/off mimimus brightness
 *          above display on/off as AUZ_TEST_FAIL_TIME_INTERVAL frequency
 * @param {none}
 * @retval: none
 */
STATIC VOID vUnauthorTimeCB(VOID)
{
    OPERATE_LIGHT opRet = -1;
    STATIC UCHAR_T ucInitFlag = FALSE;
    STATIC UCHAR_T ucStatus = FALSE;
    STATIC UCHAR_T ucLedList = 0;
    STATIC USHORT_T usBrightMin = 0xFFFF;
    STATIC USHORT_T usStandCnt = 0;

    if( FALSE == ucInitFlag ) {
        if(tProdConfigData.eLightWay >= PROD_LIGHT_RGB) {
            usBrightMin = PROD_RGB_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitRGBMin / 100.0);         /* calc min bright */
            ucLedList = LED_R;
        } else {
            usBrightMin = PROD_CW_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitCWMin / 100.0);          /* calc min bright */
            ucLedList = LED_C;
        }


        ucInitFlag = TRUE;
    }

    if( TRUE == ucStatus ) {
        usStandCnt++;
        vProdSetRGBCW(ucLedList, usBrightMin);
        if( (usStandCnt * UNAUTHOR_BASE_TIME_INTERVAL) >= AUZ_TEST_FAIL_TIME_INTERVAL ) {
            usStandCnt = 0;
            ucStatus = FALSE;
        }
    } else {
        usStandCnt++;
        vProdSetRGBCW(ucLedList, 0);
        if( (usStandCnt * UNAUTHOR_BASE_TIME_INTERVAL) >= AUZ_TEST_FAIL_TIME_INTERVAL ) {
            usStandCnt = 0;
            ucStatus = TRUE;
        }
    }

    opRet = opUserSWTimerStart(UNAUTHORIZED_SW_TIME, UNAUTHOR_BASE_TIME_INTERVAL, vUnauthorTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("unauthorized display restart failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("unauthorized display restart failure,light shut down!");
        }
    }
}

/**
 * @brief: unauthorized process init
 *           create timer cb handle
 * @param {none}
 * @retval: none
 */
STATIC VOID vUnauthorizedProc(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opUserSWTimerStart(UNAUTHORIZED_SW_TIME, UNAUTHOR_BASE_TIME_INTERVAL, vUnauthorTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("unauthorized display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("unauthorized display init failure,light shut down!");
        }
        return;
    }
}

/**
 * @brief: Production factory test check overall function display timer callback
 *          display styple
 *              C     ->  C blink
 *              CW    ->  C > W blink
 *              RGB   ->  R > G > B blink
 *              RGBC  ->  R > G > B > C blink
 *              RGBCW ->  R > G > B > C > W blink
 *          above display base on PROD_CHECK_BASE_INTERVAL frequency
 * @param {none}
 * @retval: none
 */
STATIC VOID vProdCheckTimeCB(VOID)
{
    OPERATE_LIGHT opRet = -1;
    STATIC UCHAR_T ucInitFlag = FALSE;
    STATIC UCHAR_T ucLEDList[5] = {0};
    STATIC UCHAR_T ucLEDNum = 0;
    STATIC UCHAR_T ucIndex = 0;
    STATIC USHORT_T usStandCnt = 0;
    STATIC UINT_T ulCheckCnt = 0;
    STATIC USHORT_T usCWBrightMax = 0;
    STATIC USHORT_T usRGBBrightMax = 0;
    LIGHT_PROD_TEST_DATA_FLASH_T tProdResult = {0};

    if( FALSE == ucInitFlag ) {
        switch(tProdConfigData.eLightWay)
        {
            case PROD_LIGHT_C:
                ucLEDList[0] = LED_C;
                ucLEDList[1] = LED_OFF;
                break;
            case PROD_LIGHT_CW:
                ucLEDList[0] = LED_C;
                ucLEDList[1] = LED_W;
                break;
            case PROD_LIGHT_RGB:
                ucLEDList[0] = LED_R;
                ucLEDList[1] = LED_G;
                ucLEDList[2] = LED_B;
                break;
            case PROD_LIGHT_RGBC:
                ucLEDList[0] = LED_R;
                ucLEDList[1] = LED_G;
                ucLEDList[2] = LED_B;
                ucLEDList[3] = LED_C;
                break;
            case PROD_LIGHT_RGBCW:
                ucLEDList[0] = LED_R;
                ucLEDList[1] = LED_G;
                ucLEDList[2] = LED_B;
                ucLEDList[3] = LED_C;
                ucLEDList[4] = LED_W;
                break;
            default:
                break;
        }

        ucLEDNum = tProdConfigData.eLightWay;
        if(PROD_LIGHT_C == tProdConfigData.eLightWay) {
            ucLEDNum += 1;              /* @attention: when one way,blink as turn on as C and blink turn off */
        }

        usCWBrightMax = PROD_CW_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitCWMax / 100.0);             /* calc CW max bright */
        usRGBBrightMax = PROD_RGB_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitRGBMax / 100.0);          /* calc RGB max bright */
        ucInitFlag = TRUE;
    }

    if(ucLEDList[ucIndex] <= LED_B ) {  /* R\G\B color led! */
        vProdSetRGBCW(ucLEDList[ucIndex], usRGBBrightMax);
    } else {
        vProdSetRGBCW(ucLEDList[ucIndex], usCWBrightMax);
    }

    usStandCnt++;
    if( (usStandCnt * PROD_CHECK_BASE_INTERVAL) >= PORD_CHECK_ONOF_TIMER_INTERVAL) {                /* blink as PORD_CHECK_ONOF_TIMER_INTERVAL */
        usStandCnt = 0;
        ucIndex++;
        if(ucIndex >= ucLEDNum) {
            ucIndex = 0;
        }
    }

    ulCheckCnt++;
    if( (ulCheckCnt * PROD_CHECK_BASE_INTERVAL) >= PROD_CHECK_TIMER_INTERVAL ) {   /* overall check time over,the test mode change to aging mode! */

        tProdResult.usAgingTestedTime = 0;
        tProdResult.eTestMode = PROD_AGING;

        opRet = opUserFlashWriteProdData(&tProdResult);
        if( LIGHT_OK == opRet ) {
            vAgingStartProc();      /* display goto aging process */
            opRet = opUserSWTimerStop(OVERALLCHECK_SW_TIME);    /* stop timer dominantly error */
            if(opRet != LIGHT_OK) {
                PR_ERR("prod check finish stop timer dominantly error!");
            }
            return;                 /* don't restart OVERALLCHECK_SW_TIME software timer */
        } else {
            PR_NOTICE("Write prod aging time error!");
        }
    }

    opRet = opUserSWTimerStart(OVERALLCHECK_SW_TIME, PROD_CHECK_BASE_INTERVAL, vProdCheckTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("production overall check display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("production overall check display init failure,light shut down!");
        }
    }
}

/**
 * @brief: production overall function check
 * @param {none}
 * @retval: none
 */
STATIC VOID vProdCheckProc(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opUserSWTimerStart(OVERALLCHECK_SW_TIME, PROD_CHECK_BASE_INTERVAL, vProdCheckTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("production overall check display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("production overall check display init failure,light shut down!");
        }
        return;
    }
}

/**
 * @brief: Production factory test aging function start display timer callback
 *          display styple
 *              C     ->  C blink
 *              CW    ->  C W blink
 *              RGB   ->  R G B blink
 *              RGBC  ->  R G B C blink
 *              RGBCW ->  R G B C W blink
 *          above display base on PROD_AGING_START_ONOFF_TIMER_INTERVAL frequency
 * @param {none}
 * @retval: none
 */
STATIC VOID vAgingStartTimeCB(VOID)
{
    OPERATE_LIGHT opRet = -1;
    STATIC UCHAR_T ucInitFlag = FALSE;
    STATIC UCHAR_T ucLEDList[5] = {0};
    STATIC UCHAR_T ucLEDNum = 0;
    STATIC UCHAR_T ucIndex = 0;
    STATIC USHORT_T usStandCnt = 0;
    STATIC USHORT_T usCheckCnt = 0;
    STATIC USHORT_T usCWBrightMax = 0;
    STATIC USHORT_T usRGBBrightMax = 0;
    LIGHT_PROD_TEST_DATA_FLASH_T tProdResult = {0};

    if( FALSE == ucInitFlag ) {
        PR_NOTICE("ligth way %d", tProdConfigData.eLightWay);
        switch(tProdConfigData.eLightWay)
        {
            case PROD_LIGHT_C:
                ucLEDList[0] = LED_C;
                ucLEDList[1] = LED_OFF;
                break;
            case PROD_LIGHT_CW:
                ucLEDList[0] = LED_C;
                ucLEDList[1] = LED_W;
                break;
            case PROD_LIGHT_RGB:
                ucLEDList[0] = LED_R;
                ucLEDList[1] = LED_G;
                ucLEDList[2] = LED_B;
                break;
            case PROD_LIGHT_RGBC:
                ucLEDList[0] = LED_R;
                ucLEDList[1] = LED_G;
                ucLEDList[2] = LED_B;
                ucLEDList[3] = LED_C;
                break;
            case PROD_LIGHT_RGBCW:
                ucLEDList[0] = LED_R;
                ucLEDList[1] = LED_G;
                ucLEDList[2] = LED_B;
                ucLEDList[3] = LED_C;
                ucLEDList[4] = LED_W;
                break;

            default:
                break;
        }

        ucLEDNum = tProdConfigData.eLightWay;
        if(PROD_LIGHT_C == tProdConfigData.eLightWay) {
            ucLEDNum += 1;              /* @attention: when one way,blink as turn on as C and blink turn off */
        }

        usCWBrightMax = PROD_CW_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitCWMax / 100.0);          /* calc cw max bright */
        usRGBBrightMax = PROD_RGB_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitRGBMax / 100.0);       /* calc RGB max bright */
        ucInitFlag = TRUE;
    }

    if(ucLEDList[ucIndex] <= LED_B ) {  /* R\G\B color led! */
        vProdSetRGBCW(ucLEDList[ucIndex], usRGBBrightMax);
    } else {
        vProdSetRGBCW(ucLEDList[ucIndex], usCWBrightMax);
    }

    usStandCnt++;
    if( (usStandCnt * PROD_AGING_START_BASE_INTERVAL) >= PROD_AGING_START_ONOFF_TIMER_INTERVAL) {
        usStandCnt = 0;
        ucIndex++;
        if(ucIndex >= ucLEDNum) {
            ucIndex = 0;
            usCheckCnt++;
        }
    }

    if( usCheckCnt  >= PROD_AGING_START_LOOP_CNT ) {

        opRet = opUserFlashReadProdData(&tProdResult);      /* read prodaging time first */
        if( LIGHT_OK != opRet ) {
            PR_ERR("Read prod aging time error! reset aging time -> 0.");
            tProdResult.usAgingTestedTime = 0;
            tProdResult.eTestMode = PROD_AGING;
        }

        opRet = opUserFlashWriteProdData(&tProdResult);
        if( LIGHT_OK == opRet ) {
            PR_NOTICE("goto aging!");
            vProdAgingProc();       /* display goto aging process */
            opRet = opUserSWTimerStop(AGINGSTART_SW_TIME);    /* stop timer dominantly error */
            if(opRet != LIGHT_OK) {
                PR_ERR("prod aging start display finish stop timer dominantly error!");
            }
            return;                 /* don't restart OVERALLCHECK_SW_TIME software timer */
        } else {
            PR_NOTICE("Write prod aging time error!");
        }
    }

    opRet = opUserSWTimerStart(AGINGSTART_SW_TIME, PROD_AGING_START_BASE_INTERVAL, vAgingStartTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("production aging check start display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("production aging check start display init failure,light shut down error!");
        }
    }

}

/**
 * @brief: production start aging check
 * @param {none}
 * @retval: none
 */
STATIC VOID vAgingStartProc(VOID)
{
    OPERATE_LIGHT opRet = -1;

    PR_NOTICE("prod aging start!");
    opRet = opUserSWTimerStart(AGINGSTART_SW_TIME, PROD_AGING_START_BASE_INTERVAL, vAgingStartTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("production aging check start display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("production aging check start display init failure,light shut down error!");
        }
        return;
    }
}

/**
 * @brief: Production factory test aging function display timer callback
 *          display styple
 *              C     ->  C aging
 *              CW    ->  C aging > W aging
 *              RGB   ->  RGB aging
 *              RGBC  ->  C aging > RGB aging
 *              RGBCW ->  C aging > W aging > RGB aging
 *          above display base on  frequency
 * @param {none}
 * @retval: none
 */
STATIC VOID vProdAgingTimeCB(VOID)
{
    OPERATE_LIGHT opRet = -1;
    STATIC UCHAR_T ucInitFlag = FALSE;
    STATIC UCHAR_T ucAgingTimeList[3] = {0};
    UCHAR_T ucLightNum = 0;
    STATIC USHORT_T usAgingTime = 0;   /* unit:min */
    STATIC UINT_T uiStandCnt = 0;
    STATIC UCHAR_T ucIndex = 0;
    STATIC USHORT_T usCWBrightMax = PROD_CW_BRIGHT_VALUE_MAX;
    STATIC USHORT_T usCWBrightMin = PROD_CW_BRIGHT_VALUE_MAX * 0.1;
    STATIC USHORT_T usRGBBrightMax = PROD_RGB_BRIGHT_VALUE_MAX;
    STATIC USHORT_T usRGBBrightMin = PROD_CW_BRIGHT_VALUE_MAX * 0.1;
    LIGHT_PROD_TEST_DATA_FLASH_T tProdResult = {0};

    CONST UCHAR_T ucTargetTime[5][3] = {
            /* C    W       RGB aging time*/
            { 40,   0 ,     0 },          /* 1 way light  */
            { 20,   20,     0 },          /* 2 way light  */
            { 0 ,   0 ,     40},          /* 3 way light  */
            { 30,   0 ,     10},          /* 4 way light  */
            { 20,   20,     10}           /* 5 way light  */
    };

    if( FALSE == ucInitFlag ) {

        ucLightNum = tProdConfigData.eLightWay - 1;     /* attention: light way range from 1 to 5! */
        memcpy(ucAgingTimeList, ucTargetTime[ucLightNum], 3 * (SIZEOF(UCHAR_T)));

        PR_NOTICE("Aging time C --> %d W --> %d RGB --> %d", ucAgingTimeList[0], ucAgingTimeList[1], ucAgingTimeList[2]);

        opRet = opUserFlashReadProdData(&tProdResult);      /* read prodaging time first */
        if( LIGHT_OK != opRet ) {
            PR_ERR("Read prod aging time error! reset aging time -> 0.");
            tProdResult.usAgingTestedTime = 0;
            tProdResult.eTestMode = PROD_AGING;
        }

        usAgingTime = tProdResult.usAgingTestedTime;
        PR_NOTICE("already aging time:%d(min)!",usAgingTime);

        usCWBrightMax = PROD_CW_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitCWMax / 100.0);          /* calc CW max bright */
        usCWBrightMin = PROD_CW_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitCWMin / 100.0);          /* calc CW min bright */

        usRGBBrightMax = PROD_RGB_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitRGBMax / 100.0);        /* calc RGB max bright */
        usRGBBrightMin = PROD_RGB_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitRGBMin / 100.0);        /* calc RGB min bright */

        if(PROD_LIGHT_RGB == tProdConfigData.eLightWay) {
            ucIndex = AGING_RGB;        /* if light is RGB aging test need to aging RGB directly */
        } else {
            ;
        }

        ucInitFlag = TRUE;
    }

    uiStandCnt++;

    if( (uiStandCnt * PROD_AGING_BASE_INTERVAL) >=  (60 * 1000)) {      /* save aging time per minutes */
        usAgingTime++;

        tProdResult.usAgingTestedTime = usAgingTime;
        tProdResult.eTestMode = PROD_AGING;

        opRet = opUserFlashWriteProdData(&tProdResult);
        if( LIGHT_OK == opRet ) {
            uiStandCnt = 0;
            PR_NOTICE("aging has pass %d min!", usAgingTime );
        } else {
            usAgingTime--;
            PR_ERR("Write prod aging time error!");
        }
    }

    if( usAgingTime >= (ucAgingTimeList[0] + ucAgingTimeList[1] + ucAgingTimeList[2]) ) {   /* aging time satisfied! */

        tProdResult.usAgingTestedTime = usAgingTime;
        tProdResult.eTestMode = PROD_REPEAT;

        opRet = opUserFlashWriteProdData(&tProdResult);
        if( LIGHT_OK == opRet ) {

            switch (tProdConfigData.eLightWay)
            {
                case PROD_LIGHT_C:
                    vProdSetRGBCW(LED_C, usCWBrightMin);
                    break;

                case PROD_LIGHT_CW:
                    vProdSetRGBCW(LED_CW, usCWBrightMin);
                    break;

                case PROD_LIGHT_RGB:
                case PROD_LIGHT_RGBC:
                case PROD_LIGHT_RGBCW:
                    vProdSetRGBCW(LED_G, usRGBBrightMin);
                    break;

                default:
                    break;
            }

             opRet = opUserSWTimerStop(AGINGTEST_SW_TIME);    /* stop timer dominantly error */
            if(opRet != LIGHT_OK) {
                PR_ERR("prod aging check finish stop timer dominantly error!");
            }
            return;
        } else {
            PR_ERR("Prod aging finish,but write flash error! try again!");
        }
    }

    switch (ucIndex)
    {
        case AGING_C:       /* constant-expression */
            vProdSetRGBCW(LED_C, usCWBrightMax);
            if( usAgingTime >= ucAgingTimeList[0] ) {
                ucIndex = AGING_W;
            }
            break;

        case AGING_W:
            vProdSetRGBCW(LED_W, usCWBrightMax);
            if( usAgingTime >= (ucAgingTimeList[0] + ucAgingTimeList[1]) ) {
                ucIndex = AGING_RGB;
            }
            break;

        case AGING_RGB:
            vProdSetRGBCW(LED_RGB, usRGBBrightMax);
            if( usAgingTime >= (ucAgingTimeList[0] + ucAgingTimeList[1] + ucAgingTimeList[2]) ) {
                ;
            }
            break;

        default:
            break;
    }

    opRet = opUserSWTimerStart(AGINGTEST_SW_TIME, PROD_AGING_BASE_INTERVAL, vProdAgingTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("production aging check display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("production aging check display init failure,light shut down error!");
        }
    }
}

/**
 * @brief: production aging check
 * @param {none}
 * @retval: none
 */
STATIC VOID vProdAgingProc(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opUserSWTimerStart(AGINGTEST_SW_TIME, PROD_AGING_BASE_INTERVAL, vProdAgingTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("production aging check display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("production aging check display init failure,light shut down error!");
        }
        return;
    }
}

/**
 * @brief: Production factory test repeat function display timer callback
 *          display styple
 *              C     ->  C bringt up then bright down
 *              CW    ->  C bringt up then bright down > W
 *              RGB   ->  R bringt up then bright down > G > B
 *              RGBC  ->  R bringt up then bright down > G > B > C
 *              RGBCW ->  R bringt up then bright down > G > B > C > W
 *          above display base on  frequency
 * @param {none}
 * @retval: none
 */
STATIC VOID vProdRepeatTimeCB(VOID)
{
    OPERATE_LIGHT opRet = -1;
    STATIC UCHAR_T ucInitFlag = FALSE;
    STATIC UCHAR_T ucLEDList[5] = {0};
    STATIC UCHAR_T ucLEDNum = 0;
    STATIC UCHAR_T ucIndex = 0;
    STATIC UCHAR_T ucStatus = TRUE;
    STATIC USHORT_T usCWBrightMin = 0;
    STATIC USHORT_T usCWBrightMax = 0;
    STATIC USHORT_T usCWChangeStep = 0;
    STATIC USHORT_T usRGBBrightMin = 0;
    STATIC USHORT_T usRGBBrightMax = 0;
    STATIC USHORT_T usRGBChangeStep = 0;
    STATIC SHORT_T sSendValue = 0;

    if( FALSE == ucInitFlag ) {
        switch(tProdConfigData.eLightWay)
        {
            case PROD_LIGHT_C:
                ucLEDList[0] = LED_C;
                break;
            case PROD_LIGHT_CW:
                ucLEDList[0] = LED_C;
                ucLEDList[1] = LED_W;
                break;
            case PROD_LIGHT_RGB:
                ucLEDList[0] = LED_R;
                ucLEDList[1] = LED_G;
                ucLEDList[2] = LED_B;
                break;
            case PROD_LIGHT_RGBC:
                ucLEDList[0] = LED_R;
                ucLEDList[1] = LED_G;
                ucLEDList[2] = LED_B;
                ucLEDList[3] = LED_C;
                break;
            case PROD_LIGHT_RGBCW:
                ucLEDList[0] = LED_R;
                ucLEDList[1] = LED_G;
                ucLEDList[2] = LED_B;
                ucLEDList[3] = LED_C;
                ucLEDList[4] = LED_W;
                break;

            case PROD_LIGHT_MAX:
            default:
                break;
        }

        ucLEDNum = tProdConfigData.eLightWay;

        usCWBrightMin = PROD_CW_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitCWMin / 100.0);             /* calc cw min bright */
        usCWBrightMax = PROD_CW_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitCWMax / 100.0);             /* calc cw max bright */
        usCWChangeStep = (usCWBrightMax - usCWBrightMin) / (PROD_REPEAT_ONOFF_TIMER_INTERVAL / PROD_REPEAT_BASE_INERVAL);

        usRGBBrightMin = PROD_RGB_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitRGBMin / 100.0);          /* calc rgb min bright */
        usRGBBrightMax = PROD_RGB_BRIGHT_VALUE_MAX * (tProdConfigData.ucLimitRGBMax / 100.0);          /* calc rgb max bright */
        usRGBChangeStep = (usRGBBrightMax - usRGBBrightMin) / (PROD_REPEAT_ONOFF_TIMER_INTERVAL / PROD_REPEAT_BASE_INERVAL);

        if(ucLEDList[0] <= LED_B) {
            sSendValue = usRGBBrightMin;
        } else {
            sSendValue = usCWBrightMin;
        }

        ucInitFlag = TRUE;
    }

    if( TRUE == ucStatus ) {

        vProdSetRGBCW(ucLEDList[ucIndex], sSendValue);
        sSendValue += (ucLEDList[ucIndex] <= LED_B ) ? usRGBChangeStep : usCWChangeStep;
        if( sSendValue >= ((ucLEDList[ucIndex] <= LED_B ) ? usRGBBrightMax : usCWBrightMax)) {
            ucStatus = FALSE;
            sSendValue = (ucLEDList[ucIndex] <= LED_B ) ? usRGBBrightMax : usCWBrightMax;
        }

    } else {

        vProdSetRGBCW(ucLEDList[ucIndex], sSendValue);
        sSendValue -= (ucLEDList[ucIndex] <= LED_B ) ? usRGBChangeStep : usCWChangeStep;
        if( sSendValue <= ((ucLEDList[ucIndex] <= LED_B ) ? usRGBBrightMin : usCWBrightMin)) {
            ucStatus = TRUE;
            ucIndex++;
            if(ucIndex >= ucLEDNum) {
                ucIndex = 0;
            }
            sSendValue = (ucLEDList[ucIndex] <= LED_B ) ? usRGBBrightMin : usCWBrightMin;
        }
    }

    opRet = opUserSWTimerStart(REPEATTEST_SW_TIME, PROD_REPEAT_BASE_INERVAL, vProdRepeatTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("production repeat check display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("production repeat check display init failure,light shut down error!");
        }
    }
}

/**
 * @brief: production factory repeat test -- test2
 * @param {none}
 * @retval: none
 */
STATIC VOID vProdRepeatProc(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opUserSWTimerStart(REPEATTEST_SW_TIME, PROD_REPEAT_BASE_INERVAL, vProdRepeatTimeCB);
    if( LIGHT_OK != opRet ) {
        PR_ERR("production repeat check display init failure,light shut down!");
        opRet = opLightSetRGBCW(0,0,0,0,0);             /* light shut down */
        if( LIGHT_OK != opRet ) {
            PR_ERR("production repeat check display init failure,light shut down error!");
        }
        return;
    }
}

/**
 * @brief: prodution test process cb
 * @param {BOOL_T bAuthorized -> authorized flag}
 * @param {CHAR_T cSignalStrength -> singal strength}
 * @retval: none
 */
VOID vProdTestCB(BOOL_T bAuthorized, CHAR_T cSignalStrength)
{
    OPERATE_LIGHT opRet = -1;
    LIGHT_PROD_TEST_DATA_FLASH_T tProdResult;

    vLightCtrlDisable();        /* turn off ctrl proc firstly! */

    if(gucProdInitFlag != TRUE) {
        PR_ERR("Prod test not init,can't do test!");
        return;
    }

    if( cSignalStrength < AUZ_TEST_WEAK_SIGNAL ) {  /* weak singal */
        PR_ERR("Weak singal!");
        vWeakSignalProc();
        return;
    }

    if( FALSE == bAuthorized ) {                     /* unauthorized ! */
        PR_ERR("Production unauthorized!");
        vUnauthorizedProc();
        return;
    }

    PR_NOTICE("PROD FACTORY START!");

    opRet = opUserFlashReadProdData(&tProdResult);      /* read prodaging time first */
    if( LIGHT_OK != opRet ) {
        tProdResult.eTestMode = PROD_CHECK;
        tProdResult.usAgingTestedTime = 0;
        PR_NOTICE("First prod test!");
    }
    PR_NOTICE("Prod test mode -> %d, aging already time %d(min)",tProdResult.eTestMode, tProdResult.usAgingTestedTime);

    switch (tProdResult.eTestMode)
    {
        case PROD_CHECK:            /* production overall check */
            vProdCheckProc();
            break;

        case PROD_AGING:
            vAgingStartProc();
            break;

        case PROD_REPEAT:
            vProdRepeatProc();
            break;

        default:
            break;
    }
}

/**
 * @brief: prodution test process init
 * @param {none}
 * @retval: none
 */
OPERATE_LIGHT opLightProdInit(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opDeviceCfgDataLoad();       /* make sure config data have been load! */
    if(opRet != LIGHT_OK) {
        PR_ERR("Config Data Load error!");
        return LIGHT_INVALID_PARM;
    }

    tProdConfigData.eLightWay = cDeviceCfgGetColorMode();
    tProdConfigData.eBrightMode = cDeviceCfgGetCWType();
    tProdConfigData.ucLimitCWMax = cDeviceCfgGetWhiteMax();
    tProdConfigData.ucLimitCWMin = cDeviceCfgGetWhiteMin();
    tProdConfigData.ucLimitRGBMax = cDeviceCfgGetColorMax();
    tProdConfigData.ucLimitRGBMin = cDeviceCfgGetColorMin();

#if (LIGHT_CFG_PROD_DRIVER_NEED_INIT == 1)
    tProdConfigData.tDriveCfg.eMode = cDeviceCfgGetDriverMode();

    switch(tProdConfigData.tDriveCfg.eMode)
    {
        case DRIVER_MODE_PWM:{
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.usFreq = usDeviceCfgGetPwmHz();
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.ucList[0] = cDeviceCfgGetRedPin();
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.ucList[1] = cDeviceCfgGetGreenPin();
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.ucList[2] = cDeviceCfgGetBluePin();
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.ucList[3] = cDeviceCfgGetColdPin();
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.ucList[4] = cDeviceCfgGetWarmPin();
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.ucChannelNum = cDeviceCfgGetColorMode();

            if(tProdConfigData.eLightWay == PROD_LIGHT_RGB) {     /* rgb  */
                tProdConfigData.tDriveCfg.uConfig.tPwmInit.bPolarity = cDeviceCfgGetRedPinLevel();
                tProdConfigData.tDriveCfg.uConfig.tPwmInit.usDuty = (cDeviceCfgGetRedPinLevel() == 1) ?  0 : 1000;
            }else {
                tProdConfigData.tDriveCfg.uConfig.tPwmInit.bPolarity = cDeviceCfgGetColdPinLevel();
                tProdConfigData.tDriveCfg.uConfig.tPwmInit.usDuty = (cDeviceCfgGetColdPinLevel() == 1) ?  0 : 1000;
            }
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.ucCtrlPin = cDeviceCfgGetCtrlPin();
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.bCtrlLevel = cDeviceCfgGetCtrlPinLevel();
            tProdConfigData.tDriveCfg.uConfig.tPwmInit.bCCTFlag = cDeviceCfgGetCWType();
            break;
        }

        default:{
            opRet = LIGHT_COM_ERROR;
            PR_ERR("Driver mode ERROR");
            break;
        }

    }

    opRet = opLightDriveInit(&tProdConfigData.tDriveCfg);
    if(opRet != LIGHT_OK) {
        PR_ERR("Light drive init error!");
        return LIGHT_COM_ERROR;
    }
#endif
    gucProdInitFlag = TRUE;

    return LIGHT_OK;
}


