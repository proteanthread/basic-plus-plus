/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: platform.c
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
// BASIC++ Interpreter - platform.c
// ---
//
// Cross-platform backend implementation.
//
// PURPOSE:
//   Abstracts platform-specific details (OS, compiler, word size,
//   environment variables) behind a unified API. All platform
//   queries go through this module so that the rest of the
//   interpreter never needs to use #ifdef directly.
//
// HOW IT WORKS:
//   At compile time, preprocessor chains detect the platform and
//   compiler, setting DETECTED_PLATFORM, DETECTED_NAME, etc.
//   At runtime, platform_init() fills a static PlatformInfo struct
//   with these values plus runtime-measured sizes (ptr, int, long).
//   All subsequent queries are simple struct reads -- zero overhead.
//
//   Platform detection order:
//     1. DOS   -> __MSDOS__, __DOS__, MSDOS
//     2. Windows -> _WIN32, _WIN64
//     3. Linux -> __linux__, __linux
//     4. POSIX -> __unix__, __APPLE__, __MACH__
//     5. Unknown (fallback)
//
//   Compiler detection order:
//     1. OpenWatcom -> __WATCOMC__
//     2. MSVC -> _MSC_VER
//     3. Clang -> __clang__
//     4. GCC -> __GNUC__
//     5. TCC -> __TINYC__
//     6. Unknown (fallback)
//
// HOW TO EXTEND / CUSTOMIZE:
//   Adding support for a new platform (e.g., Haiku, RISC-V bare-metal):
//   1. Add a new PLAT_xxx constant to the PlatformID enum in platform.h.
//   2. Add a #elif block below for the platform's preprocessor macros.
//   3. Set DETECTED_PLATFORM, DETECTED_NAME, and DETECTED_SHORT.
//   4. If the platform needs special initialization (e.g., terminal
//      setup), add it to platform_init() behind an #ifdef.
//   5. Add any platform-specific library includes in the appropriate
//      #ifdef blocks (e.g., <sys/ioctl.h> for terminal queries).
//
//   Adding support for a new compiler:
//   1. Add a #elif block in the compiler detection section.
//   2. Set DETECTED_COMPILER to the compiler name.
//   3. Set COMPILER_VER_MAJOR and COMPILER_VER_MINOR macros.
//
// FINE-TUNING:
//   - The PlatformInfo struct is populated once at init and never
//     changes. All queries are O(1).
//   - Environment variable listing uses platform-native APIs:
//     Windows -> Registry (user/system distinction)
//     POSIX   -> environ array (no user/system distinction)
//
// TROUBLESHOOTING:
//   - "Platform: Unknown":
//     Your compiler's preprocessor macros are not recognized.
//     Add a detection block for your platform (see above).
//
//   - "Compiler: Unknown":
//     Same issue -- add a compiler detection block.
//
//   - Environment listing shows "(unable to read registry)":
//     On Windows, the user lacks read access to the registry
//     key HKCU\Environment or HKLM\SYSTEM\...\Environment.
//     Run as administrator or check security policy.
//
// PERFORMANCE:
//   - platform_init():      O(1) -- fills struct fields.
//   - All query functions:  O(1) -- struct field reads.
//   - platform_list_env_*:  O(n) -- iterates environment entries.
//     Windows registry reads are slightly slower than POSIX environ.
//
// MINIMALIZATION:
//   For a minimal build (FreeDOS, embedded):
//   - The core detection (platform_init, platform_name, etc.) is
//     required (CORE tier, ~1 KB).
//   - Environment listing can be removed entirely for embedded.
//     Delete platform_list_env_* and the Windows registry code
//     to save ~2 KB code + Windows header overhead.
//
// DEPENDENCIES:
//   - platform.h (PlatformInfo struct, PlatformID enum)
//   - config.h   (BASICPP_NAME, BASICPP_VERSION)
//   - runtime.h  (RuntimeState for memory info display)
//   - module.h   (module_count for system info display)
//   - security.h (security_level_name for system info display)
//   - Windows:   windows.h (registry API for env listing)
//   - POSIX:     extern char **environ
//
// C17 COMPLIANCE:
//   - Uses only C17 standard library + platform extensions.
//   - Windows: #include <windows.h> with TokenType collision guard.
//   - POSIX: extern char **environ (POSIX.1 standard).
//   - Compiles cleanly on MSVC /std:c17 /W3 and gcc -std=c17 -Wall.
//
// ---

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "config.h"
#include "runtime.h"
#include "module.h"
#include "security.h"
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define TokenType WinTokenType
#include <windows.h>
#undef TokenType
#include <io.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

