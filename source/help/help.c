/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: help.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Self-describing HELP interactive system, OPTION overrides, and security scope permissions.
 *
 * 2. WHAT TO EXPECT:
 *    Help prints keyword usage. Overrides rewrite token attributes at parse time.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Command summaries, help topics, scope presets mappings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Help databases lookup logic, keyword gating routines.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If help command fails, ensure help databases are sorted. Check override loop guards.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - help.c
 // ---
 //
 // Help & introspection system.
 //
 // Makes BASIC++ self-describing: every command, function, and
 // system setting is documented within the interpreter itself.
 //
 // ORGANIZATION:
 // Both HELP and CATALOG group entries by functional category,
 // sorted alphabetically within each category. Categories:
 //
 //   Control Flow       - IF, FOR, GOTO, GOSUB, WHILE, DO, etc.
 //   Input / Output     - PRINT, INPUT, CLS, LOCATE, etc.
 //   Arithmetic / Math  - ABS, SIN, COS, RND, MOD, etc.
 //   String             - LEN, LEFT$, MID$, CHR$, ASC, etc.
 //   Variables & Memory - LET, DIM, PEEK, POKE, SWAP, etc.
 //   File I/O           - OPEN, CLOSE, READ, WRITE, etc.
 //   Graphics           - SCREEN, DRAW, PSET, CIRCLE, etc.
 //   Sound              - BEEP, SOUND, PLAY
 //   System & Environ   - SYSTEM, SHELL, ENVIRON, DIALECT, etc.
 //   Debug & Test       - BREAK, CONT, VARS, ASSERT, etc.
 //   Program Mgmt       - RUN, LIST, SAVE, LOAD, NEW, etc.
 //   Editing            - RENUM, DELETE, EDIT, AUTO
 //   Devices & Network  - VDEV, VNET, DEVMAP, FUJINET, etc.
 //   Operators          - AND, OR, NOT, XOR, MOD, EQV, IMP
 //   Introspection      - HELP, INFO, CATALOG, VER
 //
//
// HOW TO EXTEND:
//   See the preamble comments in related files for
//   customization and extension instructions.
//
// TROUBLESHOOTING:
//   Check error_occurred() after operations that can fail.
//   Use error_raise(ERR_xxx, line_num) for error reporting.
 // ---

#include <stdio.h>
#include <string.h>
#include "help.h"
#include "config.h"
#include "funcreg.h"
#include "security.h"
#include "module.h"
#include "memmap.h"

// --- Help Category IDs ---
typedef enum HelpCatId {
 HCAT_FLOW = 0, // Control Flow
 HCAT_IO, // Input / Output
 HCAT_MATH, // Arithmetic / Math
 HCAT_STRING, // String Functions
 HCAT_VARMEM, // Variables & Memory
 HCAT_FILEIO, // File I/O
 HCAT_GFX, // Graphics
 HCAT_SOUND, // Sound
 HCAT_SYSENV, // System & Environment
 HCAT_DEBUG, // Debugging & Testing
 HCAT_PROGMGMT, // Program Management
 HCAT_EDIT, // Editing
 HCAT_DEVICE, // Devices & Networking
 HCAT_OPER, // Operators
 HCAT_INTRO, // Introspection
 HCAT_SYSVAR, // System Variables
 HCAT_COUNT // sentinel
} HelpCatId;

// Category display names
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
 "Introspection",
 "System Variables"
};

// --- Command Help Database ---
 // Each entry maps a keyword to a short description, usage
 // example, and category. Entries within each category are
 // sorted alphabetically by keyword.
typedef struct HelpEntry {
 const char *keyword;
 const char *summary;
 const char *usage;
 HelpCatId   category;
} HelpEntry;

