/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: light_control.c
 * @Description: light control process
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-28 14:35:54
 * @LastEditTime: 2020-03-06 12:33:12
 */

#include "light_control.h"
#include "light_printf.h"
#include "light_tools.h"
#include "user_flash.h"
#include "user_timer.h"

/// hardware timer cycle (unit:ms)
#define HW_TIMER_CYCLE_MS           HW_TIMER_CYCLE_US/1000

/// light shade change cycle (unit:ms) enable to change
#define LIGHT_SHADE_CYCLE           (HW_TIMER_CYCLE_US/1000)

/// Control calculate range 0 ~ 1000
#define CTRL_CAL_VALUE_RANGE        1000
/// Control cw calc max bright value
#define CTRL_CW_BRIGHT_VALUE_MAX    CTRL_CAL_VALUE_RANGE
/// Control cw calc min bright value (max value * 0.01)
#define CTRL_CW_BRIGHT_VALUE_MIN    (CTRL_CW_BRIGHT_VALUE_MAX * 0.01)

/// Control RGB calc max bright value
#define CTRL_RGB_BRIGHT_VALUE_MAX   CTRL_CAL_VALUE_RANGE
/// Control RGB calc min bright value
#define CTRL_RGB_BRIGHT_VALUE_MIN   (CTRL_RGB_BRIGHT_VALUE_MAX * 0.01)

/// control power gain (100 --> gain = 1 times )
#define CTRL_POEWER_GAIN_MIN        100
/// control power gain (200 --> gain = 2 times )
#define CTRL_POEWER_GAIN_MAX        200

/// control CW&RGB bright limit max
#define CTRL_BRIGHT_LIMIT_MAX       100
/// control CW&RGB bright limit min
#define CTRL_BRIGHT_LIMIT_MIN       0

/// control CW&RGB temper limit max
#define CTRL_TEMPER_LIMIT_MAX       100
/// control CW&RGB temper limit min
#define CTRL_TEMPER_LIMIT_MIN       0

#define SHADE_STEP_GAIN_DEFAULT     (CTRL_CAL_VALUE_RANGE * LIGHT_SHADE_CYCLE / SHADE_CHANG_MAX_TIME)

#define CHECK_INIT_OK();        if(opLightCheckInit() != LIGHT_OK) {\
                                    return LIGHT_COM_ERROR;\
                                }

/**
 * @brief hardware timer drive actively control structure
 */
typedef struct
{
    UINT_T uiTargetCnt;     /* shade time per one change */
    UINT_T uiCnt;           /* hardware time */
    BOOL_T bEnable;         /* hardware time deal with shade change flag */
}HW_TIMER_PARAM_S;

/// hardware time set param
STATIC HW_TIMER_PARAM_S HWTimerParam = {
    .uiTargetCnt = 0xFFFFFF,
    .uiCnt = 0,
    .bEnable = FALSE,
};

/// light control initlize flag
STATIC UCHAR_T gucLightCTRLInitFlag = FALSE;

/// light ctrl configuration
STATIC LIGHT_CTRL_CFG_T tLightCfgData;

/// light ctrl data(status)
STATIC LIGHT_CTRL_DATA_T tLightCtrlData;

/// light ctrl handle(process)
STATIC LIGHT_CTRL_HANDLE_T tLightCtrlHandle;

/// Light shade change step gain
STATIC UINT_T uiStepGain = SHADE_STEP_GAIN_DEFAULT;

STATIC VOID vLightCtrlSceneAutoRestartTimeCB(VOID);
STATIC VOID vLightCtrlSceneChangeStop(VOID);
STATIC VOID vLightCtrlShadeStop(VOID);
#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)
STATIC OPERATE_LIGHT opLightCtrlLowPowerProc(IN UCHAR_T ucONOFF);
#endif


/**
 * @brief: check light ifnot init ok
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
STATIC OPERATE_LIGHT opLightCheckInit(VOID)
{
    if(!gucLightCTRLInitFlag) {
        PR_ERR("light control don't initlize, can't active!");\
        return LIGHT_COM_ERROR;
    }

    return LIGHT_OK;
}

#if (LIGHT_CFG_INIT_PARAM_CHECK == 1)
/**
 * @brief: check lightway and def color(defcolor netcolor) vaild
 * @param {IN CTRL_LIGHT_WAY_E eLightWay -> light way type}
 * @param {IN CTRL_DEF_COLOR_E eColor -> default color}
 * @retval: OPERATE_LIGHT
 */
STATIC OPERATE_LIGHT opLightCtrlCheckColorParam(IN CTRL_LIGHT_WAY_E eLightWay, IN CTRL_DEF_COLOR_E eColor)
{
    switch(eLightWay) {
        case LIGHT_C:
            if(eColor != DEF_COLOR_C ) {
                return LIGHT_INVALID_PARM;
            }
            break;

        case LIGHT_CW:
            if((eColor != DEF_COLOR_C) && (eColor != DEF_COLOR_W)) {
                return LIGHT_INVALID_PARM;
            }
            break;

        case LIGHT_RGB:
            if((eColor == DEF_COLOR_C) || (eColor == DEF_COLOR_W)) {
                return LIGHT_INVALID_PARM;
            }
            break;

        case LIGHT_RGBC:
            if(eColor == DEF_COLOR_W) {
                return LIGHT_INVALID_PARM;
            }
            break;

        case LIGHT_RGBCW:
        case LIGHT_MAX:
        default:
            break;
    }

    return LIGHT_OK;
}
#endif

/**
 * @brief: set light ctrl data to default according lightCfgData
 * @param {none}
 * @retval: none
 */
VOID vLightCtrlDataReset(VOID)
{
    CHAR_T cStrTemp[5] = {0};
    CHAR_T cHSVTemp[13] = {0};
    USHORT_T usRedTemp = 0, usGreenTemp = 0, usBlueTemp = 0;
    USHORT_T usHTemp = 0, usSTemp = 0, usVTemp = 0;

    vLightShadeCtrlDisable();

    memset(&tLightCtrlData, 0, SIZEOF(LIGHT_CTRL_DATA_T));

    tLightCtrlData.bSwitch = TRUE;

    if((LIGHT_RGB == tLightCfgData.eLightWay) || (tLightCfgData.eDefColor >= DEF_COLOR_R)) { //RGB 3 way light,just have color mode
        tLightCtrlData.eMode = COLOR_MODE;
    } else {
        tLightCtrlData.eMode = WHITE_MODE;
    }

    PR_DEBUG("def bright %d", tLightCfgData.usDefBright);
    PR_DEBUG("def temper %d", tLightCfgData.usDefTemper);
    tLightCtrlData.usBright = tLightCfgData.usDefBright;
    tLightCtrlData.usTemper = tLightCfgData.usDefTemper;

    USHORT_T usMax = 0, usMin = 0, usBrightTemp = 0;

    usMax = CTRL_RGB_BRIGHT_VALUE_MAX * ( (FLOAT_T) tLightCfgData.ucLimitRGBMax / 100.0 );
    usMin = CTRL_RGB_BRIGHT_VALUE_MAX * ( (FLOAT_T) tLightCfgData.ucLimitRGBMin / 100.0 );

    usBrightTemp = ( tLightCfgData.usDefBright - CTRL_RGB_BRIGHT_VALUE_MIN ) * ( usMax - usMin ) / \
                ( CTRL_RGB_BRIGHT_VALUE_MAX - CTRL_RGB_BRIGHT_VALUE_MIN ) + usMin;

    if(DEF_COLOR_R == tLightCfgData.eDefColor) {      /* don't save RGB data to original ctrl data1\2\3, to avoid mistake */
        tLightCtrlData.tColor.usRed = usBrightTemp;   /* just need save rgb limit data, gamma proc will deal in calcRGBCW */
        usRedTemp = tLightCfgData.usDefBright ;
    } else if(DEF_COLOR_G == tLightCfgData.eDefColor) {
        tLightCtrlData.tColor.usGreen = usBrightTemp;
        usGreenTemp = tLightCfgData.usDefBright ;
    } else if(DEF_COLOR_B == tLightCfgData.eDefColor) {
        tLightCtrlData.tColor.usBlue = usBrightTemp;
        usBlueTemp = tLightCfgData.usDefBright ;
    } else {
        tLightCtrlData.tColor.usRed = usBrightTemp;
        usRedTemp = tLightCfgData.usDefBright ;
    }

    vLightToolRGB2HSV(usRedTemp, usGreenTemp, usBlueTemp, &usHTemp, &usSTemp, &usVTemp);

    PR_DEBUG("default color HSV %d %d %d ", usHTemp, usSTemp, usVTemp);
    vNum2Str(4, usHTemp, 5, cStrTemp);
    strcpy(cHSVTemp, cStrTemp);

    vNum2Str(4, usSTemp, 5, cStrTemp);
    strcat(cHSVTemp, cStrTemp);

    vNum2Str(4, usVTemp, 5, cStrTemp);
    strcat(cHSVTemp, cStrTemp);
    strcpy((CHAR_T*)&tLightCtrlData.tColorOrigin.ucColorStr, cHSVTemp);
    PR_DEBUG("default color origin string %s", tLightCtrlData.tColorOrigin.ucColorStr);
    tLightCtrlData.tColorOrigin.usHue = usHTemp;
    tLightCtrlData.tColorOrigin.usSat = usSTemp;
    tLightCtrlData.tColorOrigin.usValue = usVTemp;
    PR_DEBUG("default color data HSV %d %d %d",tLightCtrlData.tColorOrigin.usHue,tLightCtrlData.tColorOrigin.usSat,tLightCtrlData.tColorOrigin.usValue);

    switch(tLightCfgData.eLightWay)
    {
        case LIGHT_C:
            memcpy(tLightCtrlData.cScene, SCENE_DATA_DEFAULT_C, strlen(SCENE_DATA_DEFAULT_C));
            break;

        case LIGHT_CW:
            memcpy(tLightCtrlData.cScene, SCENE_DATA_DEFAULT_CW, strlen(SCENE_DATA_DEFAULT_CW));
            break;

        case LIGHT_RGB:
            memcpy(tLightCtrlData.cScene, SCENE_DATA_DEFAULT_RGB, strlen(SCENE_DATA_DEFAULT_RGB));
            break;

        case LIGHT_RGBC:
            memcpy(tLightCtrlData.cScene, SCENE_DATA_DEFAULT_C, strlen(SCENE_DATA_DEFAULT_C));
            break;

        case LIGHT_RGBCW:
            memcpy(tLightCtrlData.cScene, SCENE_DATA_DEFAULT_CW, strlen(SCENE_DATA_DEFAULT_CW));
            break;

        default:
            break;
    }
    PR_DEBUG("default scene data %s", tLightCtrlData.cScene);

    tLightCtrlData.bSceneFirstSet = FALSE;

    tLightCtrlData.uiCountDown = 0;

}

/**
 * @brief: Light system control hardware timer callback
 * @param {none}
 * @attention: this function need to implement by system,
 *              decide how to call vLightCtrlShadeGradually function.
 * @retval: none
 */
WEAK VOID vLightSysHWTimerCB(VOID)
{
    ;
}

/**
 * @brief: Light control hardware timer callback
 * @param {none}
 * @attention: vLightSysHWTimerCB() func need to implement by system
 * @retval: none
 */
VOID vLightCtrlHWTimerCB(VOID)
{
    if(HWTimerParam.bEnable != TRUE) {
        return;
    }

    HWTimerParam.uiCnt += HW_TIMER_CYCLE_MS;
    if(HWTimerParam.uiCnt >= HWTimerParam.uiTargetCnt) {
        vLightSysHWTimerCB();
        HWTimerParam.uiCnt = 0;
    }
}

/**
 * @brief: light control init
 * @param {IN LIGHT_CTRL_CFG_T *pConfig_Data -> init parm}
 * @attention: this function need apply bLightSysHWRebootJudge();
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlInit(IN LIGHT_CTRL_CFG_T *pConfigData)
{
    OPERATE_LIGHT opRet = -1;
    LIGHT_APP_DATA_FLASH_T tAPPData ;
    BOOL_T bHWRebootFlag;

    if(gucLightCTRLInitFlag) {
        return LIGHT_OK;
    }

#if (LIGHT_CFG_INIT_PARAM_CHECK == 1)
    /* light way cfg */
    if(pConfigData ->eLightWay >= LIGHT_MAX) {
        PR_ERR("light way set error, max way is 5-way!");
        return LIGHT_INVALID_PARM;
    }

    /* CW / CCT Drive mode setting */
    if( pConfigData ->eBrightMode >= BRIGHT_MODE_MAX ) {
        PR_ERR("light drive mode set error!!");
        return LIGHT_INVALID_PARM;
    }

    /* turn on/off change directly or gradually setting */
    if( pConfigData ->eSwitchMode >= SWITCH_MAX ) {
        PR_ERR("light turn on/off change mode set error!!");
        return LIGHT_INVALID_PARM;
    }

    /* light def color */
    opRet = opLightCtrlCheckColorParam(pConfigData ->eLightWay, pConfigData ->eDefColor);
    if(opRet != LIGHT_OK) {
        PR_ERR("Default color set not match to light way!");
        return LIGHT_INVALID_PARM;
    }

    /* light def bright, enable config max is 100 */
    if(pConfigData ->usDefBright > CTRL_BRIGHT_LIMIT_MAX) {
        PR_ERR("light def bright set error!");
        return LIGHT_INVALID_PARM;
    }

    /* light def temperature, enable config max is 100 */
    if(pConfigData ->usDefTemper > CTRL_TEMPER_LIMIT_MAX) {
        PR_ERR("light def temperature set error!");
        return LIGHT_INVALID_PARM;
    }

    /* power gain setting, enable config range: 100~200 */
    if( ( pConfigData ->ucPowerGain < CTRL_POEWER_GAIN_MIN ) || \
        (pConfigData ->ucPowerGain > CTRL_POEWER_GAIN_MAX ) ) {
        PR_ERR("light power gain set error!!");
        return LIGHT_INVALID_PARM;
    }

    /* light CW bright limit max&min setting */
    if( ( pConfigData ->ucLimitCWMax > CTRL_BRIGHT_LIMIT_MAX ) || \
        (pConfigData ->ucLimitCWMin > pConfigData ->ucLimitCWMax ) ) {
        PR_ERR("light CW white bright max&min limit set error!!");
        return LIGHT_INVALID_PARM;
    }

    /* light RGB color bright limit max&min setting */
    if( ( pConfigData ->ucLimitRGBMax > CTRL_BRIGHT_LIMIT_MAX ) || \
        (pConfigData ->ucLimitRGBMin > pConfigData ->ucLimitRGBMax ) ) {
        PR_ERR("light RGB color bright max&min limit set error!!");
        return LIGHT_INVALID_PARM;
    }

    /* reset cnt set */
    if(pConfigData ->ucResetCnt <= 0) {
        PR_ERR("light reset cnt set invaild!");
        return LIGHT_INVALID_PARM;
    }

    /* the color when connecting */
    PR_DEBUG("light net color %d",pConfigData ->eNetColor);
    opRet = opLightCtrlCheckColorParam(pConfigData ->eLightWay, pConfigData ->eNetColor);
    if(opRet != LIGHT_OK) {
        PR_ERR("Default net color set not match to light way!");
        return LIGHT_INVALID_PARM;
    }

    /* the bright when connecting */
    if(pConfigData ->usNetBright > CTRL_BRIGHT_LIMIT_MAX) {
        PR_ERR("light net match bright set error!");
        return LIGHT_INVALID_PARM;
    }

   /* the temperature when connecting */
    if(pConfigData ->usNetTemper > CTRL_TEMPER_LIMIT_MAX) {
        PR_ERR("light net match bright set error!");
        return LIGHT_INVALID_PARM;
    }
