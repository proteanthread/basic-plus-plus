/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: help.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Self-describing HELP interactive system, OPTION overrides, and security scope permissions.
 *
 * 2. WHAT TO EXPECT:
 *    Help prints keyword usage. Overrides rewrite token attributes at parse time.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Command summaries, help topics, scope presets mappings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Help databases lookup logic, keyword gating routines.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If help command fails, ensure help databases are sorted. Check override loop guards.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - help.h
 // ---
 //
 // Help & introspection system.
 //
 // Provides built-in documentation so BASIC++ is self-describing.
 // The interpreter can explain its own commands, functions, and
 // system state without external documentation.
 //
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // ---

#ifndef BASICPP_HELP_H
#define BASICPP_HELP_H

#include "runtime.h"

 // help_show - Display help for a topic or list all commands.
 //
 // If topic is NULL, displays the command summary.
 // If topic is a valid keyword, shows detailed help.
void help_show(const char *topic);

 // help_info - Display system information.
 //
 // Shows version, dialect, security level, memory usage,
 // registered functions, program size, etc.
void help_info(RuntimeState *rt);

 // help_catalog - List all registered functions.
 //
 // Groups functions by category and shows name, arg count,
 // and return type for each.
void help_catalog(void);

#endif // BASICPP_HELP_H
