10 REM Test String Case Functions: UCASE$, LCASE$, TCASE$, ICASE$, MCASE$
20 A$ = "Hello World"
30 IF UCASE$(A$) <> "HELLO WORLD" THEN PRINT "UCASE$ FAIL 1": END
40 IF LCASE$(A$) <> "hello world" THEN PRINT "LCASE$ FAIL 1": END
50 REM TCASE$ is Toggle Case
60 IF TCASE$(A$) <> "hELLO wORLD" THEN PRINT "TCASE$ FAIL 1": END
70 REM ICASE$ is Inverse Case (same as Toggle Case in some dialects, assuming it flips case)
80 IF ICASE$(A$) <> "hELLO wORLD" THEN PRINT "ICASE$ FAIL 1": END
90 REM MCASE$ is Mixed Case or Title Case
100 IF MCASE$("hello world") <> "Hello World" THEN PRINT "MCASE$ FAIL 1": END
110 PRINT "test_string_case PASSED"
120 END
