/*
 * ---
 * BASIC++ Interpreter - platform.c
 * ---
 *
 * Cross-platform backend implementation.
 *
 * IMPLEMENTATION:
 * Compile-time preprocessor chain detects platform and compiler.
 * A static PlatformInfo struct is populated once at init.
 * Runtime queries are simple struct reads - zero overhead.
 *
 * PLATFORM DETECTION ORDER:
 * 1. DOS - __MSDOS__, __DOS__, MSDOS
 * 2. Windows - _WIN32, _WIN64
 * 3. Linux - __linux__, __linux
 * 4. POSIX - __unix__, __APPLE__, __MACH__
 * 5. Unknown (fallback)
 *
 * ---
 */

#include <stdio.h>
#include "platform.h"
#include "config.h"
#include "runtime.h"
#include "module.h"
#include "security.h"

/* --- Compile-time Platform Detection ---
 */
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

/* --- Compile-time Compiler Detection ---
 */
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

/* Compiler version string (best-effort) */
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

/* --- Static Platform Info ---
 */
static PlatformInfo plat_info;
static char compiler_ver_str[16];

/* --- platform_init ---
 */
void platform_init(void)
{
 plat_info.id = DETECTED_PLATFORM;
 plat_info.name = DETECTED_NAME;
 plat_info.short_name = DETECTED_SHORT;
 plat_info.compiler = DETECTED_COMPILER;

 /* Format compiler version string */
 {
 int major = COMPILER_VER_MAJOR;
 int minor = COMPILER_VER_MINOR;
 sprintf(compiler_ver_str, "%d.%d", major, minor);
 }
 plat_info.compiler_ver = compiler_ver_str;

 plat_info.ptr_size = (int)sizeof(void *);
 plat_info.int_size = (int)sizeof(int);
 plat_info.long_size = (int)sizeof(long);
}

/* --- platform_get_info ---
 */
const PlatformInfo *platform_get_info(void)
{
 return &plat_info;
}

/* --- platform_name ---
 */
const char *platform_name(void)
{
 return plat_info.name;
}

/* --- platform_short_name ---
 */
const char *platform_short_name(void)
{
 return plat_info.short_name;
}

/* --- platform_word_size ---
 */
int platform_word_size(void)
{
 return plat_info.ptr_size * 8;
}

/* --- platform_print_info ---
 */
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

/* --- platform_print_memory ---
 */
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

/* ================================================================
 * ENVIRONMENT VARIABLE LISTING
 *
 * Three functions to list env vars by scope:
 *   platform_list_env_user   - User-scope only
 *   platform_list_env_system - System-scope only
 *   platform_list_env_all    - Entire process environment
 *
 * On Windows, user and system env vars live in separate registry
 * hives. On POSIX, there's no distinction - the process inherits
 * a merged environment from login/shell.
 * ================================================================ */

#ifdef _WIN32
#include <windows.h>

/*
 * Helper: enumerate registry key values and print as NAME=VALUE.
 * Returns the count of values printed.
 */
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

int platform_list_env_user(void)
{
    return plat_list_registry_env(HKEY_CURRENT_USER,
        "Environment");
}

int platform_list_env_system(void)
{
    return plat_list_registry_env(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Control\\"
        "Session Manager\\Environment");
}

#else /* POSIX */

int platform_list_env_user(void)
{
    /* POSIX has no user/system distinction.
     * List entire environ as "user" scope. */
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

int platform_list_env_system(void)
{
    printf("  (no user/system distinction on POSIX)\n");
    return 0;
}

#endif /* _WIN32 / POSIX */

int platform_list_env_all(void)
{
    int count = 0;
#ifdef _WIN32
    extern char **_environ;
    char **envp = _environ;
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
