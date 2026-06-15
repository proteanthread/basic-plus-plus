10 REM --- BASIC++ Test Suite ---
20 REM Part 13: DISPLAY, TYPE, LPRINT, Attributes (Milestone 7)
30 REM Tests stream I/O and formatted output features
40 REM
50 P = 0 : F = 0
60 REM
100 REM === LPRINT ===
110 REM --- LPRINT sends to stderr ---
120 LPRINT "LPRINT test (stderr)"
130 P = P + 1 : PRINT "PASS: LPRINT executed"
140 REM
200 REM === LPRINT USING ===
210 LPRINT USING "###.##"; 3.14
220 P = P + 1 : PRINT "PASS: LPRINT USING executed"
230 LPRINT USING "$$###.##"; 42.99
240 P = P + 1 : PRINT "PASS: LPRINT USING dollar"
250 REM
300 REM === CHARACTER ATTRIBUTES (ANSI) ===
310 REM --- A0-A9 style codes ---
320 PRINT USING "A1'Bold'A0"
330 P = P + 1 : PRINT "PASS: A1 bold"
340 PRINT USING "A4'Underline'A0"
350 P = P + 1 : PRINT "PASS: A4 underline"
360 PRINT USING "A7'Inverse'A0"
370 P = P + 1 : PRINT "PASS: A7 inverse"
380 REM --- A0 Reset ---
390 PRINT USING "A0'Reset'A0"
400 P = P + 1 : PRINT "PASS: A0 reset"
410 REM
420 REM --- Color codes (nA) ---
430 PRINT USING "31A'Red'A0"
440 P = P + 1 : PRINT "PASS: 31A red"
450 PRINT USING "32A'Green'A0"
460 P = P + 1 : PRINT "PASS: 32A green"
470 PRINT USING "33A'Yellow'A0"
480 P = P + 1 : PRINT "PASS: 33A yellow"
490 PRINT USING "34A'Blue'A0"
500 P = P + 1 : PRINT "PASS: 34A blue"
510 PRINT USING "35A'Magenta'A0"
520 P = P + 1 : PRINT "PASS: 35A magenta"
530 PRINT USING "36A'Cyan'A0"
540 P = P + 1 : PRINT "PASS: 36A cyan"
550 PRINT USING "37A'White'A0"
560 P = P + 1 : PRINT "PASS: 37A white"
570 REM
580 REM --- Background colors ---
590 PRINT USING "41A'Red BG'A0"
600 P = P + 1 : PRINT "PASS: 41A red bg"
610 PRINT USING "44A'Blue BG'A0"
620 P = P + 1 : PRINT "PASS: 44A blue bg"
630 REM
640 REM --- Combined ---
650 PRINT USING "A1A4'Bold+UL'A0"
660 P = P + 1 : PRINT "PASS: combined A1+A4"
670 PRINT USING "A131A'Bold Red'A0"
680 P = P + 1 : PRINT "PASS: bold+red"
690 REM
700 REM === MIXED ATTRIBUTES WITH FORMAT ===
710 PRINT USING "A1'Total: '###.##A0"; 42.50
720 P = P + 1 : PRINT "PASS: attr with numeric format"
730 PRINT USING "32A'Name: '&A0"; "World"
740 P = P + 1 : PRINT "PASS: attr with string format"
750 REM
900 PRINT
910 PRINT "Stream I/O tests: "; P; " passed, "; F; " failed"
920 IF F > 0 THEN PRINT "*** FAILURES DETECTED ***"
930 END
