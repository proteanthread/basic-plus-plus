<!--
Title:        Vintage_Ecosystems_Reference
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/dispatch/, engine/include/runtime/language_descriptor.h
Generated:    no, hand-written
Status:       current
-->

# Vintage Ecosystems Complete Language & Dialect Reference Manual

The master architectural reference for the 7 historic computer families and vintage BASIC dialect lineages supported natively in BASIC++ v6.5.2.

---

## 1. Architectural Overview

BASIC++ v6.5.2 provides unified backward compatibility across 7 vintage computer and dialect families. Rather than isolating dialects into incompatible siloed engines, all historical features are cleanly adapted into the unified AST evaluator, memory architecture, and virtual device system with zero mode switches.

---

## 2. The Seven Vintage Ecosystem Families

### Family 1: Tandy TRS-80 & CP/M Disk Systems
- Dialects: TRSDOS Level II, Disk BASIC, Microsoft MBASIC 5.21, CBASIC/CB-80.
- Features: `%   %` formatting masks, `PEEK`/`POKE` memory mapped video, sector buffers (`FIELD`, `LSET`, `RSET`), and `DEFUSR` machine-code bridges.

### Family 2: Commodore 8-Bit Line
- Dialects: Commodore PET, VIC-20, C64, C128 (BASIC v2.0, v3.5, v7.0).
- Features: Screen memory poking, `SYS` calls, `TI`/`TI$` clock counters, `GET` key input, and PETSCII graphic character mapping.

### Family 3: Apple II Family
- Dialects: Integer BASIC (Wozniak), Applesoft BASIC (Microsoft), ProDOS.
- Features: `HOME`, `TEXT`, `GR`, `HGR`, `COLOR=`, `PLOT`, `HLIN`, `VLIN`, `VTAB`, `HTAB`, and `PR#`/`IN#` slot routing.

### Family 4: Atari 8-Bit Computers
- Dialects: Atari BASIC, Turbo-BASIC XL.
- Features: 4-voice `SOUND` registers, `POSITION`, `GRAPHICS` modes, player-missile graphics, and string slicing.

### Family 5: Sinclair Computing
- Dialects: ZX81, ZX Spectrum BASIC, Sinclair QL SuperBASIC.
- Features: `BORDER`, `INK`, `PAPER`, `FLASH`, `BRIGHT`, `INVERSE`, `BEEP`, `REPeat...END REPeat`, and `s$(start TO end)` slicing.

### Family 6: Mainframe Timesharing Dialects
- Dialects: Dartmouth DTSS BASIC, GE-265, Tymshare Super BASIC, DEC BASIC-PLUS / BASIC-PLUS-2.
- Features: Matrix algebra (`MAT`), native complex numbers (`3+4I`), `CHANGE`, `LINPUT`, `MARGIN`, `ZONE`, and string slicing.

### Family 7: Microsoft PC Lineage
- Dialects: IBM Cassette/Disk BASIC, GW-BASIC, BASICA, QuickBASIC 1.0-4.5, PDS 7.x, Visual Basic for DOS 1.0.
- Features: `SCREEN` modes, `LINE`, `CIRCLE`, `PAINT`, `PALETTE`, `PLAY`, `SOUND`, `DRAW`, `SUB`/`FUNCTION`, `TYPE`, and event trapping (`ON KEY`, `ON TIMER`).

---

## 3. Folklore Compatibility Invariants

- **Relational Truth Value**: Relational expressions evaluate strictly to `-1` for true and `0` for false across all targets.
- **`FOR...NEXT` Exit State**: Loop counters retain `limit + step` after natural loop termination.
- **Bare `NEXT`**: Matches and terminates the innermost active `FOR` loop.
