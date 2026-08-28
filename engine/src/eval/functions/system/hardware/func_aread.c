// FILENAME: func_aread.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c, funcreg.h, funcreg.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (vm.h)
// Implements the AREAD built-in function to read analog pin values.
//
// ---- Includes ----

#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "esp32_hal.h"

BValue func_aread_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "AREAD expects numeric pin argument";
        return res;
    }

    int pin = (int)args[0].as.number;
    res.as.number = (double)esp32_hal_analog_read(pin);
    return res;
}

void func_aread_register(void) {
    MicroLibMetadata meta = {
        .name = "AREAD",
        .category = "Hardware & IoT",
        .syntax = "AREAD(pin)",
        .help_text = "Reads analog value (0-4095) from specified microcontroller ADC pin.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "AREAD",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_IO,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read analog ADC pin value",
        .module_name = "Hardware"
    };
    funcreg_register(&entry);
}
