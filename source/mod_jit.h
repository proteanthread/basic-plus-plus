/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_jit.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Utility or helper code for BASIC++ interpreter.
 *
 * 2. WHAT TO EXPECT:
 *    Executes with low overhead, relying on fixed compile-time limits and memory pools.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Internal helper functions, optimization passes, or local naming adjustments.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Public API structures, parameter contracts, or global type definitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check memory pool margins, look for segmentation faults, and trace parameter values via a debugger.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - mod_jit.h
 // ---
 //
 // Optional JIT (Just-In-Time) compilation module.
 //
 // PLATFORM SUPPORT:
 //   Windows 11  - VirtualAlloc + PAGE_EXECUTE_READWRITE
 //   Linux       - mmap + PROT_READ|PROT_WRITE|PROT_EXEC
 //   FreeDOS     - NOT AVAILABLE (excluded from BPP_FREEDOS builds)
 //
 // USAGE:
 //   MODULE "JIT"        ' activate the JIT engine
 //   BRUN                ' compile + JIT execute (if JIT active)
 //
 // DESIGN:
 //   The JIT module registers itself via the module system.
 //   When active, exec_brun() checks for JIT availability and
 //   delegates to jit_compile_and_run() instead of vm_exec_pcode().
 //
 //   The JIT compiles PCode bytecode to native x86-64 machine code
 //   in an executable memory region. Unsupported opcodes fall back
 //   to the portable VM interpreter.
 //
 // SCOPE:
 //   Core-only initially: arithmetic, variable load/store,
 //   comparisons, jumps, PRINT. Extensible to full coverage later.
 //
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // ---

#ifndef BASICPP_MOD_JIT_H
#define BASICPP_MOD_JIT_H

// JIT is only available on Windows and Linux x86-64
#if (defined(_WIN32) || defined(__linux__)) && !defined(BPP_FREEDOS)
#define BPP_HAS_JIT 1
#else
#define BPP_HAS_JIT 0
#endif

#include "pcode.h"

 // mod_jit_register - Register the JIT module with the module system.
 //
 // Called during boot on platforms that support JIT.
 // The module starts INACTIVE until the user issues MODULE "JIT".
void mod_jit_register(void);

 // jit_is_active - Check if the JIT engine is active.
 //
 // Returns 1 if MODULE "JIT" has been activated, 0 otherwise.
int jit_is_active(void);

 // jit_compile_and_run - JIT compile PCode and execute natively.
 //
 // Translates PCode bytecode to native x86-64 machine code,
 // allocates executable memory, and runs it.
 //
 // Parameters:
 //   rt    - RuntimeState pointer (opaque void* for C89)
 //   pcode - compiled PCode program
 //
 // Returns 0 on success, -1 on error.
 // Falls back to vm_exec_pcode() for unsupported opcodes.
int jit_compile_and_run(void *rt, PCodeProgram *pcode);

#endif // BASICPP_MOD_JIT_H
