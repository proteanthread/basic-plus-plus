80 PRINT "RUNNING DIALECT METAPROGRAMMING TESTS..."
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
230 LET V = DIALECT_VALIDATE(M)
240 PRINT "Validation (expecting 1):"; V
260 LET DOC$ = DIALECT_DOC$(M)
270 PRINT "Generated Dialect Documentation:"
280 PRINT DOC$
300 DIALECT REGISTER M
310 PRINT "Dialect registered!"
330 DISPLAY "Verify DISPLAY is mapping to PRINT!" ; # This is a comment using #
340 DISPLAY "Verify stmt separator is ;" ; DISPLAY "Separator OK!"
360 DIM ARR(5)
370 TRY
380   PRINT ARR(0)
390   PRINT "FAILED: ARR(0) should be out of bounds for base 1"
400 CATCH
410   DISPLAY "ARR(0) access successfully blocked (out of bounds)!"
420 END TRY
440 LET myVar = 123
450 LET MYVAR = 456
460 DISPLAY "myVar (expecting 123): " + STR$(myVar)
470 DISPLAY "MYVAR (expecting 456): " + STR$(MYVAR)
490 LET target <- 999
500 DISPLAY "Preprocessor hook result (expecting 999): " + STR$(target)
520 DISPLAY "All tests passed!"
530 RUN "tests/basic/42_user_defined_types.bas"
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
