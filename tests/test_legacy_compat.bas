10 PRINT "=== GW-BASIC Compatibility Test Suite ==="
20 GOSUB 1000 ' Test DEF SEG Segment Math
30 GOSUB 2000 ' Test Scalar PEEK/POKE
40 GOSUB 3000 ' Test String PEEK/POKE
50 GOSUB 4000 ' Test VARPTR$ Descriptor
60 GOSUB 5000 ' Test Array PEEK/POKE
70 PRINT "=== All Tests Completed Successfully ==="
80 END

1000 PRINT "1. Testing DEF SEG Segment Math..."
1010 DEF SEG = &H40
1020 POKE 0, 123
1030 DEF SEG
1040 V = PEEK(&H400)
1050 PRINT "   PEEK(&H400) resolved address value: "; V
1060 IF V = 123 THEN PRINT "   [PASS] DEF SEG Segment math is correct." ELSE PRINT "   [FAIL] DEF SEG Segment math is incorrect." : END
1075 RETURN

2000 PRINT "2. Testing Scalar PEEK/POKE..."
2010 A% = 258 ' 0x0102
2020 pA = VARPTR(A%)
2030 bL = PEEK(pA)
2040 bH = PEEK(pA + 1)
2050 PRINT "   A% address="; pA; " LowByte="; bL; " HighByte="; bH
2060 IF bL = 2 AND bH = 1 THEN PRINT "   [PASS] PEEK(VARPTR(A%)) returned correct bytes." ELSE PRINT "   [FAIL] PEEK(VARPTR(A%)) returned incorrect bytes." : END
2070 POKE pA, 3 ' Change LowByte to 3 (A% becomes 259)
2080 PRINT "   New value of A% after POKE: "; A%
2090 IF A% = 259 THEN PRINT "   [PASS] POKE VARPTR(A%) updated variable value." ELSE PRINT "   [FAIL] POKE VARPTR(A%) failed to update variable." : END
2095 RETURN

3000 PRINT "3. Testing String Descriptor PEEK/POKE..."
3010 S$ = "HEllo"
3020 pS = VARPTR(S$)
3030 sLen = PEEK(pS)
3040 pD = PEEK(pS + 1) + PEEK(pS + 2)*256
3050 PRINT "   S$ descriptor: length="; sLen; " data_addr="; pD
3060 IF sLen = 5 THEN PRINT "   [PASS] String length descriptor is correct." ELSE PRINT "   [FAIL] String length descriptor is incorrect." : END
3070 ' Peek first character
3080 ch = PEEK(pD)
3090 PRINT "   First char code: "; ch; " (char "; CHR$(ch); ")"
3100 IF ch = ASC("H") THEN PRINT "   [PASS] Read string data via descriptor pointer." ELSE PRINT "   [FAIL] Read string data mismatch." : END
3110 ' Poke string char (change 'e' in "HEllo" to 'a')
3120 POKE pD + 1, ASC("A")
3130 PRINT "   New value of S$ after POKE: "; S$
3140 IF S$ = "HAllo" THEN PRINT "   [PASS] Mutating string data via pointer succeeded." ELSE PRINT "   [FAIL] Mutating string data failed." : END
3150 RETURN

4000 PRINT "4. Testing VARPTR$ Descriptor..."
4010 D$ = VARPTR$(A%)
4020 tByte = ASC(MID$(D$, 1, 1))
4030 addr = ASC(MID$(D$, 2, 1)) + ASC(MID$(D$, 3, 1))*256
4040 PRINT "   VARPTR$(A%) descriptor: type_byte="; tByte; " addr="; addr
4050 IF tByte = 2 THEN PRINT "   [PASS] VARPTR$ returned correct type byte for integer." ELSE PRINT "   [FAIL] VARPTR$ type byte mismatch." : END
4060 IF addr = VARPTR(A%) THEN PRINT "   [PASS] VARPTR$ returned correct variable address." ELSE PRINT "   [FAIL] VARPTR$ address mismatch." : END
4070 RETURN

5000 PRINT "5. Testing Array PEEK/POKE..."
5010 DIM E%(2)
5020 E%(0) = 513 ' 0x0201
5030 E%(1) = 1027 ' 0x0403
5040 pE = VARPTR(E%(0))
5050 bLA = PEEK(pE)
5060 bHA = PEEK(pE + 1)
5070 bLB = PEEK(pE + 8) ' Each flat BValue is at an 8-byte boundary
5080 bHB = PEEK(pE + 9)
5090 PRINT "   Array E%: addr="; pE; " E%(0)="; bLA; bHA; " E%(1)="; bLB; bHB
5100 IF bLA = 1 AND bHA = 2 AND bLB = 3 AND bHB = 4 THEN PRINT "   [PASS] Array PEEK resolved correctly." ELSE PRINT "   [FAIL] Array PEEK mismatch." : END
5110 POKE pE + 8, 5 ' change low byte of E%(1) to 5 (E%(1) becomes 1029)
5120 PRINT "   New value of E%(1) after array POKE: "; E%(1)
5130 IF E%(1) = 1029 THEN PRINT "   [PASS] Array POKE mutated variable correctly." ELSE PRINT "   [FAIL] Array POKE failed." : END
5140 RETURN
