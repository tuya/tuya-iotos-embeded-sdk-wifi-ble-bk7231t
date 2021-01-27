/*
 * @Author: jinlu
 * @email: jinlu@tuya.com
 * @LastEditors: wls
 * @file name: light_flash.c
 * @Description: light production flash data read/write proc
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-05-06 10:57:08
 * @LastEditTime: 2019-08-28 19:18:35
 */

#include "soc_flash.h"
#include "user_flash.h"
#include "light_types.h"
#include "light_printf.h"

/**
 * @brief: save light reset cnt
 * @param {IN UCHAR_T ucData -> save reset cnt to flash} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashWriteResetCnt(IN UCHAR_T ucData)
{
    OPERATE_LIGHT opRet = -1;
    
    opRet = opSocFlashWrite(SAVE_TYP1, RESET_CNT_OFFSET, &ucData, 1);
    if(opRet != LIGHT_OK) {
        PR_ERR("Reset cnt write error %d!", opRet);
        return LIGHT_COM_ERROR;
    }
    return LIGHT_OK;
}

/**
 * @brief: save light application data
 * @param {IN LIGHT_APP_DATE_FLASH_T *pData -> save data} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashWriteAppData(IN LIGHT_APP_DATA_FLASH_T *pData)
{
    OPERATE_LIGHT opRet;
    
    opRet = opSocFlashWrite(SAVE_TYP2, LIGHT_APP_DATA_OFFSET, (UCHAR_T *)pData, SIZEOF(LIGHT_APP_DATA_FLASH_T));
    if(opRet != LIGHT_OK) {
        PR_ERR("Application data write error %d!", opRet);
        return LIGHT_COM_ERROR;
    }
    return LIGHT_OK;
}

/**
 * @brief: save light product test data
 * @param {IN LIGHT_PROD_TEST_DATA_FLASH_T *pData -> prod test data} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashWriteProdData(IN LIGHT_PROD_TEST_DATA_FLASH_T *pData)
{
    OPERATE_LIGHT opRet = -1;
    
    opRet = opSocFlashWrite(SAVE_TYP3, PROD_TEST_DATA_OFFSET, (UCHAR_T *)pData, SIZEOF(LIGHT_PROD_TEST_DATA_FLASH_T));
    if(opRet != LIGHT_OK) {
        PR_ERR("Production test data write error %d!", opRet);
        return LIGHT_COM_ERROR;
    }
    return LIGHT_OK;
}

/**
 * @brief: save light application data
 * @param {IN USHORT_T usLen -> read oem cfg data len }
 * @param {IN UCHAR_T *pData -> read oem cfg data }
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashWriteOemCfgData(IN USHORT_T usLen, IN UCHAR_T *pData)
{
    OPERATE_LIGHT opRet = -1;

    opRet = opSocOemCfgWrite(usLen, pData);
    if(opRet != LIGHT_OK) {
        PR_ERR("oem cfg move to uf file error!");
        return LIGHT_COM_ERROR;
    }

    return LIGHT_OK;
}

/**
 * @brief: read light reset cnt
 * @param {OUT UCHAR_T *data -> reset cnt} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashReadResetCnt(OUT UCHAR_T *pData)
{
    INT_T uiReadCnt = 0;
    
    uiReadCnt = uiSocFlashRead(SAVE_TYP1, RESET_CNT_OFFSET, 1, (UCHAR_T *)pData);
    if(uiReadCnt <= 0) {
        PR_ERR("Reset cnt read error!");
        return LIGHT_COM_ERROR;
    }

    return LIGHT_OK;
}

/**
 * @brief: read light application data
 * @param {IN LIGHT_APP_DATE_FLASH_T *pData -> prod test data} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashReadAppData(OUT LIGHT_APP_DATA_FLASH_T *pData)
{
    INT_T uiReadCnt = 0;
    
    uiReadCnt = uiSocFlashRead(SAVE_TYP2, LIGHT_APP_DATA_OFFSET, SIZEOF(LIGHT_APP_DATA_FLASH_T), (UCHAR_T *)pData);
    if(uiReadCnt <= 0) {
        PR_ERR("Application data read error!");
        return LIGHT_COM_ERROR;
    }
    
    return LIGHT_OK;
}

/**
 * @brief: read light prod test data
 * @param {OUT LIGHT_PROD_TEST_DATA_FLASH_T *data -> prod test data} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashReadProdData(OUT LIGHT_PROD_TEST_DATA_FLASH_T *pData)
{
    INT_T uiReadCnt = 0;
    
    uiReadCnt = uiSocFlashRead(SAVE_TYP3, PROD_TEST_DATA_OFFSET, SIZEOF(LIGHT_PROD_TEST_DATA_FLASH_T), (UCHAR_T *)pData);
    if(uiReadCnt <= 0) {
        PR_ERR("Production data read error!");
        return LIGHT_COM_ERROR;
    }

    return LIGHT_OK;
}

/**
 * @brief: read oem cfg data
 * @param {OUT USHORT_T *len -> read data len} 
 * @param {OUT UCHAR_T *data -> read data} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashReadOemCfgData(OUT USHORT_T *pLen, OUT UCHAR_T *pData)
{
    OPERATE_LIGHT opRet = -1;
    
    opRet = opSocOemCfgRead(pLen, pData);
    if(opRet != LIGHT_OK) {
        PR_ERR("oem cfg data read error!");
        return LIGHT_COM_ERROR;
    }

    return LIGHT_OK;
}

/**
 * @brief: earse light product test data
 * @param {none} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashEarseProdData(VOID)
{
    OPERATE_LIGHT opRet = -1;
    
    opRet = opSocFlashEraseSpecial(SAVE_TYP3, PROD_TEST_DATA_OFFSET);
    if(opRet != LIGHT_OK) {
        PR_ERR("Production test data earse error %d!", opRet);
        return LIGHT_COM_ERROR;
    }
    return LIGHT_OK;
}

/**
 * @brief: erase all user flash data
 * @param {none} 
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opUserFlashDataErase(VOID)
{
    OPERATE_LIGHT opRet = opSocFlashErase();
    
    if(opRet!= LIGHT_OK){
        return LIGHT_COM_ERROR; 
    }
    return LIGHT_OK;
}


