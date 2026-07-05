/**
 * GW-BASIC / BASIC++ Core Engine (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Logic inside statement handlers to optimize behaviors.
 *    - Diagnostic logs and specific error message phrasing.
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables and execution loop structure.
 *    - Memory pool layouts.
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Core interpreter execution honoring C17 standards.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Verify execution pointer updates correctly. Trace memory crashes back to pool margins.
 * -----------------------------------------------------------------------------
 */
#include "eval.h"
#include "../errors.h"
#include "tokenizer.h"
#include "variables.h"
#include "strings.h"
#include "memory.h"
#include "console.h"
#include "sdl2.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdio.h>

double gw_val_get_double(const GW_Value *v) {
    if (!v) return 0.0;
    switch (v->type) {
        case TYPE_INTEGER: return (double)v->i_val;
        case TYPE_SINGLE: return (double)v->s_val;
        case TYPE_DOUBLE: return v->d_val;
        case TYPE_COMPLEX: return v->c_val.real;
        case TYPE_STRING: return gw_str_val(&v->str);
        default: return 0.0;
    }
}

int16_t gw_val_get_int(const GW_Value *v) {
    if (!v) return 0;
    switch (v->type) {
        case TYPE_INTEGER: return v->i_val;
        case TYPE_SINGLE: return (int16_t)v->s_val;
        case TYPE_DOUBLE: return (int16_t)v->d_val;
        case TYPE_COMPLEX: return (int16_t)v->c_val.real;
        case TYPE_STRING: return (int16_t)gw_str_val(&v->str);
        default: return 0;
    }
}

static void eval_or(GW_State *state, GW_Value *result);

#include <stdio.h>

void gw_eval_expr(GW_State *state, GW_Value *result) {
    // printf("[DEBUG] gw_eval_expr: ip[0]=0x%02X\n", state->ip ? *state->ip : 0);
    // fflush(stdout);
    eval_or(state, result);
}

// Forward declarations for recursive descent
static void eval_and(GW_State *state, GW_Value *result);
static void eval_not(GW_State *state, GW_Value *result);
static void eval_relational(GW_State *state, GW_Value *result);
static void eval_add(GW_State *state, GW_Value *result);
static void eval_mod(GW_State *state, GW_Value *result);
static void eval_idiv(GW_State *state, GW_Value *result);
static void eval_mul(GW_State *state, GW_Value *result);
static void eval_unary(GW_State *state, GW_Value *result);
static void eval_pow(GW_State *state, GW_Value *result);
static void eval_primary(GW_State *state, GW_Value *result);

static void eval_or(GW_State *state, GW_Value *result) {
    eval_and(state, result);
    while (state->ip) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip != TOK_OR && *state->ip != TOK_XOR) break;
        uint8_t op = *state->ip;
        state->ip++;
        
        GW_Value rhs;
        eval_and(state, &rhs);
        int16_t left = gw_val_get_int(result);
        int16_t right = gw_val_get_int(&rhs);
        
        result->type = TYPE_INTEGER;
        if (op == TOK_OR) {
            result->i_val = left | right;
        } else {
            result->i_val = left ^ right; // XOR
        }
    }
}

static void eval_and(GW_State *state, GW_Value *result) {
    eval_not(state, result);
    while (state->ip) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip != TOK_AND) break;
        state->ip++;
        
        GW_Value rhs;
        eval_not(state, &rhs);
        int16_t left = gw_val_get_int(result);
        int16_t right = gw_val_get_int(&rhs);
        
        result->type = TYPE_INTEGER;
        result->i_val = left & right; // bitwise/logical AND
    }
}

static void eval_not(GW_State *state, GW_Value *result) {
    while (state->ip && (*state->ip == ' ' || *state->ip == '\t')) state->ip++;
    if (state->ip && *state->ip == TOK_NOT) {
        state->ip++;
        eval_not(state, result);
        int16_t val = gw_val_get_int(result);
        result->type = TYPE_INTEGER;
        result->i_val = ~val; // bitwise NOT
    } else {
        eval_relational(state, result);
    }
}

static void eval_relational(GW_State *state, GW_Value *result) {
    eval_add(state, result);
    if (state->ip) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    }
    
    if (state->ip && (*state->ip == '=' || *state->ip == '<' || *state->ip == '>')) {
        char op1 = *state->ip;
        state->ip++;
        char op2 = '\0';
        if (state->ip && (*state->ip == '=' || *state->ip == '>' || *state->ip == '<')) {
            if ((op1 == '<' && *state->ip == '=') ||
                (op1 == '>' && *state->ip == '=') ||
                (op1 == '<' && *state->ip == '>')) {
                op2 = *state->ip;
                state->ip++;
            } else if (op1 == '=' && *state->ip == '>') {
                op1 = '>';
                op2 = '=';
                state->ip++;
            } else if (op1 == '=' && *state->ip == '<') {
                op1 = '<';
                op2 = '=';
                state->ip++;
            }
        }
        
        while (state->ip && (*state->ip == ' ' || *state->ip == '\t')) state->ip++;
        
        GW_Value rhs;
        eval_add(state, &rhs);
        
        if (result->type == TYPE_STRING || rhs.type == TYPE_STRING) {
            double comp = 0.0;
            if (result->type == TYPE_STRING && rhs.type == TYPE_STRING) {
                size_t min_len = (result->str.len < rhs.str.len) ? result->str.len : rhs.str.len;
                int cmp = 0;
                if (min_len > 0 && result->str.ptr && rhs.str.ptr) {
                    cmp = memcmp(result->str.ptr, rhs.str.ptr, min_len);
                }
                if (cmp == 0) {
                    if (result->str.len < rhs.str.len) cmp = -1;
                    else if (result->str.len > rhs.str.len) cmp = 1;
                }
                comp = (double)cmp;
            } else {
                gw_error(13); // Type mismatch
            }
            
            gw_str_free(&result->str);
            gw_str_free(&rhs.str);
            
            result->type = TYPE_INTEGER;
            bool matched = false;
            if (op2 == '\0') {
                if (op1 == '=') matched = (comp == 0.0);
                else if (op1 == '<') matched = (comp < 0.0);
                else if (op1 == '>') matched = (comp > 0.0);
            } else {
                if (op1 == '<' && op2 == '=') matched = (comp <= 0.0);
                else if (op1 == '>' && op2 == '=') matched = (comp >= 0.0);
                else if (op1 == '<' && op2 == '>') matched = (comp != 0.0);
            }
            result->i_val = matched ? -1 : 0;
        } else {
            double left = gw_val_get_double(result);
            double right = gw_val_get_double(&rhs);
            
            result->type = TYPE_INTEGER;
            bool matched = false;
            if (op2 == '\0') {
                if (op1 == '=') matched = (left == right);
                else if (op1 == '<') matched = (left < right);
                else if (op1 == '>') matched = (left > right);
            } else {
                if (op1 == '<' && op2 == '=') matched = (left <= right);
                else if (op1 == '>' && op2 == '=') matched = (left >= right);
                else if (op1 == '<' && op2 == '>') matched = (left != right);
            }
            result->i_val = matched ? -1 : 0;
        }
    }
}

