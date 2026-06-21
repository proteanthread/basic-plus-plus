/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: compat.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Utility or helper code for BASIC++ interpreter.
 *
 * 2. WHAT TO EXPECT:
 *    Executes with low overhead, relying on fixed compile-time limits and memory pools.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Internal helper functions, optimization passes, or local naming adjustments.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Public API structures, parameter contracts, or global type definitions.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check memory pool margins, look for segmentation faults, and trace parameter values via a debugger.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - compat.h
 // ---
 //
 // Platform compatibility shim.
 //
 // PURPOSE:
 // Provides portable wrappers for platform-specific functions.
 // With C17, snprintf/vsnprintf and long long are guaranteed by
 // the standard. The remaining shims handle MSVC-specific naming
 // differences (e.g., _strdup, _snprintf).
 //
 // ---

#ifndef BASICPP_COMPAT_H
#define BASICPP_COMPAT_H

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

// --- snprintf / vsnprintf ---
 // C17 guarantees snprintf and vsnprintf.
 // Use the standard names directly.
#define bpp_snprintf  snprintf
#define bpp_vsnprintf vsnprintf

// --- strdup portability ---
 // strdup is POSIX, not ISO C. MSVC names it _strdup.
#if defined(_MSC_VER)
  #define bpp_strdup _strdup
#else
  #define bpp_strdup strdup
#endif

// --- long long availability ---
 // C17 guarantees long long (at least 64-bit).
#define BPP_HAS_LONGLONG 1

#endif // BASICPP_COMPAT_H
