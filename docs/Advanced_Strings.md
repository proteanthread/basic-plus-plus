<!--
Title:        Advanced_Strings
Tier:         1
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/include/runtime/strings.h, engine/src/runtime/strings.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Advanced String Handling

The technical specification of the BASIC++ reference-counted string heap, substring operators, search functions, case conversion, formatting radices, and string interpolation.

---

## 1. The String Subsystem

Strings in BASIC++ are managed by the `StringContext` subsystem (`engine/include/runtime/strings.h`). Unlike many vintage BASIC implementations that use a simple bump allocator, BASIC++ uses reference-counted string handles (`BppStringRef`) on an isolated heap. Each string object tracks its reference count, and the garbage collector (`str_gc`) reclaims unreferenced strings when the heap approaches capacity.

The string heap size depends on the active build profile:
- **Standard Desktop (`baspp`)**: 256 MB string memory pool.
- **Lite REPL (`bpp`)**: 192 MB string memory pool.
- **Batch Script (`bs`)**: 32 MB string memory pool.
- **Microcontroller IoT (`iot`)**: 512 KB string memory pool.
- **FreeDOS 16-bit**: 16 KB string pool.

These values are configured by `BASIC_DEFAULT_STR_MEM` in `engine/include/types/config.h`.

---

## 2. String Creation and Lifecycle

When the evaluator produces a string result—from a string literal, function return, or concatenation—it calls `str_create(ctx, data, length)` to allocate a new `BppStringRef`. The reference count starts at 1. The caller owns this reference and is responsible for releasing it.

When a string is assigned to a variable, `str_add_ref()` increments the reference count. When the variable is reassigned or cleared, `str_release()` decrements the count. When the count reaches zero, the string memory is freed.

---

## 3. Concatenation and Performance

The `+` operator concatenates two strings: `A$ = "HELLO" + " " + "WORLD"`. Internally, `str_concat(ctx, a, b)` allocates a new string whose length is the sum of the two operands, copies both payloads, and returns a new `BppStringRef`.

Chained concatenation in a single expression (`A$ + B$ + C$ + D$`) produces intermediate strings for each `+` operation. The evaluator releases each intermediate after the next concatenation consumes it.

For high-performance string building, use `MID$` assignment to overwrite characters within a pre-allocated buffer, or use `MICROPLEX$` for template-based string construction.

---

## 4. Substring Operations & Search

- **`LEFT$(s$, n)`**: Returns leftmost `n` characters. If `n` exceeds length, returns `s$`.
- **`RIGHT$(s$, n)`**: Returns rightmost `n` characters.
- **`MID$(s$, start [, length])`**: Returns `length` characters starting at 1-based `start`.
- **`MID$` Assignment**: `MID$(s$, start, length) = replacement$` replaces characters in-place without reallocating string descriptors.
- **`INSTR([start,] haystack$, needle$)`**: Returns the 1-based index of `needle$` within `haystack$`, or 0 if not found. Case-sensitive.

---

## 5. Case Conversion & Trimming

- **`UCASE$(s$)`**: Converts lowercase ASCII characters to uppercase.
- **`LCASE$(s$)`**: Converts uppercase ASCII characters to lowercase.
- **`TCASE$(s$)`**: Converts string to Title Case (capitalizing word boundaries).
- **`LTRIM$(s$)`**: Strips leading whitespace (spaces and tabs).
- **`RTRIM$(s$)`**: Strips trailing whitespace.
- **`TRIM$(s$)`**: Strips both leading and trailing whitespace.

---

## 6. Radix Conversion & Numeric Formatting

- **`HEX$(n)`**: Returns the hexadecimal string representation of `n`.
- **`OCT$(n)`**: Returns the octal string representation of `n`.
- **`BIN$(n)`**: Returns the binary string representation of `n`.
- **`STR$(n)`**: Converts number `n` to formatted string (positive numbers prefixed with a leading space).
- **`VAL(s$)`**: Parses string `s$` to double-precision numeric value.
- **`CHR$(n)`**: Returns the single ASCII character for code `n` (0..255).
- **`ASC(s$)`**: Returns the numeric ASCII code of the first character in `s$`.

---

## 7. MICROPLEX$ String Interpolation

`MICROPLEX$(template$, values...)` performs high-speed string interpolation using numbered placeholders (`{0}`, `{1}`, `{2}`) without intermediate string allocations:

```basic
10 NAME$ = "Alice" : AGE% = 30
20 MSG$ = MICROPLEX$("User {0} is {1} years old.", NAME$, AGE%)
30 PRINT MSG$
```

---

## 8. Random-Access File Buffer Conversion (GW-BASIC Heritage)

For binary file record access (`FIELD`, `LSET`, `RSET`, `GET`, `PUT`):
- **`MKI$(n)` / `CVI(s$)`**: 16-bit integer to/from 2-byte binary string.
- **`MKS$(n)` / `CVS(s$)`**: 32-bit single-precision float to/from 4-byte binary string.
- **`MKD$(n)` / `CVD(s$)`**: 64-bit double-precision float to/from 8-byte binary string.
