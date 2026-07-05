10 REM ============================================================
20 REM  TEST: bios_ext_test.bas - Extended BIOS features
30 REM ============================================================
40 PRINT "Testing Extended BIOS Mock Features..."
50 REM Test Printer (INT 17H)
60 PRINT "Printing to LPT1..."
70 AH = &H00 : AL = ASC("H") : INT &H17
80 AH = &H00 : AL = ASC("E") : INT &H17
90 AH = &H00 : AL = ASC("L") : INT &H17
100 AH = &H00 : AL = ASC("L") : INT &H17
110 AH = &H00 : AL = ASC("O") : INT &H17
120 AH = &H00 : AL = 10 : INT &H17
130 REM Test Video Mode check (INT 10H AH=0FH)
140 AH = &H0F : INT &H10
150 PRINT "Current video mode (AL): "; AL
160 REM Test XT blocking of AT features
170 BIOS "PCXT"
180 AH = &H88 : INT &H15
190 FLAGS_VAL = FLAGS
200 IF (FLAGS_VAL AND 1) = 0 THEN PRINT "FAIL: XT allowed INT 15H AH=88H" : END
210 PRINT "XT correctly blocked AT-only INT 15H."
220 REM Switch to AT and test features
230 BIOS "PCAT"
240 AH = &H88 : INT &H15
250 FLAGS_VAL = FLAGS
260 IF (FLAGS_VAL AND 1) <> 0 THEN PRINT "FAIL: AT blocked INT 15H AH=88H" : END
270 PRINT "AT allowed INT 15H. Extended Mem (AX) = "; AX; "KB"
280 REM Test RTC Date (INT 1AH AH=04H)
290 AH = &H04 : INT &H1A
300 FLAGS_VAL = FLAGS
310 IF (FLAGS_VAL AND 1) <> 0 THEN PRINT "FAIL: AT blocked INT 1AH RTC" : END
320 PRINT "RTC Date: CH="; HEX$(CH); " CL="; HEX$(CL); " DH="; HEX$(DH); " DL="; HEX$(DL)
330 REM New hybrid ports/signatures check
340 MEMMAP "PCJR"
350 IF PEEK(&HD0000) <> &H55 OR PEEK(&HD0001) <> &HAA THEN PRINT "FAIL: PCjr cartridge header signature" : END
360 PRINT "PCjr cartridge header signature: OK"
370 MEMMAP "PCXT"
380 IF PEEK(&HC8000) <> &H55 OR PEEK(&HC8001) <> &HAA THEN PRINT "FAIL: XT extension header signature" : END
390 PRINT "XT IDE extension signature: OK"
400 REM Port OUT/INP check
410 OUT &H61, &H55
420 IF INP(&H61) <> &H55 THEN PRINT "FAIL: PPI port B write/read" : END
430 PRINT "PPI Port B emulated I/O: OK"
440 REM CMOS AT Index/Data check
450 MEMMAP "PCAT"
460 OUT &H70, &H10
470 IF INP(&H71) <> &H44 THEN PRINT "FAIL: CMOS floppy layout read" : END
480 PRINT "CMOS AT custom I/O: OK"
490 PRINT "Extended BIOS tests completed successfully."
500 END
