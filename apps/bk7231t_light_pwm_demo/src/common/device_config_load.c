/*
 * @Author: jinlu
 * @email: jinlu@tuya.com
 * @LastEditors: wls
 * @file name: device_config_load.c
 * @Description: device oem cfg load  proc
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-24 20:42:29
 * @LastEditTime: 2020-03-06 11:28:09
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "device_config_load.h"
#include "user_flash.h"
#include "light_tools.h"
#include "light_printf.h"


#define CONFIG_DATA_LEN_MAX    1024
#define MAX_VALUE_SIZE         15 //+ 512
#define CHECK_LOAD_FINISH()    if(opDeviceCfgInitCheck() != LIGHT_OK) { \
                                    return LIGHT_COM_ERROR;\
                                }

#define PROD_AGAING_INVAILD_VALUE 0xFF

STATIC UCHAR_T ucOemConfigLoadFlag = FALSE;
STATIC DEVICE_CONFIG_T gtDeviceCfg;

typedef enum {
    TYPE_HEX = 0,
    TYPE_STRING,
}VALUE_TYPE;

typedef struct {
    CHAR_T *string;
    UCHAR_T enum_value;
}STRING_TO_ENUM_T;

STRING_TO_ENUM_T tCmodTable[] = 
{
    {"c", CMOD_C},
    {"cw", CMOD_CW},
    {"rgb", CMOD_RGB},
    {"rgbc", CMOD_RGBC},
    {"rgbcw", CMOD_RGBCW}
};

STRING_TO_ENUM_T tWfCfgTable[] = 
{
    {"old", GWCM_OLD},
    {"low", GWCM_LOW_POWER},
    {"spcl", GWCM_SPCL_MODE},
    {"prod", GWCM_OLD_PROD},
    {"spcl_auto", GWCM_SPCL_AUTOCFG}
};

STRING_TO_ENUM_T tColorTable[] = 
{
    {"c", COLOR_C},
    {"w", COLOR_W},
    {"r", COLOR_R},
    {"g", COLOR_G},
    {"b", COLOR_B},
};

#if (LIGHT_CFG_ENABLE_GAMMA == 1)
/// RED gamma 0.8 ~ 100% 
STATIC UCHAR_T ucGammaRed[] = {
                                        0,  0,  1,  1,  1,  2,  2,  3,  3,  4,  4,  5,  6,  6,  7,  7,  8,  9,  9,  10,
                                        11, 11, 12, 13, 13, 14, 15, 15, 16, 17, 18, 18, 19, 20, 21, 21, 22, 23, 24, 24,
                                        25, 26, 27, 28, 28, 29, 30, 31, 32, 32, 33, 34, 35, 36, 37, 37, 38, 39, 40, 41,
                                        42, 43, 44, 44, 45, 46, 47, 48, 49, 50, 51, 52, 52, 53, 54, 55, 56, 57, 58, 59,
                                        60, 61, 62, 63, 64, 65, 66, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,
                                        79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98,
                                        99, 100,101,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,
                                        121,122,123,124,125,126,127,128,129,130,131,132,134,135,136,137,138,139,140,141,
                                        142,144,145,146,147,148,149,150,151,152,154,155,156,157,158,159,160,162,163,164,
                                        165,166,167,168,170,171,172,173,174,175,177,178,179,180,181,182,184,185,186,187,
                                        188,189,191,192,193,194,195,196,198,199,200,201,202,204,205,206,207,208,210,211,
                                        212,213,214,216,217,218,219,220,222,223,224,225,227,228,229,230,231,233,234,235,
                                        236,238,239,240,241,243,244,245,246,248,249,250,251,253,254,255
};

/// GREEN gamma 0.6 ~ 70%
STATIC UCHAR_T ucGammaGreen[] = {
                                        0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,
                                        3,  3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  5,  6,  6,  6,  7,  7,  7,  7,  8,
                                        8,  8,  9,  9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16,
                                        16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25,
                                        26, 26, 27, 27, 28, 29, 29, 30, 30, 31, 31, 32, 33, 33, 34, 34, 35, 36, 36, 37,
                                        38, 38, 39, 39, 40, 41, 41, 42, 43, 43, 44, 45, 45, 46, 47, 47, 48, 49, 49, 50,
                                        51, 52, 52, 53, 54, 54, 55, 56, 57, 57, 58, 59, 60, 60, 61, 62, 63, 63, 64, 65,
                                        66, 66, 67, 68, 69, 70, 70, 71, 72, 73, 74, 75, 75, 76, 77, 78, 79, 80, 80, 81,
                                        82, 83, 84, 85, 86, 86, 87, 88, 89, 90, 91, 92, 93, 94, 94, 95, 96, 97, 98, 99,
                                        100,101,102,103,104,105,106,106,107,108,109,110,111,112,113,114,115,116,117,118,
                                        119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,139,
                                        140,141,142,143,144,145,146,147,148,149,150,151,152,154,155,156,157,158,159,160,
                                        161,162,164,165,166,167,168,169,170,172,173,174,175,176,177,179
};

//BLUE gama 0.6-75%
STATIC UCHAR_T ucGammaBlue[] = {
                                        0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,
                                        3,  3,  3,  3,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  7,  7,  7,  8,  8,  8,
                                        9,  9,  9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 14, 15, 15, 16, 16, 17,
                                        17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 27, 27,
                                        28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 34, 34, 35, 36, 36, 37, 38, 38, 39, 40,
                                        40, 41, 42, 42, 43, 44, 44, 45, 46, 46, 47, 48, 49, 49, 50, 51, 51, 52, 53, 54,
                                        54, 55, 56, 57, 58, 58, 59, 60, 61, 61, 62, 63, 64, 65, 65, 66, 67, 68, 69, 70,
                                        70, 71, 72, 73, 74, 75, 76, 76, 77, 78, 79, 80, 81, 82, 83, 83, 84, 85, 86, 87,
                                        88, 89, 90, 91, 92, 93, 94, 94, 95, 96, 97, 98, 99,100,101,102,103,104,105,106,
                                        107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,127,
                                        128,129,130,131,132,133,134,135,136,137,138,139,141,142,143,144,145,146,147,148,
                                        150,151,152,153,154,155,156,158,159,160,161,162,163,165,166,167,168,169,170,172,
                                        173,174,175,176,178,179,180,181,183,184,185,186,188,189,190,191
};
#endif

typedef struct {
    CHAR_T  *key;
    VOID    *value;
    USHORT_T value_size;
    VALUE_TYPE type;
}KEY_VALUE_T;

KEY_VALUE_T tJsonTable[] = 
{
    {",Jsonver:",    &gtDeviceCfg.Jsonver,   SIZEOF(gtDeviceCfg.Jsonver),        TYPE_STRING},
    {",category:",   &gtDeviceCfg.category,  SIZEOF(gtDeviceCfg.category),       TYPE_STRING},
    {",module:",     &gtDeviceCfg.module,    SIZEOF(gtDeviceCfg.module),         TYPE_STRING}, 
    
    {",cmod:",       &gtDeviceCfg.cmod,      SIZEOF(gtDeviceCfg.cmod),           TYPE_STRING}, 
    {",dmod:",       &gtDeviceCfg.dmod,      SIZEOF(gtDeviceCfg.dmod),           TYPE_HEX}, 
    {",cwtype:",     &gtDeviceCfg.cwtype,    SIZEOF(gtDeviceCfg.cwtype),         TYPE_HEX}, 
    {",onoffmode:",  &gtDeviceCfg.onoffmode, SIZEOF(gtDeviceCfg.onoffmode),      TYPE_HEX}, 
    {",pmemory:",    &gtDeviceCfg.pmemory,   SIZEOF(gtDeviceCfg.pmemory),        TYPE_HEX},
    {",ctrl_pin:",   &gtDeviceCfg.ctrl_pin,  SIZEOF(gtDeviceCfg.ctrl_pin),       TYPE_HEX},
    {",ctrl_lv:",    &gtDeviceCfg.ctrl_lv,   SIZEOF(gtDeviceCfg.ctrl_lv),        TYPE_HEX},
      
    {",defcolor:",   &gtDeviceCfg.defcolor,  SIZEOF(gtDeviceCfg.defcolor),       TYPE_STRING},
    {",defbright:",  &gtDeviceCfg.defbright, SIZEOF(gtDeviceCfg.defbright),      TYPE_HEX},
    {",deftemp:",    &gtDeviceCfg.deftemp,   SIZEOF(gtDeviceCfg.deftemp),        TYPE_HEX},
    {",cwmaxp:",     &gtDeviceCfg.cwmaxp,    SIZEOF(gtDeviceCfg.cwmaxp),         TYPE_HEX},
    {",brightmin:",  &gtDeviceCfg.brightmin, SIZEOF(gtDeviceCfg.brightmin),      TYPE_HEX},
    {",brightmax:",  &gtDeviceCfg.brightmax, SIZEOF(gtDeviceCfg.brightmax),      TYPE_HEX},
    {",colormax:",   &gtDeviceCfg.colormax,  SIZEOF(gtDeviceCfg.colormax),       TYPE_HEX},
    {",colormin:",   &gtDeviceCfg.colormin,  SIZEOF(gtDeviceCfg.colormin),       TYPE_HEX},

    {",wfcfg:",      &gtDeviceCfg.wfcfg,     SIZEOF(gtDeviceCfg.wfcfg),          TYPE_STRING},
    {",remdmode:",   &gtDeviceCfg.remdmode,   SIZEOF(gtDeviceCfg.remdmode),      TYPE_HEX},
    {",rstnum:",     &gtDeviceCfg.rstnum,    SIZEOF(gtDeviceCfg.rstnum),         TYPE_HEX},
    {",rstcor:",     &gtDeviceCfg.rstcor,    SIZEOF(gtDeviceCfg.rstcor),         TYPE_STRING},
    {",rstbr:",      &gtDeviceCfg.rstbr,     SIZEOF(gtDeviceCfg.rstbr),          TYPE_HEX},
    {",rsttemp:",    &gtDeviceCfg.rsttemp,   SIZEOF(gtDeviceCfg.rsttemp),        TYPE_HEX},
    {",pairt:",      &gtDeviceCfg.remdtime,  SIZEOF(gtDeviceCfg.remdtime),       TYPE_HEX},
    {",wfct:",       &gtDeviceCfg.wfptime,   SIZEOF(gtDeviceCfg.wfptime),        TYPE_HEX},

    {",pwmhz:",      &gtDeviceCfg.pwmhz,     SIZEOF(gtDeviceCfg.pwmhz),          TYPE_HEX},
    {",r_pin:",      &gtDeviceCfg.r_pin,     SIZEOF(gtDeviceCfg.r_pin),          TYPE_HEX},
    {",r_lv:",       &gtDeviceCfg.r_lv,      SIZEOF(gtDeviceCfg.r_lv),           TYPE_HEX},
    {",g_pin:",      &gtDeviceCfg.g_pin,     SIZEOF(gtDeviceCfg.g_pin),          TYPE_HEX},
    {",g_lv:",       &gtDeviceCfg.g_lv,      SIZEOF(gtDeviceCfg.g_lv),           TYPE_HEX},
    {",b_pin:",      &gtDeviceCfg.b_pin,     SIZEOF(gtDeviceCfg.b_pin),          TYPE_HEX},
    {",b_lv:",       &gtDeviceCfg.b_lv,      SIZEOF(gtDeviceCfg.b_lv),           TYPE_HEX},
    {",c_pin:",      &gtDeviceCfg.c_pin,     SIZEOF(gtDeviceCfg.c_pin),          TYPE_HEX},
    {",c_lv:",       &gtDeviceCfg.c_lv,      SIZEOF(gtDeviceCfg.c_lv),           TYPE_HEX},
    {",w_pin:",      &gtDeviceCfg.w_pin,     SIZEOF(gtDeviceCfg.w_pin),          TYPE_HEX},
    {",w_lv:",       &gtDeviceCfg.w_lv,      SIZEOF(gtDeviceCfg.w_lv),           TYPE_HEX},

#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)
    {",title20:",    &gtDeviceCfg.title20,   SIZEOF(gtDeviceCfg.title20),        TYPE_HEX},
#endif

#if ((LIGHT_CFG_ENABLE_GAMMA == 1 ) && (LIGHT_CFG_GAMMA_CAL == 0))
    {",gmr:",        &ucGammaRed,           SIZEOF(ucGammaRed),                 TYPE_STRING},
    {",gmg:",        &ucGammaGreen,         SIZEOF(ucGammaGreen),               TYPE_STRING},
    {",gmb:",        &ucGammaBlue,          SIZEOF(ucGammaBlue),                TYPE_STRING},
#endif

#if ((LIGHT_CFG_ENABLE_GAMMA == 1) && (LIGHT_CFG_GAMMA_CAL == 1))
    {",gmkr:",       &gtDeviceCfg.gammakr,   SIZEOF(gtDeviceCfg.gammakr),        TYPE_HEX},
    {",gmkg:",       &gtDeviceCfg.gammakg,   SIZEOF(gtDeviceCfg.gammakg),        TYPE_HEX},
    {",gmkb:",       &gtDeviceCfg.gammakb,   SIZEOF(gtDeviceCfg.gammakb),        TYPE_HEX},

    {",gmwr:",       &gtDeviceCfg.gammawr,   SIZEOF(gtDeviceCfg.gammawr),        TYPE_HEX},
    {",gmwg:",       &gtDeviceCfg.gammawg,   SIZEOF(gtDeviceCfg.gammawg),        TYPE_HEX},
    {",gmwb:",       &gtDeviceCfg.gammawb,   SIZEOF(gtDeviceCfg.gammawb),        TYPE_HEX},
#endif    
};

#define GET_ARRAY_LEN(x)    (SIZEOF(x)/SIZEOF(x[0]))

#define JSON_TABLE_LEN      GET_ARRAY_LEN(tJsonTable)
#define CMOD_TABLE_LEN      GET_ARRAY_LEN(tCmodTable)
#define WFCFG_TABLE_LEN     GET_ARRAY_LEN(tWfCfgTable)
#define COLOR_TABLE_LEN     GET_ARRAY_LEN(tColorTable)



/**
 * @brief: Device configuaration init check
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
STATIC OPERATE_LIGHT opDeviceCfgInitCheck(VOID)
{    
    if(!ucOemConfigLoadFlag) { 
        PR_ERR("oem config data not load !");
        return LIGHT_COM_ERROR;
    }
    return LIGHT_OK;
}

/**
 * @brief: get all config data from ucDeviceConfigTable[], and install it to
            the gtDeviceCfg struct. make sure all keys of tJsonTable[] can be find in ucDeviceConfigTable[]
 * @param {IN USHORT_T usMaxLen -> string len}
 * @retval: OPERATE_LIGHT
 */
