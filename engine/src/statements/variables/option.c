/**
 * @file option.c
 * @brief OPTION BASE and OPTION EXPLICIT statement handler for BASIC++.
 */
#include "statements/variables/option.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/arrays.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include <string.h>

void stmt_option_register(void) {
    static const MicroLibMetadata meta = {
        .name = "OPTION",
        .category = "Variables & Memory",
        .syntax = "OPTION BASE {0 | 1} | OPTION EXPLICIT",
        .help_text = "Sets minimum subscript array indexing base (0 or 1) or enforces explicit variable declaration.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_option_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char kw[32];
        if (tok.length >= sizeof(kw)) tok.length = sizeof(kw) - 1;
        memcpy(kw, tok.start, tok.length);
        kw[tok.length] = '\0';

        if (platform_strcasecmp(kw, "BASE") == 0) {
            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            if (val.type == VAL_NUMBER) {
                int base = (int)val.as.number;
                if (base == 0 || base == 1) {
                    arr_set_option_base(vm_get_arr(vm), base);
                } else {
                    err.code = 5;
                    err.message = "Illegal Function Call";
                }
            }
        } else if (platform_strcasecmp(kw, "EXPLICIT") == 0) {
            /* Handled at VM configuration level */
        } else {
            err.code = 2;
            err.message = "Syntax Error in OPTION statement";
        }
    }

    return err;
}
