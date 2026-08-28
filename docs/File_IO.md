# BASIC++ v6.5.2 File I/O Reference

## 1. OPENING FILES

OPEN establishes a connection between a disk file and a numbered channel (1-16). The syntax specifies the filename, access mode, and channel number:

```basic
OPEN "DATA.TXT" FOR INPUT AS #1
OPEN "OUTPUT.TXT" FOR OUTPUT AS #2
OPEN "LOG.TXT" FOR APPEND AS #3
OPEN "RECORDS.DAT" FOR RANDOM AS #4 LEN = 128
OPEN "RAW.BIN" FOR BINARY AS #5
```

**INPUT** mode reads an existing file sequentially from the beginning. If the file does not exist, Error 53 (File not found).

**OUTPUT** mode creates a new file or overwrites an existing file. Writing starts at the beginning.

**APPEND** mode opens an existing file for writing at the end. If the file does not exist, it is created.

**RANDOM** mode opens a file for fixed-length record access. The LEN clause specifies the record length in bytes (default 128). Records are numbered starting at 1.

**BINARY** mode opens a file for raw byte-level access without record structure.

FREEFILE returns the next available channel number: `F = FREEFILE : OPEN "X" FOR INPUT AS #F`.

## 2. SEQUENTIAL FILE READING

INPUT #n reads comma-delimited values from a file channel into variables:

```basic
10 OPEN "DATA.CSV" FOR INPUT AS #1
20 WHILE NOT EOF(1)
30   INPUT #1, Name$, Age, Score
40   PRINT Name$; Age; Score
50 WEND
60 CLOSE #1
```

LINE INPUT #n reads an entire line as a single string, including commas and leading spaces:

```basic
10 OPEN "TEXT.TXT" FOR INPUT AS #1
20 WHILE NOT EOF(1)
30   LINE INPUT #1, L$
40   PRINT L$
50 WEND
60 CLOSE #1
```

INPUT$(n, #channel) reads exactly n characters from the file.

## 3. SEQUENTIAL FILE WRITING

PRINT #n writes values to a file channel using the same formatting as screen PRINT:

```basic
10 OPEN "OUTPUT.TXT" FOR OUTPUT AS #1
20 PRINT #1, "Name: "; Name$
30 PRINT #1, "Score:"; Score
40 CLOSE #1
```

WRITE #n writes values in a machine-readable format with strings in quotes and values separated by commas:

```basic
10 OPEN "DATA.CSV" FOR OUTPUT AS #1
20 WRITE #1, "Alice", 34, 95.5
30 WRITE #1, "Bob", 28, 87.2
40 CLOSE #1
```

This produces: `"Alice",34,95.5` and `"Bob",28,87.2`.

## 4. RANDOM ACCESS FILES

Random access files store fixed-length records that can be read and written by record number in any order.

FIELD defines the layout of a record buffer by mapping byte ranges to string variables:

```basic
10 OPEN "STAFF.DAT" FOR RANDOM AS #1 LEN = 50
20 FIELD #1, 30 AS N$, 2 AS A$, 8 AS S$, 10 AS D$
```

LSET and RSET assign values to field buffer variables. LSET left-justifies; RSET right-justifies:

```basic
30 LSET N$ = "Alice Johnson"
40 LSET A$ = MKI$(34)
50 LSET S$ = MKD$(75000)
60 LSET D$ = "Engineering"
70 PUT #1, 1              ' Write record 1
```

GET reads a record by number:

```basic
80 GET #1, 1              ' Read record 1
90 PRINT N$; CVI(A$); CVD(S$); D$
```

CVI, CVS, CVD convert field buffer strings to numbers. MKI$, MKS$, MKD$ convert numbers to field buffer strings.

## 5. BINARY FILE I/O

BGET and BPUT perform raw byte-level transfers:

```basic
10 OPEN "IMAGE.RAW" FOR BINARY AS #1
20 DIM Buffer(1024)
30 BGET #1, Buffer(), 1024     ' Read 1024 bytes
40 ' ... process buffer ...
50 BPUT #1, Buffer(), 1024     ' Write 1024 bytes
60 CLOSE #1
```

SEEK #n, position sets the byte position within a binary file. SEEK(n) returns the current position.

## 6. FILE POSITION AND STATUS

EOF(n) returns -1 (true) when the end of file n has been reached, 0 (false) otherwise.

LOC(n) returns the current position: for sequential files, the number of 128-byte blocks read; for random files, the last record number read or written; for binary files, the byte position.

LOF(n) returns the total length of the file in bytes.

SEEK #n, pos sets the file position. For random files, pos is a record number. For binary and sequential files, pos is a byte position.

## 7. CLOSING FILES

CLOSE #n closes a specific channel. CLOSE #1, #2, #3 closes multiple channels. CLOSE with no arguments closes all open channels.

RESET closes all open files and flushes all buffers to disk. It is equivalent to CLOSE with no arguments.

## 8. FILE MANAGEMENT

FILES displays a directory listing of the current directory. FILES "*.BAS" filters by pattern. FILES "C:\DATA\" lists a specific directory.

DIR is similar to FILES but returns filenames without size or date information.

KILL "filename" deletes a file. The file must not be currently open.

NAME "oldname" AS "newname" renames a file.

MKDIR "dirname" creates a new directory. RMDIR "dirname" removes an empty directory.

CHDIR "dirname" changes the current working directory. PWD or PWD$ returns the current directory path.

SETATTR "filename", attributes sets file attributes (read-only, hidden, system, archive).

## 9. FILE LOCKING

LOCK #n locks the entire file or a range of records for exclusive access in a multi-user environment:

```basic
LOCK #1                    ' Lock entire file
LOCK #1, 5                 ' Lock record 5
LOCK #1, 5 TO 10           ' Lock records 5 through 10
```

UNLOCK releases the lock: `UNLOCK #1` or `UNLOCK #1, 5 TO 10`.

## 10. ERROR HANDLING IN FILE I/O

File operations commonly produce these errors:

| Code | Error | Common Cause |
|------|-------|-------------|
| 52 | Bad file number | Channel not open or number out of range |
| 53 | File not found | File does not exist (INPUT mode) |
| 54 | Bad file mode | Operation incompatible with mode |
| 55 | File already open | Channel already in use |
| 58 | File already exists | Cannot create file that exists |
| 61 | Disk full | No free space |
| 62 | Input past end | Reading past EOF |
| 64 | Bad file name | Invalid characters in filename |
| 67 | Too many files | More than 16 channels open |
| 70 | Permission denied | Security level blocks file access |

Use ON ERROR GOTO or TRY/CATCH to handle file errors gracefully.
