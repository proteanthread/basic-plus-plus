// FILENAME: filelistbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, filelistbox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the FILELISTBOX statement in BASIC++.

#include "statements/ui/widgets/filelistbox.h"
#include "statements/ui/widgets/ctrl_common.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_filelistbox_desc = {
    .name = "FILELISTBOX",
    .category = "User Interface",
    .syntax = "FILELISTBOX filter$, col%, row%, width%, height%",
    .description = "Renders a file list box UI control filtered by wildcard pattern.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_filelistbox_register(void) {
    lang_desc_register(&g_filelistbox_desc);
}

BppError stmt_filelistbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0) return err;

    char filter[128] = "*.*";
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            const char *s = str_data(args[i].as.string);
            if (s) runtime_strncpy(filter, s, sizeof(filter) - 1);
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
        runtime_snprintf(buf, sizeof(buf), "[FILES: %s] ", filter);
        vdev_puts(vdev, buf);
    }
    return err;
}
