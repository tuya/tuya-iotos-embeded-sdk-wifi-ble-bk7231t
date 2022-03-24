/**
 * @file tuya_mulitcast_public.h
 * @brief TUYA multicast public
 * @version 0.1
 * @date 2022-02-28
 *
 * @copyright Copyright 2019-2021 Tuya Inc. All Rights Reserved.
 *
 */

#ifndef __TUYA_MULTICAST_PUBLIC_H__
#define __TUYA_MULTICAST_PUBLIC_H__

#include "tuya_cloud_types.h"
#include "tuya_cloud_com_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef UINT8_T MULTICAST_FRAME_TYPE;
#define MULTICAST_FRAME_SCAN_REQUEST 0x1
#define MULTICAST_FRAME_SCAN_RESPONSE 0x2
#define MULTICAST_FRAME_BIND_REQUEST 0x3
#define MULTICAST_FRAME_BIND_RESPONSE 0x4
#define MULTICAST_FRAME_SESSION_KEY_SYN 0x5
#define MULTICAST_FRAME_SESSION_KEY_SYN_ACK 0x06
#define MULTICAST_FRAME_SESSION_KEY_ACK 0x7
#define MULTICAST_FRAME_CMD 0x10

#define LAN_MCAST_ADDR  0xE300000B
#define LAN_MCAST_PORT  40001

#define MULTICAST_KEY_LEN   16
#define MULTICAST_TOKEN_LEN 16
#define MULTICAST_HMAC_LEN  32
#define MULTICAST_RAND_LEN  16

#pragma pack(1)
typedef struct {
    CHAR_T token[MULTICAST_TOKEN_LEN];//随机数,由主控设备产生
} mcast_scan_request_t;
typedef struct {
    CHAR_T token[MULTICAST_TOKEN_LEN];
    CHAR_T devid[GW_ID_LEN + 1];
} mcast_scan_response_t;

typedef struct {
    UCHAR_T token[MULTICAST_TOKEN_LEN];
    UCHAR_T hmac[MULTICAST_HMAC_LEN];
} mcast_bind_request_t, mcast_bind_response_t;

typedef struct {
    CHAR_T randA[MULTICAST_RAND_LEN];
} mcast_syn_t;

typedef struct {
    CHAR_T randB[MULTICAST_RAND_LEN];
    CHAR_T randA_hmac[MULTICAST_HMAC_LEN];
} mcast_syn_ack_t;

typedef struct {
    CHAR_T randB_hmac[MULTICAST_HMAC_LEN];
} mcast_ack_t;

typedef struct {
    uint8_t version: 4;
    uint8_t reserved: 4;
    uint8_t reserved2;
    uint32_t sequence;
    uint32_t type;
    uint32_t id;//主控或者受控设备自己的id
    uint32_t total_len;//报文总的长度,从head到tail
    uint32_t frame_num;//frame的个数
} mcast_fixed_head_t;
typedef mcast_fixed_head_t mcast_additional_data_t;

typedef struct {
    UINT_T key_len;
    BYTE_T* key;
    UINT_T mate_id;//对端的devid的crc32
    UINT_T data_len;//单个frame的长度
    BYTE_T* data;
} mcast_frame_object_t;

#define KV_MCAST_MASTER_GROUP_INFO  "multicast_master_group"
#define KV_MCAST_SLAVE_GROUP_INFO  "multicast_slave_group"
typedef struct {
    UINT_T cnt;
    CHAR_T data[0];
} kv_mcast_group_t;

#define MCAST_FRAME_HEAD_SIZE        4
#define MCAST_FRAME_VERSION_SIZE     1
#define MCAST_FRAME_RESERVE_SIZE     1
#define MCAST_FRAME_SEQUENCE_SIZE    4
#define MCAST_FRAME_TYPE_SIZE        4
#define MCAST_FRAME_ID_SIZE          4
#define MCAST_FRAME_TOTAL_LEN_SIZE   4
#define MCAST_FRAME_FRAME_NUM_SIZE   4
#define MCAST_FRAME_NONCE_SIZE       12
#define MCAST_FRAME_DATALEN_SIZE     4
#define MCAST_FRAME_TAG_SIZE         16
#define MCAST_FRAME_TAIL_SIZE        4

#pragma pack()

/**
 * @brief multicast encry and send
 *
 * @param[in] fd socket
 * @param[in] sequence sequence
 * @param[in] type frame type
 * @param[in] frame_num frame num
 * @param[in] frame frame data
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_encrypt_and_send(UINT_T fd, UINT_T sequence, UINT_T type, UINT_T frame_num, mcast_frame_object_t *frame);

/**
 * @brief check incoming packet is vaild
 *
 * @param[in] frame_buffer frame buf
 * @param[in] recv_datalen len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
BOOL_T tuya_mcast_is_in_packet_vaild(BYTE_T* frame_buffer, UINT_T recv_datalen);

/**
 * @brief frame parse
 *
 * @param[in] key key
 * @param[in] key_len key len
 * @param[in] input data
 * @param[out] out_data decrypt data
 * @param[out] olen decrypt data len
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_frame_parse(BYTE_T* key, UINT_T key_len, BYTE_T* input, BYTE_T** out_data, UINT_T *olen);

/**
 * @brief one frame send
 *
 * @param[in] fd socket
 * @param[in] key key
 * @param[in] src data
 * @param[in] ilen src len
 * @param[in] sequence sequence
 * @param[in] type frame type
 * @param[in] mate_id mate id
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_single_frame_send(UINT_T fd, BYTE_T *key, BYTE_T *src, UINT_T ilen, UINT_T sequence, UINT_T type, UINT_T mate_id);

/**
 * @brief create negotiate key
 *
 * @param[in] data data
 * @param[in] src_key src key
 * @param[in] dst_key negotiate key
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_create_negotiate_key(UCHAR_T *data, UCHAR_T *src_key, UCHAR_T *dst_key);

/**
 * @brief create seesion key
 *
 * @param[in] randA random A
 * @param[in] randB random B
 * @param[in] negotiate_key negotiate key
 * @param[out] session_key session key
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_create_mcast_seesion_key(BYTE_T *randA, BYTE_T *randB, BYTE_T *negotiate_key, BYTE_T *session_key);

/**
 * @brief create multicast key
 *
 * @param[out] key_out out key
 *
 */
VOID tuya_make_mcast_key_encode(UCHAR_T *key_out);


/**
 * @brief report mate info to cloud
 *
 * @param[out] tmm_meta_sync msg
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tuya_mcast_report_mate(TM_MSG_S *tmm_meta_sync);

/**
 * @brief get create group token
 *
 * @param[out] dst token
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
VOID tuya_mcast_get_group_token(CHAR_T *dst);

#ifdef __cplusplus
}
#endif
#endif //__TUYA_MULTICAST_PUBLIC_H__

