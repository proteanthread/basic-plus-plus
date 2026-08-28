// FILENAME: control.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform, libserver
// Implements bytecode virtual machine execution and state for control.
//
// ---- Includes ----

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
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "hal/hal.h"

void vm_set_chaining(VMContext *vm, bool chaining) {
    if (vm) vm->is_chaining = chaining;
}

bool vm_get_chaining(VMContext *vm) {
    return vm ? vm->is_chaining : false;
}

void vm_jump(VMContext *vm, BppLineNumber line, const char *pos) {
    if (vm) {
        vm->next_line = line;
        vm->next_pos = pos;
        vm->jump_active = true;
    }
}

void vm_get_jump_target(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    if (vm) {
        if (out_line) *out_line = vm->next_line;
        if (out_pos) *out_pos = vm->next_pos;
    }
}

void vm_clear_jump(VMContext *vm) {
    if (vm) {
        vm->jump_active = false;
        vm->next_line = 0;
        vm->next_pos = NULL;
    }
}

const char *vm_get_current_pos(VMContext *vm) {
    return vm ? vm->current_pos : NULL;
}

void vm_set_current_pos(VMContext *vm, const char *pos) {
    if (vm) {
        vm->current_pos = pos;
    }
}

static const char *vm_map_copy_to_original(VMContext *vm, const char *pos) {
    if (!vm || !pos || !vm->active_line_original || !vm->active_line_copy) {
        return pos;
    }
    size_t len = runtime_strlen(vm->active_line_copy);
    if (pos >= vm->active_line_copy && pos <= vm->active_line_copy + len) {
        ptrdiff_t offset = pos - vm->active_line_copy;
        return vm->active_line_original + offset;
    }
    return pos;
}

bool vm_gosub_push(VMContext *vm, BppLineNumber line, const char *pos) {
    if (vm) pos = vm_map_copy_to_original(vm, pos);
    return vm ? gosub_stack_push(vm->gosub_stack, line, pos) : false;
}

bool vm_gosub_pop(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? gosub_stack_pop(vm->gosub_stack, out_line, out_pos) : false;
}

bool vm_for_push(VMContext *vm, const char *var_name, double target, double step, BppLineNumber line, const char *pos) {
    if (vm) pos = vm_map_copy_to_original(vm, pos);
    return vm ? for_stack_push(vm->for_stack, var_name, target, step, line, pos) : false;
}

bool vm_for_push_multi(VMContext *vm, const char **var_names, int var_count, double target, double step, BppLineNumber line, const char *pos) {
    if (!vm) return false;
    pos = vm_map_copy_to_original(vm, pos);
    bool ok = for_stack_push_multi(vm->for_stack, var_names, var_count, target, step, line, pos);
    if (ok && var_names && var_names[0]) {
        BValue *ptr = var_lookup(vm->var, var_names[0], true);
        if (ptr) {
            for_stack_set_cached_ptr(vm->for_stack, ptr);
        }
    }
    return ok;
}

bool vm_for_update(VMContext *vm, const char *var_name, double target, double step, const char *next_range_pos) {
    if (vm && next_range_pos) next_range_pos = vm_map_copy_to_original(vm, next_range_pos);
    return vm ? for_stack_update(vm->for_stack, var_name, target, step, next_range_pos) : false;
}

bool vm_for_pop(VMContext *vm, const char *var_name, BppForFrame *out_frame) {
    return vm ? for_stack_pop(vm->for_stack, var_name, out_frame) : false;
}

bool vm_for_peek(VMContext *vm, const char *var_name, BppForFrame *out_frame) {
    return vm ? for_stack_peek(vm->for_stack, var_name, out_frame) : false;
}

bool vm_while_push(VMContext *vm, BppLineNumber line, const char *pos) {
    if (vm) pos = vm_map_copy_to_original(vm, pos);
    return vm ? while_stack_push(vm->while_stack, line, pos) : false;
}

bool vm_while_pop(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? while_stack_pop(vm->while_stack, out_line, out_pos) : false;
}

bool vm_while_peek(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? while_stack_peek(vm->while_stack, out_line, out_pos) : false;
}

bool vm_do_push(VMContext *vm, BppLineNumber line, const char *pos) {
    if (vm) pos = vm_map_copy_to_original(vm, pos);
    return vm ? do_stack_push(vm->do_stack, line, pos) : false;
}

bool vm_do_pop(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? do_stack_pop(vm->do_stack, out_line, out_pos) : false;
}

bool vm_do_peek(VMContext *vm, BppLineNumber *out_line, const char **out_pos) {
    return vm ? do_stack_peek(vm->do_stack, out_line, out_pos) : false;
}

bool vm_select_push(VMContext *vm, BValue val, bool matched, BppLineNumber line, const char *pos) {
    if (!vm) return false;
    pos = vm_map_copy_to_original(vm, pos);
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    }
    bool ok = select_stack_push(vm->select_stack, val, matched, line, pos);
    if (!ok && val.type == VAL_STRING && val.as.string) {
        str_release(vm->str, val.as.string);
    }
    return ok;
}

bool vm_select_pop(VMContext *vm, BppSelectFrame *out_frame) {
    if (!vm) return false;
    BppSelectFrame frame;
    bool ok = select_stack_pop(vm->select_stack, &frame);
    if (ok) {
        if (out_frame) {
            *out_frame = frame; // Ownership transfers to caller
        } else {
            // No caller to receive it, release the ref
            if (frame.val.type == VAL_STRING && frame.val.as.string) {
                str_release(vm->str, frame.val.as.string);
            }
        }
    }
    return ok;
}

