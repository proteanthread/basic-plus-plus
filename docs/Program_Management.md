<!--
Title:        Program_Management
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/, engine/src/bootstrap/
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Program Management Architecture

The authoritative specification for program storage, line buffer editing, session control, file overlays, and execution management in BASIC++ v6.5.2.

---

## 1. Program Creation and Buffer Editing

The interactive interpreter provides direct manipulation commands for managing source program buffers in memory:

- **`NEW`**: Clears the stored program buffer from memory, reclaims variable and array allocations, resets open file channels, and restores the `DATA` pointer to the first entry.
- **`AUTO [start [, step]]`**: Automatically generates sequential line numbers during interactive console entry. Defaults to starting at line 10 with step 10. Exit AUTO mode by pressing `Ctrl+C` or typing a period (`.`) as the first character of a line.
- **`RENUM [new_start [, old_start [, step]]]`**: Renumbers stored program lines. All line targets in `GOTO`, `GOSUB`, `ON...GOTO`, `RESTORE`, and `RESUME` statements are automatically updated to match the renumbered lines.
- **`DELETE [start] - [end]`**: Removes a range of lines from the current program buffer. Forms include `DELETE 100-200`, `DELETE -200` (from beginning up to 200), or `DELETE 300-` (from 300 to end).
- **`LIST [range]`**: Outputs stored program source lines to standard console output.
- **`LLIST [range]`**: Outputs stored program source lines to the primary line printer channel (`LPT1:`).

---

## 2. Program Persistence and Overlays

- **`SAVE "filename.bas" [, A]`**: Writes the active program buffer to disk. The optional `, A` flag forces storage as standard 7-bit ASCII text.
- **`LOAD "filename.bas" [, R]`**: Reads a program file from disk into the interpreter buffer. The optional `, R` switch immediately begins execution upon loading.
- **`MERGE "filename.bas"`**: Merges lines from an external ASCII BASIC file into the existing memory buffer, overwriting colliding lines and inserting new lines in sorted order.
- **`CHAIN [MERGE] "filename.bas" [, line_num]`**: Transfers execution control to an external program overlay. The optional `MERGE` clause overlays lines without resetting common variable buffers.

---

## 3. Execution Control

- **`RUN [line_num | "filename.bas"]`**: Resets runtime state and begins execution from the first stored line, a specified line number, or loads and executes a program file.
- **`STOP`**: Suspends program execution, displays a break message with the current line number, and returns control to the interactive command prompt.
- **`CONT`**: Resumes execution immediately following a `STOP` statement or manual `Ctrl+C` break.
- **`END`**: Terminates program execution normally and closes all open file channels.
