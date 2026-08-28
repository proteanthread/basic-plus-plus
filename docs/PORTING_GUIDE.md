# BASIC++ v6.5.2 Porting Guide

## 1. OVERVIEW

This guide covers porting BASIC++ to a new operating system or hardware platform. The porting effort is concentrated in the platform abstraction layer (libplatform). All other engine code is platform-independent C17.

## 2. PORTING CHECKLIST

To port BASIC++ to a new platform, implement the following platform modules:

1. **plat_console.c** — Terminal initialization, cursor positioning, color attributes, raw key input, screen size queries.
2. **plat_fs.c** — File open/close/read/write, directory operations, path normalization.
3. **plat_sys.c** — Process exit, environment variables, hostname, username, shell execution.
4. **plat_time.c** — Current date/time, high-resolution timer, sleep/delay.
5. **plat_thread.c** — Mutex creation/locking (even if single-threaded, stub implementations are needed).
6. **plat_dl.c** — Dynamic library loading (or stub that always fails if not supported).
7. **plat_net.c** — TCP/UDP socket creation, connect, send, receive (or stub for no-network builds).
8. **plat_regex.c** — POSIX regex or custom implementation (or stub).
9. **plat_clipboard.c** — System clipboard read/write (or stub).

## 3. MINIMUM VIABLE PORT

A minimal port requires only three modules: plat_console, plat_fs, and plat_time. With these three, the bpp lite edition can run with PRINT, INPUT, file I/O, and timing functions. Network, clipboard, dynamic loading, and regex can be stubbed.

## 4. STUB IMPLEMENTATIONS

Each platform module has a well-defined API. Stubs return appropriate "not supported" values:

```c
// plat_net.c stub for no-network platforms
int plat_net_connect(const char* host, int port) {
    return -1;  // Connection not supported
}
```

The engine handles stub returns gracefully — denied operations produce Error 73 (Advanced feature disabled).

## 5. CONSOLE PORTING

The console module is usually the most complex port. It must provide:

- Raw mode input (character-by-character without line buffering).
- Non-blocking key reading (for INKEY$).
- Cursor positioning (absolute row/column).
- Color attribute setting (foreground/background).
- Screen size queries (rows and columns).
- Screen clearing.

On POSIX systems, this uses termios and ANSI escape sequences. On Windows, the Console API. On embedded, UART serial output with VT100 escape codes.

## 6. FILESYSTEM PORTING

The filesystem module must handle path format differences:

- Path separators (/ vs \).
- Drive letters (Windows) vs mount points (Unix).
- Case sensitivity (Unix is case-sensitive, Windows is not).
- Maximum path length.
- File attribute handling.

The VFS normalizes paths before they reach the platform layer, so most path issues are handled automatically.

## 7. BUILD SYSTEM

Add a new CMake toolchain file in cmake/ for the target platform. The toolchain file specifies the compiler, linker, system libraries, and platform-specific defines.

Create a new CMake preset or profile:

```cmake
if(MY_NEW_PLATFORM)
    set(BASIC_MY_PLATFORM ON)
    add_definitions(-DBASIC_MY_PLATFORM)
    set(PLATFORM_SOURCES
        engine/lib/platform/plat_console_myplatform.c
        engine/lib/platform/plat_fs_myplatform.c
        engine/lib/platform/plat_time_myplatform.c
    )
endif()
```

## 8. TESTING A PORT

After implementing the platform modules, verify with:

1. `bpp -c "PRINT 2+2"` — Basic arithmetic.
2. `bpp -c "SELFTEST"` — Built-in self-test.
3. Run the test suite: `bpp tests/gwbasic/*.bas`.
4. Verify file I/O: create, read, write, delete files.
5. Verify console: cursor positioning, colors, INKEY$.

## 9. EXISTING PORTS AS REFERENCE

Use the existing platform implementations as reference:

- **Windows**: engine/lib/platform/plat_*.c (Win32 API, `#ifdef _WIN32` blocks).
- **Linux**: Same files (POSIX API, `#ifdef __linux__` blocks).
- **FreeDOS**: cmake/dos16.cmake + `#ifdef BASIC_FREEDOS_16` blocks.
