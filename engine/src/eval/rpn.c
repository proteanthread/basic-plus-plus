// FILENAME: rpn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, funcreg.h, funcreg.c, struct.h, struct.c)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval_internal.h, map.h, map.c)
// Provides core logic and interface definitions for rpn within BASIC++.
//
// ---- Includes ----

#include "eval/eval_internal.h"
#include "runtime/variables.h"
#include "runtime/map.h"
#include "runtime/file.h"
#include "runtime/funcreg.h"
#include "core/struct.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/strings.h"
#include "eval/rpn.h"
#include "eval/pn.h"
#include "lexer/lexer.h"
#include "lexer/lexer_internal.h"
#include "vm/vm.h"

static RpnContext s_active_rpn_context;

static void rpn_release_val(BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_release(NULL, val.as.string);
    }
}

static void rpn_retain_val(BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    }
}

void rpn_reset(void) {
    rpn_release_val(s_active_rpn_context.x);
    rpn_release_val(s_active_rpn_context.y);
    rpn_release_val(s_active_rpn_context.z);
    rpn_release_val(s_active_rpn_context.t);
    rpn_release_val(s_active_rpn_context.last_x);

    for (int i = 0; i < 16; i++) {
        rpn_release_val(s_active_rpn_context.r[i]);
    }
    for (int i = 0; i < s_active_rpn_context.ext_depth; i++) {
        rpn_release_val(s_active_rpn_context.ext_stack[i]);
    }

    runtime_memset(&s_active_rpn_context, 0, sizeof(RpnContext));
    s_active_rpn_context.x.type = VAL_NUMBER;
    s_active_rpn_context.y.type = VAL_NUMBER;
    s_active_rpn_context.z.type = VAL_NUMBER;
    s_active_rpn_context.t.type = VAL_NUMBER;
    s_active_rpn_context.last_x.type = VAL_NUMBER;
    s_active_rpn_context.wsize = 64;

    for (int i = 0; i < 16; i++) {
        s_active_rpn_context.r[i].type = VAL_NUMBER;
    }
}

RpnContext* rpn_get_context(void) {
    return &s_active_rpn_context;
}

bool rpn_stack_push(BValue val) {
    rpn_release_val(s_active_rpn_context.t);
    s_active_rpn_context.t = s_active_rpn_context.z;
    s_active_rpn_context.z = s_active_rpn_context.y;
    s_active_rpn_context.y = s_active_rpn_context.x;
    s_active_rpn_context.x = val;
    rpn_retain_val(val);
    return true;
}

bool rpn_stack_pop(BValue *out_val) {
    if (!out_val) return false;
    *out_val = s_active_rpn_context.x;
    rpn_retain_val(*out_val);

    rpn_release_val(s_active_rpn_context.last_x);
    s_active_rpn_context.last_x = s_active_rpn_context.x;
    rpn_retain_val(s_active_rpn_context.last_x);

    s_active_rpn_context.x = s_active_rpn_context.y;
    s_active_rpn_context.y = s_active_rpn_context.z;
    s_active_rpn_context.z = s_active_rpn_context.t;
    return true;
}

void rpn_stack_swap(void) {
    BValue tmp = s_active_rpn_context.x;
    s_active_rpn_context.x = s_active_rpn_context.y;
    s_active_rpn_context.y = tmp;
}

void rpn_stack_rollup(void) {
    BValue tmp = s_active_rpn_context.x;
    s_active_rpn_context.x = s_active_rpn_context.y;
    s_active_rpn_context.y = s_active_rpn_context.z;
    s_active_rpn_context.z = s_active_rpn_context.t;
    s_active_rpn_context.t = tmp;
}

void rpn_stack_rolldn(void) {
    BValue tmp = s_active_rpn_context.t;
    s_active_rpn_context.t = s_active_rpn_context.z;
    s_active_rpn_context.z = s_active_rpn_context.y;
    s_active_rpn_context.y = s_active_rpn_context.x;
    s_active_rpn_context.x = tmp;
}

void rpn_stack_clear(void) {
    rpn_reset();
}

