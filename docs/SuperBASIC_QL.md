<!--
Title:        SuperBASIC_QL
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/, engine/src/eval/
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 SuperBASIC (Sinclair QL) Compatibility Architecture

The authoritative specification for Sinclair QL SuperBASIC language constructs, named procedures, structured loops, and inline range slicing in BASIC++ v6.5.2.

---

## 1. History & Architectural Overview

SuperBASIC was the resident advanced operating system and language of the Motorola 68008-based Sinclair QL (Quantum Leap), authored by Jan Jones in 1984. It introduced advanced structured programming features: named procedures and functions, loop labels with targeted `EXIT`, multi-condition `SELect ON`, and first-class string and array slicing.

In BASIC++ v6.5.2, all Sinclair QL SuperBASIC constructs are recognized directly by the master parser with zero dialect switching (upholding the Zero DIALECT invariant).

---

## 2. Structured Loops and Targeted Exits

SuperBASIC introduced named `REPeat` loops with targeted loop control:

```basic
REPeat LoopName
  statements
  IF exit_condition THEN EXIT LoopName
  IF skip_condition THEN NEXT LoopName
END REPeat [LoopName]
```

- **`REPeat [name]`**: Begins an infinite loop block, optionally identified by `name`.
- **`EXIT [name]`**: Breaks execution out of the innermost loop or specifically designated named loop.
- **`NEXT [name]`**: Skips remaining statements and continues with the next iteration of the target loop.
- **`END REPeat [name]`**: Closes the designated loop block.

---

## 3. Procedures and Functions (`DEF PROC` / `DEF FN`)

SuperBASIC used clean `DEF PROC` and `DEF FN` procedure headers:

```basic
DEFine PROCedure CalcTotals(x, y)
  PRINT "Sum: "; x + y
END DEFine

DEFine FuNction Average(a, b)
  RETurn (a + b) / 2
END DEFine
```
In BASIC++, shortened keyword variants (`DEF PROC`, `END PROC`, `DEF FN`, `END FN`, and `RETurn`) are fully recognized.

---

## 4. Multi-Way Selection (`SELect ON`)

```basic
SELect ON choice
  ON choice = 1: PRINT "Option 1"
  ON choice = 2 TO 5: PRINT "Option 2-5"
  ON choice = REMAINDER: PRINT "Default"
END SELect
```

---

## 5. Example: Sinclair QL Structured Loop

```basic
10 REM Sinclair QL SuperBASIC Demo
20 Count = 0
30 REPeat CounterLoop
40   Count = Count + 1
50   PRINT "Iteration: "; Count
60   IF Count >= 5 THEN EXIT CounterLoop
70 END REPeat CounterLoop
80 PRINT "Loop finished successfully."
```
