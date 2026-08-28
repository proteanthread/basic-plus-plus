// FILENAME: helpers.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (num_format.h, num_format.c, variables.h, variables.c)
// NEEDS: libengine (eval_internal.h)
// Provides core logic and interface definitions for helpers within BASIC++.
//
// ---- Includes ----

#include "eval/eval_internal.h"
#include "runtime/variables.h"
#include "runtime/num_format.h"

BValue eval_parse_string_slice(VMContext *vm, LexerContext *lex, const char *var_name, BppTokenType open_tok, BppError *out_err) {
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_STRING;


    // Consume opening token '(' or '['
    lex_next(lex);

    // Look up the scalar variable value
    BValue *var_val = var_lookup(vm_get_var(vm), var_name, false);
    const char *str_val = "";
    size_t orig_len = 0;
    if (var_val && var_val->type == VAL_STRING && var_val->as.string) {
        str_val = str_data(var_val->as.string);
        orig_len = str_len(var_val->as.string);
    }

    int start = 1;
    int end = (int)orig_len;

    BppTokenType close_tok = (open_tok == TOK_LPAREN) ? TOK_RPAREN : TOK_RBRACKET;

    // Parse bounds
    BppToken next = lex_peek(lex);
    if (next.type == TOK_KEYWORD && next.as.keyword == KW_TO) {
        // Case: (TO end) or [TO end]
        lex_next(lex); // Consume 'TO'
        if (lex_peek(lex).type != close_tok) {
            BValue end_val = eval_expression(vm, lex, out_err);
            if (out_err->code != 0) return res;
            if (end_val.type == VAL_STRING) {
                if (end_val.as.string) str_release(vm_get_str(vm), end_val.as.string);
                out_err->code = 13; out_err->message = "String slice bound must be numeric";
                return res;
            }
            end = (int)end_val.as.number;
        }
    } else if (next.type == close_tok) {
        // Case: () or [] -> full string
        // Nothing to do, defaults are 1 and str_len
    } else {
        // Parse first bound
        BValue start_val = eval_expression(vm, lex, out_err);
        if (out_err->code != 0) return res;
        if (start_val.type == VAL_STRING) {
            if (start_val.as.string) str_release(vm_get_str(vm), start_val.as.string);
            out_err->code = 13; out_err->message = "String slice bound must be numeric";
            return res;
        }
        start = (int)start_val.as.number;

        next = lex_peek(lex);
        if (next.type == TOK_KEYWORD && next.as.keyword == KW_TO) {
            lex_next(lex); // Consume 'TO'
            if (lex_peek(lex).type != close_tok) {
                BValue end_val = eval_expression(vm, lex, out_err);
                if (out_err->code != 0) return res;
                if (end_val.type == VAL_STRING) {
                    if (end_val.as.string) str_release(vm_get_str(vm), end_val.as.string);
                    out_err->code = 13; out_err->message = "String slice bound must be numeric";
                    return res;
                }
                end = (int)end_val.as.number;
            }
        } else if (next.type == TOK_COMMA) {
            // Atari style A$[start, end]
            lex_next(lex); // Consume ','
            BValue end_val = eval_expression(vm, lex, out_err);
            if (out_err->code != 0) return res;
            if (end_val.type == VAL_STRING) {
                if (end_val.as.string) str_release(vm_get_str(vm), end_val.as.string);
                out_err->code = 13; out_err->message = "String slice bound must be numeric";
                return res;
            }
            end = (int)end_val.as.number;
        } else {
            if (open_tok == TOK_LBRACKET) {
                // HP 2000 / Super BASIC bracket slice: A$[start] is from start to end of string
                end = (int)orig_len;
            } else {
                // Single character index: end = start
                end = start;
            }
        }
    }

    // Consume closing token
    if (lex_peek(lex).type != close_tok) {
        out_err->code = 2; out_err->message = (open_tok == TOK_LPAREN) ? "Expected ')' in slice" : "Expected ']' in slice";
        return res;
    }
    lex_next(lex);

    // Clip bounds (1-based, inclusive)
    if (start < 1) start = 1;
    if (end > (int)orig_len) end = (int)orig_len;

    if (start > (int)orig_len || end < start) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    } else {
        size_t slice_len = (size_t)(end - start + 1);
        res.as.string = str_create(vm_get_str(vm), str_val + (start - 1), slice_len);
    }

    return res;
}

