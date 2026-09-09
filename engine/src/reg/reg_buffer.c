// FILENAME: reg_buffer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libreg, libengine, BASIC++ runtime
// NEEDS: libreg (reg_buffer.h, reg_hw.h, reg_udx.h), runtime (strops.h, strings.h)
// Implements first-class separate buffers (FIFO, LIFO, LILO, FILO) and named channels in UDX.

#include "reg/reg_buffer.h"
#include "reg/reg_hw.h"
#include "reg/reg_udx.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/strings.h"

// Static pool of channels (Model 1 / Model 3 pool allocation)
static RegBufferChannel s_channels[REG_MAX_CHANNELS];
static bool s_initialized = false;

// Custom discipline registry
#define REG_MAX_CUSTOM_DISCIPLINES 8
typedef struct {
    char name[32];
    RegDisciplineHandler handler;
} CustomDisciplineEntry;

static CustomDisciplineEntry s_custom_disciplines[REG_MAX_CUSTOM_DISCIPLINES];
static int s_custom_discipline_count = 0;

static void buffer_release_val(BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_release(NULL, val.as.string);
    }
}

static void buffer_retain_val(BValue val) {
    if (val.type == VAL_STRING && val.as.string) {
        str_add_ref(val.as.string);
    }
}

void reg_buffer_reset(void) {
    for (int i = 0; i < REG_MAX_CHANNELS; i++) {
        for (int j = 0; j < s_channels[i].count; j++) {
            int idx = (s_channels[i].head + j) % REG_BUFFER_CAPACITY;
            buffer_release_val(s_channels[i].items[idx]);
        }
        runtime_memset(&s_channels[i], 0, sizeof(RegBufferChannel));
    }

    // Initialize 4 default system channels
    runtime_strncpy(s_channels[0].name, "FIFO", sizeof(s_channels[0].name) - 1);
    s_channels[0].discipline = REG_DISCIPLINE_FIFO;
    s_channels[0].active = true;

    runtime_strncpy(s_channels[1].name, "LIFO", sizeof(s_channels[1].name) - 1);
    s_channels[1].discipline = REG_DISCIPLINE_LIFO;
    s_channels[1].active = true;

    runtime_strncpy(s_channels[2].name, "LILO", sizeof(s_channels[2].name) - 1);
    s_channels[2].discipline = REG_DISCIPLINE_LILO;
    s_channels[2].active = true;

    runtime_strncpy(s_channels[3].name, "FILO", sizeof(s_channels[3].name) - 1);
    s_channels[3].discipline = REG_DISCIPLINE_FILO;
    s_channels[3].active = true;

    s_initialized = true;
}

static void ensure_initialized(void) {
    if (!s_initialized) {
        reg_buffer_reset();
    }
}

RegBufferChannel* reg_buffer_get_default(RegDisciplineType disc) {
    ensure_initialized();
    switch (disc) {
        case REG_DISCIPLINE_FIFO: return &s_channels[0];
        case REG_DISCIPLINE_LIFO: return &s_channels[1];
        case REG_DISCIPLINE_LILO: return &s_channels[2];
        case REG_DISCIPLINE_FILO: return &s_channels[3];
        default: return &s_channels[0];
    }
}

