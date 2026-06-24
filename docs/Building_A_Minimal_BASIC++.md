# Building a Minimal BASIC++

**Version 4.1.1**

---

## Table of Contents

- What Is a Minimal Build?
- Why Build Minimal?
- The Three Build Tiers
- Step-by-Step: Creating Your Minimal Build
  - Step 1: Choose Your Target Platform
  - Step 2: Choose Your Dialect
  - Step 3: Choose Your Feature Set
  - Step 4: Set Your Memory Pools
  - Step 5: Select Source Files
  - Step 6: Compile
  - Step 7: Test
- The Absolute Minimum (Skeleton Build)
- Source File Dependency Map
- Feature Modules: What Each One Costs
- Dialect Sizes: Memory Cost Per Dialect
- Creating a Custom Minimal Dialect
- Memory Pool Sizing Guide
  - Pool Reference Table
  - Sizing for 2 KB SRAM (Arduino Uno -- Not Viable)
  - Sizing for 8 KB SRAM (Arduino Mega)
  - Sizing for 32-96 KB SRAM (Arduino Due, RP2040)
  - Sizing for 200-520 KB SRAM (ESP32)
  - Sizing for 512 KB Conventional (FreeDOS)
  - Sizing for 4-8 GB RAM (Modern Desktop)
  - Sizing for 64-256 GB RAM (Workstation)
- Removing Features to Save Space
  - Removing File I/O
  - Removing Graphics
  - Removing Arrays and MAT
  - Removing User Types (TYPE/END TYPE)
  - Removing the Help System
  - Removing the Debug/Trace System
  - Removing the Security System
  - Removing the Config File System
  - Removing the VDev Layer
  - Removing the Standard Library
  - Removing the Error Registry
  - Removing the Transpiler/Codegen
- Union Mode in Minimal Builds
- Compiler and Platform Notes
  - GCC (Linux, Raspberry Pi, Cross-Compile)
  - MSVC (Windows 11)
  - OpenWatcom (FreeDOS)
  - ARM GCC (Arduino Due, STM32, RP2040)
  - Xtensa GCC (ESP32 via ESP-IDF)
  - AVR-GCC (Arduino Mega)
  - Clang (macOS, Linux)
- Example: Absolute Minimum Build (36 Files)
- Example: Small Interactive Build (45 Files)
- Example: Full-Featured Embedded Build (60 Files)
- Verifying Your Minimal Build
- Cross-Compiling Between Platforms
  - Cross-Compile on Linux for Windows 11
  - Cross-Compile on Linux for FreeDOS
  - Cross-Compile on Windows 11 for Linux
  - Cross-Compile on Windows 11 for FreeDOS
  - Cross-Compile on Windows 11 for Raspberry Pi
  - Cross-Compilation Summary
- Troubleshooting
- See Also

---

## 1. What Is a Minimal Build?

A minimal build of BASIC++ includes only the source files and features
you actually need. The full interpreter compiles 115 .c files and
produces an executable that uses approximately 90 MB of memory pools.
A minimal build can compile as few as 36 .c files and use under 20 KB
of memory pools.

BASIC++ was designed from the ground up for this kind of subsetting.
Every feature beyond the core parser/runtime is in its own source file
and can be independently included or excluded at compile time.


## 2. Why Build Minimal?

- **Embedded microcontrollers:** ESP32, Arduino Due, Raspberry Pi Pico,
  STM32 -- these have 32 KB to 520 KB of SRAM
- **Vintage hardware:** FreeDOS on real 286/386 PCs with 640K
- **Teaching/learning:** A stripped-down interpreter is easier to
  understand, modify, and debug
- **Single-purpose devices:** Kiosks, lab equipment, data loggers
  that only need a BASIC REPL
- **ROM/firmware size:** Smaller .text segment for flash-constrained
  microcontrollers
- **Fast compilation:** 36 files compile in under 1 second


## 3. The Three Build Tiers

BASIC++ has three pre-configured build profiles in config.h:

| Profile | Define | Pool Total | Dialects | Modules | Target |
|---------|--------|-----------|----------|---------|--------|
| Modern | (default) | ~90 MB | All 16 | All | Windows/Linux |
| FreeDOS | `BPP_FREEDOS` | ~160 KB | 1 (PATB) | None | DOS |
| Embedded | `BPP_EMBEDDED` | ~38 KB | 1 (PATB) | None | MCU |

You can also create a **custom** profile by editing config.h directly.
This guide shows you how to go beyond the pre-configured profiles and
build exactly the interpreter you need.


## 4. Step-by-Step: Creating Your Minimal Build

### Step 1: Choose Your Target Platform

Your platform determines which compiler and memory model to use:

| Platform | Compiler | Memory Model |
|----------|----------|-------------|
| Linux (x86_64) | gcc | 64-bit, plenty of RAM |
| Linux (ARM) | arm-linux-gnueabihf-gcc | 32-bit, 512 MB+ |
| Windows 11 | cl (MSVC) | 64-bit, plenty of RAM |
| FreeDOS 16-bit | wcc (Watcom) | 16-bit, 640K limit |
| FreeDOS 32-bit | wcc386 (Watcom) | 32-bit, extended memory |
| ESP32 | xtensa-esp32-elf-gcc | 32-bit, 200-520 KB |
| Arduino Due | arm-none-eabi-gcc | 32-bit, 96 KB |
| Arduino Mega | avr-gcc | 8-bit, 8 KB |
| Raspberry Pi Pico | arm-none-eabi-gcc | 32-bit, 264 KB |
| STM32 | arm-none-eabi-gcc | 32-bit, varies |

### Step 2: Choose Your Dialect

Every BASIC++ build includes exactly one **default dialect** and
the dialect engine (`dialect.c`). You can include additional dialects
by adding their source files.

**Recommended starting dialects for minimal builds:**

