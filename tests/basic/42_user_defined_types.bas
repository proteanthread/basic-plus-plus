10 REM =====================================================================
20 REM BASIC++ v6.0.0 User-Defined Types (UDT) Integration Test
30 REM =====================================================================
40 REM - What can be changed: DATA values, member assignment values.
50 REM - What cannot be changed: TYPE definition layout, DIM target structure.
60 REM - What to expect: Registers UDTs, instantiates them, assigns values,
70 REM   and copies instances. Prints results and chains to next test.
80 REM - What to do if something breaks: Check TYPE registration and member
90 REM   access implementation in src/statements/stmt_struct.c and eval.c.
100 REM =====================================================================
110 PRINT "RUNNING UDT INTEGRATION TESTS..."
120 TYPE Address
130   street AS STRING
140   zip AS NUMBER
150 END TYPE
160 TYPE Person
170   fullname AS STRING
180   age AS NUMBER
190   addr AS Address
200 END TYPE
210 DIM p1 AS Person
220 DIM p2 AS Person
230 p1.fullname = "Alice"
240 p1.age = 28
250 p1.addr.street = "123 Main St"
260 p1.addr.zip = 90210
270 PRINT "Person 1: "; p1.fullname; " age "; p1.age; " at "; p1.addr.street; " "; p1.addr.zip
280 REM Test instance assignment / copy
290 LET p2 = p1
300 PRINT "Person 2: "; p2.fullname; " age "; p2.age; " at "; p2.addr.street; " "; p2.addr.zip
310 REM Modify copy and ensure source is unaffected (deep copy/value copy semantics)
320 p2.fullname = "Bob"
330 p2.addr.street = "456 Oak Ave"
340 PRINT "Person 1 modified check: "; p1.fullname; " at "; p1.addr.street
350 PRINT "Person 2 modified check: "; p2.fullname; " at "; p2.addr.street
360 IF p1.fullname = "Alice" THEN GOTO 390
370 PRINT "FAILED: UDT source modified on copy mutation"
380 END
390 IF p2.fullname = "Bob" THEN GOTO 420
400 PRINT "FAILED: UDT copy mutation failed"
410 END
420 PRINT "UDT INTEGRATION TESTS OK"
430 RUN "tests/basic/43_oop_classes.bas"
