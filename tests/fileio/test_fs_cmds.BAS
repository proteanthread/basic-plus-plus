
10 REM Test NAME, SCRATCH, MKDIR, RMDIR, PWD, CHDIR
20 OPEN "tests/fileio/t_scratch.txt" FOR OUTPUT AS #1
30 PRINT #1, "TEST"
40 CLOSE #1
50 NAME "tests/fileio/t_scratch.txt" AS "tests/fileio/t_renamed.txt"
60 OPEN "tests/fileio/t_renamed.txt" FOR INPUT AS #1
70 LINE INPUT #1, A$
80 CLOSE #1
90 IF A$ <> "TEST" THEN PRINT "FAIL NAME": END
100 SCRATCH "tests/fileio/t_renamed.txt"
110 ON ERROR GOTO 140
120 OPEN "tests/fileio/t_renamed.txt" FOR INPUT AS #1
130 PRINT "FAIL SCRATCH - FILE STILL EXISTS": END
140 RESUME 150
150 REM Test MKDIR, RMDIR
160 MKDIR "tests/fileio/t_dir"
170 RMDIR "tests/fileio/t_dir"
180 C$ = CWD$ : ASSERT LEN(C$) > 0
185 CHDIR C$ : REM Test that we can chdir back to current dir
190 PRINT "PASS FS CMDS"
200 END
