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

#include <saradc_pub.h>
#include <drv_model_pub.h>

static int g_secondsElapsed = 0;

beken_timer_t led_timer;

const char *CFG_GetDeviceName() {
	return "obk_demo_helloWorld";
}

static int adcToGpio[] = {
	-1,		// ADC0 - VBAT
	4, //GPIO4,	// ADC1
	5, //GPIO5,	// ADC2
	23,//GPIO23, // ADC3
	2,//GPIO2,	// ADC4
	3,//GPIO3,	// ADC5
	12,//GPIO12, // ADC6
	13,//GPIO13, // ADC7
};
static int c_adcToGpio = sizeof(adcToGpio)/sizeof(adcToGpio[0]);

static uint16_t adcData[1];


static uint8_t gpioToAdc(int gpio) {
	uint8_t i;
	for ( i = 0; i < sizeof(adcToGpio); i++) {
		if (adcToGpio[i] == gpio)
			return i;
	}
	return 0;
}
uint16_t analogReadVoltage(int pinNumber) {
	UINT32 status;
	saradc_desc_t adc;
	DD_HANDLE handle;
	
	saradc_config_param_init(&adc);
	adc.channel		   = gpioToAdc(pinNumber);
	adc.mode		   = ADC_CONFIG_MODE_CONTINUE;
	adc.pData		   = adcData;
	adc.data_buff_size = 1;
	handle			   = ddev_open(SARADC_DEV_NAME, &status, (uint32_t)&adc);
	if (status)
		return 0;
	// wait for data
	while (!adc.has_data || adc.current_sample_data_cnt < 1) {
		delay(1);
	}
	ddev_control(handle, SARADC_CMD_RUN_OR_STOP_ADC, (void *)false);
	ddev_close(handle);
	return adcData[0];
}


static void app_led_timer_handler(void *data)
{
	int val;

	g_secondsElapsed ++;
    bk_printf("Hello world - timer is %i free mem %d\r\n", g_secondsElapsed, xPortGetFreeHeapSize());
	if(g_secondsElapsed > 10){
		val = analogReadVoltage(23);
		bk_printf("ADC = %i\r\n", val);
	}
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
