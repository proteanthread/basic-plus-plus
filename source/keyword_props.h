 // ---
 // BASIC++ Interpreter - keyword_props.h
 // ---
 //
 // Per-keyword property system. Each keyword can have
 // named properties (key-value strings) that modify its
 // built-in behavior.
 //
 // Example:
 //   KEYWORD PRINT UPPERCASE ON
 //   KEYWORD PRINT PREFIX ">>> "
 //   KEYWORD INPUT PROMPT "Enter: "
 //   KEYWORD GOTO STRICT ON
 //
 // Properties are keyword-specific. The parser handlers
 // check their properties at execution time.
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

#ifndef BASICPP_KEYWORD_PROPS_H
#define BASICPP_KEYWORD_PROPS_H

#include "lexer.h"

#define MAX_KW_PROPS   8
#define MAX_PROP_NAME  16
#define MAX_PROP_VALUE 64

 // keyword_props_init - Initialize property system.
void keyword_props_init(void);

 // keyword_props_reset - Clear all properties.
void keyword_props_reset(void);

 // keyword_prop_set - Set a property on a keyword.
 // Returns 0 on success, -1 if table full.
int keyword_prop_set(KeywordId kw, const char *name,
 const char *value);

 // keyword_prop_get - Get a property value.
 // Returns NULL if not set.
const char *keyword_prop_get(KeywordId kw,
 const char *name);

 // keyword_prop_is_on - Check if a boolean property
 // is "ON", "1", "YES", or "TRUE".
int keyword_prop_is_on(KeywordId kw, const char *name);

 // keyword_prop_get_int - Get a numeric property.
 // Returns default_val if not set or not numeric.
int keyword_prop_get_int(KeywordId kw, const char *name,
 int default_val);

 // keyword_prop_remove - Remove a single property.
 // Returns 0 on success, -1 if not found.
int keyword_prop_remove(KeywordId kw, const char *name);

 // keyword_prop_clear - Clear all properties for a keyword.
void keyword_prop_clear(KeywordId kw);

 // keyword_prop_list - Print properties for a keyword.
void keyword_prop_list(KeywordId kw);

 // keyword_prop_list_all - Print all keywords with properties.
void keyword_prop_list_all(void);

 // keyword_prop_count - Count properties for a keyword.
int keyword_prop_count(KeywordId kw);

 // keyword_prop_describe - Print available properties
 // for a keyword (built-in documentation).
void keyword_prop_describe(KeywordId kw);

#endif // BASICPP_KEYWORD_PROPS_H
