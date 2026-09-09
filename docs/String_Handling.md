<!--
Title:        String_Handling
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/, engine/include/runtime/string.h
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 String Handling & Manipulation Architecture

The authoritative specification for string allocation, sub-string extraction, range slicing, formatting, and character operations in BASIC++ v6.5.2.

---

## 1. String Memory Architecture

In BASIC++ v6.5.2, string variables and expressions are managed as dynamically sized, reference-counted string descriptors (`BppStringRef`) allocated within an isolated memory heap (`engine/include/runtime/string.h`). 

Unlike vintage 8-bit interpreters that imposed a rigid 255-byte limit, modern BASIC++ supports arbitrarily large strings bounded only by the configured execution memory pool (e.g., 640 MB in `baspp`, 64 MB in `bs`). Vintage programs relying on GW-BASIC, BASICA, or QBASIC string semantics execute without truncation.

---

## 2. Standard Substring Functions

- **`LEFT$(s$, n)`**: Returns the leftmost `n` characters of string `s$`.
- **`RIGHT$(s$, n)`**: Returns the rightmost `n` characters of string `s$`.
- **`MID$(s$, start [, length])`**: Extracts a substring beginning at 1-based index `start` with optional `length`.
- **`MID$(var$, start [, length]) = replacement$`**: Statement replacing characters in-place within `var$`.
- **`INSTR([start,] haystack$, needle$)`**: Searches for substring `needle$` within `haystack$`, returning 1-based character position or `0` if not found.
- **`LEN(s$)`**: Returns character length of string `s$`.

---

## 3. String Conversion and Formatting

- **`CHR$(code)`**: Converts an integer ASCII/byte code (0-255) to a single-character string.
- **`ASC(s$)`**: Returns the integer ASCII code of the first character of string `s$`.
- **`STR$(num)`**: Converts a numeric value to its formatted string representation (leading space for positive numbers).
- **`VAL(s$)`**: Parses numeric value from initial digits of string `s$`.
- **`HEX$(num)`** / **`OCT$(num)`** / **`BIN$(num)`**: Formats an integer as hexadecimal, octal, or binary string.
- **`UCASE$(s$)`** / **`LCASE$(s$)`**: Converts string to uppercase or lowercase.
- **`LTRIM$(s$)`** / **`RTRIM$(s$)`** / **`TRIM$(s$)`**: Strips leading and trailing whitespace.

---

## 4. Modern Slicing and Concatenation

- **Range Slicing**: `s$[start:end]` or `s$(start TO end)` extracts a substring range with 1-based indices.
- **String Concatenation**: `+` concatenates strings.
- **`SPACE$(n)`** / **`STRING$(n, char)`**: Generates strings of repeated spaces or characters.

---

## 5. Example: String Transformation

```basic
10 REM String Manipulation Demo
20 Text$ = "  BASIC++ Language Engine  "
30 Clean$ = TRIM$(Text$)
40 PRINT "Cleaned: ["; Clean$; "]"
50 PRINT "Upper:   "; UCASE$(Clean$)
60 PRINT "Left 7:  "; LEFT$(Clean$, 7)
70 PRINT "Slice:   "; Clean$[1:7]
```
