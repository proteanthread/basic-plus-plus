// FILENAME: exec_interrupt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (logger.h, logger.c, string.h, variables.h, variables.c)
// NEEDS: libengine (ast.h, ast.c, exec_internal.h, string.c, vm.h)
// NEEDS: libengine (vm_internal.h)
// NEEDS: libkernel (vdev.h, vdev.c, vprinter.h, vprinter.c)
// Implements bytecode virtual machine execution and state for exec_interrupt.
//
// ---- Includes ----

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../vm_internal.h"
#include "debug/logger.h"
#include "device/vdev.h"
#include "device/vprinter.h"
#include "eval/ast.h"
#include "runtime/variables.h"
#include "vm/exec_internal.h"
#include "vm/vm.h"

//
// ---- Debugger & Breakpoint Management ----

// installs a debugger callback hook for VM execution events
void vm_set_debug_hook(VMContext *vm, void (*hook)(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data), void *user_data) {
    if (vm) {
        vm->debug_hook = hook;
        vm->debug_user_data = user_data;
    }
}

// queries whether single-step execution mode is enabled
bool vm_get_single_step(VMContext *vm) {
    return vm ? vm->debug_single_step : false;
}

// enables or disables single-step execution mode
void vm_set_single_step(VMContext *vm, bool enable) {
    if (vm) {
        vm->debug_single_step = enable;
    }
}

// triggers an interactive debugger breakpoint prompt
void vm_trigger_breakpoint(VMContext *vm, const char *reason) {
    if (!vm) return;
    VDevContext *vdev = vm->vdev;

    if (vdev) {
        vdev_printf(vdev, "\n[BREAKPOINT] at line %lld: %s\n", (long long)vm->current_line, reason ? reason : "unknown");
        vdev_printf(vdev, "Commands: [s] Step, [c] Continue, [v] View Variables (VARS), [q] Quit\n");
    }
    log_warn("Breakpoint triggered at line %lld: %s", (long long)vm->current_line, reason ? reason : "unknown");

    while (true) {
        if (vdev) {
            vdev_printf(vdev, "debug> ");
        }
        char cmd_line[128] = {0};
        if (!fgets(cmd_line, sizeof(cmd_line), stdin)) {
            vm_halt(vm);
            break;
        }
        size_t len = strlen(cmd_line);
        while (len > 0 && (cmd_line[len - 1] == '\n' || cmd_line[len - 1] == '\r')) {
            cmd_line[len - 1] = '\0';
            len--;
        }

        if (strcmp(cmd_line, "s") == 0 || strcmp(cmd_line, "S") == 0 || len == 0) {
            vm->debug_single_step = true;
            break;
        } else if (strcmp(cmd_line, "c") == 0 || strcmp(cmd_line, "C") == 0) {
            vm->debug_single_step = false;
            break;
        } else if (strcmp(cmd_line, "v") == 0 || strcmp(cmd_line, "V") == 0) {
            var_print_all(vm->var, vdev);
        } else if (strcmp(cmd_line, "q") == 0 || strcmp(cmd_line, "Q") == 0) {
            vm_halt(vm);
            break;
        } else {
            if (vdev) {
                vdev_printf(vdev, "Unknown debug command. Use: s, c, v, q\n");
            }
        }
    }
}

//
// ---- Program Main Execution Loop ----