STATIC OPERATE_LIGHT opDeviceCfgAnalysis(IN USHORT_T usMaxLen, IN CHAR_T *pBuf)
{
    UCHAR_T i = 0;
    CHAR_T *pStart;
    CHAR_T *pEnd;
    CHAR_T cTempValue[MAX_VALUE_SIZE];
    USHORT_T usLen;

    
    *(pBuf + 0) = ',';          /* to make sure don't match error, such as code "abcd" and "abc" */
    
    for(i = 0; i < JSON_TABLE_LEN; i++) {
        pStart = strstr(pBuf, tJsonTable[i].key);
        if(pStart == NULL) {
            continue;
        }

        pStart += strlen(tJsonTable[i].key);
        pEnd = pStart;
        
        while(*pEnd != ',') {
            pEnd ++;
            if(pEnd - pBuf >= usMaxLen) {        /* if the last key don't has "," end! */
                break;
            }
        }

        usLen = pEnd - pStart;
        
        if(usLen > MAX_VALUE_SIZE - 1) {      /* attention: key-value, max len is 15!! */
            usLen = tJsonTable[i].value_size;
        }
        
        memcpy(cTempValue, pStart, usLen);
        cTempValue[usLen] = '\0';

        if( tJsonTable[i].type == TYPE_HEX ) {
            UINT_T uiTemp = 0;
            UCHAR_T ucDecLen = 0;
            ucDecLen = ucLightToolSTR2Dec(cTempValue, usLen, &uiTemp);     /* @attention: default little-ending format */
            memcpy(tJsonTable[i].value, &uiTemp, ucDecLen);
        } else {
    #if ((LIGHT_CFG_ENABLE_GAMMA == 1) & (LIGHT_CFG_GAMMA_CAL == 0))
            if((strcmp(tJsonTable[i].key, "gmr:") == 0) || (strcmp(tJsonTable[i].key, "gmg:") == 0) || (strcmp(tJsonTable[i].key, "gmb:") == 0)) {
                UCHAR_T ucTemp;
                USHORT_T j;
                for(j = 0; j < (usLen / 2); j++) {
                    ucTemp = ucLightToolSTR2UCHAR(ucLightToolASC2Hex(*(pStart + 2*j)), ucLightToolASC2Hex(*(pStart + 2*j + 1)));
                    *((UCHAR_T *)tJsonTable[i].value + j) = ucTemp;
                    //*((UCHAR_T *)(tJsonTable[i].value + j)) = ucTemp;
                }
            
            } else {
                memcpy(tJsonTable[i].value, cTempValue, usLen);
            } 
            
    #else
            memcpy(tJsonTable[i].value, cTempValue, usLen);
    #endif
        }  
    }
    
    return LIGHT_OK;
}

