/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors:   
 * @file name: light_toolkit.c
 * @Description: light common toolkit
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-05-06 10:57:08
 * @LastEditTime: 2019-06-21 11:18:08
 */

#include "light_types.h"
#include "light_tools.h"
#include "light_printf.h"
#include "math.h"

#define SET_BIT(value,bit) (value|=(1<<bit))
#define CLR_BIT(value,bit) (value&=~(1<<bit))
#define GET_BIT(value,bit) (value&(1<<bit))

/**
 * @brief: get the max value in 5 numbers
 * @param { UINT_T a -> number1 }
 * @param { UINT_T b -> number2 } 
 * @param { UINT_T c -> number3 }
 * @param { UINT_T d -> number4 }
 * @param { UINT_T e -> number5 } 
 * @retval: Max value
 */
UINT_T uiLightToolGetMaxValue(IN UINT_T a, IN UINT_T b, IN UINT_T c, IN UINT_T d, IN UINT_T e)
{
    UINT_T x = a > b ? a : b;
    UINT_T y = c > d ? c : d;
    UINT_T z = x > y ? x : y;

    return (z > e ? z : e);
}

/**
 * @brief: get the absolute value
 * @param { FLOAT_T value -> calc value } 
 * @retval: absolute value
 */
FLOAT_T fLightToolGetABSValue(IN FLOAT_T value)
{
    return (value > 0 ? value : -value);
}

/**
 * @brief: get the absolute value
 * @param { INT_T value -> calc value } 
 * @retval: absolute value
 */
UINT_T uiLightToolGetABSValue(IN INT_T value)
{
    return (value > 0 ? value : -value);
}

/**
 * @brief: val bit valid
 * @param { val bit } 
 * @retval: valid or not valid
 */

UCHAR_T ucLightToolIsBitValid(IN UCHAR_T val, IN UCHAR_T bit)
{
	return GET_BIT(val,bit)==0 ? 0 : 1;
}

/**
 * @brief: pow 10
 * @param {p} power
 * @retval: value
 */
STATIC UINT_T ucLightToolPow10(UCHAR_T p)
{
    UINT_T i = 0, value = 1;
    for(i = 0; i < p; i++){
        value = value * 10;
    }
    return value;
}

/**
 * @brief: convert dec string to dec hex
 * @param {decStr} dec string , like: "123"; 
 * @param {strLen} dec string len; 
 * @param {dec} dec hex, like 0x7B. 
 * @retval: dec hex length, 0 ~ 4
 */
UCHAR_T ucLightToolSTR2Dec(IN CHAR_T *decStr, IN UCHAR_T strLen, OUT UINT_T* dec)
{
    if(decStr == NULL) {
        return 0;
    }
    
    UINT_T i, dec_value = 0;
    for(i = 0; i < strLen; i++) {
        dec_value += (decStr[i] - '0') * ucLightToolPow10(strLen-i-1);
    }
    
    *dec = dec_value;
    if(dec_value <= 0xFF) {
        return 1;
    }else if(dec_value <= 0xFFFF) {
        return 2;
    }else if(dec_value <= 0xFFFFFF) {
        return 3;
    }else if(dec_value <= 0xFFFFFFFF) {
        return 4;
    }else {
        return 0;
    }
}

/**
 * @brief: ASSIC change to hex
 * @param { CHAR_T AscCode -> ASSIC code } 
 * @return: hex value
 * @retval: HEX
 */
UCHAR_T ucLightToolASC2Hex(IN CHAR_T AscCode)
{
    UCHAR_T ucResult = 0;

    if( '0' <= AscCode && AscCode <= '9' ) {
        ucResult = AscCode - '0';
    } else if ( 'a' <= AscCode && AscCode <= 'f' ) {
        ucResult = AscCode - 'a' + 10;
    } else if ( 'A' <= AscCode && AscCode <= 'F' ) {
        ucResult = AscCode - 'A' + 10;
    } else {
        ucResult = 0;
    }

    return ucResult;
}

