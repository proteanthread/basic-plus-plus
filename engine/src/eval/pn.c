// FILENAME: pn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (pn.h, rpn.h, eval_internal.h)
// Provides core logic and interface definitions for HP-based Prefix Polish Notation (PN) within BASIC++.

#include "eval/pn.h"
#include "eval/rpn.h"
#include "eval/eval_internal.h"
#include "runtime/variables.h"
#include "runtime/math/math.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "runtime/math/basic.h"
#include "lexer/lexer.h"
#include "lexer/lexer_internal.h"
#include "vm/vm.h"

// Global active PN context
static PnContext s_active_pn_context;

static uint64_t s_pn_rnd_seed = 123456789ULL;
static inline double pn_rand_float(void) {
    s_pn_rnd_seed = s_pn_rnd_seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return (double)(s_pn_rnd_seed >> 11) / (double)(1ULL << 53);
}

static void pn_release_val(BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_release(NULL, val.as.string);
    }
}

static void pn_retain_val(BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    }
}

void pn_reset(void) {
    pn_release_val(s_active_pn_context.x);
    pn_release_val(s_active_pn_context.y);
    pn_release_val(s_active_pn_context.z);
    pn_release_val(s_active_pn_context.t);
    pn_release_val(s_active_pn_context.last_x);

    runtime_memset(&s_active_pn_context, 0, sizeof(PnContext));
    s_active_pn_context.x.type = VAL_NUMBER;
    s_active_pn_context.y.type = VAL_NUMBER;
    s_active_pn_context.z.type = VAL_NUMBER;
    s_active_pn_context.t.type = VAL_NUMBER;
    s_active_pn_context.last_x.type = VAL_NUMBER;
    s_active_pn_context.wsize = 64;

    for (int i = 0; i < 16; i++) {
        s_active_pn_context.r[i].type = VAL_NUMBER;
        s_active_pn_context.r[i].as.number = 0.0;
    }
}

PnContext* pn_get_context(void) {
    return &s_active_pn_context;
}

static int pn_parse_hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

bool pn_reg_get(const char *name, BValue *out_val) {
    if (!name || !out_val) return false;
    const char *r = name;
    if (runtime_strncasecmp(r, "PN.", 3) == 0) r += 3;
    else if (runtime_strncasecmp(r, "PN_", 3) == 0) r += 3;

    if (runtime_strcasecmp(r, "X") == 0 || runtime_strcasecmp(r, "ACC") == 0) {
        *out_val = s_active_pn_context.x;
        pn_retain_val(*out_val);
        return true;
    }
    if (runtime_strcasecmp(r, "Y") == 0) {
        *out_val = s_active_pn_context.y;
        pn_retain_val(*out_val);
        return true;
    }
    if (runtime_strcasecmp(r, "Z") == 0) {
        *out_val = s_active_pn_context.z;
        pn_retain_val(*out_val);
        return true;
    }
    if (runtime_strcasecmp(r, "T") == 0) {
        *out_val = s_active_pn_context.t;
        pn_retain_val(*out_val);
        return true;
    }
    if (runtime_strcasecmp(r, "LASTX") == 0) {
        *out_val = s_active_pn_context.last_x;
        pn_retain_val(*out_val);
        return true;
    }
    if (runtime_strcasecmp(r, "WSIZE") == 0) {
        out_val->type = VAL_INTEGER;
        out_val->as.number = (double)s_active_pn_context.wsize;
        return true;
    }

    // Storage registers R0 .. RF
    if ((r[0] == 'R' || r[0] == 'r') && r[1] != '\0' && r[2] == '\0') {
        int idx = pn_parse_hex_digit(r[1]);
        if (idx >= 0 && idx < 16) {
            *out_val = s_active_pn_context.r[idx];
            pn_retain_val(*out_val);
            return true;
        }
    }

    return false;
}

