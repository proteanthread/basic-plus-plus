// FILENAME: checkbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (checkbox.h, ctrl_common.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the CHECKBOX statement in BASIC++.

#include "statements/ui/widgets/checkbox.h"
#include "statements/ui/widgets/ctrl_common.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_checkbox_desc = {
    .name = "CHECKBOX",
    .category = "User Interface",
    .syntax = "CHECKBOX caption$ [, checked% [, col%, row%, width%]]",
    .description = "Renders a check box UI control.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_checkbox_register(void) {
    lang_desc_register(&g_checkbox_desc);
}

BppError stmt_checkbox_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char caption[128] = "Check";
    int checked = 0;

    int str_idx = -1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { str_idx = i; break; }
    }

    if (str_idx >= 0 && args[str_idx].as.string) {
        const char *s = str_data(args[str_idx].as.string);
        if (s) runtime_strncpy(caption, s, sizeof(caption) - 1);
        if (str_idx + 1 < count && args[str_idx + 1].type == VAL_NUMBER) {
            checked = (args[str_idx + 1].as.number != 0.0);
        }
    } else {
        runtime_snprintf(caption, sizeof(caption), "%g", args[count - 1].as.number);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "[%c] %s ", checked ? 'X' : ' ', caption);
        vdev_puts(vdev, buf);
    }
    return err;
}
