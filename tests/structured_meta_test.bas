10 REM Test script for QBASIC metacommands
20 PRINT "Testing QBASIC metacommands..."
30 '$STATIC
40 DIM arr1(5)
50 arr1(1) = 10
60 PRINT "arr1(1) = "; arr1(1)
70 REM This should print 100 from the included file
80 '$INCLUDE: 'tests/structured_inc.bas'
90 '$DYNAMIC
100 DIM arr2(5)
110 arr2(1) = 20
120 PRINT "arr2(1) = "; arr2(1)
130 REDIM arr2(10)
140 arr2(10) = 50
150 PRINT "arr2(10) = "; arr2(10)
160 PRINT "TEST PASSED"
170 END
