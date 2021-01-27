/***********************************************************
*  File: tuya_base_utilities.h
*  Author: 愚者
*  Date: 20190808
*  Desc: 本文件提供了统一的utilities头文件引用（包括了全局的
*  错误码定义以及相关的操作宏），并提供了utilities  的一些必要
*  初始化操作。
***********************************************************/
#ifndef _TUYA_BASE_UTILITIES_H
#define _TUYA_BASE_UTILITIES_H

#include "tuya_base_utilities_error_code.h"
#include "mem_pool.h"
#include "cJSON.h"
#include "ty_cJSON.h"
#include "uni_pointer.h"
#include "uni_rfc.h"
#include "uni_queue.h"
#include "uni_slist.h"
#include "uni_hlist.h"
#include "uni_time.h"
#include "uni_thread.h"
#include "uni_network.h"
#include "mix_method.h"
#include "hashmap.h"


#ifdef __cplusplus
extern "C" {
#endif


OPERATE_RET tuya_base_utilities_init(UNW_QUERY_CUSTOM_DNS_CB dns_query_cb, UNW_SAVE_DNS_INFO_CB dns_save_cb);


#ifdef __cplusplus
}
#endif


#endif

