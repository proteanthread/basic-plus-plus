10 REM ============================================================
20 REM  BPLTEST.BAS - BPL Round-trip Test
30 REM  compile -> save -> load -> call
40 REM ============================================================
50 REM
100 PRINT "=== BPL Round-trip Test ==="
110 PRINT
120 PRINT "Step 1: Load source library..."
130 LOAD LIBRARY "samples/TURTLE.LIB"
140 PRINT
150 PRINT "Step 2: Compile to .BPL..."
160 COMPILE LIBRARY "TURTLE"
170 PRINT
180 PRINT "Step 3: Unload source library..."
190 UNLOAD LIBRARY "TURTLE"
200 PRINT
210 PRINT "Step 4: Load compiled .BPL..."
220 LOAD LIBRARY "TURTLE.bpl"
230 PRINT
240 PRINT "Step 5: Call from .BPL..."
250 CALL TURTLE_INIT(40, 12)
260 PRINT "TURTLE_INIT returned OK"
270 PRINT
280 PRINT "=== BPL round-trip complete ==="
290 UNLOAD LIBRARY "TURTLE"
300 END
