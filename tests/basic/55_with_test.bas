10 REM =====================================================================
20 REM BASIC++ WITH Statement Integration Test
30 REM =====================================================================
40 REM - What can be changed: Struct fields, assignment values.
50 REM - What cannot be changed: WITH...END WITH syntax, validations.
60 REM - What to expect: Verification of implicit WITH member resolution.
70 REM - What to do if something breaks: Check stmt_let.c or eval.c.
80 REM =====================================================================
100 PRINT "RUNNING WITH BLOCKS INTEGRATION TESTS..."

110 TYPE Address
120   street AS STRING
130   zip AS NUMBER
140 END TYPE

150 TYPE Person
160   fullname AS STRING
170   age AS NUMBER
180   addr AS Address
190 END TYPE

200 DIM p1 AS Person

210 REM Test basic WITH block assignment
220 WITH p1
230   .fullname = "Bob"
240   .age = 30
250 END WITH

260 IF p1.fullname <> "Bob" THEN PRINT "FAILED: basic WITH string assignment": END
270 IF p1.age <> 30 THEN PRINT "FAILED: basic WITH numeric assignment": END

280 REM Test nested WITH blocks
290 WITH p1
300   WITH .addr
315     .street = "789 Pine Rd"
320     .zip = 95051
330   END WITH
340 END WITH

350 IF p1.addr.street <> "789 Pine Rd" THEN PRINT "FAILED: nested WITH assignment 1": END
360 IF p1.addr.zip <> 95051 THEN PRINT "FAILED: nested WITH assignment 2": END

370 REM Test evaluation inside WITH block
380 WITH p1
390   IF .age <> 30 THEN PRINT "FAILED: WITH member evaluation": END
400 END WITH

410 PRINT "WITH BLOCKS INTEGRATION TESTS OK"
420 END
