<!--
Title:        Developer Guide
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, detok, trans)
Authority:    engine/
Generated:    no
Status:       Active
-->

# BASIC++ v6.5.2 Developer and Contributor Guide

## 1. Architectural Overview & Invariants

BASIC++ is a structured, multi-dialect BASIC compiler, interpreter, and virtual machine written in portable ISO C17. It operates as a modular, decoupled engine where core language execution is isolated from host operating system dependencies.

### Fundamental Architectural Invariants
1. **Strict Freestanding C17 Core**: Engine core micro-libraries (`libkernel`, `libengine`, `basicpp_sys.h`) strictly adhere to ISO C17 (§4 ¶6). Direct inclusion of hosted headers (`<stdio.h>`, `<stdlib.h>`, `<windows.h>`, `<unistd.h>`) is prohibited above `libplatform`.
2. **Non-Recursive VM Execution**: The expression evaluator and VM execution loop maintain all call frames, loop contexts, and evaluation stacks on heap-managed memory pools, preventing host C stack exhaustion.
3. **Platform Abstraction Boundary**: All filesystem, console, timing, socket, and process interactions pass through `libplatform` and `hal/`.
4. **Discriminated Value Safety**: The runtime value system (`BValue`) requires strict discriminator validation before accessing any union payload (`.as.number`, `.as.string`, `.as.integer`).
5. **Deterministic String Ownership**: String handles (`BppStringRef`) are reference-counted. Handlers that acquire or produce strings must balance every retain with a release on both success and error paths.
6. **No-Op Statement Advancement Guard**: Handlers must never return success (`err.code == 0`) without advancing the lexer token stream via `lex_next()`.

## 2. Repository and Engine Layout

```text
engine/
  CMakeLists.txt          — Central build configuration and micro-library definitions
  include/                — Public header files organized by subsystem
    types/                — Core definitions: config.h, errors.h, types.h, version.h
    lexer/                — Lexer tokens, scanner API: lexer.h
    vm/                   — VM context and control structures: vm.h, host.h
    memory/               — Memory pools and arenas: memory.h
    runtime/              — Strings, variables, arrays, descriptors, metadata
    device/               — Virtual devices, VDev bus, VCon console, BGI graphics
    security/             — Security rings and capabilities: security.h
    compiler/             — Native compiler and transpiler IR interfaces
    eval/                 — AST nodes, Pratt expression parser, evaluator
  src/                    — Subsystem implementation sources
    bootstrap/            — Entry controllers (desktop, headless, server, iot)
    bios/                 — IBM PC/XT/AT BIOS virtualization and video memory
    core/                 — Unified dialect definitions and struct registries
    debug/                — DAP debug server, execution tracer, logger
    device/               — VDev virtual device bus, VCon, BGI graphics engine
    editor/               — TUI multiplexer and terminal editor
    eval/                 — Pratt expression parser, AST evaluator, built-in math/string functions
    interop/              — Native C/Python/Pascal FFI, COM, and JSON-RPC
    lexer/                — Fast scanner and keyword identifier tables
    memory/               — Segmented memory (vmem) and fixed pools
    module/               — Dynamic library loader and external modules
    runtime/              — Strings, variables, arrays, VFS filesystem, VNet networking
    statements/           — 20 decomposed statement categories (loops, core, io, etc.)
    vm/                   — VM execution loop, stack frames, event trapping
  lib/                    — Platform abstraction layer (plat_console, plat_fs, etc.)
docs/                     — Markdown documentation tree
help/                     — Plaintext terminal mirror tree (CRLF, 78 cols, 7-bit ASCII)
tests/                    — Automated regression suites across all 44 dialects
tools/                    — Clean utilities, test runners, doc verification gate
```

## 3. The 12-Library Modular Architecture

BASIC++ is compiled as a linear hierarchy of static micro-libraries:

1. **libboot**: Startup and shutdown sequencing, profile bootstrap.
2. **libplatform**: OS abstraction layer (Win32, POSIX, DOS, Bare-Metal).
3. **libkernel**: Lexer, memory arenas, security rings, BIOS virtualization, VDev bus.
4. **libengine**: AST evaluator, Pratt parser, expression dispatch, runtime functions, VM loop.
5. **libhardware**: Segmented memory (`vmem`), BGI rasterizer, FujiNet adapter.
6. **libserver**: Sockets (`vnet`), Gemini/Gopher protocols, background task scheduler, VFS.
7. **libscript**: Non-interactive file execution pipeline.
8. **libcore**: REPL foundations, numeric formatting, metadata reflection.
9. **libflex**: Extended set algebra (SET, GROUP, MAP, TUPLE), metaprogramming (ALIAS, OVERRIDE, SCOPE).
10. **libstandard**: TUI editor multiplexer, DAP debug server.
11. **libadvanced**: SDL2 multimedia and graphics hardware acceleration.
12. **libext**: Dynamic third-party extension boundary.

