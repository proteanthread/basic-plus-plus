# How to Compile BASIC++

**Version 4.1.1**


---

## Table of Contents

- Source Files
  - Core
  - Language
  - Subsystems
  - Infrastructure
- Windows (MSVC)
- Linux (GCC)
- macOS (Clang)
- BSD / Other Unix
- Cross-Compiling
- Using the Makefile
- Verification
- Build-Time Configuration

---

BASIC++ is written in portable C17. It has **zero external dependencies**. Any standards-conforming C compiler will build it.

---

## 1. Source Files

The project consists of 28 source files (28 `.c` + 28 `.h`):

### Core

| File | Description |
|------|-------------|
| `main.c` | Entry point, REPL loop |
| `config.h` | Build-time constants and limits |
| `errors.c/.h` | Error codes and reporting |
| `memory.c/.h` | Program store (line storage) |

### Language

| File | Description |
|------|-------------|
| `lexer.c/.h` | Tokenizer (200+ keywords) |
| `parser.c/.h` | Statement/expression parser |
| `exec.c/.h` | Execution engine (RUN/CONT) |
| `runtime.c/.h` | Variable storage, stack, state |
| `value.c/.h` | BValue polymorphic type (int/float/string) |
| `stringpool.c/.h` | String memory pool |
| `dialect.c/.h` | Multi-dialect configuration |
| `detok.c/.h` | Detokenizer (LIST formatting) |

### Subsystems

| File | Description |
|------|-------------|
| `fileio.c/.h` | File I/O (sequential/random/binary) |
| `vdev.c/.h` | Virtual device layer |
| `gfxbuf.c/.h` | Graphics framebuffer |
| `memmap.c/.h` | Virtual memory maps |
| `security.c/.h` | Sandboxing and trust levels |
| `module.c/.h` | Module/plugin system |
| `mod_stdlib.c/.h` | Standard library module |

### Infrastructure

| File | Description |
|------|-------------|
| `funcreg.c/.h` | Function registry |
| `builtins.c/.h` | Builtin function implementations |
| `ast.c/.h` | Abstract syntax tree |
| `codegen.c/.h` | Code generator |
| `compiler.c/.h` | Compiler driver |
| `bytecode.c/.h` | Bytecode definitions |
| `vm.c/.h` | Virtual machine |
| `platform.c/.h` | Platform abstraction |
| `selftest.c/.h` | Built-in test suite |
| `help.c/.h` | HELP command content |

---

## 2. Windows (MSVC)

**Requirements:** Microsoft Visual Studio 2019 or later (Community is fine), or Build Tools for Visual Studio (`cl.exe` only).

From a Developer Command Prompt:

**Basic build:**

```batch
cl /TC /O2 /Fe:basicpp.exe *.c
```

**Optimized release build:**

```batch
cl /TC /W4 /WX /O2 /Oi /Ot /GL /GS- /Gy /fp:fast ^
   /D_CRT_SECURE_NO_WARNINGS /DNDEBUG ^
   /Fe:basicpp.exe *.c ^
   /link /LTCG /OPT:REF /OPT:ICF
```

| Flag | Description |
|------|-------------|
| `/TC` | Compile as C (not C++) |
| `/W4 /WX` | Maximum warnings, warnings as errors |
| `/O2` | Maximum speed optimization |
| `/Oi` | Enable intrinsic functions |
| `/Ot` | Favor fast code over small code |
| `/GL` | Whole-program optimization (compile time) |
| `/LTCG` | Link-time code generation (link time) |
| `/GS-` | Disable stack buffer security checks |
| `/Gy` | Enable function-level linking |
| `/fp:fast` | Fast floating-point model |
| `/OPT:REF` | Eliminate unreferenced functions |
| `/OPT:ICF` | Fold identical code sections |
| `/DNDEBUG` | Disable `assert()` in release builds |

Expected output size: **~346 KB**

---

## 3. Linux (GCC)

**Requirements:** GCC 4.x or later (any version with C17 support), GNU Make (optional).

**Basic build:**

```bash
gcc -ansi -pedantic -O2 -o basicpp *.c -lm
```

**Optimized release build:**

```bash
gcc -std=c17 -pedantic -Wall -Wextra -Werror \
    -O3 -march=native -flto -DNDEBUG \
    -o basicpp *.c -lm
```

**With Make:**

```bash
make
```

| Flag | Description |
|------|-------------|
| `-std=c17` | C17 compliance |
| `-pedantic` | Reject non-standard extensions |
| `-O3` | Maximum optimization |
| `-march=native` | Optimize for the local CPU |
| `-flto` | Link-time optimization |
| `-lm` | Link math library (required on Linux) |

---

## 4. macOS (Clang)

**Requirements:** Xcode Command Line Tools (`xcode-select --install`)

**Basic build:**

```bash
clang -ansi -O2 -o basicpp *.c -lm
```

**Optimized release build:**

```bash
clang -std=c17 -pedantic -Wall -Wextra -Werror \
      -O3 -flto -DNDEBUG \
      -o basicpp *.c -lm
```

---

## 5. BSD / Other Unix

```bash
cc -ansi -O2 -o basicpp *.c -lm
```

Any POSIX system with a C17 compiler should work.

---

## 6. Cross-Compiling

For embedded targets or older platforms:

```bash
# ARM cross-compile
arm-none-eabi-gcc -std=c17 -Os -o basicpp *.c -lm

# MIPS (OpenWrt router)
mipsel-openwrt-linux-gcc -std=c17 -Os -o basicpp *.c -lm
```

BASIC++ has no OS-specific code in the core. Platform-specific code is isolated in `platform.c` and conditionally compiled.

---

## 7. Using the Makefile

The included Makefile supports:

| Target | Description |
|--------|-------------|
| `make` | Build with default settings |
| `make release` | Optimized release build |
| `make clean` | Remove build artifacts |
| `make test` | Build and run SELFTEST |

Edit the top of the Makefile to change `CC`, `CFLAGS`, etc.

---

## 8. Verification

After building, verify your binary:

```
> basicpp
BASIC++ 0.21.0
@COPYLEFT ALL WRONGS RESERVED
Ready.
> SELFTEST
25 assertions, ALL PASSED
ALL TESTS PASSED
```

If `SELFTEST` passes, your build is correct.

---

## 9. Build-Time Configuration

Edit `config.h` to adjust limits before compiling:

| Constant | Default | Description |
|----------|---------|-------------|
| `MAX_PROGRAM_LINES` | 1000 | Maximum program lines |
| `MAX_LINE_LENGTH` | 512 | Maximum characters per line |
| `MAX_VARIABLES` | 26 | Numeric variables (A–Z) |
| `MAX_STRING_VARS` | 26 | String variables (A$–Z$) |
| `MAX_STACK_DEPTH` | 64 | GOSUB/FOR nesting depth |
| `MAX_DIM_ARRAYS` | 256 | Maximum DIM arrays |
| `MAX_FILE_CHANNELS` | 8 | Simultaneous open files |
| `MAX_DATA_ITEMS` | 256 | DATA statement items |
| `MAX_MEM_SEGMENT` | 65536 | Virtual memory size (64K) |
| `MAX_RECORD_LEN` | 256 | Random-access record size |
