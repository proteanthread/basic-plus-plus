/**
 * @file gosub.c
 * @brief GOSUB subroutine call statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements GOSUB subroutine call execution, pushing return line address and statement offset
 * onto the heap-managed GOSUB call stack before jumping to target line or global label.
 *
 * 2. WHY IT EXISTS:
 * Serves as the core subroutine invocation mechanism in standard BASIC programs.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Pushes a stack frame containing the current line number and return token offset via vm_gosub_push().
 * Verifies target line existence via mem_program_get(). Modifies current VM line position to target.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_gosub'. Directly includes "statements/core/gosub.h",
 * "vm/vm.h", "memory/memory.h", and "runtime/micro_lib_metadata.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To support local argument passing (e.g. GOSUB line(arg1, arg2)), evaluate arguments prior to
 * pushing the stack frame and store local binding values in the GOSUB frame structure.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Stack push invariant: return address MUST be pushed onto the stack BEFORE modifying VM next_line.
 *
 * 8. WHAT TO EXPECT:
 * Returns ERR_NONE on successful jump, ERR_UNDEFINED_LINE_NUMBER if target missing, or ERR_OUT_OF_MEMORY if call stack overflows.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect GOSUB stack pointer depth in vm->gosub_stack. Verify RETURN handler pops matching frame.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext. Active call stack.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Non-recursive stack management on the heap.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/memory/memory.c
 * - engine/src/vm/control.c
 * - engine/src/statements/core/return.c
 * Prerequisite Header Files:
 * - engine/include/statements/core/gosub.h
 * - engine/include/vm/vm.h
 * - engine/include/memory/memory.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "statements/core/gosub.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "memory/memory.h"
#include "runtime/metadata.h"
#include "platform/platform.h"
#include <string.h>

void stmt_gosub_register(void) {
    MicroLibMetadata meta = {
        .name = "GOSUB",
        .category = "Control Flow",
        .syntax = "GOSUB line_num",
        .help_text = "Pushes current line onto stack and branches execution to specified subroutine line number.",
        .error_codes = "Error 8: Undefined line number (subroutine target missing), Error 2: Syntax error (missing line number)"
    };
    microlib_register(&meta);
}

BppError stmt_gosub_handler(VMContext *vm, LexerContext *lex) {
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
            err.code = 38;
            err.message = "Cross-file GOSUB not supported";
            return err;
        }

        if (!vm_gosub_push(vm, vm_get_current_line(vm), lex_get_pos(lex))) {
            err.code = 12;
            err.message = "Subroutine nesting limit exceeded";
            return err;
        }

        vm_jump(vm, target_line, NULL);
        return err;
    }

    if (tok.type != TOK_NUMBER) {
        err.code = 2; /* Syntax error */
        err.message = "Expected line number after GOSUB";
        return err;
    }

    BppLineNumber target = tok.as.number;
    if (!mem_program_get(vm_get_mem(vm), target)) {
        err.code = 8; /* Undefined line number */
        err.message = "Undefined line number in GOSUB";
        return err;
    }

    BppLineNumber current = vm_get_current_line(vm);
    const char *resume_pos = lex_get_pos(lex);

    if (!vm_gosub_push(vm, current, resume_pos)) {
        err.code = 14; /* Out of memory / Stack overflow */
        err.message = "GOSUB stack overflow";
        return err;
    }

    vm_jump(vm, target, NULL);
    return err;
}
