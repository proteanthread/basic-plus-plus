// FILENAME: record_lock.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (record_lock.c, stmt_readu.c, stmt_writeu.c, stmt_release.c, func_locked.c)
// NEEDS: libkernel (types.h)
// Provides virtual record locking API for Pick BASIC and Business BASIC in BASIC++.
//
// ---- Includes ----

#ifndef RECORD_LOCK_H
#define RECORD_LOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "types/types.h"

#define MAX_RECORD_LOCKS 128
#define MAX_RECORD_ID_LEN 64
#define MAX_RECORD_DATA_LEN 512

typedef struct {
    bool active;
    int channel;
    char rec_id[MAX_RECORD_ID_LEN];
    char rec_data[MAX_RECORD_DATA_LEN];
} RecordLockEntry;

void record_lock_init(void);
bool record_lock_acquire(int ch, const char *rec_id);
bool record_lock_store(int ch, const char *rec_id, const char *data);
const char *record_lock_retrieve(int ch, const char *rec_id);
bool record_lock_release(int ch, const char *rec_id);
int record_lock_release_channel(int ch);
int record_lock_release_all(void);
bool record_lock_is_locked(int ch, const char *rec_id);

#endif // RECORD_LOCK_H
