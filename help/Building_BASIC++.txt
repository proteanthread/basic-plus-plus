# Building BASIC++ — Makefile Tutorial

## Overview

BASIC++ is written in strict ANSI C (C89/C90) with no external dependencies
beyond the C standard library.  A single `Makefile` supports all four target
platforms:

| Platform       | Compiler         | Target          | Executable     |
|----------------|------------------|-----------------|----------------|
| Linux / MinGW  | gcc / clang      | 64-bit ELF/PE   | `baspp`        |
| Windows 11     | MSVC (`cl.exe`)  | 64-bit PE       | `basicpp.exe`  |
| FreeDOS        | OpenWatcom 16-bit| 16-bit DOS MZ   | `bpp.exe`      |
| FreeDOS        | OpenWatcom 32-bit| 32-bit DOS/4GW  | `bpp.exe`      |

The source code is identical across platforms — no `#ifdef` branches in the
language engine.  Platform differences are isolated in `core/platform.c` and
`virtual/vdev.c`.

For a complete file-by-file reference, see `source/SOURCE_TREE.txt`.

---

## 1. Quick Start

### Linux / MinGW (gcc)

```bash
cd source
make
```

Output: `../baspp`

### Windows 11 (MSVC)

Open a **Developer Command Prompt** (or run `vcvarsall.bat x64`), then:

```bash
cd source
nmake /f Makefile msvc
```

Output: `..\basicpp.exe`

**Alternative** (standalone cl command):
```bash
cl /TC /W3 /O2 /I. /D_CRT_SECURE_NO_WARNINGS /Fe:..\basicpp.exe *.c /link advapi32.lib
```

### FreeDOS (OpenWatcom 16-bit)

```bash
cd source
make watcom
```

Output: `../bpp.exe` (16-bit DOS, large memory model)

> [!WARNING]
> The 16-bit build may hit the 640K conventional memory limit with all
> features enabled.  Use `make watcom386` for the full interpreter.

### FreeDOS (OpenWatcom 32-bit)

```bash
cd source
make watcom386
```

Output: `../bpp.exe` (32-bit, DOS/4GW protected mode)

### Debug Build (gcc)

```bash
cd source
make debug
```

Flags: `-g -O0 -DDEBUG` (debug symbols, no optimization)

---

## 2. How the Makefile Works

### 2.1 Source Groups

Source files are organized into named groups by domain:

```makefile
CORE_SOURCES    = core/main.c core/memory.c core/errors.c ...
LEXER_SOURCES   = lexer/lexer.c lexer/keyword_props.c ...
PARSER_SOURCES  = parser/parser.c parser/parser_expr.c
FLOW_SOURCES    = flow/parser_flow.c flow/parser_loops.c
IO_SOURCES      = io/parser_io.c io/fileio.c ...
DIALECT_SOURCES = dialect/dialect.c dialect/dialect_gwbs.c ...
MODULES_SOURCES = modules/module.c modules/mod_stdlib.c ...
```

These are concatenated into the master list:

```makefile
SOURCES = $(CORE_SOURCES) $(LEXER_SOURCES) $(PARSER_SOURCES) \
          $(FLOW_SOURCES) $(IO_SOURCES) ... $(MISC_SOURCES)
```

### 2.2 Header Dependencies

All `.h` headers live in `source/` root.  The `-I.` flag lets subdirectory
files find them.  The Makefile uses a conservative model — every `.o` depends
on ALL headers:

```makefile
%.o: %.c $(HEADERS)
    $(CC) $(CFLAGS) -c $< -o $@
```

Any header change triggers a full rebuild.  For ~80 source files, this takes
under 10 seconds on modern hardware.

### 2.3 Compiler Flags

| Compiler     | Flags                                                      |
|--------------|------------------------------------------------------------|
| **gcc**      | `-std=c90 -pedantic -Wall -Wextra -O2 -I.`               |
| **MSVC**     | `/TC /W3 /O2 /I. /D_CRT_SECURE_NO_WARNINGS`              |
| **Watcom 16**| `-ml -0 -za -wx -ox -i=.`                                 |
| **Watcom 32**| `-mf -za -wx -ox -i=.`                                    |

Key flags explained:

- `-std=c90` / `-za` / `/TC` — strict ANSI C for maximum portability
- `-pedantic` — reject non-standard extensions
- `-ml` — large memory model (16-bit: far code + far data)
- `-mf` — flat memory model (32-bit: DPMI protected mode)
- `-0` — 8086 instruction set (broadest DOS compatibility)
- `/link advapi32.lib` — Windows registry/security API

