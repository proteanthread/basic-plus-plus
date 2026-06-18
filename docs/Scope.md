SCOPE: KEYWORD ACCESS CONTROL AND BEHAVIOR HOOKS
==================================================
Version 4.0.1

SCOPE is a metaprogramming statement that controls how built-in
keywords behave at runtime.  With SCOPE you can:

  * Disable individual keywords (blocking their use entirely)
  * Enable previously disabled keywords
  * Attach BEFORE hooks (run a subroutine before a keyword)
  * Attach AFTER hooks (run a subroutine after a keyword)
  * Attach OVERRIDE hooks (replace a keyword entirely)
  * Load predefined safety presets
  * Inspect and reset all active rules

SCOPE is a session-level command: rules persist across RUN and
survive until explicitly cleared, the program calls SCOPE RESET,
or a NEW command is issued.

PREREQUISITE READING:
  Using_Aliases.txt      Renaming keywords (cosmetic)
  Security.txt           Security levels and sandboxing
  Subroutines_And_Functions.txt   GOSUB/RETURN basics


TABLE OF CONTENTS
-----------------
  1.  Disabling and Enabling Keywords
  2.  Safety Presets
  3.  BEFORE Hooks
  4.  OVERRIDE Hooks
  5.  AFTER Hooks
  6.  RESTORE: Clearing Individual Rules
  7.  LIST: Inspecting Active Rules
  8.  RESET: Clearing Everything
  9.  Re-Entrancy Safety
  10. Combining SCOPE with KEYWORD
  11. Combining SCOPE with ALIAS
  12. Combining SCOPE with SECURITY
  13. Use Cases and Patterns
  14. Limitations
  15. Quick Reference


=====================================================================
1.  DISABLING AND ENABLING KEYWORDS
=====================================================================

The most basic SCOPE operation: blocking a keyword.

Syntax:

    SCOPE DISABLE keyword
    SCOPE ENABLE keyword

When a keyword is disabled, any attempt to execute a line that
begins with that keyword raises an error:

    SCOPE DISABLE GOTO

    10 GOTO 100       ' Error: "Keyword disabled by SCOPE at line 10"

The keyword is completely blocked.  It cannot be used in programs,
in direct mode, or inside subroutines.  It is as if the keyword
does not exist.

To restore access:

    SCOPE ENABLE GOTO

    10 GOTO 100       ' Works again

IMPORTANT: SCOPE DISABLE only blocks the keyword as a STATEMENT
(i.e., the first word on a line).  It does not block the keyword
when used as a function argument or in an expression.  For
example, disabling PRINT does not prevent PRINT$ from working
if such a function existed.


1.1  What Can Be Disabled?

Any keyword recognized by the interpreter's keyword table can be
disabled.  This includes:

  * Control flow:    GOTO, GOSUB, ON, IF, FOR, WHILE, DO
  * I/O statements:  PRINT, INPUT, OPEN, CLOSE, WRITE
  * System access:   PEEK, POKE, SYS, SHELL, BSAVE, BLOAD
  * Program control: RUN, LIST, SAVE, LOAD, NEW, SYSTEM, END
  * Data statements: READ, DATA, RESTORE, DIM

You cannot disable REM (comments are always allowed) or SCOPE
itself (you could lock yourself out permanently).


1.2  Error Behavior

When a disabled keyword is encountered, the error message is:

    Keyword disabled by SCOPE at line NNN

This error can be caught with ON ERROR GOTO:

    10 ON ERROR GOTO 100
    20 SCOPE DISABLE GOTO
    30 GOTO 500             ' triggers error
    40 END
    100 IF ERR = 2 THEN PRINT "GOTO is blocked!" : RESUME NEXT


=====================================================================
2.  SAFETY PRESETS
=====================================================================

Instead of disabling keywords one at a time, you can load a
preset that disables a curated group of keywords.

Syntax:

    SCOPE "preset_name"

Available presets:

  STRUCTURED
    Disables: GOTO, ON
    Purpose:  Enforce structured programming.  Students must
              use IF/ELSE, FOR/NEXT, WHILE/WEND, and GOSUB
              instead of unstructured jumps.

  SAFE
    Disables: PEEK, POKE, SYS, BSAVE, BLOAD, SYSTEM
    Purpose:  Block low-level hardware access.  Programs cannot
              read or write arbitrary memory, call machine-
              language routines, or exit to the operating system.

  MINIMAL
    Disables: Everything in SAFE, plus file I/O keywords
              (OPEN, CLOSE, PRINT#, INPUT#, WRITE#, KILL,
               MKDIR, RMDIR, CHDIR)
    Purpose:  Maximum restriction.  Programs can only compute
              and display output to the console.

  EDUCATIONAL
    Disables: GOTO plus everything in SAFE
    Purpose:  Teaching environment.  Forces structured control
              flow AND blocks dangerous system access.

