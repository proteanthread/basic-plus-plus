KEYWORD: MODIFYING BUILT-IN KEYWORD BEHAVIOR
===============================================
Version 4.2.3

The KEYWORD statement lets you change HOW a built-in keyword works
by setting named properties.  Unlike SCOPE (which controls access
and hooks subroutines around keywords), KEYWORD modifies the
keyword's internal behavior directly.

No subroutines are needed.  You set a property, and the keyword
immediately changes its behavior.  Remove the property, and it
goes back to normal.

Think of KEYWORD as a control panel with dials for each built-in
command.  Each dial adjusts one aspect of the command's behavior.

PREREQUISITE READING:
  Using_Aliases.txt    Renaming keywords (cosmetic)
  Scope.txt            Keyword access control and hooks
  Security.txt         Security levels and sandboxing


TABLE OF CONTENTS
-----------------
  1.  Syntax Overview
  2.  PRINT Properties
  3.  INPUT Properties
  4.  Other Keyword Properties
  5.  Custom Properties
  6.  Introspection: DESCRIBE, LIST, RESET
  7.  How Properties Work Internally
  8.  Combining KEYWORD with SCOPE
  9.  Combining KEYWORD with ALIAS
  10. Use Cases and Patterns
  11. Limitations
  12. Quick Reference


=====================================================================
1.  SYNTAX OVERVIEW
=====================================================================

Setting a property:

    KEYWORD keyword property value

Removing a property:

    KEYWORD keyword property OFF

Clearing all properties for one keyword:

    KEYWORD keyword RESET

Clearing all properties for ALL keywords:

    KEYWORD RESET

Showing available properties:

    KEYWORD keyword DESCRIBE

Showing current property for one keyword:

    KEYWORD keyword

Showing all active properties:

    KEYWORD LIST

Examples:

    KEYWORD PRINT UPPERCASE ON
    KEYWORD PRINT PREFIX ">>> "
    KEYWORD PRINT ZONE 20
    KEYWORD INPUT PROMPT "Enter: "
    KEYWORD PRINT RESET
    KEYWORD RESET


1.1  Property Names

Property names are case-insensitive:

    KEYWORD PRINT UPPERCASE ON
    KEYWORD PRINT uppercase on
    KEYWORD PRINT Uppercase On

All three are equivalent.


1.2  Property Values

Values can be:

    ON / OFF        Boolean toggles
    "string"        String values (in quotes)
    number          Numeric values (integers)

To remove a property, set its value to OFF or use
KEYWORD keyword property OFF.


=====================================================================
2.  PRINT PROPERTIES
=====================================================================

PRINT has the most properties, because output formatting is the
most common customization need.


2.1  UPPERCASE

    KEYWORD PRINT UPPERCASE ON

Forces all string output from PRINT to uppercase.  Numeric
output is not affected.

    KEYWORD PRINT UPPERCASE ON
    10 PRINT "Hello World"        ' prints: HELLO WORLD
    20 PRINT 42                   ' prints: 42 (unchanged)
    30 A$ = "Mixed Case"
    40 PRINT A$                   ' prints: MIXED CASE

Remove:

    KEYWORD PRINT UPPERCASE OFF

Note: UPPERCASE modifies the output at the moment of printing.
The original string variable is NOT modified:

    A$ = "hello"
    KEYWORD PRINT UPPERCASE ON
    PRINT A$                      ' prints: HELLO
    KEYWORD PRINT UPPERCASE OFF
    PRINT A$                      ' prints: hello (unchanged)


2.2  LOWERCASE

    KEYWORD PRINT LOWERCASE ON

Forces all string output from PRINT to lowercase.

    KEYWORD PRINT LOWERCASE ON
    10 PRINT "Hello World"        ' prints: hello world

If both UPPERCASE and LOWERCASE are set, UPPERCASE takes priority.

Remove:

    KEYWORD PRINT LOWERCASE OFF


2.3  PREFIX

    KEYWORD PRINT PREFIX "text"

Prepends the specified text before each PRINT output.

    KEYWORD PRINT PREFIX "[LOG] "
    10 PRINT "Application started"     ' [LOG] Application started
    20 PRINT "Loading data..."         ' [LOG] Loading data...
    30 PRINT "Ready."                  ' [LOG] Ready.

