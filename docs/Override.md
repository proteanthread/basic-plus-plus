OVERRIDE: KEYWORD INTERPRETATION OVERRIDES
============================================
Version 4.1.1

OVERRIDE changes how the interpreter processes a keyword's arguments
at parse time.  It does NOT modify the user's stored source code
(.BAS file) and does NOT rewrite or replace any keywords.  The
program listing remains untouched -- only the parser's internal
input is affected.

Think of OVERRIDE as putting a lens in front of the parser.  The
interpreter sees a modified version of the line, but the original
line in memory is never changed.  LIST will always show the
original program exactly as you typed it.

PREREQUISITE READING:
  Using_Aliases.txt            Renaming keywords (cosmetic)
  Scope.txt                    Access control and behavior hooks
  Keyword.txt                  Modifying keyword properties
  Security.txt                 Security levels and sandboxing


TABLE OF CONTENTS
-----------------
  1.  What OVERRIDE Does
  2.  Syntax
  3.  How It Works Internally
  4.  Setting Overrides
  5.  Clearing Overrides
  6.  Listing Active Overrides
  7.  Resetting All Overrides
  8.  Protected Keywords
  9.  Re-Entrancy Safety
  10. OVERRIDE vs. SCOPE OVERRIDE
  11. OVERRIDE vs. ALIAS
  12. OVERRIDE vs. KEYWORD
  13. Combining OVERRIDE with Other Metaprogramming
  14. Security Restrictions
  15. Use Cases and Patterns
  16. Limitations
  17. Quick Reference


=====================================================================
1.  WHAT OVERRIDE DOES
=====================================================================

OVERRIDE lets you change how the parser interprets a keyword by
prepending additional text to the keyword's arguments before the
parser processes them.

Example:

    OVERRIDE PRINT "PRINT TAB(7);"

After this command, every time you execute a PRINT statement, the
interpreter internally processes it as if you had written:

    PRINT TAB(7); <your original arguments>

But the stored program line is NEVER modified.  If you type LIST,
you will see the original line exactly as you entered it.  OVERRIDE
affects interpretation only.

This is useful for changing default behavior without editing every
line in a program.  Instead of modifying 200 PRINT statements to
add TAB(7), you set one OVERRIDE and every PRINT automatically
indents.


=====================================================================
2.  SYNTAX
=====================================================================

Set an override:

    OVERRIDE keyword "replacement text"

Clear one override:

    OVERRIDE keyword CLEAR

List all active overrides:

    OVERRIDE LIST

Clear all overrides:

    OVERRIDE RESET


=====================================================================
3.  HOW IT WORKS INTERNALLY
=====================================================================

When the exec loop encounters a line, it peeks at the first keyword
on the line.  If that keyword has an active OVERRIDE, the
interpreter constructs a temporary parse string:

    override_text + " " + rest_of_line

where "rest_of_line" is everything AFTER the keyword in the
original source line.

This temporary string is then handed to the parser instead of the
original source.  The parser processes it as if it were the real
line.

