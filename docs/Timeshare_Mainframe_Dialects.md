# 1960–1979 Timeshare & Mainframe BASIC Dialects in BASIC++

## Overview
BASIC++ provides comprehensive backward compatibility with foundational 1960–1979 timesharing and mainframe BASIC environments, spanning:
- **DEC PDP-11 BASIC-PLUS & BASIC-PLUS-2** (RSTS-11, RSTS/E v1–v10, RSX-11M, RT-11)
- **DEC VAX BASIC / OpenVMS BASIC** (VAX-11, Alpha AXP, Itanium)
- **DEC PDP-10 BASIC-10** (TOPS-10, TENEX, TOPS-20)
- **Dartmouth Time-Sharing System (DTSS)** (1st through 7th Editions & SBASIC)
- **HP 2000 Time-Shared BASIC** (TSB 2000A–2000F) & **HP 3000 MPE BASIC**
- **Tymshare Super BASIC** (SDS 940, XDS 940, PDP-10)
- **IBM CALL/360 BASIC & VS-BASIC / ITF** (System/360, System/370 OS/VS, VM/370 CMS, TSO)
- **GE-265 / Mark I & Mark II Time-Sharing BASIC**
- **Univac 1100 Series CTS / Exec 8 BASIC**
- **Burroughs B5500 / B6700 CANDE BASIC**
- **CDC 6000 / CYBER KRONOS / NOS BASIC**
- **Honeywell 6000 / Multics BASIC**
- **Data General Extended / TSS BASIC & Business BASIC** (Nova, Eclipse RDOS, AOS, TSS)
- **Prime Computer Prime BASIC / BASIC/VM** (Prime 300/400/500/750 PRIMOS)
- **ICL 1900 / 2900 Series BASIC & Wang 3300 BASIC**
- **NCR Century / Criterion Time-Sharing BASIC** (VRX / B-Series)
- **Alpha Micro AMOS AlphaBASIC** (Multi-user record locking)
- **MAI Basic Four / Business BASIC (BB1, BB2, BB3, BBx)** (Channel introspection & Hex conversion)
- **IBM System/34, System/38 & AS/400 BASIC** (`WORKSTN` virtual terminal mapping)
- **Siemens BS2000 & Norsk Data SINTRAN III BASIC**
- **Xerox Sigma 5/7/9 UTS / CP-V BASIC**
- **Pick OS / PICK/BASIC & Universe** (Multivalue dynamic arrays)
- **ECMA-116 & ANSI X3.113-1987 Full BASIC** standard

All features operate seamlessly without compromising 100% compatibility with **GW-BASIC**, **BASICA**, and **QBASIC**.

---

## 1. Matrix Operations (`MAT` Suite & `DET`)
Full support for the Dartmouth DTSS and ECMA-116 standard matrix arithmetic suite:
- `MAT A = CON` — Initializes matrix with all 1s.
- `MAT A = ZER` — Initializes matrix with all 0s.
- `MAT A = IDN` — Creates an identity matrix.
- `MAT C = A + B` — Matrix addition.
- `MAT C = A - B` — Matrix subtraction.
- `MAT C = A * B` — Matrix multiplication.
- `MAT C = (k) * A` — Matrix scalar multiplication.
- `MAT C = INV(A)` — Matrix inversion.
- `MAT C = TRN(A)` — Matrix transposition.
- `D = DET(A)` — Returns the determinant of a square matrix.
- `MAT PRINT #ch, A` / `MAT INPUT #ch, A` — Formatted matrix channel I/O.
- `MAT WRITE #ch, A` / `MAT READ #ch, A` — Binary record matrix stream I/O.

---

## 2. String Translation & DEC Data Packaging
- `CHANGE S$ TO A` — Populates array `A` with the ASCII character codes of string `S$`, setting `A(0)` to the length.
- `CHANGE A TO S$` — Reconstructs string `S$` from array `A` using `A(0)` as the character count.
- `CVT%$(int_val)` — Packs a 16-bit signed integer into a 2-byte binary string (DEC RSTS/E).
- `CVT$%(str_val)` — Unpacks a 2-byte binary string into a 16-bit signed integer.
- `CVTF$(float_val)` / `CVT$F(str_val)` — Single-precision 4-byte float packing and unpacking.
- `CVT$$(str, flags)` / `EDIT$(str, flags)` — Applies bitmask string transformations (trim whitespace, compress spaces, uppercase, remove control characters).
- `RAD$(radix50_val)` — Decodes DEC Radix-50 numeric encoding into ASCII text.
- `SWAP%(int_val)` — Exchanges the low and high bytes of a 16-bit integer.

---

## 3. String Length Sizing (`DIM A$(n)*len` & `DIM S$*len`)
Support for DTSS 7th Edition, DEC BASIC-PLUS-2, and QBASIC fixed-capacity string definitions:
- `DIM A$(10) * 30` — Sized string array (allocates 10 elements with 30-char capacity metadata).
- `DIM S$ * 80` — Sized scalar string buffer.
- `DIM S AS STRING * 40` — Explicit QBASIC-style string variable declaration.

---

