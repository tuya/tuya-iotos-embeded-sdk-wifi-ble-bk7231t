/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: light_control.h
 * @Description: light control include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-26 13:55:40
 * @LastEditTime: 2020-03-06 12:35:19
 */

#ifndef __LIHGT_CONTROL_H__
#define __LIHGT_CONTROL_H__


#include "light_types.h"
#include "light_set_color.h"
#include "light_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/// hardware timer cycle (unit:us)
#define HW_TIMER_CYCLE_US           4000

/// shade change max time(the worst situation),uint:ms @attention: don't change arbitrary
#define SHADE_CHANG_MAX_TIME        800

/// scene head(scene Num) length
#define SCNE_HRAD_LENGTH            2
/// scene unit length
#define SCENE_UNIT_LENGTH           26
/// scene max unit number
#define SCENE_MAX_UNIT              8
/// scene data min length, formu -> 2+ 26 = 28
#define SCENE_MIN_LENGTH            (SCNE_HRAD_LENGTH + SCENE_UNIT_LENGTH)
/// scene data max length, formu -> 2 + 26*8  = 210
#define SCENE_MAX_LENGTH            (SCNE_HRAD_LENGTH + SCENE_UNIT_LENGTH * SCENE_MAX_UNIT)

/// scene C default scene ctrl data
#define SCENE_DATA_DEFAULT_C        "000e0d0000000000000000c803e8"
/// scene CW default scene ctrl data
#define SCENE_DATA_DEFAULT_CW       "000e0d0000000000000000c80000"
/// scene RGB/RGBC/RGBCW default scene ctrl data
#define SCENE_DATA_DEFAULT_RGB      "000e0d00002e03e802cc00000000"

#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)
#define LOWPOWER_TIMER_CYCLE_MS     3000 //ms
#endif

#define PAIRING_NORMAL_BLINK_FREQ   250
#define PAIRING_SLOW_BLINK_FREQ     1500
#define BREATH_ALWAY_ON_TIME        602         //@attention: breath alway on config time
#define BREATH_ALWAY_ON_CNT         0xFFFFFFFF   //@attention: this value to avoid breath stop!(bigger than 65535)


/**
 * @brief Light way type enum
 *          1~5 ways
 */
typedef enum {
    LIGHT_C = 1,
    LIGHT_CW,
    LIGHT_RGB,
    LIGHT_RGBC,
    LIGHT_RGBCW,
    LIGHT_MAX,
}CTRL_LIGHT_WAY_E;

typedef enum {
    DEF_COLOR_C = 0,
    DEF_COLOR_W,
    DEF_COLOR_R,
    DEF_COLOR_G,
    DEF_COLOR_B,
    DEF_COLOR_RGB,
    DEF_COLOR_MAX,
}CTRL_DEF_COLOR_E;

/**
 * @brief Light control switch change mode enum
 *          SWITCH_GRADUAL -> turn on/off gradually
 *          SWITCH_DIRECT  -> turn on/off directly
 */
typedef enum {
    SWITCH_GRADUAL = 0,
    SWITCH_DIRECT,
    SWITCH_MAX,
}CTRL_SWITCH_MODE_E;

/**
 * @brief Light control drive mode enum
 *          BRIGHT_MODE_CW  -> CW drive by pwm totally
 *          BRIGHT_MODE_CCT -> C value is bright setting essentially , w is the scale of C&W .
 *                          the light will send warm, when w is set by zero.
 */
typedef enum {
    BRIGHT_MODE_CW = 0,
    BRIGHT_MODE_CCT,
    BRIGHT_MODE_MAX,
}CTRL_BRIGHT_MODE_E;

/**
 * @brief Light control scene change mode enum
 *          SCENE_STATIC    -> scene hold on no change
 *          SCENE_JUMP      -> scene change by directly
 *          SCENE_SHADOW    -> scene change by gradually
 */
typedef enum {
    SCENE_STATIC = 0,
    SCENE_JUMP,
    SCENE_SHADE,
    SCENE_MAX,
}CTRL_SCENE_MODE_E;

typedef enum {
    RESET_MOD_CNT = 0,
}CTRL_RESET_MODE_E;             /* this mode can't choose in bulb! */

