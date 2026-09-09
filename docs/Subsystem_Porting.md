<!--
Title:        Subsystem Porting
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans, detok)
Authority:    engine/CMakeLists.txt, engine/Engine_Features.md
Generated:    no
Status:       Active
-->

# BASIC++ Subsystem Porting & Micro-Library Extraction

The engineering guide detailing how to isolate, extract, and reuse individual BASIC++ subsystems and micro-libraries in standalone external applications.

## 1. Modular Subsystem Architecture

BASIC++ v6.5.2 organizes all engine subsystems into an 11-modular library spectrum linked accumulatively (`libboot` through `libadvanced`, plus `libinterop_core`). Subsystems are designed with explicit unidirectional dependencies, allowing clean extraction of specific subsystems without linking the entire desktop runtime.

### 1.1 Accumulative Library Chain

| Library | Subsystem Role | Core Dependencies | Primary Source Modules |
| :--- | :--- | :--- | :--- |
| `libboot` | Bootstrap Sequence Controller | None (Freestanding C17) | `engine/src/bootstrap/boot.c` |
| `libplatform` | Operating System Abstraction | Host C Runtime | `engine/src/platform/` |
| `libkernel` | VM Context, Lexer, Memory, BIOS, VDev | `libboot`, `libplatform` | `engine/src/vm/`, `lexer/`, `memory/` |
| `libengine` | AST Evaluator, Parser, Variables | `libkernel` | `engine/src/eval/`, `parser/` |
| `libhardware` | Segmented Memory (`vmem`), BGI Rasterizer | `libkernel` | `engine/src/device/bgi/` |
| `libserver` | Network Sockets, Gemini, Task System | `libkernel`, `libplatform` | `engine/src/device/vnet.c` |
| `libscript` | Batch Script Runner, File I/O | `libengine` | `engine/src/statements/filesystem/` |
| `libcore` | REPL, Numeric Formatter, Metadata | `engine/src/runtime/numfmt.c` |
| `libflex` | Metaprogramming (`ALIAS`, `OVERRIDE`) | `libengine` | `engine/src/scope/`, `module/` |
| `libstandard` | TUI Editor Multiplexer, DAP Debug Server | `libcore`, `libflex` | `engine/src/editor/` |
| `libadvanced` | SDL2 Graphics, Sound, OpenGL Bindings | `libstandard` | `engine/src/device/gfx.c` |

---

## 2. Extracting Standalone Subsystems

### 2.1 Extracting the BIOS Subsystem (`libbios`)
The PC/XT/AT BIOS emulator (`engine/src/bios/`) provides complete virtual BIOS interrupt emulation:
1. Copy `engine/src/bios/` and `engine/include/bios/`.
2. Compile under freestanding ISO C17 with standard integer types (`<stdint.h>`, `<stdbool.h>`, `<stddef.h>`).
3. Bind the `BiosHAL` dispatch struct to your host environment.
4. Provides full PC interrupt trapping (INT 10h, 13h, 16h, 1Ah) with zero external engine dependencies.

### 2.2 Extracting the BGI Rasterizer Subsystem (`libbgi`)
The Borland Graphics Interface rasterizer (`engine/src/device/bgi/`) provides software graphics rendering:
1. Copy `engine/src/device/bgi/` and `engine/include/device/bgi/`.
2. Supply a linear framebuffer pointer (`uint8_t* framebuffer`).
3. Provides line drawing, circle/arc rasterization, flood fill, polygon clipping, and vector stroke font rendering.

### 2.3 Extracting the Lexer and Tokenizer
The lexical scanner (`engine/src/lexer/lexer.c`) can be extracted into external tools and IDE formatters:
1. Copy `engine/src/lexer/` and `engine/include/lexer/`.
2. Copy `engine/include/types/` for token enumerations (`BppToken`, `BppKeywordId`).
3. Performs zero-copy lexical tokenization across all vintage and modern BASIC keywords.
