 // ---
 // BASIC++ Interpreter - override.h
 // ---
 //
 // Keyword interpretation override system.
 //
 // OVERRIDE changes how the parser interprets a keyword
 // without modifying the user's source code (.BAS). When
 // active, the interpreter internally prepends additional
 // tokens to the keyword's argument list at parse time.
 //
 // Example:
 //   OVERRIDE PRINT "PRINT TAB(7);"
 //
 // After this, every PRINT statement is interpreted as if
 // it began with TAB(7). The stored program line is never
 // modified -- OVERRIDE affects interpretation only.
 //
 // Unlike ALIAS (which renames), SCOPE (which hooks/blocks),
 // or KEYWORD (which sets properties), OVERRIDE changes how
 // the parser processes the keyword's arguments.
 //
//
// HOW TO EXTEND:
//   See the corresponding .c implementation file for
//   detailed extension and customization instructions.
//
// TROUBLESHOOTING:
//   If you get 'undeclared identifier' errors after adding
//   new functions, make sure the declaration is added here
//   AND the definition exists in the .c file.
 // ---

#ifndef BASICPP_OVERRIDE_H
#define BASICPP_OVERRIDE_H

#include "lexer.h"

#define MAX_OVERRIDE_TEXT 128

 // override_init - Initialize override table (all inactive).
void override_init(void);

 // override_reset - Clear all overrides.
void override_reset(void);

 // override_set - Set an interpretation override for a keyword.
 // Returns 0 on success, -1 on error (protected keyword,
 // text too long, bad keyword ID).
int override_set(KeywordId kw, const char *text);

 // override_is_active - Check if a keyword has an override.
int override_is_active(KeywordId kw);

 // override_get - Get override text for a keyword.
 // Returns NULL if not active.
const char *override_get(KeywordId kw);

 // override_clear - Remove override for a single keyword.
void override_clear(KeywordId kw);

 // override_list - Print all active overrides.
void override_list(void);

#endif // BASICPP_OVERRIDE_H
