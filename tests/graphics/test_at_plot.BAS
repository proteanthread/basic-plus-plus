' -------------------------------------------------------------
' BASIC++ Progressive Regression Test Suite
' Target: Graphics - AT and PLOT commands
' -------------------------------------------------------------
' 1. WHAT CAN BE CHANGED:
'    Test coordinates.
' 2. WHAT CANNOT BE CHANGED:
'    The expected logical outcome of the graphics cursor state.
' 3. WHAT TO EXPECT:
'    Tests AT, PRINT @, PLOT AT, PLOT @.
' 4. TROUBLESHOOTING:
'    If these fail, check parser_graphics.c and parser_io.c for TOK_AT.
' -------------------------------------------------------------

10 PRINT "--- GRAPHICS AT/PLOT TEST ---"
20 SCREEN 13
30 AT 5, 10
40 PRINT @50, "Hello"
50 PLOT AT 20, 20, 1
60 ASSERT POINT(20, 20) = 1
70 PLOT @500, 2
80 PRINT "AT/PLOT PASSED"
90 END
