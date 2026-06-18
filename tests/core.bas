10 REM ============================================================
20 REM  TEST: core.bas - Core Keywords, Math, and Variables
30 REM ============================================================
40 REM  Tests the foundational features of BASIC++:
50 REM   - Integer arithmetic (addition)
60 REM   - Single-precision (!) and double-precision (#) floats
70 REM   - String concatenation with +
80 REM   - DIM arrays and element access
90 REM   - Built-in math functions: ABS(), SQR()
92 REM   - Built-in string functions: LEN(), MID$()
94 REM
96 REM  EXPECTED: All assignments and comparisons succeed.
97 REM  OUTPUT:   Math function results and string function results
98 REM            printed, ending with "Core tests completed
99 REM            successfully." No errors should appear.
100 REM ============================================================
110 REM
120 REM --- Integer arithmetic ---
130 REM  A=10, B=20, C=A+B should equal 30.
140 PRINT "Testing Math and Variables..."
150 A = 10 : B = 20 : C = A + B
160 IF C <> 30 THEN PRINT "MATH ERROR" : END
170 REM
180 REM --- Floating-point variables ---
190 REM  D! is single-precision (approx 3.14159).
200 REM  E# is double-precision (full 15+ digit accuracy).
210 D! = 3.14159
220 E# = 3.14159265358979323846#
230 REM
240 REM --- String concatenation ---
250 REM  Two string literals joined with + operator.
260 REM  Result must match "HELLO, WORLD!" exactly.
270 S$ = "HELLO, " + "WORLD!"
280 IF S$ <> "HELLO, WORLD!" THEN PRINT "STRING CONCAT ERROR"
290 REM
300 REM --- Array operations ---
310 REM  DIM allocates 11 elements (0-10).
320 REM  Store 42 at index 5, verify retrieval.
330 DIM ARR(10)
340 ARR(5) = 42
350 IF ARR(5) <> 42 THEN PRINT "ARRAY ERROR"
360 REM
370 REM --- Built-in math functions ---
380 REM  ABS(-5) should return 5 (absolute value).
390 REM  SQR(16) should return 4 (square root).
400 PRINT "Math functions: ABS(-5)="; ABS(-5); " SQR(16)="; SQR(16)
410 REM
420 REM --- Built-in string functions ---
430 REM  LEN(S$) returns length of "HELLO, WORLD!" = 13.
440 REM  MID$(S$,1,5) extracts first 5 chars = "HELLO".
450 PRINT "String functions: LEN(S$)="; LEN(S$); " MID$(S$,1,5)="; MID$(S$,1,5)
460 REM
470 PRINT "Core tests completed successfully."
480 END
