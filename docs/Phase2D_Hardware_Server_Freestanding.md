# Phase 2D: libhardware & libserver Freestanding Conversion Specification

## 1. Overview
Phase 2D completes the freestanding C17 architectural conversion for both the **Hardware Subsystem (`libhardware`)** and the **Cloud Server & Networking Subsystem (`libserver`)**. All standard C library allocations (`malloc`, `calloc`, `free`), memory manipulations (`memset`, `memcpy`, `memmove`), string operations (`strlen`, `strcpy`, `strcmp`, `strncpy`, `strcasecmp`), mathematical calls (`abs`, `cos`, `sin`, `sqrt`), and formatting functions (`snprintf`) have been migrated to the pure C17 freestanding Runtime Library (`runtime_*`) and the Hardware Abstraction Layer (`hal_get()->mem.*`, `hal_get()->io.*`, `hal_get()->time.*`).

---

## 2. Converted Components & Source Inventory

### 2.1 Hardware Subsystem (`libhardware`)
| Source File | Conversion Details |
|---|---|
| `engine/src/device/bgi/bgi_autodetect.c` | Freestanding C17 display hardware detection |
| `engine/src/device/bgi/bgi_core.c` | Framebuffer & VRAM management converted to `hal_get()->mem.alloc`/`free`, `runtime_memset`, `runtime_memcpy` |
| `engine/src/device/bgi/bgi_font.c` | Font metrics and glyph lookups converted to `runtime/string/strops.h` and `runtime/string/memops.h` |
| `engine/src/device/bgi/bgi_modes.c` | Video mode registry converted to `runtime_memset` and `runtime_snprintf` |
| `engine/src/device/bgi/bgi_palette.c` | Freestanding palette indexing and ARGB conversion |
| `engine/src/device/bgi/bgi_raster.c` | Primitives (lines, circles, floodfill) converted to `runtime_abs`, `runtime_cos`, `runtime_sin`, `runtime_sqrt`, `runtime_memcpy` |
| `engine/src/device/fujinet.c` | `N:`, `FUJI:`, `CLOCK:` virtual devices converted to HAL file I/O, HAL memory, and HAL epoch time |
| `engine/src/memory/segmented_mem.c` | Dynamic segment handle tables converted to `hal_get()->mem.alloc`/`free` and `runtime_memset`/`runtime_memcpy` |
| `engine/src/runtime/microplex.c` | String & bit multiplexing converted to `runtime/string/strops.h` |

### 2.2 Cloud Server & Networking Subsystem (`libserver`)
| Source File | Conversion Details |
|---|---|
| `engine/src/runtime/crypto.c` | FNV-1a, CRC64, MD5/SHA256 digests, and LZ77 compress/decompress converted to `runtime_*` and `hal_get()->mem.alloc`/`free` |
| `engine/src/runtime/gemini.c` | Gemini protocol fetcher converted to `hal_get()->mem.alloc` and `runtime_snprintf` |
| `engine/src/runtime/task.c` | Background task manager converted to `runtime_memset` and `runtime_snprintf` |
| `engine/src/runtime/vfs.c` | Virtual File System mounts and path resolvers converted to `runtime_*` and `hal_get()->mem.alloc`/`free` |
| `engine/src/runtime/vnet.c` | Virtual network sockets and VDev tunnel converted to `runtime_*` and `hal_get()->mem.alloc`/`free` |
| `engine/src/module/regex.c` | Regular expression engine bridge converted to `runtime_memset`, `runtime_strlen`, and `hal_get()->mem.free` |

---

## 3. Verification and Quality Gates
1. **Zero Hosted Libc Dependencies**: Grep audit across all 15 source files confirmed 0 `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<math.h>`, or `<time.h>` inclusions.
2. **Dedicated Unit Test Suites**:
   - `tests/hardware_freestanding_test.c`: Validates BGI modes, rasterizer, clipping, floodfill, font metrics, segmented memory, microplex, and FujiNet virtual devices.
   - `tests/server_freestanding_test.c`: Validates crypto hashes, LZ77 roundtrip compression, VFS mount/resolve, task manager lifecycle, and VNet channels.
3. **Master Regression Coverage**:
   - `tests/qb_vbdos_master.bas`: 10/10 packages PASSED (100%).
   - `tests/vintage_ecosystems_master.bas`: 14/14 packages PASSED (100%).
   - `tests/vintage_deep_fuzz_stress.bas`: 8/8 tests PASSED (100%).
