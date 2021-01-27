/***********************************************************
*  File: wf_sniffer_intf.h 
*  Author: nzy
*  Date: 20170220
***********************************************************/
#ifndef _WF_SNIFFER_INTF_H
#define _WF_SNIFFER_INTF_H
#ifdef __cplusplus
	extern "C" {
#endif

#include "tuya_os_adapter.h"
#include "tuya_cloud_types.h"
#include "wf_basic_intf.h"

#ifdef  __WF_SNIFFER_INTF_GLOBALS
    #define __WF_SNIFFER_INTF_EXT
#else
    #define __WF_SNIFFER_INTF_EXT extern
#endif

/***********************************************************
*************************micro define***********************
***********************************************************/
typedef VOID (*BC_SNIFFER_CB)(IN CONST BYTE_T *dest,IN CONST BYTE_T *src,\
                                    IN CONST BYTE_T *bssid,IN CONST USHORT_T len, \
                                    IN CONST BC_DA_CHAN_T from, IN CONST UINT16_T seq_num);

typedef VOID (*MC_SNIFFER_CB)(IN CONST BYTE_T *dest,IN CONST BYTE_T *src,\
                                    IN CONST BYTE_T *bssid,IN CONST USHORT_T len);

typedef VOID (*BEACON_INFO_CB)(IN CONST CHAR_T *ssid, IN CONST BYTE_T ssid_len, \
                                      IN CONST BYTE_T *bssid, IN CONST BYTE_T channel);

typedef VOID (*MIMO_SNIFFER_CB)(IN CONST MIMO_IF_S *user);

typedef VOID (*FF_BOARDCAST_AUTH_CB)(IN CONST BYTE_T *dst);

/***********************************************************
*************************variable define********************
***********************************************************/

/***********************************************************
*************************function define********************
***********************************************************/
/***********************************************************
*  Function: wf_nw_cfg_sniffer_enable
*  Input: b_cb->boardcast packet receive callback 
*         m_cb->multicast packet receive callback
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
__WF_SNIFFER_INTF_EXT \
OPERATE_RET wf_nw_cfg_sniffer_enable(IN CONST BC_SNIFFER_CB b_cb,\
                                     IN CONST MC_SNIFFER_CB m_cb,\
                                     IN CONST MIMO_SNIFFER_CB mimo_cfg_cb, \
                                     IN CONST BEACON_INFO_CB info_cb, \
                                     IN CONST FF_BOARDCAST_AUTH_CB ff_boardcast_auth_cb);

/***********************************************************
*  Function: wf_nw_cfg_sniffer_disable
*  Input: none
*  Output: none 
*  Return: OPERATE_RET
***********************************************************/
__WF_SNIFFER_INTF_EXT \
OPERATE_RET wf_nw_cfg_sniffer_disable(VOID);

#ifdef __cplusplus
}
#endif
#endif

