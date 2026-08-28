# BASIC++ v6.5.2 String Handling

## 1. STRING FUNDAMENTALS

String values in BASIC++ are sequences of 8-bit characters with a maximum length of 255 characters. String variables are identified by a dollar sign suffix: A$, NAME$, GREETING$. The empty string "" has length zero and occupies no space in the string heap.

String constants are enclosed in double quotes: "HELLO". To include a double quote character within a string, use CHR$(34): `A$ = "He said " + CHR$(34) + "hello" + CHR$(34)`.

## 2. ASSIGNMENT AND COMPARISON

String assignment uses the LET statement (implicit or explicit): `A$ = "HELLO"` or `LET A$ = "HELLO"`. String comparison uses the standard relational operators and is lexicographic (dictionary order), case-sensitive. "ABC" < "ABD" is true. "abc" > "ABC" is true because lowercase letters have higher ASCII codes.

Comparing a string to a number produces Error 13 (Type mismatch). Use VAL() to convert a string to a number, or STR$() to convert a number to a string.

## 3. CONCATENATION

The + operator joins two strings: `C$ = A$ + B$`. Multiple concatenations chain left-to-right: `D$ = A$ + " " + B$ + "!"`. Each + operation creates a new string in the string heap.

If the result would exceed 255 characters, Error 15 (String too long) is raised.

## 4. CORE STRING FUNCTIONS

LEN(s$) returns the length of the string. LEN("") returns 0. LEN("ABC") returns 3.

LEFT$(s$, n) returns the first n characters. LEFT$("HELLO", 3) returns "HEL".

RIGHT$(s$, n) returns the last n characters. RIGHT$("HELLO", 3) returns "LLO".

MID$(s$, start, length) returns length characters starting at position start (1-based). MID$("HELLO", 2, 3) returns "ELL". If length is omitted, returns from start to end: MID$("HELLO", 3) returns "LLO".

INSTR(s$, find$) returns the position of find$ within s$, or 0 if not found. INSTR("HELLO WORLD", "WORLD") returns 7. INSTR(start, s$, find$) begins the search at position start.

## 5. CHARACTER AND CODE FUNCTIONS

CHR$(n) returns the character with ASCII code n (0-255). Common values: CHR$(10) = line feed, CHR$(13) = carriage return, CHR$(32) = space, CHR$(65) = "A".

ASC(s$) returns the ASCII code of the first character. ASC("A") returns 65. ASC("") raises Error 5.

## 6. NUMERIC CONVERSION

STR$(n) converts a number to its string representation. STR$(42) returns " 42" (positive numbers have a leading space). STR$(-7) returns "-7".

VAL(s$) converts a string to a number. VAL("42") returns 42. VAL("3.14") returns 3.14. VAL("ABC") returns 0. VAL("  42XYZ") returns 42 (leading spaces ignored, parsing stops at first non-numeric character).

## 7. BASE CONVERSION

HEX$(n) converts to hexadecimal: HEX$(255) returns "FF". OCT$(n) converts to octal: OCT$(255) returns "377". BIN$(n) converts to binary: BIN$(255) returns "11111111".

## 8. STRING GENERATION

SPACE$(n) creates a string of n spaces. STRING$(n, code) creates a string of n copies of the character with the given ASCII code. STRING$(5, 65) returns "AAAAA". STRING$(n, s$) uses the first character of s$: STRING$(5, "*") returns "*****".

## 9. CASE AND TRIMMING

UCASE$(s$) converts to uppercase. LCASE$(s$) converts to lowercase. TCASE$(s$) converts to title case. LTRIM$(s$) removes leading spaces. RTRIM$(s$) removes trailing spaces. TRIM$(s$) removes both leading and trailing spaces.

## 10. PRINT USING

The PRINT USING statement formats output according to a template string:

```basic
10 PRINT USING "###.##"; 3.14159
20 PRINT USING "$$###.##"; 1234.5
30 PRINT USING "\\    \\"; "HELLO"
```

Numeric format characters: # (digit placeholder), . (decimal point), + (sign display), - (trailing minus), $$ (floating dollar sign), ** (leading asterisks), ^^ (scientific notation).

String format characters: ! (first character only), \\ (fixed-width field), & (variable-width field).

The USING format engine is implemented in engine/src/runtime/print_using.c and engine/src/runtime/using_engine.c.

## 11. MID$ ASSIGNMENT

MID$(target$, start, length) = source$ replaces characters within target$ in place:

```basic
10 A$ = "HELLO WORLD"
20 MID$(A$, 7, 5) = "BASIC"
30 PRINT A$
```

Output: `HELLO BASIC`

The target string's length does not change. If source$ is shorter than length, only len(source$) characters are replaced. If source$ is longer, excess characters are ignored.

## 12. INPUT$ FUNCTION

INPUT$(n) reads exactly n characters from the keyboard without echoing them and without waiting for Enter. INPUT$(n, #channel) reads n characters from a file channel.

## 13. INKEY$ FUNCTION

INKEY$ reads a single keypress without waiting. If no key has been pressed, it returns an empty string. Standard keys return a single character. Extended keys (function keys, arrow keys) return a two-character string: CHR$(0) followed by the scan code.

```basic
10 K$ = ""
20 WHILE K$ = ""
30   K$ = INKEY$
40 WEND
50 PRINT "You pressed: "; K$
```
