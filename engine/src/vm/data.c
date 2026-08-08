/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file data.c
 * @brief VM DATA, READ, and RESTORE stream pointer tracking and evaluation logic for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `vm_data_scan()`, `vm_data_read()`, and `vm_data_restore()` for inline program DATA item stream reading.
 *
 * 2. WHY IT EXISTS:
 * Provides GW-BASIC and QBASIC DATA/READ/RESTORE sequential data parsing and line label RESTORE positioning.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Scans program source code for `DATA` statements, stores item positions in `vm->data_items` array, tracks current read offset (`vm->data_ptr`), and parses string/number items into `BValue` on `READ`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake library targets 'libbasicpp' and 'libbasicpp_lite'. Includes "vm/vm.h", "vm_internal.h",
 * "stmt/stmt.h", "eval/eval.h", "runtime/variables.h", "runtime/strings.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support typed DATA streams (e.g. DATA INT, DATA FLOAT) or RESTORE with string labels.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Sequential READ pointer advancement and RESTORE line resetting invariant.
 *
 * 8. WHAT TO EXPECT:
 * `vm_data_read()` returns `VAL_STRING` or `VAL_NUMBER` BValue or ERR_OUT_OF_DATA (error 4) on stream exhaustion.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `vm_data_scan()` execution before program run and inspect `vm->data_ptr` index bounds.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid `VMContext` with loaded program lines.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. All string tokens bounded by length (`memcmp`/`strncpy`).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/vm/context.c
 * - engine/src/runtime/strings.c
 * - engine/src/eval/eval.c
 * Prerequisite Header Files:
 * - engine/include/vm/vm.h
 * - engine/src/vm/vm_internal.h
 * - engine/include/eval/eval.h
 */
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

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


void vm_build_data_table(VMContext *vm) {
    if (!vm) return;

    /* Free old table */
    if (vm->data_items) {
        free(vm->data_items);
        vm->data_items = NULL;
    }
    vm->data_count = 0;
    vm->data_ptr = 0;

    MemoryContext *mem = vm_get_mem(vm);
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    /* Allocate capacity */
    int capacity = 32;
    vm->data_items = (BppDataPosition *)calloc(capacity, sizeof(BppDataPosition));
    if (!vm->data_items) return;

    for (size_t i = 0; i < count; ++i) {
        LexerContext *lex = lex_init(mem, lines[i].text);
        if (!lex) continue;

        BppToken tok = lex_next(lex);
        while (tok.type != TOK_EOF) {
            if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_DATA) {
                while (true) {
                    BppToken val_tok = lex_peek(lex);
                    if (val_tok.type == TOK_EOL || val_tok.type == TOK_EOF || 
                        (val_tok.type == TOK_KEYWORD && val_tok.as.keyword != KW_NONE)) {
                        break;
                    }

                    if (vm->data_count >= capacity) {
                        capacity *= 2;
                        BppDataPosition *temp = (BppDataPosition *)realloc(vm->data_items, capacity * sizeof(BppDataPosition));
                        if (!temp) {
                            lex_shutdown(lex);
                            return;
                        }
                        /* Zero-init new entries per project rules */
                        memset(temp + (capacity / 2), 0, (capacity / 2) * sizeof(BppDataPosition));
                        vm->data_items = temp;
                    }

                    vm->data_items[vm->data_count].line = lines[i].line_number;
                    vm->data_items[vm->data_count].pos = val_tok.start;
                    vm->data_count++;

                    lex_next(lex); /* Consume literal */
                    BppToken comma = lex_peek(lex);
                    if (comma.type == TOK_COMMA) {
                        lex_next(lex); /* Consume ',' */
                    } else {
                        break;
                    }
                }
            }
            tok = lex_next(lex);
        }
        lex_shutdown(lex);
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