/**
 * @brief Light control need configuration sturct
 * the configuration need to set firstly ,when use the control frame!
 */
typedef struct
{
    CTRL_LIGHT_WAY_E    eLightWay;      /* light type -- 1/2/3/4/5 way light */
    CTRL_BRIGHT_MODE_E  eBrightMode;    /* CCT&CW drive mode -- 0:CW, 1:CCT */
    CTRL_SWITCH_MODE_E  eSwitchMode;    /* turn on/off mode -- 0:gradually, 1:directly */
    BOOL_T              bMemory;        /* ifnot save app control data flag -- 0: not save, 1:save */

#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)
    BOOL_T              bTitle20;       /* ifnot support T20 lowpower mode */
#endif

    CTRL_DEF_COLOR_E    eDefColor;      /* default color */
    USHORT_T            usDefBright;    /* default bright */
    USHORT_T            usDefTemper;    /* default temper */
    UCHAR_T             ucPowerGain;    /* power gain(amplification) */
    UCHAR_T             ucLimitCWMax;   /* CW limit Max value */
    UCHAR_T             ucLimitCWMin;   /* CW limit Min value */
    UCHAR_T             ucLimitRGBMax;  /* RGB limit Max value */
    UCHAR_T             ucLimitRGBMin;  /* RGB limit Min value */

    UCHAR_T             ucConnectMode;  /* connect cfg mode */
    UCHAR_T             ucResetCnt;     /* re distibute cnt */
    CTRL_DEF_COLOR_E    eNetColor;      /* the color when  */
    USHORT_T            usNetBright;    /* blink bright in connect proc */
    USHORT_T            usNetTemper;    /* blink temper in connect proc */
    UCHAR_T             ucRemindMode;   /* remind mode when pairing */
    USHORT_T            usRemindTime;   /* pairing blink time,unit:sec */
    UCHAR_T             ucPairTime;     /* pairing blink time,unit:min */
    DRIVER_CONFIG_T     tDriveCfg;      /* light drive hardware cfg */
}LIGHT_CTRL_CFG_T;

/**
 * @brief Light control data structure
 * storage the light control data(normal issued by app)
 */
typedef enum
{
    WHITE_MODE = 0,
    COLOR_MODE,
    SCENE_MODE,
    MUSIC_MODE,
    MODE_MAX,
}LIGHT_MODE_E;

/**
 * @brief software timer use id enum
 */
typedef enum {
    CLEAR_RESET_CNT_SW_TIMER = 0,
    BLINK_SW_TIMER,
    SCENE_SW_TIMER,
    AUTOSAVE_SW_TIMER,
    BREATH_SW_TIMER,
    COUNTDOWN_SW_TIMER,
    LOWPOWER_SW_TIMER,
    SCENE_AUTO_RESTART_TIMER,
    CCT_DELAY_SHUT_DOWN_TIMER,
    CCT_DELAY_RESET_TIMER,

#if 1//added by peter for IR light
    IR_JUMP_TIMER,

#endif
    /*********advance function************/
    RHYTHM_QUERY_TIMER,
    RHYTHM_AUTO_SAVE_TIMER,
    RHYTHM_DIMMING_TIMER,
    SLEEP_QUERY_TIMER,
    SLEEP_AUTO_SAVE_TIMER,
    SLEEP_DIMMING_TIMER,
    WAKE_QUERY_TIMER,
    WAKE_AUTO_SAVE_TIMER,
    WAKE_DIMMING_TIMER,
    SW_TIMER_MAX
}SW_TIMER_ID_E;

/**
 * @brief Light control color(RGB) data structure
 * storage the light control color(RGB) data(normal issued by app)
 */
typedef struct
{
    USHORT_T usRed;         /* color R setting */
    USHORT_T usGreen;
    USHORT_T usBlue;
}COLOR_RGB_T;

/**
 * @brief Light control color control original data structure
 */
typedef struct {
    USHORT_T usHue;     /* hue, range:0 ~ 360 */
    USHORT_T usSat;     /* saturation, range:0 ~ 1000 */
    USHORT_T usValue;   /* value, range:0 ~ 1000 */
    UCHAR_T ucColorStr[13];
}COLOR_ORIGIN_T;

