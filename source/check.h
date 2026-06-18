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
