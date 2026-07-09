' test_transaction_io.bas
' Tests Transaction I/O functions

10 PRINT "=== TRANSACTION IO TEST ==="
20 ' Verify that without a transaction, status returns 0
30 IF TXNSTATUS() <> 0 THEN PRINT "FAIL TXNSTATUS": END

200 PRINT "PASS TRANSACTION IO"
210 END