/**
 * @brief Light control real time control change mode enum
 */
typedef enum {
    REALTIME_CHANGE_JUMP = 0,
    REALTIME_CHANGE_SHADE,
}REALTIME_CHANGE_E;

/**
 * @brief Light control data structure
 * storage the light control data(normal issued by app)
 */
typedef struct
{
    BOOL_T bSwitch;             /* on off setting */
    LIGHT_MODE_E eMode;
    USHORT_T usBright;
    USHORT_T usTemper;
    COLOR_RGB_T tColor;
    COLOR_ORIGIN_T tColorOrigin;
    CHAR_T cScene[SCENE_MAX_LENGTH + 1];
    BOOL_T bSceneFirstSet;
    UINT_T uiCountDown;
    CHAR_T cRealTimeData[22];
    UCHAR_T ucRealTimeFlag;
}LIGHT_CTRL_DATA_T;

/**
 * @brief Light control extend data structure
 */
typedef struct
{
    BOOL_T bSwitch;             /* on off setting */
    LIGHT_MODE_E Mode;
    USHORT_T usBright;
    USHORT_T usTemper;
    COLOR_RGB_T Color;
    COLOR_ORIGIN_T ColorOrigin;
}LIGHT_CTRL_EXT_DATA_T;

typedef struct
{
    CTRL_SCENE_MODE_E ChangeMode;
    UINT_T uiSpeed;
    UINT_T uiTimes;    /* unit:ms */
}LIGHT_SCENE_CTRL_T;


/**
 * @brief Bright 5ways value structure
 * Used in light gradually change calculation process
 */
typedef struct
{
    USHORT_T usRed;
    USHORT_T usGreen;
    USHORT_T usBlue;
    USHORT_T usWhite;
    USHORT_T usWarm;
}BRIGHT_DATA_T;

/**
 * @brief Light gradually change structure
 * Used in light gradually change calculation process
 */
typedef struct
{
    BRIGHT_DATA_T tTargetVal;
    BRIGHT_DATA_T tCurrVal;
    BOOL_T bFirstChange;    /* first change flag, need to calculate change step */
    UCHAR_T ucSceneUnit;
    UCHAR_T ucUnitCnt;
    BOOL_T bSceneSetFirstFlag;
    BOOL_T bSceneStopFlag;
}LIGHT_CTRL_HANDLE_T;


/**
 * @brief Light time structure
 * Used in light advance function about time
 */
typedef struct
{
    INT_T tm_sec;     /* seconds [0-59] */
    INT_T tm_min;     /* minutes [0-59] */
    INT_T tm_hour;    /* hours [0-23] */
    INT_T tm_mday;    /* day of the month [1-31] */
    INT_T tm_mon;     /* month [0-11] */
    INT_T tm_year;    /* year. The number of years since 1900 */
    INT_T tm_wday;    /* day of the week [0-6] 0-Sunday...6-Saturday */
}LIGHT_TM_T;

typedef BOOL_T (*bLightGetLocalTime)(OUT LIGHT_TM_T *ptResult);

/**
 * @brief: set light ctrl data to default according lightCfgData
 * @param {none}
 * @retval: none
 */
VOID vLightCtrlDataReset(VOID);

/**
 * @brief: Light system control hardware timer callback
 * @param {none}
 * @attention: this function need to implement by system,
 *              decide how to call vLightCtrlShadeGradually function.
 * @retval: none
 */
VOID vLightSysHWTimerCB(VOID);

/**
 * @brief: Light control hardware timer callback
 * @param {none}
 * @attention: vLightSysHWTimerCB() func need to implement by system
 * @retval: none
 */
VOID vLightCtrlHWTimerCB(VOID);

/**
 * @brief: light control init
 * @param {IN LIGHT_CTRL_CFG_T *pConfig_Data -> init parm}
 * @attention: this function need apply bLightSysHWRebootJudge();
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlInit(IN LIGHT_CTRL_CFG_T *pConfigData);

/**
 * @brief: get change gradually process the max error of 5ways
 *          this func will calc the error between LightCtrlHandle.Target
 *          and LightCtrlHandle.Current, and change.
 * @param {none}
 * @attention: this func need to called by period
 * @retval: none
 */
