10 REM --- PROGRAM MANAGEMENT & EXECUTION TESTS ---
20 PRINT "Testing Program Management keywords..."

30 REM Test SAVE and UNSAVE
40 SAVE "tests/progmgmt/dummy.bas"
50 UNSAVE

60 REM Re-save it for subsequent tests
70 SAVE "tests/progmgmt/dummy.bas"

80 REM Test SCRATCH
90 SCRATCH "tests/progmgmt/dummy.bas"

100 PRINT "ALL TESTS PASSED"
110 ENDTEST 0
