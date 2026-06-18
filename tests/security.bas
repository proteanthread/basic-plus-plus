10 REM ============================================================
20 REM  TEST: security.bas - Context-Aware Security Verification
30 REM ============================================================
40 REM  Tests the security sandbox enforcement:
50 REM   - ON ERROR GOTO: sets up error trap at line 1000
60 REM   - SHELL: attempts a restricted operation
70 REM   - If security is RESTRICTED or STANDARD (piped),
80 REM     SHELL should be blocked and raise an error.
82 REM   - If security is OPEN (interactive), SHELL will
84 REM     execute and print a warning instead.
86 REM
88 REM  EXPECTED (RESTRICTED/piped): Error trap fires,
90 REM  prints "Security correctly intercepted the violation!"
92 REM  with the error number.
94 REM  EXPECTED (OPEN/interactive): SHELL executes,
96 REM  prints "echo MALICIOUS" output and a warning that
97 REM  security did not block it.
98 REM
99 REM  NOTE: Security mode depends on execution context.
100 REM  Piped execution (basicpp < security.bas) triggers
102 REM  RESTRICTED mode automatically. Interactive REPL
104 REM  with SECURITY=OPEN will not block SHELL.
106 REM ============================================================
110 REM
120 PRINT "Verifying Security Bounds..."
130 REM
140 REM --- Set up error trap ---
150 REM  ON ERROR GOTO redirects runtime errors to line 1000.
160 REM  Without this, a security violation would abort.
170 ON ERROR GOTO 1000
180 REM
190 REM --- Attempt restricted operation ---
200 REM  SHELL tries to execute an OS command.
210 REM  In RESTRICTED mode, SECOP_SYSTEM is blocked,
220 REM  raising a runtime error caught by ON ERROR.
230 PRINT "Attempting restricted operation..."
240 SHELL "echo MALICIOUS"
250 REM
260 REM --- If we reach here, SHELL was allowed ---
270 PRINT "Security did NOT block SHELL! Check execution context."
280 END
290 REM
1000 REM --- Error handler ---
1010 REM  RESUME sends execution to line 280 (END).
1020 REM  ERR contains the error code of the violation.
1030 PRINT "Security correctly intercepted the violation! (Error "; ERR; ")"
1040 RESUME 280
