10 REM =========================================================================
20 REM What can be changed: You can add more prints or change channel number.
30 REM What cannot be changed: The DEVICECOUNT, DEVICE$, DEVICECLASS$ assertions.
40 REM What to expect: Print registered devices list, open CON: as channel, write.
50 REM What to do if something breaks: Check if vdev.c or file.c compilation has issues.
60 REM =========================================================================
70 PRINT "--- TESTING VDEV2 DEVICE LISTING ---"
80 DEVICES
90 PRINT "DEVICE COUNT:"; DEVICECOUNT()
100 IF DEVICECOUNT() < 1 THEN PRINT "FAIL: No devices registered": END
110 FOR I = 1 TO DEVICECOUNT()
120   PRINT "DEVICE"; I; ":"; DEVICE$(I); "CLASS:"; DEVICECLASS$(I)
130 NEXT I
140 PRINT "--- TESTING CON: CHANNEL I/O ---"
150 OPEN "CON:" FOR OUTPUT AS #1
160 PRINT #1, "HELLO FROM CON: CHANNEL OUTPUT"
170 CLOSE #1
180 PRINT "--- TESTING POLL AND ERROR HANDLING ---"
190 OPEN "CON:" FOR INPUT AS #2
200 PRINT "POLL(2) STATUS:"; POLL(2)
210 CLOSE #2
220 PRINT "VDEV2 CHANNELS: PASS"
230 RUN "tests/basic/27_security_gating.bas"
