10 REM =====================================================================
20 REM Background Task Script
30 REM =====================================================================
40 REM - What can be changed: Print text.
50 REM - What cannot be changed: End statement.
60 REM - What to expect: Execution outputs background progress.
70 REM - What to do if something breaks: Trace file loaders.
80 REM =====================================================================
90 PRINT "BACKGROUND TASK STARTED"
100 FOR I = 1 TO 5
110   PRINT "BG STEP "; I
120 NEXT I
130 PRINT "BACKGROUND TASK FINISHED"
140 END