The prefix is printed once at the start of each PRINT statement,
before the first output item.

Remove:

    KEYWORD PRINT PREFIX OFF


2.4  ZONE

    KEYWORD PRINT ZONE number

Changes the width of comma-separated print zones.

In standard BASIC (ECMA-55), the comma separator advances the
cursor to the next 14-character zone boundary.  KEYWORD PRINT
ZONE lets you change that width.

    KEYWORD PRINT ZONE 20

    10 PRINT "A", "B", "C"
    ' Output:
    ' A                   B                   C

    KEYWORD PRINT ZONE 8

    10 PRINT "A", "B", "C"
    ' Output:
    ' A       B       C

This is useful for formatting tabular data.  Wide zones give
columns more breathing room; narrow zones conserve screen space.

Default zone width: 14 characters.

Remove (restores default):

    KEYWORD PRINT ZONE OFF


2.5  Stacking Properties

Multiple PRINT properties can be active simultaneously:

    KEYWORD PRINT UPPERCASE ON
    KEYWORD PRINT PREFIX "!!! "
    KEYWORD PRINT ZONE 20

    10 PRINT "alert", "message"

Output:

    !!! ALERT           MESSAGE

All active properties apply at once:
  1. PREFIX is printed first.
  2. String output is converted to UPPERCASE.
  3. Comma zones use the custom width.

To clear all PRINT properties at once:

    KEYWORD PRINT RESET


=====================================================================
3.  INPUT PROPERTIES
=====================================================================

3.1  PROMPT

    KEYWORD INPUT PROMPT "text"

Changes the default prompt displayed by INPUT when no custom
prompt string is provided in the statement.

By default, INPUT displays "? " when prompting:

    10 INPUT A$            ' displays: ?

With KEYWORD INPUT PROMPT:

    KEYWORD INPUT PROMPT "Enter> "
    10 INPUT A$            ' displays: Enter>

This does NOT affect INPUT statements that provide their own
prompt string:

    10 INPUT "Your name: "; A$     ' still shows: Your name:

It only changes the default "? " prompt.

Remove (restores "? "):

    KEYWORD INPUT PROMPT OFF


=====================================================================
4.  OTHER KEYWORD PROPERTIES
=====================================================================

The KEYWORD system is extensible.  While PRINT and INPUT have
the most commonly used wired properties, other keywords have
documented properties available for future use.

Use KEYWORD keyword DESCRIBE to see what's available:

    KEYWORD GOTO DESCRIBE

    Available properties for GOTO:
      STRICT     ON/OFF  Error if target line missing
      BOUNDS     lo-hi   Restrict jump range

    KEYWORD FOR DESCRIBE

    Available properties for FOR:
      MAXITER    number  Maximum loop iterations

These properties are documented and ready for future
implementation.  Currently, the interpreter recognizes them
in DESCRIBE output but they are not yet wired to change
behavior.  Setting them will store the value (visible in
KEYWORD LIST) but will not affect execution.

As the interpreter evolves, more properties will be wired to
actual behavior changes.


=====================================================================
5.  CUSTOM PROPERTIES
=====================================================================

You can set ANY named property on ANY keyword, even if the
property is not built into the interpreter.

    KEYWORD PRINT LOGFILE "output.log"
    KEYWORD PRINT AUTHOR "J. Smith"
    KEYWORD FOR MAXCOUNT 1000

Custom properties are stored and appear in KEYWORD LIST, but
they do not affect the interpreter's behavior directly.

Why use them?

  A. Documentation:  Annotate keywords with metadata.

  B. Hook Communication:  SCOPE hooks can read custom
     properties to modify their behavior:

       KEYWORD PRINT LOGFILE "output.log"
       SCOPE BEFORE PRINT GOSUB 9000

       9000 REM --- Log hook ---
       9010 REM Read the LOGFILE property
       9020 REM (future: use keyword_prop_get in C)
       9030 RETURN

  C. Configuration:  Store settings that external modules
     or future interpreter versions will use.

  D. Future-proofing:  When new wired properties are added
     to the interpreter, programs that already set them will
     automatically start using the new behavior.


