#ifndef __BT_NETCFG__
#define __BT_NETCFG__


typedef OPERATE_RET (*FN_BT_NET_CFG_CB)(IN CONST CHAR_T *ssid,IN CONST CHAR_T *passwd, IN CONST CHAR_T *token);


int bt_netcfg_init();

#endif
