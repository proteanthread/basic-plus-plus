/*
 * ---
 * BASIC++ Interpreter - error_registry.c
 * ---
 *
 * Extensible Error Architecture
 * Maps 32-bit error codes to syntax names.
 *
 * ---
 */

#include "error_registry.h"
#include "value.h"
#include "runtime.h"
#include "stringpool.h"
#include <string.h>

#define MAX_CUSTOM_ERRORS 128

typedef struct {
    int error_code;
    const char *syntax_name;
} ErrorEntry;

static ErrorEntry error_map[MAX_CUSTOM_ERRORS];
static int error_count = 0;

void error_registry_init(void) {
    error_count = 0;
    memset(error_map, 0, sizeof(error_map));
}

int error_registry_register(int error_code, const char *syntax_name) {
    int i;
    for (i = 0; i < error_count; i++) {
        if (error_map[i].error_code == error_code) {
            error_map[i].syntax_name = syntax_name;
            return 0;
        }
    }
    if (error_count >= MAX_CUSTOM_ERRORS) return -1;
    error_map[error_count].error_code = error_code;
    error_map[error_count].syntax_name = syntax_name;
    error_count++;
    return 0;
}

const char *error_registry_lookup(int error_code) {
    int i;
    for (i = 0; i < error_count; i++) {
        if (error_map[i].error_code == error_code) {
            return error_map[i].syntax_name;
        }
    }
    /* Fallbacks for native PATB / MS-BASIC common errors */
    switch (error_code) {
        case 1: return "NEXT without FOR";
        case 2: return "Syntax error";
        case 3: return "RETURN without GOSUB";
        case 4: return "Out of data";
        case 5: return "Illegal function call";
        case 6: return "Overflow";
        case 7: return "Out of memory";
        case 8: return "Undefined line";
        case 9: return "Subscript out of range";
        case 10: return "Redimensioned array";
        case 11: return "Division by zero";
        case 13: return "Type mismatch";
        case 14: return "Out of string space";
        case 15: return "String too long";
        case 16: return "String formula too complex";
        case 17: return "Can't continue";
        case 18: return "Undefined user function";
    }
    return "Unknown Error";
}

BValue builtin_err_str(BValue *args, int argc, void *rt) {
    RuntimeState *state = (RuntimeState *)rt;
    int err_code;
    const char *err_str;
    char *buf;
    int len;

    if (argc < 1) {
        err_code = state->last_err_code; /* if ERR$() called with no args */
    } else {
        err_code = (int)bval_to_int(&args[0]);
    }

    err_str = error_registry_lookup(err_code);
    if (!err_str) err_str = "Unknown Error";
    
    len = (int)strlen(err_str);
    buf = strpool_alloc(&state->strpool, len);
    if (buf == NULL) return bval_string(NULL, 0);
    memcpy(buf, err_str, (size_t)len);
    return bval_string(buf, len);
}
