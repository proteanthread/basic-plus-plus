10 REM ============================================================
20 REM TEST: EXISTS, FILEMOD$, FILESIZE
30 REM GOAL: Verify filesystem utility functions.
40 REM EXPECTATIONS: Should correctly identify file properties without crashing.
50 REM ============================================================
60 PRINT "Testing EXISTS, FILEMOD$, FILESIZE..."
70 OPEN "O", 1, "test_fs.tmp"
80 PRINT #1, "12345"
90 CLOSE 1
100 E = EXISTS("test_fs.tmp")
110 ASSERT E = 1, "EXISTS failed on created file"
120 S = FILESIZE("test_fs.tmp")
130 ASSERT S > 0, "FILESIZE failed"
140 M$ = FILEMOD$("test_fs.tmp")
150 ASSERT LEN(M$) > 0, "FILEMOD$ failed"
160 KILL "test_fs.tmp"
170 ASSERT EXISTS("test_fs.tmp") = 0, "EXISTS failed on deleted file"
180 PRINT "Filesystem tests passed."
