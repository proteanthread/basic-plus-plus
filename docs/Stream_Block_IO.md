# BASIC++ v6.5.2 Stream and Block I/O

## 1. STREAM I/O

Stream I/O treats a file as a continuous sequence of bytes without record structure. BASIC++ provides stream operations through the BINARY file mode and through the standard sequential modes (INPUT, OUTPUT, APPEND).

### Byte-Level Streams

BGET reads a block of bytes from an open BINARY file into a buffer array:

```basic
10 OPEN "DATA.BIN" FOR BINARY AS #1
20 DIM Buffer%(256)
30 BGET #1, Buffer%(), 256
40 CLOSE #1
```

BPUT writes a block of bytes from a buffer array to an open BINARY file:

```basic
10 OPEN "OUTPUT.BIN" FOR BINARY AS #1
20 DIM Data%(128)
30 FOR I = 0 TO 127 : Data%(I) = I : NEXT I
40 BPUT #1, Data%(), 128
50 CLOSE #1
```

### Character Streams

INPUT$(n, #channel) reads exactly n characters as a string. This is useful for reading fixed-width text fields from data files:

```basic
10 OPEN "FIXED.DAT" FOR INPUT AS #1
20 WHILE NOT EOF(1)
30   Name$ = INPUT$(20, #1)       ' 20-char name field
40   Code$ = INPUT$(5, #1)        ' 5-char code field
50   Amount$ = INPUT$(10, #1)     ' 10-char amount field
60   PRINT Name$; Code$; VAL(Amount$)
70 WEND
80 CLOSE #1
```

## 2. BLOCK I/O

Block I/O transfers entire data structures between memory and disk in a single operation. This is faster than field-by-field reading for large datasets.

### User-Defined Type Records

When a file is opened FOR RANDOM with a LEN matching a TYPE size, GET and PUT transfer entire TYPE records:

```basic
10 TYPE Invoice
20   Number AS LONG
30   Customer AS STRING * 30
40   Amount AS DOUBLE
50   Paid AS INTEGER
60 END TYPE
70 DIM Inv AS Invoice
80 OPEN "INVOICES.DAT" FOR RANDOM AS #1 LEN = LEN(Inv)
90 ' Write a record
100 Inv.Number = 1001
110 Inv.Customer = "Acme Corp"
120 Inv.Amount = 4599.99
130 Inv.Paid = 0
140 PUT #1, 1, Inv
150 ' Read it back
160 GET #1, 1, Inv
170 PRINT Inv.Number; Inv.Customer; Inv.Amount
180 CLOSE #1
```

### Array Block Transfer

BGET and BPUT can transfer entire arrays to and from disk:

```basic
10 DIM Matrix(100, 100)
20 ' Fill matrix
30 FOR I = 0 TO 100 : FOR J = 0 TO 100
40   Matrix(I, J) = RND
50 NEXT J : NEXT I
60 ' Save to disk
70 OPEN "MATRIX.DAT" FOR BINARY AS #1
80 BPUT #1, Matrix(), 101 * 101 * 8     ' 8 bytes per double
90 CLOSE #1
```

## 3. SEEK AND POSITION

SEEK #n, pos positions the file pointer for the next read or write operation. For BINARY files, pos is a byte offset (1-based). For RANDOM files, pos is a record number (1-based).

```basic
10 OPEN "DATA.BIN" FOR BINARY AS #1
20 SEEK #1, 100          ' Position at byte 100
30 A$ = INPUT$(10, #1)   ' Read 10 bytes starting at position 100
40 CLOSE #1
```

LOC(n) returns the current position. For BINARY files, it returns the byte position of the last byte read or written. For RANDOM files, it returns the last record number.

## 4. PIPED I/O

The bs batch runner supports piped I/O through standard input and output. When a BASIC++ script is executed in a pipeline, INPUT reads from stdin and PRINT writes to stdout:

```bash
echo "42" | bs compute.bas > result.txt
cat data.csv | bs process.bas | sort > sorted.csv
```

Inside the script, INPUT without a channel reads from stdin. PRINT without a channel writes to stdout. Error messages from unhandled errors go to stderr.

## 5. VIRTUAL FILE CHANNELS

BASIC++ routes all file operations through the virtual device layer (VDev). Physical file access goes through the platform abstraction (plat_fs.c). This means file operations work identically on Windows, Linux, and FreeDOS — path separators are normalized, and character encoding is handled transparently.

Special device names can be opened as files: `OPEN "CON:" FOR OUTPUT AS #1` opens the console. `OPEN "NUL:" FOR OUTPUT AS #1` opens the null device. `OPEN "LPT1:" FOR OUTPUT AS #1` opens the printer.
