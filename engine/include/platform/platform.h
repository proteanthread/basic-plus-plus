// FILENAME: platform.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libhardware, libkernel, libplatform, libserver, libstandard
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (time.h, time.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for platform.h.
//
// ---- Includes ----

// FILENAME: platform.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (common_internal.h, embedded.c, headless.c, mobile.c)
// NEEDED BY: libcore (bpp_api.c, error.c, file_internal.h, get.c, hal_hosted.c)
// NEEDED BY: libcore (hal_sdl2.c, iot_main.c, logger.c, regex.c)
// NEEDED BY: libcore (variables_internal.h, vfs.c)
// NEEDED BY: libengine (assign.c, ast_eval_stmt.c, ast_internal.h)
// NEEDED BY: libengine (ast_parse_block.c, ast_parse_expr.c, ast_parse_stmt.c)
// NEEDED BY: libengine (beep.c, bios.c, chdir.c, clock_num.c, color.c)
// NEEDED BY: libengine (context.c, control.c, data.c, date.c)
// NEEDED BY: libengine (dispatch_internal.h, doevents.c, enter.c, environ.c)
// NEEDED BY: libengine (eval_expr_internal.h, eval_internal.h)
// NEEDED BY: libengine (events_internal.h, exec_control_internal.h)
// NEEDED BY: libengine (exec_dispatch.c, exec_internal.h, field.c, files.c)
// NEEDED BY: libengine (find.c, for.c, goodbye.c, gosub.c, goto.c, inkey.c)
// NEEDED BY: libengine (introspection.c, key.c, line.c, lock.c, map.c)
// NEEDED BY: libengine (mat_internal.h, mkdir.c, name.c, next.c, on_key.c)
// NEEDED BY: libengine (open.c, option.c, pds_datetime.c, pds_sys.c, play.c)
// NEEDED BY: libengine (print_file.c, put.c, randomize.c, reformat_internal.h)
// NEEDED BY: libengine (rmdir.c, session_stmts.c, shell.c, sleep.c, sound.c)
// NEEDED BY: libengine (sub_internal.h, suspend.c, sys.c, sys_fn.c, system.c)
// NEEDED BY: libengine (task.h, task.c, ticks.c, time.c, time_fn.c, timer.c)
// NEEDED BY: libengine (tui_multiplexer.c, txn.c, type.c, vbdos_fn.c)
// NEEDED BY: libengine (vm_internal.h, voice.c, wait.c, wend.c, while.c)
// NEEDED BY: libhardware (bios_at.c, bios_pc.c)
// NEEDED BY: libkernel (console.c, fujinet.c, gfx.c, gfx_audio.c)
// NEEDED BY: libkernel (gfx_internal.h, gfx_tui.c, security.c, vdev.c)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform_core.c)
// NEEDED BY: libserver (vnet.c)
// NEEDED BY: libstandard (edit_internal.h, vi_internal.h, ws_internal.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (time.h, time.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for platform.h.
//
// ---- Includes ----

// FILENAME: platform.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (common_internal.h, embedded.c, headless.c, mobile.c)
// NEEDED BY: libcore (bpp_api.c, error.c, file_internal.h, get.c, hal_hosted.c)
// NEEDED BY: libcore (hal_sdl2.c, iot_main.c, logger.c, regex.c)
// NEEDED BY: libcore (variables_internal.h, vfs.c)
// NEEDED BY: libengine (assign.c, ast_eval_stmt.c, ast_internal.h)
// NEEDED BY: libengine (ast_parse_block.c, ast_parse_expr.c, ast_parse_stmt.c)
// NEEDED BY: libengine (beep.c, bios.c, chdir.c, clock_num.c, color.c)
// NEEDED BY: libengine (context.c, control.c, data.c, date.c)
// NEEDED BY: libengine (dispatch_internal.h, doevents.c, enter.c, environ.c)
// NEEDED BY: libengine (eval_expr_internal.h, eval_internal.h)
// NEEDED BY: libengine (events_internal.h, exec_control_internal.h)
// NEEDED BY: libengine (exec_dispatch.c, exec_internal.h, field.c, files.c)
// NEEDED BY: libengine (find.c, for.c, goodbye.c, gosub.c, goto.c, inkey.c)
// NEEDED BY: libengine (introspection.c, key.c, line.c, lock.c, map.c)
// NEEDED BY: libengine (mat_internal.h, mkdir.c, name.c, next.c, on_key.c)
// NEEDED BY: libengine (open.c, option.c, pds_datetime.c, pds_sys.c, play.c)
// NEEDED BY: libengine (print_file.c, put.c, randomize.c, reformat_internal.h)
// NEEDED BY: libengine (rmdir.c, session_stmts.c, shell.c, sleep.c, sound.c)
// NEEDED BY: libengine (sub_internal.h, suspend.c, sys.c, sys_fn.c, system.c)
// NEEDED BY: libengine (task.h, task.c, ticks.c, time.c, time_fn.c, timer.c)
// NEEDED BY: libengine (tui_multiplexer.c, txn.c, type.c, vbdos_fn.c)
// NEEDED BY: libengine (vm_internal.h, voice.c, wait.c, wend.c, while.c)
// NEEDED BY: libhardware (bios_at.c, bios_pc.c)
// NEEDED BY: libkernel (console.c, fujinet.c, gfx.c, gfx_audio.c)
// NEEDED BY: libkernel (gfx_internal.h, gfx_tui.c, security.c, vdev.c)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform_core.c)
// NEEDED BY: libserver (vnet.c)
// NEEDED BY: libstandard (edit_internal.h, vi_internal.h, ws_internal.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (time.h, time.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for platform.h.
//
// ---- Includes ----

// FILENAME: platform.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (common_internal.h, embedded.c, headless.c, mobile.c)
// NEEDED BY: libcore (bpp_api.c, error.c, file_internal.h, get.c, hal_hosted.c)
// NEEDED BY: libcore (hal_sdl2.c, logger.c, regex.c, variables_internal.h)
// NEEDED BY: libcore (vfs.c)
// NEEDED BY: libengine (assign.c, ast_eval_stmt.c, ast_internal.h)
// NEEDED BY: libengine (ast_parse_block.c, ast_parse_expr.c, ast_parse_stmt.c)
// NEEDED BY: libengine (beep.c, bios.c, chdir.c, clock_num.c, color.c)
// NEEDED BY: libengine (context.c, control.c, data.c, date.c)
// NEEDED BY: libengine (dispatch_internal.h, doevents.c, enter.c, environ.c)
// NEEDED BY: libengine (eval_expr_internal.h, eval_internal.h)
// NEEDED BY: libengine (events_internal.h, exec_control_internal.h)
// NEEDED BY: libengine (exec_dispatch.c, exec_internal.h, field.c, files.c)
// NEEDED BY: libengine (find.c, for.c, goodbye.c, gosub.c, goto.c, inkey.c)
// NEEDED BY: libengine (introspection.c, key.c, line.c, lock.c, map.c)
// NEEDED BY: libengine (mat_internal.h, mkdir.c, name.c, next.c, on_key.c)
// NEEDED BY: libengine (open.c, option.c, pds_datetime.c, pds_sys.c, play.c)
// NEEDED BY: libengine (print_file.c, put.c, randomize.c, reformat_internal.h)
// NEEDED BY: libengine (rmdir.c, session_stmts.c, shell.c, sleep.c, sound.c)
// NEEDED BY: libengine (sub_internal.h, suspend.c, sys.c, sys_fn.c, system.c)
// NEEDED BY: libengine (task.h, task.c, ticks.c, time.c, time_fn.c, timer.c)
// NEEDED BY: libengine (tui_multiplexer.c, txn.c, type.c, vbdos_fn.c)
// NEEDED BY: libengine (vm_internal.h, voice.c, wait.c, wend.c, while.c)
// NEEDED BY: libhardware (bios_at.c, bios_pc.c)
// NEEDED BY: libkernel (console.c, fujinet.c, gfx.c, gfx_audio.c)
// NEEDED BY: libkernel (gfx_internal.h, gfx_tui.c, security.c, vdev.c)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform_core.c)
// NEEDED BY: libserver (vnet.c)
// NEEDED BY: libstandard (edit_internal.h, vi_internal.h, ws_internal.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (time.h, time.c)
// NEEDS: libkernel (types.h)
// Implements component functionality for platform.h.
//
// ---- Includes ----

// FILENAME: platform.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: bpp.exe (iot.c)
// NEEDED BY: bs.exe (server.c)
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libboot (embedded.c, headless.c, mobile.c)
// NEEDED BY: libcore (file_internal.h, variables_internal.h)
// NEEDED BY: libcore (bpp_api.c, error.c, get.c, hal_hosted.c, hal_sdl2.c)
// NEEDED BY: libcore (logger.c, regex.c, vfs.c)
// NEEDED BY: libengine (ast_internal.h, dispatch_internal.h)
// NEEDED BY: libengine (eval_expr_internal.h, eval_internal.h)
// NEEDED BY: libengine (events_internal.h, exec_control_internal.h)
// NEEDED BY: libengine (exec_internal.h, mat_internal.h, reformat_internal.h)
// NEEDED BY: libengine (sub_internal.h, task.h, vm_internal.h)
// NEEDED BY: libengine (assign.c, ast_eval_stmt.c, ast_parse_block.c)
// NEEDED BY: libengine (ast_parse_expr.c, ast_parse_stmt.c, beep.c, bios.c)
// NEEDED BY: libengine (chdir.c, clock_num.c, color.c, context.c, control.c)
// NEEDED BY: libengine (data.c, date.c, doevents.c, enter.c, environ.c)
// NEEDED BY: libengine (exec_dispatch.c, field.c, files.c, find.c, for.c)
// NEEDED BY: libengine (goodbye.c, gosub.c, goto.c, inkey.c, introspection.c)
// NEEDED BY: libengine (key.c, line.c, lock.c, map.c, mkdir.c, name.c, next.c)
// NEEDED BY: libengine (on_key.c, open.c, option.c, pds_datetime.c, pds_sys.c)
// NEEDED BY: libengine (play.c, print_file.c, put.c, randomize.c, rmdir.c)
// NEEDED BY: libengine (session_stmts.c, shell.c, sleep.c, sound.c, suspend.c)
// NEEDED BY: libengine (sys.c, sys_fn.c, system.c, task.c, ticks.c, time.c)
// NEEDED BY: libengine (time_fn.c, timer.c, tui_multiplexer.c, txn.c, type.c)
// NEEDED BY: libengine (vbdos_fn.c, voice.c, wait.c, wend.c, while.c)
// NEEDED BY: libhardware (bios_at.c, bios_pc.c)
// NEEDED BY: libkernel (gfx_internal.h)
// NEEDED BY: libkernel (console.c, fujinet.c, gfx.c, gfx_audio.c, gfx_tui.c)
// NEEDED BY: libkernel (security.c, vdev.c)
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c, platform_core.c)
// NEEDED BY: libserver (vnet.c)
// NEEDED BY: libstandard (edit_internal.h, vi_internal.h, ws_internal.h)
// NEEDS: libcore (strings.h)
// NEEDS: libcore (strings.c)
// NEEDS: libengine (time.h)
// NEEDS: libengine (time.c)
// NEEDS: libkernel (types.h)
// Provides cross-platform OS abstraction primitives for platform.
//
// ---- Includes ----

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    PLAT_UNKNOWN = 0,
    PLAT_WINDOWS,
    PLAT_POSIX,     // Linux, BSD, macOS
    PLAT_DOS        // FreeDOS / MS-DOS
} BppPlatformId;

