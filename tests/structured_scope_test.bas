10 REM QBASIC Scope Test
20 CounterVar = 100
30 PRINT "Main CounterVar before = "; CounterVar
40 CALL TestSub
50 PRINT "Main CounterVar after = "; CounterVar
60 END
70 SUB TestSub
80   CounterVar = 50
90   PRINT "Inside Sub CounterVar = "; CounterVar
100 END SUB
