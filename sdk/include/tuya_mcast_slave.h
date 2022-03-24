/**
 * @file tuya_mcast_slave.h
 * @brief TUYA multicast slave include
 * @version 0.1
 * @date 2022-03-07
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_MULTICAST_SLAVE_H__
#define __TUYA_MULTICAST_SLAVE_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief control command callback
 *
 * @param[in] data handle data
 * @param[in] date_len handle data length
 *
 */
typedef OPERATE_RET(*mcast_slave_cmd_handle)(BYTE_T *data, UINT_T date_len);

/**
 * @brief reg multicast control command callback
 *
 * @param[in] cb callback
 *
 */
void tuya_slave_reg_ctrl_cb(mcast_slave_cmd_handle cb);

/**
 * @brief multicast slave init
 *
 * @param[in] cb ctrl command callback
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_slave_init(mcast_slave_cmd_handle cb);

/**
 * @brief multicast slave deinit
 *
 */
VOID tuya_mcast_slave_deinit();

/**
 * @brief dump slave ctrl info
 *
 */
VOID tuya_mcast_dump_slave_info();

#ifdef __cplusplus
}
#endif
#endif //__TUYA_MULTICAST_SLAVE_H__

