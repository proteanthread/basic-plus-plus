10 REM --- BASIC++ Comprehensive Test Suite ---
20 REM Part 1: Core Keywords, Math, and Variables
30 PRINT "Testing Math and Variables..."
40 A = 10 : B = 20 : C = A + B
50 IF C <> 30 THEN PRINT "MATH ERROR" : END
60 D! = 3.14159
70 E# = 3.14159265358979323846#
80 S$ = "HELLO, " + "WORLD!"
90 IF S$ <> "HELLO, WORLD!" THEN PRINT "STRING CONCAT ERROR"
100 DIM ARR(10)
110 ARR(5) = 42
120 IF ARR(5) <> 42 THEN PRINT "ARRAY ERROR"
130 PRINT "Math functions: ABS(-5)="; ABS(-5); " SQR(16)="; SQR(16)
140 PRINT "String functions: LEN(S$)="; LEN(S$); " MID$(S$,1,5)="; MID$(S$,1,5)
150 PRINT "Core tests completed successfully."
160 END
