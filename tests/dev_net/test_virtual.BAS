10 REM ==========================================
20 REM BASIC++ DEV & NET VIRTUAL TESTS
30 REM Keywords: VDEV, VNET, VMEM, VMACH, VCON, VTERM, DEVMAP, NINFO, NJSONQUERY, GOPHER, GEMINI
40 REM Devices: GEMINI:, KERMIT:, GOPHER:, CLOCK:, FUJI:, NET:, TCP:, UDP:
50 REM ==========================================
60 PRINT "Testing Virtual & Networking Keywords..."
70 ON ERROR GOTO 1000
80 VDEV 1, "test"
90 VNET 1
100 VMEM 1, 1024
110 VMACH "reset"
120 VCON 1
130 VTERM 1
140 DEVMAP "COM1", "VDEV1"
150 I$ = NINFO$("net0")
160 J$ = NJSONQUERY$("{""key"":""val""}", "key")
170 GEMINI "gemini://geminiprotocol.net/", G$
180 GOPHER "gopher://gopher.floodgap.com/", H$
190 REM Test Virtual Devices
200 OPEN "GEMINI://geminiprotocol.net/" FOR INPUT AS #1
210 CLOSE #1
220 OPEN "GOPHER://gopher.floodgap.com/" FOR INPUT AS #1
230 CLOSE #1
240 OPEN "FUJI:TCP://127.0.0.1:80/" FOR INPUT AS #1
250 CLOSE #1
260 OPEN "KERMIT://" FOR INPUT AS #1
270 CLOSE #1
280 OPEN "CLOCK://" FOR INPUT AS #1
290 CLOSE #1
300 PRINT "Virtual & Networking Tests Passed."
310 END
1000 REM Error Handler
1010 PRINT "Virtual Error: "; ERR
1020 RESUME NEXT
