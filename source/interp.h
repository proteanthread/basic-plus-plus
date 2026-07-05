/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Interface definitions for the statement execution engine and core loop.
 *    - Statement dispatcher registry and execution stack descriptors.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#ifndef INTERP_H
#define INTERP_H

#include "legacy_compat.h"

// Execute single statement from state->ip
void gw_exec_statement(GW_State *state);

// Run loaded program from start
void gw_run_program(GW_State *state);

// Clean program lines
void gw_program_clear(GW_State *state);

// Insert or delete program line
void gw_program_insert(GW_State *state, int32_t line_num, const uint8_t *tokens, size_t len);

#endif // INTERP_H
