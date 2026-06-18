// ---
// BASIC++ Interpreter - keyword_props.c
// ---
//
// Per-keyword property table implementation.
//
// PURPOSE:
//   Each keyword can have up to MAX_KW_PROPS named properties
//   (key-value string pairs). Parser handlers check properties
//   via keyword_prop_get() / keyword_prop_is_on() to modify
//   their behavior at runtime.
//
//   Example BASIC usage:
//     KEYWORD PRINT UPPERCASE ON
//     KEYWORD FOR MAXITER 10000
//     KEYWORD INPUT PROMPT "Enter> "
//
// SUPPORTED PROPERTIES BY KEYWORD:
//   PRINT:  UPPERCASE, LOWERCASE, PREFIX, SUFFIX, NEWLINE, ZONE
//   INPUT:  PROMPT, UPPERCASE
//   GOTO:   STRICT, BOUNDS
//   GOSUB:  STRICT, BOUNDS
//   FOR:    MAXITER
//   LIST:   NUMBERS
//   RUN:    TRACE
//   REM:    VISIBLE
//   LET:    STRICT, READONLY
//
// HOW TO EXTEND:
//   Adding properties to a new keyword:
//   1. Add a case to keyword_prop_describe() below.
//   2. In the keyword's parser handler, check the property:
//        if (keyword_prop_is_on(KW_YOURNAME, "MYPROP")) { ... }
//   No other changes needed -- the table is fully generic.
//
// FINE-TUNING:
//   MAX_KW_PROPS (8): max properties per keyword.
//   MAX_PROP_NAME (32): max property name length.
//   MAX_PROP_VALUE (64): max property value length.
//
// ---

#include <stdio.h>
#include <string.h>
#include "keyword_props.h"
#include "lexer.h"

// --- Property Entry ---
typedef struct {
 char name[MAX_PROP_NAME];
 char value[MAX_PROP_VALUE];
} KwProp;

typedef struct {
 KwProp props[MAX_KW_PROPS];
 int count;
} KwPropSet;

// --- Global Property Table ---
static KwPropSet kw_props[KW_COUNT];

// --- Case-insensitive string compare ---
static int prop_eq(const char *a, const char *b)
{
 while (*a && *b) {
 char ca = *a, cb = *b;
 if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 32);
 if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 32);
 if (ca != cb) return 0;
 a++; b++;
 }
 return (*a == '\0' && *b == '\0');
}

// --- Init / Reset ---

void keyword_props_init(void)
{
 int i;
 for (i = 0; i < (int)KW_COUNT; i++) {
 kw_props[i].count = 0;
 memset(kw_props[i].props, 0,
 sizeof(kw_props[i].props));
 }
}

void keyword_props_reset(void)
{
 keyword_props_init();
 printf("All keyword properties cleared.\n");
}

// --- Set / Get / Remove ---

int keyword_prop_set(KeywordId kw, const char *name,
 const char *value)
{
 int i;
 KwPropSet *ps;
 int nlen, vlen;

 if (kw < 0 || kw >= KW_COUNT)
 return -1;

 ps = &kw_props[kw];

 // Update existing
 for (i = 0; i < ps->count; i++) {
 if (prop_eq(ps->props[i].name, name)) {
 vlen = (int)strlen(value);
 if (vlen > MAX_PROP_VALUE - 1)
 vlen = MAX_PROP_VALUE - 1;
 memcpy(ps->props[i].value,
 value, (size_t)vlen);
 ps->props[i].value[vlen] = '\0';
 return 0;
 }
 }

 // Add new
 if (ps->count >= MAX_KW_PROPS)
 return -1;

 nlen = (int)strlen(name);
 if (nlen > MAX_PROP_NAME - 1)
 nlen = MAX_PROP_NAME - 1;
 memcpy(ps->props[ps->count].name,
 name, (size_t)nlen);
 ps->props[ps->count].name[nlen] = '\0';
 // Uppercase the name
 for (i = 0; i < nlen; i++) {
 char c = ps->props[ps->count].name[i];
 if (c >= 'a' && c <= 'z')
 ps->props[ps->count].name[i] =
 (char)(c - 32);
 }

 vlen = (int)strlen(value);
 if (vlen > MAX_PROP_VALUE - 1)
 vlen = MAX_PROP_VALUE - 1;
 memcpy(ps->props[ps->count].value,
 value, (size_t)vlen);
 ps->props[ps->count].value[vlen] = '\0';

 ps->count++;
 return 0;
}

const char *keyword_prop_get(KeywordId kw,
 const char *name)
{
 int i;
 KwPropSet *ps;

 if (kw < 0 || kw >= KW_COUNT)
 return NULL;

 ps = &kw_props[kw];

 for (i = 0; i < ps->count; i++) {
 if (prop_eq(ps->props[i].name, name))
 return ps->props[i].value;
 }
 return NULL;
}

