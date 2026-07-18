/**
 * @file print_using.c
 * @brief PRINT USING formatted output engine.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements QBASIC-compatible PRINT USING statement logic, formatting
 *   numbers and strings into precise layouts and outputting to any FILE* stream.
 * - Why it exists: Provides reports, tabular formatting, financial displays ($ prefix), and
 *   fixed-width alignments.
 * - Why it works this way: It parses the format string into layout blocks and evaluates
 *   expressions sequentially, recycling the format string if the expression list is longer.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Numeric formatting algorithms, width limits, padding characters.
 * - What cannot be changed: Token evaluation sequence and format characters specification.
 * - What to expect: Invalid format strings will print raw format characters.
 * - What to do if something breaks: If formatting is misaligned, verify string slice calculations.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard printf formatting can format decimal floats (%g, %f).
 * - Portability concerns: None. C17 standard compliant.
 */

#include "bpp_stmt.h"
#include "bpp_eval.h"
#include "bpp_vdev.h"
#include "bpp_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Helper to format a double value based on a format pattern (like "###.##").
 */
static void format_numeric(double val, const char *fmt, size_t fmt_len, char *out_buf, size_t out_max) {
    /* Determine if we have decimals */
    const char *dot = strchr(fmt, '.');
    int dec_places = 0;
    if (dot) {
        dec_places = (int)(fmt_len - (dot - fmt) - 1);
    }

    bool has_comma = (strchr(fmt, ',') != NULL);
    bool has_plus = (strchr(fmt, '+') != NULL);
    bool has_minus = (fmt[fmt_len - 1] == '-');
    bool has_dollar = (strstr(fmt, "$$") != NULL);
    bool has_star = (strstr(fmt, "**") != NULL);

    /* Basic float print */
    char raw_buf[128];
    char printf_fmt[32];
    snprintf(printf_fmt, sizeof(printf_fmt), "%%.%df", dec_places);
    snprintf(raw_buf, sizeof(raw_buf), printf_fmt, val);

    /* Format into out_buf */
    size_t out_idx = 0;
    if (has_plus && val >= 0.0) {
        if (out_idx < out_max - 1) out_buf[out_idx++] = '+';
    }

    /* Print dollar/stars prefix if applicable */
    if (has_star) {
        /* Star fill is handled after size check or simply output stars */
        if (out_idx < out_max - 1) out_buf[out_idx++] = '*';
    }
    if (has_dollar) {
        if (out_idx < out_max - 1) out_buf[out_idx++] = '$';
    }

    /* Copy raw number digits, adding commas if requested */
    char *digit_start = raw_buf;
    if (raw_buf[0] == '-') {
        if (!has_minus && out_idx < out_max - 1) {
            out_buf[out_idx++] = '-';
        }
        digit_start++;
    }

    /* Find integer portion length */
    char *dot_in_raw = strchr(digit_start, '.');
    size_t int_len = dot_in_raw ? (size_t)(dot_in_raw - digit_start) : strlen(digit_start);

    for (size_t i = 0; i < int_len; i++) {
        if (out_idx < out_max - 1) {
            out_buf[out_idx++] = digit_start[i];
        }
        if (has_comma && i < int_len - 1 && (int_len - 1 - i) % 3 == 0) {
            if (out_idx < out_max - 1) out_buf[out_idx++] = ',';
        }
    }

    /* Copy decimal portion */
    if (dot_in_raw) {
        while (*dot_in_raw && out_idx < out_max - 1) {
            out_buf[out_idx++] = *dot_in_raw++;
        }
    }

    /* Trailing minus sign */
    if (has_minus && val < 0.0) {
        if (out_idx < out_max - 1) out_buf[out_idx++] = '-';
    } else if (has_minus) {
        if (out_idx < out_max - 1) out_buf[out_idx++] = ' ';
    }

    out_buf[out_idx] = '\0';
}

/**
 * @brief Core PRINT USING output loop printing formatted items to stream.
 */
