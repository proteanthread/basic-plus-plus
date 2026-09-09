// FILENAME: mid_stmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, mid_stmt.h, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the MID_STMT statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/assignment/mid_stmt.h"
#include "runtime/language_descriptor.h"
#include "runtime/variables.h"
#include "runtime/arrays.h"
#include "runtime/file.h"
#include "runtime/strings.h"
#include "platform/platform.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"

static const LangDesc g_mid__stmt_desc = {
    .name = "MID$ STMT",
    .category = "String Manipulation",
    .syntax = "MID$(string_var, start [, length]) = replacement$",
    .description = "Replaces characters inside a string variable starting at specified position.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

BppError stmt_mid_stmt_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if ((tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) &&
        ((tok.length == 4 && platform_strncasecmp(tok.start, "MID$", 4) == 0) ||
         (tok.length == 3 && platform_strncasecmp(tok.start, "MID", 3) == 0))) {
        lex_next(lex); // Consume MID$ / MID
    }

    tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        err.code = 2;
        err.message = "Expected '(' in MID$ statement";
        return err;
    }

    BppToken target_tok = lex_next(lex);
    if (target_tok.type != TOK_IDENT && target_tok.type != TOK_KEYWORD) {
        err.code = 2;
        err.message = "Expected string variable in MID$ statement";
        return err;
    }

    char target_name[64];
    size_t tlen = (target_tok.length < sizeof(target_name) - 1) ? target_tok.length : sizeof(target_name) - 1;
    runtime_memcpy(target_name, target_tok.start, tlen);
    target_name[tlen] = '\0';

    bool is_array = false;
    int indices[8];
    int num_dims = 0;

    if (lex_peek(lex).type == TOK_LPAREN) {
        is_array = true;
        lex_next(lex); // Consume '('
        while (num_dims < 8) {
            BValue idx_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (idx_val.type != VAL_NUMBER && idx_val.type != VAL_INTEGER) {
                if (idx_val.type == VAL_STRING && idx_val.as.string) str_release(vm_get_str(vm), idx_val.as.string);
                err.code = 13;
                err.message = "Type mismatch in array index";
                return err;
            }
            indices[num_dims++] = (int)idx_val.as.number;
            BppToken ptok = lex_peek(lex);
            if (ptok.type == TOK_COMMA) {
                lex_next(lex);
            } else if (ptok.type == TOK_RPAREN) {
                lex_next(lex); // Consume ')'
                break;
            } else {
                err.code = 2;
                err.message = "Expected ',' or ')' in array index";
                return err;
            }
        }
    }

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) {
        err.code = 2;
        err.message = "Expected ',' after variable in MID$ statement";
        return err;
    }

    BValue start_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (start_val.type != VAL_NUMBER && start_val.type != VAL_INTEGER) {
        if (start_val.type == VAL_STRING && start_val.as.string) str_release(vm_get_str(vm), start_val.as.string);
        err.code = 13;
        err.message = "Type mismatch in MID$ start position";
        return err;
    }
    int start_pos = (int)start_val.as.number;
    if (start_pos == 0) {
        err.code = 5;
        err.message = "Illegal function call (start position cannot be 0)";
        return err;
    }

    int replace_len = -1;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex); // Consume ','
        BValue len_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (len_val.type != VAL_NUMBER && len_val.type != VAL_INTEGER) {
            if (len_val.type == VAL_STRING && len_val.as.string) str_release(vm_get_str(vm), len_val.as.string);
            err.code = 13;
            err.message = "Type mismatch in MID$ length";
            return err;
        }
        replace_len = (int)len_val.as.number;
        if (replace_len < 0) {
            err.code = 5;
            err.message = "Illegal function call (length must be >= 0)";
            return err;
        }
    }

    tok = lex_next(lex);
    if (tok.type != TOK_RPAREN) {
        err.code = 2;
        err.message = "Expected ')' in MID$ statement";
        return err;
    }

    tok = lex_next(lex);
    if (tok.type != TOK_EQ) {
        err.code = 2;
        err.message = "Expected '=' in MID$ statement";
        return err;
    }

    BValue rhs_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (rhs_val.type != VAL_STRING || !rhs_val.as.string) {
        err.code = 13;
        err.message = "Type mismatch in MID$ replacement (expected string)";
        return err;
    }

    const char *rep_str = str_data(rhs_val.as.string);
    size_t rep_len = str_len(rhs_val.as.string);

    if (is_array) {
        ArrayContext *arr = vm_get_arr(vm);
        BValue *elem = arr_get_element(arr, target_name, num_dims, indices, &err);
        if (!elem) {
            str_release(vm_get_str(vm), rhs_val.as.string);
            if (err.code == 0) {
                err.code = 9;
                err.message = "Subscript out of range in MID$ array target";
            }
            return err;
        }
        if (elem->type == VAL_FIELD_STRING) {
            FileContext *fc = vm_get_file(vm);
            unsigned char *buf = file_get_record_buffer(fc, elem->as.field_str.channel);
            int width = elem->as.field_str.length;
            int offset = elem->as.field_str.offset;
            size_t start_idx = (start_pos < 0) ? (size_t)(((long long)width + start_pos + 1 < 1) ? 0 : ((long long)width + start_pos)) : (size_t)(start_pos - 1);
            if (buf && start_idx < (size_t)width) {
                size_t max_copy = (size_t)width - start_idx;
                size_t to_copy = rep_len;
                if (replace_len >= 0 && (size_t)replace_len < to_copy) to_copy = (size_t)replace_len;
                if (to_copy > max_copy) to_copy = max_copy;
                runtime_memcpy(buf + offset + start_idx, rep_str, to_copy);
            }
        } else if (elem->type == VAL_STRING) {
            const char *orig = elem->as.string ? str_data(elem->as.string) : "";
            size_t orig_len = elem->as.string ? str_len(elem->as.string) : 0;
            size_t start_idx = (start_pos < 0) ? (size_t)(((long long)orig_len + start_pos + 1 < 1) ? 0 : ((long long)orig_len + start_pos)) : (size_t)(start_pos - 1);
            if (start_idx < orig_len) {
                size_t max_copy = orig_len - start_idx;
                size_t to_copy = rep_len;
                if (replace_len >= 0 && (size_t)replace_len < to_copy) to_copy = (size_t)replace_len;
                if (to_copy > max_copy) to_copy = max_copy;

                char *new_buf = (char *)runtime_malloc(orig_len + 1);
                if (new_buf) {
                    runtime_memcpy(new_buf, orig, orig_len);
                    runtime_memcpy(new_buf + start_idx, rep_str, to_copy);
                    new_buf[orig_len] = '\0';
                    BppStringRef new_ref = str_create(vm_get_str(vm), new_buf, orig_len);
                    runtime_free(new_buf);
                    if (elem->as.string) str_release(vm_get_str(vm), elem->as.string);
                    elem->as.string = new_ref;
                }
            }
        }
    } else {
        VariableContext *vc = vm_get_var(vm);
        BValue *var = var_lookup(vc, target_name, false);
        if (var && var->type == VAL_FIELD_STRING) {
            FileContext *fc = vm_get_file(vm);
            unsigned char *buf = file_get_record_buffer(fc, var->as.field_str.channel);
            int width = var->as.field_str.length;
            int offset = var->as.field_str.offset;
            size_t start_idx = (start_pos < 0) ? (size_t)(((long long)width + start_pos + 1 < 1) ? 0 : ((long long)width + start_pos)) : (size_t)(start_pos - 1);
            if (buf && start_idx < (size_t)width) {
                size_t max_copy = (size_t)width - start_idx;
                size_t to_copy = rep_len;
                if (replace_len >= 0 && (size_t)replace_len < to_copy) to_copy = (size_t)replace_len;
                if (to_copy > max_copy) to_copy = max_copy;
                runtime_memcpy(buf + offset + start_idx, rep_str, to_copy);
            }
        } else {
            const char *orig = (var && var->type == VAL_STRING && var->as.string) ? str_data(var->as.string) : "";
            size_t orig_len = (var && var->type == VAL_STRING && var->as.string) ? str_len(var->as.string) : 0;
            size_t start_idx = (start_pos < 0) ? (size_t)(((long long)orig_len + start_pos + 1 < 1) ? 0 : ((long long)orig_len + start_pos)) : (size_t)(start_pos - 1);
            if (start_idx < orig_len) {
                size_t max_copy = orig_len - start_idx;
                size_t to_copy = rep_len;
                if (replace_len >= 0 && (size_t)replace_len < to_copy) to_copy = (size_t)replace_len;
                if (to_copy > max_copy) to_copy = max_copy;

                char *new_buf = (char *)runtime_malloc(orig_len + 1);
                if (new_buf) {
                    runtime_memcpy(new_buf, orig, orig_len);
                    runtime_memcpy(new_buf + start_idx, rep_str, to_copy);
                    new_buf[orig_len] = '\0';
                    BValue new_val;
                    runtime_memset(&new_val, 0, sizeof(new_val));
                    new_val.type = VAL_STRING;
                    new_val.as.string = str_create(vm_get_str(vm), new_buf, orig_len);
                    runtime_free(new_buf);
                    var_assign(vc, target_name, new_val);
                    str_release(vm_get_str(vm), new_val.as.string);
                }
            }
        }
    }

    str_release(vm_get_str(vm), rhs_val.as.string);
    return err;
}

void stmt_mid_stmt_register(void) {
    lang_desc_register(&g_mid__stmt_desc);
}