// -----------------------------------------------------------------
// Compile-Time Platform Detection
// -----------------------------------------------------------------
// These preprocessor blocks detect the target platform and set
// three macros:
//   DETECTED_PLATFORM -- PlatformID enum value
//   DETECTED_NAME     -- human-readable name (e.g., "Windows x64")
//   DETECTED_SHORT    -- 3-letter abbreviation (e.g., "WIN")
// -----------------------------------------------------------------

#if defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
    #define DETECTED_PLATFORM PLAT_DOS
    #define DETECTED_NAME "FreeDOS"
    #define DETECTED_SHORT "DOS"
#elif defined(_WIN64)
    #define DETECTED_PLATFORM PLAT_WINDOWS
    #define DETECTED_NAME "Windows x64"
    #define DETECTED_SHORT "WIN"
#elif defined(_WIN32)
    #define DETECTED_PLATFORM PLAT_WINDOWS
    #define DETECTED_NAME "Windows"
    #define DETECTED_SHORT "WIN"
#elif defined(__linux__) || defined(__linux)
    #define DETECTED_PLATFORM PLAT_LINUX
    #define DETECTED_NAME "Linux"
    #define DETECTED_SHORT "LNX"
#elif defined(__APPLE__) && defined(__MACH__)
    #define DETECTED_PLATFORM PLAT_POSIX
    #define DETECTED_NAME "macOS"
    #define DETECTED_SHORT "UNX"
#elif defined(__unix__) || defined(__unix)
    #define DETECTED_PLATFORM PLAT_POSIX
    #define DETECTED_NAME "Unix"
    #define DETECTED_SHORT "UNX"
#else
    #define DETECTED_PLATFORM PLAT_UNKNOWN
    #define DETECTED_NAME "Unknown"
    #define DETECTED_SHORT "UNK"
#endif

// -----------------------------------------------------------------
// Compile-Time Compiler Detection
// -----------------------------------------------------------------
// Detects the compiler name and version for display in SYSTEM command.
// Order matters: Clang defines __GNUC__ too, so check Clang first.
// -----------------------------------------------------------------

#if defined(__WATCOMC__)
    #define DETECTED_COMPILER "OpenWatcom"
#elif defined(_MSC_VER)
    #define DETECTED_COMPILER "MSVC"
#elif defined(__clang__)
    #define DETECTED_COMPILER "Clang"
#elif defined(__GNUC__)
    #define DETECTED_COMPILER "GCC"
#elif defined(__TINYC__)
    #define DETECTED_COMPILER "TCC"
#else
    #define DETECTED_COMPILER "Unknown"
#endif

// Compiler version numbers (best-effort extraction)
#if defined(_MSC_VER)
    #define COMPILER_VER_MAJOR (_MSC_VER / 100)
    #define COMPILER_VER_MINOR (_MSC_VER % 100)
#elif defined(__GNUC__)
    #define COMPILER_VER_MAJOR __GNUC__
    #define COMPILER_VER_MINOR __GNUC_MINOR__
#elif defined(__clang_major__)
    #define COMPILER_VER_MAJOR __clang_major__
    #define COMPILER_VER_MINOR __clang_minor__
#elif defined(__WATCOMC__)
    #define COMPILER_VER_MAJOR (__WATCOMC__ / 100)
    #define COMPILER_VER_MINOR (__WATCOMC__ % 100)
#else
    #define COMPILER_VER_MAJOR 0
    #define COMPILER_VER_MINOR 0
#endif

// -----------------------------------------------------------------
// Static Platform Info
// -----------------------------------------------------------------
// Populated once by platform_init(). All subsequent queries
// read from this struct -- zero overhead.
// -----------------------------------------------------------------

static PlatformInfo plat_info;
static char compiler_ver_str[16];

