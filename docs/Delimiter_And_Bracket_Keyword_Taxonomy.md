<!--
Title:        Delimiter_And_Bracket_Keyword_Taxonomy
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot, bppc, trans)
Authority:    engine/src/eval/ops.c, engine/src/lexer/lexer.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ Delimiter & Bracket Architecture: Keyword-by-Keyword Analysis

The authoritative subsystem specification defining the distinct, non-interchangeable semantic roles of standard parentheses `( )`, square brackets `[ ]`, and curly braces `{ }` across all BASIC++ keywords, functions, statements, and evaluation engines.

---

## 1. Executive Summary & Core Delimiter Invariant

In BASIC++, standard parentheses `( )`, square brackets `[ ]`, and curly braces `{ }` are **strictly non-interchangeable**:

1. **Parentheses `( )` — Dedicated to Infix Notation & Algebraic Grouping**:
   - Algebraic precedence: `(a + b) * c`
   - Standard function argument lists: `SIN(x)`, `MID$(s$, 1, 3)`
   - Traditional array element subscripting: `A(i, j)`
2. **Square Brackets `[ ]` — Dedicated to Prefix Polish Notation (PN) & Bounds**:
   - Prefix Polish Notation expressions: `[+ 1 2 * 3]`
   - Array and string range slicing: `s$[1:5]`, `arr[10..20]`
   - Anonymous list and vector literals: `[1, 2, 3, 4]`
   - Hardware memory width and port offsets: `PEEK[addr, 4]`
   - UDT type array sizing: `Point[10]`
   - UDD sub-channel options: `DEV[1, aux]`
3. **Curly Braces `{ }` — Dedicated to Postfix Reverse Polish Notation (RPN) & Maps**:
   - Postfix Reverse Polish Notation: `{ 1 2 + 3 * }`
   - JSON, dictionary, and hash map literals: `{"key": "value"}`
   - Structured `TYPE` record initializers: `Point{x: 10, y: 20}`
   - Variadic mathematical set collections: `MIN{1, 5, 2, 8}`
   - String interpolation template expansion: `$'User: {name$}'`

---

## 2. Delimiter Role Comparison Matrix

| Delimiter | Primary Mathematical Discipline | Structural Data Role | Subsystem Specialization |
|:---|:---|:---|:---|
| **`( )` Parentheses** | **Infix Notation** (`A + B`) | Parameter lists, algebraic grouping | Standard GW-BASIC / QBASIC baseline compatibility. |
| **`[ ]` Square Brackets** | **Prefix Polish Notation (PN)** (`+ A B`) | Array/list literals, slices, ranges `[start:end]`, memory widths | Hardware offsets, byte-width modifiers, UDT array sizing `Type[N]`, UDD sub-channels. |
| **`{ }` Curly Braces** | **Postfix Reverse Polish Notation (RPN)** (`A B +`) | JSON/Dictionary maps, struct initializers, set collections | Scoped code blocks, closures, UDD property maps `DEV{baud: 115200}`, EXTERN FFI descriptors. |

---

## 3. Mathematical Function Taxonomy Examples

| Keyword | Standard Infix `( )` | Prefix Range / Bounds `[ ]` | Postfix / Set Collection `{ }` |
|:---|:---|:---|:---|
| **`RND`** | `RND(n)`: Random float or int in $[1, n]$. | `RND[min, max]`: Random number in explicit range. | `RND{seed, count}`: Returns array of $N$ seeded randoms. |
| **`ABS`** | `ABS(x)`: Absolute value $|x|$. | `ABS[arr]`: Element-wise absolute values. | `ABS{v1, v2}`: Euclidean magnitude $\sqrt{v_1^2 + v_2^2}$. |
| **`MIN`** | `MIN(a, b)`: Minimum of two scalars. | `MIN[arr]`: Finds minimum element in array. | `MIN{v1, v2, v3, ... vn}`: Variadic set minimum reduction. |
| **`MAX`** | `MAX(a, b)`: Maximum of two scalars. | `MAX[arr]`: Finds maximum element in array. | `MAX{v1, v2, v3, ... vn}`: Variadic set maximum reduction. |
