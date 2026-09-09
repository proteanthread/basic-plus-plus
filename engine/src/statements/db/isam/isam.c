// FILENAME: isam.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (delete.c, exec_internal.h)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, struct.h, struct.c)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, isam.h, map.h, map.c, string.c)
// Provides runtime implementation for the ISAM statement in BASIC++.
//
// ---- Includes ----

#include "statements/db/isam/isam.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/map.h"
#include "runtime/strings.h"
#include "runtime/language_descriptor.h"
#include "core/struct.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_createindex_desc = {
    .name = "CREATEINDEX",
    .category = "Database",
    .syntax = "CREATEINDEX #ch, idx$, len [, dups]",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_deleteindex_desc = {
    .name = "DELETEINDEX",
    .category = "Database",
    .syntax = "DELETEINDEX #ch, idx$",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_setindex_desc = {
    .name = "SETINDEX",
    .category = "Database",
    .syntax = "SETINDEX #ch, idx$",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_insert_desc = {
    .name = "INSERT",
    .category = "Database",
    .syntax = "INSERT #ch [, record]",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_update_desc = {
    .name = "UPDATE",
    .category = "Database",
    .syntax = "UPDATE #ch [, record]",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_delete_desc = {
    .name = "DELETE",
    .category = "Database",
    .syntax = "DELETE #ch",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_retrieve_desc = {
    .name = "RETRIEVE",
    .category = "Database",
    .syntax = "RETRIEVE #ch, record",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_seekeq_desc = {
    .name = "SEEKEQ",
    .category = "Database",
    .syntax = "SEEKEQ #ch, key",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_seekge_desc = {
    .name = "SEEKGE",
    .category = "Database",
    .syntax = "SEEKGE #ch, key",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_seekgt_desc = {
    .name = "SEEKGT",
    .category = "Database",
    .syntax = "SEEKGT #ch, key",
    .description = "",
    .error_summary = "None",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

#define MAX_ISAM_TABLES 16
#define MAX_ISAM_INDEXES 8
#define MAX_ISAM_RECORDS 2048

typedef struct {
    char key[64];
    int rec_idx;
} IsamKeyEntry;

typedef struct {
    char name[64];
    int key_len;
    bool allow_dups;
    int entry_count;
    IsamKeyEntry entries[MAX_ISAM_RECORDS];
} IsamIndex;

typedef struct {
    bool active;
    int channel;
    int rec_count;
    BValue records[MAX_ISAM_RECORDS];
    int current_rec;
    int active_index;
    int index_count;
    IsamIndex indexes[MAX_ISAM_INDEXES];
} IsamTable;

static IsamTable g_isam_tables[MAX_ISAM_TABLES];

static IsamTable *get_isam_table(int channel, bool create) {
    for (int i = 0; i < MAX_ISAM_TABLES; i++) {
        if (g_isam_tables[i].active && g_isam_tables[i].channel == channel) {
            return &g_isam_tables[i];
        }
    }
    if (!create) return NULL;
    for (int i = 0; i < MAX_ISAM_TABLES; i++) {
        if (!g_isam_tables[i].active) {
            runtime_memset(&g_isam_tables[i], 0, sizeof(IsamTable));
            g_isam_tables[i].active = true;
            g_isam_tables[i].channel = channel;
            g_isam_tables[i].current_rec = -1;
            g_isam_tables[i].active_index = -1;
            return &g_isam_tables[i];
        }
    }
    return NULL;
}

static int parse_channel_arg(VMContext *vm, LexerContext *lex, BppError *err) {
    BppToken tok = lex_peek(lex);
    bool in_bracket = false;
    if (tok.type == TOK_LBRACKET) {
        lex_next(lex);
        in_bracket = true;
        tok = lex_peek(lex);
    }
    if (tok.type == TOK_HASH) {
        lex_next(lex);
    }
    BValue val = eval_expression(vm, lex, err);
    if (err->code != 0) return 0;
    if (val.type == VAL_STRING) {
        str_release(vm_get_str(vm), val.as.string);
        err->code = 13; err->message = "Channel must be numeric";
        return 0;
    }
    if (in_bracket) {
        tok = lex_peek(lex);
        if (tok.type == TOK_RBRACKET) {
            lex_next(lex);
        }
    }
    return (int)val.as.number;
}

BppError stmt_createindex_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue val_name = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_name.type != VAL_STRING) { err.code = 13; err.message = "Index name must be string"; return err; }
    char idx_name[64] = {0};
    const char *ns = str_data(val_name.as.string);
    if (ns) runtime_strncpy(idx_name, ns, sizeof(idx_name) - 1);
    str_release(vm_get_str(vm), val_name.as.string);

    tok = lex_next(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected comma after index name in CREATEINDEX"; return err; }

    BValue val_len = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    int key_len = (int)val_len.as.number;

    bool dups = false;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue val_dup = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        dups = (val_dup.as.number != 0.0);
    }

    IsamTable *tbl = get_isam_table(ch, true);
    if (!tbl) { err.code = 5; err.message = "Cannot create ISAM table"; return err; }
    if (tbl->index_count >= MAX_ISAM_INDEXES) { err.code = 5; err.message = "Max ISAM indexes reached"; return err; }

    IsamIndex *idx = &tbl->indexes[tbl->index_count++];
    runtime_memset(idx, 0, sizeof(IsamIndex));
    runtime_strncpy(idx->name, idx_name, sizeof(idx->name) - 1);
    idx->key_len = key_len;
    idx->allow_dups = dups;
    tbl->active_index = tbl->index_count - 1;
    return err;
}

BppError stmt_deleteindex_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue val_name = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    char idx_name[64] = {0};
    if (val_name.type == VAL_STRING) {
        const char *ns = str_data(val_name.as.string);
        if (ns) runtime_strncpy(idx_name, ns, sizeof(idx_name) - 1);
        str_release(vm_get_str(vm), val_name.as.string);
    }

    IsamTable *tbl = get_isam_table(ch, false);
    if (tbl) {
        for (int i = 0; i < tbl->index_count; i++) {
            if (runtime_strcasecmp(tbl->indexes[i].name, idx_name) == 0) {
                for (int j = i; j < tbl->index_count - 1; j++) {
                    tbl->indexes[j] = tbl->indexes[j + 1];
                }
                tbl->index_count--;
                if (tbl->active_index >= tbl->index_count) tbl->active_index = tbl->index_count - 1;
                break;
            }
        }
    }
    return err;
}

BppError stmt_setindex_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue val_name = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    char idx_name[64] = {0};
    if (val_name.type == VAL_STRING) {
        const char *ns = str_data(val_name.as.string);
        if (ns) runtime_strncpy(idx_name, ns, sizeof(idx_name) - 1);
        str_release(vm_get_str(vm), val_name.as.string);
    }

    IsamTable *tbl = get_isam_table(ch, false);
    if (!tbl) { err.code = 5; err.message = "ISAM table not open"; return err; }

    bool found = false;
    for (int i = 0; i < tbl->index_count; i++) {
        if (runtime_strcasecmp(tbl->indexes[i].name, idx_name) == 0) {
            tbl->active_index = i;
            found = true;
            break;
        }
    }
    if (!found) { err.code = 5; err.message = "ISAM index not found"; }
    return err;
}

BppError stmt_insert_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }
    BValue rec_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    IsamTable *tbl = get_isam_table(ch, true);
    if (!tbl || tbl->rec_count >= MAX_ISAM_RECORDS) {
        err.code = 5; err.message = "ISAM table full or invalid";
        if (rec_val.type == VAL_STRING && rec_val.as.string) str_release(vm_get_str(vm), rec_val.as.string);
        return err;
    }

    int r_idx = tbl->rec_count++;
    tbl->records[r_idx] = rec_val;
    tbl->current_rec = r_idx;

    // If active index exists and record is string or map, add entry
    if (tbl->active_index >= 0 && tbl->active_index < tbl->index_count) {
        IsamIndex *idx = &tbl->indexes[tbl->active_index];
        if (idx->entry_count < MAX_ISAM_RECORDS) {
            char k_buf[64] = {0};
            size_t max_k = (idx->key_len > 0 && idx->key_len < 63) ? (size_t)idx->key_len : 63;
            if (rec_val.type == VAL_STRING && rec_val.as.string) {
                const char *s = str_data(rec_val.as.string);
                if (s) runtime_strncpy(k_buf, s, max_k);
            } else if (rec_val.type == VAL_MAP && rec_val.as.map) {
                BValue kv;
                if (map_get(rec_val.as.map, idx->name, &kv) && kv.type == VAL_STRING && kv.as.string) {
                    const char *s = str_data(kv.as.string);
                    if (s) runtime_strncpy(k_buf, s, max_k);
                }
            } else {
                runtime_snprintf(k_buf, sizeof(k_buf), "%g", rec_val.as.number);
            }
            runtime_strncpy(idx->entries[idx->entry_count].key, k_buf, 63);
            idx->entries[idx->entry_count].rec_idx = r_idx;
            idx->entry_count++;
        }
    }
    return err;
}

BppError stmt_update_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
    }
    BValue rec_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    IsamTable *tbl = get_isam_table(ch, false);
    if (!tbl || tbl->current_rec < 0 || tbl->current_rec >= tbl->rec_count) {
        err.code = 5; err.message = "No current ISAM record to update";
        if (rec_val.type == VAL_STRING && rec_val.as.string) str_release(vm_get_str(vm), rec_val.as.string);
        return err;
    }

    int r_idx = tbl->current_rec;
    if (tbl->records[r_idx].type == VAL_STRING && tbl->records[r_idx].as.string) {
        str_release(vm_get_str(vm), tbl->records[r_idx].as.string);
    }
    tbl->records[r_idx] = rec_val;
    return err;
}

BppError stmt_delete_rec_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    IsamTable *tbl = get_isam_table(ch, false);
    if (!tbl || tbl->current_rec < 0 || tbl->current_rec >= tbl->rec_count) {
        err.code = 5; err.message = "No current ISAM record to delete";
        return err;
    }

    int r_idx = tbl->current_rec;
    if (tbl->records[r_idx].type == VAL_STRING && tbl->records[r_idx].as.string) {
        str_release(vm_get_str(vm), tbl->records[r_idx].as.string);
    }
    tbl->records[r_idx].type = VAL_NONE;
    return err;
}

