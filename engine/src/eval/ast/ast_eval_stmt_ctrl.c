// FILENAME: ast_eval_stmt_ctrl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast_eval_stmt.c)
// NEEDS: libcore, libengine (ast_internal.h)
// AST control flow evaluation and subroutine compilation helpers.
//
// ---- Includes ----

#include "eval/ast_internal.h"
#include "runtime/variables.h"
#include "runtime/variables_internal.h"
#include "vm/vm.h"
#include "vm/jit.h"

// compiles a contiguous subroutine into a linked list of AST nodes
EvalAstNode *eval_ast_compile_subroutine(VMContext *vm, const BppProgramLine *lines, size_t count, BppLineNumber target_line) {
    size_t start_idx = 0;
    bool found = false;
    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number == target_line) {
            start_idx = i;
            found = true;
            break;
        }
    }
    if (!found) return NULL;

    EvalAstNode *head = NULL;
    EvalAstNode *tail = NULL;

    for (size_t i = start_idx; i < count; i++) {
        const char *stext = skip_line_number_and_spaces(lines[i].text);
        if (!stext || *stext == '\0' || *stext == '\'' || runtime_strncasecmp(stext, "REM", 3) == 0) continue;

        size_t skipped = 0;
        EvalAstNode *node = eval_ast_try_compile_multiline_block(vm, lines, i, count, &skipped);
        if (node) {
            if (!head) head = node;
            else tail->next = node;
            while (node->next) node = node->next;
            tail = node;
            i += skipped;
            continue;
        }

        size_t if_skipped = 0;
        node = eval_ast_try_compile_multiline_if(vm, lines, i, count, &if_skipped);
        if (node) {
            if (!head) head = node;
            else tail->next = node;
            while (node->next) node = node->next;
            tail = node;
            i += if_skipped;
            continue;
        }

        node = eval_ast_try_parse_line(vm, lines[i].text);
        if (!node) {
            if (head) eval_ast_free_tree(head);
            return NULL;
        }
        if (!head) head = node;
        else tail->next = node;
        while (node->next) node = node->next;
        tail = node;

        if (runtime_strncasecmp(stext, "RETURN", 6) == 0 && (stext[6] == '\0' || stext[6] == ' ' || stext[6] == '\t' || stext[6] == ':')) {
            break;
        }
    }
    return head;
}