// -----------------------------------------------------------------
// Public API
// -----------------------------------------------------------------

// platform_init - Detect platform and compiler at startup.
//
// Populates the static PlatformInfo struct with compile-time
// detected values and runtime-measured type sizes. Called once
// during interpreter boot (from boot.c).
//
// After this call, platform_name(), platform_get_info(), etc.
// are all available for use.
//
void platform_init(void)
{
    plat_info.id = DETECTED_PLATFORM;
    plat_info.name = DETECTED_NAME;
    plat_info.short_name = DETECTED_SHORT;
    plat_info.compiler = DETECTED_COMPILER;

    // Format compiler version as "major.minor" string
    {
        int major = COMPILER_VER_MAJOR;
        int minor = COMPILER_VER_MINOR;
        sprintf(compiler_ver_str, "%d.%d", major, minor);
    }
    plat_info.compiler_ver = compiler_ver_str;

    // Measure actual type sizes at runtime
    // (these could differ from expectations on exotic platforms)
    plat_info.ptr_size = (int)sizeof(void *);
    plat_info.int_size = (int)sizeof(int);
    plat_info.long_size = (int)sizeof(long);

#ifdef _WIN32
    // Enable ANSI escape sequence processing on Windows Console/Terminal
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE && hOut != NULL) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

// platform_get_info - Return the full PlatformInfo struct.
//
// Returns a const pointer to the static struct. Valid for the
// lifetime of the process. Never returns NULL.
//
const PlatformInfo *platform_get_info(void)
{
    return &plat_info;
}

// platform_name - Return the human-readable platform name.
//
// Examples: "Windows x64", "Linux", "FreeDOS", "macOS"
//
const char *platform_name(void)
{
    return plat_info.name;
}

// platform_short_name - Return the 3-letter platform abbreviation.
//
// Examples: "WIN", "LNX", "DOS", "UNX"
// Used in compact displays and the SYSTEM$ function.
//
const char *platform_short_name(void)
{
    return plat_info.short_name;
}

// platform_word_size - Return the native word size in bits.
//
// Returns pointer size x 8 (e.g., 64 for 64-bit, 32 for 32-bit).
// Used by the SYSTEM command and for display purposes.
//
int platform_word_size(void)
{
    return plat_info.ptr_size * 8;
}

// platform_print_info - Print a summary of platform, compiler,
// and interpreter information.
//
// Called by the SYSTEM command. Output example:
//   Platform: Windows x64 (WIN)
//   Compiler: MSVC 19.50
//   Word size: 64-bit (ptr=8 int=4 long=4)
//   BASIC++ 4.1.2
//   Security: STANDARD
//   Modules: 6 registered
//
void platform_print_info(void)
{
    printf("Platform: %s (%s)\n",
        plat_info.name, plat_info.short_name);
    printf("Compiler: %s %s\n",
        plat_info.compiler, plat_info.compiler_ver);
    printf("Word size: %d-bit (ptr=%d int=%d long=%d)\n",
        platform_word_size(),
        plat_info.ptr_size,
        plat_info.int_size,
        plat_info.long_size);
    printf("%s %s\n", BASICPP_NAME, BASICPP_VERSION);
    printf("Security: %s\n",
        security_level_name(security_get_level()));
    printf("Modules: %d registered\n", module_count());
}

// platform_print_memory - Print runtime memory usage information.
//
// Displays program line count, variable slot count, and string
// pool usage. Called by the MEMORY command.
//
// Parameters:
//   rt_ptr - pointer to RuntimeState (void* to avoid header dependency)
//
void platform_print_memory(void *rt_ptr)
{
    RuntimeState *rt = (RuntimeState *)rt_ptr;

    if (!rt || !rt->program || !rt->memory) {
        printf("Memory info unavailable.\n");
        return;
    }

    printf("Program: %d/%d lines\n",
        rt->program->count,
        rt->program->capacity);
    printf("Variables: %d slots\n",
        MAX_VARIABLES);
    printf("Strings: %ld/%ld bytes\n",
        rt->strpool.used,
        rt->strpool.size);
}

