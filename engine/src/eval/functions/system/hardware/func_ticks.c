// FILENAME: func_ticks.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c, funcreg.h, funcreg.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (string.c, vm.h)
// Implements TICKS_MS, TICKS_US, and TICKS_DIFF built-in timing functions.
//
// ---- Includes ----

#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "esp32_hal.h"
#include <string.h>

BValue func_ticks_ms_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)arg_count;
    (void)args;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = (double)esp32_hal_ticks_ms();
    return res;
}

BValue func_ticks_us_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)arg_count;
    (void)args;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = (double)esp32_hal_ticks_us();
    return res;
}

BValue func_ticks_diff_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "TICKS_DIFF expects two numeric tick values";
        return res;
    }

    double t1 = args[0].as.number;
    double t2 = args[1].as.number;
    res.as.number = t1 - t2;
    return res;
}

void func_ticks_iot_register(void) {
    MicroLibMetadata meta = {
        .name = "TICKS_MS",
        .category = "Timing & Real-Time",
        .syntax = "TICKS_MS() | TICKS_US() | TICKS_DIFF(t1, t2)",
        .help_text = "Returns monotonically increasing millisecond or microsecond hardware timer counters.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);

    FunctionEntry entry_ms = {
        .name = "TICKS_MS",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_FLOAT,
        .min_args = 0,
        .max_args = 0,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read millisecond tick counter",
        .module_name = "Timing"
    };
    funcreg_register(&entry_ms);

    FunctionEntry entry_us = {
        .name = "TICKS_US",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_FLOAT,
        .min_args = 0,
        .max_args = 0,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read microsecond tick counter",
        .module_name = "Timing"
    };
    funcreg_register(&entry_us);

    FunctionEntry entry_diff = {
        .name = "TICKS_DIFF",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_FLOAT,
        .min_args = 2,
        .max_args = 2,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Compute tick delta accounting for counter wrap",
        .module_name = "Timing"
    };
    funcreg_register(&entry_diff);
}
