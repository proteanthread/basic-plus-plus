// FILENAME: arrays.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libext
// NEEDS: libcore (arrays_internal.h)
// Provides core logic and interface definitions for arrays within BASIC++.
//
// ---- Includes ----

#include "runtime/arrays_internal.h"

// All array subsystem logic is decomposed into:
// - runtime/arrays/arr_core.c: Allocation, lifecycle, dimensioning, erasing, and bounds queries
// - runtime/arrays/arr_access.c: Multi-dimensional indexing, element lookup, and dynamic auto-expansion
// - runtime/arrays/arr_persist.c: Serialization, deserialization, chaining, and virtual arrays
