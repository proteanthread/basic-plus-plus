// FILENAME: frame.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, frame.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the FRAME statement in BASIC++.

#include "statements/ui/widgets/frame.h"
#include "statements/ui/widgets/ctrl_common.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"

static const LangDesc g_frame_desc = {
    .name = "FRAME",
    .category = "User Interface",
    .syntax = "FRAME title$ [, col%, row%, width%, height%]",
    .description = "Renders a framed container box UI control with title.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_frame_register(void) {
    lang_desc_register(&g_frame_desc);
}

BppError stmt_frame_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0 || count == 0) return err;

    char title[128] = "Frame";
    int str_idx = count - 1;
    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING) { str_idx = i; break; }
    }
    if (args[str_idx].type == VAL_STRING && args[str_idx].as.string) {
        const char *s = str_data(args[str_idx].as.string);
        if (s) runtime_strncpy(title, s, sizeof(title) - 1);
    }

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "+--- %s ---+ ", title);
        vdev_puts(vdev, buf);
    }
    return err;
}
