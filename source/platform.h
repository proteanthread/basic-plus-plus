/*
 * ---
 * BASIC++ Interpreter - platform.h
 * ---
 *
 * Cross-platform abstraction layer.
 *
 * PURPOSE:
 * Provides compile-time platform detection and a runtime API
 * for querying platform info, compiler identity, and word sizes.
 * Enables BASIC programs to introspect the host environment
 * via the SYSTEM command.
 *
 * SUPPORTED PLATFORMS:
 * DOS - FreeDOS + OpenWatcom (16-bit large model)
 * Windows - Win32/Win64 + MSVC
 * Linux - x86/x86_64/ARM + GCC/Clang
 * POSIX - macOS, BSD, other Unix + GCC/Clang
 *
 * ---
 */

#ifndef BASICPP_PLATFORM_H
#define BASICPP_PLATFORM_H

/* --- Platform IDs ---
 */
typedef enum PlatformId {
 PLAT_DOS = 0,
 PLAT_WINDOWS = 1,
 PLAT_LINUX = 2,
 PLAT_POSIX = 3, /* macOS, BSD, other Unix */
 PLAT_UNKNOWN = 4,
 PLAT_COUNT = 5
} PlatformId;

/* --- Platform Info Descriptor ---
 */
typedef struct PlatformInfo {
 PlatformId id;
 const char *name; /* "Windows", "FreeDOS", etc. */
 const char *short_name; /* "WIN", "DOS", "LNX", "UNX" */
 const char *compiler; /* "MSVC", "Watcom", "GCC", etc. */
 const char *compiler_ver; /* "19.x", "1.9", "13.x", etc. */
 int ptr_size; /* sizeof(void*) */
 int int_size; /* sizeof(int) */
 int long_size; /* sizeof(long) */
} PlatformInfo;

/* --- Platform API ---
 */

/*
 * platform_init - Detect and initialize platform info.
 *
 * Call once at boot before any platform queries.
 */
void platform_init(void);

/*
 * platform_get_info - Get the full platform descriptor.
 *
 * Returns a pointer to the static PlatformInfo struct.
 */
const PlatformInfo *platform_get_info(void);

/*
 * platform_name - Get the platform name.
 *
 * Returns "Windows", "FreeDOS", "Linux", "macOS", etc.
 */
const char *platform_name(void);

/*
 * platform_short_name - Get the 3-letter platform code.
 *
 * Returns "WIN", "DOS", "LNX", "UNX", or "UNK".
 */
const char *platform_short_name(void);

/*
 * platform_word_size - Get the word size in bits.
 *
 * Returns 16, 32, or 64 based on sizeof(void*).
 */
int platform_word_size(void);

/*
 * platform_print_info - Print full platform summary to stdout.
 *
 * Used by the SYSTEM command with no arguments.
 */
void platform_print_info(void);

/*
 * platform_print_memory - Print memory usage summary.
 *
 * Reports program store, variable, and string pool usage.
 * rt parameter is passed as void* to avoid circular includes.
 */
void platform_print_memory(void *rt);

#endif /* BASICPP_PLATFORM_H */
