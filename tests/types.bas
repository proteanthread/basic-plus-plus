10 REM ============================================================
20 REM  TEST: types.bas - Type System Enforcement (Milestone 5)
30 REM ============================================================
40 REM  Tests type coercion, radix literals, and precision:
50 REM
60 REM  SECTION 1 (100-260): Radix Literals
70 REM   - &H (hex): &HFF=255, &h0A=10
72 REM   - &O (octal): &O377=255, &o12=10
74 REM   - &B (binary): &B11111111=255, &b101010=42
76 REM
78 REM  SECTION 2 (300-430): Type Suffixes
80 REM   - % integer, ! single, # double, $ string
82 REM
84 REM  SECTION 3 (500-570): Type Coercion
86 REM   - Float-to-int truncation (3.75 -> 3)
88 REM   - Int-to-float promotion (42 -> 42.0)
90 REM
92 REM  SECTION 4 (600-670): Arithmetic with typed variables
94 REM   - Integer division truncates (10/3 = 3)
96 REM   - Float division preserves (10.0/3.0 = 3.33...)
97 REM
98 REM  SECTION 5 (700-780): DATA/READ Type Coercion
99 REM   - READ into R% (int), R! (float), R$ (string)
100 REM
102 REM  EXPECTED: Each assertion prints PASS or FAIL.
104 REM  Summary at end shows total passed/failed.
106 REM ============================================================
108 REM
110 P = 0 : F = 0
120 REM
200 REM === RADIX LITERALS ===
210 REM --- Hex literal (&H) ---
220 REM  &HFF = 15*16+15 = 255 (case-insensitive prefix)
230 A = &HFF
240 IF A = 255 THEN P = P + 1 : PRINT "PASS: &HFF = 255" ELSE F = F + 1 : PRINT "FAIL: &HFF"
250 A = &h0A
260 IF A = 10 THEN P = P + 1 : PRINT "PASS: &h0A = 10" ELSE F = F + 1 : PRINT "FAIL: &h0A"
270 REM --- Octal literal (&O) ---
280 REM  &O377 = 3*64+7*8+7 = 255
290 A = &O377
300 IF A = 255 THEN P = P + 1 : PRINT "PASS: &O377 = 255" ELSE F = F + 1 : PRINT "FAIL: &O377"
310 A = &o12
320 IF A = 10 THEN P = P + 1 : PRINT "PASS: &o12 = 10" ELSE F = F + 1 : PRINT "FAIL: &o12"
330 REM --- Binary literal (&B) ---
340 REM  &B11111111 = 255 (eight 1-bits)
350 A = &B11111111
360 IF A = 255 THEN P = P + 1 : PRINT "PASS: &B11111111 = 255" ELSE F = F + 1 : PRINT "FAIL: &B11111111"
370 A = &b101010
380 IF A = 42 THEN P = P + 1 : PRINT "PASS: &b101010 = 42" ELSE F = F + 1 : PRINT "FAIL: &b101010"
390 REM
500 REM === TYPE SUFFIXES ===
510 REM --- Integer (%) ---
520 REM  I% is an integer variable. Assignment of 42 should
530 REM  store exactly 42 with no fractional part.
540 I% = 42
550 IF I% = 42 THEN P = P + 1 : PRINT "PASS: I% = 42" ELSE F = F + 1 : PRINT "FAIL: I%"
560 REM --- Single (!) ---
570 REM  S! is a single-precision float (~7 digits).
580 S! = 3.14
590 IF S! > 3.13 THEN P = P + 1 : PRINT "PASS: S! = "; S! ELSE F = F + 1 : PRINT "FAIL: S!"
600 REM --- Double (#) ---
610 REM  D# is a double-precision float (~15 digits).
620 D# = 3.14159265358979#
630 IF D# > 3.14 THEN P = P + 1 : PRINT "PASS: D# = "; D# ELSE F = F + 1 : PRINT "FAIL: D#"
640 REM --- String ($) ---
650 REM  N$ is a string variable.
660 N$ = "HELLO"
670 IF N$ = "HELLO" THEN P = P + 1 : PRINT "PASS: N$ = HELLO" ELSE F = F + 1 : PRINT "FAIL: N$"
680 REM
700 REM === TYPE COERCION ===
710 REM --- Float to int truncation ---
720 REM  Assigning 3.75 to I% should truncate to 3 (not round).
730 I% = 3.75
740 IF I% = 3 THEN P = P + 1 : PRINT "PASS: 3.75 -> I% = 3" ELSE F = F + 1 : PRINT "FAIL: float->int"
750 REM --- Int to float promotion ---
760 REM  Assigning 42 (integer) to S! promotes to 42.0.
770 S! = 42
780 IF S! = 42 THEN P = P + 1 : PRINT "PASS: 42 -> S! = 42" ELSE F = F + 1 : PRINT "FAIL: int->float"
790 REM
800 REM === ARITHMETIC WITH TYPES ===
810 REM --- Integer division truncates ---
820 REM  10 / 3 in integer context = 3 (truncated).
830 A% = 10 : B% = 3
840 C% = A% / B%
850 IF C% = 3 THEN P = P + 1 : PRINT "PASS: 10/3 int = 3" ELSE F = F + 1 : PRINT "FAIL: int div"
860 REM --- Float division preserves precision ---
870 REM  10.0 / 3.0 in float context = 3.333...
880 A! = 10.0 : B! = 3.0
890 C! = A! / B!
900 IF C! > 3.33 THEN P = P + 1 : PRINT "PASS: 10/3 float = "; C! ELSE F = F + 1 : PRINT "FAIL: float div"
910 REM
1000 REM === DATA/READ TYPE COERCION ===
1010 REM  DATA stores mixed types. READ coerces to the target
1020 REM  variable's type: R% (int), R! (float), R$ (string).
1030 DATA 42, 3.14, "HELLO"
1040 READ R%
1050 IF R% = 42 THEN P = P + 1 : PRINT "PASS: READ int = 42" ELSE F = F + 1 : PRINT "FAIL: READ int"
1060 READ R!
1070 IF R! > 3.13 THEN P = P + 1 : PRINT "PASS: READ float = "; R! ELSE F = F + 1 : PRINT "FAIL: READ float"
1080 READ R$
1090 IF R$ = "HELLO" THEN P = P + 1 : PRINT "PASS: READ string" ELSE F = F + 1 : PRINT "FAIL: READ string"
1100 REM
1200 PRINT
1210 PRINT "Type system tests: "; P; " passed, "; F; " failed"
1220 IF F > 0 THEN PRINT "*** FAILURES DETECTED ***"
1230 END
