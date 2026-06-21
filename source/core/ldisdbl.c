/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: ldisdbl.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Core interpreter engine infrastructure, memory pool allocator, error model, values, platform, security gating, and boot configurations.
 *
 * 2. WHAT TO EXPECT:
 *    Fixed memory footprint utilizing compile-time pool allocators (defined in config.h). Avoids malloc/free at runtime.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Diagnostic logs, specific error message phrasing, platform detection strings, security sandbox policy matrices.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    BValue tagged union structure fields, core memory allocator logic, security capability ratings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Check config.h pool sizes (e.g. increase PROGRAM_MEMORY_SIZE). If security level is ratcheted, check security level enforcement policies.
 * ===================================================================== */

// ---
// BASIC++ Interpreter - ldisdbl.c
// ---
//
// Stub for missing _LDisDouble_ in OpenWatcom v2 beta.
//
// PURPOSE:
//   OpenWatcom v2 (beta) has a bug where the linker expects a
//   function called _LDisDouble_() but the runtime library doesn't
//   provide it. This stub satisfies the linker without changing
//   any behavior -- it always returns 1 (long double IS double on
//   x86 platforms where OpenWatcom targets).
//
// MINIMALIZATION:
//   Only needed for OpenWatcom (FreeDOS) builds.
//   Safe to exclude from MSVC and GCC builds (they never call this).
//
// ---
#pragma aux _LDisDouble_ "*";
int _LDisDouble_(long double *ld)
{
    (void)ld;
    return 1;  // long double == double on OpenWatcom x86
}
