10 REM ============================================================
20 REM  TEST: streamio.bas - Stream I/O, LPRINT, and ANSI
30 REM                       Attributes (Milestone 7)
40 REM ============================================================
50 REM  Tests stream output and formatted display features:
60 REM
62 REM  SECTION 1 (100-140): LPRINT
64 REM   - LPRINT sends output to stderr (not stdout).
66 REM   - Historically for line printers; mapped to stderr
68 REM     for modern terminal use.
70 REM
72 REM  SECTION 2 (200-250): LPRINT USING
74 REM   - LPRINT USING applies format specifiers to stderr.
76 REM   - Tests "###.##" and "$$###.##" formats.
78 REM
80 REM  SECTION 3 (300-410): Character Attributes (A0-A9)
82 REM   - A1 = Bold, A4 = Underline, A7 = Inverse
84 REM   - A0 = Reset all attributes
86 REM   - These emit ANSI SGR escape sequences.
88 REM
90 REM  SECTION 4 (420-630): Color Codes (nA)
92 REM   - Foreground: 31A=Red, 32A=Green, 33A=Yellow,
94 REM     34A=Blue, 35A=Magenta, 36A=Cyan, 37A=White
96 REM   - Background: 41A=Red BG, 44A=Blue BG
97 REM
98 REM  SECTION 5 (640-690): Combined Attributes
99 REM   - A1+A4 = Bold + Underline
100 REM   - A1+31A = Bold + Red
102 REM
104 REM  SECTION 6 (700-750): Mixed Attributes with Format
106 REM   - Attributes + numeric format: A1'Total: '###.##A0
108 REM   - Attributes + string format: 32A'Name: '&A0
110 REM
112 REM  EXPECTED: LPRINT output goes to stderr (may not
114 REM  appear in stdout capture). ANSI sequences appear
116 REM  as colored/styled text in compatible terminals.
118 REM  Counter P tracks all tests. Summary at end.
120 REM
122 REM  NOTE: ANSI attributes require a terminal that
124 REM  supports ANSI/VT100 escape sequences. Output may
126 REM  show raw escape codes in non-ANSI environments.
128 REM ============================================================
130 REM
140 P = 0 : F = 0
150 REM
200 REM === LPRINT ===
210 REM --- LPRINT sends to stderr ---
220 REM  LPRINT writes to the error stream, separate from
230 REM  normal PRINT output. Useful for diagnostics.
240 LPRINT "LPRINT test (stderr)"
250 P = P + 1 : PRINT "PASS: LPRINT executed"
260 REM
300 REM === LPRINT USING ===
310 REM  LPRINT USING combines format engine with stderr output.
320 LPRINT USING "###.##"; 3.14
330 P = P + 1 : PRINT "PASS: LPRINT USING executed"
340 REM  Dollar format on stderr.
350 LPRINT USING "$$###.##"; 42.99
360 P = P + 1 : PRINT "PASS: LPRINT USING dollar"
370 REM
400 REM === CHARACTER ATTRIBUTES (ANSI) ===
410 REM --- A0-A9 style codes ---
420 REM  A1 = Bold (SGR 1), A4 = Underline (SGR 4),
430 REM  A7 = Inverse (SGR 7), A0 = Reset (SGR 0).
440 REM  Text between attribute codes is styled.
450 PRINT USING "A1'Bold'A0"
460 P = P + 1 : PRINT "PASS: A1 bold"
470 PRINT USING "A4'Underline'A0"
480 P = P + 1 : PRINT "PASS: A4 underline"
490 PRINT USING "A7'Inverse'A0"
500 P = P + 1 : PRINT "PASS: A7 inverse"
510 REM --- A0 Reset ---
520 REM  A0 resets all attributes to terminal defaults.
530 PRINT USING "A0'Reset'A0"
540 P = P + 1 : PRINT "PASS: A0 reset"
550 REM
560 REM --- Foreground color codes (nA) ---
570 REM  ANSI foreground colors: 30-37 (31=red, 32=green, etc.)
580 REM  Format: <code>A to activate, A0 to reset.
590 PRINT USING "31A'Red'A0"
600 P = P + 1 : PRINT "PASS: 31A red"
610 PRINT USING "32A'Green'A0"
620 P = P + 1 : PRINT "PASS: 32A green"
630 PRINT USING "33A'Yellow'A0"
640 P = P + 1 : PRINT "PASS: 33A yellow"
650 PRINT USING "34A'Blue'A0"
660 P = P + 1 : PRINT "PASS: 34A blue"
670 PRINT USING "35A'Magenta'A0"
680 P = P + 1 : PRINT "PASS: 35A magenta"
690 PRINT USING "36A'Cyan'A0"
700 P = P + 1 : PRINT "PASS: 36A cyan"
710 PRINT USING "37A'White'A0"
720 P = P + 1 : PRINT "PASS: 37A white"
730 REM
740 REM --- Background colors ---
750 REM  ANSI background colors: 40-47 (41=red bg, 44=blue bg)
760 PRINT USING "41A'Red BG'A0"
770 P = P + 1 : PRINT "PASS: 41A red bg"
780 PRINT USING "44A'Blue BG'A0"
790 P = P + 1 : PRINT "PASS: 44A blue bg"
800 REM
810 REM --- Combined attributes ---
820 REM  Multiple attribute codes can be chained.
830 REM  A1+A4 = Bold + Underline simultaneously.
840 PRINT USING "A1A4'Bold+UL'A0"
850 P = P + 1 : PRINT "PASS: combined A1+A4"
860 REM  A1+31A = Bold + Red foreground.
870 PRINT USING "A131A'Bold Red'A0"
880 P = P + 1 : PRINT "PASS: bold+red"
890 REM
900 REM === MIXED ATTRIBUTES WITH FORMAT ===
910 REM  Attributes can wrap numeric and string format specs.
920 REM  A1'Total: '###.##A0 = Bold label + formatted number.
930 PRINT USING "A1'Total: '###.##A0"; 42.50
940 P = P + 1 : PRINT "PASS: attr with numeric format"
950 REM  32A'Name: '&A0 = Green label + full string.
960 PRINT USING "32A'Name: '&A0"; "World"
970 P = P + 1 : PRINT "PASS: attr with string format"
980 REM
1000 PRINT
1010 PRINT "Stream I/O tests: "; P; " passed, "; F; " failed"
1020 IF F > 0 THEN PRINT "*** FAILURES DETECTED ***"
1030 END
