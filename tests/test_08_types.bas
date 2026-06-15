10 REM --- BASIC++ Test Suite ---
20 REM Part 8: Type System Enforcement (Milestone 5)
30 REM Tests type coercion, radix literals, precision
40 REM
50 P = 0 : F = 0
60 REM
100 REM === RADIX LITERALS ===
110 REM --- Hex literal (&H) ---
120 A = &HFF
130 IF A = 255 THEN P = P + 1 : PRINT "PASS: &HFF = 255" ELSE F = F + 1 : PRINT "FAIL: &HFF"
140 A = &h0A
150 IF A = 10 THEN P = P + 1 : PRINT "PASS: &h0A = 10" ELSE F = F + 1 : PRINT "FAIL: &h0A"
160 REM --- Octal literal (&O) ---
170 A = &O377
180 IF A = 255 THEN P = P + 1 : PRINT "PASS: &O377 = 255" ELSE F = F + 1 : PRINT "FAIL: &O377"
190 A = &o12
200 IF A = 10 THEN P = P + 1 : PRINT "PASS: &o12 = 10" ELSE F = F + 1 : PRINT "FAIL: &o12"
210 REM --- Binary literal (&B) ---
220 A = &B11111111
230 IF A = 255 THEN P = P + 1 : PRINT "PASS: &B11111111 = 255" ELSE F = F + 1 : PRINT "FAIL: &B11111111"
240 A = &b101010
250 IF A = 42 THEN P = P + 1 : PRINT "PASS: &b101010 = 42" ELSE F = F + 1 : PRINT "FAIL: &b101010"
260 REM
300 REM === TYPE SUFFIXES ===
310 REM --- Integer (%) ---
320 I% = 42
330 IF I% = 42 THEN P = P + 1 : PRINT "PASS: I% = 42" ELSE F = F + 1 : PRINT "FAIL: I%"
340 REM --- Single (!) ---
350 S! = 3.14
360 IF S! > 3.13 THEN P = P + 1 : PRINT "PASS: S! = "; S! ELSE F = F + 1 : PRINT "FAIL: S!"
370 REM --- Double (#) ---
380 D# = 3.14159265358979#
390 IF D# > 3.14 THEN P = P + 1 : PRINT "PASS: D# = "; D# ELSE F = F + 1 : PRINT "FAIL: D#"
400 REM --- String ($) ---
410 N$ = "HELLO"
420 IF N$ = "HELLO" THEN P = P + 1 : PRINT "PASS: N$ = HELLO" ELSE F = F + 1 : PRINT "FAIL: N$"
430 REM
500 REM === TYPE COERCION ===
510 REM --- Float to int truncation ---
520 I% = 3.75
530 IF I% = 3 THEN P = P + 1 : PRINT "PASS: 3.75 -> I% = 3" ELSE F = F + 1 : PRINT "FAIL: float->int"
540 REM --- Int to float promotion ---
550 S! = 42
560 IF S! = 42 THEN P = P + 1 : PRINT "PASS: 42 -> S! = 42" ELSE F = F + 1 : PRINT "FAIL: int->float"
570 REM
600 REM === ARITHMETIC WITH TYPES ===
610 A% = 10 : B% = 3
620 C% = A% / B%
630 IF C% = 3 THEN P = P + 1 : PRINT "PASS: 10/3 int = 3" ELSE F = F + 1 : PRINT "FAIL: int div"
640 A! = 10.0 : B! = 3.0
650 C! = A! / B!
660 IF C! > 3.33 THEN P = P + 1 : PRINT "PASS: 10/3 float = "; C! ELSE F = F + 1 : PRINT "FAIL: float div"
670 REM
700 REM === DATA/READ TYPE COERCION ===
710 DATA 42, 3.14, "HELLO"
720 READ R%
730 IF R% = 42 THEN P = P + 1 : PRINT "PASS: READ int = 42" ELSE F = F + 1 : PRINT "FAIL: READ int"
740 READ R!
750 IF R! > 3.13 THEN P = P + 1 : PRINT "PASS: READ float = "; R! ELSE F = F + 1 : PRINT "FAIL: READ float"
760 READ R$
770 IF R$ = "HELLO" THEN P = P + 1 : PRINT "PASS: READ string" ELSE F = F + 1 : PRINT "FAIL: READ string"
780 REM
900 PRINT
910 PRINT "Type system tests: "; P; " passed, "; F; " failed"
920 IF F > 0 THEN PRINT "*** FAILURES DETECTED ***"
930 END
