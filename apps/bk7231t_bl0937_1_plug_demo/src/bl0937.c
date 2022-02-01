/**
 * @file bl0937.c
 * @brief bl0937 power statistics chip driver
 * @date 2020-03-04
 * 
 * @copyright Copyright (c) {2018-2020} Graffiti Technology www.tuya.com
 * 
 */
 #define _BL0937_GLOBAL
#include "bl0937.h"
#include "tuya_hal_semaphore.h"
#include "tuya_gpio.h"
#include "cJSON.h"
#include "uni_time_queue.h"
#include "uni_log.h"
#include "uf_file.h"
#include "BkDriverTimer.h"
#include "sys_timer.h"
#include "tuya_cloud_com_defs.h"
#include "gw_intf.h"
#include "tuya_iot_com_api.h"

#define COE_SAVE_KEY "coe_save_key"
#define PROD_RSLT_KEY "prod_rslt_key"

#define E_001E_GAIN     4
#define HW_TIMER_ID 0
#define ELE_HW_TIME 1

//--------------------------------------------------------------------------------------------
//Time1 timer, time base = 1ms
#define D_TIME1_20MS				20 		
#define D_TIME1_100MS				100 
#define D_TIME1_150MS				150 	
#define D_TIME1_200MS				200 	
#define D_TIME1_300MS				300 	
#define D_TIME1_400MS				400 	
#define D_TIME1_500MS				500 	
#define D_TIME1_1S				1000		//Time1 Timer timing 1S time constant
#define D_TIME1_2S				2000 	
#define D_TIME1_3S				3000 	
#define D_TIME1_4S				4000 	
#define D_TIME1_5S				5000 
#define D_TIME1_6S				6000 
#define D_TIME1_8S				8000 
#define D_TIME1_9S				9000 
#define D_TIME1_10S				10000 
#define D_TIME1_11S				11000 
#define D_TIME1_20S				20000 

#define I_CAL_TIME                  10000      //Current detection time, SEL low time
#define V_CAL_TIME                  1000       //Voltage detection time, SEL high time
#define D_TIME1_V_OVERFLOW          500        //Time1 timer, the voltage overflow constant is set to 500mS, the overflow indicates that the pulse width period is greater than 500mS
#define D_TIME1_I_OVERFLOW          5000       //Time1 timer, the current overflow constant is set to 5S, the overflow indicates that the pulse width period is greater than 5S
#define D_TIME1_P_OVERFLOW          5000       //Time1 timer, the power overflow constant is set to 5S, the overflow indicates that the pulse width period is greater than 5S
//Correction time, record the number of pulses during this time
#define D_TIME1_CAL_TIME        (18000000/E_001E_GAIN/driver_dltj.p_def)

#define Get_ELE_PORT            driver_dltj.epin                //Power statistics port
#define Get_CUR_VOL             driver_dltj.ivpin               //Current and voltage statistics port
#define CUR_VOL_SWITCH          driver_dltj.ivcpin.pin          //Current and voltage switch port   
#define ENTER_V_MODE            tuya_gpio_write(CUR_VOL_SWITCH, (driver_dltj.ivcpin.type == IO_DRIVE_LEVEL_LOW ? DRV_GPIO_LOW : DRV_GPIO_HIGH))
#define ENTER_I_MODE            tuya_gpio_write(CUR_VOL_SWITCH, (driver_dltj.ivcpin.type == IO_DRIVE_LEVEL_LOW ? DRV_GPIO_HIGH : DRV_GPIO_LOW))
//#define GPIO16_ENTER_V          gpio16_output_set(driver_dltj.ivcpin.type == IO_DRIVE_LEVEL_LOW ? 0 : 1)
//#define GPIO16_ENTER_I          gpio16_output_set(driver_dltj.ivcpin.type == IO_DRIVE_LEVEL_LOW ? 1 : 0)
#define DEF_V                   driver_dltj.v_def
#define DEF_I                   driver_dltj.i_def
#define DEF_P                   driver_dltj.p_def

#define my_abs(x,y) ((x)>(y) ? (x)-(y):(y)-(x))

enum drv_gpio_level{
    DRV_GPIO_LOW = 0,
    DRV_GPIO_HIGH,
};

typedef struct{
    uint32 v;
    uint32 i;
    uint32 p;
    uint32 e;
    uint32 prod_rslt;
}ELE_CAL_DATA;

SEM_HANDLE ele_cal_busy;
//xSemaphoreHandle get_ele_busy;
TIMER_ID get_ele_timer;
BOOL_T ele_cal_flag = FALSE;
STATIC DLTJ_CONFIG driver_dltj;

u32 P_VAL;
u32 V_VAL;
u32 I_VAL;
u32 E_VAL;
ELE_CAL_DATA ele_cal_data;

//--------------------------------------------------------------------------------------------
u16	U16_P_TotalTimes;			//Current pulse Power measurement total time
u16	U16_V_TotalTimes;			//Current pulse Voltage measurement total time
u16	U16_I_TotalTimes;			//Current pulse Current measurement total time

u16	U16_P_OneCycleTime;			//Power Measurement Time Parameters
u16	U16_V_OneCycleTime;			//Voltage measurement time parameter
u16	U16_I_OneCycleTime;			//Current measurement time parameter

u16	U16_P_Last_OneCycleTime;		//Power measurement time parameter, last quantity value
u16	U16_V_Last_OneCycleTime;		//Voltage measurement time parameter, last quantity value
u16	U16_I_Last_OneCycleTime;		//Current measurement time parameter, last quantity value

u16	U16_P_CNT;				//Number of power measurement pulses
u16	U16_V_CNT;				//Number of voltage measurement pulses
u16	U16_I_CNT;				//Number of current measurement pulses

u16	U16_P_Last_CNT;				//Power measurement pulse number, last number value
u16	U16_V_Last_CNT;				//Voltage measurement pulse number, last number value
u16	U16_I_Last_CNT;				//Current measurement pulse number, last number value

BOOL	B_P_TestOneCycle_Mode;			//Power measurement mode 1: single cycle measurement, 0:1S timing measurement
BOOL	B_V_TestOneCycle_Mode;
BOOL	B_I_TestOneCycle_Mode;

BOOL	B_P_Last_TestOneCycle_Mode;
BOOL	B_V_Last_TestOneCycle_Mode;
BOOL	B_I_Last_TestOneCycle_Mode;
    		
BOOL  	B_P_OVERFLOW;       			// Power Pulse Period Overflow Flag 
BOOL  	B_V_OVERFLOW;       			// Voltage pulse period Overflow flag bit
BOOL  	B_I_OVERFLOW;       			// Current pulse period Overflow flag bit