#endif

    /* attention oem cfg defbright range from 0~100,need to chang 0~1000 */
    pConfigData ->usDefBright = (USHORT_T) (CTRL_CW_BRIGHT_VALUE_MAX * ((FLOAT_T) pConfigData ->usDefBright / CTRL_BRIGHT_LIMIT_MAX));
    /* attention oem cfg deftemper range from 0~100,need to chang 0~1000 */
    pConfigData ->usDefTemper = (USHORT_T) (CTRL_CW_BRIGHT_VALUE_MAX * ((FLOAT_T) pConfigData ->usDefTemper / CTRL_TEMPER_LIMIT_MAX));
    /* attention oem cfg netbright range from 0~100,need to chang 0~1000 */
    pConfigData ->usNetBright = CTRL_CW_BRIGHT_VALUE_MAX * ((FLOAT_T) pConfigData ->usNetBright / CTRL_BRIGHT_LIMIT_MAX);
    pConfigData ->usNetTemper = CTRL_CW_BRIGHT_VALUE_MAX * ((FLOAT_T) pConfigData ->usNetTemper / CTRL_TEMPER_LIMIT_MAX);
    /* the hardware drive cfg & init! */
    opRet = opLightDriveInit(&(pConfigData ->tDriveCfg));
    if(opRet != LIGHT_OK) {
        PR_ERR("Light drive init error!");
        return LIGHT_COM_ERROR;
    }

    memset(&tLightCfgData, 0, SIZEOF(LIGHT_CTRL_CFG_T));
    memcpy(&tLightCfgData, pConfigData, SIZEOF(LIGHT_CTRL_CFG_T));
    memset(&tLightCtrlHandle, 0, SIZEOF(LIGHT_CTRL_HANDLE_T));

    memset(&tAPPData, 0, SIZEOF(LIGHT_APP_DATA_FLASH_T));
    opRet = opUserFlashReadAppData(&tAPPData);       /* read app flash data! */
    if(opRet != LIGHT_OK) {
        PR_NOTICE("No application data!");
        vLightCtrlDataReset();
    }else {
        bHWRebootFlag = bLightSysHWRebootJudge();
        tLightCtrlData.bSwitch = bHWRebootFlag ? TRUE : tAPPData.bPower;
        tLightCtrlData.eMode = tAPPData.eMode;
        tLightCtrlData.usBright = tAPPData.usBright;
        tLightCtrlData.usTemper = tAPPData.usTemper;
        tLightCtrlData.tColor.usRed = tAPPData.tColor.usRed;
        tLightCtrlData.tColor.usGreen = tAPPData.tColor.usGreen;
        tLightCtrlData.tColor.usBlue = tAPPData.tColor.usBlue;
        tLightCtrlData.tColorOrigin.usHue = tAPPData.tColorOrigin.usHue;
        tLightCtrlData.tColorOrigin.usSat = tAPPData.tColorOrigin.usSat;
        tLightCtrlData.tColorOrigin.usValue = tAPPData.tColorOrigin.usValue;
        strcpy((CHAR_T*)&tLightCtrlData.tColorOrigin.ucColorStr, (CHAR_T*)&tAPPData.tColorOrigin.ucColorStr);
        memcpy(&tLightCtrlData.cScene, &tAPPData.cScene, SIZEOF(tAPPData.cScene));
        tLightCtrlData.bSceneFirstSet = TRUE;
        tLightCtrlData.uiCountDown = 0;
    }
    opRet = opUserHWTimerStart(HW_TIMER_CYCLE_US, (VOID *)vLightCtrlHWTimerCB);     /* start shade need hardware timer! */
    if(opRet != LIGHT_OK) {
        PR_ERR("Light hardware timer init error!");
        return opRet;
    }
    gucLightCTRLInitFlag = TRUE;

    return LIGHT_OK;
}

/**
 * @brief: get change gradually process the max error of 5ways
 * @param {IN BRIGHT_DATA_T *pTargetVal -> target set}
 * @param {IN BRIGHT_DATA_T *pCurrVal -> current value}
 * @retval: none
 */
STATIC USHORT_T usLightGetShadeMAX(IN BRIGHT_DATA_T *pTargetVal, IN BRIGHT_DATA_T *pCurrVal)
{
    USHORT_T usMaxValue = 0;

    usMaxValue = (USHORT_T) uiLightToolGetMaxValue(\
                uiLightToolGetABSValue(pTargetVal->usRed - pCurrVal->usRed), \
                uiLightToolGetABSValue(pTargetVal->usGreen - pCurrVal->usGreen),\
                uiLightToolGetABSValue(pTargetVal->usBlue - pCurrVal->usBlue),\
                uiLightToolGetABSValue(pTargetVal->usWhite - pCurrVal->usWhite),\
                uiLightToolGetABSValue(pTargetVal->usWarm - pCurrVal->usWarm)\
                );

    return usMaxValue;
}

/**
 * @brief: Light control shade change start
 * @param {UINT_T uiShadeTimeMS -> Shade period,unit:ms}
 * @retval: none
 */
STATIC VOID vLightCtrlShadeStart(IN UINT_T uiShadeTimeMS)
{
    HWTimerParam.bEnable = FALSE;
    HWTimerParam.uiCnt = 0;

    if(uiShadeTimeMS < HW_TIMER_CYCLE_MS) {
        PR_NOTICE("start shade time less than the minimum setting");
        uiShadeTimeMS = HW_TIMER_CYCLE_MS;
    }

    HWTimerParam.uiTargetCnt = uiShadeTimeMS;
    HWTimerParam.bEnable = TRUE;
    tLightCtrlHandle.bFirstChange = TRUE;

    PR_DEBUG("start shade >>>");
}

/**
 * @brief: Light control shade change stop
 * @param {none}
 * @retval: none
 */
STATIC VOID vLightCtrlShadeStop(VOID)
{
    HWTimerParam.bEnable = FALSE;
    HWTimerParam.uiCnt = 0;
    HWTimerParam.uiTargetCnt = 0xFFFFFF;
}

/**
 * @brief: set shade change step gain
 * @param {IN UINT_T uiGain -> gain}
 * @retval: none
 */
STATIC VOID vLightCtrlShadeStepGainSet(IN UINT_T uiGain)
{
    uiStepGain = uiGain;
    if(uiStepGain < 1) {     /* avoid gain calc too small */
        uiStepGain = 1;
    }
}

/**
 * @brief: get shade change step gain
 * @param {none}
 * @retval: UINT_T uiStepGain
 */
STATIC UINT_T uiLightCtrlShadeStepGainGet(VOID)
{
    return (uiStepGain);
}

/**
 * @brief: cct delay 1s turn off
 * @param {none}
 * @attention: only active as CCT
 * @retval: none
 */
STATIC VOID vLightCtrlCCTShutDownTimeCB(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opUserSWTimerStop(CCT_DELAY_SHUT_DOWN_TIMER);   /* stop timer dominantly */
    if(opRet != LIGHT_OK) {
        PR_ERR("CCT delay timer stop error!");
    }

    if(tLightCtrlData.bSwitch) {     /* if turn on again ,don't active */
        return;
    }

    memset(&tLightCtrlHandle.tCurrVal, 0, SIZEOF(BRIGHT_DATA_T));
    /* when CCT shut down, need to turn off CCT */
    PR_DEBUG("CCT turn off!");
    opRet = opLightSetRGBCW(0, 0, 0, 0, 0);
    if(opRet != LIGHT_OK) {
        PR_ERR("shut down CCT error!");
    }
}

/**
 * @brief: get change gradually process the max error of 5ways
 *          this func will calc the error between LightCtrlHandle.Target
 *          and LightCtrlHandle.Current, and change.
 * @param {none}
 * @attention: this func need to called by period
 * @retval: none
 */
VOID vLightCtrlShadeGradually(VOID)
{
    OPERATE_LIGHT opRet = -1;

    INT_T iErrorRed = 0;
    INT_T iErrorGreen = 0;
    INT_T iErrorBlue = 0;
    INT_T iErrorWhite = 0;
    INT_T iErrorWarm = 0;

    USHORT_T usMaxError = 0;

    STATIC FLOAT_T fScaleRed = 0;
    STATIC FLOAT_T fScaleGreen = 0;
    STATIC FLOAT_T fScaleBlue = 0;
    STATIC FLOAT_T fScaleWhite = 0;
    STATIC FLOAT_T fScaleWarm = 0;

    UINT_T uiStepRed = 0;
    UINT_T uiStepGreen = 0;
    UINT_T uiStepBlue = 0;
    UINT_T uiStepWhite = 0;
    UINT_T uiStepWarm = 0;

    UINT_T uiGain = 0;
    UINT_T uiDelta = 0;

    uiGain = uiLightCtrlShadeStepGainGet();
    if( memcmp(&tLightCtrlHandle.tTargetVal, &tLightCtrlHandle.tCurrVal, SIZEOF(BRIGHT_DATA_T)) != 0 ) {  /* exist error */

        /* calulate the error between current value and target value */
        iErrorRed = tLightCtrlHandle.tTargetVal.usRed - tLightCtrlHandle.tCurrVal.usRed;
        iErrorGreen = tLightCtrlHandle.tTargetVal.usGreen - tLightCtrlHandle.tCurrVal.usGreen;
        iErrorBlue = tLightCtrlHandle.tTargetVal.usBlue - tLightCtrlHandle.tCurrVal.usBlue;
        iErrorWhite = tLightCtrlHandle.tTargetVal.usWhite - tLightCtrlHandle.tCurrVal.usWhite;
        iErrorWarm = tLightCtrlHandle.tTargetVal.usWarm - tLightCtrlHandle.tCurrVal.usWarm;

        usMaxError = usLightGetShadeMAX(&tLightCtrlHandle.tTargetVal, &tLightCtrlHandle.tCurrVal);

        if( TRUE == tLightCtrlHandle.bFirstChange ) {    /* calculate change scale */
            fScaleRed = uiLightToolGetABSValue(iErrorRed) / 1.0 / usMaxError;
            fScaleGreen = uiLightToolGetABSValue(iErrorGreen) / 1.0 / usMaxError;
            fScaleBlue = uiLightToolGetABSValue(iErrorBlue) / 1.0 / usMaxError;
            fScaleWhite = uiLightToolGetABSValue(iErrorWhite) / 1.0 / usMaxError;
            fScaleWarm = uiLightToolGetABSValue(iErrorWarm) / 1.0 / usMaxError;
            tLightCtrlHandle.bFirstChange = FALSE;
        }

        if( usMaxError == uiLightToolGetABSValue(iErrorRed) ) {
            uiStepRed = 1 * uiGain;
        } else {
            uiDelta = uiLightToolGetABSValue(iErrorRed) - usMaxError * fScaleRed + fScaleRed * uiGain;
            uiStepRed = ( uiDelta < uiGain) ? (uiDelta % uiGain) : uiGain;
        }

        if( usMaxError == uiLightToolGetABSValue(iErrorGreen) ) {
            uiStepGreen = 1 * uiGain;
        } else {
            uiDelta = uiLightToolGetABSValue(iErrorGreen) - usMaxError * fScaleGreen + fScaleGreen * uiGain;
            uiStepGreen = ( uiDelta < uiGain ) ? (uiDelta % uiGain)  : uiGain;
        }

        if( usMaxError == uiLightToolGetABSValue(iErrorBlue) ) {
            uiStepBlue = 1 * uiGain;
        } else {
            uiDelta = uiLightToolGetABSValue(iErrorBlue) - usMaxError *fScaleBlue + fScaleBlue * uiGain;
            uiStepBlue = ( uiDelta < uiGain) ? (uiDelta % uiGain)  : uiGain;
        }

        if( usMaxError == uiLightToolGetABSValue(iErrorWhite) ) {
            uiStepWhite = 1 * uiGain;
        } else {
            uiDelta = uiLightToolGetABSValue(iErrorWhite) - usMaxError *fScaleWhite + fScaleWhite * uiGain;
            uiStepWhite = ( uiDelta < uiGain) ? (uiDelta % uiGain)  : uiGain;
        }

        if( usMaxError == uiLightToolGetABSValue(iErrorWarm) ) {
            uiStepWarm = 1 * uiGain;
        } else {
            uiDelta = uiLightToolGetABSValue(iErrorWarm) - usMaxError *fScaleWarm + fScaleWarm * uiGain;
            uiStepWarm = ( uiDelta < uiGain ) ? (uiDelta % uiGain)  : uiGain;
        }

        if( iErrorRed != 0 ) {
            if( uiLightToolGetABSValue(iErrorRed) < uiStepRed ) {
                tLightCtrlHandle.tCurrVal.usRed += iErrorRed;
            } else {
                if( iErrorRed < 0 ) {
                    tLightCtrlHandle.tCurrVal.usRed -= uiStepRed;
                } else {
                    tLightCtrlHandle.tCurrVal.usRed += uiStepRed;
                }
            }
        }

        if( iErrorGreen != 0 ) {
            if( uiLightToolGetABSValue(iErrorGreen) < uiStepGreen ) {
                tLightCtrlHandle.tCurrVal.usGreen += iErrorGreen;
            } else {
                if( iErrorGreen < 0 ) {
                    tLightCtrlHandle.tCurrVal.usGreen -= uiStepGreen;
                } else {
                    tLightCtrlHandle.tCurrVal.usGreen += uiStepGreen;
                }
            }
        }

        if( iErrorBlue != 0 ) {
            if( uiLightToolGetABSValue(iErrorBlue) < uiStepBlue ) {
                tLightCtrlHandle.tCurrVal.usBlue += iErrorBlue;
            } else {
                if( iErrorBlue < 0 ) {
                    tLightCtrlHandle.tCurrVal.usBlue -= uiStepBlue;
                } else {
                    tLightCtrlHandle.tCurrVal.usBlue += uiStepBlue;
                }
            }
        }

        if( iErrorWhite != 0 ) {
            if( uiLightToolGetABSValue(iErrorWhite) < uiStepWhite ) {
                tLightCtrlHandle.tCurrVal.usWhite += iErrorWhite;
            } else {
                if( iErrorWhite < 0 ) {
                    tLightCtrlHandle.tCurrVal.usWhite -= uiStepWhite;
                } else {
                    tLightCtrlHandle.tCurrVal.usWhite += uiStepWhite;
                }
            }
        }

        if( iErrorWarm!= 0 ) {
            if( uiLightToolGetABSValue(iErrorWarm) < uiStepWarm ) {
                tLightCtrlHandle.tCurrVal.usWarm += iErrorWarm;
            } else {
                if( iErrorWarm < 0 ) {
                    tLightCtrlHandle.tCurrVal.usWarm -= uiStepWarm;
                } else {
                    tLightCtrlHandle.tCurrVal.usWarm += uiStepWarm;
                }
            }
        }

        if(HWTimerParam.bEnable != TRUE) {
            //PR_DEBUG("light shade can't not output set!");
            return;
        }
        opRet = opLightSetRGBCW(tLightCtrlHandle.tCurrVal.usRed, tLightCtrlHandle.tCurrVal.usGreen, tLightCtrlHandle.tCurrVal.usBlue, \
                                    tLightCtrlHandle.tCurrVal.usWhite, tLightCtrlHandle.tCurrVal.usWarm);
        if(opRet != LIGHT_OK) {
            //PR_ERR("light shade set RGBCW error %d!",opRet);
            return;
        }

    } else { /* no error between target and current */
        //PR_DEBUG("shade change no need, stop");
        vLightCtrlShadeStop();    /* just stop the change */

        if((BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) && (FALSE == tLightCtrlData.bSwitch)) {
            opRet = opUserSWTimerStart(CCT_DELAY_SHUT_DOWN_TIMER, 1000, vLightCtrlCCTShutDownTimeCB);
            if(opRet != LIGHT_OK) {
                PR_ERR("CCT shut down err!");
                /* when CCT shut down, need to turn off CCT */
                opRet = opLightSetRGBCW(0, 0, 0, 0, 0);
            }
        }
    }
}

/**
 * @brief: CW bright output limit process
 * @param {IN USHORT_T usBright -> bright value}
 * @retval: USHORT_T usResult
 */
STATIC USHORT_T usLightCtrlDataCWLimit(IN USHORT_T usBright)
{
    USHORT_T usMax = 0, usMin = 0, usResult = 0;

    usMax = (USHORT_T) (CTRL_CW_BRIGHT_VALUE_MAX * ( (FLOAT_T) tLightCfgData.ucLimitCWMax / CTRL_BRIGHT_LIMIT_MAX));
    usMin = (USHORT_T) (CTRL_CW_BRIGHT_VALUE_MAX * ( (FLOAT_T) tLightCfgData.ucLimitCWMin / CTRL_BRIGHT_LIMIT_MAX ));

    usResult = (USHORT_T)(( usBright - CTRL_CW_BRIGHT_VALUE_MIN ) * ( usMax - usMin ) / 1.0 / \
                ( CTRL_CW_BRIGHT_VALUE_MAX - CTRL_CW_BRIGHT_VALUE_MIN ) + usMin);

    return(usResult);
}

/**
 * @brief: calculate the CW Light_Handle.TargetVal according to
 *          the Light_Data value, and cw output limit and amplify process!
 * @param {IN USHORT_T usBright -> Light_Data bright }
 * @param {IN USHORT_T usTemperature -> Light_Data temperature}
 * @param {IN BRIGHT_DATA_T *pResult -> calc result}
 * @retval: none
 */
STATIC VOID vLightCtrlDataCalcCW(IN USHORT_T usBright, IN USHORT_T usTemperatue, OUT BRIGHT_DATA_T *pResult)
{
    USHORT_T usBrightTemp = 0;

    PR_DEBUG("Bright %d", usBright);

    if(usBright < CTRL_CW_BRIGHT_VALUE_MIN) {
        usBrightTemp = 0;
    } else {
        PR_DEBUG("cw limit proc");
        usBrightTemp = usLightCtrlDataCWLimit(usBright);     /* limit CW bright output limit */
    }
    PR_DEBUG("usBrightTmep %d", usBrightTemp);

    if(BRIGHT_MODE_CW == tLightCfgData.eBrightMode) {
        usBrightTemp = (USHORT_T) (((FLOAT_T)( tLightCfgData.ucPowerGain / CTRL_POEWER_GAIN_MIN )) * usBrightTemp );     /* power amplification */

        PR_DEBUG("usBrightTmep %d", usBrightTemp);

        pResult ->usWhite = (USHORT_T) (usBrightTemp * ((FLOAT_T)usTemperatue / CTRL_CAL_VALUE_RANGE));
        pResult ->usWarm = usBrightTemp - pResult ->usWhite;

        /* output max limit --  power amplification, convert overflow power to another  */
        if(pResult ->usWhite > CTRL_CAL_VALUE_RANGE) {
            pResult ->usWhite = CTRL_CAL_VALUE_RANGE;
        }
        if( pResult ->usWarm > CTRL_CAL_VALUE_RANGE ) {
            pResult ->usWarm = CTRL_CAL_VALUE_RANGE;
        }
    } else if ( BRIGHT_MODE_CCT == tLightCfgData.eBrightMode ) {
        pResult ->usWhite = usBrightTemp;
        pResult ->usWarm = usTemperatue;
    }

}

