// FILENAME: language_descriptor.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel, libflex, libscript, libstandard, libadvanced
// NEEDS: types.h
// Provides core data structures and interfaces for language descriptors and dialect reflection.

#ifndef RUNTIME_LANGUAGE_DESCRIPTOR_H
#define RUNTIME_LANGUAGE_DESCRIPTOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "types/types.h"

//
// ---- Constants ----

#define LANG_DESC_INITIAL_CAPACITY 1024

//
// ---- Types & Enumerations ----

typedef enum {
    FEATURE_STATEMENT = 0,
    FEATURE_FUNCTION,
    FEATURE_COMMAND,
    FEATURE_VARIABLE,
    FEATURE_MODULE,
    FEATURE_OPERATOR,
    FEATURE_DEVICE,
    FEATURE_WIDGET
} FeatureType;

typedef enum {
    SAFETY_PURE = 0,       // no side-effects, deterministic (e.g. SIN, COS)
    SAFETY_SAFE,           // safe managed state (e.g. LEFT$, MID$)
    SAFETY_IO,             // file or console I/O (e.g. PRINT, INPUT)
    SAFETY_SYSTEM,         // system resources (e.g. PEEK, POKE, SHELL)
    SAFETY_UNSAFE          // privileged hardware access
} SafetyLevel;

typedef enum {
    SUBSYSTEM_BOOT = 0,
    SUBSYSTEM_PLATFORM,
    SUBSYSTEM_KERNEL,
    SUBSYSTEM_ENGINE,
    SUBSYSTEM_HARDWARE,
    SUBSYSTEM_SERVER,
    SUBSYSTEM_SCRIPT,
    SUBSYSTEM_CORE,
    SUBSYSTEM_FLEX,
    SUBSYSTEM_STANDARD,
    SUBSYSTEM_ADVANCED,
    SUBSYSTEM_EXT
} FunctionalSubsystem;

typedef enum {
    DELIM_NONE    = 0,
    DELIM_PAREN   = 1 << 0,  // Standard Infix ( )
    DELIM_BRACKET = 1 << 1,  // Prefix PN / Array Slice / Option [ ]
    DELIM_BRACE   = 1 << 2,  // Postfix RPN / Map / Set / Struct { }
    DELIM_ALL     = (DELIM_PAREN | DELIM_BRACKET | DELIM_BRACE)
} DelimiterMask;

typedef struct DescriptorError {
    int         code;           // Numeric error code (e.g. ERR_SYNTAX, ERR_TYPE_MISMATCH)
    const char *description;    // Concise description / reason
} DescriptorError;

// Authoritative feature descriptor and runtime dispatch descriptor
typedef struct LanguageDescriptor {
    const char             *name;          // Canonical feature name (e.g. "DIM", "SIN", "PRINT")
    const char             *category;      // Functional category (e.g. "Variables & Memory")
    const char             *syntax;        // Formal syntax signature
    union {
        const char         *description;   // Concise human-readable description
        const char         *help_text;     // Alias for description
    };
    union {
        const char         *error_summary; // Formatted error summary string
        const char         *error_codes;   // Alias for error_summary
    };
    const DescriptorError  *errors;        // Optional structured error array
    size_t                  error_count;   // Number of structured errors
    uint32_t                subsystem;     // 12-library functional spectrum enum
    SafetyLevel             safety;        // Sandbox and execution safety classification
    FeatureType             type;          // Feature classification (statement, function, variable, etc.)
    const char             *examples;      // Ready-to-run code example string
    void                   *dispatch_fn;   // Statement, function evaluation, or variable accessor pointer
    uint32_t                delim_mask;    // Delimiter capability bitmask (DELIM_PAREN, DELIM_BRACKET, DELIM_BRACE)
    void                   *dispatch_bracket_fn; // Optional dedicated bracket evaluation handler
    void                   *dispatch_brace_fn;   // Optional dedicated brace evaluation handler

    // Extended Architectural & Provenance Fields
    const char             *rationale;     // Architectural rationale / why it exists
    const char             *design_notes;  // Implementation & design notes / why it works this way
    const char             *tunables;      // What can be changed (runtime settings, arguments)
    const char             *invariants;    // What cannot be changed (invariants, constraints)
    const char             *assumptions;   // Assumptions made about environment or arguments
    uint32_t                platform_mask; // Platform capability mask (desktop, server, iot, dos, etc.)
    const char             *roadmap;       // Planned expansions / future roadmap
    const char             *compat;        // Vintage dialect compatibility / lineage
    const char             *since_version; // Version when feature was introduced
    const char             *source_file;   // C source implementation file authority
    const char             *see_also;      // Related keywords / cross-references
} LanguageDescriptor;

typedef LanguageDescriptor LangDesc;

//
// ---- Registration & Query API ----

void lang_desc_init(void);
void lang_desc_shutdown(void);
int  lang_desc_register(const LanguageDescriptor *desc);
int  lang_desc_override(const char *name, const LanguageDescriptor *desc);
const LanguageDescriptor *lang_desc_find(const char *name);
const LanguageDescriptor *lang_desc_get(int index);
int  lang_desc_count(void);
int  lang_desc_query_category(const char *category, const LanguageDescriptor **results, int max_results);
int  lang_desc_get_categories(char out_categories[][64], int max_categories);
const char *lang_desc_lookup_error_msg(int code);
char *lang_desc_export_json(void);
void  lang_desc_free_json(char *json);

//
// ---- Declarative Compile-Time Macro Helpers ----

#define REGISTER_STATEMENT(desc_ptr, handler_fn) do { \
    (desc_ptr)->type = FEATURE_STATEMENT; \
    (desc_ptr)->dispatch_fn = (void *)(handler_fn); \
    lang_desc_register(desc_ptr); \
} while(0)

#define REGISTER_FUNCTION(desc_ptr, handler_fn) do { \
    (desc_ptr)->type = FEATURE_FUNCTION; \
    (desc_ptr)->dispatch_fn = (void *)(handler_fn); \
    lang_desc_register(desc_ptr); \
} while(0)

#define REGISTER_VARIABLE(desc_ptr, getter_fn) do { \
    (desc_ptr)->type = FEATURE_VARIABLE; \
    (desc_ptr)->dispatch_fn = (void *)(getter_fn); \
    lang_desc_register(desc_ptr); \
} while(0)

#endif // RUNTIME_LANGUAGE_DESCRIPTOR_H
