10 REM =====================================================================
20 REM BASIC++ v6.0.0 OOP Classes Integration Test
30 REM =====================================================================
40 REM - What can be changed: Method return computations, print statements.
50 REM - What cannot be changed: CLASS definition layout, method signature.
60 REM - What to expect: Registers OOP classes, instantiates them, and invokes
70 REM   methods. Prints output and chains to next test.
80 REM - What to do if something breaks: Trace class parser in src/statements/stmt_struct.c
90 REM   and call dispatcher in src/expression/eval.c.
100 REM =====================================================================
110 PRINT "RUNNING OOP CLASSES INTEGRATION TESTS..."
120 CLASS Counter
130   count AS NUMBER
140   FUNCTION inc()
150     count = count + 1
160   END FUNCTION
170 END CLASS
180 DIM c AS Counter
190 c.count = 10
200 PRINT "Counter initial count: "; c.count
215 c.inc()
220 PRINT "Counter incremented count: "; c.count
230 IF c.count = 11 THEN GOTO 260
240 PRINT "FAILED: Class method invocation did not increment field"
250 END
260 PRINT "OOP CLASSES INTEGRATION TESTS OK"
270 RUN "tests/basic/44_extended_matrix.bas"
