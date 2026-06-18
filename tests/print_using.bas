10 REM ============================================================
20 REM  TEST: print_using.bas - PRINT USING Format Engine
30 REM                          (Milestone 7)
40 REM ============================================================
50 REM  Tests all PRINT USING format specifiers:
60 REM
62 REM  NUMERIC SPECIFIERS (100-480):
64 REM   #     Digit placeholder (right-justified)
66 REM   .     Decimal point position
68 REM   +     Leading sign (+ or -)
70 REM   -     Trailing minus for negatives (space if positive)
72 REM   $$    Floating dollar sign
74 REM   **    Asterisk fill (check protection)
76 REM   ^^^^ Exponential (scientific) notation
78 REM   0     Zero-fill leading digits
80 REM   %%    Percentage display (value * 100)
82 REM   %     Sign-only display
84 REM
86 REM  RADIX SPECIFIERS (600-730):
88 REM   H     Hexadecimal output
90 REM   O     Octal output
92 REM   B     Binary output
94 REM   I     Integer truncation output
96 REM
97 REM  STRING SPECIFIERS (800-900):
98 REM   !     First character only
99 REM   \ \   Fixed-width field (width = spaces + 2)
100 REM   &     Variable-length (full string)
102 REM
104 REM  CONTROL SPECIFIERS (1000-1100):
106 REM   nX    Insert n spaces
108 REM   ''    Literal text (quoted)
110 REM   _     Escape next format char as literal
112 REM
114 REM  IMAGE STATEMENT (1200):
116 REM   IMAGE defines a reusable format at a line number.
118 REM   PRINT USING <linenum> references it.
120 REM
122 REM  EXPECTED: Each specifier produces formatted output.
124 REM  Counter P tracks how many tests executed.
126 REM  Summary shows total passed at end.
128 REM ============================================================
130 REM
140 P = 0 : F = 0
150 REM
200 REM === NUMERIC FORMAT SPECIFIERS ===
210 REM
220 REM --- Hash digit (#) ---
230 REM  "###" formats 42 as right-justified in 3 columns: " 42"
240 PRINT USING "###"; 42
250 P = P + 1
260 REM --- Decimal point (.) ---
270 REM  "###.##" formats 3.14 as "  3.14"
280 PRINT USING "###.##"; 3.14
290 P = P + 1
300 REM --- Negative numbers ---
310 REM  "###.##" formats -42.5 as "-42.50" (leading minus)
320 PRINT USING "###.##"; -42.5
330 P = P + 1
340 REM
350 REM --- Leading sign (+) ---
360 REM  "+###.##" shows + for positive, - for negative.
370 REM  +3.14 -> "+  3.14", -42.5 -> "- 42.50"
380 PRINT USING "+###.##"; 3.14
390 P = P + 1
400 PRINT USING "+###.##"; -42.5
410 P = P + 1
420 REM
430 REM --- Trailing minus (-) ---
440 REM  "###.##-" puts minus after number for negatives,
450 REM  space after for positives.
460 PRINT USING "###.##-"; -42.5
470 P = P + 1
480 PRINT USING "###.##-"; 42.5
490 P = P + 1
500 REM
510 REM --- Floating dollar sign ($$) ---
520 REM  "$$###.##" places $ immediately before first digit.
530 REM  12.50 -> "$ 12.50"
540 PRINT USING "$$###.##"; 12.50
550 P = P + 1
560 REM
570 REM --- Asterisk fill (**) ---
580 REM  "**###.##" fills leading spaces with * (check style).
590 REM  12.50 -> "***12.50"
600 PRINT USING "**###.##"; 12.50
610 P = P + 1
620 REM
630 REM --- Exponential (^^^^) ---
640 REM  "##.##^^^^" displays in scientific notation.
650 REM  31415.9 -> "3.14E+04" (approx)
660 PRINT USING "##.##^^^^"; 31415.9
670 P = P + 1
680 REM
690 REM --- Zero fill (0) ---
700 REM  "0####" pads with leading zeros.
710 REM  42 -> "00042"
720 PRINT USING "0####"; 42
730 P = P + 1
740 REM
750 REM --- Percent (%%) ---
760 REM  "%%" multiplies value by 100 and appends %.
770 REM  0.15 -> "15%"
780 PRINT USING "%%"; 0.15
790 P = P + 1
800 REM
810 REM --- Sign display (%) ---
820 REM  "%" shows just the sign: + or -
830 PRINT USING "%"; 5
840 P = P + 1
850 PRINT USING "%"; -5
860 P = P + 1
870 REM
900 REM === RADIX SPECIFIERS ===
910 REM
920 REM --- Hex (H) ---
930 REM  "H" outputs 255 as "FF" (hexadecimal).
940 PRINT USING "H"; 255
950 P = P + 1
960 REM --- Octal (O) ---
970 REM  "O" outputs 255 as "377" (octal).
980 PRINT USING "O"; 255
990 P = P + 1
1000 REM --- Binary (B) ---
1010 REM  "B" outputs 42 as "101010" (binary).
1020 PRINT USING "B"; 42
1030 P = P + 1
1040 REM --- Integer (I) ---
1050 REM  "I" truncates 3.14 to "3" (integer display).
1060 PRINT USING "I"; 3.14
1070 P = P + 1
1080 REM
1100 REM === STRING SPECIFIERS ===
1110 REM
1120 REM --- First char (!) ---
1130 REM  "!" extracts only the first character of the string.
1140 REM  "Hello" -> "H"
1150 PRINT USING "!"; "Hello"
1160 P = P + 1
1170 REM --- Fixed field (\ \) ---
1180 REM  "\   \" creates a 5-char field (backslashes + 3 spaces).
1190 REM  "Hello World" -> "Hello" (truncated to field width).
1200 PRINT USING "\   \"; "Hello World"
1210 P = P + 1
1220 REM --- Variable field (&) ---
1230 REM  "&" outputs the full string, no truncation.
1240 PRINT USING "&"; "Hello World"
1250 P = P + 1
1260 REM
1300 REM === CONTROL SPECIFIERS ===
1310 REM
1320 REM --- Space (nX) ---
1330 REM  "3X" inserts 3 spaces before the literal text.
1340 PRINT USING "3X'Test'";
1350 P = P + 1
1360 REM --- Text literal ('') ---
1370 REM  Text between single quotes is output verbatim.
1380 REM  "'Result: '###.##" outputs "Result: " then "42.50"
1390 PRINT USING "'Result: '###.##"; 42.50
1400 P = P + 1
1410 REM --- Underscore escape (_) ---
1420 REM  "_#" outputs a literal # instead of treating it
1430 REM  as a digit placeholder. "_#_#_#" outputs "###".
1440 PRINT USING "_#_#_#";
1450 P = P + 1
1460 REM
1500 REM === IMAGE STATEMENT ===
1510 REM  IMAGE defines a format string at a line number.
1520 REM  Syntax: IMAGE <format>
1530 REM  Referenced by: PRINT USING <linenum>; <args>
1540 REM  "3D.2D,2X,5A" = 3-digit.2-decimal, 2 spaces, 5-char string
1550 IMAGE 3D.2D,2X,5A
1560 PRINT USING 1550; 3.14; "Hello"
1570 P = P + 1
1580 REM
1600 PRINT
1610 PRINT "PRINT USING tests: "; P; " passed"
1620 END
