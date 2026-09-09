<!--
Title:        Universal_Registers_And_UDX
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_udx.c, engine/src/statements/system/stmt_reg.c
Generated:    no, hand-written
Status:       current
-->

# Universal Register Bus, Calculators & Universal Data Exchange (UDX) Architecture

The authoritative specification for the universal register bus (`REG.*`), Prefix (`PN`) and Postfix (`RPN`) operational calculators, Universal Data Exchange (`UDX`), and delimiter orthogonality in BASIC++ v6.5.2.

---

## 1. Overview and Structural Invariants

BASIC++ implements a unified data bus architecture built on two complementary foundations:

1. **Universal Register Bus (`libreg`)**: A bank of fast, typed virtual CPU registers (`REG.A` through `REG.Z`, `REG.R0` through `REG.R15`) providing low-latency scalar and vector storage.
2. **Universal Data Exchange (`UDX`)**: A shared-memory channel and transfer mechanism (`stmt_udx.c`) enabling zero-copy data routing between virtual devices, background tasks, and peripheral busses.
3. **Strict Delimiter Orthogonality**: Standard parentheses `( )`, square brackets `[ ]`, and curly braces `{ }` maintain non-interchangeable semantics:
   - `( )`: Infix algebraic grouping and function argument lists.
   - `[ ]`: Prefix Polish Notation (`PN`), array/string slicing, memory width indexing.
   - `{ }`: Postfix Reverse Polish Notation (`RPN`), dictionary maps, UDT initializers.

---

## 2. The Universal Register Bus (`REG.*`)

Registers are addressed directly in expressions and assignment statements:
- Scalar registers: `REG.A`, `REG.B`, `REG.X`, `REG.Y`.
- Indexed vector registers: `REG.R(0)` through `REG.R(15)`.
- Register operations: `REG.RESET`, `REG.DUMP`, `REG.SWAP REG.A, REG.B`.

---

## 3. Operational Calculators: Prefix (PN) and Postfix (RPN)

BASIC++ integrates dedicated stack calculators:
- **Prefix Polish Notation (`[ ]`)**: `[+ 10 (* 2 5)]` evaluates operators in prefix sequence.
- **Postfix Reverse Polish Notation (`{ }`)**: `{ 10 2 5 * + }` pushes values onto the calculation stack and evaluates postfix operators.

---

## 4. Universal Data Exchange (`UDX`)

Implemented in `engine/src/statements/system/stmt_udx.c`:
- **`UDX TRANSFER src_channel, dst_channel`**: Directly streams data from `src_channel` to `dst_channel` with zero intermediate copying.
- **`UDX CHANNEL name$, discipline$`**: Configures a named UDX pipe with a specific buffering discipline (`FIFO`, `LIFO`, `RING`).

---

## 5. Example: Register Operations and Prefix Calculator

```basic
10 REM Universal Registers & Prefix Math
20 REG.A = 42
30 REG.B = 100
40 REG.SWAP REG.A, REG.B
50 PRINT "REG.A: "; REG.A; " REG.B: "; REG.B
60 Result = [+ REG.A (* REG.B 2)]
70 PRINT "Prefix Expression Result: "; Result
```
