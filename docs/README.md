# BASIC++ Interpreter

**Version 2.0.0**
*@COPYLEFT ALL WRONGS RESERVED*

BASIC++ is a multi-dialect BASIC interpreter written in portable ANSI C89/C90. It supports 16 classic BASIC dialects — GW-BASIC, QBasic, Commodore 64, Apple II, Atari, TRS-80, ZX Spectrum, Sinclair QL, CP/M MBASIC, Tymshare SUPER BASIC, ECMA standards, and more — all in a single executable.

## Features

- **16-dialect engine** with runtime switching (`DIALECT` command)
- Full **GW-BASIC / QBasic file I/O** (sequential, random-access, binary)
- **12 virtual memory maps** (MSDOS, C64, C128, VIC-20, Plus/4, PET, Atari 8-bit, Apple II, TRS-80, ZX Spectrum, Sinclair QL, bare)
- **1D, 2D, and 3D arrays** (numeric and string)
- Extended math: `MIN`, `MAX`, `AVG`, `MED`, `ROUND`, `COMPLEX`, `REAL`, `IMAG`
- Shell scripting: `SHELL$()`, pipes, redirects, shebang support
- Security sandboxing with configurable trust levels
- Extensible module/plug-in system
- Virtual device layer (console, error, file, user-defined)
- Built-in self-test (`SELFTEST` command)
- `ALIAS` system for keyword remapping
- 512K variable memory, 64K virtual address space
- Modular built-in function architecture (7 category files)
- **No external dependencies** — pure ANSI C

## Quick Start

### Build

```bash
# Windows (MSVC)
cl /O2 /Fe:basicpp.exe *.c

# Linux (GCC)
gcc -O2 -o basicpp *.c -lm

# macOS (Clang)
clang -O2 -o basicpp *.c -lm
```

### Run

```bash
basicpp                   # Interactive REPL
basicpp program.bas       # Load and run a program
```

### First Commands

```basic
PRINT "Hello, World!"     ' Direct mode
10 PRINT "Hello"          ' Enter a program line
LIST                      ' Show the program
RUN                       ' Execute
SAVE "hello.bas"          ' Save to disk
NEW                       ' Clear program
```

## Documentation

