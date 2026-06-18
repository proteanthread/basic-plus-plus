Library System
==============

BASIC++ supports loading external libraries (.LIB source files
and .BPL pre-compiled binary files) that provide reusable
SUB, FUNCTION, and DEF FN definitions.

LOADING LIBRARIES
-----------------

  LOAD LIBRARY "filename.lib"

    Loads a source library. The interpreter:
    1. Reads REM @LIBRARY, @VERSION, @SECURITY headers
    2. Checks security level compatibility
    3. Scans for SUB/FUNCTION/DEF FN declarations
    4. Stores source lines for interpreter-mode execution
    5. Initializes an isolated variable space (26 vars A-Z)

  LOAD LIBRARY "filename.bpl"

    Loads a pre-compiled binary library. Same as source
    but skips parsing — loads symbols and source lines
    directly from the portable binary format.

UNLOADING LIBRARIES
-------------------

  UNLOAD LIBRARY "LIBNAME"

    Removes a loaded library from memory. Frees all
    associated symbol table entries and source lines.

CALLING LIBRARY FUNCTIONS
-------------------------

  CALL LibName_SubName(arg1, arg2, ...)

    Calls a SUB defined in a loaded library. Arguments
    are evaluated and passed to the library's variable
    space. The caller's variables are saved and restored.

  LET result = LibName_FuncName(arg1, arg2)

    Calls a FUNCTION and returns a value.

COMPILING LIBRARIES
-------------------

  COMPILE LIBRARY "LIBNAME"

    Saves a loaded library to .BPL (BASIC++ Portable
    Library) binary format. The output file defaults
    to LIBNAME.bpl.

  COMPILE LIBRARY "LIBNAME", "output.bpl"

    Saves to a specific output file.

.BPL FILE FORMAT
----------------

  The .BPL format is a portable, OS-independent binary:

  HEADER (32 bytes):
    Bytes  0-3:   Magic "BPL\x1A"
    Byte   4:     Format version (1)
    Byte   5:     Security level
    Byte   6:     Extension type (LIB/FN/FT/MOD/PLG)
    Byte   7:     Flags (reserved)
    Bytes  8-9:   Symbol count (LE16)
    Bytes 10-11:  Source line count (LE16)
    Bytes 12-13:  Reserved (LE16)
    Bytes 14-15:  CRC16 checksum
    Bytes 16-31:  Library name (NUL-padded)

  SYMBOL TABLE (variable-length entries):
    1 byte:   type (0=SUB, 1=FUNCTION, 2=DEF_FN)
    1 byte:   parameter count
    2 bytes:  entry offset (LE16)
    1 byte:   name length
    N bytes:  name (ASCII)

  SOURCE LINES (4 + len bytes each):
    2 bytes:  virtual line number (LE16)
    2 bytes:  text length (LE16)
    N bytes:  line text (ASCII)

LIBRARY FILE FORMAT (.LIB)
--------------------------

  Libraries are plain text files with REM header tags:

    REM @LIBRARY LibName
    REM @VERSION 1.0
    REM @SECURITY OPEN
    REM @REQUIRES NONE

    SUB LibName_Init(width, height)
        LOCAL w, h
        w = width: h = height
        ...
    END SUB

    FUNCTION LibName_Calculate(x, y)
        LibName_Calculate = x * y + 1
    END FUNCTION

    DEF FN LibName_Square(x) = x * x

SECURITY
--------

  Libraries specify their required security level via:

    REM @SECURITY OPEN       (Level 0 - no restrictions)
    REM @SECURITY SAFE       (Level 1)
    REM @SECURITY STANDARD   (Level 2)
    REM @SECURITY RESTRICTED (Level 3)
    REM @SECURITY STRICT     (Level 4)

  The interpreter's current security level must be >=
  the library's required level for loading to succeed.
  Level 5 (PARANOID) blocks all external loading.

  See: HELP SECURITY

VARIABLE ISOLATION
------------------

  Each library has its own isolated variable space:
  - 26 numeric variables (A-Z) per library
  - 26 string variables (A$-Z$) per library
  - Named variables within SUBs use LOCAL for scoping
  - Library variables persist between calls
  - Caller's variables are saved/restored on CALL

SAMPLE LIBRARIES
----------------

  BASIC++ ships with 7 sample libraries in ./samples/:

  TURTLE.LIB  - Text-based turtle graphics engine
  TINYDB.LIB  - Simple SQL-like database commands
  REGEX.LIB   - Regular expression matching
  ADVENT.LIB  - Text adventure game toolkit
  GAMEKIT.LIB - Console game development kit
  IOTKIT.LIB  - IoT device simulation toolkit
  EXTCMDS.LIB - Additional utility commands

EXAMPLES
--------

  Example 1: Load and use turtle graphics

    LOAD LIBRARY "samples/TURTLE.LIB"
    CALL TURTLE_INIT(40, 20)
    CALL TURTLE_PENDOWN
    CALL TURTLE_FORWARD(10)
    CALL TURTLE_RIGHT(90)
    CALL TURTLE_FORWARD(10)
    CALL TURTLE_DRAW
    UNLOAD LIBRARY "TURTLE"

  Example 2: Compile a library

    LOAD LIBRARY "samples/TURTLE.LIB"
    COMPILE LIBRARY "TURTLE"
    REM Creates TURTLE.bpl
    UNLOAD LIBRARY "TURTLE"
    LOAD LIBRARY "TURTLE.bpl"
    REM Loaded from pre-compiled binary

SEE ALSO
--------

  HELP SECURITY
  HELP MODULES
  HELP SUB
  HELP FUNCTION
