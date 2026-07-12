====================================================================
BASIC++ TUTORIAL: USER-DEFINED DIALECTS
====================================================================

Chapter: K - Creating and Modifying Dialects
Level: Intermediate to Advanced
Prerequisite: Understanding of BASIC++ (any dialect)

====================================================================
TABLE OF CONTENTS
====================================================================

  PART 1:  What Is a Dialect?
  PART 2:  How Dialect Files Work
  PART 3:  The 16 Built-In Dialects
  PART 4:  How to Modify an Existing Dialect
  PART 5:  How to Create a New Dialect
  PART 6:  Understanding the DialectConfig Struct
  PART 7:  The Keyword Bitmask System
  PART 8:  Adding Dialect-Specific Features
  PART 9:  Achieving 100% Dialect Compatibility
  PART 10: Testing Your Dialect
  PART 11: Contributing Your Dialect

====================================================================
PART 1: WHAT IS A DIALECT?
====================================================================

A dialect in BASIC++ is a configuration profile that tells the
interpreter how to behave. Different BASIC dialects were shipped
with different machines in the 1970s-1990s, and each had its own
quirks, features, and limitations.

BASIC++ supports 16 built-in dialects. You can switch between them
at any time:

    DIALECT "GWBS"      Switch to GW-BASIC mode
    DIALECT "C64B"      Switch to Commodore 64 mode
    DIALECT "ATRI"      Switch to Atari BASIC mode
    DIALECT LIST        List all available dialects

When you switch dialects, BASIC++ changes:
  - Which keywords are available (OPTION STRICT mode)
  - How PRINT formats output (zone width)
  - Whether LET is required or optional
  - Whether THEN is required in IF statements
  - What the ready prompt looks like ("Ok", "READY.", "]")
  - Maximum line number allowed
  - What features are available (WHILE/WEND, DO/LOOP, etc.)

====================================================================
PART 2: HOW DIALECT FILES WORK
====================================================================

Each dialect lives in its own C source file:

    dialect_patb.c      Palo Alto Tiny BASIC
    dialect_trs1.c      TRS-80 Level I
    dialect_trs2.c      TRS-80 Level II
    dialect_gwbs.c      GW-BASIC
    dialect_ecma55.c    ECMA-55 Minimal BASIC
    dialect_ecma116.c   ECMA-116 Full BASIC
    dialect_qbasic.c    QBasic
    dialect_aint.c      Apple II Integer BASIC
    dialect_asft.c      AppleSoft BASIC
    dialect_atari.c     Atari BASIC
    dialect_c64.c       Commodore BASIC v2
    dialect_coco.c      Color Computer BASIC
    dialect_sinclair.c  Sinclair BASIC (ZX Spectrum)
    dialect_superbasic.c SuperBASIC (Sinclair QL)
    dialect_mbasic.c    MBASIC (Microsoft, CP/M)
    dialect_sbasic.c    SUPER BASIC (Tymshare SDS-940)

Each file contains exactly three things:

  1. A HEADER COMMENT with machine history, quirks, what's
     implemented, what's missing, and how to modify the dialect.

  2. A STATIC DialectConfig STRUCT that defines all the
     feature flags for that dialect.

  3. A REGISTRATION FUNCTION that registers the dialect
     with the BASIC++ dialect system at boot time.

Here's the simplest possible dialect file:

    #include "dialect.h"

    static const DialectConfig my_config = {
        DIALECT_MY_BASIC,       /* id from enum */
        "My Custom BASIC",      /* display name */
        ':',                    /* statement separator */
        /* ... feature flags ... */
        "MYBAS",                /* short name */
        DFLAG_MYBAS             /* bitmask flag */
    };

    void dialect_register_mybas(void)
    {
        dialect_register(&my_config);
    }

At boot time, main.c calls dialect_register_all(), which calls
every dialect_register_XXXX() function. Each dialect copies its
config into the central registry. Then dialect_init() selects
the default dialect, and you're running.

====================================================================
PART 3: THE 16 BUILT-IN DIALECTS
====================================================================

