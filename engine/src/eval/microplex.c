// FILENAME: microplex.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (microplex.h, strings.h, strings.c)
// NEEDS: libengine (eval_internal.h, vm.h)
// Provides core logic and interface definitions for microplex within BASIC++.
//
// ---- Includes ----

#include "eval/eval_internal.h"
#include "runtime/microplex.h"
#include "runtime/strings.h"
#include "vm/vm.h"

BValue func_microplex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "MICROPLEX$") != 0 && runtime_strcmp(uname, "MICROPLEX") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        err->code = 13;
        err->message = "MICROPLEX$ requires two string arguments";
        return res;
    }

    const char *s1 = args[0].as.string ? str_data(args[0].as.string) : "";
    size_t len1 = args[0].as.string ? str_len(args[0].as.string) : 0;

    const char *s2 = args[1].as.string ? str_data(args[1].as.string) : "";
    size_t len2 = args[1].as.string ? str_len(args[1].as.string) : 0;

    size_t out_cap = len1 + len2 + 1;
    HalContext *hal = hal_get();
    char *out_buf = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(out_cap) : NULL;
    if (!out_buf) {
        err->code = 7;
        err->message = "Out of memory in MICROPLEX$";
        return res;
    }

    size_t out_len = microplex_string(s1, len1, s2, len2, out_buf, out_cap);
    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), out_buf, out_len);
    if (hal && hal->mem.free) hal->mem.free(out_buf);

    return res;
}

