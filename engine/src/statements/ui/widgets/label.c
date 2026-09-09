// FILENAME: label.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, label.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the LABEL statement in BASIC++.

#include "statements/ui/widgets/label.h"
#include "statements/ui/widgets/ctrl_common.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_label_desc = {
    .name = "LABEL",
    .category = "User Interface",
    .syntax = "LABEL text$ [, col%, row%, width%]",
    .description = "Renders a static text label UI control.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_label_register(void) {
    lang_desc_register(&g_label_desc);
}

BppError stmt_label_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char text[128] = "Label";
    int txt_idx = count - 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { txt_idx = i; break; }
    }
    if (args[txt_idx].type == VAL_STRING && args[txt_idx].as.string) {
        const char *s = str_data(args[txt_idx].as.string);
        if (s) runtime_strncpy(text, s, sizeof(text) - 1);
    } else {
        runtime_snprintf(text, sizeof(text), "%g", args[txt_idx].as.number);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "%s ", text);
        vdev_puts(vdev, buf);
    }
    return err;
}
