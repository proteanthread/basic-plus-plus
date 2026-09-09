// FILENAME: semantic_harvester.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (semantic_harvester.c, stmt_rename.c, stmt_revert.c)
// NEEDS: libkernel (types.h, vm.h)
// Provides semantic context clue harvesting from PRINT/INPUT strings and REM comments,
// along with RAMbank snapshot and rollback facilities for legacy BASIC modernization.
//
// ---- Includes ----

#ifndef STATEMENTS_PROGRAM_MODERNIZE_SEMANTIC_HARVESTER_H
#define STATEMENTS_PROGRAM_MODERNIZE_SEMANTIC_HARVESTER_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"
#include "vm/vm.h"

// Casing styles for modern variable synthesis
typedef enum {
    MODERNIZE_CASE_MIXED = 0, // PascalCase / MixedCase: Bushels, AsciiChar (DEFAULT)
    MODERNIZE_CASE_SNAKE,     // snake_case: bushels, ascii_char
    MODERNIZE_CASE_CAMEL,     // camelCase: bushels, asciiChar
    MODERNIZE_CASE_SCREAMING, // SCREAMING_SNAKE_CASE: BUSHELS, ASCII_CHAR
    MODERNIZE_CASE_UPPER      // UPPERCASE: BUSHELS, ASCIICHAR
} ModernizeCaseStyle;

// Inferred variable metadata candidate
typedef struct {
    char old_name[64];
    char modern_name[64];
    int confidence; // 0 - 100 percentage
    const char *source_rule; // "Prompt String Adjacency", "REM Comment Affinity", etc.
    int line_number;
} SemanticInferenceCandidate;

// Context lexicon gathered during analysis
typedef struct {
    SemanticInferenceCandidate candidates[128];
    size_t candidate_count;
    ModernizeCaseStyle case_style;
} ContextLexicon;

// Harvests context clues across program lines and populates lexicon
void semantic_harvest_program(VMContext *vm, ContextLexicon *lexicon, ModernizeCaseStyle style);

// Infers modern variable name from surrounding prompt strings, comments, and AST expression affinity
bool semantic_infer_variable(VMContext *vm, const char *var_name, char *out_name, size_t out_cap, ModernizeCaseStyle style, int *out_confidence);

// Harvests DEFINT A-Z / range declarations across program lines
bool harvest_defint_map(VMContext *vm, bool out_defint_map[26]);

// Formats an identifier into the requested casing style
void semantic_apply_casing(const char *input_words, char *out_buf, size_t out_cap, ModernizeCaseStyle style);

// ---- Multi-Level RAMbank Snapshot and Rollback Shield ----

#define RAMBANK_MAX_SNAPSHOTS 8

// Pushes current program lines into in-memory RAMbank snapshot stack tagged by tool
int rambank_snapshot_push(VMContext *vm, const char *tool_tag);

// Pops and restores program lines from in-memory RAMbank snapshot stack
bool rambank_snapshot_pop(VMContext *vm, const char *tool_tag);

// Returns the number of snapshots available for the given tool_tag (or all if NULL)
int rambank_snapshot_depth(const char *tool_tag);

// Clears all snapshots in the RAMbank stack
void rambank_snapshot_clear_all(void);

// Legacy single-depth compatibility wrappers
bool modernize_snapshot_save(VMContext *vm);
bool modernize_snapshot_restore(VMContext *vm);
bool modernize_snapshot_available(void);
void modernize_snapshot_clear(void);

#endif // STATEMENTS_PROGRAM_MODERNIZE_SEMANTIC_HARVESTER_H
