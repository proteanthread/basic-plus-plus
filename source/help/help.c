/*
 * ---
 * BASIC++ Interpreter - help.c
 * ---
 *
 * Help & introspection system.
 *
 * Makes BASIC++ self-describing: every command, function, and
 * system setting is documented within the interpreter itself.
 *
 * ORGANIZATION:
 * Both HELP and CATALOG group entries by functional category,
 * sorted alphabetically within each category. Categories:
 *
 *   Control Flow       - IF, FOR, GOTO, GOSUB, WHILE, DO, etc.
 *   Input / Output     - PRINT, INPUT, CLS, LOCATE, etc.
 *   Arithmetic / Math  - ABS, SIN, COS, RND, MOD, etc.
 *   String             - LEN, LEFT$, MID$, CHR$, ASC, etc.
 *   Variables & Memory - LET, DIM, PEEK, POKE, SWAP, etc.
 *   File I/O           - OPEN, CLOSE, READ, WRITE, etc.
 *   Graphics           - SCREEN, DRAW, PSET, CIRCLE, etc.
 *   Sound              - BEEP, SOUND, PLAY
 *   System & Environ   - SYSTEM, SHELL, ENVIRON, DIALECT, etc.
 *   Debug & Test       - BREAK, CONT, VARS, ASSERT, etc.
 *   Program Mgmt       - RUN, LIST, SAVE, LOAD, NEW, etc.
 *   Editing            - RENUM, DELETE, EDIT, AUTO
 *   Devices & Network  - VDEV, VNET, DEVMAP, FUJINET, etc.
 *   Operators          - AND, OR, NOT, XOR, MOD, EQV, IMP
 *   Introspection      - HELP, INFO, CATALOG, VER
 *
 * ---
 */

#include <stdio.h>
#include <string.h>
#include "help.h"
#include "config.h"
#include "funcreg.h"
#include "dialect.h"
#include "security.h"
#include "module.h"
#include "memmap.h"

/* --- Help Category IDs ---
 */
typedef enum HelpCatId {
 HCAT_FLOW = 0,   /* Control Flow */
 HCAT_IO,         /* Input / Output */
 HCAT_MATH,       /* Arithmetic / Math */
 HCAT_STRING,     /* String Functions */
 HCAT_VARMEM,     /* Variables & Memory */
 HCAT_FILEIO,     /* File I/O */
 HCAT_GFX,        /* Graphics */
 HCAT_SOUND,      /* Sound */
 HCAT_SYSENV,     /* System & Environment */
 HCAT_DEBUG,      /* Debugging & Testing */
 HCAT_PROGMGMT,   /* Program Management */
 HCAT_EDIT,       /* Editing */
 HCAT_DEVICE,     /* Devices & Networking */
 HCAT_OPER,       /* Operators */
 HCAT_INTRO,      /* Introspection */
 HCAT_COUNT       /* sentinel */
} HelpCatId;

/* Category display names */
static const char *help_cat_names[] = {
 "Control Flow",
 "Input / Output",
 "Arithmetic / Math",
 "String Functions",
 "Variables & Memory",
 "File I/O",
 "Graphics",
 "Sound",
 "System & Environment",
 "Debugging & Testing",
 "Program Management",
 "Editing",
 "Devices & Networking",
 "Operators",
 "Introspection"
};

/* --- Command Help Database ---
 * Each entry maps a keyword to a short description, usage
 * example, and category. Entries within each category are
 * sorted alphabetically by keyword.
 */
typedef struct HelpEntry {
 const char *keyword;
 const char *summary;
 const char *usage;
 HelpCatId   category;
} HelpEntry;

