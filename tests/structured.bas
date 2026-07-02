10 REM ============================================================
20 REM  TEST: structured.bas - Structured Programming (Milestone 9)
30 REM ============================================================
40 REM  Tests SUB/FUNCTION features, scoping, and string SELECT:
50 REM
52 REM  Test 1  (100): Basic SUB call with parameter
54 REM  Test 2  (200): FUNCTION with return value
56 REM  Test 3  (300): Recursive factorial (6! = 720)
58 REM  Test 3b (350): Recursive Fibonacci (fib(10) = 55)
60 REM  Test 4  (400): Local scope isolation (X unchanged)
62 REM  Test 5  (500): LOCAL variable save/restore
64 REM  Test 6  (600): STATIC variable persistence across calls
66 REM  Test 7  (700): SHARED variable propagation to caller
68 REM  Test 8  (800): SELECT CASE string exact match
70 REM  Test 9  (900): SELECT CASE string range (TO)
72 REM  Test 10 (1000): SELECT CASE string IS comparison
74 REM
76 REM  WHAT CAN BE CHANGED:
78 REM   - Test values and parameter values
79 REM   - Additional SUB/FUNCTION definitions
80 REM
81 REM  WHAT CANNOT BE CHANGED:
82 REM   - OPTION DIALECT QBASIC is required for SUB/FUNCTION
83 REM   - Do NOT use single-line IF/THEN/ELSE for FUNCTION
84 REM     return value assignment (parser limitation)
85 REM   - Use multi-line IF with GOTO for branched returns
86 REM
87 REM  WHAT TO EXPECT:
88 REM   - All tests should print PASS
89 REM   - Summary shows total passed/failed count
90 REM
91 REM  TROUBLESHOOTING:
92 REM   - WHAT? on SUB/FUNCTION: ensure OPTION DIALECT QBASIC
93 REM   - ASSERTION FAILED in Factorial/Fibonacci: ensure
94 REM     functions use GOTO-based branching, not ELSE
95 REM   - LOCAL/STATIC/SHARED errors: verify QBASIC dialect
96 REM ============================================================
98 OPTION DIALECT QBASIC
99 REM
100 P = 0 : F = 0
110 REM
200 REM --- Test 1: Basic SUB call ---
210 REM  AddOne(5) prints "6" inside the SUB.
220 CALL AddOne(5)
230 PRINT "Test 1: SUB call done"
240 P = P + 1
250 REM
300 REM --- Test 2: FUNCTION with return ---
310 REM  Doubler(7) returns 7*2 = 14.
320 PRINT "Doubler(7) = "; Doubler(7)
330 IF Doubler(7) = 14 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
340 REM
400 REM --- Test 3: Recursive factorial ---
410 REM  Factorial(6) = 6*5*4*3*2*1 = 720.
420 REM  Tests recursive FUNCTION calls with base case (N<=1).
430 PRINT "Factorial(6) = "; Factorial(6)
440 IF Factorial(6) = 720 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
450 REM
500 REM --- Test 3b: Recursive Fibonacci ---
510 REM  Fibonacci(10) = 55. Uses double-recursive formula:
520 REM  fib(N) = fib(N-1) + fib(N-2), base fib(0)=0, fib(1)=1.
530 PRINT "Fibonacci(10) = "; Fibonacci(10)
540 IF Fibonacci(10) = 55 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
550 REM
600 REM --- Test 4: Local scope isolation ---
610 REM  Set X=999 in main, call SetX which sets X=42 locally.
620 REM  After return, main's X should still be 999 because
630 REM  SUB variables are isolated from the caller.
640 X = 999
650 CALL SetX
660 PRINT "X after SetX = "; X
670 IF X = 999 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
680 REM
700 REM --- Test 5: LOCAL variable save/restore ---
710 REM  Set A=100 in main. UseLocal declares LOCAL A, sets
720 REM  A=42 inside. After return, main's A must be 100 again.
730 A = 100
740 CALL UseLocal
750 PRINT "A after UseLocal = "; A
760 IF A = 100 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
770 REM
800 REM --- Test 6: STATIC variable persistence ---
810 REM  GetCount uses STATIC C, increments C each call.
820 REM  Three calls should return 1, 2, 3 respectively.
830 A = GetCount(0)
840 A = GetCount(0)
850 A = GetCount(0)
860 PRINT "Static count = "; A
870 IF A = 3 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
880 REM
900 REM --- Test 7: SHARED variable propagation ---
910 REM  SetShared declares SHARED G, sets G=42.
920 REM  After return, main's G should be 42 because SHARED
930 REM  gives the SUB direct access to the caller's variable.
940 G = 0
950 CALL SetShared
960 PRINT "G after SetShared = "; G
970 IF G = 42 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
980 REM
1000 REM --- Test 8: SELECT CASE string exact match ---
1010 REM  A$="HELLO" should match CASE "HELLO" (R=2).
1020 R = 0
1030 A$ = "HELLO"
1040 SELECT CASE A$
1050   CASE "GOODBYE"
1060     R = 1
1070   CASE "HELLO"
1080     R = 2
1090   CASE ELSE
1100     R = 3
1110 END SELECT
1120 PRINT "SELECT string match = "; R
1130 IF R = 2 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
1140 REM
1200 REM --- Test 9: SELECT CASE string range ---
1210 REM  A$="DOG" should match CASE "D" TO "F" (R=2)
1220 REM  because "D" <= "DOG" <= "F" lexicographically.
1230 R = 0
1240 A$ = "DOG"
1250 SELECT CASE A$
1260   CASE "A" TO "C"
1270     R = 1
1280   CASE "D" TO "F"
1290     R = 2
1300   CASE ELSE
1310     R = 3
1320 END SELECT
1330 PRINT "SELECT string range = "; R
1340 IF R = 2 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
1350 REM
1400 REM --- Test 10: SELECT CASE string IS ---
1410 REM  A$="ZEBRA" should match CASE IS >= "M" (R=2)
1420 REM  because "ZEBRA" >= "M" lexicographically.
1430 R = 0
1440 A$ = "ZEBRA"
1450 SELECT CASE A$
1460   CASE IS < "M"
1470     R = 1
1480   CASE IS >= "M"
1490     R = 2
1500 END SELECT
1510 PRINT "SELECT string IS = "; R
1520 IF R = 2 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
1530 REM
4900 REM --- Summary ---
4910 PRINT
4920 PRINT "Results: "; P; " passed, "; F; " failed"
4930 END
4940 REM
5000 REM ============================================================
5010 REM  SUB AND FUNCTION DEFINITIONS
5020 REM  (Must be after END to avoid execution as mainline code)
5030 REM ============================================================
5040 REM
5100 REM --- AddOne: prints N+1 ---
5110 SUB AddOne(N)
5120   PRINT "AddOne: "; N; " + 1 = "; N + 1
5130 END SUB
5140 REM
5200 REM --- Doubler: returns N*2 ---
5210 FUNCTION Doubler(N)
5220   Doubler = N * 2
5230 END FUNCTION
5240 REM
5300 REM --- Factorial: recursive N! ---
5310 REM  Base case: N<=1 returns 1.
5320 REM  Recursive case: N * Factorial(N-1).
5330 FUNCTION Factorial(N)
5335   IF N <= 1 THEN GOTO 5345
5340   Factorial = N * Factorial(N - 1) : GOTO 5348
5345   Factorial = 1
5348 END FUNCTION
5360 REM
5400 REM --- Fibonacci: recursive fib(N) ---
5410 REM  Base case: N<=1 returns N (fib(0)=0, fib(1)=1).
5420 REM  Recursive case: fib(N-1) + fib(N-2).
5430 FUNCTION Fibonacci(N)
5435   IF N <= 1 THEN GOTO 5445
5440   Fibonacci = Fibonacci(N-1) + Fibonacci(N-2) : GOTO 5448
5445   Fibonacci = N
5448 END FUNCTION
5460 REM
5500 REM --- SetX: sets X=42 in SUB scope ---
5510 REM  Main's X should NOT be affected (scope isolation).
5520 SUB SetX
5530   X = 42
5540 END SUB
5550 REM
5600 REM --- UseLocal: declares LOCAL A ---
5610 REM  LOCAL saves caller's A, sets A=42 inside,
5620 REM  restores caller's A on END SUB.
5630 SUB UseLocal
5640   LOCAL A
5650   A = 42
5660   PRINT "A inside UseLocal = "; A
5670 END SUB
5680 REM
5700 REM --- Counter: STATIC demo (not called in tests) ---
5710 REM  STATIC C persists across calls.
5720 SUB Counter
5730   STATIC C
5740   C = C + 1
5750   PRINT "Counter C = "; C
5760 END SUB
5770 REM
5800 REM --- GetCount: STATIC with return ---
5810 REM  Each call increments C and returns the new count.
5820 FUNCTION GetCount(Dummy)
5830   STATIC C
5840   C = C + 1
5850   GetCount = C
5860 END FUNCTION
5870 REM
5900 REM --- SetShared: SHARED variable propagation ---
5910 REM  SHARED G gives this SUB access to the caller's G.
5920 REM  Setting G=42 here changes the caller's G directly.
5930 SUB SetShared
5940   SHARED G
5950   G = 42
5960 END SUB
