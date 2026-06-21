/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vfs.c
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
// BASIC++ Interpreter - vfs.c
// ---
//
// Virtual Filesystem implementation.
//
// PURPOSE:
//   Provides a virtual filesystem layer between BASIC file
//   operations and the real OS filesystem. This enables:
//   - Virtual drive letters (A:, B:, USB:, NET:) mapped to
//     real directories via the MOUNT command.
//   - Search paths (VPATH) for finding files in multiple
//     directories, similar to Unix PATH or make's VPATH.
//
// HOW IT WORKS:
//   All file path resolution goes through vfs_resolve(), which
//   applies three steps in order:
//
//   Step 1 -- Mount Table Lookup:
//     Check if the path starts with a mounted prefix (e.g., "A:").
//     If so, replace the prefix with the mount target directory.
//     Example: MOUNT "A:", "C:\projects"
//              "A:hello.bas" -> "C:\projects\hello.bas"
//
//   Step 2 -- VPATH Search (reads only):
//     If the file is not found at the resolved path and this is
//     a read operation, search each directory in VPATH.
//     Example: OPTION VPATH "C:\lib;D:\stdlib"
//              LOAD "utils.bas" -> tries "utils.bas" in CWD,
//              then "C:\lib\utils.bas", then "D:\stdlib\utils.bas"
//
//   Step 3 -- Passthrough:
//     If nothing matched, the path is returned as-is.
//
// HOW TO EXTEND / CUSTOMIZE:
//   Adding a new virtual prefix type:
//   - Currently, prefixes are simple string-to-directory mappings.
//   - To add protocol-style prefixes (e.g., "HTTP:", "FTP:"),
//     add a new resolution step in vfs_resolve() that detects
//     the protocol and delegates to a handler function.
//
//   Increasing mount slots:
//   - Edit VFS_MAX_MOUNTS in vfs.h (default: 8).
//   - Each slot costs ~532 bytes (prefix + target buffers).
//
//   Changing VPATH delimiter:
//   - VFS_PATH_DELIM is ';' on Windows, ':' on Linux/macOS.
//   - Defined in vfs.h. Change if you need a different separator.
//
// FINE-TUNING:
//   VFS_MAX_MOUNTS   -- maximum simultaneous mount points (default 8)
//   VFS_MAX_PREFIX   -- max prefix string length (default 16)
//   VFS_MAX_TARGET   -- max target path length (default 512)
//   VFS_MAX_VPATH    -- max VPATH string length (default 2048)
//
// TROUBLESHOOTING:
//   - "Mount prefix must end with ':'":
//     The MOUNT command requires the prefix to end with a colon,
//     following GW-BASIC/QBasic drive letter convention.
//     Use: MOUNT "A:", "C:\path"  (not MOUNT "A", "C:\path")
//
//   - "Mount prefix too long":
//     The prefix exceeds VFS_MAX_PREFIX characters. Shorten it
//     or increase VFS_MAX_PREFIX in vfs.h.
//
//   - "No free mount slots":
//     All VFS_MAX_MOUNTS slots are in use. UMOUNT an existing
//     mount first, or increase VFS_MAX_MOUNTS in vfs.h.
//
//   - VPATH not finding files:
//     VPATH only applies to read operations (LOAD, OPEN FOR INPUT).
//     It does NOT apply to writes (SAVE, OPEN FOR OUTPUT).
//     Check that the VPATH delimiter matches your platform.
//
// PERFORMANCE:
//   - vfs_resolve() is O(M * P + V * D) where:
//     M = mount count, P = prefix length (comparison)
//     V = VPATH entries, D = access() syscalls per entry
//   - Mount lookup is linear scan (M <= 8, negligible).
//   - VPATH search incurs one access() syscall per directory.
//     Keep VPATH short for best performance.
//
// MINIMALIZATION:
//   For builds without virtual filesystem (PATB, embedded):
//   - Remove vfs.c from the build entirely.
//   - Replace vfs_resolve() calls with direct path passthrough.
//   - Saves ~2 KB code + ~3 KB static buffers.
//
// DEPENDENCIES:
//   - vfs.h    (VFSMount struct, constants)
//   - errors.h (error_raise for mount failures)
//   - Platform: _access() (Windows) or access() (POSIX)
//
// C17 COMPLIANCE:
//   - Uses _access/_io.h on Windows, access/unistd.h on POSIX.
//   - All platform differences handled via #ifdef _WIN32.
//   - Compiles cleanly on MSVC /std:c17 and gcc -std=c17.
//   - Fixed: bool variables now use true/false consistently.
//   - Fixed: shadow variable 'i' eliminated in vfs_mount/vfs_umount.
//
// ---

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "vfs.h"
#include "errors.h"

