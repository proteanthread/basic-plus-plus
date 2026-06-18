/*
 * ---
 * BASIC++ Interpreter - ext_func_api.h
 * ---
 *
 * External Function API.
 *
 * PURPOSE:
 * Defines the interface for single-function C extensions.
 * This is the lightest-weight extension mechanism: one .dll
 * or .so exports one function callable from BASIC++.
 *
 * HOW TO WRITE AN EXTERNAL FUNCTION:
 * 1. Include this header in your C project.
 * 2. Define a static BppExtFunc descriptor with your
 *    function's metadata and handler pointer.
 * 3. Implement bpp_func_register() to return a pointer
 *    to your descriptor.
 * 4. Compile as a shared library (.dll / .so).
 *
 * HOW TO USE FROM BASIC++:
 *   LOAD FUNCTION "double.dll"
 *   PRINT DOUBLE(21)              ' -> 42
 *   UNLOAD FUNCTION "DOUBLE"
 *
 * SECURITY:
 * External functions MUST declare a required_level.
 * The function only loads when the current security
 * level matches. Use SEC_SAFE for most functions.
 *
 * HOT-RELOAD:
 * LOAD FUNCTION replaces an already-loaded function of
 * the same name (hot-reload). Best practice is to call
 * UNLOAD FUNCTION first, but hot-reload is supported for
 * real-time development.
 *
 * C89/C90 COMPLIANT.
 *
 * ---
 */

#ifndef BASICPP_EXT_FUNC_API_H
#define BASICPP_EXT_FUNC_API_H

#include "../config.h"
#include "../security.h"
#include "../value.h"

#ifdef _WIN32
#define BPP_EXPORT __declspec(dllexport)
#else
#define BPP_EXPORT
#endif

/* --- External Function Descriptor ---
 *
 * Metadata for a single function exported by a .dll/.so.
 * All strings must be string literals (not heap-allocated).
 *
 * Fields:
 *   name           - BASIC name ("DOUBLE", "HTTPGET$")
 *   min_args       - minimum argument count
 *   max_args       - maximum argument count
 *   ret_type       - 0=int, 1=float, 2=string, 3=any
 *   safety         - FSAFE_PURE / STATE / IO / SYSTEM
 *   required_level - must match current security level
 *   help_text      - one-line description for HELP
 *   handler        - function pointer (FuncHandler)
 */
typedef struct BppExtFunc {
    const char  *name;
    int          min_args;
    int          max_args;
    int          ret_type;
    int          safety;
    SecLevel     required_level;
    const char  *help_text;
    BValue     (*handler)(BValue *args, int argc, void *rt);
} BppExtFunc;

/* --- Entry Point ---
 *
 * This function must be defined and exported by your .dll/.so.
 * It is called immediately after LoadLibrary/dlopen.
 * Return a pointer to a static BppExtFunc descriptor.
 *
 * Example:
 *   static BValue my_double(BValue *args, int argc,
 *                           void *rt)
 *   {
 *       (void)rt;
 *       return bval_float(bval_to_float(&args[0]) * 2.0);
 *   }
 *
 *   static const BppExtFunc my_desc = {
 *       "DOUBLE", 1, 1, 1, 0,
 *       SEC_SAFE,
 *       "Doubles a numeric value",
 *       my_double
 *   };
 *
 *   BPP_EXPORT const BppExtFunc *bpp_func_register(void)
 *   {
 *       return &my_desc;
 *   }
 */
BPP_EXPORT const BppExtFunc *bpp_func_register(void);

/* --- Host-Side API ---
 *
 * These functions are called by the interpreter, not by
 * external function authors.
 */

/*
 * ext_func_init - Initialize the external function table.
 * Call once at boot.
 */
void ext_func_init(void);

/*
 * ext_func_load - Load an external function from a .dll/.so.
 *
 * Security checks:
 *   1. SECOP_EXT_LOAD must be permitted
 *   2. Path must pass security_check_path()
 *   3. Function's required_level must match current level
 *
 * If a function with the same name is already loaded,
 * it is replaced (hot-reload).
 *
 * Returns 0 on success, -1 on error.
 */
int ext_func_load(const char *path, void *rt);

/*
 * ext_func_unload - Unload an external function by name.
 *
 * Unregisters from funcreg and frees the library handle.
 * Returns 0 on success, -1 if not found.
 */
int ext_func_unload(const char *name);

/*
 * ext_func_find - Find a loaded external function by name.
 * Returns NULL if not found.
 */
const BppExtFunc *ext_func_find(const char *name);

/*
 * ext_func_list - Print all loaded external functions.
 */
void ext_func_list(void);

/*
 * ext_func_count - Return the number of loaded external
 * functions.
 */
int ext_func_count(void);

#endif /* BASICPP_EXT_FUNC_API_H */
