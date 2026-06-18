/*
 * ---
 * BASIC++ Interpreter - ext_lib_api.h
 * ---
 *
 * External Library API.
 *
 * PURPOSE:
 * Defines the interface for BASIC++ source libraries.
 * Libraries are .lib or .bas files containing reusable
 * DEF FN, SUB, FUNCTION definitions, and DATA statements.
 *
 * DESIGN DECISION:
 * Libraries load into a SEPARATE library program space,
 * NOT into the user's line-number range. This avoids
 * conflicts with user programs and supports QBasic-style
 * code that does not use line numbers.
 *
 * Each library gets its own isolated slot in the library
 * program space. Functions defined in the library are
 * callable from the main program via the function registry.
 *
 * HOW TO USE FROM BASIC++:
 *   LOAD LIBRARY "mathlib.lib"
 *   PRINT FACTORIAL(10)
 *   UNLOAD LIBRARY "mathlib"
 *
 * LIBRARY FILE FORMAT:
 * The first line may declare metadata via REM comments:
 *   REM @LIBRARY MATHLIB
 *   REM @VERSION 1.0
 *   REM @SECURITY SAFE
 *
 * FORMAT SUPPORT (Hybrid):
 * Libraries can be plain BASIC++ source (portable, any
 * platform) or tokenized/pre-compiled (faster loading
 * on constrained platforms like FreeDOS). The loader
 * detects the format automatically by checking for a
 * binary header signature.
 *
 * SECURITY:
 * Libraries can declare a required security level via
 * the REM @SECURITY tag. If declared, the library only
 * loads when the current level matches.
 *
 * C89/C90 COMPLIANT.
 *
 * ---
 */

#ifndef BASICPP_EXT_LIB_API_H
#define BASICPP_EXT_LIB_API_H

#include "../config.h"
#include "../security.h"

/* --- Library Format ---
 *
 * EXT_LIB_SOURCE    - Plain BASIC++ source text
 * EXT_LIB_TOKENIZED - Pre-tokenized binary format
 */
#define EXT_LIB_SOURCE     0
#define EXT_LIB_TOKENIZED  1

/* --- External Library Descriptor ---
 *
 * Tracks a loaded BASIC++ library in the interpreter.
 * Libraries occupy a separate program space, not the
 * user's line-number range.
 */
typedef struct BppExtLib {
    char         name[64];        /* library name             */
    char         version[16];     /* version string           */
    char         path[256];       /* file path (CWD-relative) */
    SecLevel     required_level;  /* SEC_COUNT = unpinned     */
    int          format;          /* EXT_LIB_SOURCE/TOKENIZED */
    int          func_count;      /* functions registered     */
    int          loaded;          /* 1=loaded, 0=not          */
} BppExtLib;

/* --- Library API ---
 *
 * These functions are called by the interpreter to
 * manage the library lifecycle.
 */

/*
 * ext_lib_init - Initialize the library table.
 * Call once at boot.
 */
void ext_lib_init(void);

/*
 * ext_lib_load - Load a BASIC++ library file.
 *
 * Parses the file, extracts DEF FN / SUB / FUNCTION
 * definitions, and registers them in the function
 * registry. The library code lives in a separate
 * program space isolated from user code.
 *
 * Security checks:
 *   1. SECOP_EXT_LOAD must be permitted
 *   2. Path must pass security_check_path()
 *   3. Library's required_level (if declared) must
 *      match current level
 *
 * The format (source vs tokenized) is auto-detected.
 *
 * Returns 0 on success, -1 on error.
 */
int ext_lib_load(const char *path, void *rt);

/*
 * ext_lib_unload - Unload a library by name.
 *
 * Unregisters all functions from the library and
 * frees the library program space slot.
 * Returns 0 on success, -1 if not found.
 */
int ext_lib_unload(const char *name);

/*
 * ext_lib_is_loaded - Check if a library is loaded.
 * Returns 1 if loaded, 0 if not.
 */
int ext_lib_is_loaded(const char *name);

/*
 * ext_lib_find - Find a loaded library by name.
 * Returns NULL if not found.
 */
const BppExtLib *ext_lib_find(const char *name);

/*
 * ext_lib_list - Print all loaded libraries.
 */
void ext_lib_list(void);

/*
 * ext_lib_count - Return the number of loaded libraries.
 */
int ext_lib_count(void);

#endif /* BASICPP_EXT_LIB_API_H */