/**
 * @brief: calculate HSV limit v.
 * @param {IN USHORT_T usValue -> HSV, value}
 * @retval: limit adjust value
 */
STATIC USHORT_T usLightCtrlDataCalcHSVLimit(IN USHORT_T usValue)
{
    USHORT_T usLimitMin = 10;
    USHORT_T usLimitMax = 1000;

    if(usValue < CTRL_RGB_BRIGHT_VALUE_MIN) {
        return 0;       /* value is illegal, HSV v must be 0 */
    }

    usLimitMin = (USHORT_T)(CTRL_RGB_BRIGHT_VALUE_MAX * ( (FLOAT_T) tLightCfgData.ucLimitRGBMin / 100.0 ));       /* RGB scene data color limit */
    usLimitMax = (USHORT_T)(CTRL_RGB_BRIGHT_VALUE_MAX * ( (FLOAT_T) tLightCfgData.ucLimitRGBMax / 100.0 ));

    return (USHORT_T)((usValue - CTRL_RGB_BRIGHT_VALUE_MIN) * (usLimitMax - usLimitMin) / (CTRL_RGB_BRIGHT_VALUE_MAX - CTRL_RGB_BRIGHT_VALUE_MIN) + usLimitMin);
}

/**
 * @brief: calculate RGB gamma,format:0~1000
 * @param {IN CTRL_DEF_COLOR_E  eColor -> R\G\B}
 * @param {IN USHORT_T usGamma -> before gamma param}
 * @attention:
 * @retval: none
 */
STATIC USHORT_T usLightCtrlDataCalcGamma(IN CTRL_DEF_COLOR_E  eColor, IN USHORT_T usGamma)
{
#if (LIGHT_CFG_ENABLE_GAMMA == 1)
    USHORT_T usResult;
    UCHAR_T ucIndex;

    usResult = usGamma;
    ucIndex = (UCHAR_T)(usGamma * 1.0 / CTRL_CAL_VALUE_RANGE * 255);

    switch (eColor)
    {
        case DEF_COLOR_R:
            usResult = (USHORT_T)(ucDeviceCfgGetGammaRed(ucIndex) * CTRL_CAL_VALUE_RANGE / 255.0);
            break;

        case DEF_COLOR_G:
            usResult = (USHORT_T)(ucDeviceCfgGetGammaGreen(ucIndex) * CTRL_CAL_VALUE_RANGE / 255.0);
            break;

        case DEF_COLOR_B:
            usResult = (USHORT_T)(ucDeviceCfgGetGammaBlue(ucIndex) * CTRL_CAL_VALUE_RANGE / 255.0);
            break;

        case DEF_COLOR_C:
        case DEF_COLOR_W:
        default:
            break;
    }

    return usResult;
#else
    PR_DEBUG("gamma adjust don't active");
    return usGamma;
#endif
}

/**
 * @brief: calculate the RGB tLight_Handle.tTargetVal according to
 *          the tLight_Data value.
 * @param {IN COLOR_RGB_T *pColor -> RGB ctrl data}
 * @param {IN BRIGHT_DATA_T *pResult -> Result handle data}
 * @attention: gamma adjust proc
 * @retval: none
 */
STATIC VOID vLightCtrlDataCalcRGB(IN COLOR_RGB_T *pColor, OUT BRIGHT_DATA_T *pResult)
{

    PR_DEBUG("before gamma adjust RGB %d %d %d", pColor ->usRed, pColor ->usGreen, pColor ->usBlue);

    pResult ->usRed = usLightCtrlDataCalcGamma(DEF_COLOR_R, pColor ->usRed);

    pResult ->usGreen = usLightCtrlDataCalcGamma(DEF_COLOR_G, pColor ->usGreen);

    pResult ->usBlue = usLightCtrlDataCalcGamma(DEF_COLOR_B, pColor ->usBlue);

    PR_DEBUG(" After gamma adjust RGB %d %d %d", pResult ->usRed, pResult ->usGreen, pResult ->usBlue);

}

/**
 * @brief: Calc RGBCW handle data
 * @param {IN LIGHT_MODE_E Mode -> current mode}
 * @param {IN LIGHT_CTRL_DATA_T *CtrlData -> current ctrl data}
 * @param {OUT BRIGHT_DATA_T *Result -> output handle data}
 * @attention: support white and color mode only
 * @retval: none
 */
STATIC VOID vLightCtrlDataCalcRGBCW(IN LIGHT_MODE_E Mode, IN LIGHT_CTRL_DATA_T *CtrlData, OUT BRIGHT_DATA_T *Result)
{
    STATIC LIGHT_MODE_E LastMode = MODE_MAX ;

    memset(Result, 0, SIZEOF(BRIGHT_DATA_T));

    switch(Mode) {
        case WHITE_MODE:
            vLightCtrlDataCalcCW(CtrlData ->usBright, CtrlData ->usTemper, Result);
            PR_DEBUG(" set bright %d temper %d,calc ctrl data %d %d %d %d %d", CtrlData ->usBright, CtrlData ->usTemper,\
                Result ->usRed, Result->usGreen, Result ->usBlue, Result ->usWhite, Result ->usWarm);
            if((BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) && (LastMode != Mode)) {
                tLightCtrlHandle.tCurrVal.usWarm = Result ->usWarm;
            }
            break;

        case COLOR_MODE:
            if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {
            /* to aviod the CCT temperature change, when CCT set as white mode, make sure color mode not change CCT! */
                PR_DEBUG("current warm %d", tLightCtrlHandle.tCurrVal.usWarm);
                PR_DEBUG("target warm %d", tLightCtrlHandle.tTargetVal.usWarm);
                Result ->usWarm = tLightCtrlHandle.tCurrVal.usWarm;
            }
            vLightCtrlDataCalcRGB(&(CtrlData ->tColor), Result);
            PR_DEBUG(" set bright %d temper %d,calc ctrl data %d %d %d %d %d", CtrlData ->usBright, CtrlData ->usTemper,\
                Result ->usRed, Result->usGreen, Result ->usBlue, Result ->usWhite, Result ->usWarm);
            break;

        case SCENE_MODE:
            vLightCtrlDataCalcRGB(&(CtrlData ->tColor), Result);
            vLightCtrlDataCalcCW(CtrlData ->usBright, CtrlData ->usTemper, Result);
            PR_DEBUG("mode is scene mode!");

            PR_DEBUG(" set bright %d temper %d,calc ctrl data %d %d %d %d %d", CtrlData ->usBright, CtrlData ->usTemper,\
                Result ->usRed, Result->usGreen, Result ->usBlue, Result ->usWhite, Result ->usWarm);
            break;

        case MUSIC_MODE:
            vLightCtrlDataCalcRGB(&(CtrlData ->tColor), Result);
            PR_DEBUG("mode is music mode!");
            PR_DEBUG(" set bright %d temper %d,calc ctrl data %d %d %d %d %d", CtrlData ->usBright, CtrlData ->usTemper,\
                Result ->usRed, Result->usGreen, Result ->usBlue, Result ->usWhite, Result ->usWarm);
            if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {
                Result ->usWarm = tLightCtrlHandle.tCurrVal.usWarm;
            }
            break;
        default:
            break;
    }
    LastMode = Mode;
}

/**
 * @brief: light HSVBT data analysize
 * @param {IN CHAR_T *pHSVBT -> HSVBT string }
 * @param {OUT *pValH -> Hue}
 * @param {OUT *pValS -> Sat}
 * @param {OUT *pValV -> Value}
 * @param {OUT *pValB -> Brightness}
 * @param {OUT *pValT -> Temperature}
 * @retval: none
 */
STATIC VOID vLightHSVBTAnalysize(IN CHAR_T *pHSVBT, OUT USHORT_T *pValH, OUT USHORT_T *pValS, OUT USHORT_T *pValV, OUT USHORT_T *pValB, OUT USHORT_T *pValT)
{
    UCHAR_T i = 0;
    USHORT_T usTempBuf[5] = {0};

    for(i = 0; i < 5; i++) {
        usTempBuf[i] = usLightToolSTR2USHORT( ucLightToolASC2Hex(*(pHSVBT + 4*i + 0)), ucLightToolASC2Hex(*(pHSVBT + 4*i + 1)),\
                                      ucLightToolASC2Hex(*(pHSVBT + 4*i + 2)), ucLightToolASC2Hex(*(pHSVBT + 4*i + 3)) );
    }

    PR_DEBUG("HSVBT %d %d %d %d", usTempBuf[0],usTempBuf[1],usTempBuf[2],usTempBuf[3],usTempBuf[4]);

    *pValH = usTempBuf[0];
    *pValS = usTempBuf[1];
    *pValV = usTempBuf[2];
    *pValB = usTempBuf[3];
    *pValT = usTempBuf[4];
}


/**
 * @brief: Calc Light scene handle data
 * @param {IN UCHAR_T *pSceneData -> scene data string }
 * @param {IN LIGHT_SCENE_CTRL_T *pCtrlParam -> scene ctr param(change mode, time, speed)}
 * @param {OUT BRIGHT_DATA_T *pResult -> result(already changed to RGBCW data)}
 * @attention: times & speed unit:ms
 * @retval: none
 */
STATIC VOID vLightCtrlDataCalcScene(IN CHAR_T *pSceneData, OUT LIGHT_SCENE_CTRL_T *pCtrlParam, OUT BRIGHT_DATA_T *pResult)
{
    USHORT_T usBrightness = 0, usTempature = 0;
    USHORT_T usValH = 0, usValS = 0, usValV = 0;
    USHORT_T usValR = 0, usValG = 0, usValB = 0;
    COLOR_RGB_T tRGBValue;


    /* APP setting in constrast ,APP data is rang from 40~ 100 */
    /* attention:(105 - x) is attentin to make when the APP send the fastest speed 100, ucTimes isn't equal to zeoro! */
    /* attention: APP send the times is equal to speed set ! */
    /* attention: times and speed unit:ms ! */
    pCtrlParam ->ChangeMode = ucLightToolSTR2UCHAR( ucLightToolASC2Hex(pSceneData[4]), ucLightToolASC2Hex(pSceneData[5]) );

    if(pCtrlParam ->ChangeMode != SCENE_SHADE) {
        pCtrlParam ->uiTimes = (105 - ucLightToolSTR2UCHAR( ucLightToolASC2Hex(pSceneData[0]), ucLightToolASC2Hex(pSceneData[1]))) * 100;
        pCtrlParam ->uiSpeed = (105 - ucLightToolSTR2UCHAR( ucLightToolASC2Hex(pSceneData[2]), ucLightToolASC2Hex(pSceneData[3]))) * 100;
    } else {
        pCtrlParam ->uiTimes = (110 - ucLightToolSTR2UCHAR( ucLightToolASC2Hex(pSceneData[0]), ucLightToolASC2Hex(pSceneData[1]))) * 100;
        pCtrlParam ->uiSpeed = pCtrlParam ->uiTimes / 2;          /* scene change speed is time / 2 */
    }


    PR_DEBUG("Speed is %d, time is %d", pCtrlParam ->uiSpeed , pCtrlParam ->uiTimes);

    vLightHSVBTAnalysize((pSceneData + 6), &usValH, &usValS, &usValV, &usBrightness, &usTempature);

    usValV = usLightCtrlDataCalcHSVLimit(usValV);
    vLightToolHSV2RGB(usValH, usValS, usValV, &usValR, &usValG, &usValB);

    tRGBValue.usRed = usValR;
    tRGBValue.usGreen = usValG;
    tRGBValue.usBlue = usValB;
    vLightCtrlDataCalcRGB(&tRGBValue, pResult);

    vLightCtrlDataCalcCW(usBrightness, usTempature, pResult);

}

/**
 * @brief: reponse switch property process,
 *          this need to implement by system.
 * @param {OUT BOOL_T bONOFF -> switch status, TRUE is ON}
 * @retval: none
 */
WEAK VOID vLightCtrlDataSwitchRespone(OUT BOOL_T bONOFF)
{
    ;
}

/**
 * @brief: set light switch data, adapte control data issued by system
 *          to control data format.
 * @param {IN BOOL_T bONOFF -> switch data, TRUE will turn on}
 * @retval: OPERATE_LIGHT -> LIGHT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataSwitchSet(IN BOOL_T bONOFF)
{
    BOOL_T bLastStatus ;

    bLastStatus = tLightCtrlData.bSwitch;

    if( TRUE == bONOFF ) {
        tLightCtrlData.bSwitch = TRUE;
        if(SCENE_MODE == tLightCtrlData.eMode) {              /* restart scene mode */
            tLightCtrlData.bSceneFirstSet = TRUE;
        }
    } else {
        tLightCtrlData.bSwitch = FALSE;
    }

    vLightCtrlDataSwitchRespone(bONOFF);      /* reponse property */

    if(bONOFF == bLastStatus) {
        PR_DEBUG("the same switch set");
        return LIGHT_INVALID_PARM;
    }

    return LIGHT_OK;
}

/**
 * @brief: reponse mode property process,
 *          this need to implement by system.
 * @param {OUT LIGHT_MODE_E Mode}
 * @retval: none
 */
WEAK VOID vLightCtrlDataModeResponse(OUT LIGHT_MODE_E eMode)
{
    ;
}

/**
 * @brief: set light mode data
 * @param {IN LIGHT_MODE_E eMode}
 * @attention: Mode value is below:
 *                                  WHITE_MODE = 0,
 *                                  COLOR_MODE = 1,
 *                                  SCENE_MODE = 2,
 *                                  MUSIC_MODE = 3,
 * @retval: OPERATE_LIGHT -> LIGHT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataModeSet(IN LIGHT_MODE_E eMode)
{
    OPERATE_LIGHT opRet = -1;
    LIGHT_MODE_E eLastMode ;

    eLastMode = tLightCtrlData.eMode;

    /* mode set need limit when light way is RGB, or CW or C */
    if( ((LIGHT_RGB == tLightCfgData.eLightWay) && (WHITE_MODE == eMode)) \
        ||((tLightCfgData.eLightWay <= LIGHT_CW) && (COLOR_MODE == eMode)) \
        || (eMode > MODE_MAX)) {

        PR_ERR("mode is illegal,set error");
        return LIGHT_INVALID_PARM;
    }

    tLightCtrlData.eMode = eMode;
    vLightCtrlDataModeResponse(eMode);

    if((eMode == eLastMode) && (eMode != SCENE_MODE)) {      /* only scene mode can be set many time */
        PR_DEBUG("the same mode set");
        return LIGHT_INVALID_PARM;
    }

    /* don't need to proc, as the adjusted new scene control data will issued later  */
    if((eMode == SCENE_MODE) && (tLightCtrlData.ucRealTimeFlag == TRUE)) {
        PR_DEBUG("this scene mode don't need proc!");

        opRet = opUserSWTimerStart(SCENE_AUTO_RESTART_TIMER, 1500, vLightCtrlSceneAutoRestartTimeCB);
        if( opRet != LIGHT_OK) {
            PR_ERR("scene auto restart timer init error!");
        }
        return LIGHT_INVALID_PARM;
    }

    if(SCENE_MODE == tLightCtrlData.eMode) {              /* restart scene mode */
        tLightCtrlData.bSceneFirstSet = TRUE;
    }

    return LIGHT_OK;
}

/**
 * @brief: reponse bright property process,
 *          this need to implement by system.
 * @param {OUT LIGHT_MODE_E eMode}
 * @param {OUT USHORT_T usBright}
 * @attention: need reponse mode property,as set bright value, will auto set the Mode to WHITE_MODE!
 * @retval: none
 */
WEAK VOID vLightCtrlDataBrightResponse(OUT LIGHT_MODE_E eMode, OUT USHORT_T usBright)
{
    ;
}

/**
 * @brief: set light bright data, adapte control data issued by system
 *          to control data format.
 * @param {IN USHORT_T usBright}
 * @attention: acceptable range:10~1000
 * @attention: set bright value, will auto set the Mode to WHITE_MODE !
 * @retval: OPERATE_LIGHT -> LIGHT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataBrightSet(IN USHORT_T usBright)
{

    if(((usBright < CTRL_CW_BRIGHT_VALUE_MIN) || (usBright > CTRL_CW_BRIGHT_VALUE_MAX)) \
        || (LIGHT_RGB == tLightCfgData.eLightWay)){
        PR_ERR("bright value is exceed range,set error");
        return LIGHT_INVALID_PARM;
    }

    tLightCtrlData.usBright = usBright;
    tLightCtrlData.eMode = WHITE_MODE;    /* change mode to white mode forcibly */

    vLightCtrlDataBrightResponse(tLightCtrlData.eMode, usBright);

    return LIGHT_OK;
}

/**
 * @brief: reponse temperature property process,
 *          this need to implement by system.
 * @param {OUT LIGHT_MODE_E eMode}
 * @param {OUT USHORT_T usTemperature}
 * @attention: need reponse mode property,as set temperature value, will auto set the Mode to WHITE_MODE!
 * @retval: none
 */
