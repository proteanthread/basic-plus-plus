10 REM ============================================================
20 REM TEST: CVD, CVI, CVS
30 REM GOAL: Verify string-to-numeric type conversions.
40 REM EXPECTATIONS: MKD$/CVD, MKI$/CVI, MKS$/CVS should be reversible.
50 REM ============================================================
60 PRINT "Testing CVI..."
70 I_VAL = 1234
80 I_STR$ = MKI$(I_VAL)
90 ASSERT CVI(I_STR$) = I_VAL, "CVI/MKI$ roundtrip failed"
100 PRINT "Testing CVS..."
110 S_VAL = 123.45!
120 S_STR$ = MKS$(S_VAL)
130 REM Floating point might have slight precision issues, use ABS
140 ASSERT ABS(CVS(S_STR$) - S_VAL) < 0.01, "CVS/MKS$ roundtrip failed"
150 PRINT "Testing CVD..."
160 D_VAL = 1234567.89#
170 D_STR$ = MKD$(D_VAL)
180 ASSERT ABS(CVD(D_STR$) - D_VAL) < 0.01, "CVD/MKD$ roundtrip failed"
190 PRINT "CV tests passed."
