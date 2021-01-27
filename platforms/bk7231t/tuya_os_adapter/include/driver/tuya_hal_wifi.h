/**
 * @file tuya_hal_wifi.h
 * @brief WIFI设备操作接口
 * 
 * @copyright Copyright(C),2018-2020, 涂鸦科技 www.tuya.com
 * 
 */

#ifndef _WIFI_HWL_H
#define _WIFI_HWL_H


#include <stdbool.h>
#include "tuya_hal_network.h"


#ifdef __cplusplus
extern "C" {
#endif


#define TAG_SSID_NUMBER 0
//#define TAG_SUPPORT_RATES_NUMBER 1
#define TAG_PAYLOAD_NUMBER 221

#define PROBE_REQUEST_TYPE_SUBTYPE 0x0040
#define PROBE_REQSPONSE_TYPE_SUBTYPE 0x0050
#define PROBE_REQUEST_DURATION_ID 0x0
#define PROBE_RESPONSET_DURATION_ID 0x0
#define PROBE_REQUEST_PAYLOAD_LEN_MAX 255
#define BROADCAST_MAC_ADDR 0xFFFFFFFF
#define PROBE_SSID "tuya_smart"

/* tuya sdk definition of wifi ap info */
#define WIFI_SSID_LEN 32    // tuya sdk definition WIFI SSID MAX LEN
#define WIFI_PASSWD_LEN 64  // tuya sdk definition WIFI PASSWD MAX LEN
typedef struct
{
    uint8_t channel;                 ///< AP channel
    int8_t  rssi;                    ///< AP rssi
    uint8_t bssid[6];                ///< AP bssid
    uint8_t ssid[WIFI_SSID_LEN+1];   ///< AP ssid array
    uint8_t s_len;                   ///< AP ssid len
    uint8_t resv1;
}AP_IF_S;

typedef enum {
    COUNTRY_CODE_CN,
    COUNTRY_CODE_US,
    COUNTRY_CODE_JP,
    COUNTRY_CODE_EU
} COUNTRY_CODE_E;


/**
 * @brief WIFI芯片探测本地AP信息结构体
 * @struct MIMO_IF_S
 */
typedef enum
{
    MIMO_TYPE_NORMAL = 0,
    MIMO_TYPE_HT40,
    MIMO_TYPE_2X2,
    MIMO_TYPE_LDPC,

    MIMO_TYPE_NUM,
}MIMO_TYPE_E;

typedef struct
{
    int8_t rssi;                /*!< MIMO包信号 */
    MIMO_TYPE_E type;           /*!< MIMO包类型 */
    uint16_t len;               /*!< MIMO包长度 */
    uint8_t channel;            /*!< MIMO包信道 */
    uint8_t mcs;
} MIMO_IF_S;

/*!
\brief 802.11 frame info
\enum WLAN_FRM_TP_E
*/
typedef enum {
    WFT_PROBE_REQ   = 0x40,     ///< Probe request
    WFT_PROBE_RSP   = 0x50,     ///< Probe response
    WFT_AUTH    = 0xB0,       //auth
    WFT_BEACON      = 0x80,     ///< Beacon
    WFT_DATA        = 0x08,     ///< Data
    WFT_QOS_DATA    = 0x88,     ///< QOS Data
    WFT_MIMO_DATA   = 0xff,     ///< MIMO Data
}WLAN_FRM_TP_E;

#pragma pack(1)
typedef struct {
    //802.11 management
    uint8_t id;
    uint8_t len;
    
    char data[0];   ///< data
} WLAN_MANAGEMENT_S;

typedef struct {
    uint8_t  frame_type;            ///< WLAN Frame type
    uint8_t  frame_ctrl_flags;      ///< Frame Control flags
    uint16_t duration;              ///< Duration
    uint8_t  dest[6];               ///< Destination MAC Address
    uint8_t  src[6];                ///< Source MAC Address
    uint8_t  bssid[6];              ///< BSSID MAC Address
    uint16_t seq_frag_num;          ///< Sequence and Fragmentation number
} WLAN_PROBE_REQ_IF_S;

typedef struct {
    uint8_t     frame_ctrl_flags;   ///< Frame Control flags
    uint16_t    duration;           ///< Duration
    uint8_t     dest[6];            ///< Destination MAC Address
    uint8_t     src[6];             ///< Source MAC Address
    uint8_t     bssid[6];           ///< BSSID MAC Address
    uint16_t    seq_frag_num;       ///< Sequence and Fragmentation number
    uint8_t     timestamp[8];       ///< Time stamp
    uint16_t    beacon_interval;    ///< Beacon Interval
    uint16_t    cap_info;           ///< Capability Information
    uint8_t     ssid_element_id;    ///< SSID Element ID
    uint8_t     ssid_len;           ///< SSID Length
    char        ssid[0];            ///< SSID
}WLAN_BEACON_IF_S;

#define TO_FROM_DS_MASK 0x03
#define TFD_IBSS 0x00           ///< da+sa+bssid
#define TFD_TO_AP 0x01          ///< bssid+sa+da
#define TFD_FROM_AP 0x02        ///< ds+bssid+sa
#define TFD_WDS 0x03            ///< ra+ta+da

typedef uint8_t BC_DA_CHAN_T;
#define BC_TO_AP 0
#define BC_FROM_AP 1
#define BC_CHAN_NUM 2

typedef struct {
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
}WLAN_COM_ADDR_S;

typedef struct {
    uint8_t bssid[6];
    uint8_t src[6];
    uint8_t dst[6];
}WLAN_TO_AP_ADDR_S;

typedef struct {
    uint8_t dst[6];
    uint8_t bssid[6];
    uint8_t src[6];
}WLAN_FROM_AP_ADDR_S;

typedef union {
    WLAN_COM_ADDR_S com;
    WLAN_TO_AP_ADDR_S to_ap;
    WLAN_FROM_AP_ADDR_S from_ap;
}WLAN_ADDR_U;

typedef struct {
    uint8_t frame_ctrl_flags;   ///< Frame Control flags
    uint16_t duration;          ///< Duration
    WLAN_ADDR_U addr;           ///< address
    uint16_t seq_frag_num;      ///< Sequence and Fragmentation number
    uint16_t qos_ctrl;          ///< QoS Control bits
}WLAN_DATA_IF_S;

/*!
\brief WLAN Frame info
\struct WLAN_FRAME_S
*/
typedef struct {
    uint8_t frame_type;                 ///< WLAN Frame type
    union {
        WLAN_BEACON_IF_S beacon_info;   ///< WLAN Beacon info
        WLAN_DATA_IF_S   data_info;     ///< WLAN Data info
        MIMO_IF_S        mimo_info;     ///< mimo info
    } frame_data;
}WLAN_FRAME_S,*P_WLAN_FRAME_S;

typedef struct
{
    uint16_t type_and_subtype;
    uint16_t duration_id;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t seq_ctrl;
} PROBE_REQUEST_PACKAGE_HEAD_S;

typedef struct
{
    uint8_t index;
    uint8_t len;
    uint8_t ptr[0];
} BEACON_TAG_DATA_UNIT_S;
#pragma pack()

typedef struct
{
    PROBE_REQUEST_PACKAGE_HEAD_S pack_head;
    BEACON_TAG_DATA_UNIT_S tag_ssid;
} PROBE_REQUEST_FIX_S;

/**
 * @brief callback function: SNIFFER_CALLBACK
 *        when wifi sniffers package from air, notify tuya-sdk
 *        with this callback. the package should include
 * @param[in]       buf         the buf wifi recv
 * @param[in]       len         the len of buf
 */
typedef void (*SNIFFER_CALLBACK)(const uint8_t *buf, const uint16_t len);

/* tuya sdk definition of wifi function type */
typedef enum
{
    WF_STATION = 0,     ///< station type
    WF_AP,              ///< ap type
}WF_IF_E;

/* tuya sdk definition of wifi work mode */
typedef enum
{
    WWM_LOWPOWER = 0,   ///< wifi work in lowpower mode
    WWM_SNIFFER,        ///< wifi work in sniffer mode
    WWM_STATION,        ///< wifi work in station mode
    WWM_SOFTAP,         ///< wifi work in ap mode
    WWM_STATIONAP,      ///< wifi work in station+ap mode
}WF_WK_MD_E;

/* tuya sdk definition of wifi encryption type */
typedef enum
{
    WAAM_OPEN = 0,      ///< open
    WAAM_WEP,           ///< WEP
    WAAM_WPA_PSK,       ///< WPA—PSK
    WAAM_WPA2_PSK,      ///< WPA2—PSK
    WAAM_WPA_WPA2_PSK,  ///< WPA/WPA2
}WF_AP_AUTH_MODE_E;

/* tuya sdk definition of ap config info */
typedef struct {
    uint8_t ssid[WIFI_SSID_LEN+1];       ///< ssid
    uint8_t s_len;                       ///< len of ssid
    uint8_t passwd[WIFI_PASSWD_LEN+1];   ///< passwd
    uint8_t p_len;                       ///< len of passwd
    uint8_t chan;                        ///< channel. default:0
    WF_AP_AUTH_MODE_E md;                ///< encryption type
    uint8_t ssid_hidden;                 ///< ssid hidden  default:0
    uint8_t max_conn;                    ///< max sta connect nums default:0
    uint16_t ms_interval;                ///< broadcast interval default:0
}WF_AP_CFG_IF_S;

/* tuya sdk definition of wifi station work status */
typedef enum {
    WSS_IDLE = 0,                       ///< not connected
    WSS_CONNECTING,                     ///< connecting wifi
    WSS_PASSWD_WRONG,                   ///< passwd not match
    WSS_NO_AP_FOUND,                    ///< ap is not found
    WSS_CONN_FAIL,                      ///< connect fail
    WSS_CONN_SUCCESS,                   ///< connect wifi success
    WSS_GOT_IP,                         ///< get ip success
}WF_STATION_STAT_E;

/* for fast connect*/
typedef struct {
    uint8_t ssid[WIFI_SSID_LEN+4];            ///< ssid
    uint8_t passwd[WIFI_PASSWD_LEN+1];        ///< passwd
    uint32_t security;                        ///< security type
    uint8_t chan;                             ///< channel
}FAST_WF_CONNECTED_AP_INFO_S;

/**
 * @brief scan current environment and obtain all the ap
 *        infos in current environment
 * 
 * @param[out]      ap_ary      current ap info array
 * @param[out]      num         the num of ar_ary
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_all_ap_scan(AP_IF_S **ap_ary, uint32_t *num);

/**
 * @brief scan current environment and obtain the specific
 *        ap info.
 * 
 * @param[in]       ssid        the specific ssid
 * @param[out]      ap          the ap info
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_assign_ap_scan(const char *ssid, AP_IF_S **ap);

/**
 * @brief release the memory malloced in <tuya_hal_wifi_all_ap_scan>
 *        and <tuya_hal_wifi_assign_ap_scan> if needed. tuya-sdk
 *        will call this function when the ap info is no use.
 * 
 * @param[in]       ap          the ap info
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_release_ap(AP_IF_S *ap);

/**
 * @brief close concurrent ap
 * 
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_close_concurrent_ap(void);

/**
 * @brief set wifi interface work channel
 * 
 * @param[in]       chan        the channel to set
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_set_cur_channel(const uint8_t chan);

/**
 * @brief get wifi interface work channel
 * 
 * @param[out]      chan        the channel wifi works
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_get_cur_channel(uint8_t *chan);

/**
 * @brief enable / disable wifi sniffer mode.
 *        if wifi sniffer mode is enabled, wifi recv from
 *        packages from the air, and user shoud send these
 *        packages to tuya-sdk with callback <cb>.
 * 
 * @param[in]       en          enable or disable
 * @param[in]       cb          notify callback
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_sniffer_set(const bool en, const SNIFFER_CALLBACK cb);

/**
 * @brief get wifi ip info.when wifi works in
 *        ap+station mode, wifi has two ips.
 * 
 * @param[in]       wf          wifi function type
 * @param[out]      ip          the ip addr info
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_get_ip(const WF_IF_E wf, NW_IP_S *ip);

/**
 * @brief set wifi ip info.when wifi works in
 *        ap+station mode, wifi has two ips.
 * 
 * @param[in]       wf          wifi function type
 * @param[in]       ip          the ip addr info
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_set_ip(const WF_IF_E wf, const NW_IP_S *ip);

/**
 * @brief get wifi bssid
 * 
 * @param[out]      mac         uplink mac
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_get_bssid(uint8_t mac[6]);

/**
 * @brief get wifi mac info.when wifi works in
 *        ap+station mode, wifi has two macs.
 * 
 * @param[in]       wf          wifi function type
 * @param[out]      mac         the mac info
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_get_mac(const WF_IF_E wf, NW_MAC_S *mac);

/**
 * @brief set wifi mac info.when wifi works in
 *        ap+station mode, wifi has two macs.
 * 
 * @param[in]       wf          wifi function type
 * @param[in]       mac         the mac info
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_set_mac(const WF_IF_E wf, const NW_MAC_S *mac);

/**
 * @brief set wifi work mode
 * 
 * @param[in]       mode        wifi work mode
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_set_work_mode(const WF_WK_MD_E mode);

/**
 * @brief get wifi work mode
 * 
 * @param[out]      mode        wifi work mode
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_get_work_mode(WF_WK_MD_E *mode);

/**
 * @brief connect wifi with ssid and passwd
 * 
 * @param[in]       ssid
 * @param[in]       passwd
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_station_connect(const char *ssid, const char *passwd);

/**
 * @brief disconnect wifi from connect ap
 * 
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_station_disconnect(void);

/**
 * @brief get wifi connect rssi
 * 
 * @param[out]      rssi        the return rssi
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_station_get_conn_ap_rssi(int8_t *rssi);

/**
 * @brief get wifi connect ap mac
 * 
 * @param[in,out]   mac    the return ap mac
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_station_get_ap_mac(NW_MAC_S *mac);

/**
 * @brief get wifi station work status
 * 
 * @param[out]      stat        the wifi station work status
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_station_get_status(WF_STATION_STAT_E *stat);

/**
 * @brief start a soft ap
 * 
 * @param[in]       cfg         the soft ap config
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_ap_start(const WF_AP_CFG_IF_S *cfg);

/**
 * @brief stop a soft ap
 * 
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_ap_stop(void);

/**
 * @brief set wifi country code
 * 
 * @param[in]       p_country_code  country code
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_set_country_code(const char *p_country_code);

/**
 * @brief get wifi country code
 * 
 * @return  COUNTRY_CODE_E: country code
 */
COUNTRY_CODE_E tuya_hal_wifi_get_cur_country_code(void);

/**
 * @brief get wifi rf cal flag
 * 
 * @return  bool: true or false
 */
bool tuya_hal_wifi_get_rf_cal_flag(void);

/**
 * @brief enable low power
 * 
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_lowpower_enable(void);

/**
 * @brief disable low power
 * 
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_lowpower_disable(void);

/**
 * @brief receive wifi management callback
 * 
 */
typedef void (*WIFI_REV_MGNT_CB)(uint8_t *buf, int buf_len);

/**
 * @brief send wifi management
 * 
 * @param[in]       buf         pointer to buffer
 * @param[in]       len         length of buffer
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_send_mgnt(const uint8_t *buf, const uint32_t len);

/**
 * @brief register receive wifi management callback
 * 
 * @param[in]       enable
 * @param[in]       recv_cb     receive callback
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_register_recv_mgnt_callback(bool enable, WIFI_REV_MGNT_CB recv_cb);

/**
 * @brief set socket broadcast all
 * 
 * @param[in]       socket_fd
 * @param[in]       enable
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_set_socket_broadcast_all(const int socket_fd, const bool enable);

/**
 * @brief tuya_hal_wifi_wps_pbc_start
 *
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_wps_pbc_start(void);

/**
 * @brief tuya_hal_wifi_wps_ap_info_get
 *
 * @param[out]      ssid
 * @param[out]      pwd
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_wps_ap_info_get(uint8_t* ssid, uint8_t* pwd);

/**
 * @brief : tuya_hal_wifi_wps_pbc_stop
 *
 * @return  OPRT_OK: success  Other: fail
 */
int tuya_hal_wifi_wps_pbc_stop(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