WEAK VOID vLightCtrlDataTemperatureResponse(OUT LIGHT_MODE_E eMode, OUT USHORT_T usTemperature)
{
    ;
}

/**
 * @brief: set light temrperature data, adapte control data issued by system
 *          to control data format.
 * @param {IN USHORT_T usTemperature}
 * @attention: acceptable range:0~1000
 * @retval: OPERATE_LIGHT -> LIGHT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataTemperatureSet(IN USHORT_T usTemperature)
{

    if((usTemperature > CTRL_CW_BRIGHT_VALUE_MAX) \
        || (LIGHT_RGB == tLightCfgData.eLightWay)) {
        PR_ERR("temperature value is exceed range,set error");
        return LIGHT_INVALID_PARM;
    }

    tLightCtrlData.usTemper = usTemperature;
    tLightCtrlData.eMode = WHITE_MODE;    /* change mode to white mode forcibly */

    vLightCtrlDataTemperatureResponse(tLightCtrlData.eMode, usTemperature);

    return LIGHT_OK;
}

/**
 * @brief: reponse RGB property process,
 *          this need to implement by system
 * @param {OUT COLOR_ORIGIN_T *ptColorOrigin}
 * @retval: none
 */
WEAK VOID vLightCtrlDataRGBResponse(OUT COLOR_ORIGIN_T *ptColorOrigin)
{
    ;
}

/**
 * @brief: set light RGB data
 * @param {IN COLOR_RGB_T *ptColor}
 * @param {IN COLOR_ORIGINAL_T *ptColorOrigin -> color origin data save}
 * @attention: acceptable format is RGB module, R,G,B range：0~1000
 * @retval: OPERATE_LIGHT -> LIGHT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataRGBSet(IN COLOR_RGB_T *ptColor, IN COLOR_ORIGIN_T *ptColorOrigin)
{

    if((ptColor ->usRed > CTRL_CAL_VALUE_RANGE) ||
        (ptColor ->usGreen > CTRL_CAL_VALUE_RANGE) ||
        (ptColor ->usBlue > CTRL_CAL_VALUE_RANGE)) {
        PR_ERR("RGB color value is exceed range, set error!");
        return LIGHT_INVALID_PARM;
    }

    if(tLightCfgData.eLightWay <= LIGHT_CW) {
        return LIGHT_INVALID_PARM;
    }

    /* to avoid the situation:adjust color data ver fast and turn off the light with remote cmd,
    as exist realtime control dp, the ightCtrlData.Color will equal to the control data */
    if(ptColorOrigin != NULL) {
        memcpy(&tLightCtrlData.tColorOrigin, ptColorOrigin, SIZEOF(COLOR_ORIGIN_T));    /* must save the origin data */
    }

    memcpy(&tLightCtrlData.tColor, ptColor, SIZEOF(COLOR_RGB_T));

    vLightCtrlDataRGBResponse(ptColorOrigin);

    return LIGHT_OK;
}

/**
 * @brief: reponse scene property process,
 *          this need to implement by system
 * @param {OUT CHAR_T *pSceneData}
 * @retval: none
 */
WEAK VOID vLightCtrlDataSceneResponse(OUT CHAR_T *pSceneData)
{
    ;
}

/**
 * @brief: set light scene data
 * @param {IN UCHAR_T *pSceneData}
 * @attention: SceneData format: please reference to DP protocol
 * @retval: none
 */
OPERATE_LIGHT opLightCtrlDataSceneSet(IN CHAR_T *pSceneData)
{
    CHAR_T cLastSceneData[SCENE_MAX_LENGTH + 1] = {0};

    strcpy(cLastSceneData, (CHAR_T *)&tLightCtrlData.cScene);

    if((strlen(pSceneData) < SCENE_MIN_LENGTH) || (strlen(pSceneData) > SCENE_MAX_LENGTH)) {
        PR_ERR("Scene data is error! please chek!");
        return LIGHT_INVALID_PARM;
    }

    strcpy((CHAR_T*)&tLightCtrlData.cScene, pSceneData);
    PR_DEBUG("light ctrl scene buf %s", tLightCtrlData.cScene);

    vLightCtrlDataSceneResponse(pSceneData);

    if(TRUE == tLightCtrlData.ucRealTimeFlag) {
        PR_DEBUG("this commad need to save and proc!");
        tLightCtrlData.ucRealTimeFlag = FALSE;
    } else {
        if(strcmp(pSceneData, cLastSceneData) == 0) {
            PR_DEBUG("the same scene set");
            return LIGHT_INVALID_PARM;
        }
    }
    tLightCtrlData.bSceneFirstSet = TRUE;

    return LIGHT_OK;
}

/**
 * @brief: get light switch data
 * @param {OUT BOOL_T *pONOFF -> switch data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataSwitchGet(OUT BOOL_T *pONOFF)
{
    if(NULL == pONOFF) {
        return LIGHT_INVALID_PARM;
    }

    *pONOFF = tLightCtrlData.bSwitch;

    return LIGHT_OK;
}

/**
 * @berief: get light time Count Down data
 * @param {OUT UINT_T *pCountDown -> time count down value}
 * @retval: OPERATE_RET
 */
OPERATE_RET opLightCtrlDataCountDownGet(OUT UINT_T *pCountDown)
{
	*pCountDown = tLightCtrlData.uiCountDown;

	return OPRT_OK;
}

/**
 * @brief: get light mode data
 * @param {OUT LIGHT_MODE_E *pMode -> mode data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataModeGet(OUT LIGHT_MODE_E *pMode)
{
    if(NULL == pMode) {
        return LIGHT_INVALID_PARM;
    }

    *pMode = tLightCtrlData.eMode;

    return LIGHT_OK;
}

/**
 * @brief: geta light bright data
 * @param {OUT USHORT_T *pBright -> bright data return}
 * @retval: OOPERATE_RET
 */
OPERATE_LIGHT opLightCtrlDataBrightGet(OUT USHORT_T *pBright)
{
    if(NULL == pBright) {
        return LIGHT_INVALID_PARM;
    }

    if(tLightCfgData.eLightWay == LIGHT_RGB) {
        return LIGHT_COM_ERROR;
    }

    *pBright = tLightCtrlData.usBright;

    return LIGHT_OK;
}

/**
 * @brief: get light temrperature data
 * @param {OUT USHORT_T *pTemperature -> temperature data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataTemperatureGet(OUT USHORT_T *pTemperature)
{
    if(NULL == pTemperature) {
        return LIGHT_INVALID_PARM;
    }

    if((tLightCfgData.eLightWay != LIGHT_RGBCW) && (tLightCfgData.eLightWay != LIGHT_CW)) {
        return LIGHT_COM_ERROR;
    }

    *pTemperature = tLightCtrlData.usTemper;

    return LIGHT_OK;
}

/**
 * @brief: get light RGB data & original data
 * @param {OUT COLOR_ORIGIN_T *ptOriginalColor -> color original data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataRGBGet(OUT COLOR_ORIGIN_T *ptOriginalColor)
{
    if(NULL == ptOriginalColor) {
        return LIGHT_INVALID_PARM;
    }

    if(tLightCfgData.eLightWay < LIGHT_RGB) {
        return LIGHT_COM_ERROR;
    }

    PR_DEBUG("color original %s", tLightCtrlData.tColorOrigin.ucColorStr);
    memcpy(ptOriginalColor, &tLightCtrlData.tColorOrigin, SIZEOF(COLOR_ORIGIN_T));

    return LIGHT_OK;
}

/**
 * @brief: get light scene data
 * @param {OUT CHAR_T *pSceneData -> scene data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataSceneGet(OUT CHAR_T *pSceneData)
{
    if(NULL == pSceneData) {
        return LIGHT_INVALID_PARM;
    }

    PR_DEBUG("str len %d", strlen(tLightCtrlData.cScene));

    strcpy(pSceneData, (CHAR_T*)&tLightCtrlData.cScene);

    return LIGHT_OK;
}

/**
 * @brief: scene change proc
 *          get scene ctrl data and change to RGBCW value form,
 *          start to scene unitX(X:1~8);
 * @param {OUT BOOL_T *pCycleEnable -> need to cycle}
 * @param {OUT UINT_T *pCycleTime_ms -> unit change time, ms}
 * @attention: this func need to called period, period time is transfered by pCycletime,
 *              when pCycleEnable flag equal to TRUE;
 * @retval: OPERATE_LIGHT
 */
STATIC OPERATE_LIGHT opLightCtrlSceneChange(OUT BOOL_T *pCycleEnable, OUT UINT_T *pCycleTimeMs)
{
    OPERATE_LIGHT opRet = -1;
    LIGHT_SCENE_CTRL_T tLightSceneCtrlParm;
    UINT_T uiGainTemp = 0;
    STATIC USHORT_T usCCTWarm = 0xFFFF;         /* save CCT mode, warm set! */
    STATIC UCHAR_T ucCCTLastRGBFlag = FALSE;
    UCHAR_T ucColor2SceneModeFlag = FALSE;
    UCHAR_T ucWhite2SceneModeFlag = FALSE;

    if(tLightCtrlHandle.bSceneStopFlag) {        /* don't permit to change! */
        PR_NOTICE("Scene don't allow to change!");
        *pCycleEnable = FALSE;
        if(tLightCtrlData.bSwitch == FALSE) {        /* to avoid can't shut down!  */
            opRet = opLightCtrlProc();
            PR_DEBUG("scene shut down again!");
            if(opRet != LIGHT_OK) {
                PR_ERR("scene shut down again error!");
            }
        }
        return LIGHT_OK;
    }

    *pCycleEnable = TRUE;

    PR_DEBUG("firstly stop shade !");
    vLightCtrlShadeStop();  /* stop other shade firstly */

    memset(&tLightSceneCtrlParm, 0, SIZEOF(LIGHT_SCENE_CTRL_T));

    tLightCtrlHandle.ucSceneUnit = strlen(tLightCtrlData.cScene + 2) / SCENE_UNIT_LENGTH;

    PR_DEBUG("Scene total uint %d  current uint %d",tLightCtrlHandle.ucSceneUnit, tLightCtrlHandle.ucUnitCnt );

    PR_DEBUG("before RGBCW: %d %d %d %d %d",tLightCtrlHandle.tTargetVal.usRed,tLightCtrlHandle.tTargetVal.usGreen,tLightCtrlHandle.tTargetVal.usBlue,\
                    tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);

    /* attention!!!! */
    tLightCtrlHandle.bSceneSetFirstFlag = tLightCtrlData.bSceneFirstSet;
    if(tLightCtrlHandle.bSceneSetFirstFlag) {
        usCCTWarm = 0xFFFF;                 /* when scene data is new, clear CCT warm set! */
        tLightCtrlHandle.ucUnitCnt = 0;
		/* color mode or music mode change to scene! */
        if((tLightCtrlHandle.tCurrVal.usRed != 0) || ( tLightCtrlHandle.tCurrVal.usGreen != 0) || (tLightCtrlHandle.tCurrVal.usBlue != 0)) {
            PR_DEBUG("color mode or music mode change to scene mode!");
            ucColor2SceneModeFlag = TRUE;
        }
		/* white mode change to scene! */
		if((tLightCtrlHandle.tCurrVal.usWhite != 0) ||(tLightCtrlHandle.tCurrVal.usWarm != 0)) {
            PR_DEBUG("color mode or music mode change to scene mode!");
            ucWhite2SceneModeFlag = TRUE;
        }
    }
    tLightCtrlData.bSceneFirstSet = FALSE;
    tLightCtrlHandle.bSceneSetFirstFlag = FALSE;     /* attention:scene first set，don't display the first unit!!! */

    vLightCtrlDataCalcScene(tLightCtrlData.cScene + tLightCtrlHandle.ucUnitCnt * SCENE_UNIT_LENGTH + SCNE_HRAD_LENGTH, \
                                &tLightSceneCtrlParm, &tLightCtrlHandle.tTargetVal);

    if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {        /* CCT drive mode make sure don't change warm */
        if(tLightCtrlHandle.tTargetVal.usWhite != 0) {        /* unit data is white led set cfg */
            if((ucCCTLastRGBFlag) || (ucColor2SceneModeFlag)) {
            /* last unit is RGB data, this unit is white data */
                tLightCtrlHandle.tCurrVal.usWarm = tLightCtrlHandle.tTargetVal.usWarm;
                /* set the warm set when unit firstly set as white led */
                ucColor2SceneModeFlag = FALSE;
            }
            usCCTWarm = tLightCtrlHandle.tTargetVal.usWarm;
            ucCCTLastRGBFlag = FALSE;
        } else {
            if(usCCTWarm != 0XFFFF) {       /* when RGB... + C or RGB... + W or RGB... + C + W */
                tLightCtrlHandle.tTargetVal.usWarm = usCCTWarm;
            }

			if(ucWhite2SceneModeFlag) {     /* white mode chang to scene, and first unit is color!  */
                tLightCtrlHandle.tTargetVal.usWarm = tLightCtrlHandle.tCurrVal.usWarm;
                usCCTWarm = tLightCtrlHandle.tTargetVal.usWarm;
                ucWhite2SceneModeFlag = FALSE;
            }
            ucCCTLastRGBFlag = TRUE;        /* last unit data is RGB */
        }
    }

    PR_DEBUG("before RGBCW: %d %d %d %d %d",tLightCtrlHandle.tCurrVal.usRed,tLightCtrlHandle.tCurrVal.usGreen,tLightCtrlHandle.tCurrVal.usBlue,\
                        tLightCtrlHandle.tCurrVal.usWhite, tLightCtrlHandle.tCurrVal.usWarm);

    PR_DEBUG("after RGBCW: %d %d %d %d %d",tLightCtrlHandle.tTargetVal.usRed,tLightCtrlHandle.tTargetVal.usGreen,tLightCtrlHandle.tTargetVal.usBlue,\
                    tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);

    *pCycleTimeMs = tLightSceneCtrlParm.uiTimes;

    tLightCtrlHandle.ucUnitCnt++;        /* @attention:turn on/off on APP,will change the unit num++! */
    if(tLightCtrlHandle.ucUnitCnt >= tLightCtrlHandle.ucSceneUnit ) {
        tLightCtrlHandle.ucUnitCnt = 0;
    }

    PR_DEBUG("scene mode %d",tLightSceneCtrlParm.ChangeMode);
    switch(tLightSceneCtrlParm.ChangeMode) {
        case SCENE_STATIC:
            vLightCtrlShadeStepGainSet(SHADE_STEP_GAIN_DEFAULT);
            *pCycleEnable = FALSE;   /* only one unit,do't need cycle */
            vLightCtrlShadeStart(LIGHT_SHADE_CYCLE);
            break;

        case SCENE_JUMP:
            memcpy(&tLightCtrlHandle.tCurrVal, &tLightCtrlHandle.tTargetVal, SIZEOF(tLightCtrlHandle.tTargetVal));
            opRet = opLightSetRGBCW(tLightCtrlHandle.tCurrVal.usRed, tLightCtrlHandle.tCurrVal.usGreen, tLightCtrlHandle.tCurrVal.usBlue, \
                                        tLightCtrlHandle.tCurrVal.usWhite, tLightCtrlHandle.tCurrVal.usWarm);
            if(opRet != LIGHT_OK) {
                PR_ERR("Light ctrl turn on set RGBCW error!");
                return opRet;
            }
            break;

        case SCENE_SHADE:
            uiGainTemp = (UINT_T)(CTRL_CAL_VALUE_RANGE * LIGHT_SHADE_CYCLE / 1.0 / tLightSceneCtrlParm.uiSpeed + 0.5);
            vLightCtrlShadeStepGainSet(uiGainTemp);
            if(tLightCtrlHandle.bSceneSetFirstFlag == TRUE) {
                PR_DEBUG("send first unit firstly!");
                memcpy(&tLightCtrlHandle.tCurrVal, &tLightCtrlHandle.tTargetVal, SIZEOF(tLightCtrlHandle.tTargetVal));
                opRet = opLightSetRGBCW(tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, tLightCtrlHandle.tTargetVal.usBlue, \
                                            tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);
                if(opRet != LIGHT_OK) {
                    PR_ERR("Light ctrl turn on set RGBCW error!");
                    return opRet;
                }
                tLightCtrlHandle.bSceneSetFirstFlag = FALSE;
            } else {
                PR_DEBUG("send second unit continue!");
                vLightCtrlShadeStart(LIGHT_SHADE_CYCLE);
            }
            break;

        default:
            PR_ERR("Scene change mode err!");
            break;
    }

    return LIGHT_OK;
}

/**
 * @brief: Light scene change timer callback
 * @param {none}
 * @retval: none
 */
