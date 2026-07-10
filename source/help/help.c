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
// --- Command Help Database ---
const char *help_cat_names[HCAT_COUNT] = {
    "Control Flow",
    "Input / Output",
    "Arithmetic / Math",
    "String Functions",
    "Variables & Memory",
    "File I/O",
    "Graphics",
    "Sound",
    "System & Environ",
    "Debug & Testing",
    "Program Mgmt",
    "Editing",
    "Devices & Network",
    "Operators",
    "Introspection",
    "System Variables"
};

extern const HelpEntry help_db_flow[];
extern const HelpEntry help_db_io[];
extern const HelpEntry help_db_math[];
extern const HelpEntry help_db_string[];
extern const HelpEntry help_db_varmem[];
extern const HelpEntry help_db_fileio[];
extern const HelpEntry help_db_gfx[];
extern const HelpEntry help_db_sound[];
extern const HelpEntry help_db_sysenv[];
extern const HelpEntry help_db_intro[];
extern const HelpEntry help_db_sysvar[];
extern const HelpEntry help_db_oper[];
extern const HelpEntry help_db_progmgmt[];
extern const HelpEntry help_db_testing[];
extern const HelpEntry help_db_devices[];


const HelpEntry *help_dbs[] = {
    help_db_flow,
    help_db_io,
    help_db_math,
    help_db_string,
    help_db_varmem,
    help_db_fileio,
    help_db_gfx,
    help_db_sound,
    help_db_sysenv,
    help_db_intro,
    help_db_sysvar,
    help_db_oper,
    help_db_progmgmt,
    help_db_testing,
    help_db_devices,
    NULL
};

 // Each entry maps a keyword to a short description, usage
 // example, and category. Entries within each category are
 // sorted alphabetically by keyword.
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

// Sort an array of pointers into help_db[] by keyword name
static void sort_entries(const HelpEntry **entries, int count)
{
    int i, j;
    for (i = 1; i < count; i++) {
        const HelpEntry *key = entries[i];
        j = i - 1;
        while (j >= 0 &&
               str_cmp_ci(entries[j]->keyword,
                          key->keyword) > 0) {
            entries[j + 1] = entries[j];
            j--;
        }
        entries[j + 1] = key;
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
 const HelpEntry *sorted[512];
 int scount;

 printf("=== BASIC++ COMMAND REFERENCE ===\n\n");

 for (cat = 0; cat < HCAT_COUNT; cat++) {
 // Collect indices for this category
 scount = 0;
 int d;
        const HelpEntry *db;
        for (d = 0; help_dbs[d] != NULL; d++) {
            db = help_dbs[d];
            for (i = 0; db[i].keyword != NULL; i++) {
                if ((int)db[i].category == cat && scount < 512) {
                    sorted[scount++] = &db[i];
                }
            }
        }
 if (scount == 0) continue;

 // Sort alphabetically
 sort_entries(sorted, scount);

 printf("[%s]\n", help_cat_names[cat]);
 for (i = 0; i < scount; i++) {
 printf(" %-12s %s\n", sorted[i]->keyword, sorted[i]->summary);
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
 int d;
        const HelpEntry *db;
        for (d = 0; help_dbs[d] != NULL && !found; d++) {
            db = help_dbs[d];
            for (i = 0; db[i].keyword != NULL; i++) {
                const char *a = topic;
                const char *b = db[i].keyword;
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
     db[i].keyword,
     db[i].summary);
 printf("Usage: %s\n",
     db[i].usage);
 printf("Category: %s\n",
     help_cat_names[
         db[i].category]);
 found = 1;
 break;
 }
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
 const HelpEntry *sorted[512];
 int scount;

 printf("=== BASIC++ CATALOG ===\n\n");

 // --- Part 1: Statements & Commands (sorted) ---
 for (cat = 0; cat < HCAT_COUNT; cat++) {
 int col;

 // Collect indices for this category
 scount = 0;
 int d;
        const HelpEntry *db;
        for (d = 0; help_dbs[d] != NULL; d++) {
            db = help_dbs[d];
            for (i = 0; db[i].keyword != NULL; i++) {
                if ((int)db[i].category == cat && scount < 512) {
                    sorted[scount++] = &db[i];
                }
            }
        }
 if (scount == 0) continue;

 // Sort alphabetically
 sort_entries(sorted, scount);

 printf("[%s]\n ", help_cat_names[cat]);
 col = 1;

 for (j = 0; j < scount; j++) {
 int klen = (int)strlen(sorted[j]->keyword);
   if (col + klen + 2 > 72) {
   printf("\n ");
   col = 1;
   }
   printf("%-15s", sorted[j]->keyword);
 col += 15;
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
 printf("%-15s", f->name);
 col += 15;
 }
 printf("\n\n");
 }
 }

 printf("%d commands, %d functions registered.\n",
 total_cmds, fcount);
}
#endif

bool help_has_keyword(const char *keyword) {
    if (!keyword) return false;
    int d, i;
    for (d = 0; help_dbs[d] != NULL; d++) {
        const HelpEntry *db = help_dbs[d];
        for (i = 0; db[i].keyword != NULL; i++) {
            if (str_cmp_ci(db[i].keyword, keyword) == 0) {
                return true;
            }
        }
    }
    return false;
}
