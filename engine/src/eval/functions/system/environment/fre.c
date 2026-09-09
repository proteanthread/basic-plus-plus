// FILENAME: fre.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (fre.h, string.c)
// Provides runtime implementation for the FRE built-in function in BASIC++.

#include "eval/functions/system/environment/fre.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "memory/memory.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"

static const LangDesc g_fre_desc = {
    .name = "FRE",
    .category = "System Functions",
    .syntax = "FRE([pool_code])",
    .description = "Returns available memory bytes for subsystem pools (-1: vars, -2: strings, -3: stack, -4: files, -5: disk, -6: vdev, -7: vram, -8: net, -9: audio, 0: heap, 1..64: 1MB RAMBanks).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_fre_register(void) {
    lang_desc_register(&g_fre_desc);
}

BValue func_fre_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "MEM") == 0) {
        if (arg_count != 0) {
            err->code = 13;
            err->message = "MEM expects no arguments";
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = (double)mem_get_total_ram(vm_get_mem(vm));
        return res;
    }

    if (arg_count > 1) {
        err->code = 13;
        err->message = "FRE expects 0 or 1 arguments";
        return res;
    }

    int pool_code = 0;
    if (arg_count == 1) {
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
            pool_code = 0;
        } else if (args[0].type == VAL_NUMBER || args[0].type == VAL_INTEGER) {
            pool_code = (int)args[0].as.number;
        }
    }

    res.type = VAL_NUMBER;
    MemoryContext *mem = vm_get_mem(vm);
    double free_ram = mem ? (double)mem_get_free_ram(mem) : 671088640.0;

    switch (pool_code) {
        case 0:
            // General Free RAM Pool
            res.as.number = free_ram;
            break;
        case -1:
            // Variable / Symbol Table Memory Pool
            res.as.number = 16777216.0; // 16 MB allocated symbol table capacity
            break;
        case -2:
            // String Heap / Pool Capacity
            res.as.number = free_ram * 0.40; // 40% reserved for dynamic string allocations
            break;
        case -3:
            // VM Execution Stack Remaining Bytes
            res.as.number = 8388608.0; // 8 MB execution stack
            break;
        case -4:
            // File Channels / VFS Buffer Pool
            res.as.number = 4194304.0; // 4 MB I/O channel buffers
            break;
        case -5:
            // Virtual Disk / Storage Capacity
            res.as.number = 1073741824.0; // 1 GB virtual disk storage pool
            break;
        case -6:
            // Virtual Device / VFS Cache Pool
            res.as.number = 33554432.0; // 32 MB VFS cache pool
            break;
        case -7:
            // Graphics & Canvas VRAM Pool
            res.as.number = 67108864.0; // 64 MB VRAM graphics buffer
            break;
        case -8:
            // Network & Socket Buffer Pool
            res.as.number = 16777216.0; // 16 MB network/socket buffer pool
            break;
        case -9:
            // Audio & Sound Synthesis Buffer Pool
            res.as.number = 8388608.0; // 8 MB audio PCM buffer pool
            break;
        default:
            if (pool_code >= 1 && pool_code <= 64) {
                // Segmented 1MB RAMBank (1048576 bytes per bank in IBM PC BIOS VM)
                res.as.number = 1048576.0;
            } else if (pool_code > 64) {
                res.as.number = 0.0;
            } else {
                res.as.number = free_ram;
            }
            break;
    }

    return res;
}
