#ifndef TUYA_BLE_TYPE_H__
#define TUYA_BLE_TYPE_H__

#include <stdbool.h>
#include <stdint.h>
#include "tuya_ble_config.h"


typedef struct {
    uint8_t *data;
    uint32_t len;
}tuya_ble_data_buf_t;


#endif

