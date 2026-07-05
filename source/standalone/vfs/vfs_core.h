/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vfs_core.h
 * Subsystem: Virtual Filesystem Directory Driver
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Emulates disk directories and catalog files structures.
 *
 * 2. WHAT TO EXPECT:
 *    Intercepts file path routes mapping to local folders.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Drive mappings presets.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    VFS catalog formats.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If path is not found, verify mapping definitions.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE VIRTUAL FILESYSTEM (VFS) SUBSYSTEM
 * File: vfs_core.h
 * =====================================================================
 * Standard C interface for virtual mounting and path resolution.
 * Completely independent of the interpreter.
 * ===================================================================== */

#ifndef STANDALONE_VFS_CORE_H
#define STANDALONE_VFS_CORE_H

#define VFS_MAX_MOUNTS 8
#define VFS_MAX_PREFIX 16
#define VFS_MAX_TARGET 512
#define VFS_MAX_VPATH  1024

#ifdef _WIN32
#define VFS_PATH_DELIM ';'
#else
#define VFS_PATH_DELIM ':'
#endif

typedef struct VFSMount {
    char prefix[VFS_MAX_PREFIX]; /* "A:", "USB:", etc. */
    char target[VFS_MAX_TARGET]; /* Real physical folder path */
    int active;                  /* 1 = active, 0 = free */
} VFSMount;

/* Public API */
void vfs_core_init(void);
int vfs_core_mount(const char *prefix, const char *target);
int vfs_core_umount(const char *prefix);
void vfs_core_list_mounts(void (*print_cb)(const char *prefix, const char *target));
int vfs_core_resolve(const char *path, char *out, int out_max, int for_write);
void vfs_core_set_vpath(const char *path);
const char *vfs_core_get_vpath(void);

#endif /* STANDALONE_VFS_CORE_H */
