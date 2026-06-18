# Compiling BASIC++ Programs

**Version 4.0.1**


---

## Table of Contents

- The COMPILE Command
- What the Compiler Generates
- Compiling for Windows 11
- Compiling for Linux
- Compiling for FreeDOS
- Compiling for Other Platforms
- Supported BASIC Features in Compiled Output
  - Fully Supported
  - Partial Support
  - Not Yet Supported in Compiled Output
- Optimization Tips
- Workflow Summary
- Example: Full Workflow
- Advanced: Embedding C in BASIC Programs
- Troubleshooting

---

BASIC++ includes a built-in transpiler that converts your BASIC program into a self-contained ANSI C89 source file. This C file can then be compiled with any C compiler on any platform — giving you native executables for Linux, Windows, FreeDOS, or any system with a C compiler.

The key insight: you **write** your program on one platform (e.g., Windows 11) and **compile** the output `.c` file on **any** other platform (e.g., Linux, FreeDOS) to get a native binary.

---

## 1. The COMPILE Command

In the BASIC++ interpreter, use:

```basic
COMPILE "output.c"
```

This transpiles the currently loaded program into a complete, standalone C89 source file.

**Full workflow:**

```
> LOAD "myprog.bas"
> COMPILE "myprog.c"
Compiling 50 lines to 'myprog.c'...
Compilation successful: myprog.c
```

The generated `.c` file:
- Is **100% self-contained** (no headers beyond stdio/stdlib/math)
- Is strict **ANSI C89/C90** (compiles everywhere)
- Includes an embedded runtime shim with all helpers
- Uses goto-based flow control (matching BASIC semantics)
- Runs standalone — does **not** require the interpreter

---

## 2. What the Compiler Generates

The `COMPILE` command performs a 3-pass pipeline:

| Pass | Action |
|------|--------|
| Pass 1 | **Tokenize** — Reuses the BASIC++ lexer on each line |
| Pass 2 | **AST Build** — Builds an abstract syntax tree per line |
| Pass 3 | **Codegen** — Walks the AST and emits C code |

The generated C file contains:
- Standard includes (stdio, stdlib, string, math)
- Runtime variables (A–Z numeric, A$–Z$ strings)
- GOSUB stack
- DIM array allocator
- DATA/READ pool
- String helpers (concat, left, right, mid, chr, str)
- `main()` with labeled blocks (`L10:`, `L20:`, etc.)
- GOSUB/RETURN dispatch table

**Example:**

BASIC:
```basic
10 A = 5
20 B = 10
30 PRINT A + B
40 END
```

Generated C:
```c
int main(void) {
L10:
    bpp_vars[0] = 5;
L20:
    bpp_vars[1] = 10;
L30:
    { double _v = (bpp_vars[0] + bpp_vars[1]);
      if (_v == (double)(long)_v) printf("%*ld", 6, (long)_v);
      else printf("%g", _v); }
    printf("\n");
L40:
    goto bpp_end;
bpp_end:
    return 0;
}
```

---

## 3. Compiling for Windows 11

**Step 1:** Write your program in the BASIC++ interpreter.

```
> NEW
> 10 PRINT "Hello from compiled BASIC++!"
> 20 INPUT "Your name: "; N$
> 30 PRINT "Hello, "; N$
> 40 END
> COMPILE "hello.c"
```

**Step 2:** Compile the generated C file.

```batch
REM Using MSVC (Developer Command Prompt):
cl /O2 hello.c

REM Using MinGW (GCC for Windows):
gcc -O2 -o hello.exe hello.c -lm

REM Using Clang:
clang -O2 -o hello.exe hello.c -lm
```

**Step 3:** Run the native executable.

```
> hello.exe
Hello from compiled BASIC++!
Your name: Alice
Hello, Alice
```

The `.exe` runs **without** the interpreter. It is a standalone native Windows binary.

---

## 4. Compiling for Linux

You can write your program on Windows and compile for Linux.

**Step 1:** Write and transpile on Windows.

```
> LOAD "myprog.bas"
> COMPILE "myprog.c"
```

**Step 2:** Transfer `myprog.c` to the Linux machine (scp, USB drive, git, etc.)