bool pn_reg_set(const char *name, BValue val) {
    if (!name) return false;
    const char *r = name;
    if (runtime_strncasecmp(r, "PN.", 3) == 0) r += 3;
    else if (runtime_strncasecmp(r, "PN_", 3) == 0) r += 3;

    if (runtime_strcasecmp(r, "X") == 0 || runtime_strcasecmp(r, "ACC") == 0) {
        pn_retain_val(val);
        pn_release_val(s_active_pn_context.x);
        s_active_pn_context.x = val;
        return true;
    }
    if (runtime_strcasecmp(r, "Y") == 0) {
        pn_retain_val(val);
        pn_release_val(s_active_pn_context.y);
        s_active_pn_context.y = val;
        return true;
    }
    if (runtime_strcasecmp(r, "Z") == 0) {
        pn_retain_val(val);
        pn_release_val(s_active_pn_context.z);
        s_active_pn_context.z = val;
        return true;
    }
    if (runtime_strcasecmp(r, "T") == 0) {
        pn_retain_val(val);
        pn_release_val(s_active_pn_context.t);
        s_active_pn_context.t = val;
        return true;
    }
    if (runtime_strcasecmp(r, "LASTX") == 0) {
        pn_retain_val(val);
        pn_release_val(s_active_pn_context.last_x);
        s_active_pn_context.last_x = val;
        return true;
    }
    if (runtime_strcasecmp(r, "WSIZE") == 0) {
        int w = (int)val.as.number;
        if (w >= 1 && w <= 64) s_active_pn_context.wsize = w;
        return true;
    }

    // Storage registers R0 .. RF
    if ((r[0] == 'R' || r[0] == 'r') && r[1] != '\0' && r[2] == '\0') {
        int idx = pn_parse_hex_digit(r[1]);
        if (idx >= 0 && idx < 16) {
            pn_retain_val(val);
            pn_release_val(s_active_pn_context.r[idx]);
            s_active_pn_context.r[idx] = val;
            return true;
        }
    }

    return false;
}

bool pn_reg_get_by_index(int index, BValue *out_val) {
    if (!out_val || index < 0 || index >= 16) return false;
    *out_val = s_active_pn_context.r[index];
    pn_retain_val(*out_val);
    return true;
}

bool pn_reg_set_by_index(int index, BValue val) {
    if (index < 0 || index >= 16) return false;
    pn_retain_val(val);
    pn_release_val(s_active_pn_context.r[index]);
    s_active_pn_context.r[index] = val;
    return true;
}

// Checks if a token is a prefix operator or prefix function keyword
bool eval_is_pn_operator(BppToken tok) {
    if (tok.type == TOK_PLUS || tok.type == TOK_MINUS || tok.type == TOK_MUL ||
        tok.type == TOK_DIV || tok.type == TOK_BACKSLASH || tok.type == TOK_POW ||
        tok.type == TOK_LT || tok.type == TOK_GT || tok.type == TOK_LE ||
        tok.type == TOK_GE || tok.type == TOK_EQ || tok.type == TOK_NE ||
        tok.type == TOK_NOT || tok.type == TOK_MIN || tok.type == TOK_MAX ||
        tok.type == TOK_HYPOT || tok.type == TOK_REMAINDER || tok.type == TOK_ATAN2) {
        return true;
    }
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_AND || tok.as.keyword == KW_OR ||
            tok.as.keyword == KW_XOR || tok.as.keyword == KW_MOD ||
            tok.as.keyword == KW_NOT || tok.as.keyword == KW_NEG) {
            return true;
        }
    }
    if (tok.type == TOK_IDENT) {
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "NEG", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "MOD", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "ABS", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "SQR", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "SGN", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "INT", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "FIX", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "MIN", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "MAX", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "SIN", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "COS", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "TAN", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "LOG", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "EXP", 3) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "RND", 3) == 0) return true;
    }
    return false;
}

