// FILENAME: end.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h)
// NEEDS: libengine (end.h, string.c, vm.h)
// Provides runtime implementation for the END statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/end.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/file.h"
#include "vm/vm.h"
#include <string.h>

void stmt_end_register(void) {
    MicroLibMetadata meta = {
        .name = "END",
        .category = "Control Flow",
        .syntax = "END [IF | SUB | FUNCTION | SELECT | STRUCT]",
        .help_text = "Terminates program execution or closes structured block definitions.",
        .error_codes = "Error 2: Syntax error (mismatched block terminator)"
    };
    microlib_register(&meta);
}

extern BppError stmt_end_sub_handler(VMContext *vm, LexerContext *lex);
extern BppError stmt_end_function_handler(VMContext *vm, LexerContext *lex);

BppError stmt_end_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        bool is_select  = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_SELECT) ||
                          (tok.type == TOK_IDENT && tok.length == 6 && strncasecmp(tok.start, "SELECT", 6) == 0);
        bool is_if      = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_IF) ||
                          (tok.type == TOK_IDENT && tok.length == 2 && strncasecmp(tok.start, "IF", 2) == 0);
        bool is_try     = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_TRY) ||
                          (tok.type == TOK_IDENT && tok.length == 3 && strncasecmp(tok.start, "TRY", 3) == 0);
        bool is_atomic  = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ATOMIC) ||
                          (tok.type == TOK_IDENT && tok.length == 6 && strncasecmp(tok.start, "ATOMIC", 6) == 0);
        bool is_sub     = (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_SUB || tok.as.keyword == KW_PROCEDURE)) ||
                          (tok.type == TOK_IDENT && tok.length == 3 && strncasecmp(tok.start, "SUB", 3) == 0);
        bool is_func    = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_FUNCTION) ||
                          (tok.type == TOK_IDENT && tok.length == 8 && strncasecmp(tok.start, "FUNCTION", 8) == 0);
        bool is_type    = (tok.type == TOK_KEYWORD && (tok.as.keyword == KW_TYPE || tok.as.keyword == KW_ENUM)) ||
                          (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "TYPE", 4) == 0);
        bool is_with    = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_WITH) ||
                          (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "WITH", 4) == 0);
        bool is_class   = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_CLASS) ||
                          (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "CLASS", 5) == 0);
        bool is_record  = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_RECORD) ||
                          (tok.type == TOK_IDENT && tok.length == 6 && strncasecmp(tok.start, "RECORD", 6) == 0);
        bool is_when    = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_WHEN) ||
                          (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "WHEN", 4) == 0);
        bool is_module  = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_MODULE) ||
                          (tok.type == TOK_IDENT && tok.length == 6 && strncasecmp(tok.start, "MODULE", 6) == 0);
        bool is_scope   = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_SCOPE) ||
                          (tok.type == TOK_IDENT && tok.length == 5 && strncasecmp(tok.start, "SCOPE", 5) == 0);
        bool is_picture = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_PICTURE) ||
                          (tok.type == TOK_IDENT && tok.length == 7 && strncasecmp(tok.start, "PICTURE", 7) == 0);
        bool is_handler = (tok.type == TOK_KEYWORD && tok.as.keyword == KW_HANDLER) ||
                          (tok.type == TOK_IDENT && tok.length == 7 && strncasecmp(tok.start, "HANDLER", 7) == 0);

        if (is_select) {
            lex_next(lex);
            BppSelectFrame frame;
            if (!vm_select_pop(vm, &frame)) {
                err.code = 2; err.message = "END SELECT without SELECT CASE";
            } else if (frame.val.type == VAL_STRING && frame.val.as.string) {
                str_release(vm_get_str(vm), frame.val.as.string);
            }
            return err;
        }
        if (is_if || is_when || is_module || is_scope || is_picture || is_handler || is_record) {
            lex_next(lex);
            return err;
        }
        if (is_try) {
            lex_next(lex);
            BppTryFrame frame;
            if (try_stack_pop(vm_get_try_stack(vm), &frame)) {
            }
            return err;
        }
        if (is_atomic) {
            lex_next(lex);
            file_txn_commit(vm_get_file(vm));
            return err;
        }
        if (is_sub) {
            lex_next(lex);
            return stmt_end_sub_handler(vm, lex);
        }
        if (is_func) {
            lex_next(lex);
            return stmt_end_function_handler(vm, lex);
        }
        if (is_type) {
            lex_next(lex);
            return err;
        }
        if (is_with) {
            lex_next(lex);
            vm_with_stack_pop(vm);
            return err;
        }
        if (is_class) {
            lex_next(lex);
            return err;
        }
    }

    vm_halt(vm);
    return err;
}
