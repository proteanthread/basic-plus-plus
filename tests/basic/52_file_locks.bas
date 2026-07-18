' Tests for Record/Range Locks

ON ERROR GOTO ErrorHandler

OPEN "locktest.dat" FOR RANDOM AS #1 LEN = 32
' Write some dummy data
A = 123.45
BPUT #1, 1, A

' Lock record 1
LOCK #1, 1 TO 1

' Try to open same file on channel 2
OPEN "locktest.dat" FOR RANDOM SHARED AS #2 LEN = 32

' Try to read locked record
BGET #2, 1, B

GOTO Pass

ErrorHandler:
    IF ERR = 70 THEN
        PRINT "Lock collision detected correctly!"
        UNLOCK #1, 1 TO 1
        CLOSE #1, #2
        END
    END IF
    PRINT "UNEXPECTED ERROR: "; ERR
    END
Pass:
    PRINT "TEST FAILED: Lock collision did not occur!"
    END
