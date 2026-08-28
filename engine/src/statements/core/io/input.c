// FILENAME: input.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c, using.h, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, stmt.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the INPUT statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "runtime/using.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void stmt_input_register(void) {
    MicroLibMetadata meta = {
        .name = "INPUT",
        .category = "Console I/O",
        .syntax = "INPUT [;] [\"prompt\"{;|,}] var1[, var2...]",
        .help_text = "Prompts the user or reads values from console input into target variables.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}

BppError stmt_file_input_handler(VMContext *vm, LexerContext *lex);

typedef struct {
    char var_name[64];
    bool is_array;
    int indices[8];
    int num_dims;
    bool is_str;
} InputTarget;

BppError stmt_input_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
        return stmt_file_input_handler(vm, lex);
    }

    VDevContext *vdev = vm_get_vdev(vm);
    VariableContext *var_ctx = vm_get_var(vm);
    StringContext *str_ctx = vm_get_str(vm);
    ArrayContext *arr_ctx = vm_get_arr(vm);

    bool is_using = false;
    char format_mask[128] = "";

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); // Consume USING
        BValue fmt_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (fmt_val.type != VAL_STRING) {
            err.code = 13; err.message = "Type mismatch: INPUT USING expects format string";
            return err;
        }
        strncpy(format_mask, str_data(fmt_val.as.string), sizeof(format_mask) - 1);
        str_release(vm_get_str(vm), fmt_val.as.string);
        is_using = true;

        // Consume comma or semicolon
        tok = lex_next(lex);
        if (tok.type != TOK_COMMA && tok.type != TOK_SEMICOLON) {
            err.code = 2; err.message = "Expected ',' or ';' after USING format string";
            return err;
        }
    }

    const char *prompt = NULL;
    char prompt_buf[512] = "";
    bool has_question_mark = true;

    tok = lex_peek(lex);
    if (tok.type == TOK_SEMICOLON) {
        lex_next(lex); // Consume leading semicolon (suppress newline)
        tok = lex_peek(lex);
    }
    if (tok.type == TOK_STRING) {
        lex_next(lex); // Consume prompt string
        prompt = tok.as.string;
        size_t prompt_len = tok.length;

        // Check separator
        tok = lex_next(lex);
        if (tok.type == TOK_SEMICOLON) {
            has_question_mark = true;
        } else if (tok.type == TOK_COMMA) {
            has_question_mark = false;
        } else {
            err.code = 2; err.message = "Expected ';' or ',' after INPUT prompt";
            return err;
        }

        size_t copy_len = (prompt_len < sizeof(prompt_buf) - 1) ? prompt_len : sizeof(prompt_buf) - 1;
        memcpy(prompt_buf, prompt, copy_len);
        prompt_buf[copy_len] = '\0';
    }

    // Parse 1 or more comma-separated target variables / array elements
    InputTarget targets[32];
    int target_count = 0;

    while (target_count < 32) {
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected variable name in INPUT statement";
            return err;
        }

        InputTarget *tgt = &targets[target_count++];
        memset(tgt, 0, sizeof(InputTarget));
        size_t copy_len = (tok.length < sizeof(tgt->var_name) - 1) ? tok.length : sizeof(tgt->var_name) - 1;
        memcpy(tgt->var_name, tok.start, copy_len);
        tgt->var_name[copy_len] = '\0';
        tgt->is_str = (tgt->var_name[strlen(tgt->var_name) - 1] == '$');

        BppToken peek_sub = lex_peek(lex);
        if (peek_sub.type == TOK_LPAREN) {
            tgt->is_array = true;
            lex_next(lex); // Consume '('
            while (tgt->num_dims < 8) {
                BValue idx_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (idx_val.type != VAL_NUMBER) {
                    err.code = 13;
                    err.message = "Type Mismatch in array subscript (expected numeric index)";
                    return err;
                }
                tgt->indices[tgt->num_dims++] = (int)idx_val.as.number;

                peek_sub = lex_peek(lex);
                if (peek_sub.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (peek_sub.type == TOK_RPAREN) {
                    lex_next(lex);
                    break;
                } else {
                    err.code = 2;
                    err.message = "Syntax error in array subscript (expected ',' or ')')";
                    return err;
                }
            }
        }

        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    VDev *con_dev = vdev_get(vdev, "CON:");
    char input_buf[2048];

    while (true) {
        if (prompt) {
            vdev_puts(vdev, prompt_buf);
        }
        if (has_question_mark) {
            vdev_puts(vdev, "? ");
        }
        if (con_dev && con_dev->ops.flush) {
            con_dev->ops.flush(con_dev);
        }

        // Read line from VDev
        if (!con_dev || !con_dev->ops.gets || !con_dev->ops.gets(con_dev, input_buf, sizeof(input_buf))) {
            err.code = 5; err.message = "Failed to read input from console device";
            return err;
        }

        // Strip trailing newline
        size_t in_len = strlen(input_buf);
        while (in_len > 0 && (input_buf[in_len - 1] == '\n' || input_buf[in_len - 1] == '\r')) {
            input_buf[in_len - 1] = '\0';
            in_len--;
        }

        // Perform validation if USING mask is specified
        if (is_using && format_mask[0]) {
            char val_err[256] = "";
            if (!using_validate_input_string(input_buf, format_mask, val_err, sizeof(val_err))) {
                vdev_puts(vdev, "\x07"); // Beep
                vdev_puts(vdev, "?Redo from start: ");
                vdev_puts(vdev, val_err);
                vdev_puts(vdev, "\n");
                continue;
            }
        }
        break;
    }

    // Assign input values to target variables
    const char *scan_ptr = input_buf;
    for (int i = 0; i < target_count; ++i) {
        InputTarget *tgt = &targets[i];

        while (*scan_ptr && isspace((unsigned char)*scan_ptr)) scan_ptr++;

        char val_buf[512] = "";
        size_t val_len = 0;

        if (*scan_ptr == '"') {
            scan_ptr++;
            while (*scan_ptr && *scan_ptr != '"' && val_len < sizeof(val_buf) - 1) {
                val_buf[val_len++] = *scan_ptr++;
            }
            if (*scan_ptr == '"') scan_ptr++;
            while (*scan_ptr && *scan_ptr != ',') scan_ptr++;
            if (*scan_ptr == ',') scan_ptr++;
        } else {
            while (*scan_ptr && *scan_ptr != ',' && val_len < sizeof(val_buf) - 1) {
                val_buf[val_len++] = *scan_ptr++;
            }
            while (val_len > 0 && isspace((unsigned char)val_buf[val_len - 1])) val_len--;
            if (*scan_ptr == ',') scan_ptr++;
        }
        val_buf[val_len] = '\0';

        BValue assign_val;
        memset(&assign_val, 0, sizeof(assign_val));

        if (tgt->is_str) {
            assign_val.type = VAL_STRING;
            assign_val.as.string = str_create(str_ctx, val_buf, val_len);
        } else {
            char *endptr;
            double val = strtod(val_buf, &endptr);
            assign_val.type = VAL_NUMBER;
            assign_val.as.number = val;
        }

        if (tgt->is_array) {
            BValue *target = arr_get_element(arr_ctx, tgt->var_name, tgt->num_dims, tgt->indices, &err);
            if (err.code != 0 || !target) {
                if (assign_val.type == VAL_STRING && assign_val.as.string) {
                    str_release(str_ctx, assign_val.as.string);
                }
                return err;
            }
            if (target->type == VAL_STRING && target->as.string) {
                str_release(str_ctx, target->as.string);
            }
            *target = assign_val;
            if (assign_val.type == VAL_STRING && assign_val.as.string) {
                str_add_ref(assign_val.as.string);
            }
        } else {
            if (!var_assign(var_ctx, tgt->var_name, assign_val)) {
                err.code = 13;
                err.message = "Type mismatch assigning input to variable";
                if (assign_val.type == VAL_STRING && assign_val.as.string) {
                    str_release(str_ctx, assign_val.as.string);
                }
                return err;
            }
        }

        if (assign_val.type == VAL_STRING && assign_val.as.string) {
            str_release(str_ctx, assign_val.as.string);
        }
    }

    return err;
}