Step by step:

  1. The exec loop peeks at the first token on the line.
  2. It finds the keyword (e.g., PRINT).
  3. It checks the override table for PRINT.
  4. If active, it reads the override text (e.g., "PRINT TAB(7);").
  5. It reads the rest of the original line after PRINT (e.g.,
     the user's arguments like "Hello").
  6. It constructs a temporary string:
         PRINT TAB(7); Hello
  7. A new lexer is initialized on this temporary string.
  8. The parser processes the temporary string.
  9. The original source line in memory is untouched.

IMPORTANT: The override text replaces the keyword AND becomes the
new beginning of the line.  The override text should normally
begin with the keyword itself, otherwise the parser will see a
line that starts with something unexpected.


=====================================================================
4.  SETTING OVERRIDES
=====================================================================

To set an override, specify the keyword and the replacement text
as a quoted string:

    OVERRIDE PRINT "PRINT TAB(7);"

This means: whenever the parser encounters PRINT as the first
keyword on a line, interpret it as if the line began with
PRINT TAB(7); followed by whatever arguments the user wrote.

More examples:

    OVERRIDE PRINT "PRINT TAB(10);"

    Every PRINT statement is interpreted with a 10-column indent.

    OVERRIDE PRINT "PRINT CHR$(27);\"[32m\";"

    Every PRINT statement is interpreted with an ANSI green
    color escape prefix.  (The inner quotes must be escaped
    with backslash.)

    OVERRIDE INPUT "INPUT TAB(5);"

    Every INPUT statement is interpreted with a 5-column indent
    before the prompt.


4.1  What Can Be Overridden?

Any keyword that is not protected can be overridden.  This includes
all standard BASIC statements:

  * I/O:           PRINT, INPUT, WRITE, LINE INPUT
  * Control flow:  GOTO, GOSUB, FOR, WHILE, DO, IF
  * File I/O:      OPEN, CLOSE, GET, PUT
  * Graphics:      DRAW, PLAY, CIRCLE, LINE, PSET
  * Data:          READ, DATA, DIM
  * System:        SHELL, SYS, PEEK, POKE

See Section 8 for the list of protected keywords that cannot
be overridden.


4.2  The Override Text Should Include the Keyword

The override text is used AS-IS to replace the keyword portion
of the line.  If you want the keyword to still execute, the
override text must include the keyword itself:

    CORRECT:   OVERRIDE PRINT "PRINT TAB(7);"
    INCORRECT: OVERRIDE PRINT "TAB(7);"

In the incorrect example, the parser would see TAB(7); as the
first token on the line, which is not a valid statement.

This design gives you maximum flexibility.  You could technically
redirect one keyword to another:

    OVERRIDE PRINT "LPRINT"

After this, every PRINT statement is interpreted as LPRINT
(line printer output).  The original source still says PRINT,
but the parser sees LPRINT.


=====================================================================
5.  CLEARING OVERRIDES
=====================================================================

To remove the override for a single keyword:

    OVERRIDE PRINT CLEAR

After this, PRINT returns to its default behavior.  The override
table entry for PRINT is erased.


=====================================================================
6.  LISTING ACTIVE OVERRIDES
=====================================================================

To see all currently active overrides:

    OVERRIDE LIST

Output example:

    Active overrides:
      PRINT        "PRINT TAB(7);"
      INPUT        "INPUT TAB(5);"

If no overrides are active:

    Active overrides:
      (none)

OVERRIDE LIST is useful for debugging.  When a keyword behaves
unexpectedly, check whether a forgotten override is affecting it.


=====================================================================
7.  RESETTING ALL OVERRIDES
=====================================================================

To clear ALL overrides at once:

    OVERRIDE RESET

Output:

    All overrides cleared.

This removes every override in the table and restores all keywords
to their default behavior.  Use it at the start of a program to
ensure a clean state:

    10 OVERRIDE RESET
    20 OVERRIDE PRINT "PRINT TAB(7);"
    30 REM ... rest of program ...


=====================================================================
8.  PROTECTED KEYWORDS
=====================================================================

Certain keywords are protected and CANNOT be overridden.  Attempting
to override a protected keyword prints an error message:

    Cannot override SCOPE (protected)

Protected keywords:

  SCOPE        Overriding SCOPE would break access control.
  ALIAS        Overriding ALIAS would break keyword renaming.
  KEYWORD      Overriding KEYWORD would break property changes.
  OVERRIDE     Overriding OVERRIDE itself would create paradoxes.
  REM          Comments must always work.
  SECURITY     Security level changes must not be intercepted.

These protections exist to prevent metaprogramming commands from
disabling each other.  If you could override OVERRIDE, you could
make it impossible to clear overrides.  If you could override
SECURITY, you could bypass security restrictions.


=====================================================================
9.  RE-ENTRANCY SAFETY
=====================================================================

A common concern: what happens when the override text contains the
same keyword that triggered it?

    OVERRIDE PRINT "PRINT TAB(7);"

    10 PRINT "Hello"

Would this cause an infinite loop?  The parser sees PRINT TAB(7);
"Hello", which starts with PRINT, which has an override, which
produces PRINT TAB(7); TAB(7); "Hello", which starts with
PRINT ...

NO.  BASIC++ uses a re-entrancy guard.  When an override is being
processed, all overrides are SUPPRESSED until the override
completes.  The interpreter tracks this using the same stack-depth
guard used by SCOPE hooks.

This means:

  * The override fires ONCE per line, never recursively.
  * The override text can safely begin with the same keyword.
  * Inside the override's execution, no further overrides fire.
  * After the line completes, overrides are active again for
    the next line.

The guard is automatic and invisible.  You do not need to do
anything special.


=====================================================================
10.  OVERRIDE vs. SCOPE OVERRIDE
=====================================================================

BASIC++ has TWO features with similar names that work very
differently:

  OVERRIDE (this document):
    Changes how the parser INTERPRETS a keyword's arguments.
    The keyword still executes.  The source code is not modified.
    Works by constructing a modified parse string.

  SCOPE OVERRIDE keyword GOSUB line:
    REPLACES the keyword with a subroutine call.  The keyword
    does NOT execute at all.  The subroutine runs instead.
    Works by jumping to a GOSUB and skipping the original line.

Key differences:

  Feature             OVERRIDE           SCOPE OVERRIDE
  -----------------------------------------------------------
  Keyword executes?   YES (modified)     NO (replaced)
  Mechanism           Parse-time text    GOSUB dispatch
  Needs subroutine?   No                 Yes (GOSUB target)
  Source changed?      No                 No
  Arguments passed?   Yes (original)     No (hook cannot see)
  Can modify args?    Yes                No

When to use which:

  * Use OVERRIDE to change DEFAULT ARGUMENTS.  The keyword still
    runs, but with different or additional arguments.

  * Use SCOPE OVERRIDE to REPLACE the keyword entirely with
    custom behavior.  The original keyword never executes.

They can even be used together on different keywords:

    OVERRIDE PRINT "PRINT TAB(7);"
    SCOPE OVERRIDE INPUT GOSUB 9000

IMPORTANT: If both OVERRIDE and SCOPE OVERRIDE are set on the
SAME keyword, SCOPE OVERRIDE takes priority.  The SCOPE hook
fires first and jumps away; the OVERRIDE text is never reached.


=====================================================================
11.  OVERRIDE vs. ALIAS
=====================================================================

  ALIAS:     Renames a keyword.  Cosmetic.  The original keyword
             works exactly the same, it just has a new name.

  OVERRIDE:  Changes how the parser processes the keyword's
             arguments.  The keyword name is unchanged.

They are complementary:

    ALIAS "AFFICHER" = PRINT
    OVERRIDE PRINT "PRINT TAB(7);"

    10 AFFICHER "Bonjour"

The alias AFFICHER resolves to PRINT.  The OVERRIDE on PRINT
then modifies the parse to PRINT TAB(7); "Bonjour".  Both
apply.


=====================================================================
12.  OVERRIDE vs. KEYWORD
=====================================================================

  KEYWORD:   Changes a keyword's PROPERTIES (uppercase, bounds,
             prefix, logging).  These are built-in behavioral
             modifiers.

  OVERRIDE:  Changes the keyword's ARGUMENTS at parse time.
             This is a text-level modification.

