# Virtual Filesystem API Reference

Header File: [`include/bpp_vfs.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_vfs.h)

## Overview
Provides a sandboxed unified directory structure mapping memory buffers or disk to paths.

## Exposed API Entities
### Structs & Types
- `VfsContext VfsContext`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `vfs_shutdown` | `void` | `VfsContext *ctx` |
| `vfs_mount` | `bool` | `VfsContext *ctx, const char *prefix, const char *target, BppMountType type` |
| `vfs_umount` | `bool` | `VfsContext *ctx, const char *prefix` |
| `vfs_resolve` | `bool` | `VfsContext *ctx, const char *virtual_path, char *resolved_path, size_t max_len` |
| `vfs_list_mounts` | `void` | `VfsContext *ctx, VDevContext *vdev` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_vfs.h"

void mount_vfs(VfsContext *ctx) {
    vfs_mount(ctx, "/ram", "/tmp", MNT_DIR);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
