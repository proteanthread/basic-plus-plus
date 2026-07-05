10 REM QBASIC Shared Variables Test
20 DIM SHARED GlobalVar
30 GlobalVar = 100
40 PRINT "Before: GlobalVar ="; GlobalVar
50 CALL ModifyGlobal()
60 PRINT "After: GlobalVar ="; GlobalVar
70 IF GlobalVar <> 999 THEN PRINT "FAILED: GlobalVar not shared!": END
80 PRINT "STRUCTURED_SHARED_TEST PASSED"
90 END
100 SUB ModifyGlobal ()
110   GlobalVar = 999
120 END SUB
