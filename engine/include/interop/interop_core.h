// FILENAME: interop_core.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libcore (interop_core.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for interop_core within BASIC++.
//
// ---- Includes ----

#ifndef INTEROP_CORE_H
#define INTEROP_CORE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Forward declarations for all interop types
typedef uint64_t InteropHandle;
typedef struct InteropError InteropError;
typedef enum InteropValueType InteropValueType;
typedef struct InteropValue InteropValue;

// @brief Initialize the interop subsystem.
//
// @return 0 on success, non-zero error code on failure.
int interop_init(void);

// @brief Shutdown the interop subsystem.
void interop_shutdown(void);

// @brief Get the version string of the interop subsystem.
//
// @return Null-terminated version string.
const char* interop_version_string(void);

#endif // INTEROP_CORE_H