// @brief Initialize platform subsystems (e.g. terminal settings).
void platform_init(void);

// @brief Restore platform subsystems (e.g. restore raw mode settings).
void platform_shutdown(void);

void platform_fatal(const char *msg);
void platform_panic(int code, const char *msg);

// @brief Get the identifier of the host platform.
BppPlatformId platform_get_id(void);

// @brief Get the name string of the host platform.
const char *platform_name(void);

// @brief Sleep for a specific duration in milliseconds.
void platform_sleep_ms(uint32_t ms);

// @brief Emit a standard 800 Hz speaker beep tone for 250 milliseconds.
void platform_sound_beep(void);

// @brief Emit a tone of specified frequency in Hertz for specified duration in milliseconds.
// If frequency_hz is 0, pauses sound/silence for duration_ms.
void platform_sound_tone(uint32_t frequency_hz, uint32_t duration_ms);

// @brief Immediately stop any active tone output.
void platform_sound_stop(void);

// @brief Non-blocking check for keyboard input.
// @return true if keyboard character is waiting, false otherwise.
bool platform_kbhit(void);

// @brief Non-destructive peek of next character in keyboard queue (returns 0 if empty).
int platform_peek_key(void);

// @brief Reads next character from keyboard queue non-blockingly (returns 0 if empty).
int platform_inkey_char(void);

