// FILENAME: bios.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, libhardware, libkernel
// NEEDS: libcore (alloc.h, alloc.c, hal.h, memops.h, memops.c)
// NEEDS: libengine (bios.h)
// NEEDS: libhardware (bios_at.h, bios_at.c, bios_jr.h, bios_jr.c)
// NEEDS: libhardware (bios_pc.h, bios_pc.c, bios_xt.h, bios_xt.c)
// Implements virtual BIOS interrupt and hardware emulation for bios.
//
// ---- Includes ----

#include "bios/bios.h"
#include "bios/bios_pc.h"
#include "bios/bios_xt.h"
#include "bios/bios_at.h"
#include "bios/bios_jr.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "hal/hal.h"

typedef struct {
    BiosIntHandlerFn fn;
    void*            user_data;
} BiosCustomInt;

struct BiosContext {
    BiosModel       model;
    BiosRevision    revision;
    BiosClockMode   clock_mode;
    double          clock_freq;
    BiosMemoryMap   mem_map;
    BiosVRAMObserver vram_observer;
    uint8_t*        flat_1mb;
    uint8_t         cmos_regs[128];
    uint8_t         cmos_idx;
    uint8_t         last_post_code;
    BiosCustomInt   custom_ints[256];
};

// Global BIOS context pointer for subsystems that lack direct VM access
// (e.g., console.c PRINT-to-VRAM sync). Set by bios_create(), cleared by
// bios_destroy(). Follows same pattern as g_vcon_context in vcon.c.
BiosContext *g_bios_context = NULL;

static uint8_t default_mem_read(void* user_data, uint32_t addr) {
    BiosContext* ctx = (BiosContext*)user_data;
    if (!ctx || !ctx->flat_1mb || addr >= 1048576U) {
        return 0xFF;
    }
    return ctx->flat_1mb[addr];
}

static void default_mem_write(void* user_data, uint32_t addr, uint8_t val) {
    BiosContext* ctx = (BiosContext*)user_data;
    if (!ctx || !ctx->flat_1mb || addr >= 1048576U) {
        return;
    }
    // Protect BIOS ROM region (0xF0000-0xFFFFF) from accidental overwrite
    if (addr >= 0xF0000U) {
        return;
    }
    ctx->flat_1mb[addr] = val;
}

BiosContext* bios_create(BiosModel model) {
    HalContext *hal = hal_get();
    BiosContext* ctx = NULL;
    if (hal && hal->mem.alloc) {
        ctx = (BiosContext*)hal->mem.alloc(sizeof(BiosContext));
    }
    if (!ctx) return NULL;
    runtime_memset(ctx, 0, sizeof(BiosContext));

    if (hal && hal->mem.alloc) {
        ctx->flat_1mb = (uint8_t*)hal->mem.alloc(1048576U);
    }
    if (!ctx->flat_1mb) {
        if (hal && hal->mem.free) hal->mem.free(ctx);
        return NULL;
    }
    runtime_memset(ctx->flat_1mb, 0, 1048576U);

    ctx->mem_map.user_data = ctx;
    ctx->mem_map.read_u8 = default_mem_read;
    ctx->mem_map.write_u8 = default_mem_write;
    ctx->mem_map.flat_1mb_ptr = ctx->flat_1mb;

    ctx->model = model;
    ctx->clock_mode = BIOS_CLOCK_AUTOMATIC;
    ctx->clock_freq = (model == BIOS_MODEL_IBM_AT) ? 6.00 : 4.77;

    switch (model) {
        case BIOS_MODEL_IBM_PC:   ctx->revision = BIOS_REV_PC_1981_04_24; break;
        case BIOS_MODEL_IBM_XT:   ctx->revision = BIOS_REV_XT_1982_11_08; break;
        case BIOS_MODEL_IBM_AT:   ctx->revision = BIOS_REV_AT_1984_01_10; break;
        case BIOS_MODEL_IBM_PCJR: ctx->revision = BIOS_REV_PCJR_1983_06_01; break;
    }

    bios_init(ctx);
    g_bios_context = ctx;
    return ctx;
}

void bios_destroy(BiosContext* ctx) {
    if (!ctx) return;
    if (g_bios_context == ctx) {
        g_bios_context = NULL;
    }
    HalContext *hal = hal_get();
    if (ctx->flat_1mb) {
        if (hal && hal->mem.free) hal->mem.free(ctx->flat_1mb);
        ctx->flat_1mb = NULL;
    }
    if (hal && hal->mem.free) {
        hal->mem.free(ctx);
    }
}

