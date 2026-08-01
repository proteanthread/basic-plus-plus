/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file mod_mathext.c
 * @brief Extended Math standard module.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements and registers extended mathematical functions (CUBE, HYPOT, CLAMP, DEG2RAD, POW, etc.).
 * - Why it exists: Provides an example of a built-in module extending the VM's vocabulary.
 * - Why it works this way: It defines static FunctionEntry entries and registers them during module activation.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: List of exported functions and their C handlers.
 * - What cannot be changed: Registry registration loop, argument counts validation.
 * - What to expect: Activating the module dynamically adds the functions to the global registry.
 * - What to do if something breaks: Trace registration loops and verify math.h operations.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard C17. Uses standard math.h libraries.
 * - Portability concerns: Re-uses double operations to avoid precision loss.
 */

#include "module/module.h"
#include "runtime/funcreg.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* C CUBE function handler */
static BValue fn_cube(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double val = bval_to_float(&args[0]);
    return bval_float(val * val * val);
}

/* C HYPOT function handler */
static BValue fn_hypot(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double a = bval_to_float(&args[0]);
    double b = bval_to_float(&args[1]);
    return bval_float(sqrt(a * a + b * b));
}

/* C CLAMP function handler */
static BValue fn_clamp(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double x = bval_to_float(&args[0]);
    double min_val = bval_to_float(&args[1]);
    double max_val = bval_to_float(&args[2]);
    if (x < min_val) return bval_float(min_val);
    if (x > max_val) return bval_float(max_val);
    return bval_float(x);
}

/* C DEG2RAD function handler */
static BValue fn_deg2rad(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double deg = bval_to_float(&args[0]);
    return bval_float(deg * M_PI / 180.0);
}

/* C RAD2DEG function handler */
static BValue fn_rad2deg(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double rad = bval_to_float(&args[0]);
    return bval_float(rad * 180.0 / M_PI);
}

/* C POW function handler */
static BValue fn_pow(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double base = bval_to_float(&args[0]);
    double exponent = bval_to_float(&args[1]);
    return bval_float(pow(base, exponent));
}

static BValue fn_testsys(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(42.0);
}

static BValue fn_testio(BValue *args, int argc, void *rt) {
    (void)args; (void)argc; (void)rt;
    return bval_float(84.0);
}

static const FunctionEntry mathext_funcs[] = {
    { "CUBE",    KW_NONE, FCAT_MATH, FRET_FLOAT, 1, 1, FSAFE_PURE, 0, fn_cube,    "x^3",               NULL },
    { "HYPOT",   KW_NONE, FCAT_MATH, FRET_FLOAT, 2, 2, FSAFE_PURE, 0, fn_hypot,   "Hypotenuse",        NULL },
    { "CLAMP",   KW_NONE, FCAT_MATH, FRET_FLOAT, 3, 3, FSAFE_PURE, 0, fn_clamp,   "Clamp to range",    NULL },
    { "DEG2RAD", KW_NONE, FCAT_MATH, FRET_FLOAT, 1, 1, FSAFE_PURE, 0, fn_deg2rad, "Degrees to radians", NULL },
    { "RAD2DEG", KW_NONE, FCAT_MATH, FRET_FLOAT, 1, 1, FSAFE_PURE, 0, fn_rad2deg, "Radians to degrees", NULL },
    { "POW",     KW_NONE, FCAT_MATH, FRET_FLOAT, 2, 2, FSAFE_PURE, 0, fn_pow,     "x to the power y",   NULL },
    { "TESTSYS", KW_NONE, FCAT_MATH, FRET_FLOAT, 0, 0, FSAFE_SYSTEM, 0, fn_testsys, "Test system safety function", NULL },
    { "TESTIO",  KW_NONE, FCAT_MATH, FRET_FLOAT, 0, 0, FSAFE_IO,     0, fn_testio,  "Test I/O safety function",      NULL }
};

static int mathext_init(void *rt) {
    (void)rt;
    int count = sizeof(mathext_funcs) / sizeof(mathext_funcs[0]);
    for (int i = 0; i < count; i++) {
        funcreg_register(&mathext_funcs[i]);
    }
    return 0;
}

static const BppModuleInfo mathext_info = {
    "MATHEXT",
    "1.0",
    "Extended math functions",
    MOD_LIBRARY,
    CAP_MATH,
    SEC_STANDARD, /* Pinned to SEC_STANDARD or lower for testing security levels */
    mathext_init,
    NULL
};

void mod_mathext_register(void) {
    module_register(&mathext_info);
}
