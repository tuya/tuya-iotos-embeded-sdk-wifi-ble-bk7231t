#include "include.h"
#include "fake_clock_pub.h"
#include "icu_pub.h"
#include "drv_model_pub.h"
#include "uart_pub.h"

#if CFG_SUPPORT_ALIOS
#include "rtos_pub.h"
#include "ll.h"
#include "k_api.h"
#else
#include "sys_rtos.h"
#include "arm_arch.h"
#endif

#include "start_type_pub.h"


static RESET_SOURCE_STATUS start_type;
RESET_SOURCE_STATUS bk_misc_get_start_type()
{
    return start_type;
}

//only can be do once
 RESET_SOURCE_STATUS bk_misc_init_start_type(void)
{
    uint32_t misc_value = *((volatile uint32_t *)(START_TYPE_ADDR));

    switch(misc_value) 
    { 
        case RESET_SOURCE_REBOOT:
            start_type = misc_value;
            break;      
        case CRASH_UNDEFINED_VALUE:
            start_type = RESET_SOURCE_CRASH_UNDEFINED;
            break;        
        case CRASH_PREFETCH_ABORT_VALUE:
            start_type = RESET_SOURCE_CRASH_PREFETCH_ABORT;
            break;        
        case CRASH_DATA_ABORT_VALUE:
            start_type = RESET_SOURCE_CRASH_DATA_ABORT;
            break;        
        case CRASH_UNUSED_VALUE:
            start_type = RESET_SOURCE_CRASH_UNUSED;
            break;
        case CRASH_XAT0_VALUE:
            start_type = RESET_SOURCE_CRASH_XAT0;
            break;

        case RESET_SOURCE_WATCHDOG:
            if((uint32_t)CRASH_XAT0_VALUE ==
                *((volatile uint32_t *)(START_TYPE_DMEMORY_ADDR)))
            {
                start_type = RESET_SOURCE_CRASH_XAT0;
            }
            else
            {
                start_type = misc_value;
            }
            break;  
        
        default: 
            start_type = RESET_SOURCE_POWERON;
            break; 
    }

    *((volatile uint32_t *)(START_TYPE_DMEMORY_ADDR)) = (uint32_t)CRASH_XAT0_VALUE;

    //os_printf("bk_misc_init_start_type %x %x\r\n",start_type,misc_value);
    return start_type;
}

 void bk_misc_update_set_type(RESET_SOURCE_STATUS type)
{
    *((volatile uint32_t *)(START_TYPE_ADDR)) = (uint32_t)type;
    //os_printf("bk_wlan_update_set_type %d\r\n",*((volatile uint32_t *)(START_TYPE_ADDR)));
}
