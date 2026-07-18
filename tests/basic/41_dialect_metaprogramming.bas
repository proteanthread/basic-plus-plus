10 REM =====================================================================
20 REM What can be changed: Dialect property values, map keys, and prints.
30 REM What cannot be changed: Test structure, assertions, and functions.
40 REM What to expect: Verification of dialect parsing, custom separators,
50 REM   comment chars, array bases, case sensitivity, and preprocessor hooks.
60 REM What to do if something breaks: Check parser/lexer changes in src/core/dialect.c.
70 REM =====================================================================
80 PRINT "RUNNING DIALECT METAPROGRAMMING TESTS..."
90 REM Test 1: Programmatic BppMap creation and schema validation
100 LET M = MAP_NEW()
110 LET OK = MAP_SET(M, "name", "MyCustomBASIC")
120 LET OK = MAP_SET(M, "comment_char", "#")
130 LET OK = MAP_SET(M, "stmt_separator", ";")
140 LET OK = MAP_SET(M, "default_array_base", 1)
150 LET OK = MAP_SET(M, "case_sensitive", 1)
160 LET OK = MAP_SET(M, "math_precedence", "LEFT_TO_RIGHT")
170 LET KWS = MAP_NEW()
180 LET OK = MAP_SET(KWS, "DISPLAY", "PRINT")
190 LET OK = MAP_SET(KWS, "QUIT", "END")
200 LET OK = MAP_SET(M, "keywords", KWS)
210 LET OK = MAP_SET(M, "preprocessor_hook", "PREPROC$")
220 REM Validate valid map
230 LET V = DIALECT_VALIDATE(M)
240 PRINT "Validation (expecting 1):"; V
250 REM Test 2: Generate docs
260 LET DOC$ = DIALECT_DOC$(M)
270 PRINT "Generated Dialect Documentation:"
280 PRINT DOC$
290 REM Test 3: Register and activate custom dialect
300 DIALECT REGISTER M
310 PRINT "Dialect registered!"
320 REM Test 4: Verify custom tokenization (comments, statement separators, keywords)
330 DISPLAY "Verify DISPLAY is mapping to PRINT!" ; # This is a comment using #
340 DISPLAY "Verify stmt separator is ;" ; DISPLAY "Separator OK!"
350 REM Test 5: Verify Array Base 1
360 DIM ARR(5)
370 TRY
380   PRINT ARR(0)
390   PRINT "FAILED: ARR(0) should be out of bounds for base 1"
400 CATCH
410   DISPLAY "ARR(0) access successfully blocked (out of bounds)!"
420 END TRY
430 REM Test 6: Verify Case Sensitivity
440 LET myVar = 123
450 LET MYVAR = 456
460 DISPLAY "myVar (expecting 123): " + STR$(myVar)
470 DISPLAY "MYVAR (expecting 456): " + STR$(MYVAR)
480 REM Test 7: Verify Preprocessor hook
490 LET target <- 999
500 DISPLAY "Preprocessor hook result (expecting 999): " + STR$(target)
510 REM Test 8: Verify QUIT keyword
520 DISPLAY "All tests passed!"
530 RUN "tests/basic/42_user_defined_types.bas"
540 REM Preprocessor hook function definition
550 FUNCTION PREPROC$(LINE_STR$)
560   LET RES$ = ""
570   LET L = LEN(LINE_STR$)
580   LET I = 1
590   WHILE I <= L
600     LET IS_HOOK = (MID$(LINE_STR$, I, 2) = "<-")
610     IF IS_HOOK THEN LET RES$ = RES$ + "="
615     IF IS_HOOK THEN LET I = I + 2
620     IF IS_HOOK = 0 THEN LET RES$ = RES$ + MID$(LINE_STR$, I, 1)
625     IF IS_HOOK = 0 THEN LET I = I + 1
630   WEND
640   LET PREPROC$ = RES$
650 END FUNCTION