// executes an AST control-flow node
bool eval_ast_exec_ctrl(VMContext *vm, EvalAstNode *node, BppError *err) {
    if (!vm || !node) return false;

    switch (node->type) {
        case AST_NODE_IF: {
            BValue cval = eval_ast_evaluate(vm, node->condition, err);
            if (err->code != 0) return true;
            double cond = (cval.type == VAL_NUMBER || cval.type == VAL_INTEGER) ? cval.as.number : 0.0;
            EvalAstNode *branch = (cond != 0.0) ? node->then_branch : node->else_branch;
            if (branch) {
                if (branch->next == NULL && branch->type == AST_NODE_ASSIGNMENT &&
                    !((branch->var_name[0] != '\0' && branch->var_name[runtime_strlen(branch->var_name) - 1] == '$') ||
                      (branch->cached_var_ptr && branch->cached_var_ptr->type == VAL_STRING) ||
                      is_ast_string_node(branch->right))) {
                    if (!branch->cached_var_ptr) {
                        DynamicVarEntry *dvar = var_find_dynamic(vm_get_var(vm), branch->var_name);
                        if (!dvar || (!dvar->setter && dvar->write_fn[0] == '\0')) {
                            branch->cached_var_ptr = var_lookup(vm_get_var(vm), branch->var_name, true);
                        }
                    }
                    if (branch->cached_var_ptr) {
                        double num = eval_ast_eval_num(vm, branch->right, err);
                        if (err->code != 0) return true;
                        if (branch->cached_var_ptr->type == VAL_INTEGER) {
                            branch->cached_var_ptr->as.number = (double)((int32_t)num);
                        } else {
                            branch->cached_var_ptr->type = VAL_NUMBER;
                            branch->cached_var_ptr->as.number = num;
                        }
                    } else {
                        *err = eval_ast_execute(vm, branch);
                    }
                } else {
                    *err = eval_ast_execute(vm, branch);
                }
            }
            return true;
        }
        case AST_NODE_GOTO: {
            vm_jump(vm, node->target_line, NULL);
            return true;
        }
        case AST_NODE_GOSUB: {
            if (!node->target_ast) {
                size_t count = 0;
                BppProgramLine *lines = mem_program_get_all(vm_get_mem(vm), &count);
                if (lines) {
                    node->target_ast = eval_ast_compile_subroutine(vm, lines, count, node->target_line);
                }
            }
            if (node->target_ast) {
                *err = eval_ast_execute(vm, node->target_ast);
                return true;
            }
            const char *resume_pos = node->next ? node->next->source_pos : (node->source_pos ? (node->source_pos + runtime_strlen(node->source_pos)) : NULL);
            if (!vm_gosub_push(vm, vm_get_current_line(vm), resume_pos)) {
                err->code = 12; err->message = "Subroutine nesting limit exceeded";
                return true;
            }
            vm_jump(vm, node->target_line, NULL);
            return true;
        }
        case AST_NODE_RETURN: {
            BppLineNumber ret_line = 0;
            const char *ret_pos = NULL;
            if (!vm_gosub_pop(vm, &ret_line, &ret_pos)) {
                return true;
            }
            vm_jump(vm, ret_line, ret_pos);
            return true;
        }
        case AST_NODE_FOR_LOOP: {
            double start = eval_ast_eval_num(vm, node->left, err);
            if (err->code != 0) return true;
            double end = eval_ast_eval_num(vm, node->right, err);
            if (err->code != 0) return true;
            double step = 1.0;
            if (node->condition) {
                step = eval_ast_eval_num(vm, node->condition, err);
                if (err->code != 0) return true;
            }
            if (!node->cached_var_ptr) {
                node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
            }
            if (!node->cached_var_ptr) {
                err->code = 7; err->message = "Out of memory in FOR loop";
                return true;
            }
            node->cached_var_ptr->type = VAL_NUMBER;
            double c = start;
            double *counter = &node->cached_var_ptr->as.number;
            *counter = c;

            bool jit_handled = false;
            if (jit_get_mode(vm) != JIT_MODE_OFF) {
                *err = jit_execute_for_loop(vm, node, &jit_handled);
                if (jit_handled) return true;
            }

            if (step >= 0.0) {
                while (c <= end) {
                    if (!vm_is_running(vm) || vm_is_jump_active(vm)) break;
                    if (node->then_branch) {
                        *err = eval_ast_execute(vm, node->then_branch);
                        if (err->code != 0 || vm_is_jump_active(vm)) break;
                    }
                    c += step;
                    *counter = c;
                }
            } else {
                while (c >= end) {
                    if (!vm_is_running(vm) || vm_is_jump_active(vm)) break;
                    if (node->then_branch) {
                        *err = eval_ast_execute(vm, node->then_branch);
                        if (err->code != 0 || vm_is_jump_active(vm)) break;
                    }
                    c += step;
                    *counter = c;
                }
            }
            return true;
        }
        case AST_NODE_WHILE_LOOP: {
            while (vm_is_running(vm) && !vm_is_jump_active(vm)) {
                double cond = eval_ast_eval_num(vm, node->condition, err);
                if (err->code != 0) return true;
                if (cond == 0.0) break;

                if (node->then_branch) {
                    *err = eval_ast_execute(vm, node->then_branch);
                    if (err->code != 0 || vm_is_jump_active(vm)) break;
                }
            }
            return true;
        }
        default:
            return false;
    }
}
