<!--
Title:        Source_Map
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/CMakeLists.txt, engine/include/, engine/src/
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Source Map & Repository Structure

The comprehensive architectural navigation map of the BASIC++ repository, top-level directories, engine/ subsystem trees, decomposed micro-libraries, and implementing source file mappings.

---

## 1. Architectural Overview & Library Spectrum

The BASIC++ v6.5.2 codebase is decomposed into an 11-layer modular micro-library chain (`engine/CMakeLists.txt`):

1. **`libboot`**: Bootstrap initialization, hardware probe, command-line processing.
2. **`libplatform`**: OS abstraction (POSIX, Win32, FreeDOS, bare-metal hardware).
3. **`libkernel`**: Core primitives (memory arenas, error handling, lexer, parser, vdev).
4. **`libengine`**: Virtual machine execution loop, statement dispatch, evaluator.
5. **`libhardware`**: Physical I/O virtualization, BIOS emulation, timer/interrupt dispatch.
6. **`libserver`**: TCP/UDP networking (VNet), inter-process communication, remote sessions.
7. **`libscript`**: Script execution pipeline, batch processing, stream I/O.
8. **`libcore`**: Data structures, strings, associative collections, types.
9. **`libflex`**: Advanced language runtime (UDT, arrays, matrices, pattern matching).
10. **`libstandard`**: Standard built-in function library and math intrinsics.
11. **`libadvanced`**: Graphics subsystems (BGI, SDL2), audio synthesis, TUI editor.
- **`libinterop_core`**: Multi-language interoperation and FFI bridges.

---

## 2. Top-Level Repository Directory Layout

```text
basic-plus-plus/
  engine/                 — v6.5.2 source code (active codebase)
  docs/                   — Markdown documentation files (Tier 0–4)
  help/                   — Plaintext mirrors of docs/ (100% parity, 78 cols)
  tests/                  — Regression test suites (unit, stress, dialect)
  classics/               — Preserved standalone educational modules
  openrgb/                — OpenRGB hardware data and morse test files
  tools/                  — Build, test, and quality verification utilities
  CMakeLists.txt          — Root CMake configuration
  README.md               — Project overview and quickstart
```

---

## 3. Engine Subsystem Directory Structure

All v6.5.2 source files reside strictly under `engine/`:

```text
engine/
  CMakeLists.txt          — Engine modular build targets
  Engine_Features.md      — Engine feature catalog and summary counts
  Engine_Features.TXT     — Plaintext mirror of feature catalog
  include/                — Public header files organized by subsystem:
    bios/                 — PC BIOS and interrupt emulation headers
    compiler/             — Bytecode compiler and transpiler headers
    core/                 — Engine core API and registration headers
    debug/                — Logging, tracing, and DAP debug server headers
    device/               — Virtual device and VHAL bus headers
    docgen/               — Documentation generator headers
    editor/               — Interactive TUI editor headers
    eval/                 — Pratt expression evaluator and dispatch headers
    functions/            — Built-in function headers
    interop/              — Foreign Function Interface (FFI) headers
    lexer/                — Lexical analyzer and token headers
    memory/               — Memory arenas and allocator headers
    module/               — Dynamic module loading headers
    platform/             — Platform abstraction headers
    runtime/              — Language runtime and string headers
    scope/                — Variable scoping and stack headers
    security/             — 6-level security system headers
    statements/           — Statement handler headers
    types/                — Core types, opcodes, and error definition headers
    vm/                   — Virtual machine and context headers
  src/                    — Implementation source files:
    bios/                 — BIOS emulation implementation
    bootstrap/            — System bootstrap and startup routines
    compiler/             — Compiler, bytecode emitter, C17 emitter
    core/                 — Engine entry points and feature registration
    debug/                — Debugger server and logging implementation
    device/               — Virtual device bus and driver implementations
    docgen/               — Metadata docgen implementation
    editor/               — TUI text editor implementation
    eval/                 — Expression evaluator implementation
    functions/            — Built-in intrinsic functions
    interop/              — C/Python/Pascal FFI implementation
    lexer/                — Lexer implementation and scanner tables
    memory/               — Certified allocation model implementations
    module/               — Module loader and dynamic linkers
    platform/             — Host platform bindings (Win32/POSIX/FreeDOS)
    runtime/              — String manager, collections, and algorithms
    scope/                — Scope frames and symbol tables
    security/             — Access control matrix and sandbox enforcement
    statements/           — Statement executors categorized by domain
    types/                — Type coercion and error code implementations
    vm/                   — Execution engine and VM dispatch loops
```

---

## 4. Subsystem-to-Source-File Mapping

| Subsystem Area | Headers (`engine/include/`) | Implementations (`engine/src/`) | Description |
|:---|:---|:---|:---|
| **Version & Config** | `types/version.h`, `types/config.h` | — | Target version, memory profiles, feature flags. |
| **Types & Errors** | `types/types.h`, `types/errors.h` | `types/errors.c` | BValue, ValueType, BppErrorCode enum. |
| **Lexer** | `lexer/lexer.h` | `lexer/lexer.c`, `lexer/scan_*.c` | Tokenizer, keyword scanners, token tables. |
| **Parser** | `lexer/parser.h` | `parser/parser.c` | Block structure matching, statement routing. |
| **Virtual Machine** | `vm/vm.h`, `vm/host.h` | `vm/exec/exec_dispatch.c`, `vm/control/` | VM execution loop, GOSUB/FOR stacks. |
| **Evaluator** | `eval/eval.h`, `eval/ast.h` | `eval/eval.c`, `eval/dispatch/` | Pratt expression evaluator, AST trees, dispatch. |
| **Strings & Heap** | `runtime/strings.h` | `runtime/strings.c`, `runtime/string/` | Reference-counted string pool, memory GC. |
| **Virtual Devices** | `device/vdev.h`, `device/bus.h` | `device/vdev.c`, `device/bus.c` | VHAL device bus, sub-device multiplexing. |
| **Virtual Console** | `device/vcon.h` | `device/vcon.c` | Terminal screen output, cursor, keystrokes. |
| **Graphics & BGI** | `device/bgi/bgi.h`, `device/gfx.h` | `device/bgi/*.c`, `device/gfx.c` | BGI rasterizer, modes, SDL2 screen bridge. |
| **Networking** | `runtime/vnet.h` | `runtime/vnet.c`, `platform/plat_net.c`| BSD socket wrappers, TCP/UDP channels. |
| **Security Sandbox**| `security/security.h` | `security/security.c` | 6-tier security enforcement and access matrix.|
| **Compiler & Tools**| `compiler/compiler.h` | `compiler/*.c` | AST to bytecode compiler, freestanding C17. |

---

## 5. Build Targets & Executable Roles

The build system compiles seven decoupled executable targets:

1. **`baspp.exe` / `baspp`** (Desktop Edition): Console + SDL2 graphics, 640 MB memory pool.
2. **`bpp.exe` / `bpp`** (Lite REPL Edition): Headless terminal REPL, 384 MB memory pool.
3. **`bs.exe` / `bs`** (Batch Script Runner): Non-interactive script runner, 64 MB memory pool.
4. **`iot.exe` / `iot`** (Microcontroller & IoT Edition): Headless micro-REPL, 2 MB memory pool.
5. **`bppc.exe` / `bppc`** (Compiler & Transpiler): Native compiler target emitting bytecode or C17.
6. **`trans.exe` / `trans`** (Transpiler): Source-to-source dialect conversion tool.
7. **`detok.exe` / `detok`** (Detokenizer): Legacy binary format decoder.
