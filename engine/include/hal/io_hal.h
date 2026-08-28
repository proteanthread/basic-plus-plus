// FILENAME: io_hal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (file.h, file_internal.h, hal.h)
// NEEDS: platform, memory
// Hardware/OS Abstraction Layer for console and file I/O operations.
//
// ---- Includes ----

#ifndef HAL_IO_HAL_H
#define HAL_IO_HAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque IO handle
typedef intptr_t IoHandle;
#define IO_HANDLE_INVALID ((IoHandle)-1)

// Standard stream identifiers
#define IO_STDIN_HANDLE   ((IoHandle)0)
#define IO_STDOUT_HANDLE  ((IoHandle)1)
#define IO_STDERR_HANDLE  ((IoHandle)2)

// Seek origin flags
typedef enum {
    IO_SEEK_SET = 0,
    IO_SEEK_CUR = 1,
    IO_SEEK_END = 2
} IoSeekOrigin;

typedef struct IoHal {
    // Console / Terminal primitive operations
    int   (*console_putchar)(int c);
    int   (*console_getchar)(void);
    int   (*console_puts)(const char *str);
    bool  (*console_kbhit)(void);
    void  (*console_flush)(void);
    int   (*console_get_width)(void);
    int   (*console_get_height)(void);

    // File I/O operations
    IoHandle (*file_open)(const char *path, const char *mode);
    int      (*file_close)(IoHandle handle);
    size_t   (*file_read)(IoHandle handle, void *buffer, size_t size, size_t count);
    size_t   (*file_write)(IoHandle handle, const void *buffer, size_t size, size_t count);
    int      (*file_seek)(IoHandle handle, int64_t offset, IoSeekOrigin origin);
    int64_t  (*file_tell)(IoHandle handle);
    int      (*file_flush)(IoHandle handle);
    bool     (*file_eof)(IoHandle handle);
    int64_t  (*file_size)(const char *path);
    int      (*file_remove)(const char *path);
    int      (*file_rename)(const char *oldpath, const char *newpath);
    bool     (*file_exists)(const char *path);
} IoHal;

#ifdef __cplusplus
}
#endif

#endif // HAL_IO_HAL_H
