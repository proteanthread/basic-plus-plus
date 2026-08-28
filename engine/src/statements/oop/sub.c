// FILENAME: sub.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (call.c, eval_expr_internal.h, exec_control_internal.h)
// NEEDED BY: libengine (exec_dispatch.c, exec_internal.h, let.c, ops.c)
// NEEDED BY: libengine (sub_internal.h)
// NEEDS: libengine (sub_internal.h)
// Provides runtime implementation for the SUB statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/sub_internal.h"

//
// ---- Statement Handlers ----

BppError stmt_sub_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (vm_is_running(vm)) {
        BppLineNumber target_line = 0;
        int nest = 0;
        MemoryContext *mem = vm_get_mem(vm);
        size_t count = 0;
        BppProgramLine *lines = mem_program_get_all(mem, &count);

        BppLineNumber cur_line = vm_get_current_line(vm);
        for (size_t i = 0; i < count; i++) {
            if (lines[i].line_number > cur_line) {
                LexerContext *chk_lex = lex_init(mem, lines[i].text);
                if (chk_lex) {
                    BppToken tok = lex_next(chk_lex);
                    if (tok.type == TOK_NUMBER) tok = lex_next(chk_lex);
                    if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_PUBLIC || tok.as.keyword == KW_PRIVATE)) {
                        tok = lex_next(chk_lex);
                    }
                    if (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_SUB || tok.as.keyword == KW_FUNCTION)) {
                        nest++;
                    } else if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_END) {
                        BppToken ntok = lex_next(chk_lex);
                        if (ntok.type == TOK_KEYWORD && (ntok.as.keyword == KW_SUB || ntok.as.keyword == KW_FUNCTION)) {
                            if (nest > 0) {
                                nest--;
                            } else {
                                target_line = lines[i].line_number;
                                lex_shutdown(chk_lex);
                                break;
                            }
                        }
                    }
                    lex_shutdown(chk_lex);
                }
            }
        }
        if (target_line > 0) {
            vm_jump(vm, target_line, NULL);
        }
    }
    return err;
}

BppError stmt_end_sub_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    return err;
}

BppError stmt_subend_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    return err;
}

BppError stmt_subexit_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err;
    memset(&err, 0, sizeof(err));
    BppSubFrame frame;
    if (!vm_sub_pop(vm, &frame)) {
        err.code = 33;
        err.message = "SUBEXIT without active SUB/FUNCTION";
        return err;
    }
    vm_jump(vm, frame.line, frame.pos);
    return err;
}

BppError stmt_procedure_handler(VMContext *vm, LexerContext *lex) {
    return stmt_sub_handler(vm, lex);
}

//
// ---- Metadata Registration ----

void stmt_sub_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SUB",
        .category = "Procedures & OOP",
        .syntax = "SUB name [(param1, param2...)] [STATIC]",
        .help_text = "Declares a named subroutine block with formal parameters.",
        .error_codes = "Error 2: Syntax Error, Error 35: Undefined SUB"
    };
    microlib_register(&meta);
}

void stmt_end_sub_register(void) {
    static const MicroLibMetadata meta = {
        .name = "END SUB",
        .category = "Procedures & OOP",
        .syntax = "END SUB",
        .help_text = "Terminates a SUB procedure block.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}

void stmt_procedure_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PROCEDURE",
        .category = "Procedures & OOP",
        .syntax = "PROCEDURE name [(param_list)]",
        .help_text = "Declares a named procedure block.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}
