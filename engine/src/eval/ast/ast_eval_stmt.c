// FILENAME: ast_eval_stmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Core AST statement execution loop and dispatcher.
//
// ---- Includes ----

#include "bios/bios.h"
#include "device/bus.h"
#include "esp32_regs.h"
#include "device/vdev.h"
#include "eval/ast_internal.h"
#include "memory/memory.h"
#include "memory/segmented_mem.h"
#include "statements/system/hardware/def_seg.h"
#include "platform/platform.h"
#include "runtime/arrays.h"
#include "runtime/file.h"
#include "runtime/map.h"
#include "runtime/num_format.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "runtime/variables_internal.h"
#include "statements/oop/sub.h"
#include "vm/vm.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

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

        // Fast Path: Direct scalar numeric assignment via Inline Cache
        if (node->type == AST_NODE_ASSIGNMENT && node->cached_var_ptr &&
            node->cached_var_ptr->type != VAL_STRING && node->cached_var_ptr->type != VAL_NONE) {
            double num = eval_ast_eval_num(vm, node->right, &err);
            if (err.code != 0) return err;
            if (node->cached_var_ptr->type == VAL_INTEGER) {
                node->cached_var_ptr->as.number = (double)((int32_t)num);
            } else {
                node->cached_var_ptr->type = VAL_NUMBER;
                node->cached_var_ptr->as.number = num;
            }
            node = node->next;
            continue;
        }

        // Try assignment node execution
        if (eval_ast_exec_assign(vm, node, &err)) {
            if (err.code != 0) return err;
            node = node->next;
            continue;
        }

        // Try control-flow node execution
        if (eval_ast_exec_ctrl(vm, node, &err)) {
            if (err.code != 0 || vm_is_jump_active(vm)) return err;
            node = node->next;
            continue;
        }

        switch (node->type) {
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
                if (err.code != 0) {
                    if (addr_val.type == VAL_STRING && addr_val.as.string) str_release(vm_get_str(vm), addr_val.as.string);
                    return err;
                }
                uint32_t addr = (uint32_t)((addr_val.type == VAL_NUMBER || addr_val.type == VAL_INTEGER) ? addr_val.as.number : 0);
                uint16_t def_seg = 0;
#ifndef BASIC_LITE_BUILD
                if (vm_get_vmem(vm)) def_seg = vmem_get_def_seg(vm_get_vmem(vm));
#endif
                if (def_seg == 0) def_seg = runtime_get_def_seg();
                if (def_seg != 0 && addr < 0x10000U) {
                    addr = ((uint32_t)def_seg << 4) + addr;
                }
                if (addr_val.type == VAL_STRING && addr_val.as.string) str_release(vm_get_str(vm), addr_val.as.string);

                if (val_val.type == VAL_STRING) {
                    if (val_val.as.string) {
                        const char *sdata = str_data(val_val.as.string);
                        size_t slen = str_len(val_val.as.string);
                        for (size_t k = 0; k < slen; k++) {
                            uint32_t curr_addr = addr + (uint32_t)k;
                            uint8_t b = (uint8_t)sdata[k];
                            if (esp32_is_hardware_addr(curr_addr)) {
                                esp32_reg_write8(curr_addr, b);
                            } else if (vm_get_bios(vm)) {
                                bios_poke(vm_get_bios(vm), curr_addr, b);
                            } else {
                                bool intercepted = false;
                                vdev_bus_poke(curr_addr, b, &intercepted);
                            }
                        }
                        str_release(vm_get_str(vm), val_val.as.string);
                    }
                    break;
                }

                int width = 1;
                if (node->condition) {
                    BValue wval = eval_ast_evaluate(vm, node->condition, &err);
                    if (err.code != 0) {
                        if (val_val.type == VAL_STRING && val_val.as.string) str_release(vm_get_str(vm), val_val.as.string);
                        return err;
                    }
                    if (wval.type == VAL_STRING && wval.as.string) {
                        str_release(vm_get_str(vm), wval.as.string);
                    } else {
                        width = (int)wval.as.number;
                        if (width != 1 && width != 2 && width != 4 && width != 8) width = 1;
                    }
                }

                uint64_t val = (uint64_t)val_val.as.number;
                for (int k = 0; k < width; k++) {
                    uint32_t curr_addr = addr + (uint32_t)k;
                    uint8_t b = (uint8_t)((val >> (k * 8)) & 0xFF);
                    if (esp32_is_hardware_addr(curr_addr)) {
                        esp32_reg_write8(curr_addr, b);
                    } else if (vm_get_bios(vm)) {
                        bios_poke(vm_get_bios(vm), curr_addr, b);
                    } else {
                        bool intercepted = false;
                        vdev_bus_poke(curr_addr, b, &intercepted);
                    }
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
                    print_buf[ppos] = '\0';
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
            case AST_NODE_DIM: {
                BValue d1_val = eval_ast_evaluate(vm, node->index_expr, &err);
                if (err.code != 0) return err;
                int d1 = (int)((d1_val.type == VAL_NUMBER || d1_val.type == VAL_INTEGER) ? d1_val.as.number : 0);
                if (d1_val.type == VAL_STRING && d1_val.as.string) str_release(vm_get_str(vm), d1_val.as.string);
                int dims[2];
                dims[0] = d1;
                if (node->dims == 2 && node->index2_expr) {
                    BValue d2_val = eval_ast_evaluate(vm, node->index2_expr, &err);
                    if (err.code != 0) return err;
                    int d2 = (int)((d2_val.type == VAL_NUMBER || d2_val.type == VAL_INTEGER) ? d2_val.as.number : 0);
                    if (d2_val.type == VAL_STRING && d2_val.as.string) str_release(vm_get_str(vm), d2_val.as.string);
                    dims[1] = d2;
                }
                ValueType elem_type = (node->var_name[runtime_strlen(node->var_name) - 1] == '$') ? VAL_STRING : VAL_NUMBER;
                err = arr_dim(vm_get_arr(vm), node->var_name, node->dims, dims);
                if (err.code == 0) {
                    arr_set_type(vm_get_arr(vm), node->var_name, elem_type);
                }
                break;
            }
            case AST_NODE_FILE_OPEN: {
                BValue fn_val = eval_ast_evaluate(vm, node->left, &err);
                if (err.code != 0) return err;
                const char *fn = (fn_val.type == VAL_STRING && fn_val.as.string) ? str_data(fn_val.as.string) : "";
                BppFileMode fmode = FILE_MODE_OUTPUT;
                if (node->file_mode == 1) fmode = FILE_MODE_OUTPUT;
                else if (node->file_mode == 2) fmode = FILE_MODE_INPUT;
                else if (node->file_mode == 3) fmode = FILE_MODE_APPEND;
                else if (node->file_mode == 4) fmode = FILE_MODE_BINARY;
                else if (node->file_mode == 5) fmode = FILE_MODE_RANDOM;
                err = file_open(vm_get_file(vm), vm_get_vdev(vm), node->channel, fn, fmode, FILE_ACCESS_DEFAULT, FILE_LOCK_DEFAULT, 0);
                if (fn_val.type == VAL_STRING && fn_val.as.string) str_release(vm_get_str(vm), fn_val.as.string);
                break;
            }
            case AST_NODE_FILE_CLOSE: {
                if (node->channel == 0) {
                    file_close_all(vm_get_file(vm));
                } else {
                    file_close(vm_get_file(vm), node->channel);
                }
                break;
            }
            case AST_NODE_FILE_KILL: {
                BValue fn_val = eval_ast_evaluate(vm, node->left, &err);
                if (err.code != 0) return err;
                const char *fn = (fn_val.type == VAL_STRING && fn_val.as.string) ? str_data(fn_val.as.string) : "";
                if (fn && *fn) {
                    HalContext *hal = hal_get();
                    if (hal && hal->io.file_remove) hal->io.file_remove(fn);
                }
                if (fn_val.type == VAL_STRING && fn_val.as.string) str_release(vm_get_str(vm), fn_val.as.string);
                break;
            }
            case AST_NODE_END: {
                vm_set_running(vm, false);
                return err;
            }
            case AST_NODE_SWAP: {
                if (!node->cached_var_ptr) node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, true);
                if (!node->cached_extra_var_ptr) node->cached_extra_var_ptr = var_lookup(vm_get_var(vm), node->extra_var, true);
                if (node->cached_var_ptr && node->cached_extra_var_ptr) {
                    BValue tmp = *node->cached_var_ptr;
                    *node->cached_var_ptr = *node->cached_extra_var_ptr;
                    *node->cached_extra_var_ptr = tmp;
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
