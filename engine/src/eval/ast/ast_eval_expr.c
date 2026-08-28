// FILENAME: ast_eval_expr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, file.h, file.c)
// NEEDS: libcore (num_format.h, num_format.c, strings.h, strings.c)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (ast_internal.h, bios.h, bios.c, eval_internal.h)
// NEEDS: libkernel (bus.h, bus.c)
// Implements AST parsing and evaluation structures for ast_eval_expr.
//
// ---- Includes ----

#include "bios/bios.h"
#include "device/bus.h"
#include "eval/ast_internal.h"
#include "eval/eval_internal.h"
#include "runtime/arrays.h"
#include "runtime/file.h"
#include "runtime/num_format.h"
#include "runtime/strings.h"
#include "runtime/variables.h"


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

// calculates the scalar result of a binary arithmetic or logical operation
double eval_ast_calc_binary_op(BppTokenType op, double n1, double n2) {
    switch (op) {
        case TOK_PLUS: return n1 + n2;
        case TOK_MINUS: return n1 - n2;
        case TOK_MUL: return n1 * n2;
        case TOK_DIV: return (n2 != 0.0) ? (n1 / n2) : 0.0;
        case TOK_POW: return runtime_pow(n1, n2);
        case TOK_MOD: return (n2 != 0.0) ? (double)((int64_t)n1 % (int64_t)n2) : 0.0;
        case TOK_BACKSLASH: return (n2 != 0.0) ? (double)((int64_t)n1 / (int64_t)n2) : 0.0;
        case TOK_LT: return (n1 < n2) ? -1.0 : 0.0;
        case TOK_LE: return (n1 <= n2) ? -1.0 : 0.0;
        case TOK_GT: return (n1 > n2) ? -1.0 : 0.0;
        case TOK_GE: return (n1 >= n2) ? -1.0 : 0.0;
        case TOK_EQ: return (n1 == n2) ? -1.0 : 0.0;
        case TOK_NE: return (n1 != n2) ? -1.0 : 0.0;
        case TOK_AND: return (double)((int64_t)n1 & (int64_t)n2);
        case TOK_OR: return (double)((int64_t)n1 | (int64_t)n2);
        case TOK_XOR: return (double)((int64_t)n1 ^ (int64_t)n2);
        case TOK_IMP: return (double)(~((int64_t)n1) | (int64_t)n2);
        case TOK_EQV: return (double)(~((int64_t)n1 ^ (int64_t)n2));
        default: return 0.0;
    }
}

