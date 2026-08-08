/**
 * @file fre.c
 * @brief FRE / MEM free memory function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements FRE (queries free heap RAM in bytes or triggers GC) and MEM system functions.
 *
 * 2. WHY IT EXISTS:
 * Provides standard memory footprint querying matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates argument counts (0 or 1 for FRE, 0 for MEM), queries memory manager via `mem_get_free_ram(vm->mem_ctx)`, and returns `VAL_NUMBER`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_fre'. Includes "eval/functions/system/fre.h",
 * "runtime/micro_lib_metadata.h", "memory/memory.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support string compaction / garbage collection when FRE("-1$") is evaluated.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Argument validation rules and return type `VAL_NUMBER`.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_NUMBER BValue containing available heap bytes or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `vm->mem_ctx` initialization and memory manager response.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer `vm` with active memory context `vm->mem_ctx`.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 64-bit integer cast to double for large heap sizes.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/memory/memory.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/fre.h
 * - engine/include/memory/memory.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/fre.h"
#include "runtime/micro_lib_metadata.h"
#include "memory/memory.h"
#include <string.h>

void func_fre_register(void) {
    MicroLibMetadata meta = {
        .name = "FRE",
        .category = "System Functions",
        .syntax = "FRE(dummy)",
        .help_text = "Returns the number of available free memory bytes in the VM heap.",
        .error_codes = "Error 13: Type Mismatch (FRE expects 1 argument)"
    };
    microlib_register(&meta);
}

BValue func_fre_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "FRE") == 0) {
        if (arg_count > 1) {
            err->code = 13;
            err->message = "FRE expects 0 or 1 arguments";
            return res;
        }
    } else {
        if (arg_count != 0) {
            err->code = 13;
            err->message = "MEM expects no arguments";
            return res;
        }
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)mem_get_free_ram(vm_get_mem(vm));
    return res;
}
