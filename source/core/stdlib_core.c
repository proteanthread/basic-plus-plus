 // ---
 // BASIC++ Interpreter - stdlib_core.c
 // ---
 //
 // Core Immutable API (VM Contract Layer)
 // These functions cannot be overridden by dialects unless security is in OPEN mode.
 //
//
// HOW TO EXTEND:
//   See the preamble comments in related files for
//   customization and extension instructions.
//
// TROUBLESHOOTING:
//   Check error_occurred() after operations that can fail.
//   Use error_raise(ERR_xxx, line_num) for error reporting.
 // ---

#include "stdlib_core.h"
#include "funcreg.h"
#include "runtime.h"
#include "value.h"
#include <string.h>
#include "error_registry.h"
#include "dialect.h"

 // PCG32 Pseudo-Random Number Generator
 // Deterministic, cross-platform, massive period, non-repeating
 // relative to simple LCGs.
static unsigned long pcg32_random(RuntimeState *state) {
    unsigned long oldstate = state->rnd_seed;
    // Advance internal state
    state->rnd_seed = oldstate * 6364136223846793005ULL + (12345ULL | 1);
    // Calculate output function (XSH RR)
    unsigned long xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    unsigned long rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

BValue stdlib_core_rnd(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    long n;
    // GW-BASIC: bare RND is equivalent to RND(1)
    n = (argc > 0) ? bval_to_int(&args[0]) : 1;

    if (dialect_get_config()->has_float) {
        if (n < 0) {
            state->rnd_seed = (unsigned long)(-n);
        }
        if (n != 0) {
            // Generate next random 32-bit int
            unsigned long r = pcg32_random(state);
            // Return as float in [0, 1)
            return bval_float((double)r / 4294967296.0);
        }
        // n == 0 returns the last generated float, which is tricky to extract
         // from PCG state without advancing. So we just re-evaluate it based on seed.
        unsigned long oldstate = state->rnd_seed;
        unsigned long xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        unsigned long rot = oldstate >> 59u;
        unsigned long r = (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
        return bval_float((double)r / 4294967296.0);
    }
    
    // PATB mode: return integer 1..n
    if (n <= 0) return bval_int(1);
    return bval_int((pcg32_random(state) % (unsigned long)n) + 1);
}

BValue stdlib_core_len(BValue *args, int argc, void *rt)
{
    (void)argc; (void)rt;
    return bval_len(&args[0], 0);
}

BValue stdlib_core_chr(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    return bval_chr(&args[0], 0, &state->strpool);
}

BValue stdlib_core_str(BValue *args, int argc, void *rt)
{
    RuntimeState *state = (RuntimeState *)rt;
    (void)argc;
    return bval_str(&args[0], 0, &state->strpool);
}

// --- MS-BASIC Math ---
BValue stdlib_core_abs(BValue *args, int argc, void *rt) { (void)argc; (void)rt; return bval_abs(&args[0], 0); }
BValue stdlib_core_sgn(BValue *args, int argc, void *rt) { (void)argc; (void)rt; return bval_sgn(&args[0], 0); }
BValue stdlib_core_int(BValue *args, int argc, void *rt) { (void)argc; (void)rt; return bval_int_func(&args[0], 0); }
BValue stdlib_core_sqr(BValue *args, int argc, void *rt) { (void)argc; (void)rt; return bval_sqr(&args[0], 0); }
BValue stdlib_core_log(BValue *args, int argc, void *rt) { (void)argc; (void)rt; return bval_log(&args[0], 0); }
BValue stdlib_core_exp(BValue *args, int argc, void *rt) { (void)argc; (void)rt; return bval_exp(&args[0], 0); }

// Note: sin, cos, tan, atn are normally affected by OPTION ANGLE, but are core
#include <math.h>
#define DEG_TO_RAD (3.14159265358979323846 / 180.0)
#define RAD_TO_DEG (180.0 / 3.14159265358979323846)

BValue stdlib_core_sin(BValue *args, int argc, void *rt) {
    RuntimeState *state = (RuntimeState *)rt; (void)argc;
    if (state->angle_degrees) return bval_float(sin(bval_to_float(&args[0]) * DEG_TO_RAD));
    return bval_sin(&args[0], 0);
}
BValue stdlib_core_cos(BValue *args, int argc, void *rt) {
    RuntimeState *state = (RuntimeState *)rt; (void)argc;
    if (state->angle_degrees) return bval_float(cos(bval_to_float(&args[0]) * DEG_TO_RAD));
    return bval_cos(&args[0], 0);
}
BValue stdlib_core_tan(BValue *args, int argc, void *rt) {
    RuntimeState *state = (RuntimeState *)rt; (void)argc;
    if (state->angle_degrees) return bval_float(tan(bval_to_float(&args[0]) * DEG_TO_RAD));
    return bval_tan(&args[0], 0);
}
BValue stdlib_core_atn(BValue *args, int argc, void *rt) {
    RuntimeState *state = (RuntimeState *)rt; (void)argc;
    if (state->angle_degrees) return bval_float(atan(bval_to_float(&args[0])) * RAD_TO_DEG);
    return bval_atn(&args[0], 0);
}

// --- MS-BASIC Strings ---
BValue stdlib_core_left(BValue *args, int argc, void *rt) {
    RuntimeState *state = (RuntimeState *)rt; (void)argc;
    return bval_left(&args[0], &args[1], 0, &state->strpool);
}
BValue stdlib_core_right(BValue *args, int argc, void *rt) {
    RuntimeState *state = (RuntimeState *)rt; (void)argc;
    return bval_right(&args[0], &args[1], 0, &state->strpool);
}
BValue stdlib_core_mid(BValue *args, int argc, void *rt) {
    RuntimeState *state = (RuntimeState *)rt;
    if (argc < 3) {
        BValue len_val = bval_int(255);
        return bval_mid(&args[0], &args[1], &len_val, 0, &state->strpool);
    }
    return bval_mid(&args[0], &args[1], &args[2], 0, &state->strpool);
}
BValue stdlib_core_instr(BValue *args, int argc, void *rt) {
    const char *h, *n; int hl, nl, i, start_off; (void)rt;
     // 2-arg: INSTR(haystack$, needle$)
     // 3-arg: INSTR(start%, haystack$, needle$)
    if (argc >= 3) {
        // 3-arg form
        start_off = (int)bval_to_int(&args[0]) - 1;
        if (!bval_is_string(&args[1]) || !bval_is_string(&args[2]))
            return bval_int(0);
        h = args[1].v.sval.data; hl = args[1].v.sval.length;
        n = args[2].v.sval.data; nl = args[2].v.sval.length;
    } else {
        // 2-arg form
        start_off = 0;
        if (argc < 2 || !bval_is_string(&args[0]) ||
            !bval_is_string(&args[1]))
            return bval_int(0);
        h = args[0].v.sval.data; hl = args[0].v.sval.length;
        n = args[1].v.sval.data; nl = args[1].v.sval.length;
    }
    if (!h || !n || nl > hl || nl == 0) return bval_int(0);
    if (start_off < 0) start_off = 0;
    if (start_off > hl - nl) return bval_int(0);
    for (i = start_off; i <= hl - nl; i++) {
        if (memcmp(h + i, n, (size_t)nl) == 0) return bval_int(i + 1);
    }
    return bval_int(0);
}

BValue stdlib_core_err_str(BValue *args, int argc, void *rt) {
    return builtin_err_str(args, argc, rt);
}

void stdlib_core_register(void)
{
    funcreg_override(KW_LEN, stdlib_core_len);
    funcreg_override(KW_CHR, stdlib_core_chr);
    funcreg_override(KW_STR_FUNC, stdlib_core_str);
    funcreg_override(KW_RND, stdlib_core_rnd);
    
    funcreg_override(KW_ABS, stdlib_core_abs);
    funcreg_override(KW_SGN, stdlib_core_sgn);
    funcreg_override(KW_INT_FUNC, stdlib_core_int);
    funcreg_override(KW_SQR, stdlib_core_sqr);
    funcreg_override(KW_SIN, stdlib_core_sin);
    funcreg_override(KW_COS, stdlib_core_cos);
    funcreg_override(KW_TAN, stdlib_core_tan);
    funcreg_override(KW_ATN, stdlib_core_atn);
    funcreg_override(KW_LOG_FUNC, stdlib_core_log);
    funcreg_override(KW_EXP, stdlib_core_exp);
    
    funcreg_override(KW_LEFT, stdlib_core_left);
    funcreg_override(KW_RIGHT, stdlib_core_right);
    funcreg_override(KW_MID, stdlib_core_mid);
    funcreg_override(KW_INSTR, stdlib_core_instr);
    
    funcreg_override(KW_ERR_STR, stdlib_core_err_str);
}
