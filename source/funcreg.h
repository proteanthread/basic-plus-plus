/*
 * =====================================================================
 * BASIC++ Interpreter - funcreg.h
 * =====================================================================
 *
 * Function Registry System interface.
 *
 * PURPOSE:
 * Provides a formal, centralized registry for all callable functions
 * in the BASIC++ runtime. Every built-in function (ABS, SIN, LEN,
 * LEFT$, etc.) is registered here with metadata describing its
 * behavior, argument requirements, return type, safety classification,
 * and dialect-override status.
 *
 * WHY THIS EXISTS:
 * The BASIC++ specification requires:
 * "all built-in functionality shall pass through the standard
 * library layer"
 * "dialects may extend or override library behavior, but shall
 * not bypass it"
 * "all callable functions shall be registered in a global
 * function table"
 *
 * Previously, functions were handled inline in the parser via
 * large switch/if chains. This made it impossible for dialects
 * to override functions, for modules to register new ones, or
 * for the system to introspect its own capabilities.
 *
 * The registry solves all of these problems:
 * 1. Functions are data, not hardcoded parser logic.
 * 2. Dialects override by swapping handler pointers.
 * 3. Modules register new functions at runtime.
 * 4. Introspection queries the registry for capabilities.
 * 5. The compiler can enumerate available functions.
 *
 * ARCHITECTURE:
 * The registry has two tiers:
 *
 * CORE IMMUTABLE API (cannot be overridden by dialects):
 * Fundamental operations that define BASIC++ semantics.
 * These are marked with overridable = 0.
 * Example: variable access, basic arithmetic operations.
 *
 * DIALECT-OVERRIDABLE API (dialects may replace handlers):
 * Functions whose behavior varies between BASIC dialects.
 * These are marked with overridable = 1.
 * Example: PRINT formatting, INPUT prompting, CHR$().
 *
 * FUNCTION HANDLER SIGNATURE:
 * All function handlers use the same signature:
 *
 * BValue handler(BValue *args, int argc, void *rt)
 *
 * Where:
 * args - array of evaluated argument values
 * argc - number of arguments passed
 * rt - opaque pointer to RuntimeState (for RND seed, strpool)
 *
 * This uniform signature allows:
 * - Registry-based dispatch without type casts
 * - Module functions to use the same interface
 * - Safe argument count validation before calling
 *
 * HOW TO ADD A NEW BUILT-IN FUNCTION:
 * 1. Write the handler in builtins.c following the FuncHandler
 * signature.
 * 2. Add a FunctionEntry to the builtins_register() table in
 * builtins.c with the keyword ID, category, arg counts, etc.
 * 3. If the function needs a new keyword, add it to lexer.h/c.
 * 4. The parser will automatically dispatch through the registry.
 *
 * HOW TO WRITE EXTERNAL FUNCTIONS (modules/plugins):
 * 1. Write a handler with the FuncHandler signature:
 *
 * BValue my_func(BValue *args, int argc, void *rt)
 * {
 * double x = bval_to_float(&args[0]);
 * return bval_float(x * 2.0);
 * }
 *
 * 2. Create a FunctionEntry:
 *
 * FunctionEntry entry;
 * memset(&entry, 0, sizeof(entry));
 * entry.name = "DOUBLE";
 * entry.keyword = KW_COUNT; (or a registered keyword)
 * entry.category = FCAT_USER;
 * entry.ret_type = FRET_FLOAT;
 * entry.min_args = 1;
 * entry.max_args = 1;
 * entry.safety = FSAFE_PURE;
 * entry.overridable = 0;
 * entry.handler = my_func;
 *
 * 3. Register it:
 *
 * funcreg_register(&entry);
 *
 * 4. The function is now callable from BASIC programs:
 *
 * PRINT DOUBLE(21) -> 42
 *
 * PORTABILITY:
 * - Uses only C89-safe constructs
 * - Function pointers use a flat signature (no va_list)
 * - Registry is a static array (no dynamic allocation)
 * - All metadata is constant-compatible
 *
 * =====================================================================
 */

#ifndef BASICPP_FUNCREG_H
#define BASICPP_FUNCREG_H

#include "value.h"
#include "lexer.h"

/* =====================================================================
 * Function Categories
 * =====================================================================
 * Organizes functions into logical groups for introspection,
 * documentation, and capability checking.
 *
 * FCAT_CORE - Language primitives (always available)
 * FCAT_MATH - Mathematical functions (SIN, COS, ABS, etc.)
 * FCAT_STRING - String manipulation (LEN, LEFT$, MID$, etc.)
 * FCAT_IO - Input/output (future: INKEY$, etc.)
 * FCAT_UTIL - Utility functions (RND, SIZE, etc.)
 * FCAT_USER - User-defined or module-registered functions
 */
typedef enum FuncCategory {
 FCAT_CORE = 0,
 FCAT_MATH = 1,
 FCAT_STRING = 2,
 FCAT_IO = 3,
 FCAT_UTIL = 4,
 FCAT_USER = 5
} FuncCategory;

/* =====================================================================
 * Function Return Types
 * =====================================================================
 * Declares the expected return type for type-checking and
 * optimization. FRET_ANY means the return type depends on inputs.
 */
typedef enum FuncReturnType {
 FRET_INT = 0, /* Returns integer */
 FRET_FLOAT = 1, /* Returns float */
 FRET_STRING = 2, /* Returns string */
 FRET_ANY = 3 /* Return type varies with input */
} FuncReturnType;

