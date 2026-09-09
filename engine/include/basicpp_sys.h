// FILENAME: basicpp_sys.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: freestanding C17 systems, bare-metal kernels, UEFI firmware
// NEEDS: <stdint.h>, <stddef.h>, <stdbool.h>
// Freestanding C17 Systems Programming and Bare-Metal Hardware Access API.
//

#ifndef BASICPP_SYS_H
#define BASICPP_SYS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Freestanding primitive aliases
typedef uint8_t  sys_u8;
typedef uint16_t sys_u16;
typedef uint32_t sys_u32;
typedef uint64_t sys_u64;
typedef int8_t   sys_i8;
typedef int16_t  sys_i16;
typedef int32_t  sys_i32;
typedef int64_t  sys_i64;
typedef size_t   sys_usize;
typedef ptrdiff_t sys_isize;

// Compiler memory barrier
static inline void sys_memory_barrier(void) {
#if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("" ::: "memory");
#elif defined(_MSC_VER)
    #if defined(_M_IX86) || defined(_M_X64)
    _ReadWriteBarrier();
    #endif
#endif
}

// Volatile MMIO 8-bit accessor
static inline sys_u8 sys_mem_read8(uintptr_t addr) {
    sys_memory_barrier();
    return *(volatile sys_u8 *)addr;
}

// Volatile MMIO 8-bit writer
static inline void sys_mem_write8(uintptr_t addr, sys_u8 val) {
    *(volatile sys_u8 *)addr = val;
    sys_memory_barrier();
}

// Volatile MMIO 16-bit accessor
static inline sys_u16 sys_mem_read16(uintptr_t addr) {
    sys_memory_barrier();
    return *(volatile sys_u16 *)addr;
}

// Volatile MMIO 16-bit writer
static inline void sys_mem_write16(uintptr_t addr, sys_u16 val) {
    *(volatile sys_u16 *)addr = val;
    sys_memory_barrier();
}

// Volatile MMIO 32-bit accessor
static inline sys_u32 sys_mem_read32(uintptr_t addr) {
    sys_memory_barrier();
    return *(volatile sys_u32 *)addr;
}

// Volatile MMIO 32-bit writer
static inline void sys_mem_write32(uintptr_t addr, sys_u32 val) {
    *(volatile sys_u32 *)addr = val;
    sys_memory_barrier();
}

// Volatile MMIO 64-bit accessor
static inline sys_u64 sys_mem_read64(uintptr_t addr) {
    sys_memory_barrier();
    return *(volatile sys_u64 *)addr;
}

// Volatile MMIO 64-bit writer
static inline void sys_mem_write64(uintptr_t addr, sys_u64 val) {
    *(volatile sys_u64 *)addr = val;
    sys_memory_barrier();
}

// x86/x86_64 Port I/O 8-bit output
static inline void sys_out8(sys_u16 port, sys_u8 val) {
#if defined(__GNUC__) || defined(__clang__)
    #if defined(__i386__) || defined(__x86_64__)
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
    #else
    (void)port; (void)val;
    #endif
#else
    (void)port; (void)val;
#endif
}

// x86/x86_64 Port I/O 8-bit input
static inline sys_u8 sys_in8(sys_u16 port) {
#if defined(__GNUC__) || defined(__clang__)
    #if defined(__i386__) || defined(__x86_64__)
    sys_u8 ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
    #else
    (void)port; return 0;
    #endif
#else
    (void)port; return 0;
#endif
}

// x86/x86_64 Port I/O 16-bit output
static inline void sys_out16(sys_u16 port, sys_u16 val) {
#if defined(__GNUC__) || defined(__clang__)
    #if defined(__i386__) || defined(__x86_64__)
    __asm__ __volatile__("outw %0, %1" : : "a"(val), "Nd"(port));
    #else
    (void)port; (void)val;
    #endif
#else
    (void)port; (void)val;
#endif
}

// x86/x86_64 Port I/O 16-bit input
static inline sys_u16 sys_in16(sys_u16 port) {
#if defined(__GNUC__) || defined(__clang__)
    #if defined(__i386__) || defined(__x86_64__)
    sys_u16 ret;
    __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
    #else
    (void)port; return 0;
    #endif
#else
    (void)port; return 0;
#endif
}

// Freestanding memory copy implementation (zero libc)
static inline void sys_memcpy(void *dst, const void *src, sys_usize n) {
    sys_u8 *d = (sys_u8 *)dst;
    const sys_u8 *s = (const sys_u8 *)src;
    for (sys_usize i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

// Freestanding memory set implementation (zero libc)
static inline void sys_memset(void *dst, sys_u8 val, sys_usize n) {
    sys_u8 *d = (sys_u8 *)dst;
    for (sys_usize i = 0; i < n; i++) {
        d[i] = val;
    }
}

// Freestanding string length (zero libc)
static inline sys_usize sys_strlen(const char *s) {
    if (!s) return 0;
    sys_usize len = 0;
    while (s[len]) len++;
    return len;
}

// Certified Memory Allocation Model 5: Monotonic Arena for Bare-Metal
typedef struct SysArena {
    sys_u8 *buffer;
    sys_usize capacity;
    sys_usize offset;
} SysArena;

// Initialize monotonic arena with static buffer
static inline void sys_arena_init(SysArena *arena, sys_u8 *buf, sys_usize cap) {
    arena->buffer = buf;
    arena->capacity = cap;
    arena->offset = 0;
}

// Allocate from monotonic arena with 8-byte alignment
static inline void *sys_arena_alloc(SysArena *arena, sys_usize bytes) {
    sys_usize aligned = (bytes + 7) & ~((sys_usize)7);
    if (arena->offset + aligned > arena->capacity) {
        return NULL;
    }
    void *ptr = &arena->buffer[arena->offset];
    arena->offset += aligned;
    return ptr;
}

// Reset arena offset for bulk teardown
static inline void sys_arena_reset(SysArena *arena) {
    arena->offset = 0;
}

// Bare-metal text console / serial device hooks
typedef struct SysConsole {
    void (*putc)(void *ctx, char c);
    void (*puts)(void *ctx, const char *s);
    char (*getc)(void *ctx);
    bool (*has_char)(void *ctx);
    void (*clear)(void *ctx);
    void *ctx;
} SysConsole;

// Convert integer to ASCII string (freestanding itoa)
static inline void sys_itoa(sys_i64 val, char *buf, int base) {
    if (base < 2 || base > 16 || !buf) return;
    char tmp[65];
    int idx = 0;
    bool neg = false;
    uint64_t uval;
    if (val < 0 && base == 10) {
        neg = true;
        uval = (uint64_t)(-val);
    } else {
        uval = (uint64_t)val;
    }
    if (uval == 0) tmp[idx++] = '0';
    while (uval > 0) {
        int rem = (int)(uval % (uint64_t)base);
        tmp[idx++] = (rem < 10) ? ('0' + rem) : ('A' + rem - 10);
        uval /= (uint64_t)base;
    }
    int out = 0;
    if (neg) buf[out++] = '-';
    while (idx > 0) buf[out++] = tmp[--idx];
    buf[out] = '\0';
}

#endif // BASICPP_SYS_H