## 4. Pick OS Multivalue Dynamic Arrays
Full support for Pick OS, PICK/BASIC, and UniVerse dynamic delimited data structures:
- `DCOUNT(str, delim)` — Returns the number of fields/attributes delimited by `delim` in `str`.
- `FIELD(str, delim, instance [, count])` — Extracts one or more delimited fields by index.
- `EXTRACT(dyn_arr, attr [, val [, subval]])` — Extracts an attribute (delimited by `^` or ASCII 254), value (`]` or ASCII 253), or subvalue (`\` or ASCII 252).
- `COUNT(str, substr)` — Counts occurrences of `substr` within `str`.
- `INS item BEFORE dyn_arr<attr>` / `DEL dyn_arr<attr>` / `REPLACE(dyn_arr, attr, item)` — Dynamic array mutations.

---

## 5. HP 2000 / 3000 String Slicing & Multi-File Binding
- `A$[start, end]` — Bracket substring extraction (HP 2000 / HP 3000 syntax).
- `A$[start]` — Bracket slice from `start` to end of string.
- `A$[start, end] = expr$` — Substring in-place mutation.
- `A$(start, end)` — Parenthesis substring slicing (automatically disambiguated from 2D string arrays `A$(row, col)` with zero GW-BASIC regression).
- `FILES F1$, F2$, *` — Multi-file channel assignment (binds channel #1 to F1, channel #2 to F2, skips channel #3). Single-argument `FILES [pattern$]` preserves standard GW-BASIC directory listing.

---

## 6. NCR Century / Criterion Banking Decimals
- `ROUND(x [, decimals])` — Commercial decimal rounding to `decimals` digits (default 0).
- `TRUNC(x [, decimals])` / `TRUNCATE(x [, decimals])` — Commercial truncation towards zero.

---

## 7. Business BASIC (BBx) & IBM WORKSTN Terminal Device
- `FID(channel)` / `FID$(channel)` — Returns file/device identification string (file path, mode).
- `FIN(channel)` — Returns numeric file/device information (current position or file length).
- `FPT(number)` — Returns fractional part of a number (`FPT(123.456)` -> `0.456`).
- `IPT(number)` — Returns integer part of a number (`IPT(123.456)` -> `123.0`).
- `HTA$(str$)` / `HTA(str$)` — Converts ASCII string into 2-digit Hexadecimal representation (Hex to ASCII).
- `ATH$(hex_str$)` / `ATH(hex_str$)` — Converts Hexadecimal string into raw ASCII bytes (ASCII to Hex).
- `OPEN "WORKSTN" AS #ch` / `OPEN "WS" AS #ch` — IBM System/34 / AS-400 5250 Workstation virtual terminal screen device mapping.

---

## 8. Tektronix 4010/4014 Vector Storage Tubes & Honeywell GCOS Qualifiers
- `TEK$(x, y)` — Generates 4-byte 10-bit Tektronix vector coordinate word (`Hi-Y, Lo-Y, Hi-X, Lo-X`).
- `VEC$(x1, y1, x2, y2)` — Generates 9-byte vector line draw stream (`GS` + move + draw).
- `OPEN "TEK:" AS #ch` / `OPEN "TEKTRONIX" AS #ch` / `OPEN "4010" AS #ch` — Tektronix virtual vector display stream device.
- `OPEN "PRMFL:file" AS #ch` / `OPEN "TAPE:file" AS #ch` / `OPEN "DISC:file" AS #ch` / `OPEN "DISK:file" AS #ch` — Honeywell 6000 / GCOS & Multics storage stream qualifiers.
- `DISP <expr> [;|, ...]` — Wang 2200 / HP 9830 display statement (alias for `PRINT`).
- `KEYIN$` — Wang 2200 single non-blocking console keystroke polling function (alias for `INKEY$`).

---

## 9. Control Flow: Standalone `UNLESS` & Postfix Modifiers
- `UNLESS <cond> [THEN] <stmts> [ELSE <stmts>]` — Conditional branch executed when condition is false (Tymshare Super BASIC / DEC).
- **Trailing Postfix Modifiers**:
  - `stmt IF <cond>` — Executes statement if condition is true.
  - `stmt UNLESS <cond>` — Executes statement if condition is false.
  - `stmt WHILE <cond>` — Loops statement while condition remains true.
  - `stmt UNTIL <cond>` — Loops statement until condition becomes true.
  - `stmt FOR var = start TO end [STEP s]` — Loops statement across variable range.
  - Supports multiple unnested postfix modifiers evaluated right-to-left.

---

## 10. Timesharing Terminal, Process & Resource Locking
- `WHO` — Displays active timesharing processes, logged-in accounts, TTY devices, and priorities.
- `TTY <line>` — Inspects or redirects I/O to a specific terminal line.
- `PRIORITY <level>` — Adjusts scheduling priority (1–10) in the multitasking engine.
- `PAGE [lines]` / `NOPAGE` — Sets page height or disables pagination for console and printer output.
- `NOMARGIN` / `MARGIN [width]` — Controls terminal column line-wrapping margins.
- `REWIND #ch` — Resets the file channel pointer to the beginning of the file.
- `BACKSPACE #ch` — Positions the file channel pointer back by one record or line.
- `LOCK #ch [, [start] [TO end]]` / `UNLOCK #ch [, [start] [TO end]]` — Multi-user file record locking (Alpha Micro AMOS & MS-DOS).
- `SYS(n)` — Multi-mode timesharing telemetry enquiry function (RAM, CPU, system stats).
- `WHENEVER <condition> DO <statement>` — Continuous condition monitor (IBM VS-BASIC).

---

## 11. Virtual Arrays (`VDIM` / `DIM #`)
- `VDIM array_name(dims)` / `DIM #ch, array_name(dims)` — Allocates virtual arrays mapped directly to file channels or segmented virtual memory (`vmem`), allowing multi-gigabyte matrices with minimal physical RAM footprints.
