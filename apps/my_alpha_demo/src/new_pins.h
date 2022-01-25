#ifndef __NEW_PINS_H__
#define __NEW_PINS_H__

#define BIT_SET(PIN,N) (PIN |=  (1<<N))
#define BIT_CLEAR(PIN,N) (PIN &= ~(1<<N))
#define BIT_TGL(PIN,N) (PIN ^=  (1<<N))
#define BIT_CHECK(PIN,N) !!((PIN & (1<<N)))

enum IORole {
	IOR_None,
	IOR_Relay,
	IOR_Relay_n,
	IOR_Button,
	IOR_Button_n,
	IOR_LED,
	IOR_LED_n,
	IOR_Total_Options,
};

#define GPIO_MAX 27
#define CHANNEL_MAX 32

void PIN_ClearPins();
int PIN_GetPinRoleForPinIndex(int index);
int PIN_GetPinChannelForPinIndex(int index);
void PIN_SetPinRoleForPinIndex(int index, int role);
void PIN_SetPinChannelForPinIndex(int index, int ch);
void CHANNEL_Toggle(int ch);
bool CHANNEL_Check(int ch);
void CHANNEL_SetChangeCallback(void (*cb)(int idx, int iVal));
void CHANNEL_Set(int ch, int iVal);

#endif

