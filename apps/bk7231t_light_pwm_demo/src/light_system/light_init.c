/*
 * @Author: wuls
 * @email: wuls@tuya.com
 * @LastEditors: wls
 * @file name: light_init.c
 * @Description: light init process 
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-28 14:35:54
 * @LastEditTime: 2020-03-06 12:01:27
 */

#include "light_printf.h"
#include "light_types.h"
#include "light_control.h"
#include "device_config_load.h"
#include "light_system.h"

/// Light init flag
STATIC BOOL_T bLightInitFlag = FALSE;

#define CONFIG_SET_BY_USER  0

#if (CONFIG_SET_BY_USER == 1) /* user set config */
#define DEMO_LIGHT_WAY CMOD_C
#define DEMO_LIHGT_DRIVE_MODE DMOD_PWM
STATIC VOID vLightCfgSetDemo(VOID) 
{
    OPERATE_LIGHT opRet = -1;
    DEVICE_CONFIG_T tDeviceCfg;

    memset(&tDeviceCfg, 0, SIZEOF(tDeviceCfg));

#if (DEMO_LIGHT_WAY == CMOD_C)
    //light C 
    tDeviceCfg.cmod = CMOD_C;                       // don't modify! 
    tDeviceCfg.dmod = DMOD_PWM;                     // don't modify! 
    tDeviceCfg.cwtype = CW_TYPE;                    // don't modify! 
    tDeviceCfg.onoffmode = CHANGE_GRADUALLY;//CHANGE_DIRECTLY;
    tDeviceCfg.pmemory = MEM_SAVE;//MEM_SAVE_NOT;   

    tDeviceCfg.title20 = 0;//1;                     //this function need config support in light_cfg.h

    tDeviceCfg.defcolor = COLOR_C;                  // don't modify!
    tDeviceCfg.defbright = 100;                     // default brightness
    tDeviceCfg.deftemp = 100;                       // don't modify!
    tDeviceCfg.cwmaxp = 100;                        // don't modify!
    
    tDeviceCfg.brightmin = 10;                      // Minimum brightness: 1~100
    tDeviceCfg.brightmax = 100;                     // Maximum brightness: 1~100
    tDeviceCfg.colormax = 10;                       // don't modify!
    tDeviceCfg.colormin = 100;                      // don't modify!

    tDeviceCfg.pwmhz = 1000;                        //pwm phase 1k(unit:hz)
    tDeviceCfg.c_pin = 7;                           // this io need adjust by platform, io num
    tDeviceCfg.c_lv = 1;                            // high level active

    tDeviceCfg.ctrl_pin = PIN_NOEXIST;              // don't has ctrl pin

#endif

#if (DEMO_LIGHT_WAY == CMOD_CW)
    // light CW
    tDeviceCfg.cmod = CMOD_CW;                      // don't modify! 
    tDeviceCfg.dmod = DMOD_PWM;                     // don't modify! 
    tDeviceCfg.cwtype = CW_TYPE; //CCT_TYPE;        // CW or CCT 
    tDeviceCfg.onoffmode = CHANGE_GRADUALLY;//CHANGE_DIRECTLY;
    tDeviceCfg.pmemory = MEM_SAVE;//MEM_SAVE_NOT;

    tDeviceCfg.title20 = 0;//1;                     //this function need config support in light_cfg.h

    tDeviceCfg.defcolor = COLOR_C;                  // don't modify!
    tDeviceCfg.defbright = 100;                     // default brightness
    tDeviceCfg.deftemp = 100;                       // don't modify!
    tDeviceCfg.cwmaxp = 100;                        // don't modify!
    
    tDeviceCfg.brightmin = 10;                      // Minimum brightness: 1~100
    tDeviceCfg.brightmax = 100;                     // Maximum brightness: 1~100
    tDeviceCfg.colormax = 10;                       // don't modify!
    tDeviceCfg.colormin = 100;                      // don't modify!

    tDeviceCfg.pwmhz = 1000;                        //pwm phase 1k(unit:hz)
    tDeviceCfg.c_pin = 7;                           // this io need adjust by platform, io num
    tDeviceCfg.c_lv = 1;                            // high level active

    tDeviceCfg.w_pin = 8;                           // this io need adjust by platform, io num
    tDeviceCfg.w_lv = 1;                            // high level active

    tDeviceCfg.ctrl_pin = PIN_NOEXIST;              // don't has ctrl pin
#endif

#if (DEMO_LIGHT_WAY == CMOD_RGBCW)

    #if (DEMO_LIHGT_DRIVE_MODE DMOD_PWM == DMOD_PWM)
        // light RGB
        tDeviceCfg.cmod = CMOD_RGBCW;                  // don't modify! 
        tDeviceCfg.dmod = DMOD_PWM;                    
        tDeviceCfg.cwtype = CW_TYPE;//CCT_TYPE;         
        tDeviceCfg.onoffmode = CHANGE_GRADUALLY;//CHANGE_DIRECTLY;
        tDeviceCfg.pmemory = MEM_SAVE;//MEM_SAVE_NOT;

        tDeviceCfg.title20 = 0;//1;                     //this function need config support in light_cfg.h

        tDeviceCfg.defcolor = COLOR_C;//COLOR_W;//COLOR_R;//COLOR_G;//COLOR_B 
        tDeviceCfg.defbright = 100;
        tDeviceCfg.deftemp = 100;                       
        tDeviceCfg.cwmaxp = 100;                        // don't modify!
        
        tDeviceCfg.brightmin = 10;                      // Minimum brightness: 1~100
        tDeviceCfg.brightmax = 100;                     // Minimum brightness: 1~100
        tDeviceCfg.colormax = 10;                       // Minimum brightness: 1~100
        tDeviceCfg.colormin = 100;                      // Maximum brightness: 1~100

        tDeviceCfg.r_pin = 6;                           // this io need adjust by platform, io num
        tDeviceCfg.r_lv = 1;                            // high level active

        tDeviceCfg.g_pin = 7;                           // this io need adjust by platform, io num
        tDeviceCfg.g_lv = 1;                            // high level active

        tDeviceCfg.b_pin = 8;                           // this io need adjust by platform, io num
        tDeviceCfg.b_lv = 1;                            // high level active

        tDeviceCfg.c_pin = 19;                          // this io need adjust by platform, io num
        tDeviceCfg.c_lv = 1;                            // high level active

        tDeviceCfg.w_pin = 18;                          // this io need adjust by platform, io num
        tDeviceCfg.w_lv = 1;                            // high level active
        
        tDeviceCfg.ctrl_pin = PIN_NOEXIST;              // don't has ctrl pin
    #endif

    
#endif

    opRet = opDeviceCfgDataDefaultSet(&tDeviceCfg);
    if(opRet != LIGHT_OK) {
        PR_ERR("Default oem uConfig error!");
    }
}
#endif


