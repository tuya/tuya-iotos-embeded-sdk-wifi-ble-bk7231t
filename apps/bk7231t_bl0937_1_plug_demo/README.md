# Power statistics single plug（ PID：aumib22ytuftcydk） 

## 1.Hardware IO port description
	(1)relay:PWM5,Active high;
	
	(2) Switch button: PWM3, active low level;

	(3) wifi indicator: PWM0, active low;

	(4) Measurement information: working voltage 220V, overcurrent point 17A, sampling resistance 1 milliohm; measurement chip: bl0937;

	(5) Electricity statistics (CF): PWM1; current and voltage detection (CF1): PWM2; current and voltage switching (SEL): PWM4, the level is direct connection mode;

## 2. Network status corresponding indicator
	(1) No network distribution: Indicates the status of the relay;

	(2) In the distribution network: fast flashing (250ms flashing) and slow flashing (1500ms flashing);

	(3) Distribution network failure: indicating relay status;

	(4) The distribution network is successful: the relay status is indicated;

## 3. Other functions

	(1) The wifi mode is WCM_OLD_LOW, power-on low power mode, long press the button for 5 seconds to enter the network configuration mode;

	(2) The power-on default is the power-off state;

## 4. File Description

	(1) device.h/device.c: Mainly the header files and functions of the module framework part, mainly including device initialization, callbacks for production testing, callbacks for app operation, etc.;

	(2) app_switch.h/app_switch.c: This part is mainly the header file and function logic of the switch part, mainly including the display of the switch state corresponding to the hardware key operation, the display of the switch state corresponding to the app operation, the display of the indicator light, and the countdown function Wait;

	(3) app_dltj.c/app_dltj.h: This part is the header file and function code of the application layer of power statistics, the main power real-time parameter (current, voltage and power) reporting logic and corresponding event processing, power increment reporting logic/offline Electricity storage/electricity storage in the state of undistributed network, etc.;

	(4) bl0937.h/bl0937.c: This part is the driving part of the power statistics chip bl0937, mainly the hardware initialization of the chip / the calculation of current, voltage, power and power / the processing of the production measurement part, etc.; 
