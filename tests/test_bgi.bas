REM =====================================================================
REM BASIC++ BGI (BASIC++ Graphics Interface) Test Suite
REM =====================================================================
REM
REM What can be changed: Test values, coordinates, colors.
REM What cannot be changed: INITGRAPH/CLOSEGRAPH sequence, assertion checks.
REM What to expect: Tests BGI primitives in headless mode (no SDL2 window).
REM   Each test validates argument parsing and returns without error.
REM What to do if something breaks: Check BGI mode table registration and
REM   verify BGI_init() is called during INITGRAPH.
REM =====================================================================

10 PRINT "--- BGI Graphics Interface Test Suite ---"

REM Test 1: INITGRAPH with VGA Mode 13h (320x200, 256 colors)
20 INITGRAPH 113
30 PRINT "Test 1: INITGRAPH 113 (VGA Mode 13h) ... PASS"

REM Test 2: PUTPIXEL at center of screen
40 PUTPIXEL 160, 100, 15
50 PRINT "Test 2: PUTPIXEL 160, 100, 15 ... PASS"

REM Test 3: BAR (filled rectangle)
60 BAR 10, 10, 50, 50
70 PRINT "Test 3: BAR 10, 10, 50, 50 ... PASS"

REM Test 4: RECTANGLE (unfilled rectangle outline)
80 RECTANGLE 60, 10, 100, 50
90 PRINT "Test 4: RECTANGLE 60, 10, 100, 50 ... PASS"

REM Test 5: ELLIPSE (arc)
100 ELLIPSE 160, 100, 0, 360, 30, 20
110 PRINT "Test 5: ELLIPSE 160, 100, 0, 360, 30, 20 ... PASS"

REM Test 6: OUTTEXTXY (text rendering)
120 OUTTEXTXY 10, 80, "Hello BGI"
130 PRINT "Test 6: OUTTEXTXY 10, 80, 'Hello BGI' ... PASS"

REM Test 7: PALETTE (set palette entry)
140 PALETTE 1, 0, 128, 255
150 PRINT "Test 7: PALETTE 1, 0, 128, 255 ... PASS"

REM Test 8: CLOSEGRAPH
160 CLOSEGRAPH
170 PRINT "Test 8: CLOSEGRAPH ... PASS"

REM Test 9: INITGRAPH with CGA Mode (320x200, 4 colors)
180 INITGRAPH 101
190 PRINT "Test 9: INITGRAPH 101 (CGA SCREEN 1) ... PASS"
200 CLOSEGRAPH

REM Test 10: INITGRAPH with NES PPU NTSC (256x240, 54 colors)
210 INITGRAPH 300
220 PUTPIXEL 128, 120, 4
230 PRINT "Test 10: INITGRAPH 300 (NES PPU NTSC) + PUTPIXEL ... PASS"
240 CLOSEGRAPH

250 PRINT ""
260 PRINT "--- ALL BGI TESTS PASSED ---"
