/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: platform.h
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
 // BASIC++ Interpreter - platform.h
 // ---
 //
 // Cross-platform abstraction layer.
 //
 // PURPOSE:
 // Provides compile-time platform detection and a runtime API
 // for querying platform info, compiler identity, and word sizes.
 // Enables BASIC programs to introspect the host environment
 // via the SYSTEM command.
 //
 // SUPPORTED PLATFORMS:
 // DOS - FreeDOS + OpenWatcom (16-bit large model)
 // Windows - Win32/Win64 + MSVC
 // Linux - x86/x86_64/ARM + GCC/Clang
 // POSIX - macOS, BSD, other Unix + GCC/Clang
 //
 // ---

#ifndef BASICPP_PLATFORM_H
#define BASICPP_PLATFORM_H

// --- Platform IDs ---
typedef enum PlatformId {
 PLAT_DOS = 0,
 PLAT_WINDOWS = 1,
 PLAT_LINUX = 2,
 PLAT_POSIX = 3, // macOS, BSD, other Unix
 PLAT_UNKNOWN = 4,
 PLAT_COUNT = 5
} PlatformId;

// --- Platform Info Descriptor ---
typedef struct PlatformInfo {
 PlatformId id;
 const char *name; // "Windows", "FreeDOS", etc.
 const char *short_name; // "WIN", "DOS", "LNX", "UNX"
 const char *compiler; // "MSVC", "Watcom", "GCC", etc.
 const char *compiler_ver; // "19.x", "1.9", "13.x", etc.
 int ptr_size; // sizeof(void*)
 int int_size; // sizeof(int)
 int long_size; // sizeof(long)
} PlatformInfo;

// --- Platform API ---

 // platform_init - Detect and initialize platform info.
 //
 // Call once at boot before any platform queries.
void platform_init(void);

 // platform_get_info - Get the full platform descriptor.
 //
 // Returns a pointer to the static PlatformInfo struct.
const PlatformInfo *platform_get_info(void);

 // platform_name - Get the platform name.
 //
 // Returns "Windows", "FreeDOS", "Linux", "macOS", etc.
const char *platform_name(void);

 // platform_short_name - Get the 3-letter platform code.
 //
 // Returns "WIN", "DOS", "LNX", "UNX", or "UNK".
const char *platform_short_name(void);

 // platform_word_size - Get the word size in bits.
 //
 // Returns 16, 32, or 64 based on sizeof(void*).
int platform_word_size(void);

 // platform_print_info - Print full platform summary to stdout.
 //
 // Used by the SYSTEM command with no arguments.
void platform_print_info(void);

 // platform_print_memory - Print memory usage summary.
 //
 // Reports program store, variable, and string pool usage.
 // rt parameter is passed as void* to avoid circular includes.
void platform_print_memory(void *rt);

 // platform_list_env_user - List user-scope environment variables.
 //
 // On Windows: enumerates HKCU\Environment registry keys.
 // On POSIX: lists all environment variables (no user/system
 // distinction on Unix systems). Returns count printed.
int platform_list_env_user(void);

 // platform_list_env_system - List system-scope environment variables.
 //
 // On Windows: enumerates HKLM\...\Session Manager\Environment.
 // On POSIX: reports "not available" (no distinction).
 // Returns count printed.
int platform_list_env_system(void);

 // platform_list_env_all - List all process environment variables.
 //
 // Dumps the entire inherited environment block (merged user +
 // system + process). This is what VARS ENV uses.
 // Returns count printed.
int platform_list_env_all(void);

// platform_cleanup_logs - Clean up logs and test outputs.
void platform_cleanup_logs(int full_cleanup);

// platform_stdin_is_tty - Check if standard input is a terminal/TTY.
int platform_stdin_is_tty(void);

// platform_stdin_is_redirected - Check if stdin is redirected from file/pipe.
// Returns 1 if redirected, 0 if interactive terminal.
int platform_stdin_is_redirected(void);

// platform_kbhit - Non-blocking check if a key has been pressed.
// Returns 1 if a key is available, 0 otherwise.
int platform_kbhit(void);

// platform_getch - Read a single character without echo or line buffering.
// Blocks until a character is available.
int platform_getch(void);

// platform_nb_read_char - Non-blocking read of a single character from stdin.
// Returns the character code (0-255) or -1 if no character is available.
int platform_nb_read_char(void);

// platform_sleep_ms - Sleep for the specified number of milliseconds.
// Portable across Windows (Sleep) and POSIX (usleep/nanosleep).
void platform_sleep_ms(int duration_ms);

// platform_get_available_ram - Get currently available physical RAM.
long long platform_get_available_ram(void);

// platform_get_console_height - Query terminal/console height.
// Returns height in rows, or -1 if unable to determine.
int platform_get_console_height(void);

// platform_get_console_width - Query terminal/console width.
// Returns width in columns, or -1 if unable to determine.
int platform_get_console_width(void);

// plat_strdup - Portable string duplication avoiding MSVC deprecation warnings
char *plat_strdup(const char *s);

// Portable Directory Operations
int plat_mkdir(const char *path);
int plat_rmdir(const char *path);
int plat_chdir(const char *path);
char *plat_getcwd(char *buffer, int maxlen);

#endif // BASICPP_PLATFORM_H
