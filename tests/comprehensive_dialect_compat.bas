10 REM ============================================================
20 REM  TEST: comprehensive_dialect_compat.bas
30 REM ============================================================
40 REM  Tests complex numbers, postfix modifiers, virtual arrays,
50 REM  and scoped procedures.
60 REM
70 REM  WHAT TO EXPECT:
80 REM   - All tests print PASS
90 REM   - Summary shows total passed/failed count
100 REM ============================================================
110 P = 0 : F = 0
120 REM
130 REM --- Test 1: Postfix Modifiers (IF/UNLESS) ---
140 X = 5 : T1 = 0
150 T1 = 1 IF X = 5
160 IF T1 = 1 THEN PRINT "Test 1a: PASS" : P = P + 1 ELSE PRINT "Test 1a: FAIL" : F = F + 1
170 T2 = 0
180 T2 = 1 UNLESS X = 5
190 IF T2 = 0 THEN PRINT "Test 1b: PASS" : P = P + 1 ELSE PRINT "Test 1b: FAIL" : F = F + 1
200 REM
210 REM --- Test 2: Postfix Modifiers (WHILE/UNTIL) ---
220 Y = 1
235 Y = Y + 1 WHILE Y < 5
240 IF Y = 5 THEN PRINT "Test 2a: PASS" : P = P + 1 ELSE PRINT "Test 2a: FAIL" : F = F + 1
250 Z = 1
265 Z = Z + 1 UNTIL Z = 5
270 IF Z = 5 THEN PRINT "Test 2b: PASS" : P = P + 1 ELSE PRINT "Test 2b: FAIL" : F = F + 1
280 REM
290 REM --- Test 3: Postfix Modifiers (FOR) ---
300 W = 0
315 W = W + I FOR I = 1 TO 4
320 IF W = 10 THEN PRINT "Test 3: PASS" : P = P + 1 ELSE PRINT "Test 3: FAIL" : F = F + 1
330 REM
340 REM --- Test 4: Complex Numbers (TymShare Super BASIC) ---
350 C = COMPLEX(3, 4)
360 R = REAL(C)
370 IM = IMAG(C)
380 IF R = 3 AND IM = 4 THEN PRINT "Test 4a: PASS" : P = P + 1 ELSE PRINT "Test 4a: FAIL" : F = F + 1
390 C2 = COMPLEX(1, 2)
400 C3 = C + C2
410 R3 = REAL(C3)
420 IM3 = IMAG(C3)
430 IF R3 = 4 AND IM3 = 6 THEN PRINT "Test 4b: PASS" : P = P + 1 ELSE PRINT "Test 4b: FAIL" : F = F + 1
440 REM
450 REM --- Test 5: DEC Virtual Arrays (Hybrid File-Backed Cache) ---
460 OPEN "test_va.bin" FOR RANDOM AS #1
470 DIM #1, V%(5) = 2
480 V%(0) = 42
490 V%(1) = 99
500 CLOSE #1
510 OPEN "test_va.bin" FOR RANDOM AS #2
520 DIM #2, V2%(5) = 2
530 V0 = V2%(0)
540 V1 = V2%(1)
550 CLOSE #2
560 IF V0 = 42 AND V1 = 99 THEN PRINT "Test 5: PASS" : P = P + 1 ELSE PRINT "Test 5: FAIL" : F = F + 1
570 REM
580 REM --- Test 6: Procedure Local Scoping (QBasic/ECMA-116) ---
590 GLOB = 100
600 CALL ScopedSub(glob)
610 IF GLOB = 100 THEN PRINT "Test 6: PASS" : P = P + 1 ELSE PRINT "Test 6: FAIL" : F = F + 1
615 REM
616 REM --- Test 7: Context-aware keyword-as-variable rewrite ---
617 UNLESS = 42
618 COMPLEX = 123
619 REAL = 456
620 IMAG = 789
621 A = UNLESS + COMPLEX
622 B = REAL + IMAG
623 IF A = 165 AND B = 1245 THEN PRINT "Test 7: PASS" : P = P + 1 ELSE PRINT "Test 7: FAIL" : F = F + 1
624 REM
630 PRINT "==========================================="
640 PRINT "COMPREHENSIVE DIALECT COMPATIBILITY SUMMARY"
650 PRINT "PASSED: "; P
660 PRINT "FAILED: "; F
670 PRINT "==========================================="
680 END
690 REM
700 SUB ScopedSub(g)
710   REM Change g locally
720   g = 42
730   REM Declare local GLOB
740   LOCAL GLOB
750   GLOB = 999
760 END SUB