static int rpn_parse_hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static BValue* rpn_get_reg_ptr(const char *r) {
    if (runtime_strcasecmp(r, "X") == 0) return &s_active_rpn_context.x;
    if (runtime_strcasecmp(r, "Y") == 0) return &s_active_rpn_context.y;
    if (runtime_strcasecmp(r, "Z") == 0) return &s_active_rpn_context.z;
    if (runtime_strcasecmp(r, "T") == 0) return &s_active_rpn_context.t;
    if (runtime_strcasecmp(r, "LASTX") == 0) return &s_active_rpn_context.last_x;
    if ((r[0] == 'R' || r[0] == 'r') && r[1] != '\0' && r[2] == '\0') {
        int idx = rpn_parse_hex_digit(r[1]);
        if (idx >= 0 && idx < 16) return &s_active_rpn_context.r[idx];
    }
    return NULL;
}

bool rpn_reg_get(const char *name, BValue *out_val) {
    if (!name || !out_val) return false;
    const char *r = name;
    if (runtime_strncasecmp(r, "STACK.", 6) == 0 || runtime_strncasecmp(r, "STACK_", 6) == 0) r += 6;
    else if (runtime_strncasecmp(r, "RPN.", 4) == 0 || runtime_strncasecmp(r, "RPN_", 4) == 0) r += 4;
    BValue *ptr = rpn_get_reg_ptr(r);
    if (ptr) { *out_val = *ptr; rpn_retain_val(*out_val); return true; }
    if (runtime_strcasecmp(r, "WSIZE") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = (double)s_active_rpn_context.wsize; return true;
    }
    if (runtime_strcasecmp(r, "CARRY") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = s_active_rpn_context.carry ? -1.0 : 0.0; return true;
    }
    if (runtime_strcasecmp(r, "OVERFLOW") == 0) {
        out_val->type = VAL_INTEGER; out_val->as.number = s_active_rpn_context.overflow ? -1.0 : 0.0; return true;
    }
    return false;
}

bool rpn_reg_set(const char *name, BValue val) {
    if (!name) return false;
    const char *r = name;
    if (runtime_strncasecmp(r, "STACK.", 6) == 0 || runtime_strncasecmp(r, "STACK_", 6) == 0) r += 6;
    else if (runtime_strncasecmp(r, "RPN.", 4) == 0 || runtime_strncasecmp(r, "RPN_", 4) == 0) r += 4;
    BValue *ptr = rpn_get_reg_ptr(r);
    if (ptr) { rpn_retain_val(val); rpn_release_val(*ptr); *ptr = val; return true; }
    if (runtime_strcasecmp(r, "WSIZE") == 0) {
        int w = (int)val.as.number; if (w >= 1 && w <= 64) s_active_rpn_context.wsize = w; return true;
    }
    if (runtime_strcasecmp(r, "CARRY") == 0) {
        s_active_rpn_context.carry = (val.type == VAL_INTEGER || val.type == VAL_NUMBER) && (val.as.number != 0.0); return true;
    }
    if (runtime_strcasecmp(r, "OVERFLOW") == 0) {
        s_active_rpn_context.overflow = (val.type == VAL_INTEGER || val.type == VAL_NUMBER) && (val.as.number != 0.0); return true;
    }
    return false;
}

bool rpn_reg_get_by_index(int index, BValue *out_val) {
    if (!out_val || index < 0 || index >= 16) return false;
    *out_val = s_active_rpn_context.r[index];
    rpn_retain_val(*out_val);
    return true;
}

bool rpn_reg_set_by_index(int index, BValue val) {
    if (index < 0 || index >= 16) return false;
    rpn_retain_val(val);
    rpn_release_val(s_active_rpn_context.r[index]);
    s_active_rpn_context.r[index] = val;
    return true;
}

