/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_platform.h
 * @brief Cross-Platform Abstraction API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares OS-independent interfaces for keyboard, console sizes, directory creation,
 *   sleeping, and platform identifiers.
 * - Why it exists: Isolate OS-specific calls in a single directory structure, preventing platform checks (#ifdef)
 *   from cluttering the interpreter VM and parsing files.
 * - Why it works this way: Function names are standard across targets. The build system links the target platform
 *   source file (platform_win.c, platform_posix.c, platform_dos.c) based on compiler options.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Console dimension lookups, path separators, sleep mappings.
 * - What cannot be changed: Declared platform interfaces (must remain uniform).
 * - What to expect: Calling platform sleep will map to Sleep on Windows, usleep on POSIX, and a delay loop on DOS.
 * - What to do if something breaks: If a platform build fails (e.g. Watcom DOS), verify that DOS headers
 *   (like dos.h, bios.h) are correctly wrapped in the platform compilation unit.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Standard platform detection macros are provided by compilers.
 * - Portability concerns: DOS lacks standard threads and high-precision timers, so fallbacks are implemented.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add sound/graphics platform abstractions or VFS file system hooks.
 * - How to write external extensions: External plugins use these APIs to perform platform actions safely.
 */

#ifndef BPP_PLATFORM_H
#define BPP_PLATFORM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PLAT_UNKNOWN = 0,
    PLAT_WINDOWS,
    PLAT_POSIX,     /* Linux, BSD, macOS */
    PLAT_DOS        /* FreeDOS / MS-DOS */
} BppPlatformId;

/**
 * @brief Initialize platform subsystems (e.g. terminal settings).
 */
void platform_init(void);

/**
 * @brief Restore platform subsystems (e.g. restore raw mode settings).
 */
void platform_shutdown(void);

void platform_fatal(const char *msg);
void platform_panic(int code, const char *msg);

/**
 * @brief Get the identifier of the host platform.
 */
BppPlatformId platform_get_id(void);

/**
 * @brief Get the name string of the host platform.
 */
const char *platform_name(void);

/**
 * @brief Sleep for a specific duration in milliseconds.
 */
void platform_sleep_ms(uint32_t ms);

/**
 * @brief Non-blocking check for keyboard input.
 * @return true if keyboard character is waiting, false otherwise.
 */
bool platform_kbhit(void);

/**
 * @brief Blocking read of a single raw character from keyboard (no echoing).
 */
int platform_getch(void);

/**
 * @brief Retrieve terminal output console height (rows).
 */
int platform_console_height(void);

/**
 * @brief Retrieve terminal output console width (columns).
 */
int platform_console_width(void);

/* File system wrappers */
int platform_chdir(const char *path);

/* OS shell and signals */
void platform_setup_signals(void *vm_ptr);
void platform_execute_shell(void);
void platform_execute_command(const char *cmd);
int platform_mkdir(const char *path);
int platform_rmdir(const char *path);
char *platform_getcwd(char *buf, size_t size);
int platform_remove(const char *path);
int platform_rename(const char *oldpath, const char *newpath);
long platform_filesize(const char *path);
int platform_filemod(const char *path, char *out_buf, size_t buf_size);
int platform_list_files(void *vdev_ptr, const char *pattern);

/* Advanced Environment and Attributes */
int platform_setenv(const char *name, const char *value);
char *platform_getenv(const char *name);
void platform_get_hostname(char *buf, size_t size);
void platform_get_username(char *buf, size_t size);
int platform_get_attributes(const char *path);
int platform_set_attributes(const char *path, int attr);

/* File Locking */
#include <stdio.h>
int platform_lock_file(FILE *fp);
int platform_unlock_file(FILE *fp);

/* Directory Iteration */
typedef struct BppDirSearch BppDirSearch;
BppDirSearch *platform_find_first_file(const char *pattern, char *out_name, size_t out_size);
int platform_find_next_file(BppDirSearch *search, char *out_name, size_t out_size);
void platform_find_close(BppDirSearch *search);