// =================================================================
// Environment Variable Listing
// =================================================================
//
// Three functions to list environment variables by scope:
//   platform_list_env_user   -- User-scope only
//   platform_list_env_system -- System-scope only
//   platform_list_env_all    -- Entire process environment
//
// On Windows, user and system env vars live in separate registry
// hives (HKCU\Environment and HKLM\SYSTEM\...\Environment).
// On POSIX, there is no distinction -- the process inherits a
// merged environment from the login shell.
//
// How to customize:
//   To add environment variable SETTING (not just listing):
//   Add a platform_set_env() function using:
//     Windows: RegSetValueExA() for persistent, _putenv() for process
//     POSIX:   setenv() for process, or write to ~/.bashrc for persistent
//
// Security note:
//   These functions only READ environment variables. Writing is
//   gated by SECOP_ENV_SET in the security system.
// =================================================================

#ifdef _WIN32

// Collision guard: winnt.h defines 'TokenType' which conflicts
// with the interpreter's TokenType in lexer.h.
#define TokenType WinTokenType
#include <windows.h>
#include "../console.h"
#undef TokenType

// plat_list_registry_env - Enumerate registry key values.
//
// Reads all REG_SZ and REG_EXPAND_SZ values from a registry key
// and prints them as "NAME=VALUE" lines.
//
// Parameters:
//   root   - registry root (HKEY_CURRENT_USER or HKEY_LOCAL_MACHINE)
//   subkey - registry subkey path
//
// Returns:
//   Number of values printed.
//
static int plat_list_registry_env(HKEY root, const char *subkey)
{
    HKEY hKey;
    DWORD index = 0;
    char name_buf[256];
    char data_buf[4096];
    DWORD name_len, data_len, type;
    int count = 0;
    LONG rc;

    rc = RegOpenKeyExA(root, subkey, 0, KEY_READ, &hKey);
    if (rc != ERROR_SUCCESS) {
        printf("  (unable to read registry)\n");
        return 0;
    }

    while (1) {
        name_len = sizeof(name_buf);
        data_len = sizeof(data_buf);
        rc = RegEnumValueA(hKey, index, name_buf,
            &name_len, NULL, &type,
            (LPBYTE)data_buf, &data_len);
        if (rc != ERROR_SUCCESS) break;

        if (type == REG_SZ || type == REG_EXPAND_SZ) {
            printf(" %s=%s\n", name_buf, data_buf);
            count++;
        }
        index++;
    }

    RegCloseKey(hKey);
    return count;
}

// platform_list_env_user - List user-scope environment variables.
//
// On Windows, reads from HKCU\Environment registry key.
// These are the variables set via System Properties -> Environment.
//
int platform_list_env_user(void)
{
    return plat_list_registry_env(HKEY_CURRENT_USER,
        "Environment");
}

// platform_list_env_system - List system-scope environment variables.
//
// On Windows, reads from HKLM\SYSTEM\CurrentControlSet\Control\
// Session Manager\Environment. Requires read access (usually
// available to all users).
//
int platform_list_env_system(void)
{
    return plat_list_registry_env(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\"
        "Session Manager\\Environment");
}

#else // POSIX

// platform_list_env_user - List all environment variables (POSIX).
//
// On POSIX systems, there is no user/system distinction.
// The entire process environment is listed as "user" scope.
//
int platform_list_env_user(void)
{
    extern char **environ;
    char **envp = environ;
    int count = 0;

    if (envp != NULL) {
        while (*envp != NULL) {
            printf(" %s\n", *envp);
            envp++;
            count++;
        }
    }
    return count;
}

// platform_list_env_system - No-op on POSIX.
//
// POSIX has no separate system-scope environment.
// Returns 0 and prints a message explaining why.
//
int platform_list_env_system(void)
{
    printf("  (no user/system distinction on POSIX)\n");
    return 0;
}

#endif // _WIN32 / POSIX

