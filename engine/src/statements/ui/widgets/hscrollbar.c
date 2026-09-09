// FILENAME: hscrollbar.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, hscrollbar.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the HSCROLLBAR statement in BASIC++.

#include "statements/ui/widgets/hscrollbar.h"
#include "statements/ui/widgets/ctrl_common.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "runtime/strings.h"

static const LangDesc g_hscrollbar_desc = {
    .name = "HSCROLLBAR",
    .category = "User Interface",
    .syntax = "HSCROLLBAR min%, max%, val%, col%, row%, width%",
    .description = "Renders a horizontal scroll bar UI control.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_hscrollbar_register(void) {
    lang_desc_register(&g_hscrollbar_desc);
}

BppError stmt_hscrollbar_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[8];
    int count = parse_ctrl_args(vm, lex, args, 8, &err);
    if (err.code != 0 || count == 0) return err;

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        vdev_puts(vdev, "[<---#------->] ");
    }
    return err;
}