RegBufferChannel* reg_buffer_channel_get(const char *name, const char *discipline_name) {
    ensure_initialized();
    if (!name || name[0] == '\0') return &s_channels[0];

    // Check existing default/named channels
    for (int i = 0; i < REG_MAX_CHANNELS; i++) {
        if (s_channels[i].active && runtime_strcasecmp(s_channels[i].name, name) == 0) {
            return &s_channels[i];
        }
    }

    // Allocate from static pool
    for (int i = 4; i < REG_MAX_CHANNELS; i++) {
        if (!s_channels[i].active) {
            s_channels[i].active = true;
            runtime_strncpy(s_channels[i].name, name, sizeof(s_channels[i].name) - 1);
            s_channels[i].head = 0;
            s_channels[i].tail = 0;
            s_channels[i].count = 0;

            RegDisciplineType disc = REG_DISCIPLINE_FIFO;
            if (discipline_name) {
                if (runtime_strcasecmp(discipline_name, "LIFO") == 0) disc = REG_DISCIPLINE_LIFO;
                else if (runtime_strcasecmp(discipline_name, "LILO") == 0) disc = REG_DISCIPLINE_LILO;
                else if (runtime_strcasecmp(discipline_name, "FILO") == 0) disc = REG_DISCIPLINE_FILO;
                else if (runtime_strcasecmp(discipline_name, "FIFO") == 0) disc = REG_DISCIPLINE_FIFO;
                else disc = REG_DISCIPLINE_CUSTOM;
            }
            s_channels[i].discipline = disc;
            return &s_channels[i];
        }
    }

    // Fallback if pool full
    return &s_channels[0];
}

bool reg_buffer_push(RegBufferChannel *chan, BValue val) {
    if (!chan || chan->count >= REG_BUFFER_CAPACITY) return false;

    // Check custom discipline
    if (chan->discipline == REG_DISCIPLINE_CUSTOM) {
        for (int i = 0; i < s_custom_discipline_count; i++) {
            if (s_custom_disciplines[i].handler) {
                BValue dummy;
                return s_custom_disciplines[i].handler(chan, false, val, &dummy);
            }
        }
    }

    buffer_retain_val(val);
    chan->items[chan->tail] = val;
    chan->tail = (chan->tail + 1) % REG_BUFFER_CAPACITY;
    chan->count++;
    return true;
}

bool reg_buffer_pop(RegBufferChannel *chan, BValue *out_val) {
    if (!chan || !out_val || chan->count <= 0) return false;

    // Check custom discipline
    if (chan->discipline == REG_DISCIPLINE_CUSTOM) {
        for (int i = 0; i < s_custom_discipline_count; i++) {
            if (s_custom_disciplines[i].handler) {
                BValue dummy;
                dummy.type = VAL_NUMBER; dummy.as.number = 0.0;
                return s_custom_disciplines[i].handler(chan, true, dummy, out_val);
            }
        }
    }

    if (chan->discipline == REG_DISCIPLINE_LIFO || chan->discipline == REG_DISCIPLINE_FILO) {
        // Pop last inserted item (stack top)
        chan->tail = (chan->tail - 1 + REG_BUFFER_CAPACITY) % REG_BUFFER_CAPACITY;
        *out_val = chan->items[chan->tail];
        chan->count--;
        return true;
    }

    // FIFO / LILO: Pop oldest inserted item (queue front)
    *out_val = chan->items[chan->head];
    chan->head = (chan->head + 1) % REG_BUFFER_CAPACITY;
    chan->count--;
    return true;
}

bool reg_buffer_peek(RegBufferChannel *chan, BValue *out_val) {
    if (!chan || !out_val || chan->count <= 0) return false;

    if (chan->discipline == REG_DISCIPLINE_LIFO || chan->discipline == REG_DISCIPLINE_FILO) {
        int idx = (chan->tail - 1 + REG_BUFFER_CAPACITY) % REG_BUFFER_CAPACITY;
        *out_val = chan->items[idx];
        buffer_retain_val(*out_val);
        return true;
    }

    *out_val = chan->items[chan->head];
    buffer_retain_val(*out_val);
    return true;
}

int reg_buffer_count(RegBufferChannel *chan) {
    return chan ? chan->count : 0;
}

void reg_buffer_clear(RegBufferChannel *chan) {
    if (!chan) return;
    for (int j = 0; j < chan->count; j++) {
        int idx = (chan->head + j) % REG_BUFFER_CAPACITY;
        buffer_release_val(chan->items[idx]);
    }
    chan->head = 0;
    chan->tail = 0;
    chan->count = 0;
}