// platform_list_env_all - List the entire process environment.
//
// Iterates the process environ array and prints each entry.
// Works identically on Windows and POSIX.
//
// Note on Windows: MSVC uses _environ, OpenWatcom uses environ.
// Both are declared in stdlib.h.
//
int platform_list_env_all(void)
{
    int count = 0;
#ifdef _WIN32
  #ifdef __WATCOMC__
    // OpenWatcom: 'environ' declared in stdlib.h
    char **envp = environ;
  #else
    // MSVC: '_environ' declared in stdlib.h
    char **envp = _environ;
  #endif
#else
    extern char **environ;
    char **envp = environ;
#endif

    if (envp != NULL) {
        while (*envp != NULL) {
            printf(" %s\n", *envp);
            envp++;
            count++;
        }
    }
    return count;
}

static int plat_str_eq_nocase(const char *s1, const char *s2) {
    if (!s1 || !s2) return 0;
    while (*s1 && *s2) {
        char c1 = *s1;
        char c2 = *s2;
        if (c1 >= 'A' && c1 <= 'Z') c1 = (char)(c1 + 32);
        if (c2 >= 'A' && c2 <= 'Z') c2 = (char)(c2 + 32);
        if (c1 != c2) return 0;
        s1++;
        s2++;
    }
    return (*s1 == '\0' && *s2 == '\0');
}

#ifdef _WIN32
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
    char name[260];
    time_t mtime;
    int is_log;
    int is_output;
} CleanupFile;

void platform_cleanup_logs(int full_cleanup)
{
    static CleanupFile files[1000];
    int count = 0;
    struct _finddata_t data;
    intptr_t handle;

    const char *wildcards[] = {
        "*.LOG", "*.log", "*.OUT", "*.out", "out*.txt", "test_out*.txt",
        "stub_module.*", "temp_test.bas", "temp_input.bas"
    };

    for (int i = 0; i < (int)(sizeof(wildcards)/sizeof(wildcards[0])); i++) {
        handle = _findfirst(wildcards[i], &data);
        if (handle != -1) {
            do {
                if (data.attrib & _A_SUBDIR) continue;
                
                const char *ext = strrchr(data.name, '.');
                if (ext != NULL && (plat_str_eq_nocase(ext, ".c") || plat_str_eq_nocase(ext, ".h"))) {
                    continue;
                }

                int dup = 0;
                for (int d = 0; d < count; d++) {
                    if (strcmp(files[d].name, data.name) == 0) {
                        dup = 1;
                        break;
                    }
                }
                if (dup) continue;

                if (count < 1000) {
                    strncpy(files[count].name, data.name, 255);
                    files[count].name[255] = '\0';
                    files[count].mtime = data.time_write;
                    
                    files[count].is_log = 0;
                    files[count].is_output = 0;
                    if (ext != NULL && (plat_str_eq_nocase(ext, ".log") || plat_str_eq_nocase(ext, ".LOG"))) {
                        files[count].is_log = 1;
                    }
                    if (ext != NULL && (plat_str_eq_nocase(ext, ".txt") || plat_str_eq_nocase(ext, ".TXT")) && 
                        (strncmp(data.name, "out", 3) == 0 || strncmp(data.name, "test_out", 8) == 0)) {
                        files[count].is_output = 1;
                    }
                    if (ext != NULL && (plat_str_eq_nocase(ext, ".out") || plat_str_eq_nocase(ext, ".OUT"))) {
                        files[count].is_output = 1;
                    }
                    count++;
                }
            } while (_findnext(handle, &data) == 0);
            _findclose(handle);
        }
    }

    int recent_log_idx = -1;
    time_t max_log_time = 0;
    int recent_output_idx = -1;
    time_t max_output_time = 0;

    for (int i = 0; i < count; i++) {
        if (files[i].is_log) {
            if (files[i].mtime > max_log_time) {
                max_log_time = files[i].mtime;
                recent_log_idx = i;
            }
        }
        if (files[i].is_output) {
            if (files[i].mtime > max_output_time) {
                max_output_time = files[i].mtime;
                recent_output_idx = i;
            }
        }
    }

    int deleted = 0;
    for (int i = 0; i < count; i++) {
        int keep = 0;
        if (!full_cleanup) {
            if (i == recent_log_idx) keep = 1;
            if (i == recent_output_idx) keep = 1;
        }
        if (!keep) {
            if (remove(files[i].name) == 0) {
                deleted++;
            }
        }
    }
    printf("Clean-up finished. Removed %d files.\n", deleted);
    if (!full_cleanup) {
        if (recent_log_idx != -1) printf("Preserved most recent log: %s\n", files[recent_log_idx].name);
        if (recent_output_idx != -1) printf("Preserved most recent test output: %s\n", files[recent_output_idx].name);
    }
}
#else // POSIX (e.g. Linux, Unix, macOS)
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct {
    char name[260];
    time_t mtime;
    int is_log;
    int is_output;
} CleanupFile;

