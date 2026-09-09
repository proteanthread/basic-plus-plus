// FILENAME: func_bios.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: reg/reg_hw.h, bios/bios.h, runtime/funcreg.h, runtime/language_descriptor.h
// Implements the freestanding BIOS service function in BASIC++.

#include "functions/hardware/func_bios.h"
#include "reg/reg_hw.h"
#include "bios/bios.h"
#include "runtime/funcreg.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/strings.h"
#include "vm/vm.h"

static const LangDesc g_func_bios_desc = {
    .name = "BIOS",
    .category = "System & Hardware",
    .syntax = "BIOS(int_num [, ax, bx, cx, dx]) | BIOS(op$, ...)",
    .description = "Dispatches virtual IBM PC BIOS interrupts using native REG.* hardware registers and returns execution status.",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_FUNCTION
};

BValue func_bios_eval(BValue *args, int arg_count, void *rt) {
    VMContext *vm = (VMContext *)rt;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_INTEGER;

    if (arg_count == 0) return res;

    BiosContext *bios = vm ? vm_get_bios(vm) : NULL;

    // Numeric interrupt call: BIOS(int_num [, ax, bx, cx, dx])
    if (args[0].type == VAL_INTEGER || args[0].type == VAL_NUMBER) {
        int int_num = (int)args[0].as.number;

        if (arg_count >= 2 && (args[1].type == VAL_INTEGER || args[1].type == VAL_NUMBER)) {
            reg_hw_set("AX", args[1].as.number);
        }
        if (arg_count >= 3 && (args[2].type == VAL_INTEGER || args[2].type == VAL_NUMBER)) {
            reg_hw_set("BX", args[2].as.number);
        }
        if (arg_count >= 4 && (args[3].type == VAL_INTEGER || args[3].type == VAL_NUMBER)) {
            reg_hw_set("CX", args[3].as.number);
        }
        if (arg_count >= 5 && (args[4].type == VAL_INTEGER || args[4].type == VAL_NUMBER)) {
            reg_hw_set("DX", args[4].as.number);
        }

        if (bios) {
            BiosRegs regs;
            runtime_memset(&regs, 0, sizeof(regs));
            regs.ax = (uint16_t)reg_hw_get("AX", NULL);
            regs.bx = (uint16_t)reg_hw_get("BX", NULL);
            regs.cx = (uint16_t)reg_hw_get("CX", NULL);
            regs.dx = (uint16_t)reg_hw_get("DX", NULL);
            regs.flags = (uint16_t)reg_hw_get("FLAGS", NULL);

            bios_interrupt(bios, (uint8_t)(int_num & 0xFF), &regs);

            reg_hw_set("AX", (double)regs.ax);
            reg_hw_set("BX", (double)regs.bx);
            reg_hw_set("CX", (double)regs.cx);
            reg_hw_set("DX", (double)regs.dx);
            reg_hw_set("FLAGS", (double)regs.flags);
        }

        bool found = false;
        res.as.number = reg_hw_get("AX", &found);
        return res;
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        const char *op = str_data(args[0].as.string);

        if (runtime_strcasecmp(op, "MODEL") == 0) {
            if (bios && arg_count >= 2 && args[1].type == VAL_STRING && args[1].as.string) {
                const char *mname = str_data(args[1].as.string);
                BiosModel m = BIOS_MODEL_IBM_PC;
                if (runtime_strcasecmp(mname, "PCXT") == 0) m = BIOS_MODEL_IBM_XT;
                else if (runtime_strcasecmp(mname, "PCAT") == 0) m = BIOS_MODEL_IBM_AT;
                else if (runtime_strcasecmp(mname, "PCJR") == 0) m = BIOS_MODEL_IBM_PCJR;
                bios_set_model(bios, m);
            }
            res.type = VAL_STRING;
            const char *ret_m = "IBMPC";
            if (bios) {
                BiosModel m = bios_get_model(bios);
                if (m == BIOS_MODEL_IBM_XT) ret_m = "PCXT";
                else if (m == BIOS_MODEL_IBM_AT) ret_m = "PCAT";
                else if (m == BIOS_MODEL_IBM_PCJR) ret_m = "PCJR";
            }
            res.as.string = str_create(vm ? vm_get_str(vm) : NULL, ret_m, runtime_strlen(ret_m));
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

void func_bios_register(void) {
    lang_desc_register(&g_func_bios_desc);

    FunctionEntry entry = {
        .name = "BIOS",
        .keyword = KW_NONE,
        .category = FCAT_IO,
        .ret_type = FRET_ANY,
        .min_args = 1,
        .max_args = 5,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = func_bios_eval,
        .help_text = "Virtual IBM PC BIOS interrupt dispatch and service control",
        .module_name = "Hardware"
    };
    funcreg_register(&entry);
}
