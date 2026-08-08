/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file reformat.h
 * @brief Public interface header for REFORMAT statement handler and multi-pass formatting engine.
 *
 * 1. WHAT IT DOES:
 * Declares data structures, indentation rules, analysis diagnostic types, and function prototypes
 * for the REFORMAT statement handler and formatting engine.
 *
 * 2. WHY IT EXISTS:
 * Replaces the legacy v5 REFORMAT command with a full-featured, multi-pass, table-driven code
 * formatter compliant with modern BASIC++ v6 architecture and classic BASIC guidelines.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Uses a push/pop stack state machine alongside a table-driven IndentRule structure to track
 * block nesting (FOR/NEXT, WHILE/WEND, DO/LOOP, IF/END IF, SELECT/CASE, SUB/FUNCTION, TRY/CATCH,
 * TYPE, CLASS, ATOMIC, WITH) and compute clean indentation levels.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_reformat'. Includes "types/types.h", "vm/vm.h",
 * "lexer/lexer.h", <stdint.h>, <stdbool.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in desktop ('baspp') and REPL ('bpp') editions.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add new block keyword types to ReformatBlockType enum or tune REFORMAT_MAX_NESTING limits.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Standard BppError (*)(VMContext*, LexerContext*) statement handler signature.
 *
 * 8. WHAT TO EXPECT:
 * Exposes stmt_reformat_handler(), stmt_reformat_register(), and pass-execution prototypes.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guards and ensure all dependent types in types/types.h are satisfied.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Initialized VMContext with active memory context and valid source program lines.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Fixed array sizes for embedded / cross-platform memory safety.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/program/reformat.c
 * - engine/src/statements/program/reformat_engine.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_PROGRAM_REFORMAT_H
#define STATEMENTS_PROGRAM_REFORMAT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdint.h>
#include <stdbool.h>

#define REFORMAT_MAX_NESTING     64
#define REFORMAT_MAX_DIAGNOSTICS 256

typedef enum {
    BLOCK_NONE = -1,
    BLOCK_FOR = 0,    /* FOR ... NEXT */
    BLOCK_WHILE,      /* WHILE ... WEND */
    BLOCK_DO,         /* DO ... LOOP */
    BLOCK_IF,         /* IF ... END IF */
    BLOCK_SELECT,     /* SELECT CASE ... END SELECT */
    BLOCK_SUB,        /* SUB ... END SUB */
    BLOCK_FUNCTION,   /* FUNCTION ... END FUNCTION */
    BLOCK_TRY,        /* TRY ... END TRY */
    BLOCK_TYPE,       /* TYPE ... END TYPE */
    BLOCK_CLASS,      /* CLASS ... END CLASS */
    BLOCK_ATOMIC,     /* ATOMIC ... END ATOMIC */
    BLOCK_WITH,       /* WITH ... END WITH */
    BLOCK_COUNT
} ReformatBlockType;

typedef struct {
    BppKeywordId      keyword;
    int8_t            pre_adjust;    /* Applied BEFORE printing this line */
    int8_t            post_adjust;   /* Applied AFTER printing this line */
    bool              is_bounce;     /* ELSE/CASE/CATCH: dedent self, indent children */
    ReformatBlockType push_block;    /* Block type to push (-1 = don't push) */
    ReformatBlockType pop_block;     /* Block type to pop (-1 = don't pop) */
} IndentRule;

typedef enum {
    DIAG_WARNING = 0, /* Suspicious code — blocks STRICT */
    DIAG_ERROR        /* Structural errors — blocks STRICT */
} DiagSeverity;

typedef struct {
    DiagSeverity  severity;
    double        line;          /* Line number (double internally per Rule #10) */
    char          what[256];     /* What the problem is */
    char          why[256];      /* Why it matters */
    char          how[256];      /* How to fix it */
} ReformatDiagnostic;

typedef struct {
    ReformatBlockType type;
    double            line_opened;
} ReformatBlockEntry;

typedef struct {
    ReformatBlockEntry entries[REFORMAT_MAX_NESTING];
    int                depth;
} ReformatBlockStack;

typedef struct {
    double line;
    char   text[256];
    char   reason[256];
} ReformatSuggestion;

#define REFORMAT_MAX_SUGGESTIONS 64

typedef enum {
    MOD_NONE = 0,
    MOD_CHECK,
    MOD_STRICT,
    MOD_SPLIT,
    MOD_UPPER,
    MOD_LOWER,
    MOD_PRESERVE,
    MOD_SPACES
} ReformatModifier;

typedef struct {
    ReformatDiagnostic diagnostics[REFORMAT_MAX_DIAGNOSTICS];
    int                diag_count;
    int                error_count;
    int                warning_count;
    ReformatSuggestion suggestions[REFORMAT_MAX_SUGGESTIONS];
    int                suggestion_count;
    int                total_lines;
    int                changed_lines;
    int                unchanged_lines;
    int                spaces_per_indent;
    double             target_start;
    double             target_end;
    bool               has_filename;
    char               filename[256];
} ReformatPlan;

/**
 * @brief REFORMAT statement handler for VM dispatch.
 */
BppError stmt_reformat_handler(VMContext *vm, LexerContext *lex);

/**
 * @brief Self-registration function for REFORMAT micro-library metadata.
 */
void stmt_reformat_register(void);

/**
 * @brief Initialize a ReformatPlan context with default spacing.
 */
void reformat_plan_init(ReformatPlan *plan, int spaces);

/**
 * @brief Pass 1 Analysis: Validates block structure, detects errors/warnings, and plans formatting.
 */
void reformat_pass1_analyze(VMContext *vm, ReformatPlan *plan);

/**
 * @brief Pass 2 Structural: Inserts blank REM separator lines before SUB/FUNCTION definitions.
 */
void reformat_pass2_blank_lines(VMContext *vm, ReformatPlan *plan);

/**
 * @brief SPLIT Pass: Selectively expands compound statements into individual numbered lines.
 */
void reformat_pass_split(VMContext *vm, ReformatPlan *plan);

/**
 * @brief Pass 3 Indentation: Applies leading whitespace formatting and keyword/operator normalization to program lines.
 */
BppError reformat_pass3_indent(VMContext *vm, const ReformatPlan *plan, ReformatModifier mod);

/**
 * @brief Render the 3-section REFORMAT CHECK report to console and .CHK file.
 */
void reformat_render_check_report(VMContext *vm, const ReformatPlan *plan, bool save_requested);

/**
 * @brief Render suggestions collected for non-split compound lines.
 */
void reformat_render_suggestions_summary(VMContext *vm, const ReformatPlan *plan);

/**
 * @brief Find start and end line numbers for a named SUB or FUNCTION procedure.
 */
bool reformat_find_sub_range(VMContext *vm, const char *sub_name, double *out_start, double *out_end);

#endif /* STATEMENTS_PROGRAM_REFORMAT_H */