### 2.4 Output Location

All executables go to the project root (parent of `source/`):

```makefile
OUTDIR = ..
TARGET = $(OUTDIR)/baspp
```

---

## 3. Customizing for Your System

### 3.1 Changing the Compiler

```bash
make CC=clang
```

Or edit the Makefile:

```makefile
CC = clang
```

Tested compilers: gcc 12+, clang 15+, MSVC 19+, OpenWatcom 2.0.

### 3.2 Changing Optimization

```makefile
# Maximum speed
CFLAGS = -std=c90 -pedantic -Wall -Wextra -O3 -I.

# Smallest binary
CFLAGS = -std=c90 -pedantic -Wall -Wextra -Os -I.

# Debugging
CFLAGS = -std=c90 -pedantic -Wall -Wextra -g -O0 -DDEBUG -I.
```

### 3.3 Changing the Output Name

```bash
make TARGET=../mybasic
```

### 3.4 Changing the Output Directory

```bash
make OUTDIR=/usr/local/bin
```

### 3.5 Cross-Compiling (ARM / Raspberry Pi)

```bash
make CC=arm-linux-gnueabihf-gcc
```

The code has no x86-specific dependencies.

### 3.6 Adding Linker Flags

```makefile
LDFLAGS = -lreadline -lncurses    # add libraries
LDFLAGS = -static                  # static linking
```

---

## 4. Selecting Dialects

By default, all 16 dialects are compiled in.  To build a smaller binary with
only specific dialects:

**Step 1.** Edit `DIALECT_SOURCES` in the Makefile:

```makefile
# Example: ECMA-55 + Tiny BASIC only
DIALECT_SOURCES = \
    dialect/dialect.c \
    dialect/dialect_patb.c dialect/dialect_ecma55.c
```

**Step 2.** Edit `dialect/dialect.c` — `dialect_register_all()`:

```c
void dialect_register_all(void)
{
    dialect_register_patb();
    dialect_register_ecma55();
    /* All other calls removed */
}
```

**Step 3.** Rebuild: `make clean && make`

### Dialect Reference

| Flag | File                   | Description                          |
|------|------------------------|--------------------------------------|
| PATB | `dialect_patb.c`       | Palo Alto Tiny BASIC (1976)          |
| TRS1 | `dialect_trs1.c`       | TRS-80 Level I (1977)                |
| TRS2 | `dialect_trs2.c`       | TRS-80 Level II / Model III (1978)   |
| GWBS | `dialect_gwbs.c`       | GW-BASIC / BASICA (1983)             |
| EC55 | `dialect_ecma55.c`     | ECMA-55 Minimal BASIC (1984)         |
| E116 | `dialect_ecma116.c`    | ECMA-116 Full BASIC (1991)           |
| QBAS | `dialect_qbasic.c`     | QBasic (1991)                        |
| AINT | `dialect_aint.c`       | Apple II Integer BASIC (1977)        |
| ASFT | `dialect_asft.c`       | Applesoft BASIC (1978)               |
| ATAR | `dialect_atari.c`      | Atari BASIC (1979)                   |
| C64  | `dialect_c64.c`        | Commodore BASIC v2 (1982)            |
| COCO | `dialect_coco.c`       | Color Computer BASIC (1980)          |
| MBAS | `dialect_mbasic.c`     | MBASIC / BASIC-80 / CP/M (1978)     |
| SINC | `dialect_sinclair.c`   | Sinclair ZX Spectrum (1982)          |
| QLSB | `dialect_superbasic.c` | Sinclair QL SuperBASIC (1984)        |
| SBAS | `dialect_sbasic.c`     | Tymshare SUPER BASIC (1968)          |

---

## 5. Selecting Modules

Optional modules can be omitted to reduce binary size:

**Step 1.** Edit `MODULES_SOURCES`:

```makefile
# Minimal (stdlib only)
MODULES_SOURCES = \
    modules/module.c modules/mod_stdlib.c
```

**Step 2.** Edit `core/main.c` — remove registration calls:

```c
/* mod_usb_register();     -- removed */
/* mod_fujinet_register(); -- removed */
/* mod_upnp_register();    -- removed */
```

### Module Reference

| Module  | File              | Devices Registered     | Required? |
|---------|-------------------|------------------------|-----------|
| STDLIB  | `mod_stdlib.c`    | *(function library)*   | **Yes**   |
| USB     | `mod_usb.c`       | `USB:`, `HID:`, `USBSER:` | No    |
| FUJINET | `mod_fujinet.c`   | `N:`, `FUJI:`, `CLOCK:`   | No    |
| UPNP    | `mod_upnp.c`     | `UPNP:`, `SOAP:`          | No    |

