/*
 * @Author: jinlu
 * @email: jinlu@tuya.com
 * @LastEditors:
 * @file name: user_flash.h
 * @Description: light production flash read/write include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-26 13:55:40
 * @LastEditTime: 2019-08-10 13:01:32
 */

#ifndef __USER_FLASH_H__
#define __USER_FLASH_H__

#include "light_types.h"
#include "soc_flash.h"


#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */


#define LIGHT_SCENE_MAX_LENGTH    210

#pragma pack(1)

typedef enum
{
    LIGHT_MODE_WHITE = 0,
    LIGHT_MODE_COLOR,
    LIGHT_MODE_SCENE,
    LIGHT_MODE_MUSIC,
    LIGHT_MODE_MAX,
}LIGHT_MODE_FLASH_E;

typedef struct
{
    USHORT_T usRed;         /* color R setting */
    USHORT_T usGreen;
    USHORT_T usBlue;
}COLOR_RGB_FLASH_T;

typedef struct
{
    USHORT_T usHue;
    USHORT_T usSat;
    USHORT_T usValue;
    UCHAR_T  ucColorStr[13];
}COLORT_ORIGIN_FLASH_T;

/**
 * @brief Light save data structure
 */
typedef struct
{
    BOOL_T                  bPower;
    LIGHT_MODE_FLASH_E      eMode;
    USHORT_T                usBright;
    USHORT_T                usTemper;
    COLOR_RGB_FLASH_T       tColor;
    COLORT_ORIGIN_FLASH_T   tColorOrigin;
    CHAR_T                  cScene[LIGHT_SCENE_MAX_LENGTH + 1];
}LIGHT_APP_DATA_FLASH_T;

/**
 * @brief Light save data structure
 */
typedef enum {
    FUC_TEST1 = 0,
    AGING_TEST,
    FUC_TEST2,
}TEST_MODE_E;

#pragma pack(4)
/**
 * @brief Light prod test save structure
 */
typedef struct {
    TEST_MODE_E  eTestMode;
    USHORT_T     usAgingTestedTime;
}LIGHT_PROD_TEST_DATA_FLASH_T;
#pragma pack()

typedef struct
{
    USHORT_T usRed;         /* color R setting */
    USHORT_T usGreen;
    USHORT_T usBlue;
}color_rgb_t;

typedef struct
{
    USHORT_T usColorData1;
    USHORT_T usColorData2;
    USHORT_T usColorData3;
    UCHAR_T ucColorStr[13];
}color_origin_t;

/**
 * @brief Light save data structure
 */
typedef struct
{
    BOOL_T                bPower;
    LIGHT_MODE_FLASH_E    Mode;
    USHORT_T              usBright;
    USHORT_T              usTemper;
    color_rgb_t           Color;
    color_origin_t        ColorOrigin;
    UCHAR_T         ucScene[LIGHT_SCENE_MAX_LENGTH + 1];
}light_app_data_flash_t;

typedef struct {
    UCHAR_T     valid; 
    UCHAR_T     hour;
    UCHAR_T     min;
    UCHAR_T     h_per[2];
    UCHAR_T     s_per; 
    UCHAR_T     v_per;  
    UCHAR_T     bright_per;
    UCHAR_T     temper_per;
}RHYTHM_Node_Param_FLASH_S;

typedef struct {
    UCHAR_T     valid; 
    UCHAR_T     wday;   
    UCHAR_T     step;   
    UCHAR_T     hour;    
    UCHAR_T     min;    
    UCHAR_T     h_per[2]; 
    UCHAR_T     s_per;
    UCHAR_T     v_per;   
    UCHAR_T     bright_per;
    UCHAR_T     temper_per;
}SLEEP_Node_Param_FLASH_S;

typedef struct {
    UCHAR_T     version;
    UCHAR_T     onoff;  
    UCHAR_T     mode; 
    UCHAR_T     wday;  
    UCHAR_T     node_cnt;
    RHYTHM_Node_Param_FLASH_S  node[8];
}RHYTHM_PROTL_FLASH_S;

typedef struct {
    UCHAR_T     version;
    UCHAR_T     node_cnt;
    SLEEP_Node_Param_FLASH_S  node[4];
}SLEEP_PROTL_FLASH_S;

/**
 * @brief Light oem cfg data structure
 */
//@attention!!!
#pragma pack()

#define RESET_CNT_OFFSET          0
#define LIGHT_APP_DATA_OFFSET     (RESET_CNT_OFFSET + 2)
#define PROD_TEST_DATA_OFFSET     (LIGHT_APP_DATA_OFFSET + sizeof(LIGHT_APP_DATA_FLASH_T) + 1)

#define RHYTHM_DATASAVE_TYPE      SAVE_TYP4
#define RHYTHM_DATASAVE_OFFSET    (PROD_TEST_DATA_OFFSET + sizeof(light_app_data_flash_t)+1)

#define SLEEP_DATASAVE_TYPE       SAVE_TYP5
#define SLEEP_DATASAVE_OFFSET     (RHYTHM_DATASAVE_OFFSET + sizeof(RHYTHM_PROTL_FLASH_S) + 1)

#define WAKE_DATASAVE_TYPE        SAVE_TYP6
#define WAKE_DATASAVE_OFFSET      (SLEEP_DATASAVE_OFFSET + sizeof(SLEEP_PROTL_FLASH_S) + 1)

/**
 * @brief: save light reset cnt
 * @param {IN UCHAR_T ucData -> save reset cnt to flash}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashWriteResetCnt(IN UCHAR_T ucData);

/**
 * @brief: save light application data
 * @param {IN LIGHT_APP_DATA_FLASH_T *pData -> save data}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashWriteAppData(IN LIGHT_APP_DATA_FLASH_T *pData);

/**
 * @brief: save light product test data
 * @param {IN LIGHT_PROD_TEST_DATA_FLASH_T *pData -> prod test data}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashWriteProdData(IN LIGHT_PROD_TEST_DATA_FLASH_T *pData);

/**
 * @brief: save light application data
 * @param {IN USHORT_T usLen -> read oem cfg data len }
 * @param {IN UCHAR_T *pData -> read oem cfg data }
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashWriteOemCfgData(IN USHORT_T usLen, IN UCHAR_T *pData);

/**
 * @brief: read light reset cnt
 * @param {OUT UCHAR_T *data -> reset cnt}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashReadResetCnt(OUT UCHAR_T *pData);

/**
 * @brief: read light application data
 * @param {IN LIGHT_APP_DATE_FLASH_T *pData -> prod test data}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashReadAppData(OUT LIGHT_APP_DATA_FLASH_T *pData);

/**
 * @brief: read light prod test data
 * @param {OUT LIGHT_PROD_TEST_DATA_FLASH_T *data -> prod test data}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashReadProdData(OUT LIGHT_PROD_TEST_DATA_FLASH_T *pData);

/**
 * @brief: read oem cfg data
 * @param {OUT USHORT_T *len -> read data len}
 * @param {OUT UCHAR_T *data -> read data}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashReadOemCfgData(OUT USHORT_T *pLen, OUT UCHAR_T *pData);

/**
 * @brief: earse light product test data
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashEarseProdData(VOID);

/**
 * @brief: erase all user flash data
 * @param {none}
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashDataErase(VOID);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __USER_FLASH_H__ */
