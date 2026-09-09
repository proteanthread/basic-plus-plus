<!--
Title:        Super_BASIC
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/math/complex.c, engine/src/statements/io/
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Super BASIC (Tymshare) Compatibility Architecture

The authoritative specification for Tymshare Super BASIC (SDS 940 / DEC PDP-10) language features, native complex numbers, matrix operations, and layout controls in BASIC++ v6.5.2.

---

## 1. History & Architectural Overview

SDS 940 (1968) and DEC PDP-10 (1970-1972) Super BASIC, developed by Tymshare Inc., introduced pioneering additions to early Dartmouth BASIC: first-class complex numbers, bidirectional string/array conversion (`CHANGE`), layout zones (`MARGIN`, `ZONE`), direct stream modes (`TEXT`, `MODIFY`, `APPEND`), template-based formatted printing (`IMAGE`, `FORM`), full line preservation (`LINPUT`), and matrix file I/O (`MAT`).

In BASIC++ v6.5.2, all Tymshare Super BASIC features are supported as native first-class constructs integrated into the unified engine, with 100% backward compatibility for GW-BASIC and QBASIC.

---

## 2. Native Complex Number System

Imaginary literals are lexed directly using trailing `I` or `i` notation:
- Constant syntax: `3 + 4I`, `2.5 - 1.2i`, `0 + 1I`.
- Built-in Complex Functions:
  - `MAG(z)`: Magnitude / absolute value $|z| = \sqrt{	ext{Re}^2 + 	ext{Im}^2}$.
  - `ANGLE(z)`: Phase angle $	heta = 	ext{atan2}(	ext{Im}, 	ext{Re})$ (respects `OPTION ANGLE DEGREES/RADIANS`).
  - `CONJG(z)`: Complex conjugate $	ext{Re} - 	ext{Im} \cdot i$.
  - `REAL(z)` / `IMAG(z)`: Extracts real or imaginary component.

---

## 3. String / Array Conversion (`CHANGE`)

The `CHANGE` statement provides bidirectional conversion between strings and integer ASCII arrays:

- **`CHANGE s$ TO arr%`**: Populates `arr%(0)` with string length and elements `arr%(1)`..`arr%(n)` with ASCII character codes.
- **`CHANGE arr% TO s$`**: Reconstructs string `s$` from character codes in `arr%` using length in `arr%(0)`.

---

## 4. Console and File Layout Controls

- **`MARGIN n`**: Configures maximum print column boundary for the console or file channel.
- **`ZONE n`**: Sets the tab zone width (default 14 columns, configurable from 1 to 64).
- **`LINPUT [#filenum,] var$`**: Reads an entire input line verbatim without stripping commas, semicolons, or quotes.

---

## 5. Matrix Operations (`MAT`)

Supports matrix manipulation and file streaming:
- **`MAT READ #ch, A`** / **`MAT WRITE #ch, A`**: Reads or writes entire matrices directly to or from files.
- **`MAT PRINT [#ch,] A`**: Formats and displays matrix elements using active layout zones.

---

## 6. Example: Tymshare Super BASIC Features

```basic
10 REM Tymshare Super BASIC Demo
20 Z = 3 + 4I
30 PRINT "Complex Z: "; Z; " Mag: "; MAG(Z); " Angle: "; ANGLE(Z)
40 A$ = "HEURISTIC"
50 CHANGE A$ TO V%
60 PRINT "Length in V%(0): "; V%(0)
70 PRINT "First byte in V%(1): "; V%(1)
80 MARGIN 60 : ZONE 10
90 PRINT 10, 20, 30, 40
```
