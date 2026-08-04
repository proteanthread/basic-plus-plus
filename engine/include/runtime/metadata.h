/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file metadata.h
 * @brief Metadata registry defining schemas for namespace resolution, cross-file global labels, and queryable docstrings.
 *
 * WHAT IT DOES:
 * This header declares structures and functions for tracking module namespaces, registering and resolving
 * cross-file global labels, and storing docstring documentation associated with program entities.
 *
 * WHY IT EXISTS:
 * In BASIC++ v6.0.0, we introduced advanced modularity features. This module bridges namespace scoping,
 * cross-file execution jumps, and the HELP introspection command to make BASIC++ feel modern and robust.
 *
 * WHY IT WORKS THIS WAY:
 * Registries use fixed-size static allocations to remain compatible with FreeDOS memory constraints
 * and avoid C heap fragmentation. All string keys are stored in inline character arrays to maximize memory locality.
 *
 * WHAT CAN BE CHANGED:
 * Limits like MAX_GLOBAL_LABELS, MAX_DOCSTRINGS, or string length bounds can be tuned if memory permits.
 *
 * WHAT CANNOT BE CHANGED:
 * The memory layout must remain packing-safe and compile cleanly under C17. Struct sizes must stay bounded.
 *
 * WHAT TO EXPECT:
 * Fast, O(N) array scans for labels and docstring keys. Constant-time namespace active context swaps.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Check for string overflow in key buffers. Ensure metadata initialization is called during boot setup.
 *
 * ASSUMPTIONS:
 * String inputs are ASCII. Pre-scanned files populate metadata before execution steps begin.
 *
 * PORTABILITY CONCERNS:
 * Standard C17 compliant, fully safe for 16-bit, 32-bit, and 64-bit targets.
 *
 * FUTURE EXPANSIONS:
 * Can support hierarchical namespaces (e.g. ::[MATH.STATS]) or docstring classifications.
 *
 * EXTERNAL EXTENSION HOOKS:
 * Dynamic modules (.BPL/.BPE) can insert their own export names and docstrings into this registry.
 */

#ifndef RUNTIME_METADATA_H
#define RUNTIME_METADATA_H

#include <stdbool.h>
#include <stddef.h>

#include "types/config.h"
#include "lexer/lexer.h"

typedef struct VMContext VMContext;

#define MAX_GLOBAL_LABELS 256
#define MAX_DOCSTRINGS 512

/* Cross-file global label entry mapping label names to filenames and lines */
typedef struct {
    char label_name[64];
    char filename[256];
    BppLineNumber line_number;
} BppGlobalLabel;

/* Introspection docstring mapping target names to documentation text */
typedef struct {
    char target_name[128]; /* e.g. "MATH.ADD" or "::MY_LABEL" */
    char docstring[256];
} BppDocstring;

#ifdef BASIC_FREEDOS_16
  #define MAX_METADATA_BLOCKS 8
  #define MAX_BLOCK_BODY_LEN 256
#else
  #define MAX_METADATA_BLOCKS 64
  #define MAX_BLOCK_BODY_LEN 1024
#endif

/* Stateful metadata block structure for KEYWORD, SCOPE, ALIAS blocks */
typedef struct {
    char block_type[32];   /* "KEYWORD", "SCOPE", "ALIAS" */
    char target_name[64];  /* The target of the block, e.g. "PRINT" */
    char docstring[256];   /* The docstring of the block */
    char body[MAX_BLOCK_BODY_LEN]; /* Concatenated line text of the block body */
} BppMetadataBlock;

/* Registry context holding all metadata */
typedef struct {
    BppGlobalLabel global_labels[MAX_GLOBAL_LABELS];
    int global_label_count;

    BppDocstring docstrings[MAX_DOCSTRINGS];
    int docstring_count;

    BppMetadataBlock metadata_blocks[MAX_METADATA_BLOCKS];
    int metadata_block_count;

    char current_namespace[64];      /* Active namespace during loading/parsing */
    char last_docstring_buffer[256]; /* Holds the last parsed docstring temporarily */
    bool option_strict;              /* Strict variable and type checks enabled */
} BppMetadataRegistry;

/**
 * @brief Initialize the metadata registry.
 */
void metadata_init(BppMetadataRegistry *reg);

/**
 * @brief Register a global label mapping to a file and line.
 */
bool metadata_register_label(BppMetadataRegistry *reg, const char *label, const char *filename, BppLineNumber line);

/**
 * @brief Resolve a global label to its file and line location.
 */
bool metadata_resolve_label(const BppMetadataRegistry *reg, const char *label, char *out_filename, size_t max_len, BppLineNumber *out_line);

/**
 * @brief Register a docstring for a target identifier or namespace.
 */
bool metadata_register_docstring(BppMetadataRegistry *reg, const char *target, const char *docstring);

/**
 * @brief Retrieve a registered docstring for a target.
 */
const char *metadata_get_docstring(const BppMetadataRegistry *reg, const char *target);

/**
 * @brief Register a metadata block (KEYWORD, SCOPE, ALIAS).
 */
bool metadata_register_block(BppMetadataRegistry *reg, const char *type, const char *target, const char *docstring, const char *body);

/**
 * @brief Retrieve a metadata block by its type and target.
 */
const BppMetadataBlock *metadata_get_block(const BppMetadataRegistry *reg, const char *type, const char *target);

void metadata_pre_scan_line(VMContext *vm, const char *filename, BppLineNumber line_num, const char *text);

/**
 * @brief Performs a full pre-scan pass on all stored program memory lines.
 */
void metadata_pre_scan_program(VMContext *vm, const char *filename);

#endif /* RUNTIME_METADATA_H */
