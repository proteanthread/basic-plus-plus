10 REM === Milestone 9: Structured Programming Tests ===
20 P = 0 : F = 0
100 REM --- Test 1: Basic SUB call ---
110 CALL AddOne(5)
120 PRINT "Test 1: SUB call done"
130 P = P + 1
200 REM --- Test 2: FUNCTION with return ---
210 PRINT "Doubler(7) = "; Doubler(7)
220 IF Doubler(7) = 14 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
300 REM --- Test 3: Recursive factorial ---
310 PRINT "Factorial(6) = "; Factorial(6)
320 IF Factorial(6) = 720 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
350 REM --- Test 3b: Recursive Fibonacci ---
360 PRINT "Fibonacci(10) = "; Fibonacci(10)
370 IF Fibonacci(10) = 55 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
400 REM --- Test 4: Local scope isolation ---
410 X = 999
420 CALL SetX
430 PRINT "X after SetX = "; X
440 IF X = 999 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
500 REM --- Test 5: LOCAL variable save/restore ---
510 A = 100
520 CALL UseLocal
530 PRINT "A after UseLocal = "; A
540 IF A = 100 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
600 REM --- Test 6: STATIC variable persistence ---
610 A = GetCount(0)
620 A = GetCount(0)
630 A = GetCount(0)
640 PRINT "Static count = "; A
650 IF A = 3 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
700 REM --- Test 7: SHARED variable propagation ---
710 G = 0
720 CALL SetShared
730 PRINT "G after SetShared = "; G
740 IF G = 42 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
800 REM --- Test 8: SELECT CASE string exact match ---
810 R = 0
820 A$ = "HELLO"
830 SELECT CASE A$
840   CASE "GOODBYE"
850     R = 1
860   CASE "HELLO"
870     R = 2
880   CASE ELSE
890     R = 3
895 END SELECT
896 PRINT "SELECT string match = "; R
897 IF R = 2 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
900 REM --- Test 9: SELECT CASE string range ---
910 R = 0
920 A$ = "DOG"
930 SELECT CASE A$
940   CASE "A" TO "C"
950     R = 1
960   CASE "D" TO "F"
970     R = 2
980   CASE ELSE
990     R = 3
995 END SELECT
996 PRINT "SELECT string range = "; R
997 IF R = 2 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
1000 REM --- Test 10: SELECT CASE string IS ---
1010 R = 0
1020 A$ = "ZEBRA"
1030 SELECT CASE A$
1040   CASE IS < "M"
1050     R = 1
1060   CASE IS >= "M"
1070     R = 2
1095 END SELECT
1096 PRINT "SELECT string IS = "; R
1097 IF R = 2 THEN P=P+1 : PRINT "PASS" ELSE F=F+1 : PRINT "FAIL"
4900 REM --- Summary ---
4910 PRINT
4920 PRINT "Results: "; P; " passed, "; F; " failed"
4930 END
5000 SUB AddOne(N)
5010   PRINT "AddOne: "; N; " + 1 = "; N + 1
5020 END SUB
5100 FUNCTION Doubler(N)
5110   Doubler = N * 2
5120 END FUNCTION
5200 FUNCTION Factorial(N)
5210   IF N <= 1 THEN Factorial = 1 ELSE Factorial = N * Factorial(N - 1)
5220 END FUNCTION
5300 FUNCTION Fibonacci(N)
5310   IF N <= 1 THEN Fibonacci = N ELSE Fibonacci = Fibonacci(N-1) + Fibonacci(N-2)
5320 END FUNCTION
5400 SUB SetX
5410   X = 42
5420 END SUB
5500 SUB UseLocal
5510   LOCAL A
5520   A = 42
5530   PRINT "A inside UseLocal = "; A
5540 END SUB
5600 SUB Counter
5610   STATIC C
5620   C = C + 1
5630   PRINT "Counter C = "; C
5640 END SUB
5700 FUNCTION GetCount(Dummy)
5710   STATIC C
5720   C = C + 1
5730   GetCount = C
5740 END FUNCTION
5800 SUB SetShared
5810   SHARED G
5820   G = 42
5830 END SUB