/**
 * @brief: light hardware init
 *          get oem json set, and init hardware
 * @param {none} 
 * @return: OPERATE_LIGHT
 */
STATIC OPERATE_LIGHT opLightHardwareInit(VOID)
{
    OPERATE_LIGHT opRet = -1;

#ifdef _IS_OEM /* load config from flash! */
    /* get oem get json!! */
    /* if read failure,return */
    opRet = opDeviceCfgDataLoad();
    if(opRet != LIGHT_OK) {
        PR_NOTICE("oem cfg load error!");
        return LIGHT_COM_ERROR;
    }
#else /* set configuration by use */
    #if (CONFIG_SET_BY_USER == 1)
    vLightCfgSetDemo(); 
    #else
        opRet = opDeviceCfgDataSet(strlen(DEFAULT_CONFIG), DEFAULT_CONFIG);
        if(opRet != LIGHT_OK) {
            PR_ERR("Default oem uConfig error!");
        }
    #endif
#endif
    LIGHT_CTRL_CFG_T tLightConfigData = {0};
    
    /************ production cfg ************/
    tLightConfigData.eLightWay = cDeviceCfgGetColorMode();
    /************ production cfg ************/

    /************ pairing cfg ************/
    tLightConfigData.ucConnectMode = cDeviceCfgGetWificfg();
    tLightConfigData.ucResetCnt = cDeviceCfgGetResetNum();
    tLightConfigData.eNetColor = cDeviceCfgGetResetColor();
    tLightConfigData.usNetBright = cDeviceCfgGetResetBrightness();
    tLightConfigData.usNetTemper = cDeviceCfgGetResetTemperature();

    tLightConfigData.ucRemindMode = cDeviceCfgGetRemindMode();
    tLightConfigData.usRemindTime = sDeviceCfgGetRemindTime();
    tLightConfigData.ucPairTime = cDeviceCfgGetPairingTime();

    /************ pairing cfg ************/

    
    /************ production light cfg ************/
    tLightConfigData.eDefColor = cDeviceCfgGetDefColor();
    tLightConfigData.usDefBright = cDeviceCfgGetDefBrightness();
    tLightConfigData.usDefTemper = cDeviceCfgGetDefTemperature();
    tLightConfigData.eSwitchMode = cDeviceCfgGetOnOffMode();
    tLightConfigData.bMemory = cDeviceCfgGetPmemoryCfg();
    /************ production light cfg ************/
    
    /************ production hardware cfg ************/
    
    tLightConfigData.eBrightMode = cDeviceCfgGetCWType();
    tLightConfigData.ucLimitCWMax = cDeviceCfgGetWhiteMax();
    tLightConfigData.ucLimitCWMin = cDeviceCfgGetWhiteMin();
    tLightConfigData.ucLimitRGBMax = cDeviceCfgGetColorMax();
    tLightConfigData.ucLimitRGBMin = cDeviceCfgGetColorMin();
    
#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)
    tLightConfigData.bTitle20 = cDeviceCfgGetTitle20();
#endif

    tLightConfigData.tDriveCfg.eMode = cDeviceCfgGetDriverMode();
    tLightConfigData.ucPowerGain = cDeviceCfgGetCWMaxPower();
    
    switch(tLightConfigData.tDriveCfg.eMode) {
        case DRIVER_MODE_PWM:{
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.usFreq = usDeviceCfgGetPwmHz();
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.ucList[0] = cDeviceCfgGetRedPin();
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.ucList[1] = cDeviceCfgGetGreenPin();
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.ucList[2] = cDeviceCfgGetBluePin();
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.ucList[3] = cDeviceCfgGetColdPin();
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.ucList[4] = cDeviceCfgGetWarmPin();
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.ucChannelNum = cDeviceCfgGetColorMode();
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.bCCTFlag = cDeviceCfgGetCWType();
            
            if(tLightConfigData.eLightWay == LIGHT_RGB) {     /* rgb  */
                tLightConfigData.tDriveCfg.uConfig.tPwmInit.bPolarity = cDeviceCfgGetRedPinLevel();
                tLightConfigData.tDriveCfg.uConfig.tPwmInit.usDuty = (cDeviceCfgGetRedPinLevel() == 1) ?  0 : 1000;
            }else {
                tLightConfigData.tDriveCfg.uConfig.tPwmInit.bPolarity = cDeviceCfgGetColdPinLevel();
                tLightConfigData.tDriveCfg.uConfig.tPwmInit.usDuty = (cDeviceCfgGetColdPinLevel() == 1) ?  0 : 1000;
            }
            
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.ucCtrlPin = cDeviceCfgGetCtrlPin();
            tLightConfigData.tDriveCfg.uConfig.tPwmInit.bCtrlLevel = cDeviceCfgGetCtrlPinLevel();
            break;
        }
        
        default:{
            PR_ERR("Driver mode ERROR");
            break;
        }

    }

    opRet = opLightCtrlInit(&tLightConfigData);
    if(opRet != LIGHT_OK) {
        PR_ERR("Light control init error!");
        return LIGHT_COM_ERROR;
    }
    
    return LIGHT_OK;

}

