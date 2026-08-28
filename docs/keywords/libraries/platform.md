# `platform` Platform Abstraction Layer (`libplatform`)

## 1. Architectural Purpose & Overview

The `libplatform` subsystem encapsulates all host operating system `#ifdef` logic (Win32, POSIX, Linux, macOS, BSD, and freestanding embedded), ensuring that all upper layers (`libkernel`, `libengine`, etc.) contain **zero OS-specific code**.

### Component Modules:
- **`plat_console`**: Terminal raw mode, ANSI escape sequences, window sizing.
- **`plat_fs`**: Cross-platform path normalization, file stat, directory iteration.
- **`plat_sys`**: Monotonic high-resolution timers, process spawn, hostname, username.
- **`plat_thread`**: Threads, mutexes, condition variables.
- **`plat_dl`**: Dynamic library loading (`LoadLibrary` / `dlopen`).
- **`plat_net`**: Network socket primitives (BSD sockets / Winsock).

---

## 2. Technical API Signatures (C17)

```c
const char *platform_name(void);
uint64_t platform_get_ticks_ms(void);
int platform_execute_command(const char *cmd);
struct tm *platform_localtime(const time_t *t, struct tm *buf);
```
