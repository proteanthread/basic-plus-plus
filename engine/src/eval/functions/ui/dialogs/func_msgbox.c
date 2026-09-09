// FILENAME: func_msgbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_msgbox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the MSGBOX function in BASIC++.

#include "eval/functions/ui/dialogs/func_msgbox.h"
#include "runtime/language_descriptor.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_func_msgbox_desc = {
    .name = "MSGBOX",
    .category = "User Interface",
    .syntax = "result% = MSGBOX(prompt$ [, buttons% [, title$]])",
    .description = "Displays a message in a dialog box and returns an integer indicating which button was clicked.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_msgbox_register(void) {
    lang_desc_register(&g_func_msgbox_desc);
}

BValue func_msgbox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = 1.0;

    if (arg_count < 1) {
        if (err) {
            err->code = 13;
            err->message = "MSGBOX expects at least 1 argument (prompt)";
        }
        res.type = VAL_NONE;
        return res;
    }

    char prompt_buf[1024] = {0};
    if (args[0].type == VAL_STRING) {
        const char *s = str_data(args[0].as.string);
        if (s) runtime_strncpy(prompt_buf, s, sizeof(prompt_buf) - 1);
    } else if (args[0].type == VAL_NUMBER) {
        runtime_snprintf(prompt_buf, sizeof(prompt_buf), "%g", args[0].as.number);
    }

    int buttons = 0;
    if (arg_count >= 2 && args[1].type == VAL_NUMBER) {
        buttons = (int)args[1].as.number;
    }

    char title_buf[256] = "Visual Basic";
    if (arg_count >= 3 && args[2].type == VAL_STRING) {
        const char *s = str_data(args[2].as.string);
        if (s) runtime_strncpy(title_buf, s, sizeof(title_buf) - 1);
    }

    (void)buttons;
    VDevContext *vdev_ctx = vm_get_vdev(vm);
    if (vdev_ctx) {
        char out[1536];
        runtime_snprintf(out, sizeof(out), "[ %s ]\n%s\n", title_buf, prompt_buf);
        vdev_puts(vdev_ctx, out);
    }

    return res;
}
