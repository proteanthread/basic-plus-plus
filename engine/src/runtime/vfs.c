// FILENAME: vfs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libcore (error.c, spec.c)
// NEEDED BY: libengine (context.c, control.c, data.c, events_internal.h)
// NEEDED BY: libengine (exec_internal.h, vm_internal.h)
// NEEDS: libcore (hal.h, memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c, vfs.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for vfs within BASIC++.
//
// ---- Includes ----

#include "runtime/vfs.h"
#include "device/vdev.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "platform/platform.h"

static void safe_strncpy(char *dest, const char *src, size_t max_len) {
    if (!dest || max_len == 0) return;
    if (!src) {
        dest[0] = '\0';
        return;
    }
    size_t len = runtime_strlen(src);
    if (len >= max_len) {
        len = max_len - 1;
    }
    runtime_memcpy(dest, src, len);
    dest[len] = '\0';
}

struct VfsContext {
    MemoryContext *mem;
    BppMountPoint  mounts[VFS_MAX_MOUNTS];
    char           search_path[VFS_MAX_PATH];
    char           working_path[VFS_MAX_PATH];
    char           data_path[VFS_MAX_PATH];
    char           exec_path[VFS_MAX_PATH];
    char           program_path[VFS_MAX_PATH];
};

VfsContext *vfs_init(MemoryContext *mem) {
    HalContext *hal = hal_get();
    VfsContext *ctx = (VfsContext *)(hal && hal->mem.alloc ? hal->mem.alloc(sizeof(VfsContext)) : NULL);
    if (!ctx) return NULL;
    runtime_memset(ctx, 0, sizeof(VfsContext));
    ctx->mem = mem;

    for (int i = 0; i < VFS_MAX_MOUNTS; ++i) {
        ctx->mounts[i].active = false;
        ctx->mounts[i].prefix[0] = '\0';
        ctx->mounts[i].target[0] = '\0';
    }
    ctx->search_path[0] = '\0';

    extern char *platform_getcwd(char *buf, size_t size);
    char cwd[VFS_MAX_PATH] = "";
    if (platform_getcwd(cwd, sizeof(cwd))) {
        safe_strncpy(ctx->working_path, cwd, VFS_MAX_PATH);
        safe_strncpy(ctx->data_path, cwd, VFS_MAX_PATH);
    } else {
        ctx->working_path[0] = '\0';
        ctx->data_path[0] = '\0';
    }
    ctx->exec_path[0] = '\0';
    ctx->program_path[0] = '\0';

    return ctx;
}

void vfs_shutdown(VfsContext *ctx) {
    if (ctx) {
        HalContext *hal = hal_get();
        if (hal && hal->mem.free) hal->mem.free(ctx);
    }
}


bool vfs_mount(VfsContext *ctx, const char *prefix, const char *target, BppMountType type) {
    if (!ctx || !prefix || !target) return false;

    // Normalize prefix (should end with colon)
    char norm_prefix[VFS_MAX_PREFIX];
    size_t plen = runtime_strlen(prefix);
    if (plen == 0 || plen >= VFS_MAX_PREFIX) return false;
    runtime_strcpy(norm_prefix, prefix);
    if (norm_prefix[plen - 1] != ':') {
        if (plen + 1 >= VFS_MAX_PREFIX) return false;
        norm_prefix[plen] = ':';
        norm_prefix[plen + 1] = '\0';
    }

    // Check if already mounted
    for (int i = 0; i < VFS_MAX_MOUNTS; ++i) {
        if (ctx->mounts[i].active && runtime_strcasecmp(ctx->mounts[i].prefix, norm_prefix) == 0) {
            // Overwrite existing mount target
            safe_strncpy(ctx->mounts[i].target, target, VFS_MAX_PATH);
            ctx->mounts[i].type = type;
            return true;
        }
    }

    // Find free slot
    for (int i = 0; i < VFS_MAX_MOUNTS; ++i) {
        if (!ctx->mounts[i].active) {
            safe_strncpy(ctx->mounts[i].prefix, norm_prefix, VFS_MAX_PREFIX);
            safe_strncpy(ctx->mounts[i].target, target, VFS_MAX_PATH);
            ctx->mounts[i].type = type;
            ctx->mounts[i].active = true;
            return true;
        }
    }

    return false;
}

