// FILENAME: rnd.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, language_descriptor.h, string.h)
// NEEDS: libengine (math.c, rnd.h, string.c)
// Provides runtime implementation for the RND built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/random/rnd.h"
#include "runtime/language_descriptor.h"
#include "runtime/string.h"
#include "runtime/math.h"
#include "runtime/string/strops.h"

static const LangDesc g_rnd_desc = {
    .name = "RND",
    .category = "Math Functions",
    .syntax = "RND[(x)]",
    .description = "Returns a double-precision pseudo-random number in range [0.0, 1.0). If x < 0, seeds generator.",
    .error_summary = "Error 13: Type Mismatch (non-numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};
void func_rnd_register(void) {
    lang_desc_register(&g_rnd_desc);
}

static uint64_t g_rnd_seed = 123456789ULL;
static double g_last_rnd = 0.0;

void func_rnd_set_seed(uint64_t seed) {
    g_rnd_seed = (seed == 0) ? 123456789ULL : seed;
}

BValue func_rnd_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "RND") != 0) {
        return res;
    }

    if (arg_count == 0) {
        // Bare RND or RND() -> returns next pseudo-random float in [0.0, 1.0)
        g_rnd_seed = g_rnd_seed * 6364136223846793005ULL + 1442695040888963407ULL;
        double val = (double)(g_rnd_seed >> 11) / (double)(1ULL << 53);
        g_last_rnd = val;
        vm_set_last_rnd(vm, val);
        res.as.number = val;
        return res;
    }

    if (arg_count == 1) {
        if (args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER) {
            if (err) {
                err->code = 13;
                err->message = "Type mismatch for RND argument";
            }
            return res;
        }
        double arg = args[0].as.number;
        if (arg < 0.0) {
            // RND(-n): Seeds PRNG with n, and returns first deterministic float in [0.0, 1.0)
            uint64_t s = (uint64_t)(-arg);
            if (s == 0) s = 123456789ULL;
            g_rnd_seed = s * 6364136223846793005ULL + 1442695040888963407ULL;
            double val = (double)(g_rnd_seed >> 11) / (double)(1ULL << 53);
            g_last_rnd = val;
            vm_set_last_rnd(vm, val);
            res.as.number = val;
            return res;
        }
        if (arg == 0.0) {
            // RND(0): Returns most recent previously generated random number without advancing PRNG
            double last = (vm && vm_get_last_rnd(vm) != 0.0) ? vm_get_last_rnd(vm) : g_last_rnd;
            res.as.number = last;
            return res;
        }
        if (arg <= 1.0) {
            // RND(1) or RND(0.5): Returns next random float in [0.0, 1.0)
            g_rnd_seed = g_rnd_seed * 6364136223846793005ULL + 1442695040888963407ULL;
            double val = (double)(g_rnd_seed >> 11) / (double)(1ULL << 53);
            g_last_rnd = val;
            vm_set_last_rnd(vm, val);
            res.as.number = val;
            return res;
        }
        // arg > 1.0
        g_rnd_seed = g_rnd_seed * 6364136223846793005ULL + 1442695040888963407ULL;
        double val = (double)(g_rnd_seed >> 11) / (double)(1ULL << 53);
        g_last_rnd = val;
        vm_set_last_rnd(vm, val);

        if (arg == runtime_floor(arg)) {
            // Parenthesized RND(N) for integer N > 1 returns integer in [1, N] inclusive (BBC BASIC / retro standard)
            int64_t n_int = (int64_t)arg;
            int64_t rand_int = 1 + (int64_t)(val * (double)n_int);
            if (rand_int > n_int) rand_int = n_int;
            res.as.number = (double)rand_int;
        } else {
            // Float > 1.0: returns float in [0.0, arg)
            res.as.number = val * arg;
        }
        return res;
    }

    if (arg_count >= 2) {
        if ((args[0].type != VAL_NUMBER && args[0].type != VAL_INTEGER) ||
            (args[1].type != VAL_NUMBER && args[1].type != VAL_INTEGER)) {
            if (err) {
                err->code = 13;
                err->message = "Type mismatch for RND range arguments";
            }
            return res;
        }
        double min_v = args[0].as.number;
        double max_v = args[1].as.number;

        g_rnd_seed = g_rnd_seed * 6364136223846793005ULL + 1442695040888963407ULL;
        double val = (double)(g_rnd_seed >> 11) / (double)(1ULL << 53);
        g_last_rnd = val;
        vm_set_last_rnd(vm, val);

        double low = (min_v < max_v) ? min_v : max_v;
        double high = (min_v < max_v) ? max_v : min_v;

        if (low == runtime_floor(low) && high == runtime_floor(high)) {
            int64_t low_i = (int64_t)low;
            int64_t high_i = (int64_t)high;
            int64_t span = high_i - low_i + 1;
            int64_t r_i = low_i + (int64_t)(val * (double)span);
            if (r_i > high_i) r_i = high_i;
            res.as.number = (double)r_i;
        } else {
            res.as.number = low + val * (high - low);
        }
        return res;
    }

    return res;
}

