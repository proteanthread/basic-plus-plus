/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: codegen_core.h
 * Subsystem: Standalone C89 Transpiler Code Generator
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Renders C89 transpiled shims, header boilerplates, and runtime structures.
 *
 * 2. WHAT TO EXPECT:
 *    Outputs boilerplate compiler templates for embedded or native targets.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Shim header libraries, static array sizes.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Basic transpilation shims contracts.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure output stream is valid and writable.
 * ===================================================================== */

#ifndef STANDALONE_CODEGEN_CORE_H
#define STANDALONE_CODEGEN_CORE_H

#include <stdio.h>

typedef struct CodegenTarget {
    const char *name;
    const char *target_macro;
    int has_multitasking;
    int has_graphics;
    int has_filesystem;
    long default_array_limit;
} CodegenTarget;

void codegen_core_emit_header(FILE *out, const CodegenTarget *target);
void codegen_core_emit_footer(FILE *out);

#endif /* STANDALONE_CODEGEN_CORE_H */
