// FILENAME: spec.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (error.c, spec.c)
// NEEDED BY: libengine (category.c, context.c, control.c, data.c)
// NEEDED BY: libengine (events_internal.h, exec_dispatch.c, exec_internal.h)
// NEEDED BY: libengine (help.c, vm_internal.h)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides core logic and interface definitions for spec within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_SPEC_H
#define RUNTIME_SPEC_H

#include <stdbool.h>

#include "lexer/lexer.h"
#include "vm/vm.h"

#define MAX_SPECS 128

typedef enum {
    SPEC_CAT_STATEMENT = 0,
    SPEC_CAT_FUNCTION
} SpecCategory;

typedef struct {
    char         name[64];
    char         version[16];
    SpecCategory category;
    char         lib_path[256];
    char         required_level[32];
    BppKeywordId kw_id;
    bool         lib_loaded;
} SpecObject;

// @brief Initialize the specification registry.
void spec_registry_init(void);

// @brief Load a specification from an external file (.spec or .yaml).
int spec_load_file(VMContext *vm, const char *filename);

// @brief Load all companion libraries for registered specifications.
int spec_load_companion_libraries(VMContext *vm, const char *dir_part);

// @brief Register an inline specification from a metadata block context.
int spec_register_inline(VMContext *vm, const char *name, SpecCategory cat, const char *lib_path, const char *req_level);

// @brief Look up a specification object by its name.
SpecObject *spec_find_by_name(const char *name);

// @brief Look up a specification object by its assigned keyword ID.
SpecObject *spec_find_by_kw_id(BppKeywordId kw_id);

// @brief Get the total number of registered specifications.
int spec_get_count(void);

// @brief Retrieve a registered specification by index.
SpecObject *spec_get_by_index(int idx);

#endif // RUNTIME_SPEC_H
