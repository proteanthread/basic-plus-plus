# C17 API Reference: Platform Abstraction Layer (`platform/platform.h`)

## 1. Subsystem Overview & Responsibilities

The Platform Abstraction Layer (`platform/platform.h`, implemented in `engine/src/platform/`) encapsulates all host operating system `#ifdef` branching, Win32 vs POSIX system interfaces, hardware timing, audio generation, terminal console raw modes, thread management, dynamic linking, and filesystem operations in BASIC++ v6.5.2.

Key architectural responsibilities include:
- **Zero OS-Specific Code in Upper Layers**: Upper engine layers (`libkernel`, `libengine`, `libhardware`, `libserver`, `libscript`, `libcore`) contain ZERO operating system headers (`<windows.h>`, `<unistd.h>`, `<sys/stat.h>`, `<dlfcn.h>`); all host interactions route strictly through `libplatform`.
- **Target Platform Classification (`BppPlatformId`)**:
  - `PLAT_WINDOWS`: Win32 / Win64 console, threads, file locks, DLL loader.
  - `PLAT_POSIX`: Linux, macOS, BSD, Solaris, Android terminal termios, pthreads, dlopen.
  - `PLAT_DOS`: 16-bit / 32-bit FreeDOS / MS-DOS raw BIOS and DOS interrupts.
- **Unified Subsystem Modules**:
  - `plat_console.c`: Raw mode switching, terminal size queries, key events.
  - `plat_fs.c`: Filesystem navigation, directory listings, file stat, renaming, deletion.
  - `plat_thread.c`: OS worker threads, mutexes, condition variables.
  - `plat_time.c`: High-resolution microsecond timer clocks and sleep delays.
  - `plat_dl.c`: Dynamic shared library loading (`dlopen` / `LoadLibraryA`).
  - `plat_net.c`: Berkeley sockets and WinSock TCP/UDP abstractions.
  - `plat_clipboard.c`: System clipboard text reading and writing.

## 2. Header Inclusion & Prerequisites

```c
#include "platform/platform.h"
#include <stdint.h>
#include <stdbool.h>
```

## 3. Data Structures & Types

```c
typedef enum {
    PLAT_UNKNOWN = 0,
    PLAT_WINDOWS,   /* Microsoft Windows Win32/Win64 */
    PLAT_POSIX,     /* Linux, macOS, FreeBSD, NetBSD */
    PLAT_DOS        /* FreeDOS / MS-DOS / OpenWatcom */
} BppPlatformId;
```

## 4. Function Prototypes & Operational Contracts

### Lifecycle & Identification
```c
/**
 * @brief Initializes host platform drivers, console modes, and socket stacks.
 */
void platform_init(void);

/**
 * @brief Restores original host terminal cooked mode and cleans up platform resources.
 */
void platform_shutdown(void);

/**
 * @brief Returns the enum identifier of the active host platform.
 */
BppPlatformId platform_get_id(void);

/**
 * @brief Returns the human-readable name of the active operating system.
 */
const char *platform_name(void);
```

### High-Resolution Timing & Sound
```c
/**
 * @brief Suspends execution of the calling thread for specified milliseconds.
 */
void platform_sleep_ms(uint32_t ms);

/**
 * @brief Emits a standard 800 Hz speaker beep tone for 250 ms (BEEP statement).
 */
void platform_sound_beep(void);

/**
 * @brief Emits a tone of specified frequency in Hz for duration in ms (SOUND statement).
 */
void platform_sound_tone(uint32_t frequency_hz, uint32_t duration_ms);

/**
 * @brief Immediately stops any active audio tone output.
 */
void platform_sound_stop(void);
```

### Platform Filesystem & Subprocesses
```c
char *platform_getcwd(char *buf, size_t size);
bool  plat_fs_chdir(const char *path);
bool  plat_fs_mkdir(const char *path);
bool  plat_fs_rmdir(const char *path);
bool  plat_fs_exists(const char *path);
bool  plat_fs_rename(const char *old_path, const char *new_path);
bool  plat_fs_delete(const char *path);
int   platform_system_exec(const char *command);
```

## 5. Architectural Invariants

- **Freestanding Cleanliness**: Header `platform.h` exports only pure ISO C17 signatures without leaking OS header types (`HANDLE`, `HWND`, `pid_t`, `pthread_t`).
- **Signal & Terminal Safety**: Guarantees terminal state restoration upon unhandled signals (SIGINT, SIGTERM).

## 6. Code Example: Cross-Platform Sound and Sleep in C

```c
#include "platform/platform.h"

void play_startup_chime(void) {
    platform_sound_tone(440, 100); /* A4 */
    platform_sleep_ms(20);
    platform_sound_tone(880, 150); /* A5 */
}
```
