/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: trans.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Common transpiler interface for BASIC++ AST to target languages.
 * ===================================================================== */

#ifndef BASICPP_TRANS_H
#define BASICPP_TRANS_H

#include <stdio.h>
#include "../ast.h"
#include "../memory.h"
#include "target.h"

typedef enum TransTarget {
    TRANS_TARGET_C17,
    TRANS_TARGET_C89,
    TRANS_TARGET_PY3,
    TRANS_TARGET_PAS,
    TRANS_TARGET_F90
} TransTarget;

// Master dispatch function
int trans_emit(FILE *out, AstLine *lines, int line_count,
               ProgramStore *program, const TargetConfig *target, TransTarget lang);

// Backend specific generators
int trans_emit_c17(FILE *out, AstLine *lines, int line_count, ProgramStore *program, const TargetConfig *target);
int trans_emit_c89(FILE *out, AstLine *lines, int line_count, ProgramStore *program, const TargetConfig *target);
int trans_emit_py3(FILE *out, AstLine *lines, int line_count, ProgramStore *program, const TargetConfig *target);
int trans_emit_pas(FILE *out, AstLine *lines, int line_count, ProgramStore *program, const TargetConfig *target);
int trans_emit_f90(FILE *out, AstLine *lines, int line_count, ProgramStore *program, const TargetConfig *target);

#endif // BASICPP_TRANS_H