void print_using_internal(VMContext *vm, LexerContext *lex, int channel) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* 1. Format string expression */
    BValue fmt_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        vm_set_error(vm, err.code, err.message);
        return;
    }
    if (fmt_val.type != VAL_STRING) {
        vm_set_error(vm, 13, "Type mismatch: PRINT USING expects format string");
        return;
    }
    const char *fmt_str = str_data(fmt_val.as.string);

    /* 2. Semicolon separator */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_SEMICOLON) {
        vm_set_error(vm, 2, "Expected ';' in PRINT USING statement");
        str_release(vm_get_str(vm), fmt_val.as.string);
        return;
    }

    /* 3. Expressions list formatting loop */
    const char *fmt_ptr = fmt_str;
    bool last_was_sep = false;

    while (true) {
        tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE)) {
            break;
        }

        /* Evaluate next expression to format */
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            vm_set_error(vm, err.code, err.message);
            str_release(vm_get_str(vm), fmt_val.as.string);
            return;
        }

        /* Recycle format pointer if we reached end of format string */
        if (*fmt_ptr == '\0') {
            fmt_ptr = fmt_str;
        }

        /* Find next format specifier in pattern */
        char spec[128] = {0};
        size_t spec_len = 0;
        bool is_string_spec = false;

        while (*fmt_ptr) {
            char c = *fmt_ptr;
            if (c == '!' || c == '&' || c == '\\') {
                is_string_spec = true;
                spec[spec_len++] = c;
                fmt_ptr++;
                if (c == '\\') {
                    while (*fmt_ptr && *fmt_ptr == ' ') {
                        spec[spec_len++] = *fmt_ptr++;
                    }
                    if (*fmt_ptr == '\\') {
                        spec[spec_len++] = *fmt_ptr++;
                    }
                }
                break;
            } else if (c == '#' || c == '+' || c == '-' || c == '$' || c == '*' || c == '.' || c == ',') {
                spec[spec_len++] = c;
                fmt_ptr++;
                while (*fmt_ptr && (*fmt_ptr == '#' || *fmt_ptr == '+' || *fmt_ptr == '-' || 
                                    *fmt_ptr == '$' || *fmt_ptr == '*' || *fmt_ptr == '.' || 
                                    *fmt_ptr == ',')) {
                    spec[spec_len++] = *fmt_ptr++;
                }
                break;
            } else {
                /* Print literal characters */
                if (channel == -1) {
                    vdev_putc(vm_get_vdev(vm), c);
                } else {
                    file_putc(vm_get_file(vm), channel, c);
                }
                fmt_ptr++;
            }
        }
        spec[spec_len] = '\0';

        /* Apply format specifier to the value */
        if (spec_len > 0) {
            char out_buf[256];
            out_buf[0] = '\0';

            if (val.type == VAL_STRING) {
                const char *s_data = str_data(val.as.string);
                if (spec[0] == '!') {
                    out_buf[0] = s_data[0];
                    out_buf[1] = '\0';
                } else if (spec[0] == '&') {
                    strncpy(out_buf, s_data, sizeof(out_buf) - 1);
                } else if (spec[0] == '\\') {
                    size_t target_len = spec_len; /* width of \   \ is spec_len */
                    memset(out_buf, ' ', target_len);
                    size_t copy_len = strlen(s_data);
                    if (copy_len > target_len) copy_len = target_len;
                    memcpy(out_buf, s_data, copy_len);
                    out_buf[target_len] = '\0';
                } else {
                    /* Fallback: print raw string if spec is numeric */
                    strncpy(out_buf, s_data, sizeof(out_buf) - 1);
                }
                str_release(vm_get_str(vm), val.as.string);
            } else {
                if (is_string_spec) {
                    /* Format number into string spec */
                    char temp[64];
                    snprintf(temp, sizeof(temp), "%g", val.as.number);
                    if (spec[0] == '!') {
                        out_buf[0] = temp[0];
                        out_buf[1] = '\0';
                    } else {
                        strncpy(out_buf, temp, sizeof(out_buf) - 1);
                    }
                } else {
                    /* Format numeric */
                    format_numeric(val.as.number, spec, spec_len, out_buf, sizeof(out_buf));
                }
            }

            if (channel == -1) {
                vdev_puts(vm_get_vdev(vm), out_buf);
            } else {
                file_puts(vm_get_file(vm), channel, out_buf);
            }
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA || tok.type == TOK_SEMICOLON) {
            lex_next(lex);
            last_was_sep = true;
        } else {
            last_was_sep = false;
        }
    }

    if (!last_was_sep) {
        if (channel == -1) {
            vdev_putc(vm_get_vdev(vm), '\n');
        } else {
            file_putc(vm_get_file(vm), channel, '\n');
        }
    }
    if (channel != -1) {
        file_flush(vm_get_file(vm), channel);
    }
    str_release(vm_get_str(vm), fmt_val.as.string);
}

/**
 * @brief PRINT USING statement handler.
 */
BppError stmt_print_using_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    print_using_internal(vm, lex, -1);
    return err;
}
