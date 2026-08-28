// FILENAME: vfs.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libcore (error.c, spec.c, vfs.c)
// NEEDED BY: libengine (context.c, control.c, data.c, events_internal.h)
// NEEDED BY: libengine (exec_internal.h, vm_internal.h)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Provides core logic and interface definitions for vfs within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_VFS_H
#define RUNTIME_VFS_H

#include <stdbool.h>
#include <stddef.h>
#include "types/types.h"
#include "memory/memory.h"
#include "device/vdev.h"

#define VFS_MAX_MOUNTS 16
#define VFS_MAX_PREFIX 16
#define VFS_MAX_PATH   512

typedef enum {
    MNT_DIR,
    MNT_ZIP,
    MNT_DISK
} BppMountType;

typedef struct {
    char         prefix[VFS_MAX_PREFIX];
    char         target[VFS_MAX_PATH];
    BppMountType type;
    bool         active;
} BppMountPoint;

typedef struct VfsContext VfsContext;

VfsContext *vfs_init(MemoryContext *mem);
void        vfs_shutdown(VfsContext *ctx);

bool        vfs_mount(VfsContext *ctx, const char *prefix, const char *target, BppMountType type);
bool        vfs_umount(VfsContext *ctx, const char *prefix);
bool        vfs_resolve(VfsContext *ctx, const char *virtual_path, char *resolved_path, size_t max_len);
void        vfs_list_mounts(VfsContext *ctx, VDevContext *vdev);

const char *vfs_get_search_path(VfsContext *ctx);
void        vfs_set_search_path(VfsContext *ctx, const char *path);
const char *vfs_get_category_path(VfsContext *ctx, const char *category);
void        vfs_set_category_path(VfsContext *ctx, const char *category, const char *path);

#endif // RUNTIME_VFS_H
