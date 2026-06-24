# OPTION Statement Reference

**Version 4.1.1**


---

## Table of Contents

- OPTION BASE
- OPTION STRICT
- OPTION ANGLE
- OPTION TAB
- OPTION ZONE
- OPTION ALIASES
- OPTION KEYWORD
- OPTION ARITHMETIC
- Summary Table
- See Also

---

The `OPTION` statement configures interpreter behavior at runtime. All `OPTION` sub-commands take effect immediately and persist until changed, overridden by a dialect switch, or the interpreter exits.

---

## 1. OPTION BASE

```basic
OPTION BASE 0
OPTION BASE 1
```

Sets the default lower bound for array subscripts created with `DIM`.

| Setting | Behavior |
|---------|----------|
| `OPTION BASE 0` *(default)* | `DIM A(10)` creates elements `A(0)` through `A(10)` — 11 elements |
| `OPTION BASE 1` | `DIM A(10)` creates elements `A(1)` through `A(10)` — 10 elements |

`OPTION BASE` must be set **before** any `DIM` statement. Changing it after arrays are dimensioned does not resize existing arrays.

This corresponds to ECMA-55 and ECMA-116 standard BASIC behavior.

```basic
OPTION BASE 1
DIM Scores(5)
FOR I = 1 TO 5
  Scores(I) = I * 10
NEXT I
```

---

## 2. OPTION STRICT

```basic
OPTION STRICT          ' Enable strict mode
OPTION STRICT ON       ' Enable strict mode (same)
OPTION STRICT OFF      ' Disable strict mode (union mode)
```

Controls whether keyword filtering is enforced based on the active dialect.

**Union Mode** *(default, `OPTION STRICT OFF`)*:
All keywords from all 16 dialects are available. A program can freely mix GW-BASIC's `WHILE/WEND` with Commodore's `CLR`, ECMA-116's `WHEN EXCEPTION`, and SUPER BASIC's `UNLESS`.

**Strict Mode** *(`OPTION STRICT` or `OPTION STRICT ON`)*:
Only keywords that belong to the active dialect's bitmask are accepted. Any keyword not native to the current dialect raises a `WHAT?` error.

```basic
DIALECT "C64B"
OPTION STRICT
WHILE I < 10     ' ERROR! WHILE not in Commodore BASIC
CLR              ' OK — CLR is in Commodore BASIC

OPTION STRICT OFF
WHILE I < 10     ' OK — union mode, all keywords accepted
```

Use `OPTION STRICT` to verify that a program is compatible with a specific machine's BASIC.

---

## 3. OPTION ANGLE

```basic
OPTION ANGLE DEGREES
OPTION ANGLE RADIANS
```

Controls whether trigonometric functions (`SIN`, `COS`, `TAN`, `ATN`, `ASIN`, `ACOS`) expect and return values in degrees or radians.

| Setting | Example |
|---------|---------|
| **Radians** *(default)* | `SIN(3.14159)` ≈ 0, `ATN(1)` = 0.7854 |
| **Degrees** | `SIN(90)` = 1.0, `ATN(1)` = 45.0 |

This setting is part of the ECMA-116 Full BASIC standard.

```basic
OPTION ANGLE DEGREES
PRINT SIN(90)          ' 1
PRINT COS(0)           ' 1
PRINT ATN(1)           ' 45

OPTION ANGLE RADIANS
PRINT SIN(3.14159/2)   ' 1
PRINT ATN(1)           ' 0.7854
```

---

## 4. OPTION TAB

```basic
OPTION TAB REAL
OPTION TAB SPACES
OPTION TAB             ' Show current mode
```

Controls how the `TAB()` function in `PRINT` statements moves to a column position.

| Setting | Behavior |
|---------|----------|
| **Spaces** *(default)* | `TAB` emits space characters (ASCII 32) to reach the target column. Compatible with all terminals and file output. |
| **Real** | `TAB` emits actual horizontal tab characters (HT, ASCII 9). Smaller output but depends on terminal tab stop settings (usually every 8 columns). |

```basic
OPTION TAB SPACES
PRINT TAB(20); "HERE"     ' 20 spaces then "HERE"

OPTION TAB REAL
PRINT TAB(20); "HERE"     ' HT characters then "HERE"
```

