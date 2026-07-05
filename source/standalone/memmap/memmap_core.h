/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: memmap_core.h
 * Subsystem: Virtual Memory Layout Presets
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Initializes 64K segment presets for C64, Atari, DOS, etc.
 *
 * 2. WHAT TO EXPECT:
 *    Fills virtual RAM with system variables and screen characters.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Initial maps data.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Memory segment mapping bounds.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If RAM fills incorrectly, verify preset startup addresses.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE MEMMAP PRESET REGISTRY CORE
 * File: memmap_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_MEMMAP_CORE_H
#define BASICPP_STANDALONE_MEMMAP_CORE_H

#include "../../memmap.h"

void memmap_core_init(unsigned char *mem, MemMapType type);
const char *memmap_core_get_name(MemMapType type);
MemMapType memmap_core_from_string(const char *name, int len);
MemMapType memmap_core_default_for_dialect(int dialect_id);

#endif // BASICPP_STANDALONE_MEMMAP_CORE_H