BOOL	B_P_Last_OVERFLOW;       		// Power Pulse Period Overflow Flag
BOOL  	B_V_Last_OVERFLOW;       		// Voltage pulse period Overflow flag bit
BOOL  	B_I_Last_OVERFLOW;       		// Current pulse period Overflow flag bit

BOOL    B_VI_Test_Mode;				// 1: Voltage measurement mode; 0: Current measurement mode
u16   	U16_VI_Test_Times;				
u16   	U16_Cal_Times;	

u16   	U16_AC_P;				//Power value 1000.0W
u16   	U16_AC_V;				//Voltage value 220.0V
u16   	U16_AC_I;				//Current value 4.545A
u32   	U32_AC_E;				//energy used   0.01 degrees

u32  	U16_REF_001_E_Pluse_CNT;        	//0.1 degree reference value of the total number of electrical pulses
u32   	U16_E_Pluse_CNT;          	 	//Pulse number register

u32   	U32_Cal_Times;                 		//Correction time

u32   	U32_P_REF_PLUSEWIDTH_TIME;      	//Reference power Pulse period
u32   	U32_V_REF_PLUSEWIDTH_TIME;      	//Reference voltage Pulse period
u32   	U32_I_REF_PLUSEWIDTH_TIME;      	//Reference current Pulse period

u32   	U32_P_CURRENT_PLUSEWIDTH_TIME;      	//Reference current Pulse period Current power Pulse period
u32   	U32_V_CURRENT_PLUSEWIDTH_TIME;      	//Current voltage Pulse period
u32   	U32_I_CURRENT_PLUSEWIDTH_TIME;      	//Current current Pulse period

u16   	U16_P_REF_Data;				//Reference power value, such as 1000W correction. 1000.0W
u16   	U16_V_REF_Data;				//Reference voltage 220.0V
u16   	U16_I_REF_Data;				//Reference current 1000W, 4.545A at 220V

u8    	U8_CURR_WorkMode;
//--------------------------------------------------------------------------------------------

/*-------------------------------------------- Power, voltage, current calculation -------------------------------------------*/
STATIC OPERATE_RET get_ele_coe_flash(VOID);

/*********************************************************************************
 * FUNCTION:       dltj_copy
 * DESCRIPTION:    global variable assignment
 * INPUT:          src：Structure of parameters related to electricity statistics
 * OUTPUT:         des：Structure of parameters related to power statistics 
 * RETURN:         none
 * OTHERS:         bl0937 chip pin configuration and parameter assignment
 * HISTORY:        2020-07-29
 *******************************************************************************/
VOID dltj_copy(DLTJ_CONFIG *des, DLTJ_CONFIG *src)
{
    des->epin           = src->epin;
    des->ivpin          = src->ivpin;
    des->ivcpin.pin     = src->ivcpin.pin;
    des->ivcpin.type    = src->ivcpin.type;
    des->v_ref          = src->v_ref;
    des->i_ref          = src->i_ref;
    des->p_ref          = src->p_ref;
    des->e_ref          = src->e_ref;
    des->v_def          = src->v_def;
    des->i_def          = src->i_def;
    des->p_def          = src->p_def;
    des->dp_vcoe        = src->dp_vcoe;
    des->dp_icoe        = src->dp_icoe;
    des->dp_pcoe        = src->dp_pcoe;
    des->dp_ecoe        = src->dp_ecoe;
    des->dp_pt_rslt     = src->dp_pt_rslt;
}

/*********************************************************************************
 * FUNCTION:       bl0937_init
 * DESCRIPTION:    bl0937 chip pin configuration and parameter settings
 * INPUT:          dltj: Structure of parameters related to electricity statistics
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         bl0937 chip pin configuration and parameter settings
 * HISTORY:        2020-07-29
 *******************************************************************************/
VOID bl0937_init(DLTJ_CONFIG *dltj)
{
    dltj_copy(&(driver_dltj),dltj);
}

