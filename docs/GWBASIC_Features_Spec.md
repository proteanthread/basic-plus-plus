# GW-BASIC Compatibility & Feature Restoration Specification

## Executive Summary
This document provides the authoritative specification and user guide for all **GW-BASIC compatible features, statements, functions, commands, and keywords** implemented in BASIC++.

---

## 1. Console & Text Screen Display
- `CLS [n]`: Clears display: `CLS -1` (text viewport), `CLS -2` (graphics viewport), `CLS -3` (full display buffer, default without arguments), or `CLS n` (`0`–`15` background color clear and active background color update). Emits standard ANSI cursor position reset (`\033[2J\033[H`) through `VConContext`.
- `COLOR [fg] [, bg [, border]]`: Configures text foreground (0-15), background (0-15), and border colors using standard 16-color ANSI SGR escape sequences.
- `LOCATE [row] [, col [, vis]]`: Moves text cursor to 1-indexed `(row, col)` coordinate and toggles cursor visibility (`vis = 0` hides, `vis = 1` shows).
- `HOME [n]`: Moves text cursor to top-left corner `(1, 1)` without clearing text matrix or screen buffer; optionally changes text foreground color to `n` (`0`–`15`).
- `CSRLIN`: Returns live text cursor row coordinate (1-25).
- `POS(0)`: Returns live text cursor column coordinate (1-80).
- `SCREEN [mode]`: Switches visual screen modes (0 = text, 1-13 = graphics).

---

## 2. File I/O & Random Access Buffers
- `OPEN "mode", #n, "filespec" [, reclen]`: Classic GW-BASIC file open syntax.
- `OPEN "filespec" FOR mode AS #n [LEN=reclen]`: Modern QBASIC file open syntax.
- `CLOSE [[#]n1...]` / `RESET`: Flushes and closes specific or all open file channels (1-16).
- `FIELD [#]n, width AS var1$ [, width AS var2...]`: Maps fixed-width string variables directly to random-access file record buffer slices.
- `GET [#]n [, record_num]`: Reads structured record bytes from random-access or binary file channels into record buffer.
- `PUT [#]n [, record_num]`: Writes record buffer bytes directly to random-access disk files.
- `PRINT #n, expression_list`: Writes formatted text data items to open sequential file streams.
- `INPUT #n, var_list` / `LINE INPUT #n, string_var`: Reads delimited values or full line strings from sequential files.
- `WRITE #n, expression_list`: Exports CSV-formatted double-quoted strings and serialized numbers to disk files.

---

## 3. Data & Memory Management
- `DATA constant1 [, constant2...]`: Stores static numeric and string literals.
- `READ var1 [, var2...]`: Sequential reading of DATA literals into target variables.
- `RESTORE [line_num]`: Resets DATA read pointer to beginning of program or specified line number.
- `OPTION BASE {0 | 1}`: Configures 0-indexed or 1-indexed array subscript base.
- `OPTION EXPLICIT`: Enforces mandatory explicit variable declarations (`DIM`/`LET`).
- `SWAP var1, var2`: In-place value exchange between two numeric or string variables.
- `LSET var$ = expr$` / `RSET var$ = expr$`: Left or right justifies string expressions within fixed-length string variables or FIELD buffers.
- `ERASE array_name1 [, array_name2...]`: Deallocates array memory buffers.
- `CLEAR`: Clears all variables and array allocations.

---

## 4. Error Trapping & Recovery
- `ON ERROR GOTO {line | 0}`: Enables asynchronous error handler routine or disables error trapping (`0`).
- `RESUME [0 | NEXT | line]`: Clears active error status and resumes execution at faulting statement, next line, or target line.
- `ERROR n`: Simulates runtime error code `n`.

---

## 5. Keyboard Input, Soft-Keys & Program Management
- `KEY ON` / `KEY OFF`: Toggles row 25 function soft-key label display.
- `KEY n, "string"`: Customizes function key `n` label string.
- `ON KEY(n) GOSUB line`: Sets interrupt handler for function key presses.
- `VERSION "x.y.z"`: Tags in-code program version number.
- `VER` / `VER$([target$])`: Queries host engine or tagged program version.
