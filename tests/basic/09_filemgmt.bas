10 REM =====================================================================
20 REM BASIC++ v6.0.0 Filesystem Management Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Test directory names, file targets.
60 REM - What cannot be changed: Order of directory creation, naming, and deletion.
70 REM - What to expect: Creates a test directory, renames a file, and removes the test directory.
80 REM - What to do if something breaks: If directory operations fail, check OS write permissions and paths.
90 REM =====================================================================
100 PRINT "RUNNING FILE SYSTEM MANAGEMENT TEST..."
110 MKDIR "test_dir"
120 CHDIR "test_dir"
130 OPEN "test_file.txt" FOR OUTPUT AS #1
140 PRINT #1, "TEMP DATA"
150 CLOSE #1
160 NAME "test_file.txt" AS "renamed.txt"
170 UNSAVE "renamed.txt"
180 CHDIR ".."
190 RMDIR "test_dir"
200 PRINT "FILE SYSTEM MANAGEMENT OK"
210 RUN "tests/basic/10_select.bas"