/*=====================================================
 * Function : HLW8012_Measure_P
 * Describe : none
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2014/04/14
=====================================================*/
STATIC VOID HLW8012_Measure_P(VOID)
{
    if (B_P_Last_TestOneCycle_Mode == 1) {
        U32_P_CURRENT_PLUSEWIDTH_TIME = U16_P_Last_OneCycleTime*1000;
//        PR_NOTICE("[caojq]single_preoid p_last_time:%d,p_plusewidth_time:%d",U16_P_Last_OneCycleTime,U32_P_CURRENT_PLUSEWIDTH_TIME);
    }else {
        if(U16_P_Last_CNT < 2){
            return;
        }
        U32_P_CURRENT_PLUSEWIDTH_TIME = U16_P_Last_OneCycleTime*1000/(U16_P_Last_CNT-1);
//        PR_NOTICE("[caojq]aver_preoid p_last_time:%d,p_last_count:%d,p_plusewidth_time:%d",U16_P_Last_OneCycleTime,U16_P_Last_CNT,U32_P_CURRENT_PLUSEWIDTH_TIME);
    }

    if (U8_CURR_WorkMode == D_CAL_START_MODE) {
        U32_P_REF_PLUSEWIDTH_TIME = U32_P_CURRENT_PLUSEWIDTH_TIME;	   // Take the U32_P_CURRENT_PLUSEWIDTH_TIME parameter as the reference value during calibration 
        return;
    }
    if(U32_P_CURRENT_PLUSEWIDTH_TIME == 0){
        U16_AC_P = 0;
    }else{
        U16_AC_P = U16_P_REF_Data * U32_P_REF_PLUSEWIDTH_TIME/U32_P_CURRENT_PLUSEWIDTH_TIME;
    }
    
    if (U16_AC_P == 0xffff)     //U32_P_CURRENT_PLUSEWIDTH_TIME = 0 at boot time, calculation overflow
    {
        U16_AC_P = 0; 
    }
    
    if (B_P_Last_OVERFLOW == TRUE) {
        U16_AC_P = 0;
    }
}
/*=====================================================
 * Function : HLW8012_Measure_V
 * Describe : none
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2020-07-29
=====================================================*/
VOID HLW8012_Measure_V(VOID)
{
    if (B_V_Last_TestOneCycle_Mode == 1) {
        U32_V_CURRENT_PLUSEWIDTH_TIME = U16_V_Last_OneCycleTime*1000;  
//        PR_NOTICE("[caojq]single_preoid v_last_time:%d,v_plusewidth_time:%d",U16_V_Last_OneCycleTime,U32_V_CURRENT_PLUSEWIDTH_TIME);
    }else {
        if(U16_V_Last_CNT < 2){
            return;
        }
        U32_V_CURRENT_PLUSEWIDTH_TIME = U16_V_Last_OneCycleTime*1000/(U16_V_Last_CNT-1);  
//        PR_NOTICE("[caojq]aver_preoid v_last_time:%d,v_last_count:%d,v_plusewidth_time:%d",U16_V_Last_OneCycleTime,U16_V_Last_CNT,U32_V_CURRENT_PLUSEWIDTH_TIME);
    }
    if (U8_CURR_WorkMode == D_CAL_START_MODE) {
        U32_V_REF_PLUSEWIDTH_TIME = U32_V_CURRENT_PLUSEWIDTH_TIME;	   // Take the u32_V_Period parameter as the reference value during calibration
        return;
    }
    if(U32_V_CURRENT_PLUSEWIDTH_TIME == 0){
        U16_AC_V = 0;
    }else{
        U16_AC_V = U16_V_REF_Data * U32_V_REF_PLUSEWIDTH_TIME/U32_V_CURRENT_PLUSEWIDTH_TIME;
    }

    if (U16_AC_V == 0xffff)     //U32_V_CURRENT_PLUSEWIDTH_TIME = 0 at boot time, calculation overflow
    {
        U16_AC_V = 0; 
    }
    
     if (B_V_Last_OVERFLOW == TRUE)
    {
        U16_AC_V = 0;
    }
    
}
/*=====================================================
 * Function : HLW8012_Measure_I
 * Describe : none
 * Input    : none
 * Output   : none
 * Return   : none
 * Record   : 2020-07-29
=====================================================*/
VOID HLW8012_Measure_I(VOID)
{
    if(B_I_Last_TestOneCycle_Mode == 1) {
        U32_I_CURRENT_PLUSEWIDTH_TIME = U16_I_Last_OneCycleTime*1000; 
//        PR_NOTICE("[caojq]single_preoid I_last_time:%d,I_plusewidth_time:%d",U16_I_Last_OneCycleTime,U32_I_CURRENT_PLUSEWIDTH_TIME);
    }else {
        if(U16_I_Last_CNT < 2){
            return;
        }
         U32_I_CURRENT_PLUSEWIDTH_TIME = U16_I_Last_OneCycleTime*1000/(U16_I_Last_CNT-1); 
//       PR_NOTICE("[caojq]aver_preoid I_last_time:%d,I_last_count:%d,I_plusewidth_time:%d",U16_I_Last_OneCycleTime,U16_I_Last_CNT,U32_I_CURRENT_PLUSEWIDTH_TIME);
    }
    if (U8_CURR_WorkMode == D_CAL_START_MODE) {
        U32_I_REF_PLUSEWIDTH_TIME = U32_I_CURRENT_PLUSEWIDTH_TIME;	   // Take the U32_V_CURRENT_PLUSEWIDTH_TIME parameter as the reference value during calibration	 
        return;
    }
    if(U32_I_CURRENT_PLUSEWIDTH_TIME == 0){
        U16_AC_I = 0;
    }else{
        U16_AC_I = U16_I_REF_Data * U32_I_REF_PLUSEWIDTH_TIME/U32_I_CURRENT_PLUSEWIDTH_TIME;
    }

    if (U16_AC_P == 0) {
        U16_AC_I = 0;
    }
  
    if (U16_AC_I == 0xffff)     //U32_I_CURRENT_PLUSEWIDTH_TIME = 0 at boot, calculation overflow
    {
        U16_AC_I = 0; 
    }
    
    if (B_I_Last_OVERFLOW == TRUE)
    {
        U16_AC_I = 0;  
    }
}

