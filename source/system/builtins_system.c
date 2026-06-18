 // ---
 // BASIC++ Interpreter - builtins_system.c
 // ---
 //
 // System and environment function handlers for the built-in
 // function registry.
 //
 // Contains handlers for environment variable access and other
 // system-level queries.
 //
//
// HOW TO EXTEND:
//   To add a new built-in function:
//   1. Write a handler: BValue my_func(BValue *args, int argc, void *ctx)
//   2. Register it in the init function with funcreg_add().
//   3. Specify min/max argument counts and return type.
//
// TROUBLESHOOTING:
//   - Wrong arg count: check min_args/max_args in registration.
//   - Type mismatch: use bval_to_float/bval_to_int for conversion.
 // ---

#include <stdlib.h>
#include <string.h>
#include "builtins.h"
#include "runtime.h"
#include "stringpool.h"
#include "value.h"

 // ENVIRON$(name$) - Get environment variable.
 // Returns the value of the named env var, or empty
 // string if not found.
 // Category: FCAT_UTIL | Safety: FSAFE_STATE
BValue builtin_environ(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 char namebuf[256];
 const char *val;
 char *buf;
 int len;
 (void)argc;

 if (!bval_is_string(&args[0]))
 return bval_string(NULL, 0);
 len = args[0].v.sval.length;
 if (len > 254) len = 254;
 if (args[0].v.sval.data != NULL)
 memcpy(namebuf, args[0].v.sval.data, (size_t)len);
 namebuf[len] = '\0';

 val = getenv(namebuf);
 if (val == NULL)
 return bval_string(NULL, 0);

 len = (int)strlen(val);
 buf = strpool_alloc(&state->strpool, len);
 if (buf == NULL) return bval_string(NULL, 0);
 memcpy(buf, val, (size_t)len);
 return bval_string(buf, len);
}
