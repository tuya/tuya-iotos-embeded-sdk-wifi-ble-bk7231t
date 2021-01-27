/**
 * \file tuya_ble_port.h
 *
 * \brief 
 */
/*
 *  Copyright (C) 2014-2019, Tuya Inc., All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of tuya ble sdk 
 */


#ifndef TUYA_BLE_PORT_H__
#define TUYA_BLE_PORT_H__


#include <stdbool.h>
#include <stdint.h>
#include "tuya_cloud_types.h"
#include "tuya_ble_type.h"
#include "tuya_ble_config.h"


typedef enum {
    TY_BT_EVENT_DISCONNECTED,   /* APP断开连接 */
    TY_BT_EVENT_CONNECTED,      /* APP连接上设备 */
    TY_BT_EVENT_RX_DATA,        /* 接收到APP数据*/
    TY_BT_EVENT_ADV_READY,      /* start adv. */
}ty_bt_cb_event_t;

typedef enum {
    TY_BT_MODE_PERIPHERAL,
    TY_BT_MODE_CENTRAL,
}ty_bt_mode_t;


typedef VOID (*TY_BT_MSG_CB)(INT_T id, ty_bt_cb_event_t e, tuya_ble_data_buf_t *buf);

typedef struct {
    CHAR_T name[DEVICE_NAME_LEN];
    ty_bt_mode_t mode;
    UCHAR_T link_num;
    TY_BT_MSG_CB cb;
    tuya_ble_data_buf_t *adv;
    tuya_ble_data_buf_t *scan_rsp;
}ty_bt_param_t;

typedef enum {
    TY_BT_SCAN_BY_NAME = 0x01,
    TY_BT_SCAN_BY_MAC = 0x02,
}ty_bt_scan_type_t;

typedef struct {
    CHAR_T scan_type; /* ref ty_bt_scan_type_t. */
    CHAR_T name[DEVICE_NAME_LEN];
    CHAR_T bd_addr[6];
    SCHAR_T rssi;
    UCHAR_T channel;
    UCHAR_T timeout_s; /* second. */
}ty_bt_scan_info_t;


OPERATE_RET tuya_bt_port_init(ty_bt_param_t *p);

OPERATE_RET tuya_bt_port_deinit(void);

OPERATE_RET tuya_bt_gap_disconnect(void);

OPERATE_RET tuya_bt_send(BYTE_T *data, UINT8_T len);

OPERATE_RET tuya_bt_reset_adv(tuya_ble_data_buf_t *adv, tuya_ble_data_buf_t *scan_resp);

OPERATE_RET tuya_bt_get_rssi(SCHAR_T *rssi);

OPERATE_RET tuya_bt_start_adv(void);

OPERATE_RET tuya_bt_stop_adv(void);

OPERATE_RET tuya_bt_assign_scan(INOUT ty_bt_scan_info_t *info);

#endif