STATIC VOID vLightCtrlSceneChangeTimerCB(VOID)
{
    OPERATE_LIGHT opRet = -1;
    UINT_T uiCycleTime = 0;
    BOOL_T bEableFlag = 0;

    opRet = opLightCtrlSceneChange(&bEableFlag, &uiCycleTime);
    if(opRet != LIGHT_OK) {
        PR_ERR("Scene change err!");
    }

    if(bEableFlag == FALSE) {
        PR_DEBUG("No need next scene change!");
        opRet = opUserSWTimerStop(SCENE_SW_TIMER);      /* stop timer dominantly */
        if(opRet != LIGHT_OK) {
            PR_ERR("stop scene timer error!");
        }
    }

    if(TRUE == bEableFlag) {
        opRet = opUserSWTimerStart(SCENE_SW_TIMER, uiCycleTime, (VOID*)vLightCtrlSceneChangeTimerCB);
        if(opRet != LIGHT_OK) {
            PR_ERR("Scene timer start error!");
        }
    }

    PR_DEBUG("current %d %d %d %d %d", tLightCtrlHandle.tCurrVal.usRed, tLightCtrlHandle.tCurrVal.usGreen, \
                tLightCtrlHandle.tCurrVal.usBlue,tLightCtrlHandle.tCurrVal.usWhite, tLightCtrlHandle.tCurrVal.usWarm);
}

/**
 * @brief: Light scenen change start
 * @param {IN UINT_T uiChangeTimeMs -> change time, unit:ms}
 * @retval: none
 */
STATIC VOID vLightCtrlSecneChangeStart(IN UINT_T uiChangeTimeMs)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opUserSWTimerStart(SCENE_SW_TIMER, uiChangeTimeMs, (VOID*)vLightCtrlSceneChangeTimerCB);
    if(opRet != LIGHT_OK) {
        PR_ERR("Scene timer start error!");
    }
}

/**
 * @brief: Light scene change stop
 * @param {none}
 * @retval: none
 */
STATIC VOID vLightCtrlSceneChangeStop(VOID)
{
    OPERATE_LIGHT opRet = -1;

    PR_DEBUG("stop scene time!!!!!");
    opRet = opUserSWTimerStop(SCENE_SW_TIMER);
    if(opRet != LIGHT_OK) {
        PR_ERR("Scene timer start error!");
    }
}

/**
 * @brief: scene auto restart
 * @param {none}
 * @attention: to restart scene when app don't submit the scene data!!
 * @retval: none
 */
STATIC VOID vLightCtrlSceneAutoRestartTimeCB(VOID)
{
    OPERATE_LIGHT opRet = -1;
    UINT_T uiCycleTime = 0;
    BOOL_T bEableFlag = 0;

    if((tLightCtrlData.ucRealTimeFlag) && (tLightCtrlData.bSwitch)) {
        tLightCtrlData.ucRealTimeFlag = FALSE;
        tLightCtrlHandle.bSceneStopFlag = FALSE;
        opRet = opLightCtrlSceneChange(&bEableFlag, &uiCycleTime);   /* start */
        if(opRet != LIGHT_OK){
            PR_ERR("Scene change err!");
        }

        if(bEableFlag == TRUE) {
            vLightCtrlSecneChangeStart(uiCycleTime);
        } else {
            PR_DEBUG("Scene don't need change!");
            opRet = opUserSWTimerStop(SCENE_SW_TIMER);      /* stop timer dominantly */
            if(opRet != LIGHT_OK) {
                PR_ERR("stop scene timer error!");
            }
        }
   } else {
        /* don't need to proc! */
   }

    opRet = opUserSWTimerStop(SCENE_AUTO_RESTART_TIMER);    /* stop timer dominantly */
    if(opRet != LIGHT_OK) {
        PR_ERR("scene auto restart timer stop error!");
    }
}

/**
 * @brief: set light data
 * @param {NONE}
 * @attention: this function only deal white or color mode when light is on,
 *              light will set value directly!
 * @retval: OPERATE_RET
 */
OPERATE_RET opLightCtrlDataExtActive(VOID)
{
    OPERATE_RET opRet = 0;

    if(tLightCtrlData.bSwitch == FALSE) {
        opRet = opLightCtrlProc();
    } else { /* only deal white and color mode */
        vLightCtrlShadeStop();          /* first stop shade change! */
        tLightCtrlHandle.bSceneStopFlag = TRUE;  /* stop scene change to make sure don't change scene again */
        vLightCtrlSceneChangeStop();    /* stop scene change firstly */

        vLightCtrlDataCalcRGBCW(tLightCtrlData.eMode, &tLightCtrlData, &tLightCtrlHandle.tTargetVal);

        PR_DEBUG("%d %d %d %d %d",tLightCtrlHandle.tTargetVal.usRed,tLightCtrlHandle.tTargetVal.usGreen,tLightCtrlHandle.tTargetVal.usBlue,\
                                                tLightCtrlHandle.tTargetVal.usWhite,tLightCtrlHandle.tTargetVal.usWarm);

        opRet = opLightSetRGBCW(tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, tLightCtrlHandle.tTargetVal.usBlue, \
                                                tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);
        /* make sure current value = target value */
        memcpy(&tLightCtrlHandle.tCurrVal, &tLightCtrlHandle.tTargetVal, SIZEOF(BRIGHT_DATA_T));
        if(opRet != OPRT_OK) {
                PR_ERR("Light ctrl turn on set RGBCW error!");
                return opRet;
        }
    }

    return opRet;
}

/**
 * @brief: light extern set response
 * @param {NONE}
 * @attention:sysytem need apply all dp response or need dp
 * @retval: none
 */
WEAK VOID vLightCtrlDataExtRespone(VOID)
{

}

/**
 * @brief: extern set light data
 * @param {IN LIGHT_CTRL_EXT_DATA_T *ptExtData --> extern set param}
 * @param {IN BOOL_T bNeedUpload --> need upload}
 * @param {IN BOOL_T bActiveImmed --> actice immediately, don't need call opLightCtrl() function }
 * @param {IN BOOL_T bNeedSave --> need to save to flash }
 * @attention: when turn off light, mode bright temperature and rgb will not accept!
 *                                only set white mode or color mode.
 * @retval: OPERATE_RET
 */
OPERATE_RET opLightCtrlDataExtSet(IN LIGHT_CTRL_EXT_DATA_T *ptExtData, IN BOOL_T bNeedUpload, IN BOOL_T bActiveImmed, IN BOOL_T bNeedSave)
{
    OPERATE_RET opRet = -1;

    if(WHITE_MODE == ptExtData ->Mode) {    /* adjust CW */
        if((ptExtData ->usBright < CTRL_CW_BRIGHT_VALUE_MIN) || (ptExtData ->usBright > CTRL_CW_BRIGHT_VALUE_MAX)) {
            PR_ERR("bright value is exceed range,set error");
            return OPRT_INVALID_PARM;
        }

        if(ptExtData ->usTemper > CTRL_CW_BRIGHT_VALUE_MAX) {
            PR_ERR("temperature value is exceed range,set error:%d", ptExtData ->usTemper);
            return OPRT_INVALID_PARM;
        }
        tLightCtrlData.bSwitch = ptExtData ->bSwitch;

        tLightCtrlData.usBright = ptExtData ->usBright;
        tLightCtrlData.usTemper = ptExtData ->usTemper;
        tLightCtrlData.eMode = WHITE_MODE;    /* change mode to white mode forcibly */
        if(bActiveImmed) {
            opRet = opLightCtrlDataExtActive();
            if(opRet != OPRT_OK) {
                PR_ERR("extern active Immediately error");
                return opRet;
            }
        }

    } else if (COLOR_MODE == ptExtData ->Mode) {

        if((ptExtData ->Color.usRed > CTRL_CAL_VALUE_RANGE) ||
            (ptExtData ->Color.usGreen > CTRL_CAL_VALUE_RANGE) ||
            (ptExtData ->Color.usBlue > CTRL_CAL_VALUE_RANGE)) {

            PR_ERR("RGB color value is exceed range,set error");
            return OPRT_INVALID_PARM;
        }

        tLightCtrlData.bSwitch = ptExtData ->bSwitch;
        tLightCtrlData.eMode = COLOR_MODE;    /* change mode to color mode */
        memcpy(&tLightCtrlData.tColorOrigin, &(ptExtData ->ColorOrigin), SIZEOF(COLOR_ORIGIN_T));    /* must save the sorigin data */

        memcpy(&tLightCtrlData.tColor, &(ptExtData ->Color), SIZEOF(COLOR_RGB_T));
        if(bActiveImmed) {
            opRet = opLightCtrlDataExtActive();
            if(opRet != OPRT_OK) {
                PR_ERR("extern active Immediately error");
                return opRet;
            }
        }

    }else{
        PR_DEBUG("opLightCtrlDataExtSet scene don't need deal!");
        tLightCtrlData.bSwitch = ptExtData ->bSwitch;
        tLightCtrlData.eMode = ptExtData ->Mode;

    }


    if(bNeedUpload) {
        vLightCtrlDataExtRespone();
    }

    if(bNeedSave) {
        opRet = opLightCtrlDataAutoSaveStart(5000);
        if(opRet != OPRT_OK) {
            PR_ERR("extern auto save time start error");
            return opRet;
        }
    }

    return OPRT_OK;
}

/**
 * @berief: Light control proc
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlProc(VOID)
{
    OPERATE_LIGHT opRet = -1;
    UINT_T uiCycleTime = 0;
    BOOL_T bEableFlag = 0;
    STATIC BOOL_T bLastSwitchStatus = FALSE;

    CHECK_INIT_OK();

#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)
    if( tLightCfgData.bTitle20 ) {      /* lowpower function enable */
        opRet = opLightCtrlLowPowerProc(tLightCtrlData.bSwitch);    /* lowpower process */
        if(opRet != LIGHT_OK) {
            PR_ERR("Lowpower proc error!");
        }
    }
#endif

    if(tLightCtrlData.bSwitch == FALSE) {  /* onoff ctrl state -- turn off */

        vLightShadeCtrlDisable();       /* stop all shade process! */

        memset(&tLightCtrlHandle.tTargetVal, 0, sizeof(BRIGHT_DATA_T));  /* set target contol data!!! */
        if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {   /* CCT drive mode make sure don't change warm */
            tLightCtrlHandle.tTargetVal.usWarm = tLightCtrlHandle.tCurrVal.usWarm;
        }

        if(SWITCH_GRADUAL == tLightCfgData.eSwitchMode) {     /* shut down gradually */
            switch(tLightCtrlData.eMode)
            {
                case WHITE_MODE:
                case COLOR_MODE:
                    PR_DEBUG("start to shut down!");
                    vLightCtrlShadeStepGainSet(SHADE_STEP_GAIN_DEFAULT);
                    vLightCtrlShadeStart(LIGHT_SHADE_CYCLE);
                    PR_DEBUG("current %d %d %d %d %d", tLightCtrlHandle.tCurrVal.usRed, tLightCtrlHandle.tCurrVal.usGreen, \
                                tLightCtrlHandle.tCurrVal.usBlue, tLightCtrlHandle.tCurrVal.usWhite, tLightCtrlHandle.tCurrVal.usWarm);

                    PR_DEBUG("target %d %d %d %d %d", tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, \
                                tLightCtrlHandle.tTargetVal.usBlue, tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);
                    break;

                case SCENE_MODE:

                    PR_DEBUG("current %d %d %d %d %d", tLightCtrlHandle.tCurrVal.usRed, tLightCtrlHandle.tCurrVal.usGreen, \
                        tLightCtrlHandle.tCurrVal.usBlue, tLightCtrlHandle.tCurrVal.usWhite, tLightCtrlHandle.tCurrVal.usWarm);
                    PR_DEBUG("turn off disable scene timer flag!");

                    vLightCtrlShadeStepGainSet(SHADE_STEP_GAIN_DEFAULT);
                    vLightCtrlShadeStart(LIGHT_SHADE_CYCLE);
                    break;

                case MUSIC_MODE:
                    opRet = opLightSetRGBCW(tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, tLightCtrlHandle.tTargetVal.usBlue, \
                                                tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);

                    memset(&tLightCtrlHandle.tCurrVal, 0, sizeof(tLightCtrlHandle.tCurrVal));
                    if(opRet != LIGHT_OK) {
                        PR_ERR("Light ctrl music mode shutdown set RGBCW error!");
                        return opRet;
                    }
                    break;

                default:
                    break;
            }
        } else if(SWITCH_DIRECT == tLightCfgData.eSwitchMode) {   /* shut down directly */

            tLightCtrlHandle.bSceneStopFlag = TRUE;
            vLightCtrlSceneChangeStop();
            opRet = opLightSetRGBCW(tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, tLightCtrlHandle.tTargetVal.usBlue, \
                                        tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);
            memset(&tLightCtrlHandle.tCurrVal, 0, sizeof(tLightCtrlHandle.tCurrVal));
            if(opRet != LIGHT_OK) {
                PR_ERR("Light ctrl music mode shutdown set RGBCW error!");
                return opRet;
            }

            /* make sure current value to clear */
            memcpy(&tLightCtrlHandle.tCurrVal, &tLightCtrlHandle.tTargetVal, SIZEOF(BRIGHT_DATA_T));

            if((BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) && (FALSE == tLightCtrlData.bSwitch)) {
                opRet = opUserSWTimerStart(CCT_DELAY_SHUT_DOWN_TIMER, 1000, vLightCtrlCCTShutDownTimeCB);
                if(opRet != LIGHT_OK) {
                    PR_ERR("CCT shut down err!");
                    /* when CCT shut down, need to turn off CCT */
                    opRet = opLightSetRGBCW(0, 0, 0, 0, 0);
                }
            }

        } else {    /* LightCfgData.Switch_mode */
            ;       /* no possible  */
        }


    } else {    /* onoff ctrl state -- turn on */

        opRet = opUserSWTimerStop(CCT_DELAY_SHUT_DOWN_TIMER);
        if(opRet != OPRT_OK) {
            PR_ERR("CCT shut down delay timer stop!");
        }
        PR_DEBUG("current mode %d", tLightCtrlData.eMode);

        if(tLightCtrlData.eMode == SCENE_MODE) {

            tLightCtrlHandle.bSceneStopFlag = FALSE;
            opRet = opLightCtrlSceneChange(&bEableFlag, &uiCycleTime);   /* start */
            if(opRet != LIGHT_OK){
                PR_ERR("Scene change err!");
            }

            if(bEableFlag == TRUE) {
                vLightCtrlSecneChangeStart(uiCycleTime);
            } else {
                PR_DEBUG("Scene don't need change!");
                opRet = opUserSWTimerStop(SCENE_SW_TIMER);      /* stop timer dominantly */
                if(opRet != LIGHT_OK) {
                    PR_ERR("stop scene timer error!");
                }
            }
        } else if(tLightCtrlData.eMode == MUSIC_MODE) {   /* mode is music mode!!!! */ /* this mode only appear turn on firstly */
            tLightCtrlHandle.bSceneStopFlag = TRUE;      /* stop scene */
            vLightCtrlSceneChangeStop();

            LIGHT_CTRL_DATA_T tMusicCtrlData;
            memset(&tMusicCtrlData, 0, sizeof(LIGHT_CTRL_DATA_T));
            memcpy(&tMusicCtrlData, &tLightCtrlData, SIZEOF(LIGHT_CTRL_DATA_T));     /* make sure music mode restart as red color */

            tMusicCtrlData.tColor.usRed = CTRL_RGB_BRIGHT_VALUE_MAX;
            tMusicCtrlData.tColor.usGreen = 0;
            tMusicCtrlData.tColor.usBlue = 0;

            vLightCtrlDataCalcRGBCW(tLightCtrlData.eMode,  &tMusicCtrlData, &tLightCtrlHandle.tTargetVal);
            PR_NOTICE("%d %d %d %d %d",tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, tLightCtrlHandle.tTargetVal.usBlue,\
                                tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);

            if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {   /* CCT drive mode make sure don't change warm */
                tLightCtrlHandle.tCurrVal.usWarm = tLightCtrlHandle.tTargetVal.usWarm;
            }

            vLightCtrlShadeStepGainSet(SHADE_STEP_GAIN_DEFAULT);
            vLightCtrlShadeStart(LIGHT_SHADE_CYCLE);

        } else {    /* mode is not scene and music mode ,mode is white or color mode ! */
            tLightCtrlHandle.bSceneStopFlag = TRUE;  /* stop scene */
            vLightCtrlSceneChangeStop();
            if(bLastSwitchStatus != FALSE) {        /* already turn on */
                vLightCtrlDataCalcRGBCW(tLightCtrlData.eMode, &tLightCtrlData, &tLightCtrlHandle.tTargetVal);

                PR_DEBUG("%d %d %d %d %d",tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, tLightCtrlHandle.tTargetVal.usBlue,\
                            tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);

                vLightCtrlShadeStepGainSet(SHADE_STEP_GAIN_DEFAULT);
                vLightCtrlShadeStart(LIGHT_SHADE_CYCLE);

            } else {        /* firstly turn on */

                if(SWITCH_GRADUAL == tLightCfgData.eSwitchMode) {
                    vLightCtrlDataCalcRGBCW(tLightCtrlData.eMode, &tLightCtrlData, &tLightCtrlHandle.tTargetVal);

                    PR_DEBUG("%d %d %d %d %d",tLightCtrlHandle.tTargetVal.usRed,tLightCtrlHandle.tTargetVal.usGreen,tLightCtrlHandle.tTargetVal.usBlue,\
                                        tLightCtrlHandle.tTargetVal.usWhite,tLightCtrlHandle.tTargetVal.usWarm);

                    if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {   /* CCT drive mode make sure don't change warm */
                        PR_DEBUG("CCT need to proc!");
                        tLightCtrlHandle.tCurrVal.usWarm = tLightCtrlHandle.tTargetVal.usWarm;
                    }

                    vLightCtrlShadeStepGainSet(SHADE_STEP_GAIN_DEFAULT);
                    vLightCtrlShadeStart(LIGHT_SHADE_CYCLE);

                } else if(SWITCH_DIRECT == tLightCfgData.eSwitchMode) {
                    vLightCtrlDataCalcRGBCW(tLightCtrlData.eMode,  &tLightCtrlData, &tLightCtrlHandle.tTargetVal);
                    PR_DEBUG("%d %d %d %d %d",tLightCtrlHandle.tTargetVal.usRed,tLightCtrlHandle.tTargetVal.usGreen,tLightCtrlHandle.tTargetVal.usBlue,\
                                        tLightCtrlHandle.tTargetVal.usWhite,tLightCtrlHandle.tTargetVal.usWarm);

                    if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {   /* CCT drive mode make sure don't change warm */
                        tLightCtrlHandle.tCurrVal.usWarm = tLightCtrlHandle.tTargetVal.usWarm;
                    }

                    opRet = opLightSetRGBCW(tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, tLightCtrlHandle.tTargetVal.usBlue, \
                                                        tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);

                    /* make sure current value = target value */
                    memcpy(&tLightCtrlHandle.tCurrVal, &tLightCtrlHandle.tTargetVal, SIZEOF(BRIGHT_DATA_T));

                    if(opRet != LIGHT_OK) {
                        PR_ERR("Light ctrl turn on set RGBCW error!");
                        return opRet;
                    }
                } else {    /* LightCfgData.Switch_mode */
                    ;       /* no possible  */
                }
            }
       }

    }

    bLastSwitchStatus = tLightCtrlData.bSwitch;

    return LIGHT_OK;
}

