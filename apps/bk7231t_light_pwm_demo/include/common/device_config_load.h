/**
 * @Author: jinlu
 * @file name: device_config_load.h
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 */
#ifndef __DEVICE_CONFIG_LOAD__
#define __DEVICE_CONFIG_LOAD__

#include "stdio.h"
#include "stdlib.h"
#include "light_types.h"
#include "light_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

typedef enum{
    CMOD_C = 0x01,
    CMOD_CW = 0x02,
    CMOD_RGB = 0x03,
    CMOD_RGBC = 0x04,
    CMOD_RGBCW = 0x05
}CMODE_T;

typedef enum{
    DMOD_PWM = 0x00,
    DMOD_IIC_SM16726  = 0x01,
    DMOD_IIC_SM2135 = 0x02,
    DMOD_IIC_SM2135EH = 0x03,
    DMOD_IIC_SM2135EJ = 0x04,
    DMOD_IIC_NP1658CJ = 0x05
}DMODE_T;

typedef enum{
    GWCM_OLD = 0x00,
    GWCM_LOW_POWER = 0x01,
    GWCM_SPCL_MODE = 0x02,
    GWCM_OLD_PROD = 0x03,
    GWCM_SPCL_AUTOCFG = 0x05
}WFCFG_T;

typedef enum{
    COLOR_C = 0x00,
    COLOR_W = 0x01,
    COLOR_R = 0x02,
    COLOR_G = 0x03,
    COLOR_B = 0x04,
    COLOR_RGB = 0x05
}COLOR_T;

typedef enum{
    CW_TYPE = 0,
    CCT_TYPE = 1
}CWTYPE_T;

typedef enum{
    CHANGE_GRADUALLY = 0,
    CHANGE_DIRECTLY = 1 
}ONOFFMODE_T;

typedef enum{
    MEM_SAVE_NOT = 0,
    MEM_SAVE = 1 
}MEMORY_T;

#pragma pack(1)
typedef struct {
//common
    CHAR_T Jsonver[5];          // json version
    CHAR_T category[5];         // ble(sigmesh) dedicated

//device function 
    CHAR_T  module[15];         // module choosed for the light
    CHAR_T  cmod[6];            // Color model: 1 -> C; 2 -> CW; 3 -> RGB; 4 -> RGBC; 5 ->RGBCW;
    UCHAR_T dmod;               // Color driver mode: 0->pwm; 1->sm16726b; 2->sm2135;
    UCHAR_T cwtype;             // Color temperature drive mode: 0 -> CW; 1: -> CCT;
    UCHAR_T onoffmode;          // Is there a gradient when switching: 0 -> turn on gradually; 1 -> turn ondirectly;
    UCHAR_T pmemory;            // Is there a power-off memory: 1 -> save app data; 0 -> don't save
    UCHAR_T ctrl_pin;           // CTRL pin: 
    UCHAR_T ctrl_lv;            // CTRL pin level
    
//light config
    CHAR_T  defcolor;           // light color after connected
    UCHAR_T defbright;          // light brightness after connected
    UCHAR_T deftemp;            // light default temperature
    UCHAR_T cwmaxp;             // Maximum Power configuration of Cold and warm Light mixing
    UCHAR_T brightmin;          // Minimum brightness: 1~100
    UCHAR_T brightmax;          // Maximum brightness: 1~100
    UCHAR_T colormax;           // Minimum brightness: 1~100
    UCHAR_T colormin;           // Maximum brightness: 1~100

//connection config
    CHAR_T  wfcfg[10];          // Low power / flash, value: spcl,prod
    UCHAR_T remdmode;           // light reset pairing mode
    UCHAR_T rstnum;             // number of times required to reset by switching on and off
    CHAR_T  rstcor;             // light color while connecting
    UCHAR_T rstbr;              // light brightness while connecting
    UCHAR_T rsttemp;            // light brightness while connecting
    USHORT_T remdtime;          // light reset pairing reminde time,unit:sec
    UCHAR_T wfptime;            // light pairing time,unit:min

//pwm config
    UINT_T  pwmhz;              // PWM frequency
    UCHAR_T r_pin;              // Red color control pin
    UCHAR_T r_lv;               // Red color control pin level
    UCHAR_T g_pin;              // Green color control pin
    UCHAR_T g_lv;               // Green color control pin level
    UCHAR_T b_pin;              // Blue  color control pin
    UCHAR_T b_lv;               // Blue  color control pin level
    UCHAR_T c_pin;              // Cold white color control pin
    UCHAR_T c_lv;               // Cold white color control pin level
    UCHAR_T w_pin;              // Warm white color control pin
    UCHAR_T w_lv;               // Warm white color control pin level

    
#if(LIGHT_CFG_SUPPORT_LOWPOWER == 1)
    UCHAR_T title20;            // title20 flag: 0 -> don't support T20; 1 -> support T20
#endif

#if LIGHT_CFG_ENABLE_GAMMA
//Gamma param
    UCHAR_T ucGammaRedbuf;
    UCHAR_T ucGammaGreenbuf;
    UCHAR_T ucGammaBluebuf;
#endif

#if (LIGHT_CFG_GAMMA_CAL == 1)
    UCHAR_T gammakr;
    UCHAR_T gammakg;
    UCHAR_T gammakb;

    UCHAR_T gammawr;
    UCHAR_T gammawg;
    UCHAR_T gammawb;
#endif

}DEVICE_CONFIG_T;
#pragma pack()

