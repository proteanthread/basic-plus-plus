/*
 * ---
 * BASIC++ Interpreter - lib_space.h
 * ---
 *
 * Library Program Space manager.
 *
 * PURPOSE:
 * Manages a separate program space for loaded libraries,
 * external functions, external features, external modules,
 * and external plugins. This space is ISOLATED from the
 * user's program (no line-number conflicts).
 *
 * ARCHITECTURE:
 * Each loaded extension gets a slot containing:
 *   - Compiled PCode bytecode (PCodeProgram)
 *   - Symbol table (SUB/FUNCTION/DEF FN entry points)
 *   - Isolated variable space (hybrid scope model)
 *   - Security metadata
 *
 * VARIABLE SCOPE (Hybrid Model):
 * The scope model depends on the extension type:
 *
 *   External Libraries (.LIB):
 *     - Own isolated 26-var space (A-Z), plus named vars
 *     - LOCAL declarations create stack-based temporaries
 *     - Named variables (longer names) supported
 *
 *   External Functions (.FN):
 *     - Stack-based parameters and LOCAL vars only
 *     - No persistent state between calls
 *
 *   External Features (.FT):
 *     - Own isolated variable space (named vars)
 *     - LOCAL declarations supported
 *
 *   External Modules (.MOD):
 *     - Full isolated variable space (A-Z + named)
 *     - LOCAL declarations supported
 *     - Persistent state across calls
 *
 *   External Plugins (.PLG):
 *     - Own isolated variable space (named vars)
 *     - LOCAL declarations supported
 *     - Can expose both SUBs and FUNCTIONs
 *
 * LOADING MODES:
 *   Interpreter mode: compile-on-load (source -> bytecode)
 *   Compiled mode: load pre-compiled .BPL bytecode
 *
 * COMPATIBILITY:
 *   - GWBASIC: line-numbered programs can CALL library SUBs
 *   - QBASIC: structured programs can CALL library SUBs
 *   - ECMA-116: standard CALL semantics preserved
 *   - Libraries may optionally contain line numbers
 *
 * C89/C90 COMPLIANT.
 *
 * ---
 */

#ifndef BASICPP_LIB_SPACE_H
#define BASICPP_LIB_SPACE_H

#include "../pcode.h"
#include "../security.h"
#include "../config.h"
#include "../value.h"

/* --- Limits --- */
#ifndef MAX_LOADED_LIBS
#define MAX_LOADED_LIBS     16
#endif

#ifndef MAX_LIB_SYMBOLS
#define MAX_LIB_SYMBOLS     64
#endif

#ifndef MAX_LIB_NAME
#define MAX_LIB_NAME        64
#endif

#ifndef MAX_LIB_NAMED_VARS
#define MAX_LIB_NAMED_VARS  64
#endif

/* --- Symbol Types ---
 * What kind of callable entry a symbol represents.
 */
typedef enum LibSymbolType {
    LIB_SYM_SUB = 0,       /* SUB name(args) ... END SUB       */
    LIB_SYM_FUNCTION,       /* FUNCTION name(args) ... END FUNC */
    LIB_SYM_DEF_FN          /* DEF FN name(args) = expr         */
} LibSymbolType;

/* --- Symbol Entry ---
 * One callable entry point in a loaded library.
 */
typedef struct LibSymbol {
    char            name[MAX_LIB_NAME]; /* symbol name (uppercase) */
    LibSymbolType   type;               /* SUB / FUNCTION / DEF_FN */
    int             entry_offset;       /* instruction index entry  */
    int             param_count;        /* number of parameters     */
    int             active;             /* 1=valid, 0=removed       */
} LibSymbol;

/* --- Named Variable ---
 * A named variable in the library's isolated scope.
 * Supports names longer than single A-Z letters.
 */
typedef struct LibNamedVar {
    char    name[MAX_LIB_NAME]; /* variable name (uppercase)    */
    BValue  val;                /* current value                */
    int     is_string;          /* 1=string var (name ends '$') */
    int     active;             /* 1=in use, 0=free             */
} LibNamedVar;

/* --- Extension Type ---
 * Which kind of extension this slot holds.
 * Determines the variable scope model.
 */
typedef enum LibExtType {
    LIB_EXT_LIBRARY = 0,    /* .LIB - libraries            */
    LIB_EXT_FUNCTION,        /* .FN  - external functions   */
    LIB_EXT_FEATURE,         /* .FT  - external features    */
    LIB_EXT_MODULE,          /* .MOD - external modules     */
    LIB_EXT_PLUGIN           /* .PLG - external plugins     */
} LibExtType;

#ifndef MAX_LIB_LINES
#define MAX_LIB_LINES       512
#endif

/* --- Source Line Entry ---
 * One stored source line from a library file.
 * Used in interpreter-mode: lines are stored here and
 * executed via parser_execute_line() when CALL'd.
 */
typedef struct LibSourceLine {
    char    text[256];      /* source line text              */
    int     vline;          /* virtual line number           */
} LibSourceLine;

/* --- Loaded Library Slot ---
 * One loaded extension in the library program space.
 */