/* Time utilities */
#include <time.h>

/**
 * @brief Thread-safe local-time conversion.
 *
 * Converts a time_t value to a broken-down struct tm in local time.
 * Uses localtime_s on Windows/MSVC and localtime_r on POSIX.
 *
 * @param timep  Pointer to the time_t value to convert.
 * @param result Pointer to a caller-supplied struct tm buffer.
 * @return       Pointer to result on success, NULL on failure.
 */
struct tm *platform_localtime(const time_t *timep, struct tm *result);
struct tm *platform_gmtime(const time_t *timep, struct tm *result);
double platform_get_timer(void);
double platform_get_uptime(void);

/* Threading and Mutex Abstractions */
typedef struct {
    void *handle;
    unsigned long id;
    int has_thread;
} BppThread;

typedef struct {
    void *lock;
} BppMutex;

void platform_mutex_init(BppMutex *mutex);
void platform_mutex_lock(BppMutex *mutex);
void platform_mutex_unlock(BppMutex *mutex);
void platform_mutex_destroy(BppMutex *mutex);

int platform_thread_create(BppThread *thread, void *(*start_routine)(void *), void *arg);
int platform_thread_join(BppThread *thread);

/* Dynamic Library Loading */
void *platform_load_library(const char *path);
void *platform_get_proc_address(void *library_handle, const char *name);
void platform_free_library(void *library_handle);
const char *platform_library_last_error(void);

/* Cross-platform Socket API */
#include <stdint.h>
typedef intptr_t BppSocket;
#define BPP_INVALID_SOCKET ((BppSocket)-1)
#define BPP_SOCK_STREAM 1
#define BPP_SOCK_DGRAM  2

#include "bpp_types.h"
int platform_net_init(void);
void platform_net_cleanup(void);
BppSocket platform_socket_connect(const char *host, int port, int socktype, BppError *err);
BppSocket platform_socket_listen(int port, BppError *err);
BppSocket platform_socket_accept(BppSocket listen_sock, char *client_ip_buf, int ip_buf_len, BppError *err);
int platform_socket_send(BppSocket sock, const void *buf, int len);
int platform_socket_recv(BppSocket sock, void *buf, int len, int *err_code);
void platform_socket_close(BppSocket sock);
int platform_socket_set_nonblocking(BppSocket sock, int nonblock);
int platform_socket_poll_readable(BppSocket sock, int timeout_ms);

#if defined(_WIN32) || defined(_MSC_VER)
    #define platform_strcasecmp _stricmp
    #define platform_strncasecmp _strnicmp
#else
    #include <strings.h>
    #define platform_strcasecmp strcasecmp
    #define platform_strncasecmp strncasecmp
#endif

int platform_get_executable_path(char *buf, size_t size);

/* Regex Helpers */
int platform_regex_match(const char *text, const char *pattern);
char *platform_regex_replace(const char *text, const char *pattern, const char *replacement);

/* TUI Abstractions */
void platform_tui_init(void);
void platform_tui_shutdown(void);

/* Screen Querying Helpers */
int platform_screen_get_char(int row, int col);
int platform_screen_get_attr(int row, int col);

/* Mouse Abstractions */
void platform_mouse_enable(bool enable);
void platform_mouse_get_position(int *col, int *row);
void platform_mouse_set_position(int col, int row);
int platform_mouse_get_button(int btn_idx);
int platform_mouse_get_modifiers(void);
void platform_mouse_set_cursor(int char_code, int attrib);
void platform_mouse_get_cursor(int *char_code, int *attrib);
bool platform_mouse_is_visible(void);

/* Workspace Cleanup */
void platform_cleanup_workspace(bool full_cleanup);

/* Clipboard Abstractions */
char *platform_clipboard_get(void);
void platform_clipboard_set(const char *text);

#endif /* BPP_PLATFORM_H */