/**
 * @brief: device configuration data init operate.
 * @param {None} 
 * @retval: none
 */
STATIC VOID vDeviceCfgDataInit(VOID)
{
    ucOemConfigLoadFlag = FALSE;
    memset(&gtDeviceCfg, 0, SIZEOF(DEVICE_CONFIG_T));
    
    gtDeviceCfg.ctrl_pin = PIN_NOEXIST;

}

/**
 * @brief: set device configuration data.
 * @param {IN UCHAR_T *pConfig -> light cfg data} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opDeviceCfgDataDefaultSet(IN DEVICE_CONFIG_T *ptConfig)
{
    if(ptConfig == NULL) {
        PR_ERR("Default config set param invalid!");
        return LIGHT_INVALID_PARM;
    }

    if(ucOemConfigLoadFlag) {
        PR_NOTICE("device config data already load! Don't load again!!");
        return LIGHT_OK;
    }
    
    vDeviceCfgDataInit();
    memcpy(&gtDeviceCfg, ptConfig, SIZEOF(DEVICE_CONFIG_T));
    ucOemConfigLoadFlag = TRUE;
    
    return LIGHT_OK;   
}

/**
 * @brief: set device configuration data.
 * @param {IN USHORT_T usLen -> oem cfg len} 
 * @param {IN CHAR_T *pConfig -> oem cfg data} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opDeviceCfgDataSet(IN USHORT_T usLen, IN CONST CHAR_T *pConfig)
{
    OPERATE_LIGHT opRet = -1;

    if(ucOemConfigLoadFlag) {
        PR_NOTICE("device config data already load! Don't load again!!");
        return LIGHT_OK;
    }
    
    if(usLen >= CONFIG_DATA_LEN_MAX) {
        PR_ERR("oem config set ERROR!");
        return LIGHT_COM_ERROR;
    }

    vDeviceCfgDataInit();
    opRet = opDeviceCfgAnalysis(usLen, (CHAR_T *)pConfig);
    if(opRet != LIGHT_OK) {
         PR_ERR("oem data vaild!");
         return LIGHT_COM_ERROR;
    }

#if ((LIGHT_CFG_ENABLE_GAMMA == 1) && (LIGHT_CFG_GAMMA_CAL == 1)) 
    USHORT_T i;  
    if((gtDeviceCfg.gammakr != 0) || (gtDeviceCfg.gammawr != 0) || \
        (gtDeviceCfg.gammakg != 0) || (gtDeviceCfg.gammawg != 0) || \
        (gtDeviceCfg.gammakb != 0) || (gtDeviceCfg.gammawb != 0)) {
        for(i = 0; i < 256 ; i++) {
            ucGammaRed[i] = ucLightToolCalcGamma(i, (FLOAT_T)(gtDeviceCfg.gammakr / 100.0), (FLOAT_T)(gtDeviceCfg.gammawr / 100.0));
            ucGammaGreen[i] = ucLightToolCalcGamma(i, (FLOAT_T)(gtDeviceCfg.gammakg / 100.0), (FLOAT_T)(gtDeviceCfg.gammawg / 100.0));
            ucGammaBlue[i] = ucLightToolCalcGamma(i, (FLOAT_T)(gtDeviceCfg.gammakb / 100.0), (FLOAT_T)(gtDeviceCfg.gammawb / 100.0));
        }
    }
    
#endif

    ucOemConfigLoadFlag = TRUE;
    
    return LIGHT_OK;
}

/**
 * @brief: load device configuration data.
 * @param {none} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opDeviceCfgDataLoad(VOID)
{
    USHORT_T usLen = 0;
    OPERATE_LIGHT opRet = -1;
    CHAR_T * pBuf = NULL;
    
    if(ucOemConfigLoadFlag) {
        PR_NOTICE("device config data already load! Don't load again!!");
        return LIGHT_OK;
    }

    pBuf = Malloc(2 * CONFIG_DATA_LEN_MAX);
    if(pBuf == NULL) {
        PR_ERR("Malloc failure!");
        return LIGHT_COM_ERROR;
    }
    
    opRet = opUserFlashReadOemCfgData(&usLen, pBuf);
    if((opRet != LIGHT_OK) || (usLen <= 0) || (usLen >= CONFIG_DATA_LEN_MAX)) {
        PR_ERR("oem data ERROR");
        Free(pBuf);     /* Free !! */
        pBuf = NULL;
        return LIGHT_COM_ERROR;
    }
    
    PR_DEBUG("oem len: %d", usLen);
    
    vDeviceCfgDataInit();
    
    opRet = opDeviceCfgAnalysis(usLen, pBuf);
    if(opRet != LIGHT_OK) {
        PR_ERR("oem data ERROR");
        return LIGHT_COM_ERROR;
    }
    
    if(pBuf != NULL) {
        Free(pBuf);
        pBuf = NULL;
    }
    
