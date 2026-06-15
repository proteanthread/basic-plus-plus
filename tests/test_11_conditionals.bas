10 REM --- BASIC++ Test Suite ---
20 REM Part 11: Conditional Output Specifiers (Phase 6)
30 REM Tests >>, <<, >=, <=, ==, <>, >< in PRINT USING
40 REM
50 P = 0 : F = 0
60 REM
100 REM === GREATER THAN (>>) ===
110 REM 10 > 5 is TRUE, should print 5
120 PRINT ">> 10,5 = ";
130 PRINT USING ">>"; 10; 5
140 P = P + 1
150 REM 5 > 10 is FALSE, should print nothing
160 PRINT "<< 5,10 expect empty: ";
170 PRINT USING ">>"; 5; 10
180 P = P + 1
190 REM
200 REM === LESS THAN (<<) ===
210 REM 5 < 10 is TRUE, should print 10
220 PRINT "<< 5,10 = ";
230 PRINT USING "<<"; 5; 10
240 P = P + 1
250 REM 10 < 5 is FALSE, should print nothing
260 PRINT "<< 10,5 expect empty: ";
270 PRINT USING "<<"; 10; 5
280 P = P + 1
290 REM
300 REM === GREATER OR EQUAL (>=) ===
310 REM 10 >= 10 is TRUE, should print 10
320 PRINT ">= 10,10 = ";
330 PRINT USING ">="; 10; 10
340 P = P + 1
350 REM 5 >= 10 is FALSE, should print nothing
360 PRINT ">= 5,10 expect empty: ";
370 PRINT USING ">="; 5; 10
380 P = P + 1
390 REM
400 REM === LESS OR EQUAL (<=) ===
410 REM 10 <= 10 is TRUE, should print 10
420 PRINT "<= 10,10 = ";
430 PRINT USING "<="; 10; 10
440 P = P + 1
450 REM 20 <= 10 is FALSE, should print nothing
460 PRINT "<= 20,10 expect empty: ";
470 PRINT USING "<="; 20; 10
480 P = P + 1
490 REM
500 REM === EQUAL (==) ===
510 REM 7 == 7 is TRUE, should print 7
520 PRINT "== 7,7 = ";
530 PRINT USING "=="; 7; 7
540 P = P + 1
550 REM 7 == 8 is FALSE, should print nothing
560 PRINT "== 7,8 expect empty: ";
570 PRINT USING "=="; 7; 8
580 P = P + 1
590 REM
600 REM === NOT EQUAL (<>) ===
610 REM 5 <> 6 is TRUE, should print 6
620 PRINT "<> 5,6 = ";
630 PRINT USING "<>"; 5; 6
640 P = P + 1
650 REM 5 <> 5 is FALSE, should print nothing
660 PRINT "<> 5,5 expect empty: ";
670 PRINT USING "<>"; 5; 5
680 P = P + 1
690 REM
700 REM === NOT EQUAL ALT (><) ===
710 REM 5 >< 6 is TRUE, should print 6
720 PRINT ">< 5,6 = ";
730 PRINT USING "><"; 5; 6
740 P = P + 1
750 REM
800 PRINT
810 PRINT "Conditional output tests: "; P; " passed"
820 END
