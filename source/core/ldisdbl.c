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