static const HelpEntry help_db[] = {

 /* ===== Control Flow ===== (alphabetical) */
 { "CALL", "Invoke a named subroutine",
 "CALL MySub(10, \"test\")",               HCAT_FLOW },
 { "CASE", "Case clause within SELECT CASE",
 "CASE 1,2,3 or CASE IS > 10 or CASE ELSE", HCAT_FLOW },
 { "DECLARE", "Forward-declare a SUB or FUNCTION",
 "DECLARE SUB MySub(x, y$)",             HCAT_FLOW },
 { "DEF", "Define a user function (one-line)",
 "DEF FNA(X)=X*X+1",                     HCAT_FLOW },
 { "DO", "Begin a DO loop",
 "DO ... LOOP WHILE cond",               HCAT_FLOW },
 { "ELSE", "Alternate branch of IF statement",
 "IF A>5 THEN PRINT \"BIG\" ELSE PRINT \"SMALL\"",
                                          HCAT_FLOW },
 { "ELSEIF", "Additional condition in block IF",
 "ELSEIF X>10 THEN ...",                  HCAT_FLOW },
 { "END", "Terminate program execution",
 "END",                                   HCAT_FLOW },
 { "ENDIF", "End a block IF structure",
 "END IF",                                HCAT_FLOW },
 { "EXIT", "Exit a FOR or DO loop early",
 "EXIT FOR or EXIT DO",                   HCAT_FLOW },
 { "FOR", "Begin a counting loop",
 "FOR I=1 TO 10 STEP 2 (or BY 2 in SBASIC)", HCAT_FLOW },
 { "FUNCTION", "Define a named function",
 "FUNCTION MyFn (x) ... END FUNCTION",   HCAT_FLOW },
 { "GOSUB", "Call a subroutine at a line number",
 "GOSUB 500",                             HCAT_FLOW },
 { "GOTO", "Jump to a line number",
 "GOTO 100",                              HCAT_FLOW },
 { "IF", "Conditional execution (IF/THEN/ELSE)",
 "IF A>5 THEN PRINT \"BIG\" ELSE PRINT \"SMALL\"",
                                          HCAT_FLOW },
 { "LOOP", "End of a DO loop (with condition)",
 "LOOP WHILE A>0 or LOOP UNTIL A=0",     HCAT_FLOW },
 { "NEXT", "End of a FOR loop",
 "NEXT I",                                HCAT_FLOW },
 { "ON", "Computed GOTO/GOSUB or error/event handler",
 "ON X GOTO 100,200 or ON ERROR GOTO 500", HCAT_FLOW },
 { "ON BREAK", "Set Ctrl+C / signal interrupt handler",
 "ON BREAK GOSUB 900",                    HCAT_FLOW },
 { "ON COM", "Set serial port interrupt handler",
 "ON COM(1) GOSUB 800",                   HCAT_FLOW },
 { "ON KEY", "Set key press interrupt handler",
 "ON KEY(1) GOSUB 700",                   HCAT_FLOW },
 { "ON PEN", "Set light pen interrupt handler",
 "ON PEN GOSUB 600",                      HCAT_FLOW },
 { "ON PLAY", "Set music buffer interrupt handler",
 "ON PLAY(3) GOSUB 500",                  HCAT_FLOW },
 { "ON STRIG", "Set joystick trigger interrupt handler",
 "ON STRIG(0) GOSUB 400",                 HCAT_FLOW },
 { "ON TIMER", "Set timer interrupt handler",
 "ON TIMER(5) GOSUB 300",                 HCAT_FLOW },
 { "RETURN", "Return from a GOSUB subroutine",
 "RETURN",                                HCAT_FLOW },
 { "SELECT", "Begin a SELECT CASE block",
 "SELECT CASE X",                         HCAT_FLOW },
 { "STOP", "Pause program (use CONT to resume)",
 "STOP",                                  HCAT_FLOW },
 { "SUB", "Define a named subroutine",
 "SUB MySub (x, y$) ... END SUB",        HCAT_FLOW },
 { "UNLESS", "Negated IF (SBASIC/JOSS-style)",
 "UNLESS X=0 THEN PRINT \"nonzero\"",    HCAT_FLOW },
 { "WEND", "End of a WHILE loop",
 "WEND",                                  HCAT_FLOW },
 { "WHILE", "Begin a conditional loop",
 "WHILE A>0 ... WEND",                   HCAT_FLOW },

 /* ===== Input / Output ===== (alphabetical) */
 { "CLS", "Clear the screen",
 "CLS",                                   HCAT_IO },
 { "COLOR", "Set text foreground/background",
 "COLOR 14,1 (yellow on blue)",           HCAT_IO },
 { "HOME", "Move cursor to top-left",
 "HOME",                                  HCAT_IO },
 { "INKEY$", "Non-blocking keyboard read",
 "K$=INKEY$",                             HCAT_IO },
 { "INPUT", "Read user input into a variable",
 "INPUT A or INPUT \"Name? \";N$",        HCAT_IO },
 { "INPUT$", "Read N chars from keyboard/file",
 "A$ = INPUT$(5) or A$ = INPUT$(10, #1)", HCAT_IO },
 { "LINE", "Read entire line (LINE INPUT)",
 "LINE INPUT A$ or LINE INPUT #1, A$",   HCAT_IO },
 { "LOCATE", "Move cursor to row,column",
 "LOCATE 10,20",                          HCAT_IO },
 { "LPRINT", "Print to standard error",
 "LPRINT \"Status: OK\"",                 HCAT_IO },
 { "PRINT", "Display output to the console",
 "PRINT \"Hello\";A or PRINT A,B",       HCAT_IO },
 { "SPC", "Print N spaces",
 "PRINT SPC(10);\"TEXT\"",                HCAT_IO },
 { "TAB", "Move cursor to column position",
 "PRINT TAB(20);\"HERE\"",               HCAT_IO },
 { "USING", "Format output with template",
 "PRINT USING \"###.##\"; 3.14",         HCAT_IO },
 { "WIDTH", "Set screen width",
 "WIDTH 80 or WIDTH 80,25",              HCAT_IO },
 { "WRITE", "Write comma-delimited data",
 "WRITE #1, A, B$, C",                   HCAT_IO },

 /* ===== Arithmetic / Math ===== (alphabetical) */
 { "ABS", "Absolute value",
 "PRINT ABS(-5)  ' prints 5",            HCAT_MATH },
 { "ACOS", "Arccosine (SBASIC, returns radians)",
 "PRINT ACOS(0)  ' 1.5708 (PI/2)",      HCAT_MATH },
 { "ASIN", "Arcsine (SBASIC, returns radians)",
 "PRINT ASIN(1)  ' 1.5708 (PI/2)",      HCAT_MATH },
 { "ATN", "Arctangent (returns radians)",
 "PI = 4*ATN(1)",                         HCAT_MATH },
 { "CDBL", "Convert to double precision",
 "PRINT CDBL(X)",                         HCAT_MATH },
 { "CINT", "Round to nearest integer",
 "PRINT CINT(3.6)  ' prints 4",          HCAT_MATH },
 { "COMP", "Compare two values (SBASIC, returns -1/0/+1)",
 "PRINT COMP(3,5)  ' -1",               HCAT_MATH },
 { "COS", "Cosine (radians)",
 "PRINT COS(0)  ' prints 1",             HCAT_MATH },
 { "COSH", "Hyperbolic cosine (SBASIC)",
 "PRINT COSH(0)  ' 1",                  HCAT_MATH },
 { "CSNG", "Convert to single precision",
 "PRINT CSNG(X)",                         HCAT_MATH },
 { "EXP", "Exponential (e^x)",
 "PRINT EXP(1)  ' prints 2.71828",       HCAT_MATH },
 { "FIX", "Truncate toward zero",
 "PRINT FIX(-3.7)  ' prints -3",         HCAT_MATH },
 { "INT", "Truncate to integer (toward -infinity)",
 "PRINT INT(3.7)  ' prints 3",           HCAT_MATH },
 { "LOG", "Natural logarithm (base e)",
 "PRINT LOG(2.71828)",                    HCAT_MATH },
 { "LOG10", "Common logarithm base 10 (SBASIC)",
 "PRINT LOG10(100)  ' 2",               HCAT_MATH },
 { "LOG2", "Binary logarithm base 2 (SBASIC)",
 "PRINT LOG2(8)  ' 3",                  HCAT_MATH },
 { "PDIF", "Positive difference (SBASIC, MAX(a-b,0))",
 "PRINT PDIF(5,3)  ' 2",                HCAT_MATH },
 { "PI", "Constant PI (SBASIC, 3.14159...)",
 "PRINT PI  ' 3.14159265358979",         HCAT_MATH },
 { "RANDOMIZE", "Seed the random number generator",
 "RANDOMIZE or RANDOMIZE 42",            HCAT_MATH },
 { "RND", "Random number (0 to 1)",
 "PRINT RND(1) or PRINT INT(RND(1)*6)+1", HCAT_MATH },
 { "SGN", "Sign of a number (-1, 0, or 1)",
 "PRINT SGN(-5)  ' prints -1",           HCAT_MATH },
 { "SIN", "Sine (radians)",
 "PRINT SIN(3.14159/2)",                  HCAT_MATH },
 { "SINH", "Hyperbolic sine (SBASIC)",
 "PRINT SINH(1)  ' 1.1752",             HCAT_MATH },
 { "SQR", "Square root",
 "PRINT SQR(144)  ' prints 12",          HCAT_MATH },
 { "TAN", "Tangent (radians)",
 "PRINT TAN(3.14159/4)",                  HCAT_MATH },
 { "TANH", "Hyperbolic tangent (SBASIC)",
 "PRINT TANH(1)  ' 0.7616",             HCAT_MATH },

 /* ===== String Functions ===== (alphabetical) */
 { "ASC", "ASCII code of first character (array unpack form)",
 "X=ASC(\"A\") '65 | DIM A(10): A(0)=ASC(\"HELLO\") 'unpack to A(0..4)",
                                          HCAT_STRING },
 { "BIN$", "Convert to binary string (8-bit groups)",
 "PRINT BIN$(255)  ' 11111111",          HCAT_STRING },
 { "CHR$", "Character from ASCII code",
 "PRINT CHR$(65)  ' A",                  HCAT_STRING },
 { "HEX$", "Convert to hexadecimal string",
 "PRINT HEX$(255)  ' FF",               HCAT_STRING },
 { "INSTR", "Find substring position",
 "PRINT INSTR(\"HELLO\",\"LL\")  ' 3",   HCAT_STRING },
 { "LCASE$", "Convert string to lowercase",
 "PRINT LCASE$(\"HELLO\")  ' hello",     HCAT_STRING },
 { "LEFT$", "Left N characters",
 "PRINT LEFT$(\"HELLO\",3)  ' HEL",     HCAT_STRING },
 { "LEN", "Length of a string",
 "PRINT LEN(\"HELLO\")  ' prints 5",    HCAT_STRING },
 { "LTRIM$", "Remove leading spaces",
 "PRINT LTRIM$(\"  hello\")",            HCAT_STRING },
 { "MID$", "Substring from position",
 "PRINT MID$(\"HELLO\",2,3)  ' ELL",    HCAT_STRING },
 { "OCT$", "Convert to octal string",
 "PRINT OCT$(255)  ' 377",              HCAT_STRING },
 { "RIGHT$", "Right N characters",
 "PRINT RIGHT$(\"HELLO\",3)  ' LLO",    HCAT_STRING },
 { "RTRIM$", "Remove trailing spaces",
 "PRINT RTRIM$(\"hello  \")",            HCAT_STRING },
 { "SPACE$", "Generate N space characters",
 "PRINT SPACE$(10)",                      HCAT_STRING },
 { "STR$", "Convert number to string",
 "A$=STR$(42)",                           HCAT_STRING },
 { "STRING$", "Generate N copies of a character",
 "PRINT STRING$(10,\"*\")",              HCAT_STRING },
 { "TCASE$", "Convert string to title case",
 "PRINT TCASE$(\"hello world\")",        HCAT_STRING },
 { "TRIM$", "Remove leading and trailing spaces",
 "PRINT TRIM$(\"  hello  \")  ' hello",  HCAT_STRING },
 { "UCASE$", "Convert string to uppercase",
 "PRINT UCASE$(\"hello\")  ' HELLO",    HCAT_STRING },
 { "VAL", "Convert string to number",
 "A=VAL(\"42\")",                         HCAT_STRING },

 /* ===== Variables & Memory ===== (alphabetical) */
 { "CLEAR", "Clear stack and variables",
 "CLEAR",                                 HCAT_VARMEM },
 { "CLR", "Clear all variables (Atari/C64)",
 "CLR",                                   HCAT_VARMEM },
 { "COMMON", "Share vars with CHAINed program",
 "COMMON A, B$",                          HCAT_VARMEM },
 { "CONST", "Define a named constant",
 "CONST PI = 3.14159",                    HCAT_VARMEM },
 { "CSRLIN", "Current cursor row",
 "PRINT CSRLIN",                          HCAT_VARMEM },
 { "DATA", "Embed data values in the program",
 "DATA 1,2,3,\"HELLO\"",                 HCAT_VARMEM },
 { "DEFDBL", "Declare double variable range",
 "DEFDBL A-Z",                            HCAT_VARMEM },
 { "DEFINT", "Declare integer variable range",
 "DEFINT A-Z",                            HCAT_VARMEM },
 { "DEFSNG", "Declare single variable range",
 "DEFSNG A-Z",                            HCAT_VARMEM },
 { "DEFSTR", "Declare string variable range",
 "DEFSTR A-Z",                            HCAT_VARMEM },
 { "DIM", "Declare an array with dimensions",
 "DIM A(10) or DIM M(3,3) or DIM A$(25)", HCAT_VARMEM },
 { "ERASE", "Erase an array from memory",
 "ERASE A",                               HCAT_VARMEM },
 { "FRE", "Free memory available",
 "PRINT FRE(0)",                          HCAT_VARMEM },
 { "LET", "Assign a value to a variable",
 "LET A=10 or LET A$=\"HELLO\"",         HCAT_VARMEM },
 { "MAT", "Matrix operations",
 "MAT PRINT A or MAT A=ZER",             HCAT_VARMEM },
 { "MEMMAP", "Select memory map preset",
 "MEMMAP 48 or MEMMAP 64",               HCAT_VARMEM },
 { "OPTION", "Set interpreter options",
 "OPTION BASE 0|1 | STRICT ON|OFF | MIXED D1,D2,...", HCAT_VARMEM },
 { "PEEK", "Read byte from virtual memory",
 "PRINT PEEK(addr)",                      HCAT_VARMEM },
 { "POKE", "Write byte to virtual memory",
 "POKE addr, value",                      HCAT_VARMEM },
 { "POS", "Current cursor column",
 "PRINT POS(0)",                          HCAT_VARMEM },
 { "READ", "Read the next DATA value",
 "READ A or READ A$",                    HCAT_VARMEM },
 { "REDIM", "Resize a dynamic array",
 "REDIM A(20)",                           HCAT_VARMEM },
 { "REM", "Comment (ignored by interpreter)",
 "REM This is a comment (or ' shorthand)", HCAT_VARMEM },
 { "RESTORE", "Reset DATA pointer to beginning",
 "RESTORE or RESTORE 200",               HCAT_VARMEM },
 { "SHARED", "Share variables with main program",
 "SHARED A, B$",                          HCAT_VARMEM },
 { "SIZE", "Show free memory (Tiny BASIC)",
 "SIZE",                                  HCAT_VARMEM },
 { "STATIC", "Preserve local variables across calls",
 "STATIC count",                          HCAT_VARMEM },
 { "SWAP", "Exchange two variables",
 "SWAP A,B",                              HCAT_VARMEM },
 { "TYPE", "Define a user-defined type",
 "TYPE recname ... END TYPE",             HCAT_VARMEM },
 { "VARPTR", "Pointer index of a variable",
 "PRINT VARPTR(A)",                       HCAT_VARMEM },

 /* ===== File I/O ===== (alphabetical) */
 { "ASK", "Query file info (ECMA-116)",
 "ASK #1: FILESIZE F",                   HCAT_FILEIO },
 { "BLOAD", "Load compiled bytecode",
 "BLOAD \"prog.bpp\"",                   HCAT_FILEIO },
 { "BSAVE", "Save compiled bytecode",
 "BSAVE \"prog.bpp\"",                   HCAT_FILEIO },
 { "CLOSE", "Close a file channel",
 "CLOSE #1",                              HCAT_FILEIO },
 { "CVD", "Unpack 8-byte string to double",
 "D = CVD(A$)",                           HCAT_FILEIO },
 { "CVI", "Unpack 2-byte string to integer",
 "N = CVI(A$)",                           HCAT_FILEIO },
 { "CVS", "Unpack 4-byte string to single",
 "F = CVS(A$)",                           HCAT_FILEIO },
 { "EOF", "Check for end of file",
 "IF EOF(1) THEN PRINT \"Done\"",        HCAT_FILEIO },
 { "FIELD", "Define record buffer fields",
 "FIELD #1, 20 AS N$, 4 AS A$",          HCAT_FILEIO },
 { "GET", "Read a record from random file",
 "GET #1, recnum",                        HCAT_FILEIO },
 { "LOC", "Current file position",
 "PRINT LOC(1)",                          HCAT_FILEIO },
 { "LOCK", "Lock file records",
 "LOCK #1, record",                       HCAT_FILEIO },
 { "LOF", "Get length of open file",
 "PRINT LOF(1)",                          HCAT_FILEIO },
 { "LSET", "Left-justify in field buffer",
 "LSET N$ = \"Smith\"",                  HCAT_FILEIO },
 { "MKD$", "Pack double to 8-byte string",
 "A$ = MKD$(D)",                          HCAT_FILEIO },
 { "MKI$", "Pack integer to 2-byte string",
 "A$ = MKI$(N)",                          HCAT_FILEIO },
 { "MKS$", "Pack single to 4-byte string",
 "A$ = MKS$(F)",                          HCAT_FILEIO },
 { "OPEN", "Open a file channel",
 "OPEN \"data.txt\" FOR INPUT AS #1",    HCAT_FILEIO },
 { "PUT", "Write a record to random file",
 "PUT #1, recnum",                        HCAT_FILEIO },
 { "RESET", "Close all open files",
 "RESET",                                 HCAT_FILEIO },
 { "RSET", "Right-justify in field buffer",
 "RSET N$ = \"Smith\"",                  HCAT_FILEIO },
 { "SEEK", "Set or get file position",
 "SEEK #1, 100",                          HCAT_FILEIO },
 { "SET", "Set file pointer (ECMA-116)",
 "SET #1: POINTER 0",                    HCAT_FILEIO },
 { "UNLOCK", "Unlock file records",
 "UNLOCK #1, record",                     HCAT_FILEIO },

 /* ===== Graphics ===== (alphabetical) */
 { "CIRCLE", "Draw a circle",
 "CIRCLE (160,100), 50",                  HCAT_GFX },
 { "DRAW", "Draw using graphics macros",
 "DRAW \"R10 D10 L10 U10\"",             HCAT_GFX },
 { "PAINT", "Flood fill an area",
 "PAINT (160,100), color",                HCAT_GFX },
 { "PALETTE", "Remap a palette color",
 "PALETTE colornum, newcolor",            HCAT_GFX },
 { "POINT", "Read color at pixel (x,y)",
 "C = POINT(100,50)",                     HCAT_GFX },
 { "PRESET", "Reset a pixel at (x,y)",
 "PRESET (100,50)",                       HCAT_GFX },
 { "PSET", "Set a pixel at (x,y)",
 "PSET (100,50), color",                  HCAT_GFX },
 { "SCREEN", "Set screen mode",
 "SCREEN mode [,color]",                  HCAT_GFX },
 { "VIEW", "Set graphics viewport",
 "VIEW (x1,y1)-(x2,y2)",                 HCAT_GFX },
 { "WINDOW", "Set logical coordinate system",
 "WINDOW (0,0)-(639,199)",               HCAT_GFX },

 /* ===== Sound ===== (alphabetical) */
 { "BEEP", "Emit an audible beep",
 "BEEP",                                  HCAT_SOUND },
 { "PLAY", "Play music macro language",
 "PLAY \"T120 O4 L4 CDEFGAB\"",          HCAT_SOUND },
 { "SOUND", "Play a tone at frequency",
 "SOUND 440, 18",                         HCAT_SOUND },

 /* ===== System & Environment ===== (alphabetical) */
 { "ALARM$", "Get/set alarm time",
 "ALARM$=\"23:00:00\" or PRINT ALARM$",  HCAT_SYSENV },
 { "BYE", "Exit the interpreter",
 "BYE",                                   HCAT_SYSENV },
 { "CHAIN", "Load and run another program",
 "CHAIN \"next.bas\"",                    HCAT_SYSENV },
 { "CHDIR", "Change current directory",
 "CHDIR \"C:\\GAMES\"",                   HCAT_SYSENV },
 { "CLOCK$", "Full timestamp string",
 "PRINT CLOCK$  ' YYYY-MM-DD HH:MM:SS",  HCAT_SYSENV },
 { "COPY", "Copy a file (no SHELL needed)",
 "COPY \"src.bas\" TO \"dst.bas\"",           HCAT_SYSENV },
 { "CURDIR$", "Current directory (alias for CWD$)",
 "PRINT CURDIR$",                          HCAT_SYSENV },
 { "CWD$", "Current working directory string",
 "A$ = CWD$ or PRINT CWD$",               HCAT_SYSENV },
 { "DATE$", "Current date string",
 "PRINT DATE$",                           HCAT_SYSENV },
 { "DIALECT", "Set the BASIC dialect",
 "DIALECT \"PATB\" or DIALECT",           HCAT_SYSENV },
 { "DIR", "List filenames (no details)",
 "DIR or DIR \"*.BAS\"",                  HCAT_SYSENV },
 { "ENVIRON", "Read environment variable",
 "A$=ENVIRON$(\"PATH\")",                HCAT_SYSENV },
 { "ERRORLEVEL","Last shell exit code",
 "PRINT ERRORLEVEL",                      HCAT_SYSENV },
 { "EXEC", "Fire-and-forget OS command",
 "EXEC \"notepad\"",                      HCAT_SYSENV },
 { "EXIST", "Check if a file exists (returns 1 or 0)",
 "IF EXIST(\"GAME.BAS\") THEN LOAD \"GAME.BAS\"",
                                          HCAT_SYSENV },
 { "FILELEN", "Get file size in bytes (-1 if not found)",
 "PRINT FILELEN(\"DATA.BIN\")",            HCAT_SYSENV },
 { "FILES", "List files with sizes (native, no SHELL)",
 "FILES or FILES \"*.BAS\"",              HCAT_SYSENV },
 { "KILL", "Reserved for future use",
 "KILL (use SCRATCH to delete files)",   HCAT_SYSENV },
 { "SCRATCH", "Delete a file from disk",
 "SCRATCH \"temp.dat\"",                  HCAT_SYSENV },
 { "MKDIR", "Create a new directory",
 "MKDIR \"newdir\"",                      HCAT_SYSENV },
 { "MOVE", "Move a file (no SHELL needed)",
 "MOVE \"src.bas\" TO \"subdir/dst.bas\"", HCAT_SYSENV },
 { "MODULE", "Activate/list modules",
 "MODULE \"stdlib\" or MODULE",           HCAT_SYSENV },
 { "NAME", "Rename a file (GW-BASIC)",
 "NAME \"old.bas\" AS \"new.bas\"",       HCAT_SYSENV },
 { "RENAME", "Rename a file",
 "RENAME \"old.bas\",\"new.bas\"",        HCAT_SYSENV },
 { "PWD", "Print current working directory",
 "PWD",                                   HCAT_SYSENV },
 { "RMDIR", "Remove a directory",
 "RMDIR \"olddir\"",                      HCAT_SYSENV },
 { "SECURITY", "Set security level (one-way ratchet)",
 "SECURITY 0|1|2 | \"OPEN\"|\"STANDARD\"|\"RESTRICTED\" | LEVEL n",
                                          HCAT_SYSENV },
 { "SHELL", "Execute an OS command",
 "SHELL \"dir\" or A$=SHELL$(\"date /t\")",
                                          HCAT_SYSENV },
 { "SLEEP", "Pause execution for N seconds",
 "SLEEP 2",                               HCAT_SYSENV },
 { "SYSTEM", "Query platform info",
 "SYSTEM \"OS\" or SYSTEM \"ARCH\"",      HCAT_SYSENV },
 { "TIME$", "Current time string",
 "PRINT TIME$",                           HCAT_SYSENV },
 { "TIMER", "Seconds elapsed since midnight (also: event trap)",
 "PRINT TIMER | TIMER ON | TIMER OFF | TIMER STOP",
                                          HCAT_SYSENV },
 { "KEY(n)", "Enable/disable key event trapping",
 "KEY(1) ON | KEY(1) OFF | KEY(1) STOP", HCAT_SYSENV },
 { "PLAY(n)", "Enable/disable music buffer interrupt",
 "PLAY ON | PLAY OFF | PLAY STOP",       HCAT_SYSENV },

 /* ===== Error Handling ===== */
 { "CAUSE", "Raise an exception (ECMA-116)",
 "CAUSE EXCEPTION 1000",                  HCAT_FLOW },
 { "ERL", "Line number of last error",
 "PRINT ERL",                             HCAT_FLOW },
 { "ERR", "Error code of last error",
 "PRINT ERR",                             HCAT_FLOW },
 { "ERROR", "Raise a user error",
 "ERROR 5",                               HCAT_FLOW },
 { "RESUME", "Return from an error handler",
 "RESUME or RESUME NEXT or RESUME 100",  HCAT_FLOW },
 { "WHEN", "Begin protected exception block",
 "WHEN EXCEPTION IN ... USE ... END WHEN", HCAT_FLOW },
 { "TRAP", "Set error/event trap handler (hybrid)",
 "TRAP n | TRAP e,s,n | TRAP (status) | ON TRAP GOSUB n", HCAT_FLOW },

 /* ===== Debugging & Testing ===== (alphabetical) */
 { "ASSERT", "Test an assertion (pass/fail)",
 "ASSERT 2+3=5 or ASSERT A>0",           HCAT_DEBUG },
 { "BREAK", "Set/clear/list breakpoints",
 "BREAK 100 | BREAK -100 | BREAK",       HCAT_DEBUG },
 { "CONT", "Continue from STOP/BREAK",
 "CONT",                                  HCAT_DEBUG },
 { "ENDTEST", "End test block and report",
 "ENDTEST",                               HCAT_DEBUG },
 { "SELFTEST", "Run built-in validation suite",
 "SELFTEST",                              HCAT_DEBUG },
 { "TEST", "Start a named test block",
 "TEST \"Arithmetic\"",                   HCAT_DEBUG },
 { "TROFF", "Disable line trace output",
 "TROFF",                                 HCAT_DEBUG },
 { "TRON", "Enable line trace output",
 "TRON",                                  HCAT_DEBUG },
 { "VARS", "Display variables and environment",
 "VARS [USER|ENV|SYSTEM|ALL] - program, user, OS, or BASIC++ env",
                                          HCAT_DEBUG },

 /* ===== Program Management ===== (alphabetical) */
 { "COMPILE", "Transpile BASIC to C source",
 "COMPILE \"output.c\"",                  HCAT_PROGMGMT },
 { "LIST", "Display program lines",
 "LIST or LIST 100-200",                  HCAT_PROGMGMT },
 { "LLIST", "List program to printer",
 "LLIST or LLIST 100-200",               HCAT_PROGMGMT },
 { "LOAD", "Load program from a file",
 "LOAD \"prog.bas\"",                    HCAT_PROGMGMT },
 { "MERGE", "Merge program lines from file",
 "MERGE \"extra.bas\"",                  HCAT_PROGMGMT },
 { "NEW", "Clear program from memory",
 "NEW",                                   HCAT_PROGMGMT },
 { "RUN", "Execute the stored program",
 "RUN",                                   HCAT_PROGMGMT },
 { "SAVE", "Save program to a file (auto-names if bare)",
 "SAVE [\"file\"]  Bare SAVE => DATE$TIME$.BAS", HCAT_PROGMGMT },
 { "UNSAVE", "Delete the last saved file",
 "UNSAVE  Deletes last SAVE/LOAD filename",     HCAT_PROGMGMT },

 /* ===== Editing ===== (alphabetical) */
 { "ALIAS", "Remap keywords, operators, load language packs",
 "ALIAS PRINT = \"X\" | LIST | CLEAR | COUNT | REMOVE | SAVE | LOAD | LANG",
                                          HCAT_EDIT },
 { "ALIAS$", "Lookup alias<->keyword mapping (read-only)",
 "PRINT ALIAS$(\"IMPRE\") or ALIAS$(\"PRINT\")",
                                          HCAT_EDIT },
 { "SCOPE", "Keyword access control and behavior hooks",
 "SCOPE DISABLE kw | BEFORE kw GOSUB n | OVERRIDE | \"preset\"",
                                          HCAT_EDIT },
 { "KEYWORD", "Modify built-in keyword behavior",
 "KEYWORD PRINT UPPERCASE ON | KEYWORD kw DESCRIBE",
                                          HCAT_EDIT },
 { "OVERRIDE", "Change how a keyword is interpreted (no source changes)",
 "OVERRIDE PRINT \"PRINT TAB(7);\" | OVERRIDE LIST | OVERRIDE RESET",
                                          HCAT_EDIT },
 { "AUTO", "Auto-generate line numbers",
 "AUTO or AUTO 100,5",                    HCAT_EDIT },
 { "DELETE", "Delete lines by range",
 "DELETE 100 or DELETE 100-200",          HCAT_EDIT },
 { "EDIT", "Edit a program line",
 "EDIT 100",                              HCAT_EDIT },
 { "RENUM", "Renumber program lines",
 "RENUM or RENUM 100,5",                 HCAT_EDIT },

 /* ===== Devices & Networking ===== (alphabetical) */
 { "CLOCK:", "FujiNet NTP clock device",
 "Read time via CLOCK: device (ISO/binary formats)",
                                          HCAT_DEVICE },
 { "DEVMAP", "Device slot mapping",
 "DEVMAP - file channels + VDev slot table", HCAT_DEVICE },
 { "FUJI:", "FujiNet configuration device",
 "WiFi config, host slots, AppKey storage", HCAT_DEVICE },
 { "FUJINET", "FujiNet virtual device module",
 "MODULE \"FUJINET\" to activate N:/FUJI:/CLOCK: devices",
                                          HCAT_DEVICE },
 { "IOCTL", "Device control command",
 "IOCTL #ch, cmd [, arg]",               HCAT_DEVICE },
 { "N:", "FujiNet network device",
 "OPEN #1,\"N:TCP://host:port/\",\"RW\" then READ/WRITE",
                                          HCAT_DEVICE },
 { "SYS", "Call machine language routine",
 "SYS addr",                              HCAT_DEVICE },
 { "VCON", "Virtual console information",
 "VCON - console device, input/output capabilities",
                                          HCAT_DEVICE },
 { "VDEV", "List registered virtual devices",
 "VDEV - shows slot, name, class, caps, description",
                                          HCAT_DEVICE },
 { "VMACH", "Virtual machine state",
 "VMACH - VM state, opcodes, modules, security",
                                          HCAT_DEVICE },
 { "VMEM", "Virtual memory status",
 "VMEM - memory map, pools, stack, program storage",
                                          HCAT_DEVICE },
 { "VNET", "Virtual network status",
 "VNET - FujiNet channels, connections, adapter info",
                                          HCAT_DEVICE },
 { "VTERM", "Virtual terminal information",
 "VTERM - screen size, cursor, encoding, dialect",
                                          HCAT_DEVICE },
 { "UPNP:", "UPnP/SSDP network device discovery",
 "MODULE \"UPNP\" to activate UPNP: and SOAP: devices",
                                          HCAT_DEVICE },
 { "WAIT", "Wait for port status",
 "WAIT port, AND_mask [,XOR_mask]",       HCAT_DEVICE },

 /* ===== Operators ===== (alphabetical) */
 { "AND", "Logical/bitwise AND operator",
 "IF A>0 AND B>0 THEN ...",               HCAT_OPER },
 { "EQV", "Logical equivalence operator",
 "PRINT 5 EQV 3",                        HCAT_OPER },
 { "IMP", "Logical implication operator",
 "PRINT 5 IMP 3",                        HCAT_OPER },
 { "MOD", "Modulo (remainder) operator",
 "PRINT 10 MOD 3  ' prints 1",           HCAT_OPER },
 { "NOT", "Logical/bitwise NOT operator",
 "IF NOT A THEN ...",                     HCAT_OPER },
 { "OR", "Logical/bitwise OR operator",
 "IF A=0 OR B=0 THEN ...",               HCAT_OPER },
 { "XOR", "Bitwise exclusive OR",
 "PRINT 5 XOR 3  ' prints 6",            HCAT_OPER },

 /* ===== Introspection ===== (alphabetical) */
 { "CATALOG", "List all commands & functions",
 "CATALOG",                               HCAT_INTRO },
 { "DIALECT$", "Current dialect name (read-only)",
 "PRINT DIALECT$ or IF DIALECT$=\"GWBS\" THEN",
                                          HCAT_INTRO },
 { "HELP", "Show this help (or HELP keyword)",
 "HELP or HELP PRINT or HELP ABS",       HCAT_INTRO },
 { "INFO", "Display system information",
 "INFO",                                  HCAT_INTRO },
 { "KEY", "Assign function key string",
 "KEY 1, \"RUN\" + CHR$(13)",             HCAT_INTRO },
 { "LPOS", "Printer head column position",
 "PRINT LPOS(0)",                         HCAT_INTRO },
 { "MEMMAP$", "Current memory map name (read-only)",
 "PRINT MEMMAP$",                         HCAT_INTRO },
 { "VER", "Display version information",
 "VER",                                   HCAT_INTRO },

 { NULL, NULL, NULL, 0 }
};