#ifdef BPP_LITE_BUILD
static const HelpEntry help_db[] = {
    // ===== Control Flow =====
    { "END", "Terminate program execution", "END", HCAT_FLOW },
    { "FOR", "Begin a loop count", "FOR I = 1 TO 10 STEP 2", HCAT_FLOW },
    { "GOTO", "Jump to line number", "GOTO 100", HCAT_FLOW },
    { "IF", "Conditional branch (single-line only)", "IF X > 10 THEN PRINT X", HCAT_FLOW },
    { "NEXT", "End of FOR loop", "NEXT I", HCAT_FLOW },
    { "ON", "Computed GOTO", "ON X GOTO 10, 20, 30", HCAT_FLOW },
    { "STEP", "Specify loop step size", "FOR I = 1 TO 10 STEP 2", HCAT_FLOW },
    { "THEN", "IF statement clause", "IF X = 5 THEN GOTO 10", HCAT_FLOW },
    { "TO", "Specify loop end value", "FOR I = 1 TO 10", HCAT_FLOW },

    // ===== Core Commands =====
    { "BEEP", "Sound the PC speaker", "BEEP", HCAT_SOUND },
    { "DATA", "Define constants to read", "DATA 10, 20, \"hello\"", HCAT_FLOW },
    { "DIM", "Dimension an array", "DIM A(10) or DIM B$(5, 5)", HCAT_VARMEM },
    { "HELP", "List available keywords or get details", "HELP or HELP \"PRINT\"", HCAT_INTRO },
    { "INPUT", "Request input from user", "INPUT \"Enter age: \", A", HCAT_IO },
    { "LET", "Assign value to variable", "LET A = 5", HCAT_VARMEM },
    { "LIST", "Show current program code", "LIST or LIST 10-100", HCAT_PROGMGMT },
    { "NEW", "Clear current program", "NEW", HCAT_PROGMGMT },
    { "PRINT", "Display output to console", "PRINT A$ or PRINT 2 + 2", HCAT_IO },
    { "READ", "Read next constant from DATA", "READ X or READ Y$", HCAT_FLOW },
    { "REM", "Add a comment line", "REM comments", HCAT_VARMEM },
    { "RESTORE", "Reset the DATA read pointer", "RESTORE", HCAT_FLOW },
    { "RUN", "Execute current program", "RUN", HCAT_PROGMGMT },
    { "STOP", "Suspend execution", "STOP", HCAT_FLOW },

    // ===== Math & String Functions =====
    { "ABS", "Absolute value function", "ABS(X)", HCAT_MATH },
    { "ASC", "ASCII value of first character", "ASC(A$)", HCAT_STRING },
    { "CHR$", "Convert code to character", "CHR$(65)", HCAT_STRING },
    { "INT", "Truncate to integer", "INT(X)", HCAT_MATH },
    { "LEFT$", "Get left portion of string", "LEFT$(A$, 5)", HCAT_STRING },
    { "LEN", "Length of a string", "LEN(A$)", HCAT_STRING },
    { "MID$", "Substring function", "MID$(A$, start, len)", HCAT_STRING },
    { "RIGHT$", "Get right portion of string", "RIGHT$(A$, 5)", HCAT_STRING },
    { "RND", "Random number function", "RND(10)", HCAT_MATH },
    { "SGN", "Sign function (-1, 0, or 1)", "SGN(X)", HCAT_MATH },
    { "STR$", "Convert number to string", "STR$(X)", HCAT_STRING },
    { "VAL", "Convert string to number", "VAL(A$)", HCAT_STRING },
    { "INSTR", "Find string position", "INSTR(source$, find$)", HCAT_STRING },
    { "SPACE$", "Generate N spaces", "SPACE$(10)", HCAT_STRING },
    { "STRING$", "Generate string of copies", "STRING$(5, \"*\")", HCAT_STRING },
    { "HEX$", "Convert to hexadecimal string", "HEX$(X)", HCAT_STRING },
    { "OCT$", "Convert to octal string", "OCT$(X)", HCAT_STRING },
    { "BIN$", "Convert to binary string", "BIN$(X)", HCAT_STRING },
    { "FRE", "Free memory query", "FRE(0) or FRE(-1) or FRE(-2)", HCAT_VARMEM },

    // ===== System Variables =====
    { "VER", "Current BASIC++ version number", "X = VER", HCAT_SYSVAR },
    { "DATE$", "Current date as MM-DD-YYYY", "PRINT DATE$", HCAT_SYSVAR },
    { "TIME$", "Current time as HH:MM:SS", "PRINT TIME$", HCAT_SYSVAR },
    { "TIMER", "Seconds elapsed since midnight", "PRINT TIMER", HCAT_SYSVAR },
    { "CLOCK$", "Current timestamp as YYYY-MM-DD HH:MM:SS", "PRINT CLOCK$", HCAT_SYSVAR },
    { "INKEY$", "Non-blocking single-char keyboard read", "A$ = INKEY$", HCAT_SYSVAR },
    { "ONKEY$", "Event-aware keyboard read", "A$ = ONKEY$", HCAT_SYSVAR },
    { "ERR", "Numeric code of last runtime error", "PRINT ERR", HCAT_SYSVAR },
    { "ERL", "Line number of last runtime error", "PRINT ERL", HCAT_SYSVAR },
    { "HOSTNAME$", "System hostname string", "PRINT HOSTNAME$", HCAT_SYSVAR },
    { "USERNAME$", "Current system username string", "PRINT USERNAME$", HCAT_SYSVAR },
    { "PWD$", "Current working directory path string", "PRINT PWD$", HCAT_SYSVAR },
    { "CWD$", "Current working directory name string", "PRINT CWD$", HCAT_SYSVAR },
    { "MONTH", "Current numeric month (1-12)", "PRINT MONTH", HCAT_SYSVAR },
    { "MONTH$", "Current month name", "PRINT MONTH$", HCAT_SYSVAR },
    { "DAY", "Current day of month (1-31)", "PRINT DAY", HCAT_SYSVAR },
    { "DAY$", "Current day of the week", "PRINT DAY$", HCAT_SYSVAR },
    { "YEAR", "Current 4-digit year", "PRINT YEAR", HCAT_SYSVAR },
    { "HOURS", "Current hour (0-23)", "PRINT HOURS", HCAT_SYSVAR },
    { "MINUTES", "Current minute (0-59)", "PRINT MINUTES", HCAT_SYSVAR },
    { "SECONDS", "Current second (0-59)", "PRINT SECONDS", HCAT_SYSVAR },
    { "TRUE", "Boolean value 1", "PRINT TRUE", HCAT_SYSVAR },
    { "FALSE", "Boolean value -1", "PRINT FALSE", HCAT_SYSVAR },
    { "MEM", "Free variable space in bytes", "PRINT MEM", HCAT_SYSVAR },
    { "SIZE", "Program memory size in bytes", "PRINT SIZE", HCAT_SYSVAR },

    { NULL, NULL, NULL, 0 }
};
#else
static const HelpEntry help_db[] = {
  // ===== Control Flow ===== (alphabetical)
  { "BY", "Counting step in loop (SBASIC FOR loop alias)",
    "FOR I = 1 TO 10 BY 2", HCAT_FLOW },
  { "CASE", "Case clause within SELECT CASE",
    "CASE 1,2,3 or CASE IS > 10 or CASE ELSE", HCAT_FLOW },
  { "CAUSE", "Raise an exception (ECMA-116)",
    "CAUSE EXCEPTION 1000", HCAT_FLOW },
  { "CONTINUE", "Resume execution of loop",
    "CONTINUE FOR or CONTINUE DO", HCAT_FLOW },
  { "DATA", "Store numeric and string constants used by READ",
    "DATA 10, 20, \"hello\"", HCAT_FLOW },
  { "DECLARE", "Forward-declare a SUB or FUNCTION",
    "DECLARE SUB MySub(x, y$)", HCAT_FLOW },
  { "DEF", "Define a user function (one-line)",
    "DEF FNA(X)=X*X+1", HCAT_FLOW },
  { "DEFINE", "Begin user-defined block function",
    "DEFINE FUNCTION my_fn(x) ... ENDDEFINE", HCAT_FLOW },
  { "DO", "Begin a DO loop",
    "DO ... LOOP WHILE cond", HCAT_FLOW },
  { "ELSE", "Alternate branch in block IF or single-line IF",
    "IF X > 5 THEN PRINT X ELSE PRINT Y", HCAT_FLOW },
  { "ELSEIF", "Additional condition in block IF",
    "ELSEIF X>10 THEN ...", HCAT_FLOW },
  { "END", "Terminate program execution",
    "END", HCAT_FLOW },
  { "ENDDEFINE", "End user-defined function block",
    "ENDDEFINE", HCAT_FLOW },
  { "ENDFOR", "End counting loop (Structured BASIC)",
    "ENDFOR", HCAT_FLOW },
  { "ENDIF", "End a block IF structure",
    "END IF", HCAT_FLOW },
  { "ENDREPEAT", "End repeat loop block",
    "ENDREPEAT", HCAT_FLOW },
  { "ERL", "Line number of last error",
    "PRINT ERL", HCAT_FLOW },
  { "ERR", "Error code of last error",
    "PRINT ERR", HCAT_FLOW },
  { "ERROR", "Raise a user error",
    "ERROR 5", HCAT_FLOW },
  { "EXIT", "Exit a FOR or DO loop early",
    "EXIT FOR or EXIT DO", HCAT_FLOW },
  { "FN", "Call user-defined function",
    "PRINT FN my_fn(x)", HCAT_FLOW },
  { "FOR", "Begin a counting loop",
    "FOR I=1 TO 10 STEP 2 (or BY 2 in SBASIC)", HCAT_FLOW },
  { "FUNCTION", "Define a named function",
    "FUNCTION MyFn (x) ... END FUNCTION", HCAT_FLOW },
  { "GOSUB", "Call a subroutine at a line number",
    "GOSUB 500", HCAT_FLOW },
  { "GOTO", "Jump to a line number",
    "GOTO 100", HCAT_FLOW },
  { "IN", "Check containment in list or array",
    "IF X IN (1, 2, 3) THEN ...", HCAT_FLOW },
  { "IF", "Conditional decision structure",
    "IF X > 10 THEN PRINT X ELSE PRINT Y", HCAT_FLOW },
  { "IS", "Case selector comparison operator",
    "CASE IS > 10", HCAT_FLOW },
  { "LOOP", "End of a DO loop (with condition)",
    "LOOP WHILE A>0 or LOOP UNTIL A=0", HCAT_FLOW },
  { "NEXT", "End of a FOR loop",
    "NEXT I", HCAT_FLOW },
  { "ON", "Computed GOTO/GOSUB or error/event handler",
    "ON X GOTO 100,200 or ON ERROR GOTO 500", HCAT_FLOW },
  { "ON BREAK", "Set Ctrl+C / signal interrupt handler",
    "ON BREAK GOSUB 900", HCAT_FLOW },
  { "ON COM", "Set serial port interrupt handler",
    "ON COM(1) GOSUB 800", HCAT_FLOW },
  { "ON KEY", "Set key press interrupt handler",
    "ON KEY(1) GOSUB 700", HCAT_FLOW },
  { "ON PEN", "Set light pen interrupt handler",
    "ON PEN GOSUB 600", HCAT_FLOW },
  { "ON PLAY", "Set music buffer interrupt handler",
    "ON PLAY(3) GOSUB 500", HCAT_FLOW },
  { "ON STRIG", "Set joystick trigger interrupt handler",
    "ON STRIG(0) GOSUB 400", HCAT_FLOW },
  { "ON TIMER", "Set timer interrupt handler",
    "ON TIMER(5) GOSUB 300", HCAT_FLOW },
  { "PROCEDURE", "Begin procedure block definition",
    "PROCEDURE MyProc ... ENDPROC", HCAT_FLOW },
  { "REPEAT", "Begin repeat loop block",
    "REPEAT ... UNTIL cond", HCAT_FLOW },
  { "RESUME", "Return from an error handler",
    "RESUME or RESUME NEXT or RESUME 100", HCAT_FLOW },
  { "RETRY", "Retry failed transaction statement",
    "RETRY", HCAT_FLOW },
  { "RETURN", "Return from a GOSUB subroutine",
    "RETURN", HCAT_FLOW },
  { "SELECT", "Begin a SELECT CASE block",
    "SELECT CASE X", HCAT_FLOW },
  { "STEP", "Count loop step increments",
    "FOR I = 1 TO 10 STEP 2", HCAT_FLOW },
  { "STOP", "Pause program (use CONT to resume)",
    "STOP", HCAT_FLOW },
  { "SUB", "Define a named subroutine",
    "SUB MySub (x, y$) ... END SUB", HCAT_FLOW },
  { "THEN", "Conditional branch target indicator",
    "IF A > B THEN PRINT A", HCAT_FLOW },
  { "TO", "Counting upper limit indicator",
    "FOR I = 1 TO 10", HCAT_FLOW },
  { "TRAP", "Set error/event trap handler (hybrid)",
    "TRAP n | TRAP e,s,n | TRAP (status) | ON TRAP GOSUB n", HCAT_FLOW },
  { "UNTIL", "Exit condition for repeat loop",
    "LOOP UNTIL X > 5", HCAT_FLOW },
  { "USE", "Protected exception block handler",
    "WHEN EXCEPTION IN ... USE ... END WHEN", HCAT_FLOW },
  { "USR", "Invoke machine language function",
    "PRINT USR(x)", HCAT_FLOW },
  { "WEND", "End of a WHILE loop",
    "WEND", HCAT_FLOW },
  { "WHEN", "Begin protected exception block",
    "WHEN EXCEPTION IN ... USE ... END WHEN", HCAT_FLOW },
  { "WHILE", "Begin a conditional loop",
    "WHILE A>0 ... WEND", HCAT_FLOW },

  // ===== Input / Output ===== (alphabetical)
  { "CLS", "Clear the screen",
    "CLS", HCAT_IO },
  { "COLOR", "Set text foreground/background",
    "COLOR 14,1 (yellow on blue)", HCAT_IO },
  { "CONSOLE", "Set or query active console device",
    "CONSOLE 2", HCAT_IO },
  { "HOME", "Move cursor to top-left",
    "HOME", HCAT_IO },
  { "IMAGE", "Format template string for PRINT USING",
    "IMAGE \"###.##\"", HCAT_IO },
  { "INKEY", "Non-blocking key read value",
    "K = INKEY", HCAT_IO },
  { "INKEY$", "Non-blocking keyboard read",
    "K$=INKEY$", HCAT_IO },
  { "INPUT", "Prompt user for input and store in variables",
    "INPUT \"Enter age: \", A", HCAT_IO },
  { "INPUT$", "Read N chars from keyboard/file",
    "A$ = INPUT$(5) or A$ = INPUT$(10, #1)", HCAT_IO },
  { "LINE", "Read entire line (LINE INPUT)",
    "LINE INPUT A$ or LINE INPUT #1, A$", HCAT_IO },
  { "LOCATE", "Move cursor to row,column",
    "LOCATE 10,20", HCAT_IO },
  { "PRINT", "Display text or variable values on console or file",
    "PRINT \"hello\" or PRINT A, B or PRINT #1, A", HCAT_IO },
  { "WIDTH", "Set screen width",
    "WIDTH 80 or WIDTH 80,25", HCAT_IO },
  { "WRITE", "Write comma-delimited data",
    "WRITE #1, A, B$, C", HCAT_IO },

  // ===== Arithmetic / Math ===== (alphabetical)
  { "ABS", "Absolute value",
    "PRINT ABS(-5) ' prints 5", HCAT_MATH },
  { "ACOS", "Arccosine (SBASIC, returns radians)",
    "PRINT ACOS(0) ' 1.5708 (PI/2)", HCAT_MATH },
  { "ASIN", "Arcsine (SBASIC, returns radians)",
    "PRINT ASIN(1) ' 1.5708 (PI/2)", HCAT_MATH },
  { "ATN", "Arctangent (returns radians)",
    "PI = 4*ATN(1)", HCAT_MATH },
  { "CDBL", "Convert to double precision",
    "PRINT CDBL(X)", HCAT_MATH },
  { "CINT", "Round to nearest integer",
    "PRINT CINT(3.6) ' prints 4", HCAT_MATH },
  { "COMP", "Compare two values (SBASIC, returns -1/0/+1)",
    "PRINT COMP(3,5) ' -1", HCAT_MATH },
  { "COS", "Cosine (radians)",
    "PRINT COS(0) ' prints 1", HCAT_MATH },
  { "COSH", "Hyperbolic cosine (SBASIC)",
    "PRINT COSH(0) ' 1", HCAT_MATH },
  { "CSNG", "Convert to single precision",
    "PRINT CSNG(X)", HCAT_MATH },
  { "DET", "Determinant of a matrix",
    "PRINT DET(A)", HCAT_MATH },
  { "EXP", "Exponential (e^x)",
    "PRINT EXP(1) ' prints 2.71828", HCAT_MATH },
  { "FIX", "Truncate toward zero",
    "PRINT FIX(-3.7) ' prints -3", HCAT_MATH },
  { "INT", "Truncate to integer (math) or trigger software interrupt",
    "PRINT INT(3.7) or INT &H10", HCAT_MATH },
  { "LN", "Natural logarithm (base e)",
    "PRINT LN(2.71828)", HCAT_MATH },
  { "LOG", "Natural logarithm (base e)",
    "PRINT LOG(2.71828)", HCAT_MATH },
  { "LOG10", "Common logarithm base 10 (SBASIC)",
    "PRINT LOG10(100) ' 2", HCAT_MATH },
  { "LOG2", "Binary logarithm base 2 (SBASIC)",
    "PRINT LOG2(8) ' 3", HCAT_MATH },
  { "LGT", "Common logarithm base 10 (HP BASIC)",
    "PRINT LGT(100) ' 2", HCAT_MATH },
  { "TIM", "Time components mode (HP BASIC, 0=min,1=hr,2=yday,3=yr)",
    "PRINT TIM(1)", HCAT_MATH },
  { "HI", "High 8 bits (byte) of integer",
    "PRINT HI(256) ' 1", HCAT_MATH },
  { "LO", "Low 8 bits (byte) of integer",
    "PRINT LO(257) ' 1", HCAT_MATH },
  { "PDIF", "Positive difference (SBASIC, MAX(a-b,0))",
    "PRINT PDIF(5,3) ' 2", HCAT_MATH },
  { "PI", "Constant PI (SBASIC, 3.14159...)",
    "PRINT PI ' 3.14159265358979", HCAT_MATH },
  { "RANDOMIZE", "Seed the random number generator",
    "RANDOMIZE or RANDOMIZE 42", HCAT_MATH },
  { "REMAINDER", "Modulo remainder of division",
    "PRINT REMAINDER(10, 3)", HCAT_MATH },
  { "RND", "Random number (0 to 1)",
    "PRINT RND(1) or PRINT INT(RND(1)*6)+1", HCAT_MATH },
  { "SGN", "Sign of a number (-1, 0, or 1)",
    "PRINT SGN(-5) ' prints -1", HCAT_MATH },
  { "SIN", "Sine (radians)",
    "PRINT SIN(3.14159/2)", HCAT_MATH },
  { "SINH", "Hyperbolic sine (SBASIC)",
    "PRINT SINH(1) ' 1.1752", HCAT_MATH },
  { "SQR", "Square root",
    "PRINT SQR(144) ' prints 12", HCAT_MATH },
  { "TAN", "Tangent (radians)",
    "PRINT TAN(3.14159/4)", HCAT_MATH },
  { "TANH", "Hyperbolic tangent (SBASIC)",
    "PRINT TANH(1) ' 0.7616", HCAT_MATH },

  // ===== String Functions ===== (alphabetical)
  { "BIN", "Represent integer in binary form",
    "PRINT BIN(255)", HCAT_STRING },
  { "BIN$", "Convert to binary string (8-bit groups)",
    "PRINT BIN$(255) ' 11111111", HCAT_STRING },
  { "CHR", "Convert ASCII code to string character",
    "PRINT CHR(65)", HCAT_STRING },
  { "CHR$", "Character from ASCII code",
    "PRINT CHR$(65) ' A", HCAT_STRING },
  { "EDIT$", "Format/manipulate string using a bitmask (DEC VAX)",
    "PRINT EDIT$(\"  abc [123]  \", 8+32+64+128) ' prints \"ABC (123)\"", HCAT_STRING },
  { "HASH", "Compute FNV-1a hash of string/data",
    "PRINT HASH(\"input\")", HCAT_STRING },
  { "HEX", "Convert number to hex representation",
    "PRINT HEX(255)", HCAT_STRING },
  { "HEX$", "Convert to hexadecimal string",
    "PRINT HEX$(255) ' FF", HCAT_STRING },
  { "ICASE", "Invert character case of string",
    "PRINT ICASE(\"AbC\")", HCAT_STRING },
  { "LCASE", "Convert string to lowercase",
    "PRINT LCASE(\"hello\")", HCAT_STRING },
  { "LEFT", "Extract leftmost N characters",
    "PRINT LEFT(\"HELLO\", 2)", HCAT_STRING },
  { "LTRIM", "Remove leading whitespace from string",
    "PRINT LTRIM(\"  hi\")", HCAT_STRING },
  { "MCASE", "Convert string to mixed case",
    "PRINT MCASE(\"hello\")", HCAT_STRING },
  { "MID", "Extract or replace substring",
    "PRINT MID(\"HELLO\", 2, 2)", HCAT_STRING },
  { "NUM", "Convert string to number (DEC VAX, alias of VAL)",
    "PRINT NUM(\"42\") ' prints 42", HCAT_STRING },
  { "NUM$", "Convert number to string without space (DEC VAX)",
    "PRINT NUM$(42) ' prints \"42\"", HCAT_STRING },
  { "OCT", "Convert number to octal representation",
    "PRINT OCT(255)", HCAT_STRING },
  { "OCT$", "Convert to octal string",
    "PRINT OCT$(255) ' 377", HCAT_STRING },
  { "ONKEY$", "Event-aware keyboard read (like INKEY$)",
    "K$=ONKEY$", HCAT_STRING },
  { "REPLACE", "Replace substring occurrences in string",
    "PRINT REPLACE(\"abc\", \"b\", \"x\")", HCAT_STRING },
  { "RIGHT", "Extract rightmost N characters",
    "PRINT RIGHT(\"HELLO\", 2)", HCAT_STRING },
  { "RTRIM", "Remove trailing whitespace from string",
    "PRINT RTRIM(\"hi  \")", HCAT_STRING },
  { "SPACE", "Generate string of N spaces",
    "PRINT SPACE(10)", HCAT_STRING },
  { "SPACE$", "Generate N space characters",
    "PRINT SPACE$(10)", HCAT_STRING },
  { "STR$", "Convert number to string",
    "A$=STR$(42)", HCAT_STRING },
  { "TCASE", "Convert string to title case",
    "PRINT TCASE(\"hello world\")", HCAT_STRING },
  { "TRIM", "Remove leading/trailing whitespace",
    "PRINT TRIM(\"  hello  \")", HCAT_STRING },
  { "UCASE", "Convert string to uppercase",
    "PRINT UCASE(\"hello\")", HCAT_STRING },

  // ===== Variables & Memory ===== (alphabetical)
  { "AS", "Specify variable type or file channel alias",
    "DIM X AS INTEGER | OPEN \"f.dat\" FOR INPUT AS #1", HCAT_VARMEM },
  { "BANK", "RAMBANK segmented virtual memory control",
    "BANK n SHARED | BANK n PRIVATE | BANK n CLEAR | BANK n STATUS | BANK LIST | BANK n", HCAT_VARMEM },
  { "CLEAR", "Clear stack and variables",
    "CLEAR", HCAT_VARMEM },
  { "CLR", "Clear all variables (Atari/C64)",
    "CLR", HCAT_VARMEM },
  { "COMMON", "Share vars with CHAINed program",
    "COMMON A, B$", HCAT_VARMEM },
  { "CONST", "Define a named constant",
    "CONST PI = 3.14159", HCAT_VARMEM },
  { "CSRLIN", "Current cursor row",
    "PRINT CSRLIN", HCAT_VARMEM },
  { "DEFDBL", "Declare double variable range",
    "DEFDBL A-Z", HCAT_VARMEM },
  { "DEFINT", "Declare integer variable range",
    "DEFINT A-Z", HCAT_VARMEM },
  { "DEFSNG", "Declare single variable range",
    "DEFSNG A-Z", HCAT_VARMEM },
  { "DEFSTR", "Declare string variable range",
    "DEFSTR A-Z", HCAT_VARMEM },
  { "DIM", "Declare an array with dimensions",
    "DIM A(10) or DIM M(3,3) or DIM A$(25)", HCAT_VARMEM },
  { "ERASE", "Erase an array from memory",
    "ERASE A", HCAT_VARMEM },
  { "FRE", "Free memory available",
    "PRINT FRE(0)", HCAT_VARMEM },
  { "LBOUND", "Array lower bound index",
    "PRINT LBOUND(A, 1)", HCAT_VARMEM },
  { "LET", "Assign value to variable (optional in most dialects)",
    "LET A = 5 or A = 5", HCAT_VARMEM },
  { "LOCAL", "Declare local variables in sub/fn",
    "LOCAL X, Y$", HCAT_VARMEM },
  { "MAT", "Matrix operations",
    "MAT PRINT A or MAT A=ZER", HCAT_VARMEM },
  { "MEMMAP", "Select memory map preset",
    "MEMMAP 48 or MEMMAP 64", HCAT_VARMEM },
  { "MKD", "Pack double to 8-byte string",
    "A$ = MKD(D)", HCAT_VARMEM },
  { "MKI", "Pack integer to 2-byte string",
    "A$ = MKI(I)", HCAT_VARMEM },
  { "MKS", "Pack single to 4-byte string",
    "A$ = MKS(S)", HCAT_VARMEM },
  { "OPTION", "Set interpreter options",
    "OPTION BASE 0|1 | STRICT ON|OFF | MIXED D1,D2,...", HCAT_VARMEM },
  { "PEEK", "Read byte from virtual memory",
    "PRINT PEEK(addr)", HCAT_VARMEM },
  { "PEEKB", "Read byte from a RAMBANK segment",
    "PRINT PEEKB(bank, offset)", HCAT_VARMEM },
  { "POKE", "Write byte to virtual memory",
    "POKE addr, value", HCAT_VARMEM },
  { "POKEB", "Write byte to a RAMBANK segment",
    "POKEB bank, offset, value", HCAT_VARMEM },
  { "POS", "Current cursor column",
    "PRINT POS(0)", HCAT_VARMEM },
  { "READ", "Read the next DATA value",
    "READ A or READ A$", HCAT_VARMEM },
  { "REDIM", "Resize a dynamic array",
    "REDIM A(20)", HCAT_VARMEM },
  { "REM", "Comment (ignored by interpreter)",
    "REM This is a comment (or ' shorthand)", HCAT_VARMEM },
  { "RESTORE", "Reset DATA pointer to beginning",
    "RESTORE or RESTORE 200", HCAT_VARMEM },
  { "SEG", "Get memory segment index of variable",
    "PRINT SEG(X)", HCAT_VARMEM },
  { "SHARED", "Share variables with main program",
    "SHARED A, B$", HCAT_VARMEM },
  { "SIZE", "Show free memory (Tiny BASIC)",
    "SIZE", HCAT_VARMEM },
  { "STATIC", "Preserve local variables across calls",
    "STATIC count", HCAT_VARMEM },
  { "STRING", "Specify string data type",
    "DIM name AS STRING", HCAT_VARMEM },
  { "SWAP", "Exchange two variables",
    "SWAP A,B", HCAT_VARMEM },
  { "TYPE", "Define a user-defined type",
    "TYPE recname ... END TYPE", HCAT_VARMEM },
  { "UBOUND", "Array upper bound index",
    "PRINT UBOUND(A, 1)", HCAT_VARMEM },
  { "VARPTR", "Pointer index of a variable",
    "PRINT VARPTR(A)", HCAT_VARMEM },

  // ===== File I/O ===== (alphabetical)
  { "ACCESS", "Query file channel permissions",
    "PRINT ACCESS(1)", HCAT_FILEIO },
  { "ASK", "Query file info (ECMA-116)",
    "ASK #1: FILESIZE F", HCAT_FILEIO },
  { "CLOSE", "Close a file channel",
    "CLOSE #1", HCAT_FILEIO },
  { "CURDIR", "Query current directory path",
    "PRINT CURDIR", HCAT_FILEIO },
  { "CVD", "Unpack 8-byte string to double",
    "D = CVD(A$)", HCAT_FILEIO },
  { "CVI", "Unpack 2-byte string to integer",
    "N = CVI(A$)", HCAT_FILEIO },
  { "CVS", "Unpack 4-byte string to single",
    "F = CVS(A$)", HCAT_FILEIO },
  { "CWD", "Query current working directory",
    "PRINT CWD", HCAT_FILEIO },
  { "FIELD", "Define record buffer fields",
    "FIELD #1, 20 AS N$, 4 AS A$", HCAT_FILEIO },
  { "FILEMOD", "Get file modification timestamp",
    "PRINT FILEMOD(\"data.txt\")", HCAT_FILEIO },
  { "GET", "Read a record from random file",
    "GET #1, recnum", HCAT_FILEIO },
  { "LOC", "Current file position",
    "PRINT LOC(1)", HCAT_FILEIO },
  { "LOCK", "Lock file records",
    "LOCK #1, record", HCAT_FILEIO },
  { "LOF", "Get length of open file",
    "PRINT LOF(1)", HCAT_FILEIO },
  { "MKD$", "Pack double to 8-byte string",
    "A$ = MKD$(D)", HCAT_FILEIO },
  { "MKI$", "Pack integer to 2-byte string",
    "A$ = MKI$(N)", HCAT_FILEIO },
  { "MKS$", "Pack single to 4-byte string",
    "A$ = MKS$(F)", HCAT_FILEIO },
  { "MOUNT", "Mount virtual VFS directory slot",
    "MOUNT \"C:\\DATA\" AS \"/vfs\"", HCAT_FILEIO },
  { "MOUNTS", "Display all active VFS mounts",
    "MOUNTS", HCAT_FILEIO },
  { "POINTER", "Query/set file position pointer",
    "POINTER #1, offset", HCAT_FILEIO },
  { "PRETRIEVE", "Retrieve persistent variable (MUMPS)",
    "N = PRETRIEVE(\"key\")", HCAT_FILEIO },
  { "PRETRIEVE$", "Retrieve persistent string variable (MUMPS)",
    "S$ = PRETRIEVE$(\"key\")", HCAT_FILEIO },
  { "PSTORE", "Store persistent variable (MUMPS)",
    "R = PSTORE(\"key\", val)", HCAT_FILEIO },
  { "PUT", "Write a record to random file",
    "PUT #1, recnum", HCAT_FILEIO },
  { "RESET", "Close all open files",
    "RESET", HCAT_FILEIO },
  { "REWRITE", "Re-open channel for writing",
    "REWRITE #1", HCAT_FILEIO },
  { "SEEK", "Set or get file position",
    "SEEK #1, 100", HCAT_FILEIO },
  { "SET", "Set file pointer (ECMA-116)",
    "SET #1: POINTER 0", HCAT_FILEIO },
  { "UMOUNT", "Unmount virtual VFS slot",
    "UMOUNT \"/vfs\"", HCAT_FILEIO },
  { "UNLOAD", "Unload loaded dynamic library",
    "UNLOAD \"lib.dll\"", HCAT_FILEIO },
  { "UNLOCK", "Unlock file records",
    "UNLOCK #1, record", HCAT_FILEIO },
  { "VPATH", "Query resolved virtual VFS path",
    "PRINT VPATH(\"/vfs/f.txt\")", HCAT_FILEIO },

  // ===== Graphics ===== (alphabetical)
  { "AT", "Position cursor or draw coordinate",
    "PRINT AT(10,20);\"Hello\"", HCAT_GFX },
  { "BORDER", "Set screen border color",
    "BORDER 4", HCAT_GFX },
  { "BRIGHT", "Set text screen brightness attribute",
    "BRIGHT 1 | BRIGHT 0", HCAT_GFX },
  { "CIRCLE", "Draw a circle",
    "CIRCLE (160,100), 50", HCAT_GFX },
  { "DISPLAY", "Configure or query terminal displays",
    "DISPLAY ON | DISPLAY OFF", HCAT_GFX },
  { "FLASH", "Toggle text flashing screen attribute",
    "FLASH 1 | FLASH 0", HCAT_GFX },
  { "GRAPHICS", "Set Atari 8-bit graphics/text mode (abbreviated GR.)",
    "GRAPHICS 8+16", HCAT_GFX },
  { "INK", "Set graphics pen/ink color",
    "INK 14", HCAT_GFX },
  { "INVERSE", "Toggle text screen reverse video",
    "INVERSE 1 | INVERSE 0", HCAT_GFX },
  { "OVER", "Toggle text overstrike rendering mode",
    "OVER 1 | OVER 0", HCAT_GFX },
  { "PAINT", "Flood fill an area",
    "PAINT (160,100), color", HCAT_GFX },
  { "PALETTE", "Remap a palette color",
    "PALETTE colornum, newcolor", HCAT_GFX },
  { "PAPER", "Set text screen background paper color",
    "PAPER 1", HCAT_GFX },
  { "PCOPY", "Copy screen memory page",
    "PCOPY 1, 0", HCAT_GFX },
  { "PEN", "Read or configure light pen slot",
    "PRINT PEN(0)", HCAT_GFX },
  { "PLOT", "Plot point or draw line",
    "PLOT 100, 50", HCAT_GFX },
  { "PMAP", "Map physical to logical coordinates",
    "PRINT PMAP(X, 0)", HCAT_GFX },
  { "POINT", "Read color at pixel (x,y)",
    "C = POINT(100,50)", HCAT_GFX },
  { "PRESET", "Reset a pixel at (x,y)",
    "PRESET (100,50)", HCAT_GFX },
  { "PSET", "Set a pixel at (x,y)",
    "PSET (100,50), color", HCAT_GFX },
  { "REVERSE", "Toggle reverse video text mode",
    "REVERSE 1 | REVERSE 0", HCAT_GFX },
  { "SCREEN", "Set screen mode",
    "SCREEN mode [,color]", HCAT_GFX },
  { "VIEW", "Set graphics viewport",
    "VIEW (x1,y1)-(x2,y2)", HCAT_GFX },
  { "WINDOW", "Set logical coordinate system",
    "WINDOW (0,0)-(639,199)", HCAT_GFX },

  // ===== Sound ===== (alphabetical)
  { "BEEP", "Emit an audible beep",
    "BEEP", HCAT_SOUND },
  { "SOUND", "Play a tone at frequency",
    "SOUND 440, 18", HCAT_SOUND },

  // ===== System & Environment ===== (alphabetical)
  { "ALARM", "Set or trigger a timed alarm",
    "ON ALARM(\"12:00:00\") GOSUB 500", HCAT_SYSENV },
  { "ATOMIC", "Begin a transactional block",
    "ATOMIC: A=A+1: B=B-1: COMMIT", HCAT_SYSENV },
  { "BIOS", "Switch active mock BIOS profile",
    "BIOS \"IBMPC\" or BIOS \"PCAT\"", HCAT_SYSENV },
  { "BYE", "Exit the interpreter",
    "BYE", HCAT_SYSENV },
  { "CLOCK", "Access system NTP clock",
    "CLOCK ON | CLOCK OFF", HCAT_SYSENV },
  { "CLOCK$", "Full timestamp string",
    "PRINT CLOCK$ ' YYYY-MM-DD HH:MM:SS", HCAT_SYSENV },
  { "COMMIT", "Commit active transaction changes",
    "COMMIT", HCAT_SYSENV },
  { "CURDIR$", "Current directory (alias for CWD$)",
    "PRINT CURDIR$", HCAT_SYSENV },
  { "CWD", "Print current working directory folder name",
    "CWD", HCAT_SYSENV },
  { "CWD$", "Current working directory name string",
    "A$ = CWD$ or PRINT CWD$", HCAT_SYSENV },
  { "DATE", "Query or set system date",
    "PRINT DATE or DATE = \"2026-06-29\"", HCAT_SYSENV },
  { "DATE$", "Current date string",
    "PRINT DATE$", HCAT_SYSENV },
  { "DELAY", "Pause execution for milliseconds",
    "DELAY 500", HCAT_SYSENV },
  { "ERDEV", "Error device code of last device error",
    "PRINT ERDEV", HCAT_SYSENV },
  { "ERRORLEVEL", "Last shell exit code",
    "PRINT ERRORLEVEL", HCAT_SYSENV },
  { "EXTERR", "Extended error information",
    "PRINT EXTERR(0)", HCAT_SYSENV },
  { "HOSTNAME", "Print system hostname",
    "HOSTNAME", HCAT_SYSENV },
  { "HOSTNAME$", "System hostname string",
    "PRINT HOSTNAME$", HCAT_SYSENV },
  { "KEY(N)", "Enable/disable key event trapping",
    "KEY(1) ON | KEY(1) OFF | KEY(1) STOP", HCAT_SYSENV },
  { "KILL", "Reserved for future use",
    "KILL (use SCRATCH to delete files)", HCAT_SYSENV },
  { "ONKEY", "Trigger key interrupt handler",
    "ONKEY GOSUB 500", HCAT_SYSENV },
  { "PAUSE", "Pause program for N frames/seconds",
    "PAUSE 60", HCAT_SYSENV },
  { "PLAY(N)", "Enable/disable music buffer interrupt",
    "PLAY ON | PLAY OFF | PLAY STOP", HCAT_SYSENV },
  { "PWD", "Print current working directory path",
    "PWD", HCAT_SYSENV },
  { "PWD$", "Current working directory path string",
    "PRINT PWD$", HCAT_SYSENV },
  { "ROLLBACK", "Rollback current transaction state",
    "ROLLBACK", HCAT_SYSENV },
  { "SLEEP", "Pause execution for N seconds",
    "SLEEP 2", HCAT_SYSENV },
  { "STICK", "Read joystick coordinate value",
    "PRINT STICK(0)", HCAT_SYSENV },
  { "STRIG", "Read joystick trigger status",
    "PRINT STRIG(0)", HCAT_SYSENV },
  { "TIME", "Query or set system time",
    "PRINT TIME or TIME = \"12:00:00\"", HCAT_SYSENV },
  { "TIME$", "Current time string",
    "PRINT TIME$", HCAT_SYSENV },
  { "TIMER", "Seconds elapsed since midnight (also: event trap)",
    "PRINT TIMER | TIMER ON | TIMER OFF | TIMER STOP", HCAT_SYSENV },
  { "TXN", "Query current transaction status",
    "PRINT TXN", HCAT_SYSENV },
  { "USERNAME", "Print current system username",
    "USERNAME", HCAT_SYSENV },
  { "USERNAME$", "Current system username string",
    "PRINT USERNAME$", HCAT_SYSENV },

  // ===== Debugging & Testing ===== (alphabetical)
  { "ASSERT", "Test an assertion (pass/fail)",
    "ASSERT 2+3=5 or ASSERT A>0", HCAT_DEBUG },
  { "BACKTRACE", "Show call stack backtrace",
    "BACKTRACE", HCAT_DEBUG },
  { "BREAK", "Set/clear/list breakpoints",
    "BREAK 100 | BREAK -100 | BREAK", HCAT_DEBUG },
  { "CHECK", "Analyze program currently in memory",
    "CHECK", HCAT_DEBUG },
  { "CONT", "Continue from STOP/BREAK",
    "CONT", HCAT_DEBUG },
  { "DEBUG", "Print debug message or enter debugger",
    "DEBUG \"State: \"; X", HCAT_DEBUG },
  { "DUMP", "Dump variables or memory state",
    "DUMP or DUMP \"vars.txt\"", HCAT_DEBUG },
  { "ENDTEST", "End test block and report",
    "ENDTEST", HCAT_DEBUG },
  { "SELFTEST", "Run built-in validation suite",
    "SELFTEST", HCAT_DEBUG },
  { "TRACE", "Print execution line trace",
    "TRACE ON | TRACE OFF", HCAT_DEBUG },
  { "TROFF", "Disable line trace output",
    "TROFF", HCAT_DEBUG },
  { "TRON", "Enable line trace output",
    "TRON", HCAT_DEBUG },
  { "VARS", "Display variables and environment",
    "VARS [USER|ENV|SYSTEM|ALL] - program, user, OS, or BASIC++ env", HCAT_DEBUG },
  { "VERIFY", "Analyze program stored on disk",
    "VERIFY \"prog.bas\"", HCAT_DEBUG },

  // ===== Program Management ===== (alphabetical)
  { "BRUN", "Load and run binary program",
    "BRUN \"game.bin\"", HCAT_PROGMGMT },
  { "LIST", "Display program lines",
    "LIST or LIST 100-200", HCAT_PROGMGMT },
  { "LLIST", "List program to printer",
    "LLIST or LLIST 100-200", HCAT_PROGMGMT },
  { "NEW", "Clear program from memory",
    "NEW", HCAT_PROGMGMT },
  { "RUN", "Execute the stored program",
    "RUN", HCAT_PROGMGMT },
  { "UNSAVE", "Delete the last saved file",
    "UNSAVE Deletes last SAVE/LOAD filename", HCAT_PROGMGMT },

  // ===== Editing ===== (alphabetical)
  { "AUTO", "Auto-generate line numbers",
    "AUTO or AUTO 100,5", HCAT_EDIT },
  { "DELETE", "Delete lines by range",
    "DELETE 100 or DELETE 100-200", HCAT_EDIT },
  { "EDIT", "Edit a program line",
    "EDIT 100", HCAT_EDIT },
  { "KEYWORD", "Modify built-in keyword behavior",
    "KEYWORD PRINT UPPERCASE ON | KEYWORD kw DESCRIBE", HCAT_EDIT },
  { "RENUM", "Renumber program lines",
    "RENUM or RENUM 100,5", HCAT_EDIT },

  // ===== Devices & Networking ===== (alphabetical)
  { "BIOREAD", "Read block from block-level device",
    "BIOREAD #1, block_num, array()", HCAT_DEVICE },
  { "CLOCK:", "FujiNet NTP clock device",
    "Read time via CLOCK: device (ISO/binary formats)", HCAT_DEVICE },
  { "COM", "Configure serial port communication",
    "COM(1) ON | COM(1) OFF | COM(1) STOP", HCAT_DEVICE },
  { "DEVMAP", "Device slot mapping",
    "DEVMAP - file channels + VDev slot table", HCAT_DEVICE },
  { "FUJI:", "FujiNet configuration device",
    "WiFi config, host slots, AppKey storage", HCAT_DEVICE },
  { "INP", "Read byte from hardware port",
    "PRINT INP(&H3F8)", HCAT_DEVICE },
  { "IOCTL", "Device control command",
    "IOCTL #ch, cmd [, arg]", HCAT_DEVICE },
  { "NINFO", "Query network adapter information",
    "PRINT NINFO", HCAT_DEVICE },
  { "NJSONQUERY", "Perform JSON query over network channel",
    "PRINT NJSONQUERY(\"url\", \"path\")", HCAT_DEVICE },
  { "OUT", "Write byte to hardware port",
    "OUT &H3F8, 1", HCAT_DEVICE },
  { "SIOREAD", "Read block from stream/serial port",
    "SIOREAD #1, buffer$, size", HCAT_DEVICE },
  { "SIOREADLN", "Read line from stream/serial port",
    "SIOREADLN #1, buffer$", HCAT_DEVICE },
  { "SYS", "Call machine language routine",
    "SYS addr", HCAT_DEVICE },
  { "VCON", "Virtual console information",
    "VCON - console device, input/output capabilities", HCAT_DEVICE },
  { "VDEV", "List registered virtual devices",
    "VDEV - shows slot, name, class, caps, description", HCAT_DEVICE },
  { "VMACH", "Virtual machine state",
    "VMACH - VM state, opcodes, modules, security", HCAT_DEVICE },
  { "VMEM", "Virtual memory status",
    "VMEM - memory map, pools, stack, program storage", HCAT_DEVICE },
  { "VNET", "Virtual network status",
    "VNET - FujiNet channels, connections, adapter info", HCAT_DEVICE },
  { "VTERM", "Virtual terminal information",
    "VTERM - screen size, cursor, encoding, dialect", HCAT_DEVICE },
  { "WAIT", "Wait for port status",
    "WAIT port, AND_mask [,XOR_mask]", HCAT_DEVICE },

  // ===== Operators ===== (alphabetical)
  { "AND", "Logical/bitwise AND operator",
    "IF A>0 AND B>0 THEN ...", HCAT_OPER },
  { "EQV", "Logical equivalence operator",
    "PRINT 5 EQV 3", HCAT_OPER },
  { "IMP", "Logical implication operator",
    "PRINT 5 IMP 3", HCAT_OPER },
  { "MOD", "Modulo (remainder) operator",
    "PRINT 10 MOD 3 ' prints 1", HCAT_OPER },
  { "NOT", "Logical/bitwise NOT operator",
    "IF NOT A THEN ...", HCAT_OPER },
  { "OR", "Logical/bitwise OR operator",
    "IF A=0 OR B=0 THEN ...", HCAT_OPER },
  { "XOR", "Bitwise exclusive OR",
    "PRINT 5 XOR 3 ' prints 6", HCAT_OPER },

  // ===== Introspection ===== (alphabetical)
  { "CATALOG", "List all commands & functions",
    "CATALOG", HCAT_INTRO },
  { "HELP", "Show this help (or HELP keyword)",
    "HELP or HELP PRINT or HELP ABS", HCAT_INTRO },
  { "INFO", "Display system information",
    "INFO", HCAT_INTRO },
  { "LPOS", "Printer head column position",
    "PRINT LPOS(0)", HCAT_INTRO },
  { "MEMMAP$", "Current memory map name (read-only)",
    "PRINT MEMMAP$", HCAT_INTRO },
  { "VER", "Display version information",
    "VER", HCAT_INTRO },

  // ===== System Variables =====
  { "VER", "Current BASIC++ version number", "X = VER", HCAT_SYSVAR },
  { "DATE$", "Current date as MM-DD-YYYY", "PRINT DATE$", HCAT_SYSVAR },
  { "TIME$", "Current time as HH:MM:SS", "PRINT TIME$", HCAT_SYSVAR },
  { "TIMER", "Seconds elapsed since midnight", "PRINT TIMER", HCAT_SYSVAR },
  { "CLOCK$", "Current timestamp as YYYY-MM-DD HH:MM:SS", "PRINT CLOCK$", HCAT_SYSVAR },
  { "INKEY$", "Non-blocking single-char keyboard read", "A$ = INKEY$", HCAT_SYSVAR },
  { "ONKEY$", "Event-aware keyboard read", "A$ = ONKEY$", HCAT_SYSVAR },
  { "ERR", "Numeric code of last runtime error", "PRINT ERR", HCAT_SYSVAR },
  { "ERL", "Line number of last runtime error", "PRINT ERL", HCAT_SYSVAR },
   { "HOSTNAME$", "System hostname string", "PRINT HOSTNAME$", HCAT_SYSVAR },
   { "USERNAME$", "Current system username string", "PRINT USERNAME$", HCAT_SYSVAR },
   { "PWD$", "Current working directory path string", "PRINT PWD$", HCAT_SYSVAR },
   { "CWD$", "Current working directory name string", "PRINT CWD$", HCAT_SYSVAR },
  { "MONTH", "Current numeric month (1-12)", "PRINT MONTH", HCAT_SYSVAR },
  { "MONTH$", "Current month name", "PRINT MONTH$", HCAT_SYSVAR },
  { "DAY", "Current day of month (1-31)", "PRINT DAY", HCAT_SYSVAR },
  { "DAY$", "Current day of the week", "PRINT DAY$", HCAT_SYSVAR },
  { "YEAR", "Current 4-digit year", "PRINT YEAR", HCAT_SYSVAR },
  { "HOURS", "Current hour (0-23)", "PRINT HOURS", HCAT_SYSVAR },
  { "MINUTES", "Current minute (0-59)", "PRINT MINUTES", HCAT_SYSVAR },
  { "SECONDS", "Current second (0-59)", "PRINT SECONDS", HCAT_SYSVAR },
  { "TRUE", "Boolean value 1", "PRINT TRUE", HCAT_SYSVAR },
  { "FALSE", "Boolean value -1", "PRINT FALSE", HCAT_SYSVAR },
  { "MEM", "Free variable space in bytes", "PRINT MEM", HCAT_SYSVAR },
  { "SIZE", "Program memory size in bytes", "PRINT SIZE", HCAT_SYSVAR },

  { NULL, NULL, NULL, 0 }
};
#endif

