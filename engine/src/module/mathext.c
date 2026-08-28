// FILENAME: mathext.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, math.h)
// NEEDS: libengine (math.c)
// Provides core logic and interface definitions for mathext within BASIC++.
//
// ---- Includes ----

#include "runtime/funcreg.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// C CUBE function handler
static BValue fn_cube(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double val = bval_to_float(&args[0]);
    return bval_float(val * val * val);
}

// C HYPOT function handler
static BValue fn_hypot(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double a = bval_to_float(&args[0]);
    double b = bval_to_float(&args[1]);
    return bval_float(sqrt(a * a + b * b));
}

// C CLAMP function handler
static BValue fn_clamp(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double x = bval_to_float(&args[0]);
    double min_val = bval_to_float(&args[1]);
    double max_val = bval_to_float(&args[2]);
    if (x < min_val) return bval_float(min_val);
    if (x > max_val) return bval_float(max_val);
    return bval_float(x);
}

// C DEG2RAD function handler
static BValue fn_deg2rad(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double deg = bval_to_float(&args[0]);
    return bval_float(deg * M_PI / 180.0);
}

// C RAD2DEG function handler
static BValue fn_rad2deg(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    double rad = bval_to_float(&args[0]);
    return bval_float(rad * 180.0 / M_PI);
}

// C POW function handler
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

int mod_mathext_init(void *rt) {
    (void)rt;
    int count = sizeof(mathext_funcs) / sizeof(mathext_funcs[0]);
    for (int i = 0; i < count; i++) {
        funcreg_register(&mathext_funcs[i]);
    }
    return 0;
}
