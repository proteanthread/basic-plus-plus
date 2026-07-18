10 REM =====================================================================
20 REM BASIC++ v6.0.0 SELECT CASE Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Input matching values, case body print messages.
60 REM - What cannot be changed: Selection ranges, case conditions, and expected match assertions.
70 REM - What to expect: Enters SELECT CASE with a test number, matches correct cases, and outputs OK.
80 REM - What to do if something breaks: Trace SELECT nesting matching levels and stack allocations.
90 REM =====================================================================
100 PRINT "RUNNING SELECT CASE TEST..."
110 LET X = 42
120 SELECT CASE X
130   CASE 1 TO 10
140     PRINT "FAILED: MATCHED WRONG RANGE"
150     END
160   CASE IS > 50
170     PRINT "FAILED: MATCHED WRONG CONDITION"
180     END
190   CASE 42
200     PRINT "SELECT CASE MATCHED OK"
210   CASE ELSE
220     PRINT "FAILED: REACHED CASE ELSE"
230     END
240 END SELECT
250 PRINT "SELECT CASE OK"
260 RUN "tests/basic/11_sub_func.bas"
