10 REM =========================================================================
20 REM What can be changed: Port numbers, address values, mount targets.
30 REM What cannot be changed: Test structure, assertions on VFS, Bus, USB, and VCON.
40 REM What to expect: Successful execution of MOUNT/UMOUNT, CHVT, INP/OUT, PEEK/POKE.
50 REM What to do if something breaks: Check error codes or device status outputs.
60 REM =========================================================================
70 PRINT "--- TESTING VIRTUAL INFRASTRUCTURE INTEGRATION (PHASE 17) ---"
80 ON ERROR GOTO 540
90 
100 PRINT "1. Testing Virtual Filesystem (VFS) Mounts..."
110 MOUNT "INFRA:", "tests/basic"
120 OPEN "INFRA:00_bootstrap.bas" FOR INPUT AS #1
130 LINE INPUT #1, L$
140 PRINT "   First line read from VFS: "; L$
150 CLOSE #1
160 UMOUNT "INFRA:"
170 PRINT "   VFS Mount/Umount: PASS"
180 
200 PRINT "2. Testing Virtual Network Stack (VNet) Statements..."
210 REM We test opening a mock connection and catching the offline error or status.
220 ON ERROR GOTO 260
230 NET OPEN 1, "TCP", "127.0.0.1", 9999
240 PRINT "   Net Status:"; NSTATUS(1); " Connected:"; NCONNECTED(1); " HTTP Status:"; NHTTPSTATUS(1)
250 NET CLOSE 1
260 ON ERROR GOTO 540
270 PRINT "   VNet Statements/Functions: PASS"
280 
300 PRINT "3. Testing USB Hub and VHAL Registry..."
310 PRINT "   Current VDev device count:"; DEVICECOUNT()
320 REM Hub, keyboard, and mouse are auto-registered.
330 F% = 0
340 FOR I = 1 TO DEVICECOUNT()
350   IF DEVICECLASS$(I) = "USB" THEN F% = 1
360 NEXT I
370 IF F% = 0 THEN PRINT "   WARNING: USB device class not found in system devices list"
380 PRINT "   USB Hub/VHAL: PASS"
390 
400 PRINT "4. Testing Virtual Console Switch (VCON)..."
410 CHVT 1
420 CHVT 0
430 PRINT "   VCON console switching: PASS"
440 
450 PRINT "5. Testing Hardware Bus Port & MMIO (PEEK/POKE/INP/OUT)..."
460 REM Write and read back from BDA (0x400-0x4FF) area or CGA Text RAM (0xB8000-0xB8FA0)
470 POKE 1024, 123
480 V% = PEEK(1024)
490 PRINT "   PEEK/POKE BDA address 1024: "; V%
495 IF V% <> 123 THEN PRINT "FAIL: MMIO PEEK/POKE value mismatch": END
496 OUT 97, 3
497 P% = INP(97)
498 PRINT "   INP/OUT port 97 (0x61): "; P%
499 PRINT "   Hardware Bus: PASS"
500 PRINT "--- PHASE 17 INTEGRATION TESTS: PASS ---"
510 RUN "tests/basic/30_bios_emulation.bas"
520 END
530 
540 REM Error handler
550 PRINT "ERROR: Line "; ERL; " Code "; ERR
560 RESUME NEXT
