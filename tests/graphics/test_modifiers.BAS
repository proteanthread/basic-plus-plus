' -------------------------------------------------------------
' BASIC++ Progressive Regression Test Suite
' Target: Graphics - Modifiers
' -------------------------------------------------------------
' 1. WHAT CAN BE CHANGED:
'    Test variables.
' 2. WHAT CANNOT BE CHANGED:
'    The expected logical outcome of the graphics modifiers.
' 3. WHAT TO EXPECT:
'    Tests BORDER, PAPER, INK, INVERSE, REVERSE, BRIGHT, FLASH, OVER.
' 4. TROUBLESHOOTING:
'    If these fail, check parser_graphics.c implementations and runtime.h structure.
' -------------------------------------------------------------

10 PRINT "--- GRAPHICS MODIFIERS TEST ---"
20 BORDER 1
30 PAPER 2
40 INK 3
50 INVERSE 1
60 REVERSE 0
70 BRIGHT 1
80 FLASH 1
90 OVER 1
110 PRINT "MODIFIERS PASSED"
120 END
