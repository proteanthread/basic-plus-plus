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

/* --- Command Help Database ---
 * Each entry maps a keyword to a short description and usage example.
 * The table is searched linearly - fine for interactive use.
 */
typedef struct HelpEntry {
 const char *keyword;
 const char *summary;
 const char *usage;
} HelpEntry;

static const HelpEntry help_db[] = {
 /* Core commands */
 { "LET", "Assign a value to a variable",
 "LET A=10 or LET A$=\"HELLO\"" },
 { "PRINT", "Display output to the console",
 "PRINT \"Hello\";A or PRINT A,B" },
 { "INPUT", "Read user input into a variable",
 "INPUT A or INPUT \"Name? \";N$" },
 { "IF", "Conditional execution (IF/THEN/ELSE)",
 "IF A>5 THEN PRINT \"BIG\" ELSE PRINT \"SMALL\"" },
 { "GOTO", "Jump to a line number",
 "GOTO 100" },
 { "GOSUB", "Call a subroutine at a line number",
 "GOSUB 500" },
 { "RETURN", "Return from a GOSUB subroutine",
 "RETURN" },
 { "FOR", "Begin a counting loop",
 "FOR I=1 TO 10 STEP 2" },
 { "NEXT", "End of a FOR loop",
 "NEXT I" },
 { "END", "Terminate program execution",
 "END" },
 { "STOP", "Pause program (use CONT to resume)",
 "STOP" },
 { "REM", "Comment (ignored by interpreter)",
 "REM This is a comment (or ' shorthand)" },
 { "DIM", "Declare an array with dimensions",
 "DIM A(10) or DIM M(3,3) or DIM A$(25)" },
 { "DATA", "Embed data values in the program",
 "DATA 1,2,3,\"HELLO\"" },
 { "READ", "Read the next DATA value",
 "READ A or READ A$" },
 { "RESTORE", "Reset DATA pointer to beginning",
 "RESTORE or RESTORE 200" },
 { "ON", "Computed GOTO/GOSUB or error handler",
 "ON X GOTO 100,200 or ON ERROR GOTO 500" },
 /* Program management */
 { "RUN", "Execute the stored program",
 "RUN" },
 { "LIST", "Display program lines",
 "LIST or LIST 100-200" },
 { "NEW", "Clear program from memory",
 "NEW" },
 { "SAVE", "Save program to a file",
 "SAVE \"prog.bas\"" },
 { "LOAD", "Load program from a file",
 "LOAD \"prog.bas\"" },
 { "MERGE", "Merge program lines from file",
 "MERGE \"extra.bas\"" },
 { "CHAIN", "Load and run another program",
 "CHAIN \"next.bas\"" },
 { "COMPILE", "Transpile BASIC to C source",
 "COMPILE \"output.c\"" },
 /* Flow control */
 { "WHILE", "Begin a conditional loop",
 "WHILE A>0 ... WEND" },
 { "WEND", "End of a WHILE loop",
 "WEND" },
 { "DO", "Begin a DO loop",
 "DO ... LOOP WHILE cond" },
 { "LOOP", "End of a DO loop (with condition)",
 "LOOP WHILE A>0 or LOOP UNTIL A=0" },
 { "ELSE", "Alternate branch of IF statement",
 "IF A>5 THEN PRINT \"BIG\" ELSE PRINT \"SMALL\"" },
 { "ELSEIF", "Additional condition in block IF",
 "ELSEIF X>10 THEN ..." },
 { "ENDIF", "End a block IF structure",
 "END IF" },
 { "SELECT", "Begin a SELECT CASE block",
 "SELECT CASE X" },
 { "CASE", "Case clause within SELECT CASE",
 "CASE 1,2,3 or CASE IS > 10 or CASE ELSE" },
 { "EXIT", "Exit a FOR or DO loop early",
 "EXIT FOR or EXIT DO" },
 /* Error handling */
 { "ERROR", "Raise a user error",
 "ERROR 5" },
 { "RESUME", "Return from an error handler",
 "RESUME or RESUME NEXT or RESUME 100" },
 /* Subroutines */
 { "SUB", "Define a named subroutine",
 "SUB MySub (x, y$) ... END SUB" },
 { "FUNCTION", "Define a named function",
 "FUNCTION MyFn (x) ... END FUNCTION" },
 { "CALL", "Invoke a named subroutine",
 "CALL MySub(10, \"test\")" },
 { "DECLARE", "Forward-declare a SUB or FUNCTION",
 "DECLARE SUB MySub(x, y$)" },
 { "DEF", "Define a user function (one-line)",
 "DEF FNA(X)=X*X+1" },
 /* Math functions */
 { "ABS", "Absolute value",
 "PRINT ABS(-5)  ' prints 5" },
 { "RND", "Random number (0 to 1)",
 "PRINT RND(1) or PRINT INT(RND(1)*6)+1" },
 { "SGN", "Sign of a number (-1, 0, or 1)",
 "PRINT SGN(-5)  ' prints -1" },
 { "SIN", "Sine (radians)",
 "PRINT SIN(3.14159/2)" },
 { "COS", "Cosine (radians)",
 "PRINT COS(0)  ' prints 1" },
 { "TAN", "Tangent (radians)",
 "PRINT TAN(3.14159/4)" },
 { "ATN", "Arctangent (returns radians)",
 "PI = 4*ATN(1)" },
 { "SQR", "Square root",
 "PRINT SQR(144)  ' prints 12" },
 { "LOG", "Natural logarithm (base e)",
 "PRINT LOG(2.71828)" },
 { "EXP", "Exponential (e^x)",
 "PRINT EXP(1)  ' prints 2.71828" },
 { "INT", "Truncate to integer (toward -infinity)",
 "PRINT INT(3.7)  ' prints 3" },
 { "FIX", "Truncate toward zero",
 "PRINT FIX(-3.7)  ' prints -3" },
 { "CINT", "Round to nearest integer",
 "PRINT CINT(3.6)  ' prints 4" },
 { "CSNG", "Convert to single precision",
 "PRINT CSNG(X)" },
 { "CDBL", "Convert to double precision",
 "PRINT CDBL(X)" },
 { "RANDOMIZE", "Seed the random number generator",
 "RANDOMIZE or RANDOMIZE 42" },
 { "MOD", "Modulo (remainder) operator",
 "PRINT 10 MOD 3  ' prints 1" },
 /* Logical / bitwise operators */
 { "AND", "Logical/bitwise AND operator",
 "IF A>0 AND B>0 THEN ..." },
 { "OR", "Logical/bitwise OR operator",
 "IF A=0 OR B=0 THEN ..." },
 { "NOT", "Logical/bitwise NOT operator",
 "IF NOT A THEN ..." },
 { "XOR", "Bitwise exclusive OR",
 "PRINT 5 XOR 3  ' prints 6" },
 /* String functions */
 { "LEN", "Length of a string",
 "PRINT LEN(\"HELLO\")  ' prints 5" },
 { "LEFT$", "Left N characters",
 "PRINT LEFT$(\"HELLO\",3)  ' HEL" },
 { "RIGHT$", "Right N characters",
 "PRINT RIGHT$(\"HELLO\",3)  ' LLO" },
 { "MID$", "Substring from position",
 "PRINT MID$(\"HELLO\",2,3)  ' ELL" },
 { "CHR$", "Character from ASCII code",
 "PRINT CHR$(65)  ' A" },
 { "ASC", "ASCII code of first character",
 "PRINT ASC(\"A\")  ' 65" },
 { "STR$", "Convert number to string",
 "A$=STR$(42)" },
 { "VAL", "Convert string to number",
 "A=VAL(\"42\")" },
 { "INSTR", "Find substring position",
 "PRINT INSTR(\"HELLO\",\"LL\")  ' 3" },
 { "SPACE$", "Generate N space characters",
 "PRINT SPACE$(10)" },
 { "STRING$", "Generate N copies of a character",
 "PRINT STRING$(10,\"*\")" },
 { "UCASE$", "Convert string to uppercase",
 "PRINT UCASE$(\"hello\")  ' HELLO" },
 { "LCASE$", "Convert string to lowercase",
 "PRINT LCASE$(\"HELLO\")  ' hello" },
 { "TCASE$", "Convert string to title case",
 "PRINT TCASE$(\"hello world\")  ' Hello World" },
 { "LTRIM$", "Remove leading spaces",
 "PRINT LTRIM$(\"  hello\")" },
 { "RTRIM$", "Remove trailing spaces",
 "PRINT RTRIM$(\"hello  \")" },
 { "TRIM$", "Remove leading and trailing spaces",
 "PRINT TRIM$(\"  hello  \")  ' hello" },
 { "HEX$", "Convert to hexadecimal string",
 "PRINT HEX$(255)  ' FF" },
 { "OCT$", "Convert to octal string",
 "PRINT OCT$(255)  ' 377" },
 /* Display / screen */
 { "CLS", "Clear the screen",
 "CLS" },
 { "CLR", "Clear all variables (Atari/C64)",
 "CLR" },
 { "CLEAR", "Clear stack and variables",
 "CLEAR" },
 { "TAB", "Move cursor to column position",
 "PRINT TAB(20);\"HERE\"" },
 { "SPC", "Print N spaces",
 "PRINT SPC(10);\"TEXT\"" },
 { "LOCATE", "Move cursor to row,column",
 "LOCATE 10,20" },
 { "HOME", "Move cursor to top-left",
 "HOME" },
 { "COLOR", "Set text foreground/background",
 "COLOR 14,1 (yellow on blue)" },
 { "TRON", "Enable line trace output",
 "TRON" },
 { "TROFF", "Disable line trace output",
 "TROFF" },
 /* Matrix */
 { "MAT", "Matrix operations",
 "MAT PRINT A or MAT A=ZER" },
 /* File I/O */
 { "OPEN", "Open a file channel",
 "OPEN \"data.txt\" FOR INPUT AS #1" },
 { "CLOSE", "Close a file channel",
 "CLOSE #1" },
 { "EOF", "Check for end of file",
 "IF EOF(1) THEN PRINT \"Done\"" },
 { "LOF", "Get length of open file",
 "PRINT LOF(1)" },
 { "SEEK", "Set or get file position",
 "SEEK #1, 100" },
 { "FIELD", "Define record buffer fields",
 "FIELD #1, 20 AS N$, 4 AS A$" },
 { "LSET", "Left-justify in field buffer",
 "LSET N$ = \"Smith\"" },
 { "RSET", "Right-justify in field buffer",
 "RSET N$ = \"Smith\"" },
 { "GET", "Read a record from random file",
 "GET #1, recnum" },
 { "PUT", "Write a record to random file",
 "PUT #1, recnum" },
 { "WRITE", "Write comma-delimited data",
 "WRITE #1, A, B$, C" },
 { "LINE", "Read entire line (LINE INPUT)",
 "LINE INPUT A$ or LINE INPUT #1, A$" },
 { "USING", "Format output with template",
 "PRINT USING \"###.##\"; 3.14" },
 { "LPRINT", "Print to standard error",
 "LPRINT \"Status: OK\"" },
 { "INPUT$", "Read N chars from keyboard/file",
 "A$ = INPUT$(5) or A$ = INPUT$(10, #1)" },
 { "CVI", "Unpack 2-byte string to integer",
 "N = CVI(A$)" },
 { "CVS", "Unpack 4-byte string to single",
 "F = CVS(A$)" },
 { "CVD", "Unpack 8-byte string to double",
 "D = CVD(A$)" },
 { "MKI$", "Pack integer to 2-byte string",
 "A$ = MKI$(N)" },
 { "MKS$", "Pack single to 4-byte string",
 "A$ = MKS$(F)" },
 { "MKD$", "Pack double to 8-byte string",
 "A$ = MKD$(D)" },
 /* Bytecode */
 { "BSAVE", "Save compiled bytecode",
 "BSAVE \"prog.bpp\"" },
 { "BLOAD", "Load compiled bytecode",
 "BLOAD \"prog.bpp\"" },
 /* Modules */
 { "MODULE", "Activate/list modules",
 "MODULE \"stdlib\" or MODULE" },
 /* Security */
 { "SECURITY", "Set security level",
 "SECURITY 0 (open) | 1 (standard) | 2 (restricted)" },
 /* System */
 { "SYSTEM", "Query platform info",
 "SYSTEM \"OS\" or SYSTEM \"ARCH\"" },
 { "DIALECT", "Set the BASIC dialect",
 "DIALECT \"PATB\" or DIALECT" },
 { "OPTION", "Set interpreter options",
 "OPTION BASE 0|1 or OPTION STRICT ON|OFF" },
 { "SHELL", "Execute an OS command",
 "SHELL \"dir\" or A$=SHELL$(\"date /t\")" },
 { "ENVIRON", "Read environment variable",
 "A$=ENVIRON$(\"PATH\")" },
 { "CHDIR", "Change current directory",
 "CHDIR \"C:\\GAMES\"" },
 { "MKDIR", "Create a new directory",
 "MKDIR \"newdir\"" },
 { "RMDIR", "Remove a directory",
 "RMDIR \"olddir\"" },
 { "KILL", "Delete a file",
 "KILL \"temp.dat\"" },
 { "NAME", "Rename a file (GW-BASIC)",
 "NAME \"old.bas\" AS \"new.bas\"" },
 { "RENAME", "Rename a file",
 "RENAME \"old.bas\",\"new.bas\"" },
 /* Debugger */
 { "BREAK", "Set/clear/list breakpoints",
 "BREAK 100 | BREAK -100 | BREAK" },
 { "CONT", "Continue from STOP/BREAK",
 "CONT" },
 { "VARS", "Display all non-zero variables",
 "VARS" },
 /* Testing */
 { "ASSERT", "Test an assertion (pass/fail)",
 "ASSERT 2+3=5 or ASSERT A>0" },
 { "TEST", "Start a named test block",
 "TEST \"Arithmetic\"" },
 { "ENDTEST", "End test block and report",
 "ENDTEST" },
 { "SELFTEST", "Run built-in validation suite",
 "SELFTEST" },
 /* Introspection */
 { "HELP", "Show this help (or HELP keyword)",
 "HELP or HELP PRINT or HELP ABS" },
 { "INFO", "Display system information",
 "INFO" },
 { "CATALOG", "List all registered functions",
 "CATALOG" },
 /* Editing */
 { "RENUM", "Renumber program lines",
 "RENUM or RENUM 100,5" },
 { "DELETE", "Delete lines by range",
 "DELETE 100 or DELETE 100-200" },
 { "EDIT", "Edit a program line",
 "EDIT 100" },
 { "AUTO", "Auto-generate line numbers",
 "AUTO or AUTO 100,5" },
 { "VER", "Display version information",
 "VER" },
 { "ALIAS", "Remap a keyword to a new name",
 "ALIAS PRINT = \"IMPRE\"" },
 /* Memory and variables */
 { "PEEK", "Read byte from virtual memory",
 "PRINT PEEK(addr)" },
 { "POKE", "Write byte to virtual memory",
 "POKE addr, value" },
 { "FRE", "Free memory available",
 "PRINT FRE(0)" },
 { "SWAP", "Exchange two variables",
 "SWAP A,B" },
 { "CONST", "Define a named constant",
 "CONST PI = 3.14159" },
 { "ERASE", "Erase an array from memory",
 "ERASE A" },
 { "REDIM", "Resize a dynamic array",
 "REDIM A(20)" },
 { "COMMON", "Share vars with CHAINed program",
 "COMMON A, B$" },
 { "SLEEP", "Pause execution for N seconds",
 "SLEEP 2" },
 { "TIMER", "Seconds elapsed since midnight",
 "PRINT TIMER" },
 { "INKEY$", "Non-blocking keyboard read",
 "K$=INKEY$" },
 { "DATE$", "Current date string",
 "PRINT DATE$" },
 { "TIME$", "Current time string",
 "PRINT TIME$" },
 { "FILES", "List files in current directory",
 "FILES or FILES \"*.BAS\"" },
 { "DIR", "List filenames (no details)",
 "DIR or DIR \"*.BAS\"" },
 /* Graphics */
 { "SCREEN", "Set screen mode",
 "SCREEN mode [,color]" },
 { "DRAW", "Draw using graphics macros",
 "DRAW \"R10 D10 L10 U10\"" },
 { "PSET", "Set a pixel at (x,y)",
 "PSET (100,50), color" },
 { "PRESET", "Reset a pixel at (x,y)",
 "PRESET (100,50)" },
 { "CIRCLE", "Draw a circle",
 "CIRCLE (160,100), 50" },
 { "PAINT", "Flood fill an area",
 "PAINT (160,100), color" },
 { "PALETTE", "Remap a palette color",
 "PALETTE colornum, newcolor" },
 { "POINT", "Read color at pixel (x,y)",
 "C = POINT(100,50)" },
 /* Sound */
 { "BEEP", "Emit an audible beep",
 "BEEP" },
 { "SOUND", "Play a tone at frequency",
 "SOUND 440, 18" },
 { "PLAY", "Play music macro language",
 "PLAY \"T120 O4 L4 CDEFGAB\"" },
 /* Terminal */
 { "WIDTH", "Set screen width",
 "WIDTH 80 or WIDTH 80,25" },
 /* ECMA-116 */
 { "SET", "Set file pointer (ECMA-116)",
 "SET #1: POINTER 0" },
 { "ASK", "Query file info (ECMA-116)",
 "ASK #1: FILESIZE F" },
 /* Misc */
 { "BYE", "Exit the interpreter",
 "BYE" },
 { "MEMMAP", "Select memory map preset",
 "MEMMAP 48 or MEMMAP 64" },
 { "SIZE", "Show free memory (Tiny BASIC)",
 "SIZE" },
 { "ERRORLEVEL","Last shell exit code",
 "PRINT ERRORLEVEL" },
 /* Error variables */
 { "ERL", "Line number of last error",
 "PRINT ERL" },
 { "ERR", "Error code of last error",
 "PRINT ERR" },
 /* Variable info */
 { "VARPTR", "Pointer index of a variable",
 "PRINT VARPTR(A)" },
 { "POS", "Current cursor column",
 "PRINT POS(0)" },
 { "CSRLIN", "Current cursor row",
 "PRINT CSRLIN" },
 /* Type declarations */
 { "DEFINT", "Declare integer variable range",
 "DEFINT A-Z" },
 { "DEFDBL", "Declare double variable range",
 "DEFDBL A-Z" },
 { "DEFSNG", "Declare single variable range",
 "DEFSNG A-Z" },
 { "DEFSTR", "Declare string variable range",
 "DEFSTR A-Z" },
 { "TYPE", "Define a user-defined type",
 "TYPE recname ... END TYPE" },
 { "SHARED", "Share variables with main program",
 "SHARED A, B$" },
 { "STATIC", "Preserve local variables across calls",
 "STATIC count" },
 /* File locking */
 { "LOCK", "Lock file records",
 "LOCK #1, record" },
 { "UNLOCK", "Unlock file records",
 "UNLOCK #1, record" },
 { "RESET", "Close all open files",
 "RESET" },
 { "LOC", "Current file position",
 "PRINT LOC(1)" },
 { "IOCTL", "I/O control for device",
 "IOCTL #1, cmd$" },
 /* Logical operators */
 { "EQV", "Logical equivalence operator",
 "PRINT 5 EQV 3" },
 { "IMP", "Logical implication operator",
 "PRINT 5 IMP 3" },
 /* Function keys */
 { "KEY", "Assign function key string",
 "KEY 1, \"RUN\" + CHR$(13)" },
 { "LLIST", "List program to printer",
 "LLIST or LLIST 100-200" },
 /* Machine interface */
 { "SYS", "Call machine language routine",
 "SYS addr" },
 { "EXEC", "Fire-and-forget OS command",
 "EXEC \"notepad\"" },
 /* ECMA-116 structured error handling */
 { "WHEN", "Begin protected exception block",
 "WHEN EXCEPTION IN ... USE ... END WHEN" },
 { "CAUSE", "Raise an exception (ECMA-116)",
 "CAUSE EXCEPTION 1000" },
 /* Remaining GW-BASIC */
 { "LPOS", "Printer head column position",
 "PRINT LPOS(0)" },
 { "VIEW", "Set graphics viewport",
 "VIEW (x1,y1)-(x2,y2)" },
 { "WAIT", "Wait for port status",
 "WAIT port, AND_mask [,XOR_mask]" },
 { "WINDOW", "Set logical coordinate system",
 "WINDOW (0,0)-(639,199)" },
 /* FujiNet module */
 { "FUJINET", "FujiNet virtual device module",
 "MODULE \"FUJINET\" to activate N:/FUJI:/CLOCK: devices" },
 { "N:", "FujiNet network device",
 "OPEN #1,\"N:TCP://host:port/\",\"RW\" then READ/WRITE" },
 { "FUJI:", "FujiNet configuration device",
 "WiFi config, host slots, AppKey storage" },
 { "CLOCK:", "FujiNet NTP clock device",
 "Read time via CLOCK: device (ISO/binary formats)" },
 { "IOCTL", "Device control command",
 "IOCTL #ch, cmd [, arg]" },

 { NULL, NULL, NULL }
};

