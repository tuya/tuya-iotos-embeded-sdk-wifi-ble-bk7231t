
#include <FreeRTOS.h>
#include "tuya_hal_memory.h"
#include "tuya_hal_system_internal.h"


/***********************************************************
*************************micro define***********************
***********************************************************/

/***********************************************************
*************************variable define********************
***********************************************************/
static TUYA_MALLOC_FUNC_T s_internal_malloc_func = NULL;
static TUYA_FREE_FUNC_T   s_internal_free_func   = NULL;

/***********************************************************
*************************function define********************
***********************************************************/

void *tuya_hal_system_malloc(const size_t size)
{
    return pvPortMalloc(size);
}

void tuya_hal_system_free(void* ptr)
{
    vPortFree(ptr);
}

int tuya_hal_set_mem_func(TUYA_MALLOC_FUNC_T malloc_func, TUYA_FREE_FUNC_T free_func)
{
    s_internal_malloc_func = malloc_func;
    s_internal_free_func = free_func;
    return 0;
}

void* tuya_hal_internal_malloc(const size_t size)
{
    if (s_internal_malloc_func) {
        return s_internal_malloc_func(size);
    } else {
        return pvPortMalloc(size);
    }
}

void tuya_hal_internal_free(void* ptr)
{
    if (s_internal_free_func) {
        s_internal_free_func(ptr);
    } else {
        vPortFree(ptr);
    }
}

