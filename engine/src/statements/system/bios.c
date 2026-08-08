/**
 * @file bios.c
 * @brief BIOS int_num low-level PC interrupt emulation statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements BIOS int_num statement handler for invoking x86/PC BIOS interrupts (INT 10h Video, INT 13h Disk, INT 16h Keyboard, INT 21h DOS).
 *
 * 2. WHY IT EXISTS:
 * Enables low-level BIOS interrupt emulation by mapping x86 register variables (AX%, BX%, CX%, DX%, FLAGS%) to VM virtual bus registers.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Reads registers from current variable context (var_lookup), populates BiosRegs structure, dispatches to bios_exec_interrupt(), and writes updated register values back to BASIC variables.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_bios'. Includes "statements/system/bios.h",
 * "bios/bios.h", "eval/eval.h", "security/security.h", "runtime/variables.h", "vm/vm.h", "device/vdev.h", "device/bus.h", "platform/platform.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs'). Security sandbox validation restricts low-level I/O in restricted profiles.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Extend register mapping to include 32-bit extended registers (EAX%, EBX%, ESI%, EDI%) for 386+ execution modes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Security invariant: MUST invoke sec_check_permission(SEC_SYS_HARDWARE) before dispatching hardware interrupts.
 *
 * 8. WHAT TO EXPECT:
 * Executes BIOS interrupt handler and updates BASIC register variables; returns ERR_NONE or ERR_PERMISSION_DENIED under sandbox violation.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify variable lookup/assignment logic for register names (AX%, BX%, CX%, DX%).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext, VariableContext, and active BiosRegs emulator.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses uint32_t register bitmasks and safe uint32_t casts.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/bios/bios.c
 * - engine/src/security/security.c
 * - engine/src/runtime/variables.c
 * Prerequisite Header Files:
 * - engine/include/statements/system/bios.h
 * - engine/include/bios/bios.h
 * - engine/include/vm/vm.h
 */

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

    /* Sandbox Check: requires SECOP_SYSTEM permission */
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

    /* 1. Fetch register values from BASIC variables */
    BiosRegs regs;
    memset(&regs, 0, sizeof(regs));
    regs.ax = (uint16_t)get_reg_var(var_ctx, "AX%");
    regs.bx = (uint16_t)get_reg_var(var_ctx, "BX%");
    regs.cx = (uint16_t)get_reg_var(var_ctx, "CX%");
    regs.dx = (uint16_t)get_reg_var(var_ctx, "DX%");
    regs.flags = (uint16_t)get_reg_var(var_ctx, "FLAGS%");

    /* 2. Execute BIOS interrupt */
    bios_interrupt(bios, (uint8_t)int_num, &regs);

    /* 3. Retrieve updated register values and write back to variables */
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