static inline bool eval_is_clause_delimiter(BppToken tok) {
    if (tok.type == TOK_KEYWORD) {
        BppKeywordId kw = tok.as.keyword;
        return (kw == KW_THEN || kw == KW_ELSE || kw == KW_TO || kw == KW_STEP ||
                kw == KW_AS || kw == KW_GOTO || kw == KW_GOSUB || kw == KW_IF ||
                kw == KW_UNLESS || kw == KW_UNTIL);
    }
    if (tok.type == TOK_IDENT && tok.start && tok.length > 0) {
        if (tok.length == 2 && runtime_strncasecmp(tok.start, "TO", 2) == 0) return true;
        if (tok.length == 2 && runtime_strncasecmp(tok.start, "AS", 2) == 0) return true;
        if (tok.length == 2 && runtime_strncasecmp(tok.start, "IF", 2) == 0) return true;
        if (tok.length == 3 && runtime_strncasecmp(tok.start, "FOR", 3) == 0) return true;
        if (tok.length == 4 && runtime_strncasecmp(tok.start, "THEN", 4) == 0) return true;
        if (tok.length == 4 && runtime_strncasecmp(tok.start, "ELSE", 4) == 0) return true;
        if (tok.length == 4 && runtime_strncasecmp(tok.start, "STEP", 4) == 0) return true;
        if (tok.length == 4 && runtime_strncasecmp(tok.start, "GOTO", 4) == 0) return true;
        if (tok.length == 5 && runtime_strncasecmp(tok.start, "GOSUB", 5) == 0) return true;
        if (tok.length == 5 && runtime_strncasecmp(tok.start, "UNTIL", 5) == 0) return true;
        if (tok.length == 5 && runtime_strncasecmp(tok.start, "WHILE", 5) == 0) return true;
        if (tok.length == 6 && runtime_strncasecmp(tok.start, "UNLESS", 6) == 0) return true;
    }
    return false;
}

