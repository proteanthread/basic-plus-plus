10 REM =====================================================================
20 REM BASIC++ v6.0.0 Multitasking Task Manager Integration Test
30 REM =====================================================================
40 REM - What can be changed: Prints, loop steps.
50 REM - What cannot be changed: Spawning target name, join operations.
60 REM - What to expect: Spawns task_bg.bas and waits for completion.
70 REM - What to do if something breaks: Trace thread handles in src/runtime/task.c.
80 REM =====================================================================
90 PRINT "RUNNING MULTITASKING TEST..."
100 PID = TASK("tests/basic/task_bg.bas")
110 IF PID <= 0 THEN GOTO 160
120 TASK
130 TASK WAIT PID
140 PRINT "MULTITASKING OK"
145 ON ERROR GOTO 148
146 OPEN "lite.flag" FOR INPUT AS #1: CLOSE #1: RUN "tests/basic/17_program_edit.bas"
148 RUN "tests/basic/16_matrix_math.bas"
160 PRINT "FAILED TO SPAWN TASK"
170 END