// @brief Blocking read of a single raw character from keyboard (no echoing).
int platform_getch(void);

// @brief Check if standard input is an interactive console terminal (vs pipe or file).
bool platform_stdin_is_console(void);

// @brief Retrieve terminal output console height (rows).
int platform_console_height(void);

// @brief Retrieve terminal output console width (columns).
int platform_console_width(void);

// Console Output Primitives
int platform_console_putchar(int c);
int platform_console_getchar(void);
char *platform_console_gets(char *buf, size_t size);
int platform_console_puts(const char *str);
int platform_console_printf(const char *fmt, ...);
int platform_console_eputs(const char *str);
void platform_console_eprintf(const char *fmt, ...);
void platform_console_flush(void);

// File system wrappers
int platform_chdir(const char *path);
void *platform_file_open(const char *path, const char *mode);
int platform_file_close(void *handle);
size_t platform_file_read(void *handle, void *buffer, size_t size);
size_t platform_file_write(void *handle, const void *buffer, size_t size);
int platform_file_printf(void *handle, const char *format, ...);
char *platform_file_gets(char *buf, size_t size, void *handle);
#ifndef PLATFORM_SEEK_SET
#define PLATFORM_SEEK_SET 0
#define PLATFORM_SEEK_CUR 1
#define PLATFORM_SEEK_END 2
#endif

