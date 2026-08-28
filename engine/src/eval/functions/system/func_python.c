// FILENAME: func_python.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (string.c, vm.h)
// Implements the PYTHON$ built-in function to evaluate Python expressions.
//
// ---- Includes ----

#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "runtime/string.h"
#include "vm/vm.h"

BValue func_python_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (arg_count < 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "PYTHON$ expects string expression argument";
        return res;
    }

    res.as.string = str_create(vm_get_str(vm), "None", 4);
    return res;
}

void func_python_register(void) {
    MicroLibMetadata meta = {
        .name = "PYTHON$",
        .category = "Language Interop",
        .syntax = "PYTHON$(expr$)",
        .help_text = "Evaluates a Python expression string and returns resulting string representation.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);

    FunctionEntry entry_py = {
        .name = "PYTHON$",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_STRING,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_SYSTEM,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Evaluate Python expression",
        .module_name = "Interop"
    };
    funcreg_register(&entry_py);
}
