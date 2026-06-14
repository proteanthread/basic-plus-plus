#ifndef ERROR_REGISTRY_H
#define ERROR_REGISTRY_H

/*
 * Extensible Error Architecture
 * Maps 32-bit error codes to syntax names.
 * Allows core errors, dialect errors, and custom module errors.
 */

void error_registry_init(void);

/* Register a custom error string for a 32-bit code */
int error_registry_register(int error_code, const char *syntax_name);

/* Retrieve the syntax name for an error code (returns NULL if not found) */
const char *error_registry_lookup(int error_code);

/* The default built-in ERR$ function handler */
struct BValue;
struct BValue builtin_err_str(struct BValue *args, int argc, void *rt);

#endif
