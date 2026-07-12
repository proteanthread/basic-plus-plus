EXTERNAL MODULES, FUNCTIONS, AND PLUG-INS
==========================================
Version 4.2.3

BASIC++ has a module system that enables external code to extend
the interpreter with new functions, statements, devices, and
access to system services.

This manual is your complete guide to building extensions — from
a simple one-function add-on to a full-featured OS-integration
module.  It covers writing extensions in both C and BASIC++,
accessing platform-specific features safely, and using the
security system to protect users from malicious modules.


TABLE OF CONTENTS
=================

  Part I:   Architecture
  1.  Module System Overview
  2.  The Extension Stack (How Extensions Fit In)
  3.  The Function Registry (funcreg)
  4.  The Module Descriptor (ModuleInfo)
  5.  Capability Flags and Security

  Part II:  Writing Extensions in BASIC++
  6.  BASIC Modules via MERGE/CHAIN
  7.  DEF FN User Functions
  8.  BASIC Library Files (Pattern)
  9.  Multi-File BASIC Projects

  Part III: Writing Extensions in C
  10. Your First C Function (Step-by-Step)
  11. The BValue API (Working with Values)
  12. The FuncHandler Signature
  13. Registering Functions (FunctionEntry)
  14. Writing a Complete C Module
  15. Adding New Statements (VMHandler)
  16. Adding New VDev Devices
  17. Dialect-Overridable Functions
  18. Build Integration (Makefile)

  Part IV:  Accessing OS-Specific Features Safely
  19. The Safety Philosophy
  20. The Capability Gate
  21. Platform Detection (#ifdef Guards)
  22. Windows-Specific Features
      22.1 Registry Access
      22.2 Clipboard Access
      22.3 Process Management
      22.4 Windows Services
      22.5 COM/OLE Automation
  23. Linux-Specific Features
      23.1 POSIX File Operations
      23.2 Process Forking and Pipes
      23.3 Signal Handling
      23.4 Syslog Integration
      23.5 GPIO and Hardware (Embedded)
  24. FreeDOS-Specific Features
      24.1 BIOS Interrupts
      24.2 Direct Port I/O
      24.3 Real-Mode Memory Access
      24.4 TSR (Terminate-Stay-Resident)
  25. Cross-Platform Patterns

  Part V:   Security in Depth
  26. The Security Model for Modules
  27. The Permission Matrix
  28. Security Levels and Module Capabilities
  29. How module_activate() Enforces Security
  30. Writing Secure Modules (Best Practices)
  31. Input Validation and Sanitization
  32. Principle of Least Privilege
  33. Auditing and Logging

  Part VI:  Complete Module Examples
  34. Example: Math Extensions Module
  35. Example: String Utilities Module
  36. Example: Environment and OS Info Module
  37. Example: File Utilities Module
  38. Example: Serial Port Module (Windows)
  39. Example: Network HTTP Module (Windows)
  40. Example: SQLite Database Module

  Part VII: Self-Hosting Specifications
  41. The .BPPSPEC File Format
  42. LOAD FEATURE Command
  43. Companion .LIB Dispatch

  Part VIII: Reference
  44. API Quick Reference
  45. Module Checklist
  46. Troubleshooting
  47. Related Manuals


=====================================================================

              PART I: ARCHITECTURE

=====================================================================


=====================================================================
1. MODULE SYSTEM OVERVIEW
=====================================================================

The module system has three layers, from simplest to most
powerful:

  Layer 1: BASIC Modules (MERGE/CHAIN/DEF FN)
    Write extension code in BASIC++ itself.
    No compilation required.  Portable across all platforms.
    Limited to what BASIC++ can express natively.

  Layer 2: Built-In C Modules (mod_*.c)
    Write extension code in C, compile it into the
    interpreter.  Full access to the host OS.
    Registered via the function registry (funcreg) and
    module system (module.h).

  Layer 3: External Plug-Ins (prototype only)
    Dynamically loaded shared libraries (.dll/.so) that
    register new keywords and functions at runtime without
    recompiling the interpreter.  The module_load_dynamic()
    prototype exists in module.h, but the full implementation
    is not yet complete.

> [!WARNING]
> **PLANNED / FUTURE** — Layer 3 dynamic plug-in loading is not yet fully implemented.

This manual covers Layers 1 and 2 in full detail.


=====================================================================
2. THE EXTENSION STACK (HOW EXTENSIONS FIT IN)
=====================================================================

When a BASIC program calls a function, the call follows
this path:

  BASIC code: X = MYFUNC(42)
    |
    v
  Parser: recognizes MYFUNC as a function name
    |
    v
  Function Registry: funcreg_find_by_name("MYFUNC")
    |
    v
  FunctionEntry: validates arg count (min=1, max=1)
    |
    v
  Security Check: is MYFUNC's safety level allowed?
    |
    v
  FuncHandler: my_func(args, argc, rt) called
    |
    v
  Handler: computes result, returns BValue
    |
    v
  Parser: assigns result to X

Extensions plug into this pipeline at three points:

  1. FUNCTION REGISTRY (funcreg)
     Register new callable functions.
     Example: MYFUNC(), HTTPGET$(), GPIO_READ()

  2. VM DISPATCH TABLE (vm)
     Register new statement handlers.
     Example: PLOT, MQTT, DBQUERY

  3. VDEV DEVICE TABLE (vdev)
     Register new I/O devices.
     Example: SER:, NET:, GPIO:


=====================================================================
3. THE FUNCTION REGISTRY (funcreg)
=====================================================================

The function registry is a static table of up to 128
FunctionEntry structures.  Every callable function in
BASIC++ — from ABS() to VAL() to user-defined modules —
lives in this table.

Registry structure:

  struct FunctionEntry {
      const char     *name;        /* "ABS", "MYFUNC" */
      KeywordId       keyword;     /* KW_ABS or KW_COUNT */
      FuncCategory    category;    /* FCAT_MATH, FCAT_USER */
      FuncReturnType  ret_type;    /* FRET_INT, FRET_STRING */
      int             min_args;    /* minimum arguments */
      int             max_args;    /* maximum arguments */
      FuncSafety      safety;      /* FSAFE_PURE ... FSAFE_SYSTEM */
      int             overridable; /* 1=dialect can replace */
      FuncHandler     handler;     /* C function pointer */
      const char     *help_text;   /* one-line description */
  };

Registry API:

  funcreg_init()                Clear the registry (boot)
  funcreg_register(&entry)      Add a function
  funcreg_find_by_name("ABS")   Look up by name
  funcreg_find_by_keyword(kw)   Look up by keyword
  funcreg_override(kw, handler) Replace handler (dialects)
  funcreg_count()               Number of registered funcs
  funcreg_get(index)            Get entry by index

Categories:

  FCAT_CORE    Language primitives (always available)
  FCAT_MATH    Math functions (SIN, COS, ABS, SQR)
  FCAT_STRING  String functions (LEN, LEFT$, MID$)
  FCAT_IO      Input/output functions
  FCAT_UTIL    Utility functions (RND, TIMER)
  FCAT_USER    User/module-registered functions

Return types:

  FRET_INT     Returns integer (INT, ASC, LEN)
  FRET_FLOAT   Returns float (SIN, COS, SQR)
  FRET_STRING  Returns string (CHR$, LEFT$, MID$)
  FRET_ANY     Return type depends on input

Safety levels:

  FSAFE_PURE    No side effects (ABS, LEN, SIN)
  FSAFE_STATE   Reads/modifies interpreter state (RND)
  FSAFE_IO      Performs I/O (INKEY$, ENVIRON$)
  FSAFE_SYSTEM  Accesses OS resources (SHELL$, PEEK)


=====================================================================
4. THE MODULE DESCRIPTOR (ModuleInfo)
=====================================================================

A module is a named, versioned unit that groups related
functions, devices, or dialect extensions together.

  struct ModuleInfo {
      const char    *name;         /* "STDLIB", "SERIAL" */
      const char    *version;      /* "1.0" */
      const char    *description;  /* one-line summary */
      ModuleClass    mod_class;    /* Library/Dialect/Device/Extension */
      unsigned int   capabilities; /* CAP_ bitfield */
      int          (*init)(void*); /* init callback */
      void         (*cleanup)(void); /* cleanup callback */
  };

> [!WARNING]
> **PLANNED / FUTURE** — The `required_level` field (security pinning for modules) is not yet implemented in the ModuleInfo struct. The security pinning description below describes planned behavior.

  The planned required_level field will control security pinning:
    SEC_COUNT     Unpinned (works at any security level)
    SEC_OPEN      Only loads when security is OPEN
    SEC_STANDARD  Only loads when security is STANDARD or lower
    SEC_SAFE      Only loads when security is SAFE or lower

  Built-in modules should use SEC_COUNT (unpinned).
  External modules should set the level that matches their
  actual security requirements.

Module classes:

  MOD_LIBRARY    Function library (registers functions)
  MOD_DIALECT    Dialect extension (reconfigures syntax)
  MOD_DEVICE     Virtual device driver (registers VDevs)
  MOD_EXTENSION  General extension (any combination)

Module lifecycle:

  1. module_register(&info)       Store in module table
  2. module_activate("NAME", rt)  Call init callback
  3. ... module is active ...     Functions available
  4. module_deactivate("NAME")    Call cleanup callback

From BASIC:

  MODULE "SERIAL"                 Activate the SERIAL module
  MODULE                          List all modules

Maximum 64 modules (configurable in config.h).


=====================================================================
5. CAPABILITY FLAGS AND SECURITY
=====================================================================

Every module declares what system resources it needs:

  Flag            Hex      Letter  Description
  ----            ---      ------  -----------
  CAP_NONE        0x0000   -       No capabilities needed
  CAP_MATH        0x0001   M       Mathematical operations
  CAP_STRING      0x0002   S       String manipulation
  CAP_IO          0x0004   I       General I/O operations
  CAP_FILE        0x0008   F       File system access
  CAP_SYSTEM      0x0010   Y       System-level operations
  CAP_GRAPHICS    0x0020   G       Graphics device access
  CAP_SOUND       0x0040   A       Audio/sound device
  CAP_NETWORK     0x0080   N       Network access
  CAP_GPIO        0x0100   P       GPIO / digital I/O pins
  CAP_I2C         0x0200   2       I2C bus access
  CAP_SPI         0x0400   3       SPI bus access
  CAP_SENSOR      0x0800   R       Sensor hardware (accel, GPS)
  CAP_CAMERA      0x1000   C       Camera / video capture
  CAP_BLUETOOTH   0x2000   B       Bluetooth / BLE
  CAP_USB         0x4000   U       USB device access

  The modern capability flags (GPIO through USB) follow
  the same enforcement rules as the core flags. At
  SEC_RESTRICTED, only CAP_MATH and CAP_STRING are
  permitted; all hardware and I/O flags are denied.

These flags serve two purposes:

  1. DOCUMENTATION: Users can inspect a module's capabilities
     before activating it:
       MODULE
       STDLIB    1.0  Library  MS   Standard function library
       SERIAL    1.0  Device   IY   RS-232 serial port driver
     The "IY" tells the user this module does I/O and uses
     system resources.

  2. ENFORCEMENT: The security system checks capabilities
     against the current security level before activation:
       SEC_OPEN:       all modules allowed
       SEC_STANDARD:   modules with CAP_SYSTEM blocked
       SEC_RESTRICTED: modules with IO/FILE/SYSTEM/GFX/SND/NET blocked


=====================================================================

              PART II: WRITING EXTENSIONS IN BASIC++

=====================================================================


=====================================================================
6. BASIC MODULES VIA MERGE/CHAIN
=====================================================================

The simplest form of code extension — load BASIC code from
a file and merge it into the current program.

6.1  MERGE (Add code to current program)

  File: MATHLIB.BAS
  ----
  5000 REM ============= MATH LIBRARY =============
  5010 REM Call: A=x, B=y, GOSUB line -> result in R
  5100 REM --- MAX(A, B) ---
  5110 IF A > B THEN R = A ELSE R = B
  5120 RETURN
  5200 REM --- MIN(A, B) ---
  5210 IF A < B THEN R = A ELSE R = B
  5220 RETURN
  5300 REM --- CLAMP(A, lo=B, hi=C) ---
  5310 IF A < B THEN R = B : RETURN
  5320 IF A > C THEN R = C : RETURN
  5330 R = A
  5340 RETURN
  5400 REM --- LERP(A=t, B=start, C=end) ---
  5410 R = B + A * (C - B)
  5420 RETURN
  ----

  Main program:
  ----
  10 MERGE "MATHLIB.BAS"
  20 A = 150 : B = 0 : C = 100
  30 GOSUB 5300 : REM CLAMP
  40 PRINT "Clamped: "; R     ' 100
  50 END
  ----

  Convention: Library subroutines use high line numbers
  (5000+, 8000+, 9000+) to avoid collisions with the
  main program.

6.2  CHAIN (Load and execute another program)

  CHAIN "NEXTPROG.BAS"

  This clears the current program, loads the new one,
  and begins execution.  Variables from the current
  program are available in the new program.

  Use CHAIN for multi-module applications:

    MENU.BAS  -> user selects an option
    CHAIN "EDITOR.BAS"   -> load the editor
    CHAIN "GAME.BAS"     -> load the game
    CHAIN "MENU.BAS"     -> return to menu


=====================================================================
7. DEF FN USER FUNCTIONS
=====================================================================

Define callable functions directly in BASIC:

  DEF FN SQUARE(X) = X * X
  DEF FN CUBE(X) = X * X * X
  DEF FN HYPOTENUSE(A, B) = SQR(A*A + B*B)
  DEF FN CLAMP(X, LO, HI) = -(X<LO)*LO -(X>HI)*HI -(X>=LO AND X<=HI)*X

  PRINT FN SQUARE(7)         ' 49
  PRINT FN CUBE(3)           ' 27
  PRINT FN HYPOTENUSE(3, 4)  ' 5

  Limits:
  - Maximum 256 user functions (MAX_USER_FUNCS)
  - Maximum 4 parameters per function (MAX_FN_PARAMS)
  - Body is a single expression (no multi-line)
  - Can reference global variables and other FN functions
  - Cannot perform I/O or control flow

  See L_Advanced_DEF.txt for advanced patterns.


=====================================================================
8. BASIC LIBRARY FILES (PATTERN)
=====================================================================

A reusable pattern for BASIC libraries:

  File: STRLIB.BAS
  ----
  8000 REM ========= STRING LIBRARY =========
  8010 REM
  8020 REM GOSUB 8100: REVERSE$(A$) -> R$
  8030 REM GOSUB 8200: REPEAT$(A$, N) -> R$
  8040 REM GOSUB 8300: CONTAINS(A$, B$) -> R (1/0)
  8050 REM GOSUB 8400: TRIM$(A$) -> R$
  8060 REM
  8100 REM --- REVERSE$ ---
  8110 R$ = ""
  8120 FOR ZZ = LEN(A$) TO 1 STEP -1
  8130   R$ = R$ + MID$(A$, ZZ, 1)
  8140 NEXT ZZ
  8150 RETURN
  8200 REM --- REPEAT$ ---
  8210 R$ = ""
  8220 FOR ZZ = 1 TO N
  8230   R$ = R$ + A$
  8240 NEXT ZZ
  8250 RETURN
  8300 REM --- CONTAINS ---
  8310 R = (INSTR(A$, B$) > 0)
  8320 RETURN
  8400 REM --- TRIM$ ---
  8410 R$ = LTRIM$(RTRIM$(A$))
  8420 RETURN
  ----

  Usage:
    10 MERGE "STRLIB.BAS"
    20 A$ = "Hello World"
    30 GOSUB 8100
    40 PRINT R$            ' "dlroW olleH"

8.5 LOAD LIBRARY / UNLOAD LIBRARY (NEW)
=====================================================================

BASIC++ 4.2.3 adds a dedicated library loading system that replaces
MERGE for reusable code. Libraries use SUB/FUNCTION definitions
with isolated variable spaces.

LOADING:

  LOAD LIBRARY "filename.lib"     Load source library
  LOAD LIBRARY "filename.bpl"     Load pre-compiled binary

  The interpreter:
  1. Reads REM @LIBRARY / @VERSION / @SECURITY headers
  2. Checks security level compatibility
  3. Scans for SUB / FUNCTION / DEF FN declarations
  4. Stores source lines for interpreter-mode execution
  5. Initializes an isolated variable space (A-Z per library)

UNLOADING:

  UNLOAD LIBRARY "LIBNAME"

  Removes the library from memory and frees all associated
  symbol table entries and source lines.

CALLING LIBRARY FUNCTIONS:

  CALL LibName_SubName(arg1, arg2, ...)
  LET result = LibName_FuncName(arg1, arg2)

  Arguments are passed into the library's isolated A-Z variable
  space. The caller's variables are saved and restored on return.

COMPILING TO .BPL:

  COMPILE LIBRARY "LIBNAME"
  COMPILE LIBRARY "LIBNAME", "output.bpl"

  Saves a loaded library to the .BPL (BASIC++ Portable Library)
  binary format. .BPL files are OS-independent and load faster
  than source .LIB files.

LIBRARY FILE FORMAT:

  REM @LIBRARY LibName
  REM @VERSION 1.0
  REM @SECURITY OPEN
  REM @REQUIRES NONE

  SUB LibName_Init(width, height)
      LOCAL w, h
      w = width: h = height
  END SUB

  FUNCTION LibName_Calculate(x, y)
      LibName_Calculate = x * y + 1
  END FUNCTION

SEE ALSO: HELP LIBRARY (for full .BPL format spec and examples)


=====================================================================
9. MULTI-FILE BASIC PROJECTS
=====================================================================

For larger projects, split into multiple .BAS files:

  PROJECT/
    MAIN.BAS         10-999     Main program logic
    UI.BAS           1000-1999  User interface subroutines
    DATA.BAS         2000-2999  Data handling subroutines
    FILEOPS.BAS      3000-3999  File operations
    MATHLIB.BAS      5000-5999  Math library
    STRLIB.BAS       8000-8999  String library

  MAIN.BAS:
    1 REM === MAIN PROGRAM ===
    2 REM Load all modules
    5 MERGE "UI.BAS"
    6 MERGE "DATA.BAS"
    7 MERGE "FILEOPS.BAS"
    8 MERGE "MATHLIB.BAS"
    9 MERGE "STRLIB.BAS"
    10 REM === Begin main logic ===
    ...

  The MERGE commands add all the library subroutines to
  the program.  Line number ranges prevent collisions.


=====================================================================

              PART III: WRITING EXTENSIONS IN C

=====================================================================


=====================================================================
10. YOUR FIRST C FUNCTION (STEP-BY-STEP)
=====================================================================

Let's create a function called CUBE(x) that returns x^3.

STEP 1: Write the handler function

  The handler must follow the FuncHandler signature:

    BValue handler(BValue *args, int argc, void *rt);

  Complete handler:

    #include "value.h"
    #include "funcreg.h"

    BValue fn_cube(BValue *args, int argc, void *rt)
    {
        double x;
        (void)argc;  /* already validated by registry */
        (void)rt;    /* not needed for pure math */

        x = bval_to_float(&args[0]);
        return bval_float(x * x * x);
    }

  That's it.  The function receives one argument (args[0]),
  converts it to a double, computes x^3, and returns the
  result as a BValue float.

STEP 2: Create the FunctionEntry

    FunctionEntry cube_entry;
    memset(&cube_entry, 0, sizeof(cube_entry));
    cube_entry.name       = "CUBE";
    cube_entry.keyword    = KW_COUNT;     /* no keyword - name-based */
    cube_entry.category   = FCAT_MATH;
    cube_entry.ret_type   = FRET_FLOAT;
    cube_entry.min_args   = 1;
    cube_entry.max_args   = 1;
    cube_entry.safety     = FSAFE_PURE;   /* no side effects */
    cube_entry.overridable = 0;
    cube_entry.handler    = fn_cube;
    cube_entry.help_text  = "Return x cubed";

STEP 3: Register it

    funcreg_register(&cube_entry);

STEP 4: Use it from BASIC

    PRINT CUBE(3)         ' 27
    PRINT CUBE(10)        ' 1000
    PRINT CUBE(-2)        ' -8


=====================================================================
11. THE BVALUE API (WORKING WITH VALUES)
=====================================================================

BValue is the universal value type in BASIC++.  Every
argument and return value uses BValue.

CREATING VALUES:

  bval_int(42)              Integer value: 42
  bval_float(3.14)          Float value: 3.14
  bval_string(ptr, len)     String value (borrows pointer)

READING VALUES FROM ARGUMENTS:

  long   n = bval_to_int(&args[0]);     Convert arg to integer
  double x = bval_to_float(&args[0]);   Convert arg to float

  /* For string arguments: */
  if (bval_is_string(&args[0])) {
      char *s = args[0].v.sval.data;
      int   l = args[0].v.sval.length;
  }

TYPE CHECKING:

  bval_is_int(&v)           Is it an integer?
  bval_is_float(&v)         Is it a float?
  bval_is_string(&v)        Is it a string?
  bval_is_numeric(&v)       Is it int OR float?

TYPE COERCION:

  Coercion is automatic and follows these rules:
    int -> float:     lossless promotion
    float -> int:     truncation (INT function)
    string -> number: parsed via strtol/strtod
    number -> string: formatted via sprintf

RETURNING VALUES:

  return bval_int(42);           Return an integer
  return bval_float(3.14);       Return a float

  /* Returning a string requires the string pool: */
  RuntimeState *state = (RuntimeState *)rt;
  char *buf = stringpool_alloc(&state->strpool, 5);
  memcpy(buf, "Hello", 5);
  return bval_string(buf, 5);

STRING POOL RULE:

  String data lives in a pool, NOT on the heap.  When
  returning strings from a function, you MUST allocate
  from the string pool via RuntimeState.  Never use
  malloc() for string return values.

  The string pool is reset on every RUN, so pool-allocated
  strings are automatically freed.


=====================================================================
12. THE FUNCHANDLER SIGNATURE
=====================================================================

  BValue handler(BValue *args, int argc, void *rt)

  Parameters:
    args  - Array of pre-evaluated argument values.
            args[0] is the first argument, args[1] the second, etc.
            Arguments have already been type-coerced as needed.

    argc  - Number of arguments actually passed.
            Always satisfies: min_args <= argc <= max_args.
            The registry validates this BEFORE calling the handler.
            You do NOT need to check argc yourself (unless you
            support optional arguments via min != max).

    rt    - Opaque pointer to RuntimeState.
            Cast to (RuntimeState*) when you need:
              - String pool access (for returning strings)
              - Variable access
              - Program state queries
            For pure math functions, you can ignore this parameter.

  Return:
    A BValue containing the function result.
    On error, call error_raise() and return bval_int(0).

  OPTIONAL ARGUMENT PATTERN:

    /* MYFUNC(x) or MYFUNC(x, y) */
    /* min_args=1, max_args=2 */
    BValue fn_myfunc(BValue *args, int argc, void *rt)
    {
        double x = bval_to_float(&args[0]);
        double y = 1.0;  /* default value */
        (void)rt;

        if (argc >= 2) {
            y = bval_to_float(&args[1]);
        }

        return bval_float(x * y);
    }


=====================================================================
13. REGISTERING FUNCTIONS (FunctionEntry)
=====================================================================

SINGLE FUNCTION REGISTRATION:

  FunctionEntry entry;
  memset(&entry, 0, sizeof(entry));
  entry.name     = "MYFUNC";
  entry.keyword  = KW_COUNT;     /* no lexer keyword */
  entry.category = FCAT_USER;
  entry.ret_type = FRET_FLOAT;
  entry.min_args = 1;
  entry.max_args = 2;
  entry.safety   = FSAFE_PURE;
  entry.handler  = fn_myfunc;
  entry.help_text = "Custom function";
  funcreg_register(&entry);

BATCH REGISTRATION (recommended for modules):

  static const FunctionEntry my_entries[] = {
      { "CUBE",   KW_COUNT, FCAT_MATH, FRET_FLOAT,
        1, 1, FSAFE_PURE, 0, fn_cube,
        "Return x cubed" },
      { "HYPOT",  KW_COUNT, FCAT_MATH, FRET_FLOAT,
        2, 2, FSAFE_PURE, 0, fn_hypot,
        "Hypotenuse of a,b" },
      { "CLAMP",  KW_COUNT, FCAT_MATH, FRET_FLOAT,
        3, 3, FSAFE_PURE, 0, fn_clamp,
        "Clamp x between min,max" },
  };

  int i;
  for (i = 0; i < 3; i++)
      funcreg_register(&my_entries[i]);


=====================================================================
14. WRITING A COMPLETE C MODULE
=====================================================================

A complete module consists of three files:

  mod_mymod.h    Header (declarations)
  mod_mymod.c    Implementation (handlers + registration)
  ModuleInfo     Descriptor (metadata)

TEMPLATE: mod_mymod.h

  #ifndef MOD_MYMOD_H
  #define MOD_MYMOD_H
  void mod_mymod_register(void);
  #endif

TEMPLATE: mod_mymod.c

  #include <string.h>
  #include "mod_mymod.h"
  #include "module.h"
  #include "funcreg.h"
  #include "value.h"

  /* ======================== */
  /*  Function Handlers       */
  /* ======================== */

  static BValue fn_myfunc1(BValue *args, int argc, void *rt)
  {
      (void)argc; (void)rt;
      return bval_float(bval_to_float(&args[0]) * 2.0);
  }

  static BValue fn_myfunc2(BValue *args, int argc, void *rt)
  {
      (void)argc; (void)rt;
      return bval_int(bval_to_int(&args[0]) + bval_to_int(&args[1]));
  }

  /* ======================== */
  /*  Registration Tables     */
  /* ======================== */

  static const FunctionEntry mymod_functions[] = {
      { "MYFUNC1", KW_COUNT, FCAT_USER, FRET_FLOAT,
        1, 1, FSAFE_PURE, 0, fn_myfunc1,
        "Double a number" },
      { "MYFUNC2", KW_COUNT, FCAT_USER, FRET_INT,
        2, 2, FSAFE_PURE, 0, fn_myfunc2,
        "Add two numbers" },
  };

  #define MYMOD_FUNC_COUNT 2

  /* ======================== */
  /*  Module Init/Cleanup     */
  /* ======================== */

  static int mymod_init(void *rt)
  {
      int i;
      (void)rt;
      for (i = 0; i < MYMOD_FUNC_COUNT; i++)
          funcreg_register(&mymod_functions[i]);
      return 0;
  }

  static void mymod_cleanup(void)
  {
      /* Release resources if any */
  }

  /* ======================== */
  /*  Module Descriptor       */
  /* ======================== */

  static const ModuleInfo mymod_info = {
      "MYMOD",                       /* name */
      "1.0",                         /* version */
      "My custom extension module",  /* description */
      MOD_LIBRARY,                   /* class */
      CAP_MATH,                      /* capabilities */
      SEC_COUNT,                     /* unpinned */
      mymod_init,                    /* init */
      mymod_cleanup                  /* cleanup */
  };

  void mod_mymod_register(void)
  {
      module_register(&mymod_info);
  }

INTEGRATION IN main.c:

  #include "mod_mymod.h"

  /* In main(), after module_system_init(): */
  mod_mymod_register();
  module_activate("MYMOD", rt);

USAGE FROM BASIC:

  MODULE "MYMOD"                   ' Activate (if not auto-activated)
  PRINT MYFUNC1(21)                ' 42
  PRINT MYFUNC2(10, 32)            ' 42


=====================================================================
15. ADDING NEW STATEMENTS (VMHandler)
=====================================================================

In addition to functions (which return values), you can add
new statements (which perform actions):

  Statement handler signature:

    void handler(Lexer *lex, void *rt, int line_num);

  Example: Add a BEEPN statement that beeps N times:

    #include "lexer.h"
    #include "vdev.h"

    static void parse_beepn(Lexer *lex, void *rt, int line_num)
    {
        int n, i;
        BValue val;
        (void)rt;

        /* Parse the argument */
        val = expr_evaluate(lex, rt, line_num);
        n = (int)bval_to_int(&val);
        if (n < 1) n = 1;
        if (n > 10) n = 10;

        for (i = 0; i < n; i++) {
            vdev_beep();
            vdev_sleep(200);
        }
    }

  Registration requires adding a keyword to the lexer and
  an opcode to the VM dispatch table.  This is more involved
  than adding a function — see the source code for examples
  of how existing statements are registered.


=====================================================================
16. ADDING NEW VDEV DEVICES
=====================================================================

A module can register new virtual devices by combining the
module system with the VDev system:

  static int mydev_init(void *rt)
  {
      VDev dev;
      (void)rt;
      memset(&dev, 0, sizeof(dev));
      dev.name     = "MYDEV:";
      dev.dev_putc = mydev_putc;
      dev.dev_puts = mydev_puts;
      dev.dev_getc = mydev_getc;
      vdev_register(&dev);
      return 0;
  }

  static const ModuleInfo mydev_module = {
      "MYDEV", "1.0",
      "Custom device driver",
      MOD_DEVICE,
      CAP_IO,
      mydev_init,
      NULL
  };

See R_Virtual_Devices.txt for complete VDev tutorials.


=====================================================================
17. DIALECT-OVERRIDABLE FUNCTIONS
=====================================================================

Some functions behave differently across BASIC dialects.
The registry supports this via the overridable flag:

  entry.overridable = 1;   /* Dialects CAN replace this */
  entry.overridable = 0;   /* Core function - cannot replace */

A dialect module can replace a function's handler:

  funcreg_override(KW_CHR, my_custom_chr_handler);

This replaces the CHR$() implementation for the active
dialect without modifying the original code.

Rules:
  - Only functions with overridable=1 can be overridden
  - The replacement handler must match the original signature
  - Override persists until the dialect is deactivated
  - Core functions (overridable=0) cannot be replaced


=====================================================================
18. BUILD INTEGRATION (MAKEFILE)
=====================================================================

To add a new module to the build:

1. Add the source file to the Makefile:

  # In the SOURCES variable:
  SOURCES = main.c lexer.c parser.c ... mod_mymod.c

2. Add the object file:

  OBJECTS = main.obj lexer.obj ... mod_mymod.obj

3. The module is now compiled and linked automatically.

For MSVC (Windows):
  cl /O2 /Fe:basicpp.exe *.c

For GCC (Linux):
  gcc -O2 -o basicpp *.c -lm

For Clang (macOS):
  clang -O2 -o basicpp *.c -lm

Since all .c files in the directory are compiled, simply
adding your mod_mymod.c file to the basicpp directory is
sufficient — the wildcard *.c picks it up.


=====================================================================

          PART IV: ACCESSING OS-SPECIFIC FEATURES SAFELY

=====================================================================


=====================================================================
19. THE SAFETY PHILOSOPHY
=====================================================================

BASIC++ provides access to OS-specific features through
C modules.  This is powerful but dangerous:

  POWER: Your module can do anything C can do — read
  hardware registers, call Win32 APIs, access /dev/,
  spawn processes, open network connections.

  DANGER: A module with full access can delete files,
  exfiltrate data, crash the system, or worse.

BASIC++ handles this tension with a three-layer safety model:

  Layer 1: CAPABILITY DECLARATIONS
    Every module declares what it needs (CAP_FILE,
    CAP_SYSTEM, etc.).  The user can see this BEFORE
    activating the module.

  Layer 2: SECURITY GATE
    The security system checks capabilities against the
    active security level before allowing activation.

  Layer 3: FUNCTION SAFETY LEVELS
    Individual functions within a module have safety
    classifications (FSAFE_PURE through FSAFE_SYSTEM)
    that control per-function access.

The key principle: MODULES DECLARE, THE SYSTEM ENFORCES.

A module author cannot bypass security.  A user running
at SEC_RESTRICTED cannot activate a module that requires
CAP_FILE.  Period.


=====================================================================
20. THE CAPABILITY GATE
=====================================================================

When a user types MODULE "SERIAL", this is what happens:

  1. module_activate("SERIAL", rt) is called
  2. The module's capabilities are read: CAP_IO | CAP_SYSTEM
  3. security_module_allowed(CAP_IO | CAP_SYSTEM) is called
  4. The permission matrix is consulted:

     SEC_OPEN:
       All capabilities allowed.  Module activates.

     SEC_STANDARD:
       CAP_SYSTEM is checked.  DENIED.
       "Module 'SERIAL' blocked by security level STANDARD."

     SEC_RESTRICTED:
       CAP_IO is checked.  DENIED.
       "Module 'SERIAL' blocked by security level RESTRICTED."

  5. If allowed, the module's init() callback is called.
     If denied, activation fails and the module stays inactive.

The gate logic (from security.c):

  int security_module_allowed(unsigned int caps)
  {
      if (current_level == SEC_OPEN) return 1;

      if (current_level == SEC_RESTRICTED) {
          if (caps & (CAP_IO | CAP_FILE | CAP_SYSTEM |
                      CAP_GRAPHICS | CAP_SOUND |
                      CAP_NETWORK))
              return 0;
          return 1;  /* only CAP_MATH | CAP_STRING */
      }

      if (current_level == SEC_STANDARD) {
          if (caps & CAP_SYSTEM) return 0;
          return 1;
      }

      return 1;
  }


=====================================================================
21. PLATFORM DETECTION (#ifdef GUARDS)
=====================================================================

OS-specific code MUST be wrapped in preprocessor guards
to ensure the module compiles on all platforms:

  Windows detection:
    #if defined(_WIN32) || defined(_WIN64)
    /* Windows-specific code */
    #endif

  Linux detection:
    #if defined(__linux__)
    /* Linux-specific code */
    #endif

  FreeDOS / DOS detection:
    #if defined(__MSDOS__) || defined(__DOS__) || defined(MSDOS)
    /* DOS-specific code */
    #endif

  POSIX (macOS, BSD, Linux):
    #if defined(__unix__) || defined(__APPLE__)
    /* POSIX-specific code */
    #endif

PATTERN: Provide a fallback for unsupported platforms:

  static BValue fn_clipboard_get(BValue *args, int argc, void *rt)
  {
      (void)args; (void)argc; (void)rt;
  #if defined(_WIN32)
      /* Windows clipboard access */
      ...
      return bval_string(text, len);
  #elif defined(__linux__)
      /* Linux: try xclip */
      ...
      return bval_string(text, len);
  #else
      /* Unsupported platform */
      return bval_string("", 0);
  #endif
  }

PATTERN: Return an error on unsupported platforms:

  static int mymod_init(void *rt)
  {
      (void)rt;
  #if defined(_WIN32)
      /* Register Windows-specific functions */
      register_win32_functions();
      return 0;
  #else
      /* Not available on this platform */
      printf("Module WINAPI requires Windows.\n");
      return -1;  /* init failure */
  #endif
  }


=====================================================================
22. WINDOWS-SPECIFIC FEATURES
=====================================================================


---------------------------------------------------------------------
22.1  Registry Access
---------------------------------------------------------------------

  Read and write Windows Registry values from BASIC++.

  SECURITY: Requires CAP_SYSTEM.  Blocked at SEC_STANDARD+.

    #if defined(_WIN32)
    #include <windows.h>

    static BValue fn_regread(BValue *args, int argc, void *rt)
    {
        char key_path[512], value_name[256], result[1024];
        HKEY hKey;
        DWORD type, size = sizeof(result);
        RuntimeState *state = (RuntimeState *)rt;
        char *buf;
        (void)argc;

        /* args[0] = key path, args[1] = value name */
        bval_to_string_buf(&args[0], key_path, sizeof(key_path));
        bval_to_string_buf(&args[1], value_name, sizeof(value_name));

        if (RegOpenKeyExA(HKEY_CURRENT_USER, key_path,
                          0, KEY_READ, &hKey) != ERROR_SUCCESS)
            return bval_string("", 0);

        if (RegQueryValueExA(hKey, value_name, NULL,
                             &type, (BYTE*)result,
                             &size) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return bval_string("", 0);
        }
        RegCloseKey(hKey);

        /* Allocate in string pool and return */
        buf = stringpool_alloc(&state->strpool, (int)size);
        if (buf) memcpy(buf, result, size);
        return bval_string(buf ? buf : "", buf ? (int)size : 0);
    }
    #endif

  BASIC usage:
    V$ = REGREAD$("Software\BASIC++", "InstallPath")


---------------------------------------------------------------------
22.2  Clipboard Access
---------------------------------------------------------------------

  SECURITY: Requires CAP_IO.

    #if defined(_WIN32)
    #include <windows.h>

    static BValue fn_clipget(BValue *args, int argc, void *rt)
    {
        HANDLE hData;
        char *text;
        int len;
        RuntimeState *state = (RuntimeState *)rt;
        char *buf;
        (void)args; (void)argc;

        if (!OpenClipboard(NULL)) return bval_string("", 0);
        hData = GetClipboardData(CF_TEXT);
        if (!hData) { CloseClipboard(); return bval_string("", 0); }
        text = (char *)GlobalLock(hData);
        if (!text)  { CloseClipboard(); return bval_string("", 0); }

        len = (int)strlen(text);
        buf = stringpool_alloc(&state->strpool, len);
        if (buf) memcpy(buf, text, len);

        GlobalUnlock(hData);
        CloseClipboard();
        return bval_string(buf ? buf : "", buf ? len : 0);
    }

    static BValue fn_clipset(BValue *args, int argc, void *rt)
    {
        HGLOBAL hMem;
        char *mem;
        int len;
        (void)argc; (void)rt;

        if (!bval_is_string(&args[0])) return bval_int(0);
        len = args[0].v.sval.length;

        if (!OpenClipboard(NULL)) return bval_int(0);
        EmptyClipboard();
        hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
        if (!hMem) { CloseClipboard(); return bval_int(0); }
        mem = (char *)GlobalLock(hMem);
        memcpy(mem, args[0].v.sval.data, len);
        mem[len] = '\0';
        GlobalUnlock(hMem);
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();
        return bval_int(-1);  /* success = TRUE */
    }
    #endif


---------------------------------------------------------------------
22.3  Process Management
---------------------------------------------------------------------

  SECURITY: Requires CAP_SYSTEM.

    #if defined(_WIN32)
    static BValue fn_shellexec(BValue *args, int argc, void *rt)
    {
        char cmd[1024];
        (void)argc; (void)rt;
        bval_to_string_buf(&args[0], cmd, sizeof(cmd));
        return bval_int(system(cmd));
    }

    static BValue fn_getpid(BValue *args, int argc, void *rt)
    {
        (void)args; (void)argc; (void)rt;
        return bval_int((long)GetCurrentProcessId());
    }
    #endif


---------------------------------------------------------------------
22.4  Windows Services
---------------------------------------------------------------------

  Access Windows multimedia, power management, etc.

    #if defined(_WIN32)
    #include <mmsystem.h>
    #pragma comment(lib, "winmm.lib")

    static BValue fn_playsound(BValue *args, int argc, void *rt)
    {
        char path[512];
        (void)argc; (void)rt;
        bval_to_string_buf(&args[0], path, sizeof(path));
        PlaySoundA(path, NULL, SND_FILENAME | SND_ASYNC);
        return bval_int(-1);
    }

    static BValue fn_stopsound(BValue *args, int argc, void *rt)
    {
        (void)args; (void)argc; (void)rt;
        PlaySoundA(NULL, NULL, 0);
        return bval_int(-1);
    }
    #endif


---------------------------------------------------------------------
22.5  COM/OLE Automation
---------------------------------------------------------------------

  Accessing COM objects (Excel, Word, etc.) requires
  CoInitialize + IDispatch, which goes beyond C89.
  This is best done via an external DLL wrapper that
  the module calls through LoadLibrary/GetProcAddress.

  This is advanced and platform-specific.  A full
  COM module would be a significant undertaking.


=====================================================================
23. LINUX-SPECIFIC FEATURES
=====================================================================


---------------------------------------------------------------------
23.1  POSIX File Operations
---------------------------------------------------------------------

  SECURITY: Requires CAP_FILE.

    #if defined(__linux__) || defined(__unix__)
    #include <sys/stat.h>
    #include <unistd.h>

    static BValue fn_fileexists(BValue *args, int argc, void *rt)
    {
        char path[512];
        struct stat st;
        (void)argc; (void)rt;
        bval_to_string_buf(&args[0], path, sizeof(path));
        return bval_int(stat(path, &st) == 0 ? -1 : 0);
    }

    static BValue fn_filesize(BValue *args, int argc, void *rt)
    {
        char path[512];
        struct stat st;
        (void)argc; (void)rt;
        bval_to_string_buf(&args[0], path, sizeof(path));
        if (stat(path, &st) != 0) return bval_int(0);
        return bval_int((long)st.st_size);
    }

    static BValue fn_chmod(BValue *args, int argc, void *rt)
    {
        char path[512];
        int mode;
        (void)rt;
        bval_to_string_buf(&args[0], path, sizeof(path));
        mode = (int)bval_to_int(&args[1]);
        return bval_int(chmod(path, (mode_t)mode) == 0 ? -1 : 0);
    }
    #endif


---------------------------------------------------------------------
23.2  Process Forking and Pipes
---------------------------------------------------------------------

  SECURITY: Requires CAP_SYSTEM.

    #if defined(__linux__) || defined(__unix__)
    #include <stdio.h>

    static BValue fn_popen_read(BValue *args, int argc, void *rt)
    {
        char cmd[1024], result[4096];
        FILE *pp;
        int total = 0;
        RuntimeState *state = (RuntimeState *)rt;
        char *buf;
        (void)argc;

        bval_to_string_buf(&args[0], cmd, sizeof(cmd));
        pp = popen(cmd, "r");
        if (!pp) return bval_string("", 0);

        while (fgets(result + total, sizeof(result) - total, pp)) {
            total = (int)strlen(result);
            if (total >= (int)sizeof(result) - 1) break;
        }
        pclose(pp);

        buf = stringpool_alloc(&state->strpool, total);
        if (buf) memcpy(buf, result, total);
        return bval_string(buf ? buf : "", buf ? total : 0);
    }
    #endif

  BASIC usage:
    FILES$ = POPEN_READ$("ls -la /home")
    PRINT FILES$


---------------------------------------------------------------------
23.3  Signal Handling
---------------------------------------------------------------------

  SECURITY: Requires CAP_SYSTEM.

    #if defined(__linux__)
    #include <signal.h>

    static volatile int signal_caught = 0;

    static void signal_handler(int sig)
    {
        signal_caught = sig;
    }

    static BValue fn_trap_signal(BValue *args, int argc, void *rt)
    {
        int sig = (int)bval_to_int(&args[0]);
        (void)argc; (void)rt;
        signal(sig, signal_handler);
        return bval_int(-1);
    }

    static BValue fn_check_signal(BValue *args, int argc, void *rt)
    {
        int result;
        (void)args; (void)argc; (void)rt;
        result = signal_caught;
        signal_caught = 0;
        return bval_int(result);
    }
    #endif


---------------------------------------------------------------------
23.4  Syslog Integration
---------------------------------------------------------------------

  SECURITY: Requires CAP_IO.

    #if defined(__linux__)
    #include <syslog.h>

    static BValue fn_syslog(BValue *args, int argc, void *rt)
    {
        char msg[1024];
        int priority = LOG_INFO;
        (void)rt;
        bval_to_string_buf(&args[0], msg, sizeof(msg));
        if (argc >= 2)
            priority = (int)bval_to_int(&args[1]);
        openlog("basicpp", LOG_PID, LOG_USER);
        syslog(priority, "%s", msg);
        closelog();
        return bval_int(-1);
    }
    #endif


---------------------------------------------------------------------
23.5  GPIO and Hardware (Embedded)
---------------------------------------------------------------------

  SECURITY: Requires CAP_SYSTEM | CAP_IO.

    #if defined(__linux__)
    static BValue fn_gpio_write(BValue *args, int argc, void *rt)
    {
        int pin, value;
        char path[256];
        FILE *fp;
        (void)rt; (void)argc;

        pin   = (int)bval_to_int(&args[0]);
        value = (int)bval_to_int(&args[1]);

        /* Export pin */
        fp = fopen("/sys/class/gpio/export", "w");
        if (fp) { fprintf(fp, "%d", pin); fclose(fp); }

        /* Set direction */
        sprintf(path, "/sys/class/gpio/gpio%d/direction", pin);
        fp = fopen(path, "w");
        if (fp) { fputs("out", fp); fclose(fp); }

        /* Write value */
        sprintf(path, "/sys/class/gpio/gpio%d/value", pin);
        fp = fopen(path, "w");
        if (fp) {
            fprintf(fp, "%d", value ? 1 : 0);
            fclose(fp);
        }

        return bval_int(-1);
    }
    #endif

  BASIC usage:
    GPIO_WRITE 17, 1        ' Turn on GPIO pin 17
    DELAY 1000               ' Wait 1 second
    GPIO_WRITE 17, 0        ' Turn off


=====================================================================
24. FREEDOS-SPECIFIC FEATURES
=====================================================================


---------------------------------------------------------------------
24.1  BIOS Interrupts
---------------------------------------------------------------------

  On FreeDOS with OpenWatcom, you can call BIOS interrupts:

    #if defined(__MSDOS__) || defined(__DOS__)
    #include <dos.h>
    #include <i86.h>

    static BValue fn_bios_vidmode(BValue *args, int argc, void *rt)
    {
        union REGS regs;
        (void)args; (void)argc; (void)rt;
        regs.h.ah = 0x0F;       /* Get video mode */
        int86(0x10, &regs, &regs);
        return bval_int(regs.h.al);
    }

    static BValue fn_bios_keyscan(BValue *args, int argc, void *rt)
    {
        union REGS regs;
        (void)args; (void)argc; (void)rt;
        regs.h.ah = 0x01;       /* Check keystroke */
        int86(0x16, &regs, &regs);
        if (regs.x.flags & 0x40)  /* ZF set = no key */
            return bval_int(0);
        return bval_int(regs.x.ax);
    }
    #endif


---------------------------------------------------------------------
24.2  Direct Port I/O
---------------------------------------------------------------------

    #if defined(__MSDOS__) || defined(__DOS__)
    #include <conio.h>

    static BValue fn_port_in(BValue *args, int argc, void *rt)
    {
        int port = (int)bval_to_int(&args[0]);
        (void)argc; (void)rt;
        return bval_int(inp(port));
    }

    static BValue fn_port_out(BValue *args, int argc, void *rt)
    {
        int port  = (int)bval_to_int(&args[0]);
        int value = (int)bval_to_int(&args[1]);
        (void)rt;
        outp(port, value);
        return bval_int(0);
    }
    #endif


---------------------------------------------------------------------
24.3  Real-Mode Memory Access
---------------------------------------------------------------------

    #if defined(__MSDOS__) || defined(__DOS__)
    #include <dos.h>

    static BValue fn_farpeek(BValue *args, int argc, void *rt)
    {
        unsigned int seg, ofs;
        unsigned char far *ptr;
        (void)argc; (void)rt;
        seg = (unsigned int)bval_to_int(&args[0]);
        ofs = (unsigned int)bval_to_int(&args[1]);
        ptr = MK_FP(seg, ofs);
        return bval_int(*ptr);
    }

    static BValue fn_farpoke(BValue *args, int argc, void *rt)
    {
        unsigned int seg, ofs;
        unsigned char far *ptr;
        (void)rt;
        seg = (unsigned int)bval_to_int(&args[0]);
        ofs = (unsigned int)bval_to_int(&args[1]);
        ptr = MK_FP(seg, ofs);
        *ptr = (unsigned char)bval_to_int(&args[2]);
        return bval_int(0);
    }
    #endif


---------------------------------------------------------------------
24.4  TSR (Terminate-Stay-Resident)
---------------------------------------------------------------------

  TSR programming is complex and requires interrupt hooking.
  This is beyond the scope of a simple module but can be
  implemented by a module that:
  1. Hooks INT 08h (timer) or INT 09h (keyboard)
  2. Saves the interpreter state
  3. Calls _dos_keep() to remain resident

  This is advanced DOS programming and requires intimate
  knowledge of real-mode memory management.


=====================================================================
25. CROSS-PLATFORM PATTERNS
=====================================================================

PATTERN 1: Platform-switching function

  static BValue fn_hostname(BValue *args, int argc, void *rt)
  {
      char name[256] = "";
      RuntimeState *state = (RuntimeState *)rt;
      char *buf;
      int len;
      (void)args; (void)argc;

  #if defined(_WIN32)
      {
          DWORD size = sizeof(name);
          GetComputerNameA(name, &size);
      }
  #elif defined(__linux__) || defined(__unix__)
      gethostname(name, sizeof(name));
  #else
      strcpy(name, "UNKNOWN");
  #endif

      len = (int)strlen(name);
      buf = stringpool_alloc(&state->strpool, len);
      if (buf) memcpy(buf, name, len);
      return bval_string(buf ? buf : "", buf ? len : 0);
  }

PATTERN 2: Feature availability check

  static BValue fn_has_feature(BValue *args, int argc, void *rt)
  {
      char feature[64];
      (void)argc; (void)rt;
      bval_to_string_buf(&args[0], feature, sizeof(feature));

  #if defined(_WIN32)
      if (strcmp(feature, "CLIPBOARD") == 0) return bval_int(-1);
      if (strcmp(feature, "REGISTRY") == 0)  return bval_int(-1);
      if (strcmp(feature, "PLAYSOUND") == 0) return bval_int(-1);
  #elif defined(__linux__)
      if (strcmp(feature, "SYSLOG") == 0)    return bval_int(-1);
      if (strcmp(feature, "GPIO") == 0)      return bval_int(-1);
      if (strcmp(feature, "POPEN") == 0)     return bval_int(-1);
  #endif
      return bval_int(0);  /* feature not available */
  }

  BASIC usage:
    IF HAS_FEATURE("CLIPBOARD") THEN
      CLIPSET "Hello from BASIC++"
    ELSE
      PRINT "Clipboard not supported on this platform"
    END IF

PATTERN 3: Unified API with platform backends

  /* Public API - same on all platforms */
  BValue fn_get_temp_dir(BValue *args, int argc, void *rt);
  BValue fn_get_home_dir(BValue *args, int argc, void *rt);
  BValue fn_get_username(BValue *args, int argc, void *rt);

  /* Implementation switches by platform internally */
  static BValue fn_get_temp_dir(BValue *args, int argc, void *rt)
  {
      char path[512] = "";
      RuntimeState *state = (RuntimeState *)rt;
      char *buf;
      int len;
      (void)args; (void)argc;

  #if defined(_WIN32)
      GetTempPathA(sizeof(path), path);
  #elif defined(__linux__)
      strcpy(path, getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
  #else
      strcpy(path, ".");
  #endif

      len = (int)strlen(path);
      buf = stringpool_alloc(&state->strpool, len);
      if (buf) memcpy(buf, path, len);
      return bval_string(buf ? buf : "", buf ? len : 0);
  }


=====================================================================

              PART V: SECURITY IN DEPTH

=====================================================================


=====================================================================
26. THE SECURITY MODEL FOR MODULES
=====================================================================

The security model has three principles:

  1. DECLARE BEFORE USE
     Every module declares its capabilities in its
     ModuleInfo descriptor.  This cannot be changed
     after registration.

  2. GATE BEFORE INIT
     The security system checks capabilities BEFORE
     calling the module's init() function.  If denied,
     init() never runs — no code executes.

  3. TRUST THE LEVEL
     The security level is set by the USER (or system
     administrator), not by the module.  A module cannot
     escalate its own privileges.

Timeline:
  Boot -> security_init(SEC_STANDARD) -> ...
  User -> MODULE "DANGEROUS_MOD"
  System -> security_module_allowed(CAP_SYSTEM)
  System -> DENIED -> module stays inactive
  User -> "Module 'DANGEROUS_MOD' blocked by security
           level STANDARD."


=====================================================================
27. THE PERMISSION MATRIX
=====================================================================

  Capability       SEC_OPEN   SEC_STANDARD   SEC_RESTRICTED
  ----------       --------   ------------   --------------
  CAP_MATH          ALLOW       ALLOW           ALLOW
  CAP_STRING        ALLOW       ALLOW           ALLOW
  CAP_IO            ALLOW       ALLOW           DENY
  CAP_FILE          ALLOW       ALLOW           DENY
  CAP_SYSTEM        ALLOW       DENY            DENY
  CAP_GRAPHICS      ALLOW       ALLOW           DENY
  CAP_SOUND         ALLOW       ALLOW           DENY
  CAP_NETWORK       ALLOW       ALLOW           DENY

Read: At SEC_STANDARD, a module requiring CAP_SYSTEM
will be DENIED.  At SEC_RESTRICTED, only pure math and
string modules are allowed.


=====================================================================
28. SECURITY LEVELS AND MODULE CAPABILITIES
=====================================================================

How to choose capability flags for your module:

  If your module...                  Declare...
  ----------------                   ----------
  Only does math                     CAP_MATH
  Only manipulates strings           CAP_STRING
  Reads/writes console               CAP_IO
  Reads/writes files                 CAP_FILE
  Calls OS APIs (registry, etc.)     CAP_SYSTEM
  Uses graphics framebuffer          CAP_GRAPHICS
  Plays audio                        CAP_SOUND
  Opens network connections          CAP_NETWORK

  Multiple capabilities: use bitwise OR:
    CAP_IO | CAP_FILE | CAP_NETWORK

  RULE: Declare the MINIMUM capabilities your module
  actually needs.  Don't declare CAP_SYSTEM if you only
  need CAP_FILE.  Over-declaring reduces the security
  levels at which your module can be activated.


=====================================================================
29. HOW module_activate() ENFORCES SECURITY
=====================================================================

  The full activation sequence:

  1. Find module by name (case-insensitive search)
  2. Check if already active (idempotent - return success)
  3. Read module's capabilities bitfield
  4. Call security_module_allowed(capabilities)
  5. If denied:
     - Print "Module 'X' blocked by security level Y."
     - Return -1 (activation fails)
     - Module stays inactive
     - No init() callback is called
     - No functions are registered
  6. If allowed:
     - Call module's init() callback
     - If init() returns non-zero:
       - Print "Module 'X' init failed."
       - Return -1
     - If init() returns 0:
       - Mark module as active
       - Return 0 (success)

  CRITICAL: The init() callback only runs if the security
  check passes.  This means a CAP_SYSTEM module's init()
  function — which might call dangerous OS APIs — is never
  even entered at SEC_STANDARD or SEC_RESTRICTED.


=====================================================================
30. WRITING SECURE MODULES (BEST PRACTICES)
=====================================================================

  1. DECLARE HONESTLY
     Set capability flags that accurately reflect what
     your module does.  Don't understate to bypass security.

  2. VALIDATE ALL INPUTS
     Never trust string arguments from BASIC.  Always
     bounds-check, null-check, and sanitize:

       char buf[256];
       bval_to_string_buf(&args[0], buf, sizeof(buf));
       /* buf is now guaranteed null-terminated and <=255 chars */

  3. USE SAFE BUFFER SIZES
     Never use unbounded sprintf.  Always use snprintf
     or fixed-size buffers:

       /* WRONG: buffer overflow risk */
       char buf[64];
       sprintf(buf, "Long string: %s", user_input);

       /* RIGHT: bounded */
       char buf[64];
       snprintf(buf, sizeof(buf), "Long string: %s", user_input);

     Note: snprintf is C99.  For strict C89, use truncation:
       char buf[64];
       strncpy(buf, user_input, sizeof(buf) - 1);
       buf[sizeof(buf) - 1] = '\0';

  4. LIMIT RESOURCE USAGE
     Don't allocate huge amounts of memory.  Don't open
     hundreds of file handles.  Don't run infinite loops
     in the init() callback.

  5. CLEAN UP ON DEACTIVATION
     If your init() opens handles, allocates memory, or
     registers callbacks, your cleanup() MUST release them:

       static void mymod_cleanup(void)
       {
           if (serial_handle) CloseHandle(serial_handle);
           serial_handle = NULL;
       }

  6. DON'T STORE SENSITIVE DATA
     Don't store passwords, API keys, or tokens in global
     variables.  They persist in memory and could be read
     via PEEK.

  7. LOG SECURITY-RELEVANT ACTIONS
     If your module does something dangerous (deletes files,
     opens network connections), log it so the user can audit.

  8. FAIL SAFELY
     On error, return a safe default (empty string, 0)
     rather than crashing or returning garbage:

       if (fopen(...) == NULL)
           return bval_string("", 0);  /* safe default */


=====================================================================
31. INPUT VALIDATION AND SANITIZATION
=====================================================================

  String arguments from BASIC programs are untrusted.
  Always validate:

  PATH VALIDATION:
    char path[256];
    bval_to_string_buf(&args[0], path, sizeof(path));

    /* Reject path traversal */
    if (strstr(path, "..")) {
        error_raise(ERR_HOW, line_num);
        return bval_int(0);
    }

    /* Reject absolute paths (if you want relative only) */
    if (path[0] == '/' || path[0] == '\\' ||
        (path[1] == ':' && path[2] == '\\')) {
        error_raise(ERR_HOW, line_num);
        return bval_int(0);
    }

  NUMERIC RANGE VALIDATION:
    int port = (int)bval_to_int(&args[0]);
    if (port < 1 || port > 65535) {
        error_raise(ERR_HOW, line_num);
        return bval_int(0);
    }

  STRING LENGTH VALIDATION:
    if (!bval_is_string(&args[0]) ||
        args[0].v.sval.length > 255) {
        error_raise(ERR_HOW, line_num);
        return bval_string("", 0);
    }


=====================================================================
32. PRINCIPLE OF LEAST PRIVILEGE
=====================================================================

  When designing a module, ask: "What is the MINIMUM
  access this module needs?"

  BAD:  A calculator module that declares CAP_FILE |
        CAP_SYSTEM because "maybe we'll add features later."

  GOOD: A calculator module that declares CAP_MATH only.
        It can be used even at SEC_RESTRICTED.

  BAD:  A file utility module that declares CAP_SYSTEM
        because one function calls system("ls").

  GOOD: Split the module into two:
        - mod_fileutil.c  (CAP_FILE) — file size, exists, etc.
        - mod_shell.c     (CAP_SYSTEM) — system commands

  This way, the file utility functions work at SEC_STANDARD
  while the shell functions are correctly restricted.


=====================================================================
33. AUDITING AND LOGGING
=====================================================================

  For sensitive operations, log what happened:

    static BValue fn_delete_file(BValue *args, int argc, void *rt)
    {
        char path[256];
        (void)argc; (void)rt;
        bval_to_string_buf(&args[0], path, sizeof(path));

        /* LOG the operation */
        fprintf(stderr, "[AUDIT] DELETE FILE: %s\n", path);

        /* Perform the operation */
        if (remove(path) != 0) return bval_int(0);
        return bval_int(-1);
    }

  The audit log goes to stderr (ERR: device), which can
  be redirected to a file:

    basicpp myprog.bas 2> audit.log


=====================================================================

          PART VI: COMPLETE MODULE EXAMPLES

=====================================================================


=====================================================================
34. EXAMPLE: MATH EXTENSIONS MODULE
=====================================================================

  /* mod_mathext.c - Extended math functions for BASIC++ */

  #include <math.h>
  #include <string.h>
  #include "module.h"
  #include "funcreg.h"
  #include "value.h"

  static BValue fn_cube(BValue *a, int c, void *r)
  { (void)c;(void)r; double x=bval_to_float(&a[0]); return bval_float(x*x*x); }

  static BValue fn_hypot(BValue *a, int c, void *r)
  { (void)c;(void)r; return bval_float(sqrt(
      bval_to_float(&a[0])*bval_to_float(&a[0]) +
      bval_to_float(&a[1])*bval_to_float(&a[1]))); }

  static BValue fn_clamp(BValue *a, int c, void *r)
  { double x,lo,hi; (void)c;(void)r;
    x=bval_to_float(&a[0]); lo=bval_to_float(&a[1]); hi=bval_to_float(&a[2]);
    if(x<lo)return bval_float(lo); if(x>hi)return bval_float(hi);
    return bval_float(x); }

  static BValue fn_lerp(BValue *a, int c, void *r)
  { (void)c;(void)r; double t=bval_to_float(&a[0]);
    return bval_float(bval_to_float(&a[1])*(1-t)+bval_to_float(&a[2])*t); }

  static BValue fn_deg2rad(BValue *a, int c, void *r)
  { (void)c;(void)r; return bval_float(bval_to_float(&a[0])*3.14159265358979/180.0); }

  static BValue fn_rad2deg(BValue *a, int c, void *r)
  { (void)c;(void)r; return bval_float(bval_to_float(&a[0])*180.0/3.14159265358979); }

  static BValue fn_log10(BValue *a, int c, void *r)
  { (void)c;(void)r; return bval_float(log10(bval_to_float(&a[0]))); }

  static BValue fn_log2(BValue *a, int c, void *r)
  { (void)c;(void)r; return bval_float(log(bval_to_float(&a[0]))/log(2.0)); }

  static BValue fn_pow(BValue *a, int c, void *r)
  { (void)c;(void)r; return bval_float(pow(bval_to_float(&a[0]),bval_to_float(&a[1]))); }

  static BValue fn_ceil(BValue *a, int c, void *r)
  { (void)c;(void)r; return bval_float(ceil(bval_to_float(&a[0]))); }

  static BValue fn_floor(BValue *a, int c, void *r)
  { (void)c;(void)r; return bval_float(floor(bval_to_float(&a[0]))); }

  static BValue fn_round(BValue *a, int c, void *r)
  { (void)c;(void)r; return bval_float(floor(bval_to_float(&a[0])+0.5)); }

  static const FunctionEntry mathext_funcs[] = {
    {"CUBE",    KW_COUNT, FCAT_MATH, FRET_FLOAT, 1,1, FSAFE_PURE, 0, fn_cube,    "x^3"},
    {"HYPOT",   KW_COUNT, FCAT_MATH, FRET_FLOAT, 2,2, FSAFE_PURE, 0, fn_hypot,   "Hypotenuse"},
    {"CLAMP",   KW_COUNT, FCAT_MATH, FRET_FLOAT, 3,3, FSAFE_PURE, 0, fn_clamp,   "Clamp to range"},
    {"LERP",    KW_COUNT, FCAT_MATH, FRET_FLOAT, 3,3, FSAFE_PURE, 0, fn_lerp,    "Linear interpolate"},
    {"DEG2RAD", KW_COUNT, FCAT_MATH, FRET_FLOAT, 1,1, FSAFE_PURE, 0, fn_deg2rad, "Degrees to radians"},
    {"RAD2DEG", KW_COUNT, FCAT_MATH, FRET_FLOAT, 1,1, FSAFE_PURE, 0, fn_rad2deg, "Radians to degrees"},
    {"LOG10",   KW_COUNT, FCAT_MATH, FRET_FLOAT, 1,1, FSAFE_PURE, 0, fn_log10,   "Base-10 logarithm"},
    {"LOG2",    KW_COUNT, FCAT_MATH, FRET_FLOAT, 1,1, FSAFE_PURE, 0, fn_log2,    "Base-2 logarithm"},
    {"POW",     KW_COUNT, FCAT_MATH, FRET_FLOAT, 2,2, FSAFE_PURE, 0, fn_pow,     "x to the power y"},
    {"CEIL",    KW_COUNT, FCAT_MATH, FRET_FLOAT, 1,1, FSAFE_PURE, 0, fn_ceil,    "Round up"},
    {"FLOOR",   KW_COUNT, FCAT_MATH, FRET_FLOAT, 1,1, FSAFE_PURE, 0, fn_floor,   "Round down"},
    {"ROUND",   KW_COUNT, FCAT_MATH, FRET_FLOAT, 1,1, FSAFE_PURE, 0, fn_round,   "Round to nearest"},
  };

  static int mathext_init(void *rt)
  { int i; (void)rt;
    for(i=0;i<12;i++) funcreg_register(&mathext_funcs[i]);
    return 0; }

  static const ModuleInfo mathext_info = {
    "MATHEXT","1.0","Extended math functions",
    MOD_LIBRARY, CAP_MATH, SEC_COUNT, mathext_init, NULL };

  void mod_mathext_register(void)
  { module_register(&mathext_info); }

  BASIC usage:
    MODULE "MATHEXT"
    PRINT CUBE(5)           ' 125
    PRINT HYPOT(3, 4)       ' 5
    PRINT CLAMP(150, 0,100) ' 100
    PRINT DEG2RAD(180)      ' 3.14159...
    PRINT LOG10(1000)       ' 3
    PRINT POW(2, 10)        ' 1024


=====================================================================
35-40. ADDITIONAL MODULE EXAMPLES
=====================================================================

  The following modules follow the same pattern as section 34.
  For brevity, only the function tables and key handlers
  are shown.  See the complete examples earlier in this
  manual for the full module boilerplate.

  35. STRING UTILITIES (CAP_STRING):
      REVERSE$, REPEAT$, PADL$, PADR$, CENTER$,
      WORDCOUNT, WORD$, ISDIGIT, ISALPHA

  36. ENVIRONMENT AND OS INFO (CAP_IO):
      HOSTNAME$, USERNAME$, OSNAME$, TEMPDIR$,
      CPUCOUNT, TICKCOUNT, GETENV$, HAS_FEATURE

  37. FILE UTILITIES (CAP_FILE):
      FEXISTS, FSIZE, FDATE$, FCOPY, FMOVE,
      DIRLIST$, TEMPFILE$

  38. SERIAL PORT (CAP_IO | CAP_SYSTEM):
      SER_OPEN, SER_CLOSE, SER_SEND$, SER_RECV$
      (See R_Virtual_Devices.txt Section 7.1)

  39. NETWORK HTTP (CAP_NETWORK | CAP_IO):
      HTTPGET$, HTTPPOST$, URLENC$, URLDEC$
      (Requires Winsock2 on Windows, sockets on Linux)

  40. SQLITE DATABASE (CAP_FILE | CAP_IO):
      DBOPEN, DBCLOSE, DBEXEC, DBQUERY$, DBROW$
      (Requires SQLite3 library linked in)


=====================================================================

              PART VII: SELF-HOSTING SPECIFICATIONS

=====================================================================

BASIC++ can extend itself at runtime using declarative .BPPSPEC
specification files.  This is the most powerful and portable
extension mechanism — no C compilation required.

See also: Self_Hosting_Specs.md for the full architecture document.


=====================================================================
41. THE .BPPSPEC FILE FORMAT
=====================================================================

Specification files define new statements and functions using a
declarative block syntax:

  DEFINE SPECIFICATION "TURTLE"
      CATEGORY "STATEMENT"
      VERSION "1.0"
      SECURITY "SAFE"
      DEPENDS "GRAPHICS"
      LIB "turtle.lib"
  END SPECIFICATION

Directives:

  DEFINE SPECIFICATION "name"   Opens a spec block
  CATEGORY "type"               STATEMENT, FUNCTION, DIALECT,
                                MODULE, LIBRARY, DEVICE
  VERSION "x.y"                 Version string
  LIB "path.lib"                Companion library path
  SECURITY "level"              Required security level
  DEPENDS "name1,name2"         Comma-separated dependencies
  END SPECIFICATION             Closes block, registers keyword

Lines starting with # are comments.  Blank lines are ignored.
Only STATEMENT and FUNCTION categories trigger dynamic keyword
registration via keyword_register_custom().


=====================================================================
42. LOAD FEATURE COMMAND
=====================================================================

From the BASIC prompt or within a program:

  LOAD FEATURE "turtle.spec"

This command:
  1. Checks SECOP_EXT_LOAD permission
  2. Parses the .BPPSPEC file via spec_load_file()
  3. Registers new keywords dynamically (IDs 1000+)
  4. Loads the companion .LIB into library program space
  5. Verifies security pinning against current level

After loading, the new keyword is immediately available:

  LOAD FEATURE "turtle.spec"
  TURTLE FORWARD 100
  TURTLE TURNLEFT 90
  TURTLE PENUP

SPEC REGISTRY:

  The spec registry holds up to 128 loaded specifications.
  Each spec is stored as a SpecObject with:
    - name (keyword name)
    - version string
    - category (SpecCategory enum)
    - lib_path (companion library)
    - depends (dependency list)
    - required_level (security pin)
    - kw_id (assigned keyword ID)

PLUGIN AUTO-LOADING:

  Plugins can auto-load .spec files during activation.
  When a plugin directory contains .spec files, they are
  loaded automatically by ext_plugin.c.

CONFIG FILE ALIASES:

  The config file supports keyword aliasing:
    alias.MOSTRAR = PRINT
    alias.ENTRADA = INPUT

  These are registered at startup as global aliases.


=====================================================================
43. COMPANION .LIB DISPATCH
=====================================================================

The behavior of custom statements is implemented in companion
.LIB files — pure BASIC++ source files that define SUBs and
FUNCTIONs.  No native code required.

LIBRARY FORMAT:

  REM @LIBRARY TURTLE
  REM @VERSION 1.0
  REM @SECURITY SAFE

  SUB FORWARD(D)
      REM Move turtle forward D pixels
  END SUB

  SUB TURNLEFT(A)
      REM Turn turtle left A degrees
  END SUB

  SUB PENUP
      REM Lift the pen
  END SUB

DISPATCH FLOW:

  When the parser encounters a custom keyword (ID >= 1000),
  it calls pi_parse_custom_statement(), which:

  1. Looks up the SpecObject by keyword ID
  2. Checks security pinning (required_level)
  3. Calls lib_space_find_by_name(spec_name)
  4. Calls lib_space_invoke_by_name(lib, spec_name, rt)
  5. The library SUB executes in its isolated variable space

  If no companion library is loaded, the interpreter prints:
    SORRY? SPEC 'TURTLE' has no loaded companion library
           (expected: turtle.lib) in line N

SECURITY PINNING:

  Specs can declare a required security level:
    SECURITY "OPEN"
    SECURITY "SAFE"
    SECURITY "STANDARD"

  Checked at both load time and execution time.  If the
  current level doesn't match:
    SORRY? SPEC 'name' requires security level X
           (current: Y) in line N

  Omitting SECURITY makes the spec "unpinned" — it runs
  at any security level.

COMPLETE EXAMPLE:

  File: counter.spec
  ----
  DEFINE SPECIFICATION "COUNTER"
      CATEGORY "STATEMENT"
      VERSION "1.0"
      LIB "counter.lib"
  END SPECIFICATION
  ----

  File: counter.lib
  ----
  REM @LIBRARY COUNTER
  REM @VERSION 1.0

  SUB COUNTER(N)
      DIM I
      FOR I = 1 TO N
          PRINT I; " ";
      NEXT I
      PRINT
  END SUB
  ----

  Usage:
    LOAD FEATURE "counter.spec"
    COUNTER 10
    ' Output: 1  2  3  4  5  6  7  8  9  10


=====================================================================

              PART VIII: REFERENCE

=====================================================================


=====================================================================
44. API QUICK REFERENCE
=====================================================================

  FUNCTION REGISTRY:
    funcreg_init()                  Clear registry
    funcreg_register(&entry)        Add function
    funcreg_find_by_name("NAME")    Look up by name
    funcreg_find_by_keyword(kw)     Look up by keyword
    funcreg_override(kw, handler)   Replace handler
    funcreg_count()                 Count functions
    funcreg_get(index)              Get by index

  MODULE SYSTEM:
    module_system_init()            Clear module table
    module_register(&info)          Register module
    module_activate("NAME", rt)     Activate module
    module_deactivate("NAME")       Deactivate module
    module_is_active("NAME")        Check active
    module_find("NAME")             Find descriptor
    module_count()                  Count modules
    module_get(index)               Get by index

  BVALUE API:
    bval_int(n)                     Create integer
    bval_float(x)                   Create float
    bval_string(ptr, len)           Create string
    bval_to_int(&v)                 Convert to int
    bval_to_float(&v)               Convert to float
    bval_to_string_buf(&v,buf,sz)   Format to string
    bval_is_int(&v)                 Type check
    bval_is_float(&v)               Type check
    bval_is_string(&v)              Type check
    bval_is_numeric(&v)             Type check

  SECURITY:
    security_init(level)            Set initial level
    security_get_level()            Get current level
    security_set_level(level)       Change level
    security_check(op, line)        Check operation
    security_module_allowed(caps)   Check module caps


=====================================================================
45. MODULE CHECKLIST
=====================================================================

  Before releasing a module, verify:

  [ ] All functions have correct safety levels (FSAFE_*)
  [ ] Capability flags are minimal and accurate
  [ ] All platform-specific code has #ifdef guards
  [ ] init() returns -1 on unsupported platforms
  [ ] cleanup() releases all resources
  [ ] String returns use string pool (not malloc)
  [ ] All user inputs are validated and bounded
  [ ] No buffer overflows (snprintf or fixed buffers)
  [ ] Compiles cleanly with -Wall -Werror (GCC)
  [ ] Compiles cleanly with /W4 (MSVC)
  [ ] Works on at least 2 platforms (or gracefully fails)
  [ ] help_text is filled in for every function
  [ ] Module has been tested at each security level


=====================================================================
46. TROUBLESHOOTING
=====================================================================

  "Module 'X' not found."
    The module was never registered.  Ensure mod_X_register()
    is called in main.c before module_activate().

  "Module 'X' blocked by security level Y."
    The module's capabilities exceed the current security
    level.  Lower the security level or use a module with
    fewer capabilities.

  "Module 'X' init failed."
    The module's init() callback returned non-zero.
    Check if the module is available on the current platform.

  Function not recognized after MODULE command:
    The module's init() function must call funcreg_register()
    for each function.  Verify the registration code runs.

  Crash in function handler:
    Most common cause: using args[1] when argc is 1.
    Always check argc for optional arguments.
    Second most common: returning a string without using
    the string pool.


=====================================================================
47. RELATED MANUALS
=====================================================================

  B_Programmers_Guide.txt       Language reference
  K_Creating_Dialects.txt       Dialect modules
  L_Advanced_DEF.txt            DEF FN user functions
  Q_Security.txt                Security system reference
  R_Virtual_Devices.txt         VDev device drivers
  W_Virtual_Filesystem.txt      File access architecture
  Self_Hosting_Specs.md         Specification architecture


=====================================================================
END OF DOCUMENT
=====================================================================
