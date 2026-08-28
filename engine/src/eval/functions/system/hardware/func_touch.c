// FILENAME: func_touch.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, iot_sensors.h, iot_sensors.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (vm.h)
// Implements the TOUCH.READ built-in function for capacitive touch pin sensing.
//
// ---- Includes ----

#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "iot_sensors.h"

BValue func_touch_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "TOUCH expects numeric pin argument";
        return res;
    }

    int pin = (int)args[0].as.number;
    res.as.number = (double)iot_touch_read(pin);
    return res;
}

void func_touch_register(void) {
    MicroLibMetadata meta = {
        .name = "TOUCH",
        .category = "Hardware & IoT",
        .syntax = "TOUCH(pin) | TOUCH.READ(pin)",
        .help_text = "Reads capacitive touch sensor threshold from microcontroller pin.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "TOUCH",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_IO,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read capacitive touch sensor",
        .module_name = "Hardware"
    };
    funcreg_register(&entry);
}