---

## 5. OPTION ZONE

```basic
OPTION ZONE n          ' Set zone width to n columns
OPTION ZONE 0          ' Reset to dialect default
```

Overrides the `PRINT` comma zone width. When `PRINT` uses commas to separate output items, each item is placed at the next multiple of the zone width.

**Default zone widths by dialect:**

| Dialect | Zone Width |
|---------|-----------|
| PATB (Palo Alto Tiny BASIC) | 8 columns |
| Atari / C64B | 10 columns |
| GW-BASIC | 14 columns *(Microsoft standard)* |
| Apple / CoCo | 16 columns |

```basic
OPTION ZONE 20
PRINT 1, 2, 3
' Output: 1                   2                   3

OPTION ZONE 0
' Reset to dialect default (e.g., 14 for GW-BASIC)
```

---

## 6. OPTION ALIASES

```basic
OPTION ALIASES
```

Displays the current alias system capacity — how many keyword aliases are defined vs. the maximum allowed.

```
Alias capacity: 5 / 256
```

This is a diagnostic command. To manage aliases themselves, use the `ALIAS` command. See [Using_Aliases](Using_Aliases.md) for complete alias documentation.

---

## 7. OPTION KEYWORD

```basic
OPTION KEYWORD UPPER     ' All keywords stored/displayed uppercase
OPTION KEYWORD LOWER     ' All keywords stored/displayed lowercase
OPTION KEYWORD TITLE     ' Keywords in Title Case
OPTION KEYWORD MIXED     ' Keywords stored as-entered (default)
OPTION KEYWORD           ' Show current mode
```

Controls how keywords are stored and displayed in `LIST` output and error messages.

| Setting | Behavior | Example |
|---------|----------|---------|
| **Mixed** *(default)* | Keywords stored exactly as typed | `"print"` stays `"print"` |
| **Upper** | Converted to uppercase | `"print"` → `"PRINT"` |
| **Lower** | Converted to lowercase | `"PRINT"` → `"print"` |
| **Title** | Converted to title case | `"PRINT"` → `"Print"` |

```basic
OPTION KEYWORD UPPER
10 print "hello"
LIST
' Output: 10 PRINT "hello"
```

> **Note:** `OPTION KEYWORD` affects `LIST` output and keyword storage, not parsing. Keywords are always recognized case-insensitively.

---

## 8. OPTION ARITHMETIC

```basic
OPTION ARITHMETIC NATIVE
OPTION ARITHMETIC DECIMAL
```

Controls the arithmetic mode as defined by ECMA-116.

| Setting | Description |
|---------|-------------|
| **Native** *(default)* | Uses hardware IEEE 754 floating-point arithmetic. Fast and compatible with all dialects. |
| **Decimal** | Base-10 decimal arithmetic for exact representation of values like `0.1`. Defined in ECMA-116 but **not currently implemented** — raises a `SORRY` error. |

---

## Summary Table

| Command | Default | Standard |
|---------|---------|----------|
| `OPTION BASE 0\|1` | 0 | ECMA-55 |
| `OPTION STRICT [ON\|OFF]` | OFF (union) | BPP |
| `OPTION ANGLE DEGREES\|RADIANS` | RADIANS | ECMA-116 |
| `OPTION TAB REAL\|SPACES` | SPACES | BPP |
| `OPTION ZONE n` | Dialect default | BPP |
| `OPTION ALIASES` | *(diagnostic)* | BPP |
| `OPTION KEYWORD UPPER\|LOWER\|TITLE\|MIXED` | MIXED | BPP |
| `OPTION ARITHMETIC NATIVE\|DECIMAL` | NATIVE | ECMA-116 |

---

## See Also

- [Quick_Reference](Quick_Reference.md) — Complete keyword listing
- [Creating_Dialects](Creating_Dialects.md) — Dialect configuration
- [Using_Aliases](Using_Aliases.md) — ALIAS command reference
- [Mixing_Dialects](Mixing_Dialects.md) — Multi-dialect programming

*@COPYLEFT ALL WRONGS RESERVED*