#if ((LIGHT_CFG_ENABLE_GAMMA == 1) && (LIGHT_CFG_GAMMA_CAL == 1))  
    USHORT_T i;
    if((gtDeviceCfg.gammakr != 0) || (gtDeviceCfg.gammawr != 0) || \
        (gtDeviceCfg.gammakg != 0) || (gtDeviceCfg.gammawg != 0) || \
        (gtDeviceCfg.gammakb != 0) || (gtDeviceCfg.gammawb != 0)) {
        for(i = 0; i < 256 ; i++) {
            ucGammaRed[i] = ucLightToolCalcGamma(i, (FLOAT_T)(gtDeviceCfg.gammakr / 100.0), (FLOAT_T)(gtDeviceCfg.gammawr / 100.0));
            ucGammaGreen[i] = ucLightToolCalcGamma(i, (FLOAT_T)(gtDeviceCfg.gammakg / 100.0), (FLOAT_T)(gtDeviceCfg.gammawg / 100.0));
            ucGammaBlue[i] = ucLightToolCalcGamma(i, (FLOAT_T)(gtDeviceCfg.gammakb / 100.0), (FLOAT_T)(gtDeviceCfg.gammawg / 100.0));
        }
    }
#endif  

    ucOemConfigLoadFlag = TRUE;
    return LIGHT_OK;
}

