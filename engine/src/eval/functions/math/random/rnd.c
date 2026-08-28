// FILENAME: rnd.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, rnd.h, string.c)
// Provides runtime implementation for the RND built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/random/rnd.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"
#include "runtime/math.h"
void func_rnd_register(void) {
    MicroLibMetadata meta = {
        .name = "RND",
        .category = "Math Functions",
        .syntax = "RND[(x)]",
        .help_text = "Returns a double-precision pseudo-random number in range [0.0, 1.0). If x < 0, seeds generator.",
        .error_codes = "Error 13: Type Mismatch (non-numeric argument)"
    };
    microlib_register(&meta);
}

static uint64_t g_rnd_seed = 123456789ULL;

BValue func_rnd_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)err;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "RND") != 0) {
        return res;
    }

    res.type = VAL_NUMBER;
    double arg = 1.0;
    if (arg_count > 0 && args[0].type == VAL_NUMBER) {
        arg = args[0].as.number;
    }
    if (arg < 0.0) {
        g_rnd_seed = (uint64_t)(-arg);
        if (g_rnd_seed == 0) g_rnd_seed = 123456789ULL;
        g_rnd_seed = g_rnd_seed * 6364136223846793005ULL + 1442695040888963407ULL;
        double val = (double)(g_rnd_seed >> 11) / (double)(1ULL << 53);
        vm_set_last_rnd(vm, val);
        res.as.number = val;
    } else if (arg == 0.0) {
        res.as.number = vm_get_last_rnd(vm);
    } else {
        g_rnd_seed = g_rnd_seed * 6364136223846793005ULL + 1442695040888963407ULL;
        double val = (double)(g_rnd_seed >> 11) / (double)(1ULL << 53);
        vm_set_last_rnd(vm, val);
        res.as.number = val;
    }
    return res;
}

