THE BASIC++ STRING HANDLING SYSTEM
=====================================
Version 4.1.1

This manual explains how BASIC++ programs create,
manipulate, compare, and convert strings — from basic
concatenation and substrings to format conversion, the
string pool memory model, and binary packing functions.

Unlike a quick reference, this manual gives you deep
understanding.  By the time you finish reading it, you will
know:

  - How strings are represented internally (BValue)
  - How the string pool manages memory
  - How all string functions work in detail
  - How string concatenation and comparison work
  - How type conversions between strings and numbers work
  - How MID$ works as both function and statement
  - How binary packing (MKI$/CVI etc.) works
  - How to build practical string-processing programs
  - How string variables (A$-Z$) and named string vars work


TABLE OF CONTENTS
=================

  Part I:   String Fundamentals
  1.  What Is a String in BASIC++?
  2.  The BValue String Representation
  3.  The String Pool (Bump Allocator)
  4.  String Variables: A$-Z$ and Named
  5.  String Literals and Escape Characters
  6.  Empty Strings and NULL Data

  Part II:  String Operators
  7.  Concatenation (+)
  8.  String Comparison (=, <, >, <=, >=, <>)
  9.  Comparison Rules: Lexicographic Ordering
  10. Mixed Type Errors (String + Number)

  Part III: Extraction Functions
  11. LEN(s$) — String Length
  12. LEFT$(s$, n) — Left Substring
  13. RIGHT$(s$, n) — Right Substring
  14. MID$(s$, start, len) — Middle Substring
  15. MID$ Statement — In-Place Replacement
  16. Combining Extraction Functions

  Part IV:  Conversion Functions
  17. ASC(s$) — Character to ASCII Code
  18. CHR$(n) — ASCII Code to Character
  19. VAL(s$) — String to Number
  20. STR$(n) — Number to String
  21. Conversion Round-Trips

  Part V:   Transformation Functions
  22. LCASE$(s$) — Convert to Lowercase
  23. UCASE$(s$) — Convert to Uppercase
  24. LTRIM$(s$) — Trim Leading Spaces
  25. RTRIM$(s$) — Trim Trailing Spaces
  26. Combining Trim Functions

  Part VI:  Utility Functions
  27. STRING$(n, char) — Repeat Character
  28. SPACE$(n) — Generate Spaces
  29. DATE$ — Current Date
  30. TIME$ — Current Time
  31. INKEY$ — Non-Blocking Key Read
  32. INPUT$(n) — Read N Characters
  33. SHELL$(cmd$) — Capture Command Output
  34. IOCTL$(#n) — Channel Status

  Part VII: Binary Packing Functions
  35. MKI$(n) — Pack Integer (2 bytes)
  36. MKS$(n) — Pack Single (4 bytes)
  37. MKD$(n) — Pack Double (8 bytes)
  38. CVI(s$) — Unpack Integer
  39. CVS(s$) — Unpack Single
  40. CVD(s$) — Unpack Double
  41. FIELD/LSET/RSET — Random-Access Fields
  42. Binary File Patterns

  Part VIII: String Variables in Depth
  43. Single-Letter String Variables (A$-Z$)
  44. Named String Variables
  45. DIM String Arrays
  46. String Variables in SUB/FUNCTION
  47. SWAP — Exchange Variables

  Part IX:  Internal Architecture
  48. BValue Tagged Union
  49. String Pool Design
  50. Pool Allocation (strpool_alloc)
  51. Pool Storage (strpool_store)
  52. Pool Reset
  53. Capacity Limits

  Part X:   Complete Examples
  54. Example: String Tokenizer
  55. Example: CSV Parser
  56. Example: Text Formatter
  57. Example: Caesar Cipher
  58. Example: Pattern Matching

  Part XI:  Reference
  59. All String Functions
  60. All String Statements
  61. ASCII Code Table (Partial)
  62. Error Codes for Strings
  63. Limits Table


======================================================================
Part I:  STRING FUNDAMENTALS
======================================================================


1.  WHAT IS A STRING IN BASIC++?
-----------------------------------

A string is a sequence of characters (bytes) with a
known length.  Strings in BASIC++ are:

  - Length-counted (not null-terminated)
  - Stored in a pool (bump allocator)
  - Immutable once created (new operations create
    new strings)
  - Maximum length limited by pool size (32K default)

Examples:

  A$ = "Hello, World!"       : REM 13 characters
  B$ = ""                     : REM empty string (len 0)
  C$ = CHR$(65)               : REM "A" (1 character)
  D$ = STRING$(40, "-")       : REM 40 dashes


2.  THE BVALUE STRING REPRESENTATION
----------------------------------------

Internally, every string is a BValue with type =
VAL_STRING:

  typedef struct BValue {
      ValueType type;       /* VAL_STRING */
      union {
          struct {
              char *data;   /* pointer into string pool */
              int   length; /* length in bytes */
          } sval;
      } v;
  } BValue;

Key properties:

  - data points into the string pool (borrowed pointer)
  - length is the number of bytes (not null-terminated)
  - data can be NULL for empty strings
  - BValue does NOT own the memory — the pool does


3.  THE STRING POOL (BUMP ALLOCATOR)
---------------------------------------

All string data lives in a single contiguous memory
block called the string pool:

  typedef struct StringPool {
      char *base;     /* start of allocated block */
      long  size;     /* total size (32768 default) */
      long  used;     /* current watermark */
  } StringPool;

How it works:

  1. At startup, malloc(MAX_STRING_POOL) allocates the
     block (default: 32,768 bytes)

  2. Each string allocation bumps the 'used' pointer:

       before: [################...............]
                ^              ^                ^
                base           used             size

       after:  [################NEWSTRING......]
                ^                       ^       ^
                base                    used    size

  3. Strings are NEVER individually freed

  4. The entire pool is reset at the start of each RUN:
     used = 0

This design is fast (O(1) allocation), simple, and
avoids garbage collection.  The tradeoff is that long-
running programs that create many strings will eventually
exhaust the pool (ERR_SORRY).


4.  STRING VARIABLES: A$-Z$ AND NAMED
-----------------------------------------

BASIC++ provides two kinds of string variables:

  Single-letter:  A$ through Z$ (26 variables)
    Stored in rt->string_vars[0..25]
    Saved/restored during SUB calls

  Named:  any multi-character name ending in $
    Stored in the named variable table
    NOT saved/restored during SUB calls (global)

Examples:

  A$ = "Hello"              : REM single-letter
  Name$ = "Alice"            : REM named variable
  FullPath$ = "C:\DATA"      : REM named variable


5.  STRING LITERALS AND ESCAPE CHARACTERS
--------------------------------------------

String literals are enclosed in double quotes:

  PRINT "Hello, World!"
  A$ = "BASIC++"
  B$ = "She said ""Hi"""     : REM embedded quote

BASIC++ does not have C-style escape sequences (\n, \t)
inside strings.  Instead, use CHR$() for special chars:

  NL$ = CHR$(13) + CHR$(10)  : REM CR+LF (newline)
  TAB$ = CHR$(9)              : REM Tab
  ESC$ = CHR$(27)             : REM Escape
  BELL$ = CHR$(7)             : REM Bell
  QUOTE$ = CHR$(34)           : REM Double quote


6.  EMPTY STRINGS AND NULL DATA
----------------------------------

An empty string has length = 0 and data = NULL:

  A$ = ""
  PRINT LEN(A$)       : REM Output: 0

All string functions handle NULL data safely:

  PRINT LEFT$("", 5)  : REM Output: (empty)
  PRINT MID$("", 1, 3): REM Output: (empty)

To test for empty strings:

  IF LEN(A$) = 0 THEN PRINT "Empty!"
  IF A$ = "" THEN PRINT "Also empty!"


======================================================================
Part II:  STRING OPERATORS
======================================================================


7.  CONCATENATION (+)
-----------------------

The + operator joins two strings:

  100 A$ = "Hello"
  110 B$ = ", World!"
  120 C$ = A$ + B$
  130 PRINT C$          : REM Output: Hello, World!

Multiple concatenations:

  100 FULL$ = FIRST$ + " " + MIDDLE$ + " " + LAST$
  110 PRINT FULL$

Concatenation creates a new string in the pool.  The
original strings remain unchanged.

Under the hood, bval_concat():

  1. Validates both operands are strings
  2. Calculates total = a.length + b.length
  3. Allocates 'total' bytes from the pool
  4. Copies a.data then b.data into the new buffer
  5. Returns bval_string(ptr, total)

Type mismatch: concatenating a string with a number
raises ERR_WHAT:

  PRINT "Value: " + 42      : REM ERROR!
  PRINT "Value: " + STR$(42): REM Correct


8.  STRING COMPARISON (=, <, >, <=, >=, <>)
----------------------------------------------

Strings can be compared with all relational operators:

  100 IF A$ = "YES" THEN PRINT "Confirmed"
  110 IF A$ <> "" THEN PRINT "Not empty"
  120 IF A$ < B$ THEN PRINT "A before B"
  130 IF A$ >= "M" THEN PRINT "Second half"


9.  COMPARISON RULES: LEXICOGRAPHIC ORDERING
------------------------------------------------

String comparison is:

  - Case-sensitive ("A" <> "a")
  - Byte-by-byte from left to right
  - Shorter strings are "less than" longer strings
    with the same prefix

The comparison uses memcmp() for efficiency:

  1. Compare the first min(len_a, len_b) bytes
  2. If all bytes match, the shorter string is "less"
  3. If lengths are equal and all bytes match, equal

Examples:

  "ABC" = "ABC"      → True  (same length, same bytes)
  "ABC" < "ABD"      → True  (C < D at position 3)
  "ABC" < "ABCD"     → True  (prefix match, shorter)
  "abc" > "ABC"      → True  (lowercase > uppercase)
  "A" < "a"          → True  (65 < 97 in ASCII)
  "" < "A"           → True  (empty < anything)

For case-insensitive comparison, use UCASE$ or LCASE$:

  IF UCASE$(A$) = "YES" THEN PRINT "Confirmed"


10.  MIXED TYPE ERRORS (STRING + NUMBER)
-------------------------------------------

Arithmetic with strings raises ERR_WHAT:

  X = "Hello" + 5     : REM ERROR: type mismatch
  X = "3" * 2          : REM ERROR: type mismatch
  IF "5" > 3 THEN      : REM ERROR: mixed comparison

The correct approach is explicit conversion:

  X = VAL("3") * 2     : REM OK: 6
  IF VAL(A$) > 3 THEN  : REM OK: numeric comparison
  S$ = "Count: " + STR$(N%)  : REM OK: string concat


======================================================================
Part III:  EXTRACTION FUNCTIONS
======================================================================


11.  LEN(s$) — STRING LENGTH
-------------------------------

  Syntax: n = LEN(string$)
  Returns: Integer length in characters

  100 A$ = "Hello"
  110 PRINT LEN(A$)          : REM Output: 5
  120 PRINT LEN("")           : REM Output: 0
  130 PRINT LEN("BASIC++")   : REM Output: 7

LEN reads the .length field of the BValue directly.
No counting is needed (O(1) operation).

LEN on a non-string raises ERR_WHAT.


12.  LEFT$(s$, n) — LEFT SUBSTRING
--------------------------------------

  Syntax: result$ = LEFT$(string$, count)
  Returns: First 'count' characters

  100 A$ = "Hello, World!"
  110 PRINT LEFT$(A$, 5)     : REM Output: Hello
  120 PRINT LEFT$(A$, 1)     : REM Output: H
  130 PRINT LEFT$(A$, 100)   : REM Output: Hello, World!

If count > length, returns the entire string.
If count < 0, count is clamped to 0.
If count = 0, returns empty string.


13.  RIGHT$(s$, n) — RIGHT SUBSTRING
----------------------------------------

  Syntax: result$ = RIGHT$(string$, count)
  Returns: Last 'count' characters

  100 A$ = "Hello, World!"
  110 PRINT RIGHT$(A$, 6)    : REM Output: orld!
  120 PRINT RIGHT$(A$, 1)    : REM Output: !
  130 PRINT RIGHT$(A$, 100)  : REM Output: Hello, World!

RIGHT$ calculates start = length - count, then copies
'count' bytes from start.


14.  MID$(s$, start, len) — MIDDLE SUBSTRING
------------------------------------------------

  Syntax: result$ = MID$(string$, start [, length])
  Returns: Substring starting at position 'start'

  100 A$ = "Hello, World!"
  110 PRINT MID$(A$, 8, 5)   : REM Output: World
  120 PRINT MID$(A$, 1, 5)   : REM Output: Hello
  130 PRINT MID$(A$, 8)      : REM Output: World!

  Positions are 1-based (first character is position 1).

If start < 1, it is clamped to 1.
If start > length, returns empty string.
If length is omitted, returns from start to end.
If start + length > string length, returns to end.


15.  MID$ STATEMENT — IN-PLACE REPLACEMENT
----------------------------------------------

  Syntax: MID$(var$, start [, length]) = replacement$

MID$ can also be used as a STATEMENT to replace
characters within an existing string variable:

  100 A$ = "Hello, World!"
  110 MID$(A$, 8, 5) = "BASIC"
  120 PRINT A$               : REM Output: Hello, BASIC!

  200 B$ = "XXXXXXXXXXXX"
  210 MID$(B$, 3, 4) = "test"
  220 PRINT B$               : REM Output: XXtestXXXXXX

Rules:

  - Only replaces up to 'length' characters
  - If replacement is shorter, only that many are copied
  - If replacement is longer, excess is ignored
  - The string's total length does NOT change
  - A new string is allocated in the pool with the
    modification applied

This is one of the few functions that serves as both
a function (in expressions) and a statement (on its
own line).


16.  COMBINING EXTRACTION FUNCTIONS
---------------------------------------

  REM -- Extract filename from path --
  100 PATH$ = "C:\USERS\DATA\REPORT.TXT"
  110 FOR I% = LEN(PATH$) TO 1 STEP -1
  120   IF MID$(PATH$, I%, 1) = "\" THEN
  130     FILE$ = RIGHT$(PATH$, LEN(PATH$) - I%)
  140     EXIT FOR
  150   END IF
  160 NEXT I%
  170 PRINT "Filename: "; FILE$
  180 REM Output: Filename: REPORT.TXT

  REM -- Extract extension --
  200 DOT% = 0
  210 FOR I% = LEN(FILE$) TO 1 STEP -1
  220   IF MID$(FILE$, I%, 1) = "." THEN
  230     DOT% = I%
  240     EXIT FOR
  250   END IF
  260 NEXT I%
  270 IF DOT% > 0 THEN
  280   EXT$ = RIGHT$(FILE$, LEN(FILE$) - DOT%)
  290   PRINT "Extension: "; EXT$
  300 END IF
  310 REM Output: Extension: TXT


======================================================================
Part IV:  CONVERSION FUNCTIONS
======================================================================


17.  ASC(s$) — CHARACTER TO ASCII CODE
------------------------------------------

  Syntax: n = ASC(string$)
  Returns: ASCII code of first character (0-255)

  100 PRINT ASC("A")         : REM Output: 65
  110 PRINT ASC("a")         : REM Output: 97
  120 PRINT ASC("0")         : REM Output: 48
  130 PRINT ASC(" ")         : REM Output: 32

ASC on an empty string raises ERR_WHAT.

Common codes:

  "A"-"Z" → 65-90    "a"-"z" → 97-122
  "0"-"9" → 48-57    " "     → 32
  Enter   → 13       Escape  → 27
  Tab     → 9        Null    → 0


17b. ASC() ARRAY UNPACK — STRING TO NUMERIC ARRAY
-----------------------------------------------------

  Syntax: ArrayName(start) = ASC(string$)

When the target of an ASC() assignment is a DIM'd
1D numeric array, each character of string$ is
unpacked into consecutive array slots starting at
index 'start'.

  100 DIM A(10)
  110 A$ = "TEST"
  120 A(0) = ASC(A$)
  130 REM A(0)=84, A(1)=69, A(2)=83, A(3)=84

This is equivalent to the manual loop:

  FOR I = 0 TO LEN(A$)-1
    A(I) = ASC(MID$(A$, I+1, 1))
  NEXT I

Rules:

  - Only works with 1D arrays (DIM A(n))
  - If the string is longer than remaining array
    slots, excess characters are silently truncated
  - If the string is empty, no slots are modified
  - The starting index can be any valid array position

  Example — Truncation:

    200 DIM B(3)
    210 B$ = "ABCDEFGH"         : REM 8 characters
    220 B(0) = ASC(B$)
    230 REM Only B(0)..B(3) are filled:
    240 REM B(0)=65, B(1)=66, B(2)=67, B(3)=68

  Example — Offset start:

    300 DIM C(5)
    310 C(2) = ASC("XY")
    320 REM C(0)=0, C(1)=0, C(2)=88, C(3)=89

  Example — Named array:

    400 DIM MyArr(20)
    410 MyArr(0) = ASC("Hello")
    420 REM MyArr(0)=72, MyArr(1)=101, ...

This feature is useful for character-by-character
processing without repeated MID$/ASC calls.

  Added in Version 4.1.1.


18.  CHR$(n) — ASCII CODE TO CHARACTER
-----------------------------------------

  Syntax: s$ = CHR$(code)
  Returns: Single-character string

  100 PRINT CHR$(65)         : REM Output: A
  110 PRINT CHR$(97)         : REM Output: a
  120 PRINT CHR$(48)         : REM Output: 0
  130 PRINT CHR$(7)          : REM (bell sound)

Valid range: 0-255.  Values outside raise ERR_HOW.

CHR$ allocates 1 byte from the string pool.

Building strings with CHR$:

  100 REM -- Box-drawing characters --
  110 TL$ = CHR$(218)   : REM ┌
  120 TR$ = CHR$(191)   : REM ┐
  130 BL$ = CHR$(192)   : REM └
  140 BR$ = CHR$(217)   : REM ┘
  150 HZ$ = CHR$(196)   : REM ─
  160 VT$ = CHR$(179)   : REM │


19.  VAL(s$) — STRING TO NUMBER
----------------------------------

  Syntax: n = VAL(string$)
  Returns: Numeric value parsed from string

  100 PRINT VAL("42")        : REM Output: 42
  110 PRINT VAL("3.14")      : REM Output: 3.14
  120 PRINT VAL("-100")      : REM Output: -100
  130 PRINT VAL("abc")       : REM Output: 0

VAL parsing rules:

  - Tries integer first (strtol, base 10)
  - If a decimal point is found, tries float (strtod)
  - Unparseable strings return 0 (no error)
  - Leading whitespace is handled by strtol/strtod

VAL on a non-string raises ERR_WHAT.


20.  STR$(n) — NUMBER TO STRING
-----------------------------------

  Syntax: s$ = STR$(number)
  Returns: String representation of number

  100 PRINT STR$(42)         : REM Output: 42
  110 PRINT STR$(3.14)       : REM Output: 3.14
  120 PRINT STR$(-100)       : REM Output: -100

STR$ formats integers with %ld and floats with %g.

STR$ on a non-numeric value raises ERR_WHAT.

Building display strings:

  100 SCORE% = 9500
  110 MSG$ = "Score: " + STR$(SCORE%)
  120 PRINT MSG$       : REM Output: Score: 9500


21.  CONVERSION ROUND-TRIPS
------------------------------

Convert back and forth between strings and numbers:

  100 N% = 42
  110 S$ = STR$(N%)         : REM "42"
  120 N2% = VAL(S$)         : REM 42
  130 PRINT N% = N2%        : REM Output: -1 (true)

  200 C% = ASC("A")         : REM 65
  210 C$ = CHR$(C%)         : REM "A"
  220 PRINT C$              : REM Output: A

These round-trips are lossless for integers and
single characters.


======================================================================
Part V:  TRANSFORMATION FUNCTIONS
======================================================================


22.  LCASE$(s$) — CONVERT TO LOWERCASE
------------------------------------------

  Syntax: result$ = LCASE$(string$)
  Returns: Lowercase copy of string

  100 PRINT LCASE$("Hello, World!")
  110 REM Output: hello, world!
  120 PRINT LCASE$("BASIC++")
  130 REM Output: basic++

Only ASCII letters A-Z (65-90) are converted.
Non-letter characters are unchanged.

Uses C tolower() on each byte.  Max input: 255 chars.


23.  UCASE$(s$) — CONVERT TO UPPERCASE
------------------------------------------

  Syntax: result$ = UCASE$(string$)
  Returns: Uppercase copy of string

  100 PRINT UCASE$("Hello, World!")
  110 REM Output: HELLO, WORLD!
  120 PRINT UCASE$("basic++")
  130 REM Output: BASIC++

Only ASCII letters a-z (97-122) are converted.

Case-insensitive comparison pattern:

  100 INPUT "Continue? (yes/no): "; R$
  110 IF UCASE$(R$) = "YES" THEN
  120   PRINT "Continuing..."
  130 END IF


24.  LTRIM$(s$) — TRIM LEADING SPACES
-----------------------------------------

  Syntax: result$ = LTRIM$(string$)
  Returns: String with leading spaces removed

  100 A$ = "   Hello"
  110 PRINT "["; LTRIM$(A$); "]"
  120 REM Output: [Hello]

Only ASCII space (32) is trimmed.  Tabs and other
whitespace are NOT trimmed.


25.  RTRIM$(s$) — TRIM TRAILING SPACES
------------------------------------------

  Syntax: result$ = RTRIM$(string$)
  Returns: String with trailing spaces removed

  100 A$ = "Hello   "
  110 PRINT "["; RTRIM$(A$); "]"
  120 REM Output: [Hello]


26.  COMBINING TRIM FUNCTIONS
-------------------------------

Trim both sides:

  100 A$ = "   Hello, World!   "
  110 B$ = LTRIM$(RTRIM$(A$))
  120 PRINT "["; B$; "]"
  130 REM Output: [Hello, World!]

Clean user input:

  100 LINE INPUT "Enter name: "; RAW$
  110 CLEAN$ = LTRIM$(RTRIM$(UCASE$(RAW$)))
  120 PRINT "Name: "; CLEAN$


======================================================================
Part VI:  UTILITY FUNCTIONS
======================================================================


27.  STRING$(n, char) — REPEAT CHARACTER
--------------------------------------------

  Syntax: result$ = STRING$(count, character)

Create a string of repeated characters.  STRING$ is
used extensively for drawing and formatting:

  100 PRINT STRING$(40, "-")     : REM 40 dashes
  110 PRINT STRING$(20, "=")     : REM 20 equals signs
  120 PRINT STRING$(10, CHR$(219))  : REM 10 solid blocks

STRING$ is a built-in keyword (KW_STRING_FUNC).


28.  SPACE$(n) — GENERATE SPACES
------------------------------------

  Syntax: result$ = SPACE$(count)

Create a string of n space characters:

  100 PRINT "Hello" + SPACE$(10) + "World"
  110 REM Output: Hello          World


29.  DATE$ — CURRENT DATE
----------------------------

  Syntax: d$ = DATE$
  Returns: Date string in "MM-DD-YYYY" format

  100 PRINT DATE$
  110 REM Output: 06-08-2026

DATE$ calls time()/localtime() and formats with
%02d-%02d-%04d.  Returns a 10-character string.

No parentheses needed — DATE$ is variable-like.


30.  TIME$ — CURRENT TIME
----------------------------

  Syntax: t$ = TIME$
  Returns: Time string in "HH:MM:SS" format

  100 PRINT TIME$
  110 REM Output: 14:05:33

Returns an 8-character string formatted with
%02d:%02d:%02d.

Timing a computation:

  100 T1$ = TIME$
  110 REM ... computation ...
  120 T2$ = TIME$
  130 PRINT "Started: "; T1$; "  Finished: "; T2$


31.  INKEY$ — NON-BLOCKING KEY READ
---------------------------------------

  Syntax: k$ = INKEY$
  Returns: Empty string or 1-char string

INKEY$ checks the keyboard buffer without waiting:

  100 K$ = INKEY$
  110 IF K$ <> "" THEN PRINT "Key: "; K$

See Screen_And_Console.txt for detailed INKEY$ coverage.


32.  INPUT$(n) — READ N CHARACTERS
--------------------------------------

  Syntax: s$ = INPUT$(count [, #channel])

Read exactly n characters from keyboard or file:

  REM -- From keyboard (no echo) --
  100 PRINT "Press 3 keys: ";
  110 K$ = INPUT$(3)
  120 PRINT "You pressed: "; K$

  REM -- From file --
  200 OPEN "data.bin" FOR INPUT AS #1
  210 HEADER$ = INPUT$(4, #1)
  220 PRINT "Header: "; HEADER$
  230 CLOSE #1

Max count is 255 characters.

From keyboard: reads without echo (raw mode).
From file: reads bytes directly via fgetc().


33.  SHELL$(cmd$) — CAPTURE COMMAND OUTPUT
----------------------------------------------

  Syntax: result$ = SHELL$(command$)
  Returns: stdout of the command as a string

  100 FILES$ = SHELL$("dir /b")
  110 PRINT FILES$

  200 VER$ = SHELL$("ver")
  210 PRINT "OS: "; VER$

SHELL$ uses popen()/pclose() to capture up to 32K of
output.  Trailing newlines are stripped.

The exit code is stored in ERRORLEVEL:

  100 OUTPUT$ = SHELL$("command")
  110 PRINT "Exit code: "; ERRORLEVEL


34.  IOCTL$(#n) — CHANNEL STATUS
------------------------------------

  Syntax: mode$ = IOCTL$(#channel)
  Returns: Single-character file mode string

  "I" = INPUT    "O" = OUTPUT    "A" = APPEND
  "R" = RANDOM   "B" = BINARY

  100 OPEN "test.txt" FOR OUTPUT AS #1
  110 PRINT IOCTL$(#1)        : REM Output: O
  120 CLOSE #1


======================================================================
Part VII:  BINARY PACKING FUNCTIONS
======================================================================


35.  MKI$(n) — PACK INTEGER (2 BYTES)
-----------------------------------------

  Syntax: s$ = MKI$(integer_value)
  Returns: 2-byte string (little-endian)

  100 S$ = MKI$(1000)
  110 PRINT LEN(S$)          : REM Output: 2

MKI$ stores the value as two bytes (low byte first):

  Value 1000 = 0x03E8
  Byte 0: 0xE8 (232)
  Byte 1: 0x03 (3)


36.  MKS$(n) — PACK SINGLE (4 BYTES)
----------------------------------------

  Syntax: s$ = MKS$(float_value)
  Returns: 4-byte string (IEEE 754 single precision)

  100 S$ = MKS$(3.14)
  110 PRINT LEN(S$)          : REM Output: 4


37.  MKD$(n) — PACK DOUBLE (8 BYTES)
----------------------------------------

  Syntax: s$ = MKD$(float_value)
  Returns: 8-byte string (IEEE 754 double precision)

  100 S$ = MKD$(3.14159265358979)
  110 PRINT LEN(S$)          : REM Output: 8


38.  CVI(s$) — UNPACK INTEGER
---------------------------------

  Syntax: n = CVI(string$)
  Returns: Integer from 2-byte string

  100 S$ = MKI$(42)
  110 PRINT CVI(S$)          : REM Output: 42

CVI reads bytes as little-endian:

  value = byte[0] | (byte[1] << 8)

Values > 32767 are sign-extended (two's complement).


39.  CVS(s$) — UNPACK SINGLE
---------------------------------

  Syntax: n = CVS(string$)
  Returns: Float from 4-byte string

  100 S$ = MKS$(3.14)
  110 PRINT CVS(S$)          : REM Output: 3.14


40.  CVD(s$) — UNPACK DOUBLE
---------------------------------

  Syntax: n = CVD(string$)
  Returns: Double from 8-byte string

  100 S$ = MKD$(3.14159265358979)
  110 PRINT CVD(S$)          : REM Output: 3.14159265359


41.  FIELD/LSET/RSET — RANDOM-ACCESS FIELDS
-----------------------------------------------

These statements work with random-access files and
fixed-length string buffers:

  FIELD #n, width AS var$ [, width AS var$ ...]

Define field variables for a random-access file:

  100 OPEN "data.dat" FOR RANDOM AS #1 LEN = 80
  110 FIELD #1, 20 AS Name$, 10 AS Age$, 50 AS Addr$

  LSET var$ = value$  Left-justify into field
  RSET var$ = value$  Right-justify into field

  200 LSET Name$ = "Alice"
  210 LSET Age$ = MKI$(30)
  220 LSET Addr$ = "123 Main St"
  230 PUT #1, 1            : REM Write record #1

See File_IO.txt for detailed FIELD/LSET/RSET coverage.


42.  BINARY FILE PATTERNS
----------------------------

  REM -- Write binary records --
  100 OPEN "records.dat" FOR RANDOM AS #1 LEN = 32
  110 FIELD #1, 20 AS F.Name$, 4 AS F.Score$, 8 AS F.Time$
  120 LSET F.Name$ = "Alice"
  130 LSET F.Score$ = MKS$(95.5)
  140 LSET F.Time$ = MKD$(TIMER)
  150 PUT #1, 1
  160 CLOSE #1

  REM -- Read binary records --
  200 OPEN "records.dat" FOR RANDOM AS #1 LEN = 32
  210 FIELD #1, 20 AS R.Name$, 4 AS R.Score$, 8 AS R.Time$
  220 GET #1, 1
  230 PRINT "Name:  "; R.Name$
  240 PRINT "Score: "; CVS(R.Score$)
  250 PRINT "Time:  "; CVD(R.Time$)
  260 CLOSE #1


======================================================================
Part VIII:  STRING VARIABLES IN DEPTH
======================================================================


43.  SINGLE-LETTER STRING VARIABLES (A$-Z$)
-----------------------------------------------

  A$ = "Hello"
  Z$ = "Goodbye"
  PRINT A$; " and "; Z$

Single-letter string variables use the $ suffix.
They are stored in rt->string_vars[0..25].

During SUB/FUNCTION calls, all 26 string variables
are saved and restored automatically.


44.  NAMED STRING VARIABLES
------------------------------

In dialects with extended variables, multi-character
names ending in $ are string variables:

  FileName$ = "report.txt"
  UserName$ = "Alice"
  ErrorMsg$ = "File not found"

Named string variables are stored in the named variable
table (named_vars[]) and are NOT saved/restored during
SUB calls — they act as global state.


45.  DIM STRING ARRAYS
-------------------------

DIM creates string arrays:

  100 DIM Names$(10)
  110 Names$(1) = "Alice"
  120 Names$(2) = "Bob"
  130 Names$(3) = "Charlie"
  140 FOR I% = 1 TO 3
  150   PRINT Names$(I%)
  160 NEXT I%

2D string arrays:

  100 DIM Grid$(3, 3)
  110 Grid$(1, 1) = "X"
  120 Grid$(1, 2) = "O"
  130 Grid$(2, 2) = "X"


46.  STRING VARIABLES IN SUB/FUNCTION
-----------------------------------------

String parameters in SUB/FUNCTION use $ suffix:

  100 SUB Greet(Name$)
  110   PRINT "Hello, "; Name$; "!"
  120 END SUB
  130 CALL Greet("Alice")

The param_is_string flag in SubDef controls whether
a parameter is bound as a string (A$-Z$) or numeric
(A-Z) variable.

Inside a SUB, changes to A$-Z$ are local (saved/
restored).  Changes to named string variables are global.


47.  SWAP — EXCHANGE VARIABLES
---------------------------------

  Syntax: SWAP A, B

Exchange the values of two single-letter variables:

  100 A = 10 : B = 20
  110 SWAP A, B
  120 PRINT A; B           : REM Output: 20 10

SWAP works with BValues, so it exchanges any type
including strings stored in numeric variable slots.


======================================================================
Part IX:  INTERNAL ARCHITECTURE
======================================================================


48.  BVALUE TAGGED UNION
--------------------------

The BValue type is the universal value container:

  Type Tag       Union Field    Description
  ─────────────  ─────────────  ──────────────────────
  VAL_INTEGER    v.ival (long)  Whole numbers
  VAL_FLOAT      v.fval (dbl)   Floating-point
  VAL_STRING     v.sval.data    Pointer to pool
                 v.sval.length  Byte count

All string operations check the type tag first.  If
the wrong type is passed, ERR_WHAT is raised.


49.  STRING POOL DESIGN
--------------------------

Design goals:

  1. Fast allocation (O(1) bump pointer)
  2. No individual deallocation needed
  3. Simple implementation (< 50 lines of C)
  4. Predictable memory usage
  5. C17 compatible

Tradeoffs:

  - Pool can be exhausted by string-heavy programs
  - No garbage collection (dead strings waste space)
  - Pool is reset only on RUN (not per-statement)
  - Maximum single allocation = pool size


50.  POOL ALLOCATION (STRPOOL_ALLOC)
---------------------------------------

  char *strpool_alloc(StringPool *pool, int len);

  1. Check: used + len <= size
  2. If not: return NULL (caller raises ERR_SORRY)
  3. ptr = base + used
  4. used += len
  5. Return ptr

No alignment padding.  No headers.  Just bump and go.


51.  POOL STORAGE (STRPOOL_STORE)
------------------------------------

  char *strpool_store(StringPool *pool,
                      const char *src, int len);

Convenience function:

  1. ptr = strpool_alloc(pool, len)
  2. If ptr != NULL: memcpy(ptr, src, len)
  3. Return ptr

This is the most common allocation path — used by
LEFT$, RIGHT$, MID$, CHR$, STR$, LCASE$, UCASE$,
LTRIM$, RTRIM$, and all string transformations.


52.  POOL RESET
-----------------

  void strpool_reset(StringPool *pool);

Called at the start of each RUN:

  pool->used = 0;

All previously allocated strings become invalid.  This
is safe because RUN reinitializes all variables.


53.  CAPACITY LIMITS
----------------------

  Constant            Default    Purpose
  ──────────────────  ─────────  ──────────────────────
  MAX_STRING_POOL     32768      Total pool size (bytes)
  MAX_LINE_LENGTH     255        Max single line length
  MAX_STRING_VARS     26         A$-Z$ variables
  MAX_NAMED_VARS      256        Named variables (all)

To increase the pool, change MAX_STRING_POOL in
config.h and recompile.


======================================================================
Part X:  COMPLETE EXAMPLES
======================================================================


54.  EXAMPLE: STRING TOKENIZER
---------------------------------

  100 REM ======================================
  110 REM  Split a string by delimiter
  120 REM ======================================
  130 SUB Tokenize(Text$, Delim$)
  140   P% = 1
  150   N% = 0
  160   DO WHILE P% <= LEN(Text$)
  170     REM Find next delimiter
  180     FOUND% = 0
  190     FOR I% = P% TO LEN(Text$)
  200       IF MID$(Text$, I%, 1) = Delim$ THEN
  210         FOUND% = I%
  220         EXIT FOR
  230       END IF
  240     NEXT I%
  250     REM Extract token
  260     IF FOUND% > 0 THEN
  270       TOKEN$ = MID$(Text$, P%, FOUND% - P%)
  280       P% = FOUND% + 1
  290     ELSE
  300       TOKEN$ = MID$(Text$, P%, LEN(Text$) - P% + 1)
  310       P% = LEN(Text$) + 1
  320     END IF
  330     N% = N% + 1
  340     PRINT "Token "; N%; ": ["; TOKEN$; "]"
  350   LOOP
  360 END SUB
  370 REM
  380 CALL Tokenize("Alice,Bob,Charlie,Dave", ",")

Output:

  Token 1: [Alice]
  Token 2: [Bob]
  Token 3: [Charlie]
  Token 4: [Dave]


55.  EXAMPLE: CSV PARSER
---------------------------

  100 REM ======================================
  110 REM  CSV Line Parser
  120 REM ======================================
  130 SUB ParseCSV(Line$)
  140   COL% = 0
  150   P% = 1
  160   DO WHILE P% <= LEN(Line$)
  170     COL% = COL% + 1
  180     C$ = MID$(Line$, P%, 1)
  190     IF C$ = CHR$(34) THEN
  200       REM Quoted field
  210       P% = P% + 1
  220       FIELD$ = ""
  230       DO WHILE P% <= LEN(Line$)
  240         C$ = MID$(Line$, P%, 1)
  250         IF C$ = CHR$(34) THEN
  260           P% = P% + 1
  270           EXIT DO
  280         END IF
  290         FIELD$ = FIELD$ + C$
  300         P% = P% + 1
  310       LOOP
  320       IF P% <= LEN(Line$) THEN
  330         IF MID$(Line$, P%, 1) = "," THEN P% = P% + 1
  340       END IF
  350     ELSE
  360       REM Unquoted field
  370       FIELD$ = ""
  380       DO WHILE P% <= LEN(Line$)
  390         C$ = MID$(Line$, P%, 1)
  400         IF C$ = "," THEN
  410           P% = P% + 1
  420           EXIT DO
  430         END IF
  440         FIELD$ = FIELD$ + C$
  450         P% = P% + 1
  460       LOOP
  470     END IF
  480     PRINT "  Col "; COL%; ": "; FIELD$
  490   LOOP
  500 END SUB
  510 REM
  520 PRINT "Parsing:"
  530 CALL ParseCSV("Alice,30,Denver")
  540 CALL ParseCSV(CHR$(34) + "Bob, Jr." + CHR$(34) + ",25,Portland")


56.  EXAMPLE: TEXT FORMATTER
------------------------------

  100 REM ======================================
  110 REM  Word-Wrap Text Formatter
  120 REM ======================================
  130 SUB WordWrap(Text$, Width%)
  140   P% = 1
  150   DO WHILE P% <= LEN(Text$)
  160     REM Take next chunk
  170     REMAIN% = LEN(Text$) - P% + 1
  180     IF REMAIN% <= Width% THEN
  190       PRINT MID$(Text$, P%, REMAIN%)
  200       EXIT DO
  210     END IF
  220     REM Find last space in chunk
  230     BREAK% = 0
  240     FOR I% = P% + Width% - 1 TO P% STEP -1
  250       IF MID$(Text$, I%, 1) = " " THEN
  260         BREAK% = I%
  270         EXIT FOR
  280       END IF
  290     NEXT I%
  300     IF BREAK% = 0 THEN BREAK% = P% + Width%
  310     PRINT MID$(Text$, P%, BREAK% - P%)
  320     P% = BREAK% + 1
  330   LOOP
  340 END SUB
  350 REM
  360 T$ = "The quick brown fox jumps over the lazy dog. "
  370 T$ = T$ + "BASIC++ provides powerful string handling "
  380 T$ = T$ + "for text processing applications."
  390 CALL WordWrap(T$, 40)


57.  EXAMPLE: CAESAR CIPHER
------------------------------

  100 REM ======================================
  110 REM  Caesar Cipher (Encrypt/Decrypt)
  120 REM ======================================
  130 FUNCTION CaesarEncrypt$(Text$, Shift%)
  140   R$ = ""
  150   FOR I% = 1 TO LEN(Text$)
  160     C% = ASC(MID$(Text$, I%, 1))
  170     IF C% >= 65 AND C% <= 90 THEN
  180       REM Uppercase letter
  190       C% = ((C% - 65 + Shift%) MOD 26) + 65
  200     ELSEIF C% >= 97 AND C% <= 122 THEN
  210       REM Lowercase letter
  220       C% = ((C% - 97 + Shift%) MOD 26) + 97
  230     END IF
  240     R$ = R$ + CHR$(C%)
  250   NEXT I%
  260   CaesarEncrypt$ = R$
  270 END FUNCTION
  280 REM
  290 PLAIN$ = "Hello, World!"
  300 CIPHER$ = CaesarEncrypt$(PLAIN$, 13)
  310 DECODED$ = CaesarEncrypt$(CIPHER$, 13)
  320 PRINT "Plain:   "; PLAIN$
  330 PRINT "Cipher:  "; CIPHER$
  340 PRINT "Decoded: "; DECODED$


58.  EXAMPLE: PATTERN MATCHING
---------------------------------

  100 REM ======================================
  110 REM  Simple Pattern Matcher
  120 REM  Supports * (any chars) and ? (one char)
  130 REM ======================================
  140 FUNCTION Match%(Text$, Pattern$)
  150   T% = 1 : P% = 1
  160   TL% = LEN(Text$)
  170   PL% = LEN(Pattern$)
  180   DO WHILE P% <= PL%
  190     PC$ = MID$(Pattern$, P%, 1)
  200     IF PC$ = "*" THEN
  210       REM * matches rest of string
  220       P% = P% + 1
  230       IF P% > PL% THEN
  240         Match% = 1  : REM * at end matches all
  250         EXIT FUNCTION
  260       END IF
  270       REM Try matching rest from each position
  280       FOR I% = T% TO TL%
  290         IF Match%(MID$(Text$, I%), MID$(Pattern$, P%)) THEN
  300           Match% = 1
  310           EXIT FUNCTION
  320         END IF
  330       NEXT I%
  340       Match% = 0
  350       EXIT FUNCTION
  360     ELSEIF PC$ = "?" THEN
  370       REM ? matches exactly one character
  380       IF T% > TL% THEN
  390         Match% = 0
  400         EXIT FUNCTION
  410       END IF
  420       T% = T% + 1
  430       P% = P% + 1
  440     ELSE
  450       REM Literal character match
  460       IF T% > TL% THEN
  470         Match% = 0
  480         EXIT FUNCTION
  490       END IF
  500       IF MID$(Text$, T%, 1) <> PC$ THEN
  510         Match% = 0
  520         EXIT FUNCTION
  530       END IF
  540       T% = T% + 1
  550       P% = P% + 1
  560     END IF
  570   LOOP
  580   Match% = (T% > TL%)
  590 END FUNCTION
  600 REM
  610 PRINT Match%("hello.txt", "*.txt")   : REM 1
  620 PRINT Match%("data.csv", "*.txt")    : REM 0
  630 PRINT Match%("file1.bas", "file?.bas"): REM 1


======================================================================
Part XI:  REFERENCE
======================================================================


59.  ALL STRING FUNCTIONS
---------------------------

  Function               Returns   Description
  ─────────────────────  ────────  ──────────────────────
  LEN(s$)                Integer   String length
  LEFT$(s$, n)           String    First n characters
  RIGHT$(s$, n)          String    Last n characters
  MID$(s$, pos, len)     String    Substring at position
  ASC(s$)                Integer   ASCII code of 1st char
  CHR$(n)                String    Character from code
  VAL(s$)                Number    Parse string as number
  STR$(n)                String    Format number as string
  LCASE$(s$)             String    Convert to lowercase
  UCASE$(s$)             String    Convert to uppercase
  LTRIM$(s$)             String    Trim leading spaces
  RTRIM$(s$)             String    Trim trailing spaces
  STRING$(n, c$)         String    Repeat character n times
  SPACE$(n)              String    N space characters
  DATE$                  String    Current date MM-DD-YYYY
  TIME$                  String    Current time HH:MM:SS
  INKEY$                 String    Non-blocking key read
  INPUT$(n [,#c])        String    Read n characters
  SHELL$(cmd$)           String    Capture command output
  IOCTL$(#n)             String    File channel mode
  MKI$(n)                String    Pack integer (2 bytes)
  MKS$(n)                String    Pack single (4 bytes)
  MKD$(n)                String    Pack double (8 bytes)
  CVI(s$)                Integer   Unpack 2-byte integer
  CVS(s$)                Float     Unpack 4-byte single
  CVD(s$)                Double    Unpack 8-byte double
  VARPTR$(var)           String    Variable pointer (stub)


60.  ALL STRING STATEMENTS
----------------------------

  Statement                    Description
  ─────────────────────────────────────────────────────
  A$ = "value"                 String assignment
  A$ = A$ + B$                 Concatenation assignment
  MID$(A$, pos, len) = rep$    In-place replacement
  SWAP A, B                    Exchange two variables
  LSET var$ = value$           Left-justify into field
  RSET var$ = value$           Right-justify into field


61.  ASCII CODE TABLE (PARTIAL)
---------------------------------

  Code  Char    Code  Char    Code  Char    Code  Char
  ────  ──────  ────  ──────  ────  ──────  ────  ──────
  0     NUL     32    (space) 48    0       65    A
  7     BEL     33    !       49    1       66    B
  8     BS      34    "       50    2       90    Z
  9     TAB     35    #       51    3       97    a
  10    LF      36    $       52    4       98    b
  13    CR      37    %       53    5       122   z
  27    ESC     38    &       57    9       127   DEL

  Box-Drawing (Code Page 437):
  ────────────────────────────
  179  │    191  ┐    196  ─    218  ┌
  180  ┤    192  └    205  ═    219  █
  186  ║    193  ┴    217  ┘    220  ▄


62.  ERROR CODES FOR STRINGS
-------------------------------

  Error       When It Occurs
  ──────────  ────────────────────────────────────────
  ERR_WHAT    Type mismatch (arithmetic on strings,
              comparison of string vs number, LEN on
              number, ASC on empty string)
  ERR_HOW     CHR$ code out of range (< 0 or > 255)
  ERR_SORRY   String pool exhausted (too many strings
              allocated without RUN reset)


63.  LIMITS TABLE
-------------------

  Limit                     Value    What Happens
  ────────────────────────  ───────  ──────────────────
  MAX_STRING_POOL           32768    ERR_SORRY
  Max single string         32768    Pool limit
  LCASE$/UCASE$ max input  255      Truncated
  LTRIM$/RTRIM$ max input  255      Truncated
  MKI$ output              2 bytes  Fixed
  MKS$ output              4 bytes  Fixed
  MKD$ output              8 bytes  Fixed
  INPUT$ max count          255      Clamped
  SHELL$ max output         32760    Truncated
  DATE$ output              10 chars Fixed
  TIME$ output              8 chars  Fixed


======================================================================
  END OF TUTORIAL: STRING HANDLING
======================================================================