/* =====================================================================
 * Function Safety Classification
 * =====================================================================
 * Used by the module system and security layer to determine what
 * capabilities a function requires.
 *
 * FSAFE_PURE - No side effects, deterministic, always safe.
 * Example: ABS, SIN, LEN
 * FSAFE_STATE - Modifies runtime state (e.g., RND seed).
 * Example: RND
 * FSAFE_IO - Performs I/O through virtual devices.
 * Example: future INKEY$, PRINT extensions
 * FSAFE_SYSTEM - Accesses system resources.
 * Example: future file operations, timers
 */
typedef enum FuncSafety {
 FSAFE_PURE = 0,
 FSAFE_STATE = 1,
 FSAFE_IO = 2,
 FSAFE_SYSTEM = 3
} FuncSafety;

/* =====================================================================
 * Function Handler Signature
 * =====================================================================
 * All registered functions use this uniform signature:
 *
 * args - array of pre-evaluated BValue arguments
 * argc - number of arguments (already validated against min/max)
 * rt - opaque pointer to RuntimeState (cast inside handler)
 *
 * Returns a BValue result. On error, the handler calls error_raise()
 * and returns bval_int(0).
 *
 * WHY void* for rt:
 * To avoid a circular dependency between funcreg.h and runtime.h.
 * RuntimeState includes the function registry, and the function
 * registry needs to call runtime services. The void* breaks the
 * cycle cleanly. Handlers cast to (RuntimeState*) internally.
 */
typedef BValue (*FuncHandler)(BValue *args, int argc, void *rt);

/* =====================================================================
 * Function Entry
 * =====================================================================
 * A single entry in the function registry. Contains all metadata
 * needed for dispatch, validation, introspection, and documentation.
 *
 * Fields:
 * name - Function name as typed in BASIC (e.g., "ABS")
 * keyword - Corresponding KeywordId from lexer.h
 * category - Logical grouping (FCAT_MATH, etc.)
 * ret_type - Expected return type
 * min_args - Minimum number of arguments
 * max_args - Maximum number of arguments
 * safety - Safety classification for module system
 * overridable - 1 if dialects may replace the handler, 0 if core
 * handler - Function pointer to the implementation
 */
typedef struct FunctionEntry {
 const char *name;
 KeywordId keyword;
 FuncCategory category;
 FuncReturnType ret_type;
 int min_args;
 int max_args;
 FuncSafety safety;
 int overridable;
 FuncHandler handler;
 const char *help_text; /* one-line description for HELP */
} FunctionEntry;

/* =====================================================================
 * Registry Limits
 * =====================================================================
 * Maximum number of registered functions. Built-in functions use
 * approximately 20 slots. The remaining slots are available for
 * module-registered and user-defined functions.
 */
#ifndef MAX_FUNCTIONS
#define MAX_FUNCTIONS 128
#endif

/* =====================================================================
 * Registry Functions
 * =====================================================================
 */

/*
 * funcreg_init - Initialize the function registry.
 *
 * Clears the registry table. Must be called once during boot,
 * before any functions are registered.
 *
 * Boot sequence position: ("Standard Library Init")
 * per the BASIC++ specification.
 */
void funcreg_init(void);

/*
 * funcreg_register - Register a function in the registry.
 *
 * Copies the FunctionEntry into the next available slot.
 * Returns 0 on success, -1 if the registry is full.
 *
 * EXTENSION POINT:
 * External modules call this to add new functions.
 * The function becomes immediately available for dispatch.
 */
int funcreg_register(const FunctionEntry *entry);

/*
 * funcreg_find_by_keyword - Look up a function by keyword ID.
 *
 * Returns a pointer to the FunctionEntry, or NULL if not found.
 * This is the primary lookup path used by the parser during
 * expression evaluation.
 *
 * Performance: linear scan is acceptable at <128 entries.
 * If the registry grows significantly, a hash or sorted
 * array with binary search could be substituted.
 */
const FunctionEntry *funcreg_find_by_keyword(KeywordId kw);

/*
 * funcreg_find_by_name - Look up a function by name string.
 *
 * Returns a pointer to the FunctionEntry, or NULL if not found.
 * Used for module loading, introspection, and future DEF FN
 * override resolution.
 *
 * Comparison is case-insensitive (BASIC convention).
 */
const FunctionEntry *funcreg_find_by_name(const char *name);

/*
 * funcreg_override - Replace the handler for a registered function.
 *
 * Used by dialect modules to swap function behavior. Only functions
 * marked overridable=1 can be overridden. Attempting to override
 * a core (overridable=0) function returns -1.
 *
 * Parameters:
 * kw - keyword ID of the function to override
 * handler - new handler function pointer
 *
 * Returns 0 on success, -1 on error (not found or not overridable).
 */
int funcreg_override(KeywordId kw, FuncHandler handler);

/*
 * funcreg_count - Return the number of registered functions.
 *
 * Used for introspection and diagnostic output.
 */
int funcreg_count(void);

/*
 * funcreg_get - Get a function entry by index.
 *
 * Returns a pointer to the entry at the given index, or NULL
 * if the index is out of range. Used for enumeration.
 *
 * Example (listing all functions):
 * for (i = 0; i < funcreg_count(); i++) {
 * const FunctionEntry *f = funcreg_get(i);
 * printf("%s (%d-%d args)\n", f->name, f->min_args, f->max_args);
 * }
 */
const FunctionEntry *funcreg_get(int index);

#endif /* BASICPP_FUNCREG_H */
