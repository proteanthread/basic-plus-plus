/*
 * =====================================================================
 * BASIC++ Interpreter - help.h
 * =====================================================================
 *
 * Help & introspection system.
 *
 * Provides built-in documentation so BASIC++ is self-describing.
 * The interpreter can explain its own commands, functions, and
 * system state without external documentation.
 *
 * =====================================================================
 */

#ifndef BASICPP_HELP_H
#define BASICPP_HELP_H

#include "runtime.h"

/*
 * help_show - Display help for a topic or list all commands.
 *
 * If topic is NULL, displays the command summary.
 * If topic is a valid keyword, shows detailed help.
 */
void help_show(const char *topic);

/*
 * help_info - Display system information.
 *
 * Shows version, dialect, security level, memory usage,
 * registered functions, program size, etc.
 */
void help_info(RuntimeState *rt);

/*
 * help_catalog - List all registered functions.
 *
 * Groups functions by category and shows name, arg count,
 * and return type for each.
 */
void help_catalog(void);

#endif /* BASICPP_HELP_H */
