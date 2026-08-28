// FILENAME: interop_handle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (bpp_api.h, interop_core.c)
// NEEDS: libcore (interop_handle.h, string.h)
// NEEDS: libengine (string.c)
// Provides core logic and interface definitions for interop_handle within BASIC++.
//
// ---- Includes ----

#include "interop/interop_handle.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    void* ptr;
    uint32_t refcount;
    uint32_t generation;
    bool active;
} InteropHandleSlot;

static InteropHandleSlot* g_handle_table = NULL;
static size_t g_handle_capacity = 0;
static size_t g_next_free_hint = 0;

int interop_handle_table_init(void) {
    if (g_handle_table != NULL) {
        return 0;
    }
    g_handle_capacity = 4096;
    g_handle_table = (InteropHandleSlot*)calloc(g_handle_capacity, sizeof(InteropHandleSlot));
    if (g_handle_table == NULL) {
        g_handle_capacity = 0;
        return -1;
    }
    for (size_t i = 0; i < g_handle_capacity; ++i) {
        g_handle_table[i].generation = 1; // Start generation at 1 so 0 is invalid
    }
    g_next_free_hint = 1; // Reserve slot 0 as invalid
    return 0;
}

void interop_handle_table_shutdown(void) {
    if (g_handle_table != NULL) {
        free(g_handle_table);
        g_handle_table = NULL;
    }
    g_handle_capacity = 0;
    g_next_free_hint = 0;
}

InteropHandle interop_handle_create(void* ptr) {
    if (g_handle_table == NULL) {
        return INTEROP_INVALID_HANDLE;
    }
    
    size_t slot_idx = 0;
    bool found = false;
    
    for (size_t i = g_next_free_hint; i < g_handle_capacity; ++i) {
        if (!g_handle_table[i].active) {
            slot_idx = i;
            found = true;
            break;
        }
    }
    
    if (!found) {
        for (size_t i = 1; i < g_next_free_hint; ++i) {
            if (!g_handle_table[i].active) {
                slot_idx = i;
                found = true;
                break;
            }
        }
    }
    
    if (!found) {
        size_t new_capacity = g_handle_capacity * 2;
        InteropHandleSlot* new_table = (InteropHandleSlot*)calloc(new_capacity, sizeof(InteropHandleSlot));
        if (new_table == NULL) {
            return INTEROP_INVALID_HANDLE;
        }
        memcpy(new_table, g_handle_table, g_handle_capacity * sizeof(InteropHandleSlot));
        for (size_t i = g_handle_capacity; i < new_capacity; ++i) {
            new_table[i].ptr = NULL;
            new_table[i].refcount = 0;
            new_table[i].generation = 1;
            new_table[i].active = false;
        }
        free(g_handle_table);
        g_handle_table = new_table;
        slot_idx = g_handle_capacity;
        g_handle_capacity = new_capacity;
    }
    
    g_handle_table[slot_idx].ptr = ptr;
    g_handle_table[slot_idx].refcount = 1;
    g_handle_table[slot_idx].generation++;
    if (g_handle_table[slot_idx].generation == 0) {
        g_handle_table[slot_idx].generation = 1;
    }
    g_handle_table[slot_idx].active = true;
    
    g_next_free_hint = slot_idx + 1;
    
    InteropHandle handle = ((uint64_t)g_handle_table[slot_idx].generation << 32) | (uint32_t)slot_idx;
    return handle;
}

void* interop_handle_resolve(InteropHandle handle) {
    if (handle == INTEROP_INVALID_HANDLE || g_handle_table == NULL) {
        return NULL;
    }
    
    uint32_t slot_idx = (uint32_t)(handle & 0xFFFFFFFF);
    uint32_t generation = (uint32_t)(handle >> 32);
    
    if (slot_idx == 0 || slot_idx >= g_handle_capacity) {
        return NULL;
    }
    
    if (!g_handle_table[slot_idx].active || g_handle_table[slot_idx].generation != generation) {
        return NULL;
    }
    
    return g_handle_table[slot_idx].ptr;
}

void interop_handle_release(InteropHandle handle) {
    if (handle == INTEROP_INVALID_HANDLE || g_handle_table == NULL) {
        return;
    }
    
    uint32_t slot_idx = (uint32_t)(handle & 0xFFFFFFFF);
    uint32_t generation = (uint32_t)(handle >> 32);
    
    if (slot_idx == 0 || slot_idx >= g_handle_capacity) {
        return;
    }
    
    if (g_handle_table[slot_idx].active && g_handle_table[slot_idx].generation == generation) {
        if (g_handle_table[slot_idx].refcount > 0) {
            g_handle_table[slot_idx].refcount--;
            if (g_handle_table[slot_idx].refcount == 0) {
                g_handle_table[slot_idx].active = false;
                g_handle_table[slot_idx].ptr = NULL;
                g_next_free_hint = slot_idx;
            }
        }
    }
}

void interop_handle_retain(InteropHandle handle) {
    if (handle == INTEROP_INVALID_HANDLE || g_handle_table == NULL) {
        return;
    }
    
    uint32_t slot_idx = (uint32_t)(handle & 0xFFFFFFFF);
    uint32_t generation = (uint32_t)(handle >> 32);
    
    if (slot_idx == 0 || slot_idx >= g_handle_capacity) {
        return;
    }
    
    if (g_handle_table[slot_idx].active && g_handle_table[slot_idx].generation == generation) {
        g_handle_table[slot_idx].refcount++;
    }
}

bool interop_handle_is_valid(InteropHandle handle) {
    return interop_handle_resolve(handle) != NULL;
}