// evaluates an AST expression node recursively returning a BValue result
BValue eval_ast_evaluate(VMContext *vm, EvalAstNode *node, BppError *err) {
    BValue null_val;
    runtime_memset(&null_val, 0, sizeof(null_val));
    if (!vm || !node) return null_val;

    switch (node->type) {
        case AST_NODE_LITERAL: {
            if (node->val.type == VAL_STRING && node->val.as.string) {
                str_add_ref(node->val.as.string);
            }
            return node->val;
        }
        case AST_NODE_VARIABLE: {
            if (node->cached_var_ptr) {
                BValue res = *node->cached_var_ptr;
                if (res.type == VAL_STRING && res.as.string) {
                    str_add_ref(res.as.string);
                }
                return res;
            }
            VariableContext *vc = vm_get_var(vm);
            BValue *ptr = var_lookup(vc, node->var_name, true);
            if (ptr) {
                node->cached_var_ptr = ptr;
                if (ptr->type == VAL_NONE) {
                    ptr->type = VAL_NUMBER;
                    ptr->as.number = 0.0;
                }
                BValue res = *ptr;
                if (res.type == VAL_STRING && res.as.string) {
                    str_add_ref(res.as.string);
                }
                return res;
            }
            BValue zero = { .type = VAL_NUMBER, .as.number = 0.0 };
            return zero;
        }
        case AST_NODE_MATH_FUNC: {
            BValue arg = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            double x = (arg.type == VAL_NUMBER || arg.type == VAL_INTEGER) ? arg.as.number : 0.0;
            double ans = 0.0;
            switch (node->math_func) {
                case AST_MATH_SIN: ans = runtime_sin(x); break;
                case AST_MATH_COS: ans = runtime_cos(x); break;
                case AST_MATH_TAN: ans = runtime_tan(x); break;
                case AST_MATH_SQR:
                    if (x < 0.0) { err->code = 5; err->message = "Illegal function call in SQR"; return null_val; }
                    ans = runtime_sqrt(x);
                    break;
                case AST_MATH_ABS: ans = runtime_fabs(x); break;
                case AST_MATH_INT: ans = runtime_floor(x); break;
                case AST_MATH_FIX: ans = (x >= 0.0) ? runtime_floor(x) : runtime_ceil(x); break;
                case AST_MATH_EXP: ans = runtime_exp(x); break;
                case AST_MATH_LOG:
                    if (x <= 0.0) { err->code = 5; err->message = "Illegal function call in LOG"; return null_val; }
                    ans = runtime_log(x);
                    break;
                case AST_MATH_ATN: ans = runtime_atan(x); break;
                case AST_MATH_SGN: ans = (x > 0.0) ? 1.0 : ((x < 0.0) ? -1.0 : 0.0); break;
                case AST_MATH_RND: {
                    double last = vm_get_last_rnd(vm);
                    uint64_t seed = (uint64_t)(last * 4294967296.0);
                    if (seed == 0) seed = 123456789ULL;
                    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
                    ans = (double)(seed & 0xFFFFFFFF) / 4294967296.0;
                    vm_set_last_rnd(vm, ans);
                    break;
                }
                case AST_MATH_EOF: {
                    int ch = (int)x;
                    ans = file_eof(vm_get_file(vm), ch) ? -1.0 : 0.0;
                    break;
                }
            }
            BValue res = { .type = VAL_NUMBER, .as.number = ans };
            return res;
        }

        case AST_NODE_STRING_FUNC: {
            StringContext *sc = vm_get_str(vm);
            switch (node->str_func) {
                case AST_STR_STR: {
                    BValue val = eval_ast_evaluate(vm, node->left, err);
                    if (err->code != 0) return null_val;
                    double d = (val.type == VAL_NUMBER || val.type == VAL_INTEGER) ? val.as.number : 0.0;
                    if (val.type == VAL_STRING && val.as.string) str_release(sc, val.as.string);
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
                    char ch = (char)(code & 0xFF);
                    BValue res = { .type = VAL_STRING, .as.string = str_create(sc, &ch, 1) };
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
                    if (start < 1) start = 1;
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
                        if ((size_t)start > total_len) {
                            str_release(sc, sval.as.string);
                            BValue res = { .type = VAL_STRING, .as.string = str_create(sc, "", 0) };
                            return res;
                        }
                        size_t sub_start = (size_t)(start - 1);
                        size_t sub_len = (length < 0 || (sub_start + (size_t)length > total_len)) ? (total_len - sub_start) : (size_t)length;
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
                    if (count < 0) count = 0;
                    if (sval.type == VAL_STRING && sval.as.string) {
                        const char *src = str_data(sval.as.string);
                        size_t total_len = str_len(sval.as.string);
                        size_t take = ((size_t)count > total_len) ? total_len : (size_t)count;
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
                    if (count < 0) count = 0;
                    if (sval.type == VAL_STRING && sval.as.string) {
                        const char *src = str_data(sval.as.string);
                        size_t total_len = str_len(sval.as.string);
                        size_t take = ((size_t)count > total_len) ? total_len : (size_t)count;
                        size_t start_off = total_len - take;
                        BppString *bs = str_create(sc, src + start_off, take);
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
                    if (count < 0) count = 0;
                    char fill_ch = ' ';
                    if (char_val.type == VAL_STRING && char_val.as.string) {
                        const char *sd = str_data(char_val.as.string);
                        if (sd && *sd) fill_ch = *sd;
                        str_release(sc, char_val.as.string);
                    } else if (char_val.type == VAL_NUMBER || char_val.type == VAL_INTEGER) {
                        fill_ch = (char)((int)char_val.as.number & 0xFF);
                    }
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
                    BValue arg1 = eval_ast_evaluate(vm, node->left, err);
                    if (err->code != 0) return null_val;
                    BValue arg2 = eval_ast_evaluate(vm, node->right, err);
                    if (err->code != 0) {
                        if (arg1.type == VAL_STRING && arg1.as.string) str_release(sc, arg1.as.string);
                        return null_val;
                    }
                    int start_pos = 1;
                    const char *target = "";
                    const char *search = "";
                    BppString *s_to_rel1 = NULL;
                    BppString *s_to_rel2 = NULL;
                    if (node->condition) {
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
                    } else {
                        target = (arg1.type == VAL_STRING && arg1.as.string) ? str_data(arg1.as.string) : "";
                        search = (arg2.type == VAL_STRING && arg2.as.string) ? str_data(arg2.as.string) : "";
                        s_to_rel1 = (arg1.type == VAL_STRING) ? arg1.as.string : NULL;
                        s_to_rel2 = (arg2.type == VAL_STRING) ? arg2.as.string : NULL;
                    }
                    double found_idx = 0.0;
                    if (start_pos >= 1 && (size_t)(start_pos - 1) <= runtime_strlen(target)) {
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
                    int addr = (int)((aval.type == VAL_NUMBER || aval.type == VAL_INTEGER) ? aval.as.number : 0);
                    if (aval.type == VAL_STRING && aval.as.string) str_release(sc, aval.as.string);
                    double bval = 0.0;
                    if (vm_get_bios(vm)) {
                        bval = (double)bios_peek(vm_get_bios(vm), (uint32_t)addr);
                    } else {
                        bool intercepted = false;
                        bval = (double)vdev_bus_peek((uint32_t)addr, &intercepted);
                    }
                    BValue res = { .type = VAL_NUMBER, .as.number = bval };
                    return res;
                }
                default:
                    break;
            }
            return null_val;
        }
        case AST_NODE_ARRAY_READ: {
            BValue ival = eval_ast_evaluate(vm, node->index_expr, err);
            if (err->code != 0) return null_val;
            int idx = (int)((ival.type == VAL_NUMBER || ival.type == VAL_INTEGER) ? ival.as.number : 0);
            int indices[1] = { idx };
            BValue *elem = arr_get_element(vm_get_arr(vm), node->var_name, 1, indices, err);
            if (!elem || err->code != 0) return null_val;
            BValue res = *elem;
            if (res.type == VAL_STRING && res.as.string) {
                str_add_ref(res.as.string);
            }
            return res;
        }
        case AST_NODE_ARRAY2D_READ: {
            BValue ival1 = eval_ast_evaluate(vm, node->index_expr, err);
            if (err->code != 0) return null_val;
            BValue ival2 = eval_ast_evaluate(vm, node->index2_expr, err);
            if (err->code != 0) return null_val;
            int idx1 = (int)((ival1.type == VAL_NUMBER || ival1.type == VAL_INTEGER) ? ival1.as.number : 0);
            int idx2 = (int)((ival2.type == VAL_NUMBER || ival2.type == VAL_INTEGER) ? ival2.as.number : 0);
            int indices[2] = { idx1, idx2 };
            BValue *elem = arr_get_element(vm_get_arr(vm), node->var_name, 2, indices, err);
            if (!elem || err->code != 0) return null_val;
            BValue res = *elem;
            if (res.type == VAL_STRING && res.as.string) {
                str_add_ref(res.as.string);
            }
            return res;
        }
        case AST_NODE_BINARY_OP: {
            if (node->left && node->right &&
                node->left->type == AST_NODE_VARIABLE && node->right->type == AST_NODE_VARIABLE) {
                if (!node->left->cached_var_ptr) node->left->cached_var_ptr = var_lookup(vm_get_var(vm), node->left->var_name, false);
                if (!node->right->cached_var_ptr) node->right->cached_var_ptr = var_lookup(vm_get_var(vm), node->right->var_name, false);
                BValue *vp1 = node->left->cached_var_ptr;
                BValue *vp2 = node->right->cached_var_ptr;
                if (vp1 && vp2 && (vp1->type == VAL_NUMBER || vp1->type == VAL_INTEGER) &&
                    (vp2->type == VAL_NUMBER || vp2->type == VAL_INTEGER)) {
                    BValue res = { .type = VAL_NUMBER, .as.number = eval_ast_calc_binary_op(node->op, vp1->as.number, vp2->as.number) };
                    return res;
                }
            }

            if (node->left && node->right &&
                node->left->type == AST_NODE_VARIABLE && node->right->type == AST_NODE_LITERAL &&
                (node->right->val.type == VAL_NUMBER || node->right->val.type == VAL_INTEGER)) {
                if (!node->left->cached_var_ptr) node->left->cached_var_ptr = var_lookup(vm_get_var(vm), node->left->var_name, false);
                BValue *vp1 = node->left->cached_var_ptr;
                if (vp1 && (vp1->type == VAL_NUMBER || vp1->type == VAL_INTEGER)) {
                    BValue res = { .type = VAL_NUMBER, .as.number = eval_ast_calc_binary_op(node->op, vp1->as.number, node->right->val.as.number) };
                    return res;
                }
            }

            if (node->left && node->right &&
                node->left->type == AST_NODE_LITERAL && node->right->type == AST_NODE_VARIABLE &&
                (node->left->val.type == VAL_NUMBER || node->left->val.type == VAL_INTEGER)) {
                if (!node->right->cached_var_ptr) node->right->cached_var_ptr = var_lookup(vm_get_var(vm), node->right->var_name, false);
                BValue *vp2 = node->right->cached_var_ptr;
                if (vp2 && (vp2->type == VAL_NUMBER || vp2->type == VAL_INTEGER)) {
                    BValue res = { .type = VAL_NUMBER, .as.number = eval_ast_calc_binary_op(node->op, node->left->val.as.number, vp2->as.number) };
                    return res;
                }
            }

            BValue lval = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            BValue rval = eval_ast_evaluate(vm, node->right, err);
            if (err->code != 0) {
                if (lval.type == VAL_STRING && lval.as.string) str_release(vm_get_str(vm), lval.as.string);
                return null_val;
            }

            if (node->op == TOK_PLUS && (lval.type == VAL_STRING || rval.type == VAL_STRING)) {
                StringContext *sc = vm_get_str(vm);
                const char *parts[16];
                size_t lens[16];
                char num_bufs[16][64];
                size_t num_buf_idx = 0;
                BppString *to_rel[16];
                size_t scount = 0, rcount = 0;

                eval_ast_flatten_concat(vm, node->left, parts, lens, num_bufs, &num_buf_idx, to_rel, &scount, &rcount, 16, err);
                if (err->code == 0) {
                    eval_ast_flatten_concat(vm, node->right, parts, lens, num_bufs, &num_buf_idx, to_rel, &scount, &rcount, 16, err);
                }
                if (lval.type == VAL_STRING && lval.as.string) str_release(sc, lval.as.string);
                if (rval.type == VAL_STRING && rval.as.string) str_release(sc, rval.as.string);

                if (err->code != 0) {
                    for (size_t i = 0; i < rcount; i++) str_release(sc, to_rel[i]);
                    return null_val;
                }

                BppString *res_str = str_concat_multi(sc, parts, lens, scount);
                for (size_t i = 0; i < rcount; i++) {
                    str_release(sc, to_rel[i]);
                }
                BValue res = { .type = VAL_STRING, .as.string = res_str };
                return res;
            }

            if ((lval.type == VAL_NUMBER || lval.type == VAL_INTEGER) &&
                (rval.type == VAL_NUMBER || rval.type == VAL_INTEGER)) {
                double n1 = lval.as.number;
                double n2 = rval.as.number;
                double ans = 0.0;
                switch (node->op) {
                    case TOK_PLUS:  ans = n1 + n2; break;
                    case TOK_MINUS: ans = n1 - n2; break;
                    case TOK_MUL:   ans = n1 * n2; break;
                    case TOK_DIV:
                        if (n2 == 0.0) {
                            err->code = 11; err->message = "Division by zero"; return null_val;
                        }
                        ans = n1 / n2;
                        break;
                    case TOK_EQ: ans = (n1 == n2) ? -1.0 : 0.0; break;
                    case TOK_NE: ans = (n1 != n2) ? -1.0 : 0.0; break;
                    case TOK_LT: ans = (n1 < n2)  ? -1.0 : 0.0; break;
                    case TOK_GT: ans = (n1 > n2)  ? -1.0 : 0.0; break;
                    case TOK_LE: ans = (n1 <= n2) ? -1.0 : 0.0; break;
                    case TOK_GE: ans = (n1 >= n2) ? -1.0 : 0.0; break;
                    case TOK_AND: ans = (double)((int64_t)n1 & (int64_t)n2); break;
                    case TOK_OR:  ans = (double)((int64_t)n1 | (int64_t)n2); break;
                    case TOK_XOR: ans = (double)((int64_t)n1 ^ (int64_t)n2); break;
                    case TOK_IMP: ans = (double)(~(int64_t)n1 | (int64_t)n2); break;
                    case TOK_EQV: ans = (double)(~((int64_t)n1 ^ (int64_t)n2)); break;
                    case TOK_SHL: ans = (double)((int64_t)n1 << (int64_t)n2); break;
                    case TOK_SHR: ans = (double)((int64_t)n1 >> (int64_t)n2); break;
                    case TOK_MOD:
                        if ((int64_t)n2 == 0) {
                            err->code = 11; err->message = "Division by zero in MOD"; return null_val;
                        }
                        ans = (double)((int64_t)n1 % (int64_t)n2);
                        break;
                    case TOK_BACKSLASH:
                        if ((int64_t)n2 == 0) {
                            err->code = 11; err->message = "Division by zero in \\"; return null_val;
                        }
                        ans = (double)((int64_t)n1 / (int64_t)n2);
                        break;
                    case TOK_POW:
                        if (n2 == (double)(int)n2 && n2 >= -100 && n2 <= 100) {
                            ans = fast_int_pow(n1, (int)n2);
                        } else if (n1 < 0.0 && n2 != (double)(int)n2) {
                            err->code = 5; err->message = "Illegal function call: negative base with fractional power"; return null_val;
                        } else {
                            ans = runtime_pow(n1, n2);
                        }
                        break;

                    default: {
                        BValue vstack[2];
                        vstack[0] = lval;
                        vstack[1] = rval;
                        size_t vptr = 2;
                        if (!eval_execute_op(vm, node->op, vstack, &vptr, err)) return null_val;
                        return vstack[0];
                    }
                }
                BValue res = { .type = VAL_NUMBER, .as.number = ans };
                return res;
            }

            BValue vstack[2];
            vstack[0] = lval;
            vstack[1] = rval;
            size_t vptr = 2;
            if (!eval_execute_op(vm, node->op, vstack, &vptr, err)) return null_val;
            return vstack[0];
        }
        case AST_NODE_UNARY_OP: {
            BValue val = eval_ast_evaluate(vm, node->left, err);
            if (err->code != 0) return null_val;
            if (node->op == TOK_UNARY_MINUS) {
                if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                    val.as.number = -val.as.number;
                }
            } else if (node->op == TOK_NOT) {
                if (val.type == VAL_NUMBER || val.type == VAL_INTEGER) {
                    val.as.number = (double)(~(int64_t)val.as.number);
                }
            }
            return val;
        }
        default:
            return null_val;
    }
}
