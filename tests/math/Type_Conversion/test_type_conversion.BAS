10 REM Test Type Conversion Functions: HEX$, OCT$, BIN$, NUM, NUM$, VAL, STR$
20 IF HEX$(255) <> "FF" THEN PRINT "HEX$ FAIL 1": END
30 IF OCT$(8) <> "10" THEN PRINT "OCT$ FAIL 1": END
40 IF BIN$(5) <> "101" THEN PRINT "BIN$ FAIL 1": END
50 REM Testing STR$ and VAL
60 IF STR$(123) <> " 123" THEN PRINT "STR$ FAIL 1": END
70 IF STR$(-45) <> "-45" THEN PRINT "STR$ FAIL 2": END
80 IF VAL("123") <> 123 THEN PRINT "VAL FAIL 1": END
90 IF VAL("-45") <> -45 THEN PRINT "VAL FAIL 2": END
100 REM Testing NUM and NUM$ (CVI/CVS/CVD equivalents or similar)
110 REM Assuming NUM$ converts number to string bytes, and NUM converts bytes back
120 REM Just a basic check if they exist and run without error
130 A$ = NUM$(12345)
140 IF NUM(A$) <> 12345 THEN PRINT "NUM FAIL 1": END
150 PRINT "test_type_conversion PASSED"
160 END
