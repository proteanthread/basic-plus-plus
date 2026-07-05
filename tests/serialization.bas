10 REM ============================================================
20 REM  SERIALIZATION.BAS - Bytecode & Archive Serialization Test
30 REM  round-trip compile -> bsave/bload -> compile bpe -> bload
40 REM ============================================================
50 PRINT "=== Serialization & Archive Test ==="
60 PRINT
70 PRINT "Step 1: Save a temporary source file..."
80 OPEN "ser_temp.bas" FOR OUTPUT AS #1
90 PRINT #1, "10 PRINT \"Hello from ser_temp!\""
100 PRINT #1, "20 X = 42"
110 PRINT #1, "30 PRINT \"Value X:\"; X"
120 CLOSE #1
130 PRINT
140 PRINT "Step 2: Load the source file..."
150 LOAD "ser_temp.bas"
160 PRINT
170 PRINT "Step 3: Compile to bytecode (.BPP)..."
180 COMPILE "ser_temp.bpp"
190 PRINT
200 PRINT "Step 4: Load compiled bytecode..."
210 LOAD "ser_temp.bpp"
220 PRINT
230 PRINT "Step 5: Run compiled bytecode..."
240 RUN
250 PRINT
260 PRINT "Step 6: Package to execution archive (.BPE)..."
270 COMPILE "ser_temp.bas", "ser_temp.bpe"
280 PRINT
290 PRINT "Step 7: Load execution archive..."
300 LOAD "ser_temp.bpe"
310 PRINT
320 PRINT "Step 8: Run execution archive..."
330 RUN
340 PRINT
350 PRINT "Step 9: Testing orphaned bytecode (missing source)..."
360 KILL "ser_temp.bas"
370 LOAD "ser_temp.bpp"
380 RUN
390 PRINT
400 PRINT "Step 10: Clean up temp files..."
410 KILL "ser_temp.bpp"
420 KILL "ser_temp.bpe"
430 PRINT "=== Serialization & Archive test complete ==="
440 END
