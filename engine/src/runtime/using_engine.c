/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file using_engine.c
 * @brief Unified bidirectional USING formatting & validation engine.
 */

#include "runtime/using.h"
#include "device/vdev.h"
#include "runtime/num_format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

void using_parse_mask(const char *fmt_str, UsingMask *mask) {
    mask->token_count = 0;
    const char *p = fmt_str;
    while (*p && mask->token_count < MAX_USING_TOKENS) {
        UsingToken *tok = &mask->tokens[mask->token_count];
        memset(tok, 0, sizeof(*tok));

        /* 1. Check for single quoted literal block like 'Hex: ' */
        if (*p == '\'') {
            tok->type = USING_TOK_LITERAL;
            size_t len = 0;
            p++; /* Consume opening quote */
            while (*p && *p != '\'') {
                if (len < sizeof(tok->text) - 1) {
                    tok->text[len++] = *p;
                }
                p++;
            }
            if (*p == '\'') p++; /* Consume closing quote */
            tok->text[len] = '\0';
            mask->token_count++;
            continue;
        }

        /* 2. Check for attribute block like \A{code} */
        if (p[0] == '\\' && p[1] == 'A' && p[2] == '{') {
            tok->type = USING_TOK_ATTR;
            const char *start = p + 3;
            char *endptr;
            tok->code = (int)strtol(start, &endptr, 10);
            if (*endptr == '}') {
                p = endptr + 1;
                mask->token_count++;
                continue;
            }
        }

        /* 3. Check for inline conditional block like [==10] or [> >100] */
        if (*p == '[') {
            const char *start = p + 1;
            const char *end = strchr(start, ']');
            if (end) {
                char cond_buf[64] = {0};
                size_t clen = (size_t)(end - start);
                if (clen < sizeof(cond_buf)) {
                    memcpy(cond_buf, start, clen);
                    char op[8] = {0};
                    char clean_buf[64] = {0};
                    size_t c_idx = 0;
                    for (size_t i = 0; cond_buf[i]; i++) {
                        if (cond_buf[i] != ' ') clean_buf[c_idx++] = cond_buf[i];
                    }
                    char *val_ptr = NULL;
                    if (strncmp(clean_buf, "==", 2) == 0) { strcpy(op, "=="); val_ptr = clean_buf + 2; }
                    else if (strncmp(clean_buf, ">=", 2) == 0) { strcpy(op, ">="); val_ptr = clean_buf + 2; }
                    else if (strncmp(clean_buf, "<=", 2) == 0) { strcpy(op, "<="); val_ptr = clean_buf + 2; }
                    else if (strncmp(clean_buf, "<>", 2) == 0) { strcpy(op, "<>"); val_ptr = clean_buf + 2; } /* Between */
                    else if (strncmp(clean_buf, "><", 2) == 0) { strcpy(op, "><"); val_ptr = clean_buf + 2; } /* Excluding */
                    else if (clean_buf[0] == '>') { strcpy(op, ">"); val_ptr = clean_buf + 1; }
                    else if (clean_buf[0] == '<') { strcpy(op, "<"); val_ptr = clean_buf + 1; }

                    if (op[0]) {
                        tok->has_condition = true;
                        strcpy(tok->cond_op, op);
                        char *comma = strchr(val_ptr, ',');
                        if (comma) {
                            *comma = '\0';
                            tok->cond_val1 = strtod(val_ptr, NULL);
                            tok->cond_val2 = strtod(comma + 1, NULL);
                        } else {
                            tok->cond_val1 = strtod(val_ptr, NULL);
                        }
                    }
                }
                p = end + 1;
                /* Do NOT continue. Let the specifier directly following the condition be parsed under this same token */
            }
        }

        /* 4. String Field specifiers: !, &, \, '' */
        if (*p == '!') {
            tok->type = USING_TOK_STRING_FIELD;
            strcpy(tok->text, "!");
            p++;
            mask->token_count++;
            continue;
        }
        if (*p == '&') {
            tok->type = USING_TOK_STRING_FIELD;
            strcpy(tok->text, "&");
            p++;
            mask->token_count++;
            continue;
        }
        if (*p == '\\') {
            tok->type = USING_TOK_STRING_FIELD;
            size_t len = 0;
            tok->text[len++] = '\\';
            p++;
            while (*p && *p == ' ') {
                if (len < sizeof(tok->text) - 2) tok->text[len++] = *p;
                p++;
            }
            if (*p == '\\') {
                if (len < sizeof(tok->text) - 1) tok->text[len++] = '\\';
                p++;
            }
            tok->text[len] = '\0';
            mask->token_count++;
            continue;
        }

        /* 5. Form Feed / Line Feed / Bell / Column */
        if (*p == 'F' || *p == 'L') {
            char type_char = *p;
            const char *start = p + 1;
            char *endptr;
            int n = (int)strtol(start, &endptr, 10);
            if (endptr != start) {
                tok->type = (type_char == 'F') ? USING_TOK_FORM_FEED : USING_TOK_LINE_FEED;
                tok->code = n;
                p = endptr;
                mask->token_count++;
                continue;
            }
        }
        if (*p == 'G') {
            tok->type = USING_TOK_BELL;
            strcpy(tok->text, "G");
            p++;
            mask->token_count++;
            continue;
        }
        if (*p == 'C') {
            tok->type = USING_TOK_COLUMN;
            strcpy(tok->text, "C");
            p++;
            mask->token_count++;
            continue;
        }

        /* 6. Numeric formats: #, +, -, $, *, ., , */
        if (*p == '#' || *p == '+' || *p == '-' || *p == '$' || *p == '*' || *p == '.' || *p == ',' ||
            *p == 'Z' || *p == '0' || *p == 'E' || *p == '^' || *p == 'S' || *p == 'D' || *p == 'I' ||
            *p == 'O' || *p == 'H' || *p == 'B' || *p == 'T') {

            tok->type = USING_TOK_NUMERIC;
            size_t len = 0;
            while (*p && (*p == '#' || *p == '+' || *p == '-' || *p == '$' || *p == '*' || *p == '.' || *p == ',' ||
                          *p == 'Z' || *p == '0' || *p == 'E' || *p == '^' || *p == 'S' || *p == 'D' || *p == 'I' ||
                          *p == 'O' || *p == 'H' || *p == 'B' || *p == 'T' || *p == ' ')) {
                if (*p != ' ' || (len > 0 && tok->text[len - 1] != ' ')) {
                    if (len < sizeof(tok->text) - 1) tok->text[len++] = *p;
                }
                p++;
            }
            tok->text[len] = '\0';
            mask->token_count++;
            continue;
        }

        /* 7. Literal */
        tok->type = USING_TOK_LITERAL;
        tok->text[0] = *p;
        tok->text[1] = '\0';
        p++;
        mask->token_count++;
    }
}

