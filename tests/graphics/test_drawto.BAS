' -------------------------------------------------------------
' BASIC++ Progressive Regression Test Suite
' Target: Graphics - DRAWTO & LINE -(X,Y)
' -------------------------------------------------------------
' 1. WHAT CAN BE CHANGED:
'    Test coordinates and colors.
' 2. WHAT CANNOT BE CHANGED:
'    The expected logical outcome of the graphics cursor state.
' 3. WHAT TO EXPECT:
'    Tests the Atari-style DRAWTO and GW-BASIC LINE -(X,Y) logic.
' 4. TROUBLESHOOTING:
'    If these fail, check gfxbuf_set_cursor() inside parser_graphics.c.
' -------------------------------------------------------------

10 PRINT "--- GRAPHICS DRAWTO / LINE TEST ---"
20 SCREEN 13
30 PSET (10, 10), 1
40 ASSERT POINT(0) = 10
50 ASSERT POINT(1) = 10
60 DRAWTO 20, 10, 2
70 ASSERT POINT(15, 10) = 2
80 ASSERT POINT(20, 10) = 2
90 ASSERT POINT(0) = 20
100 ASSERT POINT(1) = 10
110 LINE -(20, 20), 3
120 ASSERT POINT(20, 15) = 3
130 ASSERT POINT(20, 20) = 3
140 PRINT "DRAWTO / LINE PASSED"
150 END
