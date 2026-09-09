// FILENAME: func_cpu86.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: reg/reg_hw.h, bios/bios_cpu8086.h, runtime/funcreg.h, runtime/language_descriptor.h
// Implements the freestanding CPU / CPU86 function in BASIC++.

#include "functions/hardware/func_cpu86.h"
#include "reg/reg_hw.h"
#include "bios/bios_cpu8086.h"
#include "runtime/funcreg.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/strings.h"
#include "vm/vm.h"

static const LangDesc g_func_cpu86_desc = {
    .name = "CPU",
    .category = "System & Hardware",
    .syntax = "CPU(op$, ...) | CPU86(op$, ...)",
    .description = "Controls 8086/8088 virtual CPU execution, interrupts, and hardware registers.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

BValue func_cpu86_eval(BValue *args, int arg_count, void *rt) {
    (void)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_INTEGER;

    if (arg_count == 0) {
        bool found = false;
        res.as.number = reg_hw_get("IP", &found);
        return res;
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *op = str_data(args[0].as.string);

        if (runtime_strcasecmp(op, "RESET") == 0) {
            reg_hw_reset();
            res.as.number = 0.0;
            return res;
        }

        if (runtime_strcasecmp(op, "STEP") == 0) {
            BiosCpu8086Context *cpu = bios_cpu8086_get_active();
            bios_cpu8086_step(cpu, NULL);
            res.as.number = (double)cpu->ip;
            return res;
        }

        if (runtime_strcasecmp(op, "INT") == 0 && arg_count >= 2) {
            int int_num = (int)args[1].as.number;
            BiosCpu8086Context *cpu = bios_cpu8086_get_active();
            bios_cpu8086_int(cpu, (uint8_t)(int_num & 0xFF), NULL);
            res.as.number = (double)cpu->ax;
            return res;
        }

        if (runtime_strcasecmp(op, "REG") == 0 && arg_count >= 2 &&
            args[1].type == VAL_STRING && args[1].as.string) {
            const char *rname = str_data(args[1].as.string);
            if (arg_count >= 3) {
                double val = (args[2].type == VAL_INTEGER || args[2].type == VAL_NUMBER) ? args[2].as.number : 0.0;
                reg_hw_set(rname, val);
            }
            bool found = false;
            res.as.number = reg_hw_get(rname, &found);
            return res;
        }

        if (runtime_strcasecmp(op, "SET") == 0 && arg_count >= 3 &&
            args[1].type == VAL_STRING && args[1].as.string) {
            double val = (args[2].type == VAL_INTEGER || args[2].type == VAL_NUMBER) ? args[2].as.number : 0.0;
            reg_hw_set(str_data(args[1].as.string), val);
            bool found = false;
            res.as.number = reg_hw_get(str_data(args[1].as.string), &found);
            return res;
        }

        // Direct register query
        bool found = false;
        double val = reg_hw_get(op, &found);
        if (found) {
            res.as.number = val;
            return res;
        }
    }

    return res;
}

void func_cpu86_register(void) {
    lang_desc_register(&g_func_cpu86_desc);

    FunctionEntry entry_cpu = {
        .name = "CPU",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_ANY,
        .min_args = 0,
        .max_args = 3,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_cpu86_eval,
        .help_text = "Virtual 8086 CPU execution and register control",
        .module_name = "Hardware"
    };
    funcreg_register(&entry_cpu);

    FunctionEntry entry_cpu86 = {
        .name = "CPU86",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_ANY,
        .min_args = 0,
        .max_args = 3,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_cpu86_eval,
        .help_text = "Virtual 8086 CPU execution and register control",
        .module_name = "Hardware"
    };
    funcreg_register(&entry_cpu86);
}