/**
 * @brief: ASSIC change to hex
 * @param { UCHAR_T H -> high 4 bit } 
 * @param { UCHAR_T L -> low 4 bit } 
 * @retval: UCHAR_T
 */
UCHAR_T ucLightToolSTR2UCHAR(IN UCHAR_T H, IN UCHAR_T L)
{
    return((H << 4) | (L & 0x0F));
}

/**
 * @brief: four unsigned char merge into unsigned short
 * @param { UCHAR_T HH -> USHORT Hight hight 4bit }
 * @param { UCHAR_T HL -> USHORT Hight low 4bit   }
 * @param { UCHAR_T LH -> USHORT low hight 4bit   }
 * @param { UCHAR_T LL -> USHORT low low 4bit     }
 * @retval: USHORT_T
 */
USHORT_T usLightToolSTR2USHORT(IN UCHAR_T HH, IN UCHAR_T HL, IN UCHAR_T LH, IN UCHAR_T LL)
{
    return ( (HH << 12) | (HL << 8) | (LH << 4) | (LL & 0x0F) );
}

/**
 * @brief: HSV change to RGB
 * @param {IN USHORT_T h -> range 0~360 }
 * @param {IN USHORT_T s -> range 0~1000}
 * @param {IN USHORT_T v -> range 0~1000}
 * @param {OUT USHORT_T *r -> R result,rang from 0~1000}
 * @param {OUT USHORT_T *g -> G result,rang from 0~1000}
 * @param {OUT USHORT_T *b -> B result,rang from 0~1000}
 * @retval: none
 */
VOID vLightToolHSV2RGB(IN USHORT_T h, IN USHORT_T s, IN USHORT_T v, OUT USHORT_T *r, OUT USHORT_T *g, OUT USHORT_T *b)
{
    INT_T i;
    FLOAT_T RGB_min, RGB_max;
    FLOAT_T RGB_Adj;
    INT_T difs;

    if(h >= 360) {
        h = 0;
    }
    
    RGB_max = v * 1.0f;
    RGB_min = RGB_max*(1000 - s) / 1000.0f;

    i = h / 60;
    difs = h % 60; /* factorial part of h */


    /* RGB adjustment amount by hue */
    RGB_Adj = (RGB_max - RGB_min)*difs / 60.0f;

    switch (i) {
        case 0:
            *r = (USHORT_T)RGB_max;
            *g = (USHORT_T)(RGB_min + RGB_Adj);
            *b = (USHORT_T)RGB_min;
            break;
            
        case 1:
            *r = (USHORT_T)(RGB_max - RGB_Adj);
            *g = (USHORT_T)RGB_max;
            *b = (USHORT_T)RGB_min;
            break;
            
        case 2:
            *r = (USHORT_T)RGB_min;
            *g = (USHORT_T)RGB_max;
            *b = (USHORT_T)(RGB_min + RGB_Adj);
            break;
            
        case 3:
            *r = (USHORT_T)RGB_min;
            *g = (USHORT_T)(RGB_max - RGB_Adj);
            *b = (USHORT_T)RGB_max;
            break;
            
        case 4:
            *r = (USHORT_T)(RGB_min + RGB_Adj);
            *g = (USHORT_T)RGB_min;
            *b = (USHORT_T)RGB_max;
            break;
            
        default:        // case 5:
            *r = (USHORT_T)RGB_max;
            *g = (USHORT_T)RGB_min;
            *b = (USHORT_T)(RGB_max - RGB_Adj);
            break;
    }

    if(*r > 1000) {
        *r = 1000;
    }

    if(*g > 1000) {
        *g = 1000;
    }

    if(*b > 1000) {
        *b = 1000;
    }
}

/**
 * @brief: get the max data in 2 float number
 * @param {IN FLOAT_T a}
 * @param {IN FLOAT_T b}
 * @retval: MAX FLOAT_T
 */
FLOAT_T fLightToolGetMAX(IN FLOAT_T a, IN FLOAT_T b)
{
    if(a >= b) {
        return a;
    } else {
        return b;
    }
}

/**
 * @brief: get the min data in 2 float number
 * @param {IN FLOAT_T a}
 * @param {IN FLOAT_T b}
 * @retval: MIN FLOAT_T
 */
