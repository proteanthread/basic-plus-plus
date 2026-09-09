// FILENAME: record_lock.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_readu.c, stmt_writeu.c, stmt_release.c, func_locked.c)
// NEEDS: libengine (record_lock.h)
// Provides virtual record locking implementation for Pick BASIC and Business BASIC.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/record_lock.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

static RecordLockEntry g_record_locks[MAX_RECORD_LOCKS];
static bool g_record_locks_initialized = false;

void record_lock_init(void) {
    runtime_memset(g_record_locks, 0, sizeof(g_record_locks));
    g_record_locks_initialized = true;
}

bool record_lock_acquire(int ch, const char *rec_id) {
    if (!g_record_locks_initialized) {
        record_lock_init();
    }
    if (!rec_id) rec_id = "";

    // Check if already locked by someone or this channel
    for (int i = 0; i < MAX_RECORD_LOCKS; i++) {
        if (g_record_locks[i].active &&
            g_record_locks[i].channel == ch &&
            runtime_strcmp(g_record_locks[i].rec_id, rec_id) == 0) {
            return true; // Already held
        }
    }

    // Allocate new slot
    for (int i = 0; i < MAX_RECORD_LOCKS; i++) {
        if (!g_record_locks[i].active) {
            g_record_locks[i].active = true;
            g_record_locks[i].channel = ch;
            runtime_strncpy(g_record_locks[i].rec_id, rec_id, MAX_RECORD_ID_LEN - 1);
            g_record_locks[i].rec_id[MAX_RECORD_ID_LEN - 1] = '\0';
            return true;
        }
    }
    return false; // Table full
}

bool record_lock_store(int ch, const char *rec_id, const char *data) {
    if (!g_record_locks_initialized) {
        record_lock_init();
    }
    if (!rec_id) rec_id = "";
    if (!data) data = "";

    for (int i = 0; i < MAX_RECORD_LOCKS; i++) {
        if (g_record_locks[i].active &&
            g_record_locks[i].channel == ch &&
            runtime_strcmp(g_record_locks[i].rec_id, rec_id) == 0) {
            runtime_strncpy(g_record_locks[i].rec_data, data, MAX_RECORD_DATA_LEN - 1);
            g_record_locks[i].rec_data[MAX_RECORD_DATA_LEN - 1] = '\0';
            return true;
        }
    }

    if (record_lock_acquire(ch, rec_id)) {
        return record_lock_store(ch, rec_id, data);
    }
    return false;
}

const char *record_lock_retrieve(int ch, const char *rec_id) {
    if (!g_record_locks_initialized) return "";
    if (!rec_id) rec_id = "";

    for (int i = 0; i < MAX_RECORD_LOCKS; i++) {
        if (g_record_locks[i].active &&
            g_record_locks[i].channel == ch &&
            runtime_strcmp(g_record_locks[i].rec_id, rec_id) == 0) {
            return g_record_locks[i].rec_data;
        }
    }
    return "";
}

bool record_lock_release(int ch, const char *rec_id) {
    if (!g_record_locks_initialized) return false;
    if (!rec_id) rec_id = "";

    for (int i = 0; i < MAX_RECORD_LOCKS; i++) {
        if (g_record_locks[i].active &&
            g_record_locks[i].channel == ch &&
            runtime_strcmp(g_record_locks[i].rec_id, rec_id) == 0) {
            g_record_locks[i].active = false;
            g_record_locks[i].rec_id[0] = '\0';
            g_record_locks[i].rec_data[0] = '\0';
            return true;
        }
    }
    return false;
}

int record_lock_release_channel(int ch) {
    if (!g_record_locks_initialized) return 0;
    int released = 0;
    for (int i = 0; i < MAX_RECORD_LOCKS; i++) {
        if (g_record_locks[i].active && g_record_locks[i].channel == ch) {
            g_record_locks[i].active = false;
            g_record_locks[i].rec_id[0] = '\0';
            g_record_locks[i].rec_data[0] = '\0';
            released++;
        }
    }
    return released;
}

int record_lock_release_all(void) {
    if (!g_record_locks_initialized) return 0;
    int released = 0;
    for (int i = 0; i < MAX_RECORD_LOCKS; i++) {
        if (g_record_locks[i].active) {
            g_record_locks[i].active = false;
            g_record_locks[i].rec_id[0] = '\0';
            g_record_locks[i].rec_data[0] = '\0';
            released++;
        }
    }
    return released;
}

bool record_lock_is_locked(int ch, const char *rec_id) {
    if (!g_record_locks_initialized) return false;
    if (!rec_id) rec_id = "";

    for (int i = 0; i < MAX_RECORD_LOCKS; i++) {
        if (g_record_locks[i].active &&
            g_record_locks[i].channel == ch &&
            runtime_strcmp(g_record_locks[i].rec_id, rec_id) == 0) {
            return true;
        }
    }
    return false;
}
