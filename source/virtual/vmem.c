/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: vmem.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Virtual Memory / RAMBANK page swapping, encryption, and caching.
 * ===================================================================== */

#ifndef BPP_LITE_BUILD

#include "vdev.h"
#include "runtime.h"
#include "security.h"
#include "errors.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Paging & Encryption Helper
static void rambank_crypt(char *buffer, long size, int bank_id) {
    if (security_get_level() >= SEC_STANDARD) {
        unsigned char key = (unsigned char)(0x5A ^ bank_id);
        for (long i = 0; i < size; i++) {
            buffer[i] ^= key;
        }
    }
}

// Get file path for a bank's swap file
static void rambank_get_swap_path(int bank_id, char *path_out) {
    sprintf(path_out, "bank_swap_%d.tmp", bank_id);
}

// Evict Least Recently Used (LRU) resident bank to swap file
static void rambank_evict_lru(MemorySystem *mem) {
    int victim = -1;
    long oldest_access = -1;

    for (int i = 1; i < MAX_RAMBANKS; i++) {
        if (mem->banks[i].resident && mem->banks[i].base != NULL) {
            if (oldest_access == -1 || mem->banks[i].last_access < oldest_access) {
                oldest_access = mem->banks[i].last_access;
                victim = i;
            }
        }
    }

    if (victim != -1) {
        RamBank *b = &mem->banks[victim];
        if (b->dirty) {
            char path[260];
            rambank_get_swap_path(victim, path);
            FILE *f = fopen(path, "wb");
            if (f != NULL) {
                rambank_crypt(b->base, RAMBANK_SIZE, victim);
                fwrite(b->base, 1, RAMBANK_SIZE, f);
                fclose(f);
                rambank_crypt(b->base, RAMBANK_SIZE, victim);
                b->dirty = 0;
            }
        }
        free(b->base);
        b->base = NULL;
        b->resident = 0;
    }
}

// Bring bank into resident memory (page fault handler)
void rambank_ensure_resident(MemorySystem *mem, int bank_id) {
    if (bank_id <= 0 || bank_id >= MAX_RAMBANKS) return;
    RamBank *b = &mem->banks[bank_id];
    
    mem->access_counter++;
    b->last_access = mem->access_counter;

    if (b->resident && b->base != NULL) {
        return;
    }

    int resident_count = 0;
    for (int i = 1; i < MAX_RAMBANKS; i++) {
        if (mem->banks[i].resident) {
            resident_count++;
        }
    }

    if (resident_count >= MAX_RESIDENT_BANKS) {
        rambank_evict_lru(mem);
    }

    b->base = (char *)malloc(RAMBANK_SIZE);
    if (b->base == NULL) {
        error_raise(ERR_SORRY, 0);
        return;
    }
    memset(b->base, 0, RAMBANK_SIZE);
    b->resident = 1;
    b->dirty = 0;

    char path[260];
    rambank_get_swap_path(bank_id, path);
    FILE *f = fopen(path, "rb");
    if (f != NULL) {
        size_t read_bytes = fread(b->base, 1, RAMBANK_SIZE, f);
        fclose(f);
        if (read_bytes == RAMBANK_SIZE) {
            rambank_crypt(b->base, RAMBANK_SIZE, bank_id);
        }
    }
}

void rambank_init(MemorySystem *mem) {
    mem->access_counter = 0;
    for (int i = 0; i < MAX_RAMBANKS; i++) {
        mem->banks[i].base = NULL;
        mem->banks[i].id = i;
        mem->banks[i].resident = 0;
        mem->banks[i].dirty = 0;
        mem->banks[i].shared = 0;
        mem->banks[i].last_access = 0;
    }
}

void rambank_shutdown(MemorySystem *mem) {
    for (int i = 0; i < MAX_RAMBANKS; i++) {
        RamBank *b = &mem->banks[i];
        if (b->base != NULL) {
            free(b->base);
            b->base = NULL;
        }
        b->resident = 0;
        char path[260];
        rambank_get_swap_path(i, path);
        remove(path);
    }
}

unsigned char rambank_peek(MemorySystem *mem, int bank_id, long offset, int line_num) {
    if (bank_id <= 0 || bank_id >= MAX_RAMBANKS) {
        error_raise(ERR_HOW, line_num);
        return 0;
    }
    if (offset < 0 || offset >= RAMBANK_SIZE) {
        error_raise(ERR_HOW, line_num);
        return 0;
    }

    MemorySystem *main_mem = task_get_main_mem();
    int is_shared = (main_mem != NULL && main_mem->banks[bank_id].shared);

    if (mem != main_mem) {
        BasicTask *curr = task_get_current();
        if (curr != NULL && curr->pid != 0) {
            int allowed = (bank_id == curr->active_bank_id || is_shared);
            if (!allowed) {
                error_raise(ERR_HOW, line_num);
                return 0;
            }
        }
    }

    MemorySystem *target_mem = mem;
    if (is_shared && main_mem != NULL) {
        target_mem = main_mem;
    }

    if (is_shared) {
        task_mutex_lock();
    }
    rambank_ensure_resident(target_mem, bank_id);
    RamBank *b = &target_mem->banks[bank_id];
    unsigned char ret = 0;
    if (b->base != NULL) {
        ret = (unsigned char)b->base[offset];
    }
    if (is_shared) {
        task_mutex_unlock();
    }
    return ret;
}

