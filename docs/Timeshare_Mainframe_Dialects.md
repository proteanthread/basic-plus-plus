<!--
Title:        Timeshare Mainframe Dialects
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/dispatch/, engine/src/device/
Generated:    no
Status:       Active
-->

# BASIC++ 1960–1979 Timesharing & Mainframe Dialects Architecture

The engineering specification detailing backward compatibility with foundational 1960–1979 timesharing, minicomputer, and mainframe BASIC environments.

## 1. Architectural Role & Capabilities

BASIC++ implements a universal, dialect-agnostic virtual machine that natively accommodates commercial, academic, scientific, and industrial timesharing and mainframe BASIC dialects developed between 1960 and 1979. All features co-exist within the single unified evaluator without mode switching:

1. **DEC PDP-11 & PDP-10 Ecosystem**: Backslash (`\`) and space statement chaining, trailing postfix modifiers (`IF`, `UNLESS`, `WHILE`, `UNTIL`, `FOR`), Virtual Arrays (`VDIM` / `DIM #ch`), byte packing (`CVT%$`, `CVT$%`, `CVTF$`, `CVT$F`), string trimming (`CVT$$` / `EDIT$`), Radix-50 (`RAD$`), byte swap (`SWAP%`), and timeshare session telemetry (`SYS()`, `WHO`, `TTY`, `PRIORITY`).
2. **Dartmouth Time-Sharing System (DTSS 1st–7th Editions)**: Foundational statements, multiline `DEF FN`, full `MAT` matrix suite (`CON`, `ZER`, `IDN`, `INV`, `TRN`, `DET`), and ASCII translation (`CHANGE`).
3. **HP 2000 Time-Shared BASIC (TSB 2000A–2000F)**: Bracket substring slicing (`A$[start, end]`), substring assignment mutation (`A$[s, e] = expr$`), multi-file channel binding (`FILES`), print formatters (`LIN()`, `SPA()`), and timed input (`ENTER`).
4. **Tymshare Super BASIC**: Standalone `UNLESS <cond> [THEN] <stmts> [ELSE <stmts>]`, right-to-left trailing postfix modifiers, and zero-based array indexing (`OPTION BASE 0` / `BASE 1`).
5. **Pick OS / UniVerse Multivalue Architecture**: Dynamic array manipulation (`DCOUNT`, `FIELD`, `EXTRACT`, `INS`, `DEL`, `REPLACE`) using ASCII field delimiters (254, 253, 252).
6. **Business BASIC (BBx / MAI Basic Four)**: Channel introspection (`FID`, `FIN`), fractional math (`FPT`, `IPT`), and hexadecimal string conversion (`HTA$`, `ATH$`).
7. **Mainframe Terminals & Virtual Devices**: IBM System/34 5250 workstation mapping (`WORKSTN`), Tektronix 4010 vector displays (`TEK:`), and Honeywell GCOS permanent file qualifiers.

## 2. Chronological Evolution (1964–1979)

### 1964: DTSS 1st Edition (Dartmouth College)
- **Innovators**: John G. Kemeny & Thomas E. Kurtz.
- **Hardware & OS**: GE-225 paired with Datanet-30 communications processor running DTSS.
- **Milestones**: 14 foundational statements (`LET`, `READ`, `DATA`, `PRINT`, `GOTO`, `IF..THEN`, `FOR..NEXT`, `GOSUB..RETURN`, `DEF FN`, `DIM`, `REM`, `STOP`, `END`), single-character variables (`A`-`Z`), and Teletype Model 33 terminal interface.

### 1965–1966: DTSS 2nd/3rd Edition & Tymshare Super BASIC
- **Innovators**: Dartmouth College, Scientific Data Systems (SDS 940).
- **Milestones**: Immediate-mode commands (`RUN`, `LIST`, `NEW`), `DEF FNA(X)` single-line functions, string variables (`A$`), standalone `UNLESS`, and right-to-left postfix statement modifiers.

### 1968: The Matrix Revolution & Multi-User Expansion
- **Systems**: Dartmouth DTSS 4th Edition, IBM CALL/360, DEC TSS-8 (PDP-8), HP 2000A TSB, Microdata / Pick OS.
- **Milestones**: Complete `MAT` matrix suite, `DET()`, `CHANGE S$ TO A`, System/360 32-bit math, and Pick OS multivalue dynamic arrays.

### 1970–1975: DEC PDP-11 BASIC-PLUS & PDP-10 BASIC-10
- **Systems**: PDP-11 RSTS-11 / RSTS/E, PDP-10 TOPS-10 / TENEX.
- **Milestones**: Space and backslash statement separators, `VDIM` virtual disk arrays, `CVT` numeric-to-string packing, 36-bit floating-point math, terminal control (`MARGIN`, `PAGE`), and multi-user commands (`LOGIN`, `LOGOUT`, `WHO`, `TTY`).

### 1976–1979: High-Precision Mainframe & Structured Era
- **Systems**: DEC BASIC-PLUS-2 / VAX BASIC, IBM VS-BASIC, Dartmouth DTSS 7th Edition / SBASIC, ANSI X3.113.
- **Milestones**: High-precision decimal string arithmetic (`SUM$`, `DIF$`, `PROD$`, `QUO$`), sized scalar strings (`DIM S$*80`), structured `RECORD` and `MAP` buffers, and structured exceptions (`WHEN ERROR IN..USE..END WHEN`).

## 3. Vendor Dialect Reference Matrix

| Vendor / System | Primary Hardware | Key Dialect Features in BASIC++ |
| :--- | :--- | :--- |
| **Dartmouth DTSS** | GE-225, GE-635, Honeywell 66 | `MAT` operations, `CHANGE`, multiline `DEF FN`, `COMPLEX` math |
| **DEC BASIC-PLUS** | PDP-11 (RSTS/E), PDP-10 (TOPS-10) | Trailing modifiers, `CVT` packing, `EDIT$`, `VDIM`, `WHO`, `TTY` |
| **HP 2000 TSB** | HP 2116, 2100, 21MX | Substring slicing `A$[s,e]`, `FILES`, `ENTER`, `LIN()`, `SPA()` |
| **IBM CALL/360 & VS** | System/360, System/370 | `PAUSE`, `WHENEVER` continuous trapping, `WORKSTN` buffers |
| **Tymshare Super BASIC** | SDS 940, XDS 940 | Standalone `UNLESS`, right-to-left postfix modifiers, `BASE 0/1` |
| **Pick OS / UniVerse** | Microdata 1600, Reality | Multivalue dynamic arrays (`DCOUNT`, `FIELD`, `EXTRACT`, `INS`) |
| **Business BASIC** | MAI Basic Four, Data General | `FID()`, `FIN()`, `FPT()`, `IPT()`, `HTA$()`, `ATH$()` |
| **Tektronix** | Tektronix 4010/4014 | Vector graphics escape terminal streams via `TEK:` device |

## 4. Implementation Invariants & Runtime Routing

1. **Strict Delimiter Non-Interchangeability**: HP-style substring slicing (`A$[start, end]`) maps strictly to brackets `[ ]`, maintaining separation from standard infix parentheses `( )` and set braces `{ }`.
2. **Unified AST Dispatch**: All vintage statements (`MAT`, `UNLESS`, `CHANGE`, `EDIT$`) parse into first-class AST nodes.
3. **Postfix Evaluation Guard**: Right-to-left trailing modifier evaluation exempts all 22 structured block keywords, eliminating loop hijacking hangs.
4. **Deterministic Binary Packing**: Data conversion functions (`CVT%$`, `CVTF$`) pack numbers into deterministic byte sequences matching vintage representations.
