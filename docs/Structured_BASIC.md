<!--
Title:        Structured_BASIC
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/, engine/src/statements/loops/,
              engine/src/statements/core/program/, engine/src/vm/,
              engine/src/statements/event/, engine/src/statements/extended/
Generated:    no, hand-written
Status:       current
-->

# Structured BASIC

Block-structured control flow, and where BASIC++ sits against the
international standards for the language.

---

## 1. The control-flow model

BASIC++ keeps line numbers, `GOTO` and `GOSUB` working, because Rule #1 says
a GW-BASIC or QBASIC program must run unchanged. Structured constructs are
layered on top rather than replacing them.

Control constructs are typed frame records on the VM execution stack
(`engine/src/vm/`). A mismatched structure — a `WHILE` closed by a `NEXT` —
produces a specific diagnostic rather than corrupting the stack, which is the
practical difference between a structured BASIC and a BASIC with structured
keywords bolted on.

Line numbers are an **interpreter** constraint. The compiler and the
transpiler do not require them, and a program written entirely in labelled,
structured form compiles. See `Compiler_Transpiler_Tool_Targets`.

---

## 2. Selection

### Block `IF`

```basic
IF condition THEN
  statements
ELSEIF condition2 THEN
  statements
ELSE
  statements
END IF
```

Single-line `IF ... THEN ... ELSE` works alongside the block form, and the
postfix form (`IF_POSTFIX` in the registry) allows a trailing condition.

### `SELECT CASE`

```basic
SELECT CASE test_expression
  CASE value1, value2
    statements
  CASE value3 TO value4
    statements
  CASE IS > threshold
    statements
  CASE ELSE
    statements
END SELECT
```

### `UNLESS` and `WHEN`

`UNLESS` is a negated `IF` and `WHEN` a guard clause; both are registered
statements with their own handlers in `engine/src/statements/control/branch/`.

---

## 3. Iteration

### `FOR ... NEXT`

```basic
FOR counter = start TO limit [STEP step_val]
  statements
  [EXIT FOR]
NEXT [counter]
```

On normal termination `counter` holds `limit + step_val`. That is the vintage
invariant and programs depend on it, so it is preserved exactly.

### `WHILE ... WEND`

```basic
WHILE condition
  statements
  [EXIT WHILE]
WEND
```

### `DO ... LOOP`

Pre-test and post-test, with `WHILE` and `UNTIL`:

```basic
DO WHILE condition          DO
  statements                  statements
LOOP                        LOOP WHILE condition

DO UNTIL condition          DO
  statements                  statements
LOOP                        LOOP UNTIL condition
```

### `REPEAT ... UNTIL`

```basic
REPEAT
  statements
UNTIL condition
```

`EXIT` leaves the innermost loop of any kind; `CONTINUE` starts the next
iteration; `RETRY` re-runs the current one; `ENDLOOP` closes a `LOOP` opened
without a `DO`.

---

## 4. Procedures and scope

`SUB`, `FUNCTION`, `DECLARE`, `PARAM`, `STATIC`, `SHARED`, `GLOBAL`, `PUBLIC`,
`CONST` and `LOCAL` scoping are all registered statements. `SCOPE` reports the
current scope chain at runtime. `TYPE`, `CLASS`, `ENUM` and `WITH` give
structured data; see `User_Defined_Types` and `Set_Based_Object_Model`.

---

## 5. Structured exception handling

This is the part most BASIC dialects do not have, and it comes from the ANSI
standard rather than from Microsoft.

| Statement | Origin | Purpose |
|---|---|---|
| `TRY` | modern | Guarded block |
| `HANDLER` | ANSI Full BASIC | Names an exception handler |
| `CAUSE` | ANSI Full BASIC | Raises an exception |
| `WHENEVER` | IBM VS BASIC / CMS | Conditional event trap: `WHENEVER {ERROR \| [NOT] EOF #channel} THEN {GOTO line \| statement}` |
| `RETRY` | ANSI Full BASIC | Re-executes the statement that raised |
| `ON ERROR GOTO` | GW-BASIC | The vintage form, still supported |
| `RESUME` | GW-BASIC | Vintage resumption |
| `RAISE`, `RAISESIGNAL` | modern | Explicit signalling |

The descriptor for `WHENEVER` records its lineage as IBM VS BASIC and CMS
explicitly. Having both `ON ERROR GOTO` and `HANDLER`/`CAUSE`/`RETRY` in one
language is unusual and is deliberate: the vintage form for vintage programs,
the structured form for new ones.

---

## 6. Where BASIC++ sits against the standards

The question this section answers: **is BASIC++ Structured BASIC in the sense
the standards bodies mean, and which standards apply?**

### The standards that exist

