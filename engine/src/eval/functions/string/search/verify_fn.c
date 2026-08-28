// FILENAME: verify_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, verify_fn.h)
// Provides runtime implementation for the VERIFY_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/search/verify_fn.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_verify_register(void) {
    static const MicroLibMetadata meta = {
        .name = "VERIFY",
        .category = "String Functions",
        .syntax = "VERIFY(target$, charset$ [, start%])",
        .help_text = "Finds first character in target$ that is not present in charset$. Returns 1-based index or 0.",
        .error_codes = "Error 13: Type Mismatch, Error 5: Illegal function call"
    };
    microlib_register(&meta);
}

BValue func_verify_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "VERIFY") != 0) {
        return res;
    }

    if (arg_count < 2 || arg_count > 3) {
        err->code = 5; err->message = "VERIFY expects 2 or 3 arguments";
        return res;
    }

    if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        err->code = 13; err->message = "Type mismatch in VERIFY";
        return res;
    }

    const char *target = args[0].as.string ? str_data(args[0].as.string) : "";
    const char *charset = args[1].as.string ? str_data(args[1].as.string) : "";
    int tlen = (int)runtime_strlen(target);
    int start = 1;

    if (arg_count == 3) {
        if (args[2].type != VAL_NUMBER) {
            err->code = 13; err->message = "Type mismatch in VERIFY start position";
            return res;
        }
        start = (int)args[2].as.number;
        if (start < 1) start = 1;
    }

    for (int i = start - 1; i < tlen; ++i) {
        char c = target[i];
        if (runtime_strchr(charset, c) == NULL) {
            res.as.number = (double)(i + 1);
            return res;
        }
    }

    res.as.number = 0.0;
    return res;
}