/**
 * @brief: get json version string
 * @param {out UCHAR_T *pJsonVer -> json version string }
 * @retval: none
 */
CHAR_T cDeviceCfgGetJsonVer(OUT CHAR_T *pJsonVer)
{   
    CHECK_LOAD_FINISH();
    memcpy(pJsonVer, gtDeviceCfg.Jsonver, 5);
    
    return LIGHT_OK;
}

/**
 * @brief: get json config data :category
 * @param {type} none
 * @retval: category
 */
CHAR_T cDeviceCfgGetCategory(OUT CHAR_T *pCategory)
{
    CHECK_LOAD_FINISH();
    memcpy(pCategory, gtDeviceCfg.category, 5);
    
    return LIGHT_OK;
}

/**
 * @brief: get json config data: module name string.
 * @param {out} module: module name string.
 * @param {out} len: length of module name string.
 * @retval: none
 */
CHAR_T cDeviceCfgGetModule(OUT CHAR_T *pModule, OUT UCHAR_T *pLen)
{
    CHECK_LOAD_FINISH();
    
    *pLen = strlen(gtDeviceCfg.module);
    memcpy(pModule, &gtDeviceCfg.module[0], strlen(gtDeviceCfg.module));

    return LIGHT_OK;
}

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
CHAR_T cDeviceCfgGetColorMode(VOID)
{
    CHECK_LOAD_FINISH();
    
    UCHAR_T i;
    for(i = 0; i < CMOD_TABLE_LEN; i++) {
        if(bStringCompare(&gtDeviceCfg.cmod[0], tCmodTable[i].string)) {
            return tCmodTable[i].enum_value;
        }
    }
    
    return LIGHT_COM_ERROR;
}

