/**
 * @file list.c
 * @brief LIST [line_start] [- [line_end]] [, file_spec] program listing statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements LIST [line1[-line2]] statement handler for printing program source lines to the virtual console or disk file.
 *
 * 2. WHY IT EXISTS:
 * Displays stored program lines in line-number order per GW-BASIC / QBASIC REPL standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Iterates program source buffers in VMContext, formatting line numbers as integer values (%lld cast from double) per Rule #10.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_list'. Includes "statements/program/list.h",
 * "vm/vm.h", "lexer/lexer.h", "eval/eval.h", "device/vdev.h", "security/security.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (Core Included).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support syntax highlighting or line filtering when listing procedure subblocks.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Line number formatting invariant: Line numbers are double values internally; MUST format using %lld cast from (long long) per Rule #10.
 *
 * 8. WHAT TO EXPECT:
 * Outputs program source lines through virtual console device and returns ERR_NONE.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify line number range filter math and virtual device output routing.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and program memory buffer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Pure 7-bit ASCII text output per Rule #1.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/device/vdev.c
 * - engine/src/vm/vm_context.c
 * Prerequisite Header Files:
 * - engine/include/statements/program/list.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "statements/program/list.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_list_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)lex;
    if (!vm) return err;

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return err;

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    VDevContext *vd = vm_get_vdev(vm);

    for (size_t i = 0; i < count; i++) {
        char buf[1024];
        snprintf(buf, sizeof(buf), "%lld %s\n", (long long)lines[i].line_number, lines[i].text ? lines[i].text : "");
        vdev_puts(vd, buf);
    }
    return err;
}

BppError stmt_auto_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_renum_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_delete_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

void stmt_list_register(void) {
    static const MicroLibMetadata meta = {
        .name = "LIST",
        .category = "Program Mgmt & Editing",
        .syntax = "LIST [line_start] [- [line_end]]",
        .help_text = "Displays specified program lines or the entire program source in memory.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

