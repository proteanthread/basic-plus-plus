10 MOUNT "T:" TO "C:\Users\rtdos\test"
20 MOUNTS
30 OPEN "T:hello.txt" FOR OUTPUT AS #1
40 PRINT #1, "Hello from virtual drive!"
50 CLOSE #1
60 PRINT "File written via mount."
70 VPATH "lib;modules;."
80 PRINT "VPATH = "; VPATH$
90 VPATH
100 UMOUNT "T:"
110 MOUNTS
120 PRINT "Done."
