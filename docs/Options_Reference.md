# BASIC++ v6.5.2 Options Reference

## 1. STANDARD EDITION (baspp / baspp.exe)

The standard edition is the flagship desktop build that includes the full language, SDL2 graphics, TUI editor multiplexer, BGI rasterizer, segmented virtual memory, and all optional subsystems. Default memory pool is 640 MB.

### Invocation

`baspp` — Start the interactive REPL with no program loaded.

`baspp filename.bas` — Load the specified file and start the REPL with the program in memory. The program is not automatically executed.

`baspp -c "statement"` — Execute a single statement in non-interactive mode and exit. Used for command-line automation and pipeline integration. The exit code is 0 on success, non-zero on error.

`baspp -c "SELFTEST"` — Run the built-in self-test suite and report results.

### Dialect Selection

`baspp -d CODE` — Set the active BASIC dialect. Available codes:

| Code | Dialect |
|------|---------|
| GWBS | GW-BASIC (default) |
| QBAS | QBASIC |
| E116 | ECMA-116 Full BASIC |
| E055 | ECMA-55 Minimal BASIC |
| SBAS | Tymshare Super BASIC |
| PATB | Palo Alto Tiny BASIC |
| SINC | Sinclair ZX Spectrum BASIC |
| SQLB | SuperBASIC (Sinclair QL) |
| APPL | Applesoft BASIC |

### Logging

`baspp --log` — Enable file logging. The default log file is baspp.log in the current directory.

`baspp --log=path` — Enable file logging to a specific path.

`baspp --debug` — Enable debug-level logging (includes statement execution trace).

`baspp --trace` — Enable trace-level logging (includes lexer tokens, AST nodes, and VM state).

Without any log flag, no log files are generated on disk. This is a firm invariant: BASIC++ never creates .LOG or .OUT files unless explicitly requested.

### Security

`baspp --security=N` — Set the initial security level (0-5). Default is 0 (OPEN).

`baspp --restrict=OP` — Restrict a specific operation at startup. Can be specified multiple times.

## 2. LITE EDITION (bpp / bpp.exe)

The lite edition is a headless interactive REPL optimized for terminal environments, IoT devices, and server-side scripting. It excludes SDL2 graphics, the BGI canvas, SDL audio, the TUI editor multiplexer, and segmented virtual memory (vmem). Default memory pool is 384 MB.

### Invocation

`bpp` — Start the lite interactive REPL.

`bpp filename.bas` — Load a file and start the REPL.

`bpp -c "statement"` — Execute a single statement and exit.

`bpp -d CODE` — Set the active dialect.

### Prompt Style

The lite edition uses the Apple II / Commodore prompt style with the ] prompt character and Ready. status message. The banner reads:

```text
BASIC++ Lite Edition v6.5.2
384 MB RAM Available.

Ready.
]
```

## 3. BATCH SCRIPT RUNNER (bs / bs.exe)

The batch script runner is a headless non-interactive executor optimized for PowerShell, Bash, CGI pipelines, and automated jobs. Default memory pool is 64 MB. It produces zero banner, zero prompt, and zero REPL iterations.

### Invocation

`bs filename.bas` — Execute the script file and exit. Output goes to stdout, errors to stderr.

`bs -d CODE filename.bas` — Execute with a specific dialect.

### Exit Codes

0 — Script completed successfully.
1 — Runtime error occurred.
2 — File not found or could not be opened.
3 — Syntax error during parsing.

### Pipeline Integration

The batch runner is designed for use in shell pipelines. It accepts input from stdin when the program uses INPUT statements without a file channel. Output from PRINT goes to stdout. Error messages go to stderr. This separation allows clean pipeline composition:

```bash
echo "42" | bs compute.bas > result.txt
```

## 4. COMPILER AND TRANSPILER (bppc / bppc.exe)

The compiler transpiles BASIC++ source to clean C17 or appends bytecode directly to a C17 VM stub for standalone executable creation.

### Invocation

`bppc input.bas -o output.c` — Transpile to C17 source.

`bppc input.bas -o output.exe` — Compile to standalone executable (appends bytecode to VM stub).

`bppc input.bas --target c17` — Explicitly select C17 output.

## 5. DETOKENIZER (detok / detok.exe)

The detokenizer decodes legacy GW-BASIC binary-format files to readable ASCII text.

### Invocation

`detok input.bas -o output.txt` — Decode a tokenized GW-BASIC file.

`detok input.bas` — Print decoded text to stdout.

## 6. MEMORY PROFILES

| Profile | Program | Variables | Strings | Scratch | Stack |
|---------|---------|-----------|---------|---------|-------|
| Modern (baspp) | 128 MB | 128 MB | 256 MB | 128 MB | 1023 |
| Lite (bpp) | 64 MB | 64 MB | 192 MB | 64 MB | 1023 |
| FreeDOS 16-bit | 32 KB | 16 KB | 16 KB | 8 KB | 63 |
| Embedded | 8 KB | 4 KB | 4 KB | 2 KB | 31 |

The total announced memory (640 MB for standard, 384 MB for lite, 64 MB for batch runner) is the sum of all pools. Individual pool sizes are configurable at build time through the BASIC_DEFAULT_PROG_MEM, BASIC_DEFAULT_VAR_MEM, BASIC_DEFAULT_STR_MEM, and BASIC_DEFAULT_SCRATCH_MEM defines in engine/include/types/config.h.

## 7. FEATURE SUPPORT GATES

Build-time feature gates control which subsystems are compiled into each edition. The gates are defined in engine/include/types/config.h:

| Feature | Standard | Lite | Notes |
|---------|----------|------|-------|
| SUPPORT_FILES | Yes | Yes | File I/O subsystem |
| SUPPORT_TRY | Yes | Yes | TRY/CATCH exception handling |
| SUPPORT_STRUCT | Yes | Yes | TYPE/CLASS structures |
| SUPPORT_MODULE | Yes | Yes | Module loading system |
| SUPPORT_TASK | Yes | Yes | Background task system |
| SUPPORT_HELP | Yes | Yes | Interactive help system |
| SUPPORT_GRAPHICS | Yes | No | SDL2 graphics and BGI |
| SUPPORT_MAT | Yes | Yes | Matrix operations |
| SUPPORT_NET | Yes | Yes | Network operations |
| SUPPORT_GEMINI | Yes | Yes | Gemini protocol support |
| SUPPORT_BIOS | Yes | No | BIOS virtualization |
| SUPPORT_JSON | Yes | No | JSON parsing |
| SUPPORT_XML | Yes | No | XML parsing |
| SUPPORT_INI | Yes | No | INI file parsing |
| SUPPORT_YAML | Yes | Yes | YAML support |
| SUPPORT_OOP | Yes | Yes | Object-oriented features |
| SUPPORT_EDITOR | Yes | No | TUI editor multiplexer |