// Security level names for INFO
static const char *sec_names[] = {
 "OPEN", "STANDARD", "RESTRICTED"
};

// --- Alphabetical sorting helpers ---
 //
 // help_db[] is intended to be alphabetical in source, but
 // to guarantee correct output regardless of source order we
 // sort indices at display time.

// Case-insensitive string compare (like stricmp but portable C89)
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

// Sort an array of indices into help_db[] by keyword name
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

// --- help_show - Display command help ---
void help_show(const char *topic)
{
 if (topic == NULL || topic[0] == '\0') {
 // No topic - show categorized command summary
 int cat;
 int i;
 int fcount;
 // Index buffer for sorting within each category
 int sorted[256];
 int scount;

 printf("=== BASIC++ COMMAND REFERENCE ===\n\n");

 for (cat = 0; cat < HCAT_COUNT; cat++) {
 // Collect indices for this category
 scount = 0;
 for (i = 0; help_db[i].keyword != NULL; i++) {
 if ((int)help_db[i].category == cat &&
 scount < 256) {
 sorted[scount++] = i;
 }
 }
 if (scount == 0) continue;

 // Sort alphabetically
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

  // Also list registered functions that have
  // help_text set. This covers functions added
  // by external modules, libraries, and plugins.
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
#ifndef BPP_LITE_BUILD
  printf("Type CATALOG for categorized list.\n");
  printf("Type INFO for system information.\n");
#endif
 } else {
 // Search for specific topic
 int i;
 int found = 0;

 // 1. Search static command help database
 for (i = 0; help_db[i].keyword != NULL; i++) {
 // Case-insensitive compare
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

  // 2. If not found in static db, search function
  // registry. This covers built-in functions AND
  // any functions added by external modules.
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

  // 3. If not found in functions, search module
  // registry. External modules have descriptions.
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

// --- help_info - Display system information ---
void help_info(RuntimeState *rt)
{
 SecLevel sec;
 int lines;

 printf("=== BASIC++ SYSTEM INFO ===\n\n");
 printf(" Version: %s\n", BASICPP_VERSION);
 printf(" Name: %s\n", BASICPP_NAME);
 printf(" Dialect: %s [%s]\n",
 "BASIC++",
 "BPP");
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
 printf(" Standard: ISO/IEC 9899:2018 (C17)\n");
}

// --- help_catalog - List all commands & functions ---
 //
 // Shows BOTH the static help database (statements/commands)
 // AND the function registry, organized by category with
 // alphabetical sorting within each category.

// Function category names (from funcreg.h)
#ifndef BPP_LITE_BUILD
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

 // --- Part 1: Statements & Commands (sorted) ---
 for (cat = 0; cat < HCAT_COUNT; cat++) {
 int col;

 // Collect indices for this category
 scount = 0;
 for (i = 0; help_db[i].keyword != NULL; i++) {
 if ((int)help_db[i].category == cat &&
 scount < 256) {
 sorted[scount++] = i;
 }
 }
 if (scount == 0) continue;

 // Sort alphabetically
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

 // --- Part 2: Registered Functions (sorted) ---
 if (fcount > 0) {
 int fcat;
 int shown_any = 0;
 // Reuse sorted[] for function indices
 int fsorted[256];
 int fscount;

 for (fcat = 0; fcat < 6; fcat++) {
 int col;

 // Collect function indices for this category
 fscount = 0;
 for (i = 0; i < fcount && fscount < 256; i++) {
 const FunctionEntry *f = funcreg_get(i);
 if (f != NULL && (int)f->category == fcat) {
 fsorted[fscount++] = i;
 }
 }
 if (fscount == 0) continue;

 // Sort by function name
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
#endif
