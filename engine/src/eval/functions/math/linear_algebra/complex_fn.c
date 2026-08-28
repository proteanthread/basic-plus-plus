// FILENAME: complex_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (complex_num.h, complex_num.c, math.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (complex_fn.h, math.c, string.c)
// Provides runtime implementation for the COMPLEX_FN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/linear_algebra/complex_fn.h"
#include "core/complex_num.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"

#include "runtime/math.h"
void func_complex_register(void) {
    static const MicroLibMetadata meta = {
        .name = "COMPLEX",
        .category = "Math & Trigonometry",
        .syntax = "COMPLEX(real, imag) | REAL(z) | IMAG(z) | CONJG(z) | ARG(z) | CABS(z) | CSIN(z) | CCOS(z) | CEXP(z) | CLOG(z) | CSQR(z)",
        .help_text = "Evaluates Dartmouth DTSS Complex number arithmetic and transcendental operations.",
        .error_codes = "Error 13: Type mismatch, Error 11: Division by zero, Error 5: Illegal function call"
    };
    microlib_register(&meta);
}

BValue func_complex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "COMPLEX") == 0) {
        if (arg_count != 2) {
            err->code = 5; err->message = "COMPLEX expects 2 arguments (real, imag)";
            return res;
        }
        res.type = VAL_COMPLEX;
        res.as.complex_val.real = (args[0].type == VAL_NUMBER) ? args[0].as.number : 0.0;
        res.as.complex_val.imag = (args[1].type == VAL_NUMBER) ? args[1].as.number : 0.0;
        return res;
    }

    if (runtime_strcmp(uname, "REAL") == 0 || runtime_strcmp(uname, "RE") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "REAL expects 1 argument";
            return res;
        }
        res.type = VAL_NUMBER;
        if (args[0].type == VAL_COMPLEX) {
            res.as.number = args[0].as.complex_val.real;
        } else if (args[0].type == VAL_NUMBER) {
            res.as.number = args[0].as.number;
        } else {
            err->code = 13; err->message = "Type mismatch in REAL";
        }
        return res;
    }

    if (runtime_strcmp(uname, "IMAG") == 0 || runtime_strcmp(uname, "IM") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "IMAG expects 1 argument";
            return res;
        }
        res.type = VAL_NUMBER;
        if (args[0].type == VAL_COMPLEX) {
            res.as.number = args[0].as.complex_val.imag;
        } else if (args[0].type == VAL_NUMBER) {
            res.as.number = 0.0;
        } else {
            err->code = 13; err->message = "Type mismatch in IMAG";
        }
        return res;
    }

    if (runtime_strcmp(uname, "CONJG") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "CONJG expects 1 argument";
            return res;
        }
        res.type = VAL_COMPLEX;
        if (args[0].type == VAL_COMPLEX) {
            res.as.complex_val.real = args[0].as.complex_val.real;
            res.as.complex_val.imag = -args[0].as.complex_val.imag;
        } else if (args[0].type == VAL_NUMBER) {
            res.as.complex_val.real = args[0].as.number;
            res.as.complex_val.imag = 0.0;
        } else {
            err->code = 13; err->message = "Type mismatch in CONJG";
        }
        return res;
    }

    if (runtime_strcmp(uname, "CABS") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "CABS expects 1 argument";
            return res;
        }
        res.type = VAL_NUMBER;
        if (args[0].type == VAL_COMPLEX) {
            BppComplex z = { args[0].as.complex_val.real, args[0].as.complex_val.imag };
            res.as.number = complex_abs(z);
        } else if (args[0].type == VAL_NUMBER) {
            res.as.number = (args[0].as.number < 0.0) ? -args[0].as.number : args[0].as.number;
        } else {
            err->code = 13; err->message = "Type mismatch in CABS";
        }
        return res;
    }

    if (runtime_strcmp(uname, "ARG") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "ARG expects 1 argument";
            return res;
        }
        res.type = VAL_NUMBER;
        if (args[0].type == VAL_COMPLEX) {
            BppComplex z = { args[0].as.complex_val.real, args[0].as.complex_val.imag };
            res.as.number = complex_arg(z);
        } else if (args[0].type == VAL_NUMBER) {
            res.as.number = (args[0].as.number < 0.0) ? 3.14159265358979323846 : 0.0;
        } else {
            err->code = 13; err->message = "Type mismatch in ARG";
        }
        return res;
    }

    if (runtime_strcmp(uname, "CSIN") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "CSIN expects 1 argument";
            return res;
        }
        res.type = VAL_COMPLEX;
        if (args[0].type == VAL_COMPLEX) {
            BppComplex z = { args[0].as.complex_val.real, args[0].as.complex_val.imag };
            BppComplex r = complex_sin(z);
            res.as.complex_val.real = r.real;
            res.as.complex_val.imag = r.imag;
        } else {
            err->code = 13; err->message = "Type mismatch in CSIN";
        }
        return res;
    }

    if (runtime_strcmp(uname, "CCOS") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "CCOS expects 1 argument";
            return res;
        }
        res.type = VAL_COMPLEX;
        if (args[0].type == VAL_COMPLEX) {
            BppComplex z = { args[0].as.complex_val.real, args[0].as.complex_val.imag };
            BppComplex r = complex_cos(z);
            res.as.complex_val.real = r.real;
            res.as.complex_val.imag = r.imag;
        } else {
            err->code = 13; err->message = "Type mismatch in CCOS";
        }
        return res;
    }

    if (runtime_strcmp(uname, "CEXP") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "CEXP expects 1 argument";
            return res;
        }
        res.type = VAL_COMPLEX;
        if (args[0].type == VAL_COMPLEX) {
            BppComplex z = { args[0].as.complex_val.real, args[0].as.complex_val.imag };
            BppComplex r = complex_exp(z);
            res.as.complex_val.real = r.real;
            res.as.complex_val.imag = r.imag;
        } else {
            err->code = 13; err->message = "Type mismatch in CEXP";
        }
        return res;
    }

    if (runtime_strcmp(uname, "CLOG") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "CLOG expects 1 argument";
            return res;
        }
        res.type = VAL_COMPLEX;
        if (args[0].type == VAL_COMPLEX) {
            BppComplex z = { args[0].as.complex_val.real, args[0].as.complex_val.imag };
            BppComplex r = complex_log(z);
            res.as.complex_val.real = r.real;
            res.as.complex_val.imag = r.imag;
        } else {
            err->code = 13; err->message = "Type mismatch in CLOG";
        }
        return res;
    }

    if (runtime_strcmp(uname, "CSQR") == 0) {
        if (arg_count != 1) {
            err->code = 5; err->message = "CSQR expects 1 argument";
            return res;
        }
        res.type = VAL_COMPLEX;
        if (args[0].type == VAL_COMPLEX) {
            BppComplex z = { args[0].as.complex_val.real, args[0].as.complex_val.imag };
            BppComplex r = complex_sqr(z);
            res.as.complex_val.real = r.real;
            res.as.complex_val.imag = r.imag;
        } else {
            err->code = 13; err->message = "Type mismatch in CSQR";
        }
        return res;
    }

    return res;
}