/**
 * @brief: get json config data: driver mode
 * @param {type} none
 * @return: 
 *          0x00 -> PWM
 *          0x01 -> SM16726B
 *          0x02 -> SM2135
 */
CHAR_T cDeviceCfgGetDriverMode(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.dmod;
}

/**
 * @brief: get json config data: cwtype
 * @param {type} none
 * @return: 
 *          0x00 -> CW drive
 *          0x01 -> CCT drive
 */
CHAR_T cDeviceCfgGetCWType(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.cwtype;
}

/**
 * @brief: get json config data: onoffmode
 * @param {type} none
 * @return: 
 *          0x00 -> turn on/off change gradually 
 *          0x01 -> turn on/off change directly
 */
CHAR_T cDeviceCfgGetOnOffMode(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.onoffmode;
}

/**
 * @brief: get memory cfg -- if not save app control data
 * @param {type} none
 * @return: 
 *          0x00 -> don't save app data in flash
 *          0x01 -> save app data in flash
 */
CHAR_T cDeviceCfgGetPmemoryCfg(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.pmemory;
}

/**
 * @brief: get ctrl pin number
 * @param {type} none
 * @return: ctrl pin number
 */
CHAR_T cDeviceCfgGetCtrlPin(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.ctrl_pin;
}

/**
 * @brief: get ctrl pin level
 * @param {type} none
 * @return: ctrl pin number
 */
