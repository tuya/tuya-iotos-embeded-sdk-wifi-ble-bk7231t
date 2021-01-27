/***********************************************************
*  File: tuya_gpio_test.h
*  Author: lql
*  Date: 20180502
***********************************************************/

typedef BYTE_T BOARD_TYPE; 
#define BK_BOARD_WB3S  0
#define BK_BOARD_WB3L  1
#define BK_BOARD_WB2S  2
#define BK_BOARD_WB1S  3
#define BK_BOARD_WB2L  4
#define BK_BOARD_WBLC5 5
#define BK_BOARD_WB8P  6


#define BK_BOARD_MAX  7

BOOL_T gpio_test_cb(BOARD_TYPE type);

BOOL_T gpio_test_all(IN CONST CHAR_T *in, OUT CHAR_T *out);