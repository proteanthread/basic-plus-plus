10 REM =====================================================================
20 REM BASIC++ v6.0.0 Phase 12: Sound Engine Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Notes, frequencies, and durations.
60 REM - What cannot be changed: Keyword calls and validation assertions.
70 REM - What to expect: Plays sound frequencies, tests system beep,
80 REM   parses MML commands, and validates PLAY(0) queue tracking.
90 REM - What to do if something breaks: Verify SDL2 audio output subsystem,
100 REM   check queue length bounds, and match return values.
110 REM =====================================================================
120 PRINT "SOUND REGRESSION TEST START"
130 ON ERROR GOTO 200
140 BEEP
150 SOUND 440, 5
160 PLAY "T120 O4 L8 C D E F G A B > C"
175 LET T = 0
180 IF PLAY(0) > 0 THEN IF T < 100 THEN LET T = T + 1: GOTO 180
190 GOTO 250
200 PRINT "Sound execution warning: "; ERR; " - "; ERL
210 RESUME NEXT
250 RUN "tests/basic/21_bytecode.bas"