CHAR_T cDeviceCfgGetCtrlPinLevel(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.ctrl_lv;
}

/**
 * @brief: get json config data: color set when connected.
 * @param {type} none
 * @return: 
 *          0x00 -> default bright is C when connected.
 *          0x01 -> default bright is W when connected.
 *          0x02 -> default bright is R when connected.
 *          0x03 -> default bright is G when connected.
 *          0x04 -> default bright is B when connected.
 */
CHAR_T cDeviceCfgGetDefColor(VOID)
{
    CHECK_LOAD_FINISH();
    
    UCHAR_T i;
    CHAR_T cTempColor[2] = {0,0}; //the second value is end of string: "\0"
    
    cTempColor[0] = gtDeviceCfg.defcolor;
    for(i = 0; i < COLOR_TABLE_LEN; i++) { 
       if(bStringCompare(cTempColor, tColorTable[i].string)) {
          return tColorTable[i].enum_value;
       }
    }
    
    return LIGHT_COM_ERROR;
}

/**
 * @brief: get json config data: brightness when connected.
 * @param {type} none
 * @return: brightness
 */
CHAR_T cDeviceCfgGetDefBrightness(VOID)
{
    CHECK_LOAD_FINISH();
    
    return  gtDeviceCfg.defbright;
}

/**
 * @brief: get json config data: deftemp when connected.
 * @param {type} none
 * @return: default temperature when connected.
 */
CHAR_T cDeviceCfgGetDefTemperature(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.deftemp;
}

/**
 * @brief: get json config data: white max power
 * @param {type} none
 * @return: white max power
 */
CHAR_T cDeviceCfgGetCWMaxPower(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.cwmaxp;
}

/**
 * @brief: get the CW minimum brightness 
 * @param {type} none
 * @return: CW minimum brightness 
 */
CHAR_T cDeviceCfgGetWhiteMin(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.brightmin;
}

/**
 * @brief: get the CW maxinum brightness
 * @param {type} none
 * @return: CW maxinum brightness
 */
CHAR_T cDeviceCfgGetWhiteMax(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.brightmax;
}

/**
 * @brief: get the RGB minimum brightness 
 * @param {type} none
 * @return: RGB mininum brightness
 */
CHAR_T cDeviceCfgGetColorMin(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.colormin;
}

/**
 * @brief: get the RGB maxinum brightness 
 * @param {type} none
 * @return: RGB maxinum brightness
 */
CHAR_T cDeviceCfgGetColorMax(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.colormax;
}

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
CHAR_T cDeviceCfgGetWificfg(VOID)
{
    CHECK_LOAD_FINISH();
    
    UCHAR_T i;
    for(i = 0;i < WFCFG_TABLE_LEN; i++){
        if(bStringCompare(&gtDeviceCfg.wfcfg[0], tWfCfgTable[i].string)) {
            return tWfCfgTable[i].enum_value;
        }
    }
    
    return LIGHT_COM_ERROR;
}

/**
 * @brief: remind mode when pairing(blink or breath)
 * @param {type} none
 * @return:
 */
CHAR_T cDeviceCfgGetRemindMode(VOID)
{
    CHECK_LOAD_FINISH(); 
       
    return gtDeviceCfg.remdmode;   /* default remind mode is 0 --> blink */
}

/**
 * @brief: get pairing reset number.
 * @param {type} none
 * @return: reset number
 */
CHAR_T cDeviceCfgGetResetNum(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.rstnum;
}

/**
 * @brief: get color set when pairing.
 * @param {type} none
 * @return: 
 *          0x00 -> default bright is C when pairing.
 *          0x01 -> default bright is W when pairing.
 *          0x02 -> default bright is R when pairing.
 *          0x03 -> default bright is G when pairing.
 *          0x04 -> default bright is B when pairing.
 */
CHAR_T cDeviceCfgGetResetColor(VOID)
{
    CHECK_LOAD_FINISH();
    
    UCHAR_T i;
    CHAR_T color[2] = {0,0};

    color[0] = gtDeviceCfg.rstcor;
    for(i = 0; i < COLOR_TABLE_LEN; i++){ 
       if(bStringCompare(color, tColorTable[i].string)){
          return tColorTable[i].enum_value;
       }
    }
    
    return LIGHT_COM_ERROR;
}

/**
 * @brief: get reset brightness when pairing
 * @param {type} none
 * @return: brightness when pairing
 */
CHAR_T cDeviceCfgGetResetBrightness(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.rstbr;
}

