/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: selftest.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Shell command execution, debugging tools (TRON/TROFF, breakpoints), self-test diagnostics, and static analysis verification.
 *
 * 2. WHAT TO EXPECT:
 *    Interactions with the host OS shell are capability-gated. Self-test runs isolated test codes.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Self-test test cases, diagnostic log prints, static analyzer syntax warnings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Breakpoints registry, self-test verification formulas.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If shell fails, check host environment permissions. If self-test fails, check recent syntax changes.
 * ===================================================================== */

// ---
// BASIC++ Interpreter - selftest.h
// ---
//
// Self-test framework.
//
// Provides a built-in validation suite that the interpreter can
// run on itself -- "the interpreter tests itself."
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

#ifndef BASICPP_SELFTEST_H
#define BASICPP_SELFTEST_H

#include "runtime.h"

// selftest_run - Execute the built-in self-test suite.
//
// Loads and runs a hardcoded BASIC++ program that exercises
// core interpreter features: arithmetic, comparisons, strings,
// flow control, functions, and more.
//
// Returns 0 if all tests pass, -1 if any fail.
//
int selftest_run(RuntimeState *rt);

#endif // BASICPP_SELFTEST_H
