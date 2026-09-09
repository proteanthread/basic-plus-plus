<!--
Title:        Stream_Block_IO
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/, engine/src/runtime/vfs.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Stream and Block I/O Architecture

The authoritative specification for unformatted byte streams, binary channel operations, record locking, block transfers, and virtual file streams in BASIC++ v6.5.2.

---

## 1. Stream I/O Architecture

BASIC++ provides comprehensive stream I/O mechanisms treating files and virtual devices as unformatted byte sequences. Unlike record-oriented random access files, stream channels allow arbitrary byte reading, writing, seeking, and bulk transfers.

The I/O subsystem is implemented in `engine/src/statements/io/` and virtualized via `libscript` and `vfs.c`.

---

## 2. Binary File Channel Modes

Binary streams are opened using the `FOR BINARY` clause:

```basic
OPEN "filename.dat" FOR BINARY AS #1
```

In binary mode:
- End-of-line translations (CRLF to LF) are disabled.
- Null bytes (`0x00`) and arbitrary binary octets pass without filtering.
- Record lengths are unconstrained; file position is maintained as a 64-bit byte offset.

---

## 3. Byte and Block Operations

### A. Position and Inspection
- **`SEEK #filenum, position`**: Positions the file pointer to the specified 1-based byte offset.
- **`SEEK(filenum)`**: Built-in function returning the current 1-based byte position in channel `filenum`.
- **`LOC(filenum)`**: Returns the current position in the file channel (in bytes or blocks).
- **`LOF(filenum)`**: Built-in function returning the total length of file `filenum` in bytes.
- **`EOF(filenum)`**: Returns true (`-1`) when the file pointer reaches or exceeds the end of file.

### B. Block Reading and Writing
- **`GET #filenum, [position], variable`**: Reads bytes directly into `variable` from the current or specified byte offset.
- **`PUT #filenum, [position], variable`**: Writes the raw binary representation of `variable` into the file stream.
- **`GET$ #filenum, count`**: Reads an exact count of raw bytes from the channel as a string.

---

## 4. Example: Binary File Copy

```basic
10 REM Binary Stream File Copy
20 InFile$ = "source.bin" : OutFile$ = "dest.bin"
30 OPEN InFile$ FOR BINARY AS #1
40 OPEN OutFile$ FOR BINARY AS #2
50 TotalBytes = LOF(1)
60 BlockSize = 4096
70 WHILE NOT EOF(1)
80   Remaining = LOF(1) - LOC(1)
90   Chunk = MIN(BlockSize, Remaining)
100  IF Chunk <= 0 THEN EXIT WHILE
110  Buffer$ = GET$(1, Chunk)
120  PUT #2, , Buffer$
130 WEND
140 CLOSE #1, #2
150 PRINT "Copied "; TotalBytes; " bytes successfully."
```
