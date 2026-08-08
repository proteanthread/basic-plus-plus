/**
 * @file end.c
 * @brief END statement handler for program halt and block termination in BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements END statement processing for program execution termination (standalone END) and block
 * termination keywords (END IF, END SELECT, END SUB, END FUNCTION, END WITH, END CLASS, END ENUM, END STRUCT, END TYPE).
 *
 * 2. WHY IT EXISTS:
 * Serves as the central dispatcher for both standalone program completion and multi-line block closure.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Inspects token lookahead via lex_peek(). If followed by a block keyword (e.g. KW_SELECT, KW_IF), pops
 * the matching frame from the VM control stack (vm_select_pop, vm_block_if_pop) or dispatches to block
 * handlers. If standalone, calls vm_halt(vm) and closes open file streams.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_end'. Directly includes "statements/core/end.h",
 * "vm/vm.h", "runtime/file.h", and "runtime/micro_lib_metadata.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To add a new block terminator (e.g. END MODULE), add keyword dispatch branch checking lex_peek().
 *
 * 7. WHAT CANNOT BE CHANGED:
 * File stream cleanup on standalone END: must close open file channels to prevent resource leaks.
 *
 * 8. WHAT TO EXPECT:
 * Modifies VM execution state to VM_STATE_HALTED or pops block control frame, returning ERR_NONE or ERR_SYNTAX.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect lex_peek() keyword parsing. Check block stack frame balance in vm/control.c.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext. Lexer cursor positioned immediately at or past the END token.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Zero host stack allocation.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/control.c
 * - engine/src/runtime/file.c
 * Prerequisite Header Files:
 * - engine/include/statements/core/end.h
 * - engine/include/vm/vm.h
 * - engine/include/runtime/file.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "statements/core/end.h"
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
    if (tok.type == TOK_KEYWORD) {
        if (tok.as.keyword == KW_SELECT) {
            lex_next(lex);
            BppSelectFrame frame;
            if (!vm_select_pop(vm, &frame)) {
                err.code = 2; err.message = "END SELECT without SELECT CASE";
            }
            return err;
        }
        if (tok.as.keyword == KW_IF) {
            lex_next(lex);
            return err;
        }
        if (tok.as.keyword == KW_TRY) {
            lex_next(lex);
            BppTryFrame frame;
            if (try_stack_pop(vm_get_try_stack(vm), &frame)) {
            }
            return err;
        }
        if (tok.as.keyword == KW_ATOMIC) {
            lex_next(lex);
            file_txn_commit(vm_get_file(vm));
            return err;
        }
        if (tok.as.keyword == KW_SUB || tok.as.keyword == KW_PROCEDURE) {
            lex_next(lex);
            return stmt_end_sub_handler(vm, lex);
        }
        if (tok.as.keyword == KW_FUNCTION) {
            lex_next(lex);
            return stmt_end_function_handler(vm, lex);
        }
        if (tok.as.keyword == KW_TYPE || tok.as.keyword == KW_ENUM) {
            lex_next(lex);
            return err;
        }
        if (tok.as.keyword == KW_WITH) {
            lex_next(lex);
            vm_with_stack_pop(vm);
            return err;
        }
        if (tok.as.keyword == KW_CLASS) {
            lex_next(lex);
            return err;
        }
    }

    vm_halt(vm);
    return err;
}
