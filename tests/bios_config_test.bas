10 REM bios_config_test.bas - Test switchable mock BIOS settings
20 PRINT "=== RUNNING BIOS CONFIG TESTS ==="
50 PRINT "Model byte at &HFFFFE: "; PEEK(&HFFFFE)
60 IF PEEK(&HFFFFE) <> 255 THEN PRINT "FAIL: Default model byte is not 255" : END
70 PRINT "Initial screen width (BDA &H44A): "; PEEK(&H044A)
80 IF PEEK(&H044A) <> 80 THEN PRINT "FAIL: Default width is not 80" : END
90 REM Check Memory Size via INT &H12
100 AX = 0 : BX = 0 : CX = 0 : DX = 0
110 INT &H12
120 PRINT "Conventional memory size (AX): "; AX; " KB"
130 IF AX <> 640 THEN PRINT "FAIL: Conventional memory size not 640" : END
140 REM Switch to PCjr
150 PRINT "Switching to PCjr..."
160 BIOS "PCJR"
190 PRINT "PCjr model byte: "; PEEK(&HFFFFE)
200 IF PEEK(&HFFFFE) <> 253 THEN PRINT "FAIL: PCjr model byte not 253" : END
210 PRINT "PCjr screen width: "; PEEK(&H044A)
220 IF PEEK(&H044A) <> 40 THEN PRINT "FAIL: PCjr width not 40" : END
230 REM Switch to XT
240 PRINT "Switching to XT..."
250 BIOS "XT"
260 PRINT "XT model byte: "; PEEK(&HFFFFE)
270 IF PEEK(&HFFFFE) <> 254 THEN PRINT "FAIL: XT model byte not 254" : END
280 REM Switch to AT
290 PRINT "Switching to AT..."
300 BIOS "AT"
310 PRINT "AT model byte: "; PEEK(&HFFFFE)
320 IF PEEK(&HFFFFE) <> 252 THEN PRINT "FAIL: AT model byte not 252" : END
420 REM Get Equipment List via INT &H11
430 AX = 0
440 INT &H11
450 PRINT "Equipment list flags (AX): &H"; HEX$(AX)
460 IF (AX AND &H21) <> &H21 THEN PRINT "FAIL: Equipment list mismatch" : END
470 PRINT "=== BIOS CONFIG TESTS PASSED ==="
