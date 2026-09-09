// FILENAME: timercontrol.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, timercontrol.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the TIMERCONTROL statement in BASIC++.

#include "statements/ui/widgets/timercontrol.h"
#include "statements/ui/widgets/ctrl_common.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_timercontrol_desc = {
    .name = "TIMERCONTROL",
    .category = "User Interface",
    .syntax = "TIMERCONTROL interval_ms%, enabled%",
    .description = "Configures an interactive background timer event control.",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_timercontrol_register(void) {
    lang_desc_register(&g_timercontrol_desc);
}

BppError stmt_timercontrol_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue args[6];
    int count = parse_ctrl_args(vm, lex, args, 6, &err);
    if (err.code != 0) return err;

    int interval = 1000;
    int enabled = 1;
    if (count >= 1 && args[0].type == VAL_NUMBER) interval = (int)args[0].as.number;
    if (count >= 2 && args[1].type == VAL_NUMBER) enabled = (args[1].as.number != 0.0);

    for (int i = 0; i < count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            str_release(vm_get_str(vm), args[i].as.string);
        }
    }

    VDevContext *vdev = vm_get_vdev(vm);
    if (vdev) {
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "[TIMER: %dms %s] ", interval, enabled ? "ON" : "OFF");
        vdev_puts(vdev, buf);
    }
    return err;
}
