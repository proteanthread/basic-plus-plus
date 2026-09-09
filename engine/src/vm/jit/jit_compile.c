// FILENAME: jit_compile.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (jit_manager.c, jit_vm.c)
// NEEDS: libcore, libengine, libkernel
// Implements AST to linear bytecode translation for the JIT compiler.
//
// ---- Includes ----

#include "vm/jit.h"
#include "vm/vm.h"
#include "eval/ast.h"
#include "eval/ast_internal.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/variables.h"
#include "runtime/variables_internal.h"

static int emit_instr(JitBytecodeProgram *prog, JitOpcode op, int line_num) {
    if (!prog) return -1;
    if (prog->count >= prog->capacity) {
        size_t new_cap = prog->capacity == 0 ? 64 : prog->capacity * 2;
        JitInstr *new_instrs = (JitInstr *)runtime_calloc(new_cap, sizeof(JitInstr));
        if (!new_instrs) return -1;
        if (prog->instrs && prog->count > 0) {
            runtime_memcpy(new_instrs, prog->instrs, prog->count * sizeof(JitInstr));
            runtime_free(prog->instrs);
        }
        prog->instrs = new_instrs;
        prog->capacity = new_cap;
    }
    int idx = (int)prog->count++;
    runtime_memset(&prog->instrs[idx], 0, sizeof(JitInstr));
    prog->instrs[idx].opcode = op;
    prog->instrs[idx].line_num = line_num;
    return idx;
}

static bool compile_expr(VMContext *vm, JitBytecodeProgram *prog, EvalAstNode *node) {
    if (!node) return false;

    switch (node->type) {
        case AST_NODE_LITERAL: {
            int idx = emit_instr(prog, JIT_OP_LOAD_CONST_NUM, 0);
            if (idx < 0) return false;
            prog->instrs[idx].as.num = node->val.as.number;
            return true;
        }
        case AST_NODE_VARIABLE: {
            if (!node->cached_var_ptr && vm) {
                node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
            }
            if (node->cached_var_ptr) {
                int idx = emit_instr(prog, JIT_OP_LOAD_VAR_FAST, 0);
                if (idx < 0) return false;
                prog->instrs[idx].as.var_ptr = &node->cached_var_ptr->as.number;
            } else {
                int idx = emit_instr(prog, JIT_OP_LOAD_VAR_NAME, 0);
                if (idx < 0) return false;
                runtime_strncpy(prog->instrs[idx].as.name, node->var_name, sizeof(prog->instrs[idx].as.name) - 1);
            }
            return true;
        }
        case AST_NODE_BINARY_OP: {
            if (!compile_expr(vm, prog, node->left)) return false;
            if (!compile_expr(vm, prog, node->right)) return false;
            JitOpcode op = JIT_OP_NOP;
            switch (node->op) {
                case TOK_PLUS:       op = JIT_OP_ADD; break;
                case TOK_MINUS:      op = JIT_OP_SUB; break;
                case TOK_MUL:        op = JIT_OP_MUL; break;
                case TOK_DIV:        op = JIT_OP_DIV; break;
                case TOK_BACKSLASH:  op = JIT_OP_DIV; break;
                case TOK_MOD:        op = JIT_OP_MOD; break;
                case TOK_EQ:         op = JIT_OP_CMP_EQ; break;
                case TOK_NE:         op = JIT_OP_CMP_NE; break;
                case TOK_LT:         op = JIT_OP_CMP_LT; break;
                case TOK_GT:         op = JIT_OP_CMP_GT; break;
                case TOK_LE:         op = JIT_OP_CMP_LE; break;
                case TOK_GE:         op = JIT_OP_CMP_GE; break;
                default:             return false;
            }
            return (emit_instr(prog, op, 0) >= 0);
        }
        case AST_NODE_UNARY_OP: {
            if (!compile_expr(vm, prog, node->left)) return false;
            if (node->op == TOK_MINUS) {
                return (emit_instr(prog, JIT_OP_NEG, 0) >= 0);
            }
            return true;
        }
        default:
            return false;
    }
}

static bool compile_stmt(VMContext *vm, JitBytecodeProgram *prog, EvalAstNode *node);

static bool compile_stmt_list(VMContext *vm, JitBytecodeProgram *prog, EvalAstNode *head) {
    EvalAstNode *curr = head;
    while (curr) {
        if (!compile_stmt(vm, prog, curr)) return false;
        curr = curr->next;
    }
    return true;
}