int keyword_prop_is_on(KeywordId kw, const char *name)
{
 const char *v = keyword_prop_get(kw, name);
 if (v == NULL) return 0;
 if (prop_eq(v, "ON") || prop_eq(v, "1") ||
 prop_eq(v, "YES") || prop_eq(v, "TRUE"))
 return 1;
 return 0;
}

int keyword_prop_get_int(KeywordId kw, const char *name,
 int default_val)
{
 const char *v = keyword_prop_get(kw, name);
 int result = 0;
 int neg = 0;
 int digit;
 const char *p;

 if (v == NULL) return default_val;

 p = v;
 if (*p == '-') { neg = 1; p++; }
 while (*p >= '0' && *p <= '9') {
 digit = *p - '0';
 // Overflow guard: 2147483647 / 10 = 214748364
 if (result > 214748364 ||
  (result == 214748364 && digit > 7))
  return default_val;
 result = result * 10 + digit;
 p++;
 }
 if (p == v || (neg && p == v + 1))
 return default_val;

 return neg ? -result : result;
}

int keyword_prop_remove(KeywordId kw, const char *name)
{
 int i;
 KwPropSet *ps;

 if (kw < 0 || kw >= KW_COUNT)
 return -1;

 ps = &kw_props[kw];

 for (i = 0; i < ps->count; i++) {
 if (prop_eq(ps->props[i].name, name)) {
 // Shift remaining
 int j;
 for (j = i; j < ps->count - 1; j++)
 ps->props[j] = ps->props[j + 1];
 ps->count--;
 return 0;
 }
 }
 return -1;
}

void keyword_prop_clear(KeywordId kw)
{
 if (kw >= 0 && kw < KW_COUNT) {
 kw_props[kw].count = 0;
 }
}

int keyword_prop_count(KeywordId kw)
{
 if (kw < 0 || kw >= KW_COUNT) return 0;
 return kw_props[kw].count;
}

// --- Introspection ---

void keyword_prop_list(KeywordId kw)
{
 int i;
 KwPropSet *ps;
 const char *kname;

 if (kw < 0 || kw >= KW_COUNT) return;
 ps = &kw_props[kw];
 kname = lexer_keyword_name(kw);

 if (ps->count == 0) {
 printf("%s: (default behavior)\n", kname);
 return;
 }

 printf("%s properties:\n", kname);
 for (i = 0; i < ps->count; i++) {
 printf("  %-12s = %s\n",
 ps->props[i].name,
 ps->props[i].value);
 }
}

void keyword_prop_list_all(void)
{
 int i, found = 0;

 printf("Keywords with active properties:\n");
 for (i = 0; i < (int)KW_COUNT; i++) {
 if (kw_props[i].count > 0) {
 keyword_prop_list((KeywordId)i);
 found = 1;
 }
 }
 if (!found)
 printf("  (none)\n");
}

// --- Built-in Documentation ---

void keyword_prop_describe(KeywordId kw)
{
 const char *name = lexer_keyword_name(kw);

 printf("Available properties for %s:\n", name);

 switch (kw) {
 case KW_PRINT:
 printf(
 "  UPPERCASE  ON/OFF"
 "  Force output to uppercase\n"
 "  LOWERCASE  ON/OFF"
 "  Force output to lowercase\n"
 "  PREFIX     string"
 "  Prepend to every output\n"
 "  SUFFIX     string"
 "  Append to every output\n"
 "  NEWLINE    ON/OFF"
 "  Control trailing newline\n"
 "  ZONE       number"
 "  Print zone width\n");
 break;
 case KW_INPUT:
 printf(
 "  PROMPT     string"
 "  Custom default prompt\n"
 "  UPPERCASE  ON/OFF"
 "  Force input to uppercase\n");
 break;
 case KW_GOTO:
 case KW_GOSUB:
 printf(
 "  STRICT     ON/OFF"
 "  Error if target line missing\n"
 "  BOUNDS     lo-hi "
 "  Restrict jump range\n");
 break;
 case KW_FOR:
 printf(
 "  MAXITER    number"
 "  Maximum loop iterations\n");
 break;
 case KW_LIST:
 printf(
 "  NUMBERS    ON/OFF"
 "  Show/hide line numbers\n");
 break;
 case KW_RUN:
 printf(
 "  TRACE      ON/OFF"
 "  Auto-enable TRON on RUN\n");
 break;
 case KW_REM:
 printf(
 "  VISIBLE    ON/OFF"
 "  Show REM text during execution\n");
 break;
 case KW_LET:
 printf(
 "  STRICT     ON/OFF"
 "  Require explicit LET keyword\n"
 "  READONLY   ON/OFF"
 "  Prevent variable overwrites\n");
 break;
 default:
 printf("  (no documented properties"
 " -- custom properties allowed)\n");
 break;
 }
}