static void eval_add(GW_State *state, GW_Value *result) {
    eval_mod(state, result);
    while (state->ip) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip != '+' && *state->ip != '-') break;
        char op = *state->ip;
        state->ip++;
        
        GW_Value rhs;
        eval_mod(state, &rhs);
        
        if (result->type == TYPE_STRING || rhs.type == TYPE_STRING) {
            if (op == '+') {
                GW_String temp = gw_str_concat(&result->str, &rhs.str);
                gw_str_free(&result->str);
                gw_str_free(&rhs.str);
                result->type = TYPE_STRING;
                result->str = temp;
            } else {
                gw_error(13); // Type mismatch
            }
        } else {
            double left = gw_val_get_double(result);
            double right = gw_val_get_double(&rhs);
            
            result->type = TYPE_DOUBLE;
            if (op == '+') {
                result->d_val = left + right;
            } else {
                result->d_val = left - right;
            }
        }
    }
}

static void eval_mod(GW_State *state, GW_Value *result) {
    eval_idiv(state, result);
    while (state->ip) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip != TOK_MOD) break;
        state->ip++;
        
        GW_Value rhs;
        eval_idiv(state, &rhs);
        
        if (result->type == TYPE_STRING || rhs.type == TYPE_STRING) {
            gw_error(13); // Type mismatch
            if (result->type == TYPE_STRING) gw_str_free(&result->str);
            if (rhs.type == TYPE_STRING) gw_str_free(&rhs.str);
            result->type = TYPE_INTEGER;
            result->i_val = 0;
            break;
        }
        
        double left_d = gw_val_get_double(result);
        double right_d = gw_val_get_double(&rhs);
        
        int16_t left = (int16_t)round(left_d);
        int16_t right = (int16_t)round(right_d);
        
        result->type = TYPE_INTEGER;
        if (right == 0) {
            error_raise(ERR_DIV0, 0); // Division by zero
            result->i_val = 0;
        } else {
            result->i_val = left % right;
        }
    }
}

static void eval_idiv(GW_State *state, GW_Value *result) {
    eval_mul(state, result);
    while (state->ip) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip != '\\') break;
        state->ip++;
        
        GW_Value rhs;
        eval_mul(state, &rhs);
        
        if (result->type == TYPE_STRING || rhs.type == TYPE_STRING) {
            gw_error(13); // Type mismatch
            if (result->type == TYPE_STRING) gw_str_free(&result->str);
            if (rhs.type == TYPE_STRING) gw_str_free(&rhs.str);
            result->type = TYPE_INTEGER;
            result->i_val = 0;
            break;
        }
        
        double left_d = gw_val_get_double(result);
        double right_d = gw_val_get_double(&rhs);
        
        int16_t left = (int16_t)round(left_d);
        int16_t right = (int16_t)round(right_d);
        
        result->type = TYPE_INTEGER;
        if (right == 0) {
            error_raise(ERR_DIV0, 0); // Division by zero
            result->i_val = 0;
        } else {
            result->i_val = left / right;
        }
    }
}


static void eval_mul(GW_State *state, GW_Value *result) {
    eval_unary(state, result);
    while (state->ip) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip != '*' && *state->ip != '/') break;
        char op = *state->ip;
        state->ip++;
        
        GW_Value rhs;
        eval_unary(state, &rhs);
        
        double left = gw_val_get_double(result);
        double right = gw_val_get_double(&rhs);
        
        result->type = TYPE_DOUBLE;
        if (op == '*') {
            result->d_val = left * right;
        } else {
            if (right == 0.0) {
                error_raise(ERR_DIV0, 0); // Division by zero
                result->d_val = 0.0;
            } else {
                result->d_val = left / right;
            }
        }
    }
}

static void eval_unary(GW_State *state, GW_Value *result) {
    if (state->ip) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    }
    if (state->ip && *state->ip == '-') {
        state->ip++;
        eval_pow(state, result);
        double val = gw_val_get_double(result);
        result->type = TYPE_DOUBLE;
        result->d_val = -val;
    } else {
        eval_pow(state, result);
    }
}

static void eval_pow(GW_State *state, GW_Value *result) {
    eval_primary(state, result);
    if (state->ip) {
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    }
    if (state->ip && *state->ip == '^') {
        state->ip++;
        GW_Value rhs;
        eval_primary(state, &rhs);
        double left = gw_val_get_double(result);
        double right = gw_val_get_double(&rhs);
        result->type = TYPE_DOUBLE;
        result->d_val = pow(left, right);
    }
}