/**
 * @brief: get reset temperature(reset color is white) when pairing
 * @param {type} none
 * @return: reset temperature(reset color is white) when pairing
 */
CHAR_T cDeviceCfgGetResetTemperature(VOID)
{   
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.rsttemp;
}

/**
 * @brief: get remind time when remind mode is breathing,unit:sec
 * @param {type} none
 * @return:remind time
 */
SHORT_T sDeviceCfgGetRemindTime(VOID)
{
    CHECK_LOAD_FINISH();  

    return gtDeviceCfg.remdtime;   /* remind time only use as breathing  */
}

/**
 * @brief: get pairing time,unit:min
 * @param {type} none
 * @return: pairting time
 */
CHAR_T cDeviceCfgGetPairingTime(VOID)
{
    CHECK_LOAD_FINISH();

    if(0 == gtDeviceCfg.wfptime) {     /* to avoid ota, 3min */
        gtDeviceCfg.wfptime = 3;
    }
    
    return gtDeviceCfg.wfptime;
}

/**
 * @brief: get pwm frequency 
 * @param {type} none
 * @return: pwm frequency 
 */
USHORT_T usDeviceCfgGetPwmHz(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.pwmhz;
}

/**
 * @brief: get red pin number
 * @param {type} none
 * @return: red pin number
 */
CHAR_T cDeviceCfgGetRedPin(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.r_pin;
}

/**
 * @brief: get red pin effective level.
 * @param {type} none
 * @return: red pin effective level.
 */
CHAR_T cDeviceCfgGetRedPinLevel(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.r_lv;
}

/**
 * @brief: get green pin number
 * @param {type} none
 * @return: green pin number
 */
CHAR_T cDeviceCfgGetGreenPin(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.g_pin;
}

/**
 * @brief: get green pin effective level.
 * @param {type} none
 * @return: green pin effective level.
 */
CHAR_T cDeviceCfgGetGreenPinLevel(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.g_lv;
}

/**
 * @brief: get blue pin number
 * @param {type} none
 * @return: blue pin number
 */
CHAR_T cDeviceCfgGetBluePin(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.b_pin;
}

/**
 * @brief: get blue pin effective level.
 * @param {type} none
 * @return: blue pin effective level.
 */
CHAR_T cDeviceCfgGetBluePinLevel(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.b_lv;
}

/**
 * @brief: get cold white pin number
 * @param {type} none
 * @return: cold white pin number
 */
CHAR_T cDeviceCfgGetColdPin(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.c_pin;
}

/**
 * @brief: get cold white pin pin effective level.
 * @param {type} none
 * @return: cold white pin effective level.
 */
CHAR_T cDeviceCfgGetColdPinLevel(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.c_lv;
}

/**
 * @brief: get warm white pin number
 * @param {type} none
 * @return: warm white pin number
 */
CHAR_T cDeviceCfgGetWarmPin(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.w_pin;
}

/**
 * @brief: get warm white effective level.
 * @param {type} none
 * @return: warm white pin effective level.
 */
CHAR_T cDeviceCfgGetWarmPinLevel(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.w_lv;
}


#if (LIGHT_CFG_SUPPORT_LOWPOWER == 1)
/**
 * @brief: get title20 flag
 * @param {type} none
 * @return: 
 *          0x00 -> don't support lowpower mode
 *          0x01 -> support lowpower mode
 */
CHAR_T cDeviceCfgGetTitle20(VOID)
{
    CHECK_LOAD_FINISH();
    
    return gtDeviceCfg.title20;
}
#endif

#if (LIGHT_CFG_ENABLE_GAMMA == 1)
/**
 * @brief: get red gamma 
 * @param {type} none
 * @return: red gamma
 */
UCHAR_T ucDeviceCfgGetGammaRed(IN UCHAR_T ucIndex)
{
    CHECK_LOAD_FINISH();
    
    return ucGammaRed[ucIndex];
}

/**
 * @brief: get green gamma 
 * @param {type} none
 * @return: green gamma
 */
UCHAR_T ucDeviceCfgGetGammaGreen(IN UCHAR_T ucIndex)
{
    CHECK_LOAD_FINISH();
    
    return ucGammaGreen[ucIndex];
}

/**
 * @brief: get blue gamma 
 * @param {type} none
 * @return: blue gamma
 */
UCHAR_T ucDeviceCfgGetGammaBlue(IN UCHAR_T ucIndex)
{
    CHECK_LOAD_FINISH();
    
    return ucGammaBlue[ucIndex];
}

#endif