/**
 * @brief: light ctrl disable
 * @param {none}
 * @retval: none
 */
VOID vLightCtrlDisable(VOID)
{
    OPERATE_LIGHT opRet = -1;
    UCHAR_T i = 0;

    opRet = opUserHWTimerStop();
    if(opRet != LIGHT_OK) {
        PR_ERR("hardware timer stop error!");
    }

    for(i = 0; i < SW_TIMER_MAX; i++) {
        opRet = opUserSWTimerStop(i);
        if(opRet != LIGHT_OK) {
           PR_ERR("stop %d timer error!");
        }
    }

    opRet = opUserFlashWriteResetCnt(0);     /* Reset cnt ++ &save to flash */
    if(opRet != LIGHT_OK) {
        PR_ERR("Reset cnt clear error!");
    }

#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)
    if(tLightCfgData.bTitle20) {
        vLightSysCtrlLowPowerExist();
    }
#endif
}

/**
 * @brief: shade process disable
 * @attention:
 * @retval: none
 */
VOID vLightShadeCtrlDisable(VOID)
{
    vLightCtrlShadeStop();                  /* first stop shade change! */
    tLightCtrlHandle.bSceneStopFlag = TRUE;  /* stop scene change to make sure don't change scene again */
    vLightCtrlSceneChangeStop();            /* stop scene change firstly */
}

/**
 * @brief: countdown lave time return proc
 * @param {OUT UINT_T uiRemainTimeSec -> left time,unit:sec}
 * @attention: this function need to implement in system
 * @retval: none
 */
WEAK VOID vLightCtrlDataCountDownResponse(OUT UINT_T uiRemainTimeSec)
{
    ;
}

/**
 * @brief: light ctrl breath proc
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
STATIC VOID vLightCtrlCountDownTimerCB(VOID)
{
    OPERATE_LIGHT opRet = -1;

    if(tLightCtrlData.uiCountDown > 1) {    /* to avoid ulCountDown = 0 %60 also equal to 0 */
        tLightCtrlData.uiCountDown --;

        if((tLightCtrlData.uiCountDown % 60) == 0) {     /* upload countdown value per min */
            vLightCtrlDataCountDownResponse( tLightCtrlData.uiCountDown);
        }

    } else {
        tLightCtrlData.uiCountDown = 0;
        tLightCtrlData.bSwitch = (tLightCtrlData.bSwitch != FALSE) ? FALSE : TRUE;

        opRet = opLightCtrlProc();
        if(opRet != LIGHT_OK) {
            PR_ERR("CountDown process error!");
        }

        vLightCtrlDataCountDownResponse(0);      /* opload after ctrl deal with */

        opRet = opUserSWTimerStop(COUNTDOWN_SW_TIMER);  /* stop timer */
        if(opRet != LIGHT_OK) {
           PR_ERR("stop countdown software timer error!");
        }
        opRet = opLightCtrlDataAutoSaveStart(5000);
        if(opRet != LIGHT_OK) {
            PR_ERR("coutdown auto save time start error");
        }
        return;

    }

    opRet = opUserSWTimerStart(COUNTDOWN_SW_TIMER, 1000, (VOID*)vLightCtrlCountDownTimerCB);
    if(opRet != LIGHT_OK) {
        PR_ERR("CountDown timer restart error!");
    }

}

/**
 * @brief: set light countdown value
 * @param {IN INT_T uiCountDownSec -> unit:second}
 * @attention: countdown lave time will return with
 *              calling vLightDataCountDownResponse function every minutes.
 *              switch status will return with calling
 *              vLightCtrlDataSwitchRespone function when countdown active.
 * @retval: OPERATE_LIGHT -> LIGHT_OK set countdown OK.
 */
OPERATE_LIGHT opLightCtrlDataCountDownSet(IN INT_T uiCountDownSec)
{
    OPERATE_LIGHT opRet = -1;

    if((uiCountDownSec < 0)|| (uiCountDownSec > 86400)){
        PR_ERR("Set countdwon value error!");
        return LIGHT_INVALID_PARM;
    }

    tLightCtrlData.uiCountDown = uiCountDownSec;
    if(uiCountDownSec <= 0) {
        opRet = opUserSWTimerStop(COUNTDOWN_SW_TIMER);      /* cancel coutdown proc */
        if(opRet != LIGHT_OK) {
            PR_ERR("Stop countdown timer error!");
        }
        vLightCtrlDataCountDownResponse(0);                 /* don't need response switch status,because user set initiactive */
        return LIGHT_OK;

    } else {
        opRet = opUserSWTimerStart(COUNTDOWN_SW_TIMER, 1000, (VOID*)vLightCtrlCountDownTimerCB);
        if(opRet != LIGHT_OK) {
            PR_ERR("Start countdown timer error!");
        }
    }
    vLightCtrlDataCountDownResponse(uiCountDownSec);       /* upload countdown value */

    return LIGHT_OK;
}

/**
 * @brief: set light realtime control data
 * @param {IN BOOL_T bMusicFlag}
 * @param {IN CHAR_T *pRealTimeData}
 * @attention: data format: please reference to DP protocol
 * @retval: OPERATE_LIGHT -> LIGHT_OK need to call opLightRealTimeCtrlProc function.
 */
OPERATE_LIGHT opLightCtrlDataRealTimeAdjustSet(IN BOOL_T bMusicFlag, IN CHAR_T *pRealTimeData)
{
    if(bMusicFlag) {
        if(tLightCtrlData.eMode != MUSIC_MODE) {
            PR_DEBUG("Music data don't accpected, bacause mode is not music mode!");
            return LIGHT_INVALID_PARM;
        }
    }

    if(strcmp(pRealTimeData, tLightCtrlData.cRealTimeData) == 0) {
        PR_DEBUG("the same realtime adjust data");
        return LIGHT_INVALID_PARM;
    }

    if(strlen(pRealTimeData) != 21) {
        PR_ERR("Real time adjust data is error! please chek!");
        return LIGHT_INVALID_PARM;
    }

    strcpy((CHAR_T*)&tLightCtrlData.cRealTimeData, pRealTimeData);
    PR_DEBUG("light real time adjust ctrl data buf %s",tLightCtrlData.cRealTimeData);

    return LIGHT_OK;
}

/**
 * @brief: Light realtime ctrl process
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightRealTimeCtrlProc(VOID)
{
    UCHAR_T ucChangeMode = 0;
    USHORT_T usBright = 0,usTemp = 0;
    USHORT_T usHue = 0, usSat = 0, usVal = 0;
    LIGHT_CTRL_DATA_T tCtrlDataTemp;
    OPERATE_LIGHT opRet = -1;

    if(!tLightCtrlData.bSwitch) {        /* make sure realtime adjust dp can't active! */
        return LIGHT_OK;
    }

    vLightShadeCtrlDisable();       /* stop all shade process! */

    vLightHSVBTAnalysize(&tLightCtrlData.cRealTimeData[1], &usHue, &usSat, &usVal, &usBright, &usTemp);

    usVal = usLightCtrlDataCalcHSVLimit(usVal);

    PR_DEBUG("hsv %d %d %d",usHue,usSat,usVal);

    memset(&tCtrlDataTemp, 0, SIZEOF(LIGHT_CTRL_DATA_T));

    if((WHITE_MODE == tLightCtrlData.eMode) || (COLOR_MODE == tLightCtrlData.eMode)){

        /* mix group proc!!!! */ //attention：only recevice correct data according to lightway
        switch(tLightCfgData.eLightWay) {
            case LIGHT_C:
                if(usVal > 0) {    /* when mix group, if receive color realtime data,just stand */
                    tCtrlDataTemp.usBright = tLightCtrlData.usBright;
                } else {
                    tCtrlDataTemp.usBright = usBright;
                }
                tCtrlDataTemp.usTemper = CTRL_CW_BRIGHT_VALUE_MAX;
                break;
            case LIGHT_CW:
                if(usVal > 0 ) {    /* when mix group, if receive color realtime data,just stand */
                    tCtrlDataTemp.usBright = tLightCtrlData.usBright;
                    tCtrlDataTemp.usTemper = tLightCtrlData.usTemper;
                } else {
                    tCtrlDataTemp.usBright = usBright;
                    tCtrlDataTemp.usTemper = usTemp;
                }
                break;
            case LIGHT_RGB:
                if(usBright >= CTRL_CW_BRIGHT_VALUE_MIN) {  /* when mix group, if receive white realtime data,just stand */
                    tCtrlDataTemp.tColor.usRed = tLightCtrlData.tColor.usRed;
                    tCtrlDataTemp.tColor.usGreen = tLightCtrlData.tColor.usGreen;
                    tCtrlDataTemp.tColor.usBlue = tLightCtrlData.tColor.usBlue;
                }else {
                    vLightToolHSV2RGB(usHue, usSat, usVal, &tCtrlDataTemp.tColor.usRed, &tCtrlDataTemp.tColor.usGreen, &tCtrlDataTemp.tColor.usBlue);
                }
                break;
            case LIGHT_RGBC:
                tCtrlDataTemp.usBright = usBright;
                tCtrlDataTemp.usTemper = CTRL_CW_BRIGHT_VALUE_MAX;
                vLightToolHSV2RGB(usHue, usSat, usVal, &tCtrlDataTemp.tColor.usRed, &tCtrlDataTemp.tColor.usGreen, &tCtrlDataTemp.tColor.usBlue);
                break;
            case LIGHT_RGBCW:
                tCtrlDataTemp.usBright = usBright;
                tCtrlDataTemp.usTemper = usTemp;
                vLightToolHSV2RGB(usHue, usSat, usVal, &tCtrlDataTemp.tColor.usRed, &tCtrlDataTemp.tColor.usGreen, &tCtrlDataTemp.tColor.usBlue);
                break;
            default:
                break;

        }
        vLightCtrlDataCalcCW(tCtrlDataTemp.usBright, tCtrlDataTemp.usTemper, &tLightCtrlHandle.tTargetVal);
        vLightCtrlDataCalcRGB(&(tCtrlDataTemp.tColor), &tLightCtrlHandle.tTargetVal);
        PR_DEBUG("realtime RGBCW %d %d %d %d %d", tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, tLightCtrlHandle.tTargetVal.usBlue,\
            tLightCtrlHandle.tTargetVal.usWhite,tLightCtrlHandle.tTargetVal.usWarm);

    } else if(SCENE_MODE == tLightCtrlData.eMode)  {    /* scene mode */


        tLightCtrlData.ucRealTimeFlag = TRUE;        /* have set realtime data  */

        memset(&tCtrlDataTemp, 0, SIZEOF(LIGHT_CTRL_DATA_T));

        tCtrlDataTemp.usBright = usBright;
        tCtrlDataTemp.usTemper = usTemp;

        PR_DEBUG(" scene hsv %d %d %d",usHue,usSat,usVal);

        vLightToolHSV2RGB(usHue, usSat, usVal, &tCtrlDataTemp.tColor.usRed, &tCtrlDataTemp.tColor.usGreen, &tCtrlDataTemp.tColor.usBlue);


        vLightCtrlDataCalcRGBCW(tLightCtrlData.eMode, &tCtrlDataTemp, &tLightCtrlHandle.tTargetVal);

    } else {  /* music mode */
        memset(&tCtrlDataTemp, 0, SIZEOF(LIGHT_CTRL_DATA_T));

        tCtrlDataTemp.usBright = usBright;
        tCtrlDataTemp.usTemper = usTemp;

        PR_DEBUG(" scene hsv %d %d %d",usHue,usSat,usVal);

        vLightToolHSV2RGB(usHue, usSat, usVal, &tCtrlDataTemp.tColor.usRed, &tCtrlDataTemp.tColor.usGreen, &tCtrlDataTemp.tColor.usBlue);

        vLightCtrlDataCalcCW(tCtrlDataTemp.usBright, tCtrlDataTemp.usTemper, &tLightCtrlHandle.tTargetVal);
        vLightCtrlDataCalcRGB(&(tCtrlDataTemp.tColor), &tLightCtrlHandle.tTargetVal);

        if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) { /* as music mode realtime will send jump, also need keep warm when cct */
            tLightCtrlHandle.tTargetVal.usWarm = tLightCtrlHandle.tCurrVal.usWarm ;
        }

    }

    ucChangeMode = tLightCtrlData.cRealTimeData[0] - '0';



    if(REALTIME_CHANGE_JUMP == ucChangeMode) {

        memcpy(&tLightCtrlHandle.tCurrVal, &tLightCtrlHandle.tTargetVal, SIZEOF(tLightCtrlHandle.tTargetVal));

        PR_DEBUG("%d %d %d %d %d",tLightCtrlHandle.tTargetVal.usRed,tLightCtrlHandle.tTargetVal.usGreen,tLightCtrlHandle.tTargetVal.usBlue,\
                                    tLightCtrlHandle.tTargetVal.usWhite,tLightCtrlHandle.tTargetVal.usWarm);
        opRet = opLightSetRGBCW(tLightCtrlHandle.tTargetVal.usRed, tLightCtrlHandle.tTargetVal.usGreen, tLightCtrlHandle.tTargetVal.usBlue, \
                                            tLightCtrlHandle.tTargetVal.usWhite, tLightCtrlHandle.tTargetVal.usWarm);
        if(opRet != LIGHT_OK) {
            PR_ERR("Light ctrl turn on set RGBCW error!");
            return opRet;
        }
    } else {
        USHORT_T usMaxError ;
        usMaxError = usLightGetShadeMAX(&tLightCtrlHandle.tTargetVal, &tLightCtrlHandle.tCurrVal);
        vLightCtrlShadeStepGainSet((UINT_T)((usMaxError * LIGHT_SHADE_CYCLE / 300.0)));     /* 150ms APP issue realtime control dp */
        vLightCtrlShadeStart(LIGHT_SHADE_CYCLE);
    }

    return LIGHT_OK;
}

/**
 * @brief: Light reset cnt clear timercallback
 * @param {none}
 * @retval: none
 */
STATIC VOID vLightCtrlResetCntClearTimrCB(VOID)
{
    OPERATE_LIGHT opRet = -1;

    PR_DEBUG("reset cnt clear!");

    opRet = opUserFlashWriteResetCnt(0);     /* Reset cnt ++ &save to flash */
    if(opRet != LIGHT_OK) {
        PR_ERR("Reset cnt clear error!");
        return ;
    }

    opRet = opUserSWTimerStop(CLEAR_RESET_CNT_SW_TIMER);        /* stop timer dominantly */
    if(opRet != LIGHT_OK) {
        PR_ERR("stop reset clear timer error!");
    }
}

/**
 * @brief: system reboot as hardware mode judge proc
 * @param {none}
 * @attention: this function need to implement by each plantform.
 * @retval: BOOL_T TRUE -> system reboot
 */
WEAK BOOL_T bLightSysHWRebootJudge(VOID)
{
    return 1;
}

/**
 * @berief: Light hardware reboot judge & proc
 *          process detail:
 *                  1. hardware reset judge;
 *                  2. load reboot cnt data;
 *                  3. reboot cnt data increase;
 *                  4. start software time to clear reboot cnt;
 * @param {none}
 * @attention: this function need bLightSysHWRebootJudge();
 * @retval: none
 */
