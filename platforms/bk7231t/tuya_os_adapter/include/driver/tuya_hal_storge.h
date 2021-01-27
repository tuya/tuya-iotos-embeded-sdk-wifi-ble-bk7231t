/**
 * @file tuya_hal_storage.h
 * @brief STORAGE设备操作接口
 * 
 * @copyright Copyright(C),2018-2020, 涂鸦科技 www.tuya.com
 * 
 */

#ifndef __TUYA_HAL_STORAGE_H__
#define __TUYA_HAL_STORAGE_H__


#include <stdint.h>


#ifdef __cplusplus
	extern "C" {
#endif


/**
 * @brief storage description
 * 
 */
typedef struct {
    uint32_t start_addr;    ///< user physical flash start address 
    uint32_t flash_sz;      ///< user flash size
    uint32_t block_sz;      ///< flash block/sector size

    // For data backup and power down protection data recovery
    uint32_t swap_start_addr;   ///< swap flash start address
    uint32_t swap_flash_sz;     ///< swap flash size    

    /// for restore factor of flash encryption key 
    uint32_t key_restore_addr;
} UNI_STORAGE_DESC_S;


/**
 * @brief UF partition
 * 
 */
typedef struct {
    uint32_t uf_partition_start_addr;
    uint32_t uf_partiton_flash_sz;
} UF_PARTITION;


/**
 * @brief UF description
 * 
 */
typedef struct {
    uint32_t sector_size;
    int32_t  uf_partition_num;
    UF_PARTITION uf_partition[3];
} UF_PARTITION_TABLE_S;


/**
 * @brief read data from flash
 * 
 * @param[in]       addr        flash address
 * @param[out]      dst         pointer of buffer
 * @param[in]       size        size of buffer
 * @retval          zero        success
 * @retval          not zero    failed
 */
int tuya_hal_flash_read(const uint32_t addr, uint8_t *dst, const uint32_t size);


/**
 * @brief write data to flash
 * 
 * @param[in]       addr        flash address
 * @param[in]       src         pointer of buffer
 * @param[in]       size        size of buffer
 * @retval          zero        success
 * @retval          not zero    failed
 */
int tuya_hal_flash_write(const uint32_t addr, const uint8_t *src, const uint32_t size);


/**
 * @brief erase flash block
 * 
 * @param[in]       addr        flash block address
 * @param[in]       size        size of flash block
 * @retval          zero        success
 * @retval          not zero    failed
 */
int tuya_hal_flash_erase(const uint32_t addr, const uint32_t size);


/**
 * @brief get description of storage
 * 
 * @return  pointer to storage description structure
 */
UNI_STORAGE_DESC_S* tuya_hal_storage_get_desc(void);


/**
 * @brief get UF file description
 * 
 * @return  pointer to descrtion of UF file
 */
UF_PARTITION_TABLE_S* tuya_hal_uf_get_desc(void);


#ifdef __cplusplus
}
#endif

#endif // __TUYA_HAL_STORAGE_H__

