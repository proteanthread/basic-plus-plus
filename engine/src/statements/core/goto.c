/**
 * @file goto.c
 * @brief GOTO unconditional branch statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements standard GOTO line number jumps and global label branching (GOTO @label).
 *
 * 2. WHY IT EXISTS:
 * Serves as the core unconditional control flow micro-library across all execution profiles.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Parses target numeric line number or global label token. Verifies existence in the program line map
 * via mem_program_get(). Modifies current VM line position pointer without host stack frame allocation.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_goto'. Directly includes "statements/core/goto.h",
 * "vm/vm.h", "memory/memory.h", and "runtime/micro_lib_metadata.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To add scoped label jumps, integrate scope label resolution hooks before checking program line store.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Strict line lookup verification invariant: jumping to a non-existent line number MUST emit
 * ERR_UNDEFINED_LINE_NUMBER (Error 8).
 *
 * 8. WHAT TO EXPECT:
 * Modifies VM current line pointer and returns ERR_NONE on success, or ERR_UNDEFINED_LINE_NUMBER.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect mem_program_get() line indexing table. Verify double-to-integer line number conversion formatting.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Target line exists in program memory structure. Valid VMContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 64-bit line number comparison safety.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/memory/memory.c
 * - engine/src/vm/exec.c
 * Prerequisite Header Files:
 * - engine/include/statements/core/goto.h
 * - engine/include/vm/vm.h
 * - engine/include/memory/memory.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "statements/core/goto.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "memory/memory.h"
#include "runtime/metadata.h"
#include "platform/platform.h"
#include <string.h>

void stmt_goto_register(void) {
    MicroLibMetadata meta = {
        .name = "GOTO",
        .category = "Control Flow",
        .syntax = "GOTO line_num",
        .help_text = "Unconditionally transfers execution to the specified program line number.",
        .error_codes = "Error 8: Undefined line number (target line does not exist), Error 2: Syntax error (missing line number)"
    };
    microlib_register(&meta);
}

BppError stmt_goto_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_GLOBAL_LABEL) {
        char label_name[64];
        int len = (int)(tok.length < sizeof(label_name) - 1 ? tok.length : sizeof(label_name) - 1);
        memcpy(label_name, tok.as.string, len);
        label_name[len] = '\0';

        char filename[256];
        BppLineNumber target_line = 0.0;
        if (!metadata_resolve_label(vm_get_metadata(vm), label_name, filename, sizeof(filename), &target_line)) {
            err.code = 8;
            err.message = "Undefined global label";
            return err;
        }

        const char *cur_file = vm_get_current_filename(vm);
        if (filename[0] != '\0' && cur_file[0] != '\0' && strcasecmp(filename, cur_file) != 0) {
            BppError load_err = vm_load_program_file(vm, filename);
            if (load_err.code != 0) return load_err;
        }

        vm_jump(vm, target_line, NULL);
        return err;
    }

    if (tok.type != TOK_NUMBER) {
        err.code = 2; /* Syntax error */
        err.message = "Expected line number after GOTO";
        return err;
    }

    BppLineNumber target = tok.as.number;
    if (!mem_program_get(vm_get_mem(vm), target)) {
        err.code = 8; /* Undefined line number */
        err.message = "Undefined line number in GOTO";
        return err;
    }

    vm_jump(vm, target, NULL);
    return err;
}
