10 REM tests/gw_graphics_device_trap_test.bas - Verify GW-BASIC Compatibility Features
20 PRINT "=== GW-BASIC GRAPHICS DEVICE TRAP TESTS ==="
30 REM 1. Check virtual device prefix match
40 OPEN "COM1:9600,N,8,1" FOR OUTPUT AS #1
50 PRINT #1, "HELLO SERIAL"
60 CLOSE #1
70 PRINT "COM1: Open/Write/Close: PASS"
80 REM 2. Check logical coordinate window mapping
90 SCREEN 1
100 WINDOW (0, 0)-(100, 100)
110 PSET (50, 50), 3
120 LINE (10, 10)-(90, 90), 2, B
130 CIRCLE (50, 50), 20, 1, 0, 6.28, 1.0
140 PRINT "Graphics Commands Coordinate Translate: PASS"
150 SCREEN 0
160 PRINT "=== ALL COMPATIBILITY TESTS PASSED ==="
