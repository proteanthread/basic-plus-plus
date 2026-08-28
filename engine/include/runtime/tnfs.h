// FILENAME: tnfs.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (tnfs.c)
// NEEDED BY: libengine (func_tnfs.c, stmt_tnfs.c)
// NEEDED BY: libkernel (fujinet.c)
// NEEDS: platform, memory
// Provides the TNFS (Trusted Network File System - UDP port 9868) client protocol.
//
// ---- Includes ----

#ifndef RUNTIME_TNFS_H
#define RUNTIME_TNFS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TNFS_DEFAULT_PORT 9868

#define TNFS_CMD_MOUNT    0x00
#define TNFS_CMD_UNMOUNT  0x01
#define TNFS_CMD_OPENDIR  0x10
#define TNFS_CMD_READDIR  0x11
#define TNFS_CMD_CLOSEDIR 0x12
#define TNFS_CMD_OPEN     0x20
#define TNFS_CMD_READ     0x21
#define TNFS_CMD_WRITE    0x22
#define TNFS_CMD_CLOSE    0x23
#define TNFS_CMD_STAT     0x30

#define TNFS_SUCCESS      0x00

typedef struct TnfsSession {
    char host[128];
    uint16_t port;
    char mount_path[256];
    uint16_t session_id;
    uint8_t seq_num;
    bool is_mounted;
} TnfsSession;

void tnfs_init(void);
void tnfs_shutdown(void);

bool tnfs_mount(const char *host, uint16_t port, const char *mount_path);
void tnfs_unmount(void);
bool tnfs_is_mounted(void);

const char *tnfs_get_mount_host(void);
const char *tnfs_get_mount_path(void);

// Directory operations
char *tnfs_list_directory(const char *dir_path, const char *pattern);

// File transfer operations
char *tnfs_read_file(const char *file_path, size_t *out_size);
bool tnfs_write_file(const char *file_path, const char *data, size_t size);

#endif // RUNTIME_TNFS_H