// executes the loaded program from start to completion or interruption
void vm_run_program(VMContext *vm) {
    if (!vm) return;

    BValue *p_err = var_lookup(vm->var, "ERR", true);
    if (p_err) {
        p_err->type = VAL_NUMBER;
        p_err->as.number = 0.0;
    }
    BValue *p_erl = var_lookup(vm->var, "ERL", true);
    if (p_erl) {
        p_erl->type = VAL_NUMBER;
        p_erl->as.number = 0.0;
    }

    vm_build_data_table(vm);
    vm->running = true;
    vm->jump_active = false;
    vm_clear_error(vm);
    gosub_stack_clear(vm->gosub_stack);
    for_stack_clear(vm->for_stack);
    while_stack_clear(vm->while_stack);
    do_stack_clear(vm->do_stack);

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(vm->mem, &count);
    if (count == 0) {
        vm->running = false;
        return;
    }

    if (vm->start_line > 0.0) {
        vm->current_line = vm->start_line;
        vm->start_line = 0.0;
    } else {
        vm->current_line = lines[0].line_number;
    }
    vm->current_pos = NULL;
    size_t idx_hint = 0;

    while (vm->running && !vm_has_error(vm)) {
        if (vm->jump_active) {
            vm->current_line = vm->next_line;
            vm->current_pos = vm->next_pos;
            vm->jump_active = false;
            idx_hint = (size_t)-1;
        }

        lines = mem_program_get_all(vm->mem, &count);
        size_t idx = 0;
        bool found = false;
        bool is_lib = false;

        if (idx_hint < count && lines[idx_hint].line_number == vm->current_line) {
            idx = idx_hint;
            found = true;
        }

        if (!found) {
            for (int c = 0; c < 16; ++c) {
                if (vm->jump_cache[c].valid && vm->jump_cache[c].line == vm->current_line) {
                    idx = vm->jump_cache[c].idx;
                    is_lib = vm->jump_cache[c].is_lib;
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            if (mem_program_find_line_index(vm->mem, vm->current_line, &idx)) {
                found = true;
                uint8_t slot = vm->jump_cache_head;
                vm->jump_cache[slot].line = vm->current_line;
                vm->jump_cache[slot].idx = idx;
                vm->jump_cache[slot].is_lib = false;
                vm->jump_cache[slot].valid = true;
                vm->jump_cache_head = (slot + 1) & 15;
            } else if (idx < count) {
                found = true;
                vm->current_line = lines[idx].line_number;
            }
        }

        size_t lib_count = 0;
        BppProgramLine *lib_lines = mem_lib_program_get_all(vm->mem, &lib_count);
        if (!found && lib_lines) {
            for (size_t i = 0; i < lib_count; ++i) {
                if (lib_lines[i].line_number >= vm->current_line) {
                    idx = i;
                    found = true;
                    is_lib = true;
                    vm->current_line = lib_lines[i].line_number;
                    break;
                }
            }
        }

        if (!found) {
            vm->running = false;
            break;
        }

        mem_scratch_reset(vm->mem);

        BppProgramLine *active_lines = is_lib ? lib_lines : lines;
        size_t active_count = is_lib ? lib_count : count;

        if (logger_is_trace()) {
            VDevContext *vdev = vm->vdev;
            if (vdev) {
                vdev_printf(vdev, "[Line %lld]\n", (long long)vm->current_line);
            }
            log_info("[Line %lld]", (long long)vm->current_line);
        }

        if (vm->debug_single_step) {
            char reason_buf[128];
            snprintf(reason_buf, sizeof(reason_buf), "Line %lld: %s", (long long)vm->current_line, active_lines[idx].text);
            vm_trigger_breakpoint(vm, reason_buf);
        }

        if (vm->debug_hook) {
            vm->debug_hook(vm, "line", (int)vm->current_line, NULL, vm->debug_user_data);
        }

        BppError err;
        memset(&err, 0, sizeof(err));

        if (active_lines[idx].ast_valid && active_lines[idx].ast_cache) {
            err = eval_ast_execute(vm, (EvalAstNode *)active_lines[idx].ast_cache);
            if (active_lines[idx].ast_skip_lines > 0 && !vm->jump_active) {
                idx += active_lines[idx].ast_skip_lines;
            }
        } else if (!active_lines[idx].ast_valid && active_lines[idx].ast_cache == NULL) {
            size_t skipped = 0;
            EvalAstNode *parsed_ast = eval_ast_try_compile_multiline_block(vm, active_lines, idx, active_count, &skipped);
            if (!parsed_ast) {
                parsed_ast = eval_ast_try_parse_line(vm, active_lines[idx].text);
            }
            if (parsed_ast) {
                active_lines[idx].ast_cache = parsed_ast;
                active_lines[idx].ast_valid = true;
                active_lines[idx].ast_skip_lines = skipped;
                err = eval_ast_execute(vm, parsed_ast);
                if (skipped > 0 && !vm->jump_active) {
                    idx += skipped;
                }
            } else {
                active_lines[idx].ast_valid = true;
                active_lines[idx].ast_cache = NULL;
                active_lines[idx].ast_skip_lines = 0;
                err = vm_execute_line(vm, active_lines[idx].text);
            }
        } else {
            err = vm_execute_line(vm, active_lines[idx].text);
        }
#ifndef BASIC_LITE_BUILD
        vdev_gfx_poll_events();
#endif
        if (err.code != 0) {
            if (try_stack_count(vm->try_stack) > 0) {
                vm_trigger_try_catch_handler(vm, err.code, err.message);
                memset(&vm->last_error, 0, sizeof(BppError));
                vm->jump_active = true;
                err.code = 0;
            } else if (vm->error_trap_line > 0.0 && !vm->in_error_handler) {
                BppLineNumber err_ln = (err.line != 0.0) ? err.line : vm->current_line;
                vm_trigger_error_trap(vm, err.code, err_ln, vm->current_pos, vm->next_pos);
                memset(&vm->last_error, 0, sizeof(BppError));
                vm->jump_active = true;
                err.code = 0;
            } else {
                vm->last_error = err;
                if (vm->last_error.line == 0.0) {
                    vm->last_error.line = vm->current_line;
                }
                break;
            }
        }

        if (err.code == 0) {
            vm_trigger_event_polling(vm);
        }

        if (!vm->jump_active) {
            if (idx + 1 < active_count) {
                vm->current_line = active_lines[idx + 1].line_number;
                vm->current_pos = NULL;
                idx_hint = idx + 1;
            } else {
                vm->running = false;
            }
        } else {
            idx_hint = (size_t)-1;
        }
    }

    if (vm->break_triggered) {
        vdev_printf(vm->vdev, "Break at line %lld\n", (long long)vm->current_line);
        vm->break_triggered = false;
    }

    if (vprinter_has_output()) {
        vprinter_flush_pdf(NULL);
    }
}
