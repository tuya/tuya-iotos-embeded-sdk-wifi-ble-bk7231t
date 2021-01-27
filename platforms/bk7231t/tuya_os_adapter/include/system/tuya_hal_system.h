/**
 * @file tuya_hal_system.c
 * @author maht@tuya.com
 * @brief 系统接口封装
 * @version 0.1
 * @date 2019-08-15
 * 
 * @copyright Copyright (c) tuya.inc 2019
 * 
 */

#ifndef _TUYA_HAL_SYSTEM_H
#define _TUYA_HAL_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if OPERATING_SYSTEM == SYSTEM_LINUX
typedef uint64_t SYS_TICK_T;
#else
typedef uint32_t SYS_TICK_T;
#endif

typedef enum {
    TUYA_RESET_REASON_POWERON = 0,
    TUYA_RESET_REASON_HARDWARE_WATCHDOG,
    TUYA_RESET_REASON_SOFTWARE_WATCHDOG = 3,
    TUYA_RESET_REASON_SOFTWARE = 4,
    TUYA_RESET_REASON_DEEPSLEEP = 5,
} TUYA_RESET_REASON_E;

#define STR_POWERUP_EVENT(S) \
    ((S) == TUYA_RESET_REASON_POWERON ? "TUYA_RESET_REASON_POWERON" : \
    ((S) == TUYA_RESET_REASON_HARDWARE_WATCHDOG ? "TUYA_RESET_REASON_HARDWARE_WATCHDOG" : \
    ((S) == TUYA_RESET_REASON_SOFTWARE_WATCHDOG ? "TUYA_RESET_REASON_SOFTWARE_WATCHDOG" : \
    ((S) == TUYA_RESET_REASON_SOFTWARE ? "TUYA_RESET_REASON_SOFTWARE" : \
    ((S) == TUYA_RESET_REASON_DEEPSLEEP ? "TUYA_RESET_REASON_DEEPSLEEP" : \
"Unknown")))))

typedef enum {
    TY_RST_POWER_OFF = 0,
    TY_RST_HARDWARE_WATCHDOG,
    TY_RST_FATAL_EXCEPTION,
    TY_RST_SOFTWARE_WATCHDOG,
    TY_RST_SOFTWARE,
    TY_RST_DEEPSLEEP,
    TY_RST_HARDWARE,
    TY_RST_OTHER = 0xAA,
    TY_RST_UNSUPPORT = 0xFF,
} TY_RST_REASON_E;

/**
 * @brief tuya_hal_get_systemtickcount用于获取系统运行ticket 
 * 		count
 * @return SYS_TICK_T 
 */
SYS_TICK_T tuya_hal_get_systemtickcount(void);

/**
 * @brief tuya_hal_get_tickratems用于获取系统ticket是多少个ms
 * 
 * @return uint32_t the time is ms of a system ticket
 */
uint32_t tuya_hal_get_tickratems(void);

/**
 * @brief tuya_hal_system_sleep用于系统sleep
 * 
 * @param[in] msTime sleep time is ms
 */
void tuya_hal_system_sleep(const unsigned long msTime);

/**
 * @brief tuya_hal_system_reset用于重启系统
 * 
 */
void tuya_hal_system_reset(void);

/**
 * @brief tuya_hal_watchdog_init_start用于初始化并运行watchdog
 * 
 * @param[in] timeval watch dog检测时间间隔
 */
void tuya_hal_watchdog_init_start(int timeval);

/**
 * @brief tuya_hal_watchdog_refresh用于刷新watch dog
 * 
 */
void tuya_hal_watchdog_refresh(void);

/**
 * @brief tuya_hal_watchdog_stop用于停止watch dog
 * 
 */
void tuya_hal_watchdog_stop(void);

/**
 * @brief tuya_hal_system_getheapsize用于获取堆大小/剩余内存大小
 * 
 * @return int <0: don't support  >=0: current heap size/free memory
 */
int tuya_hal_system_getheapsize(void);

/**
 * @brief tuya_hal_get_serialno 用于获取硬件串口号
 * 
 * @return char* 硬件串口号字符串
 */
char *tuya_hal_get_serialno(void);

/**
 * @brief tuya_hal_system_get_rst_info用于获取硬件重启原因
 * 
 * @return char* 硬件重启原因
 */
TY_RST_REASON_E tuya_hal_system_get_rst_info(void);

/**
 * @brief tuya_hal_random用于获取随机数
 * 
 * @return uint32_t 随机数
 */
uint32_t tuya_hal_random(void);

/**
 * @brief tuya_hal_get_random_data用于获取指定条件下的随机数
 * 
 * @param[in] dst 
 * @param[in] size 
 * @param[in] range 
 * @return int 0=成功，非0=失败
 */
int tuya_hal_get_random_data(uint8_t* dst, int size, uint8_t range);

/**
 * @brief tuya_hal_set_lp_mode用于设置低功耗模式
 * 
 * @param[in] lp_enable 是否使能低功耗模式
 */
void tuya_hal_set_lp_mode(bool lp_enable);


/**
 * @brief tuya_hal_get_lp_mode用于获取当前低功耗模式
 * 
 * @return true  当前为低功耗模式
 * @return false 当前为非低功耗模式
 */
bool tuya_hal_get_lp_mode(void);

/**
 * @brief 判断本次重启原因是否为电源重启，仅用于计数重置模式。
 *        若模块无法判断重启原因，且需要计数重置功能，直接返回true;
 *        若模块无法判断重启原因，且不需要计数重置功能，直接返回false;
 * 
 * @return true 本次重启原因是电源重启
 * @return false 本次重启原因不是电源重启
 */
bool tuya_hal_system_rst_reason_poweron(void);

/**
 * @brief get reset reason
 * 
 * @return  TUYA_RESET_REASON_E
 */
TUYA_RESET_REASON_E tuya_hal_system_get_rst_reason(void);

/**
 * @brief tuya_hal_output_log用于输出log信息
 * 
 * @param[in] *str log buffer指针
 */
void tuya_hal_output_log(const char *str);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

