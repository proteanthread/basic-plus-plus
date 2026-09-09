// FILENAME: pol_rec.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, dispatch_call.c)
// NEEDS: libkernel (vm.h, eval.h)
// Provides runtime implementation for POL and REC coordinate conversion functions.
//
// ---- Includes ----

#include "eval/functions/math/trig/pol_rec.h"
#include "runtime/language_descriptor.h"
#include "runtime/math.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/math/math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static const LangDesc g_pol_desc = {
    .name = "POL",
    .category = "Math & Trigonometry",
    .syntax = "POL(x, y [, idx]) | POL[x, y]",
    .description = "Converts rectangular coordinates (x, y) to polar coordinates (r, theta) (Sharp Pocket BASIC PC-1211/PC-1500 / Casio).",
    .error_summary = "Error 13: Type Mismatch, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

static const LangDesc g_rec_desc = {
    .name = "REC",
    .category = "Math & Trigonometry",
    .syntax = "REC(r, theta [, idx]) | REC[r, theta]",
    .description = "Converts polar coordinates (r, theta) to rectangular coordinates (x, y) (Sharp Pocket BASIC PC-1211/PC-1500 / Casio).",
    .error_summary = "Error 13: Type Mismatch, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_pol_rec_register(void) {
    lang_desc_register(&g_pol_desc);
    lang_desc_register(&g_rec_desc);
}

BValue func_pol_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 2 || arg_count > 3) {
        err->code = 5;
        err->message = "POL expects 2 or 3 arguments: POL(x, y [, idx])";
        return res;
    }

    if (args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "POL requires numeric arguments";
        return res;
    }

    double x = args[0].as.number;
    double y = args[1].as.number;

    double r = runtime_sqrt(x * x + y * y);
    double theta = runtime_atan2(y, x);

    int mode = vm_get_angle_mode(vm);
    if (mode == 1) {
        theta *= (180.0 / M_PI);
    } else if (mode == 2) {
        theta *= (200.0 / M_PI);
    }

    res.type = VAL_NUMBER;
    if (arg_count == 3 && (int)args[2].as.number == 1) {
        res.as.number = theta;
    } else {
        res.as.number = r;
    }
    return res;
}

BValue func_rec_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 2 || arg_count > 3) {
        err->code = 5;
        err->message = "REC expects 2 or 3 arguments: REC(r, theta [, idx])";
        return res;
    }

    if (args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "REC requires numeric arguments";
        return res;
    }

    double r = args[0].as.number;
    double theta = args[1].as.number;

    int mode = vm_get_angle_mode(vm);
    double rad = theta;
    if (mode == 1) {
        rad *= (M_PI / 180.0);
    } else if (mode == 2) {
        rad *= (M_PI / 200.0);
    }

    double x = r * runtime_cos(rad);
    double y = r * runtime_sin(rad);

    res.type = VAL_NUMBER;
    if (arg_count == 3 && (int)args[2].as.number == 1) {
        res.as.number = y;
    } else {
        res.as.number = x;
    }
    return res;
}
