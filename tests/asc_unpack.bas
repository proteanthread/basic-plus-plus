10 REM === ASC() Array Unpack Test ===
3 DIALECT "GWBS"
30 DIM A(10)
40 A$ = "HELLO"
50 A(0) = ASC(A$)
60 FOR I = 0 TO 4
70   PRINT A(I);
80 NEXT I
90 PRINT
100 REM Test 2: Truncation (string longer than array)
110 DIM B(3)
120 B$ = "ABCDEFGH"
130 B(0) = ASC(B$)
140 FOR I = 0 TO 3
150   PRINT B(I);
160 NEXT I
170 PRINT
180 REM Test 3: Offset start
190 DIM C(5)
200 C(2) = ASC("XY")
210 PRINT C(0); C(1); C(2); C(3)
220 REM Test 4: Single character (same as normal ASC)
230 DIM D(5)
240 D(0) = ASC("Z")
250 PRINT D(0)
260 REM Test 5: Empty string (no-op)
270 DIM E(5)
280 E(0) = 99
290 E$ = ""
300 E(0) = ASC(E$)
310 PRINT E(0)