void bios_set_model(BiosContext* ctx, BiosModel model) {
    if (!ctx) return;
    ctx->model = model;
    switch (model) {
        case BIOS_MODEL_IBM_PC:   ctx->revision = BIOS_REV_PC_1981_04_24; ctx->clock_freq = 4.77; break;
        case BIOS_MODEL_IBM_XT:   ctx->revision = BIOS_REV_XT_1982_11_08; ctx->clock_freq = 4.77; break;
        case BIOS_MODEL_IBM_AT:   ctx->revision = BIOS_REV_AT_1984_01_10; ctx->clock_freq = 6.00; break;
        case BIOS_MODEL_IBM_PCJR: ctx->revision = BIOS_REV_PCJR_1983_06_01; ctx->clock_freq = 4.77; break;
    }
    bios_init(ctx);
}

BiosModel bios_get_model(const BiosContext* ctx) {
    return ctx ? ctx->model : BIOS_MODEL_IBM_PC;
}

void bios_set_revision(BiosContext* ctx, BiosRevision revision) {
    if (!ctx) return;
    ctx->revision = revision;
    switch (revision) {
        case BIOS_REV_PC_1981_04_24:
        case BIOS_REV_PC_1981_10_19:
        case BIOS_REV_PC_1982_08_16:
        case BIOS_REV_PC_1982_10_27:
            ctx->model = BIOS_MODEL_IBM_PC;
            ctx->clock_freq = 4.77;
            break;
        case BIOS_REV_XT_1982_11_08:
        case BIOS_REV_XT_1986_01_10:
        case BIOS_REV_XT_1986_05_09:
            ctx->model = BIOS_MODEL_IBM_XT;
            ctx->clock_freq = 4.77;
            break;
        case BIOS_REV_AT_1984_01_10:
        case BIOS_REV_AT_1985_06_10:
            ctx->model = BIOS_MODEL_IBM_AT;
            ctx->clock_freq = 6.00;
            break;
        case BIOS_REV_AT_1985_11_15:
            ctx->model = BIOS_MODEL_IBM_AT;
            ctx->clock_freq = 8.00;
            break;
        case BIOS_REV_PCJR_1983_06_01:
            ctx->model = BIOS_MODEL_IBM_PCJR;
            ctx->clock_freq = 4.77;
            break;
    }
    bios_init(ctx);
}

BiosRevision bios_get_revision(const BiosContext* ctx) {
    return ctx ? ctx->revision : BIOS_REV_PC_1981_04_24;
}

const char* bios_get_part_number(const BiosContext* ctx) {
    if (!ctx) return "5700051";
    switch (ctx->revision) {
        case BIOS_REV_PC_1981_04_24: return "5700051";
        case BIOS_REV_PC_1981_10_19: return "5700671";
        case BIOS_REV_PC_1982_08_16: return "5000024";
        case BIOS_REV_PC_1982_10_27: return "1501476";
        case BIOS_REV_XT_1982_11_08: return "1501512";
        case BIOS_REV_XT_1986_01_10: return "62X0819";
        case BIOS_REV_XT_1986_05_09: return "62X0851";
        case BIOS_REV_AT_1984_01_10: return "1501529";
        case BIOS_REV_AT_1985_06_10: return "6181028";
        case BIOS_REV_AT_1985_11_15: return "61F9018";
        case BIOS_REV_PCJR_1983_06_01: return "1501401";
    }
    return "5700051";
}

void bios_set_clock_mode(BiosContext* ctx, BiosClockMode mode) {
    if (ctx) ctx->clock_mode = mode;
}

BiosClockMode bios_get_clock_mode(const BiosContext* ctx) {
    return ctx ? ctx->clock_mode : BIOS_CLOCK_AUTOMATIC;
}

void bios_set_clock_freq(BiosContext* ctx, double mhz) {
    if (ctx && mhz > 0.0) ctx->clock_freq = mhz;
}

double bios_get_clock_freq(const BiosContext* ctx) {
    return ctx ? ctx->clock_freq : 4.77;
}

void bios_set_memory_map(BiosContext* ctx, const BiosMemoryMap* map) {
    if (!ctx || !map) return;
    ctx->mem_map = *map;
}

BiosDataArea* bios_get_bda(BiosContext* ctx) {
    if (!ctx) return NULL;
    if (ctx->mem_map.flat_1mb_ptr) {
        return (BiosDataArea*)&ctx->mem_map.flat_1mb_ptr[0x0400];
    }
    return NULL;
}

bool bios_init(BiosContext* ctx) {
    if (!ctx) return false;

    // Initialize model specific BDA & ROM signatures
    switch (ctx->model) {
        case BIOS_MODEL_IBM_PC:
            bios_pc_init(ctx);
            break;
        case BIOS_MODEL_IBM_XT:
            bios_xt_init(ctx);
            break;
        case BIOS_MODEL_IBM_AT:
            bios_at_init(ctx);
            break;
        case BIOS_MODEL_IBM_PCJR:
            bios_jr_init(ctx);
            break;
        default:
            bios_pc_init(ctx);
            break;
    }
    return true;
}