VOID gpio_interrupt(UCHAR_T en_pin)
{
    if(en_pin == Get_ELE_PORT){
		U16_P_TotalTimes = 0;		//Complete a valid measurement, the overflow register is cleared
		U16_P_CNT++;
		if (B_P_OVERFLOW == TRUE)
		{  
			//从溢出模式转入,开始测量	  
			B_P_TestOneCycle_Mode = 0;	//Initialize to count pulse measurement mode
			U16_P_TotalTimes = 0;		//clear overflow register
			U16_P_OneCycleTime = 0; 	//clear measurement register
			U16_P_CNT = 1;				
			B_P_OVERFLOW = FALSE;		//clear overflow flag
		}
		else
		{
			if (B_P_TestOneCycle_Mode == 1)
			{
				if (U16_P_OneCycleTime >= D_TIME1_100MS)
				{
					//Single cycle measurement mode 
					U16_P_Last_OneCycleTime = U16_P_OneCycleTime;
					B_P_Last_TestOneCycle_Mode = B_P_TestOneCycle_Mode;
					B_P_OVERFLOW = FALSE;		//The overflow flag is cleared
					B_P_Last_OVERFLOW = B_P_OVERFLOW;
					 //Clear the status parameters and restart the test
					B_P_TestOneCycle_Mode = 0;	//Initialize to count pulse measurement mode
					U16_P_TotalTimes = 0;		//Complete a valid measurement, the overflow register is cleared
					U16_P_OneCycleTime = 0; 	//clear measurement register
					U16_P_CNT = 1;
				}
			}
			else
			{
				if (U16_P_OneCycleTime >= D_TIME1_2S)
				{	
					U16_P_Last_OneCycleTime = U16_P_OneCycleTime;
					U16_P_Last_CNT = U16_P_CNT;
					B_P_Last_TestOneCycle_Mode = B_P_TestOneCycle_Mode;
					B_P_OVERFLOW = FALSE;		//The overflow flag is cleared
					B_P_Last_OVERFLOW = B_P_OVERFLOW;
					//Clear the status parameters and restart the test
					B_P_TestOneCycle_Mode = 0;	//Initialize to count pulse measurement mode
					U16_P_TotalTimes = 0;		//Complete a valid measurement, the overflow register is cleared
					U16_P_OneCycleTime = 0; 	//clear measurement register
					U16_P_CNT = 1;
				}
			}
		}
		//Correction mode
		if (U8_CURR_WorkMode == D_CAL_START_MODE)
		{
			//Record the electricity consumption per unit time
			U16_E_Pluse_CNT++;
		}
		
	//Electricity consumption metering, every 0.1 kWh, the electricity consumption register increases by 0.1 kWh
		if (U8_CURR_WorkMode == D_NORMAL_MODE)
		{
			U16_E_Pluse_CNT++;
			if (U16_E_Pluse_CNT == 2 * U16_REF_001_E_Pluse_CNT )
			{
				U16_E_Pluse_CNT = 0;
				U32_AC_E++;
			}
		}
	}
	if(en_pin == Get_CUR_VOL){
		//Voltage test mode
		if (B_VI_Test_Mode == 1)
		{
			U16_V_TotalTimes = 0; 
			U16_V_CNT++;
			if (B_V_OVERFLOW == TRUE)
			{				   
				//Transfer from overflow mode to start measurement
				B_V_TestOneCycle_Mode = 0;	//Initialize to count pulse measurement mode
				U16_V_TotalTimes = 0;		//clear overflow register
				U16_V_OneCycleTime = 0; 	//clear measurement register
				U16_V_CNT = 1;				
				B_V_OVERFLOW = FALSE;		//clear overflow flag
			}
			else
			{
				if (B_V_TestOneCycle_Mode == 1)
				{
					if (U16_V_OneCycleTime >= D_TIME1_100MS)
					{
						//Single cycle measurement mode 
						U16_V_Last_OneCycleTime = U16_V_OneCycleTime;
						B_V_Last_TestOneCycle_Mode = B_V_TestOneCycle_Mode;
						B_V_OVERFLOW = FALSE;		//The overflow flag is cleared
						B_V_Last_OVERFLOW = B_V_OVERFLOW;
						 //Clear the status parameters and restart the test
						B_V_TestOneCycle_Mode = 0;	//Initialize to count pulse measurement mode
						U16_V_TotalTimes = 0;		//Complete a valid measurement, the overflow register is cleared
						U16_V_OneCycleTime = 0; 	//clear measurement register
						U16_V_CNT = 1;
					}
				}
				else
				{
					if (U16_V_OneCycleTime >= D_TIME1_300MS)
					{	
						U16_V_Last_OneCycleTime = U16_V_OneCycleTime;
						U16_V_Last_CNT = U16_V_CNT;
						B_V_Last_TestOneCycle_Mode = B_V_TestOneCycle_Mode; 
						B_V_OVERFLOW = FALSE;		//The overflow flag is cleared
						B_V_Last_OVERFLOW = B_V_OVERFLOW;
						//Clear the status parameters and restart the test
						B_V_TestOneCycle_Mode = 0;	//Initialize to count pulse measurement mode
						U16_V_TotalTimes = 0;		//Complete a valid measurement, the overflow register is cleared
						U16_V_OneCycleTime = 0; 	//clear measurement register
						U16_V_CNT = 1;
						B_V_OVERFLOW = FALSE;		//The overflow flag is cleared
					}
				}
			}
		 }

	//Current test mode
		if (B_VI_Test_Mode == 0)
		{
			U16_I_TotalTimes = 0; 
			U16_I_CNT++;
			if (B_I_OVERFLOW == TRUE)
			{
				//Transfer from overflow mode to start measurement	  
				B_I_TestOneCycle_Mode = 0;	//Initialize to count pulse measurement mode
				U16_I_TotalTimes = 0;		//clear overflow register
				U16_I_OneCycleTime = 0; 	//clear measurement register
				U16_I_CNT = 1;				
				B_I_OVERFLOW = FALSE;		//clear overflow flag
			}
			else
			{
				if (B_I_TestOneCycle_Mode == 1)
				{
					if (U16_I_OneCycleTime >= D_TIME1_100MS)
					{
						//Single cycle measurement mode 
						U16_I_Last_OneCycleTime = U16_I_OneCycleTime;
						B_I_Last_TestOneCycle_Mode = B_I_TestOneCycle_Mode;
						B_I_OVERFLOW = FALSE;		//The overflow flag is cleared
						B_I_Last_OVERFLOW = B_I_OVERFLOW;
						 //Clear the status parameters and restart the test
						B_I_TestOneCycle_Mode = 0;	//Initialize to count pulse measurement mode
						U16_I_TotalTimes = 0;		//Complete a valid measurement, the overflow register is cleared
						U16_I_OneCycleTime = 0; 	//clear measurement register
						U16_I_CNT = 1;
					}
				}
				else
				{
					if (U16_I_OneCycleTime >= D_TIME1_1S)
					{	
						U16_I_Last_OneCycleTime = U16_I_OneCycleTime;
						U16_I_Last_CNT = U16_I_CNT;
						B_I_Last_TestOneCycle_Mode = B_I_TestOneCycle_Mode;  
						B_I_OVERFLOW = FALSE;		//The overflow flag is cleared
						B_I_Last_OVERFLOW = B_I_OVERFLOW;
						//Clear the status parameters and restart the test
						B_I_TestOneCycle_Mode = 0;	//Initialize to count pulse measurement mode
						U16_I_TotalTimes = 0;		//Complete a valid measurement, the overflow register is cleared
						U16_I_OneCycleTime = 0; 	//clear measurement register
						U16_I_CNT = 1;
					}
				}
			}
		}
	}
}

