<!--
Title:        Self_Programming
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/func_program.c, engine/src/statements/system/stmt_chain.c, engine/src/statements/introspection/override.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Self-Programming & Runtime Metaprogramming Architecture

The authoritative specification for source line introspection, program overlay merging, keyword redirection, and runtime metaprogramming in BASIC++ v6.5.2.

---

## 1. Overview and Design Principles

BASIC++ programs can inspect their own source code, merge external code overlays, and dynamically intercept keyword execution at runtime. Rather than using unsafe string injection, the BASIC++ runtime provides bounded introspection and metaprogramming primitives:

- **Program Source Introspection**: Programs examine their stored source lines using `PROGRAM$`.
- **Dynamic Program Overlays**: Programs overlay and chain external modules using `MERGE` and `CHAIN`.
- **Execution Metaprogramming**: Programs intercept and modify statement execution using `OVERRIDE`, `ALIAS`, and `SCOPE HOOK`.

---

## 2. Program Source Introspection (`PROGRAM$`)

Implemented in `engine/src/eval/functions/system/environment/func_program.c`:

- **`PROGRAM$(line_number)`**: Returns the exact source text of the stored program line with number `line_number`, including line number and statements. Returns an empty string if the line does not exist.
- **`PROGRAM$("COUNT")`**: Returns the total number of lines currently stored in the active program buffer.

```basic
10 REM Self-Inspection Loop
20 Total = VAL(PROGRAM$("COUNT"))
30 PRINT "Total lines stored: "; Total
40 FOR L = 10 TO 100 STEP 10
50   LineText$ = PROGRAM$(L)
60   IF LineText$ <> "" THEN PRINT "Line "; L; ": "; LineText$
70 NEXT L
```

---

## 3. Dynamic Code Overlays (`MERGE` and `CHAIN`)

- **`MERGE "filename.bas"`**: Merges source lines from a specified external ASCII file into the existing program buffer without clearing variables.
- **`CHAIN [MERGE] "filename.bas" [, line_num]`**: Transfers execution control to a secondary BASIC program overlay, optionally preserving variable state with `COMMON` and merging line buffers.

---

## 4. Statement Hooking & Metaprogramming

- **`OVERRIDE keyword WITH GOSUB line | SUB name`**: Intercepts built-in keyword execution with user-defined procedures.
- **`ALIAS existing_name AS new_name`**: Dynamically creates synonym identifiers for existing keywords.
- **`SCOPE HOOK BEFORE|AFTER cmd GOSUB target`**: Installs pre-execution and post-execution hooks around specific commands.

*(Note: Direct runtime text injection via hypothetical statements like `EXEC "100 DATA..."` is not part of the engine; line buffer management is handled via `MERGE` and `Program_Management.md` lifecycle commands.)*