=====================================================================
6.  INTROSPECTION: DESCRIBE, LIST, RESET
=====================================================================

6.1  KEYWORD keyword DESCRIBE

Shows all documented properties for a keyword, whether or not
they are currently set.

    KEYWORD PRINT DESCRIBE

Output:

    Available properties for PRINT:
      UPPERCASE  ON/OFF  Force output to uppercase
      LOWERCASE  ON/OFF  Force output to lowercase
      PREFIX     string  Prepend to every output
      SUFFIX     string  Append to every output
      NEWLINE    ON/OFF  Control trailing newline
      ZONE       number  Print zone width

If a keyword has no documented properties, DESCRIBE reports:

    KEYWORD LET DESCRIBE

    No documented properties for LET.

Note: You can still set custom properties on keywords that have
no documented properties.


6.2  KEYWORD keyword

Without a property name, shows the currently active properties
for that keyword.

    KEYWORD PRINT UPPERCASE ON
    KEYWORD PRINT PREFIX ">> "

    KEYWORD PRINT

Output:

    PRINT properties:
      UPPERCASE    = ON
      PREFIX       = >>

If no properties are set:

    KEYWORD PRINT

    PRINT: no properties set.


6.3  KEYWORD LIST

Shows all keywords that have at least one active property.

    KEYWORD PRINT UPPERCASE ON
    KEYWORD PRINT PREFIX ">> "
    KEYWORD INPUT PROMPT "? > "

    KEYWORD LIST

Output:

    Keywords with active properties:
    PRINT properties:
      UPPERCASE    = ON
      PREFIX       = >>
    INPUT properties:
      PROMPT       = ? >

If no properties are active anywhere:

    KEYWORD LIST

    Keywords with active properties:
      (none)


6.4  KEYWORD keyword RESET

Clears all properties for a single keyword.

    KEYWORD PRINT UPPERCASE ON
    KEYWORD PRINT PREFIX ">> "
    KEYWORD PRINT ZONE 20

    KEYWORD PRINT RESET

Output:

    PRINT: properties cleared.

After this, PRINT behaves exactly as default.


6.5  KEYWORD RESET

Clears ALL properties for ALL keywords.

    KEYWORD RESET

Output:

    All keyword properties cleared.

Use this at the start of a program to ensure a clean state:

    10 KEYWORD RESET
    20 KEYWORD PRINT UPPERCASE ON
    30 REM ... rest of program ...


=====================================================================
7.  HOW PROPERTIES WORK INTERNALLY
=====================================================================

Properties are stored in a global array indexed by keyword ID.
Each keyword can have up to 8 key-value pairs (MAX_KWPROPS).

    typedef struct {
        char key[32];        /* property name */
        char value[64];      /* property value as string */
        int  set;            /* 1 = active, 0 = cleared */
    } KwProp;

When a keyword handler executes (e.g., parse_print), it calls
keyword_prop_get() to check whether a property is set:

    const char *val = keyword_prop_get(KW_PRINT, "UPPERCASE");
    if (val && strcmp(val, "ON") == 0) {
        /* convert output to uppercase */
    }

This check is done every time the keyword executes, so changing
a property takes effect immediately on the next execution.

Properties are stored as strings regardless of whether the value
is a number, boolean, or text.  The keyword handler interprets
the value as needed.

For C module authors:  If you register a new keyword handler,
you can read properties with keyword_prop_get() and add your own
behavior customizations.


=====================================================================
8.  COMBINING KEYWORD WITH SCOPE
=====================================================================

KEYWORD and SCOPE are complementary:

    Feature       SCOPE                      KEYWORD
    -----------   -----------------------    --------------------
    Mechanism     GOSUB hooks + disable      Named properties
    User code     Requires subroutines       No subroutines
    Granularity   Before/after/instead-of    Internal behavior
    Extensible    Via user BASIC code        Via interpreter C code

They can be used together without conflict:

    SCOPE BEFORE PRINT GOSUB 9000    ' log before each PRINT
    KEYWORD PRINT UPPERCASE ON       ' force uppercase output

    10 PRINT "hello"
    20 END
    9000 PRINT "[LOG] "; : RETURN

Output:

    [LOG] HELLO

