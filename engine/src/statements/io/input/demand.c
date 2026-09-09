// FILENAME: demand.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for DEMAND statement (JOSS / RAND P-2922).
//
// ---- Includes ----

#include "statements/io/input/demand.h"
#include "runtime/language_descriptor.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/strings.h"
#include "runtime/format/snprintf.h"
#include "device/vdev.h"
#include "platform/platform.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"
#include "runtime/conv/float_parse.h"

static const LangDesc g_demand_desc = {
    .name = "DEMAND",
    .category = "Console I/O",
    .syntax = "DEMAND var1 [, var2 ...] [IN min TO max] [DEFAULT def_val]",
    .description = "Prompts for variable values with automatic '<var> = ' naming and optional constraint validation (JOSS / RAND P-2922).",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_demand_register(void) {
    lang_desc_register(&g_demand_desc);
}

BppError stmt_demand_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2;
            err.message = "Expected variable name in DEMAND";
            return err;
        }

        char var_name[64] = {0};
        size_t vlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
        runtime_memcpy(var_name, tok.start, vlen);

        bool is_arr = false;
        int dims = 0;
        int indices[8] = {0};

        if (lex_peek(lex).type == TOK_LPAREN) {
            is_arr = true;
            lex_next(lex); // Consume '('
            while (dims < 8) {
                BValue idx_val = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (idx_val.type != VAL_NUMBER && idx_val.type != VAL_INTEGER) {
                    if (idx_val.type == VAL_STRING && idx_val.as.string) str_release(vm_get_str(vm), idx_val.as.string);
                    err.code = 13; err.message = "Type Mismatch in array subscript";
                    return err;
                }
                indices[dims++] = (int)idx_val.as.number;
                BppToken ptok = lex_peek(lex);
                if (ptok.type == TOK_COMMA) {
                    lex_next(lex);
                } else if (ptok.type == TOK_RPAREN) {
                    lex_next(lex);
                    break;
                } else {
                    err.code = 2; err.message = "Expected ',' or ')' in array index";
                    return err;
                }
            }
        }

        bool has_range = false;
        double min_val = 0.0, max_val = 0.0;
        bool has_default = false;
        BValue def_val;
        runtime_memset(&def_val, 0, sizeof(def_val));

        BppToken opt_tok = lex_peek(lex);
        if (opt_tok.type == TOK_IDENT && opt_tok.length == 2 && platform_strncasecmp(opt_tok.start, "IN", 2) == 0) {
            lex_next(lex); // Consume IN
            BValue min_res = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            min_val = min_res.as.number;

            BppToken to_tok = lex_next(lex);
            if (to_tok.type != TOK_KEYWORD && (to_tok.type != TOK_IDENT || to_tok.length != 2 || platform_strncasecmp(to_tok.start, "TO", 2) != 0)) {
                err.code = 2; err.message = "Expected TO in DEMAND range";
                return err;
            }
            BValue max_res = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            max_val = max_res.as.number;
            has_range = true;
        }

        opt_tok = lex_peek(lex);
        if (opt_tok.type == TOK_IDENT && opt_tok.length == 7 && platform_strncasecmp(opt_tok.start, "DEFAULT", 7) == 0) {
            lex_next(lex); // Consume DEFAULT
            def_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            has_default = true;
        }

        // Generate prompt string
        char prompt_buf[128];
        if (is_arr) {
            char idx_str[64] = "";
            for (int i = 0; i < dims; ++i) {
                char tmp[16];
                runtime_snprintf(tmp, sizeof(tmp), "%s%d", (i > 0 ? "," : ""), indices[i]);
                runtime_strcat(idx_str, tmp);
            }
            runtime_snprintf(prompt_buf, sizeof(prompt_buf), "%s(%s) = ", var_name, idx_str);
        } else {
            runtime_snprintf(prompt_buf, sizeof(prompt_buf), "%s = ", var_name);
        }

        // Loop until valid input received
        VDevContext *vdev = vm_get_vdev(vm);
        VDev *con_dev = vdev ? vdev_get(vdev, "CON:") : NULL;
        while (true) {
            if (vdev) vdev_puts(vdev, prompt_buf);
            if (con_dev && con_dev->ops.flush) {
                con_dev->ops.flush(con_dev);
            }

            char input_buf[256] = {0};
            if (!con_dev || !con_dev->ops.gets || !con_dev->ops.gets(con_dev, input_buf, sizeof(input_buf))) {
                // EOF on input or error
                break;
            }

            // Strip trailing newline/CR
            size_t in_len = runtime_strlen(input_buf);
            while (in_len > 0 && (input_buf[in_len - 1] == '\r' || input_buf[in_len - 1] == '\n')) {
                input_buf[--in_len] = '\0';
            }

            // Handle empty input with default
            if (in_len == 0 && has_default) {
                if (is_arr) {
                    BValue *elem = arr_get_element(vm_get_arr(vm), var_name, dims, indices, &err);
                    if (elem) *elem = def_val;
                } else {
                    var_assign(vm_get_var(vm), var_name, def_val);
                }
                break;
            }

            // Parse entered value
            bool is_str_var = (var_name[runtime_strlen(var_name) - 1] == '$');
            if (is_str_var) {
                BValue s_val;
                runtime_memset(&s_val, 0, sizeof(s_val));
                s_val.type = VAL_STRING;
                s_val.as.string = str_create(vm_get_str(vm), input_buf, in_len);
                if (is_arr) {
                    BValue *elem = arr_get_element(vm_get_arr(vm), var_name, dims, indices, &err);
                    if (elem) {
                        if (elem->type == VAL_STRING && elem->as.string) str_release(vm_get_str(vm), elem->as.string);
                        *elem = s_val;
                    }
                } else {
                    var_assign(vm_get_var(vm), var_name, s_val);
                }
                str_release(vm_get_str(vm), s_val.as.string);
                break;
            } else {
                double num_val = runtime_atof(input_buf);
                if (has_range && (num_val < min_val || num_val > max_val)) {
                    char err_msg[64];
                    runtime_snprintf(err_msg, sizeof(err_msg), "?Value must be between %g and %g\n", min_val, max_val);
                    if (vdev) vdev_puts(vdev, err_msg);
                    continue;
                }
                BValue n_val;
                runtime_memset(&n_val, 0, sizeof(n_val));
                n_val.type = VAL_NUMBER;
                n_val.as.number = num_val;
                if (is_arr) {
                    BValue *elem = arr_get_element(vm_get_arr(vm), var_name, dims, indices, &err);
                    if (elem) *elem = n_val;
                } else {
                    var_assign(vm_get_var(vm), var_name, n_val);
                }
                break;
            }
        }

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex); // Consume comma and continue with next variable
        } else {
            break;
        }
    }

    return err;
}
