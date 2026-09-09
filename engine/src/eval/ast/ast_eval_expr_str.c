// FILENAME: ast_eval_expr_str.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast_eval_expr.c, ast_eval_stmt.c)
// NEEDS: libcore, libengine (ast_internal.h)
// AST string function evaluators and string flattening helpers.
//
// ---- Includes ----

#include "eval/ast_internal.h"
#include "bios/bios.h"
#include "device/bus.h"
#include "esp32_regs.h"
#include "runtime/file.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/variables.h"
#include "statements/system/hardware/def_seg.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif

// flattens nested string concatenation nodes into a linear list of segments
void eval_ast_flatten_concat(VMContext *vm, EvalAstNode *node, const char **parts, size_t *lens,
                             char num_bufs[16][64], size_t *num_buf_idx,
                             BppString **to_release, size_t *scount, size_t *rcount,
                             size_t max_parts, BppError *err) {
    if (!node || *scount >= max_parts || err->code != 0) return;

    if (node->type == AST_NODE_BINARY_OP && node->op == TOK_PLUS) {
        eval_ast_flatten_concat(vm, node->left, parts, lens, num_bufs, num_buf_idx, to_release, scount, rcount, max_parts, err);
        if (err->code != 0) return;
        eval_ast_flatten_concat(vm, node->right, parts, lens, num_bufs, num_buf_idx, to_release, scount, rcount, max_parts, err);
        return;
    }

    if (node->type == AST_NODE_LITERAL && node->val.type == VAL_STRING && node->val.as.string) {
        parts[*scount] = str_data(node->val.as.string);
        lens[*scount] = str_len(node->val.as.string);
        (*scount)++;
        return;
    }

    if (node->type == AST_NODE_VARIABLE) {
        if (!node->cached_var_ptr) {
            node->cached_var_ptr = var_lookup(vm_get_var(vm), node->var_name, false);
        }
        BValue *vp = node->cached_var_ptr;
        if (vp && vp->type == VAL_STRING && vp->as.string) {
            parts[*scount] = str_data(vp->as.string);
            lens[*scount] = str_len(vp->as.string);
            (*scount)++;
            return;
        } else if (vp && (vp->type == VAL_NUMBER || vp->type == VAL_INTEGER) && *num_buf_idx < 16) {
            char *nb = num_bufs[(*num_buf_idx)++];
            size_t nlen = ast_format_num(nb, vp->as.number);
            parts[*scount] = nb;
            lens[*scount] = nlen;
            (*scount)++;
            return;
        }
    }

    if (node->type == AST_NODE_STRING_FUNC && node->str_func == AST_STR_STR) {
        BValue nval = eval_ast_evaluate(vm, node->left, err);
        if (err->code != 0) return;
        if (nval.type == VAL_STRING && nval.as.string) {
            str_release(vm_get_str(vm), nval.as.string);
        }
        if (*num_buf_idx < 16) {
            char *nb = num_bufs[(*num_buf_idx)++];
            size_t nlen = ast_format_num(nb, nval.as.number);
            if (nlen > 0 && nb[nlen - 1] == ' ') nb[--nlen] = '\0';
            parts[*scount] = nb;
            lens[*scount] = nlen;
            (*scount)++;
            return;
        }
    }

    BValue val = eval_ast_evaluate(vm, node, err);
    if (err->code != 0) return;
    if (val.type == VAL_STRING && val.as.string) {
        parts[*scount] = str_data(val.as.string);
        lens[*scount] = str_len(val.as.string);
        (*scount)++;
        to_release[(*rcount)++] = val.as.string;
    } else if ((val.type == VAL_NUMBER || val.type == VAL_INTEGER) && *num_buf_idx < 16) {
        char *nb = num_bufs[(*num_buf_idx)++];
        size_t nlen = ast_format_num(nb, val.as.number);
        parts[*scount] = nb;
        lens[*scount] = nlen;
        (*scount)++;
    }
}