**Step 3:** Compile on Linux.

```bash
gcc -O2 -o myprog myprog.c -lm

# Or with maximum optimization:
gcc -std=c17 -O3 -march=native -o myprog myprog.c -lm

# Or with Clang:
clang -O2 -o myprog myprog.c -lm
```

**Cross-compiling from Windows to Linux:**

```bash
# If you have a Linux cross-compiler installed on Windows:
x86_64-linux-gnu-gcc -O2 -o myprog myprog.c -lm

# With WSL (Windows Subsystem for Linux):
wsl gcc -O2 -o myprog myprog.c -lm
wsl ./myprog
```

---

## 5. Compiling for FreeDOS

FreeDOS runs on real-mode x86 or emulators (DOSBox, 86Box). The generated C compiles with any DOS C compiler.

**Step 1:** Transpile on Windows (or any machine).

```
> LOAD "game.bas"
> COMPILE "game.c"
```

**Step 2:** Transfer `game.c` to a FreeDOS-accessible drive.

**Step 3:** Compile with a DOS C compiler:

```
REM Using DJGPP (32-bit DOS):
C:\> gcc -O2 -o game.exe game.c -lm

REM Using Open Watcom:
C:\> wcl -ox game.c

REM Using Turbo C 2.0:
C:\> tcc -O game.c

REM Using Pacific C:
C:\> pacc game.c
```

**Step 4:** Run on FreeDOS: `C:\> game.exe`

> **Notes for FreeDOS:**
> - The generated C is strict C89 — Turbo C, Watcom, and DJGPP all handle it perfectly
> - For 16-bit real-mode (Turbo C, Pacific C), double arrays may be limited by 64K segments. Keep programs small or use DJGPP for the 32-bit flat memory model.
> - DJGPP is the recommended compiler for FreeDOS — it gives you full 32-bit protected mode with no segment limits.
> - No operating system calls are used — the generated code only needs stdio, stdlib, string, and math.

---

## 6. Compiling for Other Platforms

| Platform | Command |
|----------|---------|
| macOS | `clang -O2 -o myprog myprog.c -lm` |
| FreeBSD / OpenBSD | `cc -O2 -o myprog myprog.c -lm` |
| Raspberry Pi (ARM Linux) | `gcc -O2 -o myprog myprog.c -lm` |
| RISC-V | `riscv64-linux-gnu-gcc -O2 -o myprog myprog.c -lm` |
| WebAssembly (Emscripten) | `emcc -O2 -o myprog.html myprog.c -lm` |

For embedded (bare metal): The generated code needs stdio (`printf`, `fgets`) and math (`fabs`, `floor`, `sqrt`, etc.). Provide a minimal libc or substitute these functions for your platform.

**The point is: if it has a C compiler, it can run your program.**

---

## 7. Supported BASIC Features in Compiled Output

### Fully Supported

- `PRINT` (with expressions, separators, format)
- `LET` / variable assignment (A–Z, A$–Z$)
- `IF` / `THEN` (with relational operators)
- `GOTO`
- `GOSUB` / `RETURN` (with dispatch table)
- `FOR` / `NEXT`
- `INPUT` (numeric and string)
- `DIM` arrays (1D and 2D)
- `DATA` / `READ` / `RESTORE`
- `END` / `STOP`
- `REM` / comments
- Arithmetic: `+ - * / MOD`
- Relational: `= <> < > <= >=`
- Math: `ABS`, `SIN`, `COS`, `TAN`, `ATN`, `SQR`, `LOG`, `EXP`, `SGN`, `INT`
- String: `LEN`, `ASC`, `CHR$`, `STR$`, `VAL`, `LEFT$`, `RIGHT$`, `MID$`
- String concatenation
- `@()` legacy array

### Partial Support

- `WHILE` / `WEND` (loop structure preserved as comments)
- `DO` / `LOOP` (loop structure preserved as comments)
- `ON GOTO` / `ON GOSUB` (computed jumps)

### Not Yet Supported in Compiled Output

