======================================================================
  BASIC++ V6.5.2 README
======================================================================


======================================================================
  1. WHAT IS BASIC++
======================================================================

BASIC++ is a modern BASIC language interpreter, compiler, and virtual machine written in portable C17. It provides a unified dialect that is fully compatible with GW-BASIC, QBASIC, Tymshare Super BASIC, ECMA-116 Full BASIC, and multiple retro computing platforms (ZX Spectrum, Sinclair QL, Apple II, Commodore 64, CP/M).

Version 6.5.2, codename "Phoenix", is the current production release.


======================================================================
  2. BUILD TARGETS
======================================================================

| Target | Description |
|--------|-------------|
| baspp | Standard Desktop Edition ? Full console + SDL2 graphics. GW-BASIC style (> prompt, Ok status). 640 MB RAM. |
| bpp | Lite Edition ? Headless REPL for terminal/IoT. Apple II / Commodore style (] prompt, Ready. status). 384 MB RAM. |
| bs | Batch Script Runner ? Non-interactive. Zero banner, zero prompt. 64 MB RAM. |
| bppc | Compiler & Transpiler ? Compiles BASIC to C17 or bytecode. |
| detok | Detokenizer ? Decodes GW-BASIC binary files to text. |


======================================================================
  3. QUICK START
======================================================================


----------------------------------------------------------------------
  Building on Windows
----------------------------------------------------------------------

```bash
mkdir build_win
cd build_win
cmake ..
cmake --build . --config Release


----------------------------------------------------------------------
  Building on Linux
----------------------------------------------------------------------

```bash
mkdir build_linux
cd build_linux
cmake ..
cmake --build .


----------------------------------------------------------------------
  Running
----------------------------------------------------------------------

```bash
baspp                          # Interactive REPL
baspp program.bas              # Load and run a program
baspp -c "PRINT 2+2"           # Execute a single statement
bpp                            # Lite REPL
bs script.bas                  # Run a script non-interactively


======================================================================
  4. KEY FEATURES
======================================================================

- **367 keywords** covering statements, functions, operators, and subsystem commands.
- **9 dialect modes**: GW-BASIC, QBASIC, ECMA-116, ECMA-55, Super BASIC, Palo Alto Tiny BASIC, Sinclair ZX Spectrum, SuperBASIC QL, Applesoft BASIC.
- **Non-recursive VM** ? All state on heap, no C stack overflow risk.
- **Reference-counted strings** with automatic garbage collection.
- **Structured programming** ? IF/ELSEIF/ELSE/END IF, SELECT CASE, DO/LOOP, SUB/FUNCTION, TRY/CATCH.
- **Metaprogramming** ? ALIAS, OVERRIDE, SCOPE, KEYWORD, custom dialects.
- **12-library modular architecture** scaling from 18 KB embedded to 640 MB desktop.
- **6-level security system** from OPEN to PARANOID.
- **Graphics** ? 13+ screen modes, custom resolutions, 42 retro hardware mode profiles.
- **Sound** ? BEEP, SOUND, PLAY music language, WAV file playback.
- **Networking** ? TCP/UDP, Gemini protocol, FujiNet compatibility.
- **File I/O** ? Sequential, random, binary modes with locking.
- **BIOS emulation** ? Full IBM PC/XT/AT BIOS with video RAM and I/O port emulation.
- **Cross-platform** ? Windows, Linux, macOS, FreeDOS, embedded (ESP32, Pico, Arduino).
- **Self-hosting** ? Programs can define, validate, and document their own language specifications.


======================================================================
  5. DOCUMENTATION
======================================================================

Documentation is available in two formats:
- **docs/*.md** ? Markdown files with full formatting.
- **help/*.TXT** ? Plaintext files for in-interpreter HELP display.

Interactive help: HELP keyword displays help for a specific keyword. CATALOG lists all available keywords. INFO shows system configuration.


======================================================================
  6. TESTING
======================================================================

```bash
baspp -c "SELFTEST"            # Run built-in self-test
baspp tests/gwbasic/*.bas      # Run GW-BASIC compatibility tests


======================================================================
  7. PROJECT STRUCTURE
======================================================================

engine/          ? v6.5.2 source code
  include/       ? Public header files
  src/           ? Implementation source files
  lib/           ? Platform abstraction layer
docs/            ? Markdown documentation
help/            ? Plaintext help files
tests/           ? Regression test suites
samples/         ? Sample programs and model documents
classics/        ? Preserved standalone educational modules


======================================================================
  8. CONTRIBUTING
======================================================================

See the Developer Guide (docs/DEVELOPER_GUIDE.md) for coding standards, the library chain, and contribution workflow. All contributions must pass SELFTEST on both baspp and bpp targets.


======================================================================
  9. LICENSE
======================================================================

MIT License. See docs/LICENSE.md for full text.
