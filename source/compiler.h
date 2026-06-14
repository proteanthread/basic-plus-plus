/*
 * ---
 * BASIC++ Compiler - compiler.h
 * ---
 *
 * Compiler driver interface.
 *
 * PURPOSE:
 * Orchestrates the compilation pipeline: reads stored program
 * lines, builds ASTs, and invokes the code generator to produce
 * a C89 source file.
 *
 * ---
 */

#ifndef BASICPP_COMPILER_H
#define BASICPP_COMPILER_H

#include "memory.h"

/*
 * compiler_compile - Compile the stored program to a C source file.
 *
 * Parameters:
 * program - the program store containing stored BASIC lines
 * filename - output filename (e.g., "output.c")
 *
 * Returns 0 on success, -1 on error.
 * Prints diagnostic messages to stdout on success, error messages
 * on failure.
 */
int compiler_compile(ProgramStore *program, const char *filename, const char *target);

#endif /* BASICPP_COMPILER_H */
