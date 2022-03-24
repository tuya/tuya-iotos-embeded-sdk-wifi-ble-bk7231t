/**
 * @file tuya_mcast_master.h
 * @brief TUYA multicast master include
 * @version 0.1
 * @date 2022-03-07
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_MULTICAST_MASTER_H__
#define __TUYA_MULTICAST_MASTER_H__

#include "tuya_cloud_types.h"
#include "tuya_mcast_public.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
typedef struct {
    BOOL_T checked;
    BYTE_T slave_devid[GW_ID_LEN + 1];
    BYTE_T negotiate_key[MULTICAST_KEY_LEN];
} create_group_info_t;

typedef struct {
    BYTE_T slave_devid[GW_ID_LEN + 1];
} scan_data_t;

typedef struct {
    BOOL_T result;
    BYTE_T slave_devid[GW_ID_LEN + 1];
} group_negotiate_result_t;

typedef struct {
    BYTE_T slave_devid[GW_ID_LEN + 1];
    UINT_T len;
    CHAR_T data[0];
} ctrl_cmd_info_t;
#pragma pack()

/**
 * @brief multicast scan cb
 *
 * @param[in] num data num
 * @param[in] data scan data
 *
 */
typedef OPERATE_RET(*mcast_master_scan_cb)(UINT_T num, scan_data_t *data);

/**
 * @brief multicast create group cb
 *
 * @param[in] num negotiate result num
 * @param[in] data negotiate result data
 *
 */
typedef OPERATE_RET(*mcast_master_create_group_cb)(UINT_T num, group_negotiate_result_t *data);

/**
 * @brief multicast scan
 *
 * @param[in] timeout time out,unit was ms
 * @param[in] maxDevNum max slave num
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_do_scan(UINT_T timeout, UINT_T maxDevNum);

/**
 * @brief multicast create group
 *
 * @param[in] group_cnt group cnt
 * @param[in] checked_cnt checked cnt
 * @param[in] group_data group data
 * @param[in] timeout time out,unit was ms
 * @param[in] token token
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_create_group(UINT_T group_cnt, UINT_T checked_cnt, create_group_info_t *group_data, UINT_T timeout, UCHAR_T *token);

/**
 * @brief multicast master init
 *
 * @param[in] scan_cb scan report cb
 * @param[in] group_cb create group cb
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_master_init(mcast_master_scan_cb scan_cb, mcast_master_create_group_cb group_cb);

/**
 * @brief multicast master deinit
 *
 */
VOID tuya_mcast_master_deinit();

/**
 * @brief dump master ctrl info
 *
 */
VOID tuya_mcast_dump_master_info();

/**
 * @brief master send ctrl command
 *
 * @param[in] cnt ctrl num
 * @param[in] cmd_data ctrl command data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_ctrl_cmd_send(UINT_T cnt, ctrl_cmd_info_t *cmd_data);

/**
 * @brief get create group num
 *
 * @return group num
 */
UINT_T tuya_mcast_master_get_group_cnt();

/**
 * @brief get negotiate result num
 *
 * @return result num
 */
UINT_T tuya_mcast_master_get_result_cnt();

/**
 * @brief reg scan cb
 *
 * @param[in] cb scan callback
 *
 */
VOID tuya_master_reg_scan_cb(mcast_master_scan_cb cb);

/**
 * @brief reg create group cb
 *
 * @param[in] cb create group callback
 *
 */
VOID tuya_master_reg_group_cb(mcast_master_create_group_cb cb);

#ifdef __cplusplus
}
#endif
#endif //__TUYA_MULTICAST_MASTER_H__

