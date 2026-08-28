// FILENAME: index_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (index_fn.h, string.c)
// Provides runtime implementation for the INDEX_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/search/index_fn.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_index_fn_register(void) {
    static const MicroLibMetadata meta = {
        .name = "INDEX",
        .category = "String Functions",
        .syntax = "INDEX(str_expr, search_expr [, start_pos])",
        .help_text = "Returns the 1-based character position of search_expr in str_expr (Univac 1100 / DTSS).",
        .error_codes = "Error 13: Type Mismatch, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BValue func_index_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;
    (void)vm; (void)uname;

    if (arg_count < 2 || arg_count > 3) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "INDEX expects 2 or 3 arguments (string, search_str [, start_pos])";
        return res;
    }

    if (args[0].type != VAL_STRING || !args[0].as.string ||
        args[1].type != VAL_STRING || !args[1].as.string) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "Type mismatch in INDEX: expected string arguments";
        return res;
    }

    const char *src = str_data(args[0].as.string);
    size_t src_len = str_len(args[0].as.string);
    const char *target = str_data(args[1].as.string);
    size_t target_len = str_len(args[1].as.string);

    int start_pos = 1;
    if (arg_count == 3) {
        if (args[2].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            return res;
        }
        start_pos = (int)args[2].as.number;
    }

    if (target_len == 0) {
        res.as.number = (start_pos <= (int)src_len + 1) ? (double)start_pos : 0.0;
        return res;
    }

    if (start_pos < 1 || (size_t)start_pos > src_len || target_len > src_len) {
        res.as.number = 0.0;
        return res;
    }

    const char *match = runtime_strstr(src + (start_pos - 1), target);
    if (match) {
        res.as.number = (double)(match - src + 1);
    } else {
        res.as.number = 0.0;
    }

    return res;
}
