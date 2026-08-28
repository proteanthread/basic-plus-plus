// FILENAME: dialect.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (context.c, print.c, try.c)
// NEEDS: libcore (alloc.h, alloc.c, dialect.h, hal.h, memops.h, memops.c)
// NEEDS: libcore (snprintf.h, snprintf.c, strops.h, strops.c)
// Provides core logic and interface definitions for dialect within BASIC++.
//
// ---- Includes ----

#include "core/dialect.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

BppDialect *dialect_create(void) {
    HalContext *hal = hal_get();
    BppDialect *d = NULL;
    if (hal && hal->mem.alloc) {
        d = (BppDialect *)hal->mem.alloc(sizeof(BppDialect));
    }
    if (!d) return NULL;
    runtime_memset(d, 0, sizeof(BppDialect));
    runtime_snprintf(d->name, sizeof(d->name), "BASIC++");
    d->comment_char = '\'';
    d->stmt_separator = ':';
    d->default_array_base = 1;
    d->case_sensitive = false;
    d->math_precedence = PRECEDENCE_STANDARD;
    d->member_access_char = '.';
    return d;
}

void dialect_free(BppDialect *d) {
    if (d) {
        HalContext *hal = hal_get();
        if (hal && hal->mem.free) {
            hal->mem.free(d);
        }
    }
}

bool dialect_load_from_map(VMContext *vm, BppMap *map, BppDialect *d, char *err_buf, size_t err_len) {
    (void)vm;
    (void)map;
    if (!d) {
        if (err_buf && err_len > 0) {
            runtime_snprintf(err_buf, err_len, "Invalid dialect context");
        }
        return false;
    }
    return true;
}

bool dialect_validate_map(VMContext *vm, BppMap *map, char *err_buf, size_t err_len) {
    (void)vm;
    (void)map;
    (void)err_buf;
    (void)err_len;
    return true;
}

char *dialect_generate_docs(VMContext *vm, BppDialect *d) {
    (void)vm;
    if (!d) return NULL;
    HalContext *hal = hal_get();
    char *buf = NULL;
    if (hal && hal->mem.alloc) {
        buf = (char *)hal->mem.alloc(1024);
    }
    if (!buf) return NULL;
    runtime_memset(buf, 0, 1024);
    runtime_snprintf(buf, 1024, "Dialect: %s\nComment: %c\nSeparator: %c\nBase: %d\n",
             d->name, d->comment_char, d->stmt_separator, d->default_array_base);
    return buf;
}

