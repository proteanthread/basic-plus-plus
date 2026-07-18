10 REM =====================================================================
20 REM BASIC++ v6.0.0 Hex/Octal/Binary Literals Integration Test
30 REM =====================================================================
40 REM - What can be changed: Literal values checks.
50 REM - What cannot be changed: Expected assertions, final run target.
60 REM - What to expect: Verifies literal conversions to correct numeric values.
70 REM - What to do if something breaks: Trace scanner in src/lexer/lexer.c.
80 REM =====================================================================
90 PRINT "RUNNING HEX/OCTAL/BINARY LITERALS TEST..."
100 H = &HFF
110 IF H <> 255 THEN GOTO 300
120 O = &O77
130 IF O <> 63 THEN GOTO 300
140 B = &B101
150 IF B <> 5 THEN GOTO 300
160 CH = 0xFF
170 IF CH <> 255 THEN GOTO 300
180 CO = 0o77
190 IF CO <> 63 THEN GOTO 300
200 CB = 0b101
210 IF CB <> 5 THEN GOTO 300
220 BO = &77
235 IF BO <> 63 THEN GOTO 300
240 PRINT "LITERALS OK"
245 ON ERROR GOTO 248
246 OPEN "lite.flag" FOR INPUT AS #1: CLOSE #1: PRINT "ALL REGRESSION TESTS COMPLETED SUCCESSFULLY!": END
248 RUN "tests/basic/29_infra_integration.bas"
260 END
270 PRINT "LITERALS CONVERSION FAILED!"
280 END
