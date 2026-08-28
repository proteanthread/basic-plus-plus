// FILENAME: map.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the MAP statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CORE_MAP_H
#define STATEMENTS_CORE_MAP_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdbool.h>

#define BASIC_MAX_MAP_BUFFERS 32
#define BASIC_MAX_MAP_FIELDS  64
#define BASIC_MAX_MAP_SIZE    4096

typedef enum {
    MAP_TYPE_DOUBLE,
    MAP_TYPE_SINGLE,
    MAP_TYPE_INTEGER,
    MAP_TYPE_STRING
} MapFieldType;

typedef struct {
    char         var_name[64];
    MapFieldType type;
    int          offset;
    int          length;
} MapField;

typedef struct {
    char          map_name[64];
    int           total_length;
    int           field_count;
    MapField      fields[BASIC_MAX_MAP_FIELDS];
    unsigned char buffer[BASIC_MAX_MAP_SIZE];
} MapBuffer;

// Statement Handlers
BppError stmt_map_handler(VMContext *vm, LexerContext *lex);
BppError stmt_mapend_handler(VMContext *vm, LexerContext *lex);
void stmt_map_register(void);

// MAP Buffer Registry Management
bool map_registry_add_field(VMContext *vm, const char *map_name, const char *var_name, MapFieldType type, int length);
bool map_bind_channel(VMContext *vm, int channel, const char *map_name);
const char *map_get_channel_map(VMContext *vm, int channel);
MapBuffer *map_get_buffer(VMContext *vm, const char *map_name);
void map_sync_to_variables(VMContext *vm, const char *map_name, const unsigned char *raw_buf, int buf_len);
void map_sync_from_variables(VMContext *vm, const char *map_name, unsigned char *raw_buf, int buf_len);
void map_registry_clear_all(VMContext *vm);

#endif // STATEMENTS_CORE_MAP_H
