#ifndef __DIRECT_CONNECT_TUYA_H__
#define __DIRECT_CONNECT_TUYA_H__

#include "tuya_cloud_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef VOID (*REV_BIND_TOKEN_CB)(IN CONST CHAR_T *token);

typedef struct {
    CHAR_T *p_uuid;
    CHAR_T *p_authkey;
    CHAR_T *p_region;
    REV_BIND_TOKEN_CB rev_token_cb;
}DIRECT_CON_INPUT_S;

OPERATE_RET direct_connect_tuya_cloud(IN CONST DIRECT_CON_INPUT_S *p_input);



#ifdef __cplusplus
}
#endif
#endif

