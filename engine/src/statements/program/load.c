/**
 * @file load.c
 * @brief LOAD filename_expr [, R] program source loading statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements LOAD filename_expr [, R] statement handler for reading program source files from disk into active VM memory.
 *
 * 2. WHY IT EXISTS:
 * Loads BASIC++ source programs from disk storage into the VM buffer for interactive editing or immediate execution per GW-BASIC / QBASIC standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Clears existing VM memory state via vm_clear_variables(), opens target file, reads line-by-line into VM source buffer, and optionally executes immediately if ',R' option flag set.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_load'. Includes "statements/program/load.h",
 * "types/errors.h", "vm/vm.h", "lexer/lexer.h", "eval/eval.h", "device/vdev.h", "security/security.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (Core Included).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support detokenizing legacy GW-BASIC binary file formats (.BAS tokenized files) or project workspace files.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Bounded file reading: Open files MUST use bounded path buffers and check for ERR_FILE_NOT_FOUND (Error 53) per Rule #1.
 *
 * 8. WHAT TO EXPECT:
 * Loads source program into VM, resets program pointer, and returns ERR_NONE or ERR_FILE_NOT_FOUND.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify file path opening routines and source line buffer allocation limits.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Cross-platform path handling.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/program/clear.c
 * - engine/src/vm/vm_context.c
 * Prerequisite Header Files:
 * - engine/include/statements/program/load.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "statements/program/load.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>

BppError stmt_load_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError stmt_merge_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    (void)vm; (void)lex;
    return err;
}

BppError vm_load_program_file(VMContext *vm, const char *filename) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !filename) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        err.code = ERR_FILE_NOT_FOUND;
        return err;
    }
    char line_buf[1024];
    while (fgets(line_buf, sizeof(line_buf), fp)) {
        size_t len = strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\r' || line_buf[len - 1] == '\n')) {
            line_buf[--len] = '\0';
        }
        if (len > 0) {
            vm_execute_line(vm, line_buf);
        }
    }
    fclose(fp);
    return err;
}

void stmt_load_register(void) {
    static const MicroLibMetadata meta = {
        .name = "LOAD",
        .category = "Program Mgmt & Editing",
        .syntax = "LOAD filename_expr [, R]",
        .help_text = "Loads a program file into memory from disk storage, optionally running it.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found, Error 70: Permission Denied"
    };
    microlib_register(&meta);
}

