/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
#include "vm/vm.h"
#include "vm_internal.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "types/config.h"
#include "runtime/metadata.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#ifndef BPP_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "device/usb.h"
#include "runtime/file.h"
#include "device/vcon.h"
#include "device/bus.h"
#include "bios/mock_bios.h"
#include "runtime/spec.h"
#include "security/security.h"
#include "eval/eval.h"
#include "debug/logger.h"
#include "runtime/variables.h"
#include "platform/platform.h"
#include "core/dialect.h"
#include "core/struct.h"
#include "module/module.h"

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