They are complementary:

    KEYWORD PRINT UPPERCASE ON
    OVERRIDE PRINT "PRINT TAB(7);"

    10 PRINT "hello"

The OVERRIDE makes the parser see PRINT TAB(7); "hello".
The KEYWORD UPPERCASE property then forces the output to
uppercase.  Result: the cursor moves to column 7, then HELLO
is printed in uppercase.


=====================================================================
13.  COMBINING OVERRIDE WITH OTHER METAPROGRAMMING
=====================================================================

All four metaprogramming commands can be used together.  They
operate at different levels:

  ALIAS     Lexer level    Renames keywords (cosmetic)
  OVERRIDE  Parse level    Modifies keyword arguments
  KEYWORD   Behavior level Changes keyword properties
  SCOPE     Execution level Controls access and hooks

Processing order for a single line:

  1. ALIAS resolves any renamed keywords in the lexer.
  2. SCOPE checks for disabled keywords or BEFORE/OVERRIDE hooks.
  3. If SCOPE hook fires, it takes priority and OVERRIDE is
     skipped.
  4. If no SCOPE hook, OVERRIDE constructs the modified parse
     string.
  5. The parser processes the (possibly modified) line.
  6. KEYWORD properties apply during execution (e.g., uppercase).
  7. SCOPE AFTER hook fires if set (after execution).

Example combining all four:

    ALIAS "AFFICHER" = PRINT
    OVERRIDE PRINT "PRINT TAB(7);"
    KEYWORD PRINT UPPERCASE ON
    SCOPE AFTER PRINT GOSUB 9000

    10 AFFICHER "hello"
    20 END
    9000 PRINT "[logged]" : RETURN

Processing of line 10:

  1. ALIAS resolves AFFICHER to PRINT.
  2. SCOPE: no BEFORE or OVERRIDE hook, not disabled.
  3. OVERRIDE: constructs PRINT TAB(7); "hello".
  4. Parser executes PRINT TAB(7); "hello".
  5. KEYWORD UPPERCASE converts output to HELLO.
  6. SCOPE AFTER fires: prints [logged].

