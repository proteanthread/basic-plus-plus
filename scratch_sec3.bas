3000 PRINT "3. Testing String Descriptor PEEK/POKE..."
3010 S$ = "HEllo"
3020 PRINT "After assignment"
3025 pS = VARPTR(S$)
3030 PRINT "After VARPTR: "; pS
3035 sLen = PEEK(pS)
3040 PRINT "After PEEK length: "; sLen
3050 pData = PEEK(pS + 1) + PEEK(pS + 2)*256
3060 PRINT "After data pointer math: "; pData
