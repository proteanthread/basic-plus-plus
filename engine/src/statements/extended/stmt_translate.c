// FILENAME: stmt_translate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c)
// NEEDS: libkernel (vm.h, eval.h, lexer.h, errors.h, translate.h)
// Provides runtime implementation for the TRANSLATE statement (Wang 2200).
//
// ---- Includes ----

#include "statements/extended/stmt_translate.h"
#include "eval/functions/string/manipulation/translate.h"
#include "runtime/language_descriptor.h"
#include "runtime/memory.h"
#include "memory/memory.h"
#include "runtime/strings.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/variables.h"

static const LangDesc g_stmt_translate_desc = {
    .name = "TRANSLATE",
    .category = "Strings & Formatting",
    .syntax = "TRANSLATE var$ USING table$ | TRANSLATE var$, from$, to$ | TRANSLATE[from$, to$] var$",
    .description = "Performs in-place character translation on a string variable (Wang 2200).",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_translate_register(void) {
    lang_desc_register(&g_stmt_translate_desc);
}

BppError stmt_translate_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5;
        err.message = "Null VM or lexer context";
        return err;
    }

    BppToken tok = lex_peek(lex);
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_TRANSLATE) ||
        (tok.type == TOK_IDENT && runtime_strcasecmp(tok.start, "TRANSLATE") == 0)) {
        lex_next(lex);
    }

    const char *from_str = NULL;
    size_t from_len = 0;
    const char *to_str = NULL;
    size_t to_len = 0;
    char target_var[128];
    target_var[0] = '\0';

    if (lex_peek(lex).type == TOK_LBRACKET) {
        lex_next(lex); // consume [
        BValue vfrom = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (vfrom.type != VAL_STRING || !vfrom.as.string) {
            err.code = 13;
            err.message = "TRANSLATE requires string translation arguments";
            return err;
        }
        from_str = str_data(vfrom.as.string);
        from_len = str_len(vfrom.as.string);

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
            BValue vto = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (vto.type != VAL_STRING || !vto.as.string) {
                err.code = 13;
                err.message = "TRANSLATE requires string translation arguments";
                return err;
            }
            to_str = str_data(vto.as.string);
            to_len = str_len(vto.as.string);
        }

        if (lex_peek(lex).type == TOK_RBRACKET) {
            lex_next(lex);
        }

        BppToken vtok = lex_next(lex);
        if (vtok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected variable name in TRANSLATE";
            return err;
        }
        size_t nlen = (vtok.length < sizeof(target_var) - 1) ? vtok.length : sizeof(target_var) - 1;
        runtime_memcpy(target_var, vtok.start, nlen);
        target_var[nlen] = '\0';
    } else {
        BppToken vtok = lex_next(lex);
        if (vtok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected variable name in TRANSLATE";
            return err;
        }
        size_t nlen = (vtok.length < sizeof(target_var) - 1) ? vtok.length : sizeof(target_var) - 1;
        runtime_memcpy(target_var, vtok.start, nlen);
        target_var[nlen] = '\0';

        BppToken nxt = lex_peek(lex);
        if (nxt.type == TOK_KEYWORD && nxt.as.keyword == KW_USING) {
            lex_next(lex); // consume USING
            BValue vtbl = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (vtbl.type != VAL_STRING || !vtbl.as.string) {
                err.code = 13;
                err.message = "TRANSLATE USING requires string table";
                return err;
            }
            from_str = str_data(vtbl.as.string);
            from_len = str_len(vtbl.as.string);
        } else if (nxt.type == TOK_COMMA) {
            lex_next(lex); // consume comma
            BValue vfrom = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (vfrom.type != VAL_STRING || !vfrom.as.string) {
                err.code = 13;
                err.message = "TRANSLATE requires string translation arguments";
                return err;
            }
            from_str = str_data(vfrom.as.string);
            from_len = str_len(vfrom.as.string);

            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex);
                BValue vto = eval_expression(vm, lex, &err);
                if (err.code != 0) return err;
                if (vto.type != VAL_STRING || !vto.as.string) {
                    err.code = 13;
                    err.message = "TRANSLATE requires string translation arguments";
                    return err;
                }
                to_str = str_data(vto.as.string);
                to_len = str_len(vto.as.string);
            }
        } else {
            err.code = 2;
            err.message = "Expected USING or comma in TRANSLATE";
            return err;
        }
    }

    StringContext *str_ctx = vm_get_str(vm);
    VariableContext *var_ctx = vm_get_var(vm);
    BValue *curr_val = var_lookup(var_ctx, target_var, false);
    if (!curr_val || curr_val->type != VAL_STRING) {
        err.code = 13;
        err.message = "TRANSLATE target must be a string variable";
        return err;
    }

    const char *src = (curr_val->as.string) ? str_data(curr_val->as.string) : "";
    size_t src_len = (curr_val->as.string) ? str_len(curr_val->as.string) : 0;

    if (src_len == 0) {
        return err;
    }

    char stack_buf[2048];
    char *dest = stack_buf;

    if (src_len + 1 > sizeof(stack_buf)) {
        dest = (char *)mem_scratch_alloc(vm_get_mem(vm), src_len + 1);
        if (!dest) {
            err.code = 7;
            err.message = "Out of memory in TRANSLATE";
            return err;
        }
    }

    size_t final_len = 0;
    translate_string_in_place(dest, src_len + 1, src, src_len, from_str, from_len, to_str, to_len, &final_len);

    BppString *new_str = str_create(str_ctx, dest, final_len);
    BValue new_val;
    runtime_memset(&new_val, 0, sizeof(new_val));
    new_val.type = VAL_STRING;
    new_val.as.string = new_str;
    var_assign(var_ctx, target_var, new_val);

    return err;
}
