/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
#include "vm/vm.h"
#include "vm_internal.h"
#include "stmt/stmt.h"
#include "device/vdev.h"
#include "types/config.h"
#include "runtime/metadata.h"
#include "runtime/vfs.h"
#include "runtime/vnet.h"
#ifndef BPP_LITE_BUILD
#include "memory/segmented_mem.h"
#endif
#include "device/usb.h"
#include "runtime/file.h"
#include "device/vcon.h"
#include "device/bus.h"
#include "bios/mock_bios.h"
#include "runtime/spec.h"
#include "security/security.h"
#include "eval/eval.h"
#include "debug/logger.h"
#include "runtime/variables.h"
#include "platform/platform.h"
#include "core/dialect.h"
#include "core/struct.h"
#include "module/module.h"

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
