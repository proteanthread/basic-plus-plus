10 REM ============================================================
20 REM  TEST: oop_test.bas - Optional Object Processing (OOP)
30 REM ============================================================
40 REM
50 P = 0 : F = 0
60 REM
70 REM --- Define CLASS Point ---
80 CLASS Point
90   DIM x AS INTEGER
100  DIM y AS INTEGER
110  SHARED DIM counter AS INTEGER
120  PRIVATE DIM secret AS INTEGER
130  
140  SUB Point(xVal, yVal)
150    self.x = xVal
160    self.y = yVal
170    Point::counter = Point::counter + 1
180    self.secret = 42
190  END SUB
200  
210  SUB Move(dx, dy)
220    self.x = self.x + dx
230    self.y = self.y + dy
240  END SUB
250  
260  FUNCTION GetSecret()
270    GetSecret = self.secret
280  END FUNCTION
290 END CLASS
300 
310 REM --- Define MODULE MathUtils ---
320 MODULE MathUtils
330   PUBLIC DIM PI
340   
350   SUB Init()
360     MathUtils.PI = 3.14159
370   END SUB
380   
390   FUNCTION Add(a, b)
400     Add = a + b
410   END FUNCTION
420 END MODULE
430 
440 REM --- Initialize counter ---
450 Point::counter = 0
460 
470 REM --- Instantiate Point object ---
480 DIM pt AS Point
490 pt = NEW Point(10, 20)
500 
510 REM --- Test field accesses ---
520 IF pt.x = 10 THEN P = P + 1 : PRINT "PASS: pt.x = 10" ELSE F = F + 1 : PRINT "FAIL: pt.x = "; pt.x
530 IF pt.y = 20 THEN P = P + 1 : PRINT "PASS: pt.y = 20" ELSE F = F + 1 : PRINT "FAIL: pt.y = "; pt.y
540 
550 REM --- Test method invocation ---
560 CALL pt.Move(5, 10)
570 IF pt.x = 15 AND pt.y = 30 THEN P = P + 1 : PRINT "PASS: pt.Move worked" ELSE F = F + 1 : PRINT "FAIL: pt.Move: "; pt.x; pt.y
580 
590 REM --- Test double colon invocation alias ---
600 CALL pt::Move(-5, -10)
610 IF pt.x = 10 AND pt.y = 20 THEN P = P + 1 : PRINT "PASS: pt::Move worked" ELSE F = F + 1 : PRINT "FAIL: pt::Move: "; pt.x; pt.y
620 
630 REM --- Test shared field (static class variable) ---
640 IF Point::counter = 1 THEN P = P + 1 : PRINT "PASS: Point::counter = 1" ELSE F = F + 1 : PRINT "FAIL: Point::counter = "; Point::counter
650 
660 REM --- Test private field encapsulation via method ---
670 IF pt.GetSecret() = 42 THEN P = P + 1 : PRINT "PASS: pt.GetSecret = 42" ELSE F = F + 1 : PRINT "FAIL: pt.GetSecret = "; pt.GetSecret()
680 
690 REM --- Test Module namespaced functions ---
700 CALL MathUtils::Init()
710 IF MathUtils.PI > 3.14 THEN P = P + 1 : PRINT "PASS: MathUtils.PI > 3.14" ELSE F = F + 1 : PRINT "FAIL: MathUtils.PI = "; MathUtils.PI
720 
730 V = MathUtils::Add(100, 200)
740 IF V = 300 THEN P = P + 1 : PRINT "PASS: MathUtils::Add = 300" ELSE F = F + 1 : PRINT "FAIL: MathUtils::Add = "; V
750 
760 PRINT "--- OOP TEST COMPLETE ---"
770 PRINT "PASS: "; P; " FAIL: "; F
780 IF F = 0 THEN PRINT "ALL OOP TESTS PASSED!"