- File I/O (`OPEN`, `CLOSE`, `PRINT#`, `INPUT#`)
- `SHELL` / `SHELL$` / `EXEC`
- `DEF FN` user-defined functions
- `FUNCTION` / `SUB` blocks
- Error handling (`ON ERROR GOTO`)
- Graphics (`SCREEN`, `LINE`, `CIRCLE`, etc.)
- Sound (`SOUND`, `PLAY`, `BEEP`)
- `PEEK` / `POKE` / `INP` / `OUT`
- `DIALECT` / `MEMMAP` / `ALIAS`

Programs using unsupported features will still compile to C, but those statements will appear as comments in the output.

---

## 8. Optimization Tips

**A. Let the C compiler optimize:**

```bash
gcc -O3 -march=native myprog.c -lm     # GCC, fastest
cl /O2 /Ot /GL myprog.c /link /LTCG     # MSVC, fastest
clang -O3 myprog.c -lm                  # Clang, fastest
```

**B. Profile-guided optimization (PGO):**

```bash
gcc -fprofile-generate -O2 myprog.c -lm -o myprog
./myprog                  # Run with typical input
gcc -fprofile-use -O3 myprog.c -lm -o myprog
```

**C. Link-time optimization:**

```bash
gcc -O2 -flto myprog.c -lm
```

---

## 9. Workflow Summary

```
┌──────────────────────────────────────────────┐
│ Windows 11 (Development Machine)             │
│                                              │
│  1. Write program in BASIC++ interpreter     │
│     > 10 PRINT "Hello"                       │
│     > 20 END                                 │
│                                              │
│  2. Transpile to C                           │
│     > COMPILE "myprog.c"                     │
│                                              │
│  3. Transfer myprog.c to target platform     │
└────────┬──────────┬──────────┬───────────────┘
         │          │          │
   ┌─────▼────┐ ┌───▼───┐ ┌───▼──────┐
   │ Linux    │ │ Win11 │ │ FreeDOS  │
   │          │ │       │ │          │
   │ gcc -O2  │ │ cl /O2│ │ gcc -O2  │
   │ myprog.c │ │ my... │ │ myprog.c │
   │ -lm      │ │       │ │ -lm      │
   │          │ │       │ │          │
   │ ./myprog │ │ my.exe│ │ my.exe   │
   └──────────┘ └───────┘ └──────────┘
```

The `.c` file is the portable artifact. Compile it on each target to get a native binary.

---

## 10. Example: Full Workflow

Write a number-guessing game:

```
> NEW
> 10 PRINT "=== NUMBER GUESSING GAME ==="
> 20 A = INT(RND(100)) + 1
> 30 PRINT "I'm thinking of a number 1-100."
> 40 INPUT "Your guess: "; G
> 50 IF G = A THEN PRINT "Correct!" : GOTO 80
> 60 IF G < A THEN PRINT "Too low!" : GOTO 40
> 70 PRINT "Too high!" : GOTO 40
> 80 PRINT "You got it!"
> 90 END
> COMPILE "guess.c"
Compiling 9 lines to 'guess.c'...
Compilation successful: guess.c
```

Now compile for each platform:

```bash
# Windows
cl /O2 guess.c

# Linux
gcc -O2 -o guess guess.c -lm

# FreeDOS
gcc -O2 -o guess.exe guess.c -lm
```

**Result:** 3 native binaries from 1 BASIC program, each running at full native speed with no interpreter overhead.

---

## 11. Advanced: Embedding C in BASIC Programs

Since the output is C source, you can post-process it:

1. `COMPILE "myprog.c"`
2. Edit `myprog.c` in a text editor
3. Add custom C functions (file I/O, networking, etc.)
4. Compile the modified C file

This lets you prototype in BASIC, then add C extensions for performance-critical or platform-specific features.

---

## 12. Troubleshooting

| Error | Solution |
|-------|----------|
| "No program to compile" | Load a program first: `LOAD "myprog.bas"` |
| "Compile error at line N" | Check syntax — the AST builder couldn't parse line N |
| "math.h: No such file" | Add `-lm` to your compile command (Linux/macOS) |
| "undefined reference to sqrt/sin/cos" | Add `-lm` after the source file |
| Generated `.c` won't compile with Turbo C | Arrays may exceed 64K segment limit — reduce DIM sizes or switch to DJGPP |
