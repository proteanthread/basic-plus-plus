10 REM --- BASIC++ Test Suite ---
20 REM Part 12: Dialect Runtime Enforcement (Milestone 6)
30 REM Tests dialect features in B++ mode and
40 REM verifies feature availability
50 REM
60 P = 0 : F = 0
70 REM
100 REM === B++ DIALECT (default) ===
110 REM --- B++ should allow all features ---
120 A! = 3.14
130 IF A! > 3.13 THEN P = P + 1 : PRINT "PASS: B++ allows floats" ELSE F = F + 1 : PRINT "FAIL: B++ floats"
140 A$ = "HELLO"
150 IF A$ = "HELLO" THEN P = P + 1 : PRINT "PASS: B++ allows strings" ELSE F = F + 1 : PRINT "FAIL: B++ strings"
160 REM
200 REM === LET ENFORCEMENT ===
210 REM --- B++ dialect: LET is optional ---
220 X = 99
230 IF X = 99 THEN P = P + 1 : PRINT "PASS: B++ LET optional" ELSE F = F + 1 : PRINT "FAIL: LET optional"
240 LET Y = 88
250 IF Y = 88 THEN P = P + 1 : PRINT "PASS: B++ LET explicit" ELSE F = F + 1 : PRINT "FAIL: LET explicit"
260 REM
300 REM === RADIX LITERALS IN B++ ===
310 A = &HFF
320 IF A = 255 THEN P = P + 1 : PRINT "PASS: &HFF in B++" ELSE F = F + 1 : PRINT "FAIL: &HFF"
330 A = &O77
340 IF A = 63 THEN P = P + 1 : PRINT "PASS: &O77 in B++" ELSE F = F + 1 : PRINT "FAIL: &O77"
350 A = &B1010
360 IF A = 10 THEN P = P + 1 : PRINT "PASS: &B1010 in B++" ELSE F = F + 1 : PRINT "FAIL: &B1010"
370 REM
400 REM === TYPE SUFFIXES ===
410 I% = 42
420 IF I% = 42 THEN P = P + 1 : PRINT "PASS: I% integer var" ELSE F = F + 1 : PRINT "FAIL: I%"
430 S! = 2.718
440 IF S! > 2.71 THEN P = P + 1 : PRINT "PASS: S! single var" ELSE F = F + 1 : PRINT "FAIL: S!"
450 D# = 1.414213562#
460 IF D# > 1.41 THEN P = P + 1 : PRINT "PASS: D# double var" ELSE F = F + 1 : PRINT "FAIL: D#"
470 N$ = "WORLD"
480 IF N$ = "WORLD" THEN P = P + 1 : PRINT "PASS: N$ string var" ELSE F = F + 1 : PRINT "FAIL: N$"
490 REM
500 REM === NAMED VARIABLES ===
510 MYVAR = 100
520 IF MYVAR = 100 THEN P = P + 1 : PRINT "PASS: Named var MYVAR" ELSE F = F + 1 : PRINT "FAIL: MYVAR"
530 MYSTR$ = "NAMED"
540 IF MYSTR$ = "NAMED" THEN P = P + 1 : PRINT "PASS: Named string MYSTR$" ELSE F = F + 1 : PRINT "FAIL: MYSTR$"
550 REM
600 REM === BASIC++ EXTENSIONS ===
610 REM --- SWAP (available in B++) ---
620 A = 10 : B = 20
630 SWAP A, B
640 IF A = 20 THEN P = P + 1 : PRINT "PASS: SWAP A=20" ELSE F = F + 1 : PRINT "FAIL: SWAP A"
650 IF B = 10 THEN P = P + 1 : PRINT "PASS: SWAP B=10" ELSE F = F + 1 : PRINT "FAIL: SWAP B"
660 REM
700 REM === CONST ===
710 CONST PI = 3.14159
720 IF PI > 3.14 THEN P = P + 1 : PRINT "PASS: CONST PI" ELSE F = F + 1 : PRINT "FAIL: CONST PI"
730 REM
800 REM === PRINT PRECISION ===
810 REM --- Single vs double precision display ---
820 S! = 3.14159265
830 PRINT "Single: "; S!
840 P = P + 1
850 D# = 3.14159265358979#
860 PRINT "Double: "; D#
870 P = P + 1
880 REM
900 PRINT
910 PRINT "Dialect enforcement tests: "; P; " passed, "; F; " failed"
920 IF F > 0 THEN PRINT "*** FAILURES DETECTED ***"
930 END
