# BASIC++ v6.5.2 Developer and Contributor Guide

## 1. PROJECT ORGANIZATION

The BASIC++ project is structured around the engine/ directory, which contains all v6.5.2 source code. The repository layout is:

```text
engine/
  CMakeLists.txt          — Central build configuration
  include/                — Public header files (organized by subsystem)
    types/                — Core types: config.h, errors.h, types.h, version.h
    lexer/                — Lexer interface: lexer.h
    vm/                   — VM interface: vm.h, host.h
    memory/               — Memory manager: memory.h
    runtime/              — Runtime subsystems: strings.h, variables.h, arrays.h, etc.
    device/               — Virtual device interfaces
    security/             — Security system: security.h
    compiler/             — Compiler interfaces
    editor/               — Editor interfaces
    scope/                — Scope stack interface
    eval/                 — Expression evaluator interfaces
    module/               — Module system interfaces
  src/                    — Implementation source files
    bootstrap/            — Boot sequence controllers
    bios/                 — BIOS virtualization
    core/                 — API entry point, dialect, feature registry, structs
    debug/                — Logger, DAP debug server
    device/               — VDev, VCon, bus, MUX, BGI subsystem
    docgen/               — Documentation generator
    editor/               — TUI editor, edlin, vi, ws, multiplexer
    eval/                 — AST evaluator, dispatch, builtins, RPN, ops
    interop/              — Host interoperability
    lexer/                — Lexer implementation
    memory/               — Memory system, segmented memory
    module/               — Module loader, arrayext, mathext, regex
    parser/               — Parser implementation
    runtime/              — Variables, strings, arrays, files, VFS, VNet, tasks
    scope/                — Scope implementation
    security/             — Security implementation
    statements/           — Statement handlers (organized by category)
    tools/                — Internal tools
    vm/                   — VM context, control, data, error, events, exec, math, stack
  lib/                    — Platform abstraction layer
    platform/             — plat_console, plat_fs, plat_sys, plat_time, etc.
    editor/               — Editor buffer, render, selection, terminal
```

The legacy source/ and v5/ directories contain the v5.0.5 codebase and are not used for v6.5.2 development.

## 2. BUILD SYSTEM

The build uses CMake 3.16 or later with C17 compliance required. The engine/ directory has its own CMakeLists.txt that defines all micro-library targets and the modular static library architecture.

### Building on Windows

```bash
mkdir build_win && cd build_win
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --target baspp --config Release
cmake --build . --target bpp --config Release
cmake --build . --target bs --config Release
```

### Building on Linux

```bash
mkdir build_linux && cd build_linux
cmake .. -DCMAKE_C_STANDARD=17
make baspp bpp bs
```

### Build Targets

baspp — Standard desktop edition (links libadvanced + all upstream libraries).
bpp — Lite headless REPL (links libcore + all upstream through libscript, excludes libstandard/libadvanced).
bs — Batch script runner (links libscript + upstream, excludes REPL iterations).
bppc — Compiler and transpiler.
detok — Legacy GW-BASIC detokenizer.

## 3. THE 12-LIBRARY MODULAR ARCHITECTURE

BASIC++ is built as a chain of static micro-libraries. Each library in the chain depends on all libraries below it. New features are placed in the lowest appropriate library layer:

1. **libboot** — Boot sequence controller (common.c).
2. **libplatform** — OS abstraction (plat_console, plat_fs, plat_sys, plat_time, plat_thread, plat_dl, plat_net, plat_regex, plat_clipboard). All Win32 vs POSIX #ifdef logic lives here.
3. **libkernel** — Lexer, memory manager, security, BIOS, virtual device bus (vdev, vcon), VM context and control.
4. **libengine** — AST evaluator, parser, expression dispatch, runtime functions, variables, strings, bytecode execution loop.
5. **libhardware** — Segmented memory (vmem), BGI rasterizer, FujiNet emulation.
6. **libserver** — Network sockets (vnet), Gemini protocol, background tasks, VFS, crypto, regex.
7. **libscript** — File I/O operations (file.c). The bs batch runner links at this level.
8. **libcore** — REPL foundations, HELP/CATALOG/SELFTEST introspection. The bpp lite edition links at this level.
9. **libflex** — Metaprogramming (ALIAS, OVERRIDE, SCOPE, KEYWORD, REMOVE).
10. **libstandard** — TUI editor multiplexer, DAP debug server.
11. **libadvanced** — SDL2/OpenGL graphics, multimedia. The baspp standard edition links at this level.
12. **libext** — Open-ended extension template.

## 4. ADDING A NEW KEYWORD

