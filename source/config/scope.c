 // ---
 // BASIC++ Interpreter - scope.c
 // ---
 //
 // SCOPE keyword implementation: keyword access control
 // and behavior hooks.
 //
 // The scope table is a static array indexed by KeywordId.
 // Each entry tracks disabled state and hook line numbers.
 //
 // Presets disable groups of keywords for common use cases:
 // - STRUCTURED: disable GOTO for clean control flow
 // - SAFE: disable low-level access (PEEK/POKE/EXEC)
 // - MINIMAL: disable I/O and system access
 // - EDUCATIONAL: beginner-friendly subset
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
#include "scope.h"
#include "lexer.h"

// --- Scope Table ---
static ScopeEntry scope_table[KW_COUNT];
static KeywordId last_hook_kw = KW_COUNT;

// --- Case-insensitive compare ---
static int sci_eq(const char *a, const char *b)
{
 while (*a && *b) {
 char ca, cb;
 ca = *a; cb = *b;
 if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
 if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
 if (ca != cb) return 0;
 a++; b++;
 }
 return (*a == '\0' && *b == '\0');
}

// --- Init / Reset ---

void scope_init(void)
{
 int i;
 for (i = 0; i < (int)KW_COUNT; i++) {
 scope_table[i].disabled = 0;
 scope_table[i].before_line = -1;
 scope_table[i].after_line = -1;
 scope_table[i].override_line = -1;
 }
 last_hook_kw = KW_COUNT;
}

void scope_reset(void)
{
 scope_init();
 printf("All scope rules cleared.\n");
}

// --- Protected Keywords ---
 // These keywords cannot be disabled or hooked via SCOPE.
 // Prevents lock-out scenarios where the user disables
 // the tools needed to undo the damage.
static int is_protected(KeywordId kw)
{
 return (kw == KW_SCOPE || kw == KW_ALIAS ||
  kw == KW_KEYWORD || kw == KW_REM ||
  kw == KW_SECURITY || kw == KW_OVERRIDE);
}

// --- Disable / Enable ---

int scope_is_disabled(KeywordId kw)
{
 if (kw < 0 || kw >= KW_COUNT) return 0;
 return scope_table[kw].disabled;
}

void scope_disable(KeywordId kw)
{
 if (kw < 0 || kw >= KW_COUNT) return;
 if (is_protected(kw)) {
  printf("Cannot disable %s (protected)\n",
   lexer_keyword_name(kw));
  return;
 }
 scope_table[kw].disabled = 1;
}

void scope_enable(KeywordId kw)
{
 if (kw >= 0 && kw < KW_COUNT)
 scope_table[kw].disabled = 0;
}

// --- Hooks ---

void scope_set_before(KeywordId kw, int line)
{
 if (kw < 0 || kw >= KW_COUNT) return;
 if (is_protected(kw)) {
  printf("Cannot hook %s (protected)\n",
   lexer_keyword_name(kw));
  return;
 }
 scope_table[kw].before_line = line;
}

void scope_set_after(KeywordId kw, int line)
{
 if (kw < 0 || kw >= KW_COUNT) return;
 if (is_protected(kw)) {
  printf("Cannot hook %s (protected)\n",
   lexer_keyword_name(kw));
  return;
 }
 scope_table[kw].after_line = line;
}

void scope_set_override(KeywordId kw, int line)
{
 if (kw < 0 || kw >= KW_COUNT) return;
 if (is_protected(kw)) {
  printf("Cannot hook %s (protected)\n",
   lexer_keyword_name(kw));
  return;
 }
 scope_table[kw].override_line = line;
}

int scope_get_before(KeywordId kw)
{
 if (kw < 0 || kw >= KW_COUNT) return -1;
 return scope_table[kw].before_line;
}

int scope_get_after(KeywordId kw)
{
 if (kw < 0 || kw >= KW_COUNT) return -1;
 return scope_table[kw].after_line;
}

int scope_get_override(KeywordId kw)
{
 if (kw < 0 || kw >= KW_COUNT) return -1;
 return scope_table[kw].override_line;
}

void scope_clear_hooks(KeywordId kw)
{
 if (kw >= 0 && kw < KW_COUNT) {
 scope_table[kw].before_line = -1;
 scope_table[kw].after_line = -1;
 scope_table[kw].override_line = -1;
 }
}

void scope_restore(KeywordId kw)
{
 if (kw >= 0 && kw < KW_COUNT) {
 scope_table[kw].disabled = 0;
 scope_clear_hooks(kw);
 }
}