FLOAT_T fLightToolGetMIN(FLOAT_T a, FLOAT_T b)
{
    if(a <= b) {
        return a;
    } else {
        return b;
    }
}

/**
 * @brief: RGB change to HSV
 * @param {IN USHORT_T R -> R,rang from 0~1000}
 * @param {IN USHORT_T G -> G,rang from 0~1000}
 * @param {IN USHORT_T B -> B,rang from 0~1000}
 * @param {OUT USHORT_T H -> result, range 0~360}
 * @param {OUT USHORT_T S -> result, range 0~1000}
 * @param {OUT USHORT_T V -> result, range 0~1000}
 * @retval: none
 */
VOID vLightToolRGB2HSV(IN USHORT_T R, IN USHORT_T G, IN USHORT_T B, OUT USHORT_T *H, OUT USHORT_T *S, OUT USHORT_T *V)
{
    FLOAT_T r,g,b;
    FLOAT_T minRGB,maxRGB,deltaRGB;
    FLOAT_T h,s,v;
     
    r = R / 1000.0f;
    g = G / 1000.0f;
    b = B / 1000.0f;

    minRGB = fLightToolGetMIN(r,fLightToolGetMIN(g,b));
    maxRGB = fLightToolGetMAX(r,fLightToolGetMAX(g,b));

    deltaRGB = maxRGB - minRGB;

    v = maxRGB;
    if(maxRGB != 0.0) {
        s = deltaRGB / maxRGB;
    } else {
        s = 0.0;
    }
    
    if(s <= 0.0) {
        h = -1.0f;
    } else {
        if(r == maxRGB) {
            h = (g-b) / deltaRGB;
        } else if (g == maxRGB) {
            h = 2.0 + (b-r) / deltaRGB;
        } else if (b == maxRGB) {
            h = 4.0 + (r-g) / deltaRGB;
        }
    }
    
    h = h * 60.0;
    if(h < 0.0) {
        h += 360;
    }

    *H = (USHORT_T)h;
    *S = (USHORT_T)(s * 1000);
    *V = (USHORT_T)(v * 1000);

}

/**
 * @brief: compare two string 
 * @param {IN CHAR_T* str1 -> string 1}
 * @param {IN CHAR_T* str2 -> string 2}
 * @retval: 0: if these two string is not same,
 *          1: these two string is all same
 */
BOOL_T bStringCompare(IN CHAR_T* str1, IN CHAR_T* str2)
{
    
    while(*str1 !='\0'&& *str2 != '\0')
    {
        if(*str1 != *str2){
            return 0;
        }
        str1++;
        str2++;
    }
    
    if(*str1 =='\0'&& *str2 == '\0') {
        return 1;
    } else {
      return 0;
    }
}

/**
 * @brief: change number to str
 * @param {IN CHAR_T cMode -> change format( 0 -> %d, 4-> %4x)}
 * @param {IN UINT_T uiNum -> number(unsigned int)}
 * @param {IN UCHAR_T len -> buf len(sizeof)}
 * @param {OUT CHAR_T *cStr -> string}
 * @retval: string
 */
VOID vNum2Str(IN CHAR_T cMode, IN UINT_T uiNum, IN UCHAR_T len, OUT CHAR_T *cStr)
{
    memset(cStr, 0, len);
    
    switch(cMode) {
        case 0:
            snprintf(cStr, len, "%d", uiNum);
            break;

        case 4:
            snprintf(cStr, len, "%04x", uiNum);
            break;
            
        default:
            break;
    }
   
    
}

/**
 * @brief: compress one uint scene data,from string (26 bytes) to hex (65 bits).
 * compress format:
 * bits: mode:2 bits, time1:7bits, timer2:7bits, H:9bits, S:10 bits, V:10 bits,B:10bits, T:10bits;
 * output_buf[0]: mode_1 + time1
 * output_buf[1]: H_1 + time2
 * output_buf[2]: H_8
 * output_buf[3]: S_2 + V_2 + B_2 + T_2
 * output_buf[4]: S_8
 * output_buf[5]: V_8
 * output_buf[6]: B_8
 * output_buf[7]: T_8
 * @param {input_str} one uint data of scene
 * @param {output_buf} Output buffer address,a unit of compressed data,including speed,transition time,H,S,V,B,T
 * @param {light_mode} output transition mode of the compressed data
 * @retval: OPERATE_LIGHT
 */