Each built-in dialect file is extensively documented with:

  - Machine specifications (CPU, RAM, display, sound, storage)
  - Historical notes (who wrote it, when, why)
  - Dialect quirks (how it differs from standard Microsoft BASIC)
  - WHAT'S IMPLEMENTED: checklist of working features
  - WHAT'S MISSING: specific features needed for 100% compatibility,
    with concrete fix suggestions and implementation hints
  - HOW TO MODIFY: step-by-step instructions for contributors

The dialect files serve as both CODE and DOCUMENTATION. Open any
dialect_*.c file and read the header — it tells you everything
you need to know about that machine's BASIC.

Here's a summary of each dialect's personality:

  PATB  - Li-Chen Wang's Tiny BASIC (1976). ';' separator,
          '#' not-equal, IF without THEN. Integer only.
          The grandparent of all microcomputer BASICs.

  TRS1  - TRS-80 Level I (1977). Based on Tiny BASIC.
          Integer only, minimal strings. SET/RESET block
          graphics (not yet implemented).

  TRS2  - TRS-80 Level II (1978). Full Microsoft BASIC.
          The standard for the TRS-80 family. WHILE/WEND
          supported (unusual for 1978).

  GWBS  - GW-BASIC (1983). THE IBM PC BASIC. Sound, graphics,
          PLAY macro language. 104/104 David Ahl games compile.
          "Ok" prompt (lowercase k).

  EC55  - ECMA-55 Minimal BASIC (1978). International standard.
          LET is mandatory. Very restrictive but 100% portable.

  E116  - ECMA-116 Full BASIC (1986). The gold standard.
          Structured control, exception handling, MAT operations.
          BASIC++ targets 100% compliance.

  QBAS  - QBasic (1991). Microsoft's last DOS BASIC. SUB/FUNCTION,
          SELECT CASE, line numbers optional. The bridge to
          modern programming.

  AINT  - Apple II Integer BASIC (1977). Wozniak's hand-assembled
          masterpiece. Integer only, '>' prompt. LET mandatory.
          GR/PLOT/HLIN/VLIN for lo-res graphics (not yet).

  ASFT  - AppleSoft BASIC (1978). Microsoft BASIC for Apple II.
          ']' prompt. HGR/HPLOT for hi-res graphics (not yet).
          ONERR GOTO error handling.

  ATRI  - Atari BASIC (1979). NOT Microsoft — Shepardson
          Microsystems wrote it. Tokenized storage. 10-column
          zones. GRAPHICS modes 0-8 (not yet). A$(m,n) for
          substrings instead of MID$.

  C64B  - Commodore BASIC v2 (1982). "READY." with period.
          No graphics/sound commands — must PEEK/POKE the
          SID and VIC-II chips directly. 38911 bytes free.

  COCO  - Color Computer BASIC (1980). Motorola 6809 CPU.
          "OK" (uppercase). PLAY music macros, graphics
          commands (PSET, LINE, CIRCLE). PMODE for modes.

  SINC  - Sinclair BASIC (1982). ZX Spectrum, Z80 CPU.
          Integer and float, ":" separator. CLS available.
          RANDOMIZE USR address, BEEP duration, pitch.

  SQLB  - SuperBASIC (1984). Sinclair QL, 68008 CPU.
          Structured programming: DEF PROCedure, DEF
          FuNction, IF/END IF, FOR/END FOR, REPeat/
          END REPeat, SELect ON.

  MBAS  - MBASIC (1977). Microsoft BASIC for CP/M.
          The original Microsoft BASIC. Ran on 8080/Z80.
          LET optional, ':' separator, 65529 max line.

  SUPB  - SUPER BASIC (1968). Tymshare SDS-940 mainframe.
          One of the most advanced BASICs of the 1960s.
          Extended math (ASIN, ACOS, SINH, COSH, TANH,
          LOG10, LOG2, COMP, PDIF, PI), JOSS-style
          UNLESS statement, BY keyword (FOR step),
          complex number support. Max line 99999.

====================================================================
PART 4: HOW TO MODIFY AN EXISTING DIALECT
====================================================================

Suppose you want to modify the Commodore 64 dialect to add a
missing feature. Here's the process:

STEP 1: Open dialect_c64.c

STEP 2: Read the "WHAT'S MISSING" section. It lists every
feature needed for 100% compatibility, with Fix: suggestions.

