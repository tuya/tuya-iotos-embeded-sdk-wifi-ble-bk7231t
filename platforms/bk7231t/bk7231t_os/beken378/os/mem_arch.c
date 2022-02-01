#include "include.h"
#include "arm_arch.h"
#include <string.h>

#include "sys_rtos.h"
#include "uart_pub.h"

INT32 os_memcmp(const void *s1, const void *s2, UINT32 n)
{
    return memcmp(s1, s2, (unsigned int)n);
}

void *os_memmove(void *out, const void *in, UINT32 n)
{
    return memmove(out, in, n);
}

void *os_memcpy(void *out, const void *in, UINT32 n)
{
    return memcpy(out, in, n);
}

void *os_memset(void *b, int c, UINT32 len)
{
    return (void *)memset(b, c, (unsigned int)len);
}

void *os_realloc(void *ptr, size_t size)
{
	#ifdef FIX_REALLOC_ISSUE
    return pvPortRealloc(ptr, size);
	#else
	void *tmp;

    if(platform_is_in_interrupt_context())
    {
        os_printf("realloc_risk\r\n");
    }

	tmp = (void *)pvPortMalloc(size);
	if(tmp)
	{
		os_memcpy(tmp, ptr, size);
		vPortFree(ptr);
	}

	return tmp;
	#endif
}

int os_memcmp_const(const void *a, const void *b, size_t len)
{
    return memcmp(a, b, len);
}

// becuse libraries contain os_malloc, we must provide them...
#if OSMALLOC_STATISTICAL
#undef os_malloc
#undef os_free
#undef os_zalloc
#endif



void *os_malloc(size_t size)
{
    if(platform_is_in_interrupt_context())
    {
        os_printf("malloc_risk\r\n");
    }
#if OSMALLOC_STATISTICAL
    return (void *)pvPortMalloc_cm(__FILE__, __LINE__, size, 0);
#else
    return (void *)pvPortMalloc(size);
#endif
}

void * os_zalloc(size_t size)
{
#if OSMALLOC_STATISTICAL
    return (void *)pvPortMalloc_cm(__FILE__, __LINE__, size, 1);
#else
	void *n = (void *)pvPortMalloc(size);
    
    if(platform_is_in_interrupt_context())
    {
        os_printf("zalloc_risk\r\n");
    }
    
	if (n)
		os_memset(n, 0, size);
	return n;
#endif
}

void os_free(void *ptr)
{
    if(platform_is_in_interrupt_context())
    {
        os_printf("free_risk\r\n");
    }
    
    if(ptr)
    {        
#if OSMALLOC_STATISTICAL
        vPortFree_cm(__FILE__, __LINE__, ptr);
#else
        vPortFree(ptr);
#endif
    }
}

void *__wrap_malloc(size_t size)
{    
	os_printf("__wrap_malloc\r\n");
    return (void *)os_malloc(size);
}

void * __wrap_zalloc(size_t size)
{
	os_printf("__wrap_zalloc\r\n");
	return os_zalloc(size);
}

void __wrap_free(void *ptr)
{
	os_printf("__wrap_free\r\n");
	os_free(ptr);
}

// EOF