| File | Description |
|------|-------------|
| [Users_Guide](Users_Guide.md) | Getting started, environment, commands |
| [Programmers_Guide](Programmers_Guide.md) | Complete language reference |
| [How_To_Compile](How_To_Compile.md) | Building from source on all platforms |
| [Building_BASIC++](Building_BASIC++.md) | Makefile tutorial and build system |
| [FreeDOS_Build](FreeDOS_Build.md) | FreeDOS-specific build guide |
| [Quick_Reference](Quick_Reference.md) | Alphabetical keyword reference card |
| [Options_Reference](Options_Reference.md) | All OPTION sub-commands documented |
| [Self_Programming](Self_Programming.md) | Meta-programming and self-modification |
| [Scripting_Functions](Scripting_Functions.md) | Shell integration, pipes, redirects |
| [Shell_Scripting_Integration](Shell_Scripting_Integration.md) | Bash, batch, PowerShell integration |
| [Mixing_Dialects](Mixing_Dialects.md) | Multi-dialect programming |
| [Using_Aliases](Using_Aliases.md) | Keyword remapping with ALIAS |
| [Advanced_DEF](Advanced_DEF.md) | DEF FN, FUNCTION/SUB, closures |
| [Advanced_Matrices](Advanced_Matrices.md) | DIM arrays and matrix operations |
| [Arrays_And_Matrices](Arrays_And_Matrices.md) | DIM, OPTION BASE, REDIM, ERASE, MAT ops |
| [File_IO](File_IO.md) | Sequential, random-access, binary files |
| [String_Handling](String_Handling.md) | LEN, LEFT$/RIGHT$/MID$, string pool |
| [Subroutines_And_Functions](Subroutines_And_Functions.md) | GOSUB/RETURN, SUB/FUNCTION, DEF FN |
| [Structured_BASIC](Structured_BASIC.md) | SELECT CASE, SUB/FUNCTION, ECMA-116 |
| [User_Defined_Types](User_Defined_Types.md) | TYPE...END TYPE, typed variables, fields |
| [Screen_And_Console](Screen_And_Console.md) | LOCATE, COLOR, CLS, WIDTH, PRINT USING |
| [Graphics_Sound](Graphics_Sound.md) | SCREEN, DRAW, LINE, SOUND, PLAY |
| [Error_Handling](Error_Handling.md) | ON ERROR, RESUME, ERR, ERL |
| [Event_Trapping](Event_Trapping.md) | ON KEY, ON TIMER, ON STRIG events |
| [Debugging](Debugging.md) | TRON, TROFF, STOP, CONT |
| [Debugging_And_Testing](Debugging_And_Testing.md) | ASSERT, TEST/ENDTEST, SELFTEST |
| [Security](Security.md) | Sandboxing and trust levels |
| [Secure_Coding](Secure_Coding.md) | Input validation, defensive patterns |
| [Editing_Commands](Editing_Commands.md) | ALIAS, OVERRIDE, SCOPE, RENUM |
| [System_And_Environment](System_And_Environment.md) | DIALECT, SHELL, ENVIRON, CHAIN |
| [Program_Management](Program_Management.md) | LIST, LOAD, SAVE, MERGE, RUN, COMPILE |
| [Compiling_BASIC_Programs](Compiling_BASIC_Programs.md) | Transpile BASIC to native executables |
| [Transpiler](Transpiler.md) | BASIC-to-C transpiler internals |
| [External_Modules](External_Modules.md) | Modules, plug-ins, system services |
| [How_To_Use_Help](How_To_Use_Help.md) | Built-in HELP system reference |
| [Keyword](Keyword.md) | KEYWORD command reference |
| [Override](Override.md) | OVERRIDE command reference |
| [Scope](Scope.md) | SCOPE command reference |
| [Memory_Maps](Memory_Maps.md) | Creating and using MEMMAP presets |
| [Virtual_Devices](Virtual_Devices.md) | VDev system, INP, OUT, custom devices |
| [Virtual_Machines](Virtual_Machines.md) | Virtual machines, consoles, terminals |
| [Virtual_Filesystem](Virtual_Filesystem.md) | Safe local file access (deep dive) |
| [Virtual_Network](Virtual_Network.md) | TCP, UDP, TLS, Telnet, SSH, HTTP, WebSockets |
| [Virtual_Consoles](Virtual_Consoles.md) | STDOUT and STDERR in BASIC++ |
| [USB_Devices](USB_Devices.md) | USB HID, USB serial, device enumeration |
| [FujiNet](FujiNet.md) | FujiNet network adapter support |
| [Internals_And_Architecture](Internals_And_Architecture.md) | Boot sequence, memory pools, internals |
| [Creating_Dialects](Creating_Dialects.md) | Building custom dialect configurations |
| [Specific_Machine_Dialects](Specific_Machine_Dialects.md) | All 16 dialects explained |
| [Older_Dialects](Older_Dialects.md) | Emulating classic systems |
| [Sinclair_BASIC](Sinclair_BASIC.md) | ZX Spectrum dialect reference (SINC) |
| [SuperBASIC_QL](SuperBASIC_QL.md) | Sinclair QL dialect reference (QLSB) |
| [MBASIC_CPM](MBASIC_CPM.md) | CP/M MBASIC dialect reference (MBAS) |
| [Super_BASIC](Super_BASIC.md) | Tymshare SUPER BASIC reference (SBAS) |

## Example Programs

| File | Description |
|------|-------------|
| `aceyducey.bas` | Classic Acey-Ducey card game |
| `stars.bas` | Star field animation |

## License

See [LICENSE](LICENSE.md). @COPYLEFT ALL WRONGS RESERVED.
