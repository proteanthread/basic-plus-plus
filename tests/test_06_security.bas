10 REM Part 6: Context-Aware Security Verification
20 PRINT "Verifying Security Bounds..."
30 ON ERROR GOTO 100
40 PRINT "Attempting restricted operation..."
50 ' This should fail if run piped, due to SEC_RESTRICTED blocking SECOP_SYSTEM
60 SHELL "echo MALICIOUS"
70 PRINT "Security did NOT block SHELL! Check execution context."
80 END
90 
100 PRINT "Security correctly intercepted the violation! (Error "; ERR; ")"
110 RESUME 80
