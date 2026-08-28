// FILENAME: defdbl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, variables.h, variables.c)
// NEEDS: libengine (defdbl.h, string.c)
// Provides runtime implementation for the DEFDBL statement in BASIC++.
//
// ---- Includes ----

#include "statements/variables/declaration/defdbl.h"
#include "runtime/variables.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <ctype.h>

void stmt_defdbl_register(void) {
    MicroLibMetadata meta = {
        .name = "DEFDBL",
        .category = "Variables & Types",
        .syntax = "DEFDBL letter_range [, letter_range...]",
        .help_text = "Sets default type of variables beginning with specified letters to double-precision float.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

BppError stmt_defdbl_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    VariableContext *vc = vm_get_var(vm);

    while (true) {
        BppToken tok = lex_next(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
            break;
        }
        if (tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected letter in DEFDBL type range";
            return err;
        }

        char start_letter = (char)toupper((unsigned char)tok.start[0]);
        char end_letter = start_letter;

        BppToken next = lex_peek(lex);
        if (next.type == TOK_MINUS) {
            lex_next(lex); // Consume '-'
            BppToken end_tok = lex_next(lex);
            if (end_tok.type != TOK_IDENT) {
                err.code = 2; err.message = "Expected end letter in DEFDBL range";
                return err;
            }
            end_letter = (char)toupper((unsigned char)end_tok.start[0]);
        }

        var_set_def_type(vc, NULL, start_letter, end_letter, VAL_NUMBER);

        next = lex_peek(lex);
        if (next.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
        } else {
            break;
        }
    }
    return err;
}
