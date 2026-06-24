# BASIC++ Program Management

**Version 4.1.1**


---

## Table of Contents

- Entering a Program
- Listing the Program (LIST / LLIST)
  - LIST — Display Program Lines
  - LLIST — List to Printer (STDERR)
- Running the Program (RUN)
- Saving and Loading (SAVE / LOAD)
  - SAVE — Save Program to Disk
  - LOAD — Load Program from Disk
- MERGE — Merge Lines from a File
- NEW — Clear the Program
- UNSAVE — Delete the Last Saved File
- Compiled Output (BSAVE / BLOAD / COMPILE)
  - COMPILE — Transpile BASIC to C Source
  - BSAVE — Save Compiled Bytecode
  - BLOAD — Load Compiled Bytecode
- Program Lifecycle
  - A typical session
  - For larger projects
  - For deployment
- Direct Mode vs. Program Mode
  - Direct Mode (Immediate Mode)
  - Program Mode (Deferred Mode)

---

This guide covers all commands for loading, saving, listing, running, and managing BASIC++ programs.

---

## Entering a Program

Programs are entered by typing numbered lines:

```basic
10 PRINT "Hello, World!"
20 FOR I = 1 TO 5
30 PRINT "Count:"; I
40 NEXT I
50 END
```

Each line begins with a line number (1–65535). Lines are stored in ascending order regardless of entry order.

To replace an existing line, retype it with the same number. To delete a line, type the number alone with no content:

```
30
```

Line 30 is now deleted. Use `AUTO` for faster line entry.

---

## Listing the Program (LIST / LLIST)

### LIST — Display Program Lines

```
LIST               ' Show all lines
LIST 100           ' Show line 100 only
LIST 100-200       ' Show lines 100 through 200
LIST -50           ' Show lines from start through 50
LIST 100-          ' Show lines from 100 to end
```

### LLIST — List to Printer (STDERR)

```
LLIST              ' Print all lines to stderr
LLIST 100-200      ' Print range to stderr
```

`LLIST` works identically to `LIST` but sends output to the standard error stream (typically the printer on vintage systems).

---

## Running the Program (RUN)

```
RUN                ' Execute from the first line
RUN 100            ' Execute starting from line 100
```

`RUN` clears all variables (unless you use `CONT` or `CHAIN`). After the program ends (`END` or last line), you return to direct mode.

If an error occurs during execution, BASIC++ prints:
```
?Error message in line NNN
```

---

## Saving and Loading (SAVE / LOAD)

### SAVE — Save Program to Disk

```basic
SAVE "myprog.bas"          ' Save with explicit filename
SAVE                       ' Auto-save with date/time name
```

Saves the current program in ASCII text format. The file contains the numbered lines exactly as `LIST` would show them.

When `SAVE` is used without a filename, the program is saved with an automatic default name based on the current date and time:

```
SAVE

Saves as:  06-11-202616-27-48.BAS
           (MM-DD-YYYYHH-MM-SS.BAS)
```

This is useful for quick saves during development — you never lose work because you forgot to name the file.

### LOAD — Load Program from Disk

```basic
LOAD "myprog.bas"
```

Loads a program from disk, replacing any program currently in memory. Variables are cleared.

After `LOAD`, the program is ready to `LIST` or `RUN`.

---

## MERGE — Merge Lines from a File

```basic
MERGE "extra.bas"
```

`MERGE` reads lines from a file and adds them to the current program. Existing lines with the same numbers are replaced; lines with new numbers are inserted.

This is useful for:
- Adding subroutine libraries to a program
- Combining program fragments
- Patching specific lines

---

## NEW — Clear the Program

```
NEW
```

`NEW` erases the program from memory **and** clears all variables, arrays, and DATA pointers. It does **not** affect:
- The current dialect setting
- Security level
- Loaded modules
- Alias definitions

---

## UNSAVE — Delete the Last Saved File

```
UNSAVE
```

`UNSAVE` deletes the file that was last used with `SAVE` or `LOAD`. This is especially handy after a bare `SAVE` (auto-named):

```
SAVE                       ' Creates 06-11-202616-27-48.BAS
' ... realize you don't need it ...
UNSAVE                     ' Deletes 06-11-202616-27-48.BAS
```

Without `UNSAVE`, auto-saved files would accumulate on disk. `UNSAVE` keeps your directory clean.

`UNSAVE` is equivalent to `KILL` on the last filename, and originates from the Tymshare SUPER BASIC dialect.

---

## Compiled Output (BSAVE / BLOAD / COMPILE)

### COMPILE — Transpile BASIC to C Source

```basic
COMPILE "output.c"
```

Converts the current BASIC++ program to equivalent ANSI C source code. The output can be compiled with any C compiler:

```bash
cl /O2 output.c           # MSVC
gcc -O2 -o output output.c # GCC
```

**Limitations:**
- Not all statements are supported (e.g., interactive `INPUT` may require linking a runtime support library)
- Dynamic features (`CHAIN`, `MERGE`, `EVAL`) cannot be compiled
- See [Compiling_BASIC_Programs](Compiling_BASIC_Programs.md) for full details

### BSAVE — Save Compiled Bytecode

```basic
BSAVE "prog.bpp"
```

Saves the program in BASIC++ bytecode format. This is a binary representation of the tokenized program that loads faster than ASCII text.

### BLOAD — Load Compiled Bytecode

```basic
BLOAD "prog.bpp"
```

Loads a bytecode file previously saved with `BSAVE`. The program is ready to `RUN` immediately.

---

## Program Lifecycle

### A typical session

1. Enter program lines (or `LOAD` from disk)
2. `LIST` to verify
3. `RUN` to execute
4. Debug as needed (`BREAK`, `STOP`, `VARS`, `TRON`)
5. `SAVE` to preserve your work
6. `NEW` when starting a new program

### For larger projects

1. `SAVE` individual modules as separate files
2. `LOAD` the main program
3. `MERGE` subroutine libraries
4. `RUN` the combined program
5. `SAVE` the final assembled program

### For deployment

1. `COMPILE` to produce standalone C code
2. Compile with a C compiler for your target platform
3. Distribute the native executable

---

## Direct Mode vs. Program Mode

### Direct Mode (Immediate Mode)

Commands without line numbers execute immediately:

```
PRINT 2 + 3             ' Prints: 5
A = 42                  ' Sets variable A
PRINT A                 ' Prints: 42
```

Direct mode is for testing, debugging, and running commands.

### Program Mode (Deferred Mode)

Lines with numbers are stored for later execution:

```basic
10 A = 42
20 PRINT A
```

These lines are stored but not executed until you type `RUN`.

Most commands work in both modes. Exceptions:
- `RUN`, `LIST`, `LOAD`, `SAVE`, `NEW` are direct-mode only
- `DATA`, `DEF FN` are typically program-mode only
