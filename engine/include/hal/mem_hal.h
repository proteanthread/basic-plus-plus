// FILENAME: mem_hal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (hal.h)
// NEEDS: platform, memory
// Hardware/OS Abstraction Layer for host memory allocation and arena management.
//
// ---- Includes ----

#ifndef HAL_MEM_HAL_H
#define HAL_MEM_HAL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MemHal {
    void *(*alloc)(size_t size);
    void *(*calloc)(size_t count, size_t size);
    void *(*realloc)(void *ptr, size_t size);
    void  (*free)(void *ptr);
    void  (*lock)(void);
    void  (*unlock)(void);
} MemHal;

#ifdef __cplusplus
}
#endif

#endif // HAL_MEM_HAL_H