VOID vLightCtrlShadeGradually(VOID);

/**
 * @brief: reponse switch property process,
 *          this need to implement by system.
 * @param {OUT BOOL_T bONOFF -> switch status, TRUE is ON}
 * @retval: none
 */
VOID vLightCtrlDataSwitchRespone(OUT BOOL_T bONOFF);

/**
 * @brief: set light switch data, adapte control data issued by system
 *          to control data format.
 * @param {IN BOOL_T bONOFF -> switch data, TRUE will turn on}
 * @retval: OPERATE_LIGHT -> OPRT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataSwitchSet(IN BOOL_T bONOFF);

/**
 * @brief: reponse mode property process,
 *          this need to implement by system.
 * @param {OUT LIGHT_MODE_E Mode}
 * @retval: none
 */
VOID vLightCtrlDataModeResponse(OUT LIGHT_MODE_E eMode);

/**
 * @brief: set light mode data
 * @param {IN LIGHT_MODE_E Mode}
 * @attention:Mode value is below:
 *                                  WHITE_MODE = 0,
 *                                  COLOR_MODE = 1,
 *                                  SCENE_MODE = 2,
 *                                  MUSIC_MODE = 3,
 * @retval: OPERATE_LIGHT -> OPRT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataModeSet(IN LIGHT_MODE_E Mode);

/**
 * @brief: reponse bright property process,
 *          this need to implement by system.
 * @param {OUT LIGHT_MODE_E eMode}
 * @param {OUT USHORT_T usBright}
 * @attention: need reponse mode property,as set bright value, will auto set the Mode to WHITE_MODE!
 * @retval: none
 */
VOID vLightCtrlDataBrightResponse(OUT LIGHT_MODE_E eMode, OUT USHORT_T usBright);

/**
 * @brief: set light bright data, adapte control data issued by system
 *          to control data format.
 * @param {IN USHORT_T usBright}
 * @attention: acceptable range:10~1000
 * @attention: set bright value, will auto set the Mode to WHITE_MODE !
 * @retval: OPERATE_LIGHT -> OPRT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataBrightSet(IN USHORT_T usBright);

/**
 * @brief: reponse temperature property process,
 *          this need to implement by system.
 * @param {OUT LIGHT_MODE_E eMode}
 * @param {OUT USHORT_T usTemperature}
 * @attention: need reponse mode property,as set temperature value, will auto set the Mode to WHITE_MODE!
 * @retval: none
 */
VOID vLightCtrlDataTemperatureResponse(OUT LIGHT_MODE_E eMode, OUT USHORT_T usTemperature);

/**
 * @brief: set light temrperature data, adapte control data issued by system
 *          to control data format.
 * @param {IN USHORT_T usTemperature}
 * @attention: acceptable range:0~1000
 * @retval: OPERATE_LIGHT -> OPRT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataTemperatureSet(IN USHORT_T usTemperature);

/**
 * @brief: reponse RGB property process,
 *          this need to implement by system
 * @param {OUT COLOR_ORIGIN_T *ptColorOrigin}
 * @retval: none
 */
VOID vLightCtrlDataRGBResponse(OUT COLOR_ORIGIN_T *ptColorOrigin);

/**
 * @brief: set light RGB data
 * @param {IN COLOR_RGB_T *ptColor}
 * @param {IN COLOR_ORIGINAL_T *ptColorOrigin -> color origin data save}
 * @attention: acceptable format is RGB module, R,G,B range：0~1000
 * @retval: OPERATE_LIGHT -> LIGHT_OK meaning need to call opLightCtrlProc() function!
 */
OPERATE_LIGHT opLightCtrlDataRGBSet(IN COLOR_RGB_T *ptColor, IN COLOR_ORIGIN_T *ptColorOrigin);

/**
 * @brief: reponse scene property process,
 *          this need to implement by system
 * @param {OUT CHAR_T *pSceneData}
 * @retval: none
 */
VOID vLightCtrlDataSceneResponse(OUT CHAR_T *pSceneData);

/**
 * @brief: set light scene data
 * @param {IN UCHAR_T *pSceneData}
 * @attention: SceneData format: please reference to DP protocol
 * @retval: none
 */
