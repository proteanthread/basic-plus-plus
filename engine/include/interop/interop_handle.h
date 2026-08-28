// FILENAME: interop_handle.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bpp_api.h, interop_core.c, interop_handle.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for interop_handle within BASIC++.
//
// ---- Includes ----

#ifndef INTEROP_HANDLE_H
#define INTEROP_HANDLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t InteropHandle;

#define INTEROP_INVALID_HANDLE ((InteropHandle)0)

// @brief Initialize the handle table.
// @return 0 on success, non-zero on failure.
int interop_handle_table_init(void);

// @brief Shutdown the handle table.
void interop_handle_table_shutdown(void);

// @brief Create a new handle for a pointer.
// @param ptr The pointer to track.
// @return A new InteropHandle, or INTEROP_INVALID_HANDLE on failure.
InteropHandle interop_handle_create(void* ptr);

// @brief Resolve a handle back to its pointer.
// @param handle The handle to resolve.
// @return The original pointer, or NULL if invalid.
void* interop_handle_resolve(InteropHandle handle);

// @brief Release a handle (decrement refcount).
// @param handle The handle to release.
void interop_handle_release(InteropHandle handle);

// @brief Retain a handle (increment refcount).
// @param handle The handle to retain.
void interop_handle_retain(InteropHandle handle);

// @brief Check if a handle is currently valid.
// @param handle The handle to check.
// @return true if valid, false otherwise.
bool interop_handle_is_valid(InteropHandle handle);

#endif // INTEROP_HANDLE_H