/* Category names for CATALOG */
static const char *cat_names[] = {
 "Core", "Math", "String", "I/O", "Utility", "User"
};

/* Security level names for INFO */
static const char *sec_names[] = {
 "OPEN", "STANDARD", "RESTRICTED"
};

/* --- help_show - Display command help ---
 */
void help_show(const char *topic)
{
 if (topic == NULL || topic[0] == '\0') {
 /* No topic - show command summary */
 int i;
 int fcount;

 printf("=== BASIC++ COMMAND REFERENCE ===\n\n");

 for (i = 0; help_db[i].keyword != NULL; i++) {
 printf(" %-10s %s\n",
 help_db[i].keyword,
 help_db[i].summary);
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
 printf("\n[Functions]\n");
 shown = 1;
 }
 printf(" %-10s %s\n",
 f->name,
 f->help_text);
 }
 }
 }

 printf("\nType HELP keyword for details.\n");
 printf("Type CATALOG for function list.\n");
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
 printf(" Dialect: %s\n", dialect_get_name());

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

/* --- help_catalog - List all registered functions ---
 */
void help_catalog(void)
{
 int count = funcreg_count();
 int cat;

 printf("=== FUNCTION CATALOG ===\n\n");

 for (cat = 0; cat < 6; cat++) {
 int i;
 int printed_header = 0;

 for (i = 0; i < count; i++) {
 const FunctionEntry *f = funcreg_get(i);
 if (f == NULL) continue;
 if ((int)f->category != cat) continue;

 if (!printed_header) {
 printf("[%s]\n", cat_names[cat]);
 printed_header = 1;
 }

 printf(" %-10s", f->name);
 if (f->min_args == f->max_args) {
 printf("(%d args)", f->min_args);
 } else {
 printf("(%d-%d args)",
 f->min_args, f->max_args);
 }
 printf("\n");
 }

 if (printed_header) {
 printf("\n");
 }
 }

 printf("%d functions registered.\n", count);
}
