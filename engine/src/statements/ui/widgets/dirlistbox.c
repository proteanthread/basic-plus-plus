// FILENAME: dirlistbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, dirlistbox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the DIRLISTBOX statement in BASIC++.

#include "statements/ui/widgets/dirlistbox.h"
#include "statements/ui/widgets/ctrl_common.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_dirlistbox_desc = {
    .name = "DIRLISTBOX",
    .category = "User Interface",
    .syntax = "DIRLISTBOX path$, col%, row%, width%, height%",
    .description = "Renders a directory tree list box UI control.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_dirlistbox_register(void) {
    lang_desc_register(&g_dirlistbox_desc);
}

BppError stmt_dirlistbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0) return err;

    char path[128] = "C:\\BASIC";
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            const char *s = str_data(args[i].as.string);
            if (s) runtime_strncpy(path, s, sizeof(path) - 1);
            break;
        }
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "[DIR: %s] ", path);
        vdev_puts(vdev, buf);
    }
    return err;
}