// --- Introspection ---

int scope_has_any_rules(void)
{
 int i;
 for (i = 0; i < (int)KW_COUNT; i++) {
 if (scope_table[i].disabled ||
 scope_table[i].before_line >= 0 ||
 scope_table[i].after_line >= 0 ||
 scope_table[i].override_line >= 0)
 return 1;
 }
 return 0;
}

void scope_set_last_kw(KeywordId kw)
{
 last_hook_kw = kw;
}

const char *scope_get_last_kw_name(void)
{
 if (last_hook_kw >= 0 && last_hook_kw < KW_COUNT)
 return lexer_keyword_name(last_hook_kw);
 return "";
}

void scope_list(void)
{
 int i, found = 0;
 const char *name;

 printf("Active scope rules:\n");
 for (i = 0; i < (int)KW_COUNT; i++) {
 ScopeEntry *e = &scope_table[i];
 if (!e->disabled &&
 e->before_line < 0 &&
 e->after_line < 0 &&
 e->override_line < 0)
 continue;

 name = lexer_keyword_name((KeywordId)i);
 if (name[0] == '\0') continue;

 if (e->disabled) {
 printf("  %-12s DISABLED\n", name);
 found = 1;
 }
 if (e->before_line >= 0) {
 printf("  %-12s BEFORE GOSUB %d\n",
 name, e->before_line);
 found = 1;
 }
 if (e->after_line >= 0) {
 printf("  %-12s AFTER GOSUB %d\n",
 name, e->after_line);
 found = 1;
 }
 if (e->override_line >= 0) {
 printf("  %-12s OVERRIDE GOSUB %d\n",
 name, e->override_line);
 found = 1;
 }
 }
 if (!found)
 printf("  (none)\n");
}

// --- Presets ---

int scope_load_preset(const char *name)
{
 if (sci_eq(name, "STRUCTURED")) {
 scope_disable(KW_GOTO);
 scope_disable(KW_ON); // ON...GOTO
 printf("Preset STRUCTURED: GOTO, ON"
 " disabled.\n");
 return 0;
 }
 if (sci_eq(name, "SAFE")) {
 scope_disable(KW_PEEK);
 scope_disable(KW_POKE);
 scope_disable(KW_SYS);
 scope_disable(KW_BSAVE);
 scope_disable(KW_BLOAD);
 scope_disable(KW_SYSTEM);
 printf("Preset SAFE: PEEK, POKE, SYS,"
 " BSAVE, BLOAD, SYSTEM disabled.\n");
 return 0;
 }
 if (sci_eq(name, "MINIMAL")) {
 scope_disable(KW_PEEK);
 scope_disable(KW_POKE);
 scope_disable(KW_SYS);
 scope_disable(KW_BSAVE);
 scope_disable(KW_BLOAD);
 scope_disable(KW_SYSTEM);
 scope_disable(KW_OPEN);
 scope_disable(KW_CLOSE);
 scope_disable(KW_SAVE);
 scope_disable(KW_LOAD);
 scope_disable(KW_CHAIN);
 scope_disable(KW_MERGE);
 printf("Preset MINIMAL: low-level, file"
 " I/O, and system access disabled.\n");
 return 0;
 }
 if (sci_eq(name, "EDUCATIONAL")) {
 scope_disable(KW_GOTO);
 scope_disable(KW_ON);
 scope_disable(KW_PEEK);
 scope_disable(KW_POKE);
 scope_disable(KW_SYS);
 scope_disable(KW_BSAVE);
 scope_disable(KW_BLOAD);
 scope_disable(KW_SYSTEM);
 printf("Preset EDUCATIONAL: GOTO, ON, PEEK,"
 " POKE, SYS, SYSTEM disabled.\n");
 return 0;
 }
 return -1;
}

void scope_list_presets(void)
{
 printf("Available SCOPE presets:\n");
 printf("  STRUCTURED  - Disable GOTO,"
 " ON...GOTO\n");
 printf("  SAFE        - Disable PEEK, POKE,"
 " SYS, BSAVE, BLOAD, SYSTEM\n");
 printf("  MINIMAL     - Disable file I/O,"
 " low-level, system access\n");
 printf("  EDUCATIONAL - Disable GOTO + all"
 " low-level access\n");
 printf("\nUsage: SCOPE \"STRUCTURED\"\n");
}