bool vm_select_peek(VMContext *vm, BppSelectFrame *out_frame) {
    return vm ? select_stack_peek(vm->select_stack, out_frame) : false;
}

bool vm_sub_push(VMContext *vm, const char *name, BppLineNumber line, const char *pos, bool is_func) {
    if (!vm) return false;
    pos = vm_map_copy_to_original(vm, pos);
    bool ok = sub_stack_push(vm->sub_stack, name, line, pos, is_func);
    if (ok) {
        runtime_strncpy(vm->active_proc, name, sizeof(vm->active_proc) - 1);
        vm->active_proc[sizeof(vm->active_proc) - 1] = '\0';
    }
    return ok;
}

bool vm_sub_pop(VMContext *vm, BppSubFrame *out_frame) {
    if (!vm) return false;
    BppSubFrame frame;
    bool ok = sub_stack_pop(vm->sub_stack, &frame);
    if (ok) {
        if (out_frame) *out_frame = frame;
        // Clear local variables for popped sub scope
        var_clear_scope(vm->var, frame.name);

        BppSubFrame prev;
        if (sub_stack_peek(vm->sub_stack, &prev)) {
            runtime_strncpy(vm->active_proc, prev.name, sizeof(vm->active_proc) - 1);
            vm->active_proc[sizeof(vm->active_proc) - 1] = '\0';
        } else {
            vm->active_proc[0] = '\0';
        }
    }
    return ok;
}


bool vm_sub_peek(VMContext *vm, BppSubFrame *out_frame) {
    return vm ? sub_stack_peek(vm->sub_stack, out_frame) : false;
}

const char *vm_get_active_proc(VMContext *vm) {
    return vm ? vm->active_proc : "";
}

void vm_halt(VMContext *vm) {
    if (vm) {
        vm->running = false;
    }
}

// vm_request_exit - Signal the REPL to terminate the interpreter.
//
// Called by the SYSTEM and EXIT statement handlers.
// The REPL checks vm_exit_requested() after each vm_execute_line().
void vm_request_exit(VMContext *vm) {
    if (vm) {
        vm->exit_requested = true;
    }
}

// vm_exit_requested - Check if the interpreter should exit.
bool vm_exit_requested(VMContext *vm) {
    return vm ? vm->exit_requested : false;
}

void vm_trigger_break(VMContext *vm) {
    if (vm) {
        if (vm->break_trap_line > 0.0) {
            // Trap active: transfer control to the trap line
            vm_jump(vm, vm->break_trap_line, NULL);
        } else {
            vm->break_triggered = true;
            vm->running = false;
        }
    }
}

bool vm_break_triggered(VMContext *vm) {
    return vm ? vm->break_triggered : false;
}

void vm_reset_break(VMContext *vm) {
    if (vm) {
        vm->break_triggered = false;
    }
}

void vm_set_break_enabled(VMContext *vm, bool enabled) {
    if (vm) {
        vm->break_enabled = enabled;
    }
}

bool vm_get_break_enabled(VMContext *vm) {
    return vm ? vm->break_enabled : false;
}

void vm_set_break_trap(VMContext *vm, BppLineNumber line) {
    if (vm) {
        vm->break_trap_line = line;
    }
}

BppLineNumber vm_get_break_trap(VMContext *vm) {
    return vm ? vm->break_trap_line : 0.0;
}

bool vm_is_jump_active(VMContext *vm) {
    return vm ? vm->jump_active : false;
}

bool vm_handle_jump_active(VMContext *vm, BppLineNumber *out_line) {
    if (vm && vm->jump_active) {
        vm->current_line = vm->next_line;
        vm->current_pos = vm->next_pos;
        vm->jump_active = false;
        if (out_line) *out_line = vm->current_line;
        return true;
    }
    return false;
}

void vm_clear_header_jump(VMContext *vm, BppLineNumber def_line) {
    if (vm) {
        vm->jump_active = false;
        vm->current_line = def_line;
    }
}

void vm_with_stack_push(VMContext *vm, const char *path) {
    if (vm && vm->with_stack_depth < 8) {
        runtime_strncpy(vm->with_stack[vm->with_stack_depth++], path, 255);
        vm->with_stack[vm->with_stack_depth - 1][255] = '\0';
    }
}

void vm_with_stack_pop(VMContext *vm) {
    if (vm && vm->with_stack_depth > 0) {
        vm->with_stack_depth--;
    }
}

void vm_with_stack_clear(VMContext *vm) {
    if (vm) {
        vm->with_stack_depth = 0;
    }
}

const char *vm_with_stack_peek(VMContext *vm) {
    if (vm && vm->with_stack_depth > 0) {
        return vm->with_stack[vm->with_stack_depth - 1];
    }
    return NULL;
}

void vm_set_auto_line(VMContext *vm, BppLineNumber start, BppLineNumber step) {
    if (!vm) return;
    vm->auto_line_start = start;
    vm->auto_line_step = step;
    vm->auto_line_active = true;
}

void vm_get_auto_line(VMContext *vm, BppLineNumber *start, BppLineNumber *step, bool *active) {
    if (!vm) return;
    if (start) *start = vm->auto_line_start;
    if (step) *step = vm->auto_line_step;
    if (active) *active = vm->auto_line_active;
}
