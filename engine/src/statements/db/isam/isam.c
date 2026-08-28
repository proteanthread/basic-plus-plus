// FILENAME: isam.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (delete.c, exec_internal.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
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
#include "runtime/micro_lib_metadata.h"
#include "core/struct.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
            memset(&g_isam_tables[i], 0, sizeof(IsamTable));
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
    return (int)val.as.number;
}

BppError stmt_createindex_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected comma after channel in CREATEINDEX"; return err; }

    BValue val_name = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val_name.type != VAL_STRING) { err.code = 13; err.message = "Index name must be string"; return err; }
    char idx_name[64] = {0};
    const char *ns = str_data(val_name.as.string);
    if (ns) strncpy(idx_name, ns, sizeof(idx_name) - 1);
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
    memset(idx, 0, sizeof(IsamIndex));
    strncpy(idx->name, idx_name, sizeof(idx->name) - 1);
    idx->key_len = key_len;
    idx->allow_dups = dups;
    tbl->active_index = tbl->index_count - 1;
    return err;
}

BppError stmt_deleteindex_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected comma in DELETEINDEX"; return err; }

    BValue val_name = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    char idx_name[64] = {0};
    if (val_name.type == VAL_STRING) {
        const char *ns = str_data(val_name.as.string);
        if (ns) strncpy(idx_name, ns, sizeof(idx_name) - 1);
        str_release(vm_get_str(vm), val_name.as.string);
    }

    IsamTable *tbl = get_isam_table(ch, false);
    if (tbl) {
        for (int i = 0; i < tbl->index_count; i++) {
            if (strcasecmp(tbl->indexes[i].name, idx_name) == 0) {
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
    BppError err; memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected comma in SETINDEX"; return err; }

    BValue val_name = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    char idx_name[64] = {0};
    if (val_name.type == VAL_STRING) {
        const char *ns = str_data(val_name.as.string);
        if (ns) strncpy(idx_name, ns, sizeof(idx_name) - 1);
        str_release(vm_get_str(vm), val_name.as.string);
    }

    IsamTable *tbl = get_isam_table(ch, false);
    if (!tbl) { err.code = 5; err.message = "ISAM table not open"; return err; }

    bool found = false;
    for (int i = 0; i < tbl->index_count; i++) {
        if (strcasecmp(tbl->indexes[i].name, idx_name) == 0) {
            tbl->active_index = i;
            found = true;
            break;
        }
    }
    if (!found) { err.code = 5; err.message = "ISAM index not found"; }
    return err;
}

BppError stmt_insert_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    BValue rec_val = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        rec_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
    }

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
                if (s) strncpy(k_buf, s, max_k);
            } else if (rec_val.type == VAL_MAP && rec_val.as.map) {
                BValue kv;
                if (map_get(rec_val.as.map, idx->name, &kv) && kv.type == VAL_STRING && kv.as.string) {
                    const char *s = str_data(kv.as.string);
                    if (s) strncpy(k_buf, s, max_k);
                }
            } else {
                snprintf(k_buf, sizeof(k_buf), "%g", rec_val.as.number);
            }
            strncpy(idx->entries[idx->entry_count].key, k_buf, 63);
            idx->entries[idx->entry_count].rec_idx = r_idx;
            idx->entry_count++;
        }
    }
    return err;
}

BppError stmt_update_handler(VMContext *vm, LexerContext *lex) {
    BppError err; memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_peek(lex);
    BValue rec_val = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        rec_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
    }

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
    BppError err; memset(&err, 0, sizeof(err));
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
    BppError err; memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected comma in RETRIEVE"; return err; }

    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) { err.code = 2; err.message = "Expected variable in RETRIEVE"; return err; }
    char var_name[64] = {0};
    size_t len = (tok.length < 63) ? tok.length : 63;
    memcpy(var_name, tok.start, len);

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
    BppError err; memset(&err, 0, sizeof(err));
    int ch = parse_channel_arg(vm, lex, &err);
    if (err.code != 0) return err;

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_COMMA) { err.code = 2; err.message = "Expected comma in SEEK"; return err; }

    BValue val_key = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    char target_key[64] = {0};
    if (val_key.type == VAL_STRING) {
        const char *s = str_data(val_key.as.string);
        if (s) strncpy(target_key, s, sizeof(target_key) - 1);
        str_release(vm_get_str(vm), val_key.as.string);
    } else {
        snprintf(target_key, sizeof(target_key), "%g", val_key.as.number);
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
        int cmp = strcasecmp(idx->entries[i].key, target_key);
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

void isam_system_shutdown(void) {
    for (int i = 0; i < MAX_ISAM_TABLES; i++) {
        if (g_isam_tables[i].active) {
            memset(&g_isam_tables[i], 0, sizeof(IsamTable));
        }
    }
}

void stmt_isam_register(void) {
    static const MicroLibMetadata meta_ci = { .name = "CREATEINDEX", .category = "Database", .syntax = "CREATEINDEX #ch, idx$, len [, dups]" };
    static const MicroLibMetadata meta_di = { .name = "DELETEINDEX", .category = "Database", .syntax = "DELETEINDEX #ch, idx$" };
    static const MicroLibMetadata meta_si = { .name = "SETINDEX", .category = "Database", .syntax = "SETINDEX #ch, idx$" };
    static const MicroLibMetadata meta_ins = { .name = "INSERT", .category = "Database", .syntax = "INSERT #ch [, record]" };
    static const MicroLibMetadata meta_upd = { .name = "UPDATE", .category = "Database", .syntax = "UPDATE #ch [, record]" };
    static const MicroLibMetadata meta_del = { .name = "DELETE", .category = "Database", .syntax = "DELETE #ch" };
    static const MicroLibMetadata meta_ret = { .name = "RETRIEVE", .category = "Database", .syntax = "RETRIEVE #ch, record" };
    static const MicroLibMetadata meta_seq = { .name = "SEEKEQ", .category = "Database", .syntax = "SEEKEQ #ch, key" };
    static const MicroLibMetadata meta_sge = { .name = "SEEKGE", .category = "Database", .syntax = "SEEKGE #ch, key" };
    static const MicroLibMetadata meta_sgt = { .name = "SEEKGT", .category = "Database", .syntax = "SEEKGT #ch, key" };
    microlib_register(&meta_ci);
    microlib_register(&meta_di);
    microlib_register(&meta_si);
    microlib_register(&meta_ins);
    microlib_register(&meta_upd);
    microlib_register(&meta_del);
    microlib_register(&meta_ret);
    microlib_register(&meta_seq);
    microlib_register(&meta_sge);
    microlib_register(&meta_sgt);
}
