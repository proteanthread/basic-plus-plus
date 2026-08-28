// FILENAME: dispatch_call.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (dispatch_internal.h)
// Provides core logic and interface definitions for dispatch_call within BASIC++.
//
// ---- Includes ----

#include "eval/dispatch_internal.h"

//
// ---- Function Evaluation & Call Dispatch ----

BValue eval_builtin_function(VMContext *vm, const char *name, LexerContext *lex, bool has_parens, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    char uname[64];
    normalize_func_name(name, uname, sizeof(uname));

    if (dispatch_handle_special(vm, uname, lex, has_parens, &res, err)) {
        return res;
    }

    if (!has_parens) {
        return eval_builtin_function_impl(vm, uname, 0, NULL, err);
    }

    BValue args[10];
    int arg_count = 0;
    runtime_memset(args, 0, sizeof(args));


    BppToken tok = lex_peek(lex);
    if (tok.type != TOK_RPAREN) {
        while (true) {
            if (arg_count >= 10) {
                err->code = 2;
                err->message = "Too many arguments for function";
                break;
            }
            args[arg_count] = eval_expression(vm, lex, err);
            if (err->code != 0) break;
            arg_count++;

            tok = lex_peek(lex);
            if (tok.type == TOK_COMMA) {
                lex_next(lex);
            } else if (tok.type == TOK_RPAREN) {
                break;
            } else {
                err->code = 2;
                err->message = "Expected ',' or ')' in function call";
                break;
            }
        }
    }

    if (err->code == 0) {
        tok = lex_next(lex);
        if (tok.type != TOK_RPAREN) {
            err->code = 2;
            err->message = "Expected ')' to close function call";
        }
    }

    if (err->code == 0) {
        res = eval_builtin_function_impl(vm, uname, arg_count, args, err);
    }

    if (err->code != 0) {
        for (int j = 0; j < arg_count; j++) {
            if (args[j].type == VAL_STRING && args[j].as.string) {
                str_release(vm_get_str(vm), args[j].as.string);
                args[j].as.string = NULL;
            } else if (args[j].type == VAL_MAP && args[j].as.map) {
                map_release(vm_get_str(vm), args[j].as.map);
                args[j].as.map = NULL;
            }
        }
    }

    return res;
}