BValue eval_expression_rpn(VMContext *vm, LexerContext *lex, BppError *out_err) {
    BValue null_val;
    runtime_memset(&null_val, 0, sizeof(null_val));

    MemoryContext *mem = vm_get_mem(vm);
    VariableContext *var = vm_get_var(vm);

    // Allocate stacks from local stack
    BValue val_stack[MAX_EVAL_DEPTH];
    runtime_memset(val_stack, 0, sizeof(val_stack));


    size_t val_ptr = 0;
    int open_parens = 0;
    BppToken tok = lex_peek(lex);

    while (tok.type != TOK_EOF && tok.type != TOK_EOL && tok.type != TOK_COMMA &&
           tok.type != TOK_SEMICOLON && (tok.type != TOK_RPAREN || open_parens > 0) &&
           tok.type != TOK_RBRACKET &&
           (open_parens > 0 || !eval_is_clause_delimiter(tok))) {

        // Stop parsing if we see 'AT' identifier
        if (tok.type == TOK_IDENT && tok.length == 2 &&
            (tok.start[0] == 'A' || tok.start[0] == 'a') &&
            (tok.start[1] == 'T' || tok.start[1] == 't')) {
            break;
        }

        // Read the peeked token
        lex_next(lex);

        if (tok.type == TOK_NUMBER) {
            BValue val;
            val.type = VAL_NUMBER;
            val.as.number = tok.as.number;
            val_stack[val_ptr++] = val;
        } else if (tok.type == TOK_STRING) {
            BppStringRef str_ref = str_create(vm_get_str(vm), tok.as.string, tok.length);
            BValue val;
            val.type = VAL_STRING;
            val.as.string = str_ref;
            val_stack[val_ptr++] = val;
        } else if (tok.type == TOK_RPN_LITERAL) {
            char *rpn_str = (char *)mem_scratch_alloc(mem, tok.length + 1);
            if (!rpn_str) {
                out_err->code = 14;
                out_err->message = "Scratch memory exhausted";
                return null_val;
            }
            runtime_memcpy(rpn_str, tok.as.string, tok.length);
            rpn_str[tok.length] = '\0';

            LexerContext *rpn_lex = lex_init(mem, rpn_str);
            BValue res = eval_expression_rpn(vm, rpn_lex, out_err);
            lex_shutdown(rpn_lex);
            if (out_err->code != 0) return null_val;

            val_stack[val_ptr++] = res;
        } else if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD || tok.type == TOK_MIN || tok.type == TOK_MAX || tok.type == TOK_HYPOT || tok.type == TOK_ATAN2 || tok.type == TOK_REMAINDER) {
            // Variable or function lookup
            char name_buf[256];
            size_t copy_len = (tok.length < sizeof(name_buf) - 1) ? tok.length : sizeof(name_buf) - 1;
            runtime_memcpy(name_buf, tok.start, copy_len);
            name_buf[copy_len] = '\0';

            // Accumulate dotted members such as REG.AX
            while (lex_peek(lex).type == TOK_PERIOD) {
                lex_next(lex);
                BppToken sub_tok = lex_next(lex);
                if (sub_tok.type == TOK_IDENT || sub_tok.type == TOK_KEYWORD) {
                    char sub_name[64];
                    size_t sub_len = (sub_tok.length < sizeof(sub_name) - 1) ? sub_tok.length : sizeof(sub_name) - 1;
                    runtime_memcpy(sub_name, sub_tok.start, sub_len);
                    sub_name[sub_len] = '\0';

                    char combined[256];
                    runtime_snprintf(combined, sizeof(combined), "%s.%s", name_buf, sub_name);
                    runtime_strncpy(name_buf, combined, sizeof(name_buf) - 1);
                    name_buf[sizeof(name_buf) - 1] = '\0';
                }
            }

            if (runtime_strcasecmp(name_buf, "NEG") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on NEG"; return null_val; }
                val_stack[val_ptr - 1].as.number = -val_stack[val_ptr - 1].as.number;
            } else if (runtime_strcasecmp(name_buf, "ABS") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on ABS"; return null_val; }
                val_stack[val_ptr - 1].as.number = runtime_fabs(val_stack[val_ptr - 1].as.number);
            } else if (runtime_strcasecmp(name_buf, "SQR") == 0 || runtime_strcasecmp(name_buf, "SQRT") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on SQR"; return null_val; }
                double v = val_stack[val_ptr - 1].as.number;
                val_stack[val_ptr - 1].as.number = (v >= 0.0) ? runtime_sqrt(v) : 0.0;
            } else if (runtime_strcasecmp(name_buf, "SGN") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on SGN"; return null_val; }
                double v = val_stack[val_ptr - 1].as.number;
                val_stack[val_ptr - 1].as.number = (v > 0.0) ? 1.0 : ((v < 0.0) ? -1.0 : 0.0);
            } else if (runtime_strcasecmp(name_buf, "INT") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on INT"; return null_val; }
                val_stack[val_ptr - 1].as.number = runtime_floor(val_stack[val_ptr - 1].as.number);
            } else if (runtime_strcasecmp(name_buf, "FIX") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on FIX"; return null_val; }
                double v = val_stack[val_ptr - 1].as.number;
                val_stack[val_ptr - 1].as.number = (v >= 0.0) ? runtime_floor(v) : runtime_ceil(v);
            } else if (runtime_strcasecmp(name_buf, "SIN") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on SIN"; return null_val; }
                val_stack[val_ptr - 1].as.number = runtime_sin(val_stack[val_ptr - 1].as.number);
            } else if (runtime_strcasecmp(name_buf, "COS") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on COS"; return null_val; }
                val_stack[val_ptr - 1].as.number = runtime_cos(val_stack[val_ptr - 1].as.number);
            } else if (runtime_strcasecmp(name_buf, "TAN") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on TAN"; return null_val; }
                val_stack[val_ptr - 1].as.number = runtime_tan(val_stack[val_ptr - 1].as.number);
            } else if (runtime_strcasecmp(name_buf, "LOG") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on LOG"; return null_val; }
                double v = val_stack[val_ptr - 1].as.number;
                val_stack[val_ptr - 1].as.number = (v > 0.0) ? runtime_log(v) : 0.0;
            } else if (runtime_strcasecmp(name_buf, "EXP") == 0) {
                if (val_ptr < 1) { out_err->code = 24; out_err->message = "RPN stack underflow on EXP"; return null_val; }
                val_stack[val_ptr - 1].as.number = runtime_exp(val_stack[val_ptr - 1].as.number);
            } else if (runtime_strcasecmp(name_buf, "MIN") == 0) {
                if (val_ptr < 2) { out_err->code = 24; out_err->message = "RPN stack underflow on MIN"; return null_val; }
                double b = val_stack[--val_ptr].as.number;
                double a = val_stack[val_ptr - 1].as.number;
                val_stack[val_ptr - 1].as.number = (a < b) ? a : b;
            } else if (runtime_strcasecmp(name_buf, "MAX") == 0) {
                if (val_ptr < 2) { out_err->code = 24; out_err->message = "RPN stack underflow on MAX"; return null_val; }
                double b = val_stack[--val_ptr].as.number;
                double a = val_stack[val_ptr - 1].as.number;
                val_stack[val_ptr - 1].as.number = (a > b) ? a : b;
            } else if (runtime_strcasecmp(name_buf, "MOD") == 0) {
                if (val_ptr < 2) { out_err->code = 24; out_err->message = "RPN stack underflow on MOD"; return null_val; }
                double b = val_stack[--val_ptr].as.number;
                double a = val_stack[val_ptr - 1].as.number;
                val_stack[val_ptr - 1].as.number = (b != 0.0) ? runtime_fmod(a, b) : 0.0;
            } else if (runtime_strcasecmp(name_buf, "DUP") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on DUP";
                    return null_val;
                }
                BValue val = val_stack[val_ptr - 1];
                if (val.type == VAL_STRING && val.as.string) {
                    str_add_ref(val.as.string);
                }
                val_stack[val_ptr++] = val;
            } else if (runtime_strcasecmp(name_buf, "DROP") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on DROP";
                    return null_val;
                }
                val_ptr--;
                BValue val = val_stack[val_ptr];
                if (val.type == VAL_STRING && val.as.string) {
                    str_release(vm_get_str(vm), val.as.string);
                }
            } else if (runtime_strcasecmp(name_buf, "SWAP") == 0) {
                if (val_ptr < 2) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on SWAP";
                    return null_val;
                }
                BValue temp = val_stack[val_ptr - 1];
                val_stack[val_ptr - 1] = val_stack[val_ptr - 2];
                val_stack[val_ptr - 2] = temp;
            } else if (runtime_strcasecmp(name_buf, "OVER") == 0) {
                if (val_ptr < 2) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on OVER";
                    return null_val;
                }
                BValue val = val_stack[val_ptr - 2];
                if (val.type == VAL_STRING && val.as.string) {
                    str_add_ref(val.as.string);
                }
                val_stack[val_ptr++] = val;
            } else if (runtime_strcasecmp(name_buf, "ROT") == 0) {
                if (val_ptr < 3) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on ROT";
                    return null_val;
                }
                BValue a = val_stack[val_ptr - 3];
                BValue b = val_stack[val_ptr - 2];
                BValue c = val_stack[val_ptr - 1];
                val_stack[val_ptr - 3] = b;
                val_stack[val_ptr - 2] = c;
                val_stack[val_ptr - 1] = a;
            } else if (runtime_strcasecmp(name_buf, "CLEAR") == 0) {
                while (val_ptr > 0) {
                    val_ptr--;
                    BValue val = val_stack[val_ptr];
                    if (val.type == VAL_STRING && val.as.string) {
                        str_release(vm_get_str(vm), val.as.string);
                    }
                }
            } else if (runtime_strcasecmp(name_buf, "DEPTH") == 0) {
                BValue val;
                val.type = VAL_NUMBER;
                val.as.number = (double)val_ptr;
                val_stack[val_ptr++] = val;
            } else if (runtime_strcasecmp(name_buf, "PICK") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on PICK";
                    return null_val;
                }
                BValue idx_val = val_stack[--val_ptr];
                if (idx_val.type != VAL_NUMBER) {
                    out_err->code = 13;
                    out_err->message = "PICK index must be numeric";
                    return null_val;
                }
                int n = (int)idx_val.as.number;
                if (n < 0 || n >= (int)val_ptr) {
                    out_err->code = 9;
                    out_err->message = "PICK index out of bounds";
                    return null_val;
                }
                BValue picked = val_stack[val_ptr - 1 - n];
                if (picked.type == VAL_STRING && picked.as.string) {
                    str_add_ref(picked.as.string);
                }
                val_stack[val_ptr++] = picked;
            } else if (runtime_strcasecmp(name_buf, "ROLL") == 0) {
                if (val_ptr < 1) {
                    out_err->code = 24;
                    out_err->message = "RPN stack underflow on ROLL";
                    return null_val;
                }

                BValue idx_val = val_stack[--val_ptr];
                if (idx_val.type != VAL_NUMBER) {
                    out_err->code = 13;
                    out_err->message = "ROLL index must be numeric";
                    return null_val;
                }
                int n = (int)idx_val.as.number;
                if (n < 0 || n >= (int)val_ptr) {
                    out_err->code = 9;
                    out_err->message = "ROLL index out of bounds";
                    return null_val;
                }
                if (n > 0) {
                    BValue rolled = val_stack[(int)val_ptr - 1 - n];
                    for (int i = (int)val_ptr - 1 - n; i < (int)val_ptr - 1; ++i) {
                        val_stack[i] = val_stack[i + 1];
                    }
                    val_stack[(int)val_ptr - 1] = rolled;
                }
            } else {
                // Check if followed by '('
                if (lex_peek(lex).type == TOK_LPAREN) {
                    lex_next(lex); // Consume '('
                    if (eval_is_builtin_function(name_buf)) {
                        BValue val = eval_builtin_function(vm, name_buf, lex, true, out_err);
                        if (out_err->code != 0) return null_val;
                        val_stack[val_ptr++] = val;
                    } else if (!arr_exists(vm_get_arr(vm), name_buf)) {
                        BValue args[8];
                        int argc = 0;
                        while (true) {
                            BppToken next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            }
                            if (argc >= 8) {
                                for (int ci = 0; ci < argc; ci++) {
                                    if (args[ci].type == VAL_STRING && args[ci].as.string)
                                        str_release(vm_get_str(vm), args[ci].as.string);
                                }
                                out_err->code = 2;
                                out_err->message = "Too many arguments in function call";
                                return null_val;
                            }
                            args[argc++] = eval_expression(vm, lex, out_err);
                            if (out_err->code != 0) {
                                for (int ci = 0; ci < argc; ci++) {
                                    if (args[ci].type == VAL_STRING && args[ci].as.string)
                                        str_release(vm_get_str(vm), args[ci].as.string);
                                }
                                return null_val;
                            }
                            
                            next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_COMMA) {
                                lex_next(lex);
                            } else if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            } else {
                                out_err->code = 2;
                                out_err->message = "Expected ',' or ')'";
                                return null_val;
                            }
                        }
                        BValue val = invoke_user_function(vm, name_buf, args, argc, out_err);
                        if (out_err->code != 0) return null_val;
                        val_stack[val_ptr++] = val;
                    } else {
                        // Array access
                        int indices[4];
                        int num_indices = 0;
                        while (true) {
                            BppToken next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            }
                            if (num_indices >= 4) {
                                out_err->code = 9;
                                out_err->message = "Subscript out of range (max 4 dimensions)";
                                return null_val;
                            }
                            BValue idx_val = eval_expression(vm, lex, out_err);
                            if (out_err->code != 0) return null_val;
                            indices[num_indices++] = (int)idx_val.as.number;
                            
                            next_tok = lex_peek(lex);
                            if (next_tok.type == TOK_COMMA) {
                                lex_next(lex);
                            } else if (next_tok.type == TOK_RPAREN) {
                                lex_next(lex);
                                break;
                            } else {
                                out_err->code = 2;
                                out_err->message = "Expected ',' or ')'";
                                return null_val;
                            }
                        }
                        BValue *elem = arr_get_element(vm_get_arr(vm), name_buf, num_indices, indices, out_err);
                        if (out_err->code != 0 || !elem) {
                            return null_val;
                        }
                        BValue val = *elem;
                        if (val.type == VAL_STRING && val.as.string) {
                            str_add_ref(val.as.string);
                        } else if (val.type == VAL_MAP && val.as.map) {
                            map_add_ref(val.as.map);
                        }
                        val_stack[val_ptr++] = val;
                    }
                } else {
                    // 1. Check RPN / STACK registers and in-scope bare names (X, Y, Z, T, LASTX, R0..RF)
                    BValue stk_val;
                    if (rpn_reg_get(name_buf, &stk_val)) {
                        val_stack[val_ptr++] = stk_val;
                        tok = lex_peek(lex);
                        continue;
                    }

                    // 2. Check paired math domain PN registers
                    BValue pn_val;
                    if (pn_reg_get(name_buf, &pn_val)) {
                        val_stack[val_ptr++] = pn_val;
                        tok = lex_peek(lex);
                        continue;
                    }

                    // Simple variable lookup
                    BValue *v = var_lookup(var, name_buf, false);
                    if (!v) {
                        char base_name[256];
                        char member_chain[8][64];
                        int member_count = 0;
                        eval_split_member_chain(name_buf, runtime_strlen(name_buf), base_name, sizeof(base_name), member_chain, &member_count);
                        if (member_count > 0) {
                            v = var_lookup(var, base_name, false);
                            if (v) {
                                BValue val = *v;
                                if (val.type == VAL_STRING && val.as.string) str_add_ref(val.as.string);
                                else if (val.type == VAL_MAP && val.as.map) map_add_ref(val.as.map);
                                
                                // Walk up to the last member
                                bool walk_err = false;
                                for (int m = 0; m < member_count - 1; m++) {
                                    if (val.type != VAL_MAP || !val.as.map) {
                                        walk_err = true; break;
                                    }
                                    BValue next_val;
                                    if (!map_get(val.as.map, member_chain[m], &next_val)) {
                                        walk_err = true; break;
                                    }
                                    BValue copy = next_val;
                                    if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                    else if (copy.type == VAL_MAP && copy.as.map) map_add_ref(copy.as.map);
                                    
                                    if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                                    else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                                    val = copy;
                                }
                                
                                if (!walk_err) {
                                    // Check if followed by '(' -> Method call
                                    if (lex_peek(lex).type == TOK_LPAREN) {
                                        lex_next(lex); // Consume '('
                                        if (val.type == VAL_MAP && val.as.map) {
                                            BValue type_val;
                                            if (map_get(val.as.map, "__type__", &type_val) && type_val.type == VAL_STRING && type_val.as.string) {
                                                char fully_qualified_method[512];
                                                runtime_snprintf(fully_qualified_method, sizeof(fully_qualified_method), "%s.%s",
                                                         str_data(type_val.as.string), member_chain[member_count - 1]);
                                                
                                                BValue args[9];
                                                int argc = 0;
                                                args[argc++] = val;
                                                map_add_ref(val.as.map);
                                                
                                                while (true) {
                                                    BppToken next_tok = lex_peek(lex);
                                                    if (next_tok.type == TOK_RPAREN) {
                                                        lex_next(lex);
                                                        break;
                                                    }
                                                    if (argc >= 9) {
                                                        walk_err = true; break;
                                                    }
                                                    args[argc++] = eval_expression(vm, lex, out_err);
                                                    if (out_err->code != 0) {
                                                        walk_err = true; break;
                                                    }
                                                    next_tok = lex_peek(lex);
                                                    if (next_tok.type == TOK_COMMA) {
                                                        lex_next(lex);
                                                    } else if (next_tok.type == TOK_RPAREN) {
                                                        lex_next(lex);
                                                        break;
                                                    } else {
                                                        walk_err = true; break;
                                                    }
                                                }
                                                
                                                if (!walk_err) {
                                                    BValue ret_val = invoke_user_function(vm, fully_qualified_method, args, argc, out_err);
                                                    for (int i = 0; i < argc; i++) {
                                                        if (args[i].type == VAL_STRING) str_release(vm_get_str(vm), args[i].as.string);
                                                        else if (args[i].type == VAL_MAP) map_release(vm_get_str(vm), args[i].as.map);
                                                    }
                                                    if (out_err->code == 0) {
                                                        val_stack[val_ptr++] = ret_val;
                                                        tok = lex_peek(lex);
                                                        continue;
                                                    }
                                                } else {
                                                    // Cleanup leaked args on walk_err
                                                    for (int i = 0; i < argc; i++) {
                                                        if (args[i].type == VAL_STRING && args[i].as.string) str_release(vm_get_str(vm), args[i].as.string);
                                                        else if (args[i].type == VAL_MAP && args[i].as.map) map_release(vm_get_str(vm), args[i].as.map);
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        // Standard field lookup on last member
                                        int m = member_count - 1;
                                        if (val.type == VAL_MAP && val.as.map) {
                                            BValue next_val;
                                            if (map_get(val.as.map, member_chain[m], &next_val)) {
                                                BValue copy = next_val;
                                                if (copy.type == VAL_STRING && copy.as.string) str_add_ref(copy.as.string);
                                                else if (copy.type == VAL_MAP && copy.as.map) map_add_ref(copy.as.map);
                                                
                                                map_release(vm_get_str(vm), val.as.map);
                                                val_stack[val_ptr++] = copy;
                                                tok = lex_peek(lex);
                                                continue;
                                            }
                                        }
                                    }
                                }
                                
                                if (val.type == VAL_MAP && val.as.map) map_release(vm_get_str(vm), val.as.map);
                                else if (val.type == VAL_STRING && val.as.string) str_release(vm_get_str(vm), val.as.string);
                            }
                        }
                        
                        // Return uninitialized variable (numeric 0.0)
                        BValue val;
                        val.type = VAL_NUMBER;
                        val.as.number = 0.0;
                        val_stack[val_ptr++] = val;
                    } else {
                        BValue val = *v;
                        if (val.type == VAL_STRING && val.as.string) {
                            str_add_ref(val.as.string);
                        } else if (val.type == VAL_MAP && val.as.map) {
                            map_add_ref(val.as.map);
                        }
                        val_stack[val_ptr++] = val;
                    }
                }
            }
        } else if (eval_is_operator(tok.type)) {
            BppTokenType op = tok.type;
            if (val_ptr == 1 && op == TOK_MINUS) {
                op = TOK_UNARY_MINUS;
            }
            if (!eval_execute_op(vm, op, val_stack, &val_ptr, out_err)) {
                return null_val;
            }
        } else if (tok.type == TOK_LPAREN) {
            open_parens++;
        } else if (tok.type == TOK_RPAREN) {
            open_parens--;
        } else {
            out_err->code = 2;
            out_err->message = "Unexpected token in RPN expression";
            return null_val;
        }

        tok = lex_peek(lex);
    }

    if (val_ptr == 0) {
        BValue zero;
        zero.type = VAL_NUMBER;
        zero.as.number = 0.0;
        return zero;
    }

    // Sync HP stack registers: X, Y, Z, T, LASTX
    rpn_release_val(s_active_rpn_context.last_x);
    s_active_rpn_context.last_x = s_active_rpn_context.x;
    rpn_retain_val(s_active_rpn_context.last_x);

    rpn_release_val(s_active_rpn_context.x);
    s_active_rpn_context.x = val_stack[val_ptr - 1];
    rpn_retain_val(s_active_rpn_context.x);

    if (val_ptr > 1) {
        rpn_release_val(s_active_rpn_context.y);
        s_active_rpn_context.y = val_stack[val_ptr - 2];
        rpn_retain_val(s_active_rpn_context.y);
    }
    if (val_ptr > 2) {
        rpn_release_val(s_active_rpn_context.z);
        s_active_rpn_context.z = val_stack[val_ptr - 3];
        rpn_retain_val(s_active_rpn_context.z);
    }
    if (val_ptr > 3) {
        rpn_release_val(s_active_rpn_context.t);
        s_active_rpn_context.t = val_stack[val_ptr - 4];
        rpn_retain_val(s_active_rpn_context.t);
    }

    return val_stack[val_ptr - 1];
}

// Evaluates an RPN expression string
BValue rpn_eval_expr_string(VMContext *vm, const char *expr_str, BppError *out_err) {
    BValue null_val;
    runtime_memset(&null_val, 0, sizeof(null_val));
    null_val.type = VAL_NONE;

    if (!expr_str) {
        if (out_err) { out_err->code = 5; out_err->message = "Illegal function call: NULL RPN expression"; }
        return null_val;
    }

    LexerContext *lex = lex_init(vm_get_mem(vm), expr_str);
    if (!lex) {
        if (out_err) { out_err->code = 7; out_err->message = "Out of memory initializing lexer"; }
        return null_val;
    }
    BValue res = eval_expression_rpn(vm, lex, out_err);
    lex_shutdown(lex);
    return res;
}