#ifdef _WIN32
#include <io.h>
#define VFS_ACCESS(f) (_access((f), 0) == 0)
#else
#include <unistd.h>
#define VFS_ACCESS(f) (access((f), F_OK) == 0)
#endif

// -----------------------------------------------------------------
// Static State
// -----------------------------------------------------------------

// Mount table: VFS_MAX_MOUNTS slots, each mapping a virtual
// prefix (e.g., "A:") to a real filesystem directory path.
static VFSMount vfs_mounts[VFS_MAX_MOUNTS];

// VPATH: semicolon-separated (Windows) or colon-separated (Linux)
// list of directories to search for file reads.
static char vfs_vpath[VFS_MAX_VPATH];

// -----------------------------------------------------------------
// Internal Helpers
// -----------------------------------------------------------------

// str_icmp_n - Case-insensitive prefix comparison.
//
// Compares the first 'n' characters of strings a and b,
// ignoring case (ASCII only -- converts lowercase to uppercase
// by subtracting 32, which works for A-Z/a-z).
//
// Returns 1 if a[0..n-1] matches b[0..n-1], 0 otherwise.
//
static int str_icmp_n(const char *a, const char *b, int n)
{
    for (int i = 0; i < n; i++) {
        char ca = a[i];
        char cb = b[i];
        if (ca >= 'a' && ca <= 'z')
            ca = (char)(ca - 32);
        if (cb >= 'a' && cb <= 'z')
            cb = (char)(cb - 32);
        if (ca != cb) return 0;
    }
    return 1;
}

// has_trailing_colon - Check if string ends with ':'.
//
// Used to validate that mount prefixes follow the GW-BASIC
// drive letter convention (e.g., "A:", "USB:").
//
static int has_trailing_colon(const char *s, int len)
{
    return (len > 0 && s[len - 1] == ':');
}

// path_append - Concatenate dir + separator + file into out.
//
// Builds a filesystem path by joining a directory and filename
// with the platform-appropriate separator (\ on Windows, / on Linux).
// Only adds a separator if the directory doesn't already end with one.
//
// Parameters:
//   dir     - directory path
//   dlen    - length of dir
//   file    - filename to append
//   flen    - length of file
//   out     - output buffer
//   out_max - size of output buffer
//
// Returns:
//   0 on success
//  -1 if the combined path would exceed out_max
//
static int path_append(const char *dir, int dlen,
    const char *file, int flen,
    char *out, int out_max)
{
    int total;
    int need_sep = 0;  // C17 FIX: was 'bool need_sep' with int assignment

    // Check if dir needs a trailing separator
    if (dlen > 0) {
        char last = dir[dlen - 1];
        if (last != '/' && last != '\\')
            need_sep = 1;
    }

    total = dlen + need_sep + flen;
    if (total >= out_max) return -1;

    memcpy(out, dir, (size_t)dlen);
    if (need_sep) {
#ifdef _WIN32
        out[dlen] = '\\';
#else
        out[dlen] = '/';
#endif
    }
    memcpy(out + dlen + need_sep, file, (size_t)flen);
    out[total] = '\0';
    return 0;
}

// -----------------------------------------------------------------
// Public API
// -----------------------------------------------------------------

// vfs_init - Initialize the virtual filesystem.
//
// Clears all mount slots and the VPATH. Called once during
// interpreter boot (from boot.c).
//
void vfs_init(void)
{
    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        vfs_mounts[i].prefix[0] = '\0';
        vfs_mounts[i].target[0] = '\0';
        vfs_mounts[i].active = 0;
    }
    vfs_vpath[0] = '\0';
}

