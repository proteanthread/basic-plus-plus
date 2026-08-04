# OS Platform Abstraction API Reference

Header File: [`include/platform.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/platform.h)

## Overview
Bridges platform differences for clocks, console sizes, clipboard, and file descriptors.

## Exposed API Entities
### Structs & Types
- `BppDirSearch BppDirSearch`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `platform_init` | `void` | `void` |
| `platform_shutdown` | `void` | `void` |
| `platform_get_id` | `BppPlatformId` | `void` |
| `platform_sleep_ms` | `void` | `uint32_t ms` |
| `platform_kbhit` | `bool` | `void` |
| `platform_getch` | `int` | `void` |
| `platform_console_height` | `int` | `void` |
| `platform_console_width` | `int` | `void` |
| `platform_chdir` | `int` | `const char *path` |
| `platform_setup_signals` | `void` | `void *vm_ptr` |
| `platform_execute_shell` | `void` | `void` |
| `platform_execute_command` | `void` | `const char *cmd` |
| `platform_mkdir` | `int` | `const char *path` |
| `platform_rmdir` | `int` | `const char *path` |
| `platform_remove` | `int` | `const char *path` |
| `platform_rename` | `int` | `const char *oldpath, const char *newpath` |
| `platform_filesize` | `long` | `const char *path` |
| `platform_filemod` | `int` | `const char *path, char *out_buf, size_t buf_size` |
| `platform_list_files` | `int` | `void *vdev_ptr, const char *pattern` |
| `platform_setenv` | `int` | `const char *name, const char *value` |
| `platform_get_attributes` | `int` | `const char *path` |
| `platform_set_attributes` | `int` | `const char *path, int attr` |
| `platform_lock_file` | `int` | `FILE *fp` |
| `platform_unlock_file` | `int` | `FILE *fp` |
| `platform_find_next_file` | `int` | `BppDirSearch *search, char *out_name, size_t out_size` |
| `platform_find_close` | `void` | `BppDirSearch *search` |
| `platform_get_timer` | `double` | `void` |
| `platform_mutex_init` | `void` | `BppMutex *mutex` |
| `platform_mutex_lock` | `void` | `BppMutex *mutex` |
| `platform_mutex_unlock` | `void` | `BppMutex *mutex` |
| `platform_mutex_destroy` | `void` | `BppMutex *mutex` |
| `platform_thread_join` | `int` | `BppThread *thread` |
| `platform_free_library` | `void` | `void *library_handle` |
| `platform_net_init` | `int` | `void` |
| `platform_net_cleanup` | `void` | `void` |
| `platform_socket_connect` | `BppSocket` | `const char *host, int port, int socktype, BppError *err` |
| `platform_socket_listen` | `BppSocket` | `int port, BppError *err` |
| `platform_socket_accept` | `BppSocket` | `BppSocket listen_sock, char *client_ip_buf, int ip_buf_len, BppError *err` |
| `platform_socket_send` | `int` | `BppSocket sock, const void *buf, int len` |
| `platform_socket_recv` | `int` | `BppSocket sock, void *buf, int len, int *err_code` |
| `platform_socket_close` | `void` | `BppSocket sock` |
| `platform_socket_set_nonblocking` | `int` | `BppSocket sock, int nonblock` |
| `platform_socket_poll_readable` | `int` | `BppSocket sock, int timeout_ms` |
| `platform_get_executable_path` | `int` | `char *buf, size_t size` |
| `platform_regex_match` | `int` | `const char *text, const char *pattern` |
| `platform_tui_init` | `void` | `void` |
| `platform_tui_shutdown` | `void` | `void` |
| `platform_screen_get_char` | `int` | `int row, int col` |
| `platform_screen_get_attr` | `int` | `int row, int col` |
| `platform_cleanup_workspace` | `void` | `bool full_cleanup` |
| `platform_clipboard_set` | `void` | `const char *text` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "platform.h"

void get_metrics() {
    int w = platform_console_width();
    (void)w;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