Execution order:
  1. SCOPE BEFORE hook fires (prints "[LOG] ")
  2. BEFORE hook subroutine uses PRINT -- this PRINT also has
     UPPERCASE ON, but the hook's own PRINT is not re-triggered
     by the BEFORE hook (re-entrancy guard, see Scope.txt §9)
  3. Hook returns
  4. Original line 10 PRINT executes with UPPERCASE ON


=====================================================================
9.  COMBINING KEYWORD WITH ALIAS
=====================================================================

ALIAS renames keywords; KEYWORD modifies their behavior.  They
work on the real keyword ID:

    ALIAS "AFFICHER" = PRINT
    KEYWORD PRINT UPPERCASE ON

    10 AFFICHER "hello"        ' prints: HELLO

The alias "AFFICHER" resolves to PRINT, and PRINT has the
UPPERCASE property active.  The property applies regardless of
what name was used to invoke the keyword.


=====================================================================
10.  USE CASES AND PATTERNS
=====================================================================

A. Log Formatting:

    KEYWORD PRINT PREFIX "[INFO] "
    PRINT "Server started"       ' [INFO] Server started
    PRINT "Listening on 8080"    ' [INFO] Listening on 8080
    KEYWORD PRINT PREFIX "[WARN] "
    PRINT "Disk space low"       ' [WARN] Disk space low

B. Case-Insensitive Comparisons:

    KEYWORD PRINT UPPERCASE ON
    ' All output is uppercase, making visual comparison easy

C. Wide Tables:

    KEYWORD PRINT ZONE 25

    10 PRINT "Name", "Score", "Grade"
    20 PRINT "----------", "-----", "-----"
    30 PRINT "Alice", 95, "A"
    40 PRINT "Bob", 82, "B"

    ' Clean columns with 25-character zones

D. Custom Prompts:

    KEYWORD INPUT PROMPT "> "
    ' All bare INPUT statements now show "> " instead of "? "

E. Teaching Mode:

    KEYWORD PRINT UPPERCASE ON
    KEYWORD PRINT PREFIX "OUTPUT: "
    SCOPE "STRUCTURED"
    ' Students see clear, labeled output and cannot use GOTO

F. Program Configuration Block:

    10 REM === Configuration ===
    20 KEYWORD PRINT ZONE 20
    30 KEYWORD INPUT PROMPT "? "
    40 KEYWORD RESET             ' clear at end of session

G. Annotating Keyword Usage:

    KEYWORD PRINT AUTHOR "main_module"
    KEYWORD PRINT VERSION "2.1"
    KEYWORD LIST                 ' see all annotations


=====================================================================
11.  LIMITATIONS
=====================================================================

  * Maximum 8 properties per keyword (MAX_KWPROPS).

  * Property names are limited to 31 characters.

  * Property values are limited to 63 characters.

  * Properties are global -- there is no per-subroutine or
    per-module scoping.  Setting KEYWORD PRINT UPPERCASE ON
    affects ALL PRINT statements everywhere.

  * Properties are not saved with the program.  They must be
    set up each time the program runs.

  * Only PRINT and INPUT have wired properties that change
    actual behavior.  Other keywords accept properties but
    only store them (for KEYWORD LIST and future use).

  * UPPERCASE and LOWERCASE are mutually exclusive.  If both
    are set, UPPERCASE takes priority.

  * The ZONE property affects comma-separated zones only.
    It does not affect semicolon positioning or TAB().


=====================================================================
12.  QUICK REFERENCE
=====================================================================

  Set a property:
    KEYWORD keyword property value

  Remove a property:
    KEYWORD keyword property OFF

  Clear one keyword:
    KEYWORD keyword RESET

  Clear all keywords:
    KEYWORD RESET

  Show available properties:
    KEYWORD keyword DESCRIBE

  Show current properties:
    KEYWORD keyword

  Show all active properties:
    KEYWORD LIST

  Wired properties (active behavior):

    PRINT properties:
      UPPERCASE ON/OFF    Force string output to uppercase
      LOWERCASE ON/OFF    Force string output to lowercase
      PREFIX    "text"    Prepend text before each PRINT item
      ZONE      number    Change comma zone width (default: 14)

    INPUT properties:
      PROMPT    "text"    Change default prompt (default: "? ")