## 4. Build Targets & Toolchain Commands

### Windows (MSVC 2022 / Clang-cl)
```bash
mkdir build_win && cd build_win
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --target baspp --config Release
cmake --build . --target bpp --config Release
cmake --build . --target bs --config Release
cmake --build . --target iot --config Release
```

### Linux (GCC 11+ / Clang 14+)
```bash
mkdir build_linux && cd build_linux
cmake .. -DCMAKE_C_STANDARD=17
cmake --build . --target baspp bpp bs iot
```

### Target Binary Profiles
- `baspp`: Flagship Desktop Edition (links `libadvanced` + SDL2, 640 MB memory profile).
- `bpp`: Lite Headless REPL (links through `libcore`, 384 MB memory profile).
- `bs`: Batch Script Runner (non-interactive, zero banner/prompt, 64 MB profile).
- `iot`: IoT Micro-REPL (embedded microcontrollers, 2 MB memory profile).
- `bppc`: Compiler & Transpiler (emits clean C17 or bytecode).
- `trans`: Source-to-source dialect translator.
- `detok`: GW-BASIC / BASICA binary decoder.

## 5. Adding a New Statement or Function

To introduce a new keyword, follow this sequential procedure:

1. **Register Enumeration**: Add `KW_<NAME>` to `BppKeywordId` in `engine/include/lexer/lexer.h`.
2. **Register Keyword String**: Add the keyword text mapping in `engine/src/lexer/lexer.c`.
3. **Implement Handler**: Create a dedicated source file in the appropriate subcategory under `engine/src/statements/` or `engine/src/eval/functions/`.
4. **Register Dispatch**: Register the handler function in `engine/src/vm/exec/exec_dispatch.c` (for statements) or `engine/src/eval/dispatch/` (for functions).
5. **Update Build Configuration**: Add the new `.c` file to the corresponding library target in `engine/CMakeLists.txt`.
6. **Author Documentation**: Create the Tier 3 reference page in `docs/keywords/` and its mirror in `help/keywords/`.
7. **Add Unit Test**: Add a test script under `tests/` and verify using `bs tests/your_test.bas`.

### Statement Handler Implementation Pattern
```c
BppError stmt_example_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    StringContext *str = vm_get_str(vm);
    
    // 1. Consume statement keyword
    BppToken tok = lex_next(lex);
    
    // 2. Validate arguments
    if (tok.type != TOK_NUMBER) {
        err.code = ERR_SYNTAX;
        return err;
    }
    
    // 3. Perform execution
    // ...
    
    return err; // err.code == 0 indicates success
}
```

### Function Evaluator Implementation Pattern
```c
BValue fn_example_eval(VMContext *vm, BValue *args, int argc, BppError *err) {
    if (argc < 1) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        return bvalue_make_nil();
    }
    if (args[0].type != VAL_NUMBER) {
        err->code = ERR_TYPE_MISMATCH;
        return bvalue_make_nil();
    }
    
    double res = args[0].as.number * 2.0;
    return bvalue_make_number(res);
}
```

## 6. Memory and String Safety Rules

- **Bounded Token Access**: `BppToken.start` is NOT null-terminated. Always use length-bounded comparisons (`tok.length == 4 && strncasecmp(tok.start, "NAME", 4) == 0`). Never call `strcmp` directly on `tok.start`.
- **String Ownership Balance**: Every string returned by `str_create()`, `str_concat()`, or `eval_expression()` that produces `VAL_STRING` must be released via `str_release(vm_get_str(vm), str_ref)` on all execution branches, including early error exits.
- **Buffer Pre-initialization**: All runtime structures must be zero-initialized (`memset` or `calloc`) prior to use.

## 7. Testing and Verification Protocols

- **Self-Test Diagnostics**: Run `baspp -c "SELFTEST"` to verify memory, lexer, string, variable, and array subsystems.
- **Headless Execution**: Use `bs` or pass `--batch` when invoking `baspp`/`bpp` in automated scripts to prevent terminal blocking.
- **Execution Watchdog**: Always pair long-running tests with `--timeout=<ms>` (e.g. `--timeout=15000`).
- **Documentation Parity Gate**: Run `powershell -ExecutionPolicy Bypass -File tools/verify_docs.ps1` before committing changes to ensure 100% mirror parity.
