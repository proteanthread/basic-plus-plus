// FILENAME: jit_vm.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast_eval_stmt_ctrl.c, jit_manager.c)
// NEEDS: libcore, libengine, libkernel
// Implements high-speed execution engine for JIT linear bytecode.
//
// ---- Includes ----

#include "vm/jit.h"
#include "vm/vm.h"
#include "eval/ast.h"
#include "eval/ast_internal.h"
#include "device/vdev.h"
#include "runtime/variables.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include <stdio.h>

BppError jit_execute_bytecode(VMContext *vm, const JitBytecodeProgram *prog) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !prog || !prog->instrs || prog->count == 0) return err;

    double stack[128];
    int sp = 0;
    size_t ip = 0;
    size_t count = prog->count;
    uint32_t ops_since_watchdog = 0;

    VDevContext *vdev = vm_get_vdev(vm);

    while (ip < count) {
        if (++ops_since_watchdog >= 10000) {
            ops_since_watchdog = 0;
            if (!vm_check_watchdog(vm, &err)) {
                return err;
            }
            if (vm_is_jump_active(vm) || (vm_get_current_line(vm) > 0.0 && !vm_is_running(vm))) break;
        }

        const JitInstr *instr = &prog->instrs[ip];

        switch (instr->opcode) {
            case JIT_OP_NOP:
                break;

            case JIT_OP_LOAD_CONST_NUM:
                if (sp < 128) stack[sp++] = instr->as.num;
                break;

            case JIT_OP_LOAD_VAR_FAST:
                if (sp < 128 && instr->as.var_ptr) {
                    stack[sp++] = *(instr->as.var_ptr);
                }
                break;

            case JIT_OP_STORE_VAR_FAST:
                if (sp > 0 && instr->as.var_ptr) {
                    *(instr->as.var_ptr) = stack[--sp];
                }
                break;

            case JIT_OP_LOAD_VAR_NAME: {
                BValue *v = var_lookup(vm_get_var(vm), instr->as.name, true);
                if (sp < 128) {
                    stack[sp++] = v ? v->as.number : 0.0;
                }
                break;
            }

            case JIT_OP_STORE_VAR_NAME: {
                if (sp > 0) {
                    BValue *v = var_lookup(vm_get_var(vm), instr->as.name, true);
                    if (v) {
                        v->type = VAL_NUMBER;
                        v->as.number = stack[--sp];
                    }
                }
                break;
            }

            case JIT_OP_ADD:
                if (sp >= 2) { stack[sp - 2] += stack[sp - 1]; sp--; }
                break;

            case JIT_OP_SUB:
                if (sp >= 2) { stack[sp - 2] -= stack[sp - 1]; sp--; }
                break;

            case JIT_OP_MUL:
                if (sp >= 2) { stack[sp - 2] *= stack[sp - 1]; sp--; }
                break;

            case JIT_OP_DIV:
                if (sp >= 2) {
                    stack[sp - 2] = (stack[sp - 1] != 0.0) ? (stack[sp - 2] / stack[sp - 1]) : 0.0;
                    sp--;
                }
                break;

            case JIT_OP_MOD:
                if (sp >= 2) {
                    int64_t b = (int64_t)stack[sp - 1];
                    stack[sp - 2] = (b != 0) ? (double)((int64_t)stack[sp - 2] % b) : 0.0;
                    sp--;
                }
                break;

            case JIT_OP_NEG:
                if (sp >= 1) stack[sp - 1] = -stack[sp - 1];
                break;

            case JIT_OP_CMP_EQ:
                if (sp >= 2) { stack[sp - 2] = (stack[sp - 2] == stack[sp - 1]) ? -1.0 : 0.0; sp--; }
                break;

            case JIT_OP_CMP_NE:
                if (sp >= 2) { stack[sp - 2] = (stack[sp - 2] != stack[sp - 1]) ? -1.0 : 0.0; sp--; }
                break;

            case JIT_OP_CMP_LT:
                if (sp >= 2) { stack[sp - 2] = (stack[sp - 2] < stack[sp - 1]) ? -1.0 : 0.0; sp--; }
                break;

            case JIT_OP_CMP_GT:
                if (sp >= 2) { stack[sp - 2] = (stack[sp - 2] > stack[sp - 1]) ? -1.0 : 0.0; sp--; }
                break;

            case JIT_OP_CMP_LE:
                if (sp >= 2) { stack[sp - 2] = (stack[sp - 2] <= stack[sp - 1]) ? -1.0 : 0.0; sp--; }
                break;

            case JIT_OP_CMP_GE:
                if (sp >= 2) { stack[sp - 2] = (stack[sp - 2] >= stack[sp - 1]) ? -1.0 : 0.0; sp--; }
                break;

            case JIT_OP_JMP:
                ip = (size_t)instr->as.target_ip;
                continue;

            case JIT_OP_JMP_IF_ZERO:
                if (sp > 0) {
                    double cond = stack[--sp];
                    if (cond == 0.0) {
                        ip = (size_t)instr->as.target_ip;
                        continue;
                    }
                }
                break;

            case JIT_OP_JMP_IF_NOT_ZERO:
                if (sp > 0) {
                    double cond = stack[--sp];
                    if (cond != 0.0) {
                        ip = (size_t)instr->as.target_ip;
                        continue;
                    }
                }
                break;

            case JIT_OP_FOR_STEP: {
                double *c_ptr = instr->as.for_info.counter_ptr;
                double step = instr->as.for_info.step;
                double limit = instr->as.for_info.limit;
                if (c_ptr) {
                    *c_ptr += step;
                    bool active = (step >= 0.0) ? (*c_ptr <= limit) : (*c_ptr >= limit);
                    if (active) {
                        ip = (size_t)instr->as.for_info.loop_body_ip;
                        continue;
                    }
                }
                break;
            }

            case JIT_OP_PRINT_NUM:
                if (sp > 0) {
                    double v = stack[--sp];
                    if (vdev) vdev_printf(vdev, "%g", v);
                    else printf("%g", v);
                }
                break;

            case JIT_OP_PRINT_NL:
                if (vdev) vdev_printf(vdev, "\n");
                else printf("\n");
                break;

            case JIT_OP_HALT:
                ip = count;
                continue;

            default:
                break;
        }

        ip++;
    }

    return err;
}