static BValue eval_pn_step(VMContext *vm, LexerContext *lex, int depth, BppError *out_err) {
    BValue null_val;
    runtime_memset(&null_val, 0, sizeof(null_val));
    null_val.type = VAL_NONE;

    if (depth > 64) {
        out_err->code = 14;
        out_err->message = "Out of string space / prefix nesting depth exceeded";
        return null_val;
    }

    BppToken tok = lex_next(lex);

    if (eval_is_pn_operator(tok)) {
        BppToken op_tok = tok;

        // Unary operators
        bool is_unary = false;
        if (op_tok.type == TOK_NOT ||
            (op_tok.type == TOK_KEYWORD && (op_tok.as.keyword == KW_NOT || op_tok.as.keyword == KW_NEG)) ||
            (op_tok.type == TOK_IDENT && (
                runtime_strncasecmp(op_tok.start, "NEG", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "ABS", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "SQR", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "SGN", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "INT", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "FIX", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "SIN", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "COS", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "TAN", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "LOG", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "EXP", 3) == 0 ||
                runtime_strncasecmp(op_tok.start, "RND", 3) == 0))) {
            is_unary = true;
        }

        BValue left = eval_pn_step(vm, lex, depth + 1, out_err);
        if (out_err->code != 0) return null_val;

        if (is_unary) {
            double l_num = (left.type == VAL_INTEGER || left.type == VAL_NUMBER) ? left.as.number : 0.0;
            BValue res;
            res.type = VAL_NUMBER;

            if (op_tok.type == TOK_NOT || (op_tok.type == TOK_KEYWORD && op_tok.as.keyword == KW_NOT)) {
                res.type = VAL_INTEGER;
                res.as.number = (double)(~((int32_t)l_num));
                return res;
            }
            if (op_tok.type == TOK_KEYWORD && op_tok.as.keyword == KW_NEG) {
                res.as.number = -l_num; return res;
            }
            if (op_tok.type == TOK_IDENT) {
                if (runtime_strncasecmp(op_tok.start, "NEG", 3) == 0) { res.as.number = -l_num; return res; }
                if (runtime_strncasecmp(op_tok.start, "ABS", 3) == 0) { res.as.number = (l_num < 0) ? -l_num : l_num; return res; }
                if (runtime_strncasecmp(op_tok.start, "SQR", 3) == 0) { res.as.number = runtime_sqrt(l_num); return res; }
                if (runtime_strncasecmp(op_tok.start, "SGN", 3) == 0) { res.type = VAL_INTEGER; res.as.number = (l_num > 0) ? 1.0 : ((l_num < 0) ? -1.0 : 0.0); return res; }
                if (runtime_strncasecmp(op_tok.start, "INT", 3) == 0) { res.type = VAL_INTEGER; res.as.number = runtime_floor(l_num); return res; }
                if (runtime_strncasecmp(op_tok.start, "FIX", 3) == 0) { res.type = VAL_INTEGER; res.as.number = (double)((int32_t)l_num); return res; }
                if (runtime_strncasecmp(op_tok.start, "SIN", 3) == 0) { res.as.number = runtime_sin(l_num); return res; }
                if (runtime_strncasecmp(op_tok.start, "COS", 3) == 0) { res.as.number = runtime_cos(l_num); return res; }
                if (runtime_strncasecmp(op_tok.start, "TAN", 3) == 0) { res.as.number = runtime_tan(l_num); return res; }
                if (runtime_strncasecmp(op_tok.start, "LOG", 3) == 0) { res.as.number = runtime_log(l_num); return res; }
                if (runtime_strncasecmp(op_tok.start, "EXP", 3) == 0) { res.as.number = runtime_exp(l_num); return res; }
                if (runtime_strncasecmp(op_tok.start, "RND", 3) == 0) { res.as.number = pn_rand_float(); return res; }
            }
        }

        BValue right = eval_pn_step(vm, lex, depth + 1, out_err);
        if (out_err->code != 0) return null_val;

        double l_num = (left.type == VAL_INTEGER || left.type == VAL_NUMBER) ? left.as.number : 0.0;
        double r_num = (right.type == VAL_INTEGER || right.type == VAL_NUMBER) ? right.as.number : 0.0;
        BValue res;
        res.type = VAL_NUMBER;

        if (op_tok.type == TOK_PLUS) {
            if (left.type == VAL_STRING && right.type == VAL_STRING) {
                res.type = VAL_STRING;
                res.as.string = str_concat(vm_get_str(vm), left.as.string, right.as.string);
                return res;
            }
            res.as.number = l_num + r_num;
            return res;
        }
        if (op_tok.type == TOK_MINUS) { res.as.number = l_num - r_num; return res; }
        if (op_tok.type == TOK_MUL) { res.as.number = l_num * r_num; return res; }
        if (op_tok.type == TOK_DIV) {
            if (r_num == 0.0) { out_err->code = 11; out_err->message = "Division by zero in prefix expression"; return null_val; }
            res.as.number = l_num / r_num; return res;
        }
        if (op_tok.type == TOK_BACKSLASH) {
            if ((int32_t)r_num == 0) { out_err->code = 11; out_err->message = "Division by zero in prefix expression"; return null_val; }
            res.type = VAL_INTEGER; res.as.number = (double)((int32_t)l_num / (int32_t)r_num); return res;
        }
        if (op_tok.type == TOK_POW) { res.as.number = runtime_pow(l_num, r_num); return res; }
        if (op_tok.type == TOK_EQ) { res.type = VAL_INTEGER; res.as.number = (l_num == r_num) ? -1.0 : 0.0; return res; }
        if (op_tok.type == TOK_NE) { res.type = VAL_INTEGER; res.as.number = (l_num != r_num) ? -1.0 : 0.0; return res; }
        if (op_tok.type == TOK_LT) { res.type = VAL_INTEGER; res.as.number = (l_num < r_num) ? -1.0 : 0.0; return res; }
        if (op_tok.type == TOK_LE) { res.type = VAL_INTEGER; res.as.number = (l_num <= r_num) ? -1.0 : 0.0; return res; }
        if (op_tok.type == TOK_GT) { res.type = VAL_INTEGER; res.as.number = (l_num > r_num) ? -1.0 : 0.0; return res; }
        if (op_tok.type == TOK_GE) { res.type = VAL_INTEGER; res.as.number = (l_num >= r_num) ? -1.0 : 0.0; return res; }
        if (op_tok.type == TOK_MIN) { res.as.number = (l_num < r_num) ? l_num : r_num; return res; }
        if (op_tok.type == TOK_MAX) { res.as.number = (l_num > r_num) ? l_num : r_num; return res; }
        if (op_tok.type == TOK_HYPOT) { res.as.number = runtime_hypot(l_num, r_num); return res; }
        if (op_tok.type == TOK_REMAINDER) { res.as.number = runtime_fmod(l_num, r_num); return res; }
        if (op_tok.type == TOK_ATAN2) { res.as.number = runtime_atan2(l_num, r_num); return res; }

        if (op_tok.type == TOK_KEYWORD || op_tok.type == TOK_IDENT) {
            int32_t il = (int32_t)l_num;
            int32_t ir = (int32_t)r_num;
            res.type = VAL_INTEGER;
            if ((op_tok.type == TOK_KEYWORD && op_tok.as.keyword == KW_AND) ||
                (op_tok.type == TOK_IDENT && runtime_strncasecmp(op_tok.start, "AND", 3) == 0)) {
                res.as.number = (double)(il & ir); return res;
            }
            if ((op_tok.type == TOK_KEYWORD && op_tok.as.keyword == KW_OR) ||
                (op_tok.type == TOK_IDENT && runtime_strncasecmp(op_tok.start, "OR", 2) == 0)) {
                res.as.number = (double)(il | ir); return res;
            }
            if ((op_tok.type == TOK_KEYWORD && op_tok.as.keyword == KW_XOR) ||
                (op_tok.type == TOK_IDENT && runtime_strncasecmp(op_tok.start, "XOR", 3) == 0)) {
                res.as.number = (double)(il ^ ir); return res;
            }
            if ((op_tok.type == TOK_KEYWORD && op_tok.as.keyword == KW_MOD) ||
                (op_tok.type == TOK_IDENT && runtime_strncasecmp(op_tok.start, "MOD", 3) == 0)) {
                if (ir == 0) { out_err->code = 11; out_err->message = "Division by zero in prefix expression"; return null_val; }
                res.as.number = (double)(il % ir); return res;
            }
        }
    }

    if (tok.type == TOK_LBRACKET) {
        BValue res = eval_pn_step(vm, lex, depth + 1, out_err);
        if (lex_peek(lex).type == TOK_RBRACKET) {
            lex_next(lex);
        }
        return res;
    }

    if (tok.type == TOK_PN_LITERAL) {
        char *sub_str = (char *)mem_scratch_alloc(vm_get_mem(vm), tok.length + 1);
        if (sub_str) {
            runtime_memcpy(sub_str, tok.as.string, tok.length);
            sub_str[tok.length] = '\0';
            LexerContext *sub_lex = lex_init(vm_get_mem(vm), sub_str);
            BValue res = eval_expression_pn(vm, sub_lex, out_err);
            lex_shutdown(sub_lex);
            return res;
        }
    }

    if (tok.type == TOK_NUMBER) {
        BValue res;
        res.type = VAL_NUMBER;
        res.as.number = tok.as.number;
        return res;
    }

    if (tok.type == TOK_STRING) {
        BValue res;
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), tok.start, tok.length);
        return res;
    }

    if (tok.type == TOK_IDENT) {
        BppToken op_tok = tok;
        char name_buf[128];
        size_t len = (op_tok.length < sizeof(name_buf) - 1) ? op_tok.length : sizeof(name_buf) - 1;
        runtime_memcpy(name_buf, op_tok.start, len);
        name_buf[len] = '\0';

        // Accumulate dotted members such as STACK.X, PN.ACC
        while (lex_peek(lex).type == TOK_PERIOD) {
            lex_next(lex);
            BppToken sub_tok = lex_next(lex);
            if (sub_tok.type == TOK_IDENT || sub_tok.type == TOK_KEYWORD) {
                char sub_name[64];
                size_t sub_len = (sub_tok.length < sizeof(sub_name) - 1) ? sub_tok.length : sizeof(sub_name) - 1;
                runtime_memcpy(sub_name, sub_tok.start, sub_len);
                sub_name[sub_len] = '\0';

                char combined[128];
                runtime_snprintf(combined, sizeof(combined), "%s.%s", name_buf, sub_name);
                runtime_strncpy(name_buf, combined, sizeof(name_buf) - 1);
                name_buf[sizeof(name_buf) - 1] = '\0';
            }
        }

        // 1. Check PN registers & in-scope bare registers (X, Y, Z, T, LASTX, R0..RF)
        BValue pn_val;
        if (pn_reg_get(name_buf, &pn_val)) {
            return pn_val;
        }

        // 2. Check paired math domain RPN / STACK registers
        BValue stk_val;
        if (rpn_reg_get(name_buf, &stk_val)) {
            return stk_val;
        }

        // 3. Fallback to standard variable lookup
        BValue *v = var_lookup(vm_get_var(vm), name_buf, false);
        if (v && v->type != VAL_NONE) {
            BValue res = *v;
            if (res.type == VAL_STRING && res.as.string) str_add_ref(res.as.string);
            return res;
        }
        BValue res;
        res.type = VAL_NUMBER;
        res.as.number = 0.0;
        return res;
    }

    out_err->code = 2;
    out_err->message = "Invalid token in prefix expression";
    return null_val;
}

