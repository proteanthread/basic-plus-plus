/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: codegen.h
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

 // codegen.h -- BASIC-to-C transpiler
 //
 // Walks AstLine structures and emits self-contained C89 source.
 // The output compiles with any C89 compiler (gcc, cl, clang).
 //
 // To target a different language (Python, Pascal, etc.), write
 // a parallel emitter that walks the same AST. The node types
 // in ast.h are language-neutral: AST_IF, AST_FOR, AST_WHILE,
 // AST_GOSUB, AST_ASSIGN, AST_PRINT all have clear semantics.
 //
 // Python backend sketch: track indentation depth, emit "if cond:"
 // instead of "if (cond) {", use "for i in range()" for FOR/NEXT.
 // Main challenge is GOTO/GOSUB -- Python has no goto, so you'd
 // need a state machine with while/if dispatch. See the computed
 // goto approach in codegen.c for how the C backend handles this.
 //
 // Pascal: straightforward mapping. BEGIN/END for blocks, GOTO is
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // native, WRITELN for PRINT. Arrays need 1-based indexing.

#ifndef BASICPP_CODEGEN_H
#define BASICPP_CODEGEN_H

#include <stdio.h>
#include "ast.h"
#include "memory.h"

 // codegen_emit - Generate a complete C89 program from AST lines.
 //
 // Parameters:
 // out - output file (already opened for writing)
 // lines - array of AstLine (line_number + statement chain)
 // line_count - number of lines
 // program - raw program store (for DATA extraction)
 //
 // Returns 0 on success, -1 on error.
int codegen_emit(FILE *out, AstLine *lines, int line_count,
 ProgramStore *program);

#endif // BASICPP_CODEGEN_H
