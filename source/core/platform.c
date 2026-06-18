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
//   BASIC++ 4.0.1
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
