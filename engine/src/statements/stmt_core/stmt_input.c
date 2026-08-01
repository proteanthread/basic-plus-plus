/**
 * @file stmt_input.c
 * @brief INPUT statement handler.
 */

#include "stmt/stmt.h"
#include "device/vdev.h"
#include "eval/eval.h"
#include "runtime/using.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

BppError stmt_file_input_handler(VMContext *vm, LexerContext *lex);

BppError stmt_input_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_HASH) {
        lex_next(lex); /* Consume '#' */
        return stmt_file_input_handler(vm, lex);
    }

    VDevContext *vdev = vm_get_vdev(vm);
    VariableContext *var_ctx = vm_get_var(vm);
    StringContext *str_ctx = vm_get_str(vm);

    bool is_using = false;
    char format_mask[128] = "";

    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); /* Consume USING */
        BValue fmt_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (fmt_val.type != VAL_STRING) {
            err.code = 13; err.message = "Type mismatch: INPUT USING expects format string";
            return err;
        }
        strncpy(format_mask, str_data(fmt_val.as.string), sizeof(format_mask) - 1);
        str_release(vm_get_str(vm), fmt_val.as.string);
        is_using = true;

        /* Consume comma or semicolon */
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
    if (tok.type == TOK_STRING) {
        lex_next(lex); /* Consume prompt string */
        prompt = tok.as.string;
        size_t prompt_len = tok.length;

        /* Check separator */
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

    /* Expect target variable identifier */
    tok = lex_next(lex);
    if (tok.type != TOK_IDENT) {
        err.code = 2; err.message = "Expected variable name in INPUT statement";
        return err;
    }

    char var_name[256];
    size_t copy_len = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
    memcpy(var_name, tok.as.string, copy_len);
    var_name[copy_len] = '\0';

    VDev *con_dev = vdev_get(vdev, "CON:");
    char input_buf[1024];

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

        /* Read line from VDev */
        if (!con_dev || !con_dev->ops.gets || !con_dev->ops.gets(con_dev, input_buf, sizeof(input_buf))) {
            err.code = 5; err.message = "Failed to read input from console device";
            return err;
        }

        /* Strip trailing newline */
        size_t in_len = strlen(input_buf);
        while (in_len > 0 && (input_buf[in_len - 1] == '\n' || input_buf[in_len - 1] == '\r')) {
            input_buf[in_len - 1] = '\0';
            in_len--;
        }

        /* Perform validation if USING mask is specified */
        if (is_using && format_mask[0]) {
            char val_err[256] = "";
            if (!using_validate_input_string(input_buf, format_mask, val_err, sizeof(val_err))) {
                vdev_puts(vdev, "\x07"); /* Beep */
                vdev_puts(vdev, "?Redo from start: ");
                vdev_puts(vdev, val_err);
                vdev_puts(vdev, "\n");
                continue;
            }
        }
        break;
    }

    BValue *var = var_lookup(var_ctx, var_name, true);
    if (!var) {
        err.code = 2; err.message = "Undeclared variable in INPUT statement (OPTION EXPLICIT)";
        return err;
    }

    BValue assign_val;
    memset(&assign_val, 0, sizeof(assign_val));

    if (var->type == VAL_STRING) {
        assign_val.type = VAL_STRING;
        assign_val.as.string = str_create(str_ctx, input_buf, strlen(input_buf));
    } else {
        char *endptr;
        double val = strtod(input_buf, &endptr);
        while (*endptr && isspace((unsigned char)*endptr)) {
            endptr++;
        }
        assign_val.type = VAL_NUMBER;
        assign_val.as.number = val;
    }

    if (!var_assign(var_ctx, var_name, assign_val)) {
        err.code = 13; err.message = "Type mismatch assigning input to variable";
        if (assign_val.type == VAL_STRING && assign_val.as.string) {
            str_release(str_ctx, assign_val.as.string);
        }
        return err;
    }

    if (assign_val.type == VAL_STRING && assign_val.as.string) {
        str_release(str_ctx, assign_val.as.string);
    }

    return err;
}