| Dialect | Code | File | Code Size | Best For |
|---------|------|------|-----------|----------|
| Palo Alto Tiny BASIC | PATB | `dialect_patb.c` | ~3 KB | Absolute minimum |
| ECMA-55 Minimal BASIC | E055 | `dialect_ecma55.c` | ~5 KB | ISO standard |
| Apple Integer BASIC | AINT | `dialect_aint.c` | ~4 KB | Integer-only math |
| TRS-80 Level I | TRS1 | `dialect_trs1.c` | ~4 KB | Simple, vintage |
| GW-BASIC | GWBS | `dialect_gwbs.c` | ~8 KB | DOS compatibility |

**To change the default dialect**, edit config.h:
```c
#undef BASICPP_DEFAULT_DIALECT
#define BASICPP_DEFAULT_DIALECT DIALECT_ECMA55  // your choice
```

**Remember:** In Union Mode (the default), ALL core keywords work
regardless of which dialect is active. The dialect only controls:
- Prompts and banners
- Statement separators (`:` vs. `\`)
- Feature gates (SUB/FUNCTION availability, etc.)
- Number formatting

### Step 3: Choose Your Feature Set

Decide which features you need. Each feature is one or more .c files
that can be included or excluded:

| Feature | Files | RAM Cost | Needed? |
|---------|-------|----------|---------|
| REPL (interactive prompt) | Always included | -- | Always |
| PRINT/INPUT/LET | Always included | -- | Always |
| IF/THEN/ELSE, GOTO, GOSUB | `parser_flow.c` | ~2 KB | Almost always |
| FOR/NEXT, WHILE/WEND, DO/LOOP | `parser_loops.c` | ~2 KB | Almost always |
| File I/O (OPEN/CLOSE/PRINT#) | `fileio.c`, `parser_fileio.c` | ~4 KB | If you need files |
| DIM arrays | `parser_mat.c` | ~3 KB | If you need arrays |
| String functions | `builtins_string.c` | ~4 KB | Almost always |
| Math functions | `builtins_math.c` | ~2 KB | Almost always |
| DEF FN / FUNCTION / SUB | `funcreg.c`, `parser_deffn.c` | ~3 KB | If you need functions |
| ON ERROR GOTO | `parser_errhand.c` | ~2 KB | Recommended |
| PEEK/POKE/MEMMAP | `memmap.c`, `builtins_memory.c` | ~2 KB | If you need memory access |
| Graphics (PSET/LINE/CIRCLE) | `parser_graphics.c`, `gfxbuf.c` | ~4 KB | If you need graphics |
| User types (TYPE/END TYPE) | `parser_struct.c` | ~3 KB | Optional |
| Security sandbox | `security.c` | ~2 KB | Recommended |
| HELP command | `help.c`, `parser_help.c` | ~2 KB | Optional |
| Debug (TRON/TROFF/SELFTEST) | `parser_debug.c`, `selftest.c` | ~3 KB | Optional |
| SHELL/EXEC | `parser_shell.c` | ~1 KB | Optional, needs OS |
| Config file | `config_file.c`, `parser_config.c` | ~2 KB | Optional |
| Scope/Override | `scope.c`, `override.c` | ~2 KB | Optional |
| VDev virtual devices | `vdev.c` | ~3 KB | If you need device I/O |
| Program management | `parser_progmgmt.c`, `parser_cmds.c` | ~3 KB | If you need SAVE/LOAD |
| Format USING | `format_using.c`, `format_input.c` | ~3 KB | If you need PRINT USING |
| Transpiler/codegen | `codegen.c`, `ast.c`, `bytecode.c` | ~15 KB | Optional, large |
| External modules | `ext_lib.c`, `ext_func.c`, `ext_plugin.c` | ~6 KB | Optional, needs OS |
| Network I/O | `vdev_net.c`, `builtins_net.c` | ~4 KB | Optional, needs TCP/IP |
| USB | `mod_usb.c` | ~3 KB | Optional, needs USB stack |
| FujiNet | `mod_fujinet.c` | ~3 KB | Optional, needs network |
| UPnP | `mod_upnp.c` | ~3 KB | Optional, needs network |
| JIT | `mod_jit.c` | ~5 KB | Optional, needs MMU |

### Step 4: Set Your Memory Pools

Edit config.h to set pool sizes for your target. See "Memory Pool
Sizing Guide" below for recommended values per platform.

### Step 5: Select Source Files

Create a list of .c files to compile. Start from the "Absolute
Minimum" list (section below) and add features as needed.

### Step 6: Compile

Use your platform's compiler with the appropriate flags:

```bash
# GCC (Linux) -- minimal build
gcc -std=c17 -O2 -I. -o basicpp \
    core/main.c core/memory.c core/errors.c core/value.c \
    core/stringpool.c core/platform.c core/boot.c core/rpn.c \
    core/ldisdbl.c core/security.c core/error_registry.c \
    core/stdlib_core.c core/stdlib_dialect.c \
    lexer/lexer.c lexer/keyword_props.c lexer/alias_lang.c \
    parser/parser.c parser/parser_expr.c spec.c \
    flow/parser_flow.c flow/parser_loops.c \
    variables/parser_vars.c variables/parser_assign.c \
    strings/builtins_string.c math/builtins_math.c \
    functions/funcreg.c functions/parser_deffn.c functions/builtins.c \
    dialect/dialect.c dialect/dialect_patb.c \
    modules/module.c modules/mod_stdlib.c \
    runtime/runtime.c runtime/exec.c \
    misc/parser_misc.c \
    -lm
```

### Step 7: Test

Run the interpreter and verify your feature set:
```
./basicpp
> PRINT "Hello!"
Hello!
> SELFTEST
BASIC++ Self-Test...
```


## 5. The Absolute Minimum (Skeleton Build)

This is the smallest possible BASIC++ build that can run a REPL
and execute BASIC programs. It includes:

- Lexer/tokenizer
- Recursive-descent parser
- Expression evaluator
- Core statements (PRINT, INPUT, LET, REM, END, STOP)
- Control flow (IF/THEN/ELSE, GOTO, GOSUB/RETURN, FOR/NEXT,
  WHILE/WEND, DO/LOOP)
- Single-letter variables (A-Z, A$-Z$)
- String functions (LEFT$, RIGHT$, MID$, LEN, etc.)
- Math functions (SIN, COS, SQR, ABS, INT, RND, etc.)
- DEF FN user functions
- One dialect (PATB or your choice)
- Error handling (basic errors, no ON ERROR GOTO)

**It does NOT include:**
- File I/O (no OPEN/CLOSE/PRINT#)
- DIM arrays (no DIM, no MAT)
- Graphics (no PSET/LINE/CIRCLE)
- User types (no TYPE/END TYPE)
- PEEK/POKE memory access
- HELP command
- Debug/trace (no TRON/TROFF/SELFTEST)
- Security system
- Config file loading
- VDev virtual devices
- SAVE/LOAD program management
- Transpiler/codegen
- External modules

**Files needed (36 files):**

```
core/main.c             core/memory.c           core/errors.c
core/value.c            core/stringpool.c       core/platform.c
core/boot.c             core/rpn.c              core/ldisdbl.c
core/security.c         core/error_registry.c
core/stdlib_core.c      core/stdlib_dialect.c

lexer/lexer.c           lexer/keyword_props.c   lexer/alias_lang.c

parser/parser.c         parser/parser_expr.c    spec.c

flow/parser_flow.c      flow/parser_loops.c

variables/parser_vars.c variables/parser_assign.c

strings/builtins_string.c
math/builtins_math.c

functions/funcreg.c     functions/parser_deffn.c
functions/builtins.c

dialect/dialect.c       dialect/dialect_patb.c

modules/module.c        modules/mod_stdlib.c

runtime/runtime.c       runtime/exec.c

misc/parser_misc.c
```

**Note:** Even in the skeleton build, `security.c` and
`error_registry.c` are included because they are referenced by the
core. They have minimal runtime cost when not actively used.


## 6. Source File Dependency Map

Understanding which files depend on which others is critical for
a minimal build. Here is the dependency tree:

```
core/main.c          -- Entry point, calls boot_init()
  |
  +-- core/boot.c         -- Initialization sequence
  |     |
  |     +-- core/memory.c       -- Pool allocator (required)
  |     +-- core/stringpool.c   -- String interning (required)
  |     +-- core/errors.c       -- Error reporting (required)
  |     +-- core/value.c        -- BValue tagged union (required)
  |     +-- core/platform.c     -- OS detection (required)
  |     +-- core/security.c     -- Sandbox init (required)
  |     +-- core/error_registry.c -- Error codes (required)
  |     +-- core/stdlib_core.c  -- Core stdlib (required)
  |     +-- core/stdlib_dialect.c -- Dialect stdlib (required)
  |     +-- dialect/dialect.c   -- Dialect engine (required)
  |     +-- modules/module.c    -- Module registry (required)
  |     +-- modules/mod_stdlib.c -- STDLIB module (required)
  |
  +-- runtime/runtime.c   -- REPL loop + program executor
  |     |
  |     +-- runtime/exec.c      -- Statement dispatch
  |     +-- lexer/lexer.c       -- Tokenizer (required)
  |     +-- lexer/keyword_props.c -- Keyword metadata (required)
  |     +-- lexer/alias_lang.c  -- Language aliases (required)
  |     +-- parser/parser.c     -- Statement parser (required)
  |     +-- parser/parser_expr.c -- Expression parser (required)
  |     +-- spec.c              -- Specification engine (required)
  |     +-- core/rpn.c          -- RPN evaluator (required)
  |     +-- core/ldisdbl.c      -- Long double (required)
  |
  +-- flow/parser_flow.c   -- IF/THEN, GOTO, GOSUB (recommended)
  +-- flow/parser_loops.c  -- FOR, WHILE, DO (recommended)
  |
  +-- variables/parser_vars.c   -- Variable handling (required)
  +-- variables/parser_assign.c -- LET assignment (required)
  |
  +-- strings/builtins_string.c -- String functions (recommended)
  +-- math/builtins_math.c      -- Math functions (recommended)
  |
  +-- functions/funcreg.c       -- Function registry (required)
  +-- functions/parser_deffn.c  -- DEF FN parser (recommended)
  +-- functions/builtins.c      -- Built-in functions (required)
  |
  +-- dialect/dialect_patb.c    -- At least one dialect (required)
  |
  +-- misc/parser_misc.c        -- Misc statements (required)
```

**Optional branches (add as needed):**
```
  +-- io/parser_io.c            -- PRINT/INPUT to files
  +-- io/parser_fileio.c        -- OPEN/CLOSE/etc.
  +-- io/fileio.c               -- File operations
  +-- io/format_using.c         -- PRINT USING
  +-- io/format_input.c         -- INPUT formatting
  +-- io/device_alias.c         -- Device aliases
  |
  +-- arrays/parser_mat.c       -- DIM, MAT operations
  |
  +-- graphics/parser_graphics.c -- PSET, LINE, CIRCLE
  +-- graphics/gfxbuf.c          -- Graphics buffer
  +-- graphics/builtins_graphics.c -- POINT, etc.
  |
  +-- errhand/parser_errhand.c  -- ON ERROR GOTO
  |
  +-- memory/memmap.c           -- PEEK/POKE/MEMMAP
  +-- memory/builtins_memory.c  -- FRE(), VARPTR
  |
  +-- struct/parser_struct.c    -- TYPE/END TYPE
  |
  +-- help/help.c               -- HELP engine
  +-- help/parser_help.c        -- HELP command
  |
  +-- debug/parser_debug.c      -- TRON/TROFF
  +-- debug/selftest.c          -- SELFTEST
  +-- debug/check.c             -- CHECK/VERIFY
  |
  +-- config/parser_config.c    -- CONFIG statement
  +-- config/config_file.c      -- basicpp.cfg loader
  +-- config/override.c         -- OVERRIDE system
  +-- config/scope.c            -- SCOPE system
  +-- config/scope_stack.c      -- Scope stack
  |
  +-- virtual/vdev.c            -- Virtual device layer
  |
  +-- progmgmt/parser_progmgmt.c -- SAVE/LOAD/MERGE
  +-- progmgmt/parser_cmds.c     -- LIST/RENUM/DELETE
  |
  +-- shell/parser_shell.c      -- SHELL/EXEC
  |
  +-- system/builtins_system.c  -- ENVIRON$, COMMAND$
  +-- system/builtins_io.c      -- INP/OUT
  |
  +-- codegen/ast.c             -- AST builder
  +-- codegen/codegen.c         -- C code generator
  +-- codegen/bytecode.c        -- Bytecode emitter
  +-- codegen/detok.c           -- Detokenizer
```


## 7. Feature Modules: What Each One Costs

| Feature | Files Added | Code Size | RAM Cost | Compile Time |
|---------|------------|-----------|----------|-------------|
| Core (skeleton) | 36 | ~150 KB | ~20 KB pools | 1 sec |
| + Control flow | +2 | +8 KB | +0 | +0.1 sec |
| + File I/O | +6 | +20 KB | +0 | +0.2 sec |
| + Arrays/MAT | +1 | +10 KB | +16-64 KB | +0.1 sec |
| + Graphics | +3 | +12 KB | +2-50 KB | +0.1 sec |
| + Error handling | +1 | +4 KB | +0 | +0.1 sec |
| + Memory (PEEK/POKE) | +2 | +6 KB | +4-64 KB | +0.1 sec |
| + User types | +1 | +8 KB | +2-8 KB | +0.1 sec |
| + Help system | +2 | +6 KB | +0 | +0.1 sec |
| + Debug/trace | +3 | +10 KB | +0 | +0.1 sec |
| + Program mgmt | +2 | +8 KB | +0 | +0.1 sec |
| + Security | +0 (always) | +4 KB | +0 | +0 |
| + Config system | +4 | +10 KB | +1 KB | +0.1 sec |
| + VDev layer | +1 | +8 KB | +2 KB | +0.1 sec |
| + Transpiler | +4 | +30 KB | +10 KB | +0.3 sec |
| + External modules | +4 | +15 KB | +4 KB | +0.2 sec |
| Full build | 115 | ~400 KB | ~90 MB | 5 sec |


## 8. Dialect Sizes: Memory Cost Per Dialect

Each dialect is a single .c file. You can include 1 or all 16:

| Dialect | Code | File | Code Size | Registration |
|---------|------|------|-----------|-------------|
| Palo Alto Tiny BASIC | PATB | `dialect_patb.c` | ~3 KB | `dialect_register_patb()` |
| TRS-80 Level I | TRS1 | `dialect_trs1.c` | ~4 KB | `dialect_register_trs80_i()` |
| TRS-80 Level II | TRS2 | `dialect_trs2.c` | ~5 KB | `dialect_register_trs80_ii()` |
| Apple Integer BASIC | AINT | `dialect_aint.c` | ~4 KB | `dialect_register_apple_integer()` |
| AppleSoft BASIC | ASFT | `dialect_asft.c` | ~6 KB | `dialect_register_applesoft()` |
| Atari BASIC | ATAR | `dialect_atari.c` | ~5 KB | `dialect_register_atari()` |
| Commodore 64 BASIC | C64B | `dialect_c64.c` | ~6 KB | `dialect_register_c64()` |
| Color Computer BASIC | COCO | `dialect_coco.c` | ~5 KB | `dialect_register_coco()` |
| GW-BASIC | GWBS | `dialect_gwbs.c` | ~8 KB | `dialect_register_gwbasic()` |
| ECMA-55 Minimal | E055 | `dialect_ecma55.c` | ~5 KB | `dialect_register_ecma55()` |
| ECMA-116 Full | E116 | `dialect_ecma116.c` | ~10 KB | `dialect_register_ecma116()` |
| QBasic | QBAS | `dialect_qbasic.c` | ~12 KB | `dialect_register_qbasic()` |
| MBASIC (CP/M) | MBAS | `dialect_mbasic.c` | ~5 KB | `dialect_register_mbasic()` |
| Sinclair BASIC | SINC | `dialect_sinclair.c` | ~5 KB | `dialect_register_sinclair()` |
| SuperBASIC (QL) | SBAS | `dialect_superbasic.c` | ~6 KB | `dialect_register_superbasic()` |
| SUPER BASIC | SUPS | `dialect_sbasic.c` | ~8 KB | `dialect_register_sbasic()` |
| **All 16 dialects** | | | **~97 KB** | |

**Adding a dialect to a minimal build:**

1. Add the .c file to your compile command
2. In `dialect.c`, call the registration function inside `dialect_init()`
3. Update `BASICPP_DEFAULT_DIALECT` in config.h if desired


## 9. Creating a Custom Minimal Dialect

If none of the 16 built-in dialects fits your needs, you can create
your own. A dialect is a single .c file that registers:

- A 4-character code (e.g., "MYBS")
- A display name
- Feature flags (which features are enabled in strict mode)
- Prompt strings
- Statement separator character

**Template for a custom dialect (dialect_custom.c):**

```c
// dialect_custom.c -- My Custom Minimal Dialect
//
// PURPOSE:
//   A minimal dialect for embedded use. Enables only core
//   BASIC statements with no advanced features.

#include "dialect.h"

void dialect_register_custom(void) {
    DialectProfile p;
    memset(&p, 0, sizeof(p));

    p.id          = DIALECT_USER;      // Use the USER slot
    p.code        = "MYBS";            // 4-char code
    p.name        = "My Minimal BASIC";
    p.version     = "1.0";
    p.separator   = ':';               // Statement separator
    p.prompt      = "> ";
    p.ready       = "Ready.";

    // Feature flags -- set 1 to enable, 0 to disable
    p.has_let_optional  = 1;  // LET is optional
    p.has_line_numbers  = 1;  // Line numbers required
    p.has_goto          = 1;  // GOTO allowed
    p.has_gosub         = 1;  // GOSUB/RETURN allowed
    p.has_for_next      = 1;  // FOR/NEXT loops
    p.has_while_wend    = 0;  // No WHILE/WEND (strict)
    p.has_do_loop       = 0;  // No DO/LOOP (strict)
    p.has_if_block      = 0;  // No block IF (strict)
    p.has_sub_function  = 0;  // No SUB/FUNCTION (strict)
    p.has_select_case   = 0;  // No SELECT CASE (strict)
    p.has_def_fn        = 1;  // DEF FN allowed
    p.has_dim           = 1;  // DIM arrays allowed
    p.has_data_read     = 1;  // DATA/READ allowed
    p.has_on_error      = 0;  // No ON ERROR (strict)
    p.has_file_io       = 0;  // No file I/O (strict)
    p.has_graphics      = 0;  // No graphics (strict)
    p.has_sound         = 0;  // No sound (strict)

    dialect_register(&p);
}
```

**Remember:** These flags only matter in `OPTION STRICT` mode.
In Union Mode (the default), all keywords work regardless of flags.


## 10. Memory Pool Sizing Guide

### Pool Reference Table

| Pool | config.h Define | What It Stores |
|------|----------------|---------------|
| Program memory | `PROGRAM_MEMORY_SIZE` | Stored BASIC program text (source lines) |
| Variable memory | `VARIABLE_MEMORY_SIZE` | Runtime variables, stack frames |
| Scratch memory | `SCRATCH_MEMORY_SIZE` | Tokenizer/parser workspace |
| String pool | `MAX_STRING_POOL` | All string values during RUN |
| Array elements | `MAX_ARRAY_ELEMENTS` | Total array slots (x16 bytes each) |
| Program lines | `MAX_PROGRAM_LINES` | Maximum source lines |
| Stack depth | `MAX_STACK_DEPTH` | GOSUB/FOR/WHILE nesting depth |
| Named variables | `MAX_NAMED_VARS` | Named variables (beyond A-Z) |
| DATA items | `MAX_DATA_ITEMS` | DATA statement values |
| DIM arrays | `MAX_DIM_ARRAYS` | Number of distinct arrays |
| User functions | `MAX_USER_FUNCS` | DEF FN / FUNCTION count |
| Breakpoints | `MAX_BREAKPOINTS` | Debug breakpoints |
| User types | `MAX_USER_TYPES` | TYPE definitions |
| Graphics | `GFX_WIDTH` x `GFX_HEIGHT` | Graphics buffer (WxH bytes) |
| Memory segment | `MAX_MEM_SEGMENT` | PEEK/POKE virtual memory |

### Sizing for 8 KB SRAM (Arduino Mega -- Marginal)

```c
#define PROGRAM_MEMORY_SIZE   1024L    // 1 KB
#define VARIABLE_MEMORY_SIZE  512L     // 512 bytes
#define SCRATCH_MEMORY_SIZE   256L     // 256 bytes
#define MAX_STRING_POOL       512L     // 512 bytes
#define MAX_PROGRAM_LINES     32
#define MAX_STACK_DEPTH       8
#define MAX_NAMED_VARS        0        // A-Z only (26 vars)
#define MAX_DATA_ITEMS        32
#define MAX_DIM_ARRAYS        4
#define MAX_ARRAY_ELEMENTS    64       // ~1 KB
#define MAX_USER_FUNCS        4
#define MAX_MODULES           1
#define MAX_BREAKPOINTS       0
#define MAX_USER_TYPES        0
#define GFX_WIDTH             0        // No graphics
#define GFX_HEIGHT            0
// Total: ~3 KB pools (very tight but possible)
```

### Sizing for 32-96 KB SRAM (Arduino Due, RP2040)

```c
#define PROGRAM_MEMORY_SIZE   8192L    // 8 KB
#define VARIABLE_MEMORY_SIZE  4096L    // 4 KB
#define SCRATCH_MEMORY_SIZE   2048L    // 2 KB
#define MAX_STRING_POOL       8192L    // 8 KB
#define MAX_PROGRAM_LINES     256
#define MAX_STACK_DEPTH       32
#define MAX_NAMED_VARS        64
#define MAX_DATA_ITEMS        256
#define MAX_DIM_ARRAYS        16
#define MAX_ARRAY_ELEMENTS    1024     // ~16 KB
#define MAX_USER_FUNCS        16
#define MAX_MODULES           4
#define MAX_BREAKPOINTS       8
#define MAX_USER_TYPES        4
#define GFX_WIDTH             64
#define GFX_HEIGHT            32       // ~2 KB
// Total: ~38 KB pools
```

### Sizing for 200-520 KB SRAM (ESP32)

Use the `BPP_EMBEDDED` profile as-is (38 KB pools), or increase
for PSRAM-equipped boards (see Building_For_ESP32.md).

### Sizing for 512 KB Conventional (FreeDOS)

Use the `BPP_FREEDOS` profile (160 KB pools). See
Building_For_FreeDOS.md for details.

### Sizing for 4-8 GB RAM (Modern Desktop)

Use the default profile but reduce the largest pools:

```c
#define MAX_STRING_POOL       1048576L   // 1 MB (was 16 MB)
#define MAX_ARRAY_ELEMENTS    262144     // 256K (was 4M)
// Total: ~12 MB pools (was ~90 MB)
```

### Sizing for 64-256 GB RAM (Workstation)

Maximize everything for large programs and data:

```c
#define PROGRAM_MEMORY_SIZE   16777216L  // 16 MB
#define MAX_STRING_POOL       67108864L  // 64 MB
#define MAX_ARRAY_ELEMENTS    16777216   // 16M elements (~256 MB)
#define MAX_PROGRAM_LINES     262144     // 256K lines
#define MAX_NAMED_VARS        16384
#define MAX_STACK_DEPTH       4096
// Total: ~340 MB pools
```


## 11. Removing Features to Save Space

For each feature you want to remove, the steps are:

1. Remove the .c file from your compile command / Makefile
2. Comment out or guard the registration call in the relevant
   init function
3. Rebuild

Below are specific instructions for each removable feature.

### Removing File I/O

Remove: `io/parser_io.c`, `io/parser_fileio.c`, `io/fileio.c`,
`io/format_using.c`, `io/format_input.c`, `io/device_alias.c`

Guard in `parser.c`: wrap `parser_io_dispatch()` calls in
`#ifndef BPP_NO_FILE_IO`.

**Effect:** OPEN, CLOSE, PRINT#, INPUT#, LINE INPUT#, GET#, PUT#,
FIELD, LSET, RSET, EOF, LOF, LOC, SEEK, LOCK, UNLOCK are
unavailable. PRINT and INPUT to the console still work.

### Removing Graphics

Remove: `graphics/parser_graphics.c`, `graphics/gfxbuf.c`,
`graphics/builtins_graphics.c`

Set `GFX_WIDTH` and `GFX_HEIGHT` to 0 in config.h.

**Effect:** PSET, LINE, CIRCLE, PAINT, DRAW, SCREEN, VIEW,
WINDOW, PALETTE, PCOPY, POINT are unavailable.
Saves code size plus the graphics buffer (WxH bytes).

### Removing Arrays and MAT

Remove: `arrays/parser_mat.c`

Set `MAX_DIM_ARRAYS` and `MAX_ARRAY_ELEMENTS` to 0 in config.h.

**Effect:** DIM, ERASE, MAT, LBOUND, UBOUND are unavailable.
Programs must use only single-letter variables (A-Z).
Saves the array pool (up to 64 MB on modern builds).

### Removing User Types (TYPE/END TYPE)

Remove: `struct/parser_struct.c`

Set `MAX_USER_TYPES` to 0 in config.h.

**Effect:** TYPE/END TYPE, field access with `.` are unavailable.

### Removing the Help System

Remove: `help/help.c`, `help/parser_help.c`

**Effect:** HELP command is unavailable. Saves ~2 KB code.

### Removing the Debug/Trace System

Remove: `debug/parser_debug.c`, `debug/selftest.c`, `debug/check.c`

**Effect:** TRON, TROFF, SELFTEST, CHECK, VERIFY, BACKTRACE
are unavailable. Saves ~10 KB code.

### Removing the VDev Layer

Remove: `virtual/vdev.c`

**Effect:** Virtual devices (CON:, LPT:, GPIO:, etc.) are
unavailable. File I/O still works via stdio. No DEVICES command.

### Removing the Transpiler/Codegen

Remove: `codegen/ast.c`, `codegen/codegen.c`, `codegen/bytecode.c`,
`codegen/detok.c`, `progmgmt/compiler.c`

**Effect:** COMPILE command is unavailable. Saves ~30 KB code.
This is the single largest savings for embedded builds.


## 12. Union Mode in Minimal Builds

Union Mode is BASIC++'s default operating mode and is especially
valuable in minimal builds. Even if you only compile one dialect
(e.g., PATB), Union Mode gives you access to ALL core BASIC keywords:

```basic
> ' PATB dialect active, but Union Mode lets us use GW-BASIC syntax:
> WHILE X < 10
> X = X + 1
> WEND
```

Without Union Mode (OPTION STRICT), only the active dialect's
keywords would be recognized.

**For minimal builds, Union Mode means you get a full-featured
BASIC without needing to compile multiple dialects.** One dialect
plus Union Mode gives you the same keyword coverage as all 16
dialects combined.


## 13. Compiler and Platform Notes

### GCC (Linux, Raspberry Pi, Cross-Compile)

```bash
gcc -std=c17 -Wall -Wextra -O2 -I. -o basicpp [files...] -lm
```

### MSVC (Windows 11)

```batch
cl /TC /std:c17 /W3 /O2 /I. /D_CRT_SECURE_NO_WARNINGS [files...] /Fe:basicpp.exe
```

### OpenWatcom (FreeDOS)

```
wcc -ml -0 -bt=dos -za -wx -DBPP_FREEDOS -i=. [files...]
wlink name bpp.exe system dos file *.obj
```

### ARM GCC (Arduino Due, STM32, RP2040)

```bash
arm-none-eabi-gcc -std=c17 -mcpu=cortex-m3 -mthumb -Os \
    -DBPP_EMBEDDED -I. [files...] -lm -lnosys
```

### Xtensa GCC (ESP32)

Used through ESP-IDF's build system (idf.py build). See
Building_For_ESP32.md.

### AVR-GCC (Arduino Mega)

```bash
avr-gcc -std=c17 -mmcu=atmega2560 -Os -DBPP_EMBEDDED -I. [files...]
```

**Warning:** With only 8 KB SRAM, the Arduino Mega is marginal.
Only the absolute minimum skeleton build will fit.

### Clang (macOS, Linux)

```bash
clang -std=c17 -Wall -O2 -I. -o basicpp [files...] -lm
```


## 14. Example: Absolute Minimum Build (36 Files)

REPL with PRINT/INPUT, variables, math, strings, control flow,
DEF FN. No file I/O, no arrays, no graphics, no SAVE/LOAD.

Pool total: ~3-20 KB depending on platform.
Code size: ~150 KB.

```bash
gcc -std=c17 -O2 -I. -DBPP_EMBEDDED -o basicpp_min \
    core/main.c core/memory.c core/errors.c core/value.c \
    core/stringpool.c core/platform.c core/boot.c core/rpn.c \
    core/ldisdbl.c core/security.c core/error_registry.c \
    core/stdlib_core.c core/stdlib_dialect.c \
    lexer/lexer.c lexer/keyword_props.c lexer/alias_lang.c \
    parser/parser.c parser/parser_expr.c spec.c \
    flow/parser_flow.c flow/parser_loops.c \
    variables/parser_vars.c variables/parser_assign.c \
    strings/builtins_string.c math/builtins_math.c \
    functions/funcreg.c functions/parser_deffn.c functions/builtins.c \
    dialect/dialect.c dialect/dialect_patb.c \
    modules/module.c modules/mod_stdlib.c \
    runtime/runtime.c runtime/exec.c misc/parser_misc.c -lm
```


## 15. Example: Small Interactive Build (45 Files)

Adds file I/O, arrays, error handling, SAVE/LOAD, and HELP.
Good for a self-contained learning environment.

```bash
# Same as above, plus:
    io/parser_io.c io/parser_fileio.c io/fileio.c \
    io/format_using.c io/format_input.c io/device_alias.c \
    arrays/parser_mat.c \
    errhand/parser_errhand.c \
    progmgmt/parser_progmgmt.c progmgmt/parser_cmds.c \
    help/parser_help.c help/help.c \
    memory/memmap.c memory/builtins_memory.c
```


## 16. Example: Full-Featured Embedded Build (60 Files)

Everything except network, USB, FujiNet, UPnP, JIT, and transpiler.
This is the `BPP_EMBEDDED` profile used for ESP32 and Arduino Due.

See `Building_For_ESP32.md` or `Building_For_Arduino.md` for the
complete file list.


## 17. Verifying Your Minimal Build

After building, run these tests to verify:

```basic
> PRINT 2 + 2
4
> PRINT "Hello, World!"
Hello, World!
> 10 FOR I = 1 TO 5
> 20 PRINT I; " ";
> 30 NEXT I
> RUN
1  2  3  4  5
> DEF FN SQ(X) = X * X
> PRINT FN SQ(7)
49
> PRINT LEFT$("HELLO", 3)
HEL
> PRINT SIN(3.14159/2)
1
```

If any of these fail, you are missing a required source file.
Check the dependency map (section 6) and verify all required
files are included.


## 18. Cross-Compiling Between Platforms

BASIC++ is pure C17 with zero external dependencies, which makes it
one of the easiest projects to cross-compile. You can build an
executable for a different OS without leaving your current system.

### Cross-Compile on Linux for Windows 11

Install MinGW-w64 (the Windows cross-compiler):

```bash
# Debian / Ubuntu
sudo apt install gcc-mingw-w64-x86-64

# Fedora
sudo dnf install mingw64-gcc

# Arch
sudo pacman -S mingw-w64-gcc
```

Then compile:
```bash
cd source
x86_64-w64-mingw32-gcc -std=c17 -O2 -I. \
    -D_CRT_SECURE_NO_WARNINGS \
    core/*.c parser/*.c lexer/*.c flow/*.c io/*.c \
    filemgmt/*.c graphics/*.c variables/*.c arrays/*.c \
    strings/*.c math/*.c functions/*.c struct/*.c \
    errhand/*.c config/*.c debug/*.c help/*.c \
    progmgmt/*.c memory/*.c system/*.c virtual/*.c \
    dialect/*.c modules/*.c codegen/*.c runtime/*.c \
    shell/*.c misc/*.c spec.c \
    -o ../basicpp.exe -lm
```

The resulting `basicpp.exe` runs natively on Windows 11. Copy it
to any Windows machine -- no installation needed.

**Minimal cross-compile (36 files):**
```bash
x86_64-w64-mingw32-gcc -std=c17 -O2 -I. -DBPP_EMBEDDED \
    core/main.c core/memory.c core/errors.c core/value.c \
    core/stringpool.c core/platform.c core/boot.c core/rpn.c \
    core/ldisdbl.c core/security.c core/error_registry.c \
    core/stdlib_core.c core/stdlib_dialect.c \
    lexer/lexer.c lexer/keyword_props.c lexer/alias_lang.c \
    parser/parser.c parser/parser_expr.c spec.c \
    flow/parser_flow.c flow/parser_loops.c \
    variables/parser_vars.c variables/parser_assign.c \
    strings/builtins_string.c math/builtins_math.c \
    functions/funcreg.c functions/parser_deffn.c functions/builtins.c \
    dialect/dialect.c dialect/dialect_patb.c \
    modules/module.c modules/mod_stdlib.c \
    runtime/runtime.c runtime/exec.c misc/parser_misc.c \
    -o ../basicpp_min.exe -lm
```

### Cross-Compile on Linux for FreeDOS

Install OpenWatcom for Linux:
```bash
# Download from https://github.com/open-watcom/open-watcom-v2/releases
chmod +x open-watcom-2_0-c-linux-x64
sudo ./open-watcom-2_0-c-linux-x64
export WATCOM=/opt/watcom
export PATH=$WATCOM/binl64:$PATH
export INCLUDE=$WATCOM/h
```

Build a 16-bit FreeDOS executable:
```bash
cd source
make watcom
# Produces bpp.exe (runs on FreeDOS / DOSBox)
```

Or build a 32-bit DOS/4GW executable:
```bash
make watcom386
```

Test with DOSBox:
```bash
cp ../bpp.exe ~/dosbox/
dosbox -c "mount c ~/dosbox" -c "c:" -c "bpp.exe"
```

### Cross-Compile on Windows 11 for Linux

Install WSL2 (Windows Subsystem for Linux), then build inside it:

```powershell
# From PowerShell (install WSL if needed):
wsl --install -d Ubuntu
```

Inside WSL:
```bash
cd /mnt/c/Users/YourName/GitHub/basic-plus-plus/source
gcc -std=c17 -O2 -I. \
    core/*.c parser/*.c lexer/*.c flow/*.c io/*.c \
    filemgmt/*.c graphics/*.c variables/*.c arrays/*.c \
    strings/*.c math/*.c functions/*.c struct/*.c \
    errhand/*.c config/*.c debug/*.c help/*.c \
    progmgmt/*.c memory/*.c system/*.c virtual/*.c \
    dialect/*.c modules/*.c codegen/*.c runtime/*.c \
    shell/*.c misc/*.c spec.c \
    -o ../baspp -lm
```

The resulting `baspp` is a native Linux ELF binary. Copy it to
any Linux machine.

**Alternative without WSL:** Install a Linux cross-compiler toolchain:
```powershell
# Using MSYS2 (msys2.org):
pacman -S mingw-w64-x86_64-cross-gcc
```

### Cross-Compile on Windows 11 for FreeDOS

Install OpenWatcom v2 for Windows from:
https://github.com/open-watcom/open-watcom-v2/releases

From the Developer Command Prompt:
```batch
SET WATCOM=C:\WATCOM
SET PATH=%WATCOM%\BINNT;%PATH%
SET INCLUDE=%WATCOM%\H

cd source
make watcom
REM Produces bpp.exe (runs on FreeDOS / DOSBox)
```

Or compile manually for fine control:
```batch
FOR %%f IN (core\*.c parser\*.c lexer\*.c) DO (
    wcc -ml -0 -bt=dos -za -wx -DBPP_FREEDOS -i=. %%f
)
wlink name ..\bpp.exe system dos file *.obj
```

Test with DOSBox:
```batch
copy ..\bpp.exe C:\DOSBox\
dosbox -c "mount c c:\dosbox" -c "c:" -c "bpp.exe"
```

### Cross-Compile on Windows 11 for Raspberry Pi

Install the ARM cross-compiler via MSYS2 or use WSL:

**Using WSL:**
```bash
sudo apt install gcc-arm-linux-gnueabihf
cd /mnt/c/Users/YourName/GitHub/basic-plus-plus/source
arm-linux-gnueabihf-gcc -std=c17 -O2 -I. \
    core/*.c parser/*.c lexer/*.c flow/*.c io/*.c \
    filemgmt/*.c graphics/*.c variables/*.c arrays/*.c \
    strings/*.c math/*.c functions/*.c struct/*.c \
    errhand/*.c config/*.c debug/*.c help/*.c \
    progmgmt/*.c memory/*.c system/*.c virtual/*.c \
    dialect/*.c modules/*.c codegen/*.c runtime/*.c \
    shell/*.c misc/*.c spec.c \
    -o ../baspp_rpi -lm
```

Copy `baspp_rpi` to the Raspberry Pi via SCP:
```bash
scp ../baspp_rpi pi@raspberrypi.local:~/basicpp
```

### Cross-Compilation Summary

| Host OS | Target | Compiler | Output |
|---------|--------|----------|--------|
| Linux | Windows 11 | `x86_64-w64-mingw32-gcc` | `basicpp.exe` |
| Linux | FreeDOS 16-bit | `wcc` (Watcom) | `bpp.exe` |
| Linux | FreeDOS 32-bit | `wcc386` (Watcom) | `bpp.exe` |
| Linux | Raspberry Pi | `arm-linux-gnueabihf-gcc` | `baspp` |
| Linux | ESP32 | `xtensa-esp32-elf-gcc` (ESP-IDF) | firmware.bin |
| Linux | Arduino Due | `arm-none-eabi-gcc` | firmware.bin |
| Windows 11 | Linux | `gcc` (via WSL) | `baspp` |
| Windows 11 | FreeDOS 16-bit | `wcc` (Watcom) | `bpp.exe` |
| Windows 11 | FreeDOS 32-bit | `wcc386` (Watcom) | `bpp.exe` |
| Windows 11 | Raspberry Pi | `arm-linux-gnueabihf-gcc` (WSL) | `baspp` |
| Windows 11 | ESP32 | `idf.py build` (ESP-IDF) | firmware.bin |
| Windows 11 | Arduino | Arduino IDE / PlatformIO | firmware.bin |


## 19. Troubleshooting

**Linker error: "undefined reference to parser_xxx_dispatch"**
You removed a parser module but the dispatch table in parser.c
still references it. Comment out the dispatch call or add an
`#ifdef` guard.

**"SORRY. Cannot allocate memory."**
Pool sizes are too large for your system. Reduce pool sizes in
config.h (see section 10).

**Crashes at startup with no error message**
The C stack is too small. On embedded systems, increase the stack
size in your linker script or FreeRTOS config.

**"Unknown keyword: WHILE"**
You are in OPTION STRICT mode with a dialect that does not support
WHILE. Switch to Union Mode: `OPTION STRICT OFF`

**Compiler warning: "implicit declaration of function"**
You are missing a source file that another file depends on.
Check the dependency map (section 6).

**Cross-compile produces wrong binary format**
Verify the correct compiler prefix. Use `file basicpp.exe` on
Linux to confirm it is a PE32+ executable (Windows) or
`file baspp` to confirm it is an ELF binary (Linux).

**MinGW cross-compile: "cannot find -lm"**
Some MinGW builds bundle libm into libc. Try removing `-lm` from
the command line, or install the development libraries:
`sudo apt install mingw-w64-x86-64-dev`


## 20. See Also

- `Building_For_FreeDOS.md` -- FreeDOS-specific build guide
- `Building_For_ESP32.md` -- ESP32-specific build guide
- `Building_For_Arduino.md` -- Arduino-specific build guide
- `Building_For_Raspberry_Pi.md` -- Raspberry Pi build guide
- `Embedded_Platforms.md` -- Overview of all embedded targets
- `How_To_Compile.md` -- Compiler flag reference
- `Creating_Dialects.md` -- Full dialect creation guide
- `Internals_And_Architecture.md` -- Architecture overview
