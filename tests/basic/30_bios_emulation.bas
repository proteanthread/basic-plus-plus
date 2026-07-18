10 REM =========================================================================
20 REM What can be changed: Test print descriptions, register values.
30 REM What cannot be changed: BIOS interrupt numbers, variable names (AX%, BX%, CX%, DX%, FLAGS%),
40 REM   and BDA/CGA MMIO assertions.
50 REM What to expect: Emulated INT 12h returns 640 KB conventional RAM, BDA reads match
60 REM   written values, and CGA memory read/write works successfully.
70 REM What to do if something breaks: Check mock_bios.c or stmt_bios.c registers
80 REM   handling and memory initialization.
90 REM =========================================================================
100 PRINT "RUNNING MOCK BIOS EMULATION TEST (PHASE 18)..."
110 
120 REM Test INT 12h (Get Conventional Memory Size)
130 LET AX% = 0
140 BIOS &H12
150 PRINT "   INT 12h Memory Size (KB):"; AX%
160 IF AX% <> 640 THEN PRINT "FAIL: Expected 640 KB conventional memory size": END
170 
180 REM Test INT 11h (Get Equipment List)
190 LET AX% = 0
200 BIOS &H11
210 PRINT "   INT 11h Equipment List Word:"; AX%
220 IF AX% <> 557 THEN PRINT "FAIL: Expected 557 (0x022D) equipment list": END
230 
245 REM Test BDA PEEK/POKE memory access (0x400 - 0x4FF)
250 REM Memory size is at BDA 0x413 (1043 decimal) and 0x414 (1044 decimal)
260 LET RAM_SIZE = PEEK(1043) + PEEK(1044) * 256
270 PRINT "   PEEK BDA 0x413/0x414 Memory Size:"; RAM_SIZE
280 IF RAM_SIZE <> 640 THEN PRINT "FAIL: BDA Memory size mismatch": END
290 
300 REM Test CGA Text Buffer MMIO (0xB8000 - 0xB8FA0)
310 REM Let's write to 0xB8000 (753664 decimal) and read back
320 POKE 753664, 65
330 V = PEEK(753664)
340 PRINT "   PEEK CGA Text Buffer 0xB8000:"; V
350 IF V <> 65 THEN PRINT "FAIL: CGA memory write/read failed": END
360 
370 PRINT "Mock BIOS Emulation: PASS"
380 RUN "tests/basic/31_external_features.bas"
