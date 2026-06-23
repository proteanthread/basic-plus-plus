/**
 * Original GW-BASIC Interpreter Port (C17)
 * 
 * -----------------------------------------------------------------------------
 * MAINTENANCE & EXTENSION GUIDELINES
 * -----------------------------------------------------------------------------
 * 1. WHAT CAN BE CHANGED:
 *    - Console sizing, output formatting, or ANSI color/escape sequences.
 *    - Logic inside statement handlers to optimize standard BASIC behaviors.
 *    - Math functions (tuning logic for trigonometric or random values).
 * 
 * 2. WHAT CANNOT BE CHANGED:
 *    - Keyword/token byte mapping tables (essential for loading tokenized BAS binaries).
 *    - Segmented memory layout simulation structures.
 *    - Core mathematical parsing precedence chain (eval descent hierarchy).
 * 
 * 3. EXPECTED BEHAVIOR:
 *    - Interface definitions for registering and querying pluggable interpreter modules.
 *    - External module API registration interfaces and runtime dispatcher.
 * 
 * 4. WHAT TO DO IF SOMETHING BREAKS:
 *    - Check variable tables, default variable type states, and stack pointers.
 *    - Cross-reference token layouts with original detokenization specifications.
 *    - Run diagnostic verification script to identify isolated error line numbers.
 * -----------------------------------------------------------------------------
 */
#ifndef GW_PLUGIN_H
#define GW_PLUGIN_H

#include <stdint.h>

typedef struct GW_PluginManager GW_PluginManager;

// Plugin function signature:
//   num_args  - number of arguments passed
//   args      - array of pointers to the arguments
//   arg_types - array of types (0=int16, 1=float, 2=double, 3=string)
typedef void (*GW_PluginFunc)(int num_args, void **args, int *arg_types);

GW_PluginManager *gw_plugin_init(void);
void gw_plugin_cleanup(GW_PluginManager *pm);

// Load native plugin (.dll on Windows, .so on Linux)
// Returns positive handle index on success, -1 on failure
int gw_plugin_load(GW_PluginManager *pm, const char *path);

// Unload native plugin
void gw_plugin_unload(GW_PluginManager *pm, int handle);

// Resolve and execute a subroutine in the plugin
int gw_plugin_call(GW_PluginManager *pm, int handle, const char *func_name, int num_args, void **args, int *arg_types);

#endif // GW_PLUGIN_H
