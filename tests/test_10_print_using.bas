10 REM --- BASIC++ Test Suite ---
20 REM Part 10: PRINT USING Format Engine (Milestone 7)
30 REM Tests all PRINT USING format specifiers
40 REM
50 P = 0 : F = 0
60 REM
100 REM === NUMERIC FORMAT SPECIFIERS ===
110 REM --- Hash digit (#) ---
120 PRINT USING "###"; 42
130 P = P + 1
140 REM --- Decimal point (.) ---
150 PRINT USING "###.##"; 3.14
160 P = P + 1
170 REM --- Negative numbers ---
180 PRINT USING "###.##"; -42.5
190 P = P + 1
200 REM
210 REM --- Leading sign (+) ---
220 PRINT USING "+###.##"; 3.14
230 P = P + 1
240 PRINT USING "+###.##"; -42.5
250 P = P + 1
260 REM
270 REM --- Trailing minus (-) ---
280 PRINT USING "###.##-"; -42.5
290 P = P + 1
300 PRINT USING "###.##-"; 42.5
310 P = P + 1
320 REM
330 REM --- Floating dollar sign ($$) ---
340 PRINT USING "$$###.##"; 12.50
350 P = P + 1
360 REM
370 REM --- Asterisk fill (**) ---
380 PRINT USING "**###.##"; 12.50
390 P = P + 1
400 REM
410 REM --- Exponential (^^^^) ---
420 PRINT USING "##.##^^^^"; 31415.9
430 P = P + 1
440 REM
450 REM --- Zero fill (0) ---
460 PRINT USING "0####"; 42
470 P = P + 1
480 REM
490 REM --- Percent (%%) ---
500 PRINT USING "%%"; 0.15
510 P = P + 1
520 REM
530 REM --- Sign display (%) ---
540 PRINT USING "%"; 5
550 P = P + 1
560 PRINT USING "%"; -5
570 P = P + 1
580 REM
600 REM === RADIX SPECIFIERS ===
610 REM --- Hex (H) ---
620 PRINT USING "H"; 255
630 P = P + 1
640 REM --- Octal (O) ---
650 PRINT USING "O"; 255
660 P = P + 1
670 REM --- Binary (B) ---
680 PRINT USING "B"; 42
690 P = P + 1
700 REM --- Integer (I) ---
710 PRINT USING "I"; 3.14
720 P = P + 1
730 REM
800 REM === STRING SPECIFIERS ===
810 REM --- First char (!) ---
820 PRINT USING "!"; "Hello"
830 P = P + 1
840 REM --- Fixed field (\ \) ---
850 PRINT USING "\   \"; "Hello World"
860 P = P + 1
870 REM --- Variable field (&) ---
880 PRINT USING "&"; "Hello World"
890 P = P + 1
900 REM
1000 REM === CONTROL SPECIFIERS ===
1010 REM --- Space (X) ---
1020 PRINT USING "3X'Test'"; 
1030 P = P + 1
1040 REM --- Text literal ('') ---
1050 PRINT USING "'Result: '###.##"; 42.50
1060 P = P + 1
1070 REM --- Underscore escape (_) ---
1080 PRINT USING "_#_#_#"; 
1090 P = P + 1
1100 REM
1200 REM === IMAGE STATEMENT ===
1210 IMAGE 3D.2D,2X,5A
1220 PRINT USING 1210; 3.14; "Hello"
1230 P = P + 1
1240 REM
1300 PRINT
1310 PRINT "PRINT USING tests: "; P; " passed"
1320 END