Example:

    SCOPE "STRUCTURED"

This is equivalent to:

    SCOPE DISABLE GOTO
    SCOPE DISABLE ON

Presets can be combined by loading them sequentially:

    SCOPE "STRUCTURED"
    SCOPE "SAFE"

This disables GOTO, ON, PEEK, POKE, SYS, BSAVE, BLOAD, and
SYSTEM.

IMPORTANT: Presets only DISABLE keywords.  They never enable
anything.  Loading a preset adds to the current set of disabled
keywords without removing existing rules.


=====================================================================
3.  BEFORE HOOKS
=====================================================================

A BEFORE hook runs a subroutine BEFORE the hooked keyword
executes.  After the subroutine returns, the keyword executes
normally.

Syntax:

    SCOPE BEFORE keyword GOSUB line_number

Example:

    SCOPE BEFORE PRINT GOSUB 9000

    10 PRINT "Hello"
    20 PRINT "World"
    30 END
    9000 REM --- Before-Print Hook ---
    9010 PRINT "[LOG] About to print at ";
    9020 PRINT TIMER;
    9030 PRINT ": ";
    9040 RETURN

Output:

    [LOG] About to print at 12345: Hello
    [LOG] About to print at 12345: World

The hook subroutine (9000-9040) runs before EACH execution of
PRINT.  When the subroutine RETURNs, execution returns to the
original PRINT line and the keyword executes normally.


3.1  How BEFORE Works Internally

When the exec loop encounters a line whose first keyword has a
BEFORE hook:

  1. The interpreter saves the current line index.
  2. A GOSUB frame is pushed, with a return address of the
     SAME line (so the keyword re-executes after RETURN).
  3. Execution jumps to the hook subroutine.
  4. The hook subroutine executes normally.
  5. RETURN pops the GOSUB frame, returning to the original
     line.
  6. The original keyword now executes as if nothing happened.

This means the hook subroutine is a regular GOSUB -- it can use
any BASIC statements, access variables, call other subroutines,
and even use the hooked keyword itself (see Section 9 on
re-entrancy).


3.2  Use Cases for BEFORE Hooks

  A. Logging:  Record every PRINT, INPUT, or GOTO to a log file.

  B. Validation:  Check a precondition before OPEN or WRITE.
     The hook can set an error flag and skip the operation.

  C. Counting:  Increment a counter each time a keyword runs.

       SCOPE BEFORE PRINT GOSUB 9000
       ...
       9000 PrintCount = PrintCount + 1 : RETURN

  D. Throttling:  Add a delay before each PRINT to slow
     output scrolling.


=====================================================================
4.  OVERRIDE HOOKS
=====================================================================

An OVERRIDE hook REPLACES the keyword entirely.  The original
keyword never executes -- only the hook subroutine runs.

Syntax:

    SCOPE OVERRIDE keyword GOSUB line_number

Example:

    SCOPE OVERRIDE PRINT GOSUB 9000

    10 PRINT "You will NOT see this"
    20 PRINT "This is also intercepted"
    30 SCOPE RESTORE PRINT
    40 PRINT "PRINT is back to normal"
    50 END
    9000 REM --- Override-Print Hook ---
    9010 PRINT "[INTERCEPTED]"
    9020 RETURN

Output:

    [INTERCEPTED]
    [INTERCEPTED]
    PRINT is back to normal


4.1  How OVERRIDE Works Internally

When the exec loop encounters a line whose first keyword has an
OVERRIDE hook:

  1. A GOSUB frame is pushed, with a return address of the
     NEXT line (current_index + 1).
  2. Execution jumps to the hook subroutine.
  3. The hook subroutine runs.
  4. RETURN pops the GOSUB frame and jumps to the NEXT line.
  5. The original keyword is SKIPPED entirely.

The key difference from BEFORE: the GOSUB return address is
current_index + 1 (the next line), not current_index (the same
line).  This means the original line never re-executes.


4.2  Use Cases for OVERRIDE Hooks

  A. Sandboxing:  Replace OPEN, SHELL, or KILL with a "not
     allowed" message.

  B. Mocking:  Replace INPUT with a subroutine that returns
     test data, enabling automated testing of programs that
     normally require user interaction.

  C. Redirection:  Replace PRINT with a subroutine that writes
     to a virtual device or network connection instead.

  D. Soft Deprecation:  Override a keyword to print a warning
     and then call the original behavior:

       9000 PRINT "WARNING: GOTO is deprecated"
       9010 RETURN


=====================================================================
5.  AFTER HOOKS
=====================================================================

An AFTER hook runs a subroutine AFTER the keyword has already
executed.  The keyword runs normally first, then the hook fires.

Syntax:

    SCOPE AFTER keyword GOSUB line_number

