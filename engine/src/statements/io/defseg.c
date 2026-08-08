/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file defseg.c
 * @brief DEF SEG [= address] segmented memory base address definition handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements DEF SEG [= address] statement for configuring the 16-bit base segment register used by PEEK, POKE, CALL, BLOAD, and BSAVE commands.
 *
 * 2. WHY IT EXISTS:
 * Provides backward compatibility with GW-BASIC/QBASIC 8086 memory segmentation models.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Evaluates segment integer expression (0 to 65535 or omitted). If omitted, resets segment base address to the VM default BASIC data segment (DS). Stores value in `vm->current_segment`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_defseg' (libbasicpp ONLY). Includes "stmt/stmt.h",
 * "vm/vm.h", "eval/eval.h", "memory/segmented_mem.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * EXCLUDED from libbasicpp_lite (bpp, bs) per Rule #1 (Segmented memory subsystem `vmem`). FULLY included in libbasicpp (baspp standard desktop).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support 32-bit linear address overrides (DEF SEG32) when extending virtual memory subsystems.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Default segment invariant: DEF SEG with no arguments MUST reset segment to default VM DS segment base.
 *
 * 8. WHAT TO EXPECT:
 * Updates `vm->current_segment` field and returns ERR_NONE or ERR_OVERFLOW.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Inspect segment base setting in memory/segmented_mem.c.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 16-bit uint16_t segment address range bounds checking.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/memory/segmented_mem.c
 * - engine/src/eval/eval.c
 * Prerequisite Header Files:
 * - engine/include/memory/segmented_mem.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "stmt/stmt.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "memory/segmented_mem.h"
#include <string.h>

BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Check for '=' (optional in some implementations, but standard GW-BASIC requires it or just space) */
    if (lex_peek(lex).type == TOK_EQ) {
        lex_next(lex);
    }
    
    /* Evaluate address */
    BValue addr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (addr_val.type == VAL_STRING) {
        err.code = 13; err.message = "DEF SEG address must be numeric";
        return err;
    }
    
    uint16_t seg = (uint16_t)addr_val.as.number;
    vmem_set_def_seg(vm_get_vmem(vm), seg);
    
    return err;
}
