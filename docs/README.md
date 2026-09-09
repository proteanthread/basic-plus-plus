<!--
Title:        README
Tier:         4
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, detok, trans)
Authority:    engine/
Generated:    no
Status:       Active
-->

# BASIC++ v6.5.2 README

## 1. What is BASIC++

BASIC++ is a modern, high-performance BASIC interpreter, compiler, and virtual machine written in pure ISO C17. It provides a unified dialect architecture that is fully compatible with GW-BASIC, IBM PC BASICA, QuickBASIC 4.5, VB for DOS 1.0, Tymshare Super BASIC, and ECMA-116 Full BASIC, alongside vintage ecosystems (Sinclair ZX Spectrum, Sinclair QL SuperBASIC, Apple II, Commodore 64, and CP/M).

Version 6.5.2, codename "Phoenix", is the active production release.

## 2. Build Targets

| Target | Executable | Description |
|--------|------------|-------------|
| baspp | baspp.exe / baspp | Flagship Desktop Edition: Full console + SDL2 graphics and audio, TUI editor, segmented memory (640 MB profile) |
| bpp | bpp.exe / bpp | Lite Edition: Headless interactive REPL for terminal, server, and IoT (384 MB profile) |
| bs | bs.exe / bs | Batch Script Runner: Headless non-interactive runner for CI/CD and scripts (64 MB profile) |
| iot | iot.exe / iot | IoT Micro-REPL: Embedded systems, hardware GPIO/SPI/I2C, event loops (2 MB profile) |
| bppc | bppc.exe / bppc | Compiler & Transpiler: Compiles BASIC to C17, standalone executables, or bytecode |
| trans | trans.exe / trans | Transpiler: Source-to-source dialect translator and syntax modernizer |
| detok | detok.exe / detok | Detokenizer: Decodes legacy binary GW-BASIC and BASICA files to source text |
| basicpp | basicpp.dll / .so | Shared engine dynamic library for embedding and language FFI |
| iob | iob.dll / .so | Virtual device bus and I/O buffer dynamic library |

## 3. Quick Start

### Building on Windows (MSVC 2022 / Clang-cl)

```bash
mkdir build_win
cd build_win
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Building on Linux (GCC / Clang)

```bash
mkdir build_linux
cd build_linux
cmake .. -DCMAKE_C_STANDARD=17
cmake --build .
```

### Running

```bash
baspp                          # Launch Desktop REPL (> prompt, Ok status)
baspp program.bas --batch      # Run a program in batch mode
baspp -c "PRINT 2+2"           # Execute a single statement
bpp                            # Launch Lite REPL (] prompt, Ready. status)
bs script.bas                  # Run a script headlessly with zero banner/prompt
iot                            # Launch IoT micro-controller REPL
```

## 4. Key Features

- 551 keywords covering statements, functions, operators, and subsystem commands.
- Unified dialect architecture: All vintage and modern BASIC capabilities co-exist without mode switching.
- Non-recursive VM: All execution state resides on heap-allocated structures, eliminating host C stack overflow.
- Reference-counted strings: Automatic allocation and garbage collection with zero memory leaks.
- Structured programming: IF/ELSEIF/ELSE/END IF, SELECT CASE, DO/LOOP, WHILE/WEND, REPEAT/ENDLOOP, SUB/FUNCTION, TRY/CATCH.
- Metaprogramming & Introspection: ALIAS, OVERRIDE, SCOPE, KEYWORD, SELFTEST.
- 12-library modular architecture: Scales from 18 KB embedded profiles to 640 MB workstation environments.
- 6-level security system: Configurable rings from OPEN to PARANOID.
- BGI Graphics: 13+ standard screen modes, custom resolutions, BGI stroke fonts, and AAlib ASCII terminal fallback.
- Sound: BEEP, SOUND, PLAY macro language, and sound file playback.
- Virtual Networking: TCP/UDP sockets, Gemini, Gopher, TNFS, and FujiNet virtual adapter emulation.
- File and Stream I/O: Sequential, random-access, and binary block modes with file record locking.
- Freestanding systems programming: MEM, PORT, PEEK, POKE, INP, OUT conforming to ISO C17 Section 4 Paragraph 6.

## 5. Documentation

Documentation is maintained with 100% mirror parity across two formats:
- `docs/*.md`: Markdown files for reading, GitHub browsing, and reference.
- `help/*.TXT`: Plaintext files for the in-engine interactive HELP system (pure 7-bit ASCII, wrapped <= 78 columns, CRLF).

Interactive help within the REPL:
- `HELP`: Displays help overview and categorical keyword list.
- `HELP <keyword>`: Queries documentation for a specific statement or function.
- `CATALOG`: Lists active modules and vocabulary items.

## 6. Testing

```bash
baspp -c "SELFTEST"            # Run built-in engine self-test suite
bs tests/suite.bas             # Run automated regression test suites headlessly
```

## 7. Project Structure

```text
engine/          — v6.5.2 engine source code (micro-libraries, headers, HAL)
docs/            — Markdown documentation tree
help/            — Plaintext mirror files for interactive HELP
tests/           — Regression test suites across all 44 dialects
tools/           — Verification scripts, clean utilities, and test runners
```

## 8. Contributing

See the Developer Guide (`docs/Developer_Guide.md` / `help/Developer_Guide.TXT`) for architectural invariants, coding standards, and contribution guidelines. All modifications must pass automated tests with zero timeouts and maintain 100% documentation parity.

## 9. License

MIT License. See LICENSE.md for full text.
