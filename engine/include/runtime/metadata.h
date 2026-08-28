// FILENAME: metadata.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (error.c, metadata.c)
// NEEDED BY: libengine (context.c, control.c, data.c, events_internal.h)
// NEEDED BY: libengine (exec_dispatch.c, exec_internal.h, goodbye.c, gosub.c)
// NEEDED BY: libengine (goto.c, help.c, on_timer.c, restore.c, system.c, vm.h)
// NEEDED BY: libengine (vm_internal.h)
// NEEDS: libengine (lexer.h, lexer.c)
// NEEDS: libkernel (config.h)
// Provides core logic and interface definitions for metadata within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_METADATA_H
#define RUNTIME_METADATA_H

#include <stdbool.h>
#include <stddef.h>

#include "types/config.h"
#include "lexer/lexer.h"

typedef struct VMContext VMContext;

#define MAX_GLOBAL_LABELS 256
#define MAX_DOCSTRINGS 512

// Cross-file global label entry mapping label names to filenames and lines
typedef struct {
    char label_name[64];
    char filename[256];
    BppLineNumber line_number;
} BppGlobalLabel;

// Introspection docstring mapping target names to documentation text
typedef struct {
    char target_name[128]; // e.g. "MATH.ADD" or "::MY_LABEL"
    char docstring[256];
} BppDocstring;

#ifdef BASIC_FREEDOS_16
  #define MAX_METADATA_BLOCKS 8
  #define MAX_BLOCK_BODY_LEN 256
#else
  #define MAX_METADATA_BLOCKS 64
  #define MAX_BLOCK_BODY_LEN 1024
#endif

// Stateful metadata block structure for KEYWORD, SCOPE, ALIAS blocks
typedef struct {
    char block_type[32];   // "KEYWORD", "SCOPE", "ALIAS"
    char target_name[64];  // The target of the block, e.g. "PRINT"
    char docstring[256];   // The docstring of the block
    char body[MAX_BLOCK_BODY_LEN]; // Concatenated line text of the block body
} BppMetadataBlock;

// Registry context holding all metadata
typedef struct {
    BppGlobalLabel global_labels[MAX_GLOBAL_LABELS];
    int global_label_count;

    BppDocstring docstrings[MAX_DOCSTRINGS];
    int docstring_count;

    BppMetadataBlock metadata_blocks[MAX_METADATA_BLOCKS];
    int metadata_block_count;

    char current_namespace[64];      // Active namespace during loading/parsing
    char last_docstring_buffer[256]; // Holds the last parsed docstring temporarily
    bool option_strict;              // Strict variable and type checks enabled
} BppMetadataRegistry;

// @brief Initialize the metadata registry.
void metadata_init(BppMetadataRegistry *reg);

// @brief Register a global label mapping to a file and line.
bool metadata_register_label(BppMetadataRegistry *reg, const char *label, const char *filename, BppLineNumber line);

// @brief Resolve a global label to its file and line location.
bool metadata_resolve_label(const BppMetadataRegistry *reg, const char *label, char *out_filename, size_t max_len, BppLineNumber *out_line);

// @brief Register a docstring for a target identifier or namespace.
bool metadata_register_docstring(BppMetadataRegistry *reg, const char *target, const char *docstring);

// @brief Retrieve a registered docstring for a target.
const char *metadata_get_docstring(const BppMetadataRegistry *reg, const char *target);

// @brief Register a metadata block (KEYWORD, SCOPE, ALIAS).
bool metadata_register_block(BppMetadataRegistry *reg, const char *type, const char *target, const char *docstring, const char *body);

// @brief Retrieve a metadata block by its type and target.
const BppMetadataBlock *metadata_get_block(const BppMetadataRegistry *reg, const char *type, const char *target);

void metadata_pre_scan_line(VMContext *vm, const char *filename, BppLineNumber line_num, const char *text);

// @brief Performs a full pre-scan pass on all stored program memory lines.
void metadata_pre_scan_program(VMContext *vm, const char *filename);

#endif // RUNTIME_METADATA_H