VOID vLightCtrlHWRebootProc(VOID)
{
    OPERATE_LIGHT opRet = -1;
    BOOL_T bHWRebootFlag = FALSE;
    UCHAR_T ucCnt = 0;

    bHWRebootFlag = bLightSysHWRebootJudge();
    if(TRUE != bHWRebootFlag) {
        return;
    }

    PR_DEBUG("Light hardware reboot, turn on light!");

    opRet = opUserFlashReadResetCnt(&ucCnt);     /* read cnt from flash */
    if(opRet != LIGHT_OK) {
        PR_ERR("Read reset cnt error!");
    }
    PR_DEBUG("read reset cnt %d",ucCnt);

    ucCnt++;
    opRet = opUserFlashWriteResetCnt(ucCnt);     /* Reset cnt ++ &save to flash */
    if(opRet != LIGHT_OK) {
        PR_ERR("Reset cnt add write error!");
        return ;
    }

    PR_DEBUG("start reset cnt clear timer!!!!!");

    /* start clear reset cnt timer */
    opRet = opUserSWTimerStart(CLEAR_RESET_CNT_SW_TIMER, 5000, (VOID*)vLightCtrlResetCntClearTimrCB);
    if(opRet != LIGHT_OK) {
        PR_ERR("start reset clear timer error!");
    }

}

/**
 * @brief: Light ctrl data save
 * @param {none}
 * @attention: this function directly save ctrl data.
 * @retval: none
 */
OPERATE_LIGHT opLightCtrlDataAutoSave(VOID)
{
    OPERATE_LIGHT opRet = -1;
    LIGHT_APP_DATA_FLASH_T tSaveData;

    if(tLightCfgData.bMemory != TRUE) {
        return LIGHT_OK;
    }

    memset(&tSaveData, 0, SIZEOF(LIGHT_APP_DATA_FLASH_T));

    tSaveData.bPower = tLightCtrlData.bSwitch;
    tSaveData.eMode = tLightCtrlData.eMode;
    tSaveData.usBright = tLightCtrlData.usBright;
    tSaveData.usTemper = tLightCtrlData.usTemper;
    tSaveData.tColor.usRed = tLightCtrlData.tColor.usRed;
    tSaveData.tColor.usGreen = tLightCtrlData.tColor.usGreen;
    tSaveData.tColor.usBlue = tLightCtrlData.tColor.usBlue;
    tSaveData.tColorOrigin.usHue = tLightCtrlData.tColorOrigin.usHue;
    tSaveData.tColorOrigin.usSat = tLightCtrlData.tColorOrigin.usSat;
    tSaveData.tColorOrigin.usValue  = tLightCtrlData.tColorOrigin.usValue;
    strcpy((CHAR_T*)&tSaveData.tColorOrigin.ucColorStr, (CHAR_T*)&tLightCtrlData.tColorOrigin.ucColorStr);
    memcpy(&tSaveData.cScene, &tLightCtrlData.cScene, SIZEOF(tLightCtrlData.cScene));

#if 0
    PR_DEBUG("flash mode: %d",tSaveData.Mode);
    PR_DEBUG("flash on/off: %d",tSaveData.bPower);
    PR_DEBUG("flash R: %d",tSaveData.Color.usRed);
    PR_DEBUG("flash G: %d",tSaveData.Color.usGreen);
    PR_DEBUG("flash B: %d",tSaveData.Color.usBlue);
    PR_DEBUG("flash B: %d",tSaveData.usBright);
    PR_DEBUG("flash T: %d",tSaveData.usTemper);
    PR_DEBUG("flash raw data1: %d",tSaveData.ColorOrigin.usColorData1);
    PR_DEBUG("flash raw data2: %d",tSaveData.ColorOrigin.usColorData2);
    PR_DEBUG("flash raw data3: %d",tSaveData.ColorOrigin.usColorData3);
    PR_DEBUG("flash scene data: %s",tSaveData.cScene);
#endif
    opRet = opUserFlashWriteAppData(&tSaveData);
    if(opRet != LIGHT_OK) {
        PR_ERR("Write app data ERROR!");
        return LIGHT_COM_ERROR;
    }

    return LIGHT_OK;
}

/**
 * @brief: system reset proc
 * @param {none}
 * @attention: this function need implememt by system,
 *              need to deal with different thing in each plantform.
 * @retval: none
 */
WEAK OPERATE_LIGHT opLightSysResetCntOverCB(VOID)
{
    return LIGHT_OK;
}

/**
 * @brief: system CCT drive mode delya timer cb
 * @param {none}
 * @attention: this active will increase reset time!
 * @retval: none
 */
VOID vLightCtrlCCTDelayResetTimeCB(VOID)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opUserSWTimerStop(CCT_DELAY_RESET_TIMER);   /*  */
    if(opRet != LIGHT_OK) {
        PR_ERR("CCT delay reset timer stop error!");
    }

    PR_DEBUG("CCT delay 200ms reset factory!");
    opRet = opLightSysResetCntOverCB();     /* system reset deal proc */
    if(opRet != LIGHT_OK){
        PR_ERR("Light reset proc error!");
    }
}

/**
 * @berief: Light reset cnt clear
 * @param {none}
 * @attention: this func will call in opLightCtrlResetCntProcess
 *              when no opLightSysResetCntOverCB finish callback, otherwise
 *               call in system!!!
 * @retval: none
 */
VOID vLightCtrlResetCntClear(VOID)
{
    OPERATE_RET opRet = -1;

    opRet = opUserFlashWriteResetCnt(0);    /* write cnt = 0 to flash!! */
    if(opRet != OPRT_OK) {
        PR_ERR("reset cnt set error!");
    }
}

/**
 * @brief: Light reset to re-distribute proc
 * @param {none}
 * @attention: this func will call opLightSysResetCntOverCB()
 *              opLightSysResetCntOverCB() need to implement by system
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlResetCntProcess(VOID)
{
    OPERATE_LIGHT opRet = -1;
    UCHAR_T ucCnt = 0;

    opRet = opUserFlashReadResetCnt(&ucCnt);     /* read cnt from flash */
    if(opRet != LIGHT_OK) {
        PR_ERR("Read reset cnt error!");
        return LIGHT_COM_ERROR;
    }

    if(ucCnt < tLightCfgData.ucResetCnt) {
        PR_DEBUG("Don't reset ctrl data!");
        return LIGHT_OK;
    }

    PR_DEBUG("Reset ctrl data!");

    vLightCtrlDataReset();                  /* reset ctrl data & save */
    opRet = opLightCtrlDataAutoSave();
    if(opRet != LIGHT_OK) {
        PR_ERR("reset ctrl data error!");
    }

    vLightCtrlResetCntClear();    /* @attention：this function please inmplement by function note! */
    opRet = opUserSWTimerStop(CLEAR_RESET_CNT_SW_TIMER);
    if(opRet != OPRT_OK) {
        PR_ERR("clear cnt time stop error!");
    }

    if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {
        opRet = opLightSetRGBCW(0, 0, 0, 0, tLightCtrlData.usTemper);
        if(opRet != LIGHT_OK) {
            PR_ERR("CCT reset shut down bright firstly error!");
        }

        opRet = opUserSWTimerStart(CCT_DELAY_RESET_TIMER, 200, (VOID*)vLightCtrlCCTDelayResetTimeCB);
        if(opRet != LIGHT_OK) {
            PR_ERR("CCT reset delay start timer error!");
        } else {
            return LIGHT_OK;
        }
    }

    PR_NOTICE("Light will reset!");
    opRet = opLightSysResetCntOverCB();     /* system reset deal proc */
    if(opRet != LIGHT_OK){
        PR_ERR("Light reset proc error!");
    }

    return opRet;
}

/**
 * @brief: get connect mode cfg
 * @param {none}
 * @retval: UCHAR_T connect mode
 */
UCHAR_T ucLightCtrlGetConnectMode(VOID)
{
    return (tLightCfgData.ucConnectMode);
}

/**
 * @brief: get connect mode cfg
 * @param {none}
 * @retval: UCHAR_T
 */
UCHAR_T ucLightCtrlGetConnectTime(VOID)
{
    return (tLightCfgData.ucPairTime);
}

/**
 * @brief: get color max limit
 * @param {none}
 * @retval: UCHAR_T
 */
UCHAR_T ucLightCtrlGetColorMax(VOID)
{
    return (tLightCfgData.ucLimitRGBMax);
}

/**
 * @brief: get color min limit
 * @param {none}
 * @retval: UCHAR_T
 */
UCHAR_T ucLightCtrlGetColorMin(VOID)
{
    return (tLightCfgData.ucLimitRGBMin);
}

/* @brief: light ctrl data autosave timer callback
 * @param {none}
 * @retval: none
 */
STATIC VOID vLightCtrlDataAutoSaveTimerCB(VOID)
{
    OPERATE_LIGHT opRet = -1;

    PR_DEBUG("auto save dp ctrl!");

    opRet = opLightCtrlDataAutoSave();
    if(opRet != LIGHT_OK) {
        PR_ERR("Light ctrl data auto save error!");
    }else{
        PR_DEBUG("Light ctrl app auto data save OK !");
    }

    opRet = opUserSWTimerStop(AUTOSAVE_SW_TIMER);
    if(opRet != LIGHT_OK) {
       PR_ERR("stop autosave software timer error!");
    }
}

/**
 * @brief: light ctrl data auto save proc
 * @param {IN UINT_T uiDelayTimeMs -> ctrl data auto save delay time,uint:ms}
 * @attention: save data proc will do after the last call actually!
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataAutoSaveStart(IN UINT_T uiDelayTimeMs)
{
    OPERATE_LIGHT opRet = -1;

    CHECK_INIT_OK();

    opRet = opUserSWTimerStart(AUTOSAVE_SW_TIMER, uiDelayTimeMs, (VOID*)vLightCtrlDataAutoSaveTimerCB);
    if(opRet != LIGHT_OK) {
        PR_ERR("data save timer start error!");
    }

    return LIGHT_OK;
}

/**
 * @brief: calc def bright temper and color, according to param
 * @param {IN CTRL_DEF_COLOR_E eColor -> default color}
 * @param {IN USHORT_T usBright -> default bright}
 * @param {IN USHORT_T usTemper -> default temperature}
 * @param {OUT BRIGHT_DATA_T *pCtrlResult -> calc result}
 * @attention: none
 * @retval: OPERATE_LIGHT
 */
STATIC VOID vLightCtrlDefColorBrightCalc(IN CTRL_DEF_COLOR_E eColor, IN USHORT_T usBright, IN USHORT_T usTemper, OUT BRIGHT_DATA_T *pCtrlResult)
{
    IN USHORT_T usBrightTemp = 0;

    if((eColor >= DEF_COLOR_R) && (eColor <= DEF_COLOR_RGB)) {
        usBrightTemp = usLightCtrlDataCalcHSVLimit(usBright);
    }

    switch(eColor) {
        case DEF_COLOR_C:
        case DEF_COLOR_W:
            vLightCtrlDataCalcCW(usBright, usTemper, pCtrlResult);
            break;

        case DEF_COLOR_R:
            pCtrlResult ->usRed = usLightCtrlDataCalcGamma(DEF_COLOR_R, usBrightTemp);
            break;

        case DEF_COLOR_G:
            pCtrlResult ->usGreen = usLightCtrlDataCalcGamma(DEF_COLOR_G, usBrightTemp);
            break;

        case DEF_COLOR_B:
            pCtrlResult ->usBlue = usLightCtrlDataCalcGamma(DEF_COLOR_B, usBrightTemp);
            break;

        case DEF_COLOR_RGB:
            pCtrlResult ->usRed = usLightCtrlDataCalcGamma(DEF_COLOR_R, usBrightTemp);
            pCtrlResult ->usGreen = usLightCtrlDataCalcGamma(DEF_COLOR_G, usBrightTemp);
            pCtrlResult ->usBlue = usLightCtrlDataCalcGamma(DEF_COLOR_B, usBrightTemp);
            break;

        case DEF_COLOR_MAX:
        default:
            break;
    }
}

/**
 * @brief: light ctrl normal status(constantly bright) display proc
 * @param {type}
 * @retval: none
 */
STATIC VOID vLightCtrlNormalDisplay(VOID)
{
    OPERATE_LIGHT opRet = -1;
    BRIGHT_DATA_T tCtrlData;

    PR_DEBUG("normal display....");
    memset(&tCtrlData, 0, SIZEOF(BRIGHT_DATA_T));

    vLightShadeCtrlDisable();       /* stop all shade process! */

    vLightCtrlDefColorBrightCalc(tLightCfgData.eDefColor, tLightCfgData.usDefBright, tLightCfgData.usDefTemper, &tCtrlData);

    PR_DEBUG("normal display %d %d %d %d %d", tCtrlData.usRed, tCtrlData.usGreen, tCtrlData.usBlue, tCtrlData.usWhite, tCtrlData.usWarm);

    opRet = opLightSetRGBCW(tCtrlData.usRed, tCtrlData.usGreen, tCtrlData.usBlue, tCtrlData.usWhite, tCtrlData.usWarm);
    if(opRet != LIGHT_OK){
        PR_ERR("normal display set RGBCW error!");
    }
}


/// Light blink cycletime
STATIC UINT_T guiBlinkTimeMs = 0;

/**
 * @brief: light ctrl blink proc
 * @param {type}
 * @retval: none
 */
STATIC VOID vLightCtrlBlinkDisplay(VOID)
{
    OPERATE_LIGHT opRet = -1;
    STATIC UCHAR_T ucCnt = 0;       /* first blink off */
    BRIGHT_DATA_T tCtrlData;
    USHORT_T usBrightTemp;

    memset(&tCtrlData, 0, SIZEOF(BRIGHT_DATA_T));
    vLightShadeCtrlDisable();       /* stop all shade process! */

    if((tLightCfgData.eNetColor >= DEF_COLOR_R) && (tLightCfgData.eNetColor <= DEF_COLOR_RGB)) {
        usBrightTemp = usLightCtrlDataCalcHSVLimit(tLightCfgData.usNetBright);
    }

    switch(tLightCfgData.eNetColor) {
        case DEF_COLOR_C:
        case DEF_COLOR_W:
            vLightCtrlDataCalcCW(tLightCfgData.usNetBright, tLightCfgData.usNetTemper, &tCtrlData);
            if(BRIGHT_MODE_CW == tLightCfgData.eBrightMode) {
                tCtrlData.usWhite = (ucCnt % 2) ? tCtrlData.usWhite : 0;
                tCtrlData.usWarm = (ucCnt % 2) ? tCtrlData.usWarm : 0;
            } else if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {
                tCtrlData.usWhite = (ucCnt % 2) ? tCtrlData.usWhite : 0;
                tCtrlData.usWarm = tCtrlData.usWarm ;
            }
            break;

        case DEF_COLOR_R:
            tCtrlData.usRed = (ucCnt % 2) ? (usLightCtrlDataCalcGamma(DEF_COLOR_R, usBrightTemp)) : 0;
            break;

        case DEF_COLOR_G:
            tCtrlData.usGreen = (ucCnt % 2) ? (usLightCtrlDataCalcGamma(DEF_COLOR_G, usBrightTemp)) : 0;
            break;

        case DEF_COLOR_B:
            tCtrlData.usBlue = (ucCnt % 2) ? (usLightCtrlDataCalcGamma(DEF_COLOR_B, usBrightTemp)) : 0;
            break;

        case DEF_COLOR_RGB:
            tCtrlData.usRed = (ucCnt % 2) ? (usLightCtrlDataCalcGamma(DEF_COLOR_R, usBrightTemp)) : 0;
            tCtrlData.usGreen = (ucCnt % 2) ? (usLightCtrlDataCalcGamma(DEF_COLOR_G, usBrightTemp)) : 0;
            tCtrlData.usBlue = (ucCnt % 2) ? (usLightCtrlDataCalcGamma(DEF_COLOR_B, usBrightTemp)) : 0;
            break;

        case DEF_COLOR_MAX:
        default:
            break;
    }

    opRet = opLightSetRGBCW(tCtrlData.usRed, tCtrlData.usGreen, tCtrlData.usBlue, tCtrlData.usWhite, tCtrlData.usWarm);
    if(opRet != LIGHT_OK){
        PR_ERR("blink set RGBCW error!");
    }

    ucCnt++;

}

/**
 * @brief: light ctrl blink timer callback
 * @param {type}
 * @retval: none
 */
STATIC VOID vLightCtrlBlinkTimerCB(VOID)
{
    OPERATE_LIGHT opRet = -1;

    vLightCtrlBlinkDisplay();
    opRet = opUserSWTimerStart(BLINK_SW_TIMER, guiBlinkTimeMs, (VOID*)vLightCtrlBlinkTimerCB);
    if(opRet != LIGHT_OK) {
        PR_ERR("blink timer start error!");
    }

}

/**
 * @brief: start blink function
 * @param {IN UINT_T BlinkTimeMs -> blink on/off time, unit:ms}
 * @attention: blink display will as the parm
 *             -- eNetColor, usNetBright, usNetTempr in configuration.
 * @retval: none
 */
OPERATE_LIGHT opLightCtrlBlinkStart(IN UINT_T uiBlinkTimeMs)
{
    OPERATE_LIGHT opRet = -1;

    CHECK_INIT_OK();

    guiBlinkTimeMs = uiBlinkTimeMs;
    opRet = opUserSWTimerStart(BLINK_SW_TIMER, uiBlinkTimeMs, (VOID*)vLightCtrlBlinkTimerCB);
    if(opRet != LIGHT_OK) {
        PR_ERR("blink timer start error!");
    }

    return LIGHT_OK;
}

