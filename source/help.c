/*
 * =====================================================================
 * BASIC++ Interpreter - help.c
 * =====================================================================
 *
 * Help & introspection system.
 *
 * Makes BASIC++ self-describing: every command, function, and
 * system setting is documented within the interpreter itself.
 *
 * =====================================================================
 */

#include <stdio.h>
#include <string.h>
#include "help.h"
#include "config.h"
#include "funcreg.h"
#include "dialect.h"
#include "security.h"
#include "module.h"

/* =====================================================================
 * Command Help Database
 * =====================================================================
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
 "REM This is a comment" },
 { "DIM", "Declare an array with dimensions",
 "DIM A(10) or DIM M(3,3)" },
 { "DATA", "Embed data values in the program",
 "DATA 1,2,3,\"HELLO\"" },
 { "READ", "Read the next DATA value",
 "READ A or READ A$" },
 { "RESTORE", "Reset DATA pointer to beginning",
 "RESTORE" },

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
 { "COMPILE", "Transpile BASIC to C source",
 "COMPILE \"output.c\"" },

 /* Flow control */
 { "ON", "Computed GOTO/GOSUB or error handler",
 "ON X GOTO 100,200 or ON ERROR GOTO 500" },
 { "WHILE", "Begin a conditional loop",
 "WHILE A>0" },
 { "WEND", "End of a WHILE loop",
 "WEND" },
 { "DO", "Begin a DO loop",
 "DO" },
 { "LOOP", "End of a DO loop (with condition)",
 "LOOP WHILE A>0 or LOOP UNTIL A=0" },

 /* String/display */
 { "CLS", "Clear the screen",
 "CLS" },
 { "CLR", "Clear all variables",
 "CLR" },
 { "TAB", "Move cursor to column position",
 "PRINT TAB(20);\"HERE\"" },
 { "SPC", "Print N spaces",
 "PRINT SPC(10);\"TEXT\"" },

 /* Tracing */
 { "TRON", "Enable line trace output",
 "TRON" },
 { "TROFF", "Disable line trace output",
 "TROFF" },

 /* Functions */
 { "DEF", "Define a user function",
 "DEF FNA(X)=X*X+1" },

 /* Matrix */
 { "MAT", "Matrix operations",
 "MAT PRINT A or MAT A=ZER" },

 /* File I/O */
 { "OPEN", "Open a file channel",
 "OPEN #1,\"data.txt\",\"R\"" },
 { "CLOSE", "Close a file channel",
 "CLOSE #1" },
 { "FPRINT", "Print to a file channel",
 "FPRINT #1,A,B" },
 { "FINPUT", "Read from a file channel",
 "FINPUT #1,A$" },

 /* Bytecode */
 { "BSAVE", "Save compiled bytecode (BASIC++ extension)",
 "BSAVE \"prog.bpp\"" },
 { "BLOAD", "Load compiled bytecode (BASIC++ extension)",
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
 { "ENDTEST", "End test block and report results",
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

 /* Final polish */
 { "RENUM", "Renumber program lines",
 "RENUM or RENUM 100,5" },
 { "DELETE", "Delete lines by range",
 "DELETE 100 or DELETE 100-200" },
 { "VER", "Display version information",
 "VER" },

 /* GW-BASIC compatibility */
 { "SWAP", "Exchange two variables",
 "SWAP A,B" },
 { "RANDOMIZE","Seed the random number generator",
 "RANDOMIZE or RANDOMIZE 42" },
 { "ELSE", "Alternate branch of IF statement",
 "IF A>5 THEN PRINT \"BIG\" ELSE PRINT \"SMALL\"" },
 { "FILES", "List files in current directory",
 "FILES or FILES \"*.BAS\"" },
 { "BEEP", "Emit an audible beep",
 "BEEP" },
 { "SOUND", "Play a tone at given frequency",
 "SOUND freq, duration\n"
 " freq = frequency in Hz (37-32767)\n"
 " duration = clock ticks (18.2/sec)\n"
 " SOUND 0,n = silence for n ticks\n"
 "Example:\n"
 " SOUND 440, 18 ' A4 for ~1 second\n"
 " SOUND 523, 9 ' C5 for ~0.5 sec\n"
 " SOUND 0, 18 ' silence 1 sec\n"
 "Windows: plays real tones via Beep()\n"
 "Other: best-effort terminal bell" },
 { "PLAY", "Play music using macro language",
 "PLAY string$\n"
 "Music commands in string:\n"
 " C D E F G A B - notes\n"
 " # or + - sharp (e.g. C#)\n"
 " - - flat (e.g. B-)\n"
 " O0-O6 - set octave (default 4)\n"
 " > < - octave up / down\n"
 " L1-L64 - note length (default 4)\n"
 " T32-T255 - tempo BPM (default 120)\n"
 " Pn - pause for length n\n"
 " Nn - note by number (0-84)\n"
 " . - dotted note (1.5x)\n"
 " MN/ML/MS - staccato (ignored)\n"
 "Example:\n"
 " PLAY \"T120 O4 L4 CDEFGAB\"\n"
 " PLAY \"T180 L8 CDE P8 CDE\"\n"
 " PLAY \"O3 C#4 D#4 F#4\"" },
 { "SCREEN", "Set screen mode",
 "SCREEN mode [,color]\n"
 " mode 0 = text 80x25 (default)\n"
 " mode 1 = 320x200 (4 color)\n"
 " mode 2 = 640x200 (2 color)\n"
 " mode 7-13 = EGA/VGA modes\n"
 "Text-mode interpreter: all modes\n"
 "use character canvas for DRAW.\n"
 "Example:\n"
 " SCREEN 1\n"
 " SCREEN 0 ' back to text" },
 { "DRAW", "Draw using graphics macro language",
 "DRAW string$\n"
 "Direction commands:\n"
 " U[n] D[n] L[n] R[n] - up/down/left/right\n"
 " E[n] F[n] G[n] H[n] - diagonals\n"
 " M x,y - move to position\n"
 " M +x,+y - relative move\n"
 "Prefixes:\n"
 " B - move without drawing\n"
 " N - return to start after drawing\n"
 "Settings:\n"
 " Cn - set color/pen (0-7)\n"
 " Sn - scale (4=normal)\n"
 " An - angle (0-3, 90-deg steps)\n"
 "Renders to 80x50 text canvas.\n"
 "Example:\n"
 " DRAW \"R10 D10 L10 U10\" ' square\n"
 " DRAW \"BM20,10 R5 F5 L10 E5\"" },
 { "WIDTH", "Set screen width (columns)",
 "WIDTH columns [,lines]\n"
 " WIDTH 80 - 80-column mode\n"
 " WIDTH 40 - 40-column mode\n"
 " WIDTH 80,25 - 80 cols, 25 lines\n"
 " WIDTH - display current\n"
 "Valid range: 1-255 columns, 1-60 lines" },
 { "MOD", "Modulo (remainder) operator",
 "expr1 MOD expr2\n"
 "Returns the integer remainder.\n"
 "Example:\n"
 " PRINT 10 MOD 3 ' prints 1\n"
 " PRINT 17 MOD 5 ' prints 2\n"
 " PRINT 10.5 MOD 3 ' prints 1\n"
 " A = B MOD C" },
 { "COLOR", "Set text foreground/background color",
 "COLOR 14,1 (yellow on blue) or COLOR (reset)" },
 { "DIR", "List filenames only (no details)",
 "DIR or DIR \"*.BAS\"" },
 { "AUTO", "Auto-generate line numbers",
 "AUTO or AUTO 100,5 (empty line or . to stop)" },

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

/* =====================================================================
 * help_show - Display command help
 * =====================================================================
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

/* =====================================================================
 * help_info - Display system information
 * =====================================================================
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

/* =====================================================================
 * help_catalog - List all registered functions
 * =====================================================================
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
