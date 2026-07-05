10 REM Test VAX BASIC Line Continuation &
   PRINT "Line continuation works!"
20 PRINT "A" &
   + "B" &
   + "C"
30 REM Test EDIT$ Function
40 S$ = "  hello [world]  "
50 PRINT "EDIT$ trim spaces, upper, parens: "; EDIT$(S$, 8 + 32 + 64 + 128)
60 PRINT "EDIT$ discard spaces: "; EDIT$(S$, 2)
70 PRINT "EDIT$ compress spaces: "; EDIT$("a  b   c", 16)
80 REM Test NUM$ Function
90 PRINT "NUM$(42): ["; NUM$(42); "]"
100 PRINT "NUM$(-42): ["; NUM$(-42); "]"
110 REM Test NUM Function
120 PRINT "NUM(" + CHR$(34) + "123.45" + CHR$(34) + "):"; NUM("123.45")