OPERATE_LIGHT opLightCtrlDataSceneSet(IN CHAR_T *pSceneData);

/**
 * @brief: get light switch data
 * @param {OUT BOOL_T *pONOFF -> switch data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataSwitchGet(OUT BOOL_T *pONOFF);

/**
 * @berief: get light time count down data
 * @param {OUT BOOL_T *pCountDown -> time count down data return}
 * @retval: OPERATE_RET
 */
OPERATE_RET opLightCtrlDataCountDownGet(OUT UINT_T *pCountDown);

/**
 * @brief: get light mode data
 * @param {OUT LIGHT_MODE_E *pMode -> mode data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataModeGet(OUT LIGHT_MODE_E *pMode);

/**
 * @brief: geta light bright data
 * @param {OUT USHORT_T *pBright -> bright data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataBrightGet(OUT USHORT_T *pBright);

/**
 * @brief: get light temrperature data
 * @param {OUT USHORT_T *pTemperature -> temperature data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataTemperatureGet(OUT USHORT_T *pTemperature);


/**
 * @brief: get light RGB data & original data
 * @param {OUT COLOR_RGB_T *ptColor -> color data return}
 * @param {OUT COLOR_ORIGIN_T *ptOriginalColor -> color original data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataRGBGet(OUT COLOR_ORIGIN_T *ptOriginalColor);

/**
 * @brief: get light scene data
 * @param {OUT CHAR_T *pSceneData -> scene data return}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataSceneGet(OUT CHAR_T *pSceneData);

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
OPERATE_LIGHT opLightCtrlDataExtSet(IN LIGHT_CTRL_EXT_DATA_T *ptExtData, IN BOOL_T bNeedUpload, IN BOOL_T bActiveImmed, IN BOOL_T bNeedSave);

/**
 * @berief: Light control proc
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlProc(VOID);

/**
 * @brief: light ctrl disable
 * @param {none}
 * @retval: none
 */
VOID vLightCtrlDisable(VOID);

/**
 * @brief: shade process disable
 * @attention:
 * @retval: none
 */
VOID vLightShadeCtrlDisable(VOID);

/**
 * @brief: countdown lave time return proc
 * @param {OUT UINT_T uiRemainTimeSec -> left time,unit:sec}
 * @attention: this function need to implement in system
 * @retval: none
 */
VOID vLightCtrlDataCountDownResponse(OUT UINT_T uiRemainTimeSec);

/**
 * @brief: set light countdown value
 * @param {IN INT_T uiCountDownSec -> unit:second}
 * @attention: countdown lave time will return with
 *              calling vLightDataCountDownResponse function every minutes.
 *              switch status will return with calling
 *              vLightCtrlDataSwitchRespone function when countdown active.
 * @retval: OPERATE_LIGHT -> OPRT_OK set countdown OK.
 */
OPERATE_LIGHT opLightCtrlDataCountDownSet(IN INT_T uiCountDownSec);

/**
* @brief: set light realtime control data
* @param {IN BOOL_T bMusicFlag}
* @param {IN CHAR_T *pRealTimeData}
* @attention: data format: please reference to DP protocol
* @retval: OPERATE_LIGHT -> LIGHT_OK need to call opLightRealTimeCtrlProc function.
*/
OPERATE_LIGHT opLightCtrlDataRealTimeAdjustSet(IN BOOL_T bMusicFlag, IN CHAR_T *pRealTimeData);

 /**
 * @brief: Light realtime ctrl process
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightRealTimeCtrlProc(VOID);

/**
 * @brief: system reboot as hardware mode jude proc
 * @param {none}
 * @attention: this function need to implement by each plantform.
 * @retval: BOOL_T TRUE -> system reboot
 */
extern BOOL_T bLightSysHWRebootJudge(VOID);

/**
 * @brief: Light hardware reboot judge & proc
 *          process detail:
 *                  1. hardware reset judge;
 *                  2. load reboot cnt data;
 *                  3. reboot cnt data increase;
 *                  4. start software time to clear reboot cnt;
 * @param {none}
 * @attention: this function need bLightSysHWRebootJudge();
 * @retval: none
 */
