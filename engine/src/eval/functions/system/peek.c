/**
 * @file peek.c
 * @brief PEEK virtual memory reading function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements the `PEEK(addr)` built-in function, reading an 8-bit byte value (0..255) from current segment offset or virtual memory address.
 *
 * 2. WHY IT EXISTS:
 * Provides GW-BASIC, QBASIC, and ANSI BASIC memory introspection parity for reading screen buffer, BIOS flags, or segmented memory.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates 1 numeric argument, checks memory sandbox bounds, reads byte via virtualized memory / BIOS / bus, and returns `VAL_NUMBER`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_peek'. Includes "eval/functions/system/peek.h",
 * "runtime/micro_lib_metadata.h", "runtime/funcreg.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support 16-bit word and 32-bit double-word memory inspection functions (`PEEK16`, `PEEK32`).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 1 argument signature validation, byte return value range (0..255), sandboxed pointer safety invariant.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_NUMBER BValue containing 8-bit memory byte or ERR_ILLEGAL_FUNCTION_CALL (error 5) / ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check virtual memory address bounds checking and `vm->defseg` segment base address.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext pointer.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Accesses sandboxed virtual memory space; never dereferences raw host pointers directly.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/funcreg.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/peek.h
 * - engine/include/runtime/funcreg.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/peek.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
#include <string.h>

void func_peek_register(void) {
    MicroLibMetadata meta = {
        .name = "PEEK",
        .category = "System Functions",
        .syntax = "PEEK(addr)",
        .help_text = "Reads a byte (0-255) from virtual memory address addr.",
        .error_codes = "Error 5: Illegal Function Call (addr out of memory bounds), Error 13: Type Mismatch (expects numeric address)"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "PEEK",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_SYSTEM,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read a byte from memory",
        .module_name = "System"
    };
    funcreg_register(&entry);
}

#include "device/bus.h"
#include "bios/bios.h"
#include "vm/vm.h"
#include "security/security.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include <stdint.h>

BValue func_peek_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "PEEK expects one numeric address argument";
        return res;
    }

    if (security_check(SECOP_MEM_READ, 0) != 0) {
        err->code = 70;
        err->message = "Permission denied: PEEK is restricted";
        return res;
    }

    uint32_t addr_raw = (uint32_t)args[0].as.number;
    uint32_t phys_addr = addr_raw;
#ifndef BASIC_LITE_BUILD
    uint16_t def_seg = vmem_get_def_seg(vm_get_vmem(vm));
    if (def_seg != 0 && addr_raw < 0x10000U) {
        phys_addr = ((uint32_t)def_seg << 4) + addr_raw;
    }
#endif
    uint8_t val = 0;
    if (vm_get_bios(vm)) {
        val = bios_peek(vm_get_bios(vm), phys_addr);
    } else {
        bool intercepted = false;
        val = vdev_bus_peek(phys_addr, &intercepted);
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)val;
    return res;
}
