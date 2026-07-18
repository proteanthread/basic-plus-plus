10 REM =====================================================================
20 REM BASIC++ Advanced String Functions Integration Test
30 REM =====================================================================
40 REM - What can be changed: Test string constants and comparison variables.
50 REM - What cannot be changed: Function names and assertion structures.
60 REM - What to expect: Verification of UCASE$, LCASE$, LTRIM$, RTRIM$,
70 REM   TRIM$, SPACE$, STRING$, and REPLACE$.
80 REM - What to do if something breaks: Check implementations in src/expression/eval.c.
90 REM =====================================================================
100 PRINT "RUNNING ADVANCED STRINGS REGRESSION TESTS..."
110 REM 1. UCASE$
120 A$ = "Hello World 123"
130 B$ = UCASE$(A$)
140 IF B$ <> "HELLO WORLD 123" THEN PRINT "FAILED UCASE$ check": END
150 REM 2. LCASE$
160 C$ = LCASE$(A$)
170 IF C$ <> "hello world 123" THEN PRINT "FAILED LCASE$ check": END
180 REM 3. LTRIM$
190 D$ = "   Trim Me   "
200 E$ = LTRIM$(D$)
210 IF E$ <> "Trim Me   " THEN PRINT "FAILED LTRIM$ check": END
220 REM 4. RTRIM$
230 F$ = RTRIM$(D$)
240 IF F$ <> "   Trim Me" THEN PRINT "FAILED RTRIM$ check": END
250 REM 5. TRIM$
265 G$ = TRIM$(D$)
270 IF G$ <> "Trim Me" THEN PRINT "FAILED TRIM$ check": END
280 REM 6. SPACE$
290 H$ = SPACE$(5)
300 IF H$ <> "     " THEN PRINT "FAILED SPACE$ check": END
310 REM 7. STRING$
320 I$ = STRING$(3, "A")
330 IF I$ <> "AAA" THEN PRINT "FAILED STRING$ (string) check": END
340 J$ = STRING$(4, 66)
350 IF J$ <> "BBBB" THEN PRINT "FAILED STRING$ (ascii) check": END
360 REM 8. REPLACE$
370 K$ = "apple banana apple cherry"
380 L$ = REPLACE$(K$, "apple", "orange")
390 IF L$ <> "orange banana orange cherry" THEN PRINT "FAILED REPLACE$ check": END
400 PRINT "ADVANCED STRINGS OK"
410 RUN "tests/basic/48_dialect_load_file.bas"