STATIC OPERATE_LIGHT opSceneDataCompressToOneUint(CONST IN CHAR_T* input_str, OUT UCHAR_T *output_buf , OUT UCHAR_T *light_mode)
{
    if(NULL == input_str || NULL == output_buf || strlen(input_str) < 26){
        return LIGHT_INVALID_PARM;
    }

    INT_T offset = 0;
    CHAR_T tmp_str[5] = {0};

    //unit  time
    strncpy(tmp_str, input_str + offset, 2);
    USHORT_T time1 = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 2;

    //unit stand time
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 2);
    USHORT_T time2 = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 2;

    //unit change mode
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 2);
    USHORT_T mode = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 2;

    //Hue
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_H = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;

    //Saturation
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_S = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;

    //Value
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_V = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;

    //Bright
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_L = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;

    //Temperature
    memset(tmp_str, 0, 5);
    strncpy(tmp_str, input_str + offset, 4);
    USHORT_T val_T = (USHORT_T) strtol(tmp_str, NULL, 16);
    offset += 4;
    output_buf[0] = ((mode & 0x1) << 7) | (time1 & 0x7F);
    output_buf[1] = (((val_H >> 8) & 0x1) << 7) | (time2 & 0x7F);
    output_buf[2] = (val_H & 0xFF);
    output_buf[3] = (((val_S >> 8) & 0x3) << 6) | (((val_V >> 8) & 0x3) << 4) | (((val_L >> 8) & 0x3) << 2) | ((val_T >> 8) & 0x3);
    output_buf[4] = (val_S & 0xFF);
    output_buf[5] = (val_V & 0xFF);
    output_buf[6] = (val_L & 0xFF);
    output_buf[7] = (val_T & 0xFF);

    *light_mode = mode;
    
    return LIGHT_OK;
}

/**
 * @brief: compress scene data,from string (max 2+26*8=210 bytes) to hex (8+8+64*8)bits = 66 bytes).
 * compressed data format:
 * scene id + mode + uint*8
 * @param {input_str} scene data string format
 * @param {output_buf} Output buffer address, compressed data.
 * @param {output_buf_len} output compressed data length
 * @retval: 1:success / 0:failed
 */
OPERATE_LIGHT opSceneDataCompress(CONST IN CHAR_T* input_str, OUT UCHAR_T* output_buf, OUT UINT_T *output_buf_len)
{
    OPERATE_LIGHT opRet = -1;
    INT_T str_len ; 
    INT_T i = 0;
    UCHAR_T light_mode_tmp = 0;
    CHAR_T tmp_str[5] = {0};
    USHORT_T num;
    
    if(input_str == NULL){
        return LIGHT_INVALID_PARM;
    }

    str_len = strlen(input_str);
    if((0 != (str_len % 2)) || (2 != (str_len % 26))) {
        return -1;
    }

    for(i = 0; (i * 26 + 2) < str_len; i++) {
        opRet = opSceneDataCompressToOneUint(input_str + 2 + i * 26, output_buf + 2 + i * 8, &light_mode_tmp);
        if(opRet != LIGHT_OK) {
            PR_ERR("scene compress error!");
        }
        output_buf[1] |= (((light_mode_tmp >> 1) & 0x1) << i);
        light_mode_tmp = 0;
    }

    //fill scene number
    strncpy(tmp_str, input_str, 2);
    num = (USHORT_T) strtol(tmp_str, NULL, 16);
    output_buf[0] =  (num & 0xFF);

    //calc scene length
    *output_buf_len = i * 8 + 2;

    return LIGHT_OK;
}

/**
 * @brief: decompress one uint scene data, to string format
 * @param {input_buf} scene compressed data
 * @param {light_mode} mode value
 * @param {output_str} output compressed data length
 * @retval: OPERATE_LIGHT
 */
