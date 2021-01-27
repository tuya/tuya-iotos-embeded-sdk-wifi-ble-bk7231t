/***********************************************************
*  File: tuya_gpio.c
*  Author: nzy
*  Date: 20171106
***********************************************************/
#define __TUYA_GPIO_GLOBALS
#include "tuya_gpio.h"
#include "BkDriverGpio.h"
#include "uart_pub.h"

/***********************************************************
*************************micro define***********************
***********************************************************/


/***********************************************************
*************************variable define********************
***********************************************************/


/***********************************************************
*************************function define********************
***********************************************************/
/***********************************************************
*  Function: tuya_gpio_inout_set
*  Input: port
*         in->TRUE:in
*             FALSE:out
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET tuya_gpio_inout_set(IN CONST TY_GPIO_PORT_E port,IN CONST BOOL_T in)
{

    if(TRUE == in) {
        BkGpioInitialize(port,INPUT_PULL_UP);
    }else {
        BkGpioInitialize(port,OUTPUT_NORMAL);
    }
    return OPRT_OK;
}

/***********************************************************
*  Function: tuya_gpio_inout_select
*  Input: port
*         in->TRUE:in
*             FALSE:out
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET tuya_gpio_inout_set_select(IN CONST TY_GPIO_PORT_E port,IN CONST BOOL_T in,IN CONST BOOL_T high)
{
    if(TRUE == in) {
        if(TRUE == high){
            BkGpioInitialize(port,INPUT_PULL_DOWN);
        }else{
            BkGpioInitialize(port,INPUT_PULL_UP);
        }
    }else {
        BkGpioInitialize(port,OUTPUT_NORMAL);
        if(TRUE == high){
            BkGpioOutputLow(port);
        }else{
            BkGpioOutputHigh(port);
        }
    }
    return OPRT_OK;
}


/***********************************************************
*  Function: tuya_gpio_inout_set
*  Input: port
*         in->TRUE:in
*             FALSE:out
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
INT_T tuya_gpio_read(IN CONST TY_GPIO_PORT_E port)
{
    INT_T ret = BkGpioInputGet(port);
   // os_printf("read io =%d\r\n",ret);
    return ret;
}

/***********************************************************
*  Function: tuya_gpio_write
*  Input: port
*         high->TRUE:high
*               FALSE:low
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET tuya_gpio_write(IN CONST TY_GPIO_PORT_E port,IN CONST BOOL_T high)
{
    if(high == TRUE){
        BkGpioOutputHigh(port);
    }else{
        BkGpioOutputLow(port);
    }
    return OPRT_OK;
}


