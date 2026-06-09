/*
 * =====================================================================
 * BASIC++ Interpreter - mod_stdlib.c
 * =====================================================================
 *
 * Standard Library Module.
 *
 * PURPOSE:
 * Wraps the existing built-in function registration (builtins.c)
 * as a formal module. This is the reference module demonstrating
 * the module system.
 *
 * The STDLIB module is auto-activated at boot, so all built-in
 * functions (ABS, SIN, LEN, LEFT$, etc.) remain available
 * without any user action.
 *
 * =====================================================================
 */

#include "mod_stdlib.h"
#include "builtins.h"
#include "module.h"
#include <stddef.h>

/*
 * stdlib_init - Initialize the standard library.
 *
 * Calls builtins_register() to register all built-in functions
 * in the function registry. This is the same call that was
 * previously made directly in main.c.
 */
static int stdlib_init(void *rt)
{
 (void)rt;
 builtins_register();
 return 0;
}

/*
 * Module descriptor for STDLIB.
 *
 * Class: MOD_LIBRARY (function library)
 * Capabilities: CAP_MATH | CAP_STRING (math + string functions)
 * Init: registers all built-in functions
 * Cleanup: none (functions persist in registry)
 */
static const ModuleInfo stdlib_module_info = {
 "STDLIB", /* name */
 "1.0", /* version */
 "Standard function library", /* description */
 MOD_LIBRARY, /* mod_class */
 CAP_MATH | CAP_STRING, /* capabilities */
 stdlib_init, /* init */
 NULL /* cleanup (none) */
};

/*
 * mod_stdlib_register - Register the STDLIB module.
 *
 * Called from main.c during boot. Does not activate - the
 * caller must also call module_activate("STDLIB", rt).
 */
void mod_stdlib_register(void)
{
 module_register(&stdlib_module_info);
}
