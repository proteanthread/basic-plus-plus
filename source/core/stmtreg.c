/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: stmtreg.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Implementation of the dynamic Statement Registry subsystem.
 * ===================================================================== */

#include "stmtreg.h"
#include <string.h>

static int matches_nocase(const char *a, const char *b, int len);

static StmtEntry g_stmt_table[MAX_STATEMENT_ENTRIES];
static int g_stmt_count = 0;

#define MAX_CMD_ALIASES 64

typedef struct {
    char name[32];
    char expansion[256];
} CmdAlias;

static CmdAlias g_cmd_aliases[MAX_CMD_ALIASES];
static int g_cmd_alias_count = 0;

void cmd_alias_clear(void) {
    g_cmd_alias_count = 0;
    memset(g_cmd_aliases, 0, sizeof(g_cmd_aliases));
}

int cmd_alias_register(const char *name, int name_len, const char *expansion, int exp_len) {
    if (g_cmd_alias_count >= MAX_CMD_ALIASES) return -1;
    if (name_len <= 0 || name_len >= 32 || exp_len <= 0 || exp_len >= 256) return -2;
    
    // Check for duplicates
    int i;
    for (i = 0; i < g_cmd_alias_count; i++) {
        if (matches_nocase(name, g_cmd_aliases[i].name, name_len) && g_cmd_aliases[i].name[name_len] == '\0') {
            memcpy(g_cmd_aliases[i].expansion, expansion, (size_t)exp_len);
            g_cmd_aliases[i].expansion[exp_len] = '\0';
            return 0;
        }
    }
    
    CmdAlias *entry = &g_cmd_aliases[g_cmd_alias_count++];
    memcpy(entry->name, name, (size_t)name_len);
    entry->name[name_len] = '\0';
    memcpy(entry->expansion, expansion, (size_t)exp_len);
    entry->expansion[exp_len] = '\0';
    return 0;
}

const char *cmd_alias_find(const char *name, int len) {
    if (!name || len <= 0 || len >= 32) return NULL;
    int i;
    for (i = 0; i < g_cmd_alias_count; i++) {
        if (matches_nocase(name, g_cmd_aliases[i].name, len) && g_cmd_aliases[i].name[len] == '\0') {
            return g_cmd_aliases[i].expansion;
        }
    }
    return NULL;
}

void stmtreg_init(void) {
    g_stmt_count = 0;
    memset(g_stmt_table, 0, sizeof(g_stmt_table));
    cmd_alias_clear();
}

void stmtreg_cleanup(void) {
    g_stmt_count = 0;
    cmd_alias_clear();
}

static int matches_nocase(const char *a, const char *b, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (!b[i]) return 0;
        char ca = a[i];
        char cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return 0;
    }
    return b[len] == '\0';
}

int stmtreg_register(const char *name, KeywordId kw, StmtParserCallback handler) {
    if (g_stmt_count >= MAX_STATEMENT_ENTRIES) return -1;
    
    // Check for duplicates
    int i;
    for (i = 0; i < g_stmt_count; i++) {
        if (kw != KW_COUNT && g_stmt_table[i].kw == kw) {
            g_stmt_table[i].handler = handler;
            return 0;
        }
        if (name && matches_nocase(name, g_stmt_table[i].name, (int)strlen(name))) {
            g_stmt_table[i].handler = handler;
            return 0;
        }
    }
    
    StmtEntry *entry = &g_stmt_table[g_stmt_count++];
    if (name) {
        strncpy(entry->name, name, sizeof(entry->name) - 1);
        entry->name[sizeof(entry->name) - 1] = '\0';
    } else {
        entry->name[0] = '\0';
    }
    entry->kw = kw;
    entry->handler = handler;
    return 0;
}

StmtParserCallback stmtreg_find_by_keyword(KeywordId kw) {
    if (kw == KW_COUNT) return NULL;
    int i;
    for (i = 0; i < g_stmt_count; i++) {
        if (g_stmt_table[i].kw == kw) {
            return g_stmt_table[i].handler;
        }
    }
    return NULL;
}

StmtParserCallback stmtreg_find_by_name(const char *name, int len) {
    if (!name || len <= 0) return NULL;
    int i;
    for (i = 0; i < g_stmt_count; i++) {
        if (g_stmt_table[i].name[0] != '\0' && matches_nocase(name, g_stmt_table[i].name, len)) {
            return g_stmt_table[i].handler;
        }
    }
    return NULL;
}
