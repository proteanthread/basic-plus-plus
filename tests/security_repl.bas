REM ============================================================
REM  TEST: security_repl.bas - REPL Security Probe
REM ============================================================
REM  A set of interactive REPL commands for manual testing.
REM  These are NOT numbered lines; they execute immediately
REM  when fed to the interpreter (e.g., via piped input).
REM
REM  COMMANDS TESTED:
REM   INFO     - Display interpreter version, platform,
REM              compiler, word size, and build info.
REM   SECURITY - Display current security mode
REM              (OPEN, STANDARD, or RESTRICTED).
REM   PRINT 2+2 - Basic expression evaluation (should
REM              print 4). Verifies the expression parser
REM              works in immediate/REPL mode.
REM   MODULE   - List registered and active modules.
REM
REM  EXPECTED: Each command produces its output. No errors.
REM  This test is for interactive use only and should NOT
REM  be run via CHAIN from the menu (it has no line numbers
REM  for program storage and would not work as a stored
REM  program).
REM
REM  NOTE: When piped (basicpp < security_repl.bas), the
REM  interpreter may auto-escalate to RESTRICTED security.
REM ============================================================
REM
INFO
SECURITY
PRINT 2+2
MODULE