| Standard | Body | Scope |
|---|---|---|
| ANSI X3.60-1978 | ANSI | Minimal BASIC |
| ECMA-55 (1978) | ECMA, Geneva | Minimal BASIC — the European twin of X3.60 |
| ISO 6373:1984 | ISO | Minimal BASIC |
| ANSI X3.113-1987 | ANSI | **Full BASIC** — the large one |
| ECMA-116 (1986) | ECMA | Full BASIC — the European twin of X3.113 |
| ISO/IEC 10279:1991 | ISO/IEC | Full BASIC, with the 1990 modules amendment |
| IEEE 754 | IEEE | Binary floating point, which BASIC++ arithmetic uses |
| ISO/IEC 25010 | ISO/IEC | Software product quality — the model this project's file and function size gates come from |

There is no separate European Union BASIC standard. ECMA is a European
standards body and ECMA-55 and ECMA-116 are the European texts; ISO adopted
them as 6373 and 10279. When someone says "the EU standard for BASIC" they
mean ECMA-116 or its ISO adoption.

### What BASIC++ implements from Full BASIC

More than most. These are registered statements, each verified in the source:

| Feature | Statement | Standard |
|---|---|---|
| Matrix arithmetic | `MAT`, `MAT INPUT`, `MAT PRINT`, `MAT READ`, `MAT WRITE` | X3.113 / ECMA-116 |
| Environment enquiry | `ASK` — the descriptor names ECMA-116 explicitly | ECMA-116 |
| Environment setting | `SET` | X3.113 / ECMA-116 |
| Collating sequence | `COLLATE STANDARD \| NATIVE \| table$` — the descriptor names X3.113-1987 section 9 and ECMA-116 explicitly | X3.113 / ECMA-116 |
| Structured exceptions | `HANDLER`, `CAUSE`, `RETRY` | X3.113 / ECMA-116 |
| Line input | `LINE INPUT`, `LINPUT` | X3.113 |
| Print zones and margins | `ZONE`, `MARGIN`, `NOMARGIN`, `PAGE`, `NOPAGE` | X3.113 |
| Formatted output templates | `IMAGE`, `PRINT USING` | X3.113 |
| Record files | `RECORD`, `SEEK`, `ORGANIZATION` clauses on `OPEN` | X3.113 |
| Angle mode | `OPTION ANGLE DEGREES \| RADIANS` | X3.113 / ECMA-116 |
| Arithmetic mode | `OPTION ARITHMETIC DECIMAL \| NATIVE` | X3.113 / ECMA-116 |
| Modules | `MODULE`, `IMPORT`, `PUBLIC` | ISO/IEC 10279 amendment 1 |

The trigonometric mode statements `DEGREE`, `RADIAN` and `GRAD` are a separate
lineage — the descriptors record them as Sharp PC-1211/PC-1500 and Casio
pocket BASIC — and coexist with `OPTION ANGLE`. Vintage programs use the bare
keywords; standard-conforming programs use `OPTION ANGLE`.

### The honest answer

BASIC++ is **not a conforming implementation** of ANSI X3.113 or ISO/IEC
10279, and does not claim to be. Conformance requires passing the standard's
own test suite and, more restrictively, rejecting what the standard forbids —
and BASIC++ deliberately accepts a great deal that Full BASIC forbids, because
Rule #1 requires GW-BASIC and QBASIC programs to run. A language cannot both
reject `PEEK` as non-standard and run a 1983 program that uses it.

What is true, and worth stating in those terms: BASIC++ implements a
substantially larger subset of Full BASIC than any Microsoft-lineage BASIC
does, including the matrix, collation, environment-enquiry and structured
exception features that were dropped everywhere else. A program written to the
Full BASIC subset will run here. The reverse does not hold.

For the quality standard the project does hold itself to — file and function
size limits, module boundaries, documentation parity — see `PROJECT_RULES.md`
and `ISO_EU_Quality_Audit_And_Regression`.

---

## 7. Worked example

```basic
10 REM Structured selection and looping, no line-number branching
20 DO
30   INPUT "Enter grade (0-100, -1 to quit): ", Grade
40   IF Grade < 0 THEN EXIT DO
50   SELECT CASE Grade
60     CASE 90 TO 100 : PRINT "Grade: A"
70     CASE 80 TO 89  : PRINT "Grade: B"
80     CASE 70 TO 79  : PRINT "Grade: C"
90     CASE ELSE      : PRINT "Grade: F"
100  END SELECT
110 LOOP
120 PRINT "Done."
```

The same program with structured exception handling:

```basic
10 TRY
20   OPEN "grades.dat" FOR INPUT AS #1
30   WHENEVER EOF #1 THEN EXIT DO
40   DO
50     LINE INPUT #1, Line$
60     PRINT Line$
70   LOOP
80 HANDLER IOError
90   PRINT "Could not read grades.dat: "; ERR
100 END TRY
```

---

## See also

- `Arrays_And_Matrices` for the `MAT` family in detail
- `Error_Handling` for the exception model
- `Options_Reference` for `OPTION BASE`, `OPTION ANGLE`, `OPTION COLLATE`,
  `OPTION ARITHMETIC`
- `Dialect_Migration_Reference` for what came from which dialect
- `Program_Modernization_Tutorial` for converting a line-numbered program
- `ISO_EU_Quality_Audit_And_Regression` for the ISO/IEC 25010 gates