bool using_eval_condition(double val, const UsingToken *tok) {
    if (!tok->has_condition) return true;
    const char *op = tok->cond_op;
    if (strcmp(op, "==") == 0) return val == tok->cond_val1;
    if (strcmp(op, ">") == 0) return val > tok->cond_val1;
    if (strcmp(op, "<") == 0) return val < tok->cond_val1;
    if (strcmp(op, ">=") == 0) return val >= tok->cond_val1;
    if (strcmp(op, "<=") == 0) return val <= tok->cond_val1;
    if (strcmp(op, "<    >") == 0 || strcmp(op, "<>") == 0) return val >= tok->cond_val1 && val <= tok->cond_val2;
    if (strcmp(op, "><") == 0) return val < tok->cond_val1 || val > tok->cond_val2;
    return true;
}

static void format_base(long long val, int base, char *out) {
    if (base == 16) {
        snprintf(out, 64, "%llX", val);
    } else if (base == 8) {
        snprintf(out, 64, "%llo", val);
    } else if (base == 2) {
        char temp[128];
        int idx = 0;
        unsigned long long uval = (unsigned long long)val;
        if (uval == 0) {
            strcpy(out, "0");
            return;
        }
        while (uval > 0) {
            temp[idx++] = (uval & 1) ? '1' : '0';
            uval >>= 1;
        }
        int out_idx = 0;
        for (int i = idx - 1; i >= 0; i--) {
            out[out_idx++] = temp[i];
        }
        out[out_idx] = '\0';
    }
}