void platform_cleanup_logs(int full_cleanup)
{
    static CleanupFile files[1000];
    int count = 0;
    DIR *dir = opendir(".");
    if (!dir) return;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {
        const char *name = entry->d_name;
        struct stat st;
        if (stat(name, &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) continue;

        const char *ext = strrchr(name, '.');
        if (ext == NULL) continue;

        int match = 0;
        int is_log = 0;
        int is_output = 0;

        if (plat_str_eq_nocase(ext, ".log") || plat_str_eq_nocase(ext, ".LOG")) {
            match = 1;
            is_log = 1;
        } else if (plat_str_eq_nocase(ext, ".out") || plat_str_eq_nocase(ext, ".OUT")) {
            match = 1;
            is_output = 1;
        } else if ((plat_str_eq_nocase(ext, ".txt") || plat_str_eq_nocase(ext, ".TXT")) && 
                   (strncmp(name, "out", 3) == 0 || strncmp(name, "test_out", 8) == 0)) {
            match = 1;
            is_output = 1;
        } else if (strncmp(name, "stub_module.", 12) == 0 && !plat_str_eq_nocase(ext, ".c") && !plat_str_eq_nocase(ext, ".h")) {
            match = 1;
        } else if (strcmp(name, "temp_test.bas") == 0 || strcmp(name, "temp_input.bas") == 0) {
            match = 1;
        }

        if (match) {
            int dup = 0;
            for (int d = 0; d < count; d++) {
                if (strcmp(files[d].name, name) == 0) {
                    dup = 1;
                    break;
                }
            }
            if (dup) continue;

            if (count < 1000) {
                strncpy(files[count].name, name, 255);
                files[count].name[255] = '\0';
                files[count].mtime = st.st_mtime;
                files[count].is_log = is_log;
                files[count].is_output = is_output;
                count++;
            }
        }
    }
    closedir(dir);

    int recent_log_idx = -1;
    time_t max_log_time = 0;
    int recent_output_idx = -1;
    time_t max_output_time = 0;

    for (int i = 0; i < count; i++) {
        if (files[i].is_log) {
            if (files[i].mtime > max_log_time) {
                max_log_time = files[i].mtime;
                recent_log_idx = i;
            }
        }
        if (files[i].is_output) {
            if (files[i].mtime > max_output_time) {
                max_output_time = files[i].mtime;
                recent_output_idx = i;
            }
        }
    }

    int deleted = 0;
    for (int i = 0; i < count; i++) {
        int keep = 0;
        if (!full_cleanup) {
            if (i == recent_log_idx) keep = 1;
            if (i == recent_output_idx) keep = 1;
        }
        if (!keep) {
            if (remove(files[i].name) == 0) {
                deleted++;
            }
        }
    }
    printf("Clean-up finished. Removed %d files.\n", deleted);
    if (!full_cleanup) {
        if (recent_log_idx != -1) printf("Preserved most recent log: %s\n", files[recent_log_idx].name);
        if (recent_output_idx != -1) printf("Preserved most recent test output: %s\n", files[recent_output_idx].name);
    }
}
#endif

#if defined(_WIN32) || defined(_WIN64)
long long platform_get_available_ram(void) {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex)) {
        return (long long)statex.ullAvailPhys;
    }
    return 16LL * 1024LL * 1024LL;
}
#elif defined(__linux__) || defined(__linux)
#include <sys/sysinfo.h>
long long platform_get_available_ram(void) {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return (long long)info.freeram * info.mem_unit;
    }
    return 16LL * 1024LL * 1024LL;
}
#else
long long platform_get_available_ram(void) {
    return 512L * 1024L;
}
#endif

char *plat_strdup(const char *s)
{
    if (s == NULL) return NULL;
#ifdef _MSC_VER
    return _strdup(s);
#else
    return strdup(s);
#endif
}

