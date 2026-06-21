/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vfs.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Input/Output operations, screen print layout, file streams (sequential/binary/random), block I/O formatting, and record buffers.
 *
 * 2. WHAT TO EXPECT:
 *    Delegates file operations through the Virtual Device (VDev) mapping layer. Relies on standard ANSI C streams.
 *
 * 3. WHAT CAN BE CHANGED:
 *    File channel limit rules, I/O default buffers, record layout details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Binary packing format definitions (MKI$, MKS$, MKD$), file channel index lookups.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check file channel descriptor leaks (ensure CLOSE #ch is called). Verify binary file alignments and record record size boundaries.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - vfs.h
 // ---
 //
 // Virtual Filesystem Layer.
 //
 // Provides two path-resolution features:
 //
 //   1. MOUNT TABLE -- Maps virtual drive prefixes to real
 //      filesystem paths. Prefixes use the GW-BASIC/QBasic
 //      colon convention (e.g. "A:", "USB:").
 //
 //      MOUNT "A:" TO "/mnt/usb"
 //      OPEN "A:data.txt" FOR INPUT AS #1
 //        -> resolves to "/mnt/usb/data.txt"
 //
 //   2. VPATH -- File search path for reads. When a relative
 //      filename is not found in the current directory, each
 //      directory in VPATH is tried in order.
 //
 //      VPATH "lib;modules;."      (Windows: ; delimiter)
 //      VPATH "lib:modules:."      (Linux:   : delimiter)
 //      LOAD "utils.bas"
 //        -> tries CWD, then lib/, then modules/, then ./
 //
 // DESIGN:
 //   - Static allocation (no malloc). 8 mount slots.
 //   - vfs_resolve() is the single entry point. All
 //     file-opening code calls it before fopen().
 //   - Write operations resolve mounts but skip VPATH
 //     (you always write to a specific location).
 //   - Mount prefixes require trailing colon, matching
 //     VDev naming and GW-BASIC/QBasic semantics.
 //
 // ---

#ifndef BASICPP_VFS_H
#define BASICPP_VFS_H

// Maximum mount slots
#define VFS_MAX_MOUNTS 8

// Maximum prefix length including colon (e.g. "USB:")
#define VFS_MAX_PREFIX 16

// Maximum target path length
#define VFS_MAX_TARGET 512

// Maximum VPATH string length
#define VFS_MAX_VPATH 1024

 // Platform-aware VPATH delimiter.
 // Windows uses ';' (since ':' is used in drive letters).
 // Linux uses ':' (standard Unix convention).
#ifdef _WIN32
#define VFS_PATH_DELIM ';'
#else
#define VFS_PATH_DELIM ':'
#endif

// Mount table entry
typedef struct VFSMount {
 char prefix[VFS_MAX_PREFIX]; // "A:", "USB:" etc.
 char target[VFS_MAX_TARGET]; // real path
 int  active; // 1 = in use, 0 = free
} VFSMount;

 // vfs_init - Initialize VFS (clear mounts, empty VPATH).
 // Called during boot.
void vfs_init(void);

 // vfs_mount - Add a mount point.
 //
 // prefix: Virtual drive name WITH trailing colon
 //         (e.g. "A:", "USB:", "RAM:").
 // target: Real filesystem path to map to
 //         (e.g. "/mnt/usb", "D:\External").
 // line_num: BASIC line number for error reporting.
 //
 // Returns 0 on success, -1 on error.
 // Errors: prefix too long, target too long, no free slots,
 //         prefix missing colon, prefix already mounted.
int vfs_mount(const char *prefix, const char *target,
 int line_num);

 // vfs_umount - Remove a mount point.
 //
 // prefix: Virtual drive name to unmount.
 // line_num: BASIC line number for error reporting.
 //
 // Returns 0 on success, -1 if not found.
int vfs_umount(const char *prefix, int line_num);

 // vfs_list_mounts - Print all active mounts to stdout.
 // Used by the MOUNTS command.
void vfs_list_mounts(void);

 // vfs_resolve - Resolve a virtual path to a real path.
 //
 // This is the core path resolution function. All file-opening
 // code calls this before fopen().
 //
 // Resolution order:
 //   1. Mount prefix match: "A:file.txt" -> target + "/file.txt"
 //   2. If for_write is false and file not found at path:
 //      try each directory in VPATH.
 //   3. Passthrough: copy path as-is.
 //
 // Parameters:
 //   path     - input filename from BASIC code
 //   out      - output buffer for resolved path
 //   out_max  - size of output buffer
 //   for_write - 1 = write operation (skip VPATH search),
 //               0 = read operation (search VPATH)
 //
 // Returns 0 on success (resolved path in out),
 //        -1 on error (buffer too small).
int vfs_resolve(const char *path, char *out,
 int out_max, int for_write);

 // vfs_set_vpath - Set the file search path.
 //
 // path: semicolon-delimited (Win) or colon-delimited (Linux)
 //       list of directories.
 //       Example: "lib;modules;." (Windows)
 //       Example: "lib:modules:." (Linux)
 //
 // Pass NULL or "" to clear the VPATH.
void vfs_set_vpath(const char *path);

 // vfs_get_vpath - Get the current VPATH string.
 // Returns pointer to internal static buffer.
const char *vfs_get_vpath(void);

#endif // BASICPP_VFS_H
