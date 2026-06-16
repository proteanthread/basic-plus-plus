# FreeDOS Compilation Guide

## Complete Tutorial for Building BASIC++ on FreeDOS

This guide covers everything you need to build a custom BASIC++ binary
for FreeDOS — including how to select your own dialects, modules,
keywords, sections, and memory pools.

> **See also**: [FreeDOS_Build.md](FreeDOS_Build.md) for the quick-reference
> version. This guide is the comprehensive tutorial.

---

## Table of Contents

1. [Prerequisites](#1-prerequisites)
2. [Quick Start](#2-quick-start)
3. [Build Modes](#3-build-modes-16-bit-vs-32-bit)
4. [Default Dialect (PATB)](#4-default-dialect-patb)
5. [Switching the Default Dialect](#5-switching-the-default-dialect)
6. [Selecting Dialects](#6-selecting-dialects)
7. [Selecting Modules](#7-selecting-modules)
8. [Selecting Sections (Feature Groups)](#8-selecting-sections-feature-groups)
9. [Keyword Customization](#9-keyword-customization)
10. [Memory Budget Reference](#10-memory-budget-reference)
11. [BRUN / Bytecode on FreeDOS](#11-brun--bytecode-on-freedos)
12. [Build Scripts Reference](#12-build-scripts-reference)
13. [Adding a Dialect Step-by-Step](#13-adding-a-dialect-step-by-step)
14. [Adding a Module Step-by-Step](#14-adding-a-module-step-by-step)
15. [Stripping Features for Minimal Binary](#15-stripping-features-for-minimal-binary)
16. [Troubleshooting](#16-troubleshooting)
17. [Platform Comparison Table](#17-platform-comparison-table)

---

## 1. Prerequisites

### Hardware / OS
- FreeDOS 1.3 or FreeDOS 1.4
- Minimum 640K conventional memory (512K usable)
- Hard disk with at least 10 MB free space

### Software
- **OpenWatcom C Compiler** (v1.9 or v2.0)
  - 16-bit: `wcc` (8086 real mode compiler)
  - 32-bit: `wcc386` (i386 protected mode compiler)
  - `wmake` (build tool)
  - `wlink` (linker)

### Environment Setup
After installing OpenWatcom, set up your environment:

```
SET WATCOM=C:\WATCOM
SET PATH=%WATCOM%\BINW;%PATH%
SET INCLUDE=%WATCOM%\H
```

Or run `OWSETENV.BAT` from your OpenWatcom installation directory.

### Directory Layout
```
C:\BPP\                     ← project root
├── build-fd.bat             ← FreeDOS build script
├── source\                  ← all source code
│   ├── makefd               ← wmake makefile for FreeDOS
│   ├── config.h             ← memory pools and defaults
│   ├── core\                ← core interpreter files
│   ├── dialect\             ← dialect profile files
│   ├── modules\             ← optional modules
│   └── ...                  ← other source directories
├── docs\                    ← documentation (markdown)
└── help\                    ← documentation (plaintext)
```

---

## 2. Quick Start

From the project root directory on FreeDOS:

```
build-fd
```

That's it. This builds a 16-bit `bpp.exe` with:
- **Dialect**: Palo Alto Tiny BASIC (PATB) only
- **Modules**: STDLIB only
- **Memory**: 512K conventional memory pools

To run:
```
bpp
```

You'll see:
```
BASIC++ 3.3.0
@COPYLEFT ALL WRONGS RESERVED

Ready.
>
```

---

## 3. Build Modes: 16-bit vs 32-bit

### 16-bit (Default)

```
build-fd 16
```

- Runs in real mode on any 8086 or better CPU
- Limited to 640K conventional memory
- Broadest hardware compatibility
- Recommended for original/vintage hardware

### 32-bit (DOS/4GW)

```
build-fd 32
```

- Requires 386 or better CPU
- Uses DOS/4GW protected mode extender
- Access to extended memory beyond 640K
- You can increase memory pools in `config.h`

### When to use 32-bit
- Programs with large arrays (more than 4096 elements)
- Programs with heavy string manipulation
- Programs longer than 1024 lines
- When you want to add more dialects

---

## 4. Default Dialect (PATB)

The FreeDOS build defaults to **Palo Alto Tiny BASIC (PATB)**. This
is the smallest, most memory-efficient dialect — faithful to
Li-Chen Wang's original Tiny BASIC design.

PATB provides:
- PRINT, INPUT, LET, IF/THEN, GOTO, GOSUB/RETURN
- FOR/NEXT loops
- Single-letter variables (A-Z)
- Integer arithmetic
- RND, ABS, SIZE functions
- LIST, RUN, NEW, SAVE, LOAD

PATB does **not** provide (by design):
- String variables (A$, B$, etc.)
- Multi-character variable names
- DEF FN user functions
- Block IF/THEN/ELSE
- WHILE/WEND, DO/LOOP
- SELECT CASE

If you need these features, switch to GW-BASIC or another dialect.

---

## 5. Switching the Default Dialect

### At Runtime
If you've compiled multiple dialects into your binary, you can
switch at runtime:

```
DIALECT "GWBS"
```

### At Compile Time (Permanent Default)

Edit `source\config.h`, find the FreeDOS section:

```c
#undef BASICPP_DEFAULT_DIALECT
#define BASICPP_DEFAULT_DIALECT DIALECT_TINY_BASIC
```

Change to your preferred dialect:

```c
#undef BASICPP_DEFAULT_DIALECT
#define BASICPP_DEFAULT_DIALECT DIALECT_GW_BASIC
```

**Important**: You must also add the dialect's source file to the
build. See [Section 6](#6-selecting-dialects).

### Available Dialect Constants

| Constant             | Code   | Description                    |
|----------------------|--------|--------------------------------|
| DIALECT_TINY_BASIC   | PATB   | Palo Alto Tiny BASIC           |
| DIALECT_GW_BASIC     | GWBS   | GW-BASIC / BASICA              |
| DIALECT_TRS80_I      | TRS1   | TRS-80 Level I                 |
| DIALECT_TRS80_II     | TRS2   | TRS-80 Level II / Model III    |
| DIALECT_ECMA55       | E55    | ECMA-55 Minimal BASIC          |
| DIALECT_ECMA116      | E116   | ECMA-116 Full BASIC            |
| DIALECT_QBASIC       | QBAS   | QBasic / QuickBASIC            |
| DIALECT_APPLE_INT    | AINT   | Apple Integer BASIC             |
| DIALECT_APPLESOFT    | ASFT   | Applesoft BASIC                |
| DIALECT_ATARI        | ATAR   | Atari BASIC                    |
| DIALECT_C64          | C64    | Commodore 64 BASIC V2          |
| DIALECT_COCO         | COCO   | Color Computer Extended BASIC   |
| DIALECT_MBASIC       | MBAS   | MBASIC (CP/M)                  |
| DIALECT_SINCLAIR     | SINC   | Sinclair ZX Spectrum BASIC     |
| DIALECT_SUPERBASIC   | SUPB   | QL SuperBASIC                  |
| DIALECT_SBASIC       | SBAS   | SBASIC (S-BASIC)               |

---

## 6. Selecting Dialects

### Method: Edit `source\makefd`

Open `source\makefd` in a text editor and find the dialect section:

```
INCLUDE_PATB = yes
# INCLUDE_GWBS = yes
# INCLUDE_TRS1 = yes
# INCLUDE_TRS2 = yes
...
```

Uncomment any dialect you want to include:

```
INCLUDE_PATB = yes
INCLUDE_GWBS = yes
INCLUDE_TRS1 = yes
```

Then rebuild:

```
build-fd clean
build-fd
```

### Memory Impact

Each dialect adds approximately 8-12 KB to the binary:

| Dialect | Approx. Size | Memory Impact       |
|---------|-------------|---------------------|
| PATB    | ~8 KB       | Minimal             |
| GWBS    | ~12 KB      | Small               |
| TRS1    | ~8 KB       | Minimal             |
| TRS2    | ~10 KB      | Small               |
| E55     | ~8 KB       | Minimal             |
| E116    | ~12 KB      | Small               |
| QBAS    | ~12 KB      | Small               |
| All 16  | ~160 KB     | Significant (32-bit)|

### Recommended Dialect Combinations

| Use Case                    | Dialects          | Total  |
|-----------------------------|-------------------|--------|
| Minimal Tiny BASIC          | PATB              | ~8 KB  |
| Classic home computer       | PATB + GWBS       | ~20 KB |
| Standards-compliant         | PATB + E116       | ~20 KB |
| TRS-80 compatible           | PATB + TRS1       | ~16 KB |
| Feature-rich (32-bit only)  | PATB + GWBS + E116 + QBAS | ~44 KB |

---

## 7. Selecting Modules

### Available Modules

| Module   | Description                  | Size    | FreeDOS |
|----------|------------------------------|---------|---------|
| STDLIB   | Standard library (always on) | ~8 KB   | ✅      |
| USB      | USB device support           | ~16 KB  | Optional|
| FUJINET  | FujiNet network adapter      | ~12 KB  | Optional|
| UPNP     | UPnP/SSDP discovery          | ~8 KB   | Optional|
| JIT      | Just-in-time compiler        | stub    | No-op   |

### Enabling/Disabling Modules

Edit `source\makefd`:

```
INCLUDE_STDLIB  = yes
# INCLUDE_USB     = yes
# INCLUDE_FUJINET = yes
# INCLUDE_UPNP    = yes
```

Uncomment to include:

```
INCLUDE_STDLIB  = yes
INCLUDE_USB     = yes
```

### JIT on FreeDOS

The JIT module is automatically compiled as a **no-op stub** on FreeDOS.
`MODULE "JIT"` will print `JIT not available on this platform.`

JIT native code generation requires Windows or Linux. On FreeDOS,
use `BRUN` which runs through the portable VM interpreter (same
bytecode, same results, interpreted execution).

---

## 8. Selecting Sections (Feature Groups)

Sections are groups of related source files. You can strip entire
feature groups to create a smaller binary.

### Available Sections

| Section    | Keywords Affected                              | Size    |
|------------|------------------------------------------------|---------|
| GRAPHICS   | PSET, LINE, CIRCLE, DRAW, PAINT, SCREEN       | ~12 KB  |
| SOUND      | SOUND, PLAY, BEEP                              | ~4 KB   |
| STRUCT     | SELECT CASE, SUB, FUNCTION, TYPE               | ~16 KB  |
| FILEIO     | OPEN, CLOSE, INPUT#, PRINT#, GET, PUT          | ~20 KB  |
| BLOCKIO    | BLOAD, BSAVE, block read/write                 | ~4 KB   |
| STREAMIO   | Stream-oriented I/O                             | ~4 KB   |
| SHELL      | SHELL, SYSTEM, ENVIRON$                         | ~4 KB   |
| DEBUG      | TRON, TROFF, BREAK, SELFTEST                   | ~8 KB   |
| HELP       | HELP command (built-in help system)             | ~8 KB   |
| PCODE      | BRUN (bytecode compiler and VM)                 | ~16 KB  |

### Removing a Section

Edit `source\makefd` and comment out the section:

```
INCLUDE_GRAPHICS = yes
INCLUDE_SOUND    = yes
# INCLUDE_STRUCT   = yes      ← REMOVED: no SELECT CASE, SUB, etc.
INCLUDE_FILEIO   = yes
```

**WARNING**: Programs using keywords from a removed section will
produce `WHAT?` errors at runtime.

### Ultra-Minimal Build Example

For the absolute smallest binary (PATB dialect, no file I/O, no
graphics, no sound, no structured programming):

```
INCLUDE_PATB = yes

INCLUDE_STDLIB = yes

# INCLUDE_GRAPHICS = yes
# INCLUDE_SOUND    = yes
# INCLUDE_STRUCT   = yes
# INCLUDE_FILEIO   = yes
# INCLUDE_BLOCKIO  = yes
# INCLUDE_STREAMIO = yes
# INCLUDE_SHELL    = yes
# INCLUDE_DEBUG    = yes
# INCLUDE_HELP     = yes
INCLUDE_PCODE    = yes
```

This gives you PRINT, INPUT, LET, IF, GOTO, GOSUB, FOR/NEXT, BRUN —
and very little else. Approximate binary size: ~80-100 KB.

---

## 9. Keyword Customization

### Runtime Keyword Control

BASIC++ supports runtime keyword customization through the OVERRIDE
system. On FreeDOS, you can:

**Disable a keyword**:
```
OVERRIDE "SHELL" DISABLE
```

**Add a keyword alias**:
```
OVERRIDE "PR" ALIAS "PRINT"
```

**See current overrides**:
```
OVERRIDES
```

### Compile-Time Keyword Control

Individual keywords cannot be selectively excluded at compile time
without modifying source code. However, **sections** (see above)
provide group-level exclusion.

For fine-grained keyword control, use the runtime OVERRIDE system
or edit the keyword registration in `lexer\lexer.c`.

### Configuration File (`basicpp.cfg`)

Create a `basicpp.cfg` file next to `bpp.exe`:

```
dialect=patb
override PR ALIAS PRINT
override ? ALIAS PRINT
override SHELL DISABLE
```

This loads automatically at startup.

---

## 10. Memory Budget Reference

### Default FreeDOS Pools (config.h)

| Pool                | Size      | Purpose                    |
|---------------------|-----------|----------------------------|
| Program memory      | 32 KB     | Stored program text        |
| Variable memory     | 16 KB     | Runtime variable storage   |
| Scratch memory      | 16 KB     | Tokenizer/parser workspace |
| String pool         | 32 KB     | String values during RUN   |
| Array pool          | ~64 KB    | DIM array elements         |
| **Total**           | **~160 KB** | Fits within 512 KB       |

### Limits

| Limit              | Value   | Adjustable in config.h      |
|---------------------|---------|-----------------------------|
| Program lines       | 1,024   | MAX_PROGRAM_LINES           |
| Stack depth         | 64      | MAX_STACK_DEPTH             |
| Named variables     | 128     | MAX_NAMED_VARS              |
| DATA items          | 1,024   | MAX_DATA_ITEMS              |
| DIM arrays          | 32      | MAX_DIM_ARRAYS              |
| Array elements      | 4,096   | MAX_ARRAY_ELEMENTS          |
| User functions      | 32      | MAX_USER_FUNCS              |
| Modules             | 8       | MAX_MODULES                 |
| Breakpoints         | 16      | MAX_BREAKPOINTS             |
| Graphics            | 160x100 | GFX_WIDTH, GFX_HEIGHT       |

### Tuning for Your System

**256K system (extremely tight)**:
```c
#define PROGRAM_MEMORY_SIZE   16384L    /* 16 KB */
#define VARIABLE_MEMORY_SIZE  8192L     /* 8 KB  */
#define SCRATCH_MEMORY_SIZE   8192L     /* 8 KB  */
#define MAX_STRING_POOL       16384L    /* 16 KB */
#define MAX_PROGRAM_LINES     512
#define MAX_ARRAY_ELEMENTS    2048
```

**640K system (generous)**:
```c
#define PROGRAM_MEMORY_SIZE   65536L    /* 64 KB */
#define VARIABLE_MEMORY_SIZE  32768L    /* 32 KB */
#define SCRATCH_MEMORY_SIZE   16384L    /* 16 KB */
#define MAX_STRING_POOL       65536L    /* 64 KB */
#define MAX_PROGRAM_LINES     4096
#define MAX_ARRAY_ELEMENTS    16384
```

**32-bit DOS/4GW (extended memory)**:
```c
#define PROGRAM_MEMORY_SIZE   524288L   /* 512 KB */
#define VARIABLE_MEMORY_SIZE  131072L   /* 128 KB */
#define SCRATCH_MEMORY_SIZE   65536L    /* 64 KB  */
#define MAX_STRING_POOL       1048576L  /* 1 MB   */
#define MAX_PROGRAM_LINES     16384
#define MAX_ARRAY_ELEMENTS    131072
```

After editing, rebuild:
```
build-fd clean
build-fd
```

---

## 11. BRUN / Bytecode on FreeDOS

The BRUN command compiles your BASIC program to bytecode and executes
it through a stack-based virtual machine. This is the **same** bytecode
engine used on Windows and Linux.

```
10 PRINT "HELLO WORLD"
20 FOR I=1 TO 10
30 PRINT I;
40 NEXT I
50 END
BRUN
```

Output:
```
Compiling 5 lines to bytecode...
Compiled: 11 instructions, 11 string bytes
HELLO WORLD
12345678910
```

### JIT vs VM on FreeDOS

On Windows/Linux with `MODULE "JIT"` active, BRUN generates native
x86-64 machine code for simple programs.

On FreeDOS, the JIT is not available. BRUN always uses the portable
VM interpreter. The results are identical — only execution speed
differs.

### Including/Excluding BRUN

BRUN requires the PCODE section. To exclude it:

```
# INCLUDE_PCODE = yes    ← comment out in makefd
```

This removes ~16 KB from the binary but disables `BRUN`, `pcode_compiler`,
`pcode_emit`, and `vm_exec`.

---

## 12. Build Scripts Reference

### `build-fd.bat`

```
build-fd              Build 16-bit (default)
build-fd 16           Build 16-bit (explicit)
build-fd 32           Build 32-bit (DOS/4GW)
build-fd clean        Remove all .obj files
```

### `source\makefd`

Direct wmake usage:

```
cd source
wmake -f makefd                 Build 16-bit
wmake -f makefd BITS=32         Build 32-bit
wmake -f makefd clean           Clean
```

### Build Output

The binary is always `bpp.exe` in the project root directory.

---

## 13. Adding a Dialect Step-by-Step

**Example**: Adding GW-BASIC (GWBS) to a PATB-only build.

### Step 1: Edit `source\makefd`

Uncomment the GWBS line:

```
INCLUDE_PATB = yes
INCLUDE_GWBS = yes          ← uncomment this
```

### Step 2: Register the Dialect

Check `source\dialect\dialect.c` — if the dialect registration
function is guarded by `#ifndef BPP_FREEDOS`, you need to remove
that guard or add a more specific check.

Look for:
```c
#ifndef BPP_FREEDOS
    dialect_register_gwbs();
#endif
```

Change to:
```c
    dialect_register_gwbs();
```

### Step 3: Optionally Change the Default

If you want GWBS as the default instead of PATB, edit `config.h`:

```c
#undef BASICPP_DEFAULT_DIALECT
#define BASICPP_DEFAULT_DIALECT DIALECT_GW_BASIC
```

### Step 4: Rebuild

```
build-fd clean
build-fd
```

### Step 5: Verify

```
bpp
DIALECT
```

Should list both PATB and GWBS.

---

## 14. Adding a Module Step-by-Step

**Example**: Adding the USB module.

### Step 1: Edit `source\makefd`

```
INCLUDE_USB = yes               ← uncomment
```

### Step 2: Register the Module

Check `source\core\boot.c` (or `source\core\main.c`) for the
module registration call. If guarded by `#ifndef BPP_FREEDOS`,
remove the guard:

```c
    mod_usb_register();         /* was guarded, now always called */
```

### Step 3: Rebuild

```
build-fd clean
build-fd
```

### Step 4: Verify

```
bpp
INFO
```

Should show the USB module in the module list.

### Step 5: Check Memory

If you get `SORRY. Cannot allocate memory.` at startup, the binary
plus pools exceed available conventional memory. Either:
- Remove another module/dialect to compensate
- Reduce pool sizes in `config.h`
- Switch to a 32-bit build (`build-fd 32`)

---

## 15. Stripping Features for Minimal Binary

### Goal: Smallest possible BASIC interpreter for FreeDOS

1. Use only PATB dialect
2. Remove all optional sections
3. Reduce memory pools to minimum
4. Use 16-bit build

### makefd configuration:

```
INCLUDE_PATB     = yes

INCLUDE_STDLIB   = yes

# All sections disabled except core
# INCLUDE_GRAPHICS = yes
# INCLUDE_SOUND    = yes
# INCLUDE_STRUCT   = yes
# INCLUDE_FILEIO   = yes
# INCLUDE_BLOCKIO  = yes
# INCLUDE_STREAMIO = yes
# INCLUDE_SHELL    = yes
# INCLUDE_DEBUG    = yes
# INCLUDE_HELP     = yes
# INCLUDE_PCODE    = yes
```

### config.h (ultra-slim):

```c
#define PROGRAM_MEMORY_SIZE   16384L
#define VARIABLE_MEMORY_SIZE  8192L
#define SCRATCH_MEMORY_SIZE   8192L
#define MAX_STRING_POOL       16384L
#define MAX_PROGRAM_LINES     512
#define MAX_STACK_DEPTH       32
#define MAX_NAMED_VARS        64
#define MAX_DATA_ITEMS        512
#define MAX_DIM_ARRAYS        16
#define MAX_ARRAY_ELEMENTS    2048
#define MAX_USER_FUNCS        16
#define MAX_MODULES           4
#define MAX_BREAKPOINTS       8
#define GFX_WIDTH             80
#define GFX_HEIGHT            50
```

This produces a binary under ~80 KB with ~48 KB of memory pools.

---

## 16. Troubleshooting

### "SORRY. Cannot allocate memory."
The interpreter cannot `malloc()` enough memory at startup.
- Reduce pool sizes in `config.h`
- Free conventional memory by unloading TSRs and device drivers
- Use a 32-bit build for access to extended memory

### "Unknown dialect"
You tried to switch to a dialect that isn't compiled in.
- Check which dialects are enabled in `makefd`
- Use `DIALECT` (no argument) to list available dialects

### Link error: "undefined reference to dialect_register_xxx"
You enabled a dialect in `makefd` but didn't remove the
`#ifndef BPP_FREEDOS` guard in `dialect.c`.

### Program runs out of string space
The 32 KB string pool is exhausted.
- Use shorter strings
- Increase `MAX_STRING_POOL` in `config.h`
- Use 32-bit build

### Program runs out of array space
The 4,096 element limit is hit.
- Use smaller arrays
- Increase `MAX_ARRAY_ELEMENTS` in `config.h`
- Use 32-bit build

### "WHAT?" on a valid keyword
The keyword's section may not be compiled in.
- Check that the relevant section is enabled in `makefd`
- Use `KEYWORD "PRINT"` to check if a keyword is available

### wmake errors
- Ensure OpenWatcom environment is set (`OWSETENV.BAT`)
- Ensure you're in the `source\` directory (or using `build-fd.bat`)
- Ensure `wmake`, `wcc`, and `wlink` are on your PATH

---

## 17. Platform Comparison Table

| Feature               | FreeDOS (default) | FreeDOS (full)  | Windows 11    | Linux         |
|-----------------------|-------------------|-----------------|---------------|---------------|
| Dialects              | 1 (PATB)          | Up to 16        | 16 (all)      | 16 (all)      |
| Default dialect       | PATB              | Configurable    | GW-BASIC      | GW-BASIC      |
| Optional modules      | STDLIB only       | All available   | All + JIT     | All + JIT     |
| JIT engine            | ❌ No             | ❌ No           | ✅ Yes        | ✅ Yes        |
| BRUN (VM bytecode)    | ✅ Yes            | ✅ Yes          | ✅ Yes        | ✅ Yes        |
| Program memory        | 32 KB             | Configurable    | 8 MB          | 8 MB          |
| String pool           | 32 KB             | Configurable    | 16 MB         | 16 MB         |
| Program lines         | 1,024             | Configurable    | 65,536        | 65,536        |
| Array elements        | 4,096             | Configurable    | 4,194,304     | 4,194,304     |
| Stack depth           | 64                | Configurable    | 1,024         | 1,024         |
| Named variables       | 128               | Configurable    | 4,096         | 4,096         |
| Graphics resolution   | 160x100           | Configurable    | 320x200       | 320x200       |
| Build tool            | wmake (OpenWatcom) | wmake           | nmake (MSVC)  | make (gcc)    |
| Build script          | build-fd.bat      | build-fd.bat    | build.bat     | make          |
| Binary name           | bpp.exe           | bpp.exe         | basicpp.exe   | baspp         |

---

*End of FreeDOS Compilation Guide*