void rambank_poke(MemorySystem *mem, int bank_id, long offset, unsigned char value, int line_num) {
    if (bank_id <= 0 || bank_id >= MAX_RAMBANKS) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    if (offset < 0 || offset >= RAMBANK_SIZE) {
        error_raise(ERR_HOW, line_num);
        return;
    }

    MemorySystem *main_mem = task_get_main_mem();
    int is_shared = (main_mem != NULL && main_mem->banks[bank_id].shared);

    if (mem != main_mem) {
        BasicTask *curr = task_get_current();
        if (curr != NULL && curr->pid != 0) {
            int allowed = (bank_id == curr->active_bank_id || is_shared);
            if (!allowed) {
                error_raise(ERR_HOW, line_num);
                return;
            }
        }
    }

    MemorySystem *target_mem = mem;
    if (is_shared && main_mem != NULL) {
        target_mem = main_mem;
    }

    if (is_shared) {
        task_mutex_lock();
    }
    rambank_ensure_resident(target_mem, bank_id);
    RamBank *b = &target_mem->banks[bank_id];
    if (b->base != NULL) {
        b->base[offset] = (char)value;
        b->dirty = 1;
    }
    if (is_shared) {
        task_mutex_unlock();
    }
}

long rambank_free_space(MemorySystem *mem, int bank_id) {
    if (bank_id <= 0 || bank_id >= MAX_RAMBANKS) {
        return 0;
    }
    return RAMBANK_SIZE;
}

void rambank_set_shared(MemorySystem *mem, int bank_id, int shared) {
    if (bank_id > 0 && bank_id < MAX_RAMBANKS) {
        mem->banks[bank_id].shared = shared;
    }
}

void rambank_copy(MemorySystem *mem, int src_bank, long src_offset, int dst_bank, long dst_offset, long length, int line_num) {
    if (src_bank <= 0 || src_bank >= MAX_RAMBANKS || dst_bank <= 0 || dst_bank >= MAX_RAMBANKS) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    if (src_offset < 0 || src_offset + length > RAMBANK_SIZE || dst_offset < 0 || dst_offset + length > RAMBANK_SIZE || length < 0) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    if (length == 0) return;

    MemorySystem *main_mem = task_get_main_mem();
    int src_shared = (main_mem != NULL && main_mem->banks[src_bank].shared);
    int dst_shared = (main_mem != NULL && main_mem->banks[dst_bank].shared);

    if (mem != main_mem) {
        BasicTask *curr = task_get_current();
        if (curr != NULL && curr->pid != 0) {
            if (!(src_bank == curr->active_bank_id || src_shared)) {
                error_raise(ERR_HOW, line_num);
                return;
            }
            if (!(dst_bank == curr->active_bank_id || dst_shared)) {
                error_raise(ERR_HOW, line_num);
                return;
            }
        }
    }

    MemorySystem *src_target_mem = mem;
    if (src_shared && main_mem != NULL) src_target_mem = main_mem;
    MemorySystem *dst_target_mem = mem;
    if (dst_shared && main_mem != NULL) dst_target_mem = main_mem;

    if (src_shared || dst_shared) {
        task_mutex_lock();
    }

    rambank_ensure_resident(src_target_mem, src_bank);
    rambank_ensure_resident(dst_target_mem, dst_bank);

    RamBank *sb = &src_target_mem->banks[src_bank];
    RamBank *db = &dst_target_mem->banks[dst_bank];

    if (sb->base != NULL && db->base != NULL) {
        memmove(db->base + dst_offset, sb->base + src_offset, length);
        db->dirty = 1;
    }

    if (src_shared || dst_shared) {
        task_mutex_unlock();
    }
}

void rambank_fill(MemorySystem *mem, int bank_id, long offset, long length, unsigned char value, int line_num) {
    if (bank_id <= 0 || bank_id >= MAX_RAMBANKS) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    if (offset < 0 || offset + length > RAMBANK_SIZE || length < 0) {
        error_raise(ERR_HOW, line_num);
        return;
    }
    if (length == 0) return;

    MemorySystem *main_mem = task_get_main_mem();
    int is_shared = (main_mem != NULL && main_mem->banks[bank_id].shared);

    if (mem != main_mem) {
        BasicTask *curr = task_get_current();
        if (curr != NULL && curr->pid != 0) {
            if (!(bank_id == curr->active_bank_id || is_shared)) {
                error_raise(ERR_HOW, line_num);
                return;
            }
        }
    }

    MemorySystem *target_mem = mem;
    if (is_shared && main_mem != NULL) target_mem = main_mem;

    if (is_shared) {
        task_mutex_lock();
    }

    rambank_ensure_resident(target_mem, bank_id);
    RamBank *b = &target_mem->banks[bank_id];

    if (b->base != NULL) {
        memset(b->base + offset, value, length);
        b->dirty = 1;
    }

    if (is_shared) {
        task_mutex_unlock();
    }
}

#endif // BPP_LITE_BUILD
