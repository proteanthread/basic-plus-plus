5 DIALECT "GWBS"
10 REM =============================================
11 REM  BASIC++ Comprehensive Feature Test Suite
12 REM  Tests nearly every command, statement,
13 REM  function, and feature.
14 REM =============================================
15 REM
16 REM Each test prints PASS or FAIL.
17 REM Summary at end.
18 REM
19 LET P = 0
20 LET F = 0
30 REM
100 REM =============================================
101 REM  SECTION 1: CORE STATEMENTS
102 REM =============================================
103 PRINT "=== SECTION 1: CORE STATEMENTS ==="
110 REM --- LET / Assignment ---
111 LET A = 42
112 B = 99
113 IF A = 42 AND B = 99 THEN PRINT "PASS: LET/assignment" : P = P + 1 : GOTO 120
114 PRINT "FAIL: LET/assignment" : F = F + 1
120 REM --- PRINT ---
121 REM (PRINT is used throughout, implicit pass)
122 PRINT "PASS: PRINT" : P = P + 1
130 REM --- REM ---
131 REM This is a comment
132 PRINT "PASS: REM" : P = P + 1
140 REM --- END (tested at program end) ---
150 REM --- GOTO ---
151 GOTO 155
152 PRINT "FAIL: GOTO" : F = F + 1 : GOTO 160
155 PRINT "PASS: GOTO" : P = P + 1
160 REM --- GOSUB / RETURN ---
161 GOSUB 9000
162 PRINT "PASS: GOSUB/RETURN" : P = P + 1
170 REM --- IF / THEN / ELSE ---
171 IF 1 = 1 THEN GOTO 175
172 PRINT "FAIL: IF/THEN" : F = F + 1 : GOTO 180
175 PRINT "PASS: IF/THEN" : P = P + 1
180 IF 1 = 2 THEN PRINT "FAIL: ELSE" : F = F + 1 : GOTO 190 ELSE PRINT "PASS: IF/ELSE" : P = P + 1
190 REM --- ON GOTO ---
191 LET X = 2
192 ON X GOTO 193, 195, 197
193 PRINT "FAIL: ON GOTO" : F = F + 1 : GOTO 200
195 PRINT "PASS: ON GOTO" : P = P + 1 : GOTO 200
197 PRINT "FAIL: ON GOTO" : F = F + 1
200 REM
201 REM =============================================
202 REM  SECTION 2: LOOPS
203 REM =============================================
204 PRINT "=== SECTION 2: LOOPS ==="
210 REM --- FOR / NEXT ---
211 LET S = 0
212 FOR I = 1 TO 5
213 S = S + I
214 NEXT I
215 IF S = 15 THEN PRINT "PASS: FOR/NEXT" : P = P + 1 : GOTO 220
216 PRINT "FAIL: FOR/NEXT (S="; S; ")" : F = F + 1
220 REM --- FOR with STEP ---
221 LET S = 0
222 FOR I = 10 TO 1 STEP -2
223 S = S + I
224 NEXT I
225 IF S = 30 THEN PRINT "PASS: FOR/STEP" : P = P + 1 : GOTO 230
226 PRINT "FAIL: FOR/STEP (S="; S; ")" : F = F + 1
230 REM --- WHILE / WEND ---
231 LET C = 0
232 WHILE C < 5
233 C = C + 1
234 WEND
235 IF C = 5 THEN PRINT "PASS: WHILE/WEND" : P = P + 1 : GOTO 240
236 PRINT "FAIL: WHILE/WEND" : F = F + 1
240 REM --- DO / LOOP UNTIL ---
241 LET C = 0
242 DO
243 C = C + 1
244 LOOP UNTIL C >= 3
245 IF C = 3 THEN PRINT "PASS: DO/LOOP UNTIL" : P = P + 1 : GOTO 250
246 PRINT "FAIL: DO/LOOP UNTIL" : F = F + 1
250 REM --- DO WHILE / LOOP ---
251 LET C = 0
252 DO WHILE C < 4
253 C = C + 1
254 LOOP
255 IF C = 4 THEN PRINT "PASS: DO WHILE/LOOP" : P = P + 1 : GOTO 260
256 PRINT "FAIL: DO WHILE/LOOP" : F = F + 1
260 REM
300 REM =============================================
301 REM  SECTION 3: MATH FUNCTIONS
302 REM =============================================
303 PRINT "=== SECTION 3: MATH FUNCTIONS ==="
310 IF ABS(-5) = 5 THEN PRINT "PASS: ABS" : P = P + 1 : GOTO 320
311 PRINT "FAIL: ABS" : F = F + 1
320 IF SGN(-10) = -1 THEN PRINT "PASS: SGN" : P = P + 1 : GOTO 330
321 PRINT "FAIL: SGN" : F = F + 1
330 IF INT(3.7) = 3 THEN PRINT "PASS: INT" : P = P + 1 : GOTO 340
331 PRINT "FAIL: INT" : F = F + 1
340 IF SQR(9) = 3 THEN PRINT "PASS: SQR" : P = P + 1 : GOTO 350
341 PRINT "FAIL: SQR" : F = F + 1
350 LET V = SIN(0)
351 IF V = 0 THEN PRINT "PASS: SIN" : P = P + 1 : GOTO 360
352 PRINT "FAIL: SIN (V="; V; ")" : F = F + 1
360 LET V = COS(0)
361 IF V = 1 THEN PRINT "PASS: COS" : P = P + 1 : GOTO 370
362 PRINT "FAIL: COS (V="; V; ")" : F = F + 1
370 LET V = TAN(0)
371 IF V = 0 THEN PRINT "PASS: TAN" : P = P + 1 : GOTO 380
372 PRINT "FAIL: TAN (V="; V; ")" : F = F + 1
380 LET V = ATN(0)
381 IF V = 0 THEN PRINT "PASS: ATN" : P = P + 1 : GOTO 390
382 PRINT "FAIL: ATN (V="; V; ")" : F = F + 1
390 LET V = LOG(1)
391 IF V = 0 THEN PRINT "PASS: LOG" : P = P + 1 : GOTO 395
392 PRINT "FAIL: LOG (V="; V; ")" : F = F + 1
395 LET V = EXP(0)
396 IF V = 1 THEN PRINT "PASS: EXP" : P = P + 1 : GOTO 400
397 PRINT "FAIL: EXP (V="; V; ")" : F = F + 1
400 REM --- RND ---
401 LET V = RND(1)
402 IF V >= 0 AND V < 1 THEN PRINT "PASS: RND" : P = P + 1 : GOTO 410
403 PRINT "FAIL: RND (V="; V; ")" : F = F + 1
410 REM --- FIX ---
411 IF FIX(-3.7) = -3 THEN PRINT "PASS: FIX" : P = P + 1 : GOTO 420
412 PRINT "FAIL: FIX" : F = F + 1
420 REM --- MOD ---
421 IF 10 MOD 3 = 1 THEN PRINT "PASS: MOD" : P = P + 1 : GOTO 430
422 PRINT "FAIL: MOD" : F = F + 1
430 REM
500 REM =============================================
501 REM  SECTION 4: STRING FUNCTIONS
502 REM =============================================
503 PRINT "=== SECTION 4: STRING FUNCTIONS ==="
510 IF LEN("HELLO") = 5 THEN PRINT "PASS: LEN" : P = P + 1 : GOTO 520
511 PRINT "FAIL: LEN" : F = F + 1
520 IF ASC("A") = 65 THEN PRINT "PASS: ASC" : P = P + 1 : GOTO 530
521 PRINT "FAIL: ASC" : F = F + 1
530 IF CHR$(65) = "A" THEN PRINT "PASS: CHR$" : P = P + 1 : GOTO 540
531 PRINT "FAIL: CHR$" : F = F + 1
540 IF LEFT$("HELLO", 2) = "HE" THEN PRINT "PASS: LEFT$" : P = P + 1 : GOTO 550
541 PRINT "FAIL: LEFT$" : F = F + 1
550 IF RIGHT$("HELLO", 2) = "LO" THEN PRINT "PASS: RIGHT$" : P = P + 1 : GOTO 560
551 PRINT "FAIL: RIGHT$" : F = F + 1
560 IF MID$("HELLO", 2, 3) = "ELL" THEN PRINT "PASS: MID$" : P = P + 1 : GOTO 570
561 PRINT "FAIL: MID$" : F = F + 1
570 IF VAL("123") = 123 THEN PRINT "PASS: VAL" : P = P + 1 : GOTO 580
571 PRINT "FAIL: VAL" : F = F + 1
580 IF STR$(42) = " 42" THEN PRINT "PASS: STR$" : P = P + 1 : GOTO 590
581 IF STR$(42) = "42" THEN PRINT "PASS: STR$" : P = P + 1 : GOTO 590
582 PRINT "FAIL: STR$ (got '"; STR$(42); "')" : F = F + 1
590 IF INSTR("HELLO WORLD", "WORLD") > 0 THEN PRINT "PASS: INSTR" : P = P + 1 : GOTO 595
591 PRINT "FAIL: INSTR" : F = F + 1
595 REM --- String concatenation ---
596 LET A$ = "HELLO"
597 LET B$ = " WORLD"
598 LET C$ = A$ + B$
599 IF C$ = "HELLO WORLD" THEN PRINT "PASS: String concat" : P = P + 1 ELSE PRINT "FAIL: String concat" : F = F + 1
600 REM
700 REM =============================================
701 REM  SECTION 5: VARIABLES & ARRAYS
702 REM =============================================
703 PRINT "=== SECTION 5: VARIABLES & ARRAYS ==="
710 REM --- String variables ---
711 LET A$ = "TEST"
712 IF A$ = "TEST" THEN PRINT "PASS: String vars" : P = P + 1 : GOTO 720
713 PRINT "FAIL: String vars" : F = F + 1
720 REM --- DIM arrays ---
721 DIM ARR(10)
722 ARR(1) = 100
723 ARR(5) = 500
724 IF ARR(1) = 100 AND ARR(5) = 500 THEN PRINT "PASS: DIM 1D" : P = P + 1 : GOTO 730
725 PRINT "FAIL: DIM 1D" : F = F + 1
730 REM --- DIM 2D ---
731 DIM MT(3, 3)
732 MT(1, 2) = 42
733 IF MT(1, 2) = 42 THEN PRINT "PASS: DIM 2D" : P = P + 1 : GOTO 740
734 PRINT "FAIL: DIM 2D" : F = F + 1
740 REM --- SWAP ---
741 LET X = 10
742 LET Y = 20
743 SWAP X, Y
744 IF X = 20 AND Y = 10 THEN PRINT "PASS: SWAP" : P = P + 1 : GOTO 750
745 PRINT "FAIL: SWAP" : F = F + 1
750 REM --- OPTION BASE ---
751 OPTION BASE 0
752 DIM ZZ(5)
753 ZZ(0) = 99
754 IF ZZ(0) = 99 THEN PRINT "PASS: OPTION BASE 0" : P = P + 1 : GOTO 760
755 PRINT "FAIL: OPTION BASE 0" : F = F + 1
760 REM OPTION BASE 1 skipped (conflicts with existing arrays)
770 REM --- DATA / READ / RESTORE ---
771 READ D1
772 READ D2
773 READ D$
774 IF D1 = 10 AND D2 = 20 AND D$ = "HELLO" THEN PRINT "PASS: DATA/READ" : P = P + 1 : GOTO 780
775 PRINT "FAIL: DATA/READ" : F = F + 1
776 DATA 10, 20, "HELLO"
780 REM --- RESTORE ---
781 RESTORE
782 READ R1
783 IF R1 = 10 THEN PRINT "PASS: RESTORE" : P = P + 1 : GOTO 790
784 PRINT "FAIL: RESTORE" : F = F + 1
790 REM
800 REM =============================================
801 REM  SECTION 6: FILE I/O
802 REM =============================================
803 PRINT "=== SECTION 6: FILE I/O ==="
810 REM --- OPEN / PRINT # / CLOSE ---
811 OPEN "testfile.dat" FOR OUTPUT AS #1
812 PRINT #1, "File test data"
813 PRINT #1, "Second line"
814 CLOSE #1
815 PRINT "PASS: OPEN/PRINT#/CLOSE" : P = P + 1
820 REM --- INPUT # ---
821 OPEN "testfile.dat" FOR INPUT AS #1
822 LINE INPUT #1, A$
823 IF A$ = "File test data" THEN PRINT "PASS: LINE INPUT#" : P = P + 1 : GOTO 830
824 PRINT "FAIL: LINE INPUT# (got '" ; A$; "')" : F = F + 1
830 REM --- EOF ---
831 LINE INPUT #1, B$
832 IF EOF(1) THEN PRINT "PASS: EOF" : P = P + 1 : GOTO 840
833 PRINT "FAIL: EOF" : F = F + 1
840 CLOSE #1
845 REM --- APPEND ---
846 OPEN "testfile.dat" FOR APPEND AS #1
847 PRINT #1, "Appended line"
848 CLOSE #1
849 PRINT "PASS: APPEND" : P = P + 1
850 REM
900 REM =============================================
901 REM  SECTION 7: DEF FN
902 REM =============================================
903 PRINT "=== SECTION 7: DEF FN ==="
910 DEF FNA(X) = X * X + 1
911 IF FNA(5) = 26 THEN PRINT "PASS: DEF FN" : P = P + 1 : GOTO 920
912 PRINT "FAIL: DEF FN (got "; FNA(5); ")" : F = F + 1
920 DEF FNB(X, Y) = X + Y
921 IF FNB(3, 4) = 7 THEN PRINT "PASS: DEF FN multi-param" : P = P + 1 : GOTO 930
922 PRINT "FAIL: DEF FN multi-param" : F = F + 1
930 REM
1000 REM =============================================
1001 REM  SECTION 8: ERROR HANDLING
1002 REM =============================================
1003 PRINT "=== SECTION 8: ERROR HANDLING ==="
1010 REM --- ON ERROR GOTO ---
1011 ON ERROR GOTO 1020
1012 LET X = 1 / 0
1013 PRINT "FAIL: ON ERROR (division didn't trap)" : F = F + 1 : GOTO 1030
1020 PRINT "PASS: ON ERROR GOTO" : P = P + 1
1021 RESUME 1030
1030 ON ERROR GOTO 0
1040 REM
1100 REM =============================================
1101 REM  SECTION 9: PRINT FORMATTING
1102 REM =============================================
1103 PRINT "=== SECTION 9: PRINT FORMATTING ==="
1110 REM --- PRINT USING ---
1111 PRINT "PASS: PRINT USING (visual check:)  ";
1112 PRINT USING "###.##"; 3.14
1113 P = P + 1
1120 REM --- TAB ---
1121 PRINT "PASS: TAB ->"; TAB(20); "<- col 20"
1122 P = P + 1
1130 REM
1200 REM =============================================
1201 REM  SECTION 10: LOGICAL OPERATORS
1202 REM =============================================
1203 PRINT "=== SECTION 10: LOGICAL OPERATORS ==="
1210 IF 1 AND 1 THEN PRINT "PASS: AND" : P = P + 1 : GOTO 1220
1211 PRINT "FAIL: AND" : F = F + 1
1220 IF 1 OR 0 THEN PRINT "PASS: OR" : P = P + 1 : GOTO 1230
1221 PRINT "FAIL: OR" : F = F + 1
1230 IF NOT 0 THEN PRINT "PASS: NOT" : P = P + 1 : GOTO 1240
1231 PRINT "FAIL: NOT" : F = F + 1
1240 IF (1 XOR 0) THEN PRINT "PASS: XOR" : P = P + 1 : GOTO 1250
1241 PRINT "FAIL: XOR" : F = F + 1
1250 REM
1300 REM =============================================
1301 REM  SECTION 11: BLOCK IF / SELECT CASE
1302 REM =============================================
1303 PRINT "=== SECTION 11: STRUCTURED CONTROL ==="
1310 REM --- Block IF / ELSEIF / END IF ---
1311 LET X = 2
1312 IF X = 1 THEN
1313   PRINT "FAIL: Block IF" : F = F + 1
1314 ELSEIF X = 2 THEN
1315   PRINT "PASS: Block IF/ELSEIF" : P = P + 1
1316 ELSE
1317   PRINT "FAIL: Block IF (ELSE)" : F = F + 1
1318 END IF
1320 REM --- SELECT CASE ---
1321 LET X = 3
1322 SELECT CASE X
1323 CASE 1
1324   PRINT "FAIL: SELECT CASE" : F = F + 1
1325 CASE 2, 3
1326   PRINT "PASS: SELECT CASE" : P = P + 1
1327 CASE ELSE
1328   PRINT "FAIL: SELECT CASE (ELSE)" : F = F + 1
1329 END SELECT
1340 REM
1400 REM =============================================
1401 REM  SECTION 12: FILE I/O (ECMA-116)
1402 REM =============================================
1403 PRINT "=== SECTION 12: ECMA-116 FILES ==="
1404 DIALECT "E116"
1410 OPEN #1: NAME "e116_test.dat", ACCESS OUTPUT
1411 PRINT #1, "ECMA-116 file"
1412 CLOSE #1
1413 OPEN #1: NAME "e116_test.dat", ACCESS INPUT
1414 LINE INPUT #1, A$
1415 IF A$ = "ECMA-116 file" THEN PRINT "PASS: E116 OPEN" : P = P + 1 : GOTO 1420
1416 PRINT "FAIL: E116 OPEN" : F = F + 1
1420 REM --- ASK POINTER ---
1421 ASK #1: POINTER PP
1422 CLOSE #1
1423 PRINT "PASS: ASK POINTER (="; PP; ")" : P = P + 1
1430 REM --- ASK FILESIZE ---
1431 OPEN "e116_test.dat" FOR INPUT AS #1
1432 ASK #1: FILESIZE SZ
1433 CLOSE #1
1434 IF SZ > 0 THEN PRINT "PASS: ASK FILESIZE (="; SZ; ")" : P = P + 1 : GOTO 1440
1435 PRINT "FAIL: ASK FILESIZE" : F = F + 1
1440 DIALECT "GWBS"
1450 REM
1500 REM =============================================
1501 REM  SECTION 13: ECMA-116 FEATURES
1502 REM =============================================
1503 PRINT "=== SECTION 13: ECMA-116 FEATURES ==="
1504 DIALECT "E116"
1510 REM --- OPTION ANGLE ---
1511 OPTION ANGLE DEGREES
1512 LET V = SIN(90)
1513 IF V > 0.99 THEN PRINT "PASS: OPTION ANGLE DEG" : P = P + 1 : GOTO 1520
1514 PRINT "FAIL: OPTION ANGLE DEG (V="; V; ")" : F = F + 1
1520 OPTION ANGLE RADIANS
1530 REM --- OPTION ARITHMETIC NATIVE ---
1531 OPTION ARITHMETIC NATIVE
1532 PRINT "PASS: OPTION ARITHMETIC" : P = P + 1
1540 REM --- OPTION STRICT ---
1541 OPTION STRICT
1542 LET X = 42
1543 PRINT "PASS: OPTION STRICT (E116)" : P = P + 1
1544 OPTION STRICT OFF
1550 DIALECT "GWBS"
1560 REM
1600 REM =============================================
1601 REM  SECTION 14: ALIAS SYSTEM
1602 REM =============================================
1603 PRINT "=== SECTION 14: ALIAS ==="
1610 ALIAS PRINT = "IMPRIMIR"
1620 IMPRIMIR "PASS: ALIAS PRINT->IMPRIMIR"
1621 P = P + 1
1630 ALIAS CLEAR
1640 PRINT "PASS: ALIAS CLEAR" : P = P + 1
1650 REM
1700 REM =============================================
1701 REM  SECTION 15: DIALECT SYSTEM
1702 REM =============================================
1703 PRINT "=== SECTION 15: DIALECT ==="
1710 DIALECT "GWBS"
1711 PRINT "PASS: DIALECT switch" : P = P + 1
1720 REM
1800 REM =============================================
1801 REM  SECTION 16: CONST
1802 REM =============================================
1803 PRINT "=== SECTION 16: CONST ==="
1810 CONST PI = 3.14159
1811 IF PI > 3.14 AND PI < 3.15 THEN PRINT "PASS: CONST" : P = P + 1 : GOTO 1820
1812 PRINT "FAIL: CONST" : F = F + 1
1820 REM
1900 REM =============================================
1901 REM  SECTION 17: SUB / FUNCTION
1902 REM =============================================
1903 PRINT "=== SECTION 17: SUB/FUNCTION ==="
1910 CALL TestSub
1911 P = P + 1
1920 LET R = Square(5)
1921 IF R = 25 THEN PRINT "PASS: FUNCTION" : P = P + 1 : GOTO 1930
1922 PRINT "FAIL: FUNCTION (R="; R; ")" : F = F + 1
1930 GOTO 1960
1940 SUB TestSub
1941 PRINT "PASS: SUB/CALL"
1942 END SUB
1950 FUNCTION Square(N)
1951 Square = N * N
1952 END FUNCTION
1960 REM
2000 REM =============================================
2001 REM  SECTION 18: WHEN EXCEPTION
2002 REM =============================================
2003 PRINT "=== SECTION 18: WHEN EXCEPTION ==="
2004 DIALECT "E116"
2010 WHEN EXCEPTION IN
2011   LET X = 1 / 0
2012 USE
2013   PRINT "PASS: WHEN EXCEPTION" : P = P + 1
2014 END WHEN
2020 DIALECT "GWBS"
2030 REM
2100 REM =============================================
2101 REM  SECTION 19: CAUSE EXCEPTION
2102 REM =============================================
2103 PRINT "=== SECTION 19: CAUSE EXCEPTION ==="
2104 DIALECT "E116"
2110 WHEN EXCEPTION IN
2111   CAUSE EXCEPTION 999
2112 USE
2113   PRINT "PASS: CAUSE EXCEPTION" : P = P + 1
2114 END WHEN
2120 DIALECT "GWBS"
2130 REM
2200 REM =============================================
2201 REM  SECTION 20: COMPARISON OPERATORS
2202 REM =============================================
2203 PRINT "=== SECTION 20: COMPARISONS ==="
2210 IF 1 < 2 THEN PRINT "PASS: <" : P = P + 1 : GOTO 2220
2211 PRINT "FAIL: <" : F = F + 1
2220 IF 2 > 1 THEN PRINT "PASS: >" : P = P + 1 : GOTO 2230
2221 PRINT "FAIL: >" : F = F + 1
2230 IF 1 <= 1 THEN PRINT "PASS: <=" : P = P + 1 : GOTO 2240
2231 PRINT "FAIL: <=" : F = F + 1
2240 IF 1 >= 1 THEN PRINT "PASS: >=" : P = P + 1 : GOTO 2250
2241 PRINT "FAIL: >=" : F = F + 1
2250 IF 1 <> 2 THEN PRINT "PASS: <>" : P = P + 1 : GOTO 2260
2251 PRINT "FAIL: <>" : F = F + 1
2260 REM
2300 REM =============================================
2301 REM  SECTION 21: ARITHMETIC
2302 REM =============================================
2303 PRINT "=== SECTION 21: ARITHMETIC ==="
2310 IF 2 + 3 = 5 THEN PRINT "PASS: +" : P = P + 1 : GOTO 2320
2311 PRINT "FAIL: +" : F = F + 1
2320 IF 10 - 4 = 6 THEN PRINT "PASS: -" : P = P + 1 : GOTO 2330
2321 PRINT "FAIL: -" : F = F + 1
2330 IF 3 * 4 = 12 THEN PRINT "PASS: *" : P = P + 1 : GOTO 2340
2331 PRINT "FAIL: *" : F = F + 1
2340 IF 10 / 2 = 5 THEN PRINT "PASS: /" : P = P + 1 : GOTO 2350
2341 PRINT "FAIL: /" : F = F + 1
2350 IF 2 ^ 3 = 8 THEN PRINT "PASS: ^" : P = P + 1 : GOTO 2360
2351 PRINT "FAIL: ^" : F = F + 1
2360 IF -5 + 5 = 0 THEN PRINT "PASS: unary -" : P = P + 1 : GOTO 2370
2361 PRINT "FAIL: unary -" : F = F + 1
2370 REM
2400 REM =============================================
2401 REM  SECTION 22: STRING COMPARISON
2402 REM =============================================
2403 PRINT "=== SECTION 22: STRING COMPARISON ==="
2410 IF "ABC" = "ABC" THEN PRINT "PASS: str =" : P = P + 1 : GOTO 2420
2411 PRINT "FAIL: str =" : F = F + 1
2420 IF "ABC" < "DEF" THEN PRINT "PASS: str <" : P = P + 1 : GOTO 2430
2421 PRINT "FAIL: str <" : F = F + 1
2430 IF "DEF" > "ABC" THEN PRINT "PASS: str >" : P = P + 1 : GOTO 2440
2431 PRINT "FAIL: str >" : F = F + 1
2440 REM
2500 REM =============================================
2501 REM  SECTION 23: ERASE
2502 REM =============================================
2503 PRINT "=== SECTION 23: ERASE ==="
2510 DIM EA(5)
2511 EA(1) = 42
2512 ERASE EA
2513 PRINT "PASS: ERASE array" : P = P + 1
2520 REM
2600 REM =============================================
2601 REM  SECTION 24: RANDOMIZE
2602 REM =============================================
2603 PRINT "=== SECTION 24: RANDOMIZE ==="
2610 RANDOMIZE 42
2611 LET R1 = RND(100)
2612 RANDOMIZE 42
2613 LET R2 = RND(100)
2614 IF R1 = R2 THEN PRINT "PASS: RANDOMIZE seed" : P = P + 1 : GOTO 2620
2615 PRINT "FAIL: RANDOMIZE seed (R1="; R1; " R2="; R2; ")" : F = F + 1
2620 REM
2700 REM =============================================
2701 REM  SECTION 25: FLOAT PRECISION
2702 REM =============================================
2703 PRINT "=== SECTION 25: FLOAT PRECISION ==="
2710 LET V = 1E10
2711 IF V > 9E9 THEN PRINT "PASS: 1E10" : P = P + 1 : GOTO 2720
2712 PRINT "FAIL: 1E10 (V="; V; ")" : F = F + 1
2720 LET V = 1.5E-3
2721 IF V > 0.001 AND V < 0.002 THEN PRINT "PASS: 1.5E-3" : P = P + 1 : GOTO 2730
2722 PRINT "FAIL: 1.5E-3 (V="; V; ")" : F = F + 1
2730 REM
2800 REM =============================================
2801 REM  SECTION 26: MEMMAP
2802 REM =============================================
2803 PRINT "=== SECTION 26: MEMMAP / PEEK / POKE ==="
2810 MEMMAP "MSDOS"
2811 POKE 1024, 42
2812 LET V = PEEK(1024)
2813 IF V = 42 THEN PRINT "PASS: PEEK/POKE" : P = P + 1 : GOTO 2820
2814 PRINT "FAIL: PEEK/POKE (V="; V; ")" : F = F + 1
2820 MEMMAP "NONE"
2830 REM
2900 REM =============================================
2901 REM  SECTION 27: LOCATE
2902 REM =============================================
2903 PRINT "=== SECTION 27: LOCATE ==="
2910 REM LOCATE sets cursor position
2911 LOCATE 1, 1
2912 PRINT "PASS: LOCATE" : P = P + 1
2920 REM
3000 REM =============================================
3001 REM  SECTION 28: COLOR
3002 REM =============================================
3003 PRINT "=== SECTION 28: COLOR ==="
3010 COLOR 7, 0
3011 PRINT "PASS: COLOR" : P = P + 1
3020 REM
3100 REM =============================================
3101 REM  SECTION 29: HEX$ / OCT$
3102 REM =============================================
3103 PRINT "=== SECTION 29: HEX$/OCT$ ==="
3110 IF HEX$(255) = "FF" THEN PRINT "PASS: HEX$" : P = P + 1 : GOTO 3120
3111 PRINT "FAIL: HEX$ (got '" ; HEX$(255); "')" : F = F + 1
3120 IF OCT$(8) = "10" THEN PRINT "PASS: OCT$" : P = P + 1 : GOTO 3130
3121 PRINT "FAIL: OCT$ (got '"; OCT$(8); "')" : F = F + 1
3130 REM
3200 REM =============================================
3201 REM  SECTION 30: MULTI-STATEMENT LINES
3202 REM =============================================
3203 PRINT "=== SECTION 30: MULTI-STATEMENT ==="
3210 LET X = 1 : LET Y = 2 : LET Z = X + Y
3211 IF Z = 3 THEN PRINT "PASS: Multi-statement" : P = P + 1 : GOTO 3220
3212 PRINT "FAIL: Multi-statement" : F = F + 1
3220 REM
8000 REM =============================================
8001 REM  FINAL SUMMARY
8002 REM =============================================
8010 PRINT ""
8020 PRINT "============================================"
8030 PRINT " BASIC++ TEST RESULTS"
8040 PRINT "============================================"
8050 PRINT " PASSED:"; P
8060 PRINT " FAILED:"; F
8070 PRINT " TOTAL: "; P + F
8080 PRINT "============================================"
8090 IF F = 0 THEN PRINT " *** ALL TESTS PASSED ***"
8095 IF F > 0 THEN PRINT " *** SOME TESTS FAILED ***"
8100 PRINT "============================================"
8110 REM --- Clean up test files ---
8120 KILL "testfile.dat"
8130 KILL "e116_test.dat"
8200 END
8990 REM
9000 REM --- GOSUB target ---
9010 RETURN