BppError stmt_retrieve_handler(VMContext *vm, LexerContext *lex) {
    BppError err; runtime_memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) { err.code = 2; err.message = "Expected variable in RETRIEVE"; return err; }
    char var_name[64] = {0};
    size_t len = (tok.length < 63) ? tok.length : 63;
    runtime_memcpy(var_name, tok.start, len);

    IsamTable *tbl = get_isam_table(ch, false);
    if (!tbl || tbl->current_rec < 0 || tbl->current_rec >= tbl->rec_count) {
        err.code = 5; err.message = "No current ISAM record to retrieve";
        return err;
    }

    BValue r = tbl->records[tbl->current_rec];
    var_assign(vm_get_var(vm), var_name, r);
    return err;
}

static BppError isam_seek_helper(VMContext *vm, LexerContext *lex, int mode) {
    BppError err; runtime_memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    BValue val_key = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    char target_key[64] = {0};
    if (val_key.type == VAL_STRING) {
        const char *s = str_data(val_key.as.string);
        if (s) runtime_strncpy(target_key, s, sizeof(target_key) - 1);
        str_release(vm_get_str(vm), val_key.as.string);
    } else {
        runtime_snprintf(target_key, sizeof(target_key), "%g", val_key.as.number);
    }

    IsamTable *tbl = get_isam_table(ch, false);
    if (!tbl || tbl->active_index < 0 || tbl->active_index >= tbl->index_count) {
        err.code = 5; err.message = "No active ISAM index";
        return err;
    }

    IsamIndex *idx = &tbl->indexes[tbl->active_index];
    int matched_rec = -1;

    for (int i = 0; i < idx->entry_count; i++) {
        if (tbl->records[idx->entries[i].rec_idx].type == VAL_NONE) continue;
        int cmp = runtime_strcasecmp(idx->entries[i].key, target_key);
        if (mode == 0 && cmp == 0) { matched_rec = idx->entries[i].rec_idx; break; }
        if (mode == 1 && cmp >= 0) { matched_rec = idx->entries[i].rec_idx; break; }
        if (mode == 2 && cmp > 0)  { matched_rec = idx->entries[i].rec_idx; break; }
    }

    if (matched_rec >= 0) {
        tbl->current_rec = matched_rec;
    } else {
        err.code = 50; err.message = "Record not found in ISAM seek";
    }
    return err;
}

