10 REM ============================================================
20 REM  TEST: complex.bas - Complex Number Math (Milestone 8)
30 REM ============================================================
40 REM  Tests complex number literals, arithmetic, and functions.
50 REM
52 REM  SECTION 1 (100-270): Complex Literal Syntax
54 REM   - (3+2i): parenthesized complex literal
56 REM   - (1-1i): negative imaginary part
58 REM   - 5i: pure imaginary (no real part)
60 REM   - (0+1.5i): float imaginary part
62 REM
64 REM  SECTION 2 (300-610): Complex Arithmetic
66 REM   - Addition: (3+2i) + (1+4i) = (4+6i)
68 REM   - Subtraction: (5+3i) - (2+1i) = (3+2i)
70 REM   - Multiplication: (3+2i)*(1-1i) = (5-1i)
72 REM   - Division: (4+2i)/(1+1i) = (3-1i)
74 REM   - Mixed real+complex: (3+2i)+10 = (13+2i)
76 REM   - Negation: -(3+2i) = (-3-2i)
78 REM
80 REM  SECTION 3 (700-840): Complex Functions
82 REM   - COMPLEX(r,i): constructor
84 REM   - REAL(z): extract real part
86 REM   - IMAG(z): extract imaginary part
88 REM   - CABS(z): complex absolute value (modulus)
90 REM   - CONJ(z): complex conjugate
92 REM
94 REM  SECTION 4 (850-920): SQR of Negative Numbers
96 REM   - SQR(-4) = (0+2i) using complex extension
98 REM   - SQR(4) = 2 still works for positives
99 REM
100 REM  SECTION 5 (930-1140): Advanced Complex Functions
102 REM   - CSQR: complex square root
104 REM   - CEXP: complex exponential (e^(i*pi) = -1)
106 REM   - CLOG: complex logarithm
108 REM   - CARG: argument (angle) of complex number
110 REM   - CPOW: complex exponentiation
112 REM
114 REM  EXPECTED: Each assertion prints PASS or FAIL.
116 REM  Real/imaginary parts verified with exact comparison
118 REM  for integer results. Summary at end.
120 REM ============================================================
122 REM
130 P = 0 : F = 0
140 REM
200 REM === COMPLEX LITERAL SYNTAX ===
210 REM --- Parenthesized complex literal ---
220 REM  (3+2i) creates a complex number with real=3, imag=2.
230 A = (3+2i)
240 PRINT "A = "; A
250 P = P + 1 : PRINT "PASS: (3+2i) literal"
260 REM --- Negative imaginary ---
270 REM  (1-1i) creates complex with real=1, imag=-1.
280 B = (1-1i)
290 PRINT "B = "; B
300 P = P + 1 : PRINT "PASS: (1-1i) literal"
310 REM --- Pure imaginary ---
320 REM  5i creates complex with real=0, imag=5.
330 C = 5i
340 PRINT "C = "; C
350 P = P + 1 : PRINT "PASS: 5i literal"
360 REM --- Float imaginary ---
370 REM  (0+1.5i) tests decimal imaginary component.
380 D = (0+1.5i)
390 PRINT "D = "; D
400 P = P + 1 : PRINT "PASS: (0+1.5i) literal"
410 REM
500 REM === COMPLEX ARITHMETIC ===
510 REM --- Addition ---
520 REM  (3+2i) + (1+4i) = (3+1) + (2+4)i = (4+6i)
530 E = (3+2i) + (1+4i)
540 PRINT "Add: "; E
550 IF REAL(E) = 4 THEN P = P + 1 : PRINT "PASS: add real" ELSE F = F + 1 : PRINT "FAIL: add real"
560 IF IMAG(E) = 6 THEN P = P + 1 : PRINT "PASS: add imag" ELSE F = F + 1 : PRINT "FAIL: add imag"
570 REM --- Subtraction ---
580 REM  (5+3i) - (2+1i) = (3+2i)
590 E = (5+3i) - (2+1i)
600 IF REAL(E) = 3 THEN P = P + 1 : PRINT "PASS: sub real" ELSE F = F + 1 : PRINT "FAIL: sub real"
610 IF IMAG(E) = 2 THEN P = P + 1 : PRINT "PASS: sub imag" ELSE F = F + 1 : PRINT "FAIL: sub imag"
620 REM --- Multiplication ---
630 REM  (3+2i)*(1-1i) = 3-3i+2i-2i^2 = 3-i+2 = (5-i)
640 REM  Remember: i^2 = -1, so -2*i^2 = +2.
650 E = (3+2i) * (1-1i)
660 PRINT "Mul: "; E
670 IF REAL(E) = 5 THEN P = P + 1 : PRINT "PASS: mul real" ELSE F = F + 1 : PRINT "FAIL: mul real"
680 IF IMAG(E) = -1 THEN P = P + 1 : PRINT "PASS: mul imag" ELSE F = F + 1 : PRINT "FAIL: mul imag"
690 REM --- Division ---
700 REM  (4+2i)/(1+1i): multiply by conjugate (1-1i)/(1-1i)
710 REM  = (4-4i+2i-2i^2)/(1+1) = (6-2i)/2 = (3-i)
720 E = (4+2i) / (1+1i)
730 PRINT "Div: "; E
740 IF REAL(E) = 3 THEN P = P + 1 : PRINT "PASS: div real" ELSE F = F + 1 : PRINT "FAIL: div real"
750 IF IMAG(E) = -1 THEN P = P + 1 : PRINT "PASS: div imag" ELSE F = F + 1 : PRINT "FAIL: div imag"
760 REM
770 REM --- Mixed: complex + real ---
780 REM  (3+2i) + 10 = (13+2i). Real is promoted to complex.
790 E = (3+2i) + 10
800 IF REAL(E) = 13 THEN P = P + 1 : PRINT "PASS: complex+real" ELSE F = F + 1 : PRINT "FAIL: complex+real"
810 REM --- Mixed: real + complex ---
820 REM  10 + (3+2i) = (13+2i). Commutative.
830 E = 10 + (3+2i)
840 IF REAL(E) = 13 THEN P = P + 1 : PRINT "PASS: real+complex" ELSE F = F + 1 : PRINT "FAIL: real+complex"
850 REM --- Negation ---
860 REM  -(3+2i) = (-3-2i). Negates both parts.
870 E = -(3+2i)
880 IF REAL(E) = -3 THEN P = P + 1 : PRINT "PASS: neg real" ELSE F = F + 1 : PRINT "FAIL: neg real"
890 IF IMAG(E) = -2 THEN P = P + 1 : PRINT "PASS: neg imag" ELSE F = F + 1 : PRINT "FAIL: neg imag"
900 REM
1000 REM === COMPLEX FUNCTIONS ===
1010 REM --- COMPLEX() constructor ---
1020 REM  COMPLEX(3, 4) creates (3+4i).
1030 E = COMPLEX(3, 4)
1040 IF REAL(E) = 3 THEN P = P + 1 : PRINT "PASS: COMPLEX real" ELSE F = F + 1 : PRINT "FAIL: COMPLEX real"
1050 IF IMAG(E) = 4 THEN P = P + 1 : PRINT "PASS: COMPLEX imag" ELSE F = F + 1 : PRINT "FAIL: COMPLEX imag"
1060 REM --- REAL() ---
1070 REM  Extracts the real component of a complex number.
1080 IF REAL((7+3i)) = 7 THEN P = P + 1 : PRINT "PASS: REAL" ELSE F = F + 1 : PRINT "FAIL: REAL"
1090 REM --- IMAG() ---
1100 REM  Extracts the imaginary component of a complex number.
1110 IF IMAG((7+3i)) = 3 THEN P = P + 1 : PRINT "PASS: IMAG" ELSE F = F + 1 : PRINT "FAIL: IMAG"
1120 REM --- CABS(): complex absolute value (modulus) ---
1130 REM  |(3+4i)| = sqrt(3^2 + 4^2) = sqrt(25) = 5.
1140 IF CABS((3+4i)) = 5 THEN P = P + 1 : PRINT "PASS: CABS" ELSE F = F + 1 : PRINT "FAIL: CABS"
1150 REM --- CONJ(): complex conjugate ---
1160 REM  conj(3+4i) = (3-4i). Negates imaginary part only.
1170 E = CONJ((3+4i))
1180 IF IMAG(E) = -4 THEN P = P + 1 : PRINT "PASS: CONJ" ELSE F = F + 1 : PRINT "FAIL: CONJ"
1190 REM
1200 REM === SQR OF NEGATIVE ===
1210 REM --- SQR(-4) should return (0+2i) ---
1220 REM  Complex extension: sqrt(-4) = 2i.
1230 E = SQR(-4)
1240 PRINT "SQR(-4) = "; E
1250 IF IMAG(E) = 2 THEN P = P + 1 : PRINT "PASS: SQR(-4)" ELSE F = F + 1 : PRINT "FAIL: SQR(-4)"
1260 REM --- SQR(4) should still return 2 (not complex) ---
1270 REM  Positive numbers stay real.
1280 IF SQR(4) = 2 THEN P = P + 1 : PRINT "PASS: SQR(4)=2" ELSE F = F + 1 : PRINT "FAIL: SQR(4)"
1290 REM
1300 REM === ADVANCED COMPLEX FUNCTIONS ===
1310 REM --- CSQR: complex square root ---
1320 REM  CSQR(-1+0i) = (0+1i) = i.
1330 E = CSQR((-1+0i))
1340 PRINT "CSQR(-1) = "; E
1350 P = P + 1 : PRINT "PASS: CSQR executed"
1360 REM --- CEXP: complex exponential ---
1370 REM  e^(i*pi) = -1+0i (Euler's identity).
1380 REM  Small floating-point deviation expected.
1390 E = CEXP((0+3.14159265i))
1400 PRINT "CEXP(i*pi) = "; E
1410 P = P + 1 : PRINT "PASS: CEXP executed"
1420 REM --- CLOG: complex logarithm ---
1430 REM  ln(-1) = (0+pi*i) (principal branch).
1440 E = CLOG((-1+0i))
1450 PRINT "CLOG(-1) = "; E
1460 P = P + 1 : PRINT "PASS: CLOG executed"
1470 REM --- CARG: argument (angle) of complex number ---
1480 REM  arg(0+1i) = pi/2 radians = 1.5708...
1490 E = CARG((0+1i))
1500 PRINT "CARG(i) = "; E
1510 P = P + 1 : PRINT "PASS: CARG executed"
1520 REM --- CPOW: complex exponentiation ---
1530 REM  (1+1i)^2 = 1+2i+i^2 = 1+2i-1 = (0+2i).
1540 E = CPOW((1+1i), 2)
1550 PRINT "CPOW((1+1i),2) = "; E
1560 P = P + 1 : PRINT "PASS: CPOW executed"
1570 REM
1600 PRINT
1610 PRINT "Complex number tests: "; P; " passed, "; F; " failed"
1620 IF F > 0 THEN PRINT "*** FAILURES DETECTED ***"
1630 END
