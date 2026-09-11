// FILENAME: bppc_expr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bppc_transpile.c, bppc_stmt.c)
// NEEDS: libcore (bppc_internal.h), libplatform
// C expression generator and AST token sanitization for bppc transpiler.
//
// ---- Includes ----

#include "tools/bppc_internal.h"
#include "tools/bppc_freestanding.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/memory/alloc.h"
#include "runtime/math/math.h"
#include "runtime/conv/num_parse.h"

void bppc_sanitize_ident(const char *in, char *out, size_t out_sz) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j < out_sz - 1; i++) {
        char c = in[i];
        if (runtime_isalnum((unsigned char)c) || c == '_') {
            out[j++] = (char)runtime_toupper((unsigned char)c);
        } else if (c == '$') {
            if (j + 4 < out_sz) {
                out[j++] = '_';
                out[j++] = 'S';
                out[j++] = 'T';
                out[j++] = 'R';
            }
        } else if (c == '%' || c == '!' || c == '#' || c == '&') {
            // Type suffixes stripped in C identifier
        }
    }
    out[j] = '\0';
}

static void rewrite_binary_op(char *expr, const char *op_name, const char *c_func) {
    char buf[1024] = {0};
    int b = 0;
    size_t oplen = runtime_strlen(op_name);
    for (int i = 0; expr[i] && b < (int)sizeof(buf) - 40; ) {
        // String literal shielding (never rewrite inside quotes)
        if (expr[i] == '"') {
            buf[b++] = expr[i++];
            while (expr[i] && expr[i] != '"' && b < (int)sizeof(buf) - 2) {
                if (expr[i] == '\\' && expr[i+1]) {
                    buf[b++] = expr[i++];
                }
                buf[b++] = expr[i++];
            }
            if (expr[i] == '"') buf[b++] = expr[i++];
            continue;
        }

        bool is_match = false;
        if (runtime_isalpha((unsigned char)op_name[0])) {
            is_match = ((i == 0 || runtime_isspace((unsigned char)expr[i-1]) || expr[i-1] == ')') &&
                        runtime_strncasecmp(expr + i, op_name, oplen) == 0 &&
                        (expr[i+oplen] == '\0' || runtime_isspace((unsigned char)expr[i+oplen]) || expr[i+oplen] == '('));
        } else {
            if (runtime_strncmp(expr + i, op_name, oplen) == 0) {
                char next_c = expr[i + oplen];
                char prev_c = (i > 0) ? expr[i - 1] : ' ';
                if (oplen == 1 && (op_name[0] == '<' || op_name[0] == '>' || op_name[0] == '=')) {
                    if (next_c != '=' && next_c != '>' && prev_c != '<' && prev_c != '>' && prev_c != '!' && prev_c != '=') {
                        is_match = true;
                    }
                } else {
                    is_match = true;
                }
            }
        }

        if (is_match) {
            int left_end = b - 1;
            while (left_end >= 0 && runtime_isspace((unsigned char)buf[left_end])) left_end--;
            int left_start = left_end;
            if (left_end >= 0 && buf[left_end] == ')') {
                int p = 1;
                left_start = left_end - 1;
                while (left_start >= 0 && p > 0) {
                    if (buf[left_start] == ')') p++;
                    else if (buf[left_start] == '(') p--;
                    left_start--;
                }
                left_start++;
                int fn_pos = left_start - 1;
                while (fn_pos >= 0 && runtime_isspace((unsigned char)buf[fn_pos])) fn_pos--;
                if (fn_pos >= 0 && (runtime_isalnum((unsigned char)buf[fn_pos]) || buf[fn_pos] == '_' || buf[fn_pos] == '$')) {
                    while (fn_pos >= 0 && (runtime_isalnum((unsigned char)buf[fn_pos]) || buf[fn_pos] == '_' || buf[fn_pos] == '$')) fn_pos--;
                    left_start = fn_pos + 1;
                }
            } else {
                while (left_start >= 0 && (runtime_isalnum((unsigned char)buf[left_start]) || buf[left_start] == '_' || buf[left_start] == '$' || buf[left_start] == '.')) {
                    left_start--;
                }
                left_start++;
                if (left_start > 0 && (buf[left_start - 1] == '-' || buf[left_start - 1] == '+')) {
                    int prev = left_start - 2;
                    while (prev >= 0 && runtime_isspace((unsigned char)buf[prev])) prev--;
                    if (prev < 0 || buf[prev] == '(' || buf[prev] == ',' || buf[prev] == '=' || buf[prev] == '+' || buf[prev] == '-' || buf[prev] == '*' || buf[prev] == '/') {
                        left_start--;
                    }
                }
            }

            char left_op[128] = {0};
            if (left_end >= left_start && left_end - left_start + 1 < (int)sizeof(left_op)) {
                runtime_memcpy(left_op, buf + left_start, left_end - left_start + 1);
                left_op[left_end - left_start + 1] = '\0';
            }

            int right_pos = i + (int)oplen;
            while (expr[right_pos] && runtime_isspace((unsigned char)expr[right_pos])) right_pos++;
            int right_start = right_pos;
            int right_end = right_start;
            if (expr[right_start] == '(') {
                int p = 1;
                right_end = right_start + 1;
                while (expr[right_end] && p > 0) {
                    if (expr[right_end] == '(') p++;
                    else if (expr[right_end] == ')') p--;
                    right_end++;
                }
            } else {
                if (expr[right_end] == '-' || expr[right_end] == '+') {
                    right_end++;
                }
                while (expr[right_end] && (runtime_isalnum((unsigned char)expr[right_end]) || expr[right_end] == '_' || expr[right_end] == '$' || expr[right_end] == '.')) {
                    right_end++;
                }
                if (expr[right_end] == '(') {
                    int p = 1;
                    right_end++;
                    while (expr[right_end] && p > 0) {
                        if (expr[right_end] == '(') p++;
                        else if (expr[right_end] == ')') p--;
                        right_end++;
                    }
                }
            }

            char right_op[128] = {0};
            if (right_end > right_start && right_end - right_start < (int)sizeof(right_op)) {
                runtime_memcpy(right_op, expr + right_start, right_end - right_start);
                right_op[right_end - right_start] = '\0';
            }

            b = left_start;
            // Folklore Constant Folding: if both operands are numeric integer literals
            char *lend = NULL, *rend = NULL;
            long lval = runtime_strtol(left_op, &lend, 10);
            long rval = runtime_strtol(right_op, &rend, 10);
            if (lend && *lend == '\0' && rend && *rend == '\0' && left_op[0] != '\0' && right_op[0] != '\0') {
                if (runtime_strcasecmp(op_name, "MOD") == 0 && rval != 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%ld", lval % rval);
                    i = right_end;
                    continue;
                } else if (runtime_strcmp(op_name, "\\") == 0 && rval != 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%ld", lval / rval);
                    i = right_end;
                    continue;
                } else if (runtime_strcasecmp(op_name, "AND") == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%ld", lval & rval);
                    i = right_end;
                    continue;
                } else if (runtime_strcasecmp(op_name, "OR") == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%ld", lval | rval);
                    i = right_end;
                    continue;
                } else if (runtime_strcasecmp(op_name, "XOR") == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%ld", lval ^ rval);
                    i = right_end;
                    continue;
                } else if (runtime_strcmp(op_name, "=") == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%d", (lval == rval) ? -1 : 0);
                    i = right_end;
                    continue;
                } else if (runtime_strcmp(op_name, "<>") == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%d", (lval != rval) ? -1 : 0);
                    i = right_end;
                    continue;
                } else if (runtime_strcmp(op_name, "<") == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%d", (lval < rval) ? -1 : 0);
                    i = right_end;
                    continue;
                } else if (runtime_strcmp(op_name, "<=") == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%d", (lval <= rval) ? -1 : 0);
                    i = right_end;
                    continue;
                } else if (runtime_strcmp(op_name, ">") == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%d", (lval > rval) ? -1 : 0);
                    i = right_end;
                    continue;
                } else if (runtime_strcmp(op_name, ">=") == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "%d", (lval >= rval) ? -1 : 0);
                    i = right_end;
                    continue;
                }
            }

            // Folklore Strength Reduction: MOD 2^k -> & (2^k - 1)
            if (runtime_strcasecmp(op_name, "MOD") == 0) {
                char *endptr = NULL;
                long mval = runtime_strtol(right_op, &endptr, 10);
                if (endptr && *endptr == '\0' && mval > 0 && (mval & (mval - 1)) == 0) {
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "((int64_t)(%s) & %ld)", left_op, mval - 1);
                    i = right_end;
                    continue;
                }
            }

            // Folklore Strength Reduction: \ 2^k -> >> k
            if (runtime_strcmp(op_name, "\\") == 0) {
                char *endptr = NULL;
                long dval = runtime_strtol(right_op, &endptr, 10);
                if (endptr && *endptr == '\0' && dval > 1 && (dval & (dval - 1)) == 0) {
                    int shift = 0;
                    long temp = dval;
                    while (temp > 1) { shift++; temp >>= 1; }
                    b += runtime_snprintf(buf + b, sizeof(buf) - b, "((int64_t)(%s) >> %d)", left_op, shift);
                    i = right_end;
                    continue;
                }
            }

            b += runtime_snprintf(buf + b, sizeof(buf) - b, "%s(%s, %s)", c_func, left_op, right_op);
            i = right_end;
        } else {
            buf[b++] = expr[i++];
        }
    }
    buf[b] = '\0';
    runtime_strncpy(expr, buf, 1023);
    expr[1023] = '\0';
}

