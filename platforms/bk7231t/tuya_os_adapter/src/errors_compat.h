#ifndef __ADAPTER_ERRORS_COMPAT_H__
#define __ADAPTER_ERRORS_COMPAT_H__


#define OPRT_OK                             (0)
#define OPRT_COM_ERROR                      (-1)
#define OPRT_INVALID_PARM                   (-2)
#define OPRT_MALLOC_FAILED                  (-3)
#define OPRT_NOT_SUPPORTED                  (-4)
#define OPRT_NETWORK_ERROR                  (-5)


#define OPRT_INIT_MUTEX_ATTR_FAILED         (-101)
#define OPRT_SET_MUTEX_ATTR_FAILED          (-102)
#define OPRT_DESTROY_MUTEX_ATTR_FAILED      (-103)
#define OPRT_INIT_MUTEX_FAILED              (-104)
#define OPRT_MUTEX_LOCK_FAILED              (-105)
#define OPRT_MUTEX_TRYLOCK_FAILED           (-106)
#define OPRT_MUTEX_LOCK_BUSY                (-107)
#define OPRT_MUTEX_UNLOCK_FAILED            (-108)
#define OPRT_MUTEX_RELEASE_FAILED           (-109)
#define OPRT_CR_MUTEX_ERR                   (-110)
#define OPRT_MEM_PARTITION_EMPTY            (-111)
#define OPRT_MEM_PARTITION_FULL             (-112)
#define OPRT_MEM_PARTITION_NOT_FOUND        (-113)
#define OPRT_DONOT_FOUND_MODULE             (-114)


#define OPRT_INIT_SEM_FAILED                (-201)
#define OPRT_WAIT_SEM_FAILED                (-202)
#define OPRT_POST_SEM_FAILED                (-203)


#define OPRT_THRD_STA_UNVALID               (-301)
#define OPRT_THRD_CR_FAILED                 (-302)
#define OPRT_THRD_JOIN_FAILED               (-303)
#define OPRT_THRD_SELF_CAN_NOT_JOIN         (-304)


// #define OPRT_WIFI_SCAN_FAIL                 (-601)
// #define OPRT_WF_MAC_SET_FAIL                (-602)
// #define OPRT_WF_CONN_FAIL                   (-603)
// #define OPRT_WF_NW_CFG_FAIL                 (-604)
#define OPRT_WF_AP_SACN_FAIL                (-605)
#define OPRT_WF_NOT_FIND_ASS_AP             (-606)


#define OPRT_ROUTER_NOT_FIND                (-1006)


#endif // __ADAPTER_ERRORS_COMPAT_H__

