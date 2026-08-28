# BASIC++ C17 Public API Architecture Index

## 1. Executive Summary & Architecture Overview

The BASIC++ v6.5.2 Public C17 API provides a modular, freestanding C17 interface for embedding the BASIC++ interpreter, extending the language with custom statements and functions, constructing virtual devices, and integrating with native host environments.

The API adheres to strict architectural boundaries:
- **Freestanding C17 Compliance**: Core engine headers compile under ISO C17 (`-std=c17`) with zero host OS dependencies.
- **Decoupled Executable Targets**: Embeddable across Standard Desktop Console (`baspp`), Headless Lite REPL (`bpp`), Headless Batch Script Runner (`bs`), Transpiler (`bppc`), and Bare-Metal RTOS environments.
- **11-Modular Library Chain**: Organized into distinct layers (`libboot` $\rightarrow$ `libplatform` $\rightarrow$ `libkernel` $\rightarrow$ `libengine` $\rightarrow$ `libhardware` $\rightarrow$ `libserver` $\rightarrow$ `libscript` $\rightarrow$ `libcore` $\rightarrow$ `libflex` $\rightarrow$ `libstandard` $\rightarrow$ `libadvanced` $\rightarrow$ `libext`).
- **Strict Memory Safety**: Tagged union `BValue` representations, reference-counted string pools (`StringContext`), interpreter-managed heap stacks, and non-recursive AST evaluation.

## 2. API Domain Index

The BASIC++ C17 API is partitioned into 7 functional domains:

### 1. Core Interpreter & VM (`engine/include/core/`, `vm/`, `types/`)
- [`core/boot.h`](boot.md): Boot phase initialization, memory pool allocation, and teardown.
- [`core/state.h`](state.md): Global interpreter lifecycle and signal coordination.
- [`core/spec.h`](spec.md): Language dialect profiles and specification definitions.
- [`core/version.h`](version.md): Semantic versioning constants (`6.5.2`) and build targets.
- [`types/types.h`](types.md): Tagged union values (`BValue`), error types (`BppError`), token definitions (`BppToken`), and configuration constants.
- [`vm/vm.h`](vm.md): Virtual machine execution loop, program counter, heap stacks, and context accessors.

### 2. Lexical Analysis & Evaluation (`engine/include/lexer/`, `eval/`, `stmt/`)
- [`lexer/lexer.h`](lexer.md): Ephemeral token scanning, keyword lookup, and lookahead.
- [`eval/eval.h`](eval.md): Non-recursive expression evaluation, operator precedence, and AST traversal.
- [`stmt/stmt.h`](stmt.md): Statement dispatch registration, execution handlers, and control flow branching.

### 3. Runtime Type & Data Management (`engine/include/runtime/`)
- [`runtime/variables.h`](variables.md): Variable context, scope frames, scalar binding, and lookups.
- [`runtime/strings.h`](strings.md): Reference-counted string pool (`BppString`), string concatenation, slicing, and mutation.
- [`runtime/arrays.h`](arrays.md): Multi-dimensional array descriptors, dynamic bounds, and matrix mathematics.
- [`runtime/struct.h`](struct.md): User-defined `TYPE` records and binary struct packing.
- [`runtime/file.h`](file.md): File channel tables, random-access record buffers, and byte-range locking.
- [`runtime/funcreg.h`](funcreg.md): Native C function registration and metadata blocks.
- [`runtime/metadata.h`](metadata.md): MicroLib metadata blocks, syntax cards, and documentation tables.
- [`runtime/task.h`](task.md): Asynchronous worker task spawning, thread joins, and process tables.
- [`runtime/vfs.h`](vfs.md): Virtual Filesystem mount point routing and path resolution.
- [`runtime/gemini.h`](gemini.md): Gemini protocol streaming and virtual TLS socket clients.

### 4. Memory & Virtualization Subsystems (`engine/include/memory/`, `bios/`)
- [`memory/memory.h`](memory.md): Main memory pools (640MB/384MB/64MB), block allocators, and program line storage.
- [`memory/map.h`](map.md): High-performance string-to-pointer hash maps.
- [`memory/segmented_mem.h`](segmented_mem.md): 8086 real-mode 1MB segmented memory model (`PEEK`, `POKE`, `DEF SEG`).
- [`bios/bios.h`](bios.md): Freestanding PC BIOS virtualization, interrupt vectors (`INT 10h`, `INT 16h`), and CRTC registers.

### 5. Virtual Devices & Hardware Bus (`engine/include/device/`)
- [`device/vdev.h`](vdev.md): Master Virtual Device abstraction (`VDev`), device bus multiplexer, and stream callbacks.
- [`device/vcon.h`](vcon.md): Virtual Console (`VConContext`), ANSI escape parser, screen matrix, and cursor tracking.
- [`device/vnet.h`](vnet.md): Virtual Network sockets (`N:` device), TCP streams, and packet buffers.
- [`device/bgi.h`](bgi.md): BASIC++ Graphics Interface rasterizer, Bresenham primitives, palettes, and mode profiles.
- [`device/fujinet.h`](fujinet.md): FujiNet peripheral bus emulation (`N:`, `D:`, `P:`).
- [`device/usb.h`](usb.md): Virtual USB controller, device enumeration, and HID endpoints.
- [`device/bus.h`](bus.md): I/O port address space virtualization (`INP`, `OUT`).

### 6. Platform Abstraction Layer (`engine/include/platform/`)
- [`platform/platform.h`](platform.md): OS encapsulation layer coordinating console, filesystem, threads, timing, dynamic linking, and regex.

### 7. Security, Modules & Development Tools (`engine/include/security/`, `module/`, `editor/`, `debug/`)
- [`security/security.h`](security.md): Capability-based sandbox (`CAP_FS`, `CAP_NET`, `CAP_SYS`, `CAP_IO`).
- [`module/module.h`](module.md): Dynamic C extension loader (`MODULE LOAD`, `UNLOAD`).
- [`module/mod_arrayext.h`](mod_arrayext.md): High-performance matrix mathematics extension module.
- [`editor/editor.h`](editor.md): Multi-window text user interface (TUI) code editor.
- [`debug/logger.h`](logger.md): Opt-in multi-channel logging framework.

## 3. Developer Tutorials & Integration Guides

- [Tutorial 1: Creating Custom Functions](tutorials/custom_function.md)
- [Tutorial 2: Creating Custom Statements](tutorials/custom_statement.md)
- [Tutorial 3: Building Dynamic Modules](tutorials/custom_module.md)
- [Tutorial 4: Implementing Virtual Devices](tutorials/custom_vdev.md)
- [Tutorial 5: Embedding BASIC++ in Host Applications](tutorials/embedding.md)