/* Security level names for INFO */
static const char *sec_names[] = {
 "OPEN", "STANDARD", "RESTRICTED"
};

/* --- Alphabetical sorting helpers ---
 *
 * help_db[] is intended to be alphabetical in source, but
 * to guarantee correct output regardless of source order we
 * sort indices at display time.
 */

/* Case-insensitive string compare (like stricmp but portable C89) */
static int str_cmp_ci(const char *a, const char *b)
{
 while (*a && *b) {
 char ca = *a, cb = *b;
 if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
 if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
 if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
 a++; b++;
 }
 return (unsigned char)*a - (unsigned char)*b;
}

/* Sort an array of indices into help_db[] by keyword name */
static void sort_indices(int *idx, int count)
{
 int i, j;
 for (i = 1; i < count; i++) {
 int key = idx[i];
 j = i - 1;
 while (j >= 0 &&
 str_cmp_ci(help_db[idx[j]].keyword,
 help_db[key].keyword) > 0) {
 idx[j + 1] = idx[j];
 j--;
 }
 idx[j + 1] = key;
 }
}

/* --- help_show - Display command help ---
 */
void help_show(const char *topic)
{
 if (topic == NULL || topic[0] == '\0') {
 /* No topic - show categorized command summary */
 int cat;
 int i;
 int fcount;
 /* Index buffer for sorting within each category */
 int sorted[256];
 int scount;

 printf("=== BASIC++ COMMAND REFERENCE ===\n\n");

 for (cat = 0; cat < HCAT_COUNT; cat++) {
 /* Collect indices for this category */
 scount = 0;
 for (i = 0; help_db[i].keyword != NULL; i++) {
 if ((int)help_db[i].category == cat &&
 scount < 256) {
 sorted[scount++] = i;
 }
 }
 if (scount == 0) continue;

 /* Sort alphabetically */
 sort_indices(sorted, scount);

 printf("[%s]\n", help_cat_names[cat]);
 for (i = 0; i < scount; i++) {
 int si = sorted[i];
 printf(" %-12s %s\n",
 help_db[si].keyword,
 help_db[si].summary);
 }
 printf("\n");
 }

 /*
  * Also list registered functions that have
  * help_text set. This covers functions added
  * by external modules, libraries, and plugins.
  */
 fcount = funcreg_count();
 if (fcount > 0) {
 int shown = 0;
 for (i = 0; i < fcount; i++) {
 const FunctionEntry *f = funcreg_get(i);
 if (f && f->help_text) {
 if (!shown) {
 printf("[Registered Functions]\n");
 shown = 1;
 }
 printf(" %-12s %s\n",
 f->name,
 f->help_text);
 }
 }
 if (shown) printf("\n");
 }

 printf("Type HELP keyword for details.\n");
 printf("Type CATALOG for categorized list.\n");
 printf("Type INFO for system information.\n");
 } else {
 /* Search for specific topic */
 int i;
 int found = 0;

 /* 1. Search static command help database */
 for (i = 0; help_db[i].keyword != NULL; i++) {
 /* Case-insensitive compare */
 const char *a = topic;
 const char *b = help_db[i].keyword;
 int match = 1;

 while (*a && *b) {
 char ca = *a, cb = *b;
 if (ca >= 'a' && ca <= 'z') ca -= 32;
 if (cb >= 'a' && cb <= 'z') cb -= 32;
 if (ca != cb) { match = 0; break; }
 a++; b++;
 }
 if (*a || *b) match = 0;

 if (match) {
 printf("%s - %s\n",
     help_db[i].keyword,
     help_db[i].summary);
 printf("Usage: %s\n",
     help_db[i].usage);
 printf("Category: %s\n",
     help_cat_names[
         help_db[i].category]);
 found = 1;
 break;
 }
 }

 /*
  * 2. If not found in static db, search function
  * registry. This covers built-in functions AND
  * any functions added by external modules.
  */
 if (!found) {
 const FunctionEntry *f =
     funcreg_find_by_name(topic);
 if (f) {
 printf("%s - ", f->name);
 if (f->help_text) {
 printf("%s\n", f->help_text);
 } else {
 printf("Registered function");
 if (f->min_args == f->max_args) {
 printf(" (%d args)",
     f->min_args);
 } else {
 printf(" (%d-%d args)",
     f->min_args,
     f->max_args);
 }
 printf("\n");
 }
 found = 1;
 }
 }

 /*
  * 3. If not found in functions, search module
  * registry. External modules have descriptions.
  */
 if (!found) {
 const ModuleInfo *m = module_find(topic);
 if (m) {
 printf("%s - %s\n",
     m->name,
     m->description ? m->description
     : "Module");
 printf("Version: %s\n",
     m->version ? m->version : "?");
 printf("Class: %s\n",
     module_class_name(m->mod_class));
 found = 1;
 }
 }

 if (!found) {
 printf("No help for \"%s\".\n", topic);
 printf("Type HELP for command list.\n");
 }
 }
}

