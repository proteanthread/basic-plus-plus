// FILENAME: seg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (seg.h, string.c)
// Provides runtime implementation for the SEG built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/seg.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_seg_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SEG$",
        .category = "String Functions",
        .syntax = "SEG$(str_expr, start_pos, end_pos)",
        .help_text = "Returns a substring from 1-based start_pos through end_pos inclusive (DEC BASIC-PLUS).",
        .error_codes = "Error 13: Type Mismatch, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BValue func_seg_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;
    (void)uname;

    if (arg_count != 3) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "SEG$ expects exactly 3 arguments (string, start_pos, end_pos)";
        return res;
    }

    if (args[0].type != VAL_STRING || !args[0].as.string ||
        args[1].type == VAL_STRING || args[2].type == VAL_STRING) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "Type mismatch in SEG$";
        return res;
    }

    const char *src = str_data(args[0].as.string);
    int len = (int)str_len(args[0].as.string);
    int p1 = (int)args[1].as.number;
    int p2 = (int)args[2].as.number;

    if (p1 < 1) p1 = 1;
    if (p2 > len) p2 = len;

    if (p1 > len || p2 < p1 || len == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    int sub_len = p2 - p1 + 1;
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), src + (p1 - 1), (size_t)sub_len);
    return res;
}
