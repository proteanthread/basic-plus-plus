// FILENAME: func_dread.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (esp32_hal.h, esp32_hal.c, funcreg.h, funcreg.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (vm.h)
// Implements the DREAD built-in function to read digital pin states.
//
// ---- Includes ----

#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "esp32_hal.h"

static const LangDesc g_dread_desc = {
    .name = "DREAD",
    .category = "Hardware & IoT",
    .syntax = "DREAD(pin)",
    .description = "Reads digital state (0 or 1) from specified microcontroller pin.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

BValue func_dread_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count < 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "DREAD expects numeric pin argument";
        return res;
    }

    int pin = (int)args[0].as.number;
    res.as.number = (double)esp32_hal_digital_read(pin);
    return res;
}

void func_dread_register(void) {
    lang_desc_register(&g_dread_desc);

    FunctionEntry entry = {
        .name = "DREAD",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_IO,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read digital pin state",
        .module_name = "Hardware"
    };
    funcreg_register(&entry);
}
