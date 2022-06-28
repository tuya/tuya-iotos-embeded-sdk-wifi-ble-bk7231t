/*
 * @Author: yj 
 * @email: shiliu.yang@tuya.com
 * @LastEditors: yj 
 * @file name: tuya_device.c
 * @Description: template demo for SDK WiFi & BLE for BK7231T
 * @Copyright: HANGZHOU TUYA INFORMATION TECHNOLOGY CO.,LTD
 * @Company: http://www.tuya.com
 * @Date: 2021-01-23 16:33:00
 * @LastEditTime: 2021-01-27 17:00:00
 */

#define _TUYA_DEVICE_GLOBAL

/* Includes ------------------------------------------------------------------*/
#include "uni_log.h"
#include "tuya_iot_wifi_api.h"
#include "tuya_hal_system.h"
#include "tuya_iot_com_api.h"
#include "tuya_cloud_com_defs.h"
#include "gw_intf.h"
#include "gpio_test.h"
#include "tuya_gpio.h"
#include "tuya_key.h"
#include "tuya_led.h"
#include "wlan_ui_pub.h"

#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

#include "mem_pub.h"
#include "str_pub.h"
#include "ethernet_intf.h"


static int g_secondsElapsed = 0;

beken_timer_t led_timer;

const char *CFG_GetDeviceName() {
	return "obk_demo_helloWorld";
}


static void app_led_timer_handler(void *data)
{
	g_secondsElapsed ++;
    bk_printf("Hello world - timer is %i free mem %d\r\n", g_secondsElapsed, xPortGetFreeHeapSize());
}

// receives status change notifications about wireless - could be useful
// ctxt is pointer to a rw_evt_type
void wl_status( void *ctxt ){


}

void user_main(void)
{
    OSStatus err;

    bk_printf("[OpenBK2731T Hello world]!\r\n");


    err = rtos_init_timer(&led_timer,
                          1 * 1000,
                          app_led_timer_handler,
                          (void *)0);
    ASSERT(kNoErr == err);

    err = rtos_start_timer(&led_timer);
    ASSERT(kNoErr == err);
	bk_printf("[ObkHello] started timer\r\n");
}