void using_format_output(VMContext *vm, const UsingMask *mask, int *mask_idx, BValue val, char *out_buf, size_t out_max) {
    if (mask->token_count == 0) {
        strcpy(out_buf, "");
        return;
    }

    if (*mask_idx >= mask->token_count) {
        *mask_idx = 0;
    }

    const UsingToken *tok = &mask->tokens[*mask_idx];

    /* Handle Literal Tokens */
    if (tok->type == USING_TOK_LITERAL) {
        strncpy(out_buf, tok->text, out_max - 1);
        out_buf[out_max - 1] = '\0';
        (*mask_idx)++;
        return;
    }

    /* Format string attributes */
    if (tok->type == USING_TOK_ATTR) {
        char attr_buf[32] = "";
        if (tok->code >= 1 && tok->code <= 16) {
            snprintf(attr_buf, sizeof(attr_buf), "\x1b[38;5;%dm", tok->code - 1);
        } else if (tok->code == 17) {
            strcpy(attr_buf, "\x1b[4m");
        } else if (tok->code == 18) {
            strcpy(attr_buf, "\x1b[7m");
        } else if (tok->code == 26) {
            strcpy(attr_buf, "\x1b[5m");
        }
        strncpy(out_buf, attr_buf, out_max - 1);
        (*mask_idx)++;
        return;
    }

    /* Handle Form Feeds / Line Feeds / Bells */
    if (tok->type == USING_TOK_FORM_FEED) {
        char ff[32] = "\x0c";
        strncpy(out_buf, ff, out_max - 1);
        (*mask_idx)++;
        return;
    }
    if (tok->type == USING_TOK_LINE_FEED) {
        char lf[64] = "";
        int lines = tok->code > 0 ? tok->code : 1;
        for (int i = 0; i < lines && i < 10; i++) strcat(lf, "\n");
        strncpy(out_buf, lf, out_max - 1);
        (*mask_idx)++;
        return;
    }
    if (tok->type == USING_TOK_BELL) {
        strcpy(out_buf, "\x07");
        (*mask_idx)++;
        return;
    }

    /* Evaluate conditional check */
    if (val.type == VAL_NUMBER && tok->has_condition) {
        if (!using_eval_condition(val.as.number, tok)) {
            size_t w = strlen(tok->text);
            memset(out_buf, ' ', w);
            out_buf[w] = '\0';
            (*mask_idx)++;
            return;
        }
    }

    if (val.type == VAL_STRING) {
        const char *s_data = str_data(val.as.string);
        if (tok->type == USING_TOK_STRING_FIELD) {
            if (strcmp(tok->text, "!") == 0) {
                out_buf[0] = s_data[0];
                out_buf[1] = '\0';
            } else if (strcmp(tok->text, "&") == 0) {
                strncpy(out_buf, s_data, out_max - 1);
            } else if (tok->text[0] == '\\') {
                size_t w = strlen(tok->text);
                memset(out_buf, ' ', w);
                size_t copy_len = strlen(s_data);
                if (copy_len > w) copy_len = w;
                memcpy(out_buf, s_data, copy_len);
                out_buf[w] = '\0';
            } else {
                strncpy(out_buf, s_data, out_max - 1);
            }
        } else {
            strncpy(out_buf, s_data, out_max - 1);
        }
    } else {
        double num = val.as.number;
        if (tok->type == USING_TOK_NUMERIC) {
            bool has_hex = (strchr(tok->text, 'H') != NULL);
            bool has_oct = (strchr(tok->text, 'O') != NULL);
            bool has_bin = (strchr(tok->text, 'B') != NULL);

            if (has_hex || has_oct || has_bin) {
                long long int_val = (long long)num;
                char base_buf[128] = {0};
                format_base(int_val, has_hex ? 16 : (has_oct ? 8 : 2), base_buf);
                strncpy(out_buf, base_buf, out_max - 1);
            } else {
                const char *dot = strchr(tok->text, '.');
                int dec_places = 0;
                if (dot) {
                    const char *p_dec = dot + 1;
                    while (*p_dec == '#') {
                        dec_places++;
                        p_dec++;
                    }
                }

                bool has_comma = (strchr(tok->text, ',') != NULL);
                bool has_plus = (strchr(tok->text, '+') != NULL);
                bool has_minus = (tok->text[strlen(tok->text) - 1] == '-');
                bool has_dollar = (strstr(tok->text, "$$") != NULL) || (strchr(tok->text, '$') != NULL);
                bool has_star = (strstr(tok->text, "**") != NULL);
                bool zero_fill = (strchr(tok->text, 'Z') != NULL) || (strchr(tok->text, '0') != NULL);
                bool has_inverse = (strchr(tok->text, 'N') != NULL);

                char printf_fmt[32];
                char raw_buf[128];
                snprintf(printf_fmt, sizeof(printf_fmt), "%%.%df", dec_places);
                snprintf(raw_buf, sizeof(raw_buf), printf_fmt, num);

                size_t out_idx = 0;
                if (has_inverse && num < 0.0) {
                    strcpy(out_buf + out_idx, "\x1b[7m");
                    out_idx += strlen("\x1b[7m");
                }

                if (has_plus && num >= 0.0) {
                    out_buf[out_idx++] = '+';
                }

                if (has_star) {
                    out_buf[out_idx++] = '*';
                }
                if (has_dollar) {
                    out_buf[out_idx++] = '$';
                }

                char *digit_start = raw_buf;
                if (raw_buf[0] == '-') {
                    if (!has_minus) {
                        out_buf[out_idx++] = '-';
                    }
                    digit_start++;
                }

                char *dot_in_raw = strchr(digit_start, '.');
                size_t int_len = dot_in_raw ? (size_t)(dot_in_raw - digit_start) : strlen(digit_start);

                if (zero_fill && int_len < 5) {
                    size_t diff = 5 - int_len;
                    for (size_t d = 0; d < diff; d++) {
                        out_buf[out_idx++] = '0';
                    }
                }

                for (size_t i = 0; i < int_len; i++) {
                    out_buf[out_idx++] = digit_start[i];
                    if (has_comma && i < int_len - 1 && (int_len - 1 - i) % 3 == 0) {
                        out_buf[out_idx++] = ',';
                    }
                }

                if (dot_in_raw) {
                    while (*dot_in_raw) {
                        out_buf[out_idx++] = *dot_in_raw++;
                    }
                }

                if (has_minus && num < 0.0) {
                    out_buf[out_idx++] = '-';
                }

                if (has_inverse && num < 0.0) {
                    strcpy(out_buf + out_idx, "\x1b[27m");
                    out_idx += strlen("\x1b[27m");
                }

                out_buf[out_idx] = '\0';
            }
        } else {
            num_format_display(out_buf, out_max, num, true, true);
        }
    }

    (*mask_idx)++;
}