/**
 * @brief: stop blink
 * @param {type}
 * @attention: blink stop will directly go to normal status display
 *              normal status will bright as default bright parm
 *              -- DefColor, usDefBright, usDefTemper in configuration.
 * @retval: none
 */
OPERATE_LIGHT opLightCtrlBlinkStop(VOID)
{
    OPERATE_LIGHT opRet = -1;

    CHECK_INIT_OK();

    opRet = opUserSWTimerStop(BLINK_SW_TIMER);
    if(opRet != LIGHT_OK) {
        PR_ERR("blink timer stop error!");
    }
    vLightCtrlNormalDisplay();
    return LIGHT_OK;
}

#define BREATH_UNIT_TIME             2  //2s @attention: accurate time is 2100ms
#define BREATH_TIMER_CYCLE_MS        10 //ms

/// Light breath cycle cnt
STATIC UINT_T uiBreathCnt = 0;
STATIC UCHAR_T ucBreathRate = 0;

typedef enum {
    BREATH_INIT = 0,
    BREATH_WAIT_INIT_OK,
    BREATH_CHANGE,
    BREATH_OK
}BREATH_STEP_E;

/**
 * @brief: light ctrl breath proc(breath as net config color&bright)
 * @param {none}
 * @return: OPERATE_RET
 * @retval: none
 */
STATIC OPERATE_RET opLightCtrlBreathing(VOID)
{
   //y= 0.1x*x
   /* CONST SHORT_T sBreathParm[] = {  0,   0,   0,   0,   0,   1,   1,   1,   2,  2,
                                       3,   3,   4,   4,   5,   6,   6,   7,   8,  9,
                                      10,  11,  12,  13,  14,  16,  17,  18,  20, 21,
                                      23,  24,  26,  27,  29,  31,  32,  34,  36, 38,
                                      40,  42,  44,  46,  48,  51,  53,  55,  58, 60,
                                      63,  65,  68,  70,  73,  76,  78,  81,  84, 87,
                                      90,  93,  96,  99, 102, 106, 109, 112, 116, 119,
                                      123, 126, 130, 133, 137, 141, 144, 148, 152, 156,
                                      160, 164, 168, 172, 176, 181, 185, 189, 194, 198,
                                      203, 207, 212, 216, 221, 226, 230, 235, 240, 245,
                                      250, 255, 260, 265, 270, 276, 281, 286, 292, 297,
                                      303, 308, 314, 319, 325, 331, 336, 342, 348, 354,
                                      360, 366, 372, 378, 384, 391, 397, 403, 410, 416,
                                      423, 429, 436, 442, 449, 456, 462, 469, 476, 483,
                                      490, 497, 504, 511, 518, 526, 533, 540, 548, 555,
                                      563, 570, 578, 585, 593, 601, 608, 616, 624, 632,
                                      640, 648, 656, 664, 672, 681, 689, 697, 706, 714,
                                      723, 731, 740, 748, 757, 766, 774, 783, 792, 801,
                                      810, 819, 828, 837, 846, 856, 865, 874, 884, 893,
                                      903, 912, 922, 931, 941, 951, 960, 970, 980, 990,
                                     1000};
    */

    //Gaussian distribution
    CONST SHORT_T sBreathParm[]  = {1000, 999, 997, 994, 989, 982, 975, 966, 956, 944,
                                    932,  918, 903, 887, 870, 853, 834, 815, 795, 774,
                                    753,  731, 709, 687, 665, 642, 619, 596, 573, 551,
                                    528,  506, 484, 462, 440, 419, 399, 379, 359, 340,
                                    322,  304, 286, 269, 253, 238, 223, 209, 195, 182,
                                    170,  158, 147, 136, 126, 117, 108, 100,  92,  85,
                                     78,   71,  65,  60,  55,  50,  46,  41,  38,  34,
                                     31,   28,  25,  23,  21,  19,  17,  15,  13,  12,
                                     11,   10,   8,   8,   7,   6,   5,   5,   4,   4,
                                      3,    3,   2,   2,   2,   2,   1,   1,   1,   1, 1};

    OPERATE_RET opRet = -1;
    STATIC UCHAR_T ucBreathingStep = 0;
    STATIC USHORT_T usStandCnt = 0;
    STATIC USHORT_T usStandIndex = 0;
    STATIC UCHAR_T ucStandAgainCnt = 0;
    BRIGHT_DATA_T tCtrlData;
    USHORT_T usMax = 0;
    STATIC USHORT_T usMin = 0;
    USHORT_T usBrightTemp = 0;
    USHORT_T usTempIndex = 0;

    opRet = opUserSWTimerStart(BREATH_SW_TIMER, ucBreathRate * BREATH_TIMER_CYCLE_MS, (VOID*)opLightCtrlBreathing);
    if(opRet != OPRT_OK) {
        PR_ERR("breath timer restart error!");
        return LIGHT_COM_ERROR;
    }

    if(BREATH_INIT == ucBreathingStep) {  /* change to mid brightness firstly */

        vLightShadeCtrlDisable();       /* stop all shade process! */

        if((tLightCfgData.eNetColor >= DEF_COLOR_R) && (tLightCfgData.eNetColor <= DEF_COLOR_RGB)) {
            usMin = CTRL_RGB_BRIGHT_VALUE_MAX * ( (FLOAT_T) tLightCfgData.ucLimitRGBMin / 100.0 );
        } else {
            usMin = CTRL_CW_BRIGHT_VALUE_MAX * ( (FLOAT_T) tLightCfgData.ucLimitCWMin / 100.0 );
        }

        memset(&tLightCtrlHandle.tCurrVal, 0, SIZEOF(BRIGHT_DATA_T));
        vLightCtrlDefColorBrightCalc(tLightCfgData.eNetColor, usMin, tLightCfgData.usNetTemper, &tLightCtrlHandle.tCurrVal);

        opRet = opLightSetRGBCW(tLightCtrlHandle.tCurrVal.usRed,tLightCtrlHandle.tCurrVal.usGreen,tLightCtrlHandle.tCurrVal.usBlue,tLightCtrlHandle.tCurrVal.usWhite,tLightCtrlHandle.tCurrVal.usWarm);
        if(opRet != LIGHT_OK) {
            PR_ERR("breath set RGBCW error!");
        }
        ucBreathingStep = BREATH_WAIT_INIT_OK;
    }

    if(BREATH_WAIT_INIT_OK == ucBreathingStep) {

       /* usStandIndex ++;
        if(usStandIndex >= (SHADE_CHANG_MAX_TIME/BREATH_TIMER_CYCLE_MS)) {   // wait change to breath start brightness firstly
            usStandIndex = 0;
            ucBreathingStep = 2;
        }
        */
        usStandIndex = 0;
        ucBreathingStep = BREATH_CHANGE;
    } else if(BREATH_CHANGE == ucBreathingStep) {

        memset(&tCtrlData, 0, SIZEOF(BRIGHT_DATA_T));

        if(ucStandAgainCnt < 1) {
            usTempIndex = ((SIZEOF(sBreathParm)/SIZEOF(USHORT_T)) - usStandIndex - 1);  /*  */
        } else {
            usTempIndex = usStandIndex;
        }

        usBrightTemp = (USHORT_T) (sBreathParm[usTempIndex] /1000.0 * (tLightCfgData.usNetBright - usMin)) + usMin ;
        vLightCtrlDefColorBrightCalc(tLightCfgData.eNetColor, usBrightTemp, tLightCfgData.usNetTemper, &tCtrlData);

        usStandIndex ++;
        if(usStandIndex >= (SIZEOF(sBreathParm)/SIZEOF(USHORT_T))) {
            usStandIndex = 0;
            ucStandAgainCnt++;
            if(ucStandAgainCnt >= 2) {
                ucBreathingStep = BREATH_OK;
                ucStandAgainCnt = 0;
            } else {
                 ucBreathingStep = BREATH_CHANGE;
            }

        }

         /* PR_NOTICE("target %d %d %d %d %d", tCtrlData.usRed, \
                tCtrlData.usGreen,\
                tCtrlData.usBlue,\
                tCtrlData.usWhite,\
                tCtrlData.usWarm);
         */


        opRet = opLightSetRGBCW(tCtrlData.usRed,tCtrlData.usGreen,tCtrlData.usBlue,tCtrlData.usWhite,tCtrlData.usWarm);
        if(opRet != LIGHT_OK) {
            PR_ERR("breath set RGBCW error!");
        }

    }else if(ucBreathingStep == BREATH_OK) {
        usStandIndex ++;

        if(usStandIndex < 2) {  /* wait 2*BREATH_TIMER_CYCLE_MS (ms) */
            ;
        } else {
            usStandIndex = 1;
            usStandCnt++;
            if(usStandCnt >= uiBreathCnt) { // breath unit cnt ok
            //@attention: if set alway breath, the uiBreathCnt will set 0xFFFFFFFF or (0xFFFFFFFF / 3) (bigger than 65535)
                ucBreathingStep = BREATH_INIT;
                usStandCnt = 0;
                usStandIndex = 0;

                memset(&tLightCtrlHandle.tTargetVal, 0, SIZEOF(BRIGHT_DATA_T));
                vLightCtrlDefColorBrightCalc(tLightCfgData.eNetColor, tLightCfgData.usDefBright, tLightCfgData.usNetTemper, &tLightCtrlHandle.tTargetVal);

                vLightCtrlShadeStepGainSet(SHADE_STEP_GAIN_DEFAULT);
                vLightCtrlShadeStart(LIGHT_SHADE_CYCLE);
                PR_DEBUG("Breath ok!!! stop!!!!!");
                opRet = opUserSWTimerStop(BREATH_SW_TIMER);
                if(opRet != LIGHT_OK) {
                    PR_ERR("blink timer stop error!");
                }
                return LIGHT_OK;
            } else {
                ucBreathingStep = BREATH_CHANGE;
            }
        }
    }

    return LIGHT_OK;
}

/**
 * @brief: start breathing function
 * @param {IN UINT_T BreathTimeMs -> Branth up/down time, unit:ms}
 * @attention: breath display will as the parm
 *             -- NetColor, usNetBright, usNetTemper in configuration.
 * @retval: OPERATE_RET
 */
OPERATE_RET opLightCtrlBreathingStart(IN UCHAR_T BreathRate, IN UINT_T BreathCnt)
{
    OPERATE_RET opRet = -1;

    CHECK_INIT_OK();

    ucBreathRate = (BreathRate ==  PAIRING_NORMAL_BLINK_FREQ) ? 1 : 3;
    uiBreathCnt = BreathCnt / ucBreathRate;

    PR_DEBUG("breath cnt %d",uiBreathCnt);
    opRet = opUserSWTimerStart(BREATH_SW_TIMER, BREATH_TIMER_CYCLE_MS, (VOID*)opLightCtrlBreathing);
    if(opRet != LIGHT_OK) {
        PR_ERR("breath timer stop error!");
        return LIGHT_COM_ERROR;
    }

    return LIGHT_OK;
}

/**
 * @brief: stop breathing
 * @param {type}
 * @attention: Breathing stop will directly go to normal status display
 *             normal status will bright as default bright parm
 *              -- DefColor, usDefBright, usDefTemper in configuration.
 * @retval: OPERATE_RET
 */
OPERATE_RET opLightCtrlBreathingStop(VOID)
{
    OPERATE_RET opRet = -1;

    CHECK_INIT_OK();

    opRet = opUserSWTimerStop(BREATH_SW_TIMER);
    if(opRet != LIGHT_OK) {
        PR_ERR("breath timer stop error!");
        return LIGHT_COM_ERROR;
    }

    vLightCtrlNormalDisplay();
    return LIGHT_OK;
}

/**
 * @brief: start blink auto according to pairing remind mode(breath or blink)
 * @param {IN UINT_T BlinkTimeMs -> blink on/off time, unit:ms}
 * @attention: blink or breath display will as the parm
 *             -- NetColor, usNetBright, usNetTempr in configuration.
 * @attention: breath time is don't set by BlinkTimeMs, but set by LightCfgData.usRemindTime
 * @retval: none
 */
OPERATE_RET opLightCtrlAutoBlinkStart(IN UINT_T uiBlinkTimeMs)
{
    OPERATE_RET opRet = -1;

    CHECK_INIT_OK();

    if(!tLightCfgData.ucRemindMode) {    /* blink when pairing */
        opRet = opLightCtrlBlinkStart(uiBlinkTimeMs);
        if(opRet != OPRT_OK) {
            PR_ERR("start blink timer error!");
        }

    } else {    /* breath when pairing */
        if(tLightCfgData.usRemindTime >= BREATH_ALWAY_ON_TIME) {
            opRet = opLightCtrlBreathingStart(uiBlinkTimeMs, BREATH_ALWAY_ON_CNT);
        } else {
            opRet = opLightCtrlBreathingStart(uiBlinkTimeMs, tLightCfgData.usRemindTime / BREATH_UNIT_TIME);
        }
        if(opRet != OPRT_OK) {
            PR_ERR("start breath timer error!");
        }
    }

    return opRet;
}


/**
 * @brief: stop blink or breath according to pairing remind mode
 * @param {type}
 * @retval: none
 */
OPERATE_RET opLightCtrlAutoBlinkStop(VOID)
{
    OPERATE_RET opRet = -1;

    CHECK_INIT_OK();

    if(!tLightCfgData.ucRemindMode) {    /* blink when pairing */
        opRet = opLightCtrlBlinkStop();
        if(opRet != OPRT_OK) {
            PR_ERR("blink timer stop error!");
            return OPRT_COM_ERROR;
        }
    }else{
        opRet = opLightCtrlBreathingStop();
        if(opRet != OPRT_OK) {
            PR_ERR("breath timer stop error!");
            return OPRT_COM_ERROR;
        }
    }

    return OPRT_OK;
}

#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)

/// Light low power active flag
STATIC BOOL_T bLowPowerActive = FALSE;

/**
 * @brief: get low power mode cfg
 * @param {none}
 * @retval: UCHAR_T
 */
UCHAR_T ucLightCtrlGetPowerLowMode(VOID)
{
    return (UCHAR_T) tLightCfgData.bTitle20;
}

/*
 * @brief: light ctrl exist lowPower proc
 * @param {none}
 * @attention: this function need to implement in system
 *              if system need to realize lowpower mode,
 *              system need to inmplement the flow function.
 *              exist lowpower function should turn on hardware time and
 *              disable system special lowpower API.
 * @retval: none
 */
WEAK VOID vLightSysCtrlLowPowerExist(VOID)
{
    ;
}

/*
* @brief: light ctrl exist lowPower proc
* @param {none}
* @attention: this function need to implement in system
*              if system need to realize lowpower mode,
*              system need to inmplement the flow function.
*              enter lowpower function should shut down hardware time and
*              enable system special lowpower API.
* @retval: none
*/
WEAK VOID vLightSysCtrlLowPowerEnter(VOID)
{
    ;
}

/**
 * @brief: lowpower enter process
 * @param {none}
 * @retval: none
 */
STATIC OPERATE_LIGHT opLightCtrlLowPowerEnterCB(VOID)
{
    OPERATE_LIGHT opRet = -1;

    if(BRIGHT_MODE_CCT == tLightCfgData.eBrightMode) {   /* CCT drive mode make sure don't change warm */
        opRet = opLightSetRGBCW(0,0,0,0,0);
        if(opRet != LIGHT_OK) {
            PR_ERR("CCT shut down error!");
        }
    }

    vLightSysCtrlLowPowerEnter();       /* system special process, tickless processs for example */
    PR_DEBUG("enter lowpower!");
    bLowPowerActive = TRUE;             /* attention: as base code use counting semaphore to paired */
    opRet = opUserSWTimerStop(LOWPOWER_SW_TIMER);   /* stop timer dominantly */
    if(opRet != LIGHT_OK) {
        PR_ERR("lowpower timer stop error!");
        return LIGHT_COM_ERROR;
    }

    return LIGHT_OK;
}

/**
 * @brief: lowpower deal process
 * @param {IN UCHAR_T ucONOFF}
 * @retval: none
 */
STATIC OPERATE_LIGHT opLightCtrlLowPowerProc(IN UCHAR_T ucONOFF)
{
    OPERATE_LIGHT opRet = -1;

    CHECK_INIT_OK();

    if(ucONOFF == TRUE) {       /* switch on status */
        opRet = opUserSWTimerStop(LOWPOWER_SW_TIMER);
        if(opRet != LIGHT_OK) {
            PR_ERR("lowpower timer stop error!");
            return LIGHT_COM_ERROR;
        }

        if(bLowPowerActive) {
            vLightSysCtrlLowPowerExist();
            PR_DEBUG("exist lowpower!");
            bLowPowerActive = FALSE;
        }
    } else {                    /* switch off status */
        PR_DEBUG("start lowpower timer!");
        opRet = opUserSWTimerStart(LOWPOWER_SW_TIMER, LOWPOWER_TIMER_CYCLE_MS, (VOID*)opLightCtrlLowPowerEnterCB);
        if(opRet != LIGHT_OK) {
            PR_ERR("lowpower timer start error!");
            return LIGHT_COM_ERROR;
        }
    }

    return LIGHT_OK;
}
#endif