void eval_split_member_chain(const char *start, size_t len, char *var_name, size_t var_name_max, char member_chain[8][64], int *member_count) {
    *member_count = 0;
    
    // Find first dot
    size_t dot_idx = 0;
    while (dot_idx < len && start[dot_idx] != '.') {
        dot_idx++;
    }
    
    if (dot_idx == len) {
        // No dot: single variable name
        size_t clen = (len < var_name_max - 1) ? len : var_name_max - 1;
        runtime_memcpy(var_name, start, clen);
        var_name[clen] = '\0';
        return;
    }
    
    // Copy var_name
    size_t clen = (dot_idx < var_name_max - 1) ? dot_idx : var_name_max - 1;
    runtime_memcpy(var_name, start, clen);
    var_name[clen] = '\0';
    
    // Parse members
    size_t i = dot_idx + 1;
    while (i < len && *member_count < 8) {
        size_t next_dot = i;
        while (next_dot < len && start[next_dot] != '.') {
            next_dot++;
        }
        
        size_t mlen = next_dot - i;
        size_t copy_mlen = (mlen < 63) ? mlen : 63;
        runtime_memcpy(member_chain[*member_count], start + i, copy_mlen);
        member_chain[*member_count][copy_mlen] = '\0';
        (*member_count)++;
        
        i = next_dot + 1;
    }

}

int eval_get_precedence(BppTokenType type) {
    switch (type) {
        case TOK_IMP:
            return 1; // Implication
        case TOK_EQV:
            return 2; // Equivalence
        case TOK_OR:
        case TOK_XOR:
            return 3; // Logical/Bitwise OR, XOR
        case TOK_AND:
            return 4; // Logical/Bitwise AND
        case TOK_NOT:
            return 5; // Logical/Bitwise NOT
        case TOK_EQ:
        case TOK_NE:
        case TOK_LT:
        case TOK_GT:
        case TOK_LE:
        case TOK_GE:
            return 6; // Relational
        case TOK_SHL:
        case TOK_SHR:
        case TOK_READBIT:
        case TOK_SETBIT:
        case TOK_RESETBIT:
        case TOK_TOGGLEBIT:
            return 7; // Bitwise Shift & Manipulation
        case TOK_PLUS:
        case TOK_MINUS:
        case TOK_AMPERSAND:
        case TOK_MIN:
        case TOK_MAX:
        case TOK_HYPOT:
            return 8; // Additive, Extrema & String Concat
        case TOK_MUL:
        case TOK_DIV:
        case TOK_BACKSLASH:
        case TOK_MOD:
        case TOK_REMAINDER:
        case TOK_ATAN2:
            return 9; // Multiplicative, Integer Division & Modulo
        case TOK_UNARY_MINUS:
        case TOK_UNARY_PLUS:
            return 10; // Unary
        case TOK_POW:
            return 11; // Exponentiation (highest operator precedence)
        default:
            return 0; // Parentheses / Symbols
    }
}

bool eval_has_precedence(VMContext *vm, BppTokenType top, BppTokenType op) {
    if (top == TOK_POW && op == TOK_POW) {
        return false; // Right-associative: 2^3^2 => 2^(3^2)
    }
    return eval_get_precedence(top) >= eval_get_precedence(op);
}

