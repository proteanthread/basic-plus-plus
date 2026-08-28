// FILENAME: data.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform, libserver
// Implements bytecode virtual machine execution and state for data.
//
// ---- Includes ----

#include "vm/vm.h"
#include "vm_internal.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "types/config.h"
#include "runtime/metadata.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#ifndef BASIC_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "device/usb.h"
#include "runtime/file.h"
#include "device/vcon.h"
#include "device/bus.h"
#include "runtime/spec.h"
#include "security/security.h"
#include "eval/eval.h"
#include "debug/logger.h"
#include "runtime/variables.h"
#include "platform/platform.h"
#include "core/struct.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "hal/hal.h"

void vm_build_data_table(VMContext *vm) {
    if (!vm) return;
    HalContext *hal = hal_get();

    // Free old table
    if (vm->data_items) {
        if (hal && hal->mem.free) hal->mem.free(vm->data_items);
        vm->data_items = NULL;
    }
    vm->data_count = 0;
    vm->data_ptr = 0;

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    // Allocate capacity
    int capacity = 32;
    if (hal && hal->mem.alloc) {
        vm->data_items = (BppDataPosition *)hal->mem.alloc(capacity * sizeof(BppDataPosition));
    }
    if (!vm->data_items) return;
    runtime_memset(vm->data_items, 0, capacity * sizeof(BppDataPosition));

    for (size_t i = 0; i < count; ++i) {
        LexerContext *lex = lex_init(mem, lines[i].text);
        if (!lex) continue;

        BppToken tok = lex_next(lex);
        while (tok.type != TOK_EOF) {
            if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_DATA) {
                const char *p = tok.start + tok.length;
                while (*p) {
                    while (*p && runtime_isspace((unsigned char)*p)) p++;
                    if (*p == '\0' || *p == ':' || *p == '\n' || *p == '\r') break;

                    if (vm->data_count >= capacity) {
                        int old_cap = capacity;
                        capacity *= 2;
                        BppDataPosition *temp = NULL;
                        if (hal && hal->mem.realloc) {
                            temp = (BppDataPosition *)hal->mem.realloc(vm->data_items, capacity * sizeof(BppDataPosition));
                        } else if (hal && hal->mem.alloc) {
                            temp = (BppDataPosition *)hal->mem.alloc(capacity * sizeof(BppDataPosition));
                            if (temp && vm->data_items) {
                                runtime_memcpy(temp, vm->data_items, old_cap * sizeof(BppDataPosition));
                                if (hal->mem.free) hal->mem.free(vm->data_items);
                            }
                        }
                        if (!temp) {
                            lex_shutdown(lex);
                            return;
                        }
                        runtime_memset(temp + old_cap, 0, (capacity - old_cap) * sizeof(BppDataPosition));
                        vm->data_items = temp;
                    }

                    vm->data_items[vm->data_count].line = lines[i].line_number;
                    vm->data_items[vm->data_count].pos = p;
                    vm->data_count++;


                    if (*p == '"') {
                        p++;
                        while (*p && *p != '"' && *p != '\n' && *p != '\r') p++;
                        if (*p == '"') p++;
                    } else {
                        while (*p && *p != ',' && *p != ':' && *p != '\n' && *p != '\r') p++;
                    }

                    while (*p && runtime_isspace((unsigned char)*p)) p++;
                    if (*p == ',') {
                        p++;
                    } else {
                        break;
                    }
                }
                lex_shutdown(lex);
                lex = lex_init(mem, p);
                if (!lex) break;
            }
            tok = lex_next(lex);
        }
        if (lex) lex_shutdown(lex);
    }
}

int vm_get_data_ptr(VMContext *vm) {
    return vm ? vm->data_ptr : 0;
}

void vm_set_data_ptr(VMContext *vm, int ptr) {
    if (vm) {
        vm->data_ptr = ptr;
    }
}

int vm_get_data_count(VMContext *vm) {
    return vm ? vm->data_count : 0;
}

BppDataPosition *vm_get_data_items(VMContext *vm) {
    return vm ? vm->data_items : NULL;
}