BppError stmt_seekeq_handler(VMContext *vm, LexerContext *lex) { return isam_seek_helper(vm, lex, 0); }
BppError stmt_seekge_handler(VMContext *vm, LexerContext *lex) { return isam_seek_helper(vm, lex, 1); }
BppError stmt_seekgt_handler(VMContext *vm, LexerContext *lex) { return isam_seek_helper(vm, lex, 2); }

bool isam_is_active(int channel) {
    return (get_isam_table(channel, false) != NULL);
}

int isam_get_keycount(int channel) {
    IsamTable *tbl = get_isam_table(channel, false);
    if (!tbl || tbl->active_index < 0 || tbl->active_index >= tbl->index_count) return 0;
    return tbl->indexes[tbl->active_index].entry_count;
}

const char *isam_get_current_key(int channel) {
    IsamTable *tbl = get_isam_table(channel, false);
    if (!tbl || tbl->active_index < 0 || tbl->active_index >= tbl->index_count) return "";
    if (tbl->current_rec < 0 || tbl->current_rec >= tbl->rec_count) return "";
    IsamIndex *idx = &tbl->indexes[tbl->active_index];
    for (int i = 0; i < idx->entry_count; i++) {
        if (idx->entries[i].rec_idx == tbl->current_rec) {
            return idx->entries[i].key;
        }
    }
    return "";
}

