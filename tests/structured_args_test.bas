10 REM QBASIC Sub/Function Argument Mutation Test (Pass-by-Reference)
20 A = 10
30 B = 20
40 PRINT "Before SUB: A ="; A
50 CALL UpdateValue(A)
60 PRINT "After SUB: A ="; A
70 IF A <> 15 THEN PRINT "FAILED: A not mutated!": END
80 PRINT "Before FUNCTION: B ="; B
90 RES = MultiplyValue(B)
100 PRINT "After FUNCTION: B ="; B
110 IF B <> 40 THEN PRINT "FAILED: B not mutated!": END
120 PRINT "STRUCTURED_ARGS_TEST PASSED"
130 END
140 SUB UpdateValue (X)
150   X = X + 5
160 END SUB
170 FUNCTION MultiplyValue (Y)
180   Y = Y * 2
190   MultiplyValue = Y
200 END FUNCTION
