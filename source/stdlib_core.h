/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: stdlib_core.h
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

#ifndef STDLIB_CORE_H
#define STDLIB_CORE_H

#include "value.h"

 // BASIC++ Core Immutable API
 // These functions are guaranteed across all dialects and 
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // interact directly with the VM memory pool.

void stdlib_core_register(void);

BValue stdlib_core_len(BValue *args, int argc, void *rt);
BValue stdlib_core_chr(BValue *args, int argc, void *rt);
BValue stdlib_core_str(BValue *args, int argc, void *rt);
BValue stdlib_core_rnd(BValue *args, int argc, void *rt);

// MS-BASIC Common Math Functions
BValue stdlib_core_abs(BValue *args, int argc, void *rt);
BValue stdlib_core_sgn(BValue *args, int argc, void *rt);
BValue stdlib_core_int(BValue *args, int argc, void *rt);
BValue stdlib_core_sqr(BValue *args, int argc, void *rt);
BValue stdlib_core_sin(BValue *args, int argc, void *rt);
BValue stdlib_core_cos(BValue *args, int argc, void *rt);
BValue stdlib_core_tan(BValue *args, int argc, void *rt);
BValue stdlib_core_atn(BValue *args, int argc, void *rt);
BValue stdlib_core_log(BValue *args, int argc, void *rt);
BValue stdlib_core_exp(BValue *args, int argc, void *rt);

// MS-BASIC Common String Functions
BValue stdlib_core_left(BValue *args, int argc, void *rt);
BValue stdlib_core_right(BValue *args, int argc, void *rt);
BValue stdlib_core_mid(BValue *args, int argc, void *rt);
BValue stdlib_core_instr(BValue *args, int argc, void *rt);
BValue stdlib_core_err_str(BValue *args, int argc, void *rt);

#endif
