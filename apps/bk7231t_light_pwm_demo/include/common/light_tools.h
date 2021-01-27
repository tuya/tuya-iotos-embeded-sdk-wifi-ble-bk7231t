/*
 * @Author: wls
 * @email: wuls@tuya.com
 * @LastEditors:   
 * @file name: light_toolkit.h
 * @Description: light common tool include file
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2019-04-26 13:55:40
 * @LastEditTime: 2019-05-28 15:24:01
 */

#ifndef __LIHGT_TOOLS_H__
#define __LIHGT_TOOLS_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "light_types.h"
#include "light_cfg.h"

#define size_get(x) (sizeof(x)/sizeof(x[0]))

/**
 * @brief: get the max value in 5 numbers
 * @param { UINT_T a -> number1 }
 * @param { UINT_T b -> number2 } 
 * @param { UINT_T c -> number3 }
 * @param { UINT_T d -> number4 }
 * @param { UINT_T e -> number5 } 
 * @retval: Max value
 */
UINT_T uiLightToolGetMaxValue(IN UINT_T a, IN UINT_T b, IN UINT_T c, IN UINT_T d, IN UINT_T e);

/**
 * @brief: get the absolute value
 * @param { INT_T value -> calc value } 
 * @retval: absolute value
 */
UINT_T uiLightToolGetABSValue(IN INT_T value);

/**
 * @berief: val bit valid
 * @param { val bit } 
 * @return: valid or not valid
 * @retval: none
 */

UCHAR_T ucLightToolIsBitValid(IN UCHAR_T val, IN UCHAR_T bit);

/**
 * @brief: convert dec string to dec hex
 * @param {decStr} dec string , like: "123"; 
 * @param {strLen} dec string len; 
 * @param {dec} dec hex, like 0x7B. 
 * @retval: dec hex length, 0 ~ 4
 */
UCHAR_T ucLightToolSTR2Dec(IN CHAR_T *decStr, IN UCHAR_T strLen, OUT UINT_T* dec);

/**
 * @brief: ASSIC change to hex
 * @param { CHAR_T AscCode -> ASSIC code } 
 * @return: hex value
 * @retval: HEX
 */
UCHAR_T ucLightToolASC2Hex(IN CHAR_T AscCode);

/**
 * @brief: ASSIC change to hex
 * @param { UCHAR_T H -> high 4 bit } 
 * @param { UCHAR_T L -> low 4 bit } 
 * @retval: UCHAR_T
 */
UCHAR_T ucLightToolSTR2UCHAR(IN UCHAR_T H, IN UCHAR_T L);

/**
 * @brief: four unsigned char merge into unsigned short
 * @param { UCHAR_T HH -> USHORT Hight hight 4bit }
 * @param { UCHAR_T HL -> USHORT Hight low 4bit   }
 * @param { UCHAR_T LH -> USHORT low hight 4bit   }
 * @param { UCHAR_T LL -> USHORT low low 4bit     }
 * @retval: USHORT_T
 */
USHORT_T usLightToolSTR2USHORT(IN UCHAR_T HH, IN UCHAR_T HL, IN UCHAR_T LH, IN UCHAR_T LL);

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
VOID vLightToolHSV2RGB(IN USHORT_T h, IN USHORT_T s, IN USHORT_T v, OUT USHORT_T *r, OUT USHORT_T *g, OUT USHORT_T *b);

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
VOID vLightToolRGB2HSV(IN USHORT_T R, IN USHORT_T G, IN USHORT_T B, OUT USHORT_T *H, OUT USHORT_T *S, OUT USHORT_T *V);

/**
 * @brief: compare two string 
 * @param {IN CHAR_T* str1 -> string 1}
 * @param {IN CHAR_T* str2 -> string 2}
 * @retval: 0: if these two string is not same,
 *          1: these two string is all same
 */
BOOL_T bStringCompare(IN CHAR_T* str1, IN CHAR_T* str2);

/**
 * @brief: change number to str
 * @param {IN CHAR_T cMode -> change format( 0 -> %d, 4-> %4x)}
 * @param {IN UINT_T uiNum -> number(unsigned int)}
 * @param {IN UCHAR_T len -> buf len(sizeof)}
 * @param {OUT CHAR_T *cStr -> string}
 * @retval: string
 */
VOID vNum2Str(IN CHAR_T cMode, IN UINT_T uiNum, IN UCHAR_T len, OUT CHAR_T *cStr);

/**
 * @brief: compress scene data,from string (max 2+26*8=210 bytes) to hex (8+8+64*8)bits = 66 bytes).
 * compressed data format:
 * scene id + mode + uint*8
 * @param {input_str} scene data string format
 * @param {output_buf} Output buffer address, compressed data.
 * @param {output_buf_len} output compressed data length
 * @retval: 1:success / 0:failed
 */
OPERATE_LIGHT opSceneDataCompress(CONST IN CHAR_T* input_str, OUT UCHAR_T* output_buf, OUT UINT_T *output_buf_len);

/**
 * @brief: decompress scene data to string format
 * @param {input_buf} compressed scene data
 * @param {input_buf_len} compressed scene data length
 * @param {output_str} decompressed scene data,string format
 * @retval: OPERATE_LIGHT
 */
OPERATE_LIGHT opSceneDataDecompress(CONST IN UCHAR_T* input_buf, CONST IN UINT_T input_buf_len, OUT CHAR_T* output_str);

#if (LIGHT_CFG_GAMMA_CAL == 1)
/**
 * @brief: 
 * @param {IN UCHAR_T usColorBri -> gamma index} 
 * @param {IN FLOAT_T fGamaGain -> gamma calc gain} 
 * @param {IN FLOAT_T ucGammaWhite -> gamma calc white balance param}
 * @retval: UCHAR_T
 */
UCHAR_T ucLightToolCalcGamma(IN UCHAR_T usColorBri, IN FLOAT_T fGamaGain, IN FLOAT_T ucGammaWhite);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* __LIHGT_TOOLS_H__ */
