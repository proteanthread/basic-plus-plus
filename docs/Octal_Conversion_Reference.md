<!--
Title:        Octal_Conversion_Reference
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/oct.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Octal Conversion Reference (OCT$)

The authoritative technical reference for the `OCT$` intrinsic numeric-to-string conversion function, IEEE-754 rounding rules, radix algorithms, and vintage Microsoft BASIC lineage.

---

## 1. Function Definition & Lineage

`OCT$` is an intrinsic numeric-to-string conversion function that produces the octal (base-8) representation of a numeric expression. The returned value is a string containing the base-8 digits representing the converted integer.

`OCT$` maintains exact behavioral compatibility with vintage Microsoft GW-BASIC, QuickBASIC 4.5, and IBM BASICA:
- Non-integral floating-point expressions are rounded to the nearest integer prior to radix conversion.
- Negative numbers are converted using two's complement integer representations.
- The valid octal digit set consists strictly of characters `0` through `7`.

---

## 2. Syntax & Expression Evaluation

```basic
result$ = OCT$(expression)
```

### Parameters
- `expression`: Any valid scalar numeric expression (integer, single-precision float, or double-precision float). Strings passed to `OCT$` trigger Error 13 (Type Mismatch).

### Runnable Examples

```basic
10 A$ = OCT$(255)
20 PRINT A$               ' Output: 377
30 PRINT OCT$(18)          ' Output: 22 (18 = 2 * 8 + 2)
40 PRINT OCT$(18.6)        ' Output: 23 (rounds 18.6 to 19 = 2 * 8 + 3)
50 PRINT OCT$(-1)          ' Output: 1777777777777777777777 (64-bit two's complement)
```

---

## 3. Error Conditions & Boundaries

| Error Code | Error Name | Condition |
|:---:|:---|:---|
| `Error 2` | Syntax Error | Missing parentheses or malformed expression. |
| `Error 13` | Type Mismatch | Argument evaluates to string rather than numeric value. |
| `Error 14` | Out of String Space | String heap exhaustion preventing allocation of result string. |