static void eval_primary(GW_State *state, GW_Value *result) {
    if (!state->ip || !*state->ip) {
        result->type = TYPE_NONE;
        return;
    }
    
    // Skip spaces
    while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
    
    // Hexadecimal or Octal literal
    if (*state->ip == '&') {
        state->ip++; // skip '&'
        uint32_t val = 0;
        if (*state->ip == 'h' || *state->ip == 'H') {
            state->ip++; // skip 'H'
            while (isxdigit((unsigned char)*state->ip)) {
                char c = *state->ip;
                int digit = 0;
                if (isdigit((unsigned char)c)) digit = c - '0';
                else digit = toupper((unsigned char)c) - 'A' + 10;
                val = val * 16 + digit;
                state->ip++;
            }
        } else {
            if (*state->ip == 'o' || *state->ip == 'O') {
                state->ip++; // skip 'O'
            }
            while (*state->ip >= '0' && *state->ip <= '7') {
                val = val * 8 + (*state->ip - '0');
                state->ip++;
            }
        }
        result->type = TYPE_INTEGER;
        result->i_val = (int16_t)(uint16_t)val;
        return;
    }
    
    // Numeric literal
    if (isdigit(*state->ip) || *state->ip == '.') {
        char *endptr;
        double val = strtod((char *)state->ip, &endptr);
        GW_Type type = TYPE_SINGLE;
        if (*endptr == '#') {
            type = TYPE_DOUBLE;
            endptr++;
        } else if (*endptr == '!') {
            type = TYPE_SINGLE;
            endptr++;
        } else if (*endptr == '%') {
            type = TYPE_INTEGER;
            endptr++;
        } else {
            const char *p = (const char *)state->ip;
            while (p < endptr) {
                if (*p == 'd' || *p == 'D') {
                    type = TYPE_DOUBLE;
                    break;
                }
                p++;
            }
        }
        state->ip = (uint8_t *)endptr;
        result->type = type;
        if (type == TYPE_INTEGER) {
            result->i_val = (int16_t)val;
        } else if (type == TYPE_SINGLE) {
            result->s_val = (float)val;
        } else {
            result->d_val = val;
        }
        return;
    }
    
    // String literal
    if (*state->ip == '"') {
        state->ip++; // skip '"'
        const char *start = (const char *)state->ip;
        while (*state->ip && *state->ip != '"') state->ip++;
        size_t len = (const char *)state->ip - start;
        if (*state->ip == '"') state->ip++; // skip '"'
        
        result->type = TYPE_STRING;
        result->str = gw_str_create(start, len);
        return;
    }
    
    uint16_t full_tok = *state->ip;
    if (full_tok == 0xFE || full_tok == 0xFD || full_tok == 0xFF) {
        if (state->ip[1]) {
            full_tok = (full_tok << 8) | state->ip[1];
        }
    }

    if (full_tok == TOK_ONKEY) {
        state->ip += 2; // skip extended token
        extern char gw_console_read_char(void);
        char ch = gw_console_read_char();
        result->type = TYPE_STRING;
        if (ch > 0) {
            char tmp[2] = {ch, '\0'};
            result->str = gw_str_create(tmp, 1);
        } else {
            result->str = gw_str_create("", 0);
        }
        return;
    }

    if (full_tok == TOK_ASIN || full_tok == TOK_ACOS || full_tok == TOK_SINH ||
        full_tok == TOK_COSH || full_tok == TOK_TANH || full_tok == TOK_LOG10 ||
        full_tok == TOK_LOG2) {
        state->ip += 2;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        result->type = TYPE_DOUBLE;
        if (val.type == TYPE_STRING) {
            gw_error(13); gw_str_free(&val.str); result->d_val = 0.0;
        } else {
            double v = gw_val_get_double(&val);
            if (full_tok == TOK_ASIN) result->d_val = asin(v);
            else if (full_tok == TOK_ACOS) result->d_val = acos(v);
            else if (full_tok == TOK_SINH) result->d_val = sinh(v);
            else if (full_tok == TOK_COSH) result->d_val = cosh(v);
            else if (full_tok == TOK_TANH) result->d_val = tanh(v);
            else if (full_tok == TOK_LOG10) result->d_val = log10(v);
            else if (full_tok == TOK_LOG2) result->d_val = log2(v);
        }
        return;
    }

    if (full_tok == TOK_PI) {
        state->ip += 2;
        result->type = TYPE_DOUBLE;
        result->d_val = 3.14159265358979323846;
        return;
    }

    if (full_tok == TOK_COMP || full_tok == TOK_PDIF) {
        state->ip += 2;
        if (*state->ip == '(') state->ip++;
        GW_Value val1;
        gw_eval_expr(state, &val1);
        if (*state->ip == ',') state->ip++;
        GW_Value val2;
        gw_eval_expr(state, &val2);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_DOUBLE;
        if (val1.type == TYPE_STRING || val2.type == TYPE_STRING) {
            gw_error(13); 
            if (val1.type == TYPE_STRING) gw_str_free(&val1.str);
            if (val2.type == TYPE_STRING) gw_str_free(&val2.str);
            result->d_val = 0.0;
        } else {
            double v1 = gw_val_get_double(&val1);
            double v2 = gw_val_get_double(&val2);
            if (full_tok == TOK_COMP) {
                if (v1 < v2) result->d_val = -1.0;
                else if (v1 > v2) result->d_val = 1.0;
                else result->d_val = 0.0;
            } else {
                result->d_val = (v1 > v2) ? (v1 - v2) : 0.0;
            }
        }
        return;
    }

    if (full_tok == TOK_COMPLEX) {
        state->ip += 2;
        if (*state->ip == '(') state->ip++;
        GW_Value val1;
        gw_eval_expr(state, &val1);
        if (*state->ip == ',') state->ip++;
        GW_Value val2;
        gw_eval_expr(state, &val2);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_COMPLEX;
        if (val1.type == TYPE_STRING || val2.type == TYPE_STRING) {
            gw_error(13); 
            if (val1.type == TYPE_STRING) gw_str_free(&val1.str);
            if (val2.type == TYPE_STRING) gw_str_free(&val2.str);
            result->c_val.real = 0.0; result->c_val.imag = 0.0;
        } else {
            result->c_val.real = gw_val_get_double(&val1);
            result->c_val.imag = gw_val_get_double(&val2);
        }
        return;
    }

    if (full_tok == TOK_REAL || full_tok == TOK_IMAG || full_tok == TOK_CABS ||
        full_tok == TOK_CARG) {
        state->ip += 2;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        result->type = TYPE_DOUBLE;
        if (val.type == TYPE_STRING) {
            gw_error(13); gw_str_free(&val.str); result->d_val = 0.0;
        } else {
            double r = 0.0, i = 0.0;
            if (val.type == TYPE_COMPLEX) {
                r = val.c_val.real; i = val.c_val.imag;
            } else {
                r = gw_val_get_double(&val);
            }
            
            if (full_tok == TOK_REAL) result->d_val = r;
            else if (full_tok == TOK_IMAG) result->d_val = i;
            else if (full_tok == TOK_CABS) result->d_val = sqrt(r*r + i*i);
            else if (full_tok == TOK_CARG) result->d_val = atan2(i, r);
        }
        return;
    }

    if (full_tok == TOK_CONJ || full_tok == TOK_CSQR || full_tok == TOK_CEXP ||
        full_tok == TOK_CLOG) {
        state->ip += 2;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        result->type = TYPE_COMPLEX;
        if (val.type == TYPE_STRING) {
            gw_error(13); gw_str_free(&val.str); result->c_val.real = 0.0; result->c_val.imag = 0.0;
        } else {
            double r = 0.0, i = 0.0;
            if (val.type == TYPE_COMPLEX) {
                r = val.c_val.real; i = val.c_val.imag;
            } else {
                r = gw_val_get_double(&val);
            }
            
            if (full_tok == TOK_CONJ) {
                result->c_val.real = r; result->c_val.imag = -i;
            } else if (full_tok == TOK_CSQR) {
                double mag = sqrt(r*r + i*i);
                result->c_val.real = sqrt((mag + r)/2.0);
                result->c_val.imag = (i < 0 ? -1.0 : 1.0) * sqrt((mag - r)/2.0);
            } else if (full_tok == TOK_CEXP) {
                double er = exp(r);
                result->c_val.real = er * cos(i);
                result->c_val.imag = er * sin(i);
            } else if (full_tok == TOK_CLOG) {
                result->c_val.real = log(sqrt(r*r + i*i));
                result->c_val.imag = atan2(i, r);
            }
        }
        return;
    }

    if (full_tok == TOK_CPOW) {
        state->ip += 2;
        if (*state->ip == '(') state->ip++;
        GW_Value val1;
        gw_eval_expr(state, &val1);
        if (*state->ip == ',') state->ip++;
        GW_Value val2;
        gw_eval_expr(state, &val2);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_COMPLEX;
        if (val1.type == TYPE_STRING || val2.type == TYPE_STRING) {
            gw_error(13); 
            if (val1.type == TYPE_STRING) gw_str_free(&val1.str);
            if (val2.type == TYPE_STRING) gw_str_free(&val2.str);
            result->c_val.real = 0.0; result->c_val.imag = 0.0;
        } else {
            double r1 = (val1.type == TYPE_COMPLEX) ? val1.c_val.real : gw_val_get_double(&val1);
            double i1 = (val1.type == TYPE_COMPLEX) ? val1.c_val.imag : 0.0;
            double r2 = (val2.type == TYPE_COMPLEX) ? val2.c_val.real : gw_val_get_double(&val2);
            double i2 = (val2.type == TYPE_COMPLEX) ? val2.c_val.imag : 0.0;
            
            double m = r1*r1 + i1*i1;
            double a = atan2(i1, r1);
            double log_r = 0.5 * log(m);
            double log_i = a;
            
            double p_r = r2 * log_r - i2 * log_i;
            double p_i = r2 * log_i + i2 * log_r;
            
            double e = exp(p_r);
            result->c_val.real = e * cos(p_i);
            result->c_val.imag = e * sin(p_i);
        }
        return;
    }

    if (full_tok == TOK_LCASE || full_tok == TOK_UCASE || full_tok == TOK_TCASE ||
        full_tok == TOK_MCASE || full_tok == TOK_ICASE || full_tok == TOK_REVERSE ||
        full_tok == TOK_TRIM || full_tok == TOK_LTRIM || full_tok == TOK_RTRIM ||
        full_tok == TOK_HASH_FN) {
        state->ip += 2;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        if (val.type != TYPE_STRING) {
            gw_error(13); // Type mismatch
            if (full_tok == TOK_HASH_FN) {
                result->type = TYPE_INTEGER; result->i_val = 0;
            } else {
                result->type = TYPE_STRING; result->str = gw_str_create("", 0);
            }
            return;
        }

        if (full_tok == TOK_HASH_FN) {
            int hash = 5381;
            for (int i = 0; i < val.str.len; i++) hash = ((hash << 5) + hash) + val.str.ptr[i];
            result->type = TYPE_INTEGER;
            result->i_val = hash;
            gw_str_free(&val.str);
            return;
        }

        char buf[256];
        int slen = val.str.len;
        if (slen > 255) slen = 255;
        const char *s = val.str.ptr;
        
        result->type = TYPE_STRING;
        
        if (full_tok == TOK_LCASE) {
            for (int i=0; i<slen; i++) buf[i] = tolower(s[i]);
            result->str = gw_str_create(buf, slen);
        } else if (full_tok == TOK_UCASE) {
            for (int i=0; i<slen; i++) buf[i] = toupper(s[i]);
            result->str = gw_str_create(buf, slen);
        } else if (full_tok == TOK_TCASE) {
            int after_space = 1;
            for (int i=0; i<slen; i++) {
                char c = s[i];
                if (c == ' ' || c == '\t') { buf[i] = c; after_space = 1; }
                else if (after_space) { buf[i] = toupper(c); after_space = 0; }
                else { buf[i] = tolower(c); }
            }
            result->str = gw_str_create(buf, slen);
        } else if (full_tok == TOK_MCASE) {
            for (int i=0; i<slen; i++) buf[i] = (rand() % 2) ? toupper(s[i]) : tolower(s[i]);
            result->str = gw_str_create(buf, slen);
        } else if (full_tok == TOK_ICASE) {
            for (int i=0; i<slen; i++) buf[i] = islower(s[i]) ? toupper(s[i]) : tolower(s[i]);
            result->str = gw_str_create(buf, slen);
        } else if (full_tok == TOK_REVERSE) {
            for (int i=0; i<slen; i++) buf[i] = s[slen - 1 - i];
            result->str = gw_str_create(buf, slen);
        } else if (full_tok == TOK_LTRIM) {
            int i=0; while (i<slen && s[i]==' ') i++;
            result->str = gw_str_create(s+i, slen-i);
        } else if (full_tok == TOK_RTRIM) {
            int i=slen; while (i>0 && s[i-1]==' ') i--;
            result->str = gw_str_create(s, i);
        } else if (full_tok == TOK_TRIM) {
            int left=0, right=slen;
            while (left<right && s[left]==' ') left++;
            while (right>left && s[right-1]==' ') right--;
            result->str = gw_str_create(s+left, right-left);
        }
        
        gw_str_free(&val.str);
        return;
    }
    
    if (full_tok == TOK_REPLACE) {
        state->ip += 2;
        if (*state->ip == '(') state->ip++;
        GW_Value s1, s2, s3;
        gw_eval_expr(state, &s1);
        if (*state->ip == ',') state->ip++;
        gw_eval_expr(state, &s2);
        if (*state->ip == ',') state->ip++;
        gw_eval_expr(state, &s3);
        if (*state->ip == ')') state->ip++;
        
        if (s1.type != TYPE_STRING || s2.type != TYPE_STRING || s3.type != TYPE_STRING) {
            gw_error(13); // Type mismatch
            if (s1.type == TYPE_STRING) gw_str_free(&s1.str);
            if (s2.type == TYPE_STRING) gw_str_free(&s2.str);
            if (s3.type == TYPE_STRING) gw_str_free(&s3.str);
            result->type = TYPE_STRING; result->str = gw_str_create("", 0);
            return;
        }
        
        // Simple string replacement: replace first occurrence for basic compatibility
        // (Full all-occurrences replace takes more logic, this suffices for core stub)
        char buf[256];
        int len = 0;
        int found = -1;
        
        if (s2.str.len > 0 && s1.str.len >= s2.str.len) {
            for (int i=0; i <= s1.str.len - s2.str.len; i++) {
                if (memcmp(s1.str.ptr + i, s2.str.ptr, s2.str.len) == 0) {
                    found = i; break;
                }
            }
        }
        
        if (found >= 0) {
            memcpy(buf, s1.str.ptr, found);
            len += found;
            memcpy(buf + len, s3.str.ptr, s3.str.len);
            len += s3.str.len;
            int rem = s1.str.len - found - s2.str.len;
            memcpy(buf + len, s1.str.ptr + found + s2.str.len, rem);
            len += rem;
        } else {
            memcpy(buf, s1.str.ptr, s1.str.len);
            len = s1.str.len;
        }
        
        result->type = TYPE_STRING;
        result->str = gw_str_create(buf, len);
        gw_str_free(&s1.str);
        gw_str_free(&s2.str);
        gw_str_free(&s3.str);
        return;
    }

    if (full_tok == TOK_LIKE) {
        state->ip += 2;
        if (*state->ip == '(') state->ip++;
        GW_Value s1, s2;
        gw_eval_expr(state, &s1);
        if (*state->ip == ',') state->ip++;
        gw_eval_expr(state, &s2);
        if (*state->ip == ')') state->ip++;
        
        if (s1.type != TYPE_STRING || s2.type != TYPE_STRING) {
            gw_error(13); // Type mismatch
            if (s1.type == TYPE_STRING) gw_str_free(&s1.str);
            if (s2.type == TYPE_STRING) gw_str_free(&s2.str);
            result->type = TYPE_INTEGER; result->i_val = 0;
            return;
        }
        
        // Very rudimentary LIKE implementation (exact match check for now, can extend to wildcards later)
        int match = (s1.str.len == s2.str.len && memcmp(s1.str.ptr, s2.str.ptr, s1.str.len) == 0);
        result->type = TYPE_INTEGER;
        result->i_val = match ? -1 : 0;
        
        gw_str_free(&s1.str);
        gw_str_free(&s2.str);
        return;
    }

    
    // Check ERR variable
    if (*state->ip == TOK_ERR) {
        state->ip++;
        result->type = TYPE_INTEGER;
        result->i_val = (int16_t)state->last_error_code;
        return;
    }
    
    // Check ERL variable
    if (*state->ip == TOK_ERL) {
        state->ip++;
        result->type = TYPE_INTEGER;
        result->i_val = (int16_t)state->last_error_line;
        return;
    }
    
    // Check PEEK function
    if (*state->ip == TOK_PEEK) {
        state->ip++; // skip TOK_PEEK
        if (*state->ip == '(') {
            state->ip++;
            GW_Value offset_val;
            gw_eval_expr(state, &offset_val);
            if (*state->ip == ')') state->ip++;
            
            result->type = TYPE_INTEGER;
            result->i_val = gw_mem_peek(state->mem_sys, gw_val_get_int(&offset_val));
        } else {
            gw_error(2); // Syntax error
        }
        return;
    }
    
    // Check ABS function
    if (*state->ip == TOK_ABS) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_DOUBLE;
        result->d_val = fabs(gw_val_get_double(&val));
        if (val.type == TYPE_STRING) gw_str_free(&val.str);
        return;
    }
    
    // Check INT function
    if (*state->ip == TOK_INT) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_DOUBLE;
        result->d_val = floor(gw_val_get_double(&val));
        if (val.type == TYPE_STRING) gw_str_free(&val.str);
        return;
    }
    
    // Check RND function
    if (*state->ip == TOK_RND) {
        state->ip++;
        if (*state->ip == '(') {
            state->ip++;
            GW_Value val;
            gw_eval_expr(state, &val);
            if (*state->ip == ')') state->ip++;
            if (val.type == TYPE_STRING) gw_str_free(&val.str);
        }
        
        result->type = TYPE_DOUBLE;
        result->d_val = (double)rand() / (double)RAND_MAX;
        return;
    }
    
    // Check SQR function
    if (*state->ip == TOK_SQR) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_DOUBLE;
        double arg = gw_val_get_double(&val);
        result->d_val = (arg < 0.0) ? 0.0 : sqrt(arg);
        if (val.type == TYPE_STRING) gw_str_free(&val.str);
        return;
    }
    
    // Check CINT function
    if (*state->ip == TOK_CINT) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        if (val.type == TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->type = TYPE_INTEGER;
            result->i_val = 0;
            gw_str_free(&val.str);
        } else {
            double v = gw_val_get_double(&val);
            double rounded = round(v);
            if (fabs(v - floor(v)) == 0.5) {
                if ((int)floor(v) % 2 == 0) {
                    rounded = floor(v);
                } else {
                    rounded = ceil(v);
                }
            }
            if (rounded < -32768.0 || rounded > 32767.0) {
                gw_error(6); // Overflow
            }
            result->type = TYPE_INTEGER;
            result->i_val = (int16_t)rounded;
        }
        return;
    }
    
    // Check CSNG function
    if (*state->ip == TOK_CSNG) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        if (val.type == TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->type = TYPE_SINGLE;
            result->s_val = 0.0f;
            gw_str_free(&val.str);
        } else {
            result->type = TYPE_SINGLE;
            result->s_val = (float)gw_val_get_double(&val);
        }
        return;
    }
    
    // Check CDBL function
    if (*state->ip == TOK_CDBL) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        if (val.type == TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->type = TYPE_DOUBLE;
            result->d_val = 0.0;
            gw_str_free(&val.str);
        } else {
            result->type = TYPE_DOUBLE;
            result->d_val = gw_val_get_double(&val);
        }
        return;
    }
    
    // Check FIX function
    if (*state->ip == TOK_FIX) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        if (val.type == TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->type = TYPE_DOUBLE;
            result->d_val = 0.0;
            gw_str_free(&val.str);
        } else {
            double v = gw_val_get_double(&val);
            result->type = TYPE_DOUBLE;
            result->d_val = (v >= 0.0) ? floor(v) : ceil(v);
        }
        return;
    }
    
    // Check CHR$ function
    if (*state->ip == TOK_CHR) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        int code = gw_val_get_int(&val);
        char tmp[2] = {(char)code, '\0'};
        result->type = TYPE_STRING;
        result->str = gw_str_create(tmp, 1);
        if (val.type == TYPE_STRING) gw_str_free(&val.str);
        return;
    }
    
    // Check STRING$ function
    if (*state->ip == TOK_STRING_FN) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value count_val;
        gw_eval_expr(state, &count_val);
        
        int count = gw_val_get_int(&count_val);
        if (count < 0) count = 0;
        if (count > 255) count = 255;
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') state->ip++;
        
        GW_Value char_val;
        gw_eval_expr(state, &char_val);
        if (*state->ip == ')') state->ip++;
        
        char fill_char = ' ';
        if (char_val.type == TYPE_STRING) {
            if (char_val.str.len > 0) {
                fill_char = char_val.str.ptr[0];
            }
            gw_str_free(&char_val.str);
        } else {
            fill_char = (char)gw_val_get_int(&char_val);
        }
        
        char tmp[256];
        memset(tmp, fill_char, count);
        tmp[count] = '\0';
        
        result->type = TYPE_STRING;
        result->str = gw_str_create(tmp, count);
        
        if (count_val.type == TYPE_STRING) gw_str_free(&count_val.str);
        return;
    }
    
    // Check TAB function
    if (*state->ip == TOK_TAB) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value col_val;
        gw_eval_expr(state, &col_val);
        if (*state->ip == ')') state->ip++;
        
        int col = gw_val_get_int(&col_val);
        int current_x = state->cursor_x;
        int spaces_needed = col - 1 - current_x; // col is 1-indexed in BASIC
        if (spaces_needed < 1) spaces_needed = 1;
        if (spaces_needed > 255) spaces_needed = 255;
        
        char tmp[256];
        memset(tmp, ' ', spaces_needed);
        tmp[spaces_needed] = '\0';
        
        result->type = TYPE_STRING;
        result->str = gw_str_create(tmp, spaces_needed);
        
        if (col_val.type == TYPE_STRING) gw_str_free(&col_val.str);
        return;
    }
    
    // Check INKEY$ function/variable
    if (*state->ip == TOK_INKEY) {
        state->ip++;
        char ch = gw_console_read_char();
        result->type = TYPE_STRING;
        if (ch > 0) {
            char tmp[2] = {ch, '\0'};
            result->str = gw_str_create(tmp, 1);
        } else {
            result->str = gw_str_create("", 0);
        }
        return;
    }
    
    // Check SCREEN function
    if (*state->ip == TOK_SCREEN) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value row_val;
        gw_eval_expr(state, &row_val);
        int row = gw_val_get_int(&row_val);
        if (row_val.type == TYPE_STRING) gw_str_free(&row_val.str);
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') state->ip++;
        
        GW_Value col_val;
        gw_eval_expr(state, &col_val);
        int col = gw_val_get_int(&col_val);
        if (col_val.type == TYPE_STRING) gw_str_free(&col_val.str);
        
        int flag = 0;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') {
            state->ip++;
            GW_Value flag_val;
            gw_eval_expr(state, &flag_val);
            flag = gw_val_get_int(&flag_val);
            if (flag_val.type == TYPE_STRING) gw_str_free(&flag_val.str);
        }
        
        if (*state->ip == ')') state->ip++;
        
        char ch = gw_sdl2_get_char(col - 1, row - 1);
        result->type = TYPE_DOUBLE;
        result->d_val = (double)(uint8_t)ch;
        return;
    }
    
    // Check MID$ function
    if (*state->ip == TOK_MID) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        
        GW_Value str_val;
        gw_eval_expr(state, &str_val);
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') state->ip++;
        
        GW_Value start_val;
        gw_eval_expr(state, &start_val);
        int start = gw_val_get_int(&start_val);
        
        int len = -1;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') {
            state->ip++;
            GW_Value len_val;
            gw_eval_expr(state, &len_val);
            len = gw_val_get_int(&len_val);
            if (len_val.type == TYPE_STRING) gw_str_free(&len_val.str);
        }
        
        if (*state->ip == ')') state->ip++;
        
        if (str_val.type != TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->type = TYPE_STRING;
            result->str = gw_str_create(NULL, 0);
        } else {
            result->type = TYPE_STRING;
            if (len < 0) {
                len = str_val.str.len - (start - 1);
            }
            if (len < 0) len = 0;
            result->str = gw_str_mid(&str_val.str, start, len);
        }
        
        if (str_val.type == TYPE_STRING) gw_str_free(&str_val.str);
        if (start_val.type == TYPE_STRING) gw_str_free(&start_val.str);
        return;
    }
    
    // Check LEFT$ function
    if (*state->ip == TOK_LEFT) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        
        GW_Value str_val;
        gw_eval_expr(state, &str_val);
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') state->ip++;
        
        GW_Value len_val;
        gw_eval_expr(state, &len_val);
        int len = gw_val_get_int(&len_val);
        
        if (*state->ip == ')') state->ip++;
        
        if (str_val.type != TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->type = TYPE_STRING;
            result->str = gw_str_create(NULL, 0);
        } else {
            result->type = TYPE_STRING;
            result->str = gw_str_left(&str_val.str, len);
        }
        
        if (str_val.type == TYPE_STRING) gw_str_free(&str_val.str);
        if (len_val.type == TYPE_STRING) gw_str_free(&len_val.str);
        return;
    }
    
    // Check RIGHT$ function
    if (*state->ip == TOK_RIGHT) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        
        GW_Value str_val;
        gw_eval_expr(state, &str_val);
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') state->ip++;
        
        GW_Value len_val;
        gw_eval_expr(state, &len_val);
        int len = gw_val_get_int(&len_val);
        
        if (*state->ip == ')') state->ip++;
        
        if (str_val.type != TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->type = TYPE_STRING;
            result->str = gw_str_create(NULL, 0);
        } else {
            result->type = TYPE_STRING;
            result->str = gw_str_right(&str_val.str, len);
        }
        
        if (str_val.type == TYPE_STRING) gw_str_free(&str_val.str);
        if (len_val.type == TYPE_STRING) gw_str_free(&len_val.str);
        return;
    }
    
    // Check LEN function
    if (*state->ip == TOK_LEN) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        
        GW_Value str_val;
        gw_eval_expr(state, &str_val);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_DOUBLE;
        if (str_val.type != TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->d_val = 0.0;
        } else {
            result->d_val = (double)str_val.str.len;
        }
        
        if (str_val.type == TYPE_STRING) gw_str_free(&str_val.str);
        return;
    }
    
    // Check VAL function
    if (*state->ip == TOK_VAL) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        
        GW_Value str_val;
        gw_eval_expr(state, &str_val);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_DOUBLE;
        if (str_val.type != TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->d_val = 0.0;
        } else {
            result->d_val = gw_str_val(&str_val.str);
        }
        
        if (str_val.type == TYPE_STRING) gw_str_free(&str_val.str);
        return;
    }
    
    // Check STR$ function
    if (*state->ip == TOK_STR) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        
        GW_Value num_val;
        gw_eval_expr(state, &num_val);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_STRING;
        if (num_val.type == TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->str = gw_str_create(NULL, 0);
            gw_str_free(&num_val.str);
        } else {
            double val = gw_val_get_double(&num_val);
            char tmp[64];
            if (val >= 0.0) {
                sprintf(tmp, " %g", val);
            } else {
                sprintf(tmp, "%g", val);
            }
            result->str = gw_str_create(tmp, strlen(tmp));
        }
        return;
    }
    
    // Check ASC function
    if (*state->ip == TOK_ASC) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        
        GW_Value str_val;
        gw_eval_expr(state, &str_val);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_DOUBLE;
        if (str_val.type != TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->d_val = 0.0;
        } else if (str_val.str.len == 0) {
            gw_error(5); // Illegal function call (ASC on empty string)
            result->d_val = 0.0;
        } else {
            result->d_val = (double)(uint8_t)str_val.str.ptr[0];
        }
        
        if (str_val.type == TYPE_STRING) gw_str_free(&str_val.str);
        return;
    }
    
    // Check INSTR function
    if (*state->ip == TOK_INSTR) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        
        GW_Value arg1;
        gw_eval_expr(state, &arg1);
        
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') state->ip++;
        
        GW_Value arg2;
        gw_eval_expr(state, &arg2);
        
        GW_Value arg3;
        arg3.type = TYPE_INTEGER;
        arg3.i_val = 0;
        
        bool three_args = false;
        while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
        if (*state->ip == ',') {
            state->ip++;
            gw_eval_expr(state, &arg3);
            three_args = true;
        }
        if (*state->ip == ')') state->ip++;
        
        int start = 1;
        GW_Value *s1_val = &arg1;
        GW_Value *s2_val = &arg2;
        
        if (three_args) {
            start = gw_val_get_int(&arg1);
            s1_val = &arg2;
            s2_val = &arg3;
        }
        
        result->type = TYPE_DOUBLE;
        result->d_val = 0.0;
        
        if (s1_val->type != TYPE_STRING || s2_val->type != TYPE_STRING) {
            gw_error(13); // Type mismatch
        } else if (start <= 0) {
            gw_error(5); // Illegal function call
        } else {
            int len1 = s1_val->str.len;
            int len2 = s2_val->str.len;
            if (len2 == 0) {
                result->d_val = (double)start;
            } else if (start <= len1) {
                const char *p1 = s1_val->str.ptr;
                const char *p2 = s2_val->str.ptr;
                for (int i = start - 1; i <= len1 - len2; i++) {
                    if (memcmp(p1 + i, p2, len2) == 0) {
                        result->d_val = (double)(i + 1);
                        break;
                    }
                }
            }
        }
        
        if (arg1.type == TYPE_STRING) gw_str_free(&arg1.str);
        if (arg2.type == TYPE_STRING) gw_str_free(&arg2.str);
        if (three_args && arg3.type == TYPE_STRING) gw_str_free(&arg3.str);
        return;
    }
    
    // Check SGN function
    if (*state->ip == TOK_SGN) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        
        result->type = TYPE_DOUBLE;
        if (val.type == TYPE_STRING) {
            gw_error(13); // Type mismatch
            result->d_val = 0.0;
            gw_str_free(&val.str);
        } else {
            double v = gw_val_get_double(&val);
            if (v > 0.0) result->d_val = 1.0;
            else if (v < 0.0) result->d_val = -1.0;
            else result->d_val = 0.0;
        }
        return;
    }
    
    // Check SIN function
    if (*state->ip == TOK_SIN) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        result->type = TYPE_DOUBLE;
        if (val.type == TYPE_STRING) {
            gw_error(13); gw_str_free(&val.str); result->d_val = 0.0;
        } else {
            result->d_val = sin(gw_val_get_double(&val));
        }
        return;
    }
    
    // Check COS function
    if (*state->ip == TOK_COS) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        result->type = TYPE_DOUBLE;
        if (val.type == TYPE_STRING) {
            gw_error(13); gw_str_free(&val.str); result->d_val = 0.0;
        } else {
            result->d_val = cos(gw_val_get_double(&val));
        }
        return;
    }
    
    // Check TAN function
    if (*state->ip == TOK_TAN) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        result->type = TYPE_DOUBLE;
        if (val.type == TYPE_STRING) {
            gw_error(13); gw_str_free(&val.str); result->d_val = 0.0;
        } else {
            result->d_val = tan(gw_val_get_double(&val));
        }
        return;
    }
    
    // Check ATN function
    if (*state->ip == TOK_ATN) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        result->type = TYPE_DOUBLE;
        if (val.type == TYPE_STRING) {
            gw_error(13); gw_str_free(&val.str); result->d_val = 0.0;
        } else {
            result->d_val = atan(gw_val_get_double(&val));
        }
        return;
    }
    
    // Check EXP function
    if (*state->ip == TOK_EXP) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        result->type = TYPE_DOUBLE;
        if (val.type == TYPE_STRING) {
            gw_error(13); gw_str_free(&val.str); result->d_val = 0.0;
        } else {
            result->d_val = exp(gw_val_get_double(&val));
        }
        return;
    }
    
    // Check LOG function
    if (*state->ip == TOK_LOG) {
        state->ip++;
        if (*state->ip == '(') state->ip++;
        GW_Value val;
        gw_eval_expr(state, &val);
        if (*state->ip == ')') state->ip++;
        result->type = TYPE_DOUBLE;
        if (val.type == TYPE_STRING) {
            gw_error(13); gw_str_free(&val.str); result->d_val = 0.0;
        } else {
            double arg = gw_val_get_double(&val);
            if (arg <= 0.0) {
                gw_error(5); // Illegal function call
                result->d_val = 0.0;
            } else {
                result->d_val = log(arg);
            }
        }
        return;
    }
    
    // Check POS function
    if (*state->ip == TOK_POS) {
        state->ip++;
        if (*state->ip == '(') {
            state->ip++;
            GW_Value val;
            gw_eval_expr(state, &val);
            if (*state->ip == ')') state->ip++;
            if (val.type == TYPE_STRING) gw_str_free(&val.str);
        }
        result->type = TYPE_DOUBLE;
        result->d_val = (double)(state->cursor_x + 1);
        return;
    }
    
    // Parenthesized expression
    if (*state->ip == '(') {
        state->ip++;
        gw_eval_expr(state, result);
        if (*state->ip == ')') state->ip++;
        return;
    }
    
    // Variable lookup / Array lookup
    if (isalpha(*state->ip)) {
        char var_name[40];
        int idx = 0;
        while (isalnum(*state->ip) || *state->ip == '%' || *state->ip == '!' || *state->ip == '#' || *state->ip == '$') {
            if (idx < 39) var_name[idx++] = *state->ip;
            state->ip++;
        }
        var_name[idx] = '\0';
        
        // Skip spaces to check for '('
        uint8_t *check_ptr = state->ip;
        while (*check_ptr == ' ' || *check_ptr == '\t') check_ptr++;
        
        if (*check_ptr == '(') {
            state->ip = check_ptr + 1; // skip past '('
            
            int indices[10];
            int num_dims = 0;
            while (num_dims < 10) {
                GW_Value idx_val;
                gw_eval_expr(state, &idx_val);
                indices[num_dims++] = gw_val_get_int(&idx_val);
                if (idx_val.type == TYPE_STRING) gw_str_free(&idx_val.str);
                
                while (*state->ip == ' ' || *state->ip == '\t') state->ip++;
                if (*state->ip == ',') {
                    state->ip++;
                } else if (*state->ip == ')') {
                    state->ip++;
                    break;
                } else {
                    gw_error(2); // Syntax error
                    result->type = TYPE_INTEGER;
                    result->i_val = 0;
                    return;
                }
            }
            
            GW_Array *arr = gw_arr_get(state, var_name, true);
            if (arr) {
                void *ptr = gw_arr_index_ptr(arr, num_dims, indices);
                if (ptr) {
                    result->type = arr->type;
                    if (arr->type == TYPE_INTEGER) {
                        result->i_val = *(int16_t *)ptr;
                    } else if (arr->type == TYPE_SINGLE) {
                        result->s_val = *(float *)ptr;
                    } else if (arr->type == TYPE_DOUBLE) {
                        result->d_val = *(double *)ptr;
                    } else if (arr->type == TYPE_STRING) {
                        GW_String *s_ptr = (GW_String *)ptr;
                        result->str = gw_str_create(s_ptr->ptr, s_ptr->len);
                    }
                } else {
                    gw_error(9); // Subscript out of range
                    result->type = TYPE_INTEGER;
                    result->i_val = 0;
                }
            } else {
                gw_error(8); // Undefined array
                result->type = TYPE_INTEGER;
                result->i_val = 0;
            }
            return;
        }
        
        GW_Var *var = gw_var_get(state, var_name, true);
        if (var) {
            result->type = var->type;
            if (var->type == TYPE_STRING) {
                result->str = gw_str_create(var->str.ptr, var->str.len);
            } else if (var->type == TYPE_INTEGER) {
                result->i_val = var->i_val;
            } else if (var->type == TYPE_SINGLE) {
                result->s_val = var->s_val;
            } else if (var->type == TYPE_DOUBLE) {
                result->d_val = var->d_val;
            }
        }
        return;
    }
    
    // Default fallback
    gw_error(2); // Syntax error
    result->type = TYPE_INTEGER;
    result->i_val = 0;
}
