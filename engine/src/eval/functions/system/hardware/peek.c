// FILENAME: peek.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the PEEK built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/hardware/peek.h"
#include "runtime/language_descriptor.h"

static const LangDesc g_peek_desc = {
    .name = "PEEK",
    .category = "System Functions",
    .syntax = "PEEK(addr)",
    .description = "Reads a byte (0-255) from virtual memory address addr.",
    .error_summary = "Error 5: Illegal Function Call (addr out of memory bounds), Error 13: Type Mismatch (expects numeric address)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

#include "runtime/funcreg.h"
void func_peek_register(void) {
    lang_desc_register(&g_peek_desc);

    FunctionEntry entry = {
        .name = "PEEK",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 1,
        .max_args = 2,
        .safety = FSAFE_SYSTEM,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read byte(s) from memory: PEEK(addr [, width])",
        .module_name = "System"
    };
    funcreg_register(&entry);

    FunctionEntry dpeek_entry = entry;
    dpeek_entry.name = "DPEEK";
    dpeek_entry.min_args = 1;
    dpeek_entry.max_args = 1;
    dpeek_entry.help_text = "Read 16-bit word from memory: DPEEK(addr)";
    funcreg_register(&dpeek_entry);

    FunctionEntry lpeek_entry = entry;
    lpeek_entry.name = "LPEEK";
    lpeek_entry.min_args = 1;
    lpeek_entry.max_args = 1;
    lpeek_entry.help_text = "Read 32-bit dword from memory: LPEEK(addr)";
    funcreg_register(&lpeek_entry);

    FunctionEntry qpeek_entry = entry;
    qpeek_entry.name = "QPEEK";
    qpeek_entry.min_args = 1;
    qpeek_entry.max_args = 1;
    qpeek_entry.help_text = "Read 64-bit qword from memory: QPEEK(addr)";
    funcreg_register(&qpeek_entry);

    FunctionEntry peek_str_entry = entry;
    peek_str_entry.name = "PEEK$";
    peek_str_entry.ret_type = FRET_STRING;
    peek_str_entry.min_args = 1;
    peek_str_entry.max_args = 2;
    peek_str_entry.help_text = "Read memory block as string: PEEK$(addr [, len])";
    funcreg_register(&peek_str_entry);
}

#include "device/bus.h"
#include "bios/bios.h"
#include "vm/vm.h"
#include "security/security.h"
#include "esp32_regs.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "statements/system/hardware/def_seg.h"

BValue func_peek_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 1 || (arg_count > 2 && runtime_strcmp(uname, "PEEK$") != 0) || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "PEEK expects numeric address argument";
        return res;
    }

    if (security_check(SECOP_MEM_READ, 0) != 0) {
        err->code = 70;
        err->message = "Permission denied: PEEK is restricted";
        return res;
    }

    uint32_t addr_raw = (uint32_t)args[0].as.number;
    uint32_t phys_addr = addr_raw;
    uint16_t def_seg = 0;
#ifndef BASIC_LITE_BUILD
    if (vm_get_vmem(vm)) def_seg = vmem_get_def_seg(vm_get_vmem(vm));
#endif
    if (def_seg == 0) def_seg = runtime_get_def_seg();
    if (def_seg != 0 && addr_raw < 0x10000U) {
        phys_addr = ((uint32_t)def_seg << 4) + addr_raw;
    }

    // String byte block slice
    if (uname && runtime_strcmp(uname, "PEEK$") == 0) {
        size_t len = (arg_count >= 2 && args[1].type != VAL_STRING) ? (size_t)args[1].as.number : 1;
        char *buf = (char *)runtime_malloc(len + 1);
        if (!buf) {
            err->code = 14;
            err->message = "Out of memory in PEEK$";
            return res;
        }
        for (size_t k = 0; k < len; k++) {
            uint32_t curr_addr = phys_addr + (uint32_t)k;
            uint8_t byte_val = 0;
            if (esp32_is_hardware_addr(curr_addr)) {
                byte_val = esp32_reg_read8(curr_addr);
            } else if (vm_get_bios(vm)) {
                byte_val = bios_peek(vm_get_bios(vm), curr_addr);
            } else {
                bool intercepted = false;
                byte_val = vdev_bus_peek(curr_addr, &intercepted);
            }
            buf[k] = (char)byte_val;
        }
        buf[len] = '\0';
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, len);
        runtime_free(buf);
        return res;
    }

    int width = 1;
    if (uname && (runtime_strcmp(uname, "DPEEK") == 0 || runtime_strcmp(uname, "DEEK") == 0 || runtime_strcmp(uname, "PEEK2") == 0)) {
        width = 2;
    } else if (uname && (runtime_strcmp(uname, "LPEEK") == 0 || runtime_strcmp(uname, "PEEK4") == 0)) {
        width = 4;
    } else if (uname && (runtime_strcmp(uname, "QPEEK") == 0 || runtime_strcmp(uname, "PEEK8") == 0)) {
        width = 8;
    } else if (arg_count >= 2 && args[1].type != VAL_STRING) {
        width = (int)args[1].as.number;
        if (width != 1 && width != 2 && width != 4 && width != 8) width = 1;
    }

    uint64_t accum = 0;
    for (int k = 0; k < width; k++) {
        uint32_t curr_addr = phys_addr + (uint32_t)k;
        uint8_t byte_val = 0;
        if (esp32_is_hardware_addr(curr_addr)) {
            byte_val = esp32_reg_read8(curr_addr);
        } else if (vm_get_bios(vm)) {
            byte_val = bios_peek(vm_get_bios(vm), curr_addr);
        } else {
            bool intercepted = false;
            byte_val = vdev_bus_peek(curr_addr, &intercepted);
        }
        accum |= ((uint64_t)byte_val) << (k * 8);
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)accum;
    return res;
}
