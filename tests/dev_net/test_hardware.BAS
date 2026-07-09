10 REM ==========================================
20 REM BASIC++ DEV & NET HARDWARE TESTS
30 REM Keywords: INP, OUT, WAIT, SYS, BIOREAD, SIOREAD, SIOREADLN, COM, IOCTL
40 REM ==========================================
50 PRINT "Testing Hardware Keywords..."
60 REM We use stub calls for some to avoid hard crashes on Windows/Linux
70 ON ERROR GOTO 1000
80 OUT &H3F8, 0 : REM COM1 dummy
90 X = INP(&H3F8)
100 WAIT &H3F8, 1, 0 : REM will timeout or skip in virtual mode
110 SYS "dummy" : REM system call
120 B$ = BIOREAD$(1)
130 S$ = SIOREAD$(1)
140 L$ = SIOREADLN$(1)
150 COM 1, "baud=9600"
160 IOCTL #1, "mode=raw"
170 PRINT "Hardware Keywords Passed."
180 END
1000 REM Error Handler
1010 PRINT "Hardware Error: "; ERR
1020 RESUME NEXT
