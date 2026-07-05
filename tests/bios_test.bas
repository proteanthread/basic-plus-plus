10 REM ============================================================
20 REM  TEST: bios_test.bas - Switchable Mock BIOS & Interrupts
30 REM ============================================================
40 PRINT "Testing Switchable Mock BIOS & Interrupts..."
50 REM Default BIOS check
60 BIOS "IBMPC"
70 REM Load registers and call INT 10H (Video)
80 AH = &H03 : BH = 0
90 INT &H10
100 REM Set cursor pos: AH = 2, DH = 5, DL = 10
110 AH = 2 : BH = 0 : DH = 5 : DL = 10
120 INT &H10
130 REM Get cursor pos again, verify return values
140 AH = 3 : BH = 0
150 INT &H10
160 REM DH should be 5, DL should be 10. Check register halves
170 IF DH <> 5 OR DL <> 10 THEN PRINT "BIOS CURSOR POS ERROR: DH="; DH; " DL="; DL : END
180 REM Switch to AT
190 BIOS "PCAT"
200 REM Verify clock INT 1AH works
211 AH = 0
220 INT &H1A
230 PRINT "System ticks returned: CX="; CX; " DX="; DX
231 REM Test MS-DOS services
232 REM INT 21h AH=30h: Get version
233 AH = &H30 : INT &H21
234 IF AX <> &H0500 THEN PRINT "FAIL: INT 21h AH=30h Get Version: "; HEX$(AX) : END
235 PRINT "INT 21h AH=30h version AX="; HEX$(AX); " (OK)"
236 REM INT 21h AH=09h: Print string (terminated by '$' in RAM)
237 MEMMAP "MSDOS"
238 POKE &H8000, ASC("H")
239 POKE &H8001, ASC("E")
240 POKE &H8002, ASC("L")
241 POKE &H8003, ASC("L")
242 POKE &H8004, ASC("O")
243 POKE &H8005, ASC("$")
244 AH = &H09 : DX = &H8000 : INT &H21
245 PRINT " (INT 21h AH=09h String Printed)"
246 PRINT "BIOS tests completed successfully."
247 END
