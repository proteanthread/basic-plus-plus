// ---
// BASIC++ Interpreter - error_registry.h
// ---
//
// Extensible Error Architecture.
//
// PURPOSE:
//   Maps 32-bit error codes to human-readable syntax names.
//   Supports three layers of error codes:
//     - Core errors (ERR_WHAT, ERR_HOW, ERR_SORRY)
//     - Dialect errors (GW-BASIC codes 1-77)
//     - Custom module errors (user-registered)
//
// HOW TO EXTEND:
//   1. Call error_registry_register(code, "NAME") to add a code.
//   2. Use error_registry_lookup(code) to retrieve the name.
//   3. The ERR$ function in BASIC returns the name for ERR codes.
//
// ---

#ifndef ERROR_REGISTRY_H
#define ERROR_REGISTRY_H

// error_registry_init - Initialize with built-in error codes.
//
void error_registry_init(void);

// error_registry_register - Register a custom error string.
//
// Maps 'error_code' to 'syntax_name'. Returns 0 on success.
//
int error_registry_register(int error_code, const char *syntax_name);

// error_registry_lookup - Retrieve the name for an error code.
//
// Returns the registered string, or NULL if not found.
//
const char *error_registry_lookup(int error_code);

// builtin_err_str - The ERR$() function handler.
//
// Returns a BValue string with the error name for the given code.
//
struct BValue;
struct BValue builtin_err_str(struct BValue *args, int argc, void *rt);

#endif // ERROR_REGISTRY_H
