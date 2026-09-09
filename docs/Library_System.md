<!--
Title:        Library System
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/CMakeLists.txt, engine/Engine_Features.md
Generated:    no
Status:       Active
-->

# BASIC++ v6.5.2 Library System & Micro-Library Architecture

The architectural reference defining the accumulative 11-modular library spectrum, micro-library compilation targets, and linkage points across BASIC++ editions.

## 1. The 11-Modular Library Architecture

BASIC++ is compiled as a chain of 11 static micro-libraries linked accumulatively (`libboot` through `libadvanced`, plus `libinterop_core`). Subsystems are structured with unidirectional dependencies: any library in the chain depends only on layers below it.

Executable targets link at specific layers in the spectrum:
- **`iot.exe`**: Links up to `libkernel` (2 MB pool, micro-REPL).
- **`bs.exe`**: Links up to `libscript` (64 MB pool, batch runner with file I/O).
- **`bpp.exe`**: Links up to `libcore` (384 MB pool, headless terminal REPL).
- **`baspp.exe`**: Links up to `libadvanced` (640 MB pool, desktop with SDL2 graphics).

---

## 2. Library Chain Specifications

### Layer 1: `libboot` (Bootstrap Controller)
- **Source**: `engine/src/bootstrap/boot.c`, `engine/src/bootstrap/common/common.c`.
- **Role**: Coordinates the 9-phase initialization sequence (`boot_engine()`).
- **Dependencies**: None (Freestanding C17).

### Layer 2: `libplatform` (Operating System Abstraction)
- **Source**: `engine/src/platform/plat_console.c`, `plat_fs.c`, `plat_sys.c`, `plat_time.c`, `plat_thread.c`, `plat_dl.c`, `plat_net.c`, `plat_regex.c`, `plat_clipboard.c`.
- **Role**: Encapsulates all Win32, POSIX, and bare-metal OS APIs.
- **Rule**: Zero OS-specific headers (`windows.h`, `unistd.h`) may appear above this layer.

### Layer 3: `libkernel` (Virtual Machine Core)
- **Source**: `engine/src/vm/context.c`, `lexer/lexer.c`, `memory/memory.c`, `security/security.c`, `bios/bios_pc.c`, `device/vdev.c`.
- **Role**: Lexical analyzer, monotonic memory arena, security sandbox, BIOS virtualization, and virtual device bus.

### Layer 4: `libengine` (Evaluator & Parser)
- **Source**: `engine/src/eval/eval.c`, `eval/rpn.c`, `parser/parser.c`, `vm/exec.c`, `runtime/variables.c`, `runtime/strings.c`.
- **Role**: AST evaluator, RPN expression engine, procedure dispatch, dynamic variable and string tables.

### Layer 5: `libhardware` (Hardware Emulation)
- **Source**: `engine/src/device/bgi/bgi_core.c`, `bgi_raster.c`, `bgi_font.c`, `device/fujinet.c`.
- **Role**: Segmented memory (`vmem`), BGI software rasterizer, and FujiNet network bus peripheral virtualization.

### Layer 6: `libserver` (Networking & Sockets)
- **Source**: `engine/src/device/vnet.c`, `server/gemini.c`, `server/task.c`, `security/crypto.c`.
- **Role**: TCP/UDP network sockets (`VNet`), Gemini and Gopher clients, background task scheduler, cryptographic hashing.

### Layer 7: `libscript` (Filesystem & Batch Execution)
- **Source**: `engine/src/bootstrap/server/server.c`, `engine/src/statements/filesystem/`.
- **Role**: File sector handling, random-access record fielding, and non-interactive batch script processing.

### Layer 8: `libcore` (REPL & Formatter)
- **Source**: `engine/src/bootstrap/iot/iot.c`, `runtime/numfmt.c`, `runtime/metadata.c`.
- **Role**: Terminal REPL engine, `PRINT USING` formatting mask parser, metadata catalog registry.

### Layer 9: `libflex` (Dynamic Metaprogramming)
- **Source**: `engine/src/scope/scope.c`, `module/module.c`, `statements/metaprog/`.
- **Role**: Keyword remapping (`ALIAS`), keyword overriding (`OVERRIDE`), lexical scoping (`SCOPE`), and external module loader.

### Layer 10: `libstandard` (TUI Workstation & Editor)
- **Source**: `engine/src/editor/tui_multiplexer.c`, `editor/editor_buffer.c`, `debug/dap.c`.
- **Role**: Multi-window text-user-interface editor multiplexer and Debug Adapter Protocol (DAP) server.

### Layer 11: `libadvanced` (Desktop Graphics & Multimedia)
- **Source**: `engine/src/bootstrap/desktop/desktop.c`, `device/gfx.c`.
- **Role**: SDL2 and OpenGL windowing, 2D/3D hardware-accelerated graphics primitives, audio streaming, and joystick input.
