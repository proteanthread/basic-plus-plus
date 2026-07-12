STREAM I/O, BLOCK I/O, AND DEVICE ALIASES
==========================================
BASIC++ Tutorial — Milestone 11 Primitives
Version 4.2.3


This tutorial covers the three I/O subsystems added in BASIC++
Milestone 11:

  1. Stream I/O (SIO)  — sequential byte/line operations
  2. Block I/O  (BIO)  — random-access position-addressed reads/writes
  3. Device Aliases     — cross-dialect device name mapping
  4. Transactions       — ATOMIC blocks and TXN journaling

These subsystems work alongside the traditional PRINT#/INPUT#/
FIELD/GET/PUT statements.  You can freely mix old and new I/O
on the same channels.


TABLE OF CONTENTS
=================

  Part I:   Stream I/O (SIO Primitives)
    1. What Are SIO Functions?
    2. SIO Function Reference
    3. Tutorial: Reading a File with SIO
    4. Tutorial: Writing a Log File with SIO
    5. Tutorial: Binary Protocol with SIO
    6. SIO and VDev Channels
    7. SIO vs. Traditional I/O — When to Use Which

  Part II:  Block I/O (BIO Primitives)
    8. What Are BIO Functions?
    9. BIO Function Reference
   10. Tutorial: Random-Access Record Store
   11. Tutorial: Binary File Editing with BIO
   12. Tutorial: File Integrity Checking
   13. Tutorial: Block Copy and Fill Operations
   14. BIO vs. FIELD/GET/PUT — When to Use Which

  Part III: Device Aliases
   15. What Are Device Aliases?
   16. The Alias Resolution Chain
   17. Built-In Dialect Aliases
   18. Tutorial: Using Atari Device Names on Modern Systems
   19. Tutorial: Using GW-BASIC Device Names on Linux
   20. Tutorial: Using C64 Device Numbers
   21. Tutorial: Custom Aliases with DEVMAP
   22. Alias Direction and Safety
   23. Cross-Dialect Device Mapping Table

  Part IV:  Transactions (ATOMIC / TXN)
   24. What Are Transactions?
   25. ATOMIC Blocks — Auto-Rollback on Error
   26. TXN Explicit Transactions
   27. TXNSTATUS() and TXN STATUS
   28. Tutorial: Safe Database Update
   29. Tutorial: All-or-Nothing Config File Write
   30. Journal Limits and Best Practices

  Part V:   Cross-Reference
   31. SIO/BIO/TXN Quick Reference Card
   32. Full Device Alias Table
   33. Related Documentation


=====================================================================

            PART I: STREAM I/O (SIO PRIMITIVES)

=====================================================================


=====================================================================
1. WHAT ARE SIO FUNCTIONS?
=====================================================================

SIO (Stream I/O) functions provide a uniform, low-level interface
for sequential byte-stream operations.  Unlike traditional
PRINT#/INPUT# which are text-oriented and dialect-specific, SIO
functions work at the byte level and behave identically across
all dialects.

