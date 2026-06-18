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
