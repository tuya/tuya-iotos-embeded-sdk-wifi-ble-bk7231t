



#include "new_common.h"
#include "new_http.h"
#include "new_pins.h"

#if WINDOWS

#else
#include "../../beken378/func/include/net_param_pub.h"
#endif

int g_channelStates;
#ifdef WINDOWS

#else
BUTTON_S g_buttons[GPIO_MAX];

#endif
typedef struct pinsState_s {
	byte roles[32];
	byte channels[32];
} pinsState_t;

pinsState_t g_pins;

void PIN_SaveToFlash() {
#if WINDOWS
#else
	save_info_item(NEW_PINS_CONFIG,&g_pins, 0, 0);
#endif
}
void PIN_LoadFromFlash() {
	int i;
#if WINDOWS
#else
	get_info_item(NEW_PINS_CONFIG,&g_pins, 0, 0);
#endif
	for(i = 0; i < GPIO_MAX; i++) {
		PIN_SetPinRoleForPinIndex(i,g_pins.roles[i]);
	}
}
void PIN_ClearPins() {
	memset(&g_pins,0,sizeof(g_pins));
}
int PIN_GetPinRoleForPinIndex(int index) {
	return g_pins.roles[index];
}
int PIN_GetPinChannelForPinIndex(int index) {
	return g_pins.channels[index];
}
void RAW_SetPinValue(int index, int iVal){
#if WINDOWS

#else
    bk_gpio_output(index, iVal);
#endif
}
void button_generic_short_press(int index)
{
	CHANNEL_Toggle(g_pins.channels[index]);

	PR_NOTICE("%i key_short_press\r\n", index);
}
void button_generic_double_press(int index)
{
	CHANNEL_Toggle(g_pins.channels[index]);

	PR_NOTICE("%i key_double_press\r\n", index);
}
void button_generic_long_press_hold(int index)
{
	PR_NOTICE("%i key_long_press_hold\r\n", index);
}
unsigned char button_generic_get_gpio_value(void *param)
{
#if WINDOWS
	return 0;
#else
	int index = ((BUTTON_S*)param) - g_buttons;
	return bk_gpio_input(index);
#endif
}
#define PIN_UART1_RXD 10
#define PIN_UART1_TXD 11
#define PIN_UART2_RXD 1
#define PIN_UART2_TXD 0

void PIN_SetPinRoleForPinIndex(int index, int role) {
	//if(index == PIN_UART1_RXD)
	//	return;
	//if(index == PIN_UART1_TXD)
	//	return;
	//if(index == PIN_UART2_RXD)
	//	return;
	//if(index == PIN_UART2_TXD)
	//	return;
	g_pins.roles[index] = role;
	switch(role)
	{
	case IOR_Button:
	case IOR_Button_n:
#if WINDOWS
	
#else
		{
			BUTTON_S *bt = &g_buttons[index];
			button_init(bt, button_generic_get_gpio_value, 0);
			bk_gpio_config_input_pup(index);
		/*	button_attach(bt, SINGLE_CLICK,     button_generic_short_press);
			button_attach(bt, DOUBLE_CLICK,     button_generic_double_press);
			button_attach(bt, LONG_PRESS_HOLD,  button_generic_long_press_hold);
			button_start(bt);*/
		}
#endif
		break;
	case IOR_LED:
	case IOR_LED_n:
	case IOR_Relay:
	case IOR_Relay_n:
#if WINDOWS
	
#else
		bk_gpio_config_output(index);
		bk_gpio_output(index, 0);
#endif
		break;
	default:
		break;
	}
}
void PIN_SetPinChannelForPinIndex(int index, int ch) {
	g_pins.channels[index] = ch;
}
void (*g_channelChangeCallback)(int idx, int iVal) = 0;

void CHANNEL_SetChangeCallback(void (*cb)(int idx, int iVal)) {
	g_channelChangeCallback = cb;
}
void Channel_OnChanged(int ch) {
	int i;
	int bOn;


	bOn = BIT_CHECK(g_channelStates,ch);

	if(g_channelChangeCallback!=0) {
		g_channelChangeCallback(ch,bOn);
	}

	for(i = 0; i < GPIO_MAX; i++) {
		if(g_pins.channels[i] == ch) {
			if(g_pins.roles[i] == IOR_Relay || g_pins.roles[i] == IOR_LED) {
				RAW_SetPinValue(i,bOn);
			}
			if(g_pins.roles[i] == IOR_Relay_n || g_pins.roles[i] == IOR_LED_n) {
				RAW_SetPinValue(i,!bOn);
			}
		}
	}

}
void CHANNEL_Set(int ch, int iVal) {
	if(iVal) {
		BIT_SET(g_channelStates,ch);
	} else {
		BIT_CLEAR(g_channelStates,ch);
	}

	Channel_OnChanged(ch);
}
void CHANNEL_Toggle(int ch) {
	BIT_TGL(g_channelStates,ch);

	Channel_OnChanged(ch);
}
bool CHANNEL_Check(int ch) {
	return BIT_CHECK(g_channelStates,ch);
}