// evaluates string AST functions returning a BValue result
BValue eval_ast_eval_str_func(VMContext *vm, EvalAstNode *node, BppError *err) {
    BValue null_val;
    runtime_memset(&null_val, 0, sizeof(null_val));
    if (!vm || !node) return null_val;

    StringContext *sc = vm_get_str(vm);
    switch (node->str_func) {
        case AST_STR_STR: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            double d = (val.type == VAL_NUMBER || val.type == VAL_INTEGER) ? val.as.number : 0.0;
            if (val.type == VAL_STRING && val.as.string) str_release(sc, val.as.string);

            if (node->right) {
                BValue rval = eval_ast_evaluate(vm, node->right, err);
                if (err->code != 0) return null_val;
                int radix = (int)((rval.type == VAL_NUMBER || rval.type == VAL_INTEGER) ? rval.as.number : 10);
                if (rval.type == VAL_STRING && rval.as.string) str_release(sc, rval.as.string);
                if (radix >= 2 && radix <= 36) {
                    long long nval = (long long)d;
                    bool is_neg = false;
                    if (nval < 0) { is_neg = true; nval = -nval; }
                    char tmp[66];
                    int pos = 0;
                    static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
                    if (nval == 0) tmp[pos++] = '0';
                    else {
                        while (nval > 0 && pos < 64) {
                            tmp[pos++] = digits[nval % radix];
                            nval /= radix;
                        }
                    }
                    if (is_neg && pos < 65) tmp[pos++] = '-';
                    char out[66];
                    for (int i = 0; i < pos; i++) out[i] = tmp[pos - 1 - i];
                    out[pos] = '\0';
                    BppString *bs = str_create(sc, out, (size_t)pos);
                    BValue res = { .type = VAL_STRING, .as.string = bs };
                    return res;
                }
            }

            char buf[64];
            size_t nlen = ast_format_num(buf, d);
            if (nlen > 0 && buf[nlen - 1] == ' ') {
                buf[--nlen] = '\0';
            }
            BppString *bs = str_create(sc, buf, nlen);
            BValue res = { .type = VAL_STRING, .as.string = bs };
            return res;
        }
        case AST_STR_UCASE: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            if (val.type != VAL_STRING || !val.as.string) {
                BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
                return res;
            }
            if (str_is_unique(val.as.string)) {
                char *data = str_data_mut(val.as.string);
                size_t len = str_len(val.as.string);
                if (data) {
                    for (size_t i = 0; i < len; i++) {
                        unsigned char ch = (unsigned char)data[i];
                        if (ch >= 'a' && ch <= 'z') data[i] = (char)(ch - 32);
                    }
                }
                return val;
            }
            const char *data = str_data(val.as.string);
            size_t len = str_len(val.as.string);
            char stack_buf[256];
            HalContext *hal = hal_get();
            char *buf = (len < sizeof(stack_buf)) ? stack_buf : ((hal && hal->mem.alloc) ? (char *)hal->mem.alloc(len + 1) : NULL);
            if (buf) {
                for (size_t i = 0; i < len; i++) {
                    unsigned char ch = (unsigned char)data[i];
                    buf[i] = (char)((ch >= 'a' && ch <= 'z') ? (ch - 32) : ch);
                }
                buf[len] = '\0';
                BppString *bs = str_create(sc, buf, len);
                if (buf != stack_buf && hal && hal->mem.free) hal->mem.free(buf);
                str_release(sc, val.as.string);
                BValue res = { .type = VAL_STRING, .as.string = bs };
                return res;
            }
            str_release(sc, val.as.string);
            return null_val;
        }
        case AST_STR_LCASE: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            if (val.type != VAL_STRING || !val.as.string) {
                BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
                return res;
            }
            if (str_is_unique(val.as.string)) {
                char *data = str_data_mut(val.as.string);
                size_t len = str_len(val.as.string);
                if (data) {
                    for (size_t i = 0; i < len; i++) {
                        unsigned char ch = (unsigned char)data[i];
                        if (ch >= 'A' && ch <= 'Z') data[i] = (char)(ch + 32);
                    }
                }
                return val;
            }
            const char *data = str_data(val.as.string);
            size_t len = str_len(val.as.string);
            char stack_buf[256];
            HalContext *hal = hal_get();
            char *buf = (len < sizeof(stack_buf)) ? stack_buf : ((hal && hal->mem.alloc) ? (char *)hal->mem.alloc(len + 1) : NULL);
            if (buf) {
                for (size_t i = 0; i < len; i++) {
                    unsigned char ch = (unsigned char)data[i];
                    buf[i] = (char)((ch >= 'A' && ch <= 'Z') ? (ch + 32) : ch);
                }
                buf[len] = '\0';
                BppString *bs = str_create(sc, buf, len);
                if (buf != stack_buf && hal && hal->mem.free) hal->mem.free(buf);
                str_release(sc, val.as.string);
                BValue res = { .type = VAL_STRING, .as.string = bs };
                return res;
            }
            str_release(sc, val.as.string);
            return null_val;
        }
        case AST_STR_LEN: {
            if (node->left && node->left->type == AST_NODE_VARIABLE) {
                if (!node->left->cached_var_ptr) node->left->cached_var_ptr = var_lookup(vm_get_var(vm), node->left->var_name, false);
                BValue *vp = node->left->cached_var_ptr;
                if (vp && vp->type == VAL_STRING) {
                    BValue res = { .type = VAL_NUMBER, .as.number = (double)str_len(vp->as.string) };
                    return res;
                }
            }
            if (node->left && node->left->type == AST_NODE_LITERAL && node->left->val.type == VAL_STRING) {
                BValue res = { .type = VAL_NUMBER, .as.number = (double)str_len(node->left->val.as.string) };
                return res;
            }
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            double length = 0.0;
            if (val.type == VAL_STRING && val.as.string) {
                length = (double)str_len(val.as.string);
                str_release(sc, val.as.string);
            }
            BValue res = { .type = VAL_NUMBER, .as.number = length };
            return res;
        }
        case AST_STR_ASC: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            double code = 0.0;
            if (val.type == VAL_STRING && val.as.string && str_len(val.as.string) > 0) {
                code = (double)(unsigned char)(str_data(val.as.string)[0]);
                str_release(sc, val.as.string);
            }
            BValue res = { .type = VAL_NUMBER, .as.number = code };
            return res;
        }
        case AST_STR_CHR: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            int code = (int)((val.type == VAL_NUMBER || val.type == VAL_INTEGER) ? val.as.number : 0);
            if (val.type == VAL_STRING && val.as.string) str_release(sc, val.as.string);
            if (code <= 255) {
                char ch = (char)(code & 0xFF);
                BValue res = { .type = VAL_STRING, .as.string = str_create(sc, &ch, 1) };
                return res;
            }
            char utf8_buf[8];
            size_t ulen = 0;
            if (code <= 0x7FF) {
                utf8_buf[0] = (char)(0xC0 | ((code >> 6) & 0x1F));
                utf8_buf[1] = (char)(0x80 | (code & 0x3F));
                ulen = 2;
            } else if (code <= 0xFFFF) {
                utf8_buf[0] = (char)(0xE0 | ((code >> 12) & 0x0F));
                utf8_buf[1] = (char)(0x80 | ((code >> 6) & 0x3F));
                utf8_buf[2] = (char)(0x80 | (code & 0x3F));
                ulen = 3;
            } else if (code <= 0x10FFFF) {
                utf8_buf[0] = (char)(0xF0 | ((code >> 18) & 0x07));
                utf8_buf[1] = (char)(0x80 | ((code >> 12) & 0x3F));
                utf8_buf[2] = (char)(0x80 | ((code >> 6) & 0x3F));
                utf8_buf[3] = (char)(0x80 | (code & 0x3F));
                ulen = 4;
            } else {
                utf8_buf[0] = '?';
                ulen = 1;
            }
            utf8_buf[ulen] = '\0';
            BValue res = { .type = VAL_STRING, .as.string = str_create(sc, utf8_buf, ulen) };
            return res;
        }
        case AST_STR_MID: {
            BValue sval = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            BValue start_val = eval_ast_evaluate(vm, node->right, err);
            if (err->code != 0) {
                if (sval.type == VAL_STRING && sval.as.string) str_release(sc, sval.as.string);
                return null_val;
            }
            int start = (int)((start_val.type == VAL_NUMBER || start_val.type == VAL_INTEGER) ? start_val.as.number : 1);
            int length = -1;
            if (node->condition) {
                BValue len_val = eval_ast_evaluate(vm, node->condition, err);
                if (err->code != 0) {
                    if (sval.type == VAL_STRING && sval.as.string) str_release(sc, sval.as.string);
                    return null_val;
                }
                length = (int)((len_val.type == VAL_NUMBER || len_val.type == VAL_INTEGER) ? len_val.as.number : -1);
            }
            if (sval.type == VAL_STRING && sval.as.string) {
                const char *src = str_data(sval.as.string);
                size_t total_len = str_len(sval.as.string);
                if (total_len == 0 || start == 0) {
                    str_release(sc, sval.as.string);
                    BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
                    return res;
                }
                size_t sub_start = 0;
                if (start < 0) {
                    long long calc_start = (long long)total_len + start + 1;
                    if (calc_start <= 0) calc_start = 1;
                    if ((size_t)calc_start > total_len) {
                        str_release(sc, sval.as.string);
                        BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
                        return res;
                    }
                    sub_start = (size_t)(calc_start - 1);
                } else {
                    if ((size_t)start > total_len) {
                        str_release(sc, sval.as.string);
                        BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
                        return res;
                    }
                    sub_start = (size_t)(start - 1);
                }
                size_t remaining = total_len - sub_start;
                size_t sub_len = 0;
                if (length < 0 && node->condition) {
                    size_t skip_end = (size_t)(-length);
                    sub_len = (sub_start + skip_end >= total_len) ? 0 : (total_len - sub_start - skip_end);
                } else {
                    sub_len = (length < 0 || (size_t)length > remaining) ? remaining : (size_t)length;
                }
                BppString *bs = str_create(sc, src + sub_start, sub_len);
                str_release(sc, sval.as.string);
                BValue res = { .type = VAL_STRING, .as.string = bs };
                return res;
            }
            BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
            return res;
        }
        case AST_STR_LEFT: {
            BValue sval = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            BValue cnt_val = eval_ast_evaluate(vm, node->right, err);
            if (err->code != 0) {
                if (sval.type == VAL_STRING && sval.as.string) str_release(sc, sval.as.string);
                return null_val;
            }
            int count = (int)((cnt_val.type == VAL_NUMBER || cnt_val.type == VAL_INTEGER) ? cnt_val.as.number : 0);
            if (sval.type == VAL_STRING && sval.as.string) {
                const char *src = str_data(sval.as.string);
                size_t total_len = str_len(sval.as.string);
                size_t take = 0;
                if (count < 0) {
                    size_t skip = (size_t)(-count);
                    take = (skip >= total_len) ? 0 : (total_len - skip);
                } else {
                    take = ((size_t)count > total_len) ? total_len : (size_t)count;
                }
                BppString *bs = str_create(sc, src, take);
                str_release(sc, sval.as.string);
                BValue res = { .type = VAL_STRING, .as.string = bs };
                return res;
            }
            BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
            return res;
        }
        case AST_STR_RIGHT: {
            BValue sval = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            BValue cnt_val = eval_ast_evaluate(vm, node->right, err);
            if (err->code != 0) {
                if (sval.type == VAL_STRING && sval.as.string) str_release(sc, sval.as.string);
                return null_val;
            }
            int count = (int)((cnt_val.type == VAL_NUMBER || cnt_val.type == VAL_INTEGER) ? cnt_val.as.number : 0);
            if (sval.type == VAL_STRING && sval.as.string) {
                const char *src = str_data(sval.as.string);
                size_t total_len = str_len(sval.as.string);
                size_t take = 0;
                size_t start = 0;
                if (count < 0) {
                    size_t skip = (size_t)(-count);
                    start = (skip >= total_len) ? total_len : skip;
                    take = total_len - start;
                } else {
                    take = ((size_t)count > total_len) ? total_len : (size_t)count;
                    start = (total_len > take) ? (total_len - take) : 0;
                }
                BppString *bs = str_create(sc, src + start, take);
                str_release(sc, sval.as.string);
                BValue res = { .type = VAL_STRING, .as.string = bs };
                return res;
            }
            BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
            return res;
        }
        case AST_STR_SPACE: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            int count = (int)((val.type == VAL_NUMBER || val.type == VAL_INTEGER) ? val.as.number : 0);
            if (val.type == VAL_STRING && val.as.string) str_release(sc, val.as.string);
            if (count < 0) count = 0;
            HalContext *hal = hal_get();
            char *buf = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(count + 1) : NULL;
            if (buf) {
                runtime_memset(buf, ' ', count);
                buf[count] = '\0';
                BppString *bs = str_create(sc, buf, count);
                if (hal && hal->mem.free) hal->mem.free(buf);
                BValue res = { .type = VAL_STRING, .as.string = bs };
                return res;
            }
            BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
            return res;
        }
        case AST_STR_STRING: {
            BValue cnt_val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            BValue char_val = eval_ast_evaluate(vm, node->right, err);
            if (err->code != 0) {
                if (cnt_val.type == VAL_STRING && cnt_val.as.string) str_release(sc, cnt_val.as.string);
                return null_val;
            }
            int count = (int)((cnt_val.type == VAL_NUMBER || cnt_val.type == VAL_INTEGER) ? cnt_val.as.number : 0);
            if (cnt_val.type == VAL_STRING && cnt_val.as.string) str_release(sc, cnt_val.as.string);
            if (count <= 0) {
                if (char_val.type == VAL_STRING && char_val.as.string) str_release(sc, char_val.as.string);
                BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
                return res;
            }
            if (char_val.type == VAL_STRING && char_val.as.string) {
                const char *sd = str_data(char_val.as.string);
                size_t slen = str_len(char_val.as.string);
                if (slen == 0) {
                    str_release(sc, char_val.as.string);
                    BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
                    return res;
                }
                size_t total_len = (size_t)count * slen;
                HalContext *hal = hal_get();
                char *buf = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(total_len + 1) : NULL;
                if (buf) {
                    for (int i = 0; i < count; i++) {
                        runtime_memcpy(buf + (i * slen), sd, slen);
                    }
                    buf[total_len] = '\0';
                    BppString *bs = str_create(sc, buf, total_len);
                    if (hal && hal->mem.free) hal->mem.free(buf);
                    str_release(sc, char_val.as.string);
                    BValue res = { .type = VAL_STRING, .as.string = bs };
                    return res;
                }
                str_release(sc, char_val.as.string);
            } else if (char_val.type == VAL_NUMBER || char_val.type == VAL_INTEGER) {
                char fill_ch = (char)((int)char_val.as.number & 0xFF);
                HalContext *hal = hal_get();
                char *buf = (hal && hal->mem.alloc) ? (char *)hal->mem.alloc(count + 1) : NULL;
                if (buf) {
                    runtime_memset(buf, fill_ch, count);
                    buf[count] = '\0';
                    BppString *bs = str_create(sc, buf, count);
                    if (hal && hal->mem.free) hal->mem.free(buf);
                    BValue res = { .type = VAL_STRING, .as.string = bs };
                    return res;
                }
            }
            BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
            return res;
        }
        case AST_STR_HEX: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            uint64_t n = (uint64_t)((val.type == VAL_NUMBER || val.type == VAL_INTEGER) ? val.as.number : 0);
            if (val.type == VAL_STRING && val.as.string) str_release(sc, val.as.string);
            char buf[32];
            static const char hex_digits[] = "0123456789ABCDEF";
            if (n == 0) {
                BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "0", 1) };
                return res;
            }
            char temp[16];
            int len = 0;
            while (n > 0) {
                temp[len++] = hex_digits[n & 0xF];
                n >>= 4;
            }
            for (int i = 0; i < len; i++) {
                buf[i] = temp[len - 1 - i];
            }
            buf[len] = '\0';
            BValue res = { .type = VAL_STRING, .as.string = str_create(sc, buf, (size_t)len) };
            return res;
        }
        case AST_STR_OCT: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            int64_t n = (int64_t)((val.type == VAL_NUMBER || val.type == VAL_INTEGER) ? val.as.number : 0);
            if (val.type == VAL_STRING && val.as.string) str_release(sc, val.as.string);
            char buf[64];
            runtime_snprintf(buf, sizeof(buf), "%llo", (unsigned long long)n);
            BValue res = { .type = VAL_STRING, .as.string = str_from_cstr(sc, buf) };
            return res;
        }
        case AST_STR_BIN: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            uint64_t n = (uint64_t)((val.type == VAL_NUMBER || val.type == VAL_INTEGER) ? val.as.number : 0);
            if (val.type == VAL_STRING && val.as.string) str_release(sc, val.as.string);
            char buf[65];
            int pos = 0;
            for (int bit = 63; bit >= 0; bit--) {
                if ((n >> bit) & 1 || pos > 0) {
                    buf[pos++] = ((n >> bit) & 1) ? '1' : '0';
                }
            }
            if (pos == 0) buf[pos++] = '0';
            buf[pos] = '\0';
            BValue res = { .type = VAL_STRING, .as.string = str_from_cstr(sc, buf) };
            return res;
        }
        case AST_STR_INSTR: {
            const char *target = "";
            const char *search = "";
            BppString *s_to_rel1 = NULL;
            BppString *s_to_rel2 = NULL;
            int start_pos = 1;

            if (!node->condition) {
                BValue arg1 = eval_ast_evaluate(vm, node->left, err);
                if (err->code != 0) return null_val;
                target = (arg1.type == VAL_STRING && arg1.as.string) ? str_data(arg1.as.string) : "";
                s_to_rel1 = (arg1.type == VAL_STRING) ? arg1.as.string : NULL;

                BValue arg2 = eval_ast_evaluate(vm, node->right, err);
                if (err->code != 0) {
                    if (s_to_rel1) str_release(sc, s_to_rel1);
                    return null_val;
                }
                search = (arg2.type == VAL_STRING && arg2.as.string) ? str_data(arg2.as.string) : "";
                s_to_rel2 = (arg2.type == VAL_STRING) ? arg2.as.string : NULL;
            } else {
                BValue arg1 = eval_ast_evaluate(vm, node->left, err);
                if (err->code != 0) return null_val;
                BValue arg2 = eval_ast_evaluate(vm, node->right, err);
                if (err->code != 0) {
                    if (arg1.type == VAL_STRING && arg1.as.string) str_release(sc, arg1.as.string);
                    return null_val;
                }
                BValue arg3 = eval_ast_evaluate(vm, node->condition, err);
                if (err->code != 0) {
                    if (arg1.type == VAL_STRING && arg1.as.string) str_release(sc, arg1.as.string);
                    if (arg2.type == VAL_STRING && arg2.as.string) str_release(sc, arg2.as.string);
                    return null_val;
                }
                start_pos = (int)((arg1.type == VAL_NUMBER || arg1.type == VAL_INTEGER) ? arg1.as.number : 1);
                if (arg1.type == VAL_STRING && arg1.as.string) str_release(sc, arg1.as.string);
                target = (arg2.type == VAL_STRING && arg2.as.string) ? str_data(arg2.as.string) : "";
                search = (arg3.type == VAL_STRING && arg3.as.string) ? str_data(arg3.as.string) : "";
                s_to_rel1 = (arg2.type == VAL_STRING) ? arg2.as.string : NULL;
                s_to_rel2 = (arg3.type == VAL_STRING) ? arg3.as.string : NULL;
            }

            double found_idx = 0.0;
            size_t tlen = runtime_strlen(target);
            size_t slen = runtime_strlen(search);
            if (start_pos < 0) {
                int calc = (int)tlen + start_pos + 1;
                if (calc >= 1 && (size_t)calc <= tlen && slen > 0 && slen <= tlen) {
                    int max_s = (int)tlen - (int)slen + 1;
                    int s_pos = (calc < max_s) ? calc : max_s;
                    for (int i = s_pos; i >= 1; i--) {
                        if (runtime_memcmp(target + (i - 1), search, slen) == 0) {
                            found_idx = (double)i;
                            break;
                        }
                    }
                }
            } else if (start_pos >= 1 && (size_t)(start_pos - 1) <= tlen) {
                const char *match = runtime_strstr(target + (start_pos - 1), search);
                if (match) {
                    found_idx = (double)(match - target + 1);
                }
            }
            if (s_to_rel1) str_release(sc, s_to_rel1);
            if (s_to_rel2) str_release(sc, s_to_rel2);
            BValue res = { .type = VAL_NUMBER, .as.number = found_idx };
            return res;
        }
        case AST_STR_SHA256: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            char digest[128] = {0};
            const char *data_str = (val.type == VAL_STRING && val.as.string) ? str_data(val.as.string) : "";
            hash_string("SHA256", data_str, digest, sizeof(digest));
            if (val.type == VAL_STRING && val.as.string) str_release(sc, val.as.string);
            BValue res = { .type = VAL_STRING, .as.string = str_from_cstr(sc, digest) };
            return res;
        }
        case AST_STR_MD5: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            char digest[128] = {0};
            const char *data_str = (val.type == VAL_STRING && val.as.string) ? str_data(val.as.string) : "";
            hash_string("MD5", data_str, digest, sizeof(digest));
            if (val.type == VAL_STRING && val.as.string) str_release(sc, val.as.string);
            BValue res = { .type = VAL_STRING, .as.string = str_from_cstr(sc, digest) };
            return res;
        }
        case AST_STR_PEEK: {
            BValue aval = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            uint32_t addr = (uint32_t)((aval.type == VAL_NUMBER || aval.type == VAL_INTEGER) ? aval.as.number : 0);
            uint16_t def_seg = 0;
#ifndef BASIC_LITE_BUILD
            if (vm_get_vmem(vm)) def_seg = vmem_get_def_seg(vm_get_vmem(vm));
#endif
            if (def_seg == 0) def_seg = runtime_get_def_seg();
            if (def_seg != 0 && addr < 0x10000U) {
                addr = ((uint32_t)def_seg << 4) + addr;
            }
            if (aval.type == VAL_STRING && aval.as.string) str_release(sc, aval.as.string);

            int width = 1;
            if (node->right) {
                BValue wval = eval_ast_evaluate(vm, node->right, err);
                if (err->code != 0) return null_val;
                if (wval.type == VAL_STRING && wval.as.string) {
                    str_release(sc, wval.as.string);
                } else {
                    width = (int)wval.as.number;
                    if (width != 1 && width != 2 && width != 4 && width != 8) width = 1;
                }
            }

            uint64_t accum = 0;
            for (int k = 0; k < width; k++) {
                uint32_t curr_addr = addr + (uint32_t)k;
                uint8_t byte_val = 0;
                if (esp32_is_hardware_addr(curr_addr)) {
                    byte_val = esp32_reg_read8(curr_addr);
                } else if (vm_get_bios(vm)) {
                    byte_val = bios_peek(vm_get_bios(vm), curr_addr);
                } else {
                    bool intercepted = false;
                    byte_val = vdev_bus_peek(curr_addr, &intercepted);
                }
                accum |= ((uint64_t)byte_val) << (k * 8);
            }

            BValue res = { .type = VAL_NUMBER, .as.number = (double)accum };
            return res;
        }
        case AST_STR_PEEK_STR: {
            BValue aval = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            uint32_t addr = (uint32_t)((aval.type == VAL_NUMBER || aval.type == VAL_INTEGER) ? aval.as.number : 0);
            uint16_t def_seg_str = 0;
#ifndef BASIC_LITE_BUILD
            if (vm_get_vmem(vm)) def_seg_str = vmem_get_def_seg(vm_get_vmem(vm));
#endif
            if (def_seg_str == 0) def_seg_str = runtime_get_def_seg();
            if (def_seg_str != 0 && addr < 0x10000U) {
                addr = ((uint32_t)def_seg_str << 4) + addr;
            }
            if (aval.type == VAL_STRING && aval.as.string) str_release(sc, aval.as.string);

            size_t len = 1;
            if (node->right) {
                BValue lval = eval_ast_evaluate(vm, node->right, err);
                if (err->code != 0) return null_val;
                if (lval.type == VAL_STRING && lval.as.string) {
                    str_release(sc, lval.as.string);
                } else {
                    len = (size_t)lval.as.number;
                }
            }

            char *buf = (char *)runtime_malloc(len + 1);
            if (!buf) {
                err->code = 14;
                err->message = "Out of memory in PEEK$";
                return null_val;
            }
            for (size_t k = 0; k < len; k++) {
                uint32_t curr_addr = addr + (uint32_t)k;
                uint8_t byte_val = 0;
                if (esp32_is_hardware_addr(curr_addr)) {
                    byte_val = esp32_reg_read8(curr_addr);
                } else if (vm_get_bios(vm)) {
                    byte_val = bios_peek(vm_get_bios(vm), curr_addr);
                } else {
                    bool intercepted = false;
                    byte_val = vdev_bus_peek(curr_addr, &intercepted);
                }
                buf[k] = (char)byte_val;
            }
            buf[len] = '\0';
            BValue res = { .type = VAL_STRING, .as.string = str_create(sc, buf, len) };
            runtime_free(buf);
            return res;
        }
        default:
            break;
    }
    return null_val;
}