SIO functions:
  - Operate on open file channels (#1 through #8)
  - Work with both file-backed AND VDev-backed channels
  - Return values (bytes read, bytes written, status codes)
  - Use strings as byte buffers (no FIELD mapping needed)

Think of SIO as the "C stdio" layer of BASIC++:
  SIOREAD$  ≈ fread()
  SIOWRITE  ≈ fwrite()
  SIOSEEK   ≈ fseek()
  SIOFLUSH  ≈ fflush()


=====================================================================
2. SIO FUNCTION REFERENCE
=====================================================================

---------------------------------------------------------------------
SIOREAD$(chan, n)
---------------------------------------------------------------------
  Read up to n bytes from channel, return as a string.

  Arguments:
    chan   Channel number (1-8)
    n     Maximum bytes to read (1-1024)

  Returns:
    String containing the bytes read, or "" on error/EOF.

  Notes:
    Reads raw bytes — does NOT stop at newlines.
    Actual length may be less than n (partial read/EOF).
    Works with VDev channels (calls dev_read).

  Example:
    10 OPEN "data.bin" FOR BINARY AS #1
    20 A$ = SIOREAD$(1, 256)
    30 PRINT "Read"; LEN(A$); "bytes"
    40 CLOSE #1

---------------------------------------------------------------------
SIOREADLN$(chan)
---------------------------------------------------------------------
  Read one line from channel, return as a string.

  Arguments:
    chan   Channel number (1-8)

  Returns:
    String up to the next newline (newline stripped), or ""
    on error/EOF.

  Notes:
    Maximum line length: 1024 bytes.
    Strips trailing CR/LF.
    Works with VDev channels (calls dev_gets).

  Example:
    10 OPEN "log.txt" FOR INPUT AS #1
    20 WHILE NOT EOF(1)
    30   L$ = SIOREADLN$(1)
    40   PRINT L$
    50 WEND
    60 CLOSE #1

---------------------------------------------------------------------
SIOWRITE(chan, data$)
---------------------------------------------------------------------
  Write a string (as raw bytes) to channel.

  Arguments:
    chan    Channel number (1-8)
    data$  String to write

  Returns:
    Number of bytes actually written.

  Notes:
    Does NOT add newline (unlike PRINT#).
    Transaction-aware: if a TXN or ATOMIC is active, the
    original data at the write position is journaled.
    Works with VDev channels (calls dev_write).

  Example:
    10 OPEN "out.bin" FOR BINARY AS #1
    20 NW = SIOWRITE(1, "HEADER" + CHR$(0))
    30 PRINT "Wrote"; NW; "bytes"
    40 CLOSE #1

---------------------------------------------------------------------
SIOSEEK(chan, pos)
---------------------------------------------------------------------
  Seek to byte position in channel.

  Arguments:
    chan   Channel number (1-8)
    pos   Byte position (0-based)

  Returns:
    New position after seek, or -1 on error.

  Notes:
    Position is 0-based (first byte = position 0).
    Works with seekable VDev channels.
    Not all channels are seekable (CON: is not).

  Example:
    10 OPEN "data.bin" FOR BINARY AS #1
    20 P = SIOSEEK(1, 100)    ' Jump to byte 100
    30 A$ = SIOREAD$(1, 10)   ' Read 10 bytes there
    40 CLOSE #1

---------------------------------------------------------------------
SIOFLUSH(chan)
---------------------------------------------------------------------
  Flush buffered output to the underlying device.

  Arguments:
    chan   Channel number (1-8)

  Returns:
    0 on success, -1 on error.

  Example:
    10 OPEN "log.txt" FOR OUTPUT AS #1
    20 NW = SIOWRITE(1, "Emergency: system fault")
    30 NF = SIOFLUSH(1)     ' Ensure it's on disk NOW
    40 CLOSE #1

---------------------------------------------------------------------
SIOSTATUS(chan)
---------------------------------------------------------------------
  Query channel status flags (bitmask).

  Arguments:
    chan   Channel number (1-8)

  Returns:
    Bitmask integer:
      Bit 0 (1):   Channel is open
      Bit 1 (2):   EOF reached
      Bit 2 (4):   Error state
      Bit 3 (8):   VDev-backed (not a plain file)

  Example:
    10 OPEN "test.txt" FOR INPUT AS #1
    20 S = SIOSTATUS(1)
    30 IF S AND 1 THEN PRINT "Channel open"
    40 IF S AND 8 THEN PRINT "VDev-backed"
    50 CLOSE #1

---------------------------------------------------------------------
SIOAVAIL(chan)
---------------------------------------------------------------------
  Estimate bytes available for immediate read.

  Arguments:
    chan   Channel number (1-8)

  Returns:
    For files: bytes remaining (LOF - current position).
    For VDevs: result from dev_avail callback, or -1.

  Example:
    10 OPEN "data.bin" FOR BINARY AS #1
    20 PRINT "Bytes available:"; SIOAVAIL(1)
    30 CLOSE #1


=====================================================================
3. TUTORIAL: READING A FILE WITH SIO
=====================================================================

Problem: Read a binary file and display hex bytes.

  10 REM --- Hex Dump using SIO ---
  20 OPEN "data.bin" FOR BINARY AS #1
  30 PRINT "File size:"; SIOAVAIL(1); "bytes"
  40 ADDR = 0
  50 WHILE NOT EOF(1)
  60   A$ = SIOREAD$(1, 16)
  70   IF LEN(A$) = 0 THEN GOTO 120
  80   PRINT USING "####: "; ADDR;
  90   FOR I = 1 TO LEN(A$)
 100     PRINT USING "## "; ASC(MID$(A$, I, 1));
 110   NEXT I
 120   PRINT
 130   ADDR = ADDR + LEN(A$)
 140 WEND
 150 CLOSE #1

Why SIO instead of traditional I/O?
  - SIOREAD$ reads raw bytes, not text lines
  - No FIELD mapping needed
  - No record length to configure
  - Works on any file type (text, binary, device)


=====================================================================
4. TUTORIAL: WRITING A LOG FILE WITH SIO
=====================================================================

Problem: Append timestamped entries to a log file.

  10 REM --- Log Writer using SIO ---
  20 OPEN "app.log" FOR APPEND AS #1
  30 FOR I = 1 TO 5
  40   TS$ = STR$(I) + ": Event logged"
  50   NW = SIOWRITE(1, TS$ + CHR$(13) + CHR$(10))
  60   NF = SIOFLUSH(1)
  70 NEXT I
  80 CLOSE #1
  90 PRINT "5 entries logged."

Key points:
  - SIOWRITE does NOT add a newline — you must add CHR$(13)+CHR$(10)
    yourself (or just CHR$(10) on Linux).
  - SIOFLUSH ensures each entry is written to disk immediately.
  - NW tells you exactly how many bytes were written.


=====================================================================
5. TUTORIAL: BINARY PROTOCOL WITH SIO
=====================================================================

Problem: Send/receive a simple binary protocol over a serial VDev.

  10 REM --- Binary Protocol via SIO ---
  20 REM Assumes SER: VDev is registered
  30 OPEN "SER:" FOR RANDOM AS #1
  40 REM Send a 4-byte command: STX, CMD, LEN, ETX
  50 CMD$ = CHR$(2) + CHR$(65) + CHR$(0) + CHR$(3)
  60 NW = SIOWRITE(1, CMD$)
  70 PRINT "Sent"; NW; "bytes"
  80 REM Read response (up to 64 bytes)
  90 R$ = SIOREAD$(1, 64)
 100 PRINT "Got"; LEN(R$); "bytes back"
 110 IF LEN(R$) > 0 THEN PRINT "First byte:"; ASC(LEFT$(R$, 1))
 120 CLOSE #1

Why SIO for protocols?
  - Byte-exact writes (no added newlines or formatting)
  - Byte-exact reads (no text line splitting)
  - Return values tell you exactly what happened


=====================================================================
6. SIO AND VDEV CHANNELS
=====================================================================

SIO functions work transparently with VDev-backed channels.
When a channel is connected to a VDev (instead of a plain file),
SIO calls route through the VDev callbacks:

  Function        File-backed         VDev-backed
  --------------- ------------------- -------------------
  SIOREAD$        fread(fp, ...)      vd->dev_read(...)
  SIOREADLN$      fgets(fp, ...)      vd->dev_gets(...)
  SIOWRITE        fwrite(fp, ...)     vd->dev_write(...)
  SIOSEEK         fseek(fp, ...)      vd->dev_seek(...)
  SIOFLUSH        fflush(fp)          vd->dev_flush(...)
  SIOSTATUS       ftell + feof        vd->dev_status(...)
  SIOAVAIL        LOF - ftell         vd->dev_avail(...)

This means the same BASIC code works whether the channel is
a disk file, a serial port VDev, a network VDev, or a
custom device module.


=====================================================================
7. SIO VS. TRADITIONAL I/O — WHEN TO USE WHICH
=====================================================================

  Use traditional I/O (PRINT#/INPUT#) when:
    - Reading/writing text files line by line
    - Working with comma-delimited data (WRITE#/INPUT#)
    - Compatibility with GW-BASIC/QBasic programs
    - Human-readable output

  Use SIO when:
    - Binary data (images, protocols, raw bytes)
    - Precise control over what bytes are written
    - Exact byte counts matter
    - VDev device communication
    - No automatic newline/formatting wanted
    - You need return values (bytes read/written)

  MIXING IS FINE.  On the same channel, you can:
    PRINT #1, "Header"        ' Traditional text write
    NW = SIOWRITE(1, data$)   ' Followed by raw bytes
    LINE INPUT #1, A$         ' Traditional text read
    B$ = SIOREAD$(1, 4)       ' Followed by raw byte read

  The channel position is shared.  Both approaches advance
  the same file pointer.


=====================================================================

            PART II: BLOCK I/O (BIO PRIMITIVES)

=====================================================================


=====================================================================
8. WHAT ARE BIO FUNCTIONS?
=====================================================================

BIO (Block I/O) functions provide position-addressed, random-access
operations on files and devices.  Unlike SIO (which reads/writes
sequentially at the current position), BIO specifies an absolute
byte position for every operation.

BIO functions:
  - Take an explicit position argument (0-based byte offset)
  - Save and restore the channel's stream position (non-destructive)
  - Work with both file-backed AND VDev-backed channels
  - Ideal for record stores, binary formats, and device registers

Think of BIO as the "pread/pwrite" layer of BASIC++:
  BIOREAD$    ≈ pread()    (read at position)
  BIOWRITE    ≈ pwrite()   (write at position)
  BIOFILL     ≈ memset()   (fill region)
  BIOCOPY     ≈ memmove()  (copy region)


=====================================================================
9. BIO FUNCTION REFERENCE
=====================================================================

---------------------------------------------------------------------
BIOREAD$(chan, pos, len)
---------------------------------------------------------------------
  Read len bytes from absolute position pos.

  Arguments:
    chan   Channel number (1-8)
    pos   Byte offset (0-based)
    len   Bytes to read (1-1024)

  Returns:
    String containing the bytes read, or "" on error.

  Notes:
    Does NOT alter the channel's stream position (saves/restores).
    Clamped to 1024 bytes maximum per read.
    Works with seekable VDev channels.

  Example:
    10 A$ = BIOREAD$(1, 0, 10)    ' Read first 10 bytes
    20 B$ = BIOREAD$(1, 100, 50)  ' Read 50 bytes at offset 100
    30 PRINT "Header: "; A$

---------------------------------------------------------------------
BIOWRITE(chan, pos, data$)
---------------------------------------------------------------------
  Write data$ at absolute position pos.

  Arguments:
    chan    Channel number (1-8)
    pos    Byte offset (0-based)
    data$  Data to write

  Returns:
    Number of bytes written.

  Notes:
    Does NOT alter the channel's stream position (saves/restores).
    Transaction-aware: journals original data if TXN active.
    File grows automatically if pos is beyond current EOF.

  Example:
    10 NW = BIOWRITE(1, 0, "MAGIC")        ' Write header
    20 NW = BIOWRITE(1, 256, record_data$)  ' Write at offset 256

---------------------------------------------------------------------
BIOSTATUS(chan)
---------------------------------------------------------------------
  Block-level status flags for a channel.

  Returns bitmask:
    Bit 0 (1):   Channel open
    Bit 1 (2):   Seekable
    Bit 2 (4):   Random-access mode
    Bit 3 (8):   Binary mode
    Bit 4 (16):  Readable
    Bit 5 (32):  Writable
    Bit 6 (64):  VDev-backed with binary I/O

  Example:
    10 S = BIOSTATUS(1)
    20 IF (S AND 2) THEN PRINT "Seekable"
    30 IF (S AND 32) THEN PRINT "Writable"
    40 IF (S AND 64) THEN PRINT "VDev block I/O"

---------------------------------------------------------------------
BIOSIZE(chan)
---------------------------------------------------------------------
  Total size of the file or device in bytes.

  Returns:
    File length in bytes, or -1 for VDevs/errors.

  Example:
    10 PRINT "File is"; BIOSIZE(1); "bytes"

---------------------------------------------------------------------
BIOCHECKSUM(data$)
---------------------------------------------------------------------
  Compute CRC-16/CCITT checksum of a string.

  Arguments:
    data$  Data to checksum

  Returns:
    16-bit CRC value (0-65535).

  Notes:
    Polynomial: 0x1021 (CRC-CCITT).
    Initial value: 0xFFFF.
    Does NOT operate on channels — pure computation.
    Useful for verifying data integrity after reads.

  Example:
    10 A$ = BIOREAD$(1, 0, 256)
    20 CK = BIOCHECKSUM(A$)
    30 PRINT "CRC-16:"; CK

---------------------------------------------------------------------
BIOCOMPARE(chan, pos, data$)
---------------------------------------------------------------------
  Compare block on disk with data in memory.

  Arguments:
    chan    Channel number (1-8)
    pos    Byte offset (0-based)
    data$  Data to compare against

  Returns:
    0 if data matches exactly.
    N (1-based offset) of first differing byte.
    -1 on read error.

  Example:
    10 NW = BIOWRITE(1, 0, "HELLO")
    20 D = BIOCOMPARE(1, 0, "HELLO")
    30 IF D = 0 THEN PRINT "Verified OK"

---------------------------------------------------------------------
BIOFILL(chan, pos, len, val)
---------------------------------------------------------------------
  Fill len bytes at position pos with byte value val.

  Arguments:
    chan   Channel number (1-8)
    pos   Byte offset (0-based)
    len   Number of bytes to fill
    val   Byte value (0-255)

  Returns:
    Number of bytes actually filled.

  Notes:
    Transaction-aware: journals original data if TXN active.
    Useful for zeroing sectors or initializing buffers.

  Example:
    10 REM Zero out 512 bytes at start of file
    20 NF = BIOFILL(1, 0, 512, 0)
    30 PRINT "Filled"; NF; "bytes with zeroes"

---------------------------------------------------------------------
BIOCOPY(chan, src, dst, len)
---------------------------------------------------------------------
  Copy len bytes from position src to position dst within
  the same channel.

  Arguments:
    chan   Channel number (1-8)
    src   Source byte offset (0-based)
    dst   Destination byte offset (0-based)
    len   Number of bytes to copy

  Returns:
    Number of bytes copied.

  Notes:
    Handles overlapping regions correctly.
    Transaction-aware: journals destination region if TXN active.

  Example:
    10 REM Shift data: copy 100 bytes from offset 0 to 512
    20 NC = BIOCOPY(1, 0, 512, 100)
    30 PRINT "Copied"; NC; "bytes"


=====================================================================
10. TUTORIAL: RANDOM-ACCESS RECORD STORE
=====================================================================

Problem: Build a simple phone book with fixed-size records using
BIO.  Each record is 64 bytes: 32 for name, 16 for number,
16 for city.

  10 REM --- Phone Book with BIO ---
  20 RECLEN = 64
  30 OPEN "R", 1, "phones.dat"
  40 REM
  50 REM --- Write 3 records ---
  60 GOSUB 200 : REM write records
  70 REM
  80 REM --- Read record #2 ---
  90 POS = (2 - 1) * RECLEN
 100 R$ = BIOREAD$(1, POS, RECLEN)
 110 PRINT "Record 2:"
 120 PRINT "  Name:  "; LEFT$(R$, 32)
 130 PRINT "  Phone: "; MID$(R$, 33, 16)
 140 PRINT "  City:  "; MID$(R$, 49, 16)
 150 CLOSE 1
 160 END
 170 REM
 200 REM --- WRITE RECORDS ---
 210 REM Record 1
 220 N$ = "Alice Smith" + SPACE$(32 - 11)
 230 P$ = "555-0101" + SPACE$(8)
 240 C$ = "New York" + SPACE$(8)
 250 NW = BIOWRITE(1, 0, N$ + P$ + C$)
 260 REM Record 2
 270 N$ = "Bob Jones" + SPACE$(32 - 9)
 280 P$ = "555-0202" + SPACE$(8)
 290 C$ = "Chicago" + SPACE$(9)
 300 NW = BIOWRITE(1, 64, N$ + P$ + C$)
 310 REM Record 3
 320 N$ = "Carol Wu" + SPACE$(32 - 8)
 330 P$ = "555-0303" + SPACE$(8)
 340 C$ = "Seattle" + SPACE$(9)
 350 NW = BIOWRITE(1, 128, N$ + P$ + C$)
 360 RETURN

Why BIO instead of FIELD/GET/PUT?
  - Position-based: you say WHERE to write (byte offset)
  - No FIELD setup needed
  - No MKI$/CVI conversion for packed records
  - Direct string slicing on read results
  - Works identically across all dialects


=====================================================================
11. TUTORIAL: BINARY FILE EDITING WITH BIO
=====================================================================

Problem: Patch a specific byte range in an existing binary file
without disturbing the rest.

  10 REM --- Binary Patcher ---
  20 OPEN "R", 1, "firmware.bin"
  30 PRINT "File size:"; BIOSIZE(1); "bytes"
  40 REM
  50 REM Read current value at offset 0x100
  60 OLD$ = BIOREAD$(1, 256, 4)
  70 PRINT "Old value at 0x100:";
  80 FOR I = 1 TO LEN(OLD$)
  90   PRINT ASC(MID$(OLD$, I, 1));
 100 NEXT I
 110 PRINT
 120 REM
 130 REM Write new 4-byte patch
 140 PATCH$ = CHR$(0) + CHR$(0) + CHR$(0) + CHR$(1)
 150 NW = BIOWRITE(1, 256, PATCH$)
 160 PRINT "Wrote"; NW; "bytes"
 170 REM
 180 REM Verify the patch
 190 D = BIOCOMPARE(1, 256, PATCH$)
 200 IF D = 0 THEN PRINT "Patch verified OK"
 210 IF D > 0 THEN PRINT "MISMATCH at byte"; D
 220 CLOSE 1

Key features used:
  - BIOREAD$ to inspect existing data
  - BIOWRITE to apply the patch
  - BIOCOMPARE to verify the write


=====================================================================
12. TUTORIAL: FILE INTEGRITY CHECKING
=====================================================================

Problem: Compute and verify a CRC checksum on file data.

  10 REM --- CRC Integrity Check ---
  20 OPEN "R", 1, "data.bin"
  30 SZ = BIOSIZE(1)
  40 PRINT "Checking"; SZ; "bytes..."
  50 REM
  60 REM Read file in 1K blocks and compute rolling CRC
  70 CRC = 0
  80 POS = 0
  90 WHILE POS < SZ
 100   BLK$ = BIOREAD$(1, POS, 1024)
 110   IF LEN(BLK$) = 0 THEN GOTO 140
 120   CRC = BIOCHECKSUM(BLK$)
 130   POS = POS + LEN(BLK$)
 140 WEND
 150 PRINT "CRC-16 of last block:"; CRC
 160 CLOSE 1

Note: BIOCHECKSUM computes the CRC of a single string.  For
multi-block files, each call resets the CRC.  For true file-wide
checksumming, accumulate data into a buffer or use a custom
hash loop.


=====================================================================
13. TUTORIAL: BLOCK COPY AND FILL OPERATIONS
=====================================================================

Problem: Initialize a file with a header, then copy a section.

  10 REM --- Block Operations ---
  20 OPEN "R", 1, "blocks.dat"
  30 REM Zero out 1024 bytes
  40 NF = BIOFILL(1, 0, 1024, 0)
  50 PRINT "Filled"; NF; "bytes with zeroes"
  60 REM Write a magic header
  70 NW = BIOWRITE(1, 0, "BPP1")
  80 PRINT "Header written"
  90 REM Copy header to backup location at offset 512
 100 NC = BIOCOPY(1, 0, 512, 4)
 110 PRINT "Copied"; NC; "bytes"
 120 REM Verify backup matches
 130 D = BIOCOMPARE(1, 512, "BPP1")
 140 IF D = 0 THEN PRINT "Backup verified OK"
 150 CLOSE 1


=====================================================================
14. BIO VS. FIELD/GET/PUT — WHEN TO USE WHICH
=====================================================================

  Use FIELD/GET/PUT when:
    - Porting GW-BASIC or QBasic programs as-is
    - Working with legacy .DAT files in fixed-record format
    - MKI$/CVI, MKS$/CVS, MKD$/CVD conversions needed
    - Record numbers (1-based) are more natural than byte offsets

  Use BIO when:
    - Binary file formats (headers, checksums, patches)
    - Variable-length records (different offsets per record)
    - Need verification (BIOCOMPARE) after writes
    - Need block operations (BIOFILL, BIOCOPY)
    - Working with device registers or USB endpoints
    - Position-addressed I/O (0-based byte offsets)
    - Cross-dialect portability (BIO works the same everywhere)

  MIXING IS FINE.  On the same channel:
    FIELD #1, 20 AS N$, 10 AS P$
    PUT #1, 1                       ' Traditional record write
    NW = BIOWRITE(1, 256, custom$)  ' Direct byte write
    A$ = BIOREAD$(1, 0, 30)         ' Direct byte read

  Both approaches share the same underlying file handle.


=====================================================================

         PART III: DEVICE ALIASES

=====================================================================


=====================================================================
15. WHAT ARE DEVICE ALIASES?
=====================================================================

Device aliases map legacy device names to modern VDev targets.
When you OPEN a channel using a classic device name, the alias
system translates it to the corresponding VDev:

  Atari BASIC:     OPEN #1, 4, 0, "E:"
  What happens:    "E:" → alias lookup → "CON:" → VDev slot 0
  Result:          Channel #1 is connected to the console

  GW-BASIC:        OPEN "SCRN:" FOR OUTPUT AS #1
  What happens:    "SCRN:" → alias lookup → "CON:" → VDev slot 0
  Result:          Channel #1 is connected to the console

The same OPEN syntax works on Windows, Linux, and FreeDOS —
the alias system provides the translation layer.

Aliases are:
  - Opt-in per dialect (loaded automatically when you select a dialect)
  - Case-insensitive ("e:" = "E:" = "e:")
  - Direction-aware (some devices are input-only or output-only)
  - User-extensible (DEVMAP ALIAS lets you create custom aliases)


=====================================================================
16. THE ALIAS RESOLUTION CHAIN
=====================================================================

When BASIC++ encounters OPEN with a device name, it follows
this resolution chain:

  1. Is the name a registered VDev? (CON:, ERR:, FILE:, NET:)
     → YES: use the VDev directly
     → NO: continue

  2. Is the name a device alias?
     → YES: replace with target name, go to step 1
     → NO: continue

  3. Does the name look like a path or filename?
     → YES: treat as a file (FILE: VDev)
     → NO: raise "device not found" error

This means:
  - Direct VDev names (CON:, SER:) always work
  - Alias names (E:, SCRN:, KYBD:) resolve to VDev names
  - Regular filenames pass through to fopen()


=====================================================================
17. BUILT-IN DIALECT ALIASES
=====================================================================

When you start BASIC++ with a specific dialect (e.g., --dialect=atari),
the corresponding alias table is loaded automatically.

To see which aliases are active:

  DEVMAP ALIAS

  This prints:
    Alias      Target     Dir      Dialect
    -----      ------     ---      -------
    E:         CON:       BOTH     ATARI
    S:         CON:       OUT      ATARI
    K:         CON:       IN       ATARI
    P:         ERR:       OUT      ATARI
    D:         FILE:      BOTH     ATARI
    D1:        FILE:      BOTH     ATARI
    D2:        FILE:      BOTH     ATARI
    C:         FILE:      BOTH     ATARI
    R:         SER:       BOTH     ATARI
    R1:        SER:       BOTH     ATARI

Aliases are cleared and reloaded when you switch dialects.
Manual aliases (created with DEVMAP ALIAS) persist across
dialect switches.


=====================================================================
18. TUTORIAL: USING ATARI DEVICE NAMES ON MODERN SYSTEMS
=====================================================================

On a real Atari 800, you'd write:

  10 OPEN #1, 4, 0, "E:"        ' Screen editor (I/O)
  20 OPEN #2, 4, 0, "P:"        ' Printer
  30 OPEN #3, 6, 0, "R:"        ' RS-232

On BASIC++ (any platform), with --dialect=atari:

  10 OPEN #1, 4, 0, "E:"        ' → CON: (console)
  20 OPEN #2, 4, 0, "P:"        ' → ERR: (stderr fallback)
  30 OPEN #3, 6, 0, "R:"        ' → SER: (if registered)

The OPEN syntax is IDENTICAL.  The aliases provide the mapping.

Full working example:

  10 REM --- Atari-Style I/O on Modern System ---
  20 REM Using ATARI dialect
  30 REM
  40 REM Write to "screen" (CON:)
  50 OPEN #1, 4, 0, "S:"
  60 NW = SIOWRITE(1, "Hello from S: device!")
  70 NW = SIOWRITE(1, CHR$(10))
  80 CLOSE #1
  90 REM
 100 REM Read from "keyboard" (CON:)
 110 OPEN #2, 4, 0, "K:"
 120 PRINT "Type something: ";
 130 A$ = SIOREADLN$(2)
 140 PRINT "You typed: "; A$
 150 CLOSE #2
 160 REM
 170 REM Write to "disk" (FILE:)
 180 OPEN #3, 8, 0, "D:MYDATA.TXT"
 190 NW = SIOWRITE(3, "Atari data file")
 200 CLOSE #3

What each device resolves to:
  S:             → CON: (stdout only — alias direction is OUT)
  K:             → CON: (stdin only — alias direction is IN)
  D:MYDATA.TXT   → FILE:  (opens MYDATA.TXT as a regular file)


=====================================================================
19. TUTORIAL: USING GW-BASIC DEVICE NAMES ON LINUX
=====================================================================

GW-BASIC programs often use device names like SCRN: and KYBD:
that don't exist on Linux.  With BASIC++ aliases, they work:

  10 REM --- GW-BASIC Device Names on Linux ---
  20 REM Using GW-BASIC dialect (--dialect=gwbasic)
  30 REM
  40 OPEN "SCRN:" FOR OUTPUT AS #1
  50 PRINT #1, "This goes to the screen via SCRN: alias"
  60 CLOSE #1
  70 REM
  80 OPEN "KYBD:" FOR INPUT AS #2
  90 LINE INPUT #2, A$
 100 PRINT "Got: "; A$
 110 CLOSE #2

What happens under the hood:
  "SCRN:" → alias lookup → "CON:" → stdout  (Linux: printf)
  "KYBD:" → alias lookup → "CON:" → stdin   (Linux: fgets)

The original GW-BASIC program runs UNCHANGED.


=====================================================================
20. TUTORIAL: USING C64 DEVICE NUMBERS
=====================================================================

Commodore 64 uses numeric device IDs.  BASIC++ maps these
through aliases with "DEVn:" naming:

  Commodore syntax:  OPEN 1, 8, 2, "DATA,S,R"
  BASIC++ alias:     DEV8: → FILE:

  10 REM --- C64-Style Device Access ---
  20 REM Using C64 dialect (--dialect=c64)
  30 REM
  40 REM Device 3 = screen (DEV3: → CON:)
  50 OPEN "DEV3:" FOR OUTPUT AS #1
  60 PRINT #1, "Hello from C64 device 3!"
  70 CLOSE #1
  80 REM
  90 REM Device 4 = printer (DEV4: → ERR:)
 100 OPEN "DEV4:" FOR OUTPUT AS #2
 110 PRINT #2, "Printer output via ERR:"
 120 CLOSE #2

Note: The C64's native OPEN syntax (OPEN lf, dev, sa, "name")
is NOT directly supported.  Use the alias names or the Atari-form
OPEN syntax instead.


=====================================================================
21. TUTORIAL: CUSTOM ALIASES WITH DEVMAP
=====================================================================

You can create your own device aliases at runtime:

  DEVMAP ALIAS "GPS:" "SER:" BOTH
  DEVMAP ALIAS "PAPER:" "ERR:" OUT
  DEVMAP ALIAS "TAPE:" "FILE:" BOTH

Now you can use these in OPEN:

  OPEN "GPS:" FOR RANDOM AS #1       ' Routes to SER: VDev
  OPEN "PAPER:" FOR OUTPUT AS #2     ' Routes to ERR: (stderr)
  OPEN "TAPE:" FOR OUTPUT AS #3      ' Routes to FILE: VDev

Managing aliases:

  DEVMAP ALIAS                    List all active aliases
  DEVMAP ALIAS "GPS:" OFF         Disable an alias
  DEVMAP ALIAS "GPS:" ON          Re-enable an alias
  DEVMAP ALIAS CLEAR              Remove all manual aliases

Custom aliases are useful for:
  - USB device abstraction (e.g., "USB0:" → a registered USB VDev)
  - Application-specific device names
  - Testing (redirect a device to a file for debugging)
  - Future device types not yet invented


=====================================================================
22. ALIAS DIRECTION AND SAFETY
=====================================================================

Every alias has a direction field:

  Direction    Meaning
  ---------    -------
  IN           Input only (read)
  OUT          Output only (write)
  BOTH         Bidirectional (read + write)

Direction checking prevents mistakes:

  10 REM K: is input-only on Atari
  20 OPEN #1, 4, 0, "K:"          ' Opens K: for output
  30 NW = SIOWRITE(1, "Hello")    ' May fail or be ignored

  If you OPEN an input-only alias for output, BASIC++ issues
  a warning (but allows it for compatibility — some legacy
  programs relied on this behavior).

Direction is a HINT, not a hard gate.  The underlying VDev
may enforce its own restrictions.


=====================================================================
23. CROSS-DIALECT DEVICE MAPPING TABLE
=====================================================================

This table shows how every classic device name maps to a
modern VDev in BASIC++.

  Dialect    Device       Description          Modern Target
  ------     ------       -----------          -------------
  Atari      E:           Screen Editor        CON: (both)
  Atari      S:           Screen Display       CON: (out)
  Atari      K:           Keyboard             CON: (in)
  Atari      P:           Printer              ERR: (out)
  Atari      D: / D1:     Disk Drive 1         FILE: (both)
  Atari      D2:          Disk Drive 2         FILE: (both)
  Atari      C:           Cassette             FILE: (both)
  Atari      R: / R1:     RS-232               SER: (both)
  ------     ------       -----------          -------------
  C64        DEV0:        Keyboard/Screen      CON: (in)
  C64        DEV1:        Cassette             FILE: (both)
  C64        DEV2:        User Port Serial     SER: (both)
  C64        DEV3:        Screen               CON: (out)
  C64        DEV4:        Printer              ERR: (out)
  C64        DEV5:        Printer 2            ERR: (out)
  C64        DEV8:-DEV11: Disk Drives          FILE: (both)
  ------     ------       -----------          -------------
  CoCo       CAS:         Cassette (#-1)       FILE: (both)
  CoCo       LPT:         Printer (#-2)        ERR: (out)
  CoCo       SCR:         Screen (#0)          CON: (both)
  ------     ------       -----------          -------------
  Sinclair   LOWER:       Lower Screen (#0)    CON: (out)
  Sinclair   UPPER:       Upper Screen (#1)    CON: (both)
  Sinclair   ZXPRN:       Printer (#2)         ERR: (out)
  Sinclair   ZXNET:       Network (#3)         NET: (both)
  ------     ------       -----------          -------------
  GW-BASIC   SCRN:        Screen               CON: (out)
  GW-BASIC   KYBD:        Keyboard             CON: (in)
  GW-BASIC   LPT1:/LPT2: Printer              ERR: (out)
  GW-BASIC   COM1:/COM2:  Serial Port          SER: (both)
  GW-BASIC   CAS1:        Cassette             FILE: (both)
  ------     ------       -----------          -------------
  QBasic     CONS:        Console              CON: (both)
  QBasic     SCRN:        Screen               CON: (out)
  QBasic     LPT1:        Printer              ERR: (out)
  QBasic     COM1:/COM2:  Serial Port          SER: (both)
  ------     ------       -----------          -------------
  CP/M       LST:         List Device          ERR: (out)
  CP/M       PUN:         Punch                FILE: (out)
  CP/M       RDR:         Reader               FILE: (in)
  CP/M       CON:         Console              CON: (both)
  ------     ------       -----------          -------------
  ECMA-116   TERM:        Terminal (#0)        CON: (both)
  ------     ------       -----------          -------------
  SuperBASIC CON_         Console Window       CON: (both)
  SuperBASIC SCR_         Screen Window        CON: (out)
  SuperBASIC SER1/SER2    Serial Ports         SER: (both)
  SuperBASIC MDV1_/MDV2_  Microdrive           FILE: (both)
  SuperBASIC FLP1_        Floppy               FILE: (both)
  SuperBASIC NET          Network              NET: (both)
  ------     ------       -----------          -------------
  TRS-80     *PR          Printer              ERR: (out)
  TRS-80     *CL          Cassette Load        FILE: (in)
  TRS-80     *CS          Cassette Save        FILE: (out)
  ------     ------       -----------          -------------
  S-BASIC    TTY:         Terminal             CON: (both)
  S-BASIC    PTR:         Paper Tape Reader    FILE: (in)
  S-BASIC    PTP:         Paper Tape Punch     FILE: (out)
  S-BASIC    LPT:         Line Printer         ERR: (out)
  ------     ------       -----------          -------------
  Apple      SLOT0:       Screen/Keyboard      CON: (both)
  Apple      SLOT1:       Printer Slot         ERR: (out)
  Apple      SLOT2:       Serial Slot          SER: (both)
  Apple      SLOT6:       Disk Controller      FILE: (both)

Linux equivalents (not aliases — use OPEN directly):

  Linux Path     Purpose          BASIC++ Equivalent
  ----------     -------          ------------------
  /dev/ttyS0     Serial port      OPEN "SER:" (via VDev)
  /dev/stdin     Standard input   OPEN "CON:" (in)
  /dev/stdout    Standard output  OPEN "CON:" (out)
  /dev/null      Null device      OPEN "NUL:" (via VDev)


=====================================================================

         PART IV: TRANSACTIONS (ATOMIC / TXN)

=====================================================================


=====================================================================
24. WHAT ARE TRANSACTIONS?
=====================================================================

A transaction groups multiple I/O write operations so they
either ALL succeed or ALL get undone.  This prevents partial
writes that leave files in a corrupted state.

BASIC++ provides two transaction modes:

  ATOMIC Block:
    ATOMIC
      ... writes ...
    END ATOMIC

    If any error occurs between ATOMIC and END ATOMIC, all
    writes are automatically rolled back.  If no error occurs,
    END ATOMIC commits them permanently.

  Explicit TXN:
    TXN BEGIN
      ... writes ...
    TXN COMMIT   (or COMMIT)

    You have manual control.  You decide when to COMMIT or
    ROLLBACK based on your own logic.

What gets journaled:
  - BIOWRITE writes
  - BIOFILL fills
  - BIOCOPY destination regions
  - SIOWRITE writes (file-backed channels only)

What does NOT get journaled:
  - PRINT# (traditional text output)
  - VDev-backed channel writes
  - Console output
  - PUT (FIELD/GET/PUT random-access)


=====================================================================
25. ATOMIC BLOCKS — AUTO-ROLLBACK ON ERROR
=====================================================================

ATOMIC is the simpler, safer choice.  You wrap a block of
writes and BASIC++ handles errors automatically.

Syntax:
  ATOMIC
    ... I/O operations ...
  END ATOMIC

On success: END ATOMIC commits — all writes are permanent.
On error: the error handler auto-rolls-back before the
error message even prints.

Example:

  10 OPEN "R", 1, "config.dat"
  20 REM Prepare new config data
  30 HEADER$ = "CFG2" + CHR$(0) + CHR$(0) + CHR$(0) + CHR$(1)
  40 DATA$ = "key=value" + SPACE$(55)
  50 REM
  60 REM Atomic update — all or nothing
  70 ATOMIC
  80   NW = BIOWRITE(1, 0, HEADER$)
  90   NW = BIOWRITE(1, 64, DATA$)
 100   NW = BIOFILL(1, 128, 128, 0)
 110 END ATOMIC
 120 PRINT "Config updated successfully"
 130 CLOSE 1

If line 90 or 100 fail (disk full, I/O error), all three
writes are undone — the file reverts to its pre-ATOMIC state.

Rules:
  - ATOMIC blocks cannot be nested
  - Cannot start TXN inside an ATOMIC block (or vice versa)
  - TXNSTATUS() returns 2 inside an ATOMIC block
  - You can use COMMIT as an alternative to END ATOMIC


=====================================================================
26. TXN EXPLICIT TRANSACTIONS
=====================================================================

For more control, use TXN BEGIN / TXN COMMIT / TXN ROLLBACK:

Syntax:
  TXN BEGIN                    Start the transaction
  TXN COMMIT  (or COMMIT)     Commit — writes are permanent
  TXN ROLLBACK (or ROLLBACK)  Rollback — undo all writes

Example with error checking:

  10 ON ERROR GOTO 500
  20 OPEN "R", 1, "database.dat"
  30 TXN BEGIN
  40 REM Update header
  50 NW = BIOWRITE(1, 0, "DB02")
  60 REM Update index
  70 NW = BIOWRITE(1, 16, index_data$)
  80 REM Update record
  90 NW = BIOWRITE(1, 256, record$)
 100 REM All good — commit
 110 TXN COMMIT
 120 PRINT "Database updated."
 130 CLOSE 1
 140 END
 500 REM Error handler
 510 PRINT "Error during update!"
 520 IF TXNSTATUS() > 0 THEN TXN ROLLBACK
 530 PRINT "Changes rolled back."
 540 CLOSE 1

Key differences from ATOMIC:
  - TXN does NOT auto-rollback on errors
  - You must catch errors yourself (ON ERROR GOTO)
  - You decide whether to ROLLBACK or COMMIT
  - TXNSTATUS() returns 1 inside a TXN

Additional command:
  TXN STATUS         Print diagnostic information:
                     "TXN: ACTIVE (3 entries)"
                     Shows state and journal entry count


=====================================================================
27. TXNSTATUS() AND TXN STATUS
=====================================================================

TXNSTATUS() is a function — use it in expressions:

  IF TXNSTATUS() = 0 THEN PRINT "No transaction"
  IF TXNSTATUS() = 1 THEN PRINT "TXN active"
  IF TXNSTATUS() = 2 THEN PRINT "ATOMIC block"

TXN STATUS is a command — use it as a statement:

  TXN STATUS
  ' Output: "TXN: ACTIVE (5 entries)"

Return values:
  0 = TXN_NONE    No active transaction
  1 = TXN_ACTIVE  Explicit TXN in progress
  2 = TXN_ATOMIC  ATOMIC block in progress


=====================================================================
28. TUTORIAL: SAFE DATABASE UPDATE
=====================================================================

Problem: Update multiple records in a database file.  If any
record fails to write, undo ALL changes.

  10 REM --- Safe Multi-Record Update ---
  20 OPEN "R", 1, "records.dat"
  30 RECLEN = 128
  40 NRECS = 10
  50 REM
  60 TXN BEGIN
  70 FOR I = 1 TO NRECS
  80   POS = (I - 1) * RECLEN
  90   REM Build new record
 100   R$ = "REC" + STR$(I) + SPACE$(RECLEN - 6)
 110   NW = BIOWRITE(1, POS, R$)
 120   IF NW <> RECLEN THEN GOTO 200
 130 NEXT I
 140 TXN COMMIT
 150 PRINT "All"; NRECS; "records updated."
 160 CLOSE 1
 170 END
 200 REM --- Write failed ---
 210 PRINT "Write failed at record"; I
 220 ROLLBACK
 230 PRINT "All changes undone."
 240 CLOSE 1

If the write at any record fails (returns fewer bytes than
expected), we ROLLBACK and the file is exactly as it was
before TXN BEGIN.


=====================================================================
29. TUTORIAL: ALL-OR-NOTHING CONFIG FILE WRITE
=====================================================================

Problem: Replace a config file header + body atomically.

  10 REM --- Atomic Config Writer ---
  20 OPEN "R", 1, "app.cfg"
  30 REM
  40 ATOMIC
  50   REM Write 16-byte header
  60   H$ = "APPCFG01" + MKI$(42) + SPACE$(6)
  70   NW = BIOWRITE(1, 0, H$)
  80   REM Write 256-byte config body
  90   B$ = "theme=dark" + CHR$(10)
 100   B$ = B$ + "lang=en" + CHR$(10)
 110   B$ = B$ + SPACE$(256 - LEN(B$))
 120   NW = BIOWRITE(1, 16, B$)
 130 END ATOMIC
 140 REM If we get here, both writes succeeded
 150 PRINT "Config saved."
 160 CLOSE 1

If the disk runs out of space during the body write (line 120),
ATOMIC automatically restores the original header at offset 0 —
the config file is never left in a half-written state.


=====================================================================
30. JOURNAL LIMITS AND BEST PRACTICES
=====================================================================

The transaction journal has fixed limits:

  MAX_TXN_ENTRIES = 64     Maximum write operations per transaction
  MAX_TXN_DATA   = 1024   Maximum bytes saved per operation

If you exceed these limits, BASIC++ raises a SORRY error.

Best practices:

  1. Keep transactions SHORT.  Write what you need, then commit.

  2. Do NOT put loops with hundreds of writes inside ATOMIC.
     The journal fills up at 64 entries.

  3. For large batch updates, commit in chunks:
       FOR BATCH = 1 TO 10
         ATOMIC
           FOR I = 1 TO 6
             NW = BIOWRITE(1, ...)
           NEXT I
         END ATOMIC
       NEXT BATCH

  4. Do NOT mix PRINT# (traditional) with transactions.
     PRINT# output is NOT journaled and cannot be rolled back.

  5. VDev-backed channels are logged as "no-restore" entries.
     Rollback cannot undo VDev writes (there's no way to seek
     back on a serial port or network socket).

  6. Transactions do NOT nest.  You get one level — use it wisely.

  7. TXNSTATUS() is cheap — call it anytime to check state.

  8. Always clean up: if a TXN is active and you're about to END
     or CLOSE ALL, make sure to COMMIT or ROLLBACK first.


=====================================================================

            PART V: CROSS-REFERENCE

=====================================================================


=====================================================================
31. SIO/BIO/TXN QUICK REFERENCE CARD
=====================================================================

  --- Stream I/O (SIO) ---
  SIOREAD$(ch, n)       Read n bytes from channel
  SIOREADLN$(ch)        Read one line from channel
  SIOWRITE(ch, d$)      Write string to channel → bytes written
  SIOSEEK(ch, pos)      Seek to byte position → new position
  SIOFLUSH(ch)          Flush output buffers → 0 or -1
  SIOSTATUS(ch)         Channel status bitmask
  SIOAVAIL(ch)          Bytes available for read

  --- Block I/O (BIO) ---
  BIOREAD$(ch, pos, n)        Read n bytes at position
  BIOWRITE(ch, pos, d$)       Write string at position → bytes written
  BIOSTATUS(ch)               Block status bitmask
  BIOSIZE(ch)                 File size in bytes
  BIOCHECKSUM(d$)             CRC-16/CCITT of string
  BIOCOMPARE(ch, pos, d$)     Compare block: 0=match, N=diff offset
  BIOFILL(ch, pos, n, val)    Fill n bytes with val → bytes filled
  BIOCOPY(ch, src, dst, n)    Copy n bytes src→dst → bytes copied

  --- Transaction Control ---
  ATOMIC                Begin atomic block (auto-rollback on error)
  END ATOMIC            Commit atomic block
  TXN BEGIN             Begin explicit transaction
  TXN COMMIT / COMMIT   Commit transaction
  TXN ROLLBACK / ROLLBACK  Rollback transaction
  TXN STATUS            Print transaction state and entry count
  TXNSTATUS()           Function: 0=none, 1=TXN, 2=ATOMIC

  --- Device Alias Management ---
  DEVMAP ALIAS                      List all aliases
  DEVMAP ALIAS "name" "target" dir  Create alias (dir=IN/OUT/BOTH)
  DEVMAP ALIAS "name" OFF           Disable alias
  DEVMAP ALIAS "name" ON            Enable alias
  DEVMAP ALIAS CLEAR                Clear all manual aliases


=====================================================================
32. FULL DEVICE ALIAS TABLE
=====================================================================

  Dialect      Alias     Target   Dir    Description
  ---------    ------    ------   ----   -----------
  Atari        E:        CON:     BOTH   Screen editor
  Atari        S:        CON:     OUT    Screen display
  Atari        K:        CON:     IN     Keyboard
  Atari        P:        ERR:     OUT    Printer
  Atari        D:        FILE:    BOTH   Disk drive 1
  Atari        D1:       FILE:    BOTH   Disk drive 1
  Atari        D2:       FILE:    BOTH   Disk drive 2
  Atari        C:        FILE:    BOTH   Cassette
  Atari        R:        SER:     BOTH   RS-232
  Atari        R1:       SER:     BOTH   RS-232 port 1
  C64          DEV0:     CON:     IN     Keyboard
  C64          DEV1:     FILE:    BOTH   Cassette
  C64          DEV2:     SER:     BOTH   User port serial
  C64          DEV3:     CON:     OUT    Screen
  C64          DEV4:     ERR:     OUT    Printer
  C64          DEV5:     ERR:     OUT    Printer 2
  C64          DEV8:     FILE:    BOTH   Disk (device 8)
  C64          DEV9:     FILE:    BOTH   Disk (device 9)
  C64          DEV10:    FILE:    BOTH   Disk (device 10)
  C64          DEV11:    FILE:    BOTH   Disk (device 11)
  CoCo         CAS:      FILE:    BOTH   Cassette
  CoCo         LPT:      ERR:     OUT    Printer
  CoCo         SCR:      CON:     BOTH   Screen
  Sinclair     LOWER:    CON:     OUT    Lower screen
  Sinclair     UPPER:    CON:     BOTH   Upper screen
  Sinclair     ZXPRN:    ERR:     OUT    ZX Printer
  Sinclair     ZXNET:    NET:     BOTH   Network
  GW-BASIC     SCRN:     CON:     OUT    Screen
  GW-BASIC     KYBD:     CON:     IN     Keyboard
  GW-BASIC     LPT1:     ERR:     OUT    Printer 1
  GW-BASIC     LPT2:     ERR:     OUT    Printer 2
  GW-BASIC     COM1:     SER:     BOTH   Serial port 1
  GW-BASIC     COM2:     SER:     BOTH   Serial port 2
  GW-BASIC     CAS1:     FILE:    BOTH   Cassette
  QBasic       CONS:     CON:     BOTH   Console
  QBasic       SCRN:     CON:     OUT    Screen
  QBasic       LPT1:     ERR:     OUT    Printer
  QBasic       COM1:     SER:     BOTH   Serial port 1
  QBasic       COM2:     SER:     BOTH   Serial port 2
  CP/M         LST:      ERR:     OUT    List device
  CP/M         PUN:      FILE:    OUT    Paper tape punch
  CP/M         RDR:      FILE:    IN     Paper tape reader
  ECMA-116     TERM:     CON:     BOTH   Terminal
  SuperBASIC   CON_      CON:     BOTH   Console window
  SuperBASIC   SCR_      CON:     OUT    Screen window
  SuperBASIC   SER1      SER:     BOTH   Serial port 1
  SuperBASIC   SER2      SER:     BOTH   Serial port 2
  SuperBASIC   MDV1_     FILE:    BOTH   Microdrive 1
  SuperBASIC   MDV2_     FILE:    BOTH   Microdrive 2
  SuperBASIC   FLP1_     FILE:    BOTH   Floppy drive
  SuperBASIC   NET       NET:     BOTH   Network
  TRS-80       *PR       ERR:     OUT    Printer
  TRS-80       *CL       FILE:    IN     Cassette load
  TRS-80       *CS       FILE:    OUT    Cassette save
  S-BASIC      TTY:      CON:     BOTH   Terminal
  S-BASIC      PTR:      FILE:    IN     Paper tape reader
  S-BASIC      PTP:      FILE:    OUT    Paper tape punch
  S-BASIC      LPT:      ERR:     OUT    Line printer
  Apple        SLOT0:    CON:     BOTH   Screen/keyboard
  Apple        SLOT1:    ERR:     OUT    Printer slot
  Apple        SLOT2:    SER:     BOTH   Serial slot
  Apple        SLOT6:    FILE:    BOTH   Disk controller

  Linux equivalents (direct VDev / file access, no alias needed):
    /dev/ttyS0    →  Register SER: VDev, then OPEN "SER:"
    /dev/stdin    →  OPEN "CON:" (input direction)
    /dev/stdout   →  OPEN "CON:" (output direction)
    /dev/null     →  OPEN "NUL:" (if VDev registered)


=====================================================================
33. RELATED DOCUMENTATION
=====================================================================

  File_IO.md             Core file I/O reference, legacy device
                         tutorials, VDev architecture, security

  Virtual_Devices.md     VDev system internals, writing custom
                         device drivers, registration API

  USB_Devices.md         USB device support via mod_usb VDev

  External_Modules.md    Creating loadable modules (VDevs, etc.)

  Secure_Coding.md       Security gates for file/device access

  Creating_Dialects.md   How dialect profiles configure aliases

  Virtual_Network.md     NET: VDev for network I/O


=====================================================================
END OF DOCUMENT
=====================================================================
