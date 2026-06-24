5000 PRINT "5. Testing Array PEEK/POKE..."
5010 DIM E%(2)
5015 PRINT "After DIM"
5020 E%(0) = 513
5025 PRINT "After E%(0) assignment"
5030 E%(1) = 1027
5035 PRINT "After E%(1) assignment"
5040 pE = VARPTR(E%(0))
5045 PRINT "After VARPTR: "; pE
5050 bL0 = PEEK(pE)
5055 PRINT "After PEEK pE: "; bL0
5060 bH0 = PEEK(pE + 1)
5065 PRINT "After PEEK pE+1: "; bH0
5070 bL1 = PEEK(pE + 8)
5075 PRINT "After PEEK pE+8: "; bL1
5080 bH1 = PEEK(pE + 9)
5085 PRINT "After PEEK pE+9: "; bH1