/* --- help_info - Display system information ---
 */
void help_info(RuntimeState *rt)
{
 SecLevel sec;
 int lines;

 printf("=== BASIC++ SYSTEM INFO ===\n\n");
 printf(" Version: %s\n", BASICPP_VERSION);
 printf(" Name: %s\n", BASICPP_NAME);
 printf(" Dialect: %s [%s]\n",
 dialect_get_name(),
 dialect_get_short_name());
 printf(" Memmap: %s\n",
 memmap_get_name(
 (MemMapType)rt->memmap_type));

 sec = security_get_level();
 printf(" Security: %s (%d)\n",
 (sec < SEC_COUNT) ? sec_names[sec] : "?", sec);

 printf(" Functions: %d registered\n", funcreg_count());

 lines = (rt->program != NULL) ? rt->program->count : 0;
 printf(" Program: %d lines\n", lines);
 printf(" Max Lines: %d\n", MAX_PROGRAM_LINES);
 printf(" Variables: %d (A-Z)\n", MAX_VARIABLES);
 printf(" Stack: %d levels\n", MAX_STACK_DEPTH);
 printf(" Breakpoints:%d max\n", MAX_BREAKPOINTS);
 printf(" Build: %s %s\n", __DATE__, __TIME__);
 printf(" Standard: ANSI C89/C90\n");
}

