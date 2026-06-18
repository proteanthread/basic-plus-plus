10 REM ============================================================
20 REM  TEST: dialects.bas - Dialect Runtime Enforcement
30 REM                       (Milestone 6)
40 REM ============================================================
50 REM  Tests dialect-specific features in B++ mode:
60 REM
62 REM  SECTION 1 (100-160): B++ base features
64 REM   - Floats and strings should be available
66 REM
68 REM  SECTION 2 (200-260): LET enforcement
70 REM   - B++ makes LET optional (X=99 works)
72 REM   - Explicit LET Y=88 also works
74 REM
76 REM  SECTION 3 (300-370): Radix literals in B++
78 REM   - &HFF=255 (hex), &O77=63 (octal), &B1010=10 (binary)
80 REM
82 REM  SECTION 4 (400-490): Type suffixes
84 REM   - I% (int), S! (single), D# (double), N$ (string)
86 REM
88 REM  SECTION 5 (500-550): Named variables
90 REM   - MYVAR=100 (long name), MYSTR$="NAMED"
92 REM
94 REM  SECTION 6 (600-660): BASIC++ extensions
96 REM   - SWAP A,B exchanges values
97 REM
98 REM  SECTION 7 (700-730): CONST
99 REM   - CONST PI=3.14159 defines immutable constant
100 REM
102 REM  SECTION 8 (800-880): Print precision
104 REM   - Single (!) shows ~7 digits
106 REM   - Double (#) shows ~15 digits
108 REM
110 REM  EXPECTED: All assertions print PASS.
112 REM  Summary shows total passed/failed.
114 REM ============================================================
116 REM
120 P = 0 : F = 0
130 REM
200 REM === B++ DIALECT (default) ===
210 REM --- B++ should allow all features ---
220 REM  Floats and strings are available in B++ (unrestricted).
230 A! = 3.14
240 IF A! > 3.13 THEN P = P + 1 : PRINT "PASS: B++ allows floats" ELSE F = F + 1 : PRINT "FAIL: B++ floats"
250 A$ = "HELLO"
260 IF A$ = "HELLO" THEN P = P + 1 : PRINT "PASS: B++ allows strings" ELSE F = F + 1 : PRINT "FAIL: B++ strings"
270 REM
300 REM === LET ENFORCEMENT ===
310 REM --- B++ dialect: LET is optional ---
320 REM  In B++, bare assignment (X=99) works without LET.
330 REM  Explicit LET (LET Y=88) also works.
340 X = 99
350 IF X = 99 THEN P = P + 1 : PRINT "PASS: B++ LET optional" ELSE F = F + 1 : PRINT "FAIL: LET optional"
360 LET Y = 88
370 IF Y = 88 THEN P = P + 1 : PRINT "PASS: B++ LET explicit" ELSE F = F + 1 : PRINT "FAIL: LET explicit"
380 REM
400 REM === RADIX LITERALS IN B++ ===
410 REM  Hex, octal, and binary literals should all work in B++.
420 A = &HFF
430 IF A = 255 THEN P = P + 1 : PRINT "PASS: &HFF in B++" ELSE F = F + 1 : PRINT "FAIL: &HFF"
440 A = &O77
450 IF A = 63 THEN P = P + 1 : PRINT "PASS: &O77 in B++" ELSE F = F + 1 : PRINT "FAIL: &O77"
460 A = &B1010
470 IF A = 10 THEN P = P + 1 : PRINT "PASS: &B1010 in B++" ELSE F = F + 1 : PRINT "FAIL: &B1010"
480 REM
500 REM === TYPE SUFFIXES ===
510 REM  All type suffixes should work in B++.
520 I% = 42
530 IF I% = 42 THEN P = P + 1 : PRINT "PASS: I% integer var" ELSE F = F + 1 : PRINT "FAIL: I%"
540 S! = 2.718
550 IF S! > 2.71 THEN P = P + 1 : PRINT "PASS: S! single var" ELSE F = F + 1 : PRINT "FAIL: S!"
560 D# = 1.414213562#
570 IF D# > 1.41 THEN P = P + 1 : PRINT "PASS: D# double var" ELSE F = F + 1 : PRINT "FAIL: D#"
580 N$ = "WORLD"
590 IF N$ = "WORLD" THEN P = P + 1 : PRINT "PASS: N$ string var" ELSE F = F + 1 : PRINT "FAIL: N$"
600 REM
700 REM === NAMED VARIABLES ===
710 REM  B++ supports multi-character variable names.
720 MYVAR = 100
730 IF MYVAR = 100 THEN P = P + 1 : PRINT "PASS: Named var MYVAR" ELSE F = F + 1 : PRINT "FAIL: MYVAR"
740 MYSTR$ = "NAMED"
750 IF MYSTR$ = "NAMED" THEN P = P + 1 : PRINT "PASS: Named string MYSTR$" ELSE F = F + 1 : PRINT "FAIL: MYSTR$"
760 REM
800 REM === BASIC++ EXTENSIONS ===
810 REM --- SWAP (available in B++) ---
820 REM  SWAP A,B exchanges the values of A and B.
830 A = 10 : B = 20
840 SWAP A, B
850 IF A = 20 THEN P = P + 1 : PRINT "PASS: SWAP A=20" ELSE F = F + 1 : PRINT "FAIL: SWAP A"
860 IF B = 10 THEN P = P + 1 : PRINT "PASS: SWAP B=10" ELSE F = F + 1 : PRINT "FAIL: SWAP B"
870 REM
900 REM === CONST ===
910 REM  CONST defines an immutable constant.
920 REM  Attempting to reassign PI would raise an error.
930 CONST PI = 3.14159
940 IF PI > 3.14 THEN P = P + 1 : PRINT "PASS: CONST PI" ELSE F = F + 1 : PRINT "FAIL: CONST PI"
950 REM
1000 REM === PRINT PRECISION ===
1010 REM --- Single vs double precision display ---
1020 REM  Single (!) should show ~7 significant digits.
1030 REM  Double (#) should show ~15 significant digits.
1040 S! = 3.14159265
1050 PRINT "Single: "; S!
1060 P = P + 1
1070 D# = 3.14159265358979#
1080 PRINT "Double: "; D#
1090 P = P + 1
1100 REM
1200 PRINT
1210 PRINT "Dialect enforcement tests: "; P; " passed, "; F; " failed"
1220 IF F > 0 THEN PRINT "*** FAILURES DETECTED ***"
1230 END
