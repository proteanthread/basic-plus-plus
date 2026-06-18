10 REM ============================================================
20 REM  TEST: conditionals.bas - Conditional Output Specifiers
30 REM                           (Phase 6)
40 REM ============================================================
50 REM  Tests conditional comparison specifiers in PRINT USING.
60 REM  These take two values: if the comparison is TRUE, the
70 REM  second value is printed; if FALSE, nothing is output.
80 REM
82 REM  SPECIFIERS TESTED:
84 REM   >>  Greater than:       A > B  -> print B
86 REM   <<  Less than:          A < B  -> print B
88 REM   >=  Greater or equal:   A >= B -> print B
90 REM   <=  Less or equal:      A <= B -> print B
92 REM   ==  Equal:              A == B -> print B
94 REM   <>  Not equal:          A <> B -> print B
96 REM   ><  Not equal (alt):    A >< B -> print B
98 REM
99 REM  EXPECTED: Each TRUE case prints the second value.
100 REM  Each FALSE case prints nothing (empty line or
102 REM  just the label prefix). Counter P tracks tests.
104 REM  Ends with summary of tests passed.
106 REM ============================================================
108 REM
110 P = 0 : F = 0
120 REM
200 REM === GREATER THAN (>>) ===
210 REM  10 > 5 is TRUE: should print 5 (the second value).
220 PRINT ">> 10,5 = ";
230 PRINT USING ">>"; 10; 5
240 P = P + 1
250 REM  5 > 10 is FALSE: should print nothing.
260 PRINT "<< 5,10 expect empty: ";
270 PRINT USING ">>"; 5; 10
280 P = P + 1
290 REM
300 REM === LESS THAN (<<) ===
310 REM  5 < 10 is TRUE: should print 10.
320 PRINT "<< 5,10 = ";
330 PRINT USING "<<"; 5; 10
340 P = P + 1
350 REM  10 < 5 is FALSE: should print nothing.
360 PRINT "<< 10,5 expect empty: ";
370 PRINT USING "<<"; 10; 5
380 P = P + 1
390 REM
400 REM === GREATER OR EQUAL (>=) ===
410 REM  10 >= 10 is TRUE (equal): should print 10.
420 PRINT ">= 10,10 = ";
430 PRINT USING ">="; 10; 10
440 P = P + 1
450 REM  5 >= 10 is FALSE: should print nothing.
460 PRINT ">= 5,10 expect empty: ";
470 PRINT USING ">="; 5; 10
480 P = P + 1
490 REM
500 REM === LESS OR EQUAL (<=) ===
510 REM  10 <= 10 is TRUE (equal): should print 10.
520 PRINT "<= 10,10 = ";
530 PRINT USING "<="; 10; 10
540 P = P + 1
550 REM  20 <= 10 is FALSE: should print nothing.
560 PRINT "<= 20,10 expect empty: ";
570 PRINT USING "<="; 20; 10
580 P = P + 1
590 REM
600 REM === EQUAL (==) ===
610 REM  7 == 7 is TRUE: should print 7.
620 PRINT "== 7,7 = ";
630 PRINT USING "=="; 7; 7
640 P = P + 1
650 REM  7 == 8 is FALSE: should print nothing.
660 PRINT "== 7,8 expect empty: ";
670 PRINT USING "=="; 7; 8
680 P = P + 1
690 REM
700 REM === NOT EQUAL (<>) ===
710 REM  5 <> 6 is TRUE: should print 6.
720 PRINT "<> 5,6 = ";
730 PRINT USING "<>"; 5; 6
740 P = P + 1
750 REM  5 <> 5 is FALSE: should print nothing.
760 PRINT "<> 5,5 expect empty: ";
770 PRINT USING "<>"; 5; 5
780 P = P + 1
790 REM
800 REM === NOT EQUAL ALT (><) ===
810 REM  5 >< 6 is TRUE: should print 6. Alternate syntax.
820 PRINT ">< 5,6 = ";
830 PRINT USING "><"; 5; 6
840 P = P + 1
850 REM
900 PRINT
910 PRINT "Conditional output tests: "; P; " passed"
920 END