Example:

    SCOPE AFTER PRINT GOSUB 9000

    10 PRINT "Message one"
    20 PRINT "Message two"
    30 SCOPE RESTORE PRINT
    40 PRINT "No more hooks"
    50 END
    9000 REM --- After-Print Hook ---
    9010 PRINT "  [logged]"
    9020 RETURN

Output:

    Message one
      [logged]
    Message two
      [logged]
    No more hooks


5.1  How AFTER Works Internally

  1. The exec loop records that the keyword has an AFTER hook.
  2. The keyword executes normally.
  3. After execution (and only if no error occurred), a GOSUB
     frame is pushed with return address = current_index + 1.
  4. Execution jumps to the hook subroutine.
  5. RETURN pops the frame and continues to the next line.


5.2  Use Cases for AFTER Hooks

  A. Audit Trails:  Log every successful file operation.

  B. Cleanup:  Close temporary resources after PRINT or WRITE.

  C. Assertions:  Verify postconditions after critical operations.


=====================================================================
6.  RESTORE: CLEARING INDIVIDUAL RULES
=====================================================================

SCOPE RESTORE clears all rules for a single keyword:  disabling,
BEFORE, AFTER, and OVERRIDE hooks are all removed.

Syntax:

    SCOPE RESTORE keyword

Example:

    SCOPE DISABLE GOTO
    SCOPE BEFORE PRINT GOSUB 9000
    SCOPE AFTER PRINT GOSUB 9100

    SCOPE RESTORE PRINT     ' clears BEFORE and AFTER on PRINT
    SCOPE RESTORE GOTO      ' re-enables GOTO

After SCOPE RESTORE, the keyword works exactly as if no SCOPE
rules were ever applied to it.


=====================================================================
7.  LIST: INSPECTING ACTIVE RULES
=====================================================================

SCOPE LIST displays all currently active rules.

Syntax:

    SCOPE LIST

Output example:

    Active scope rules:
      GOTO         DISABLED
      PRINT        BEFORE GOSUB 9000
      PEEK         DISABLED
      POKE         DISABLED

If no rules are active, it prints:

    Active scope rules:
      (none)

SCOPE LIST is invaluable for debugging.  When a keyword behaves
unexpectedly, check whether a forgotten SCOPE rule is affecting it.


=====================================================================
8.  RESET: CLEARING EVERYTHING
=====================================================================

SCOPE RESET removes ALL scope rules at once: all disabled
keywords are re-enabled, and all hooks are removed.

Syntax:

    SCOPE RESET

Output:

    All scope rules cleared.

This is equivalent to calling SCOPE RESTORE for every keyword
that has any rule.  Use it at the start of a program to ensure
a clean state:

    10 SCOPE RESET             ' start fresh
    20 SCOPE "STRUCTURED"      ' apply desired preset
    30 REM ... rest of program ...


=====================================================================
9.  RE-ENTRANCY SAFETY
=====================================================================

A common concern:  what happens when a hook subroutine uses the
SAME keyword that triggered the hook?

    SCOPE BEFORE PRINT GOSUB 9000

    10 PRINT "Hello"
    20 END
    9000 PRINT "[BEFORE] "       ' uses PRINT inside a PRINT hook!
    9010 RETURN

Will this cause an infinite loop?  NO.

BASIC++ uses a stack-based re-entrancy guard.  When a hook
subroutine is executing, hooks are SUPPRESSED for all keywords.
The interpreter tracks the GOSUB stack depth at hook entry and
will not fire any hooks while the stack is above that level.

This means:

  * Hook subroutines can use ANY keyword safely.
  * Hook subroutines can even use the SAME hooked keyword.
  * Hooks never fire inside hooks -- no infinite loops.
  * When RETURN pops the hook's GOSUB frame, hooks become
    active again for the next line.

The guard is automatic and invisible.  You do not need to do
anything special in your hook subroutines.


9.1  Nested Hooks

If you set hooks on multiple keywords:

    SCOPE BEFORE PRINT GOSUB 9000
    SCOPE BEFORE INPUT GOSUB 9100

And the PRINT hook uses INPUT, the INPUT hook will NOT fire
during the PRINT hook execution.  All hooks are suppressed
while any hook is active.


=====================================================================
10.  COMBINING SCOPE WITH KEYWORD
=====================================================================

SCOPE and KEYWORD are complementary systems:

    SCOPE    Controls ACCESS and FLOW
             (who can use a keyword, and what runs around it)

    KEYWORD  Controls BEHAVIOR
             (how a keyword works internally)

They can be used together:

    KEYWORD PRINT UPPERCASE ON     ' force uppercase output
    SCOPE BEFORE PRINT GOSUB 9000  ' run hook before each PRINT

    10 PRINT "hello"
    20 END
    9000 PRINT "[LOG] "; : RETURN

Output:

    [LOG] HELLO

