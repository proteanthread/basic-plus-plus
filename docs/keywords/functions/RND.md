# `RND` Random Number Generation Function

## 1. BASIC Usage and Function Definition

The `RND` function generates a pseudo-random floating-point number in the range $[0.0, 1.0)$.

### Syntax Signatures:
```basic
random_val# = RND [(numeric_expression)]
```

### Operational Rules:
- `RND(0)`: Returns the previous random number.
- `RND(1)` or `RND`: Generates the next pseudo-random number in sequence.
- `RND(-x)`: Seeds the PRNG sequence with $x$.

---

## 2. Code Examples

```basic
10 DICE% = INT(RND * 6) + 1 : REM Roll 1..6
20 PRINT "Dice roll: "; DICE%
```
