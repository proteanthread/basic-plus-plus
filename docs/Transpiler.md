# BASIC++ v6.5.2 Transpiler Reference

## 1. OVERVIEW

The BASIC++ transpiler (trans/trans.exe) converts BASIC++ source programs into other programming languages. The primary target is C17, but the transpiler architecture supports multiple backend modules for different output languages. The transpiler is implemented in the engine/src/compiler/ directory.

## 2. C17 BACKEND

The C17 backend (trans_c17.c) generates clean, readable C17 source code that compiles with any standard C17 compiler:

```bash
trans --c17 program.bas -o program.c
gcc -std=c17 -O2 -o program program.c -lm
```

The generated C code preserves the structure of the original BASIC program as comments:

```c
/* Line 10: FOR I = 1 TO 10 */
for (int bpp_I = 1; bpp_I <= 10; bpp_I++) {
    /* Line 20: PRINT I; I * I */
    printf("%d %d\n", bpp_I, bpp_I * bpp_I);
}
```

## 3. TRANSLATION RULES

The transpiler maps BASIC++ constructs to C17 equivalents:

| BASIC++ | C17 |
|---------|-----|
| LET A = 5 | double bpp_A = 5.0; |
| A$ = "hello" | char* bpp_A_str = str_create("hello"); |
| IF...THEN...ELSE...END IF | if (...) { ... } else { ... } |
| FOR...NEXT | for (...) { ... } |
| WHILE...WEND | while (...) { ... } |
| DO...LOOP | do { ... } while (...); |
| SELECT CASE | switch (...) { case ...: ... } |
| GOSUB/RETURN | function call |
| SUB/END SUB | void function |
| FUNCTION/END FUNCTION | typed function |
| PRINT | printf / runtime print function |
| DIM A(100) | double bpp_A_arr[101]; |
| OPEN/CLOSE | fopen/fclose |

## 4. RUNTIME LIBRARY

The transpiled program requires a small runtime library that provides:

- String management (reference counting, concatenation, substring).
- PRINT formatting (USING engine, zones, TAB, SPC).
- File I/O wrappers (FIELD, GET, PUT, BGET, BPUT).
- Math functions (any not directly available in C's math.h).
- Error handling (ON ERROR GOTO simulation through setjmp/longjmp).
- RND (compatible random number generator).

The runtime library source is generated alongside the program or linked from a pre-compiled library.

## 5. TRANSPILER OPTIONS

```bash
trans [--c17] [--inline-runtime] [--optimize] [--debug] input.bas -o output.c
```

--inline-runtime: Include the runtime library source directly in the output file (single-file output).
--optimize: Enable optimization passes (constant folding, dead code removal).
--debug: Include line number comments and debugging information.

## 6. LIMITATIONS

The transpiler cannot translate features that depend on runtime interpretation:

- EXEC (runtime code generation).
- EDIT, AUTO, RENUM, LIST (interactive editing).
- MODULE LOAD (dynamic module loading).
- EVAL-style expression evaluation from strings.

Programs using these features must remain interpreted.

## 7. MULTI-FILE TRANSPILATION

For programs that use CHAIN or COMMON:

```bash
trans --c17 main.bas module1.bas module2.bas -o program.c
```

The transpiler resolves COMMON variable declarations and generates shared global variables.

## 8. SYNCHRONIZATION WITH THE INTERPRETER

Whenever a new language feature, statement, function, or syntactic rule is added to the BASIC++ interpreter, the transpiler backend modules must be updated to maintain exact semantic parity. A feature is not complete until it transpiles correctly. This is enforced by the project's architectural rules.