static bool compile_stmt(VMContext *vm, JitBytecodeProgram *prog, EvalAstNode *node) {
    if (!node) return true;

    switch (node->type) {
        case AST_NODE_ASSIGNMENT: {
            if (!compile_expr(vm, prog, node->right)) return false;
            if (!node->cached_var_ptr && vm) {
                node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
            }
            if (node->cached_var_ptr) {
                node->cached_var_ptr->type = VAL_NUMBER;
                int idx = emit_instr(prog, JIT_OP_STORE_VAR_FAST, 0);
                if (idx < 0) return false;
                prog->instrs[idx].as.var_ptr = &node->cached_var_ptr->as.number;
            } else {
                int idx = emit_instr(prog, JIT_OP_STORE_VAR_NAME, 0);
                if (idx < 0) return false;
                runtime_strncpy(prog->instrs[idx].as.name, node->var_name, sizeof(prog->instrs[idx].as.name) - 1);
            }
            return true;
        }
        case AST_NODE_IF: {
            if (!compile_expr(vm, prog, node->condition)) return false;
            int jmp_false_idx = emit_instr(prog, JIT_OP_JMP_IF_ZERO, 0);
            if (jmp_false_idx < 0) return false;

            if (node->then_branch) {
                if (!compile_stmt_list(vm, prog, node->then_branch)) return false;
            }

            int jmp_exit_idx = -1;
            if (node->else_branch) {
                jmp_exit_idx = emit_instr(prog, JIT_OP_JMP, 0);
            }

            prog->instrs[jmp_false_idx].as.target_ip = (int)prog->count;

            if (node->else_branch) {
                if (!compile_stmt_list(vm, prog, node->else_branch)) return false;
                if (jmp_exit_idx >= 0) {
                    prog->instrs[jmp_exit_idx].as.target_ip = (int)prog->count;
                }
            }
            return true;
        }
        case AST_NODE_PRINT: {
            if (node->left) {
                if (!compile_expr(vm, prog, node->left)) return false;
                emit_instr(prog, JIT_OP_PRINT_NUM, 0);
            }
            emit_instr(prog, JIT_OP_PRINT_NL, 0);
            return true;
        }
        case AST_NODE_FOR_LOOP: {
            // Initial assignment
            if (!compile_expr(vm, prog, node->left)) return false;
            if (!node->cached_var_ptr && vm) {
                node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
            }
            if (!node->cached_var_ptr) return false;
            node->cached_var_ptr->type = VAL_NUMBER;
            int init_idx = emit_instr(prog, JIT_OP_STORE_VAR_FAST, 0);
            if (init_idx < 0) return false;
            prog->instrs[init_idx].as.var_ptr = &node->cached_var_ptr->as.number;

            double limit = 0.0;
            if (node->right) {
                if (node->right->type == AST_NODE_LITERAL) {
                    limit = node->right->val.as.number;
                } else if (vm) {
                    BppError dummy;
                    runtime_memset(&dummy, 0, sizeof(dummy));
                    limit = eval_ast_eval_num(vm, node->right, &dummy);
                }
            }

            double step = 1.0;
            if (node->condition) {
                if (node->condition->type == AST_NODE_LITERAL) {
                    step = node->condition->val.as.number;
                } else if (vm) {
                    BppError dummy;
                    runtime_memset(&dummy, 0, sizeof(dummy));
                    step = eval_ast_eval_num(vm, node->condition, &dummy);
                }
            }

            int body_ip = (int)prog->count;
            if (node->then_branch) {
                if (!compile_stmt_list(vm, prog, node->then_branch)) return false;
            }

            int step_idx = emit_instr(prog, JIT_OP_FOR_STEP, 0);
            if (step_idx < 0) return false;
            prog->instrs[step_idx].as.for_info.counter_ptr = &node->cached_var_ptr->as.number;
            prog->instrs[step_idx].as.for_info.step = step;
            prog->instrs[step_idx].as.for_info.limit = limit;
            prog->instrs[step_idx].as.for_info.loop_body_ip = body_ip;
            return true;
        }
        default:
            return false;
    }
}

JitBytecodeProgram *jit_compile_ast(VMContext *vm, EvalAstNode *ast) {
    if (!ast) return NULL;
    JitBytecodeProgram *prog = (JitBytecodeProgram *)runtime_calloc(1, sizeof(JitBytecodeProgram));
    if (!prog) return NULL;

    if (!compile_stmt_list(vm, prog, ast)) {
        jit_free_bytecode(prog);
        return NULL;
    }
    emit_instr(prog, JIT_OP_HALT, 0);
    return prog;
}

void jit_free_bytecode(JitBytecodeProgram *prog) {
    if (!prog) return;
    if (prog->instrs) {
        runtime_free(prog->instrs);
        prog->instrs = NULL;
    }
    runtime_free(prog);
}
