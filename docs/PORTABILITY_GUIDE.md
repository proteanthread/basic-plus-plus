# BASIC++ v6.5.2 Portability Guide

## 1. CROSS-PLATFORM DESIGN

BASIC++ is designed to run identically on Windows, Linux, macOS, FreeDOS, and embedded platforms. The platform abstraction layer (libplatform) encapsulates all OS-specific code. Upper layers — the lexer, parser, evaluator, VM, and all statement handlers — contain zero platform-specific code.

## 2. THE PLATFORM LAYER

The platform layer (engine/lib/platform/) provides nine abstraction modules:

| Module | File | Purpose |
|--------|------|---------|
| plat_console | plat_console.c | Terminal I/O, cursor, color, key input |
| plat_fs | plat_fs.c | File system operations |
| plat_sys | plat_sys.c | Process control, hostname, username |
| plat_time | plat_time.c | Date, time, timer, sleep |
| plat_thread | plat_thread.c | Thread/mutex primitives |
| plat_dl | plat_dl.c | Dynamic library loading |
| plat_net | plat_net.c | Network sockets |
| plat_regex | plat_regex.c | Regular expression engine |
| plat_clipboard | plat_clipboard.c | System clipboard access |

Each module has a single .c file with `#ifdef _WIN32` / `#ifdef __linux__` blocks. No other source file in the engine contains platform-specific code.

## 3. WRITING PORTABLE BASIC++ PROGRAMS

BASIC++ programs are automatically portable because the interpreter handles all platform differences. However, some practices ensure maximum portability:

**File paths**: Use forward slashes or the BASIC++ VFS path normalization. Avoid hardcoded drive letters on programs intended for Linux.

**Line endings**: BASIC++ handles both CR+LF (Windows) and LF (Unix) in source files.

**Screen size**: Do not assume 80x25. Use screen dimension queries to adapt layout.

**External commands**: SHELL commands are OS-specific. Avoid SHELL in portable programs, or use conditional logic:

```basic
10 IF INSTR(ENVIRON$("OS"), "Windows") > 0 THEN
20   SHELL "dir"
30 ELSE
40   SHELL "ls"
50 END IF
```

## 4. FEATURE AVAILABILITY

| Feature | baspp | bpp | bs | FreeDOS | Embedded |
|---------|-------|-----|----|---------|----------|
| PRINT/INPUT | ✓ | ✓ | ✓ | ✓ | ✓ |
| File I/O | ✓ | ✓ | ✓ | ✓ | Optional |
| Graphics | ✓ | ✗ | ✗ | ✓ (BIOS) | ✗ |
| Sound | ✓ | ✗ | ✗ | ✓ (speaker) | ✗ |
| Networking | ✓ | ✓ | ✓ | ✗ | ✗ |
| TUI Editor | ✓ | ✗ | ✗ | ✗ | ✗ |
| Modules | ✓ | ✓ | ✓ | ✗ | ✗ |
| VMEM | ✓ | ✗ | ✗ | ✗ | ✗ |

Programs that use features not available on the target platform receive Error 73 (Advanced feature disabled).

## 5. ENCODING

BASIC++ uses UTF-8 encoding on modern builds. String functions (LEN, LEFT$, MID$, etc.) operate on bytes, not Unicode code points. This maintains GW-BASIC compatibility where each character is one byte.

FreeDOS and embedded builds use ASCII (7-bit) encoding. Code page 437 (IBM PC) character graphics are available through CHR$ values 128-255 on all platforms.

## 6. NUMERIC PRECISION

All platforms use IEEE 754 double-precision (64-bit) floating point. Numeric results are identical across platforms within the limits of double-precision arithmetic. The RNG algorithm (linear congruential) produces identical sequences from the same seed on all platforms.