/**
 * @brief: set device configuration data.
 * @param {IN DEVICE_CONFIG_T *ptConfig -> light cfg data} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opDeviceCfgDataDefaultSet(IN DEVICE_CONFIG_T *ptConfig);

/**
 * @brief: set device configuration data.
 * @param {IN USHORT_T usLen -> oem cfg len} 
 * @param {IN CHAR_T *pConfig -> oem cfg data} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opDeviceCfgDataSet(IN USHORT_T usLen, IN CONST CHAR_T *pConfig);

/**
 * @brief: load device configuration data.
 * @param {none} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opDeviceCfgDataLoad(VOID);

/**
 * @brief: get json version string
 * @param {out UCHAR_T *pJsonVer -> json version string }
 * @retval: OPERATE_LIGHT 
 */
CHAR_T cDeviceCfgGetJsonVer(OUT CHAR_T *pJsonVer);

/**
 * @brief: get json config data :category
 * @param {OUT CHAR_T *pCategory -> category}
 * @retval: OPERATE_LIGHT
 */
CHAR_T cDeviceCfgGetCategory(OUT CHAR_T *pCategory);

/**
 * @brief: get json config data: module name string.
 * @param {out} module: module name string.
 * @param {out} len: length of module name string.
 * @retval: OPERATE_LIGHT
 */
CHAR_T cDeviceCfgGetModule(OUT CHAR_T *pModule, OUT UCHAR_T *pLen);

/**
 * @brief: get color mode -- production is RGBCW/RGBC/RGB/CW/C
 * @param {type} none
 * @return: 
 *          0x01 -> C, 
 *          0x02 -> CW
 *          0x03 -> RGB
 *          0x04 -> RGBC
 *          0x05 -> RGBCW
 *          -1   -> not find light way,error
 */
CHAR_T cDeviceCfgGetColorMode(VOID);

/**
 * @brief: get json config data: driver mode
 * @param {type} none
 * @return: 
 *          0x00 -> PWM
 *          0x01 -> SM16726B
 *          0x02 -> SM2135
 *          -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetDriverMode(VOID);

/**
 * @brief: get json config data: cwtype
 * @param {type} none
 * @return: 
 *          0x00 -> CW drive
 *          0x01 -> CCT drive
 *          -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetCWType(VOID);

/**
 * @brief: get json config data: onoffmode
 * @param {type} none
 * @return: 
 *          0x00 -> turn on/off change gradually 
 *          0x01 -> turn on/off change directly
 *          -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetOnOffMode(VOID);

/**
 * @brief: get memory cfg -- if not save app control data
 * @param {type} none
 * @return: 
 *          0x00 -> don't save app data in flash
 *          0x01 -> save app data in flash
 *          -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetPmemoryCfg(VOID);

/**
 * @brief: get ctrl pin number
 * @param {type} none
 * @return: ctrl pin number,  -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetCtrlPin(VOID);

/**
 * @brief: get ctrl pin level
 * @param {type} none
 * @return: ctrl pin level,  -1 -> configure not load, error
 */
CHAR_T cDeviceCfgGetCtrlPinLevel(VOID);

/**
 * @brief: get title20 flag
 * @param {type} none
 * @return: 
 *          0x00 -> don't support lowpower mode
 *          0x01 -> support lowpower mode
 *          -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetTitle20(VOID);

/**
 * @brief: get json config data: color set when connected.
 * @param {type} none
 * @return: 
 *          0x00 -> default bright is C when connected.
 *          0x01 -> default bright is W when connected.
 *          0x02 -> default bright is R when connected.
 *          0x03 -> default bright is G when connected.
 *          0x04 -> default bright is B when connected.
 *          -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetDefColor(VOID);

/**
 * @brief: get json config data: brightness when connected.
 * @param {type} none
 * @return: brightness, -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetDefBrightness(VOID);

/**
 * @brief: get json config data: deftemp when connected.
 * @param {type} none
 * @return: default temperature when connected.  -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetDefTemperature(VOID);

/**
 * @brief: get json config data: white max power
 * @param {type} none
 * @return: white max power, -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetCWMaxPower(VOID);

/**
 * @brief: get the CW minimum brightness 
 * @param {type} none
 * @return: CW minimum brightness, -1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetWhiteMin(VOID);

/**
 * @brief: get the CW maxinum brightness
 * @param {type} none
 * @return: CW maxinum brightness,-1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetWhiteMax(VOID);

/**
 * @brief: get the RGB minimum brightness 
 * @param {type} none
 * @return: RGB mininum brightness,-1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetColorMin(VOID);


/**
 * @brief: get the RGB maxinum brightness 
 * @param {type} none
 * @return: RGB maxinum brightness,-1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetColorMax(VOID);

/**
 * @brief: get wifi pair mode config
 * @param {type} none
 * @return: 
 *          GWCM_OLD -> 
 *          GWCM_LOW_POWER ->
 *          GWCM_SPCL_MODE ->
 *          GWCM_OLD_PROD ->
 *          -1   -> not find, error
 */
