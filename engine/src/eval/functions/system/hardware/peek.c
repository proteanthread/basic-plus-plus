// FILENAME: peek.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the PEEK built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/system/hardware/peek.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/funcreg.h"
void func_peek_register(void) {
    MicroLibMetadata meta = {
        .name = "PEEK",
        .category = "System Functions",
        .syntax = "PEEK(addr)",
        .help_text = "Reads a byte (0-255) from virtual memory address addr.",
        .error_codes = "Error 5: Illegal Function Call (addr out of memory bounds), Error 13: Type Mismatch (expects numeric address)"
    };
    microlib_register(&meta);

    FunctionEntry entry = {
        .name = "PEEK",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 1,
        .max_args = 1,
        .safety = FSAFE_SYSTEM,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Read a byte from memory",
        .module_name = "System"
    };
    funcreg_register(&entry);
}

#include "device/bus.h"
#include "bios/bios.h"
#include "vm/vm.h"
#include "security/security.h"
#include "esp32_regs.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#endif
BValue func_peek_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "PEEK expects one numeric address argument";
        return res;
    }

    if (security_check(SECOP_MEM_READ, 0) != 0) {
        err->code = 70;
        err->message = "Permission denied: PEEK is restricted";
        return res;
    }

    uint32_t addr_raw = (uint32_t)args[0].as.number;
    uint32_t phys_addr = addr_raw;
#ifndef BASIC_LITE_BUILD
    uint16_t def_seg = vmem_get_def_seg(vm_get_vmem(vm));
    if (def_seg != 0 && addr_raw < 0x10000U) {
        phys_addr = ((uint32_t)def_seg << 4) + addr_raw;
    }
#endif

    // Intercept ESP32 hardware register address space
    if (esp32_is_hardware_addr(phys_addr)) {
        res.type = VAL_NUMBER;
        res.as.number = (double)esp32_reg_read8(phys_addr);
        return res;
    }
    uint8_t val = 0;
    if (vm_get_bios(vm)) {
        val = bios_peek(vm_get_bios(vm), phys_addr);
    } else {
        bool intercepted = false;
        val = vdev_bus_peek(phys_addr, &intercepted);
    }

    res.type = VAL_NUMBER;
    res.as.number = (double)val;
    return res;
}