bool vfs_umount(VfsContext *ctx, const char *prefix) {
    if (!ctx || !prefix) return false;

    char norm_prefix[VFS_MAX_PREFIX];
    size_t plen = runtime_strlen(prefix);
    if (plen == 0 || plen >= VFS_MAX_PREFIX) return false;
    runtime_strcpy(norm_prefix, prefix);
    if (norm_prefix[plen - 1] != ':') {
        if (plen + 1 >= VFS_MAX_PREFIX) return false;
        norm_prefix[plen] = ':';
        norm_prefix[plen + 1] = '\0';
    }

    for (int i = 0; i < VFS_MAX_MOUNTS; ++i) {
        if (ctx->mounts[i].active && runtime_strcasecmp(ctx->mounts[i].prefix, norm_prefix) == 0) {
            ctx->mounts[i].active = false;
            return true;
        }
    }
    return false;
}

bool vfs_resolve(VfsContext *ctx, const char *virtual_path, char *resolved_path, size_t max_len) {
    if (!virtual_path || !resolved_path || max_len == 0) return false;

    if (!ctx) {
        safe_strncpy(resolved_path, virtual_path, max_len);
        return true;
    }

    // Find matching mount prefix
    for (int i = 0; i < VFS_MAX_MOUNTS; ++i) {
        if (ctx->mounts[i].active) {
            size_t plen = runtime_strlen(ctx->mounts[i].prefix);
            if (runtime_strncasecmp(virtual_path, ctx->mounts[i].prefix, plen) == 0) {
                // Translate path
                const char *subpath = virtual_path + plen;
                // Strip leading slash/backslash from subpath to prevent absolute path issues
                while (*subpath == '/' || *subpath == '\\') {
                    subpath++;
                }

                runtime_snprintf(resolved_path, max_len, "%s/%s", ctx->mounts[i].target, subpath);
                return true;
            }
        }
    }

    // Fallback to passthrough
    safe_strncpy(resolved_path, virtual_path, max_len);
    return true;
}

void vfs_list_mounts(VfsContext *ctx, VDevContext *vdev) {
    if (!ctx) return;
    vdev_printf(vdev, "%-10s %-12s %s\n", "Drive", "Type", "Physical Target");
    vdev_printf(vdev, "%-10s %-12s %s\n", "-----", "----", "---------------");
    for (int i = 0; i < VFS_MAX_MOUNTS; ++i) {
        if (ctx->mounts[i].active) {
            const char *tname = "Directory";
            if (ctx->mounts[i].type == MNT_ZIP) tname = "ZIP Archive";
            if (ctx->mounts[i].type == MNT_DISK) tname = "Virtual Disk";
            vdev_printf(vdev, "%-10s %-12s %s\n", ctx->mounts[i].prefix, tname, ctx->mounts[i].target);
        }
    }
}

const char *vfs_get_search_path(VfsContext *ctx) {
    return ctx ? ctx->search_path : "";
}

void vfs_set_search_path(VfsContext *ctx, const char *path) {
    if (ctx && path) {
        safe_strncpy(ctx->search_path, path, VFS_MAX_PATH);
    }
}

const char *vfs_get_category_path(VfsContext *ctx, const char *category) {
    if (!ctx || !category) return "";
    if (runtime_strcasecmp(category, "WORKING") == 0) {
        char cwd[VFS_MAX_PATH];
        extern char *platform_getcwd(char *buf, size_t size);
        if (platform_getcwd(cwd, sizeof(cwd))) {
            safe_strncpy(ctx->working_path, cwd, VFS_MAX_PATH);
        }
        return ctx->working_path;
    }
    if (runtime_strcasecmp(category, "DATA") == 0) {
        return ctx->data_path;
    }
    if (runtime_strcasecmp(category, "EXEC") == 0 || runtime_strcasecmp(category, "PROGRAM") == 0) {
        return ctx->exec_path;
    }
    return "";
}

void vfs_set_category_path(VfsContext *ctx, const char *category, const char *path) {
    if (!ctx || !category || !path) return;
    if (runtime_strcasecmp(category, "WORKING") == 0) {
        safe_strncpy(ctx->working_path, path, VFS_MAX_PATH);
    } else if (runtime_strcasecmp(category, "DATA") == 0) {
        safe_strncpy(ctx->data_path, path, VFS_MAX_PATH);
    } else if (runtime_strcasecmp(category, "EXEC") == 0 || runtime_strcasecmp(category, "PROGRAM") == 0) {
        safe_strncpy(ctx->exec_path, path, VFS_MAX_PATH);
        safe_strncpy(ctx->program_path, path, VFS_MAX_PATH);
    }
}