VOID vLightCtrlHWRebootProc(VOID);

/**
 * @brief: Light ctrl data save
 * @param {none}
 * @attention: this function directly save ctrl data.
 * @retval: none
 */
OPERATE_LIGHT opLightCtrlDataAutoSave(VOID);

/**
 * @brief: system reset proc
 * @param {none}
 * @attention: this function need implememt by system,
 *              need to deal with different thing in each plantform.
 * @retval: none
 */
OPERATE_LIGHT opLightSysResetCntOverCB(VOID);

/**
 * @berief: Light reset cnt clear
 * @param {none}
 * @attention: this func will call in opLightCtrlResetCntProcess
 *              when no opLightSysResetCntOverCB finish callback, otherwise
 *               call in system!!!
 * @retval: none
 */
VOID vLightCtrlResetCntClear(VOID);

/**
 * @brief: Light reset to re-distribute proc
 * @param {none}
 * @attention: this func will call opLightSysResetCntOverCB()
 *              opLightSysResetCntOverCB() need to implement by system
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlResetCntProcess(VOID);

/**
 * @brief: get connect mode cfg
 * @param {none}
 * @retval: UCHAR_T connect mode
 */
UCHAR_T ucLightCtrlGetConnectMode(VOID);

/**
 * @brief: get color max limit
 * @param {none}
 * @retval: UCHAR_T
 */
UCHAR_T ucLightCtrlGetColorMax(VOID);

/**
 * @brief: get color min limit
 * @param {none}
 * @retval: UCHAR_T
 */
UCHAR_T ucLightCtrlGetColorMin(VOID);

/**
 * @brief: light ctrl data auto save proc
 * @param {IN UINT_T uiDelayTimeMs -> ctrl data auto save delay time,uint:ms}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlDataAutoSaveStart(IN UINT_T uiDelayTimeMs);


/**
 * @brief: start blink function
 * @param {IN UINT_T BlinkTimeMs -> blink on/off time, unit:ms}
 * @attention: blink display will as the parm
 *             -- NetColor, usNetBright, usNetTempr in configuration.
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlBlinkStart(IN UINT_T uiBlinkTimeMs);

/**
 * @brief: stop blink
 * @param {type}
 * @attention: blink stop will directly go to normal status display
 *              normal status will bright as default bright parm
 *              -- DefColor, usDefBright, usDefTemper in configuration.
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlBlinkStop(VOID);

/**
 * @brief: start breathing function
 * @param {IN UINT_T uiBreathTimeMs -> Branth up/down time, unit:ms}
 * @attention: breath display will as the parm
 *             -- NetColor, usNetBright, usNetTemper in configuration.
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlBreathingStart(IN UCHAR_T BreathRate, IN UINT_T BreathCnt);

/**
 * @brief: stop breathing
 * @param {none}
 * @attention: Breathing stop will directly go to normal status display
 *             normal status will bright as default bright parm
 *              -- eDefColor, usDefBright, usDefTemper in configuration.
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightCtrlBreathingStop(VOID);

/**
 * @brief: start blink auto according to pairing remind mode(breath or blink)
 * @param {IN UINT_T uiBlinkTimeMs -> blink on/off time, unit:ms}
 * @attention: blink or breath display will as the parm
 *             -- NetColor, usNetBright, usNetTempr in configuration.
 * @attention: breath time is don't set by BlinkTimeMs, but set by LightCfgData.usRemindTime
 * @retval: none
 */
OPERATE_RET opLightCtrlAutoBlinkStart(IN UINT_T uiBlinkTimeMs);

/**
 * @brief: stop blink or breath according to pairing remind mode
 * @param {type}
 * @retval: none
 */
OPERATE_RET opLightCtrlAutoBlinkStop(VOID);


#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)
/**
 * @brief: get low power mode cfg
 * @param {none}
 * @retval: UCHAR_T
 */
UCHAR_T ucLightCtrlGetPowerLowMode(VOID);

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
VOID vLightSysCtrlLowPowerExist(VOID);

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
VOID vLightSysCtrlLowPowerEnter(VOID);

#endif



#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __LIHGT_CONTROL_H__ */