#if WINDOWS

#else

#define EVENT_CB(ev)   if(handle->cb[ev])handle->cb[ev]((BUTTON_S*)handle)

#define PIN_TMR_DURATION       5
beken_timer_t g_pin_timer;

void PIN_Input_Handler(int pinIndex)
{
	BUTTON_S *handle;
	uint8_t read_gpio_level;
	
	read_gpio_level = bk_gpio_input(pinIndex);
	handle = &g_buttons[pinIndex];

	//ticks counter working..
	if((handle->state) > 0)
		handle->ticks++;

	/*------------button debounce handle---------------*/
	if(read_gpio_level != handle->button_level) { //not equal to prev one
		//continue read 3 times same new level change
		if(++(handle->debounce_cnt) >= DEBOUNCE_TICKS) {
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
		}
	} else { //leved not change ,counter reset.
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state) {
	case 0:
		if(handle->button_level == handle->active_level) {	//start press down
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->ticks = 0;
			handle->repeat = 1;
			handle->state = 1;
		} else {
			handle->event = (uint8_t)NONE_PRESS;
		}
		break;

	case 1:
		if(handle->button_level != handle->active_level) { //released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->ticks = 0;
			handle->state = 2;

		} else if(handle->ticks > LONG_TICKS) {
			handle->event = (uint8_t)LONG_RRESS_START;
			EVENT_CB(LONG_RRESS_START);
			handle->state = 5;
		}
		break;

	case 2:
		if(handle->button_level == handle->active_level) { //press down again
			handle->event = (uint8_t)PRESS_DOWN;
			EVENT_CB(PRESS_DOWN);
			handle->repeat++;
			if(handle->repeat == 2) {
				EVENT_CB(DOUBLE_CLICK); // repeat hit
				button_generic_double_press(pinIndex);
			} 
			EVENT_CB(PRESS_REPEAT); // repeat hit
			handle->ticks = 0;
			handle->state = 3;
		} else if(handle->ticks > SHORT_TICKS) { //released timeout
			if(handle->repeat == 1) {
				handle->event = (uint8_t)SINGLE_CLICK;
				EVENT_CB(SINGLE_CLICK);
				button_generic_short_press(pinIndex);
			} else if(handle->repeat == 2) {
				handle->event = (uint8_t)DOUBLE_CLICK;
			}
			handle->state = 0;
		}
		break;

	case 3:
		if(handle->button_level != handle->active_level) { //released press up
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			if(handle->ticks < SHORT_TICKS) {
				handle->ticks = 0;
				handle->state = 2; //repeat press
			} else {
				handle->state = 0;
			}
		}
		break;

	case 5:
		if(handle->button_level == handle->active_level) {
			//continue hold trigger
			handle->event = (uint8_t)LONG_PRESS_HOLD;
			EVENT_CB(LONG_PRESS_HOLD);

		} else { //releasd
			handle->event = (uint8_t)PRESS_UP;
			EVENT_CB(PRESS_UP);
			handle->state = 0; //reset
		}
		break;
	}
}


/**
  * @brief  background ticks, timer repeat invoking interval 5ms.
  * @param  None.
  * @retval None
  */
void PIN_ticks(void *param)
{
	int i;
	for(i = 0; i < GPIO_MAX; i++) {
		if(g_pins.roles[i] == IOR_Button || g_pins.roles[i] == IOR_Button_n) {
			//PR_NOTICE("Test hold %i\r\n",i);
			PIN_Input_Handler(i);
		}
	}
}

void PIN_Init(void)
{
	OSStatus result;
	
    result = rtos_init_timer(&g_pin_timer,
                            PIN_TMR_DURATION,
                            PIN_ticks,
                            (void *)0);
    ASSERT(kNoErr == result);
	
    result = rtos_start_timer(&g_pin_timer);
    ASSERT(kNoErr == result);
}

#endif