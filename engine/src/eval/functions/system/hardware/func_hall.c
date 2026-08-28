// FILENAME: func_hall.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c, iot_sensors.h, iot_sensors.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (vm.h)
// Implements the HALL.READ built-in function for magnetic Hall effect sensing.
//
// ---- Includes ----

#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include "vm/vm.h"
#include "iot_sensors.h"

BValue func_hall_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)arg_count;
    (void)args;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = (double)iot_hall_read();
    return res;
}

void func_hall_register(void) {
    MicroLibMetadata meta = {
        .name = "HALL",
        .category = "Hardware & IoT",
        .syntax = "HALL() | HALL.READ()",
        .help_text = "Reads magnetic field intensity from ESP32 built-in Hall effect sensor.",
        .error_codes = "None"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "HALL",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 0,
        .max_args = 0,
        .safety = FSAFE_IO,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read built-in magnetic Hall sensor",
        .module_name = "Hardware"
    };
    funcreg_register(&entry);
}
