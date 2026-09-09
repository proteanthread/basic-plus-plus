// FILENAME: inputbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, conversion_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (inputbox.h)
// Provides runtime implementation for the INPUTBOX$ function in BASIC++.

#include "eval/functions/ui/dialogs/inputbox.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static const LangDesc g_inputbox_desc = {
    .name = "INPUTBOX$",
    .category = "User Interface",
    .syntax = "res$ = INPUTBOX$(prompt$ [, title$ [, default$]])",
    .description = "Displays a prompt in a dialog box and returns the text entered by the user.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_inputbox_register(void) {
    lang_desc_register(&g_inputbox_desc);
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