typedef struct LoadedLibrary {
    /* Identity */
    char            name[MAX_LIB_NAME]; /* library name          */
    char            version[16];        /* version string        */
    char            path[256];          /* source file path      */
    LibExtType      ext_type;           /* library/func/feature/etc */
    SecLevel        required_level;     /* pinned security level */

    /* Compiled bytecode (used in compiled mode) */
    PCodeProgram    pcode;              /* compiled instructions */

    /* Source lines (used in interpreter mode) */
    LibSourceLine  *src_lines;          /* dynamically allocated */
    int             src_line_count;     /* number of stored lines */
    int             src_line_cap;       /* allocated capacity    */

    /* Symbol table */
    LibSymbol       symbols[MAX_LIB_SYMBOLS];
    int             symbol_count;

    /* Isolated variable space (hybrid scope) */
    double          num_vars[26];       /* A-Z numeric vars      */
    char           *str_vars[26];       /* A$-Z$ string vars     */
    LibNamedVar     named_vars[MAX_LIB_NAMED_VARS]; /* named vars */
    int             named_var_count;

    /* State */
    int             loaded;             /* 1=active, 0=empty     */
    int             compiled;           /* 1=bytecode ready      */
} LoadedLibrary;

/* ===================================================================
 * LIBRARY SPACE API
 * ===================================================================
 */

/*
 * lib_space_init - Initialize the library program space.
 * Call once at interpreter boot.
 */
void lib_space_init(void);

/*
 * lib_space_shutdown - Free all loaded libraries.
 * Call at interpreter exit.
 */
void lib_space_shutdown(void);

/*
 * lib_space_load_source - Load a library from source.
 *
 * Reads the source file, parses SUB/FUNCTION/DEF FN blocks,
 * compiles them to PCode, and registers symbols.
 *
 * Security checks are performed BEFORE loading.
 *
 * Parameters:
 *   path     - source file path (.lib, .fn, .ft, etc.)
 *   ext_type - what kind of extension (LIB_EXT_LIBRARY, etc.)
 *   rt       - runtime state (for error reporting)
 *
 * Returns library slot index (>=0) on success, -1 on error.
 */
int lib_space_load_source(const char *path, LibExtType ext_type,
                          void *rt);

/*
 * lib_space_load_compiled - Load a pre-compiled .BPL library.
 *
 * Reads the .BPL binary container, deserializes the bytecode
 * and symbol table, and registers the library.
 *
 * Returns library slot index (>=0) on success, -1 on error.
 */
int lib_space_load_compiled(const char *path, LibExtType ext_type,
                            void *rt);

/*
 * lib_space_unload - Unload a library by name.
 *
 * Frees bytecode, clears variables, unregisters symbols.
 * Returns 0 on success, -1 if not found.
 */
int lib_space_unload(const char *name);

/*
 * lib_space_find - Find a loaded library by name.
 * Returns pointer to LoadedLibrary, or NULL.
 */
LoadedLibrary *lib_space_find(const char *name);

/*
 * lib_space_find_symbol - Find a callable symbol across
 * all loaded libraries.
 *
 * Searches all loaded libraries for a SUB/FUNCTION/DEF FN
 * matching the given name.
 *
 * Parameters:
 *   name    - symbol name to find
 *   out_lib - (out) pointer to the owning library
 *
 * Returns pointer to LibSymbol, or NULL.
 */
LibSymbol *lib_space_find_symbol(const char *name,
                                 LoadedLibrary **out_lib);

/*
 * lib_space_invoke - Call a library SUB/FUNCTION.
 *
 * Pushes current VM state, sets up parameters,
 * executes the library bytecode from the symbol's
 * entry point, then restores VM state.
 *
 * For FUNCTION/DEF FN, the return value is placed
 * on the VM value stack.
 *
 * Parameters:
 *   lib    - the library containing the symbol
 *   sym    - the symbol to invoke
 *   args   - argument values (caller provides)
 *   argc   - number of arguments
 *   result - (out) return value for FUNCTION/DEF FN
 *   rt     - runtime state
 *
 * Returns 0 on success, -1 on error.
 */
int lib_space_invoke(LoadedLibrary *lib, LibSymbol *sym,
                     BValue *args, int argc,
                     BValue *result, void *rt);

/*
 * lib_space_list - Print all loaded libraries to stdout.
 */
void lib_space_list(void);

/*
 * lib_space_count - Return number of loaded libraries.
 */
int lib_space_count(void);

/*
 * lib_space_get_var - Get a named variable from a library's
 * isolated scope.
 */
BValue *lib_space_get_var(LoadedLibrary *lib, const char *name);

/*
 * lib_space_set_var - Set a named variable in a library's
 * isolated scope.
 */
int lib_space_set_var(LoadedLibrary *lib, const char *name,
                      BValue val);

/*
 * lib_space_try_call - Try to call a library SUB/FUNCTION.
 *
 * Searches all loaded libraries for a symbol matching 'name'.
 * If found, invokes it with the given arguments.
 * This is the C89-compatible entry point for parser_struct.c
 * (avoids needing LoadedLibrary/LibSymbol types in parser).
 *
 * Returns 1 if found and called, 0 if not found.
 */
int lib_space_try_call(const char *name, int name_len,
                       BValue *args, int argc,
                       BValue *result, void *rt);

/*
 * lib_space_invoke_by_name - Convenience wrapper for spec system.
 *
 * Finds a symbol matching sym_name within the given library
 * and invokes it with zero arguments. Used by the spec
 * system to dispatch custom statements into companion .lib.
 *
 * Returns 0 on success, -1 if symbol not found or error.
 */
int lib_space_invoke_by_name(void *lib_ptr,
    const char *sym_name, void *rt);

#endif /* BASICPP_LIB_SPACE_H */