int platform_file_seek(void *handle, long offset, int origin);
long platform_file_tell(void *handle);
int platform_file_eof(void *handle);
int platform_file_flush(void *handle);

// OS shell and signals
void platform_setup_signals(void *vm_ptr);
void *platform_get_active_vm(void);
void platform_trigger_break(void);
void platform_execute_shell(void);
void platform_execute_command(const char *cmd);
char *platform_execute_capture(const char *cmd);
void platform_execute_capture_free(char *buf);
int platform_mkdir(const char *path);
int platform_rmdir(const char *path);
char *platform_getcwd(char *buf, size_t size);
int platform_remove(const char *path);
int platform_rename(const char *oldpath, const char *newpath);
long platform_filesize(const char *path);
int platform_filemod(const char *path, char *out_buf, size_t buf_size);
int platform_list_files(void *vdev_ptr, const char *pattern);

// Advanced Environment and Attributes
int platform_setenv(const char *name, const char *value);
char *platform_getenv(const char *name);
void platform_get_hostname(char *buf, size_t size);
void platform_get_username(char *buf, size_t size);
int platform_get_attributes(const char *path);
int platform_set_attributes(const char *path, int attr);

// Platform Hardware & Sensors
int platform_get_battery_level(void);
double platform_get_temperature(void);
int platform_get_cpu_load(void);
int platform_get_wifi_rssi(void);
void platform_get_lan_ipv4(char *buf, size_t size);
void platform_get_wan_ipv4(char *buf, size_t size);
void platform_get_lan_ipv6(char *buf, size_t size);
void platform_get_wan_ipv6(char *buf, size_t size);
int platform_get_cpu_cores(void);
long platform_get_pid(void);
long platform_get_ppid(void);