The KEYWORD property (UPPERCASE) modifies the PRINT behavior,
while the SCOPE hook adds the "[LOG] " prefix through a
subroutine.  Both apply simultaneously.

See Keyword.txt for full documentation of the KEYWORD system.


=====================================================================
11.  COMBINING SCOPE WITH ALIAS
=====================================================================

ALIAS renames keywords; SCOPE controls them.  They work together:

    ALIAS "AFFICHER" = PRINT
    SCOPE BEFORE PRINT GOSUB 9000

    10 AFFICHER "Bonjour"    ' ALIAS resolves to PRINT
                             ' SCOPE BEFORE PRINT fires
                             ' Hook runs, then PRINT executes

SCOPE rules apply to the REAL keyword, not the alias.  So
hooking PRINT also hooks any alias that maps to PRINT.


=====================================================================
12.  COMBINING SCOPE WITH SECURITY
=====================================================================

SCOPE and SECURITY are independent layers:

    SECURITY LEVEL     Sets a floor on what's allowed
    SCOPE DISABLE      Blocks specific keywords above that floor

Example:

    SECURITY LEVEL 1         ' block PEEK, POKE, SYS, DEF USR
    SCOPE DISABLE GOTO       ' also block GOTO (not in security)
    SCOPE "STRUCTURED"       ' and ON

SECURITY restrictions cannot be overridden by SCOPE ENABLE.  If
SECURITY LEVEL 2 blocks SHELL, then SCOPE ENABLE SHELL has no
effect -- the security system operates below SCOPE.


=====================================================================
13.  USE CASES AND PATTERNS
=====================================================================

A. Teaching environment:

    SCOPE "EDUCATIONAL"
    KEYWORD PRINT PREFIX ">>> "

    Students see clear output, cannot use GOTO or PEEK/POKE.

B. Automated testing:

    SCOPE OVERRIDE INPUT GOSUB 9000
    ' Replace interactive INPUT with scripted test data
    9000 A$ = TestData$(TestIndex)
    9010 TestIndex = TestIndex + 1
    9020 RETURN

C. Execution logging:

    SCOPE BEFORE GOTO GOSUB 9000
    SCOPE BEFORE GOSUB GOSUB 9000
    9000 PRINT "TRACE: line "; ERL; " -> ";
    9010 RETURN

D. Keyword usage profiling:

    SCOPE BEFORE PRINT GOSUB 9000
    SCOPE BEFORE INPUT GOSUB 9010
    SCOPE BEFORE GOTO GOSUB 9020
    9000 PC = PC + 1 : RETURN
    9010 IC = IC + 1 : RETURN
    9020 GC = GC + 1 : RETURN
    ' At end: PRINT "PRINT:"; PC; "INPUT:"; IC; "GOTO:"; GC

E. Restricting a loaded program:

    10 SCOPE "SAFE"
    20 SCOPE DISABLE CHAIN
    30 SCOPE DISABLE MERGE
    40 CHAIN "untrusted.bas"   ' runs with restrictions in place


=====================================================================
14.  LIMITATIONS
=====================================================================

  * Only ONE hook type per keyword.  Setting a BEFORE hook on
    PRINT replaces any existing OVERRIDE or AFTER hook on PRINT.

  * Hooks only fire on the FIRST keyword of a line.  Multi-
    statement lines (separated by colons) only trigger hooks
    for the first statement.

  * Hooks use GOSUB, which consumes one stack frame.  Deep hook
    chains combined with deeply nested GOSUB/FOR can overflow
    the 256-frame stack.

  * Hook subroutines are plain BASIC subroutines.  They cannot
    access the hooked line's arguments or parameters.

  * SCOPE rules are session-wide.  They are not saved with the
    program.  They must be set up at the start of each session
    or at the beginning of the program.

  * You cannot disable SCOPE itself or REM.

  * Presets only disable keywords; they never set hooks.


=====================================================================
15.  QUICK REFERENCE
=====================================================================

  Disable a keyword:
    SCOPE DISABLE keyword

  Enable a keyword:
    SCOPE ENABLE keyword

  Load a preset:
    SCOPE "preset"

  Set a BEFORE hook:
    SCOPE BEFORE keyword GOSUB line

  Set an AFTER hook:
    SCOPE AFTER keyword GOSUB line

  Set an OVERRIDE hook:
    SCOPE OVERRIDE keyword GOSUB line

  Clear one keyword's rules:
    SCOPE RESTORE keyword

  Show all active rules:
    SCOPE LIST

  Clear all rules:
    SCOPE RESET

  Available presets:
    "STRUCTURED"    GOTO, ON
    "SAFE"          PEEK, POKE, SYS, BSAVE, BLOAD, SYSTEM
    "MINIMAL"       SAFE + file I/O
    "EDUCATIONAL"   GOTO + SAFE