static RegBufferChannel* resolve_channel(const char *name) {
    if (!name) return reg_buffer_get_default(REG_DISCIPLINE_FIFO);
    if (runtime_strcasecmp(name, "FIFO") == 0) return reg_buffer_get_default(REG_DISCIPLINE_FIFO);
    if (runtime_strcasecmp(name, "LIFO") == 0) return reg_buffer_get_default(REG_DISCIPLINE_LIFO);
    if (runtime_strcasecmp(name, "LILO") == 0) return reg_buffer_get_default(REG_DISCIPLINE_LILO);
    if (runtime_strcasecmp(name, "FILO") == 0) return reg_buffer_get_default(REG_DISCIPLINE_FILO);
    return reg_buffer_channel_get(name, NULL);
}

bool reg_buffer_push_by_name(const char *chan_or_disc, BValue val) {
    RegBufferChannel *chan = resolve_channel(chan_or_disc);
    return reg_buffer_push(chan, val);
}

bool reg_buffer_pop_by_name(const char *chan_or_disc, BValue *out_val) {
    RegBufferChannel *chan = resolve_channel(chan_or_disc);
    return reg_buffer_pop(chan, out_val);
}

int reg_buffer_count_by_name(const char *chan_or_disc) {
    RegBufferChannel *chan = resolve_channel(chan_or_disc);
    return reg_buffer_count(chan);
}

bool reg_discipline_register(const char *name, RegDisciplineHandler handler) {
    if (!name || !handler || s_custom_discipline_count >= REG_MAX_CUSTOM_DISCIPLINES) return false;
    runtime_strncpy(s_custom_disciplines[s_custom_discipline_count].name, name,
                    sizeof(s_custom_disciplines[s_custom_discipline_count].name) - 1);
    s_custom_disciplines[s_custom_discipline_count].handler = handler;
    s_custom_discipline_count++;
    return true;
}

// Transfer data between registers/buffers across the UDX bus
bool reg_buffer_transfer(const char *src_spec, const char *dst_spec) {
    if (!src_spec || !dst_spec) return false;

    BValue val;
    runtime_memset(&val, 0, sizeof(val));
    val.type = VAL_NUMBER;

    // Read from source
    if (runtime_strncasecmp(src_spec, "FIFO", 4) == 0 || runtime_strncasecmp(src_spec, "LIFO", 4) == 0 ||
        runtime_strncasecmp(src_spec, "LILO", 4) == 0 || runtime_strncasecmp(src_spec, "FILO", 4) == 0) {
        if (!reg_buffer_pop_by_name(src_spec, &val)) return false;
    } else if (runtime_strncasecmp(src_spec, "XCHG.", 5) == 0 || runtime_strncasecmp(src_spec, "UDX.", 4) == 0) {
        if (!reg_udx_get(src_spec, &val)) return false;
    } else if (runtime_strncasecmp(src_spec, "REG.", 4) == 0) {
        bool found = false;
        double n = reg_hw_get(src_spec, &found);
        if (!found) return false;
        val.type = VAL_NUMBER;
        val.as.number = n;
    } else {
        return false;
    }

    // Write to destination
    if (runtime_strncasecmp(dst_spec, "FIFO", 4) == 0 || runtime_strncasecmp(dst_spec, "LIFO", 4) == 0 ||
        runtime_strncasecmp(dst_spec, "LILO", 4) == 0 || runtime_strncasecmp(dst_spec, "FILO", 4) == 0) {
        return reg_buffer_push_by_name(dst_spec, val);
    } else if (runtime_strncasecmp(dst_spec, "XCHG.", 5) == 0 || runtime_strncasecmp(dst_spec, "UDX.", 4) == 0) {
        return reg_udx_set(dst_spec, val);
    } else if (runtime_strncasecmp(dst_spec, "REG.", 4) == 0) {
        double n = (val.type == VAL_NUMBER || val.type == VAL_INTEGER) ? val.as.number : 0.0;
        return reg_hw_set(dst_spec, n);
    }

    return false;
}