STATIC VOID hw_test_timer_cb(void)
{
    if (U8_CURR_WorkMode == D_CAL_START_MODE) {
        U32_Cal_Times++;
        if (U32_Cal_Times == D_TIME1_CAL_TIME)
        {
            U16_REF_001_E_Pluse_CNT = E_001E_GAIN * U16_E_Pluse_CNT;		//Record the number of pulses in the calibration time, the number of pulses represents 0.0005 kWh of electricity
            tuya_hal_semaphore_post(ele_cal_busy);
        }
    }
    
//Power measurement
    if (U16_P_CNT != 0)
    {
        U16_P_OneCycleTime++;
        U16_P_TotalTimes++;
    }

    if (U16_P_TotalTimes >= D_TIME1_P_OVERFLOW)
    {
        B_P_OVERFLOW = TRUE; 		//overflow，
        B_P_Last_OVERFLOW = B_P_OVERFLOW;
        //Clear the status parameters and restart the test
        U16_P_TotalTimes = 0;       //clear overflow register
        U16_P_OneCycleTime = 0;
        U16_P_CNT = 0;              //Wait for the next interrupt to start counting
        B_P_TestOneCycle_Mode = 0;   //Initialize to count pulse measurement mode   
    }
    else if (U16_P_OneCycleTime == D_TIME1_100MS)
    {
		if (U16_P_CNT < 2)
		{
			// There is only one interrupt within 100ms, indicating that the cycle is > 100ms, and the single-cycle measurement mode is used
			B_P_TestOneCycle_Mode = 1;
		}
		else
		{
			 // There are 2 or more pulses within 100ms, indicating that the period is less than 100ms, and the counting pulse measurement mode is used.
			 B_P_TestOneCycle_Mode = 0;   
		}
    }
    
//Voltage and current measurement
    if (B_VI_Test_Mode == 1)
    {
        //Voltage measurement 
        if (U16_V_CNT != 0)
    	{
	        U16_V_OneCycleTime++;
            U16_V_TotalTimes++;
        }

        if (U16_V_TotalTimes >= D_TIME1_V_OVERFLOW)
        {
            B_V_OVERFLOW = TRUE; 
            B_V_Last_OVERFLOW = B_V_OVERFLOW;
            //Clear the status parameters and restart the test
            U16_V_TotalTimes = 0;       //clear overflow register
            U16_V_OneCycleTime = 0;
            U16_V_CNT = 0;              
            B_V_TestOneCycle_Mode = 0;   //Initialize to count pulse measurement mode  
        }
        else if (U16_V_OneCycleTime == D_TIME1_100MS)
        {
			if (U16_V_CNT < 2)
			{
				// There is only one interrupt within 100ms, indicating that the cycle is > 100ms, and the single-cycle measurement mode is used
				B_V_TestOneCycle_Mode = 1;
			}
			else
			{
				// There are 2 or more pulses within 100ms, indicating that the period is less than 100ms, and the counting pulse measurement mode is used.
				B_V_TestOneCycle_Mode = 0;   
			}
        }
    }
    else
    {
        //Current measurement   
        if (U16_I_CNT != 0)
    	{
			U16_I_OneCycleTime++;
			U16_I_TotalTimes++;
        }

        if (U16_I_TotalTimes >= D_TIME1_I_OVERFLOW)
        {
            B_I_OVERFLOW = TRUE; 
            B_I_Last_OVERFLOW = B_I_OVERFLOW;
            //Clear the status parameters and restart the test
            U16_I_TotalTimes = 0;       //clear overflow register
            U16_I_OneCycleTime = 0;
            U16_I_CNT = 0;
            B_I_TestOneCycle_Mode = 0;   //Initialize to count pulse measurement mode      
        }
        else if (U16_I_OneCycleTime == D_TIME1_100MS)
        {
			if (U16_I_CNT < 2)
			{
			// There is only one interrupt within 100ms, indicating that the cycle is > 100ms, and the single-cycle measurement mode is used
			B_I_TestOneCycle_Mode = 1;
			}
			else
			{
			 // There are 2 or more pulses within 100ms, indicating that the period is less than 100ms, and the counting pulse measurement mode is used.
			 B_I_TestOneCycle_Mode = 0;   
			}
        }
    }
      

//Voltage and current measurement mode switching B_VI_Test_Mode: (1: voltage measurement mode) (0: current test mode)
    U16_VI_Test_Times--;
	if(B_VI_Test_Mode == 1){
		if(U16_VI_Test_Times == V_CAL_TIME ){//This condition will not be triggered, the current, voltage and power data will be updated at the same time
			ele_cal_flag = TRUE;
		}
	}else{
		if(U16_VI_Test_Times == 7500) { /* 8S is close to the critical value, false detection may occur, and later changed to 7.5S */
			ele_cal_flag = TRUE;
		}
	} 
	
    if (U16_VI_Test_Times == 0)
    {
    	ele_cal_flag = FALSE;
        if (B_VI_Test_Mode == 1)
        {
            //Switch to current measurement mode
            B_VI_Test_Mode = 0;
            ENTER_I_MODE;
            #if 0
            if(CUR_VOL_SWITCH < 16){
                ENTER_I_MODE;
            }else{
                GPIO16_ENTER_I;
            }
            #endif
            U16_VI_Test_Times = I_CAL_TIME;
            //Clear state parameters
            U16_I_TotalTimes = 0;
            U16_I_OneCycleTime = 0;
            U16_I_CNT = 0;
            B_I_OVERFLOW = FALSE; 
        }
        else
        {
            //Switch to voltage measurement mode
            B_VI_Test_Mode = 1;
            ENTER_V_MODE;
            #if 0
            if(CUR_VOL_SWITCH < 16){
                ENTER_V_MODE;
            }else{
                GPIO16_ENTER_V;
            }
            #endif
            U16_VI_Test_Times = V_CAL_TIME;
            //Clear state parameters
            U16_V_TotalTimes = 0;
            U16_V_OneCycleTime = 0;
            U16_V_CNT = 0;
            B_V_OVERFLOW = FALSE; 
        }
    }
}

/*********************************************************************************
 * FUNCTION:       save_prod_test_data
 * DESCRIPTION:    Production test parameter storage
 * INPUT:          state：Production test results
 * OUTPUT:         none
 * RETURN:         OPERATE_RET：return result
 * OTHERS:         Production test results are saved in flash
 * HISTORY:        2020-03-04
 *******************************************************************************/
STATIC VOID Parameter_Init(void)
{
    U16_AC_P = 0;
    U16_AC_V = 0;
    U16_AC_I = 0;
    P_VAL = 0;
    V_VAL = 0;
    I_VAL = 0;
    E_VAL = 0;

    U16_P_TotalTimes = 0;
    U16_V_TotalTimes = 0;
    U16_I_TotalTimes = 0;

    U16_P_OneCycleTime = 0;
    U16_V_OneCycleTime = 0;
    U16_I_OneCycleTime = 0;
    U16_P_Last_OneCycleTime = 0;
    U16_V_Last_OneCycleTime = 0;
    U16_I_Last_OneCycleTime = 0;

    U16_P_CNT = 0;
    U16_V_CNT = 0;
    U16_I_CNT = 0;
    U16_P_Last_CNT = 0;
    U16_V_Last_CNT = 0;
    U16_I_Last_CNT = 0;

    //Initialize single-cycle measurement mode
    B_P_TestOneCycle_Mode = 1;
    B_V_TestOneCycle_Mode = 1;
    B_I_TestOneCycle_Mode = 1;
    B_P_Last_TestOneCycle_Mode = 1;
    B_V_Last_TestOneCycle_Mode = 1;
    B_I_Last_TestOneCycle_Mode = 1;

    //Start the measurement and set the overflow flag to 1
    B_P_OVERFLOW = 1;
    B_V_OVERFLOW = 1;
    B_I_OVERFLOW = 1;

    B_P_Last_OVERFLOW = 1;
    B_V_Last_OVERFLOW = 1;
    B_I_Last_OVERFLOW = 1;

    //Power-on initialization to voltage test mode
    B_VI_Test_Mode = 1;
    ENTER_V_MODE;
    U16_VI_Test_Times = V_CAL_TIME;

    U32_AC_E = 0;
    E_VAL = 0;
    U16_E_Pluse_CNT = 0;

    //set default value
    U16_P_REF_Data = DEF_P;     // 857 =  85.7W
    U16_V_REF_Data = DEF_V;     // 2202 = 220.2V
    U16_I_REF_Data = DEF_I;     // 386 =  386mA

    U32_Cal_Times = 0;
}

/*********************************************************************************
 * FUNCTION:       save_prod_test_data
 * DESCRIPTION:    Production test parameter storage
 * INPUT:          state：Production test results
 * OUTPUT:         none
 * RETURN:         OPERATE_RET：return result
 * OTHERS:         Production test results are saved in flash
 * HISTORY:        2020-03-04
 *******************************************************************************/
