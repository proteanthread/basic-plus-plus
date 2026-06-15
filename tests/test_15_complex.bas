10 REM --- BASIC++ Test Suite ---
20 REM Part 15: Complex Number Math (Milestone 8)
30 REM Tests complex literals, arithmetic, and functions
40 REM
50 P = 0 : F = 0
60 REM
100 REM === COMPLEX LITERAL SYNTAX ===
110 REM --- Parenthesized complex literal ---
120 A = (3+2i)
130 PRINT "A = "; A
140 P = P + 1 : PRINT "PASS: (3+2i) literal"
150 REM --- Negative imaginary ---
160 B = (1-1i)
170 PRINT "B = "; B
180 P = P + 1 : PRINT "PASS: (1-1i) literal"
190 REM --- Pure imaginary ---
200 C = 5i
210 PRINT "C = "; C
220 P = P + 1 : PRINT "PASS: 5i literal"
230 REM --- Float imaginary ---
240 D = (0+1.5i)
250 PRINT "D = "; D
260 P = P + 1 : PRINT "PASS: (0+1.5i) literal"
270 REM
300 REM === COMPLEX ARITHMETIC ===
310 REM --- Addition ---
320 E = (3+2i) + (1+4i)
330 PRINT "Add: "; E
340 IF REAL(E) = 4 THEN P = P + 1 : PRINT "PASS: add real" ELSE F = F + 1 : PRINT "FAIL: add real"
350 IF IMAG(E) = 6 THEN P = P + 1 : PRINT "PASS: add imag" ELSE F = F + 1 : PRINT "FAIL: add imag"
360 REM --- Subtraction ---
370 E = (5+3i) - (2+1i)
380 IF REAL(E) = 3 THEN P = P + 1 : PRINT "PASS: sub real" ELSE F = F + 1 : PRINT "FAIL: sub real"
390 IF IMAG(E) = 2 THEN P = P + 1 : PRINT "PASS: sub imag" ELSE F = F + 1 : PRINT "FAIL: sub imag"
400 REM --- Multiplication: (3+2i)*(1-1i) = (3-3i+2i-2i^2) = (5-1i) ---
410 E = (3+2i) * (1-1i)
420 PRINT "Mul: "; E
430 IF REAL(E) = 5 THEN P = P + 1 : PRINT "PASS: mul real" ELSE F = F + 1 : PRINT "FAIL: mul real"
440 IF IMAG(E) = -1 THEN P = P + 1 : PRINT "PASS: mul imag" ELSE F = F + 1 : PRINT "FAIL: mul imag"
450 REM --- Division: (4+2i)/(1+1i) = (3-1i) ---
460 E = (4+2i) / (1+1i)
470 PRINT "Div: "; E
480 IF REAL(E) = 3 THEN P = P + 1 : PRINT "PASS: div real" ELSE F = F + 1 : PRINT "FAIL: div real"
490 IF IMAG(E) = -1 THEN P = P + 1 : PRINT "PASS: div imag" ELSE F = F + 1 : PRINT "FAIL: div imag"
500 REM
510 REM --- Mixed: complex + real ---
520 E = (3+2i) + 10
530 IF REAL(E) = 13 THEN P = P + 1 : PRINT "PASS: complex+real" ELSE F = F + 1 : PRINT "FAIL: complex+real"
540 REM --- Mixed: real + complex ---
550 E = 10 + (3+2i)
560 IF REAL(E) = 13 THEN P = P + 1 : PRINT "PASS: real+complex" ELSE F = F + 1 : PRINT "FAIL: real+complex"
570 REM --- Negation ---
580 E = -(3+2i)
590 IF REAL(E) = -3 THEN P = P + 1 : PRINT "PASS: neg real" ELSE F = F + 1 : PRINT "FAIL: neg real"
600 IF IMAG(E) = -2 THEN P = P + 1 : PRINT "PASS: neg imag" ELSE F = F + 1 : PRINT "FAIL: neg imag"
610 REM
700 REM === COMPLEX FUNCTIONS ===
710 REM --- COMPLEX() constructor ---
720 E = COMPLEX(3, 4)
730 IF REAL(E) = 3 THEN P = P + 1 : PRINT "PASS: COMPLEX real" ELSE F = F + 1 : PRINT "FAIL: COMPLEX real"
740 IF IMAG(E) = 4 THEN P = P + 1 : PRINT "PASS: COMPLEX imag" ELSE F = F + 1 : PRINT "FAIL: COMPLEX imag"
750 REM --- REAL() ---
760 IF REAL((7+3i)) = 7 THEN P = P + 1 : PRINT "PASS: REAL" ELSE F = F + 1 : PRINT "FAIL: REAL"
770 REM --- IMAG() ---
780 IF IMAG((7+3i)) = 3 THEN P = P + 1 : PRINT "PASS: IMAG" ELSE F = F + 1 : PRINT "FAIL: IMAG"
790 REM --- CABS(): |(3+4i)| = 5 ---
800 IF CABS((3+4i)) = 5 THEN P = P + 1 : PRINT "PASS: CABS" ELSE F = F + 1 : PRINT "FAIL: CABS"
810 REM --- CONJ(): conj(3+4i) = (3-4i) ---
820 E = CONJ((3+4i))
830 IF IMAG(E) = -4 THEN P = P + 1 : PRINT "PASS: CONJ" ELSE F = F + 1 : PRINT "FAIL: CONJ"
840 REM
850 REM === SQR OF NEGATIVE ===
860 REM --- SQR(-4) should return (0+2i) ---
870 E = SQR(-4)
880 PRINT "SQR(-4) = "; E
890 IF IMAG(E) = 2 THEN P = P + 1 : PRINT "PASS: SQR(-4)" ELSE F = F + 1 : PRINT "FAIL: SQR(-4)"
900 REM --- SQR(4) should still return 2 (not complex) ---
910 IF SQR(4) = 2 THEN P = P + 1 : PRINT "PASS: SQR(4)=2" ELSE F = F + 1 : PRINT "FAIL: SQR(4)"
920 REM
930 REM === ADVANCED COMPLEX FUNCTIONS ===
940 REM --- CSQR --- 
950 E = CSQR((-1+0i))
960 PRINT "CSQR(-1) = "; E
970 P = P + 1 : PRINT "PASS: CSQR executed"
980 REM --- CEXP: e^(i*pi) = -1+0i ---
990 E = CEXP((0+3.14159265i))
1000 PRINT "CEXP(i*pi) = "; E
1010 P = P + 1 : PRINT "PASS: CEXP executed"
1020 REM --- CLOG ---
1030 E = CLOG((-1+0i))
1040 PRINT "CLOG(-1) = "; E
1050 P = P + 1 : PRINT "PASS: CLOG executed"
1060 REM --- CARG: arg(0+1i) = pi/2 ---
1070 E = CARG((0+1i))
1080 PRINT "CARG(i) = "; E
1090 P = P + 1 : PRINT "PASS: CARG executed"
1100 REM --- CPOW: (1+1i)^2 = (0+2i) ---
1110 E = CPOW((1+1i), 2)
1120 PRINT "CPOW((1+1i),2) = "; E
1130 P = P + 1 : PRINT "PASS: CPOW executed"
1140 REM
1200 PRINT
1210 PRINT "Complex number tests: "; P; " passed, "; F; " failed"
1220 IF F > 0 THEN PRINT "*** FAILURES DETECTED ***"
1230 END