Result:

           HELLO
    [logged]


=====================================================================
14.  SECURITY RESTRICTIONS
=====================================================================

OVERRIDE is blocked at SECURITY LEVEL 2 (RESTRICTED) and above.
When security is raised, attempting to use OVERRIDE results in:

    SORRY? Security: system not permitted at level RESTRICTED

This prevents untrusted programs from modifying keyword behavior.
Once security is raised (the one-way ratchet prevents lowering
it), OVERRIDE commands are rejected.

At SECURITY LEVEL 0 (OPEN):      OVERRIDE is allowed
At SECURITY LEVEL 1 (STANDARD):  OVERRIDE is blocked
At SECURITY LEVEL 2 (RESTRICTED): OVERRIDE is blocked


=====================================================================
15.  USE CASES AND PATTERNS
=====================================================================

A. Uniform indentation:

    OVERRIDE PRINT "PRINT TAB(7);"

    Every PRINT statement in the program is indented to column 7
    without editing any source code.  Useful for formatting output
    in educational or presentation contexts.


B. Output prefixing:

    OVERRIDE PRINT "PRINT \">>> \";"

    Every PRINT statement is prefixed with >>> followed by a space.
    Useful for distinguishing program output from system messages.


C. Default line printer redirect:

    OVERRIDE PRINT "LPRINT"

    Every PRINT statement is redirected to the line printer.
    The program source still says PRINT, but output goes to
    the printer.  Change it back with OVERRIDE PRINT CLEAR.


D. Debugging aid:

    OVERRIDE GOTO "PRINT \"GOTO\": GOTO"

    Every GOTO statement first prints "GOTO" before jumping.
    This creates an execution trace without modifying the
    program.  (Note: works only if multi-statement lines are
    supported with the colon separator.)


E. Input prompting:

    OVERRIDE INPUT "INPUT TAB(5); \"? \";"

    Every INPUT statement shows the prompt at column 5 with
    a question mark, regardless of what the program specifies.


F. Program cleanup with OVERRIDE RESET:

    10 OVERRIDE RESET
    20 REM start with clean state
    30 OVERRIDE PRINT "PRINT TAB(4);"
    40 PRINT "Indented output"
    50 OVERRIDE RESET
    60 PRINT "Normal output"


=====================================================================
16.  LIMITATIONS
=====================================================================

  * Only ONE override per keyword.  Setting a new override on
    PRINT replaces any existing override on PRINT.

  * Override text maximum length is 127 characters.  Longer
    text is silently truncated.

  * Overrides only fire on the FIRST keyword of a line.
    Multi-statement lines (separated by colons) only trigger
    the override for the first statement.

  * The override text must be a valid beginning of a statement.
    If you set override text that the parser cannot understand,
    you will get syntax errors on every line that uses that
    keyword.

  * Overrides do not modify the stored program.  LIST always
    shows the original source.  There is no way to "bake in"
    an override permanently.

  * Overrides are session-wide.  They persist across RUN and
    survive until OVERRIDE CLEAR, OVERRIDE RESET, or NEW.

  * Overrides are NOT saved with the program.  They must be
    set up each session or at the beginning of the program.

  * Protected keywords cannot be overridden (see Section 8).

  * SCOPE OVERRIDE takes priority over OVERRIDE.  If both are
    set on the same keyword, SCOPE OVERRIDE fires and the
    OVERRIDE text is never used.

  * The total spliced line (override text + original arguments)
    is limited to 1023 characters.  If it exceeds this, the
    override is silently skipped and the original line executes.


=====================================================================
17.  QUICK REFERENCE
=====================================================================

  Set an override:
    OVERRIDE keyword "text"

  Clear one override:
    OVERRIDE keyword CLEAR

  List all active overrides:
    OVERRIDE LIST

  Clear all overrides:
    OVERRIDE RESET

  Protected keywords (cannot be overridden):
    SCOPE, ALIAS, KEYWORD, OVERRIDE, REM, SECURITY

  Security:
    Blocked at SECURITY LEVEL 1 and above

  Priority when combined with SCOPE:
    SCOPE OVERRIDE (GOSUB) takes priority over OVERRIDE (text)

  Re-entrancy:
    Overrides fire once per line, never recursively

  Source code:
    NEVER modified -- interpretation only
