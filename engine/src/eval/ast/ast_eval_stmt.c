// FILENAME: ast_eval_stmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements AST parsing and evaluation structures for ast_eval_stmt.
//
// ---- Includes ----

#include "bios/bios.h"
#include "device/bus.h"
#include "device/vdev.h"
#include "eval/ast_internal.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "runtime/arrays.h"
#include "runtime/file.h"
#include "runtime/num_format.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "vm/vm.h"

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

// executes an AST statement tree in the VM runtime
BppError eval_ast_execute(VMContext *vm, EvalAstNode *head) {
    BppError err;
    EvalAstNode *node = head;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !node) return err;

    const char *cur_pos = vm_get_current_pos(vm);
    if (cur_pos && node && node->source_pos) {
        const char *line_start = node->source_pos;
        EvalAstNode *last = node;
        while (last->next) last = last->next;
        const char *line_end = last->source_pos ? (last->source_pos + runtime_strlen(last->source_pos)) : line_start;

        if (cur_pos >= line_start && cur_pos <= line_end) {
            while (node && node->source_pos && node->source_pos < cur_pos) {
                node = node->next;
            }
        }
        vm_set_current_pos(vm, NULL);
    }

    while (node && err.code == 0 && !vm_is_jump_active(vm) && vm_is_running(vm)) {
        if (!vm_check_watchdog(vm, &err)) {
            return err;
        }

        switch (node->type) {
            case AST_NODE_ASSIGNMENT: {
                if (!node->cached_var_ptr) {
                    node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
                }

                if (node->right && node->right->type == AST_NODE_BINARY_OP && node->right->op == TOK_PLUS &&
                    node->right->left && node->right->left->type == AST_NODE_VARIABLE &&
                    runtime_strcmp(node->right->left->var_name, node->var_name) == 0 &&
                    node->cached_var_ptr && node->cached_var_ptr->type == VAL_STRING) {

                    BValue rval = eval_ast_evaluate(vm, node->right->right, &err);
                    if (err.code != 0) return err;
                    if (rval.type == VAL_STRING && rval.as.string) {
                        node->cached_var_ptr->as.string = str_append_inplace(
                            vm_get_str(vm),
                            node->cached_var_ptr->as.string,
                            str_data(rval.as.string),
                            str_len(rval.as.string)
                        );
                        str_release(vm_get_str(vm), rval.as.string);
                        break;
                    } else if (rval.type == VAL_STRING) {
                        break;
                    }
                }

                if (node->right && node->right->type == AST_NODE_BINARY_OP && node->right->op == TOK_PLUS &&
                    node->var_name[runtime_strlen(node->var_name) - 1] == '$') {
                    StringContext *sc = vm_get_str(vm);
                    const char *parts[16];
                    size_t lens[16];
                    char num_bufs[16][64];
                    size_t num_buf_idx = 0;
                    BppString *to_rel[16];
                    size_t scount = 0, rcount = 0;


                    eval_ast_flatten_concat(vm, node->right->left, parts, lens, num_bufs, &num_buf_idx, to_rel, &scount, &rcount, 16, &err);
                    if (err.code == 0) {
                        eval_ast_flatten_concat(vm, node->right->right, parts, lens, num_bufs, &num_buf_idx, to_rel, &scount, &rcount, 16, &err);
                    }
                    if (err.code == 0) {
                        if (!node->cached_var_ptr) {
                            node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
                        }
                        if (node->cached_var_ptr && node->cached_var_ptr->type == VAL_STRING) {
                            node->cached_var_ptr->as.string = str_concat_multi_inplace(sc, node->cached_var_ptr->as.string, parts, lens, scount);
                        } else {
                            BppString *res_str = str_concat_multi(sc, parts, lens, scount);
                            if (node->cached_var_ptr) {
                                node->cached_var_ptr->type = VAL_STRING;
                                node->cached_var_ptr->as.string = res_str;
                            } else {
                                BValue val = { .type = VAL_STRING, .as.string = res_str };
                                var_assign(vm_get_var(vm), node->var_name, val);
                                str_release(sc, res_str);
                            }
                        }
                        for (size_t i = 0; i < rcount; i++) str_release(sc, to_rel[i]);
                        break;
                    }
                    for (size_t i = 0; i < rcount; i++) str_release(sc, to_rel[i]);
                    if (err.code != 0) return err;
                }

                BValue val = eval_ast_evaluate(vm, node->right, &err);
                if (err.code != 0) return err;

                if (node->cached_var_ptr && (val.type == VAL_NUMBER || val.type == VAL_INTEGER)) {
                    if (node->cached_var_ptr->type == VAL_STRING && node->cached_var_ptr->as.string) {
                        str_release(vm_get_str(vm), node->cached_var_ptr->as.string);
                        node->cached_var_ptr->type = VAL_NUMBER;
                    }
                    if (node->cached_var_ptr->type == VAL_INTEGER) {
                        node->cached_var_ptr->as.number = (double)((int32_t)val.as.number);
                    } else {
                        node->cached_var_ptr->type = VAL_NUMBER;
                        node->cached_var_ptr->as.number = val.as.number;
                    }
                } else if (node->cached_var_ptr && val.type == VAL_STRING) {
                    if (node->cached_var_ptr->type == VAL_STRING && node->cached_var_ptr->as.string) {
                        str_release(vm_get_str(vm), node->cached_var_ptr->as.string);
                    }
                    node->cached_var_ptr->type = VAL_STRING;
                    node->cached_var_ptr->as.string = val.as.string;
                } else {
                    var_assign(vm_get_var(vm), node->var_name, val);
                    if (val.type == VAL_STRING && val.as.string) {
                        str_release(vm_get_str(vm), val.as.string);
                    }
                }
                break;
            }
            case AST_NODE_ARRAY_ASSIGN: {
                BValue ival = eval_ast_evaluate(vm, node->index_expr, &err);
                if (err.code != 0) return err;
                BValue val = eval_ast_evaluate(vm, node->right, &err);
                if (err.code != 0) return err;
                int idx = (int)((ival.type == VAL_NUMBER || ival.type == VAL_INTEGER) ? ival.as.number : 0);
                int indices[1] = { idx };
                BValue *elem = arr_get_element(vm_get_arr(vm), node->var_name, 1, indices, &err);
                if (err.code == 0 && elem) {
                    if (elem->type == VAL_STRING && elem->as.string) {
                        str_release(vm_get_str(vm), elem->as.string);
                    }
                    *elem = val;
                }
                break;
            }
            case AST_NODE_ARRAY2D_ASSIGN: {
                BValue ival1 = eval_ast_evaluate(vm, node->index_expr, &err);
                if (err.code != 0) return err;
                BValue ival2 = eval_ast_evaluate(vm, node->index2_expr, &err);
                if (err.code != 0) return err;
                BValue val = eval_ast_evaluate(vm, node->right, &err);
                if (err.code != 0) return err;
                int idx1 = (int)((ival1.type == VAL_NUMBER || ival1.type == VAL_INTEGER) ? ival1.as.number : 0);
                int idx2 = (int)((ival2.type == VAL_NUMBER || ival2.type == VAL_INTEGER) ? ival2.as.number : 0);
                int indices[2] = { idx1, idx2 };
                BValue *elem = arr_get_element(vm_get_arr(vm), node->var_name, 2, indices, &err);
                if (err.code == 0 && elem) {
                    if (elem->type == VAL_STRING && elem->as.string) {
                        str_release(vm_get_str(vm), elem->as.string);
                    }
                    *elem = val;
                }
                break;
            }
            case AST_NODE_IF: {
                BValue cond = eval_ast_evaluate(vm, node->condition, &err);
                if (err.code != 0) return err;
                bool is_true = (cond.type == VAL_NUMBER || cond.type == VAL_INTEGER) && (cond.as.number != 0.0);
                if (is_true) {
                    if (node->then_branch) {
                        err = eval_ast_execute(vm, node->then_branch);
                    }
                } else {
                    if (node->else_branch) {
                        err = eval_ast_execute(vm, node->else_branch);
                    }
                }
                break;
            }
            case AST_NODE_GOTO: {
                vm_jump(vm, node->target_line, NULL);
                return err;
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
                    err = eval_ast_execute(vm, node->target_ast);
                    if (err.code != 0) return err;
                    break;
                }
                const char *resume_pos = node->next ? node->next->source_pos : (node->source_pos ? (node->source_pos + runtime_strlen(node->source_pos)) : NULL);
                if (!vm_gosub_push(vm, vm_get_current_line(vm), resume_pos)) {
                    err.code = 12; err.message = "Subroutine nesting limit exceeded";
                    return err;
                }
                vm_jump(vm, node->target_line, NULL);
                return err;
            }
            case AST_NODE_RETURN: {
                BppLineNumber ret_line = 0;
                const char *ret_pos = NULL;
                if (!vm_gosub_pop(vm, &ret_line, &ret_pos)) {
                    return err;
                }
                vm_jump(vm, ret_line, ret_pos);
                return err;
            }
            case AST_NODE_FOR_LOOP: {
                BValue sval = eval_ast_evaluate(vm, node->left, &err);
                if (err.code != 0) return err;
                BValue evalue = eval_ast_evaluate(vm, node->right, &err);
                if (err.code != 0) return err;
                double step = 1.0;
                if (node->condition) {
                    BValue stepval = eval_ast_evaluate(vm, node->condition, &err);
                    if (err.code != 0) return err;
                    step = (stepval.type == VAL_NUMBER || stepval.type == VAL_INTEGER) ? stepval.as.number : 1.0;
                }
                if (!node->cached_var_ptr) {
                    node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
                }
                if (!node->cached_var_ptr) {
                    err.code = 7; err.message = "Out of memory in FOR loop";
                    return err;
                }
                node->cached_var_ptr->type = VAL_NUMBER;
                double *counter = &node->cached_var_ptr->as.number;
                double end = (evalue.type == VAL_NUMBER || evalue.type == VAL_INTEGER) ? evalue.as.number : 0.0;
                double start = (sval.type == VAL_NUMBER || sval.type == VAL_INTEGER) ? sval.as.number : 0.0;

                for (*counter = start; step >= 0 ? (*counter <= end) : (*counter >= end); *counter += step) {
                    if (!vm_check_watchdog(vm, &err)) return err;
                    if (!vm_is_running(vm) || vm_is_jump_active(vm)) break;

                    if (node->then_branch) {
                        err = eval_ast_execute(vm, node->then_branch);
                        if (err.code != 0 || vm_is_jump_active(vm)) break;
                    }
                }
                break;
            }
            case AST_NODE_WHILE_LOOP: {
                while (vm_is_running(vm) && !vm_is_jump_active(vm)) {
                    if (!vm_check_watchdog(vm, &err)) return err;
                    BValue cond = eval_ast_evaluate(vm, node->condition, &err);
                    if (err.code != 0) return err;
                    bool is_true = (cond.type == VAL_NUMBER || cond.type == VAL_INTEGER) && (cond.as.number != 0.0);
                    if (!is_true) break;

                    if (node->then_branch) {
                        err = eval_ast_execute(vm, node->then_branch);
                        if (err.code != 0 || vm_is_jump_active(vm)) break;
                    }
                }
                break;
            }
            case AST_NODE_PRINT: {
                VDevContext *vdev = vm_get_vdev(vm);
                if (node->left) {
                    BValue val = eval_ast_evaluate(vm, node->left, &err);
                    if (err.code != 0) return err;
                    if (val.type == VAL_STRING && val.as.string) {
                        vdev_puts(vdev, str_data(val.as.string));
                        str_release(vm_get_str(vm), val.as.string);
                    } else if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                        char nbuf[64];
                        num_format_display(nbuf, sizeof(nbuf), val.as.number, true, true);
                        vdev_puts(vdev, nbuf);
                    }
                }
                if (node->op == TOK_EOL || node->op == 0) {
                    vdev_puts(vdev, "\n");
                } else if (node->op == TOK_COMMA) {
                    vdev_puts(vdev, "\t");
                }
                break;
            }
            case AST_NODE_POKE: {
                BValue addr_val = eval_ast_evaluate(vm, node->left, &err);
                if (err.code != 0) return err;
                BValue val_val = eval_ast_evaluate(vm, node->right, &err);
                if (err.code != 0) return err;
                int addr = (int)((addr_val.type == VAL_NUMBER || addr_val.type == VAL_INTEGER) ? addr_val.as.number : 0);
                uint8_t byte_val = (uint8_t)((val_val.type == VAL_NUMBER || val_val.type == VAL_INTEGER) ? (int)val_val.as.number : 0);
                if (addr_val.type == VAL_STRING && addr_val.as.string) str_release(vm_get_str(vm), addr_val.as.string);
                if (val_val.type == VAL_STRING && val_val.as.string) str_release(vm_get_str(vm), val_val.as.string);
                if (vm_get_bios(vm)) {
                    bios_poke(vm_get_bios(vm), (uint32_t)addr, byte_val);
                } else {
                    bool intercepted = false;
                    vdev_bus_poke((uint32_t)addr, byte_val, &intercepted);
                }
                break;
            }
            case AST_NODE_FILE_PRINT: {
                char print_buf[1024];
                size_t ppos = 0;
                int chan = node->channel;
                FileContext *fc = vm_get_file(vm);
                while (node && node->type == AST_NODE_FILE_PRINT && node->channel == chan) {
                    if (node->left) {
                        if (node->left->type == AST_NODE_LITERAL && node->left->val.type == VAL_STRING && node->left->val.as.string) {
                            const char *sdata = str_data(node->left->val.as.string);
                            size_t slen = str_len(node->left->val.as.string);
                            if (ppos + slen < sizeof(print_buf) - 2) {
                                runtime_memcpy(print_buf + ppos, sdata, slen);
                                ppos += slen;
                            } else {
                                if (ppos > 0) file_write_raw(fc, chan, print_buf, ppos);
                                file_write_raw(fc, chan, sdata, slen);
                                ppos = 0;
                            }
                        } else if (node->left->type == AST_NODE_VARIABLE) {
                            if (!node->left->cached_var_ptr) {
                                node->left->cached_var_ptr = var_lookup(vm_get_var(vm), node->left->var_name, false);
                            }
                            BValue *vptr = node->left->cached_var_ptr;
                            if (vptr && (vptr->type == VAL_NUMBER || vptr->type == VAL_INTEGER)) {
                                char num_buf[64];
                                size_t nlen = ast_format_num(num_buf, vptr->as.number);
                                if (ppos + nlen < sizeof(print_buf) - 2) {
                                    runtime_memcpy(print_buf + ppos, num_buf, nlen);
                                    ppos += nlen;
                                } else {
                                    if (ppos > 0) file_write_raw(fc, chan, print_buf, ppos);
                                    file_write_raw(fc, chan, num_buf, nlen);
                                    ppos = 0;
                                }
                            } else if (vptr && vptr->type == VAL_STRING && vptr->as.string) {
                                const char *sdata = str_data(vptr->as.string);
                                size_t slen = str_len(vptr->as.string);
                                if (ppos + slen < sizeof(print_buf) - 2) {
                                    runtime_memcpy(print_buf + ppos, sdata, slen);
                                    ppos += slen;
                                } else {
                                    if (ppos > 0) file_write_raw(fc, chan, print_buf, ppos);
                                    file_write_raw(fc, chan, sdata, slen);
                                    ppos = 0;
                                }
                            } else {
                                BValue val = eval_ast_evaluate(vm, node->left, &err);
                                if (err.code != 0) return err;
                                if (val.type == VAL_STRING && val.as.string) {
                                    size_t slen = str_len(val.as.string);
                                    if (ppos + slen < sizeof(print_buf) - 2) {
                                        runtime_memcpy(print_buf + ppos, str_data(val.as.string), slen);
                                        ppos += slen;
                                    } else {
                                        if (ppos > 0) file_write_raw(fc, chan, print_buf, ppos);
                                        file_write_raw(fc, chan, str_data(val.as.string), slen);
                                        ppos = 0;
                                    }
                                    str_release(vm_get_str(vm), val.as.string);
                                } else if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                                    char num_buf[64];
                                    size_t nlen = ast_format_num(num_buf, val.as.number);
                                    if (ppos + nlen < sizeof(print_buf) - 2) {
                                        runtime_memcpy(print_buf + ppos, num_buf, nlen);
                                        ppos += nlen;
                                    } else {
                                        if (ppos > 0) file_write_raw(fc, chan, print_buf, ppos);
                                        file_write_raw(fc, chan, num_buf, nlen);
                                        ppos = 0;
                                    }
                                }
                            }
                        } else if (node->left->type == AST_NODE_BINARY_OP && node->left->op == TOK_MUL &&
                                   node->left->left && node->left->left->type == AST_NODE_VARIABLE &&
                                   node->left->right && node->left->right->type == AST_NODE_LITERAL) {
                            if (!node->left->left->cached_var_ptr) {
                                node->left->left->cached_var_ptr = var_lookup(vm_get_var(vm), node->left->left->var_name, false);
                            }
                            BValue *vp = node->left->left->cached_var_ptr;
                            double v1 = vp ? vp->as.number : 0.0;
                            double v2 = node->left->right->val.as.number;
                            double res_num = v1 * v2;
                            char num_buf[64];
                            size_t nlen = ast_format_num(num_buf, res_num);
                            if (ppos + nlen < sizeof(print_buf) - 2) {
                                runtime_memcpy(print_buf + ppos, num_buf, nlen);
                                ppos += nlen;
                            } else {
                                if (ppos > 0) file_write_raw(fc, chan, print_buf, ppos);
                                file_write_raw(fc, chan, num_buf, nlen);
                                ppos = 0;
                            }
                        } else {
                            BValue val = eval_ast_evaluate(vm, node->left, &err);
                            if (err.code != 0) return err;
                            if (val.type == VAL_STRING && val.as.string) {
                                size_t slen = str_len(val.as.string);
                                if (ppos + slen < sizeof(print_buf) - 2) {
                                    runtime_memcpy(print_buf + ppos, str_data(val.as.string), slen);
                                    ppos += slen;
                                } else {
                                    if (ppos > 0) file_write_raw(fc, chan, print_buf, ppos);
                                    file_write_raw(fc, chan, str_data(val.as.string), slen);
                                    ppos = 0;
                                }
                                str_release(vm_get_str(vm), val.as.string);
                            } else if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                                char num_buf[64];
                                size_t nlen = ast_format_num(num_buf, val.as.number);
                                if (ppos + nlen < sizeof(print_buf) - 2) {
                                    runtime_memcpy(print_buf + ppos, num_buf, nlen);
                                    ppos += nlen;
                                } else {
                                    if (ppos > 0) file_write_raw(fc, chan, print_buf, ppos);
                                    file_write_raw(fc, chan, num_buf, nlen);
                                    ppos = 0;
                                }
                            }
                        }
                    }
                    if (node->op == TOK_EOL || node->op == 0) {
                        if (ppos < sizeof(print_buf) - 1) print_buf[ppos++] = '\n';
                    } else if (node->op == TOK_COMMA) {
                        if (ppos < sizeof(print_buf) - 1) print_buf[ppos++] = '\t';
                    }
                    if (node->op == TOK_EOL || node->op == 0) {
                        break;
                    }
                    node = node->next;
                }
                if (ppos > 0) {
                    file_write_raw(fc, chan, print_buf, ppos);
                }
                break;
            }
            case AST_NODE_LINE_INPUT: {
                char lbuf[2048];
                size_t l = 0;
                if (file_gets(vm_get_file(vm), node->channel, lbuf, sizeof(lbuf))) {
                    l = runtime_strlen(lbuf);
                    while (l > 0 && (lbuf[l - 1] == '\r' || lbuf[l - 1] == '\n')) {
                        lbuf[--l] = '\0';
                    }
                } else {
                    lbuf[0] = '\0';
                }

                if (!node->cached_var_ptr) {
                    node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
                }
                if (node->cached_var_ptr) {
                    BppStringRef existing = (node->cached_var_ptr->type == VAL_STRING) ? node->cached_var_ptr->as.string : NULL;
                    node->cached_var_ptr->type = VAL_STRING;
                    node->cached_var_ptr->as.string = str_assign_inplace(vm_get_str(vm), existing, lbuf, l);
                } else {
                    BppString *s = str_create(vm_get_str(vm), lbuf, l);
                    BValue val = { .type = VAL_STRING, .as.string = s };
                    var_assign(vm_get_var(vm), node->var_name, val);
                    str_release(vm_get_str(vm), s);
                }
                break;
            }
            default:
                break;
        }
        node = node->next;
    }
    return err;
}
