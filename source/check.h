/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: check.h
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
// BASIC++ Interpreter - check.h
// ---
//
// Static program analyzer.
//
// CHECK examines the user's program in memory and reports
// errors, warnings, and informational messages without
// executing the program.
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

#ifndef BASICPP_CHECK_H
#define BASICPP_CHECK_H

#include "runtime.h"

// check_program - Analyze the loaded program.
//
// Performs multi-pass static analysis on rt->program:
//   Pass 1: GOTO/GOSUB target validation
//   Pass 2: FOR/NEXT matching
//   Pass 3: WHILE/WEND matching
//   Pass 4: Variable usage analysis
//   Pass 5: DATA/READ balance
//   Pass 6: DEF FN analysis
//   Pass 7: Reachability & structure
//
// verbose: if nonzero, also prints INFO-level messages.
//
// Returns the number of errors found (0 = clean).
//
int check_program(RuntimeState *rt, int verbose);

#endif // BASICPP_CHECK_H