// Evaluates a Prefix Polish Notation (PN) expression from the lexer stream
BValue eval_expression_pn(VMContext *vm, LexerContext *lex, BppError *out_err) {
    BValue res = eval_pn_step(vm, lex, 0, out_err);
    if (out_err->code != 0) return res;

    // Update PN.LASTX and PN.X
    pn_release_val(s_active_pn_context.last_x);
    s_active_pn_context.last_x = s_active_pn_context.x;
    pn_retain_val(s_active_pn_context.last_x);

    pn_release_val(s_active_pn_context.x);
    s_active_pn_context.x = res;
    pn_retain_val(s_active_pn_context.x);

    // Consume trailing ']' if present
    if (lex_peek(lex).type == TOK_RBRACKET) {
        lex_next(lex);
    }
    return res;
}

// Evaluates a PN expression string
BValue pn_eval_expr_string(VMContext *vm, const char *expr_str, BppError *out_err) {
    BValue null_val;
    runtime_memset(&null_val, 0, sizeof(null_val));
    null_val.type = VAL_NONE;

    if (!expr_str) {
        if (out_err) { out_err->code = 5; out_err->message = "Illegal function call: NULL prefix expression"; }
        return null_val;
    }

    LexerContext *lex = lex_init(vm_get_mem(vm), expr_str);
    if (!lex) {
        if (out_err) { out_err->code = 7; out_err->message = "Out of memory initializing lexer"; }
        return null_val;
    }
    BValue res = eval_expression_pn(vm, lex, out_err);
    lex_shutdown(lex);
    return res;
}
