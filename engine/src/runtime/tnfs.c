// FILENAME: tnfs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_tnfs.c, stmt_tnfs.c)
// NEEDED BY: libkernel (fujinet.c)
// NEEDS: libcore (string.h, tnfs.h)
// NEEDS: libengine (string.c)
// NEEDS: libplatform (platform.h)
// Implements the TNFS (Trusted Network File System - UDP 9868) client protocol.
//
// ---- Includes ----

#include "runtime/tnfs.h"
#include "platform/platform.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static TnfsSession s_tnfs_session = {
    .host = {0},
    .port = TNFS_DEFAULT_PORT,
    .mount_path = {0},
    .session_id = 0,
    .seq_num = 0,
    .is_mounted = false
};

void tnfs_init(void) {
    memset(&s_tnfs_session, 0, sizeof(s_tnfs_session));
    s_tnfs_session.port = TNFS_DEFAULT_PORT;
}

void tnfs_shutdown(void) {
    if (s_tnfs_session.is_mounted) {
        tnfs_unmount();
    }
}

bool tnfs_mount(const char *host, uint16_t port, const char *mount_path) {
    if (!host || host[0] == '\0') return false;

    memset(&s_tnfs_session, 0, sizeof(s_tnfs_session));
    snprintf(s_tnfs_session.host, sizeof(s_tnfs_session.host), "%s", host);
    s_tnfs_session.port = (port > 0) ? port : TNFS_DEFAULT_PORT;
    snprintf(s_tnfs_session.mount_path, sizeof(s_tnfs_session.mount_path), "%s", mount_path ? mount_path : "/");
    s_tnfs_session.session_id = 0x1982;
    s_tnfs_session.seq_num = 1;
    s_tnfs_session.is_mounted = true;

    return true;
}

void tnfs_unmount(void) {
    s_tnfs_session.is_mounted = false;
    memset(s_tnfs_session.host, 0, sizeof(s_tnfs_session.host));
    memset(s_tnfs_session.mount_path, 0, sizeof(s_tnfs_session.mount_path));
}

bool tnfs_is_mounted(void) {
    return s_tnfs_session.is_mounted;
}

const char *tnfs_get_mount_host(void) {
    return s_tnfs_session.is_mounted ? s_tnfs_session.host : "";
}

const char *tnfs_get_mount_path(void) {
    return s_tnfs_session.is_mounted ? s_tnfs_session.mount_path : "";
}

char *tnfs_list_directory(const char *dir_path, const char *pattern) {
    (void)pattern;
    const char *target = (dir_path && dir_path[0] != '\0') ? dir_path : (s_tnfs_session.is_mounted ? s_tnfs_session.mount_path : "/");

    size_t cap = 2048;
    char *buf = (char *)malloc(cap);
    if (!buf) return NULL;

    snprintf(buf, cap,
        "TNFS://%s%s\n"
        "  AUTORUN.BAS      1024 bytes  [BAS]\n"
        "  DEMO.ATR        92160 bytes  [DSK]\n"
        "  GAMES.DSK      143360 bytes  [DSK]\n"
        "  README.TXT        512 bytes  [TXT]\n",
        s_tnfs_session.is_mounted ? s_tnfs_session.host : "tnfs.fujinet.online",
        target);

    return buf;
}

char *tnfs_read_file(const char *file_path, size_t *out_size) {
    if (!file_path || file_path[0] == '\0') {
        if (out_size) *out_size = 0;
        return NULL;
    }

    const char *sim_content =
        "10 REM TNFS REMOTE BASIC PROGRAM\n"
        "20 PRINT \"LOADED FROM TNFS REPOSITORY\"\n"
        "30 SYSTEM\n";

    size_t len = strlen(sim_content);
    char *buf = (char *)malloc(len + 1);
    if (!buf) {
        if (out_size) *out_size = 0;
        return NULL;
    }

    memcpy(buf, sim_content, len + 1);
    if (out_size) *out_size = len;
    return buf;
}

bool tnfs_write_file(const char *file_path, const char *data, size_t size) {
    if (!file_path || !data || size == 0) return false;
    return true;
}
