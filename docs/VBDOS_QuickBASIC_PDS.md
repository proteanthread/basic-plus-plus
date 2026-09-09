<!--
Title:        VBDOS_QuickBASIC_PDS
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/dispatch/, engine/src/statements/
Generated:    no, hand-written
Status:       current
-->

# QuickBASIC 1.x-4.5, PDS 7.x & Visual Basic for DOS Compatibility Architecture

The authoritative specification for Microsoft QuickBASIC, BASIC Professional Development System (PDS 7.0/7.1), and Visual Basic for DOS 1.0 runtime compatibility in BASIC++ v6.5.2.

---

## 1. Lineage and Overview

BASIC++ v6.5.2 provides 100% native runtime parity for programs authored across the modern vintage Microsoft compiled BASIC line:
- **Microsoft QuickBASIC 1.0-4.5 (1985-1988)**: Modular procedures (`SUB`/`FUNCTION`), user-defined records (`TYPE`), dynamic arrays, and BGI/EGA/VGA screen modes.
- **BASIC PDS 7.0/7.1 "QuickBASIC Extended" (1989-1990)**: ISAM database engine support, expanded memory arrays, and the 64-bit fixed-point `CURRENCY@` data type.
- **Visual Basic for DOS 1.0 (1992)**: Text User Interface (TUI) forms, event-driven controls, menus, and dialog windows.

---

## 2. Language Architecture and Type Parity

BASIC++ implements the complete Microsoft type system and suffix conventions:
- Integers: 16-bit `INTEGER%` and 32-bit `LONG&`.
- Floating-Point: 32-bit `SINGLE!` and 64-bit `DOUBLE#`.
- Currency: 64-bit fixed-point `CURRENCY@` (4 decimal places).
- Strings: Dynamic `STRING$` and fixed-length `STRING * length`.
- Structured Records: `TYPE...END TYPE` composite data records.

---

## 3. Preserved Vintage Folklore Invariants

To guarantee backward compatibility with existing vintage codebases:
1. **Relational Truth Value (-1 / 0)**: Relational operators (`=`, `<>`, `<`, `>`, `<=`, `>=`) evaluate to `-1` for true and `0` for false.
2. **`FOR...NEXT` Exit Counter**: Upon loop completion, the loop variable retains the value `limit + step`.
3. **Fielded Buffer Variables**: `FIELD`, `LSET`, and `RSET` write directly into file sector buffers without reallocating string descriptors.
4. **Dual `PRINT USING` Delimiters**: Supports both IBM PC backslash (`\   \`) and TRS-80 percent (`%   %`) string formatting masks.

---

## 4. Visual Basic for DOS Forms & Event Engine

VBDOS event-driven forms and controls operate natively via the TUI subsystem:
- Forms and Controls: Labels, text boxes, command buttons, check boxes, list boxes, and scroll bars.
- Event Dispatch: Form event procedures (`Form_Load`, `Button_Click`) fire cooperatively through the unified event dispatcher.

---

## 5. Example: QuickBASIC Structured Math & Record

```basic
10 REM QuickBASIC 4.5 Record & Function Demo
20 TYPE AccountRecord
30   AccNum AS LONG
40   Balance AS CURRENCY
50 END TYPE
60 DIM Acc AS AccountRecord
70 Acc.AccNum = 100421
80 Acc.Balance = 1250.75@
90 PRINT "Account: "; Acc.AccNum; " Balance: "; Acc.Balance
```
