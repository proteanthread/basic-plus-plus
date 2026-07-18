' What can be changed: Filename and values.
' What cannot be changed: Test structure and output assertions.
' What to expect: Proper buffering of FIELD string variables.
' What to do if something breaks: Check VDev internal record buffers.

10 F = FREEFILE()
20 IF F < 1 THEN PRINT "FAIL FREEFILE": END
30 OPEN "test_funcs.dat" FOR OUTPUT AS #F
40 PRINT #F, "HELLO WORLD"
50 CLOSE F

100 F = FREEFILE()
110 OPEN "test_funcs.dat" FOR INPUT AS #F
120 IF FILEATTR(F, 1) <> 1 THEN PRINT "FAIL FILEATTR 1": END
125 IF LOC(F) <> 0 THEN PRINT "FAIL LOC": END
130 IF FILEATTR(F, 2) <> F THEN PRINT "FAIL FILEATTR 2": END
140 ' LOF should be > 0
150 IF LOF(F) <= 0 THEN PRINT "FAIL LOF": END
160 CLOSE F
170 PRINT "PASS"
180 SYSTEM
