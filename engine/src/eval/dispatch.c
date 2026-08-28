// FILENAME: dispatch.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (dispatch_internal.h)
// Provides core logic and interface definitions for dispatch within BASIC++.
//
// ---- Includes ----

#include "eval/dispatch_internal.h"

//
// ---- File Reading Helper ----

char *eval_read_file_to_string(const char *path) {
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_open) return NULL;
    IoHandle handle = hal->io.file_open(path, "rb");
    if (handle == IO_HANDLE_INVALID) return NULL;

    hal->io.file_seek(handle, 0, IO_SEEK_END);
    int64_t size = hal->io.file_tell(handle);
    if (size < 0) {
        hal->io.file_close(handle);
        return NULL;
    }
    hal->io.file_seek(handle, 0, IO_SEEK_SET);
    char *buf = (char *)(hal->mem.alloc ? hal->mem.alloc((size_t)size + 1) : NULL);
    if (!buf) {
        hal->io.file_close(handle);
        return NULL;
    }
    size_t read_bytes = hal->io.file_read(handle, buf, 1, (size_t)size);
    buf[read_bytes] = '\0';
    hal->io.file_close(handle);
    return buf;
}