// vfs_mount - Mount a virtual prefix to a real directory.
//
// Creates or updates a mapping from a virtual prefix (e.g., "A:")
// to a real filesystem directory path. If the prefix is already
// mounted, updates the target path. If not, uses the next free slot.
//
// BASIC usage:
//   MOUNT "A:", "C:\projects\myapp"
//   MOUNT "USB:", "/media/usb0"
//
// Parameters:
//   prefix   - virtual drive prefix (must end with ':')
//   target   - real filesystem directory path
//   line_num - BASIC line number (for error reporting)
//
// Returns:
//   0 on success
//  -1 on error (invalid prefix, slot full, etc.)
//
int vfs_mount(const char *prefix, const char *target,
    int line_num)
{
    int plen, tlen;

    if (prefix == NULL || target == NULL) {
        error_raise(ERR_WHAT, line_num);
        return -1;
    }

    plen = (int)strlen(prefix);
    tlen = (int)strlen(target);

    // Prefix must end with ':' (GW-BASIC/QBasic convention)
    if (!has_trailing_colon(prefix, plen)) {
        printf("Mount prefix must end with ':' "
               "(e.g. \"A:\")\n");
        error_raise(ERR_WHAT, line_num);
        return -1;
    }

    // Validate lengths
    if (plen >= VFS_MAX_PREFIX) {
        printf("Mount prefix too long (max %d)\n",
               VFS_MAX_PREFIX - 1);
        error_raise(ERR_HOW, line_num);
        return -1;
    }
    if (tlen >= VFS_MAX_TARGET) {
        printf("Mount target path too long (max %d)\n",
               VFS_MAX_TARGET - 1);
        error_raise(ERR_HOW, line_num);
        return -1;
    }

    // Check if prefix is already mounted -- update existing
    // C17 FIX: removed shadow variable 'i' (was redeclared in inner for)
    for (int k = 0; k < VFS_MAX_MOUNTS; k++) {
        if (vfs_mounts[k].active &&
            str_icmp_n(vfs_mounts[k].prefix,
                       prefix, plen) &&
            vfs_mounts[k].prefix[plen] == '\0') {
            // Update existing mount target
            memcpy(vfs_mounts[k].target, target,
                   (size_t)(tlen + 1));
            return 0;
        }
    }

    // Find a free slot
    for (int k = 0; k < VFS_MAX_MOUNTS; k++) {
        if (!vfs_mounts[k].active) {
            memcpy(vfs_mounts[k].prefix, prefix,
                   (size_t)(plen + 1));
            memcpy(vfs_mounts[k].target, target,
                   (size_t)(tlen + 1));
            vfs_mounts[k].active = 1;
            return 0;
        }
    }

    printf("No free mount slots (max %d)\n",
           VFS_MAX_MOUNTS);
    error_raise(ERR_SORRY, line_num);
    return -1;
}

// vfs_umount - Unmount a virtual prefix.
//
// Removes the mount mapping for the given prefix. The prefix
// is matched case-insensitively.
//
// BASIC usage:
//   UMOUNT "A:"
//
// Parameters:
//   prefix   - virtual drive prefix to unmount
//   line_num - BASIC line number (for error reporting)
//
// Returns:
//   0 on success
//  -1 on error (prefix not found)
//
int vfs_umount(const char *prefix, int line_num)
{
    int plen;

    if (prefix == NULL) {
        error_raise(ERR_WHAT, line_num);
        return -1;
    }

    plen = (int)strlen(prefix);

    // C17 FIX: removed shadow variable 'i' (was redeclared in inner for)
    for (int k = 0; k < VFS_MAX_MOUNTS; k++) {
        if (vfs_mounts[k].active &&
            str_icmp_n(vfs_mounts[k].prefix,
                       prefix, plen) &&
            vfs_mounts[k].prefix[plen] == '\0') {
            vfs_mounts[k].active = 0;
            vfs_mounts[k].prefix[0] = '\0';
            vfs_mounts[k].target[0] = '\0';
            return 0;
        }
    }

    printf("Mount not found: %s\n", prefix);
    error_raise(ERR_HOW, line_num);
    return -1;
}

// vfs_list_mounts - Display all active mount points.
//
// Called by the MOUNTS command. Prints a formatted table of
// all active mount prefix-to-target mappings.
//
void vfs_list_mounts(void)
{
    int found = 0;  // C17 FIX: was 'bool found' but incremented with ++

    printf("\nVirtual Filesystem Mounts:\n");
    printf("%-12s  %s\n", "PREFIX", "TARGET");
    printf("%-12s  %s\n", "------", "------");

    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (vfs_mounts[i].active) {
            printf("%-12s  %s\n",
                   vfs_mounts[i].prefix,
                   vfs_mounts[i].target);
            found++;
        }
    }

    if (found == 0) {
        printf("(none)\n");
    }
    printf("\n");
}

