BASIC++ Interpreter
===================
Version 0.21.0
@COPYLEFT ALL WRONGS RESERVED

BASIC++ is a multi-dialect BASIC interpreter written in portable
ANSI C89/C90.  It can emulate GW-BASIC, QBasic, TRS-80 CoCo BASIC,
Tiny BASIC, Dartmouth BASIC, and its own extended dialect -- all in
a single 346 KB executable.

FEATURES
--------
  * Multi-dialect engine with runtime switching (DIALECT command)
  * Full GW-BASIC / QBasic file I/O (sequential, random-access, binary)
  * Virtual memory maps (MSDOS, C64, Atari 8-bit, Apple II, ZX Spectrum)
  * Shell scripting: SHELL$(), pipes, redirects, shebang support
  * Security sandboxing with configurable trust levels
  * Extensible module/plug-in system
  * Virtual device layer (console, error, file, user-defined)
  * Built-in self-test (SELFTEST command)
  * ALIAS system for keyword remapping
  * 512K variable memory, 64K virtual address space
  * No external dependencies -- pure ANSI C

QUICK START
-----------
  Build:
    Windows (MSVC):  cl /O2 /Fe:basicpp.exe *.c
    Linux (GCC):     gcc -O2 -o basicpp *.c -lm
    macOS (Clang):   clang -O2 -o basicpp *.c -lm

  Run:
    basicpp                   Interactive REPL
    basicpp program.bas       Load and run a program

  First commands:
    PRINT "Hello, World!"     Direct mode
    10 PRINT "Hello"          Enter a program line
    LIST                      Show the program
    RUN                       Execute
    SAVE "hello.bas"          Save to disk
    NEW                       Clear program

DOCUMENTATION
-------------
  Users_Guide.txt               Getting started, environment, commands
  Programmers_Guide.txt         Complete language reference
  How_To_Compile.txt            Building from source on all platforms
  Self_Programming.txt          Meta-programming and self-modification
  Scripting_Functions.txt       Shell integration, pipes, redirects
  Mixing_Dialects.txt           Multi-dialect programming
  Using_Aliases.txt             Keyword remapping with ALIAS
  Advanced_Matrices.txt         DIM arrays and matrix operations
  File_IO.txt                   Sequential, random-access, binary files
  Older_Dialects.txt            Emulating classic systems and memory maps
  Creating_Dialects.txt         Building custom dialect configurations
  Advanced_DEF.txt              DEF FN, FUNCTION/SUB, closures
  External_Modules.txt          Modules, plug-ins, system services
  Error_Handling.txt            ON ERROR, RESUME, ERR, ERL
  Graphics_Sound.txt            SCREEN, DRAW, LINE, SOUND, PLAY
  Debugging.txt                 TRON, TROFF, STOP, CONT
  Security.txt                  Sandboxing and trust levels
  Virtual_Devices.txt           VDev system, INP, OUT, custom devices
  Quick_Reference.txt           Alphabetical keyword reference card
  Compiling_BASIC_Programs.txt  Transpile BASIC to native executables
  Memory_Maps.txt               Creating and using MEMMAP presets
  Virtual_Machines.txt          Virtual machines, consoles, terminals
  Virtual_Filesystem.txt        Safe local file access (deep dive)
  Virtual_Network.txt           TCP, UDP, TLS, Telnet, SSH, FTP, IRC,
                                NNTP, SMTP, HTTP, WebSockets, IRATA/PLATO
  USB_Devices.txt               USB HID (gamepads, joysticks), USB serial
                                (FTDI, CH340, Arduino), device enumeration
  User_Defined_Types.txt        TYPE...END TYPE, typed variables, fields,
                                  arrays of records, internal architecture
  Screen_And_Console.txt        LOCATE, COLOR, CLS, WIDTH, PRINT USING,
                                  INKEY$, KEY macros, TAB, SPC, CSRLIN, POS
  Subroutines_And_Functions.txt GOSUB/RETURN, SUB/FUNCTION, DEF FN, CALL,
                                  call stack, scoping, recursion, EXIT SUB
  String_Handling.txt           LEN, LEFT$/RIGHT$/MID$, ASC/CHR$/VAL/STR$,
                                  LCASE$/UCASE$/LTRIM$/RTRIM$, string pool
  Internals_And_Architecture.txt  Boot sequence, memory pools, lexer/parser
                                  pipeline, RuntimeState, value system, VDev,
                                  function registry, dialect engine, modules,
                                  security model, VM layer, compiler/transpiler
  Secure_Coding.txt               Input validation, defensive file I/O, shell
                                    safety, string pool awareness, error handling
                                    patterns, security levels, deployment recipes
  Arrays_And_Matrices.txt          DIM, OPTION BASE, REDIM, ERASE, 1D/2D arrays,
                                     string arrays, MAT operations (arithmetic,
                                     TRN, INV), sorting, searching, game boards,
                                     stacks/queues, performance tips, pool budget

EXAMPLE PROGRAMS
----------------
  aceyducey.bas               Classic Acey-Ducey card game
  stars.bas                   Star field animation

LICENSE
-------
  See LICENSE.txt.  @COPYLEFT ALL WRONGS RESERVED.
