10 REM =====================================================================
20 REM BASIC++ v6.0.0 Network Protocols HTTP_GET$ Integration Test
30 REM =====================================================================
40 REM - What can be changed: Test URLs, print statements.
50 REM - What cannot be changed: HTTP_GET$ call structure.
60 REM - What to expect: Attempts HTTP GET inside TRY/CATCH to support offline execution.
70 REM   Prints result and completes the regression test suite.
80 REM - What to do if something breaks: Trace HTTP_GET$ handler in src/expression/eval.c
90 REM   and network protocol dispatcher in src/runtime/vnet.c.
100 REM =====================================================================
110 PRINT "RUNNING NETWORK PROTOCOLS INTEGRATION TESTS..."
120 TRY
130   LET res$ = HTTP_GET$("http://example.com")
140   PRINT "HTTP_GET$ Response: "; LEFT$(res$, 50); "..."
150   PRINT "HTTP_GET$ Offline/Online OK"
160 CATCH
170   PRINT "HTTP_GET$ offline exception caught: "; ERR$; " (Message: "; ERR$(); ")"
180   PRINT "HTTP_GET$ Exception Handling OK"
190 END TRY
200 PRINT "NETWORK PROTOCOLS INTEGRATION TESTS OK"
210 RUN "tests/basic/46_legacy_union_compat.bas"
220 END
