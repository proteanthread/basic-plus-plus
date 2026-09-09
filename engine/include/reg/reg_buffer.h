// FILENAME: reg_buffer.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libreg, libengine, BASIC++ runtime
// NEEDS: types/types.h, stdbool.h
// Provides definitions for first-class separate buffers (FIFO, LIFO, LILO, FILO) and named channels in UDX.

#ifndef ENGINE_REG_BUFFER_H
#define ENGINE_REG_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define REG_BUFFER_CAPACITY 256
#define REG_MAX_CHANNELS    16

// Buffer ordering discipline enumeration
typedef enum {
    REG_DISCIPLINE_FIFO, // First In, First Out
    REG_DISCIPLINE_LIFO, // Last In, First Out
    REG_DISCIPLINE_LILO, // Last In, Last Out
    REG_DISCIPLINE_FILO, // First In, Last Out
    REG_DISCIPLINE_CUSTOM
} RegDisciplineType;

// Buffer Channel Structure (fixed static ring buffer, Model 1 / Model 3 pool)
typedef struct {
    char              name[32];
    RegDisciplineType discipline;
    BValue            items[REG_BUFFER_CAPACITY];
    int               head;
    int               tail;
    int               count;
    bool              active;
} RegBufferChannel;

// Buffer transfer callback for custom disciplines
typedef bool (*RegDisciplineHandler)(RegBufferChannel *chan, bool is_pop, BValue in_val, BValue *out_val);

// Initialize/reset all system buffer channels
void reg_buffer_reset(void);

// Retrieve system default channel for a discipline ("FIFO", "LIFO", "LILO", "FILO")
RegBufferChannel* reg_buffer_get_default(RegDisciplineType disc);

// Retrieve or create named buffer channel
RegBufferChannel* reg_buffer_channel_get(const char *name, const char *discipline_name);

// Core buffer operations
bool reg_buffer_push(RegBufferChannel *chan, BValue val);
bool reg_buffer_pop(RegBufferChannel *chan, BValue *out_val);
bool reg_buffer_peek(RegBufferChannel *chan, BValue *out_val);
int  reg_buffer_count(RegBufferChannel *chan);
void reg_buffer_clear(RegBufferChannel *chan);

// Unified push/pop by discipline/channel name
bool reg_buffer_push_by_name(const char *chan_or_disc, BValue val);
bool reg_buffer_pop_by_name(const char *chan_or_disc, BValue *out_val);
int  reg_buffer_count_by_name(const char *chan_or_disc);

// Transfer data between registers/buffers across the UDX bus
bool reg_buffer_transfer(const char *src_spec, const char *dst_spec);

// Register custom discipline
bool reg_discipline_register(const char *name, RegDisciplineHandler handler);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_REG_BUFFER_H
