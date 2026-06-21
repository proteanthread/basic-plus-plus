/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: compiler.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Stored program editor commands (RENUM, DELETE), compiler driver pipeline (BASIC-to-C), and bytecode serializers.
 *
 * 2. WHAT TO EXPECT:
 *    Code generator constructs self-contained C89 files. Bytecode serializes code to files.
 *
 * 3. WHAT CAN BE CHANGED:
 *    C89 codegen shims, editor warnings, target language mapping layout.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    AST translation loops, bytecode file format specs.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Verify target C compiler settings. If transpiled C file has compilation warnings, check codegen expressions mapping.
 * ===================================================================== */

 // ---
 // BASIC++ Compiler - compiler.h
 // ---
 //
 // Compiler driver interface.
 //
 // PURPOSE:
 // Orchestrates the compilation pipeline: reads stored program
 // lines, builds ASTs, and invokes the code generator to produce
 // a C89 source file.
 //
 // ---

#ifndef BASICPP_COMPILER_H
#define BASICPP_COMPILER_H

#include "memory.h"

 // compiler_compile - Compile the stored program to a C source file.
 //
 // Parameters:
 // program - the program store containing stored BASIC lines
 // filename - output filename (e.g., "output.c")
 //
 // Returns 0 on success, -1 on error.
 // Prints diagnostic messages to stdout on success, error messages
 // on failure.
int compiler_compile(ProgramStore *program, const char *filename, const char *target);

#endif // BASICPP_COMPILER_H
