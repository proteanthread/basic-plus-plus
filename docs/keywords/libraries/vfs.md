# `vfs` Virtual File System Subsystem (`libserver`)

## 1. Architectural Purpose & Overview

The `vfs` subsystem (`engine/src/server/vfs.c`) abstracts disk files, memory-mapped assets, archive bundles (`.zip`, `.tar`), and network-mounted filesystems under a unified path hierarchy.

### Key Architectural Invariants:
- **Unified Path Routing**: Routes paths (`ram:/`, `tnfs://`, `zip://`, `file://`) to corresponding VFS driver backends.
- **Sandboxing**: Restricts file tree traversals to authorized sandbox root directories.

---

## 2. Technical API Signatures (C17)

```c
VFSFile *vfs_open(const char *path, const char *mode);
size_t vfs_read(VFSFile *f, void *buf, size_t size);
size_t vfs_write(VFSFile *f, const void *buf, size_t size);
void vfs_close(VFSFile *f);
```
