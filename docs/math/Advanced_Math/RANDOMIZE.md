# RANDOMIZE

## 1. Syntax & Parameters
### Statement Syntax:
`RANDOMIZE [expression]`
`RANDOMIZE !?` / `RANDOM`
`RANDOMIZE TIMER`
`RANDOMIZE GET[#] channel`
`RANDOMIZE INPUT[#] channel`
`RANDOMIZE READ [count]`
`RANDOMIZE array[()]`

### Function/Expression Syntax:
`X$ = RANDOMIZE(STRING$ [, len])`
`X$ = RANDOMIZE(X$)`
`X$ = RANDOMIZE(DATE$)`
`X$ = RANDOMIZE(DAY$)`
`X$ = RANDOMIZE(TIME$)`

---

## 2. Description & Usage
Seeds the pseudo-random number generator (RNG) or generates random formatted values.

### Seeding Statements:
- **RANDOMIZE** (no args), **RANDOMIZE !?**, or **RANDOM**: Prompts the user interactively on the console: `Random number seed (-32768 to 32767)? ` and seeds using the parsed value.
- **RANDOMIZE TIMER**: Seeds using the system clock ticks combined with time.
- **RANDOMIZE expression**: Seeds the RNG with the numeric value of the expression.
- **RANDOMIZE GET# channel**: Reads a single byte/character from the file/device channel and seeds using its value.
- **RANDOMIZE INPUT# channel**: Reads a line/token from the file/device channel, hashes it, and seeds.
- **RANDOMIZE READ n**: Reads the next `n` items from the program `DATA` stream, hashes them, and seeds.
- **RANDOMIZE array()**: Shuffles the elements of the specified array in-place without changing dimensions or types.

### Generator Functions:
- **RANDOMIZE("STRING$", len)**: Generates a random alphanumeric string of length `len` (default: 8).
- **RANDOMIZE(X$)**: Shuffles the characters of the string `X$` and returns the shuffled string.
- **RANDOMIZE("DATE$")**: Generates a random date string in `YYYYMMDD` format.
- **RANDOMIZE("DAY$")**: Generates a random day abbreviation (`SUN`-`SAT`).
- **RANDOMIZE("TIME$")**: Generates a random time string in `HH:MM:SS` format.

---

## 3. Code Examples
```basic
10 REM Interactive seeding
20 RANDOMIZE !?

30 REM Entropic clock seed
40 RANDOMIZE TIMER

50 REM Seed from sequential channel
60 OPEN "input.txt" FOR INPUT AS #1
70 RANDOMIZE GET #1
80 CLOSE #1

90 REM Array shuffling
100 DIM A(5)
110 FOR I = 0 TO 5: A(I) = I: NEXT I
120 RANDOMIZE A()

130 REM Random string generator
140 LET S$ = RANDOMIZE("STRING$", 16)
150 PRINT S$
```

---

## 4. Cross-References / See Also
- RND, GUID$
