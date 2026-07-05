/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: override.c
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
 // BASIC++ Interpreter - override.c
 // ---
 //
 // Keyword interpretation override implementation.
 //
 // The override table is a static array indexed by KeywordId.
 // Each entry stores a parse-time prefix string. When active,
 // the exec loop prepends this text to the keyword's arguments
 // before parsing. The user's stored source code is never
 // modified -- OVERRIDE affects interpretation only.
 //
 // Protected keywords (SCOPE, ALIAS, KEYWORD, OVERRIDE, REM,
 // SECURITY) cannot be overridden.
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
#include "override.h"
#include "lexer.h"
#include "../console.h"

// --- Override Table ---
static struct {
 char text[MAX_OVERRIDE_TEXT];
 int  active;
} override_table[KW_COUNT];

// --- Protected Keywords ---
static int is_override_protected(KeywordId kw)
{
 return (kw == KW_SCOPE || kw == KW_ALIAS ||
  kw == KW_KEYWORD || kw == KW_OVERRIDE ||
  kw == KW_REM || kw == KW_SECURITY);
}

// --- Init / Reset ---

void override_init(void)
{
 int i;
 for (i = 0; i < (int)KW_COUNT; i++) {
 override_table[i].active = 0;
 override_table[i].text[0] = '\0';
 }
}

void override_reset(void)
{
 override_init();
 printf("All overrides cleared.\n");
}

// --- Set / Get / Clear ---

int override_set(KeywordId kw, const char *text)
{
 int tlen;

 if (kw < 0 || kw >= KW_COUNT)
 return -1;

 if (is_override_protected(kw)) {
 printf("Cannot override %s (protected)\n",
  lexer_keyword_name(kw));
 return -1;
 }

 if (text == NULL || text[0] == '\0')
 return -1;

 tlen = (int)strlen(text);
 if (tlen >= MAX_OVERRIDE_TEXT)
 tlen = MAX_OVERRIDE_TEXT - 1;

 memcpy(override_table[kw].text, text, (size_t)tlen);
 override_table[kw].text[tlen] = '\0';
 override_table[kw].active = 1;
 return 0;
}

int override_is_active(KeywordId kw)
{
 if (kw < 0 || kw >= KW_COUNT) return 0;
 return override_table[kw].active;
}

const char *override_get(KeywordId kw)
{
 if (kw < 0 || kw >= KW_COUNT) return NULL;
 if (!override_table[kw].active) return NULL;
 return override_table[kw].text;
}

void override_clear(KeywordId kw)
{
 if (kw >= 0 && kw < KW_COUNT) {
 override_table[kw].active = 0;
 override_table[kw].text[0] = '\0';
 }
}

// --- Introspection ---

void override_list(void)
{
 int i, found = 0;

 printf("Active overrides:\n");
 for (i = 0; i < (int)KW_COUNT; i++) {
 if (override_table[i].active) {
  const char *name = lexer_keyword_name(
  (KeywordId)i);
  if (name[0] != '\0') {
  printf("  %-12s \"%s\"\n",
   name, override_table[i].text);
  found = 1;
  }
 }
 }
 if (!found)
 printf("  (none)\n");
}