bool using_validate_char(char c, char mask_char, bool *uppercase, bool *lowercase) {
    if (mask_char == 'N') {
        return (c >= '0' && c <= '9');
    }
    if (mask_char == 'H') {
        return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
    }
    if (mask_char == 'O') {
        return (c >= '0' && c <= '7');
    }
    if (mask_char == 'T') {
        return !(c >= '0' && c <= '9');
    }
    if (mask_char == 'U') {
        *uppercase = true;
        return true;
    }
    if (mask_char == 'L') {
        *lowercase = true;
        return true;
    }
    if (mask_char == 'A') {
        return true;
    }
    return (c == mask_char);
}

bool using_validate_input_string(const char *input, const char *mask_str, char *err_msg, size_t err_max) {
    size_t in_len = strlen(input);
    size_t mask_len = strlen(mask_str);

    if (in_len > mask_len) {
        snprintf(err_msg, err_max, "Input length exceeds format limit");
        return false;
    }

    for (size_t i = 0; i < in_len; i++) {
        bool uppercase = false;
        bool lowercase = false;
        if (!using_validate_char(input[i], mask_str[i], &uppercase, &lowercase)) {
            snprintf(err_msg, err_max, "Character mismatch at index %zu (expected '%c')", i, mask_str[i]);
            return false;
        }
    }

    return true;
}
