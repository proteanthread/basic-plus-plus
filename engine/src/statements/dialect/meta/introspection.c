// FILENAME: introspection.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (stmt.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the INTROSPECTION statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "platform/platform.h"
#include "security/security.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/memops.h"

BppError stmt_hostname_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)lex;

    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    char name[256] = "";
    platform_get_hostname(name, sizeof(name));

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "%s\n", name);
    return err;
}

BppError stmt_username_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)lex;

    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70; err.message = "Permission denied";
        return err;
    }

    char name[256] = "";
    platform_get_username(name, sizeof(name));

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_printf(vdev, "%s\n", name);
    return err;
}

void stmt_introspection_register(void) {
    static const MicroLibMetadata meta_hostname = {
        .name = "HOSTNAME",
        .category = "Introspection",
        .syntax = "HOSTNAME",
        .help_text = "Outputs the system hostname to console.",
        .error_codes = "Error 70: Permission Denied"
    };
    static const MicroLibMetadata meta_username = {
        .name = "USERNAME",
        .category = "Introspection",
        .syntax = "USERNAME",
        .help_text = "Outputs the current user identity name to console.",
        .error_codes = "Error 70: Permission Denied"
    };
    microlib_register(&meta_hostname);
    microlib_register(&meta_username);
}
