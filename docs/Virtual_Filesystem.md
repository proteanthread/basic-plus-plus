THE BASIC++ VIRTUAL FILESYSTEM
================================
Version 4.0.1

This manual explains how BASIC++ programs access files on the
local file system — how it works internally, why every layer
exists, what protections are in place, and how you as a user
can control every aspect of file access.

Unlike a quick reference (see I_File_IO.txt), this manual is
designed to give you deep understanding.  By the time you
finish reading it, you will know:

  - How a BASIC "OPEN" statement reaches the disk
  - What every layer does and why it's there
  - How security sandboxing protects you
  - How errors are caught, reported, and recovered from
  - How to handle every file format (text, records, raw bytes)
  - How line endings, character sets, and encoding work
  - How to write safe, robust BASIC programs that use files
  - How paths, directories, and permissions work on every OS
  - How to debug file problems when something goes wrong


TABLE OF CONTENTS
=================

  Part I:   How It Works
  1.  The Journey of a Byte (Architecture)
  2.  Layer 1: The BASIC Statement
  3.  Layer 2: The VM Dispatcher
  4.  Layer 3: The Security Gate
  5.  Layer 4: The File Channel Table
  6.  Layer 5: The C stdio Layer
  7.  Layer 6: The Operating System
  8.  Why These Layers Exist

  Part II:  Using the Virtual Filesystem
  9.  Opening Files
      9.1  The OPEN Statement
      9.2  Five Access Modes
      9.3  Channel Numbers (#1 - #8)
      9.4  The Record Length Parameter
  10. Reading Files
      10.1 INPUT # (Comma-Delimited Reading)
      10.2 LINE INPUT # (Raw Line Reading)
      10.3 GET # (Record/Binary Reading)
      10.4 EOF() (End-of-File Detection)
  11. Writing Files
      11.1 PRINT # (Text Output)
      11.2 PRINT # USING (Formatted Output)
      11.3 WRITE # (Machine-Readable Output)
      11.4 PUT # (Record/Binary Writing)
  12. Closing Files
  13. File Management Commands
      13.1 FILES (Directory Listing)
      13.2 KILL (Delete a File)
      13.3 NAME ... AS (Rename a File)
      13.4 CHDIR (Change Directory)
      13.5 MKDIR / RMDIR (Create/Remove Directory)
  14. File Information Functions
  15. Random-Access Files (In Depth)
      15.1 How Records Work
      15.2 The Record Buffer
      15.3 FIELD Mapping
      15.4 LSET and RSET
      15.5 Numeric Storage (MKI$/CVI, MKS$/CVS, MKD$/CVD)
      15.6 Complete Database Example
  16. Binary Files (In Depth)
      16.1 How Binary Mode Works
      16.2 Seek and Position
      16.3 Reading and Writing Raw Bytes
      16.4 Parsing Binary File Formats
  17. File Locking (Multi-Process Safety)
  18. IOCTL / IOCTL$ (Channel Control)

  Part III: Safety and Security
  19. The Security System
      19.1 Three Security Levels
      19.2 The Permission Matrix
      19.3 How Security Checks Work Internally
      19.4 Setting the Security Level
  20. Path Safety
      20.1 Relative vs. Absolute Paths
      20.2 Path Traversal Protection
      20.3 Reserved Filenames (Windows)
      20.4 Case Sensitivity (Linux vs Windows)
  21. Error Handling
      21.1 What Errors Can Occur
      21.2 How Errors Propagate
      21.3 ON ERROR GOTO (Error Recovery)
      21.4 Defensive Programming Patterns
  22. Best Practices for Safe File Access

  Part IV:  Cross-Platform Behavior
  23. Line Endings
  24. Path Separators
  25. Character Encoding
  26. File System Limits
  27. Platform-Specific Behavior Table

  Part V:   Advanced Topics
  28. SAVE and LOAD (Program File I/O)
  29. MERGE and CHAIN
  30. BSAVE and BLOAD (Memory-to-File)
  31. COMPILE (Source-to-Executable)
  32. Extending the Filesystem via VDev
  33. Troubleshooting and Debugging

  34. Quick Reference Tables
  35. Related Manuals

  Part VI:  Device Files (Phase 16)
  36. Device Files vs Regular Files
  37. Device Name Detection in OPEN
  38. The /dev/ Namespace (Linux Integration)
  39. Security Scoping for Device-Backed Paths
  40. Updated Related Manuals


=====================================================================

              PART I: HOW IT WORKS

=====================================================================


=====================================================================
1. THE JOURNEY OF A BYTE (ARCHITECTURE)
=====================================================================

When your BASIC program writes a character to a file, that
character passes through SIX layers before reaching the
disk.  Understanding these layers is the key to understanding
everything in this manual.

Let's trace what happens when this line executes:

  PRINT #1, "Hello"

Layer 1: BASIC Statement (parser.c)
  The VM fetches the line and the lexer tokenizes it.
  The first keyword is PRINT, so the VM dispatches to
  the OP_PRINT handler.  The handler sees the '#' and
  knows this is file output, not console output.
  It evaluates "Hello" into a string value.

Layer 2: VM Dispatcher (vm.c)
  vm_dispatch(KW_PRINT, lex, rt, line_num) routes to
  the parse_print handler.  This is an O(1) lookup in
  the dispatch table — no switch statement.

Layer 3: Security Gate (security.c)
  Before any file write occurs, security_check() is
  called with SECOP_FILE_WRITE.  The permission matrix
  is consulted:
    SEC_OPEN:       allowed (default)
    SEC_STANDARD:   allowed
    SEC_RESTRICTED: DENIED -> ERR_SORRY
  If denied, the byte never reaches the file system.

Layer 4: File Channel Table (fileio.c)
  fileio_print(1, "Hello", line_num) is called.
  The channel table is indexed: idx = chan - 1 = 0.
  Validation:
    - Is idx in range [0, MAX_FILE_CHANNELS)?  YES
    - Is channels[idx].fp non-NULL?             YES (file is open)
    - Is mode OUTPUT or APPEND?                 YES
  The call proceeds to C stdio.

Layer 5: C stdio Layer
  fprintf(channels[0].fp, "%s", "Hello") writes
  the string through the C standard library's buffered
  I/O system.  The C library:
    - Copies bytes into an internal buffer (usually 4K)
    - Applies text-mode transformations if applicable
      (on Windows: \n -> \r\n in text mode)
    - When the buffer fills (or fflush/fclose), calls
      the OS write system call

Layer 6: Operating System
  The OS receives a write() system call:
    Windows: WriteFile(hFile, buf, len, &written, NULL)
    Linux:   write(fd, buf, len)
    FreeDOS: INT 21h, AH=40h (DOS write)
  The OS writes bytes through the filesystem driver
  (NTFS, ext4, FAT32) to the physical storage medium.

RETURN PATH (reading follows the same layers in reverse):
  Disk -> OS -> C stdio -> File Channel -> Security -> VM -> BASIC


=====================================================================
2. LAYER 1: THE BASIC STATEMENT
=====================================================================

The first layer is what you write in BASIC.  The interpreter
accepts these file-related statements:

  Statement                           Direction  Mode
  ---------                           ---------  ----
  OPEN "file" FOR mode AS #n          --         All
  CLOSE #n                            --         All
  PRINT #n, expr                      Write      SEQ
  PRINT #n, USING fmt$; expr          Write      SEQ
  WRITE #n, expr [, expr ...]         Write      SEQ
  INPUT #n, var [, var ...]           Read       SEQ
  LINE INPUT #n, var$                 Read       SEQ
  GET #n [, rec]                      Read       RND/BIN
  PUT #n [, rec]                      Write      RND/BIN
  FIELD #n, w AS v$ [, w AS v$ ...]   Setup      RND
  LSET v$ = expr$                     Write      RND
  RSET v$ = expr$                     Write      RND
  LOCK #n [, start TO end]            Lock       RND/BIN
  UNLOCK #n [, start TO end]          Unlock     RND/BIN
  SEEK #n, position                   Seek       BIN

  Functions:
  EOF(n)                              Check end-of-file
  LOF(n)                              File length in bytes
  LOC(n)                              Current position
  IOCTL$(n)                           Channel mode string

  File management:
  FILES ["pattern"]                   List directory
  KILL "filename"                     Delete file
  NAME "old" AS "new"                 Rename file
  CHDIR "path"                        Change directory
  MKDIR "dirname"                     Create directory
  RMDIR "dirname"                     Remove directory

  Program file operations:
  SAVE "filename"                     Save program to text file
  LOAD "filename"                     Load program from text file
  MERGE "filename"                    Merge into current program
  CHAIN "filename"                    Load and run
  BSAVE "file", addr, len             Save memory range
  BLOAD "file" [, addr]               Load into memory
  COMPILE "output"                    Transpile to C


=====================================================================
3. LAYER 2: THE VM DISPATCHER
=====================================================================

When the VM encounters a statement keyword, it resolves
the keyword to an opcode and calls the registered handler:

  OPEN   -> OP_OPEN   -> parse_open_cmd()
  CLOSE  -> OP_CLOSE  -> parse_close_cmd()
  PRINT  -> OP_PRINT  -> parse_print()     (detects #n internally)
  INPUT  -> OP_INPUT  -> parse_input()     (detects #n internally)
  SAVE   -> OP_SAVE   -> parse_save_cmd()
  LOAD   -> OP_LOAD   -> parse_load_cmd()

This dispatch is O(1) — a single array lookup by keyword ID.
No performance penalty compared to a hardcoded switch.

The handler receives:
  - Lexer (positioned after the keyword token)
  - Runtime state (variables, program store, options)
  - Line number (for error messages)

The handler is responsible for:
  1. Parsing the arguments (filename, channel, mode)
  2. Calling the security gate
  3. Calling the fileio.c functions
  4. Handling errors


=====================================================================
4. LAYER 3: THE SECURITY GATE
=====================================================================

Before ANY file operation reaches the filesystem, the
security gate checks whether it's allowed.

  HOW IT WORKS:

  The security system maintains a single global setting:
  the current security level (SecLevel).  A static
  permission matrix maps (level x operation) -> allowed:

    Operation        OPEN  STANDARD  RESTRICTED
    --------------- ----  --------  ----------
    SECOP_FILE_READ   1       1          0
    SECOP_FILE_WRITE  1       1          0
    SECOP_COMPILE     1       0          0
    SECOP_CHAIN       1       0          0
    SECOP_SYSTEM      1       0          0
    SECOP_MODULE      1       1          0

  The check is a single array access: allowed[level][op].
  Zero overhead when the level is SEC_OPEN (the default),
  because it fast-paths: if (level == SEC_OPEN) return 0.

  WHY IT EXISTS:

  Imagine you download a BASIC program from the internet
  and run it.  Without the security gate, that program
  could:
    - OPEN any file on your system for writing
    - KILL your important files
    - SAVE malicious code over your other programs
    - CHAIN to execute system commands

  With SECURITY "RESTRICTED", none of those operations
  are permitted.  The program can only do math, string
  manipulation, and screen output — it cannot touch
  the file system at all.

  With SECURITY "STANDARD", the program can read and
  write files (necessary for data-processing programs)
  but cannot COMPILE, CHAIN, or access system operations.

  WHEN THE CHECK FIRES:

  The security check happens at the TOP of every file
  operation handler.  Example from the OPEN handler:

    /* Check security before ANY file operation */
    if (security_check(SECOP_FILE_WRITE, line_num) != 0)
        return;  /* denied - error already reported */

  If denied, security_check() prints:
    SORRY? Security: file write not permitted at level
    RESTRICTED in line 10

  And the operation is aborted before fopen() is called.

  IMPORTANT: The security gate cannot be bypassed from
  BASIC.  There is no BASIC command to change the security
  level — it can only be set at boot time via the command
  line or from C code.  This prevents a malicious program
  from lowering its own restrictions.


=====================================================================
5. LAYER 4: THE FILE CHANNEL TABLE
=====================================================================

The file channel table is the core data structure for all
BASIC file I/O.  It is a static array of 16 FileChannel
structures (channels 1-16, stored 0-indexed):

  static FileChannel channels[16];

  Each FileChannel contains:

    struct FileChannel {
        FILE *fp;           /* C file pointer, or NULL if closed */
        int   mode;         /* CLOSED/INPUT/.../BINARY/DEVICE */
        VDev *vdev;         /* Phase 16: device pointer (or NULL) */
        int   record_len;   /* record length for RANDOM (default 128) */
        char  record_buf[256]; /* record buffer */
        FieldMap fields[16]; /* FIELD mappings */
        int   field_count;  /* number of active FIELD mappings */
        long  current_rec;  /* current record number (1-based) */
    };

  LIFECYCLE OF A CHANNEL:

    1. At boot, fileio_channels_init() sets all 8 channels
       to FCHAN_CLOSED with fp = NULL.

    2. OPEN "file" FOR INPUT AS #3 calls fileio_open(3, ...).
       - Validates channel number (1-8)
       - Checks channel is not already open
       - Calls fopen() with the appropriate mode
       - Stores the FILE* and mode
       - Returns 0 (success) or -1 (error)

    3. INPUT #3, A$ calls fileio_input_line(3, buf, 255, ln).
       - Validates channel (in range, open, correct mode)
       - Calls fgets() on the stored FILE*
       - Strips trailing newline
       - Returns the data in buf

    4. CLOSE #3 calls fileio_close(3, ln).
       - Calls fclose() on the FILE*
       - Sets fp = NULL and mode = FCHAN_CLOSED
       - Resets record buffer and field mappings

    5. At program end (NEW, RUN, or exit),
       fileio_channels_cleanup() closes ALL open channels.
       This prevents file handle leaks even if the program
       crashes or the user types NEW without closing files.

  WHY STATIC ALLOCATION:

    The channel table is statically allocated (no malloc).
    This guarantees:
    - Predictable memory usage (no heap fragmentation)
    - Works on FreeDOS with limited memory
    - No allocation failures at runtime
    - Cleanup is trivial (iterate 8 slots)

  WHY ONLY 8 CHANNELS:

    GW-BASIC and QBasic both supported 1-15 (or 1-255 in
    some versions), but 8 is sufficient for virtually all
    BASIC programs ever written.  Classic BASIC programs
    rarely use more than 2-3 files simultaneously.  The
    limit is configurable in config.h (MAX_FILE_CHANNELS).


=====================================================================
6. LAYER 5: THE C STDIO LAYER
=====================================================================

BASIC++ uses ONLY standard C17 stdio functions for file access.
No POSIX, no Win32, no platform-specific APIs (except for
LOCK/UNLOCK on Windows).

  Functions used:

    fopen(path, mode)    Open a file
    fclose(fp)           Close a file
    fgets(buf, n, fp)    Read a line
    fgetc(fp)            Read one byte
    fprintf(fp, ...)     Write formatted text
    fputs(s, fp)         Write a string
    fputc(c, fp)         Write one byte
    fread(buf, 1, n, fp) Read raw bytes
    fwrite(buf, 1, n, fp) Write raw bytes
    fseek(fp, pos, whence) Move file position
    ftell(fp)            Get file position
    fflush(fp)           Force write to disk
    feof(fp)             Check end-of-file
    ungetc(c, fp)        Push back one byte

  File open modes used:

    "r"    Sequential INPUT (text mode, read-only)
    "w"    Sequential OUTPUT (text mode, truncate+write)
    "a"    Sequential APPEND (text mode, write at end)
    "r+b"  Random/Binary read+write (existing file)
    "w+b"  Random/Binary read+write (new file)

  WHY STDIO AND NOT RAW OS CALLS:

    1. PORTABILITY: stdio works identically on Windows,
       Linux, macOS, FreeDOS, and any system with a C
       compiler.  Raw OS calls are different everywhere.

    2. BUFFERING: stdio provides transparent buffering.
       Small reads/writes are coalesced into larger OS
       calls, which is much more efficient for BASIC
       programs that do character-at-a-time I/O.

    3. TEXT MODE: stdio handles line-ending translation
       automatically.  On Windows, "\n" in a text-mode
       file is written as "\r\n" on disk.  On Linux,
       "\n" stays as "\n".  The BASIC program never
       needs to worry about this.

    4. ANSI C89: The entire interpreter compiles with
       any C17-compliant compiler.  No extensions needed.

  TEXT MODE VS BINARY MODE:

    Sequential files use TEXT mode ("r", "w", "a"):
      - \n in memory  ->  \r\n on disk (Windows)
      - \n in memory  ->  \n on disk (Linux)
      - Ctrl-Z (0x1A) may signal EOF on DOS/Windows

    Random and Binary files use BINARY mode ("r+b", "w+b"):
      - No line ending translation
      - No Ctrl-Z EOF handling
      - Byte positions are exact (no \r\n expansion)
      - Essential for fseek()/ftell() accuracy


=====================================================================
7. LAYER 6: THE OPERATING SYSTEM
=====================================================================

The OS is the final layer.  BASIC++ never talks to it
directly — all communication goes through C stdio.  But
understanding how the OS behaves helps explain platform
differences.

  WINDOWS:
    - File handles are managed by the kernel
    - NTFS supports long filenames (up to 260 chars)
    - File names are case-INSENSITIVE ("DATA.TXT" = "data.txt")
    - Reserved names: CON, PRN, AUX, NUL, COM1-COM9, LPT1-LPT9
    - Path separator: backslash \ (forward slash / also works)
    - Line endings in text mode: \r\n
    - File locking: _locking() from <sys/locking.h>

  LINUX:
    - File descriptors managed by the kernel
    - ext4/btrfs support filenames up to 255 bytes
    - File names are case-SENSITIVE ("DATA.TXT" != "data.txt")
    - No reserved names (but avoid starting with -)
    - Path separator: forward slash /
    - Line endings in text mode: \n
    - File locking: not implemented in current build (no-op)

  FREEDOS:
    - DOS file handles (limited to ~15 per process)
    - FAT12/FAT16/FAT32 filesystem
    - File names: 8.3 format (unless LFNAPI loaded)
    - Case-insensitive
    - Path separator: backslash \
    - Line endings: \r\n
    - Ctrl-Z (0x1A) means end-of-file in text mode
    - File locking: not supported (no-op)


=====================================================================
8. WHY THESE LAYERS EXIST
=====================================================================

You might ask: why not just call fopen() directly from the
PRINT handler?  Why all these layers?

Each layer solves a specific problem:

  Layer         Problem It Solves
  -----         ------------------
  BASIC stmt    User-friendly syntax (OPEN...FOR...AS)
  VM dispatch   Uniform execution model, traceable with TRON
  Security      Protection against malicious programs
  Channel table State management (which files are open, modes)
  C stdio       Cross-platform file access
  OS kernel     Hardware access to storage media

Removing ANY layer creates real problems:

  Without security:
    A downloaded BASIC program can KILL your files.

  Without the channel table:
    No way to track which files are open, what mode
    they're in, or clean them up at program end.

  Without C stdio:
    You'd need separate code for Windows, Linux, and
    FreeDOS.  Three implementations to maintain.

  Without VM dispatch:
    TRON can't trace file operations, the debugger
    can't set breakpoints on OPEN, and the alias
    system can't remap file commands.


=====================================================================

              PART II: USING THE VIRTUAL FILESYSTEM

=====================================================================


=====================================================================
9. OPENING FILES
=====================================================================


---------------------------------------------------------------------
9.1  The OPEN Statement
---------------------------------------------------------------------

  Full syntax:

    OPEN filename$ FOR mode AS #channel [LEN = reclen]

  Examples:

    OPEN "data.txt" FOR INPUT AS #1
    OPEN "output.csv" FOR OUTPUT AS #2
    OPEN "log.txt" FOR APPEND AS #3
    OPEN "database.dat" FOR RANDOM AS #4 LEN = 100
    OPEN "image.bin" FOR BINARY AS #5

  The filename can be:
    - A string literal:  OPEN "myfile.txt" FOR ...
    - A variable:        OPEN F$ FOR ...
    - An expression:     OPEN "data" + STR$(N) + ".txt" FOR ...


---------------------------------------------------------------------
9.2  Five Access Modes
---------------------------------------------------------------------

  INPUT (Sequential Read)
    - Opens an existing file for reading
    - Reading starts at the beginning
    - Cannot write to the file
    - File must exist or ERR_HOW is raised
    - C mode: "r" (text)

  OUTPUT (Sequential Write)
    - Creates a new file or TRUNCATES an existing one
    - WARNING: This ERASES the file contents!
    - Writing starts at the beginning
    - Cannot read from the file
    - C mode: "w" (text)

  APPEND (Sequential Write at End)
    - Opens an existing file or creates a new one
    - Writing starts at the END of existing content
    - Existing content is PRESERVED
    - Cannot read from the file
    - C mode: "a" (text)

  RANDOM (Record-Based Read+Write)
    - Opens for both reading AND writing
    - File is divided into fixed-length records
    - Any record can be read or written by number
    - If file doesn't exist, it is created
    - Default record length: 128 bytes
    - C mode: "r+b" (binary, or "w+b" if new)

  BINARY (Byte-Level Read+Write)
    - Opens for both reading AND writing
    - No record structure — raw bytes
    - Any byte position can be accessed
    - If file doesn't exist, it is created
    - C mode: "r+b" (binary, or "w+b" if new)


---------------------------------------------------------------------
9.3  Channel Numbers (#1 - #8)
---------------------------------------------------------------------

  Channel numbers are arbitrary labels from 1 to 8.
  They have no inherent meaning — you choose them:

    #1 is NOT "the first file opened"
    #1 is simply "the file I chose to call #1"

  You can open channels in any order:

    OPEN "a.txt" FOR INPUT AS #5    ' this is fine
    OPEN "b.txt" FOR OUTPUT AS #2   ' this is fine

  Channels are independent.  Opening #1 has no effect
  on #2.  Each channel has its own:
    - FILE pointer
    - Mode (INPUT/OUTPUT/APPEND/RANDOM/BINARY)
    - Record buffer (for RANDOM mode)
    - Field mappings
    - Current position

  A channel must be CLOSEd before it can be re-OPENed:

    OPEN "a.txt" FOR INPUT AS #1
    ' ... use #1 ...
    CLOSE #1
    OPEN "b.txt" FOR INPUT AS #1    ' Now OK - #1 was closed

  Opening an already-open channel raises ERR_HOW:

    OPEN "a.txt" FOR INPUT AS #1
    OPEN "b.txt" FOR INPUT AS #1    ' ERROR! #1 is still open


---------------------------------------------------------------------
9.4  The Record Length Parameter
---------------------------------------------------------------------

  The LEN = n parameter is used only with RANDOM mode:

    OPEN "db.dat" FOR RANDOM AS #1 LEN = 50

  Rules:
    - Default: 128 bytes if LEN is omitted
    - Minimum: 1 byte
    - Maximum: 256 bytes (MAX_RECORD_LEN in config.h)
    - All records in the file are the same length
    - The LEN must match what was used to create the file

  WHY FIXED-LENGTH RECORDS:

    Fixed-length records allow O(1) seeking to any record:
      file_position = (record_number - 1) * record_length

    This is why GET #1, 1000 is instant — the runtime
    simply calculates 999 * record_length and calls
    fseek().  No scanning through previous records.


=====================================================================
10. READING FILES
=====================================================================


---------------------------------------------------------------------
10.1  INPUT # (Comma-Delimited Reading)
---------------------------------------------------------------------

  INPUT #n, var [, var ...]

  Reads values separated by commas from a sequential file.
  Each variable reads one value.

  Example file (data.csv):
    John,42,3.14
    Jane,25,2.72

  Reading:
    OPEN "data.csv" FOR INPUT AS #1
    INPUT #1, N$, AGE, SCORE
    PRINT N$; " is "; AGE; " years old"
    ' Output: John is 42 years old
    CLOSE #1

  HOW IT PARSES:
    - Strings are delimited by commas or newlines
    - Quoted strings: "value with, commas" are read intact
    - Numbers are parsed by VAL() rules
    - Extra values on a line are held for the next INPUT #

  ERROR CONDITIONS:
    - Reading from a closed channel: ERR_HOW
    - Reading from an OUTPUT/APPEND channel: ERR_HOW
    - Reading past EOF: ERR_HOW (check EOF() first!)


---------------------------------------------------------------------
10.2  LINE INPUT # (Raw Line Reading)
---------------------------------------------------------------------

  LINE INPUT #n, var$

  Reads an ENTIRE line, including commas, into a single
  string variable.  No parsing, no splitting.

  This is the most common file reading pattern:

    OPEN "readme.txt" FOR INPUT AS #1
    WHILE NOT EOF(1)
      LINE INPUT #1, L$
      PRINT L$
    WEND
    CLOSE #1

  WHY LINE INPUT # EXISTS:

    INPUT # splits on commas.  If your data contains
    commas (addresses, CSV files, sentences), INPUT #
    would split it incorrectly:

      "Smith, John",42      <-- INPUT # reads "Smith" as one
                                value and " John" as the next

    LINE INPUT # reads the whole line as-is:

      "Smith, John",42      <-- LINE INPUT # reads the full
                                string including the comma

  LINE ENDING HANDLING:

    LINE INPUT # strips the trailing newline (\n or \r\n).
    The returned string never contains trailing newlines.


---------------------------------------------------------------------
10.3  GET # (Record/Binary Reading)
---------------------------------------------------------------------

  For RANDOM mode:
    GET #n, record_number
    Reads the specified record into the channel's record
    buffer.  FIELD-mapped variables are updated.

  For BINARY mode:
    GET #n, byte_position
    Reads bytes starting at the given position.

  See sections 15 (Random-Access) and 16 (Binary) for
  detailed explanations.


---------------------------------------------------------------------
10.4  EOF() (End-of-File Detection)
---------------------------------------------------------------------

  EOF(n) returns:
    -1 (true)  if the file is at the end
     0 (false) if more data is available

  IMPORTANT: Always check EOF() BEFORE reading:

    ' CORRECT:
    WHILE NOT EOF(1)
      LINE INPUT #1, A$
      PRINT A$
    WEND

    ' WRONG (may crash on last read):
    DO
      LINE INPUT #1, A$     ' <-- fails at EOF!
      PRINT A$
    LOOP UNTIL EOF(1)

  HOW EOF() WORKS INTERNALLY:

    The implementation uses a peek-ahead technique:

    1. Read one byte with fgetc()
    2. If fgetc() returns EOF, we are at end-of-file
    3. If fgetc() returns a character, push it back
       with ungetc() so the next read sees it
    4. Return -1 (at EOF) or 0 (not at EOF)

    This makes EOF() PREDICTIVE — it tells you BEFORE
    you try to read whether the next read will fail.
    This matches GW-BASIC behavior exactly.

  EOF() WITH RANDOM FILES:

    For random-access files, EOF() checks whether the
    file pointer is beyond the last byte.  This works
    because random-access files use binary mode.


=====================================================================
11. WRITING FILES
=====================================================================


---------------------------------------------------------------------
11.1  PRINT # (Text Output)
---------------------------------------------------------------------

  PRINT #n, expression [; expression ...]

  Works exactly like PRINT but writes to a file instead
  of the screen.

  Examples:
    PRINT #1, "Hello World"
    PRINT #1, A; ", "; B; ", "; C
    PRINT #1, TAB(10); "Indented"

  Semicolons suppress spaces; commas insert tab stops.

  NEWLINES:
    PRINT #1, expr    produces a newline at the end
    PRINT #1, expr;   suppresses the newline

  INTERNAL OPERATION:

    The handler calls:
      fileio_print(chan, text, line_num)
        -> fprintf(channels[idx].fp, "%s", text)
      fileio_print_newline(chan, line_num)
        -> fprintf(channels[idx].fp, "\n")
        -> fflush(channels[idx].fp)

    Note: fflush() is called after every newline to ensure
    data is written to disk promptly.  Without this, data
    could be lost if the program crashes or the user hits
    Ctrl+C before the buffer is full.


---------------------------------------------------------------------
11.2  PRINT # USING (Formatted Output)
---------------------------------------------------------------------

  PRINT #n, USING format$; expression [; expression ...]

  Writes formatted output to a file.  Format characters:

    #     Digit position
    .     Decimal point
    ,     Thousands separator
    +     Print sign (+ or -)
    -     Trailing minus for negatives
    **    Fill leading spaces with asterisks
    $$    Dollar sign prefix
    ^^^^  Scientific notation
    !     First character of string
    &     Entire string
    \ \   Fixed-width string field (width = chars between \'s + 2)

  Example:
    OPEN "report.txt" FOR OUTPUT AS #1
    FOR I = 1 TO 5
      PRINT #1, USING "Item ##: $###.## ****"; I; I * 9.99
    NEXT
    CLOSE #1


---------------------------------------------------------------------
11.3  WRITE # (Machine-Readable Output)
---------------------------------------------------------------------

  WRITE #n, expr [, expr ...]

  Writes values in a standardized format:
    - Strings are enclosed in double quotes
    - Values are separated by commas
    - A newline is appended
    - Numbers have no leading/trailing spaces

  Example:
    WRITE #1, "John", 42, 3.14
    ' File contains: "John",42,3.14

  WRITE # output is designed to be read back by INPUT #:

    ' Write
    WRITE #1, N$, AGE, SCORE

    ' Read back (matching order and types)
    INPUT #1, N$, AGE, SCORE

  This write-read symmetry makes WRITE/INPUT the
  simplest way to save and load structured data.


---------------------------------------------------------------------
11.4  PUT # (Record/Binary Writing)
---------------------------------------------------------------------

  For RANDOM mode:
    PUT #n, record_number
    Writes the channel's record buffer to the specified
    record position in the file.

  For BINARY mode:
    PUT #n, byte_position
    Writes bytes at the given position.

  See sections 15 and 16 for detailed explanations.


=====================================================================
12. CLOSING FILES
=====================================================================

  CLOSE #n                 Close a specific channel
  CLOSE #1, #2, #3         Close multiple channels
  CLOSE                    Close ALL open channels
  RESET                    Same as CLOSE (all channels)

  WHY CLOSING MATTERS:

  1. DATA INTEGRITY: C stdio buffers data.  Until you
     close the file (or call fflush), some bytes may be
     sitting in an internal buffer, not yet on disk.
     Closing the file forces the buffer to disk.

  2. HANDLE LEAKS: Each open file uses an OS file handle.
     The OS has a limited number of handles (typically
     256-1024).  If you open files without closing them,
     you'll eventually run out.

  3. FILE LOCKING: Some OSes lock files while they're
     open.  On Windows, you can't delete or rename a
     file that another process (or your own program)
     has open.

  AUTOMATIC CLEANUP:

  BASIC++ automatically closes all open channels when:
    - You type NEW (clear program)
    - You type RUN (start a new execution)
    - The program reaches END
    - An unhandled error returns to the REPL
    - The interpreter exits

  This safety net prevents file handle leaks, but you
  should STILL close files explicitly.  Relying on auto-
  cleanup is sloppy and may cause data loss if the buffer
  hasn't been flushed.


=====================================================================
13. FILE MANAGEMENT COMMANDS
=====================================================================


---------------------------------------------------------------------
13.1  FILES (Directory Listing)
---------------------------------------------------------------------

  FILES                    List all files in current directory
  FILES "*.bas"            List matching files
  FILES "subdir\"          List contents of a subdirectory

  This uses the system's directory listing capability.


---------------------------------------------------------------------
13.2  KILL (Delete a File)
---------------------------------------------------------------------

  KILL "filename"          Delete a file
  KILL "temp*.dat"         Delete matching files (if supported)

  WARNING: KILL permanently deletes the file.  There is no
  undo, no recycle bin.  The file is gone.

  SECURITY: KILL is controlled by SECOP_FILE_WRITE.
  At SEC_RESTRICTED, KILL is blocked.

  SAFETY TIP: Always verify before deleting:

    10 INPUT "Delete which file"; F$
    20 INPUT "Are you sure (Y/N)"; A$
    30 IF UCASE$(A$) = "Y" THEN KILL F$


---------------------------------------------------------------------
13.3  NAME ... AS (Rename a File)
---------------------------------------------------------------------

  NAME "oldname.txt" AS "newname.txt"

  Renames a file without changing its contents.  The file
  must not be open.  Both names must be on the same drive.


---------------------------------------------------------------------
13.4  CHDIR (Change Directory)
---------------------------------------------------------------------

  CHDIR "path"             Change current directory
  CHDIR ".."               Go up one level
  CHDIR "C:\data"          Absolute path (Windows)
  CHDIR "/home/user/data"  Absolute path (Linux)

  After CHDIR, all relative filenames in OPEN, SAVE, LOAD,
  etc. resolve from the new directory.


---------------------------------------------------------------------
13.5  MKDIR / RMDIR (Create/Remove Directory)
---------------------------------------------------------------------

  MKDIR "newdir"           Create a new directory
  RMDIR "emptydir"         Remove an empty directory

  RMDIR only works on empty directories.


=====================================================================
14. FILE INFORMATION FUNCTIONS
=====================================================================

  Function    Returns                 Works With
  --------    -------                 ----------
  EOF(n)      -1 if at end, 0 if not  INPUT/RANDOM/BINARY
  LOF(n)      File length in bytes    All modes
  LOC(n)      Current position        RANDOM: record number
                                      BINARY: byte position
  SEEK #n,p   (statement) move to p   BINARY
  IOCTL$(n)   Mode letter             All modes

  IOCTL$ returns:
    "I" = Input (sequential read)
    "O" = Output (sequential write)
    "A" = Append
    "R" = Random
    "B" = Binary

  Example:
    OPEN "test.dat" FOR RANDOM AS #1
    PRINT IOCTL$(1)              ' prints "R"
    PRINT LOF(1)                 ' prints file size
    CLOSE #1


=====================================================================
15. RANDOM-ACCESS FILES (IN DEPTH)
=====================================================================


---------------------------------------------------------------------
15.1  How Records Work
---------------------------------------------------------------------

  A random-access file is a sequence of fixed-length records.
  Think of it as a simple database table:

    Record 1:  [-------- 50 bytes --------]
    Record 2:  [-------- 50 bytes --------]
    Record 3:  [-------- 50 bytes --------]
    ...

  Every record is EXACTLY the same length.  This allows
  instant access to any record by number:

    File position = (record_number - 1) * record_length

  GET #1, 500 takes the same time as GET #1, 1 because
  both are just an fseek() + fread().


---------------------------------------------------------------------
15.2  The Record Buffer
---------------------------------------------------------------------

  Each channel has a 256-byte record buffer.  This buffer
  is the workspace for building and reading records:

    channels[idx].record_buf[256]

  Operations:
    GET #n, rec    -> fseek + fread into record_buf
    PUT #n, rec    -> fseek + fwrite from record_buf
    LSET v$ = x$   -> copy into record_buf at field offset
    RSET v$ = x$   -> copy into record_buf at field offset

  The buffer is initialized with spaces (ASCII 32).
  When you GET a record that doesn't exist yet (beyond
  the current end of file), the buffer stays filled with
  spaces — no error is raised.


---------------------------------------------------------------------
15.3  FIELD Mapping
---------------------------------------------------------------------

  FIELD maps string variables to slices of the record buffer:

    FIELD #1, 20 AS N$, 15 AS CITY$, 10 AS PHONE$, 5 AS ZIP$
              |         |            |             |
              v         v            v             v
    Buffer: [0......19][20......34][35......44][45..49]
             N$ (20)    CITY$ (15)  PHONE$ (10) ZIP$ (5)
                              Total: 50 bytes

  Rules:
    - Total field widths must be <= record length (LEN)
    - Up to 16 fields per channel (MAX_FIELD_MAPS)
    - Fields are zero-offset from the start of the record
    - Variable names are case-insensitive
    - Only string variables can be FIELDed

  After GET, the FIELD variables contain the record data.
  Before PUT, you load data into FIELD variables with LSET/RSET.


---------------------------------------------------------------------
15.4  LSET and RSET
---------------------------------------------------------------------

  LSET v$ = expr$     Left-justify: pad RIGHT with spaces
  RSET v$ = expr$     Right-justify: pad LEFT with spaces

  The field is ALWAYS the full width, padded with spaces:

    FIELD #1, 10 AS N$
    LSET N$ = "Bob"
    ' N$ is now: "Bob       " (10 chars, 7 trailing spaces)

    RSET N$ = "42"
    ' N$ is now: "        42" (10 chars, 8 leading spaces)

  If the source string is LONGER than the field width,
  it is TRUNCATED (not an error):

    LSET N$ = "Christopher"
    ' N$ is now: "Christophe" (10 chars, truncated at width)

  LSET and RSET write directly into the record buffer at
  the field's offset.  They do NOT write to the file —
  you must call PUT afterwards.


---------------------------------------------------------------------
15.5  Numeric Storage (MKI$/CVI, MKS$/CVS, MKD$/CVD)
---------------------------------------------------------------------

  FIELD variables are strings, but you need to store numbers.
  The MK/CV function pairs convert between numbers and their
  binary string representations:

    Function  Bytes  Stores            Range
    --------  -----  ------            -----
    MKI$/CVI    2    Integer           -32768 to 32767
    MKS$/CVS    4    Single-precision  ~7 significant digits
    MKD$/CVD    8    Double-precision  ~15 significant digits

  Encoding:
    LSET AGE$ = MKI$(42)    ' Stores 42 as 2 bytes in record

  Decoding:
    GET #1, rec
    PRINT CVI(AGE$)         ' Converts 2 bytes back to 42

  WHY BINARY, NOT TEXT:

    Storing "42" as text takes 2 bytes but only handles
    2-digit numbers.  MKI$ always takes exactly 2 bytes
    regardless of the value (-32768 to 32767).  This
    guarantees fixed record lengths and faster I/O.


---------------------------------------------------------------------
15.6  Complete Database Example
---------------------------------------------------------------------

  ' PHONEBOOK.BAS - A simple phone book database
  ' Demonstrates random-access file I/O

  10 REM === Set up the file ===
  20 OPEN "phones.dat" FOR RANDOM AS #1 LEN = 60
  30 FIELD #1, 25 AS NAME$, 15 AS PHONE$, 20 AS CITY$

  100 REM === Main Menu ===
  110 CLS
  120 PRINT "=== PHONE BOOK ==="
  130 PRINT "1. Add Entry"
  140 PRINT "2. Look Up by Record #"
  150 PRINT "3. List All"
  160 PRINT "4. Quit"
  170 INPUT "Choice"; C
  180 IF C = 1 THEN GOSUB 1000
  190 IF C = 2 THEN GOSUB 2000
  200 IF C = 3 THEN GOSUB 3000
  210 IF C = 4 THEN GOTO 9000
  220 GOTO 100

  1000 REM === Add Entry ===
  1010 INPUT "Record #"; REC
  1020 INPUT "Name"; N$
  1030 INPUT "Phone"; P$
  1040 INPUT "City"; CI$
  1050 LSET NAME$ = N$
  1060 LSET PHONE$ = P$
  1070 LSET CITY$ = CI$
  1080 PUT #1, REC
  1090 PRINT "Saved to record "; REC
  1100 RETURN

  2000 REM === Look Up ===
  2010 INPUT "Record #"; REC
  2020 GET #1, REC
  2030 PRINT "Name:  "; NAME$
  2040 PRINT "Phone: "; PHONE$
  2050 PRINT "City:  "; CITY$
  2060 RETURN

  3000 REM === List All ===
  3010 LASTREC = LOF(1) / 60
  3020 FOR I = 1 TO LASTREC
  3030   GET #1, I
  3040   IF LEFT$(NAME$, 1) <> " " THEN
  3050     PRINT I; ": "; NAME$; " | "; PHONE$; " | "; CITY$
  3060   END IF
  3070 NEXT
  3080 RETURN

  9000 CLOSE #1
  9010 END


=====================================================================
16. BINARY FILES (IN DEPTH)
=====================================================================


---------------------------------------------------------------------
16.1  How Binary Mode Works
---------------------------------------------------------------------

  Binary mode provides raw byte-level access.  There are no
  records, no line endings, no field mappings — just bytes
  at positions.

  OPEN "data.bin" FOR BINARY AS #1

  The file is opened with "r+b" (read + write, binary mode).
  If the file doesn't exist, "w+b" creates it.


---------------------------------------------------------------------
16.2  Seek and Position
---------------------------------------------------------------------

  Positions are 1-based (position 1 = first byte):

    SEEK #1, 1              ' Go to first byte
    SEEK #1, 100            ' Go to byte 100
    SEEK #1, LOF(1)         ' Go to last byte

  LOC(n) returns the current byte position.
  LOF(n) returns the total file size in bytes.


---------------------------------------------------------------------
16.3  Reading and Writing Raw Bytes
---------------------------------------------------------------------

  Writing:
    PUT #1, position, data$
    ' Writes the bytes of data$ at the given position

    PUT #1, 1, CHR$(0) + CHR$(255) + CHR$(128)
    ' Writes 3 bytes: 0x00, 0xFF, 0x80

  Reading:
    GET #1, position, var$
    ' Reads bytes at position into var$

  For character-at-a-time access:
    PUT #1, pos, CHR$(byte_value)
    GET #1, pos, B$
    byte_value = ASC(B$)


---------------------------------------------------------------------
16.4  Parsing Binary File Formats
---------------------------------------------------------------------

  Example: reading a BMP image header:

    OPEN "image.bmp" FOR BINARY AS #1

    ' Read the 14-byte BMP file header
    GET #1, 1, H$                 ' Read header

    ' Parse fields using MID$ and ASC
    MAGIC$ = MID$(H$, 1, 2)       ' Should be "BM"
    FILESIZE = ASC(MID$(H$,3,1)) + ASC(MID$(H$,4,1))*256 + _
               ASC(MID$(H$,5,1))*65536 + ASC(MID$(H$,6,1))*16777216

    PRINT "Magic: "; MAGIC$
    PRINT "File size: "; FILESIZE; " bytes"

    CLOSE #1


=====================================================================
17. FILE LOCKING (MULTI-PROCESS SAFETY)
=====================================================================

  When multiple programs access the same file simultaneously,
  LOCK and UNLOCK prevent data corruption:

    LOCK #n [, start TO end]       Lock a range
    UNLOCK #n [, start TO end]     Release a lock

  For RANDOM files: start and end are record numbers.
  For BINARY files: start and end are byte positions.

  LOCK without a range locks the entire file:
    LOCK #1                        Lock the whole file

  HOW IT WORKS INTERNALLY:

  On Windows:
    Uses _locking() from <sys/locking.h>
    - Calculates byte offset: (start - 1) * record_len
    - Calculates byte length: (end - start + 1) * record_len
    - Calls _locking(fd, _LK_LOCK, length)
    - If another process has the range locked, returns error

  On Linux and FreeDOS:
    Locking is accepted but has no effect (no-op).
    The statements compile and run without error.

  BEST PRACTICE:

    OPEN "shared.dat" FOR RANDOM AS #1 LEN = 64
    LOCK #1, 5 TO 5               ' Lock just record 5
    GET #1, 5                     ' Read record 5
    LSET NAME$ = "Updated"
    PUT #1, 5                     ' Write record 5
    UNLOCK #1, 5 TO 5             ' Release record 5
    CLOSE #1

  Lock the MINIMUM range for the SHORTEST time possible.


=====================================================================
18. IOCTL / IOCTL$ (CHANNEL CONTROL)
=====================================================================

  IOCTL #n, command$     Send control string to channel
  IOCTL$(n)              Get channel mode/status

  IOCTL$ returns a single character indicating the mode:

    Mode      IOCTL$ Returns
    ----      ---------------
    INPUT     "I"
    OUTPUT    "O"
    APPEND    "A"
    RANDOM    "R"
    BINARY    "B"
    CLOSED    "" (empty string)

  This lets a subroutine check what mode a file is open in
  before attempting operations:

    IF IOCTL$(1) = "I" THEN
      LINE INPUT #1, A$        ' Safe to read
    ELSE
      PRINT "Error: #1 not open for input"
    END IF


=====================================================================

              PART III: SAFETY AND SECURITY

=====================================================================


=====================================================================
19. THE SECURITY SYSTEM
=====================================================================


---------------------------------------------------------------------
19.1  Three Security Levels
---------------------------------------------------------------------

  SEC_OPEN (Default)
    No restrictions.  All operations are permitted.
    Use this when running your own trusted programs.
    This is the default to maintain backward compatibility
    with every classic BASIC program ever written.

  SEC_STANDARD
    File I/O and module activation are allowed.
    COMPILE, CHAIN, and SYSTEM commands are blocked.
    Use this when running programs you mostly trust but
    want to prevent them from executing external commands
    or compiling code.

  SEC_RESTRICTED
    ALL I/O is blocked.  No file access whatsoever.
    The program can only use math, string operations,
    and screen output (PRINT to console).
    Use this when running untrusted programs — they
    literally cannot touch the file system.


---------------------------------------------------------------------
19.2  The Permission Matrix
---------------------------------------------------------------------

  This is the complete permission table.  Every file
  operation maps to one of these SecOperations:

  Operation         OPEN  STANDARD  RESTRICTED
  ---------         ----  --------  ----------
  SECOP_FILE_READ    YES     YES        NO
    (LOAD, BLOAD, MERGE, INPUT #, GET #, LINE INPUT #)

  SECOP_FILE_WRITE   YES     YES        NO
    (SAVE, BSAVE, OPEN FOR OUTPUT/APPEND/RANDOM/BINARY,
     PRINT #, PUT #, KILL, NAME, MKDIR, RMDIR)

  SECOP_COMPILE      YES      NO        NO
    (COMPILE command)

  SECOP_CHAIN        YES      NO        NO
    (CHAIN command — loads and executes another program)

  SECOP_SYSTEM       YES      NO        NO
    (SYSTEM command, SHELL, SHELL$())

  SECOP_MODULE       YES     YES        NO
    (MODULE activation)

  The matrix is stored as a 2D array in security.c:
    static const int allowed[3][6] = {
        { 1, 1, 1, 1, 1, 1 },  /* OPEN       */
        { 1, 1, 0, 0, 0, 1 },  /* STANDARD   */
        { 0, 0, 0, 0, 0, 0 }   /* RESTRICTED */
    };


---------------------------------------------------------------------
19.3  How Security Checks Work Internally
---------------------------------------------------------------------

  Every file-related handler calls security_check() before
  performing the operation:

    int security_check(SecOperation op, int line_num)
    {
        /* Fast path: SEC_OPEN permits everything */
        if (current_level == SEC_OPEN) return 0;

        if (allowed[current_level][op]) return 0;

        /* Denied */
        printf("SORRY? Security: %s not permitted "
               "at level %s", op_names[op],
               level_names[current_level]);
        if (line_num > 0)
            printf(" in line %d", line_num);
        printf("\n");
        return -1;
    }

  KEY DESIGN DECISIONS:

  1. The fast path (SEC_OPEN check) means zero overhead
     for the common case.  No array access, no branching.

  2. The check happens BEFORE fopen() is called.  If
     denied, the OS never even knows a file operation
     was attempted.

  3. The error message tells you exactly what was blocked
     and at what security level, making it easy to
     diagnose why a program can't access files.


---------------------------------------------------------------------
19.4  Setting the Security Level
---------------------------------------------------------------------

  From the command line:
    basicpp --security=standard myprogram.bas
    basicpp --security=restricted myprogram.bas

  From the REPL (direct mode):
    SECURITY "OPEN"
    SECURITY "STANDARD"
    SECURITY "RESTRICTED"

  From C code (for embedding):
    security_init(SEC_STANDARD);
    security_set_level(SEC_RESTRICTED);

  IMPORTANT: In a future version, the SECURITY command
  from BASIC will only be able to INCREASE restrictions,
  never decrease them.  A program running at SEC_STANDARD
  will be able to set SEC_RESTRICTED but not SEC_OPEN.
  This prevents malicious code from lowering its own
  restrictions.


=====================================================================
20. PATH SAFETY
=====================================================================


---------------------------------------------------------------------
20.1  Relative vs. Absolute Paths
---------------------------------------------------------------------

  Relative paths resolve from the current working directory:

    OPEN "data.txt" FOR INPUT AS #1
    ' Opens CWD/data.txt

    OPEN "subdir\data.txt" FOR INPUT AS #1
    ' Opens CWD/subdir/data.txt

  Absolute paths bypass the CWD:

    OPEN "C:\Users\data.txt" FOR INPUT AS #1   ' Windows
    OPEN "/home/user/data.txt" FOR INPUT AS #1 ' Linux

  RECOMMENDATION: Use relative paths whenever possible.
  This makes your programs portable across platforms and
  avoids hardcoded directory structures.


---------------------------------------------------------------------
20.2  Path Traversal Protection
---------------------------------------------------------------------

  Classic BASIC interpreters have no path restrictions.
  BASIC++ at SEC_OPEN follows this tradition — you can
  open any file the OS allows.

  At higher security levels, consider that even though
  file I/O is allowed at SEC_STANDARD, the program can
  still access ANY path.  Future versions may add:
    - Allowed directory whitelist
    - Path traversal detection ("../../../etc/passwd")
    - Sandboxed working directory

  FOR NOW: Be careful when running untrusted programs
  at SEC_STANDARD.  They can read and write any file
  your user account has access to.  If you're worried,
  use SEC_RESTRICTED instead.


---------------------------------------------------------------------
20.3  Reserved Filenames (Windows)
---------------------------------------------------------------------

  Windows has reserved filenames that cannot be used for
  regular files.  BASIC++ does not block these — the OS
  will return an error if you try:

    CON       Console device
    PRN       Printer
    AUX       Auxiliary device
    NUL       Null device
    COM1-COM9 Serial ports
    LPT1-LPT9 Parallel ports
    CLOCK$    System clock

  These names are reserved regardless of extension:
    CON.TXT   is still the console, NOT a file
    NUL.DAT   is still the null device, NOT a file

  On Linux and FreeDOS, there are no reserved filenames.


---------------------------------------------------------------------
20.4  Case Sensitivity (Linux vs Windows)
---------------------------------------------------------------------

  Windows and FreeDOS: case-INSENSITIVE
    OPEN "Data.TXT" and OPEN "data.txt" open the same file.

  Linux: case-SENSITIVE
    OPEN "Data.TXT" and OPEN "data.txt" are different files.

  RECOMMENDATION: Always use consistent casing.  If you
  develop on Windows and deploy on Linux, case mismatches
  will cause "file not found" errors that are hard to debug.


=====================================================================
21. ERROR HANDLING
=====================================================================


---------------------------------------------------------------------
21.1  What Errors Can Occur
---------------------------------------------------------------------

  All file errors raise ERR_HOW ("HOW?").  The error system
  uses a simple three-tier model:

    ERR_WHAT   Syntax error (bad OPEN statement syntax)
    ERR_HOW    Runtime error (file not found, wrong mode, etc.)
    ERR_SORRY  Resource error (out of channels, security denied)

  Specific file error conditions:

    Error Condition                      Error Code
    ---------------                      ----------
    File not found (INPUT mode)          ERR_HOW
    Cannot create file (permission)      ERR_HOW
    Channel number out of range          ERR_HOW
    Channel already open                 ERR_HOW
    Channel not open                     ERR_HOW
    Wrong mode for operation             ERR_HOW
    Read past end of file                ERR_HOW
    Write error (disk full, etc.)        ERR_HOW
    Lock failure                         ERR_HOW
    Security denied                      ERR_SORRY
    Invalid FIELD total exceeds LEN      ERR_HOW
    Channel table full (all 8 in use)    ERR_HOW


---------------------------------------------------------------------
21.2  How Errors Propagate
---------------------------------------------------------------------

  1. An error occurs (e.g., fopen returns NULL)
  2. error_raise(ERR_HOW, line_num) is called
  3. error_raise() prints "HOW? AT LINE 50\n"
  4. error_raise() sets the global error flag
  5. The file operation returns -1
  6. The statement handler checks the return value
  7. The handler returns early (aborts)
  8. The execution loop detects the error flag
  9. If ON ERROR GOTO is active, execution jumps
     to the error handler line
  10. If not, the VM returns to the REPL prompt

  BEEP: If error beep is enabled (default), error_raise()
  also calls vdev_beep() to emit an audible notification.
  Disable with BEEP OFF.


---------------------------------------------------------------------
21.3  ON ERROR GOTO (Error Recovery)
---------------------------------------------------------------------

  Trap and recover from file errors:

    ON ERROR GOTO 5000
    OPEN "missing.txt" FOR INPUT AS #1
    ' If the file doesn't exist, execution jumps to 5000
    LINE INPUT #1, A$
    CLOSE #1
    GOTO 5100

    5000 REM Error handler
    5010 PRINT "File not found!"
    5020 PRINT "Error "; ERR; " at line "; ERL
    5030 RESUME NEXT

    5100 REM Continue here

  Error variables:
    ERR    Numeric error code
    ERL    Line number where the error occurred

  RESUME options:
    RESUME          Retry the failed statement
    RESUME NEXT     Skip to the next statement
    RESUME linenum  Jump to a specific line


---------------------------------------------------------------------
21.4  Defensive Programming Patterns
---------------------------------------------------------------------

  PATTERN 1: Always check before reading

    OPEN "data.txt" FOR INPUT AS #1
    IF EOF(1) THEN
      PRINT "File is empty"
      CLOSE #1
      END
    END IF
    WHILE NOT EOF(1)
      LINE INPUT #1, A$
      PRINT A$
    WEND
    CLOSE #1

  PATTERN 2: Safe file open with error handling

    ON ERROR GOTO 500
    OPEN F$ FOR INPUT AS #1
    ON ERROR GOTO 0              ' Disable handler
    ' ... file operations ...
    CLOSE #1
    GOTO 600

    500 PRINT "Cannot open "; F$
    510 RESUME 600

    600 ' Continue program

  PATTERN 3: Verify channel mode before use

    IF IOCTL$(1) = "" THEN
      PRINT "Channel #1 is not open"
    ELSEIF IOCTL$(1) = "I" THEN
      LINE INPUT #1, A$
    ELSE
      PRINT "Channel #1 is not open for input"
    END IF

  PATTERN 4: Safe cleanup on error

    ON ERROR GOTO 9000
    OPEN "input.txt" FOR INPUT AS #1
    OPEN "output.txt" FOR OUTPUT AS #2
    WHILE NOT EOF(1)
      LINE INPUT #1, A$
      PRINT #2, UCASE$(A$)
    WEND
    CLOSE #1
    CLOSE #2
    END

    9000 CLOSE     ' Close ALL open files
    9010 PRINT "Error "; ERR; " at line "; ERL
    9020 END


=====================================================================
22. BEST PRACTICES FOR SAFE FILE ACCESS
=====================================================================

  1. ALWAYS CLOSE YOUR FILES
     Don't rely on automatic cleanup.  Close files as
     soon as you're done with them.

  2. CHECK EOF() BEFORE READING
     Never read without checking EOF first.

  3. USE ON ERROR GOTO FOR OPEN
     File open is the most failure-prone operation.
     Always trap errors on OPEN.

  4. USE RELATIVE PATHS
     Avoid hardcoded absolute paths.  Use CHDIR if
     needed, then relative paths.

  5. USE CONSISTENT CASING
     Prevents case-sensitivity bugs on Linux.

  6. FLUSH AFTER IMPORTANT WRITES
     Use PRINT #n, expr (with newline, which triggers
     fflush) rather than PRINT #n, expr; (no newline,
     data stays in buffer).

  7. LOCK BEFORE MODIFYING SHARED FILES
     If multiple processes access the same file, use
     LOCK/UNLOCK around write operations.

  8. VALIDATE USER INPUT FOR FILENAMES
     Don't pass raw user input to OPEN without checking:

       INPUT "Filename"; F$
       IF LEN(F$) = 0 THEN PRINT "No filename": END
       IF INSTR(F$, "..") THEN PRINT "Invalid path": END

  9. USE WRITE #/INPUT # FOR DATA
     WRITE produces machine-readable output that INPUT
     can parse back perfectly.  Use PRINT # only for
     human-readable output.

  10. PREFER SEQUENTIAL FOR TEXT, RANDOM FOR DATABASES
      Sequential is simpler and more portable.  Random
      is faster for record-oriented data but more complex.


=====================================================================

              PART IV: CROSS-PLATFORM BEHAVIOR

=====================================================================


=====================================================================
23. LINE ENDINGS
=====================================================================

  The line ending problem is the #1 source of cross-platform
  file bugs.  Here's how BASIC++ handles it:

  Three conventions exist:

    Platform        Newline    Bytes     Name
    --------        -------    -----     ----
    Windows/DOS     \r\n       0D 0A     CRLF
    Linux/macOS     \n         0A        LF
    Old Mac (pre-X) \r         0D        CR

  HOW BASIC++ HANDLES THIS:

  Writing (Sequential mode):
    PRINT #1, "Hello"
    On Windows: writes "Hello\r\n" (6 bytes on disk)
    On Linux:   writes "Hello\n"   (6 bytes on disk)
    The BASIC program writes \n; C stdio's text mode
    translates it to the platform convention.

  Reading:
    LINE INPUT #1, A$
    On any platform: A$ contains "Hello" (no newline)
    fgets() handles all line ending formats.
    fileio_input_line() strips trailing \r and \n.

  Binary mode:
    No translation.  Bytes are read/written as-is.
    A \n stays as 0x0A on all platforms.

  Cross-platform file sharing:
    If you create a file on Windows and read it on Linux,
    the text-mode reading handles the \r\n correctly.
    If you open it in BINARY mode on Linux, you'll see
    the \r bytes — strip them yourself if needed.


=====================================================================
24. PATH SEPARATORS
=====================================================================

  Windows:  backslash \    (forward / also works)
  Linux:    forward slash /
  FreeDOS:  backslash \

  RECOMMENDATION: Use forward slash / in BASIC programs.
  It works on ALL platforms including modern Windows.

    OPEN "data/files/input.txt" FOR INPUT AS #1
    ' Works on Windows, Linux, and FreeDOS

  Avoid:
    OPEN "data\files\input.txt" FOR INPUT AS #1
    ' Only works on Windows and FreeDOS


=====================================================================
25. CHARACTER ENCODING
=====================================================================

  BASIC++ operates on raw bytes.  It does not perform any
  character encoding translation.

  Text mode files:
    - CHR$(n) writes byte value n
    - ASC(c$) returns byte value
    - On Windows: CP1252 or UTF-8 depending on locale
    - On Linux: UTF-8 typically
    - On FreeDOS: CP437

  If you need to handle Unicode text files:
    - Open in BINARY mode to avoid line-ending mangling
    - Read bytes and process UTF-8 sequences manually
    - Or use an external module for Unicode support

  For data files between platforms, stick to ASCII (0-127).
  These are identical across all encodings.


=====================================================================
26. FILE SYSTEM LIMITS
=====================================================================

  Limit                    Windows (NTFS)  Linux (ext4)  FreeDOS (FAT)
  -----                    --------------  ------------  -------------
  Max filename length      260 chars       255 bytes     8.3 (or LFN)
  Max path length          260 chars       4096 bytes    64 chars
  Max file size            16 TB           16 TB         2 GB (FAT16)
                                                         4 GB (FAT32)
  Max open files (OS)      ~2048           ~1024         ~15
  Max open files (BASIC++) 8               8             8
  Case sensitive           No              Yes           No


=====================================================================
27. PLATFORM-SPECIFIC BEHAVIOR TABLE
=====================================================================

  Feature              Windows          Linux            FreeDOS
  -------              -------          -----            -------
  Text mode \n         writes \r\n      writes \n        writes \r\n
  Binary mode \n       writes \n        writes \n        writes \n
  fgets line ending    handles \r\n     handles both     handles \r\n
  fseek text mode      unreliable       reliable         unreliable
  fseek binary mode    reliable         reliable         reliable
  File locking         _locking()       no-op            no-op
  Ctrl-Z in text       signals EOF      ignored          signals EOF
  Path separator       \ and /          / only           \ and /
  Case sensitivity     insensitive      sensitive        insensitive
  Temp file path       %TEMP%           /tmp             C:\TEMP
  Shebang support      no               yes (#!/...)     no
  BEEP on error        Beep(800,200)    putchar('\a')    putchar('\a')


=====================================================================

              PART V: ADVANCED TOPICS

=====================================================================


=====================================================================
28. SAVE AND LOAD (PROGRAM FILE I/O)
=====================================================================

  SAVE and LOAD operate on BASIC program text files — they
  are separate from the file channel system (#1-#8).

  SAVE "myprogram.bas"
    - Opens the file with fopen(filename, "w")
    - Writes each stored program line with fprintf:
      fprintf(fp, "%s\n", store->lines[i].text)
    - Closes the file
    - File is plain ASCII text, one BASIC line per file line

  LOAD "myprogram.bas"
    - Clears the current program (like NEW)
    - Opens the file with fopen(filename, "r")
    - Reads lines with fgets()
    - Strips trailing \r\n
    - Skips blank lines and shebang lines (#!)
    - Parses the line number
    - Inserts each line into the program store
    - Closes the file

  SAVE/LOAD use the FILE: VDev (slot 2) and are subject to
  SECOP_FILE_WRITE / SECOP_FILE_READ security checks.

  The saved format is human-readable:
    10 PRINT "HELLO"
    20 LET A=5
    30 GOTO 10


=====================================================================
29. MERGE AND CHAIN
=====================================================================

  MERGE "library.bas"
    Like LOAD but does NOT clear the current program.
    Lines from the file are merged into the existing program.
    If a line number already exists, it is overwritten.
    New line numbers are inserted in sorted order.

  CHAIN "nextprog.bas"
    Clears the current program, loads the file, and
    begins execution (as if you typed LOAD then RUN).
    Subject to SECOP_CHAIN security check (blocked at
    SEC_STANDARD and SEC_RESTRICTED).


=====================================================================
30. BSAVE AND BLOAD (MEMORY-TO-FILE)
=====================================================================

  BSAVE "memory.bin", address, length
    Saves a range of the 64K virtual memory to a file.
    Writes raw bytes from the MEMMAP address space.

  BLOAD "memory.bin" [, address]
    Loads a file into the virtual memory space.
    Reads raw bytes into the MEMMAP address space.

  These are useful for saving/loading screen contents,
  character sets, or any other memory-mapped data.

  Security: SECOP_FILE_WRITE / SECOP_FILE_READ.


=====================================================================
31. COMPILE (SOURCE-TO-EXECUTABLE)
=====================================================================

  COMPILE "output.c"
    Transpiles the current BASIC program to an ANSI C89
    source file.  The output can be compiled with GCC,
    Clang, MSVC, or OpenWatcom to produce a native
    executable.

  Security: SECOP_COMPILE (blocked at SEC_STANDARD and
  SEC_RESTRICTED).

  See T_Compiling_BASIC_Programs.txt for details.


=====================================================================
32. EXTENDING THE FILESYSTEM VIA VDEV
=====================================================================

  The VDev system allows you to replace the built-in file
  access with custom implementations.  See R_Virtual_Devices.txt
  for complete tutorials on building:

    - ZIP archive readers
    - Network file devices
    - In-memory filesystems
    - Encrypted file devices
    - Remote file access (FTP, HTTP)

  The key insight is that BASIC++ separates the CONCEPT of
  a file (open, read, write, close) from the IMPLEMENTATION.
  By replacing function pointers in the VDev table, you can
  make OPEN "http://example.com/data.csv" FOR INPUT AS #1
  work — the BASIC program doesn't need to know that "open"
  means "HTTP GET" instead of "fopen".


=====================================================================
33. TROUBLESHOOTING AND DEBUGGING
=====================================================================

  PROBLEM: "HOW?" when trying to OPEN a file

    Possible causes:
    1. File not found (check spelling, path, case on Linux)
    2. Permission denied (file is read-only or locked)
    3. Channel already open (CLOSE it first)
    4. Channel number out of range (must be 1-8)
    5. Security blocked (check SECURITY level)

    Debug steps:
    a. PRINT the filename before OPEN:
       PRINT "Opening: ["; F$; "]"
       OPEN F$ FOR INPUT AS #1

    b. Try a known-good file:
       OPEN "test.txt" FOR OUTPUT AS #1
       PRINT #1, "test"
       CLOSE #1

    c. Check security level:
       SECURITY
       (prints current level)

  PROBLEM: "HOW?" when trying to PRINT # or INPUT #

    Possible causes:
    1. Channel not open (OPEN it first)
    2. Wrong mode (reading from OUTPUT, writing to INPUT)
    3. At EOF (check EOF() before INPUT #)

    Debug steps:
    a. Check channel status:
       PRINT IOCTL$(1)
       If "" : channel is closed
       If "I": can only read, not write
       If "O": can only write, not read

  PROBLEM: Data is corrupted or garbled

    Possible causes:
    1. Text/Binary mode mismatch (opened text, expected binary)
    2. Record length mismatch (LEN doesn't match file)
    3. FIELD widths don't match what was written
    4. Line ending issues (Windows file on Linux in binary mode)
    5. Character encoding mismatch

    Debug steps:
    a. Open in BINARY mode and read raw bytes:
       OPEN "data.dat" FOR BINARY AS #1
       FOR I = 1 TO 20
         GET #1, I, B$
         PRINT I; ": "; ASC(B$); " ("; B$; ")"
       NEXT
       CLOSE #1

  PROBLEM: File appears empty after writing

    Possible causes:
    1. Forgot to CLOSE the file (data in buffer, not on disk)
    2. Opened with OUTPUT mode (truncated existing content)
    3. Wrote with trailing semicolon (suppressed newline,
       data may be on one very long line)

    Fix: Always CLOSE files. Check with LOF() after writing.

  PROBLEM: "SORRY?" on file operations

    This means the security system blocked the operation.
    The error message tells you exactly what was denied:
      SORRY? Security: file write not permitted at level
      RESTRICTED

    Fix: Lower the security level (if you trust the program)
    or accept that the program cannot access files.


=====================================================================
34. QUICK REFERENCE TABLES
=====================================================================

  FILE OPEN MODES:

    Mode        Syntax              Read  Write  Create  Truncate
    ----        ------              ----  -----  ------  --------
    INPUT       FOR INPUT           YES   NO     NO      NO
    OUTPUT      FOR OUTPUT          NO    YES    YES     YES
    APPEND      FOR APPEND          NO    YES    YES     NO
    RANDOM      FOR RANDOM          YES   YES    YES     NO
    BINARY      FOR BINARY          YES   YES    YES     NO


  FILE CHANNEL OPERATIONS:

    Operation          INPUT  OUTPUT  APPEND  RANDOM  BINARY
    ---------          -----  ------  ------  ------  ------
    INPUT #              X
    LINE INPUT #         X
    PRINT #                      X       X
    WRITE #                      X       X
    GET #                                        X       X
    PUT #                                        X       X
    FIELD #                                      X
    LSET/RSET                                    X
    LOCK/UNLOCK                                  X       X
    EOF()                X                       X       X
    LOF()                X       X       X       X       X
    LOC()                                        X       X
    SEEK #                                               X
    IOCTL$               X       X       X       X       X


  ERROR CONDITIONS:

    What Happened                    Error    Code
    -------------                    -----    ----
    Syntax error in OPEN statement   WHAT?    ERR_WHAT
    File not found (INPUT mode)      HOW?     ERR_HOW
    Channel out of range (not 1-8)   HOW?     ERR_HOW
    Channel already open             HOW?     ERR_HOW
    Channel not open                 HOW?     ERR_HOW
    Wrong mode for operation         HOW?     ERR_HOW
    Read past EOF                    HOW?     ERR_HOW
    FIELD widths exceed record LEN   HOW?     ERR_HOW
    Security check denied            SORRY.   ERR_SORRY


=====================================================================
35. RELATED MANUALS
=====================================================================

  I_File_IO.txt                 Quick reference for file commands
  M_External_Modules.txt        Building file-access modules
  N_Error_Handling.txt          ON ERROR GOTO, ERR, ERL
  Q_Security.txt                Full security system reference
  R_Virtual_Devices.txt         Custom file devices (ZIP, network)
  T_Compiling_BASIC_Programs.txt Transpiling to native code
  V_Virtual_Machines.txt        VM architecture and console


=====================================================================

      PART VI: DEVICE FILES (PHASE 16)

=====================================================================


=====================================================================
36. DEVICE FILES VS REGULAR FILES
=====================================================================

With VDev2 (Phase 16), the OPEN statement can now target
two types of endpoints:

  REGULAR FILES (traditional):
    OPEN "data.txt" FOR INPUT AS #1
    The filename is passed to fopen() through Layer 5.
    The channel operates in FCHAN_INPUT/OUTPUT/etc. mode.
    I/O goes through C stdio.

  DEVICE FILES (Phase 16):
    OPEN "GPIO17:" FOR OUTPUT AS #1
    The filename is recognized as a device name.
    The channel operates in FCHAN_DEVICE mode.
    I/O goes through the VDev function pointers.

From the BASIC program's perspective, both look identical:
  PRINT #1, "value"   -- works on both files and devices
  LINE INPUT #1, A$   -- works on both files and devices
  CLOSE #1            -- works on both files and devices

The fileio layer transparently routes to the correct
backend based on the channel's mode.


=====================================================================
37. DEVICE NAME DETECTION IN OPEN
=====================================================================

When OPEN is called, the filename is checked BEFORE hitting
the filesystem:

  1. Is the filename a registered VDev name?
     Check via vdev_find_by_name(filename).
     This is a case-insensitive scan of all device slots.

  2. Does the filename end with ":" ?
     Device names conventionally end with colon:
       CON:  ERR:  FILE:  COM1:  GPIO17:  I2C:  BT:
     This convention follows MS-DOS and GW-BASIC traditions.

  3. If a VDev match is found:
     - Call fileio_open_device(chan, dev_id, mode, path, line)
     - The channel enters FCHAN_DEVICE mode
     - The VDev's dev_open() is called if available
     - The VDev's dev_req_caps are checked against security

  4. If NO VDev match:
     - Treat the filename as a regular file path
     - Proceed through Layer 3 (security) and Layer 5 (stdio)
     - Normal fopen() behavior

  Resolution order:

    OPEN "COM1:" ...  ->  vdev_find_by_name("COM1:") -> found -> device
    OPEN "data.txt" ...  ->  vdev_find_by_name("data.txt") -> not found
                         ->  fopen("data.txt", ...) -> file

  IMPORTANT: Device names take PRECEDENCE over filenames.
  If you register a device called "data.txt:", then
  OPEN "data.txt:" will open the device, not a file.
  This matches how MS-DOS handled CON: and PRN:.


=====================================================================
38. THE /DEV/ NAMESPACE (LINUX INTEGRATION)
=====================================================================

On Linux, device files live in /dev/:
  /dev/ttyS0     Serial port
  /dev/i2c-1     I2C bus
  /dev/spidev0.0 SPI bus
  /dev/video0    Camera
  /dev/hidraw0   HID device

BASIC++ VDev modules for Linux devices use these /dev/ paths
internally.  The BASIC program uses the abstract device name:

  BASIC sees:         Module opens internally:
  -----------         ----------------------
  "COM1:"             /dev/ttyS0
  "I2C:0x48"          /dev/i2c-1 + ioctl(I2C_SLAVE, 0x48)
  "SPI0:"             /dev/spidev0.0
  "CAM0:"             /dev/video0 (V4L2 API)
  "JOY0:"             /dev/hidraw0

The BASIC program NEVER directly opens /dev/ paths.
All /dev/ access is mediated through VDev modules which:
  1. Are compiled with platform-specific code (#ifdef __linux__)
  2. Declare appropriate CAP_ flags (CAP_GPIO, CAP_USB, etc.)
  3. Are gated by the security system

On Windows and FreeDOS, the same BASIC programs use
different modules that access hardware through their
platform's native APIs (Win32 HID, DOS BIOS, etc.)


=====================================================================
39. SECURITY SCOPING FOR DEVICE-BACKED PATHS
=====================================================================

Device access is double-gated:

  GATE 1: Module capability check (module.h)
    When a module registers a device, it declares
    dev_req_caps (required CAP_ flags).  The module system
    checks that these capabilities are allowed at the
    current security level before loading the module.

    CAP_GPIO      -> requires SEC_OPEN or explicit grant
    CAP_CAMERA    -> requires SEC_OPEN or explicit grant
    CAP_NETWORK   -> requires SEC_OPEN or SEC_STANDARD

  GATE 2: File operation security check (security.c)
    Even after the module is loaded and the device is
    registered, every OPEN/PRINT/INPUT on a device channel
    still passes through security_check():

    SECOP_FILE_READ   (for INPUT from device)
    SECOP_FILE_WRITE  (for OUTPUT to device)

    In SEC_RESTRICTED mode, ALL device I/O is blocked.
    The device exists in the registry but cannot be opened.

  COMBINED EFFECT:

    Security Level  Module Load  Device OPEN  Device I/O
    -------------- ------------ ------------ ----------
    SEC_OPEN        ALLOWED      ALLOWED      ALLOWED
    SEC_STANDARD    ALLOWED      ALLOWED      ALLOWED
    SEC_RESTRICTED  DENIED       DENIED       DENIED

  WHY DOUBLE-GATING:

    A malicious BASIC program cannot:
    1. Load a GPIO module (module load denied)
    2. Even if it could, open a GPIO device (security denied)
    3. Even if it could, write to it (I/O denied)

    Each layer is defense-in-depth.  Compromising one
    gate does not compromise the others.


=====================================================================
40. UPDATED RELATED MANUALS
=====================================================================

  I_File_IO.txt                 File I/O + device-backed channels
  M_External_Modules.txt        Building file-access and device modules
  N_Error_Handling.txt          ON ERROR GOTO, ERR, ERL
  Q_Security.txt                Full security system reference
  R_Virtual_Devices.txt         VDev/VDev2 API reference
  T_Compiling_BASIC_Programs.txt Transpiling to native code
  V_Virtual_Machines.txt        VM architecture and device bus
  W_Virtual_Filesystem.txt      (this manual)


=====================================================================
END OF DOCUMENT
=====================================================================