int platform_stdin_is_tty(void)
{
#ifdef _WIN32
    return _isatty(_fileno(stdin));
#else
    return isatty(fileno(stdin));
#endif
}

// platform_stdin_is_redirected - Check if stdin is redirected from
// file or pipe. Returns 1 if redirected, 0 if interactive terminal.
//
// On Windows, uses GetStdHandle + GetConsoleMode to detect
// non-console redirected stdin (files and pipes).
// On POSIX, uses isatty + fstat to detect regular files and FIFOs.
int platform_stdin_is_redirected(void)
{
#ifdef _WIN32
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != NULL && hIn != INVALID_HANDLE_VALUE) {
        DWORD mode;
        if (!GetConsoleMode(hIn, &mode)) {
            DWORD type = GetFileType(hIn);
            if (type == FILE_TYPE_DISK || type == FILE_TYPE_PIPE) {
                return 1;
            }
        }
    }
    return 0;
#else
    if (!isatty(fileno(stdin))) {
        struct stat st;
        if (fstat(fileno(stdin), &st) == 0) {
            if (S_ISREG(st.st_mode) || S_ISFIFO(st.st_mode)) {
                return 1;
            }
        }
    }
    return 0;
#endif
}

// platform_kbhit - Non-blocking check if a key has been pressed.
// Returns 1 if a key is available, 0 otherwise.
//
// On Windows, wraps _kbhit() from <conio.h>.
// On POSIX, uses select() with zero timeout on STDIN_FILENO
// after setting the terminal to raw mode (no canonical / no echo).
int platform_kbhit(void)
{
#ifdef _WIN32
    return _kbhit();
#else
    struct termios oldt, newt;
    int result = 0;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    fd_set rfds;
    struct timeval tv = {0, 0};
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);
    result = (select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv) > 0) ? 1 : 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return result;
#endif
}

// platform_getch - Read a single character without echo or line
// buffering. Blocks until a character is available.
//
// On Windows, wraps _getch() from <conio.h>.
// On POSIX, sets terminal to raw mode, reads one byte, restores.
int platform_getch(void)
{
#ifdef _WIN32
    return _getch();
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

// platform_nb_read_char - Non-blocking read of a single character
// from stdin. Returns the character code (0-255) or -1 if none.
//
// On Windows, checks handle type (file, pipe, char) and uses
// the appropriate non-blocking read method.
// On POSIX, uses select() + read() with zero timeout.
int platform_nb_read_char(void)
{
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD fileType = GetFileType(hStdin);
    if (fileType == FILE_TYPE_DISK) {
        int ch = fgetc(stdin);
        if (ch != EOF) return ch;
    } else if (fileType == FILE_TYPE_PIPE || fileType == FILE_TYPE_CHAR ||
               fileType == FILE_TYPE_UNKNOWN) {
        DWORD bytesAvail = 0;
        if (PeekNamedPipe(hStdin, NULL, 0, NULL, &bytesAvail, NULL) &&
            bytesAvail > 0) {
            char ch = 0;
            DWORD bytesRead = 0;
            if (ReadFile(hStdin, &ch, 1, &bytesRead, NULL) &&
                bytesRead > 0) {
                return (unsigned char)ch;
            }
        }
    }
    return -1;
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    char ch = 0;
    fd_set rfds;
    struct timeval tv = {0, 0};
    FD_ZERO(&rfds);
    FD_SET(STDIN_FILENO, &rfds);

    int result = -1;
    if (select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv) > 0) {
        if (read(STDIN_FILENO, &ch, 1) == 1) {
            result = (unsigned char)ch;
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return result;
#endif
}

// platform_sleep_ms - Sleep for the specified number of milliseconds.
// Portable across Windows (Sleep) and POSIX (usleep).
//
// Clamps duration to [1, 30000] ms range for safety.
void platform_sleep_ms(int duration_ms)
{
    if (duration_ms < 1) return;
    if (duration_ms > 30000) duration_ms = 30000;
#ifdef _WIN32
    Sleep((DWORD)duration_ms);
#else
    usleep((useconds_t)duration_ms * 1000);
#endif
}