CHAR_T cDeviceCfgGetWificfg(VOID);

/**
 * @brief: remind mode when pairing(blink or breath)
 * @param {type} none
 * @return:
 */
CHAR_T cDeviceCfgGetRemindMode(VOID);

/**
 * @brief: get pairing reset number.
 * @param {type} none
 * @return: reset number,-1   -> configure not load, error
 */
CHAR_T cDeviceCfgGetResetNum(VOID);

/**
 * @brief: get color set when pairing.
 * @param {type} none
 * @return: 
 *          0x00 -> default bright is C when pairing.
 *          0x01 -> default bright is W when pairing.
 *          0x02 -> default bright is R when pairing.
 *          0x03 -> default bright is G when pairing.
 *          0x04 -> default bright is B when pairing.
 *          -1   -> not find, error
 */
CHAR_T cDeviceCfgGetResetColor(VOID);

/**
 * @brief: get reset brightness when pairing
 * @param {type} none
 * @return: brightness when pairing,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetResetBrightness(VOID);

/**
 * @brief: get reset temperature(reset color is white) when pairing
 * @param {type} none
 * @return: reset temperature(reset color is white) when pairing,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetResetTemperature(VOID);

/**
 * @brief: get remind time when remind mode is breathing,unit:sec
 * @param {type} none
 * @return:remind time
 */
SHORT_T sDeviceCfgGetRemindTime(VOID);

/**
 * @brief: get pairing time,unit:min
 * @param {type} none
 * @return: pairting time
 */
CHAR_T cDeviceCfgGetPairingTime(VOID);

/**
 * @brief: get pwm frequency 
 * @param {type} none
 * @return: pwm frequency,-1   -> not find, error 
 */
USHORT_T usDeviceCfgGetPwmHz(VOID);

/**
 * @brief: get red pin number
 * @param {type} none
 * @return: red pin number,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetRedPin(VOID);

/**
 * @brief: get red pin effective level.
 * @param {type} none
 * @return: red pin effective level,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetRedPinLevel(VOID);

/**
 * @brief: get green pin number
 * @param {type} none
 * @return: green pin number,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetGreenPin(VOID);

/**
 * @brief: get green pin effective level.
 * @param {type} none
 * @return: green pin effective level,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetGreenPinLevel(VOID);

/**
 * @brief: get blue pin number
 * @param {type} none
 * @return: blue pin number,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetBluePin(VOID);

/**
 * @brief: get blue pin effective level.
 * @param {type} none
 * @return: blue pin effective level,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetBluePinLevel(VOID);

/**
 * @brief: get cold white pin number
 * @param {type} none
 * @return: cold white pin number,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetColdPin(VOID);

/**
 * @brief: get cold white pin pin effective level.
 * @param {type} none
 * @return: cold white pin effective level,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetColdPinLevel(VOID);

/**
 * @brief: get warm white pin number
 * @param {type} none
 * @return: warm white pin number
 */
CHAR_T cDeviceCfgGetWarmPin(VOID);

/**
 * @brief: get warm white effective level.
 * @param {type} none
 * @return: warm white pin effective level,-1   -> not find, error
 */
CHAR_T cDeviceCfgGetWarmPinLevel(VOID);


#if (LIGHT_CFG_ENABLE_GAMMA == 1)
/**
 * @brief: get red gamma 
 * @param {type} none
 * @return: red gamma
 */
UCHAR_T ucDeviceCfgGetGammaRed(IN UCHAR_T ucIndex);

/**
 * @brief: get green gamma 
 * @param {type} none
 * @return: green gamma
 */
UCHAR_T ucDeviceCfgGetGammaGreen(IN UCHAR_T ucIndex);


/**
 * @brief: get blue gamma 
 * @param {type} none
 * @return: blue gamma
 */
UCHAR_T ucDeviceCfgGetGammaBlue(IN UCHAR_T ucIndex);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __DEVICE_CONFIG_LOAD__ */



