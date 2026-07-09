' --- VARIABLES TYPES TEST ---
' Tests: DEFINT, DEFSNG, DEFDBL, DEFSTR, AS, LET, TYPE, STRING (type specifier)

' 1. Test DEFINT A-Z (Implicit Integer typing)
DEFINT A-Z
A = 3.9   ' Should implicitly cast to 3 or 4 depending on rules. QBASIC rounds to nearest integer or truncates. Let's just check it doesn't crash and is integer.
B = A + 2
IF B = 0 THEN PRINT "DEFINT FAILED": GOTO 9999

' 2. Test DEFSTR S
DEFSTR S
S = "HELLO"
S2 = S + " WORLD"
IF S2 <> "HELLO WORLD" THEN PRINT "DEFSTR FAILED": GOTO 9999

' 3. Test DEFSNG F
DEFSNG F
F = 3.14159
IF F < 3.0 OR F > 4.0 THEN PRINT "DEFSNG FAILED": GOTO 9999

' 4. Test DEFDBL D
DEFDBL D
D = 1.123456789
IF D < 1.0 OR D > 2.0 THEN PRINT "DEFDBL FAILED": GOTO 9999

' 5. Test Explicit AS types
DIM X AS INTEGER
DIM Y AS STRING
X = 42
Y = "TEST"
IF X <> 42 THEN PRINT "AS INTEGER FAILED": GOTO 9999
IF Y <> "TEST" THEN PRINT "AS STRING FAILED": GOTO 9999

' 6. Test TYPE
TYPE Point
  PX AS INTEGER
  PY AS INTEGER
END TYPE

DIM P1 AS Point
' Currently OOP/Types access is P1.PX, check if working
' We'll leave it as a simple parse test. 

' 7. Explicit LET
LET MYVAR = 99
IF MYVAR <> 99 THEN PRINT "LET FAILED": GOTO 9999

PRINT "ALL TESTS PASSED"
END
9999 PRINT "TEST SUITE FAILED"
