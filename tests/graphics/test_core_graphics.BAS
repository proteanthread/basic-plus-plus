' -------------------------------------------------------------
' BASIC++ Progressive Regression Test Suite
' Target: Graphics - Core (PSET, PRESET, LINE, CIRCLE, PAINT, POINT)
' -------------------------------------------------------------
' 1. WHAT CAN BE CHANGED:
'    Test coordinates and colors.
' 2. WHAT CANNOT BE CHANGED:
'    The expected logical outcome of the graphics buffer.
' 3. WHAT TO EXPECT:
'    Tests the standard primitive graphics rendering algorithms.
' 4. TROUBLESHOOTING:
'    If these fail, check gfxbuf.c boundary logic or parser_graphics.c.
' -------------------------------------------------------------

10 PRINT "--- GRAPHICS CORE TEST ---"
20 SCREEN 13
30 PSET (10, 10), 1
40 ASSERT POINT(10, 10) = 1
50 PRESET (10, 10)
60 ASSERT POINT(10, 10) = 0
70 LINE (0,0)-(20,20), 2, B
80 ASSERT POINT(0,0) = 2
90 ASSERT POINT(20,20) = 2
100 CIRCLE (100, 100), 5, 3
110 ASSERT POINT(100, 95) = 3
120 PAINT (100, 100), 4, 3
130 ASSERT POINT(100, 100) = 4
140 PRINT "CORE GRAPHICS PASSED"
150 END