STATIC OPERATE_LIGHT opDecompressSceneDataOneUint(CONST IN UCHAR_T *input_buf, CONST OUT UCHAR_T light_mode, OUT CHAR_T * output_str)
{
    CHAR_T output_str_tmp[27] = {0};
    
    if(NULL == input_buf || NULL == output_str) {
        return LIGHT_INVALID_PARM;
    }

    USHORT_T mode = ((light_mode & 0x1) << 1) | ((input_buf[0] >> 7) & 0x1);
    USHORT_T time1 = (input_buf[0] & 0x7F);
    USHORT_T time2 = (input_buf[1] & 0x7F);
    USHORT_T val_H = (((input_buf[1] >> 7) & 0x1) << 8) | (input_buf[2] & 0xFF);
    USHORT_T val_S = (input_buf[4] & 0xFF) | (((input_buf[3] >> 6) & 0x3) << 8);
    USHORT_T val_V = (input_buf[5] & 0xFF) | (((input_buf[3] >> 4) & 0x3) << 8);
    USHORT_T val_L = (input_buf[6] & 0xFF) | (((input_buf[3] >> 2) & 0x3) << 8);
    USHORT_T val_T = (input_buf[7] & 0xFF) | ((input_buf[3] & 0x3) << 8);

    memset(output_str_tmp, 0, 27);
    snprintf(output_str_tmp, 27, "%02x%02x%02x%04x%04x%04x%04x%04x", time1, time2, mode, val_H, val_S, val_V, val_L, val_T);
    strncpy(output_str, output_str_tmp, 26);
    
    return LIGHT_OK;
}

/**
 * @brief: decompress scene data to string format
 * @param {input_buf} compressed scene data
 * @param {input_buf_len} compressed scene data length
 * @param {output_str} decompressed scene data,string format
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opSceneDataDecompress(CONST IN UCHAR_T* input_buf, CONST IN UINT_T input_buf_len, OUT CHAR_T* output_str)
{
    OPERATE_LIGHT opRet = -1;
    	INT_T i = 0;
    CHAR_T output_str_tmp[27] = {0}; 
    UCHAR_T light_mode_tmp = 0;
    
    if(NULL == input_buf || NULL == output_str || 2 != (input_buf_len % 8)) {
        return LIGHT_INVALID_PARM;
    }
    
    //fill scene number
    snprintf(output_str, 3, "%02x", input_buf[0]);
    
    //fill scene unit
    for(i = 0; (i * 8 + 2) < input_buf_len; i++) {
        memset(output_str_tmp, 0, 27);
        light_mode_tmp = ((input_buf[1] >> i) & 0x1);
        opRet = opDecompressSceneDataOneUint(input_buf + 2 + (i * 8), light_mode_tmp, output_str_tmp);
        if(opRet != LIGHT_OK) {
            PR_ERR("scene decompress error!");
        }
        strncpy(output_str + 2 + (i * 26), output_str_tmp, 26);
        light_mode_tmp = 0;
    }

    return LIGHT_OK;
}

#if (LIGHT_CFG_GAMMA_CAL == 1)
/**
 * @brief: 
 * @param {IN UCHAR_T usColorBri -> gamma index} 
 * @param {IN FLOAT_T fGamaGain -> gamma calc gain} 
 * @param {IN FLOAT_T ucGammaWhite -> gamma calc white balance param}
 * @retval: UCHAR_T
 */
UCHAR_T ucLightToolCalcGamma(IN UCHAR_T usColorBri, IN FLOAT_T fGamaGain, IN FLOAT_T ucGammaWhite)
{

    UCHAR_T ucResult;
    DOUBLE_T lfTemp;

    double p = 255 / pow(255, fGamaGain);
    p = (double)pow(1/p, 1/fGamaGain);
    lfTemp = (double)(usColorBri);
    lfTemp = p * ucGammaWhite * (double)pow(lfTemp, 1/fGamaGain);
    ucResult = lfTemp + 0.5;

    return ucResult;
}
#endif