// vfs_resolve - Resolve a virtual path to a real filesystem path.
//
// This is the SINGLE ENTRY POINT for all path resolution in the
// interpreter. Every file open, load, save, and directory operation
// calls this function to translate virtual paths to real paths.
//
// Resolution steps (in order):
//   1. Mount prefix check (case-insensitive).
//   2. VPATH search for reads (if file not found locally).
//   3. Passthrough (return path as-is).
//
// Parameters:
//   path      - the path to resolve (may contain virtual prefix)
//   out       - output buffer for the resolved real path
//   out_max   - size of the output buffer
//   for_write - if 1, skip VPATH search (writes go to explicit path)
//
// Returns:
//   0 on success (resolved path written to out)
//  -1 on error (buffer too small, NULL arguments)
//
int vfs_resolve(const char *path, char *out,
    int out_max, int for_write)
{
    int plen;
    if (path == NULL || out == NULL || out_max < 2) {
        if (out && out_max > 0) out[0] = '\0';
        return -1;
    }

    plen = (int)strlen(path);

    // --- Step 1: Mount prefix resolution ---
    // Check each active mount to see if the path starts with
    // its prefix (case-insensitive match).
    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        int mlen;
        if (!vfs_mounts[i].active) continue;
        mlen = (int)strlen(vfs_mounts[i].prefix);
        if (plen >= mlen &&
            str_icmp_n(path,
                       vfs_mounts[i].prefix, mlen)) {
            // Match! Replace prefix with mount target.
            const char *rest = path + mlen;
            int rlen = plen - mlen;
            int tlen = (int)strlen(vfs_mounts[i].target);

            if (rlen == 0) {
                // Just the prefix alone -> resolve to target dir
                if (tlen >= out_max) return -1;
                memcpy(out, vfs_mounts[i].target,
                       (size_t)(tlen + 1));
                return 0;
            }
            // Combine: target + separator + rest_of_path
            return path_append(
                vfs_mounts[i].target, tlen,
                rest, rlen, out, out_max);
        }
    }

    // --- Step 2: VPATH search (reads only) ---
    // For read operations, if the file doesn't exist at the
    // given path, try each directory in the VPATH search path.
    if (!for_write && vfs_vpath[0] != '\0') {
        // Only search VPATH for relative paths.
        // Skip if the path is absolute or has a drive letter.
        int is_abs = 0;  // C17 FIX: was 'bool is_abs' with int assignment
#ifdef _WIN32
        // Absolute: starts with \ or / or X:\  (drive letter)
        if (path[0] == '\\' || path[0] == '/') is_abs = 1;
        if (plen >= 3 && path[1] == ':' &&
            (path[2] == '\\' || path[2] == '/'))
            is_abs = 1;
#else
        if (path[0] == '/') is_abs = 1;
#endif

        if (!is_abs) {
            // First try the file as-is in the current directory
            if (VFS_ACCESS(path)) {
                if (plen >= out_max) return -1;
                memcpy(out, path, (size_t)(plen + 1));
                return 0;
            }

            // Search each VPATH directory
            {
                const char *vp = vfs_vpath;
                while (*vp != '\0') {
                    char dir[VFS_MAX_TARGET];
                    int dlen = 0;
                    char trial[VFS_MAX_TARGET + 260];

                    // Extract next directory from VPATH
                    while (*vp != '\0' &&
                           *vp != VFS_PATH_DELIM) {
                        if (dlen < VFS_MAX_TARGET - 1)
                            dir[dlen++] = *vp;
                        vp++;
                    }
                    dir[dlen] = '\0';
                    if (*vp == VFS_PATH_DELIM) vp++;

                    // Skip empty entries
                    if (dlen == 0) continue;

                    // Build trial path: dir/filename
                    if (path_append(dir, dlen, path, plen,
                                    trial,
                                    (int)sizeof(trial))
                        == 0) {
                        if (VFS_ACCESS(trial)) {
                            int tlen2 = (int)strlen(trial);
                            if (tlen2 >= out_max) return -1;
                            memcpy(out, trial,
                                   (size_t)(tlen2 + 1));
                            return 0;
                        }
                    }
                }
            }
        }
    }

    // --- Step 3: Passthrough ---
    // No mount matched and VPATH didn't find the file.
    // Return the path as-is -- the OS will resolve it.
    if (plen >= out_max) return -1;
    memcpy(out, path, (size_t)(plen + 1));
    return 0;
}

// vfs_set_vpath - Set the VPATH search path.
//
// BASIC usage:
//   OPTION VPATH "C:\lib;D:\stdlib"    (Windows)
//   OPTION VPATH "/usr/lib:/opt/lib"   (Linux)
//
// Set to "" or NULL to clear the VPATH.
//
void vfs_set_vpath(const char *path)
{
    if (path == NULL || path[0] == '\0') {
        vfs_vpath[0] = '\0';
        return;
    }
    {
        int len = (int)strlen(path);
        if (len >= VFS_MAX_VPATH)
            len = VFS_MAX_VPATH - 1;
        memcpy(vfs_vpath, path, (size_t)len);
        vfs_vpath[len] = '\0';
    }
}

// vfs_get_vpath - Return the current VPATH string.
//
// Returns a pointer to the static VPATH buffer. The returned
// string is semicolon-delimited on Windows, colon-delimited on Linux.
//
const char *vfs_get_vpath(void)
{
    return vfs_vpath;
}
