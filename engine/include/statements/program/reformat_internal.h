// FILENAME: reformat_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (reformat_analyze.c, reformat_engine.c)
// NEEDED BY: libengine (reformat_indent.c, reformat_report.c)
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, reformat.h, reformat.c, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the REFORMAT_INTERNAL statement in BASIC++.
//
// ---- Includes ----

#ifndef REFORMAT_INTERNAL_H
#define REFORMAT_INTERNAL_H

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device/vdev.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include "statements/program/reformat.h"
#include "vm/vm.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

//
// ---- Internal Constants and Structures ----

#define MAX_BLOCK_RANGES 128
#define MAX_LINE_TOKENS 32

typedef struct {
    ReformatBlockType type;
    double            start_line;
    double            end_line;
} BlockRange;

typedef struct {
    BppKeywordId kw;
    BppTokenType type;
    char         text[64];
} FastToken;

//
// ---- Internal Helper Functions ----

static inline const char *get_block_name(ReformatBlockType type) {
    switch (type) {
        case BLOCK_FOR:      return "FOR";
        case BLOCK_WHILE:    return "WHILE";
        case BLOCK_DO:       return "DO";
        case BLOCK_IF:       return "IF";
        case BLOCK_SELECT:   return "SELECT";
        case BLOCK_SUB:      return "SUB";
        case BLOCK_FUNCTION: return "FUNCTION";
        case BLOCK_TRY:      return "TRY";
        case BLOCK_TYPE:     return "TYPE";
        case BLOCK_CLASS:    return "CLASS";
        case BLOCK_ATOMIC:   return "ATOMIC";
        case BLOCK_WITH:     return "WITH";
        default:             return "BLOCK";
    }
}

static inline const char *get_expected_closer(ReformatBlockType type) {
    switch (type) {
        case BLOCK_FOR:      return "NEXT";
        case BLOCK_WHILE:    return "WEND";
        case BLOCK_DO:       return "LOOP";
        case BLOCK_IF:       return "END IF";
        case BLOCK_SELECT:   return "END SELECT";
        case BLOCK_SUB:      return "END SUB";
        case BLOCK_FUNCTION: return "END FUNCTION";
        case BLOCK_TRY:      return "END TRY";
        case BLOCK_TYPE:     return "END TYPE";
        case BLOCK_CLASS:    return "END CLASS";
        case BLOCK_ATOMIC:   return "END ATOMIC";
        case BLOCK_WITH:     return "END WITH";
        default:             return "END";
    }
}

static inline const char *skip_leading_ws(const char *str) {
    if (!str) return "";
    while (*str && isspace((unsigned char)*str)) str++;
    return str;
}

static inline void add_diagnostic(ReformatPlan *plan, DiagSeverity sev, double line,
                                  const char *what, const char *why, const char *how) {
    if (!plan || plan->diag_count >= REFORMAT_MAX_DIAGNOSTICS) return;
    ReformatDiagnostic *d = &plan->diagnostics[plan->diag_count++];
    d->severity = sev;
    d->line = line;
    snprintf(d->what, sizeof(d->what), "%s", what ? what : "");
    snprintf(d->why,  sizeof(d->why),  "%s", why  ? why  : "");
    snprintf(d->how,  sizeof(d->how),  "%s", how  ? how  : "");

    if (sev == DIAG_ERROR) {
        plan->error_count++;
    } else {
        plan->warning_count++;
    }
}

static inline void add_suggestion(ReformatPlan *plan, double line, const char *text, const char *reason) {
    if (!plan || plan->suggestion_count >= REFORMAT_MAX_SUGGESTIONS) return;
    ReformatSuggestion *s = &plan->suggestions[plan->suggestion_count++];
    s->line = line;
    snprintf(s->text, sizeof(s->text), "%s", text ? text : "");
    snprintf(s->reason, sizeof(s->reason), "%s", reason ? reason : "");
}

int tokenize_line_fast(const char *text, FastToken *tokens, int max_tokens);
bool is_label_line(const FastToken *tokens, int count);
bool is_block_if(const FastToken *tokens, int count);
ReformatBlockType get_end_block_type(const FastToken *tokens, int count, int end_idx);

#endif // REFORMAT_INTERNAL_H
