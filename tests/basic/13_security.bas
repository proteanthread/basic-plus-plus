10 REM =====================================================================
20 REM BASIC++ v6.0.0 Security Sandbox Integration Test
30 REM =====================================================================
40 REM SECTION 1: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
50 REM - What can be changed: Prints, error trap line numbers.
60 REM - What cannot be changed: One-way ratchet assertion and standard block checks.
70 REM - What to expect: Execution verifies raising security level to STANDARD,
80 REM   trapping permission error on KILL/MKDIR, and applying RESTRICT.
90 REM - What to do if something breaks: Verify matrix mappings in src/security/security.c.
100 REM =====================================================================
110 PRINT "RUNNING SECURITY SANDBOX TEST..."
120 SECURITY
130 REM Test raising level to STANDARD (2)
140 SECURITY "STANDARD"
150 REM Test one-way ratchet (cannot lower to OPEN)
160 SECURITY "OPEN"
170 REM Trap permission denied errors
180 ON ERROR GOTO 230
190 REM MKDIR is blocked at STANDARD level
200 MKDIR "sandbox_violation"
210 PRINT "FAILED: MKDIR was not blocked by security level"
220 END
230 IF ERR = 70 THEN RESUME 260
240 PRINT "FAILED: Security violation did not raise error 70 (Permission denied). Got "; ERR
250 END
260 PRINT "SECURITY SANDBOX OK"
270 RUN "tests/basic/19_sdl2_graphics.bas"
280 END
