// FILENAME: funcreg.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libext
// NEEDS: libengine (lexer.h, lexer.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for funcreg within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_FUNCREG_H
#define RUNTIME_FUNCREG_H

#include "types/types.h"
#include "lexer/lexer.h"

#define MAX_FUNCTIONS 128

typedef enum {
    FCAT_CORE = 0,
    FCAT_MATH = 1,
    FCAT_STRING = 2,
    FCAT_IO = 3,
    FCAT_UTIL = 4,
    FCAT_USER = 5
} FuncCategory;

typedef enum {
    FRET_INT = 0,
    FRET_FLOAT = 1,
    FRET_STRING = 2,
    FRET_ANY = 3
} FuncReturnType;

typedef enum {
    FSAFE_PURE = 0,
    FSAFE_STATE = 1,
    FSAFE_IO = 2,
    FSAFE_SYSTEM = 3
} FuncSafety;

typedef BValue (*FuncHandler)(BValue *args, int argc, void *rt);

typedef struct {
    const char     *name;        // Function name, e.g. "ABS", "CUBE"
    BppKeywordId    keyword;     // KW_NONE or other BppKeywordId
    FuncCategory    category;    // FCAT_MATH, FCAT_USER, etc.
    FuncReturnType  ret_type;    // FRET_INT, FRET_STRING, etc.
    int             min_args;    // minimum arguments
    int             max_args;    // maximum arguments
    FuncSafety      safety;      // FSAFE_PURE ... FSAFE_SYSTEM
    int             overridable; // 1=can be overridden
    FuncHandler     handler;     // C function pointer
    const char     *help_text;   // help docstring
    const char     *module_name; // module that registered this function
} FunctionEntry;

// Value Helpers for C Modules
static inline BValue bval_float(double val) {
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = val;
    return res;
}

static inline double bval_to_float(const BValue *val) {
    return val->type == VAL_NUMBER ? val->as.number : 0.0;
}

void                 funcreg_init(void);
void                 funcreg_set_registering_module(const char *name);
int                  funcreg_register(const FunctionEntry *entry);
const FunctionEntry *funcreg_find_by_name(const char *name);
const FunctionEntry *funcreg_find_by_keyword(BppKeywordId kw);
int                  funcreg_override(BppKeywordId kw, FuncHandler handler);
int                  funcreg_count(void);
const FunctionEntry *funcreg_get(int index);

#endif // RUNTIME_FUNCREG_H
