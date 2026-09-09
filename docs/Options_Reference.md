<!--
Title:        Options_Reference
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, detok, trans)
Authority:    engine/src/bootstrap/desktop/desktop.c, engine/include/types/config.h
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Options Reference

The authoritative operational reference for command-line invocation flags, execution switches, memory allocation profiles, and supported language-level `OPTION` directives across all BASIC++ executable editions.

---

## 1. Executable Edition Profiles & Invocation Flags

BASIC++ provides specialized executables tailored for distinct runtime environments:

### A. Flagship Desktop Edition (`baspp` / `baspp.exe`)
- **Default Memory Pool**: 640 MB (`671088640L` bytes).
- **Prompt**: `>` prompt with `Ok` status.
- **Subsystems**: Standard console, delayed-load SDL2 graphics and audio, segmented memory (`vmem`), and full-screen TUI editors.
- **Invocation Flags**:
  - `baspp`: Start interactive console REPL.
  - `baspp script.bas`: Load program into memory and enter REPL.
  - `baspp --batch script.bas` (or `-np`, `--no-pause`): Execute script non-interactively and exit.
  - `baspp -c "statement"`: Execute a single inline BASIC statement and exit.
  - `baspp --timeout=ms`: Set hard execution time limit in milliseconds.
  - `baspp --runtime_log[=path]`: Enable structured logging to file.
  - `baspp --debug` / `--trace`: Enable execution tracing and AST telemetry.
  - `baspp --timer` (`-t`): Display elapsed execution time upon script termination.
  - `baspp --edit [file]`: Launch full-screen TUI text editor.
  - `baspp --vi [file]`: Launch modal vi-compatible editor.
  - `baspp --ws [file]`: Launch WordStar-style editor.
  - `baspp --edlin [file]`: Launch line-oriented edlin editor.
  - `baspp --export-docs [format] [target_dir]`: Export API documentation schemas.

### B. Lite Edition (`bpp` / `bpp.exe`)
- **Default Memory Pool**: 384 MB (`402653184L` bytes).
- **Prompt**: Apple II / Commodore style `]` prompt with `Ready.` status.
- **Subsystems**: Terminal REPL optimized for cloud and lightweight systems; excludes SDL2, BGI canvas drawing, SDL audio, and segmented memory.

### C. Batch Script Runner (`bs` / `bs.exe`)
- **Default Memory Pool**: 64 MB (`67108864L` bytes).
- **Behavior**: Headless non-interactive runner optimized for PowerShell, Bash, and CGI pipelines. Zero banner, zero prompt, deterministic exit codes.
- **Invocation**: `bs script.bas` or `cat data.txt | bs script.bas > out.txt`.

### D. IoT Microcontroller Edition (`iot` / `iot.exe`)
- **Default Memory Pool**: 2 MB (`2097152L` bytes).
- **Behavior**: Headless micro-REPL optimized for microcontrollers, hardware GPIO, I2C, SPI, and event loops.

---

## 2. Language-Level `OPTION` Statements

BASIC++ supports standard ECMA-116 and ANSI X3.113 option directives:

- **`OPTION BASE 0` / `OPTION BASE 1`**: Sets the default lower bound for array indexing (default is 0 unless configured).
- **`OPTION ANGLE DEGREES` / `OPTION ANGLE RADIANS`**: Sets the default trigonometric unit for `SIN`, `COS`, `TAN`, `ASN`, `ACS`, `ATN`.
- **`OPTION ARITHMETIC DECIMAL` / `OPTION ARITHMETIC NATIVE`**: Configures calculation precision between exact fixed-point decimal and native IEEE 754 floating point.
- **`OPTION COLLATE NATIVE` / `OPTION COLLATE ASCII`**: Sets relational comparison and string collation ordering rules.

*(Note: Stale `OPTION DIALECT` syntax was permanently purged under the Zero DIALECT invariant; dialect configuration is not permitted.)*