---

## 6. Memory Tuning

For memory-constrained targets (FreeDOS 16-bit), reduce pool sizes in
`source/config.h`:

```c
/* Key tunable settings */
#define MAX_PROGRAM_LINES    1000    /* Stored program lines      */
#define MAX_VARIABLES        52      /* Variable slots (A-Z, A$-Z$) */
#define MAX_DIM_ARRAYS       20      /* DIM array slots           */
#define MAX_ARRAY_ELEMENTS   5000    /* Total array element count */
#define MAX_STACK_DEPTH      50      /* GOSUB/FOR stack depth     */
#define STRINGPOOL_SIZE      32768   /* String pool bytes         */
#define MAX_MODULES          16      /* Module slots              */
#define MAX_VDEV_SLOTS       32      /* Virtual device slots      */
```

### Preset Profiles

| Setting                | Minimal (16-bit) | Default | Generous      |
|------------------------|-------------------|---------|---------------|
| `MAX_PROGRAM_LINES`    | 500               | 1000    | 10000         |
| `MAX_DIM_ARRAYS`       | 10                | 20      | 100           |
| `MAX_ARRAY_ELEMENTS`   | 2000              | 5000    | 50000         |
| `MAX_STACK_DEPTH`      | 25                | 50      | 200           |
| `STRINGPOOL_SIZE`      | 16384             | 32768   | 262144        |
| `MAX_MODULES`          | 8                 | 16      | 64            |
| `MAX_VDEV_SLOTS`       | 16                | 32      | 64            |

---

## 7. Adding a New Source File

1. Create the `.c` file in the correct subdirectory
2. Create a `.h` file in `source/` root if it has a public API
3. Add the `.c` to the appropriate `*_SOURCES` group in the Makefile
4. Add any new `.h` to the `HEADERS` list
5. For OpenWatcom targets, add an explicit compile line:

```makefile
$(WCC) $(WCFLAGS) -fo=newfile.obj subdir/newfile.c
```

6. Add the `.obj` name to `WATCOM_OBJS`

> [!IMPORTANT]
> Don't forget to also update:
> - `help/help.c` — if adding new BASIC keywords
> - `source/SOURCE_TREE.txt` — always
> - `dialect/dialect.c` — if adding a new dialect
> - `core/main.c` — if adding a new module

---

## 8. Common Issues

### "ldisdbl" pragma warning (MSVC)

```
core\ldisdbl.c(2): warning C4068: unknown pragma 'aux'
```

Expected and harmless.  The pragma is for OpenWatcom only.

### "_CRT_SECURE_NO_WARNINGS" (MSVC)

MSVC flags `fopen`, `sprintf`, etc. as "unsafe".  The
`/D_CRT_SECURE_NO_WARNINGS` flag suppresses these.  We use standard C
functions for cross-platform portability.

### 640K limit (OpenWatcom 16-bit)

The full 16-bit build may approach the 640K conventional memory limit.

**Solutions:**
- Use `make watcom386` (32-bit build with extended memory)
- Reduce pool sizes in `config.h`
- Remove optional modules and dialects

### Missing advapi32.lib (MSVC)

The `/link advapi32.lib` flag is required on Windows for the security
subsystem.  If you see linker errors about `RegOpenKeyEx`, ensure
`advapi32.lib` is linked.

### make vs nmake (Windows)

- **GNU make**: `make` or `make msvc`
- **Microsoft nmake**: `nmake /f Makefile msvc`
- The gcc/clang targets use GNU make syntax (pattern rules)
- The `msvc` target is a simple command and works with either

---

## 9. Verification

After building, verify the interpreter:

```bash
./baspp -v                        # Linux
basicpp.exe -v                    # Windows
bpp.exe -v                        # FreeDOS
```

Expected: `BASIC++ 2.0.2`

Run the built-in self-test:

```bash
./baspp -q -c "SELFTEST"
```

Expected: `ALL TESTS PASSED` (79 assertions)

---

## See Also

- `source/SOURCE_TREE.txt` — complete file-by-file reference
- `help/Event_Trapping.txt` — event & interrupt trapping tutorial
- `help/Error_Handling.txt` — ON ERROR GOTO, RESUME, error codes
- `docs/Event_Trapping.md` — markdown event trapping reference
