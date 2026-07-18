' Tests for File Sharing modes (ACCESS and SHARED)

ON ERROR GOTO ErrorHandler

' Test 1: Open for Output Shared
OPEN "shared.dat" FOR OUTPUT SHARED AS #1
PRINT #1, "HELLO"

' Try to open same file for INPUT (should succeed due to SHARED)
OPEN "shared.dat" FOR INPUT SHARED AS #2
INPUT #2, A$
IF A$ <> "HELLO" THEN PRINT "TEST 1 FAILED": END

CLOSE #1, #2

' Test 2: Access Read/Write checks
OPEN "access.dat" FOR RANDOM ACCESS READ AS #1
' Writing should fail with error 70 (Permission Denied) or similar depending on implementation
PUT #1, 1, A$

GOTO Pass
ErrorHandler:
    IF ERR = 70 THEN
        PRINT "Access Check Passed"
        CLOSE #1
        RESUME NEXT
    END IF
    PRINT "UNEXPECTED ERROR: "; ERR
    END
Pass:
    PRINT "ALL SHARING TESTS PASSED"
    END