void bppc_convert_expression_to_c(const char *expr, char *out_c, size_t out_sz) {
    char clean_expr[1024];
    runtime_strncpy(clean_expr, expr, sizeof(clean_expr) - 1);
    clean_expr[sizeof(clean_expr) - 1] = '\0';
    rewrite_binary_op(clean_expr, "<>", "bpp_rt_ne");
    rewrite_binary_op(clean_expr, "<=", "bpp_rt_le");
    rewrite_binary_op(clean_expr, ">=", "bpp_rt_ge");
    rewrite_binary_op(clean_expr, "<", "bpp_rt_lt");
    rewrite_binary_op(clean_expr, ">", "bpp_rt_gt");
    rewrite_binary_op(clean_expr, "=", "bpp_rt_eq");
    rewrite_binary_op(clean_expr, "\\", "bpp_rt_idiv");
    rewrite_binary_op(clean_expr, "MOD", "bpp_rt_mod");
    rewrite_binary_op(clean_expr, "XOR", "bpp_rt_xor");
    rewrite_binary_op(clean_expr, "AND", "bpp_rt_and");
    rewrite_binary_op(clean_expr, "OR", "bpp_rt_or");

    int o = 0;
    for (int i = 0; clean_expr[i] && o < (int)out_sz - 10; ) {
        if (clean_expr[i] == '"') {
            out_c[o++] = clean_expr[i++];
            while (clean_expr[i] && clean_expr[i] != '"' && o < (int)out_sz - 5) {
                if (clean_expr[i] == '\\') {
                    out_c[o++] = clean_expr[i++];
                }
                if (clean_expr[i]) {
                    out_c[o++] = clean_expr[i++];
                }
            }
            if (clean_expr[i] == '"') {
                out_c[o++] = clean_expr[i++];
            }
            continue;
        }

        if (runtime_isalpha((unsigned char)clean_expr[i])) {
            char ident[64] = {0};
            int id_len = 0;
            while ((runtime_isalnum((unsigned char)clean_expr[i]) || clean_expr[i] == '_' || clean_expr[i] == '$') && id_len < 60) {
                ident[id_len++] = clean_expr[i++];
            }
            ident[id_len] = '\0';

            char ucase_id[64] = {0};
            for (int k = 0; ident[k]; k++) ucase_id[k] = (char)runtime_toupper((unsigned char)ident[k]);

            if (runtime_strcmp(ucase_id, "SIN") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_sin");
            } else if (runtime_strcmp(ucase_id, "COS") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_cos");
            } else if (runtime_strcmp(ucase_id, "TAN") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_tan");
            } else if (runtime_strcmp(ucase_id, "EXP") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_exp");
            } else if (runtime_strcmp(ucase_id, "LOG") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_log");
            } else if (runtime_strcmp(ucase_id, "FLOOR") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_floor");
            } else if (runtime_strcmp(ucase_id, "CEIL") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_ceil");
            } else if (runtime_strcmp(ucase_id, "PI") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "3.14159265358979323846");
            } else if (runtime_strcmp(ucase_id, "RAD") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "(0.017453292519943295 * ");
            } else if (runtime_strcmp(ucase_id, "DEG") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "(57.29577951308232 * ");
            } else if (runtime_strcmp(ucase_id, "ROUND") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_round");
            } else if (runtime_strcmp(ucase_id, "TRUNCATE") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_trunc");
            } else if (runtime_strcmp(ucase_id, "RND") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_rnd()");
            } else if (runtime_strcmp(ucase_id, "SQR") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_sqrt");
            } else if (runtime_strcmp(ucase_id, "ATN") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_atan");
            } else if (runtime_strcmp(ucase_id, "ASIN") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_asin");
            } else if (runtime_strcmp(ucase_id, "ACOS") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_acos");
            } else if (runtime_strcmp(ucase_id, "ABS") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_fabs");
            } else if (runtime_strcmp(ucase_id, "INT") == 0 || runtime_strcmp(ucase_id, "FIX") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_floor");
            } else if (runtime_strcmp(ucase_id, "LEN") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "(double)runtime_strlen");
            } else if (runtime_strcmp(ucase_id, "ASC") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "(double)(unsigned char)");
            } else if (runtime_strcmp(ucase_id, "CHR_STR") == 0 || runtime_strcmp(ucase_id, "CHR$") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_chr");
            } else if (runtime_strcmp(ucase_id, "STR_STR") == 0 || runtime_strcmp(ucase_id, "STR$") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_str");
            } else if (runtime_strcmp(ucase_id, "UCASE_STR") == 0 || runtime_strcmp(ucase_id, "UCASE$") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_ucase");
            } else if (runtime_strcmp(ucase_id, "LCASE_STR") == 0 || runtime_strcmp(ucase_id, "LCASE$") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_lcase");
            } else if (runtime_strcmp(ucase_id, "MID_STR") == 0 || runtime_strcmp(ucase_id, "MID$") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_mid");
            } else if (runtime_strcmp(ucase_id, "INSTR") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_instr");
            } else if (runtime_strcmp(ucase_id, "INPUT_STR") == 0 || runtime_strcmp(ucase_id, "INPUT$") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_input_dollar");
            } else if (runtime_strcmp(ucase_id, "SCREEN") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_screen");
            } else if (runtime_strcmp(ucase_id, "EOF") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_eof");
            } else if (runtime_strcmp(ucase_id, "FMOD") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "runtime_fmod");
            } else if (runtime_strcmp(ucase_id, "INT64_T") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "int64_t");
            } else if (runtime_strcmp(ucase_id, "UINT8_T") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "uint8_t");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_AND") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_and");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_OR") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_or");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_XOR") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_xor");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_EQ") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_eq");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_NE") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_ne");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_LT") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_lt");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_GT") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_gt");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_LE") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_le");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_GE") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_ge");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_IDIV") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_idiv");
            } else if (runtime_strcmp(ucase_id, "BPP_RT_MOD") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_mod");
            } else if (runtime_strcmp(ucase_id, "SGN") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, "((%s > 0) ? 1.0 : (%s < 0) ? -1.0 : 0.0)", ident, ident);
            } else if (runtime_strcmp(ucase_id, "AND") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, " & ");
            } else if (runtime_strcmp(ucase_id, "OR") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, " | ");
            } else if (runtime_strcmp(ucase_id, "XOR") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, " ^ ");
            } else if (runtime_strcmp(ucase_id, "NOT") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, " !");
            } else if (runtime_strcmp(ucase_id, "MOD") == 0) {
                o += runtime_snprintf(out_c + o, out_sz - o, " %% ");
            } else if (clean_expr[i] == '(' || clean_expr[i] == '[') {
                char close_delim = (clean_expr[i] == '[') ? ']' : ')';
                char open_delim = clean_expr[i++];
                char inside[256] = {0};
                int in_len = 0;
                int pcount = 1;
                while (clean_expr[i] && pcount > 0 && in_len < 240) {
                    if (clean_expr[i] == open_delim) pcount++;
                    else if (clean_expr[i] == close_delim) {
                        pcount--;
                        if (pcount == 0) { i++; break; }
                    }
                    inside[in_len++] = clean_expr[i++];
                }
                char *comma = runtime_strchr(inside, ',');
                char sanitized[64] = {0};
                bppc_sanitize_ident(ident, sanitized, sizeof(sanitized));

                if (runtime_strcasecmp(ident, "PEEK") == 0) {
                    if (comma) {
                        *comma = '\0';
                        char a_c[128] = {0};
                        bppc_convert_expression_to_c(bppc_trim(inside), a_c, sizeof(a_c));
                        int w = runtime_atoi(bppc_trim(comma + 1));
                        if (w == 2) o += runtime_snprintf(out_c + o, out_sz - o, "BPP_MEM16(%s)", a_c);
                        else if (w == 4) o += runtime_snprintf(out_c + o, out_sz - o, "BPP_MEM32(%s)", a_c);
                        else if (w == 8) o += runtime_snprintf(out_c + o, out_sz - o, "BPP_MEM64(%s)", a_c);
                        else o += runtime_snprintf(out_c + o, out_sz - o, "BPP_MEM8(%s)", a_c);
                    } else {
                        char a_c[128] = {0};
                        bppc_convert_expression_to_c(bppc_trim(inside), a_c, sizeof(a_c));
                        if (bppc_is_freestanding_or_uefi()) {
                            o += runtime_snprintf(out_c + o, out_sz - o, "BPP_MEM8(%s)", a_c);
                        } else {
                            o += runtime_snprintf(out_c + o, out_sz - o, "bpp_rt_mem[(int)(%s) & 0xFFFF]", a_c);
                        }
                    }
                } else if (runtime_strcasecmp(ident, "MEM") == 0) {
                    char a_c[128] = {0};
                    bppc_convert_expression_to_c(bppc_trim(inside), a_c, sizeof(a_c));
                    o += runtime_snprintf(out_c + o, out_sz - o, "BPP_MEM8(%s)", a_c);
                } else if (runtime_strcasecmp(ident, "MEMW") == 0) {
                    char a_c[128] = {0};
                    bppc_convert_expression_to_c(bppc_trim(inside), a_c, sizeof(a_c));
                    o += runtime_snprintf(out_c + o, out_sz - o, "BPP_MEM16(%s)", a_c);
                } else if (runtime_strcasecmp(ident, "MEML") == 0) {
                    char a_c[128] = {0};
                    bppc_convert_expression_to_c(bppc_trim(inside), a_c, sizeof(a_c));
                    o += runtime_snprintf(out_c + o, out_sz - o, "BPP_MEM32(%s)", a_c);
                } else if (runtime_strcasecmp(ident, "PORT") == 0 || runtime_strcasecmp(ident, "INP") == 0) {
                    char p_c[128] = {0};
                    bppc_convert_expression_to_c(bppc_trim(inside), p_c, sizeof(p_c));
                    o += runtime_snprintf(out_c + o, out_sz - o, "BPP_PORT8(%s)", p_c);
                } else if (runtime_strncasecmp(ident, "FN", 2) == 0) {
                    // Function call: fn_FNA(args)
                    char args_c[512] = {0};
                    int arg_count = 0;
                    char cur_arg[128] = {0};
                    int c_len = 0;
                    int p_depth = 0;
                    for (int ci = 0; ; ci++) {
                        char c = inside[ci];
                        if (c == '(') p_depth++;
                        else if (c == ')') p_depth--;
                        if ((c == ',' && p_depth == 0) || c == '\0') {
                            cur_arg[c_len] = '\0';
                            char *targ = bppc_trim(cur_arg);
                            if (*targ) {
                                char carg_c[128] = {0};
                                bppc_convert_expression_to_c(targ, carg_c, sizeof(carg_c));
                                if (arg_count > 0) runtime_strncat(args_c, ", ", sizeof(args_c) - runtime_strlen(args_c) - 1);
                                runtime_strncat(args_c, carg_c, sizeof(args_c) - runtime_strlen(args_c) - 1);
                                arg_count++;
                            }
                            c_len = 0;
                            if (c == '\0') break;
                        } else {
                            if (c_len < (int)sizeof(cur_arg) - 2) cur_arg[c_len++] = c;
                        }
                    }
                    o += runtime_snprintf(out_c + o, out_sz - o, "fn_%s(%s)", sanitized, args_c);
                } else if (comma) {
                    *comma = '\0';
                    char c_arg1[128] = {0}, c_arg2[128] = {0};
                    bppc_convert_expression_to_c(bppc_trim(inside), c_arg1, sizeof(c_arg1));
                    bppc_convert_expression_to_c(bppc_trim(comma + 1), c_arg2, sizeof(c_arg2));
                    o += runtime_snprintf(out_c + o, out_sz - o, "var_%s[(int)(%s)][(int)(%s)]", sanitized, c_arg1, c_arg2);
                } else {
                    char c_arg1[128] = {0};
                    bppc_convert_expression_to_c(bppc_trim(inside), c_arg1, sizeof(c_arg1));
                    o += runtime_snprintf(out_c + o, out_sz - o, "var_%s[(int)(%s)]", sanitized, c_arg1);
                }
            } else {
                char sanitized[64] = {0};
                bppc_sanitize_ident(ident, sanitized, sizeof(sanitized));
                if (runtime_strncasecmp(ident, "FN", 2) == 0) {
                    o += runtime_snprintf(out_c + o, out_sz - o, "fn_%s()", sanitized);
                } else {
                    o += runtime_snprintf(out_c + o, out_sz - o, "var_%s", sanitized);
                }
            }
        } else if (clean_expr[i] == '<' && clean_expr[i+1] == '>') {
            o += runtime_snprintf(out_c + o, out_sz - o, " != ");
            i += 2;
        } else if (clean_expr[i] == '<' && clean_expr[i+1] == '=') {
            o += runtime_snprintf(out_c + o, out_sz - o, " <= ");
            i += 2;
        } else if (clean_expr[i] == '>' && clean_expr[i+1] == '=') {
            o += runtime_snprintf(out_c + o, out_sz - o, " >= ");
            i += 2;
        } else if (clean_expr[i] == '=') {
            o += runtime_snprintf(out_c + o, out_sz - o, " == ");
            i++;
        } else if (clean_expr[i] == '^') {
            o += runtime_snprintf(out_c + o, out_sz - o, " ^ ");
            i++;
        } else {
            out_c[o++] = clean_expr[i++];
        }
    }
    out_c[o] = '\0';
}

