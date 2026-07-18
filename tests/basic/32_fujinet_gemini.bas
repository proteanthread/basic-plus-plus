10 REM =========================================================================
20 REM What can be changed: Target URLs or SSIDs for testing.
30 REM What cannot be changed: Device names (N:, FUJI:, CLOCK:) or keyword syntax.
40 REM What to expect: Reading mock/live time, SSID configuration, and fetching Gemini capsules.
50 REM What to do if something breaks: Check vdev_fujinet.c, gemini.c, and OpenSSL loader logs.
60 REM =========================================================================
100 PRINT "RUNNING FUJINET & GEMINI PROTOCOL TESTS (PHASE 20)..."
110 
120 REM 1. Test CLOCK: NTP emulation
125 LET CIDX = -1
130 FOR I = 1 TO DEVICECOUNT()
135   IF DEVICE$(I) = "CLOCK:" THEN LET CIDX = I
140 NEXT I
145 IF CIDX = -1 THEN PRINT "FAIL: CLOCK: device not found": END
150 OPEN "CLOCK:" FOR INPUT AS #1
160 INPUT #1, T$
165 CLOSE #1
170 PRINT "   CLOCK: Time from device: "; T$
175 IF LEN(T$) = 0 THEN PRINT "FAIL: CLOCK: returned empty string": END
180 LET D$ = DEVICEINFO$(CIDX, "DATE")
185 LET T2$ = DEVICEINFO$(CIDX, "TIME")
190 PRINT "   CLOCK: Date: "; D$; " Time: "; T2$
195 IF LEN(D$) = 0 THEN PRINT "FAIL: CLOCK info empty": END
196 IF LEN(T2$) = 0 THEN PRINT "FAIL: CLOCK info empty": END
200 
210 REM 2. Test FUJI: configuration IOCTL and info query
215 LET FIDX = -1
220 FOR I = 1 TO DEVICECOUNT()
225   IF DEVICE$(I) = "FUJI:" THEN LET FIDX = I
230 NEXT I
235 IF FIDX = -1 THEN PRINT "FAIL: FUJI: device not found": END
240 OPEN "FUJI:" FOR INPUT AS #2
245 IOCTL #2, 1, "BASIC-NET-SSID"
250 CLOSE #2
255 LET S$ = DEVICEINFO$(FIDX, "SSID")
260 PRINT "   FUJI: SSID read back: "; S$
265 IF S$ <> "BASIC-NET-SSID" THEN PRINT "FAIL: SSID set/get mismatch": END
270 
280 REM 3. Test GEMINI statement
290 GEMINI "gemini://geminiprotocol.net/", RESP$
300 PRINT "   GEMINI statement fetch length:"; LEN(RESP$)
310 IF LEN(RESP$) = 0 THEN PRINT "FAIL: GEMINI response was empty": END
320 
330 REM 4. Test N: device stream interface with gemini:// URL
340 OPEN "N:gemini://geminiprotocol.net/" FOR INPUT AS #1
350 INPUT #1, L1$
360 INPUT #1, L2$
370 CLOSE #1
380 PRINT "   N: First line:  "; L1$
390 PRINT "   N: Second line: "; L2$
400 IF LEN(L1$) = 0 THEN PRINT "FAIL: N: device response was empty": END
410 
420 PRINT "FujiNet & Gemini Integration: PASS"
430 RUN "tests/basic/28_vdev2_channels.bas"