To add a new statement, function, or keyword to BASIC++:

1. Add the keyword identifier to the BppKeywordId enum in engine/include/lexer/lexer.h.
2. Register the keyword string mapping in engine/src/lexer/lexer.c in the keyword table.
3. Create a dedicated source file named after the keyword under the appropriate statement category directory (e.g., engine/src/statements/core/newstmt.c for a core statement, engine/src/eval/functions/math/newfunc.c for a math function).
4. Implement the statement handler or function evaluator. Statement handlers receive the VMContext and LexerContext and return a BppError. Function evaluators receive arguments as BValue values and return a BValue result.
5. Register the handler in the statement registry or function dispatch table.
6. Add the new source file to the appropriate micro-library in engine/CMakeLists.txt.
7. Add a help entry in the HELP system.
8. Add a test case under tests/.
9. Build and verify on both baspp and bpp targets.

### Statement Handler Pattern

```c
BppError stmt_example(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    StringContext *str = vm_get_str(vm);
    VDevContext *vdev = vm_get_vdev(vm);
    
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_NUMBER) {
        err.code = ERR_SYNTAX;
        err.message = "Expected numeric argument";
        return err;
    }
    
    double value = tok.as.number;
    if (value < 0 || value > 255) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        err.message = "Argument out of range (0-255)";
        return err;
    }
    
    // ... perform the statement's action ...
    return err;  // err.code == 0 means success
}
```

### Function Evaluator Pattern

```c
BValue func_example(VMContext *vm, BValue *args, int argc) {
    if (argc < 1) {
        vm_set_error(vm, ERR_SYNTAX, "Missing argument");
        BValue err = {0}; err.type = VAL_NONE; return err;
    }
    
    if (args[0].type != VAL_NUMBER) {
        vm_set_error(vm, ERR_TYPE_MISMATCH, "Expected numeric argument");
        BValue err = {0}; err.type = VAL_NONE; return err;
    }
    
    double result = /* compute */;
    BValue val; val.type = VAL_NUMBER; val.as.number = result;
    return val;
}
```

## 5. STRING MEMORY SAFETY

The StringContext manages all string values through reference-counted handles (BppStringRef). Every string created by str_create(), str_concat(), str_mid(), or any expression that produces a VAL_STRING must be properly reference-managed:

- str_add_ref(ref) increments the reference count when a second owner acquires the string.
- str_release(ctx, ref) decrements the reference count and frees the string when the count reaches zero.

Statement handlers that call expression evaluation and receive a VAL_STRING result own that reference. They must call str_release() on both success and error paths. Failing to release on error paths is the most common source of memory leaks.

## 6. TOKEN HANDLING SAFETY

Tokens are ephemeral. The BppToken struct's .start pointer points into the original source line and is NOT null-terminated. When comparing a TOK_IDENT token against a known string, always use a length-bounded comparison:

```c
if (tok.type == TOK_IDENT && tok.length == 4 &&
    strncasecmp(tok.start, "BASE", 4) == 0) {
    // matched "BASE"
}
```

Never use strcmp() or strcasecmp() directly on tok.start. Never access tok.as.string on a TOK_IDENT token or tok.as.number on a TOK_STRING token — these are different union members and accessing the wrong one is undefined behavior.

## 7. ERROR PROPAGATION

Statement handlers return a BppError struct. Setting err.code to a non-zero value signals an error. The VM reads the error code and routes it through the error trapping system (ON ERROR GOTO or TRY/CATCH). A handler must never return err.code == 0 without having consumed at least one token via lex_next(), or the VM will infinitely re-execute the same token.

For VM-level errors, use vm_set_error(vm, code, message). For structured propagation, return the BppError directly.

## 8. TESTING

The built-in SELFTEST command runs the interpreter's validation suite: `baspp -c "SELFTEST"`. All changes must pass SELFTEST on both baspp and bpp targets.

Test files live under tests/ and are organized by category. Each test file is a BASIC++ program that uses ASSERT and the TEST/ENDTEST block structure. Tests are executed using the baspp or bpp targets directly — never through external script wrappers.

## 9. CODING STANDARDS

- C17 compliance is mandatory. All code compiles under -Wall -Werror.
- No recursive C functions for parsing or execution. The VM is strictly non-recursive.
- No raw printf/putchar in statement handlers. All console output goes through VDev.
- No OS-specific code above libplatform. All #ifdef WIN32/#ifdef __linux__ logic is encapsulated in the platform layer.
- Every keyword maps to a dedicated source file named after that keyword.
- Feature gates (SUPPORT_*) control compile-time inclusion of optional subsystems.