uint8_t bios_peek(BiosContext* ctx, uint32_t addr) {
    if (!ctx || !ctx->mem_map.read_u8) return 0xFF;
    return ctx->mem_map.read_u8(ctx->mem_map.user_data, addr & 0xFFFFF);
}

uint8_t bios_peek_raw(BiosContext* ctx, uint32_t addr) {
    if (!ctx || !ctx->flat_1mb || addr >= 1048576U) return 0xFF;
    return ctx->flat_1mb[addr & 0xFFFFF];
}

void bios_poke(BiosContext* ctx, uint32_t addr, uint8_t val) {
    if (!ctx || !ctx->mem_map.write_u8) return;
    uint32_t masked = addr & 0xFFFFF;
    ctx->mem_map.write_u8(ctx->mem_map.user_data, masked, val);

    // Notify VRAM observer if write falls in a video memory region
    if (ctx->vram_observer.on_write) {
        if ((masked >= BIOS_VRAM_EGA_START && masked <= BIOS_VRAM_EGA_END) ||
            (masked >= BIOS_VRAM_MDA_START && masked <= BIOS_VRAM_MDA_END) ||
            (masked >= BIOS_VRAM_CGA_START && masked <= BIOS_VRAM_CGA_END)) {
            ctx->vram_observer.on_write(ctx->vram_observer.user_data, masked, val);
        }
    }
}

void bios_poke_raw(BiosContext* ctx, uint32_t addr, uint8_t val) {
    if (!ctx || !ctx->flat_1mb || addr >= 1048576U) return;
    ctx->flat_1mb[addr & 0xFFFFF] = val;
}

void bios_set_vram_observer(BiosContext* ctx, const BiosVRAMObserver* observer) {
    if (!ctx) return;
    if (observer) {
        ctx->vram_observer = *observer;
    } else {
        runtime_memset(&ctx->vram_observer, 0, sizeof(ctx->vram_observer));
    }
}

const BiosVRAMObserver* bios_get_vram_observer(const BiosContext* ctx) {
    if (!ctx) return NULL;
    return &ctx->vram_observer;
}

uint8_t bios_inp(BiosContext* ctx, uint16_t port) {
    if (!ctx) return 0xFF;
    switch (ctx->model) {
        case BIOS_MODEL_IBM_PC:   return bios_pc_inp(ctx, port);
        case BIOS_MODEL_IBM_XT:   return bios_xt_inp(ctx, port);
        case BIOS_MODEL_IBM_AT:   return bios_at_inp(ctx, port);
        case BIOS_MODEL_IBM_PCJR: return bios_jr_inp(ctx, port);
    }
    return 0xFF;
}

void bios_out(BiosContext* ctx, uint16_t port, uint8_t val) {
    if (!ctx) return;
    switch (ctx->model) {
        case BIOS_MODEL_IBM_PC:   bios_pc_out(ctx, port, val); break;
        case BIOS_MODEL_IBM_XT:   bios_xt_out(ctx, port, val); break;
        case BIOS_MODEL_IBM_AT:   bios_at_out(ctx, port, val); break;
        case BIOS_MODEL_IBM_PCJR: bios_jr_out(ctx, port, val); break;
    }
}

void bios_post_code(BiosContext* ctx, uint8_t code) {
    if (!ctx) return;
    ctx->last_post_code = code;
    // Write POST diagnostic code to Port 0x80
    bios_out(ctx, 0x80, code);
}

bool bios_register_interrupt(BiosContext* ctx, uint8_t int_num, BiosIntHandlerFn handler, void* user_data) {
    if (!ctx) return false;
    ctx->custom_ints[int_num].fn = handler;
    ctx->custom_ints[int_num].user_data = user_data;
    return true;
}

bool bios_interrupt(BiosContext* ctx, uint8_t int_num, BiosRegs* regs) {
    if (!ctx || !regs) return false;

    // Check custom registered interrupt handlers first
    if (ctx->custom_ints[int_num].fn) {
        if (ctx->custom_ints[int_num].fn(ctx, int_num, regs, ctx->custom_ints[int_num].user_data)) {
            return true;
        }
    }

    switch (ctx->model) {
        case BIOS_MODEL_IBM_PC:   return bios_pc_interrupt(ctx, int_num, regs);
        case BIOS_MODEL_IBM_XT:   return bios_xt_interrupt(ctx, int_num, regs);
        case BIOS_MODEL_IBM_AT:   return bios_at_interrupt(ctx, int_num, regs);
        case BIOS_MODEL_IBM_PCJR: return bios_jr_interrupt(ctx, int_num, regs);
    }
    return false;
}