OPERATE_RET save_prod_test_data(INT_T state)
{
    cJSON *root = NULL;
    UCHAR_T *buf = NULL;
    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error");
        return OPRT_CJSON_GET_ERR;
    }

    cJSON_AddNumberToObject(root, "prod_rslt", state);
    buf = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    if(NULL == buf) {
        PR_ERR("buf is NULL");
        return OPRT_COM_ERROR;
    }

    PR_DEBUG("set:%s", buf);
    uFILE* fp = NULL;
    fp = ufopen(PROD_RSLT_KEY,"w+");
    if(NULL == fp) {
        PR_ERR("write to file error");
        Free(buf);
        return OPRT_COM_ERROR;
    }else {
        PR_DEBUG("open file OK");
    }

    INT_T Num = ufwrite(fp, (UCHAR_T *)buf, strlen(buf));
    ufclose(fp);
    if( Num <= 0) {
        PR_ERR("uf write fail %d",Num);
        Free(buf);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("uf write ok %d",Num);

    Free(buf);
    return OPRT_OK;
}

/*********************************************************************************
* FUNCTION:       get_prod_test_data
* DESCRIPTION:    Get production test results
* INPUT:          state：Production test results
* OUTPUT:         none
* RETURN:         OPERATE_RET：return result
* OTHERS:         Get flash mid-production test results
* HISTORY:        2020-03-04
*******************************************************************************/
OPERATE_RET get_prod_test_data(INT_T *state)
{
    if(NULL == state) {
        PR_ERR("get_prod_test_data inparam is null");
        return OPRT_INVALID_PARM;
    }

    uFILE * fp = ufopen(PROD_RSLT_KEY,"r");
    if(NULL == fp) {     /* If it cannot be opened */
        PR_ERR("cannot open file");
        *state = 0;
        return OPRT_COM_ERROR;
    }
    INT_T buf_len = 32;
    CHAR_T *buf = (CHAR_T *)Malloc(buf_len);
    if(NULL == buf) {
        PR_ERR("malloc fail");
        *state = 0;
        return OPRT_MALLOC_FAILED;
    }

    memset(buf,0,buf_len);
    INT_T Num = ufread(fp, (UCHAR_T *)buf,buf_len);
    if(Num <= 0) {
        Free(buf);
        *state = 0;
        PR_ERR("uf read error %d",Num);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("relay_stat_get_buf:%s", buf);
    ufclose(fp);

    cJSON *root = cJSON_Parse(buf);
    if(NULL == root) {
        PR_ERR("cjson parse");
        goto JSON_PARSE_ERR;
    }

    cJSON *json = cJSON_GetObjectItem(root,"prod_rslt");
    if(NULL == json) {
        PR_ERR("cjson get ");
        goto JSON_PARSE_ERR;
    }

    *state = json->valueint;
    cJSON_Delete(root);
    Free(buf);
    return OPRT_OK;

JSON_PARSE_ERR:
    *state = 0;
    cJSON_Delete(root);
    Free(buf);
    return OPRT_COM_ERROR;
}

/*********************************************************************************
* FUNCTION:       set_coefficient
* DESCRIPTION:    Set production test calibration parameters
* INPUT:          none
* OUTPUT:         none
* RETURN:         OPERATE_RET：return result
* OTHERS:         Store calibration parameters to flash
* HISTORY:        2020-07-29
*******************************************************************************/
STATIC OPERATE_RET set_coefficient(VOID)
{
    cJSON *root = NULL;
    UCHAR_T *buf = NULL;

    root = cJSON_CreateObject();
    if(NULL == root) {
        PR_ERR("cJSON_CreateObject error");
        return OPRT_CJSON_GET_ERR;
    }
    
    cJSON_AddNumberToObject(root, "Kp", U32_P_REF_PLUSEWIDTH_TIME);
    cJSON_AddNumberToObject(root, "Kv", U32_V_REF_PLUSEWIDTH_TIME);
    cJSON_AddNumberToObject(root, "Ki", U32_I_REF_PLUSEWIDTH_TIME);
    cJSON_AddNumberToObject(root, "Ke", U16_REF_001_E_Pluse_CNT);
    buf = cJSON_PrintUnformatted(root);
    if(NULL == buf) {
        PR_ERR("buf is NULL");
        cJSON_Delete(root);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("set_coefficient_to_flash:%s",buf);
    cJSON_Delete(root);

    uFILE* fp = NULL;
    fp = ufopen(COE_SAVE_KEY,"w+");
    if(NULL == fp) {
        PR_ERR("write to file error");
        Free(buf);
        return OPRT_COM_ERROR;
    }else {
        PR_DEBUG("open file OK");
    }

    INT_T Num = ufwrite(fp, (UCHAR_T *)buf, strlen(buf));
    ufclose(fp);
    if( Num <= 0) {
        PR_ERR("uf write fail %d",Num);
        Free(buf);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("uf write ok %d",Num);

    Free(buf);
    return OPRT_OK;
}

/*********************************************************************************
* FUNCTION:       get_coefficient
* DESCRIPTION:    Get production test calibration parameters
* INPUT:          d：Calibration parameters
* OUTPUT:         none
* RETURN:         OPERATE_RET：return result
* OTHERS:         Get calibration parameters in flash
* HISTORY:        2020-07-29
*******************************************************************************/
STATIC OPERATE_RET get_coefficient(OUT ELE_CAL_DATA *d)
{
    OPERATE_RET op_ret = OPRT_OK;
    if(NULL == d) {
        PR_ERR("input param is null!");
        return OPRT_INVALID_PARM;
    }

    uFILE * fp = ufopen(COE_SAVE_KEY,"r");
    if(NULL == fp) {     /* If it cannot be opened */
        PR_ERR("cannot open file");
        return OPRT_COM_ERROR;
    }
    INT_T buf_len = 256;
    CHAR_T *buf = (CHAR_T *)Malloc(buf_len);
    if(NULL == buf) {
        PR_ERR("malloc fail");
        return OPRT_MALLOC_FAILED;
    }

    memset(buf,0,buf_len);
    INT_T Num = ufread(fp, (UCHAR_T *)buf,buf_len);
    if(Num <= 0) {
        Free(buf);
        PR_ERR("uf read error %d",Num);
        return OPRT_COM_ERROR;
    }
    PR_DEBUG("relay_stat_get_buf:%s", buf);
    ufclose(fp);

    cJSON *root = cJSON_Parse(buf);
    if(NULL == root) {
        PR_ERR("cjson parse err");
        goto JSON_PARSE_ERR;
    }

    cJSON *json = cJSON_GetObjectItem(root,"Kv");
    if(NULL == json) {
        goto JSON_PARSE_ERR;
    }else{
        d->v = json->valueint;
    }

    json = cJSON_GetObjectItem(root,"Ki");
    if(NULL == json) {
        goto JSON_PARSE_ERR;
    }else{
        d->i = json->valueint;
    }
    json = cJSON_GetObjectItem(root,"Kp");
    if(NULL == json) {
        goto JSON_PARSE_ERR;
    }else{
        d->p = json->valueint;
    }
    json = cJSON_GetObjectItem(root,"Ke");
    if(NULL == json) {
        goto JSON_PARSE_ERR;
    }else{
        d->e = json->valueint;
    }
    // The production test result cannot be read, indicating that the production test has not been carried out, prod_rslt = 2
    op_ret = get_prod_test_data(&d->prod_rslt);
    if(OPRT_OK != op_ret) {
        PR_NOTICE("not get product test data:%d",op_ret);
        d->prod_rslt = 2;
    }
    // If you can't read the production test results, and there are voltage calibration values, it means that the first batch of firmware was upgraded. The earliest batch of firmware does not store prod_rslt, only v,i,p,e
    if ((2 == d->prod_rslt) && (d->v)) {
        d->prod_rslt = 1;
    }
    PR_DEBUG("get_prod_test_data_success:%d",d->prod_rslt);
    cJSON_Delete(root);
    Free(buf);
    return OPRT_OK;

JSON_PARSE_ERR:
    cJSON_Delete(root);
    Free(buf);
    return OPRT_COM_ERROR;
}

STATIC VOID timer_init(VOID)
{
    bk_timer_initialize(HW_TIMER_ID,ELE_HW_TIME,hw_test_timer_cb);
//    bk_timer_stop(HW_TIMER_ID);
}

STATIC VOID gpio_intr_init(VOID)
{
    gpio_int_enable(driver_dltj.epin,IRQ_TRIGGER_FALLING_EDGE,gpio_interrupt);
//    gpio_int_disable(driver_dltj.epin);

    gpio_int_enable(driver_dltj.ivpin,IRQ_TRIGGER_FALLING_EDGE,gpio_interrupt);
//    gpio_int_disable(driver_dltj.ivpin);
}

STATIC VOID get_ele_timer_cb(UINT_T timerID, PVOID_T pTimerArg)
{
    if (ele_cal_flag == TRUE) {
        HLW8012_Measure_P();
        HLW8012_Measure_V();
        HLW8012_Measure_I();
//        xSemaphoreTake(get_ele_busy, portMAX_DELAY);
        P_VAL = U16_AC_P;
        V_VAL = U16_AC_V;
        I_VAL = U16_AC_I;
        E_VAL += U32_AC_E;
        U32_AC_E = 0;
//        xSemaphoreGive(get_ele_busy);
    }
}

STATIC BOOL_T cal_data_judge(VOID)
{
    BOOL_T opt = TRUE;
    PR_NOTICE("V:%d I:%d P:%d E:%d",
        U32_V_REF_PLUSEWIDTH_TIME,
        U32_I_REF_PLUSEWIDTH_TIME,
        U32_P_REF_PLUSEWIDTH_TIME,
        U16_REF_001_E_Pluse_CNT);

    if (my_abs(U32_V_REF_PLUSEWIDTH_TIME, driver_dltj.v_ref) > driver_dltj.v_ref / 2)
        opt = FALSE;
    if (my_abs(U32_I_REF_PLUSEWIDTH_TIME, driver_dltj.i_ref) > driver_dltj.i_ref / 2)
        opt = FALSE;
    if (my_abs(U32_P_REF_PLUSEWIDTH_TIME, driver_dltj.p_ref) > driver_dltj.p_ref / 2)
        opt = FALSE;
    if (my_abs(U16_REF_001_E_Pluse_CNT, driver_dltj.e_ref) > driver_dltj.e_ref / 2)
        opt = FALSE;
    ele_cal_data.v = U32_V_REF_PLUSEWIDTH_TIME;
    ele_cal_data.i = U32_I_REF_PLUSEWIDTH_TIME;
    ele_cal_data.p = U32_P_REF_PLUSEWIDTH_TIME;
    ele_cal_data.e = U16_REF_001_E_Pluse_CNT;
    ele_cal_data.prod_rslt = opt ? 1 : 0;
    return opt;
}

VOID ele_coe_init(VOID)
{
    if(1 == ele_cal_data.prod_rslt) { 
        U32_V_REF_PLUSEWIDTH_TIME = ele_cal_data.v;
        U32_I_REF_PLUSEWIDTH_TIME = ele_cal_data.i;
        U32_P_REF_PLUSEWIDTH_TIME = ele_cal_data.p;
        U16_REF_001_E_Pluse_CNT = ele_cal_data.e;
        PR_NOTICE("ele_coe have been set to cal value!");
    }else {
        U32_V_REF_PLUSEWIDTH_TIME = driver_dltj.v_ref;
        U32_I_REF_PLUSEWIDTH_TIME = driver_dltj.i_ref;
        U32_P_REF_PLUSEWIDTH_TIME = driver_dltj.p_ref;
        U16_REF_001_E_Pluse_CNT = driver_dltj.e_ref;
        PR_NOTICE("ele_coe have been set to default value!");
    }
    PR_DEBUG("ele_coe:v=%d,i=%d,p=%d,e=%d",
        U32_V_REF_PLUSEWIDTH_TIME, U32_I_REF_PLUSEWIDTH_TIME,
        U32_P_REF_PLUSEWIDTH_TIME, U16_REF_001_E_Pluse_CNT);
}

VOID dis_hw_intr_time(VOID)
{
    bk_timer_stop(HW_TIMER_ID);
    gpio_int_disable(driver_dltj.epin);
    gpio_int_disable(driver_dltj.ivpin);
}

/*********************************************************************************
 * FUNCTION:       ele_cnt_init
 * DESCRIPTION:    Initialization of battery pulse count
 * INPUT:          mode：Mode of metering chip (metering mode)
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Metering chip initialization/hardware timer initialization/interrupt initialization/pulse count initialization
 * HISTORY:        2020-03-04
 *******************************************************************************/
OPERATE_RET ele_cnt_init(INT_T mode)
{
    OPERATE_RET op_ret = OPRT_OK;
    U8_CURR_WorkMode = mode;
    if (U8_CURR_WorkMode != D_CAL_START_MODE) {
        op_ret = get_ele_coe_flash();
        ele_coe_init();
    } else {
        U32_V_REF_PLUSEWIDTH_TIME = 0;
        U32_I_REF_PLUSEWIDTH_TIME = 0;
        U32_P_REF_PLUSEWIDTH_TIME = 0;
        U16_REF_001_E_Pluse_CNT = 0;
        PR_DEBUG("enter ele cal mode");
    }
    Parameter_Init();
    timer_init();
    gpio_intr_init();

//    vSemaphoreCreateBinary(get_ele_busy); //create semaphore
    op_ret = sys_add_timer(get_ele_timer_cb, NULL, &get_ele_timer);
    if(OPRT_OK != op_ret) {
//        hw_timer_disable();
//        _xt_isr_unmask(0<<ETS_GPIO_INUM);//disable GPIO
        dis_hw_intr_time();
        PR_ERR("add get_ele_timer err,ele cnt is disable");
        return op_ret;
    }else {
        sys_start_timer(get_ele_timer,500,TIMER_CYCLE);
    }

    if(U8_CURR_WorkMode == D_CAL_START_MODE){
        #if 0
        ele_cal_busy = CreateSemaphore();
        if(NULL == ele_cal_busy){
//            hw_timer_disable();
//            _xt_isr_unmask(0<<ETS_GPIO_INUM);//disable GPIO
            dis_hw_intr_time();
            sys_delete_timer(get_ele_timer); 
            return OPRT_COM_ERROR;
        }
        #endif
        op_ret = tuya_hal_semaphore_create_init(ele_cal_busy,0,1);
        if(OPRT_OK != op_ret) {
//            hw_timer_disable();
//            _xt_isr_unmask(0<<ETS_GPIO_INUM);//disable GPIO
            sys_delete_timer(get_ele_timer); 
            dis_hw_intr_time();
            tuya_hal_semaphore_release(ele_cal_busy);       
            return op_ret;
        }
        tuya_hal_semaphore_wait(ele_cal_busy);
//        hw_timer_disable();
//        _xt_isr_unmask(0<<ETS_GPIO_INUM);//disable GPIO
        dis_hw_intr_time();
        sys_delete_timer(get_ele_timer); 
        tuya_hal_semaphore_release(ele_cal_busy); 
        PR_DEBUG("prod test has last for %d mS",D_TIME1_CAL_TIME);
        if(FALSE == cal_data_judge()){
            INT_T pt_result = 0; // If the previous production test has been successful, the data will not be saved this time, preventing the correct data from being overwritten during production
            if( OPRT_OK == get_prod_test_data(&pt_result) && (1 == pt_result)){
                return OPRT_COM_ERROR;
            }else{ // If the production test has not been successful before, the data will be saved if it fails, which is mainly used for debugging.
                op_ret = set_coefficient();
                if(OPRT_OK != op_ret){
                    return op_ret;
                }
                op_ret = save_prod_test_data(0);
                if(OPRT_OK != op_ret){
                    return op_ret;
                }
            }
            return OPRT_COM_ERROR;
        }else{
            op_ret = set_coefficient();
            if(OPRT_OK != op_ret){
                return op_ret;
            }
            op_ret = save_prod_test_data(1);;
            if(OPRT_OK != op_ret){
                return op_ret;
            }
            return OPRT_OK;
        }
    }

    return OPRT_OK;
}

/*********************************************************************************
 * FUNCTION:       get_ele_par
 * DESCRIPTION:    Get real-time power parameters
 * INPUT:          p/v/i: power/voltage/current
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Get real-time parameters of power chip
 * HISTORY:        2020-03-04
 *******************************************************************************/
VOID get_ele_par(OUT u32 *P,OUT u32 *V,OUT u32 *I)
{
//    xSemaphoreTake(get_ele_busy,portMAX_DELAY);
    *P = P_VAL;
    *V = V_VAL;
    *I = I_VAL;
//    xSemaphoreGive(get_ele_busy);
}

/*********************************************************************************
 * FUNCTION:       get_ele
 * DESCRIPTION:    Get the increase power parameter
 * INPUT:          E:Increase battery value
 * OUTPUT:         none
 * RETURN:         none
 * OTHERS:         Get the power chip increase power parameters
 * HISTORY:        2020-03-04
 *******************************************************************************/
VOID get_ele(OUT UINT_T *E)
{
//    xSemaphoreTake(get_ele_busy,portMAX_DELAY);
    *E = E_VAL;
    E_VAL = 0;
//    xSemaphoreGive(get_ele_busy);
}

 /*********************************************************************************
  * FUNCTION:       report_coe_data
  * DESCRIPTION:    Report power statistics calibration parameters and production test result bits
  * INPUT:          none
  * OUTPUT:         none
  * RETURN:         none
  * OTHERS:         Gas statistics calibration parameters and production test results
  * HISTORY:        2020-03-04
  *******************************************************************************/
OPERATE_RET report_coe_data(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;
     INT_T dp_cnt = 5;
     
     TY_OBJ_DP_S *dp_arr = (TY_OBJ_DP_S *)Malloc(dp_cnt*SIZEOF(TY_OBJ_DP_S));
     if(NULL == dp_arr) {
         PR_ERR("malloc failed");
         return OPRT_MALLOC_FAILED;
     }
     
     memset(dp_arr, 0, dp_cnt*SIZEOF(TY_OBJ_DP_S));

     dp_arr[0].dpid = driver_dltj.dp_pt_rslt;
     dp_arr[0].type = PROP_VALUE;
     dp_arr[0].time_stamp = 0;
     dp_arr[0].value.dp_value = ele_cal_data.prod_rslt;
     
     dp_arr[1].dpid = driver_dltj.dp_vcoe;
     dp_arr[1].time_stamp = 0;
     dp_arr[1].type = PROP_VALUE;
     dp_arr[1].value.dp_value = ele_cal_data.v;
     
     dp_arr[2].dpid = driver_dltj.dp_icoe;
     dp_arr[2].time_stamp = 0;
     dp_arr[2].type = PROP_VALUE;
     dp_arr[2].value.dp_value = ele_cal_data.i;
     
     dp_arr[3].dpid = driver_dltj.dp_pcoe;
     dp_arr[3].time_stamp = 0;
     dp_arr[3].type = PROP_VALUE;
     dp_arr[3].value.dp_value = ele_cal_data.p;
     
     dp_arr[4].dpid = driver_dltj.dp_ecoe;
     dp_arr[4].time_stamp = 0;
     dp_arr[4].type = PROP_VALUE;
     dp_arr[4].value.dp_value = ele_cal_data.e;

     op_ret = dev_report_dp_json_async(get_gw_cntl()->gw_if.id,dp_arr,dp_cnt);
     Free(dp_arr);
     if(OPRT_OK != op_ret) {
         PR_ERR("dev_report_dp_json_async relay_config data error,err_num",op_ret);
         return op_ret;
     }

     PR_DEBUG("report pt_rst:%d, v_ref:%d,i_ref:%d, p_ref:%d, e_ref:%d",\
     ele_cal_data.v,ele_cal_data.i,ele_cal_data.p,ele_cal_data.e,ele_cal_data.prod_rslt);
     return OPRT_OK;
 }

STATIC OPERATE_RET get_ele_coe_flash(VOID)
{
    OPERATE_RET op_ret = OPRT_OK;
    op_ret = get_coefficient(&ele_cal_data);
    if((OPRT_OK != op_ret) ||(1 != ele_cal_data.prod_rslt)) {
        PR_NOTICE("not get ele cal coe:%d", op_ret);
        return OPRT_COM_ERROR;
    } else {
        PR_DEBUG("get ele cal coe success.");
        return OPRT_OK;
    }
}

