<!--
Title:        Freestanding Standard TUI Architecture
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/
Generated:    no
Status:       Active
-->

# Freestanding Standard TUI Architecture

## 1. Architectural Overview

The BASIC++ standard TUI workstation subsystem (`libstandard`) encompasses the full-screen visual text editors, terminal multiplexing micro-libraries, Debug Adapter Protocol (DAP) server, and static analyzer under strict ISO C17 freestanding compliance (§4 ¶6).

All four integrated text editors—MS-DOS Edit clone (`edit/`), EDLIN line editor (`edlin/`), Vi clone (`vi/`), and WordStar clone (`ws/`)—together with the core text buffer micro-libraries (`editor_buffer`, `editor_render`, `editor_selection`, `editor_term`), operate without hosted standard C library (`libc`) dependencies. All file handling, terminal escape sequencing, memory allocation, and string formatting route through the Hardware Abstraction Layer (`IoHandle`, `hal->io.*`, `hal->mem.*`) and freestanding runtime routines (`runtime_*`).

---

## 2. Subsystem Architecture & Source Inventory

### 2.1 Editor Core Micro-Libraries
- **Text Line Buffer ([`engine/src/editor/editor_buffer.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/editor/editor_buffer.c))**: Text buffer line allocation, capacity expansion, and ASCII sanitization using `hal->mem.alloc`, `hal->mem.realloc`, `hal->mem.free`, `runtime_memcpy`, and `runtime_memmove`.
- **Screen Rendering & Coordinates ([`engine/src/editor/editor_render.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/editor/editor_render.c))**: Tab stop translations, screen coordinate mapping, and truncated filename formatting.
- **Visual Selection ([`engine/src/editor/editor_selection.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/editor/editor_selection.c))**: Selection bounds tracking, rectangular text block extraction, and multi-line in-place deletion.
- **Terminal Control ([`engine/src/editor/editor_term.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/editor/editor_term.c))**: Terminal dimensions query and ANSI escape sequence processing.

### 2.2 TUI Editors Subsystem (`engine/src/editor/`)
- **Editor Manager & Dispatch ([`engine/src/editor/editor.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/editor/editor.c), `editor_manager.c`)**: Built-in `EDIT` statement dispatch, editor plugin registration, and command-line parameter parsing.
- **MS-DOS Edit Clone (`engine/src/editor/edit/`)**:
  - `edit_buf.c`: File loading and saving via `hal_file_read_line` and `hal->io.file_write`.
  - `edit_cmd.c`: Interactive editor keystroke and command dispatch.
  - `edit_menu.c`: Pull-down menu actions and dialog formatting.
  - `edit_render.c`: Syntax highlighting and status bar formatting.
  - `edit_term.c`: Terminal stream flushing via `IO_STDOUT_HANDLE`.
- **EDLIN Line Editor (`engine/src/editor/edlin/`)**:
  - `edlin_buf.c`: Line buffer management and file streaming.
  - `edlin_cmd.c`: Command parser (`L`, `I`, `D`, `R`, `W`) and terminal output.
  - `edlin_exec.c`: Direct program file export using `runtime_atoll` and `hal->io.file_write`.
- **Vi Clone (`engine/src/editor/vi/`)**:
  - `vi_buf.c`: Ex-mode file loading and saving.
  - `vi_cmd.c`: Modal command dispatch (Normal, Insert, Visual, Command modes).
  - `vi_render.c`: Screen rendering and status line presentation.
  - `vi_term.c`: Raw terminal output flushing.
- **WordStar Clone (`engine/src/editor/ws/`)**:
  - `ws_buf.c`: Document file loading and saving.
  - `ws_cmd.c`: Classic WordStar control-key sequences (`^K^D`, `^Q^S`, etc.).
  - `ws_render.c`: Ruler line and menu layout formatting.
  - `ws_term.c`: Screen presentation flushing.

### 2.3 Debug & Static Analysis Subsystem (`engine/src/debug/`)
- **DAP Debug Server ([`engine/src/debug/dap_server.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/debug/dap_server.c))**: Debug Adapter Protocol JSON-RPC responses formatted via `runtime_snprintf`.
- **Static Code Analyzer ([`engine/src/debug/analyzer.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/engine/src/debug/analyzer.c))**: Code metrics, line counts, and cyclomatic complexity calculations.

---

## 3. Verification & Test Suite

1. **Unit Test Suite ([`tests/standard_tui_freestanding_test.c`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/tests/standard_tui_freestanding_test.c))**:
   - Editor core buffer allocation, capacity growth, and ASCII sanitization.
   - Tab stop coordinate translations, row rendering, and path truncation.
   - Forward and reverse visual selections, block deletion, and clipboard transfers.
   - EDLIN line insertions, modifications, disk persistence, and reloads.
   - Static analysis report generation and complexity metrics calculation.
2. **Freestanding Test Execution**: All eight freestanding test suites pass with 100% verification.
3. **Master Regression Coverage**:
   - `tests/qb_vbdos_master.bas`: 10/10 Packages PASSED (100%).
   - `tests/vintage_ecosystems_master.bas`: 14/14 Packages PASSED (100%).
   - `tests/vintage_deep_fuzz_stress.bas`: 8/8 Tests PASSED (100%).
