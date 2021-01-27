#ifndef __TUYA_WS_DB_DEV_H__
#define __TUYA_WS_DB_DEV_H__


#include "tuya_ws_db.h"
#include "gw_intf.h"


#ifdef __cplusplus
    extern "C" {
#endif




/***********************************************************
*  Function: wd_dev_if_write
*  Input: ddi
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET wd_dev_if_write(IN DEV_DESC_IF_S *ddi);

/***********************************************************
*  Function: wd_dev_if_delete_all
*  Input: none
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET wd_dev_if_delete_all(VOID);

OPERATE_RET wd_dev_if_delete(IN CONST CHAR_T *p_dev_id);



/***********************************************************
*  Function: wd_dev_if_read
*  Input: index->from 0
*  Output: index->the find index +1 ddi
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET wd_dev_if_read(INOUT UINT_T *index,OUT DEV_DESC_IF_S *ddi);

/***********************************************************
*  Function: wd_dev_schema_write
*  Input: s_id schema
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET wd_dev_schema_write(IN CONST CHAR_T *s_id,IN CONST CHAR_T *schema);

/***********************************************************
*  Function: wd_dev_schema_read
*  Input: s_id
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET wd_dev_schema_read(IN CONST CHAR_T *s_id,OUT CHAR_T **schema);


/***********************************************************
*  Function: wd_dev_schema_read
*  Input: s_id
*  Output: none
*  Return: OPERATE_RET
***********************************************************/
OPERATE_RET wd_dev_schema_delete(IN CONST CHAR_T *s_id);


#ifdef __cplusplus
}
#endif

#endif  // __TUYA_WS_DB_DEV_H__

