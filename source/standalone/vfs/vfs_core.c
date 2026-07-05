/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vfs_core.c
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
 * File: vfs_core.c
 * =====================================================================
 * Standard C implementation of virtual mounting and path resolution.
 * Completely independent of the interpreter.
 * ===================================================================== */

#include "vfs_core.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <io.h>
#define VFS_ACCESS(f) (_access((f), 0) == 0)
#else
#include <unistd.h>
#define VFS_ACCESS(f) (access((f), F_OK) == 0)
#endif

static VFSMount vfs_mounts[VFS_MAX_MOUNTS];
static char vfs_vpath[VFS_MAX_VPATH];

/* Case-insensitive prefix comparison */
static int str_icmp_n(const char *a, const char *b, int n)
{
    for (int i = 0; i < n; i++) {
        char ca = a[i];
        char cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
        if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
        if (ca != cb) return 0;
    }
    return 1;
}

/* Check if string ends with ':' */
static int has_trailing_colon(const char *s, int len)
{
    return (len > 0 && s[len - 1] == ':');
}

/* Concatenate dir + separator + file into out */
static int path_append(const char *dir, int dlen,
    const char *file, int flen,
    char *out, int out_max)
{
    int total;
    int need_sep = 0;

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

void vfs_core_init(void)
{
    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        vfs_mounts[i].prefix[0] = '\0';
        vfs_mounts[i].target[0] = '\0';
        vfs_mounts[i].active = 0;
    }
    vfs_vpath[0] = '\0';
}

int vfs_core_mount(const char *prefix, const char *target)
{
    int plen, tlen;

    if (prefix == NULL || target == NULL) {
        return -1;
    }

    plen = (int)strlen(prefix);
    tlen = (int)strlen(target);

    if (!has_trailing_colon(prefix, plen)) {
        return -1;
    }

    if (plen >= VFS_MAX_PREFIX) {
        return -1;
    }
    if (tlen >= VFS_MAX_TARGET) {
        return -1;
    }

    /* Update existing mount if prefix matches */
    for (int k = 0; k < VFS_MAX_MOUNTS; k++) {
        if (vfs_mounts[k].active &&
            str_icmp_n(vfs_mounts[k].prefix, prefix, plen) &&
            vfs_mounts[k].prefix[plen] == '\0') {
            memcpy(vfs_mounts[k].target, target, (size_t)(tlen + 1));
            return 0;
        }
    }

    /* Find a free slot */
    for (int k = 0; k < VFS_MAX_MOUNTS; k++) {
        if (!vfs_mounts[k].active) {
            memcpy(vfs_mounts[k].prefix, prefix, (size_t)(plen + 1));
            memcpy(vfs_mounts[k].target, target, (size_t)(tlen + 1));
            vfs_mounts[k].active = 1;
            return 0;
        }
    }

    return -1;
}

int vfs_core_umount(const char *prefix)
{
    int plen;

    if (prefix == NULL) {
        return -1;
    }

    plen = (int)strlen(prefix);

    for (int k = 0; k < VFS_MAX_MOUNTS; k++) {
        if (vfs_mounts[k].active &&
            str_icmp_n(vfs_mounts[k].prefix, prefix, plen) &&
            vfs_mounts[k].prefix[plen] == '\0') {
            vfs_mounts[k].active = 0;
            vfs_mounts[k].prefix[0] = '\0';
            vfs_mounts[k].target[0] = '\0';
            return 0;
        }
    }

    return -1;
}

void vfs_core_list_mounts(void (*print_cb)(const char *prefix, const char *target))
{
    if (print_cb == NULL) return;

    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (vfs_mounts[i].active) {
            print_cb(vfs_mounts[i].prefix, vfs_mounts[i].target);
        }
    }
}

int vfs_core_resolve(const char *path, char *out, int out_max, int for_write)
{
    int plen;
    if (path == NULL || out == NULL || out_max < 2) {
        if (out && out_max > 0) out[0] = '\0';
        return -1;
    }

    plen = (int)strlen(path);

    /* 1. Mount prefix resolution */
    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        int mlen;
        if (!vfs_mounts[i].active) continue;
        mlen = (int)strlen(vfs_mounts[i].prefix);
        if (plen >= mlen && str_icmp_n(path, vfs_mounts[i].prefix, mlen)) {
            const char *rest = path + mlen;
            int rlen = plen - mlen;
            int tlen = (int)strlen(vfs_mounts[i].target);

            if (rlen == 0) {
                if (tlen >= out_max) return -1;
                memcpy(out, vfs_mounts[i].target, (size_t)(tlen + 1));
                return 0;
            }
            return path_append(vfs_mounts[i].target, tlen, rest, rlen, out, out_max);
        }
    }

    /* 2. VPATH search (reads only) */
    if (!for_write && vfs_vpath[0] != '\0') {
        int is_abs = 0;
#ifdef _WIN32
        if (path[0] == '\\' || path[0] == '/') is_abs = 1;
        if (plen >= 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/'))
            is_abs = 1;
#else
        if (path[0] == '/') is_abs = 1;
#endif

        if (!is_abs) {
            if (VFS_ACCESS(path)) {
                if (plen >= out_max) return -1;
                memcpy(out, path, (size_t)(plen + 1));
                return 0;
            }

            {
                const char *vp = vfs_vpath;
                while (*vp != '\0') {
                    char dir[VFS_MAX_TARGET];
                    int dlen = 0;
                    char trial[VFS_MAX_TARGET + 260];

                    while (*vp != '\0' && *vp != VFS_PATH_DELIM) {
                        if (dlen < VFS_MAX_TARGET - 1)
                            dir[dlen++] = *vp;
                        vp++;
                    }
                    dir[dlen] = '\0';
                    if (*vp == VFS_PATH_DELIM) vp++;

                    if (dlen == 0) continue;

                    if (path_append(dir, dlen, path, plen, trial, (int)sizeof(trial)) == 0) {
                        if (VFS_ACCESS(trial)) {
                            int tlen2 = (int)strlen(trial);
                            if (tlen2 >= out_max) return -1;
                            memcpy(out, trial, (size_t)(tlen2 + 1));
                            return 0;
                        }
                    }
                }
            }
        }
    }

    /* 3. Passthrough */
    if (plen >= out_max) return -1;
    memcpy(out, path, (size_t)(plen + 1));
    return 0;
}

void vfs_core_set_vpath(const char *path)
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

const char *vfs_core_get_vpath(void)
{
    return vfs_vpath;
}