BppError jit_execute_for_loop(VMContext *vm, EvalAstNode *for_node, bool *handled) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (handled) *handled = false;
    if (!vm || !for_node || for_node->type != AST_NODE_FOR_LOOP) return err;

    int mode = jit_get_mode(vm);
    if (mode == JIT_MODE_OFF) return err;

    // Evaluate dynamic limit and step
    double limit = eval_ast_eval_num(vm, for_node->right, &err);
    if (err.code != 0) return err;
    double step = 1.0;
    if (for_node->condition) {
        step = eval_ast_eval_num(vm, for_node->condition, &err);
        if (err.code != 0) return err;
    }

    // Check if JIT bytecode is already compiled and cached on target_ast
    JitBytecodeProgram *prog = (JitBytecodeProgram *)for_node->target_ast;
    if (!prog) {
        prog = jit_compile_ast(vm, for_node);
        if (prog) {
            for_node->target_ast = (EvalAstNode *)prog;
        }
    }

    if (prog) {
        if (!for_node->cached_var_ptr && vm) {
            for_node->cached_var_ptr = var_lookup(vm_get_var(vm), for_node->var_name, true);
        }
        double *my_counter = for_node->cached_var_ptr ? &for_node->cached_var_ptr->as.number : NULL;

        // Update dynamic limit and step on matching JIT_OP_FOR_STEP instructions
        for (size_t i = 0; i < prog->count; i++) {
            if (prog->instrs[i].opcode == JIT_OP_FOR_STEP) {
                if (!my_counter || prog->instrs[i].as.for_info.counter_ptr == my_counter) {
                    prog->instrs[i].as.for_info.limit = limit;
                    prog->instrs[i].as.for_info.step = step;
                }
            }
        }
        if (handled) *handled = true;
        err = jit_execute_bytecode(vm, prog);
    }

    return err;
}