// File Locking
int platform_lock_file(void *fp);
int platform_unlock_file(void *fp);

// Directory Iteration
typedef struct BppDirSearch BppDirSearch;
BppDirSearch *platform_find_first_file(const char *pattern, char *out_name, size_t out_size);
int platform_find_next_file(BppDirSearch *search, char *out_name, size_t out_size);
void platform_find_close(BppDirSearch *search);

// Time utilities
#include <time.h>

// @brief Thread-safe local-time conversion.
//
// Converts a time_t value to a broken-down struct tm in local time.
// Uses localtime_s on Windows/MSVC and localtime_r on POSIX.
//
// @param timep  Pointer to the time_t value to convert.
// @param result Pointer to a caller-supplied struct tm buffer.
// @return       Pointer to result on success, NULL on failure.
struct tm *platform_localtime(const time_t *timep, struct tm *result);
struct tm *platform_gmtime(const time_t *timep, struct tm *result);
double platform_get_timer(void);
double platform_get_uptime(void);
double platform_get_system_uptime(void);
double platform_get_highres_time(void);
uint64_t platform_get_total_system_memory(void);
uint64_t platform_get_avail_system_memory(void);

// Threading and Mutex Abstractions
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

// Dynamic Library Loading
void *platform_load_library(const char *path);
void *platform_get_proc_address(void *library_handle, const char *name);
void platform_free_library(void *library_handle);
const char *platform_library_last_error(void);

// Cross-platform Socket API
typedef intptr_t BppSocket;
#define BASIC_INVALID_SOCKET ((BppSocket)-1)
#define BASIC_SOCK_STREAM 1
#define BASIC_SOCK_DGRAM  2

#include "types/types.h"
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
    #define platform_strcasecmp runtime_strcasecmp
    #define platform_strncasecmp runtime_strncasecmp
#else
    #include "runtime/string/strops.h"
#include "platform/platform.h"
    #define platform_strcasecmp runtime_strcasecmp
    #define platform_strncasecmp runtime_strncasecmp
#endif

int platform_get_executable_path(char *buf, size_t size);

// Regex Helpers
int platform_regex_match(const char *text, const char *pattern);
char *platform_regex_replace(const char *text, const char *pattern, const char *replacement);

// TUI Abstractions
void platform_tui_init(void);
void platform_tui_shutdown(void);

// Screen Querying Helpers
int platform_screen_get_char(int row, int col);
int platform_screen_get_attr(int row, int col);

// Mouse Abstractions
void platform_mouse_enable(bool enable);
void platform_mouse_get_position(int *col, int *row);
void platform_mouse_set_position(int col, int row);
int platform_mouse_get_button(int btn_idx);
int platform_mouse_get_modifiers(void);
void platform_mouse_set_cursor(int char_code, int attrib);
void platform_mouse_get_cursor(int *char_code, int *attrib);
bool platform_mouse_is_visible(void);

// Workspace Cleanup
void platform_cleanup_workspace(bool full_cleanup);

// Clipboard Abstractions
char *platform_clipboard_get(void);
void platform_clipboard_free(char *text);
void platform_clipboard_set(const char *text);

// Hardware Clocks & Speed Abstraction
#include "platform/plat_hw_speed.h"

#endif // PLATFORM_H
