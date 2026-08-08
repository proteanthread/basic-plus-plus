/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file error.c
 * @brief VM error handling, error code lookup, ON ERROR GOTO dispatch, and RESUME logic for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `vm_raise_error()`, `vm_error_message()`, `vm_on_error_goto()`, `vm_resume()`, and error state management.
 *
 * 2. WHY IT EXISTS:
 * Provides GW-BASIC and QBASIC ON ERROR GOTO / RESUME / RESUME NEXT error trapping and standard BASIC error code lookup.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Maps integer error codes (1..255) to standard BASIC error message strings, records error line (`ERR`), error code (`ERR`), and error line number (`ERL`), and handles GOTO dispatch when ON ERROR handler is registered.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "vm/vm.h", "vm_internal.h",
 * "stmt/stmt.h", "device/vdev.h", <stdio.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add new custom error code mappings (e.g. ECMA-116 or BASIC++ extended error codes).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Standard BASIC error code numbers (e.g. 1 = NEXT without FOR, 2 = Syntax error, 3 = RETURN without GOSUB, 4 = Out of DATA, 5 = Illegal function call, 6 = Overflow, 7 = Out of memory, 9 = Subscript out of range, 11 = Division by zero, 13 = Type mismatch).
 *
 * 8. WHAT TO EXPECT:
 * `vm_raise_error()` sets `vm->last_error`, prints error message if unhandled, or jumps to ON ERROR GOTO handler line.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `vm->on_error_line` target line existence and `RESUME` stack state.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Static constant string lookup table avoids heap allocations during panic.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/context.c
 * - engine/src/device/vdev.c
 * Prerequisite Header Files:
 * - engine/include/vm/vm.h
 * - engine/src/vm/vm_internal.h
 * - engine/include/device/vdev.h
 */
#include "vm/vm.h"
#include "vm_internal.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "types/config.h"
#include "runtime/metadata.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "device/usb.h"
#include "runtime/file.h"
#include "device/vcon.h"
#include "device/bus.h"
#include "runtime/spec.h"
#include "security/security.h"
#include "eval/eval.h"
#include "debug/logger.h"
#include "runtime/variables.h"
#include "platform/platform.h"
#include "core/struct.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


void vm_set_error(VMContext *vm, int code, const char *msg) {
    if (vm) {
        vm->last_error.code = code;
        vm->last_error.category = ERR_CAT_RUNTIME;
        vm->last_error.message = msg;
        vm->last_error.line = vm->current_line;
        vm->last_error.file = "vm.c";
    }
}

void vm_clear_error(VMContext *vm) {
    if (vm) {
        memset(&vm->last_error, 0, sizeof(BppError));
    }
}

bool vm_has_error(VMContext *vm) {
    return vm ? (vm->last_error.code != 0) : false;
}

BppError vm_get_error(VMContext *vm) {
    if (vm) return vm->last_error;
    BppError null_err;
    memset(&null_err, 0, sizeof(null_err));
    return null_err;
}

int vm_get_err_code(VMContext *vm) {
    return vm ? vm->err_code : 0;
}

BppLineNumber vm_get_err_line(VMContext *vm) {
    return vm ? vm->err_line : 0.0;
}

void vm_set_error_trap(VMContext *vm, BppLineNumber line) {
    if (vm) {
        vm->error_trap_line = line;
    }
}

void vm_reset_error_state(VMContext *vm) {
    if (vm) {
        vm->error_trap_line = 0.0;
        vm->in_error_handler = false;
        vm->err_code = 0;
        vm->err_line = 0.0;
        vm->error_occurred_line = 0.0;
        vm->error_occurred_pos = NULL;
        vm->error_next_pos = NULL;
    }
}

BppLineNumber vm_get_error_trap(VMContext *vm) {
    return vm ? vm->error_trap_line : 0.0;
}

bool vm_is_in_error_handler(VMContext *vm) {
    return vm ? vm->in_error_handler : false;
}

void vm_set_in_error_handler(VMContext *vm, bool in_handler) {
    if (vm) {
        vm->in_error_handler = in_handler;
    }
}

BppLineNumber vm_get_error_occurred_line(VMContext *vm) {
    return vm ? vm->error_occurred_line : 0.0;
}

const char *vm_get_error_occurred_pos(VMContext *vm) {
    return vm ? vm->error_occurred_pos : NULL;
}

const char *vm_get_error_next_pos(VMContext *vm) {
    return vm ? vm->error_next_pos : NULL;
}

void vm_trigger_error_trap(VMContext *vm, int code, BppLineNumber line, const char *pos, const char *next_pos) {
    if (!vm) return;
    vm->err_code = code;
    vm->err_line = line;
    vm->error_occurred_line = line;
    vm->error_occurred_pos = pos;
    vm->error_next_pos = next_pos;
    vm->in_error_handler = true;

    BValue *p_err = var_lookup(vm->var, "ERR", true);
    if (p_err) {
        p_err->type = VAL_NUMBER;
        p_err->as.number = code;
    }
    BValue *p_erl = var_lookup(vm->var, "ERL", true);
    if (p_erl) {
        p_erl->type = VAL_NUMBER;
        p_erl->as.number = line;
    }

    vm_jump(vm, vm->error_trap_line, NULL);
}

void vm_trigger_try_catch_handler(VMContext *vm, int code, const char *msg) {
    BppTryFrame frame;
    if (vm && try_stack_pop(vm->try_stack, &frame)) {
        vm_restore_stack_depths(vm, frame);

        vm->err_code = code;
        vm->err_line = vm->current_line;

        BValue err_val;
        err_val.type = VAL_NUMBER;
        err_val.as.number = (double)code;
        var_assign(vm->var, "ERR", err_val);

        const char *m = msg ? msg : "";
        BppStringRef str_ref = str_create(vm->str, m, strlen(m));
        BValue errs_val;
        errs_val.type = VAL_STRING;
        errs_val.as.string = str_ref;
        var_assign(vm->var, "ERR$", errs_val);
        str_release(vm->str, str_ref);

        vm_jump(vm, frame.catch_line, frame.catch_pos);
    }
}
