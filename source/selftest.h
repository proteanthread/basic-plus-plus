/*
 * ---
 * BASIC++ Interpreter - selftest.h
 * ---
 *
 * Self-test framework.
 *
 * Provides a built-in validation suite that the interpreter can
 * run on itself - "the interpreter tests itself."
 *
 * ---
 */

#ifndef BASICPP_SELFTEST_H
#define BASICPP_SELFTEST_H

#include "runtime.h"

/*
 * selftest_run - Execute the built-in self-test suite.
 *
 * Loads and runs a hardcoded BASIC++ program that exercises
 * core interpreter features: arithmetic, comparisons, strings,
 * flow control, functions, and more.
 *
 * Returns 0 if all tests pass, -1 if any fail.
 */
int selftest_run(RuntimeState *rt);

#endif /* BASICPP_SELFTEST_H */
