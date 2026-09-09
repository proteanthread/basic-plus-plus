# `pegsol` Peg Solitaire Puzzle Game & Solver Library

## 1. Library Overview & Usage

The `pegsol` library implements English (33-hole) and French (37-hole) Peg Solitaire / Hi-Q board representations, jump move validation, and recursive backtracking puzzle solvers.

### Features:
- **Board Masks**: Bitmask and 2D array representation of cross-shaped solitaire boards.
- **Move Validator**: Validates horizontal/vertical peg jumps over adjacent pegs into empty holes.

---

## 2. BASIC Example

```basic
10 DIM HOLES(7, 7)
20 PRINT "Peg Solitaire board initialized (32 pegs, center empty)."
```