/**
 * @brief: light software init
 *          this func will call opLightSysSoftwareInit()
 * @attention: opLightSysSoftwareInit need implement by system
 * @param {none} 
 * @return: OPERATE_LIGHT
 */
STATIC OPERATE_LIGHT opLightSoftwareInit(VOID)
{
    OPERATE_LIGHT opRet = -1;

    OPERATE_RET opLightSysSoftwareInit(VOID);
    opRet = opLightSysSoftwareInit();
    if(opRet != LIGHT_OK) {

        PR_ERR("Light software init error!");
    }
    
    return opRet;
}

/**
 * @brief: light init
 * @param {none} 
 * @return: OPERATE_LIGHT
 */
OPERATE_LIGHT opLightInit(VOID)
{
    OPERATE_LIGHT opRet = -1;

    if(bLightInitFlag) {
        PR_NOTICE("Light already init");
        return LIGHT_OK;
    }
    
    opRet = opLightHardwareInit();
    if(opRet != LIGHT_OK) {
        PR_ERR("Light hardware init error!");
        return LIGHT_COM_ERROR;
    }
    
    opRet = opLightSoftwareInit();
    if(opRet != LIGHT_OK) {
        PR_ERR("Light software init error!");
        return LIGHT_COM_ERROR;
    }

    bLightInitFlag = TRUE;
    
    return LIGHT_OK;
}


