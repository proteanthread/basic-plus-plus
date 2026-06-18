/*
 * ---
 * BASIC++ Interpreter - parser_block.h
 * ---
 *
 * Structured block parser for library source files.
 *
 * PURPOSE:
 * Compiles SUB/FUNCTION/DEF FN blocks from library source
 * files into PCode bytecode WITHOUT requiring line numbers.
 * This enables structured BASIC++ libraries to be loaded
 * and executed directly.
 *
 * HOW IT WORKS:
 * 1. Read source file line by line
 * 2. Identify block boundaries (SUB...END SUB, etc.)
 * 3. For each block, parse individual lines via the lexer
 * 4. Build AST via ast_build_line()
 * 5. Emit PCode via pcode_emit_stmt()
 * 6. Register symbols with entry point offsets
 *
 * VIRTUAL LINE NUMBERS:
 * Since library source has no line numbers, the parser
 * assigns virtual line numbers (starting at 50000) for
 * error reporting and the line map. These never conflict
 * with user programs because libraries run in a separate
 * program space.
 *
 * SUPPORTED BLOCKS:
 *   SUB name(params...)      ...  END SUB
 *   FUNCTION name(params...) ...  END FUNCTION
 *   DEF FN name(param) = expr
 *
 * Lines outside of blocks (module-level code) are compiled
 * as initialization code that runs once when the library
 * is loaded.
 *
 * COMPATIBILITY:
 * Libraries may optionally contain line-numbered code.
 * If a line begins with a number, the parser uses it as
 * the line number; otherwise a virtual number is assigned.
 *
 * C89/C90 COMPLIANT.
 *
 * ---
 */

#ifndef BASICPP_PARSER_BLOCK_H
#define BASICPP_PARSER_BLOCK_H

#include "lib_space.h"

/* --- Virtual line number base ---
 * Library lines get virtual line numbers starting here.
 * Well beyond any user program range.
 */
#define LIB_VLINE_BASE  50000

/* --- Block Parser Result --- */
typedef struct BlockParseResult {
    int  symbols_found;     /* number of SUB/FUNCTION/DEF FN */
    int  instructions;      /* total PCode instructions emitted */
    int  errors;            /* number of parse errors */
    int  lines_parsed;      /* total source lines processed */
} BlockParseResult;

/* ===================================================================
 * BLOCK PARSER API
 * ===================================================================
 */

/*
 * parser_block_compile_file - Compile a library source file.
 *
 * Opens the file, parses all SUB/FUNCTION/DEF FN blocks,
 * compiles each to PCode bytecode, and populates the
 * LoadedLibrary's pcode program and symbol table.
 *
 * Parameters:
 *   path   - source file path
 *   lib    - LoadedLibrary slot to populate (symbols already
 *            scanned by lib_space_load_source in pass 1)
 *   result - (out) parse statistics
 *
 * Returns 0 on success, -1 on error.
 */
int parser_block_compile_file(const char *path,
                              LoadedLibrary *lib,
                              BlockParseResult *result);

/*
 * parser_block_compile_line - Compile a single source line.
 *
 * Utility: compiles one line of BASIC source into the
 * library's PCode program. Used internally by the file
 * compiler and available for testing.
 *
 * Parameters:
 *   line     - source line text (may or may not have line number)
 *   vline    - virtual line number (used if line has none)
 *   lib      - target library
 *
 * Returns 0 on success, -1 on parse error.
 */
int parser_block_compile_line(const char *line, int vline,
                              LoadedLibrary *lib);

#endif /* BASICPP_PARSER_BLOCK_H */
