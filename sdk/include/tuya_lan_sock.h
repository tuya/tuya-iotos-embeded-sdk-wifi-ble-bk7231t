/**
 * @file tuya_lan_sock.h
 * @brief TUYA lan sock
 * @version 0.1
 * @date 2022-02-14
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_LAN_SOCK_H__
#define __TUYA_LAN_SOCK_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief sock handler callback
 *
 * @param[in] sock fd
 * @param[in] sock_ctx user data
 *
 */
typedef void (*sloop_sock_handler)(BYTE_T *data, UINT_T data_len, void *sock_ctx);

/**
 * @brief sock err handler
 *
 * @param[in] sock fd
 * @param[in] sock_ctx user data
 *
 */
typedef void (*sloop_sock_err)(INT_T sock, void *sock_ctx);

/**
 * @brief register sock
 *
 * @param[in] sock fd
 * @param[in] handler sock handler
 * @param[in] err sock err handler
 * @param[in] user_data handler user data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_register_lan_sock(INT_T sock, sloop_sock_handler handler, sloop_sock_err err, void *user_data);

/**
 * @brief unregister sock
 *
 * @param[in] sock fd
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_unregister_lan_sock(sloop_sock_handler handler);

/**
 * @brief set sock loop disable or enable
 *
 * @param[in] terminate terminate vaule
 *
 */
void tuya_set_sock_loop_terminate(INT_T terminate);

/**
 * @brief get sock loop terminate vaule
 *
 * @return terminate value
 *
 */
INT_T tuya_get_sock_loop_terminate();

/**
 * @brief create and set up multicast socket
 *
 * @return socket fd
 *
 */
INT_T tuya_mcast_setup_socket();

/**
 * @brief dump lan sock info
 *
 */
void tuya_dump_lan_sock_reader();

/**
 * @brief set sock opt
 *
 * @param[in] sock fd
 * @param[in] optname optname
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
INT_T tuya_mcast_setoptname(INT_T fd, INT_T optname);

#ifdef __cplusplus
}
#endif
#endif //__TUYA_LAN_SOCK_H__

