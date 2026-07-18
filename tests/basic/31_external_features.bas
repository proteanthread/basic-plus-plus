10 REM =========================================================================
20 REM What can be changed: Numeric values and print statements.
30 REM What cannot be changed: LOAD FEATURE commands, keyword names (TURTLE, CUBE).
40 REM What to expect: Custom keyword statements and functions execute correctly
50 REM   within isolated library scopes.
60 REM What to do if something breaks: Check spec.c parsing and execution dispatch.
70 REM =========================================================================
100 PRINT "RUNNING DYNAMIC KEYWORD SPECIFICATION TESTS (PHASE 19)..."
110 
120 REM 1. Load feature specification
130 LOAD FEATURE "tests/basic/turtle.spec"
140 PRINT "   Loaded turtle.spec successfully."
150 
160 REM 2. Test custom statement execution with arguments
170 TURTLE FORWARD 123
180 TURTLE RIGHT 45
190 
200 REM 3. Test custom function execution in expression evaluator
210 LET V = CUBE(5)
225 PRINT "   CUBE(5) ="; V
230 IF V <> 125 THEN PRINT "FAIL: Custom function CUBE did not return 125": END
240 
250 PRINT "Dynamic Keyword Specifications: PASS"
260 RUN "tests/basic/32_fujinet_gemini.bas"