STEP 3: Modify the DialectConfig struct. For example, if you
implement RND(0) repeat behavior:

    Before: 1,  /* has_rnd_function */
    After:  1,  /* has_rnd_function + rnd_zero_repeat */

    (You'd also add has_rnd_zero_repeat to DialectConfig in
    dialect.h first.)

STEP 4: If you need new keywords:
    a. Add KW_MYFEATURE to the KeywordId enum in lexer.h
    b. Add it to the keyword_table[] in lexer.c with the
       correct DFLAG_ bitmask
    c. Add the handler in parser.c

STEP 5: If you need dialect-specific runtime behavior:
    a. Add a function in the dialect file:
       void dialect_c64_apply(void) { ... }
    b. Call it from dialect_apply() in dialect.c
    c. This function can set up VDev devices, configure
       the virtual memory map, etc.

STEP 6: Rebuild BASIC++. Test with DIALECT "C64B".

IMPORTANT: You are editing ONE FILE (dialect_c64.c) plus the
feature implementation files (parser.c, lexer.c). You do NOT
need to touch any other dialect file. Other contributors
working on the Atari or Apple II dialects won't conflict.

====================================================================
PART 5: HOW TO CREATE A NEW DIALECT
====================================================================

Let's walk through creating a new dialect from scratch. We'll
create "BBC BASIC" for the BBC Micro (Acorn, 1981).

STEP 1: Choose a 4-character short code.

    BBC BASIC -> "BBCB"

STEP 2: Add the dialect ID to dialect.h.

    Open dialect.h and add to the DialectId enum (before
    DIALECT_COUNT):

        DIALECT_BBC_MICRO,    /* BBC BASIC (Acorn) */

    Add a DFLAG bitmask (use next available bit):

        #define DFLAG_BBCB  (1U << 12)  /* BBC BASIC */

    Add the registration function prototype:

        void dialect_register_bbcb(void);

STEP 3: Create dialect_bbcb.c.

    Copy any existing dialect file as a template. A good
    starting point for BBC BASIC would be dialect_gwbs.c
    (similar feature set). Replace:

    - All GW-BASIC references with BBC BASIC info
    - Machine specs (6502A, 32KB, Teletext display, etc.)
    - Feature flags to match BBC BASIC behavior
    - Short name: "BBCB"
    - Dialect flag: DFLAG_BBCB
    - Registration function: dialect_register_bbcb()

STEP 4: Wire it up.

    In dialect.c, add to dialect_register_all():

        dialect_register_bbcb();

STEP 5: Add BBC-specific keywords (if any).

    BBC BASIC has unique keywords like:
      REPEAT/UNTIL, PROC/ENDPROC, LOCAL, ENVELOPE,
      MODE, COLOUR, GCOL, MOVE, PLOT, DRAW, VDU

    For each keyword:
    a. Add KW_xxx to lexer.h
    b. Add to keyword_table[] in lexer.c with DFLAG_BBCB
    c. Add handler in parser.c

STEP 6: Update the Makefile.

    Add dialect_bbcb.c to DIALECT_SOURCES in the Makefile.

STEP 7: Rebuild and test.

    > cl /nologo /W4 /O2 /Fe:basicpp.exe *.c
    > echo DIALECT "BBCB" | .\basicpp.exe
    Dialect: BBC BASIC [BBCB]

That's it. Your new dialect is live. You can now use
OPTION STRICT to enforce BBC BASIC keyword restrictions.

====================================================================
PART 6: UNDERSTANDING THE DIALECTCONFIG STRUCT
====================================================================

The DialectConfig struct (defined in dialect.h) is the heart of
the dialect system. Every field is documented here:

--- IDENTITY ---

  id                DialectId enum value (e.g., DIALECT_GW_BASIC)
  name              Display name (e.g., "GW-BASIC")
  short_name        4-char code (e.g., "GWBS") — used in DIALECT cmd