// Check if token type is an operator
bool eval_is_operator(BppTokenType type) {
    return (type == TOK_PLUS || type == TOK_MINUS || type == TOK_MUL || type == TOK_DIV ||
            type == TOK_BACKSLASH ||
            type == TOK_AMPERSAND ||
            type == TOK_MOD || type == TOK_SHL || type == TOK_SHR ||
            type == TOK_READBIT || type == TOK_SETBIT || type == TOK_RESETBIT || type == TOK_TOGGLEBIT ||
            type == TOK_MIN || type == TOK_MAX || type == TOK_HYPOT || type == TOK_REMAINDER || type == TOK_ATAN2 ||
            type == TOK_POW ||
            type == TOK_EQ || type == TOK_NE || type == TOK_LT || type == TOK_GT ||
            type == TOK_LE || type == TOK_GE || type == TOK_UNARY_MINUS || type == TOK_UNARY_PLUS ||
            type == TOK_AND || type == TOK_OR || type == TOK_NOT || type == TOK_XOR ||
            type == TOK_IMP || type == TOK_EQV);
}

// Check if a token is a keyword representing a built-in function or operand
bool eval_is_builtin_function_tok(BppToken tok) {
    if (tok.type != TOK_KEYWORD) return false;
    if (tok.as.keyword == KW_NONE || tok.as.keyword == KW_TASK || tok.as.keyword == KW_PLAY ||
        tok.as.keyword == KW_HELP || tok.as.keyword == KW_CATEGORY || tok.as.keyword == KW_CATEGORIES ||
        tok.as.keyword == KW_SCREEN || tok.as.keyword == KW_SEEK ||

        tok.as.keyword == KW_TIMER || tok.as.keyword == KW_KEY || tok.as.keyword == KW_REMOVE ||
        tok.as.keyword == KW_REMOVE_STR || tok.as.keyword == KW_ALARM || tok.as.keyword == KW_ALARM_STR ||
        tok.as.keyword == KW_RANDOMIZE || tok.as.keyword == KW_DET || tok.as.keyword == KW_DOT ||
        tok.as.keyword == KW_CROSS || tok.as.keyword == KW_ZER || tok.as.keyword == KW_ONE ||
        tok.as.keyword == KW_IDN || tok.as.keyword == KW_TRN || tok.as.keyword == KW_INV ||
        tok.as.keyword == KW_VARPTR || tok.as.keyword == KW_VARSEG || tok.as.keyword == KW_SADD ||
        tok.as.keyword == KW_CINT || tok.as.keyword == KW_CSNG || tok.as.keyword == KW_CDBL ||
        tok.as.keyword == KW_DCOUNT || tok.as.keyword == KW_FREEFILE || tok.as.keyword == KW_USING || tok.as.keyword == KW_MAT) {
        return true;
    }
    if (tok.start && tok.length > 0) {
        char kbuf[64];
        size_t klen = (tok.length < sizeof(kbuf) - 1) ? tok.length : sizeof(kbuf) - 1;
        runtime_memcpy(kbuf, tok.start, klen);
        kbuf[klen] = '\0';
        if (eval_is_builtin_function(kbuf)) return true;
    }
    return false;
}

// Execute a single binary or unary operator
double eval_round_to_decimal(double val, int precision) {
    if (val == 0.0 || runtime_isnan(val) || runtime_isinf(val)) return val;
    double factor = runtime_pow(10.0, precision - (int)runtime_ceil(runtime_log10(runtime_fabs(val))));
    return runtime_round(val * factor) / factor;
}

void eval_format_double_clean(char *buf, size_t buf_size, double val, bool leading_space, bool trailing_space) {
    if (val == (double)(long long)val && runtime_fabs(val) <= 999999999.0) {
        runtime_snprintf(buf, buf_size, "%s%lld%s", leading_space ? (val >= 0.0 ? " " : "") : "", (long long)val, trailing_space ? " " : "");
        return;
    }

    if (runtime_fabs(val) <= 999999999.0 && runtime_fabs(val) >= 0.000001) {
        char temp[128];
        runtime_snprintf(temp, sizeof(temp), "%f", val);
        
        char *end = temp + runtime_strlen(temp) - 1;
        while (end > temp && *end == '0') {
            *end = '\0';
            end--;
        }
        if (end > temp && *end == '.') {
            *end = '\0';
        }
        
        runtime_snprintf(buf, buf_size, "%s%s%s", leading_space ? (val >= 0.0 ? " " : "") : "", temp, trailing_space ? " " : "");
        return;
    }
    
    num_format_display(buf, buf_size, val, leading_space, trailing_space);
}

