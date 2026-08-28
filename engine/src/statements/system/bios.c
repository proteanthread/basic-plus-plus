// FILENAME: bios.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, libhardware, libkernel
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (bios.h, eval.h, eval.c, stmt.h, string.c, vm.h)
// NEEDS: libkernel (bus.h, bus.c, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the BIOS statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "bios/bios.h"
#include "eval/eval.h"
#include "security/security.h"
#include "runtime/variables.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include "device/bus.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include <stdio.h>
#include <string.h>

static uint32_t get_reg_var(VariableContext *var_ctx, const char *name) {
    BValue *val = var_lookup(var_ctx, name, false);
    if (val && val->type != VAL_STRING) {
        return (uint32_t)val->as.number;
    }
    return 0;
}

static void set_reg_var(VariableContext *var_ctx, const char *name, uint32_t num) {
    BValue val;
    val.type = VAL_NUMBER;
    val.as.number = (double)num;
    var_assign(var_ctx, name, val);
}

BppError stmt_bios_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    // Sandbox Check: requires SECOP_SYSTEM permission
    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: BIOS interrupt execution restricted";
        return err;
    }

    BiosContext *bios = vm_get_bios(vm);
    VariableContext *var_ctx = vm_get_var(vm);
    if (!bios || !var_ctx) {
        err.code = 57;
        err.message = "Device I/O error: BIOS subsystem not initialized";
        return err;
    }

    BValue int_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (int_val.type == VAL_STRING) {
        const char *name = str_data(int_val.as.string);
        BiosModel model = BIOS_MODEL_IBM_PC;
        if (strcasecmp(name, "IBMPC") == 0) model = BIOS_MODEL_IBM_PC;
        else if (strcasecmp(name, "PCXT") == 0) model = BIOS_MODEL_IBM_XT;
        else if (strcasecmp(name, "PCAT") == 0) model = BIOS_MODEL_IBM_AT;
        else if (strcasecmp(name, "PCJR") == 0) model = BIOS_MODEL_IBM_PCJR;

        bios_set_model(bios, model);
        str_release(vm_get_str(vm), int_val.as.string);
        return err;
    }

    int int_num = (int)int_val.as.number;
    if (int_num < 0 || int_num > 255) {
        err.code = 5;
        err.message = "Illegal function call: Interrupt number must be 0-255";
        return err;
    }

    // 1. Fetch register values from BASIC variables
    BiosRegs regs;
    memset(&regs, 0, sizeof(regs));
    regs.ax = (uint16_t)get_reg_var(var_ctx, "AX%");
    regs.bx = (uint16_t)get_reg_var(var_ctx, "BX%");
    regs.cx = (uint16_t)get_reg_var(var_ctx, "CX%");
    regs.dx = (uint16_t)get_reg_var(var_ctx, "DX%");
    regs.flags = (uint16_t)get_reg_var(var_ctx, "FLAGS%");

    // 2. Execute BIOS interrupt
    bios_interrupt(bios, (uint8_t)int_num, &regs);

    // 3. Retrieve updated register values and write back to variables
    set_reg_var(var_ctx, "AX%", regs.ax);
    set_reg_var(var_ctx, "BX%", regs.bx);
    set_reg_var(var_ctx, "CX%", regs.cx);
    set_reg_var(var_ctx, "DX%", regs.dx);
    set_reg_var(var_ctx, "FLAGS%", regs.flags);

    return err;
}

void stmt_bios_register(void) {
    static const MicroLibMetadata meta = {
        .name = "BIOS",
        .category = "Devices & Network",
        .syntax = "BIOS int_num",
        .help_text = "Executes low-level x86 BIOS interrupt routines using AX%, BX%, CX%, DX% variables.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

