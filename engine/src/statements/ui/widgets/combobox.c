// FILENAME: combobox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (combobox.h, ctrl_common.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the COMBOBOX statement in BASIC++.

#include "statements/ui/widgets/combobox.h"
#include "statements/ui/widgets/ctrl_common.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_combobox_desc = {
    .name = "COMBOBOX",
    .category = "User Interface",
    .syntax = "COMBOBOX items$ [, sel% [, col%, row%, width%]]",
    .description = "Renders a dropdown combo box UI control.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_combobox_register(void) {
    lang_desc_register(&g_combobox_desc);
}

BppError stmt_combobox_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char items[128] = "Item 1,Item 2";
    int str_idx = count - 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { str_idx = i; break; }
    }
    if (args[str_idx].type == VAL_STRING && args[str_idx].as.string) {
        const char *s = str_data(args[str_idx].as.string);
        if (s) runtime_strncpy(items, s, sizeof(items) - 1);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "[ %s | v ] ", items);
        vdev_puts(vdev, buf);
    }
    return err;
}
