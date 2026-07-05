/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: keyword_props_core.c
 * Subsystem: Keyword Dynamic Behavior Overrides
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Maintains keyword flags (e.g. UPPERCASE) to modify parser rules.
 *
 * 2. WHAT TO EXPECT:
 *    Modifies syntax validation on the fly.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Default property flags, property definitions.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Property lookup rules.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If override fails to apply, verify dialect validation flags.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE KEYWORD PROPERTY CORE
 * File: keyword_props_core.c
 * ===================================================================== */

#include <string.h>
#include "keyword_props_core.h"

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

void keyword_props_core_init(KwPropSet *table, int num_kws)
{
    if (table) {
        int i;
        for (i = 0; i < num_kws; i++) {
            table[i].count = 0;
            memset(table[i].props, 0, sizeof(table[i].props));
        }
    }
}

int keyword_props_core_set(KwPropSet *table, int num_kws, int kw, const char *name, const char *value)
{
    int i;
    KwPropSet *ps;
    int nlen, vlen;

    if (!table || kw < 0 || kw >= num_kws || !name || !value) {
        return -1;
    }

    ps = &table[kw];

    // Update existing property in-place
    for (i = 0; i < ps->count; i++) {
        if (prop_eq(ps->props[i].name, name)) {
            vlen = (int)strlen(value);
            if (vlen > MAX_PROP_VALUE - 1) {
                vlen = MAX_PROP_VALUE - 1;
            }
            memcpy(ps->props[i].value, value, (size_t)vlen);
            ps->props[i].value[vlen] = '\0';
            return 0;
        }
    }

    // Add new property
    if (ps->count >= MAX_KW_PROPS) {
        return -1;
    }

    nlen = (int)strlen(name);
    if (nlen > MAX_PROP_NAME - 1) {
        nlen = MAX_PROP_NAME - 1;
    }
    memcpy(ps->props[ps->count].name, name, (size_t)nlen);
    ps->props[ps->count].name[nlen] = '\0';

    // Force property name to uppercase
    for (i = 0; i < nlen; i++) {
        char c = ps->props[ps->count].name[i];
        if (c >= 'a' && c <= 'z') {
            ps->props[ps->count].name[i] = (char)(c - 32);
        }
    }

    vlen = (int)strlen(value);
    if (vlen > MAX_PROP_VALUE - 1) {
        vlen = MAX_PROP_VALUE - 1;
    }
    memcpy(ps->props[ps->count].value, value, (size_t)vlen);
    ps->props[ps->count].value[vlen] = '\0';

    ps->count++;
    return 0;
}

const char *keyword_props_core_get(const KwPropSet *table, int num_kws, int kw, const char *name)
{
    int i;
    const KwPropSet *ps;

    if (!table || kw < 0 || kw >= num_kws || !name) {
        return NULL;
    }

    ps = &table[kw];

    for (i = 0; i < ps->count; i++) {
        if (prop_eq(ps->props[i].name, name)) {
            return ps->props[i].value;
        }
    }
    return NULL;
}

int keyword_props_core_is_on(const KwPropSet *table, int num_kws, int kw, const char *name)
{
    const char *v = keyword_props_core_get(table, num_kws, kw, name);
    if (v == NULL) return 0;
    if (prop_eq(v, "ON") || prop_eq(v, "1") ||
        prop_eq(v, "YES") || prop_eq(v, "TRUE")) {
        return 1;
    }
    return 0;
}

int keyword_props_core_get_int(const KwPropSet *table, int num_kws, int kw, const char *name, int default_val)
{
    const char *v = keyword_props_core_get(table, num_kws, kw, name);
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
        if (result > 214748364 || (result == 214748364 && digit > 7)) {
            return default_val;
        }
        result = result * 10 + digit;
        p++;
    }
    if (p == v || (neg && p == v + 1)) {
        return default_val;
    }

    return neg ? -result : result;
}

void keyword_props_core_remove(KwPropSet *table, int num_kws, int kw, const char *name)
{
    int i;
    KwPropSet *ps;

    if (table && kw >= 0 && kw < num_kws && name) {
        ps = &table[kw];
        for (i = 0; i < ps->count; i++) {
            if (prop_eq(ps->props[i].name, name)) {
                int j;
                for (j = i; j < ps->count - 1; j++) {
                    ps->props[j] = ps->props[j + 1];
                }
                ps->count--;
                return;
            }
        }
    }
}