/* --- help_catalog - List all commands & functions ---
 *
 * Shows BOTH the static help database (statements/commands)
 * AND the function registry, organized by category with
 * alphabetical sorting within each category.
 */

/* Function category names (from funcreg.h) */
static const char *fcat_names[] = {
 "Core", "Math", "String", "I/O", "Utility", "User"
};

void help_catalog(void)
{
 int cat;
 int i, j;
 int fcount = funcreg_count();
 int total_cmds = 0;
 int sorted[256];
 int scount;

 printf("=== BASIC++ CATALOG ===\n\n");

 /* --- Part 1: Statements & Commands (sorted) --- */
 for (cat = 0; cat < HCAT_COUNT; cat++) {
 int col;

 /* Collect indices for this category */
 scount = 0;
 for (i = 0; help_db[i].keyword != NULL; i++) {
 if ((int)help_db[i].category == cat &&
 scount < 256) {
 sorted[scount++] = i;
 }
 }
 if (scount == 0) continue;

 /* Sort alphabetically */
 sort_indices(sorted, scount);

 printf("[%s]\n ", help_cat_names[cat]);
 col = 1;

 for (j = 0; j < scount; j++) {
 int si = sorted[j];
 int klen = (int)strlen(help_db[si].keyword);
 if (col + klen + 2 > 72) {
 printf("\n ");
 col = 1;
 }
 printf("%-13s", help_db[si].keyword);
 col += 13;
 total_cmds++;
 }
 printf("\n\n");
 }

 /* --- Part 2: Registered Functions (sorted) --- */
 if (fcount > 0) {
 int fcat;
 int shown_any = 0;
 /* Reuse sorted[] for function indices */
 int fsorted[256];
 int fscount;

 for (fcat = 0; fcat < 6; fcat++) {
 int col;

 /* Collect function indices for this category */
 fscount = 0;
 for (i = 0; i < fcount && fscount < 256; i++) {
 const FunctionEntry *f = funcreg_get(i);
 if (f != NULL && (int)f->category == fcat) {
 fsorted[fscount++] = i;
 }
 }
 if (fscount == 0) continue;

 /* Sort by function name */
 for (i = 1; i < fscount; i++) {
 int key = fsorted[i];
 const char *kname = funcreg_get(key)->name;
 j = i - 1;
 while (j >= 0 &&
 str_cmp_ci(funcreg_get(fsorted[j])->name,
 kname) > 0) {
 fsorted[j + 1] = fsorted[j];
 j--;
 }
 fsorted[j + 1] = key;
 }

 if (!shown_any) {
 printf("[Registered Functions"
 " - %s]\n ", fcat_names[fcat]);
 } else {
 printf("[Functions - %s]\n ",
 fcat_names[fcat]);
 }
 shown_any = 1;
 col = 1;

 for (i = 0; i < fscount; i++) {
 const FunctionEntry *f =
 funcreg_get(fsorted[i]);
 int nlen = (int)strlen(f->name);
 if (col + nlen + 2 > 72) {
 printf("\n ");
 col = 1;
 }
 printf("%-13s", f->name);
 col += 13;
 }
 printf("\n\n");
 }
 }

 printf("%d commands, %d functions registered.\n",
 total_cmds, fcount);
}