--- SYNTAX RULES ---

  stmt_separator    Character that separates statements on one line.
                    ':' for most dialects, ';' for Palo Alto Tiny.

  has_then_keyword  1 = THEN required in IF statements.
                    0 = IF A>5 PRINT "BIG" is valid (PATB style).

  has_let_optional  1 = LET is optional (A=5 is valid).
                    0 = LET is mandatory (must write LET A=5).
                    ECMA-55 and Apple Integer require LET.

  not_eq_is_hash    1 = '#' means not-equal (A#B = A<>B).
                    Only Palo Alto Tiny BASIC uses this.

  max_line_number   Highest valid line number.
                    32767 (PATB, TRS), 63999 (Apple, C64, CoCo),
                    65529 (GW-BASIC, QBasic), 99999 (ECMA).

--- FEATURE FLAGS ---

  has_for_next          FOR/NEXT loops
  has_string_vars       String variables (A$, B$, ...)
  has_print_hash        PRINT # field width (PATB only)
  has_array_at          @() array syntax (PATB only)
  has_rnd_function      RND() function
  has_abs_function      ABS() function
  has_size_function     SIZE function (free memory)
  has_data_read         DATA/READ/RESTORE
  has_while_wend        WHILE/WEND loops
  has_do_loop           DO/LOOP WHILE/UNTIL
  has_extended_vars     Long variable names (SCORE, COUNT, ...)
  has_merge_chain       MERGE/CHAIN commands
  has_float             Floating-point arithmetic
  has_dim_arrays        DIM arrays
  has_string_functions  LEFT$, RIGHT$, MID$, etc.
  has_on_error          ON ERROR GOTO / RESUME
  has_cls               CLS (clear screen)
  has_tron_troff        TRON/TROFF (trace on/off)

--- DISPLAY ---

  ready_prompt      Text printed after command execution.
                    "Ok" (GW-BASIC), "READY." (C64), "]" (Apple), etc.

  print_zone_width  Column width for comma-separated PRINT items.
                    14 (Microsoft standard), 10 (Atari, C64), 8 (PATB).

--- KEYWORD FILTERING ---

  dialect_flag      Bitmask identifying this dialect. Used by
                    OPTION STRICT to filter which keywords are allowed.
                    See PART 7 for details.

====================================================================
PART 7: THE KEYWORD BITMASK SYSTEM
====================================================================

Every keyword in BASIC++ has a dialect bitmask that specifies
which dialects support it. This lives in lexer.c:

    { "WHILE",  KW_WHILE,  DFLAG_STRUCT },
    { "CIRCLE", KW_CIRCLE, DFLAG_GWQB },
    { "CLR",    KW_CLR,    DFLAG_C64B | DFLAG_ATRI },

The bitmask flags (defined in dialect.h) are:

    DFLAG_ALL     All dialects (PRINT, LET, IF, etc.)
    DFLAG_PATB    Palo Alto Tiny BASIC only
    DFLAG_TRS1    TRS-80 Level I
    DFLAG_TRS2    TRS-80 Level II
    DFLAG_GWBS    GW-BASIC
    DFLAG_EC55    ECMA-55
    DFLAG_E116    ECMA-116
    DFLAG_QBAS    QBasic
    DFLAG_AINT    Apple Integer
    DFLAG_ASFT    AppleSoft
    DFLAG_ATRI    Atari BASIC
    DFLAG_C64B    Commodore BASIC v2
    DFLAG_COCO    CoCo BASIC
    DFLAG_MBAS    MBASIC (CP/M)
    DFLAG_SINC    Sinclair BASIC
    DFLAG_SUPA    SuperBASIC (QL)
    DFLAG_SBAS    SUPER BASIC (Tymshare)

And convenience groups:

    DFLAG_MSALL     All Microsoft BASIC family + TRS-80
    DFLAG_MSBASIC   TRS2+GWBS+QBAS+ASFT+C64B+COCO+MBAS
    DFLAG_GWQB      GW-BASIC + QBasic
    DFLAG_STRUCT    GWBS + QBAS + E116 + MBAS + SUPA (structured flow)

HOW IT WORKS:

  In normal mode (OPTION STRICT OFF), all keywords from all
  dialects are available. You can use WHILE (GW-BASIC) alongside
  CLR (Commodore) alongside GR (Apple) in the same program.

  When you enable OPTION STRICT, only keywords matching the
  active dialect's bitmask are allowed:

      DIALECT "C64B"
      OPTION STRICT ON       Commodore strict mode
      WHILE I < 10           ERROR! WHILE not in C64B
      CLR                    OK — CLR is in C64B

  This is how you verify that a program will run on the target
  machine. OPTION STRICT is your compatibility checker.

TO ADD A KEYWORD TO YOUR DIALECT:

  1. Find the keyword in lexer.c keyword_table[]
  2. Add your DFLAG_ to its bitmask:

     Before: { "CIRCLE", KW_CIRCLE, DFLAG_GWQB },
     After:  { "CIRCLE", KW_CIRCLE, DFLAG_GWQB | DFLAG_COCO },

  This makes CIRCLE available in CoCo strict mode.

====================================================================
PART 8: ADDING DIALECT-SPECIFIC FEATURES
====================================================================

Some features require more than just a config flag. They need
new keywords, new parser handlers, and sometimes new runtime
support. Here's the general process:

EXAMPLE: Adding Atari GRAPHICS command

  1. ADD THE KEYWORD to lexer.h:

     KW_GRAPHICS,  /* Atari GRAPHICS mode switch */

  2. ADD TO KEYWORD TABLE in lexer.c:

     { "GRAPHICS", KW_GRAPHICS, DFLAG_ATRI },

  3. ADD PARSER HANDLER in parser.c:

     case KW_GRAPHICS:
         parse_atari_graphics(lex, rt, line_num);
         break;

     static void parse_atari_graphics(Lexer *lex,
         RuntimeState *rt, int line_num)
     {
         /* Parse: GRAPHICS n  (n = 0-8) */
         long mode = parse_expression(lex, rt);
         if (mode < 0 || mode > 8) {
             error_raise(ERR_HOW, line_num);
             return;
         }
         /* Map Atari modes to gfxbuf resolutions */
         switch ((int)mode) {
             case 0: /* text 40x24 */ break;
             case 3: gfxbuf_set_mode(40, 24, 4); break;
             case 7: gfxbuf_set_mode(160, 96, 4); break;
             case 8: gfxbuf_set_mode(320, 192, 2); break;
             /* etc. */
         }
     }

  4. UPDATE THE DIALECT FILE (dialect_atari.c):

     Move GRAPHICS from "WHAT'S MISSING" to "WHAT'S IMPLEMENTED"
     in the header comment.

  5. REBUILD AND TEST:

     DIALECT "ATRI"
     GRAPHICS 8
     PLOT 160, 96
     DRAWTO 0, 0

EXAMPLE: Adding Apple II HGR (hi-res graphics)

  1. Add KW_HGR, KW_HGR2, KW_HPLOT, KW_HCOLOR to lexer.h
  2. Tag with DFLAG_ASFT | DFLAG_AINT in lexer.c
  3. Parser handler: HGR switches gfxbuf to 280x192 mode
  4. HPLOT x,y TO x2,y2: Bresenham line in gfxbuf
  5. HCOLOR=n: set drawing color (0-7, NTSC artifact colors)

The key insight: the parser is DIALECT-AGNOSTIC. It doesn't
have if(dialect==ATARI) checks. Instead, keywords are gated
by the bitmask system, and feature flags control behavior
through dialect_get_config() queries.

====================================================================
PART 9: ACHIEVING 100% DIALECT COMPATIBILITY
====================================================================

Each dialect file has a "WHAT'S MISSING" section that lists every
feature needed for perfect compatibility. Here's the roadmap:

MACHINE-SPECIFIC GRAPHICS:

  Apple II:   GR, PLOT, HLIN, VLIN, SCRN (lo-res 40x48)
              HGR, HGR2, HPLOT, HCOLOR (hi-res 280x192)
  Atari:      GRAPHICS 0-8, SETCOLOR, COLOR, PLOT, DRAWTO,
              POSITION, LOCATE (multiple modes)
  CoCo:       PMODE, PCLS, CIRCLE, LINE ,BF (filled box)
  GW-BASIC:   CIRCLE, PAINT (flood fill)
  C64:        All via PEEK/POKE to VIC-II ($D000-$D02E)

All graphics map to gfxbuf.c (the virtual framebuffer). The
framebuffer renders to terminal using Unicode half-block
characters. Each machine's resolution and color model is
translated to the gfxbuf abstraction.

MACHINE-SPECIFIC SOUND:

  Atari:      SOUND voice, freq, distortion, volume (4 POKEY voices)
  CoCo:       SOUND freq, duration + PLAY macro language
  GW-BASIC:   SOUND freq, duration + PLAY + BEEP
  C64:        All via POKE to SID ($D400-$D418)

Sound is harder to emulate portably. Current approach: BEEP/SOUND
use the platform speaker on Windows, and are no-ops on other
platforms. Full audio emulation would need a VDev audio device.

DIALECT-SPECIFIC SYNTAX:

  Atari:      A$(5,10) substring syntax (instead of MID$)
  Apple:      ONERR GOTO (instead of ON ERROR GOTO)
  C64:        RND(0) returns last value (not re-seed)
  PATB:       @() array, '#' for <>, ';' separator

These require targeted parser modifications gated by feature
flags. The pattern is always the same:

  1. Add feature flag to DialectConfig
  2. Set flag in the dialect file
  3. Check flag in parser: dialect_get_config()->has_xxx
  4. Implement the behavior

====================================================================
PART 10: TESTING YOUR DIALECT
====================================================================

After modifying or creating a dialect, test it:

1. BASIC SMOKE TEST:

    DIALECT "MYBAS"
    PRINT "HELLO"
    LET A=42
    PRINT A
    FOR I=1 TO 5: PRINT I: NEXT I

2. STRICT MODE TEST:

    OPTION STRICT ON
    WHILE 1: WEND       Should this work in your dialect?
    DO: LOOP             Should this work in your dialect?
    CLR                  Should this work in your dialect?

3. FEATURE FLAG TEST:

    Test every feature flag you've set:
    - Try DATA/READ if has_data_read=1
    - Try WHILE/WEND if has_while_wend=1
    - Try DEF FN if applicable
    - Try DIM arrays if has_dim_arrays=1

4. READY PROMPT TEST:

    Run a command and check the prompt matches your dialect:
    GW-BASIC: "Ok"      C64: "READY."    Apple: "]"

5. COMPILE TEST:

    LOAD a program and COMPILE it to verify the transpiler
    handles your dialect's features correctly.

====================================================================
PART 11: CONTRIBUTING YOUR DIALECT
====================================================================

If you've created a new dialect or improved an existing one,
here's how to contribute:

1. YOUR DIALECT FILE is self-contained. Submit dialect_xxxx.c
   as a single file. It should include:
   - Complete machine history in the header comment
   - All feature flags accurately set
   - WHAT'S IMPLEMENTED / WHAT'S MISSING checklists
   - Your name as contributor in the header

2. PARSER CHANGES go in separate commits. If you added new
   keywords or handlers, those are separate from the dialect
   profile and should be reviewed independently.

3. NAMING CONVENTION:
   - File: dialect_{shortcode}.c (lowercase, 4 chars)
   - Dialect ID: DIALECT_{NAME} (in enum, before DIALECT_COUNT)
   - DFLAG: DFLAG_{SHORTCODE} (uppercase, 4 chars, next bit)
   - Register function: dialect_register_{shortcode}()

4. TEST PROGRAMS: If you have original .BAS programs from the
   target machine, include them as test cases. Programs that
   exercise dialect-specific features are especially valuable.

5. ACCURACY: Each dialect file should strive for 100% accuracy
   relative to the original machine's BASIC. When in doubt,
   test on an emulator (VICE for C64, Altirra for Atari,
   AppleWin for Apple II, etc.) and document the behavior.

====================================================================
END OF TUTORIAL
====================================================================

SEE ALSO:
  A_Getting_Started.txt    - Basics of BASIC++
  C_Variables.txt          - Variable types and naming
  F_Mixing_Dialects.txt    - Using features from multiple dialects
  H_Compiler_Guide.txt     - Transpiling to ANSI C
  J_Virtual_Consoles.txt   - STDOUT/STDERR/STDLOG streams

The dialect system is designed for exactly this kind of community
contribution. Each machine's BASIC was unique, and each deserves
a faithful implementation. Happy hacking!

    @COPYLEFT ALL WRONGS RESERVED
