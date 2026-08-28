// FILENAME: vbdos_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the VBDOS_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/ui/dialogs/vbdos_fn.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"
#include "runtime/memory.h"

void func_vbdos_register(void) {
    MicroLibMetadata meta1 = {
        .name = "MSGBOX",
        .category = "User Interface",
        .syntax = "result% = MSGBOX(prompt$ [, buttons% [, title$]])",
        .help_text = "Displays a message in a dialog box and returns an integer indicating which button was clicked.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta1);

    MicroLibMetadata meta2 = {
        .name = "INPUTBOX$",
        .category = "User Interface",
        .syntax = "res$ = INPUTBOX$(prompt$ [, title$ [, default$]])",
        .help_text = "Displays a prompt in a dialog box and returns the text entered by the user.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta2);

    MicroLibMetadata meta3 = {
        .name = "DOEVENTS",
        .category = "Control",
        .syntax = "numForms% = DOEVENTS()",
        .help_text = "Yields execution so the operating system can process event queue messages.",
        .error_codes = "None"
    };
    microlib_register(&meta3);
}

BValue func_msgbox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = 1.0; // vbOK = 1

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

BValue func_inputbox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;

    if (arg_count < 1) {
        if (err) {
            err->code = 13;
            err->message = "INPUTBOX$ expects at least 1 argument (prompt)";
        }
        res.type = VAL_NONE;
        return res;
    }

    char def_buf[512] = {0};
    if (arg_count >= 3 && args[2].type == VAL_STRING) {
        const char *s = str_data(args[2].as.string);
        if (s) runtime_strncpy(def_buf, s, sizeof(def_buf) - 1);
    }

    res.as.string = str_create(vm_get_str(vm), def_buf, runtime_strlen(def_buf));
    return res;
}

BValue func_doevents_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)args;
    (void)arg_count;
    (void)err;

    // Yield timeslice to OS to process event queue
    platform_sleep_ms(0);

    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    res.as.number = 0.0; // Returns count of open forms
    return res;
}

