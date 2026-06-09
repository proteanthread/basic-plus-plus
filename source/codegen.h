/*
 * =====================================================================
 * BASIC++ Compiler - codegen.h
 * =====================================================================
 *
 * Code generator interface.
 *
 * PURPOSE:
 * Walks an array of AstLine structures (one per BASIC line) and
 * emits a complete, self-contained ANSI C89 source file.
 *
 * =====================================================================
 */

#ifndef BASICPP_CODEGEN_H
#define BASICPP_CODEGEN_H

#include <stdio.h>
#include "ast.h"
#include "memory.h"

/*
 * codegen_emit - Generate a complete C89 program from AST lines.
 *
 * Parameters:
 * out - output file (already opened for writing)
 * lines - array of AstLine (line_number + statement chain)
 * line_count - number of lines
 * program - raw program store (for DATA extraction)
 *
 * Returns 0 on success, -1 on error.
 */
int codegen_emit(FILE *out, AstLine *lines, int line_count,
 ProgramStore *program);

#endif /* BASICPP_CODEGEN_H */
