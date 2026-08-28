# C17 API Reference: Virtual File System (`runtime/vfs.h`)

## 1. Subsystem Overview & Responsibilities

The Virtual File System Subsystem (`runtime/vfs.h`, implemented in `engine/src/runtime/vfs.c`) provides logical device prefix routing (`DATA:`, `ROM:`, `SD:`, `RAM:`), mount point management (`MOUNT`/`UMOUNT`), search path resolution (`VPATH`), and physical path normalization for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Mount Point Table Management**: Manages up to 16 concurrent virtual storage mount points (`VFS_MAX_MOUNTS`).
- **Storage Backend Types (`BppMountType`)**:
  - `MOUNT_DIR`: Host operating system filesystem directory.
  - `MOUNT_RAM`: In-memory volatile RAM disk image.
  - `MOUNT_ROM`: Read-only system archive image.
  - `MOUNT_DEV`: Virtual device bus redirection.
- **Fast Prefix Path Resolution (`vfs_resolve`)**: Translates virtual prefix paths (e.g. `DATA:CONFIG.INI`) into canonical host filesystem paths (`C:\Projects\Data\CONFIG.INI`) with $O(1)$ prefix matching and automatic path normalization.
- **Library Search Paths (`VPATH`)**: Resolves unadorned script filenames (`LOAD "HELPER"`) against the search path list configured in `VfsContext`.

## 2. Header Inclusion & Prerequisites

```c
#include "runtime/vfs.h"
#include "memory/memory.h"
#include "device/vdev.h"
```

## 3. Data Structures & Types

```c
#define VFS_MAX_MOUNTS 16
#define VFS_MAX_PATH   256
#define VFS_MAX_PREFIX 16

typedef enum {
    MOUNT_DIR = 0,      /* Host Directory */
    MOUNT_RAM = 1,      /* In-Memory RAM Disk */
    MOUNT_ROM = 2,      /* Read-Only ROM Archive */
    MOUNT_DEV = 3       /* Virtual Device */
} BppMountType;

typedef struct {
    char         prefix[VFS_MAX_PREFIX];    /* Logical prefix (e.g. "DATA:") */
    char         target[VFS_MAX_PATH];      /* Physical target path */
    BppMountType type;                      /* Storage type */
    bool         active;                    /* Slot in-use */
} BppMountPoint;

/* Opaque Virtual Filesystem Context */
typedef struct VfsContext VfsContext;
```

## 4. Function Prototypes & Operational Contracts

### Context Lifecycle & Mount Operations
```c
VfsContext *vfs_init(MemoryContext *mem);
void        vfs_shutdown(VfsContext *ctx);

/**
 * @brief Mounts a virtual prefix to a physical directory or memory image.
 */
bool vfs_mount(VfsContext *ctx, const char *prefix, const char *target, BppMountType type);

/**
 * @brief Unmounts an active virtual prefix.
 */
bool vfs_umount(VfsContext *ctx, const char *prefix);

/**
 * @brief Translates a virtual path into a canonical host filesystem path.
 */
bool vfs_resolve(VfsContext *ctx, const char *virtual_path, char *resolved_path, size_t max_len);

/**
 * @brief Formats and displays the active mount table to a virtual console.
 */
void vfs_list_mounts(VfsContext *ctx, VDevContext *vdev);
```

### Search Path Configuration (`VPATH`)
```c
void        vfs_set_search_path(VfsContext *ctx, const char *path);
const char *vfs_get_search_path(VfsContext *ctx);
```

## 5. Architectural Invariants

- **Trailing Colon Normalization**: Prefixes without a trailing colon are automatically padded to guarantee consistent matching.
- **Sandbox Compliance**: Mount operations verify `CAP_FS` capability in `SecurityContext`.

## 6. Code Example: Mounting a Project Volume in C

```c
#include "runtime/vfs.h"

void setup_project_mounts(VfsContext *vfs) {
    vfs_mount(vfs, "DATA:", "./assets/data", MOUNT_DIR);
    vfs_mount(vfs, "ROM:", "/usr/local/share/basicpp/rom", MOUNT_ROM);
}
```
