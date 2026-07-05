10 REM ============================================================
20 REM  TEST: conversational_test.bas - JOSS/FOCAL/MUMPS Dialects
30 REM ============================================================
40 PRINT "Testing Conversational Dialect Features..."
50 REM
80 REM
90 REM --- Test 2: DEMAND keyword (File-based input test) ---
100 OPEN "O", #1, "test_dem.txt"
110 PRINT #1, "DEMAND_DATA"
120 CLOSE #1
130 OPEN "I", #1, "test_dem.txt"
140 DEMAND #1, D$
150 CLOSE #1
160 KILL "test_dem.txt"
170 IF D$ <> "DEMAND_DATA" THEN PRINT "DEMAND ALIAS FAILED": END
180 REM
190 REM --- Test 3: Persistent Variables ---
200 R = PSTORE("my_var", "PERSISTENT_VALUE")
210 V$ = PRETRIEVE$("my_var")
215 PRINT "V$ is: ["; V$; "]"
220 IF V$ <> "PERSISTENT_VALUE" THEN PRINT "PSTORE/PRETRIEVE FAILED": END
230 R = PSTORE("my_num", 123.45)
240 N = PRETRIEVE("my_num")
250 IF ABS(N - 123.45) > 0.001 THEN PRINT "PSTORE/PRETRIEVE NUMBER FAILED": END
260 REM
270 REM --- Test 4: Virtual Device PERSIST: ---
280 OPEN "O", #1, "PERSIST:my_key"
290 PRINT #1, "VDEV_PERSIST_DATA"
300 CLOSE #1
310 OPEN "I", #1, "PERSIST:my_key"
320 INPUT #1, K$
330 CLOSE #1
340 IF K$ <> "VDEV_PERSIST_DATA" THEN PRINT "VDEV PERSIST FAILED": END
350 REM
360 REM --- Test 5: FOCAL-style DO block execution ---
370 DO 10.10
380 PRINT "Back from DO call."
390 GOTO 500
10.10 PRINT "Inside block step 10.10!"
10.20 RETURN
430 REM
500 PRINT "Conversational dialect tests completed successfully."
510 END