bool bppc_is_c_keyword_or_builtin(const char *name) {
    static const char *kw_list[] = {
        "FOR", "TO", "STEP", "BY", "NEXT", "WHILE", "WEND", "DO", "LOOP",
        "IF", "THEN", "ELSE", "ELSEIF", "ENDIF", "GOTO", "GOSUB", "RETURN", "END", "STOP",
        "PRINT", "DIM", "REDIM", "LET", "REM", "SIN", "COS", "TAN",
        "SQR", "LOG", "EXP", "ABS", "INT", "FIX", "RND", "LEN", "MID",
        "LEFT", "RIGHT", "STR", "VAL", "CHR_STR", "CHR$", "ASC", "SGN", "AND",
        "OR", "NOT", "XOR", "MOD", "SWAP", "SLEEP", "PAUSE", "CLS", "BEEP",
        "PI", "RAD", "DEG", "ROUND", "TRUNCATE", "ATN", "ASIN", "ACOS",
        "UCASE", "UCASE_STR", "UCASE$", "LCASE", "LCASE_STR", "LCASE$",
        "MID_STR", "MID$", "INSTR", "OPEN", "OUTPUT", "INPUT", "AS", "CLOSE", "KILL", "EOF", "FMOD",
        "TRON", "TROFF", "DEF",
        "PEEK", "POKE", "INP", "OUT", "MEM", "MEMW", "MEML", "PORT", "ASM",
        "BPP_RT_AND", "BPP_RT_OR", "BPP_RT_XOR", "BPP_RT_EQ", "BPP_RT_NE", "BPP_RT_LT", "BPP_RT_GT", "BPP_RT_LE", "BPP_RT_GE", "BPP_RT_IDIV", "BPP_RT_MOD",
        NULL
    };
    for (int i = 0; kw_list[i]; i++) {
        if (runtime_strcmp(name, kw_list[i]) == 0) return true;
    }
    return false;
}