bool isam_read_record(VMContext *vm, int channel, const char *key, BValue *out_val) {
    (void)vm;
    if (!out_val) return false;
    out_val->type = VAL_NONE;
    out_val->as.number = 0.0;

    IsamTable *tbl = get_isam_table(channel, false);
    if (!tbl || tbl->active_index < 0 || tbl->active_index >= tbl->index_count) return false;
    IsamIndex *idx = &tbl->indexes[tbl->active_index];

    for (int i = 0; i < idx->entry_count; i++) {
        if (tbl->records[idx->entries[i].rec_idx].type == VAL_NONE) continue;
        if (runtime_strcasecmp(idx->entries[i].key, key) == 0) {
            tbl->current_rec = idx->entries[i].rec_idx;
            *out_val = tbl->records[tbl->current_rec];
            return true;
        }
    }
    return false;
}

bool isam_write_record(VMContext *vm, int channel, const char *key, BValue val) {
    (void)vm;
    IsamTable *tbl = get_isam_table(channel, true);
    if (!tbl) return false;

    if (tbl->active_index >= 0 && tbl->active_index < tbl->index_count) {
        IsamIndex *idx = &tbl->indexes[tbl->active_index];
        for (int i = 0; i < idx->entry_count; i++) {
            if (runtime_strcasecmp(idx->entries[i].key, key) == 0) {
                int r_idx = idx->entries[i].rec_idx;
                if (tbl->records[r_idx].type == VAL_STRING && tbl->records[r_idx].as.string) {
                    str_release(vm_get_str(vm), tbl->records[r_idx].as.string);
                }
                tbl->records[r_idx] = val;
                tbl->current_rec = r_idx;
                return true;
            }
        }
    }

    if (tbl->rec_count >= MAX_ISAM_RECORDS) return false;
    int r_idx = tbl->rec_count++;
    tbl->records[r_idx] = val;
    tbl->current_rec = r_idx;

    if (tbl->active_index >= 0 && tbl->active_index < tbl->index_count) {
        IsamIndex *idx = &tbl->indexes[tbl->active_index];
        if (idx->entry_count < MAX_ISAM_RECORDS) {
            runtime_strncpy(idx->entries[idx->entry_count].key, key, 63);
            idx->entries[idx->entry_count].rec_idx = r_idx;
            idx->entry_count++;
        }
    }
    return true;
}

void isam_system_shutdown(void) {
    for (int i = 0; i < MAX_ISAM_TABLES; i++) {
        if (g_isam_tables[i].active) {
            runtime_memset(&g_isam_tables[i], 0, sizeof(IsamTable));
        }
    }
}

void stmt_isam_register(void) {
    lang_desc_register(&g_createindex_desc);
    lang_desc_register(&g_deleteindex_desc);
    lang_desc_register(&g_setindex_desc);
    lang_desc_register(&g_insert_desc);
    lang_desc_register(&g_update_desc);
    lang_desc_register(&g_delete_desc);
    lang_desc_register(&g_retrieve_desc);
    lang_desc_register(&g_seekeq_desc);
    lang_desc_register(&g_seekge_desc);
    lang_desc_register(&g_seekgt_desc);
}
