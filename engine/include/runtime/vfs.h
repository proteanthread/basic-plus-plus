/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file vfs.h
 * @brief Runtime component implementation and public API surface for vfs.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for vfs.h within the runtime subsystem.
 *
 * WHY IT EXISTS:
 * Ensures decoupled modularity, strict C17 portability, and clear micro-library architectural boundary enforcement.
 *
 * WHY IT WORKS THIS WAY:
 * Designed with zero-initialization defaults, bounded memory operations, and explicit error code propagation to the VM state.
 *
 * WHAT CAN BE CHANGED:
 * Subsystem configuration defaults, local execution helper routines, and documentation annotations.
 *
 * WHAT CANNOT BE CHANGED:
 * Public API symbol declarations, micro-library metadata structures, and thread-safe error reporting contracts.
 *
 * WHAT TO EXPECT:
 * High-performance deterministic execution with zero side-effects outside designated state structures.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify context initialization, trace BppError return codes, and inspect log outputs for bounds assertions.
 *
 * ASSUMPTIONS:
 * Valid subsystem contexts and required memory pools are allocated prior to executing API handlers.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, 64-bit pointer safety, and pure ASCII string operations across desktop, IoT, and embedded targets.
 *
 * FUTURE EXPANSIONS:
 * Additional dialect compatibility mappings, telemetry instrumentation, and microcontroller payload stubs.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file vfs.h
 * @brief Virtual Filesystem (VFS) Mount & Path virtualization interface.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares interfaces for mounting directories, zip files, or raw disk images
 *   to virtual drives, and resolving paths.
 * - Why it exists: Decouples the compiler's storage system from raw host paths, enabling
 *   virtualization and sandboxing.
 * - Why it works this way: It manages a static list of mount points and resolves paths in
 *   a thread-safe manner.
 */

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

#endif /* RUNTIME_VFS_H */
