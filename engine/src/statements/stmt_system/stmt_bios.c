/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file stmt_bios.c
 * @brief Statement handler for low-level x86 BIOS & DOS Interrupt emulation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the command handler for the BIOS statement (e.g. BIOS &H10).
 * - Why it exists: Exposes the emulated IBM PC BIOS interrupt routines to BASIC++ scripts.
 * - Why it works this way: It loads register variables (AX%, BX%, CX%, DX%, FLAGS%) from
 *   the variable context, assignments, calls mock_bios_interrupt, and saves the updated values.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Pre-check validations or additional default register setup.
 * - What cannot be changed: Variable names mapping (must use AX%, BX%, CX%, DX%, FLAGS%).
 * - What to expect: Unregistered registers default to 0. Emulation is safe.
 * - What to do if something breaks: Verify variable context lookup scopes.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Type suffixes match standard integer representation (%).
 * - Portability concerns: Fully standard C17.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add ES% and DS% segment register support.
 * - How to write external extensions: Custom VM plugins register handlers using these APIs.
 */

#include "stmt/stmt.h"
#include "bios/mock_bios.h"
#include "eval/eval.h"
#include "security/security.h"
#include "runtime/variables.h"
#include "vm/vm.h"
#include "device/vdev.h"
#include "device/bus.h"
#include <stdio.h>
#include <string.h>

static uint32_t get_reg_var(VariableContext *var_ctx, const char *name) {
    BValue *val = var_lookup(var_ctx, name, true);
    if (val && val->type != VAL_STRING) {
        return (uint32_t)val->as.number;
    }
    return 0;
}

static void set_reg_var(VariableContext *var_ctx, const char *name, uint32_t value) {
    BValue *val = var_lookup(var_ctx, name, true);
    if (val) {
        val->type = VAL_NUMBER;
        val->as.number = (double)value;
    }
}

/**
 * @brief BIOS <interrupt_number>
 */
BppError stmt_bios_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Sandbox Check: requires SECOP_SYSTEM permission */
    if (security_check(SECOP_SYSTEM, 0) != 0) {
        err.code = 70;
        err.message = "Permission denied: BIOS interrupt execution restricted";
        return err;
    }

    MockBiosContext *bios = vm_get_bios(vm);
    VariableContext *var_ctx = vm_get_var(vm);
    if (!bios || !var_ctx) {
        err.code = 57;
        err.message = "Device I/O error: Mock BIOS not initialized";
        return err;
    }

    BValue int_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (int_val.type == VAL_STRING) {
        const char *name = str_data(int_val.as.string);
        MockBiosModel model = mock_bios_model_from_string(name);
        if (model == BIOS_MODEL_NONE && strcasecmp(name, "NONE") != 0) {
            err.code = 2;
            err.message = "Invalid BIOS map. Must be NONE, MSDOS, IBMPC, PCJR, PCXT, or PCAT.";
            str_release(vm_get_str(vm), int_val.as.string);
            return err;
        }

        vdev_printf(vm_get_vdev(vm), "BIOS Map switched to: %s\n", mock_bios_model_to_string(model));

        uint8_t *bios_ram = vm_get_bios_ram(vm);
        if (bios_ram) {
            vdev_bus_set_ram(bios_ram, 1024 * 1024);
            vdev_bus_set_model(model);
            mock_bios_init_mem(bios, bios_ram, 1024 * 1024, model);
        }

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
    uint32_t ax = get_reg_var(var_ctx, "AX%");
    uint32_t bx = get_reg_var(var_ctx, "BX%");
    uint32_t cx = get_reg_var(var_ctx, "CX%");
    uint32_t dx = get_reg_var(var_ctx, "DX%");
    uint32_t flags = get_reg_var(var_ctx, "FLAGS%");

    /* Set register context */
    vm_set_bios_registers(vm, ax, bx, cx, dx, flags);

    /* 2. Execute BIOS/DOS interrupt */
    mock_bios_interrupt(bios, (uint8_t)int_num);

    /* 3. Retrieve updated register values and write back to variables */
    uint32_t ax_new = 0, bx_new = 0, cx_new = 0, dx_new = 0, flags_new = 0;
    vm_get_bios_registers(vm, &ax_new, &bx_new, &cx_new, &dx_new, &flags_new);

    set_reg_var(var_ctx, "AX%", ax_new);
    set_reg_var(var_ctx, "BX%", bx_new);
    set_reg_var(var_ctx, "CX%", cx_new);
    set_reg_var(var_ctx, "DX%", dx_new);
    set_reg_var(var_ctx, "FLAGS%", flags_new);

    return err;
}
